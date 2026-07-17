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
**  9    3dfx      1.5.1.2     10/11/00 Brent           Forced check in to enforce
**       branching.
**  8    3dfx      1.5.1.1     08/27/00 Allen Hansen    look for "SPECULARFIX" -
**       non-textured specular tri's are now handled the same as textured specular
**       tri's (both are 2 pass), caught in WHQL in one of the clipping tests, the
**       problem was when a vert has a diffuse + spec > 255, we clamp to 255 and
**       the hardware then interpolates along that edge.  What really should happen
**       is the hardware will add diffuse & spec at each pixel and clamp.  This is
**       not a T&L bug, it's just something that has always been wrong.
**  7    3dfx      1.5.1.0     07/11/00 Steve Rogers    Fixing PRS 8371: Flat
**       shaded specular on textures now works.
**  6    3dfx      1.5         01/18/00 Bob Seitsinger  Changes to correctly write
**       to selected chip components (fbi,tmu0,tmu1). Chip component selection
**       occurred correctly for command packet header  writes, but needed to also
**       be done for data packet writes.
**  5    3dfx      1.4         11/12/99 Chris W. Shaw   Fix for many of the DCT 263
**       TextureStage tests.  PRS 11235
**  4    3dfx      1.3         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  3    3dfx      1.2         10/22/99 Chris W. Shaw   Add IGX_SPECULAR_FIX(2) to
**       Napalm source.  Now DCT250-BetaPreview-DriverScenario passes 3200 and
**       fails 1153 tests (instead of passing only 2753 and failing 1601).
**  2    3dfx      1.1         10/01/99 Christopher Wilcox Removed P6FENCE macros,
**       which are no longer necessary even when !CMDFIFO, since register space is
**       not write combined.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     7/13/99 2:27p Cshaw
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
** 
** 2     6/04/99 10:34a Cshaw
** Updated code for 2pass specular with combineMode.
** 
** 1     6/02/99 6:44a Michael
** Branch from H3
** 
** 50    5/24/99 5:05p Bseitsin
** Removal of Antialiasing code.
** 
** 49    4/13/99 8:45a Russ
** for NT direct write builds, redefine SET macro to DirectX style since
** it clashes with the NT gdi SET macro
**
** 48    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 47    12/09/98 6:38p Sreid
** Merged latest K6 optimization code from Metabyte
**
** 46    12/09/98 6:38a Russ
** NT5 D3D changes for Banshee
**
** 45    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
**
** 44    11/22/98 9:05p Andrew
** Changes to support multi-monitor
**
** 43    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
**
** 42    10/07/98 10:42p Adrians
** Added K6-2 optimisations.
**
** 40    10/07/98 9:05a stit (Metabyte, Inc)
** AMD K6-2 (MMX+3DNOW) optimization
**
** 39    10/03/98 5:18p Adrians
** Fix for two pass specular with zWriteEnable set to FALSE.
** Fix for two pass specular with fog.
**
** 38    8/28/98 4:39p Martin
** Copy execute-buffer super-sampling AA implementation into
** drawPrimitive.
**
** 37    8/28/98 2:45p Martin
** fix voodoo2 compile
**
** 36    8/28/98 2:33p Martin
** Conditionalize banshee compilation.
**
** 35    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
**
** No more conditional compilation of AA.
**
** 34    8/25/98 2:08p Martin
** Copy 2 triangle flavors (IZ & IZT), and make them AA specific for super
** sampling.
**
** 33    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
**
** 32    7/24/98 1:37p Hohn
**
** 31    7/23/98 8:15p Hanson
** Added AA into main tree
**
** 30    6/30/98 5:29p Miriam
** Performance optimizations.
**
** 29    6/01/98 12:20p Adrians
** Added aa to DrawPrimitives.
**
** 27    5/26/98 7:23p Adrians
** Added antialiasing.
**
** 26    5/07/98 6:43p Adrians
** Added performance test code.
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 24    3/27/98 4:04p Adrians
** Triangle flavour CMDFIFO_CHECKROOM optimisations.
**
** 23    3/27/98 1:44p Adrians
** Code tidyup.
** Removed palette and texture clamp compile options.
**
** 22    3/27/98 11:19a Adrians
** Removed unused compile options.
**
** 21    2/19/98 2:00p Miriam
** Fixed two problems with DirectWrite mode, wrong flag for strip and
** needed some braces around else statements because direct write macro
** generates multiple statements.
**
** 20    2/14/98 3:20p Adrians
** Removed some old debug code.
**
** 19    1/28/98 4:22p Adrians
** Added 0.5 to every vertex x,y and half a texel to s,t.
 *
 * 18    1/15/98 7:23p Adrians
 * Triangles flavours will now compile when using Direct Writes.
 *
 * 17    12/10/97 11:46a Adrians
 * Removed the 0.5 we were adding to every vertex.
 *
 * 16    11/11/97 9:26p Adrians
 * Added 3 more triangle flavours (I, IT, C).
 *
 * 15    11/09/97 2:51p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 *
 * 14    10/27/97 5:39p Adrians
 * Added packet 3 to DrawPrimitives, Fans, Strips & Tri's.
 * Added packet 3 to lines and points.
 * Added texture clamping to stw (build option tc=1).
 * Fix to DrawPrimitive fog.
 *
 * 13    10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 *
 * 12    10/09/97 10:23a Adrians
 * Now have a single SETPH macro.  Tidy-up of macro code.
 *
 * 11    10/07/97 1:39p Adrians
 * Optimisation to myRenderPrimitive setup.
 * New Caching enable call changes.
 *
 * 9     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 *
 * 8     9/26/97 11:45a Adrians
 * Now supports proper cmdfifo packet 3 in these modules.
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

//#define FXPERF
#ifdef FXPERF
#include "tsc.h"
#endif

#if defined( PERFTEST )
  #undef SETPH
  #undef SETPD
  #undef SETFPD
  #define FIFO_NOP  0

  #if( PERFTEST == PCI_NOP )
    #define SETPH( hwPtr, data )              SETCF( hwPtr, FIFO_NOP )
    #define SETPD( hwPtr, hwRegister, data )  SETCF( hwPtr, FIFO_NOP )
    #define SETFPD( hwPtr, hwRegister, data ) SETCF( hwPtr, FIFO_NOP )
  #else // PERFTEST == PCI_NULL
    #define SETPH( hwPtr, data )
    #define SETPD( hwPtr, hwRegister, data )
    #define SETFPD( hwPtr, hwRegister, data )
  #endif
#endif

//------------------------------------------------------------------------------
//
//  Triangle Primitive Processing - NO support for specular, fog and alpha blending
//
//------------------------------------------------------------------------------

void __stdcall fpDrawTriangleAll(RC*           pRc,
                                 WORD          count,
                                 LPD3DTRIANGLE tri,
                                 LPD3DTLVERTEX vertices)
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
  D3DCOLOR    aColor, bColor, cColor, aSColor, bSColor, cSColor;
  FxU32       setupFlag = pRc->sst.sSetupMode;
  float       s1, t1, s2, t2, s3, t3;
  float       fogA, fogB, fogC;
  float       area;
  int         sign;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for largest block to be used.
#ifdef IGX_SPECULAR_FIX
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE + (8 * 3)) + ((PH1_SIZE * 7) + 8) + (PH3_SIZE + (8 * 3)) );
#else
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE + (8 * 3)) + ((PH1_SIZE * 7) + 8) + (PH3_SIZE + (5 * 3)) );
#endif

      if ( pRc->shadeMode == D3DSHADE_FLAT )
      {
#if 0	// SPECULARFIX!!!
        if ( (pRc->specular) && (pRc->texture == 0) )
          CLAMP888( aColor, pA->color, pA->specular );
        else
#endif
		{
          aColor = pA->color;

          // Flat shaded specular should also work
		  aSColor = pA->specular;
		  cSColor = bSColor = aSColor;
		}

        bColor = (aColor & 0x00FFFFFF) | (pB->color & 0xFF000000);
        cColor = (aColor & 0x00FFFFFF) | (pC->color & 0xFF000000);

      }
      else
      {
#if 0	// SPECULARFIX!!!
        if ( (pRc->specular) && (pRc->texture == 0) )
        {
          CLAMP888(aColor, pA->color, pA->specular);
          CLAMP888(bColor, pB->color, pB->specular);
          CLAMP888(cColor, pC->color, pC->specular);
        }
        else
#endif
        {
          aColor = pA->color;
          bColor = pB->color;
          cColor = pC->color;

		  aSColor = pA->specular;
		  bSColor = pB->specular;
		  cSColor = pC->specular;
        }
      }

      if (pRc->state & STATE_REQUIRES_VERTEXFOG)
      {
        // need to put the alpha component of specular into iterated alpha
        // and use as Afog.
        //D3DPRINT( 255,"Fog: Specular A=0x%x B=0x%x C=0x%x  fog1=%d fog2=%d fog3=%d",
        //    pA->specular, pB->specular, pC->specular,
        //    RGBA_GETALPHA(pA->specular),RGBA_GETALPHA(pB->specular),RGBA_GETALPHA(pC->specular) );
        fogA = (float)(255 - RGBA_GETALPHA(pA->specular));
        fogB = (float)(255 - RGBA_GETALPHA(pB->specular));
        fogC = (float)(255 - RGBA_GETALPHA(pC->specular));
       }
      else if (pRc->state & STATE_REQUIRES_HWFOG)
      {
        fogA = pA->rhw;
        fogB = pB->rhw;
        fogC = pC->rhw;
      }

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
        float lscaleS, lscaleT;

        s1 = pA->tu;
        t1 = pA->tv;

        s2 = pB->tu;
        t2 = pB->tv;

        s3 = pC->tu;
        t3 = pC->tv;

        lscaleS = pRc->sst.scaleS;
        lscaleT = pRc->sst.scaleT;

        // two ways to texture. D3D wraps its textures going around the other direction
        // so we need to adjust S and T in order to get the correct result
        GET_ST(s, t, pRc->wrapU, pRc->wrapV);

        // compute the slope of s t and w
        // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
        if (pRc->texturePerspective)
        {
          s1 = ( ( s1 * lscaleS ) + TEXEL_SOFFSET ) * pA->rhw;
          s2 = ( ( s2 * lscaleS ) + TEXEL_SOFFSET ) * pB->rhw;
          s3 = ( ( s3 * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw;

          t1 = ( ( t1 * lscaleT ) + TEXEL_TOFFSET ) * pA->rhw;
          t2 = ( ( t2 * lscaleT ) + TEXEL_TOFFSET ) * pB->rhw;
          t3 = ( ( t3 * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw;
        }
        // if the texture is not prespective correct then the w is effectively equal to 1
        else
        {
          s1 = ( s1 * lscaleS ) + TEXEL_SOFFSET;
          s2 = ( s2 * lscaleS ) + TEXEL_SOFFSET;
          s3 = ( s3 * lscaleS ) + TEXEL_SOFFSET;

          t1 = ( t1 * lscaleT ) + TEXEL_TOFFSET;
          t2 = ( t2 * lscaleT ) + TEXEL_TOFFSET;
          t3 = ( t3 * lscaleT ) + TEXEL_TOFFSET;
        }
      } // texture

    //CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (8 * 3) );
    #ifdef CMDFIFO
      SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_TRI, 3, setupFlag, 1 ) );
    #else
      SET( cmdFifo, ghw0->sSetupMode, setupFlag);
    #endif

      SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, aColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, fogA );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
      if(setupFlag & SST_SETUP_ST0)
      {
        SETFPD( cmdFifo, ghw0->sSow0, s1 );
        SETFPD( cmdFifo, ghw0->sTow0, t1 );
      }
    #ifndef CMDFIFO
      if(_MM(drawGlobal))
        SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
    #endif

      SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, bColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, fogB );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
      if(setupFlag & SST_SETUP_ST0)
      {
        SETFPD( cmdFifo, ghw0->sSow0, s2 );
        SETFPD( cmdFifo, ghw0->sTow0, t2 );
      }
    #ifndef CMDFIFO
      if(_MM(drawGlobal))
        SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
    #endif

      SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, cColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pC->sz) );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, fogC );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
      if(setupFlag & SST_SETUP_ST0)
      {
        SETFPD( cmdFifo, ghw0->sSow0, s3 );
        SETFPD( cmdFifo, ghw0->sTow0, t3 );
      }
    #ifndef CMDFIFO
      if(_MM(drawGlobal))
        SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
    #endif

      //---------------------
      //
      // Specular on Textures
      //
      //---------------------
      // specular highlights on texture where the specular color is not black
#if 0	// SPECULARFIX!!!
      if (   (pRc->specular) && (pRc->texture != 0)
#else
      if (   (pRc->specular) 
#endif
           && (CI_MASKALPHA(pA->specular | pB->specular | pC->specular) != 0))
      {
        // if specular is turned on then we need to add in the specular color
        // and this means re-render the triangle gouraud shaded using the specular
        // color information and then add it to the textured triangle we just rendered.
        // We can't just add it to the triangle color because this color would be
        // blended or replaced with the texture and highlights are added into textures.
        //
        ULONG fbzMode;
        ULONG combineModeFBI;
        ULONG fbzColorPath; 

        //CMDFIFO_CHECKROOM( cmdFifo, ((PH1_SIZE * 7) + 8) + (PH3_SIZE + (4 * 3)) );
        // Where FOG() = fog function
        // FOG(T1 + T2) = AlphaFog * FogColor + (1 - AlphaFog)[T1 +T2]
        //    Pass 1    = AlphaFog * FogColor + (1 - AlphaFog)T1
        //    Pass 2    =                       (1 - AlphaFog)T2
        if (pRc->fogEnable)
        {
          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
          SETPD( cmdFifo, ghw->fogMode, (pRc->sst.fogMode & ~SST_FOGMULT) | SST_FOGADD);
        }

        // let's just add in the color ignoring the effect on alpha blending for
        // the time being
        // (Src * 1 + Dst * 1)
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, alphaMode, 0xF ) );
        if (!pRc->alphaBlendEnable)
        {
            SETPD( cmdFifo, ghw->alphaMode, (SST_ENALPHABLEND | (SST_A_ONE << SST_RGBSRCFACT_SHIFT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
        }
        else
        {
            SETPD( cmdFifo, ghw->alphaMode, ((pRc->sst.alphaMode & ~SST_RGBDSTFACT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
        }

        if ( pRc->zEnable )
        {
          if( pRc->zWriteEnable )
          {
            // if z-buffering then this triangle z values equals the values written on pass 1
            fbzMode = (pRc->sst.fbzMode & ~(SST_ZFUNC_LT | SST_ZFUNC_GT)) | SST_ZFUNC_EQ;

            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzMode, 0xF ) );
            SETPD( cmdFifo, ghw->fbzMode, fbzMode );
          }
        }

#ifdef IGX_SPECULAR_FIX
        // texture mapping off and texture blending off but keep alpha the same
        fbzColorPath = (pRc->sst.fbzColorPath & ~(SST_RGBSELECT | SST_CCOMBINE)) | SST_RGBSEL_RGBA;
            
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
        SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );

        if (pRc->alphaBlendEnable)
        {
          #ifdef CMDFIFO
            SETPH( cmdFifo, CMDFIFO_BUILD_PK3( 0, 3, setupFlag, 1 ) );
          #else
            SET( cmdFifo, ghw0->sSetupMode, setupFlag);
          #endif
        }
        else
        {
          #ifdef CMDFIFO
            SETPH( cmdFifo, CMDFIFO_BUILD_PK3( 0, 3, (setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0)), 1 ) );
          #else
            SET( cmdFifo, ghw0->sSetupMode, (setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0)));
          #endif
        }
#else
        // texture mapping off and texture blending off
        if (IS_NAPALM) { //NAPALM_CU
            combineModeFBI = ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI; 
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
        }
        else {
            fbzColorPath = SST_RGBSEL_RGBA;
            if (pRc->subPixel == TRUE)
            fbzColorPath |= SST_PARMADJUST;
    
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );
        }
        
        // if subpixel is on then must resend vertices
        // I interlaced this above to spread out the PCI writes.

      #ifdef CMDFIFO
        SETPH( cmdFifo, CMDFIFO_BUILD_PK3( 0, 3, (setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0)), 1 ) );
      #else
        SET( cmdFifo, ghw0->sSetupMode, (setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0)));
      #endif
#endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, aSColor );
        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, fogA );

#ifdef IGX_SPECULAR_FIX
        if (pRc->alphaBlendEnable)
        {
            if(setupFlag & SST_SETUP_W0)
              SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
            if(setupFlag & SST_SETUP_ST0)
            {
              SETFPD( cmdFifo, ghw0->sSow0, s1 );
              SETFPD( cmdFifo, ghw0->sTow0, t1 );
            }
        }
#endif

      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, bSColor );
        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, fogB );

#ifdef IGX_SPECULAR_FIX
        if (pRc->alphaBlendEnable)
        {
            if(setupFlag & SST_SETUP_W0)
              SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
            if(setupFlag & SST_SETUP_ST0)
            {
              SETFPD( cmdFifo, ghw0->sSow0, s2 );
              SETFPD( cmdFifo, ghw0->sTow0, t2 );
            }
        }
#endif

      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, cSColor );
        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pC->sz) );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, fogC );

#ifdef IGX_SPECULAR_FIX
        if (pRc->alphaBlendEnable)
        {
            if(setupFlag & SST_SETUP_W0)
              SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
            if(setupFlag & SST_SETUP_ST0)
            {
              SETFPD( cmdFifo, ghw0->sSow0, s3 );
              SETFPD( cmdFifo, ghw0->sTow0, t3 );
            }
        }
#endif

      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        // restore everything back to the way it was
        if (IS_NAPALM) { //NAPALM_CU
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, pRc->sst.combineModeFBI );
        }
        else {
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
        }
        if (pRc->fogEnable)
        {
          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogMode, 0xF ) );
          SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
        }

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, alphaMode, 0xF ) );
        SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
        SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
      }
    }

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

#ifdef _3DNOWDPFUNCS
void __stdcall fpTriIZ_Orig(RC*           pRc,
                            WORD          count,
                            LPD3DTRIANGLE tri,
                            LPD3DTLVERTEX vertices)
#else
void __stdcall fpTriIZ(RC*           pRc,
                       WORD          count,
                       LPD3DTRIANGLE tri,
                       LPD3DTLVERTEX vertices)
#endif // _3DNOWDPFUNCS
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
#if defined( CMDFIFO )
  FxU32       pk3Hdr = CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrFan = CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrStrip = CMDFIFO_BUILD_PK3( CMD_CONT, 1, pRc->sst.sSetupMode, 1 );
#else
  FxU32       pk3HdrFan = pRc->sst.sSetupMode | SST_SETUP_FAN;
  FxU32       pk3HdrStrip = pRc->sst.sSetupMode;
#endif
  float       area;
  int         sign, test = D3DTRIFLAG_ODD;
  BOOL        culled = TRUE;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for the largest block to be used.
      CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 4) );

      if( culled || ((tri->wFlags & 0x1F) < D3DTRIFLAG_ODD) )
      {
      #ifdef CMDFIFO
        SETPH( cmdFifo, pk3Hdr );
      #else
        SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan);
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pB->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pC->sz) );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        test = D3DTRIFLAG_ODD;
      }
      else
      {
        if( (tri->wFlags & 0x1F) == test )
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrStrip );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrStrip);
        #endif
          test ^= 0x1;
        }
        else
        {
        #if defined ( CMDFIFO )
          SETPH( cmdFifo, pk3HdrFan );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan);
        #endif
        }

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
      }

      culled = FALSE;
    }
    else
      culled = TRUE;

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

#ifdef _3DNOWDPFUNCS
void __stdcall fpTriIZS_Orig(RC*           pRc,
                             WORD          count,
                             LPD3DTRIANGLE tri,
                             LPD3DTLVERTEX vertices)
#else
void __stdcall fpTriIZS(RC*           pRc,
                        WORD          count,
                        LPD3DTRIANGLE tri,
                        LPD3DTLVERTEX vertices)
#endif // _3DNOWDPFUNCS
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
  D3DCOLOR    aColor, bColor, cColor;
#if defined( CMDFIFO )
  FxU32       pk3Hdr = CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrFan = CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrStrip = CMDFIFO_BUILD_PK3( CMD_CONT, 1, pRc->sst.sSetupMode, 1 );
#else
  FxU32       pk3HdrFan = pRc->sst.sSetupMode | SST_SETUP_FAN;
  FxU32       pk3HdrStrip = pRc->sst.sSetupMode;
#endif
  float       area;
  int         sign, test = D3DTRIFLAG_ODD;
  BOOL        culled = TRUE;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for the largest block to be used.
      CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 4) );

      if( culled || ((tri->wFlags & 0x1F) < D3DTRIFLAG_ODD) )
      {
        CLAMP888(aColor, pA->color, pA->specular);
        CLAMP888(bColor, pB->color, pB->specular);
        CLAMP888(cColor, pC->color, pC->specular);

      #if defined( CMDFIFO )
        SETPH( cmdFifo, pk3Hdr );
      #else
        SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan);
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, aColor);
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, bColor );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, cColor );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pC->sz) );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        test = D3DTRIFLAG_ODD;
      }
      else
      {
        CLAMP888(cColor, pC->color, pC->specular);

        if( (tri->wFlags & 0x1F) == test )
        {
        #if defined ( CMDFIFO )
          SETPH( cmdFifo, pk3HdrStrip );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrStrip);
        #endif
          test ^= 0x1;
        }
        else
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrFan );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan);
        #endif
        }

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, cColor );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
      }

      culled = FALSE;
    }
    else
      culled = TRUE;

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

#ifdef _3DNOWDPFUNCS
void __stdcall fpTriIZT_Orig(RC*           pRc,
                             WORD          count,
                             LPD3DTRIANGLE tri,
                             LPD3DTLVERTEX vertices)
#else
void __stdcall fpTriIZT(RC*           pRc,
                        WORD          count,
                        LPD3DTRIANGLE tri,
                        LPD3DTLVERTEX vertices)
#endif // _3DNOWDPFUNCS
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
#if defined( CMDFIFO )
  FxU32       pk3Hdr = CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrFan = CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrStrip = CMDFIFO_BUILD_PK3( CMD_CONT, 1, pRc->sst.sSetupMode, 1 );
#else
  FxU32       pk3HdrFan = pRc->sst.sSetupMode | SST_SETUP_FAN;
  FxU32       pk3HdrStrip = pRc->sst.sSetupMode;
#endif
  float       lscaleS = pRc->sst.scaleS, lscaleT = pRc->sst.scaleT, area;
  int         sign, test = D3DTRIFLAG_ODD;
  BOOL        culled = TRUE;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for the largest block to be used.
      CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 7) );

      if( culled || ((tri->wFlags & 0x1F) < D3DTRIFLAG_ODD) )
      {
      #if defined( CMDFIFO )
        SETPH( cmdFifo, pk3Hdr );
      #else
        SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color);
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pA->sz ) );
        SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pA->tu * lscaleS ) + TEXEL_SOFFSET ) * pA->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pA->tv * lscaleT ) + TEXEL_TOFFSET ) * pA->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pB->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pB->sz ) );
        SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pB->tu * lscaleS ) + TEXEL_SOFFSET ) * pB->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pB->tv * lscaleT ) + TEXEL_TOFFSET ) * pB->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
        SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        test = D3DTRIFLAG_ODD;
      }
      else
      {
        if( (tri->wFlags & 0x1F) == test )
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrStrip );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrStrip );
        #endif
          test ^= 0x1;
        }
        else
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrFan );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
        #endif
        }

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
        SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
      }
      culled = FALSE;
    }
    else
      culled = TRUE;

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

#ifdef _3DNOWDPFUNCS
void __stdcall fpTriIZTS_Orig(RC*           pRc,
                             WORD          count,
                             LPD3DTRIANGLE tri,
                             LPD3DTLVERTEX vertices)
#else
void __stdcall fpTriIZTS(RC*           pRc,
                        WORD          count,
                        LPD3DTRIANGLE tri,
                        LPD3DTLVERTEX vertices)
#endif // _3DNOWDPFUNCS
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
  D3DCOLOR    aColor, bColor, cColor;
#if defined( CMDFIFO )
  FxU32       pk3Hdr = CMDFIFO_BUILD_PK3( CMD_START, 3, pRc->sst.sSetupMode, 1 );
  FxU32       pk3HdrS = CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode & ~(SST_SETUP_ST0 | SST_SETUP_W0)), 1 );
#else
  FxU32       pk3Hdr = pRc->sst.sSetupMode;
  FxU32       pk3HdrS = pRc->sst.sSetupMode & ~(SST_SETUP_ST0 | SST_SETUP_W0);
#endif
  float       lscaleS = pRc->sst.scaleS, lscaleT = pRc->sst.scaleT;
  float       area;
  int         sign;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      CLAMP888(aColor, pA->color, pA->specular);
      CLAMP888(bColor, pB->color, pB->specular);
      CLAMP888(cColor, pC->color, pC->specular);

      // Checkroom for largest block to be used.
#ifdef IGX_SPECULAR_FIX
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE + (3 * 7)) + ((PH1_SIZE * 5) + 6) + (PH3_SIZE + (3 * 8)) );
#else
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE + (3 * 7)) + ((PH1_SIZE * 5) + 6) + (PH3_SIZE + (3 * 4)) );
#endif

    #if defined ( CMDFIFO )
      //CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 7) );
      SETPH( cmdFifo, pk3Hdr );
    #else
      SET( cmdFifo, ghw0->sSetupMode, pk3Hdr );
    #endif

      SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
      SETPD( cmdFifo, ghw0->sARGB, aColor);
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pA->sz ) );
      SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
      SETFPD( cmdFifo, ghw0->sSow0, ( ( pA->tu * lscaleS ) + TEXEL_SOFFSET ) * pA->rhw );
      SETFPD( cmdFifo, ghw0->sTow0, ( ( pA->tv * lscaleT ) + TEXEL_TOFFSET ) * pA->rhw );
    #ifndef CMDFIFO
      if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
    #endif

      SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
      SETPD( cmdFifo, ghw0->sARGB, bColor );
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pB->sz ) );
      SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
      SETFPD( cmdFifo, ghw0->sSow0, ( ( pB->tu * lscaleS ) + TEXEL_SOFFSET ) * pB->rhw );
      SETFPD( cmdFifo, ghw0->sTow0, ( ( pB->tv * lscaleT ) + TEXEL_TOFFSET ) * pB->rhw );
    #ifndef CMDFIFO
      if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
    #endif

      SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
      SETPD( cmdFifo, ghw0->sARGB, cColor );
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
      SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
      SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
      SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
    #ifndef CMDFIFO
      if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
    #endif

      //---------------------
      //
      // Specular on Textures
      //
      //---------------------
      // specular highlights on texture where the specular color is not black
      if ( (CI_MASKALPHA(pA->specular | pB->specular | pC->specular) != 0))
      {
        // if specular is turned on then we need to add in the specular color
        // and this means re-render the triangle gouraud shaded using the specular
        // color information and then add it to the textured triangle we just rendered.
        // We can't just add it to the triangle color because this color would be
        // blended or replaced with the texture and highlights are added into textures.
        //
        ULONG fbzMode;
        ULONG combineModeFBI;
        ULONG fbzColorPath; 

        //CMDFIFO_CHECKROOM( cmdFifo, ((PH1_SIZE * 5) + 6) + (PH3_SIZE + (3 * 4)) );

        // let's just add in the color ignoring the effect on alpha blending for
        // the time being
        // (Src * 1 + Dst * 1)
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, alphaMode, 0xF ) );
        if (!pRc->alphaBlendEnable)
        {
            SETPD( cmdFifo, ghw->alphaMode, (SST_ENALPHABLEND | (SST_A_ONE << SST_RGBSRCFACT_SHIFT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
        }
        else
        {
            SETPD( cmdFifo, ghw->alphaMode, ((pRc->sst.alphaMode & ~SST_RGBDSTFACT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
        }

        if( pRc->zWriteEnable )
        {
          // if z-buffering then this triangle z values equals the values written on pass 1
          fbzMode = (pRc->sst.fbzMode & ~(SST_ZFUNC_LT | SST_ZFUNC_GT)) | SST_ZFUNC_EQ;

          SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzMode, 0xF ) );
          SETPD( cmdFifo, ghw->fbzMode, fbzMode );
        }

#ifdef IGX_SPECULAR_FIX
        // texture mapping off and texture blending off but keep alpha the same
        if (IS_NAPALM) { //NAPALM_CU
            combineModeFBI = ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_CCA_OTHERSELECT_TA | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI; 
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
        }
        else {
            fbzColorPath = (pRc->sst.fbzColorPath & ~(SST_RGBSELECT | SST_CCOMBINE)) | SST_RGBSEL_RGBA;
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );
        }
#else
        // texture mapping off and texture blending off
        if (IS_NAPALM) { //NAPALM_CU
            combineModeFBI = ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI; 
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
        }
        else {
            fbzColorPath = SST_RGBSEL_RGBA;

            if (pRc->subPixel == TRUE)
            fbzColorPath |= SST_PARMADJUST;

            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );
        }
#endif
        // if subpixel is on then must resend vertices
        // I interlaced this above to spread out the PCI writes.

      #if defined( CMDFIFO )
        SETPH( cmdFifo, pk3HdrS );
      #else
        SET( cmdFifo, ghw0->sSetupMode, pk3HdrS );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->specular );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );

#ifdef IGX_SPECULAR_FIX
        if (pRc->alphaBlendEnable)
        {
            SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
            SETFPD( cmdFifo, ghw0->sSow0, ( ( pA->tu * lscaleS ) + TEXEL_SOFFSET ) * pA->rhw );
            SETFPD( cmdFifo, ghw0->sTow0, ( ( pA->tv * lscaleT ) + TEXEL_TOFFSET ) * pA->rhw );
        }
#endif

      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pB->specular );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );

#ifdef IGX_SPECULAR_FIX
        if (pRc->alphaBlendEnable)
        {
            SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
            SETFPD( cmdFifo, ghw0->sSow0, ( ( pB->tu * lscaleS ) + TEXEL_SOFFSET ) * pB->rhw );
            SETFPD( cmdFifo, ghw0->sTow0, ( ( pB->tv * lscaleT ) + TEXEL_TOFFSET ) * pB->rhw );
        }
#endif

      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->specular );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pC->sz) );

#ifdef IGX_SPECULAR_FIX
        if (pRc->alphaBlendEnable)
        {
            SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
            SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
            SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
        }
#endif

      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        // restore everything back to the way it was
        if (IS_NAPALM) { //NAPALM_CU
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, pRc->sst.combineModeFBI );
        }
        else {
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
        }
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, alphaMode, 0xF ) );
        SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
        SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
      }
    }

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

#ifdef _3DNOWDPFUNCS
void __stdcall fpTriIZTH_Orig(RC*           pRc,
                              WORD          count,
                              LPD3DTRIANGLE tri,
                              LPD3DTLVERTEX vertices)
#else
void __stdcall fpTriIZTH(RC*           pRc,
                         WORD          count,
                         LPD3DTRIANGLE tri,
                         LPD3DTLVERTEX vertices)
#endif // _3DNOWDPFUNCS
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
#if defined( CMDFIFO )
  FxU32       pk3Hdr = CMDFIFO_BUILD_PK3( CMD_START, 3, ((pRc->sst.sSetupMode | SST_SETUP_FAN) & ~SST_SETUP_W0), 1 );
  FxU32       pk3HdrFan = CMDFIFO_BUILD_PK3( CMD_CONT, 1, ((pRc->sst.sSetupMode | SST_SETUP_FAN) & ~SST_SETUP_W0), 1 );
  FxU32       pk3HdrStrip = CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode & ~SST_SETUP_W0), 1 );
#else
  FxU32       pk3HdrFan = (pRc->sst.sSetupMode | SST_SETUP_FAN) & ~SST_SETUP_W0;
  FxU32       pk3HdrStrip = pRc->sst.sSetupMode & ~SST_SETUP_W0;
#endif
  float       lscaleS = pRc->sst.scaleS, lscaleT = pRc->sst.scaleT;
  float       area;
  int         sign, test = D3DTRIFLAG_ODD;
  BOOL        culled = TRUE;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for the largest block to be used.
      CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 7) );

      if( culled || ((tri->wFlags & 0x1F) < D3DTRIFLAG_ODD) )
      {
      #if defined( CMDFIFO )
        SETPH( cmdFifo, pk3Hdr );
      #else
        SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color);
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pA->sz ) );
        SETFPD( cmdFifo, ghw0->sOowfbi, pA->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pA->tu * lscaleS ) + TEXEL_SOFFSET ) * pA->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pA->tv * lscaleT ) + TEXEL_TOFFSET ) * pA->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pB->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pB->sz ) );
        SETFPD( cmdFifo, ghw0->sOowfbi, pB->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pB->tu * lscaleS ) + TEXEL_SOFFSET ) * pB->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pB->tv * lscaleT ) + TEXEL_TOFFSET ) * pB->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
        SETFPD( cmdFifo, ghw0->sOowfbi, pC->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        test = D3DTRIFLAG_ODD;
      }
      else
      {
        if( (tri->wFlags & 0x1F) == test )
        {
        #if defined ( CMDFIFO )
          SETPH( cmdFifo, pk3HdrStrip );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrStrip );
        #endif
          test ^= 0x1;
        }
        else
        {
        #if defined ( CMDFIFO )
          SETPH( cmdFifo, pk3HdrFan );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
        #endif
        }

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
        SETFPD( cmdFifo, ghw0->sOowfbi, pC->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
      }

      culled = FALSE;
    }
    else
      culled = TRUE;

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

void __stdcall fpTriI(RC*           pRc,
                      WORD          count,
                      LPD3DTRIANGLE tri,
                      LPD3DTLVERTEX vertices)
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
#if defined( CMDFIFO )
  FxU32       pk3Hdr = CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrFan = CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrStrip = CMDFIFO_BUILD_PK3( CMD_CONT, 1, pRc->sst.sSetupMode, 1 );
#else
  FxU32       pk3HdrFan = pRc->sst.sSetupMode | SST_SETUP_FAN;
  FxU32       pk3HdrStrip = pRc->sst.sSetupMode;
#endif
  float       area;
  int         sign, test = D3DTRIFLAG_ODD;
  BOOL        culled = TRUE;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for the largest block to be used.
      CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 4) );

      if( culled || ((tri->wFlags & 0x1F) < D3DTRIFLAG_ODD) )
      {
      #if defined( CMDFIFO )
        SETPH( cmdFifo, pk3Hdr );
      #else
        SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pB->color );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        test = D3DTRIFLAG_ODD;
      }
      else
      {
        if( (tri->wFlags & 0x1F) == test )
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrStrip );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrStrip );
        #endif
          test ^= 0x1;
        }
        else
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrFan );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
        #endif
        }

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
      }

      culled = FALSE;
    }
    else
      culled = TRUE;

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

#ifdef _3DNOWDPFUNCS
void __stdcall fpTriIT_Orig(RC*           pRc,
                            WORD          count,
                            LPD3DTRIANGLE tri,
                            LPD3DTLVERTEX vertices)
#else
void __stdcall fpTriIT(RC*           pRc,
                       WORD          count,
                       LPD3DTRIANGLE tri,
                       LPD3DTLVERTEX vertices)
#endif // _3DNOWDPFUNCS
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
#if defined( CMDFIFO )
  FxU32       pk3Hdr = CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrFan = CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrStrip = CMDFIFO_BUILD_PK3( CMD_CONT, 1, pRc->sst.sSetupMode, 1 );
#else
  FxU32       pk3HdrFan = pRc->sst.sSetupMode | SST_SETUP_FAN;
  FxU32       pk3HdrStrip = pRc->sst.sSetupMode;
#endif
  float       lscaleS = pRc->sst.scaleS, lscaleT = pRc->sst.scaleT, area;
  int         sign, test = D3DTRIFLAG_ODD;
  BOOL        culled = TRUE;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for the largest block to be used.
      CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 7) );

      if( culled || ((tri->wFlags & 0x1F) < D3DTRIFLAG_ODD) )
      {
      #if defined( CMDFIFO )
        SETPH( cmdFifo, pk3Hdr );
      #else
        SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color);
        SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pA->tu * lscaleS ) + TEXEL_SOFFSET ) * pA->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pA->tv * lscaleT ) + TEXEL_TOFFSET ) * pA->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pB->color );
        SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pB->tu * lscaleS ) + TEXEL_SOFFSET ) * pB->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pB->tv * lscaleT ) + TEXEL_TOFFSET ) * pB->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
        SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        test = D3DTRIFLAG_ODD;
      }
      else
      {
        if( (tri->wFlags & 0x1F) == test )
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrStrip );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrStrip );
        #endif
          test ^= 0x1;
        }
        else
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrFan );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
        #endif
        }

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pC->color );
        SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
        SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
        SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
      }

      culled = FALSE;
    }
    else
      culled = TRUE;

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

void __stdcall fpTriC(RC*           pRc,
                      WORD          count,
                      LPD3DTRIANGLE tri,
                      LPD3DTLVERTEX vertices)
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB, *pC;
#if defined( CMDFIFO )
  FxU32       pk3Hdr = CMDFIFO_BUILD_PK3( CMD_START, 3, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrFan = CMDFIFO_BUILD_PK3( CMD_CONT, 1, (pRc->sst.sSetupMode | SST_SETUP_FAN), 1 );
  FxU32       pk3HdrStrip = CMDFIFO_BUILD_PK3( CMD_CONT, 1, pRc->sst.sSetupMode, 1 );
#else
  FxU32       pk3HdrFan = pRc->sst.sSetupMode | SST_SETUP_FAN;
  FxU32       pk3HdrStrip = pRc->sst.sSetupMode;
#endif
  float       area;
  int         sign, test = D3DTRIFLAG_ODD;
  BOOL        culled = TRUE;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  while (count > 0)
  {
    pA = &vertices[tri->v1];
    pB = &vertices[tri->v2];
    pC = &vertices[tri->v3];

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for the largest block to be used.
      CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 4) );

      if( culled || ((tri->wFlags & 0x1F) < D3DTRIFLAG_ODD) )
      {
      #if defined( CMDFIFO )
        SETPH( cmdFifo, pk3Hdr );
      #else
        SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        test = D3DTRIFLAG_ODD;
      }
      else
      {
        if( (tri->wFlags & 0x1F) == test )
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrStrip );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrStrip );
        #endif
          test ^= 0x1;
        }
        else
        {
        #if defined( CMDFIFO )
          SETPH( cmdFifo, pk3HdrFan );
        #else
          SET( cmdFifo, ghw0->sSetupMode, pk3HdrFan );
        #endif
        }

        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, pA->color );
      #ifndef CMDFIFO
        if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
      }

      culled = FALSE;
    }
    else
      culled = TRUE;

    tri++;
    count--;
  }

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

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
**  7    3dfx      1.3.1.2     10/11/00 Brent           Forced check in to enforce
**       branching.
**  6    3dfx      1.3.1.1     08/21/00 Allen Hansen    look for "SPECULARFIX" -
**       non-textured specular tri's are now handled the same as textured specular
**       tri's (both are 2 pass), caught in WHQL in one of the clipping tests, the
**       problem was when a vert has a diffuse + spec > 255, we clamp to 255 and
**       the hardware then interpolates along that edge.  What really should happen
**       is the hardware will add diffuse & spec at each pixel and clamp.  This is
**       not a T&L bug, it's just something that has always been wrong.
**  5    3dfx      1.3.1.0     07/11/00 Steve Rogers    Fixing PRS 8371: Flat
**       shaded specular on textures now works.
**  4    3dfx      1.3         01/18/00 Bob Seitsinger  Changes to correctly write
**       to selected chip components (fbi,tmu0,tmu1). Chip component selection
**       occurred correctly for command packet header  writes, but needed to also
**       be done for data packet writes.
**  3    3dfx      1.2         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
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
** 16    5/24/99 5:05p Bseitsin
** Removal of Antialiasing code.
** 
** 15    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 14    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
** 
** 13    11/22/98 9:03p Andrew
** Changes to support multi-monitor
** 
** 12    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
** 
** 11    10/06/98 11:59p Adrians
** Migrate the aaTestFlags from the DX6 driver to the DX5 driver.
** 
** 10    10/03/98 5:18p Adrians
** Fix for two pass specular with zWriteEnable set to FALSE.
** Fix for two pass specular with fog.
** 
** 9     8/28/98 4:39p Martin
** Copy execute-buffer super-sampling AA implementation into
** drawPrimitive.
** 
** 8     8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
** 
** No more conditional compilation of AA.
** 
** 7     8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
** 
** 6     7/24/98 1:37p Hohn
** 
** 5     6/01/98 12:20p Adrians
** Added aa to DrawPrimitives.
** 
** 4     5/07/98 6:43p Adrians
** Added performance test code.
** 
** 1     4/29/98 6:31p Adrians
** Created
** 
** 3     3/27/98 4:04p Adrians
** Triangle flavour CMDFIFO_CHECKROOM optimisations.
** 
** 2     3/27/98 1:44p Adrians
** Code tidyup.
** Removed palette and texture clamp compile options.
** 
** 1     3/27/98 11:19a Adrians
** New DrawPrimitive code.
*/

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

//-------------------------------------------------------------------

void __stdcall dpDrawStripAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  SETUP_PPDEV(pRc) 
  D3DTLVERTEX   *pA, *pB, *pC, *pF;
  float         s1, s2, s3, t1, t2, t3;
  D3DCOLOR      aColor, bColor, cColor, aSColor, bSColor, cSColor;
  float         fogA, fogB, fogC, area;
  int           sign;
  FxU32         setupFlag = pRc->sst.sSetupMode;
  BOOL          flip = 0;
  CMDFIFO_PROLOG(cmdFifo);
  
#if( PERFTEST == TRI_NULL )
  return;
#endif

  TRIALL_INIT();
  
  for( ; count > 0; count-- )
  {  
    TRIALL_NEXT();

    area = ((pA->sx - pB->sx) * (pB->sy - pC->sy)) - ((pB->sx - pC->sx) * (pA->sy - pB->sy));
    sign = (*(unsigned long *)&area) & 0x80000000;
    
    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE + (3 * 8)) + ((PH1_SIZE * 7) + 8) + (PH3_SIZE + (5 * 3)) );
      
      if ( pRc->shadeMode == D3DSHADE_FLAT )
      {    
#if 0	// SPECULARFIX!!!
        if ( (pRc->specular) && (pRc->texture == 0) )
          CLAMP888( aColor, pF->color, pF->specular );
        else
#endif
		{
          aColor = pF->color;

          // Flat shaded specular should also work
		  aSColor = pF->specular;
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
      
      // Texture Mapping 
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
      
      //CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 8) );
    #ifdef CMDFIFO    
      SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_TRI, 3, setupFlag, 1 ) );
    #else    
      SET( cmdFifo, ghw0->sSetupMode, setupFlag);
    #endif    

      //Send 1st vertex    
      SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, aColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pA->sz ) );
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

      // Send 2nd vertex
      SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, bColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pB->sz ) );
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

      // Send 3rd vertex
      SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, cColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
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
      
      // Specular on Textures
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
        SETPD( cmdFifo, ghw->alphaMode, (SST_ENALPHABLEND | (SST_A_ONE << SST_RGBSRCFACT_SHIFT) | (SST_A_ONE << SST_RGBDSTFACT_SHIFT)) );
          
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

        // texture mapping off and texture blending off
        if (IS_NAPALM) { //NAPALM_CU
            combineModeFBI = ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI; 
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
        }
        else {
            fbzColorPath = SST_RGBSEL_RGBA | SST_PARMADJUST;
          
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );
        }
      #ifdef CMDFIFO    
        SETPH( cmdFifo, CMDFIFO_BUILD_PK3( 0, 3, (setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0)), 1 ) );
      #else    
        SET( cmdFifo, ghw0->sSetupMode, (setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0)));
      #endif

        // Send 1st vertex      
        SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, aSColor );
        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pA->sz ) );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, fogA );
      #ifndef CMDFIFO      
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        // Send 2nd vertex
        SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, bSColor );
        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pB->sz ) );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, fogB );
      #ifndef CMDFIFO      
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        // Send 3rd vertex
        SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, cSColor );
        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, fogC );
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
  } // every triangle
  
  CMDFIFO_EPILOG( cmdFifo );
} // Independat Triangle rendering

//-------------------------------------------------------------------

void __stdcall dpDrawStripIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX   *pA, *pB, *pC;
  float         lscaleS = pRc->sst.scaleS, lscaleT = pRc->sst.scaleT;
  FxU32         setupFlag = pRc->sst.sSetupMode;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  TRI_INIT();  
  
  CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 7) );
#ifdef CMDFIFO    
  SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 3, setupFlag, 1 ) );
#else    
  SET( cmdFifo, ghw0->sSetupMode, setupFlag);
#endif    

  // Send 1st vertex
  SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pA->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
  SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
  SETFPD( cmdFifo, ghw0->sSow0, ( ( pA->tu * lscaleS ) + TEXEL_SOFFSET ) * pA->rhw );
  SETFPD( cmdFifo, ghw0->sTow0, ( ( pA->tv * lscaleT ) + TEXEL_TOFFSET ) * pA->rhw );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
#endif    

  // Send 2nd vertex
  SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pB->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
  SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
  SETFPD( cmdFifo, ghw0->sSow0, ( ( pB->tu * lscaleS ) + TEXEL_SOFFSET ) * pB->rhw );
  SETFPD( cmdFifo, ghw0->sTow0, ( ( pB->tv * lscaleT ) + TEXEL_TOFFSET ) * pB->rhw );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif

  // Send 3rd vertex
  SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pC->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pC->sz) );
  SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
  SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
  SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif
  
  for( --count; count > 0; --count )
  {
    TRI_NEXT();
    
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + 7 );
  #ifdef CMDFIFO    
    SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_CONT, 1, setupFlag, 1 ) );
  #else    
    SET( cmdFifo, ghw0->sSetupMode, setupFlag);
  #endif    
  
    // Send additional vertex for next triangle    
    SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
    SETPD( cmdFifo, ghw0->sARGB, pC->color );
    SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
    SETFPD( cmdFifo, ghw0->sOow0, pC->rhw );
    SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
    SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
  #ifndef CMDFIFO    
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
      
  } // Every Triangle

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

void __stdcall dpDrawStripIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX   *pA, *pB, *pC;
  FxU32         setupFlag = pRc->sst.sSetupMode;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  TRI_INIT();  
  
  CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 4) );
#ifdef CMDFIFO    
  SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 3, setupFlag, 1 ) );
#else    
  SET( cmdFifo, ghw0->sSetupMode, setupFlag);
#endif    

  // Send 1st vertex
  SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pA->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
#endif    

  // Send 2nd vertex
  SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pB->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif

  // Send 3rd vertex
  SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pC->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pC->sz) );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif
  
  for( --count; count > 0; --count )
  {
    TRI_NEXT();
    
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + 4 );
  #ifdef CMDFIFO    
    SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_CONT, 1, setupFlag, 1 ) );
  #else    
    SET( cmdFifo, ghw0->sSetupMode, setupFlag);
  #endif    
  
    // Send additional vertex for next triangle    
    SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
    SETPD( cmdFifo, ghw0->sARGB, pC->color );
    SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
  #ifndef CMDFIFO    
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
      
  } // Every Triangle

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

void __stdcall dpDrawStripIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX   *pA, *pB, *pC;
  float         lscaleS = pRc->sst.scaleS, lscaleT = pRc->sst.scaleT;
  FxU32         setupFlag = pRc->sst.sSetupMode & ~SST_SETUP_W0;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  TRI_INIT();  
  
  CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (3 * 7) );
#ifdef CMDFIFO    
  SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 3, setupFlag, 1 ) );
#else    
  SET( cmdFifo, ghw0->sSetupMode, setupFlag);
#endif    

  // Send 1st vertex
  SETFPD( cmdFifo, ghw0->sVx, pA->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pA->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pA->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
  SETFPD( cmdFifo, ghw0->sOowfbi, pA->rhw );
  SETFPD( cmdFifo, ghw0->sSow0, ( ( pA->tu * lscaleS ) + TEXEL_SOFFSET ) * pA->rhw );
  SETFPD( cmdFifo, ghw0->sTow0, ( ( pA->tv * lscaleT ) + TEXEL_TOFFSET ) * pA->rhw );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
#endif    

  // Send 2nd vertex
  SETFPD( cmdFifo, ghw0->sVx, pB->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pB->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pB->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
  SETFPD( cmdFifo, ghw0->sOowfbi, pB->rhw );
  SETFPD( cmdFifo, ghw0->sSow0, ( ( pB->tu * lscaleS ) + TEXEL_SOFFSET ) * pB->rhw );
  SETFPD( cmdFifo, ghw0->sTow0, ( ( pB->tv * lscaleT ) + TEXEL_TOFFSET ) * pB->rhw );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif

  // Send 3rd vertex
  SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
  SETPD( cmdFifo, ghw0->sARGB, pC->color );
  SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pC->sz) );
  SETFPD( cmdFifo, ghw0->sOowfbi, pC->rhw );
  SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
  SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
#ifndef CMDFIFO    
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif
  
  for( --count; count > 0; --count )
  {
    TRI_NEXT();
    
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + 7 );
  #ifdef CMDFIFO    
    SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_CONT, 1, setupFlag, 1 ) );
  #else    
    SET( cmdFifo, ghw0->sSetupMode, setupFlag);
  #endif    
  
    // Send additional vertex for next triangle    
    SETFPD( cmdFifo, ghw0->sVx, pC->sx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, pC->sy + PIXEL_OFFSET );
    SETPD( cmdFifo, ghw0->sARGB, pC->color );
    SETFPD( cmdFifo, ghw0->sVz, ZSCALE( pC->sz ) );
    SETFPD( cmdFifo, ghw0->sOowfbi, pC->rhw );
    SETFPD( cmdFifo, ghw0->sSow0, ( ( pC->tu * lscaleS ) + TEXEL_SOFFSET ) * pC->rhw );
    SETFPD( cmdFifo, ghw0->sTow0, ( ( pC->tv * lscaleT ) + TEXEL_TOFFSET ) * pC->rhw );
  #ifndef CMDFIFO    
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
      
  } // Every Triangle

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

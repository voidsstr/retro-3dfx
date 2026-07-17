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
** File name:   d6fani.c
**
** Description: Implements DrawPrimitive fan drawing functions.
**
** Information: This file is included multiple times by d6fan.c.
**              It's used as a template for generating indexed and
**              non-indexed versions of DrawPrimitive API calls.
**
**
** $Revision: 19$
** $Date: 10/11/00 8:48:50 PM$
**
** $Log: 
**  19   3dfx      1.13.2.0.1.310/11/00 Brent           Forced check in to enforce
**       branching.
**  18   3dfx      1.13.2.0.1.208/21/00 Allen Hansen    look for "SPECULARFIX" -
**       non-textured specular tri's are now handled the same as textured specular
**       tri's (both are 2 pass), caught in WHQL in one of the clipping tests, the
**       problem was when a vert has a diffuse + spec > 255, we clamp to 255 and
**       the hardware then interpolates along that edge.  What really should happen
**       is the hardware will add diffuse & spec at each pixel and clamp.  This is
**       not a T&L bug, it's just something that has always been wrong.
**  17   3dfx      1.13.2.0.1.107/24/00 Allen Hansen    whql bug fixes (T&L tests)
**  16   3dfx      1.13.2.0.1.007/11/00 Steve Rogers    Fixing PRS 8371: Flat
**       shaded specular on textures now works.
**  15   3dfx      1.13.2.0    05/10/00 Bob Seitsinger  Add some debug statements -
**       mostly D3DPRINTTEXCOORD statements.
**  14   3dfx      1.13        02/14/00 Chris W. Shaw   Fixes for PRS 12611 and
**       other ChromaKey / Trilinear & SingleStage texturing problems.  Rewrite the
**       multi-texture setup to use CCU for single stage rendering.
**  13   3dfx      1.12        01/28/00 Scott Kephart   Big T&L Merge: Changes for
**       FVF handling
**  12   3dfx      1.11        01/18/00 Bob Seitsinger  Changes to correctly write
**       to selected chip components (fbi,tmu0,tmu1). Chip component selection
**       occurred correctly for command packet header  writes, but needed to also
**       be done for data packet writes.
**  11   3dfx      1.10        01/10/00 Chris W. Shaw   Default to white diffuse in
**       the case that no diffuse is in FVF.  Fixes PRS 12055.
**  10   3dfx      1.9         12/06/99 Russ Lind       fix for PRS 11863 - DCT D3D
**       Shading Flat/Gouroud test fails
**       use color from vertexB in dp2FanAll when shadeMode=FLAT (fixes Test 6)
**  9    3dfx      1.8         12/06/99 Matt McClure    Fix typo in referencing
**       Flat shaded triangles.  Referencing pB when flat shading is wrong, should
**       be pA.  Fixes Flat shading quality test 1 in 3DWB 2k.
**  8    3dfx      1.7         12/06/99 Chris W. Shaw   Take out IGX_SPECULAR_FIX2.
**        I suggest doing this for all branches.  The code is unnecessary.
**  7    3dfx      1.6         12/01/99 Matt McClure    Fix for 3DMark 2000 lockup
**       in Test 2 Adventure, 3DMark 2000 sends down a FVF format that does not
**       contain a diffuse color, when accessing the entry in the FVF table for the
**       color calculation, the index of 128 was used and faulted on invalid memory
**       access.
**  6    3dfx      1.5         11/12/99 Chris W. Shaw   Fix for many of the DCT 263
**       TextureStage tests.  PRS 11235
**  5    3dfx      1.4         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  4    3dfx      1.3         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
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
** 5     8/19/99 5:06p Msmith
** adding opt for integer to float conversion using lookup table
** 
** 4     7/13/99 2:27p Cshaw
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
** 
** 3     6/21/99 5:25p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 2     6/04/99 10:34a Cshaw
** Updated code for 2pass specular with combineMode.
**
** 1     6/02/99 6:45a Michael
** Branch from H3
**
** 24    5/24/99 5:06p Bseitsin
** Removal of Antialiasing code.
**
** 23    5/06/99 4:21p Andrew
** put ifdef's around DumpCmdFifoEntries
**
** 22    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 21    3/27/99 2:08p Russ
** additional debug output
**
** 20    1/25/99 4:51p Peterm
** added unified header information
**
** 19    12/09/98 6:46a Russ
** NT5 D3D changes for Banshee
**
** 18    12/06/98 3:20p Adrians
** Fix build problem with Banshee.
**
** 17    12/06/98 1:53p Adrians
** Optimisations for WinBench99.
**
** 16    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
**
** 15    11/22/98 9:10p Andrew
** Changes to support multi-monitor
**
** 14    10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 13    10/03/98 5:19p Adrians
** Fix for two pass specular with zWriteEnable set to FALSE.
** Fix for two pass specular with fog.
**
** 12    10/01/98 10:38p Adrians
** Re-coded Fog which fixes a bug when Wbuffering with Vertex Fog.
**
** 11    9/13/98 12:18p Adrians
** Added a new flag global variable to disable AA when using specular,
** alphablending or chromokeying.
** Iterated Alpha is now always enabled for DX6.
**
** 10    9/05/98 5:02p Adrians
** Added AA support to DX6.
** Validate will now fail arguments with COMPLEMENT or ALPHAREPLICATE.
**
** 9     9/02/98 12:34p Adrians
** DX6 multitexture change.
**
** 8     8/15/98 12:15a Adrians
** Take Flat Shaded color from correct vertex for strips and fans.
** Use the same alpha component for all 3 vertices when Flat Shading.
** Add a Flat Shaded vertex component parameter to the line and point
** drawing.
**
** 7     8/14/98 8:10p Adrians
** Add texture wrapping support.
**
** 1     8/10/98 3:20p Adrians
**
** 6     8/04/98 10:18a Adrians
** Added bacface removal to wireframe and point filled triangles.
** Added support for texture0 coordIndex.
**
** 5     7/30/98 4:36p Adrians
** DX6 line drawing support.
**
** 4     7/29/98 7:25p Adrians
** DX6 changes.
**
** 2     5/07/98 6:35p Adrians
** Added performance test code.
**
** 1     4/29/98 6:31p Adrians
** Created
*/

#if ( DX >= 6 )

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

/*-------------------------------------------------------------------
Function Name:  dp2FanAllFill

Description:    This function handles drawing DrawPrimitive fans in
                wireframe and point fill modes.

Information:    If (PERFTEST == TRI_NULL) this function immediately
                returns without doing any work (null-driver)

Return:         void
-------------------------------------------------------------------*/

void dp2FanAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType )
{
  LPDWORD   pA, pB, pC;
#ifdef TnL_HAL
  LPDWORD       pAOrig, pBOrig, pCOrig;
  DWORD         c0, c1, c2;
  LPDWORD       clipcodes = (LPDWORD) pRc->tl.pClipBuf;
  DWORD         ClipContinue = 0;       // Tells us if we need to continue the 
  // clipped triangle alreasy in progress
  DWORD         dwNumClippedVertices;
  LPBYTE pTLV = (LPVOID) pRc->tl.clipping.ClipBuf.alignedBuf;
  DWORD dwUnion;
  DWORD dwMask;
  DWORD TLV_idx = 3;
  BOOL  RestoreVertexPointers = 0;
#endif

  int       sign;

#if( PERFTEST == TRI_NULL )
  return;
#endif

  switch( pRc->fillMode )
  {
    case D3DFILL_WIREFRAME :

#ifdef TnL_HAL
    if ( clipcodes != NULL )
    {
      TRI_INIT_WITH_CLIP;
    }
    else
    {
      TRI_INIT;
    }
#else
    TRI_INIT;
#endif
      for( ; count > 0; count-- )
      {
#ifdef TnL_HAL
    // Okay, were compiling in the software TnL HAL
    // We might have clipping to do so let's start the process

        // Any clipping needed?
        if ( CLIPPING_NEEDED )
        {
          // Check to see if we had started a multi triangle clip
          // in the last loop.
          if(ClipContinue)
          {
            // Yes, we have a cliped triangle in progress
            // Don't bother pulling out new vertex data 
            // from the command stream. Just re-arrange the
            // vertex pointers from the new output clip
            // FVF buffer for the next blade of the fan to clip
    
            // pA is the center of the Fan, it does NOT change
            pB = pC;    // pB is assigned to the Previous blade's pC vertex
            pC = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride*TLV_idx); // pC is the next vertex in the clipped FVF buf
    
            ClipContinue--;     // Decrement the ClipContine value
            TLV_idx++;          // Increment the new FVF buf index
          }
          else
          {
            TRI_CLIP_RESTORE;
            // No clipping in the previous pass of
            // the master loop, just get the next 
            // vertices and continue.
            TRI_NEXT_WITH_CLIP;
    
            // !!! QUICK TRIVIAL REJECT TEST !!!
            // Clipping test for this triangle
            if ((c0 & c1 & c2))
            {
              // This triangle is totaly clipped
              // skip it and go on to teh next one
              // in the command stream
              continue;
            }

            // This Triangle is not TR clipped 
            // Get the OR flags and clip mask to see if we can trivial 
            // accept it
            dwUnion = (c0 | c1 | c2);
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
              if((dwNumClippedVertices = ClipFVFTriangle( pRc, pA, pB, pC, c0, c1, c2, dwMask)))
              {
                ClipContinue = dwNumClippedVertices - 3; 

                TRI_CLIP_SAVE;
                pA = (LPDWORD) (pTLV); 
                pB = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride); 
                pC = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride*2); 
                TLV_idx = 3;  // Next vertex for the fan is at postion 3

                // Let the master loop know that we ne need to process more 
                // triangles if the clippeing calls for it.
                count += ClipContinue;
              }
              else
              {

                // Clip routine said that there was nothing to
                // clip, skip it and go on to teh next one
                // in the command stream
                continue;
              }

            } // if clipping on this triangle

          } // not a clip continuation
    
        } // if valid clipBuf pointer
        else
        {
          // No valid clip code buffer, just update the 
          // vertex pointers for the next triangle to process 
          TRI_NEXT;
        }

#else
        TRI_NEXT;

#endif //endif for TnL_HAL

        ((float*)&sign)[0] = ((FLTP(pA)[FVFO_SX] - FLTP(pB)[FVFO_SX]) * (FLTP(pB)[FVFO_SY] - FLTP(pC)[FVFO_SY])) -
                             ((FLTP(pB)[FVFO_SX] - FLTP(pC)[FVFO_SX]) * (FLTP(pA)[FVFO_SY] - FLTP(pB)[FVFO_SY]));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          dp2Line( pRc, pB, pA, pB, vertexType );
          dp2Line( pRc, pB, pB, pC, vertexType );
          dp2Line( pRc, pB, pC, pA, vertexType );
        }
      }
      break;

    case D3DFILL_POINT :

#ifdef TnL_HAL
      if ( clipcodes != NULL )
      {
        TRI_INIT_WITH_CLIP;
      }
      else
      {
        TRI_INIT;
      }
#else
      TRI_INIT;
#endif

      for( ; count > 0; count-- )
      {
#ifdef TnL_HAL
    // Okay, were compiling in the software TnL HAL
    // We might have clipping to do so let's start the process

        // Any clipping needed?
        if ( CLIPPING_NEEDED )
        {
          // Check to see if we had started a multi triangle clip
          // in the last loop.
          if(ClipContinue)
          {
            // Yes, we have a cliped triangle in progress
            // Don't bother pulling out new vertex data 
            // from the command stream. Just re-arrange the
            // vertex pointers from the new output clip
            // FVF buffer for the next blade of the fan to clip
    
            // pA is the center of the Fan, it does NOT change
            pB = pC;    // pB is assigned to the Previous blade's pC vertex
            pC = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride*TLV_idx); // pC is the next vertex in the clipped FVF buf
    
            ClipContinue--;     // Decrement the ClipContine value
            TLV_idx++;          // Increment the new FVF buf index
          }
          else
          {
            TRI_CLIP_RESTORE;
            // No clipping in the previous pass of
            // the master loop, just get the next 
            // vertices and continue.
            TRI_NEXT_WITH_CLIP;
    
            // !!! QUICK TRIVIAL REJECT TEST !!!
            // Clipping test for this triangle
            if ((c0 & c1 & c2))
            {
              // This triangle is totaly clipped
              // skip it and go on to teh next one
              // in the command stream
              continue;
            }

            // This Triangle is not TR clipped 
            // Get the OR flags and clip mask to see if we can trivial 
            // accept it
            dwUnion = (c0 | c1 | c2);
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
              if((dwNumClippedVertices = ClipFVFTriangle( pRc, pA, pB, pC, c0, c1, c2, dwMask)))
              {
                ClipContinue = dwNumClippedVertices - 3; 

                TRI_CLIP_SAVE;
                pA = (LPDWORD) (pTLV); 
                pB = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride); 
                pC = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride*2); 
                TLV_idx = 3;  // Next vertex for the fan is at postion 3

                // Let the master loop know that we ne need to process more 
                // triangles if the clippeing calls for it.
                count += ClipContinue;
              }
              else
              {

                // Clip routine said that there was nothing to
                // clip, skip it and go on to teh next one
                // in the command stream
                continue;
              }

            } // if clipping on this triangle

          } // not a clip continuation
    
        } // if valid clipBuf pointer
        else
        {
          // No valid clip code buffer, just update the 
          // vertex pointers for the next triangle to process 
          TRI_NEXT;
        }

#else
        TRI_NEXT;

#endif //endif for TnL_HAL

        ((float*)&sign)[0]  = ((FLTP(pA)[FVFO_SX] - FLTP(pB)[FVFO_SX]) * (FLTP(pB)[FVFO_SY] - FLTP(pC)[FVFO_SY])) -
                              ((FLTP(pB)[FVFO_SX] - FLTP(pC)[FVFO_SX]) * (FLTP(pA)[FVFO_SY] - FLTP(pB)[FVFO_SY]));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
#ifdef TnL_HAL
          if ( clipcodes != NULL )
          {
             if(c0 == 0) dp2Point( pRc, pA, pA, vertexType );
             if(c1 == 0) dp2Point( pRc, pA, pB, vertexType );
             if(c2 == 0) dp2Point( pRc, pA, pC, vertexType );
          }
          else
          {
             dp2Point( pRc, pA, pA, vertexType );
             dp2Point( pRc, pA, pB, vertexType );
             dp2Point( pRc, pA, pC, vertexType );
          }
#else
          dp2Point( pRc, pB, pA, vertexType );
          dp2Point( pRc, pB, pB, vertexType );
          dp2Point( pRc, pB, pC, vertexType );
#endif
        }
      }
      break;

    default :
      D3DPRINT( 0, "Illegal D3D Fill Mode %d", pRc->fillMode );
  }
} // Independat Triangle rendering

/*-------------------------------------------------------------------
Function Name:  dp2FanAll

Description:    This function handles drawing DrawPrimitive fans in
                solid-filled mode

Information:    If (PERFTEST == TRI_NULL) this function immediately
                returns without doing any work (null-driver)

Return:         void
-------------------------------------------------------------------*/
void dp2FanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType )
{
  SETUP_PPDEV(pRc)
  LPDWORD       pA, pB, pC;
#ifdef TnL_HAL
  LPDWORD       pAOrig, pBOrig, pCOrig;
  DWORD         c0, c1, c2;
  LPDWORD       clipcodes = (LPDWORD) pRc->tl.pClipBuf;
  DWORD         ClipContinue = 0;       // Tells us if we need to continue the 
  // clipped triangle alreasy in progress
  DWORD         dwNumClippedVertices;
  LPBYTE pTLV = (LPVOID) pRc->tl.clipping.ClipBuf.alignedBuf;
  DWORD dwUnion;
  DWORD dwMask;
  DWORD TLV_idx = 3;
  BOOL  RestoreVertexPointers = 0;
#endif
  float         s1, s2, s3, t1, t2, t3;
  float         w1, w2, w3;
  float         z1, z2, z3;
#if (NUMTEXTUREUNITS > 1)
  float         s1a, t1a, s1b, t1b, s1c, t1c;
#endif
  D3DCOLOR      aColor, bColor, cColor, aSColor, bSColor, cSColor;
  DWORD         dwSpecularEnabled;
  int           sign;
  FxU32         setupFlag = pRc->sst.sSetupMode;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

#ifdef TnL_HAL
  if ( clipcodes != NULL )
  {
    TRI_INIT_WITH_CLIP;
  }
  else
  {
    TRI_INIT;
  }
#else
  TRI_INIT;
#endif

  dwSpecularEnabled = pRc->specular;
  if( (FVFO_SPECULAR > FVFO_SIZE) )
    dwSpecularEnabled = 0;

  for( ; count > 0; count-- )
  {
#ifdef TnL_HAL
    // Okay, were compiling in the software TnL HAL
    // We might have clipping to do so let's start the process

    // Any clipping needed?
    if ( CLIPPING_NEEDED )
    {
      // Check to see if we had started a multi triangle clip
      // in the last loop.
      if(ClipContinue)
      {
        // Yes, we have a cliped triangle in progress
        // Don't bother pulling out new vertex data 
        // from the command stream. Just re-arrange the
        // vertex pointers from the new output clip
        // FVF buffer for the next blade of the fan to clip

        // pA is the center of the Fan, it does NOT change
        pB = pC;    // pB is assigned to the Previous blade's pC vertex
        pC = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride*TLV_idx); // pC is the next vertex in the clipped FVF buf

        ClipContinue--;     // Decrement the ClipContine value
        TLV_idx++;          // Increment the new FVF buf index
      }
      else
      {

        TRI_CLIP_RESTORE;
        // No clipping in the previous pass of
        // the master loop, just get the next 
        // vertices and continue.
        TRI_NEXT_WITH_CLIP;

        // !!! QUICK TRIVIAL REJECT TEST !!!
        // Clipping test for this triangle
        if ((c0 & c1 & c2))
        {
          // This triangle is totaly clipped
          // skip it and go on to teh next one
          // in the command stream
          continue;
        }

        // This Triangle is not TR clipped 
        // Get the OR flags and clip mask to see if we can trivial 
        // accept it
        dwUnion = (c0 | c1 | c2);
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
		  //AllenH 7/23/00 Swapped order for flat shading (on fans color comes from vertex B)
          //if((dwNumClippedVertices = ClipFVFTriangle( pRc, pA, pB, pC, c0, c1, c2, dwMask)))
          if((dwNumClippedVertices = ClipFVFTriangle( pRc, pB, pC, pA, c1, c2, c0, dwMask)))
          {
            ClipContinue = dwNumClippedVertices - 3; 

            TRI_CLIP_SAVE;
            pA = (LPDWORD) (pTLV); 
            pB = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride); 
            pC = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride*2); 
            TLV_idx = 3;  // Next vertex for the fan is at postion 3

            // Let the master loop know that we ne need to process more 
            // triangles if the clippeing calls for it.
            count += ClipContinue;
          }
          else
          {

            // Clip routine said that there was nothing to
            // clip, skip it and go on to teh next one
            // in the command stream
            continue;
          }

        } // if clipping on this triangle

      } // not a clip continuation

    } // if valid clipBuf pointer
    else
    {
      // No valid clip code buffer, just update the 
      // vertex pointers for the next triangle to process 
      TRI_NEXT;
    }

#else
    TRI_NEXT;

#endif //endif for TnL_HAL

    ((float*)&sign)[0] = ((FLTP(pA)[FVFO_SX] - FLTP(pB)[FVFO_SX]) * (FLTP(pB)[FVFO_SY] - FLTP(pC)[FVFO_SY])) -
                         ((FLTP(pB)[FVFO_SX] - FLTP(pC)[FVFO_SX]) * (FLTP(pA)[FVFO_SY] - FLTP(pB)[FVFO_SY]));

    sign &= 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
#ifdef CMDFIFO
      DumpCmdFifoEntries(MakeString(Stringize,dp2FanAll), ppdev, cmdFifo);
#endif

      // Checkroom for the largest block to be used.
#ifdef IGX_SPECULAR_FIX
    #if (NUMTEXTUREUNITS > 1)
#ifdef NEW_CCU
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE * 2) + 30 + 30  + (PH1_SIZE * 9) + 10 );
#else
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE * 2) + 30 + 30  + (PH1_SIZE * 7) + 8 );
#endif
    #else
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE * 2) + 24 + 24  + (PH1_SIZE * 7) + 8 );
    #endif
#else
    #if (NUMTEXTUREUNITS > 1)
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE * 2) + 30 + 15  + (PH1_SIZE * 7) + 8 );
    #else
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE * 2) + 24 + 15  + (PH1_SIZE * 7) + 8 );
    #endif
#endif
      /* 3DMark 2000 does not send down a diffuse color in their FVF Format */
	  /* Trying to index the 128th entry in pA, pB, or pC will fault        */
      if ( !(FVFO_COLOR > FVFO_SIZE) )
	  {

        if ( pRc->shadeMode == D3DSHADE_FLAT )
        {
          if (dwSpecularEnabled)
          {
#if 0	// SPECULARFIX!!!
            if (pRc->texture == 0)
              CLAMP888( aColor, pB[FVFO_COLOR], pB[FVFO_SPECULAR] );
            else
#endif
            {
              aColor = pB[FVFO_COLOR];

              // flat shaded specular should work!
              aSColor = pB[FVFO_SPECULAR];
              bSColor = cSColor = aSColor;
            }
            bColor = aColor;
            cColor = aColor;
          }
          else
            aColor = pB[FVFO_COLOR];

          bColor = aColor;
          cColor = aColor;
        }
        else
        {
          if (dwSpecularEnabled)
          {
#if 0	// SPECULARFIX!!!
            if (pRc->texture == 0)
            {
              CLAMP888(aColor, pA[FVFO_COLOR], pA[FVFO_SPECULAR]);
              CLAMP888(bColor, pB[FVFO_COLOR], pB[FVFO_SPECULAR]);
              CLAMP888(cColor, pC[FVFO_COLOR], pC[FVFO_SPECULAR]);
            }
            else
#endif
            {
              aColor = pA[FVFO_COLOR];
              bColor = pB[FVFO_COLOR];
              cColor = pC[FVFO_COLOR];

              aSColor = pA[FVFO_SPECULAR];
              bSColor = pB[FVFO_SPECULAR];
              cSColor = pC[FVFO_SPECULAR];
            }
          }
          else
          {
            aColor = pA[FVFO_COLOR];
            bColor = pB[FVFO_COLOR];
            cColor = pC[FVFO_COLOR];
          }
        }
	  }
	  else /* Diffuse Color not in FVF Format. default to white*/
	  {
        aColor = 0xffffffff;
        bColor = 0xffffffff;
        cColor = 0xffffffff;

 		if(dwSpecularEnabled)
        {
          if ( pRc->shadeMode == D3DSHADE_FLAT )
          {
#if 0	// SPECULARFIX!!!
            if (pRc->texture == 0)
            {
              aColor = bColor = cColor = 0xFF000000 | (pB[FVFO_SPECULAR]);
            }
            else
#endif
            {
              // flat shaded specular should work!
              aSColor = pB[FVFO_SPECULAR];
              bSColor = cSColor = aSColor;
            }
          }
          else
          {
#if 0	// SPECULARFIX!!!
            if (pRc->texture == 0)
            {
              aColor = 0xFF000000 | (pA[FVFO_SPECULAR]);
              bColor = 0xFF000000 | (pB[FVFO_SPECULAR]);
              cColor = 0xFF000000 | (pC[FVFO_SPECULAR]);
            }
            else
#endif
            {
              aSColor = pA[FVFO_SPECULAR];
              bSColor = pB[FVFO_SPECULAR];
              cSColor = pC[FVFO_SPECULAR];
            }
          }
        }
      }
	  
      if( pRc->state & STATE_REQUIRES_WBUFFER )
      {
        w1 = WSCALE( FLTP(pA)[FVFO_RHW] );
        w2 = WSCALE( FLTP(pB)[FVFO_RHW] );
        w3 = WSCALE( FLTP(pC)[FVFO_RHW] );

        if (pRc->state & STATE_REQUIRES_VERTEXFOG)
        {
          z1 = (float)((255 - RGBA_GETALPHA(pA[FVFO_SPECULAR])) << 8);
          z2 = (float)((255 - RGBA_GETALPHA(pB[FVFO_SPECULAR])) << 8);
          z3 = (float)((255 - RGBA_GETALPHA(pC[FVFO_SPECULAR])) << 8);
        }
      }
      else
      {
        z1 = ZSCALE( FLTP(pA)[FVFO_SZ] );
        z2 = ZSCALE( FLTP(pB)[FVFO_SZ] );
        z3 = ZSCALE( FLTP(pC)[FVFO_SZ] );

        if (pRc->state & STATE_REQUIRES_VERTEXFOG)
        {
#ifdef PERF_USE_255_LOOKUP_TABLE
//  faster here to use a lookup vs. letting fpu do conversion -mls
          w1 = f_255_reverse_lookup[RGBA_GETALPHA(pA[FVFO_SPECULAR])];
          w2 = f_255_reverse_lookup[RGBA_GETALPHA(pB[FVFO_SPECULAR])];
          w3 = f_255_reverse_lookup[RGBA_GETALPHA(pC[FVFO_SPECULAR])];
#else
#pragma message("note - NOT compiling with performance opt for int to float conversion")
          w1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]));
          w2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR]));
          w3 = (float)(255 - RGBA_GETALPHA(pC[FVFO_SPECULAR]));
#endif // perf_use_255_lookup_table
        }
        else if (pRc->state & STATE_REQUIRES_HWFOG)
        {
          w1 = FLTP(pA)[FVFO_RHW];
          w2 = FLTP(pB)[FVFO_RHW];
          w3 = FLTP(pC)[FVFO_RHW];
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

        s3 = FLTP(pC)[FVFO_TU + pRc->t0CoordIndex];
        t3 = FLTP(pC)[FVFO_TV + pRc->t0CoordIndex];

        lscaleS = pRc->sst.scaleS;
        lscaleT = pRc->sst.scaleT;

        // two ways to texture. D3D wraps its textures going around the other direction
        // so we need to adjust S and T in order to get the correct result
        WRAP( s, (pRc->wrapT0 & D3DWRAP_U) );
        WRAP( t, (pRc->wrapT0 & D3DWRAP_V) );

        // compute the slope of s t and w
        // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
        if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
        {
          s1 = ( ( s1 * lscaleS ) + TEXEL_SOFFSET ) * FLTP(pA)[FVFO_RHW];
          s2 = ( ( s2 * lscaleS ) + TEXEL_SOFFSET ) * FLTP(pB)[FVFO_RHW];
          s3 = ( ( s3 * lscaleS ) + TEXEL_SOFFSET ) * FLTP(pC)[FVFO_RHW];

          t1 = ( ( t1 * lscaleT ) + TEXEL_TOFFSET ) * FLTP(pA)[FVFO_RHW];
          t2 = ( ( t2 * lscaleT ) + TEXEL_TOFFSET ) * FLTP(pB)[FVFO_RHW];
          t3 = ( ( t3 * lscaleT ) + TEXEL_TOFFSET ) * FLTP(pC)[FVFO_RHW];
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

    #if (NUMTEXTUREUNITS > 1)
      // Second texture stage
      if( pRc->state & STATE_REQUIRES_ST_TMU1 )
      {
        float lscaleS, lscaleT;

        s1a = FLTP(pA)[FVFO_TU + pRc->t1CoordIndex];
        t1a = FLTP(pA)[FVFO_TV + pRc->t1CoordIndex];

        s1b = FLTP(pB)[FVFO_TU + pRc->t1CoordIndex];
        t1b = FLTP(pB)[FVFO_TV + pRc->t1CoordIndex];

        s1c = FLTP(pC)[FVFO_TU + pRc->t1CoordIndex];
        t1c = FLTP(pC)[FVFO_TV + pRc->t1CoordIndex];

        lscaleS = pRc->sst.scaleS1;
        lscaleT = pRc->sst.scaleT1;

        // two ways to texture. D3D wraps its textures going around the other direction
        // so we need to adjust S and T in order to get the correct result
        DX6_WRAP( s1, (pRc->wrapT1 & D3DWRAP_U) );
        DX6_WRAP( t1, (pRc->wrapT1 & D3DWRAP_V) );

        // compute the slope of s t and w
        // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
        if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
        {
          s1a = ( ( s1a * lscaleS ) + TEXEL_S1OFFSET ) * FLTP(pA)[FVFO_RHW];
          s1b = ( ( s1b * lscaleS ) + TEXEL_S1OFFSET ) * FLTP(pB)[FVFO_RHW];
          s1c = ( ( s1c * lscaleS ) + TEXEL_S1OFFSET ) * FLTP(pC)[FVFO_RHW];

          t1a = ( ( t1a * lscaleT ) + TEXEL_T1OFFSET ) * FLTP(pA)[FVFO_RHW];
          t1b = ( ( t1b * lscaleT ) + TEXEL_T1OFFSET ) * FLTP(pB)[FVFO_RHW];
          t1c = ( ( t1c * lscaleT ) + TEXEL_T1OFFSET ) * FLTP(pC)[FVFO_RHW];
        }
        // if the texture is not prespective correct then the w is effectively equal to 1
        else
        {
          s1a = ( s1a * lscaleS ) + TEXEL_S1OFFSET;
          s1b = ( s1b * lscaleS ) + TEXEL_S1OFFSET;
          s1c = ( s1c * lscaleS ) + TEXEL_S1OFFSET;

          t1a = ( t1a * lscaleT ) + TEXEL_T1OFFSET;
          t1b = ( t1b * lscaleT ) + TEXEL_T1OFFSET;
          t1c = ( t1c * lscaleT ) + TEXEL_T1OFFSET;
        }
      } // texture
    #endif

    #ifdef CMDFIFO
      SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_TRI, 3, setupFlag, 1 ) );
    #else
      SET( cmdFifo, ghw0->sSetupMode, setupFlag);
    #endif

      //Send 1st vertex
      SETFPD( cmdFifo, ghw0->sVx, FLTP(pA)[FVFO_SX] + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, FLTP(pA)[FVFO_SY] + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, aColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, z1 );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
      if(setupFlag & SST_SETUP_ST0)
      {
        SETFPD( cmdFifo, ghw0->sSow0, s1 );
        SETFPD( cmdFifo, ghw0->sTow0, t1 );
		D3DPRINTTEXCOORD(31, "dp2FanAll: Vtx1a: ", s1, t1);
      }
    #if (NUMTEXTUREUNITS > 1)
      if(setupFlag & SST_SETUP_ST1)
      {
        SETFPD( cmdFifo, ghw0->sSow1, s1a );
        SETFPD( cmdFifo, ghw0->sTow1, t1a );
		D3DPRINTTEXCOORD(31, "dp2FanAll: Vtx1b: ", s1a, t1a);
      }
    #endif
    #ifndef CMDFIFO
      if(_MM(drawGlobal))
        SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
    #endif

      // Send 2nd vertex
      SETFPD( cmdFifo, ghw0->sVx, FLTP(pB)[FVFO_SX] + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, FLTP(pB)[FVFO_SY] + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, bColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, z2 );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
      if(setupFlag & SST_SETUP_ST0)
      {
        SETFPD( cmdFifo, ghw0->sSow0, s2 );
        SETFPD( cmdFifo, ghw0->sTow0, t2 );
		D3DPRINTTEXCOORD(31, "dp2FanAll: Vtx2a: ", s2, t2);
      }
    #if (NUMTEXTUREUNITS > 1)
      if(setupFlag & SST_SETUP_ST1)
      {
        SETFPD( cmdFifo, ghw0->sSow1, s1b );
        SETFPD( cmdFifo, ghw0->sTow1, t1b );
		D3DPRINTTEXCOORD(31, "dp2FanAll: Vtx2b: ", s1b, t1b);
      }
    #endif
    #ifndef CMDFIFO
      if(_MM(drawGlobal))
        SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
    #endif

      // Send 3rd vertex
      SETFPD( cmdFifo, ghw0->sVx, FLTP(pC)[FVFO_SX] + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, FLTP(pC)[FVFO_SY] + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, cColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, z3 );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, w3 );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, FLTP(pC)[FVFO_RHW] );
      if(setupFlag & SST_SETUP_ST0)
      {
        SETFPD( cmdFifo, ghw0->sSow0, s3 );
        SETFPD( cmdFifo, ghw0->sTow0, t3 );
		D3DPRINTTEXCOORD(31, "dp2FanAll: Vtx3a: ", s3, t3);
      }
    #if (NUMTEXTUREUNITS > 1)
      if(setupFlag & SST_SETUP_ST1)
      {
        SETFPD( cmdFifo, ghw0->sSow1, s1c );
        SETFPD( cmdFifo, ghw0->sTow1, t1c );
		D3DPRINTTEXCOORD(31, "dp2FanAll: Vtx3b: ", s1c, t1c);
      }
    #endif
    #ifndef CMDFIFO
      if(_MM(drawGlobal))
        SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
    #endif

      // Specular on Textures
      // specular highlights on texture where the specular color is not black
#if 0	// SPECULARFIX!!!
      if (   (dwSpecularEnabled) && (pRc->texture != 0)
#else
      if (   (dwSpecularEnabled)
#endif
           && (CI_MASKALPHA(pA[FVFO_SPECULAR] | pB[FVFO_SPECULAR] | pC[FVFO_SPECULAR]) != 0))
      {
        // if specular is turned on then we need to add in the specular color
        // and this means re-render the triangle gouraud shaded using the specular
        // color information and then add it to the textured triangle we just rendered.
        // We can't just add it to the triangle color because this color would be
        // blended or replaced with the texture and highlights are added into textures.
        //
#ifdef IGX_SPECULAR_FIX
        ULONG fbzMode, fbzColorPath;
        ULONG combineModeFBI;

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

        if (IS_NAPALM) { //NAPALM_CU
#ifdef NEW_CCU
            combineModeFBI = ( (SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI) |
                               (pRc->sst.combineModeFBI & (~(SST_CM_CC_OTHERSELECT |SST_CM_CC_LOCALSELECT|
                                    SST_CM_CC_MSELECT_7 |SST_CM_CC_INVERT_OTHER|
                                    SST_CM_CC_INVERT_LOCAL|SST_CM_CC_OUTSHIFT|SST_CM_CC_INVERT_ADD_LOCAL)));
            combineModeFBI |= SST_CM_CC_OTHERSELECT_IRGB; //Use iterated color (specular color)
            //fbzcolorpath: pass iterated color and leave alpha the same.
            fbzColorPath = pRc->sst.fbzColorPath &(~SST_CCOMBINE);
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 0, fbzColorPath, 0x1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->fbzColorPath, fbzColorPath );

#else // NEW_CCU
            // texture mapping off and texture blending off but keep alpha the same
            combineModeFBI = ( SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK | 
                               SST_CM_CCA_OTHERSELECT_TA | 
                               SST_CM_DISABLE_CHROMA_SUBSTITUTION | 
                               SST_CM_USE_COMBINE_MODE ) & pRc->sst.combineModeFBI; 
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, combineModeFBI );
#endif //NEW_CCU
        }
        else {
            fbzColorPath = (pRc->sst.fbzColorPath & ~(SST_RGBSELECT | SST_CCOMBINE)) | SST_RGBSEL_RGBA;
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fbzColorPath, 0xF ) );
            SETPD( cmdFifo, ghw->fbzColorPath, fbzColorPath );
        }

        if (pRc->alphaBlendEnable)
        {
          #ifdef CMDFIFO
            SETPH( cmdFifo, CMDFIFO_BUILD_PK3( 0, 3, (setupFlag | (SST_SETUP_RGB | SST_SETUP_A)), 1 ) );
          #else
            SET( cmdFifo, ghw0->sSetupMode, (setupFlag | (SST_SETUP_RGB | SST_SETUP_A)));
          #endif
        }
        else
        {
          #ifdef CMDFIFO
            SETPH( cmdFifo, CMDFIFO_BUILD_PK3( 0, 3, ((setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0 | SST_SETUP_ST1 | SST_SETUP_W1)) | (SST_SETUP_RGB | SST_SETUP_A)), 1 ) );
          #else
            SET( cmdFifo, ghw0->sSetupMode, ((setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0 | SST_SETUP_ST1 | SST_SETUP_W1)) | (SST_SETUP_RGB | SST_SETUP_A)));
          #endif
        }

        // Send 1st vertex
        SETFPD( cmdFifo, ghw0->sVx, FLTP(pA)[FVFO_SX] + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, FLTP(pA)[FVFO_SY] + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, RGBA_SETALPHA(aSColor, RGBA_GETALPHA(aColor)) );

        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, z1 );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
        if (pRc->alphaBlendEnable)
        {
            if(setupFlag & SST_SETUP_W0)
              SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
            if(setupFlag & SST_SETUP_ST0)
            {
                SETFPD( cmdFifo, ghw0->sSow0, s1 );
                SETFPD( cmdFifo, ghw0->sTow0, t1 );
				D3DPRINTTEXCOORD(31, "dp2FanAll: Specular Vtx1a: ", s1, t1);
            }
        #if (NUMTEXTUREUNITS > 1)
            if(setupFlag & SST_SETUP_ST1)
            {
                SETFPD( cmdFifo, ghw0->sSow1, s1a );
                SETFPD( cmdFifo, ghw0->sTow1, t1a );
				D3DPRINTTEXCOORD(31, "dp2FanAll: Specular Vtx1b: ", s1a, t1a);
            }
        #endif
        }
      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        // Send 2nd vertex
        SETFPD( cmdFifo, ghw0->sVx, FLTP(pB)[FVFO_SX] + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, FLTP(pB)[FVFO_SY] + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, RGBA_SETALPHA(bSColor, RGBA_GETALPHA(bColor)) );

        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, z2 );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
        if (pRc->alphaBlendEnable)
        {
            if(setupFlag & SST_SETUP_W0)
              SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
            if(setupFlag & SST_SETUP_ST0)
            {
                SETFPD( cmdFifo, ghw0->sSow0, s2 );
                SETFPD( cmdFifo, ghw0->sTow0, t2 );
				D3DPRINTTEXCOORD(31, "dp2FanAll: Specular Vtx2a: ", s2, t2);
            }
          #if (NUMTEXTUREUNITS > 1)
            if(setupFlag & SST_SETUP_ST1)
            {
                SETFPD( cmdFifo, ghw0->sSow1, s1b );
                SETFPD( cmdFifo, ghw0->sTow1, t1b );
				D3DPRINTTEXCOORD(31, "dp2FanAll: Specular Vtx2b: ", s1b, t1b);
            }
          #endif
        }
      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        // Send 3rd vertex
        SETFPD( cmdFifo, ghw0->sVx, FLTP(pC)[FVFO_SX] + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, FLTP(pC)[FVFO_SY] + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, RGBA_SETALPHA(cSColor, RGBA_GETALPHA(cColor)) );

        if(setupFlag & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, z3 );
        if(setupFlag & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, w3 );
        if (pRc->alphaBlendEnable)
        {
            if(setupFlag & SST_SETUP_W0)
              SETFPD( cmdFifo, ghw0->sOow0, FLTP(pC)[FVFO_RHW] );
            if(setupFlag & SST_SETUP_ST0)
            {
                SETFPD( cmdFifo, ghw0->sSow0, s3 );
                SETFPD( cmdFifo, ghw0->sTow0, t3 );
				D3DPRINTTEXCOORD(31, "dp2FanAll: Specular Vtx3a: ", s3, t3);
            }
          #if (NUMTEXTUREUNITS > 1)
            if(setupFlag & SST_SETUP_ST1)
            {
                SETFPD( cmdFifo, ghw0->sSow1, s1c );
                SETFPD( cmdFifo, ghw0->sTow1, t1c );
				D3DPRINTTEXCOORD(31, "dp2FanAll: Specular Vtx3b: ", s1c, t1c);
            }
          #endif
        }
      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
#else
		ULONG fbzMode, setup;
        ULONG combineModeFBI;
        ULONG fbzColorPath;
        //CMDFIFO_CHECKROOM( cmdFifo, ((PH1_SIZE * 7) + 8) + (PH3_SIZE + (4 * 3)) );

		D3DPRINT(31, "dp2FanAll: Non-Specular fix");

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

        setup = setupFlag & ~(SST_SETUP_ST0 | SST_SETUP_W0 | SST_SETUP_ST1 | SST_SETUP_W1);

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

      #ifdef CMDFIFO
        SETPH( cmdFifo, CMDFIFO_BUILD_PK3( 0, 3, (setup | (SST_SETUP_RGB | SST_SETUP_A)), 1 ) );
      #else
        SET( cmdFifo, ghw0->sSetupMode, (setup | (SST_SETUP_RGB | SST_SETUP_A)));
      #endif

        // Send 1st vertex
        SETFPD( cmdFifo, ghw0->sVx, FLTP(pA)[FVFO_SX] + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, FLTP(pA)[FVFO_SY] + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, aSColor );
        if(setup & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, z1 );
        if(setup & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
      #endif

        // Send 2nd vertex
        SETFPD( cmdFifo, ghw0->sVx, FLTP(pB)[FVFO_SX] + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, FLTP(pB)[FVFO_SY] + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, bSColor );
        if(setup & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, z2 );
        if(setup & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif

        // Send 3rd vertex
        SETFPD( cmdFifo, ghw0->sVx, FLTP(pC)[FVFO_SX] + PIXEL_OFFSET );
        SETFPD( cmdFifo, ghw0->sVy, FLTP(pC)[FVFO_SY] + PIXEL_OFFSET );
        SETPD( cmdFifo, ghw0->sARGB, cSColor );
        if(setup & SST_SETUP_Z)
          SETFPD( cmdFifo, ghw0->sVz, z3 );
        if(setup & SST_SETUP_Wfbi)
          SETFPD( cmdFifo, ghw0->sOowfbi, w3 );
      #ifndef CMDFIFO
        if(_MM(drawGlobal))
          SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
      #endif
#endif // IGX_SPECULAR_FIX
        // restore everything back to the way it was
        if (IS_NAPALM) { //NAPALM_CU
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, combineMode, 1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, pRc->sst.combineModeFBI );
#ifdef NEW_CCU
            SETPH( cmdFifo,CMDFIFO_BUILD_PK1CHIP( 1, 0, fbzColorPath,1 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->fbzColorPath, pRc->sst.fbzColorPath );
#endif
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

#ifdef CMDFIFO
  DumpCmdFifoEntries(MakeString(Stringize,dp2FanAll), ppdev, cmdFifo);
#endif
  CMDFIFO_EPILOG( cmdFifo );
} // Independat Triangle rendering

/*-------------------------------------------------------------------
Function Name:  dp2FanAll_SM3

Description:    This function handles drawing DrawPrimitive fans in
                solid-filled mode.  This version is for the special
                multitexture modes.

Information:    If (PERFTEST == TRI_NULL) this function immediately
                returns without doing any work (null-driver)

Return:         void
-------------------------------------------------------------------*/
void dp2FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType )
{
  SETUP_PPDEV(pRc)
  LPDWORD       pA, pB, pC;
#ifdef TnL_HAL
  LPDWORD       pAOrig, pBOrig, pCOrig;
  DWORD         c0, c1, c2;
  LPDWORD       clipcodes = (LPDWORD) pRc->tl.pClipBuf;
  DWORD         ClipContinue = 0;       // Tells us if we need to continue the 
  // clipped triangle alreasy in progress
  DWORD         dwNumClippedVertices;
  LPBYTE pTLV = (LPVOID) pRc->tl.clipping.ClipBuf.alignedBuf;
  DWORD dwUnion;
  DWORD dwMask;
  DWORD TLV_idx = 3;
  BOOL  RestoreVertexPointers = 0;
#endif
  float         s1, s2, s3, t1, t2, t3;
  float         w1, w2, w3;
  float         z1, z2, z3;
#if (NUMTEXTUREUNITS > 1)
  float         s1a, t1a, s1b, t1b, s1c, t1c;
#endif
  D3DCOLOR      aColor, bColor, cColor;
  int           sign;
  FxU32         setupFlag = pRc->sst.sSetupMode;
  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

#ifdef TnL_HAL
  if ( clipcodes != NULL )
  {
    TRI_INIT_WITH_CLIP;
  }
  else
  {
    TRI_INIT;
  }
#else
  TRI_INIT;
#endif

  for( ; count > 0; count-- )
  {
#ifdef TnL_HAL
    // Okay, were compiling in the software TnL HAL
    // We might have clipping to do so let's start the process

    // Any clipping needed?
    if ( CLIPPING_NEEDED )
    {
      // Check to see if we had started a multi triangle clip
      // in the last loop.
      if(ClipContinue)
      {
        // Yes, we have a cliped triangle in progress
        // Don't bother pulling out new vertex data 
        // from the command stream. Just re-arrange the
        // vertex pointers from the new output clip
        // FVF buffer for the next blade of the fan to clip

        // pA is the center of the Fan, it does NOT change
        pB = pC;    // pB is assigned to the Previous blade's pC vertex
        pC = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride*TLV_idx); // pC is the next vertex in the clipped FVF buf

        ClipContinue--;     // Decrement the ClipContine value
        TLV_idx++;          // Increment the new FVF buf index
      }
      else
      {

        TRI_CLIP_RESTORE;
        // No clipping in the previous pass of
        // the master loop, just get the next 
        // vertices and continue.
        TRI_NEXT_WITH_CLIP;

        // !!! QUICK TRIVIAL REJECT TEST !!!
        // Clipping test for this triangle
        if ((c0 & c1 & c2))
        {
          // This triangle is totaly clipped
          // skip it and go on to teh next one
          // in the command stream
          continue;
        }

        // This Triangle is not TR clipped 
        // Get the OR flags and clip mask to see if we can trivial 
        // accept it
        dwUnion = (c0 | c1 | c2);
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

          if((dwNumClippedVertices = ClipFVFTriangle( pRc, pA, pB, pC, c0, c1, c2, dwMask)))
          {
            ClipContinue = dwNumClippedVertices - 3; 

            TRI_CLIP_SAVE;
            pA = (LPDWORD) (pTLV); 
            pB = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride); 
            pC = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride*2); 
            TLV_idx = 3;  // Next vertex for the fan is at postion 3

            // Let the master loop know that we ne need to process more 
            // triangles if the clippeing calls for it.
            count += ClipContinue;
          }
          else
          {

            // Clip routine said that there was nothing to
            // clip, skip it and go on to teh next one
            // in the command stream
            continue;
          }

        } // if clipping on this triangle

      } // not a clip continuation

    } // if valid clipBuf pointer
    else
    {
      // No valid clip code buffer, just update the 
      // vertex pointers for the next triangle to process 
      TRI_NEXT;
    }
#else
    TRI_NEXT;
#endif //endif for TnL_HAL

    ((float*)&sign)[0] = ((FLTP(pA)[FVFO_SX] - FLTP(pB)[FVFO_SX]) * (FLTP(pB)[FVFO_SY] - FLTP(pC)[FVFO_SY])) -
                         ((FLTP(pB)[FVFO_SX] - FLTP(pC)[FVFO_SX]) * (FLTP(pA)[FVFO_SY] - FLTP(pB)[FVFO_SY]));

    sign &= 0x80000000;

    if ((sign ^ pRc->cullMask) != 0x80000000)
    {
      // Checkroom for the largest block to be used.
      CMDFIFO_CHECKROOM( cmdFifo, (PH3_SIZE * 2) + 30 + 15  + (PH1_SIZE * 7) + 8 );

      /* 3DMark 2000 does not send down a diffuse color in their FVF Format */
	  /* Trying to index the 128th entry in pA, pB, or pC will fault        */
      if ( !(FVFO_COLOR > FVFO_SIZE) )
	  {

        if ( pRc->shadeMode == D3DSHADE_FLAT )
        {
          if ( (pRc->specular) && (pRc->texture == 0) )
            CLAMP888( aColor, pA[FVFO_COLOR], pA[FVFO_SPECULAR] );
          else
            aColor = pA[FVFO_COLOR];

          bColor = aColor;
          cColor = aColor;
        }
        else
        {
          if ( (pRc->specular) && (pRc->texture == 0) )
          {
            CLAMP888(aColor, pA[FVFO_COLOR], pA[FVFO_SPECULAR]);
            CLAMP888(bColor, pB[FVFO_COLOR], pB[FVFO_SPECULAR]);
            CLAMP888(cColor, pC[FVFO_COLOR], pC[FVFO_SPECULAR]);
          }
          else
          {
            aColor = pA[FVFO_COLOR];
            bColor = pB[FVFO_COLOR];
            cColor = pC[FVFO_COLOR];
          }
        }
	  }
	  else /* Diffuse Color not in FVF Format. default to white*/
	  {
        aColor = 0xffffffff;
        bColor = 0xffffffff;
        cColor = 0xffffffff;
      }

      // cDiff = cDiff * 2
      CLAMP888( aColor, aColor, aColor );
      CLAMP888( bColor, bColor, bColor );
      CLAMP888( cColor, cColor, cColor );

      if( pRc->state & STATE_REQUIRES_WBUFFER )
      {
        w1 = WSCALE( FLTP(pA)[FVFO_RHW] );
        w2 = WSCALE( FLTP(pB)[FVFO_RHW] );
        w3 = WSCALE( FLTP(pC)[FVFO_RHW] );

        if (pRc->state & STATE_REQUIRES_VERTEXFOG)
        {
          z1 = (float)((255 - RGBA_GETALPHA(pA[FVFO_SPECULAR])) << 8);
          z2 = (float)((255 - RGBA_GETALPHA(pB[FVFO_SPECULAR])) << 8);
          z3 = (float)((255 - RGBA_GETALPHA(pC[FVFO_SPECULAR])) << 8);
        }
      }
      else
      {
        z1 = ZSCALE( FLTP(pA)[FVFO_SZ] );
        z2 = ZSCALE( FLTP(pB)[FVFO_SZ] );
        z3 = ZSCALE( FLTP(pC)[FVFO_SZ] );

        if (pRc->state & STATE_REQUIRES_VERTEXFOG)
        {
#ifdef PERF_USE_255_LOOKUP_TABLE
//  faster here to use a lookup vs. letting fpu do conversion -mls
          w1 = f_255_reverse_lookup[RGBA_GETALPHA(pA[FVFO_SPECULAR])];
          w2 = f_255_reverse_lookup[RGBA_GETALPHA(pB[FVFO_SPECULAR])];
          w3 = f_255_reverse_lookup[RGBA_GETALPHA(pC[FVFO_SPECULAR])];
#else
#pragma message("note - NOT compiling with performance opt for int to float conversion")
          w1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]));
          w2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR]));
          w3 = (float)(255 - RGBA_GETALPHA(pC[FVFO_SPECULAR]));
#endif // perf_use_255_lookup_table
        }
        else if (pRc->state & STATE_REQUIRES_HWFOG)
        {
          w1 = FLTP(pA)[FVFO_RHW];
          w2 = FLTP(pB)[FVFO_RHW];
          w3 = FLTP(pC)[FVFO_RHW];
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

        s3 = FLTP(pC)[FVFO_TU + pRc->t0CoordIndex];
        t3 = FLTP(pC)[FVFO_TV + pRc->t0CoordIndex];

        lscaleS = pRc->sst.scaleS;
        lscaleT = pRc->sst.scaleT;

        // two ways to texture. D3D wraps its textures going around the other direction
        // so we need to adjust S and T in order to get the correct result
        WRAP( s, (pRc->wrapT0 & D3DWRAP_U) );
        WRAP( t, (pRc->wrapT0 & D3DWRAP_V) );

        // compute the slope of s t and w
        // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
        if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
        {
          s1 = ( ( s1 * lscaleS ) + TEXEL_SOFFSET ) * FLTP(pA)[FVFO_RHW];
          s2 = ( ( s2 * lscaleS ) + TEXEL_SOFFSET ) * FLTP(pB)[FVFO_RHW];
          s3 = ( ( s3 * lscaleS ) + TEXEL_SOFFSET ) * FLTP(pC)[FVFO_RHW];

          t1 = ( ( t1 * lscaleT ) + TEXEL_TOFFSET ) * FLTP(pA)[FVFO_RHW];
          t2 = ( ( t2 * lscaleT ) + TEXEL_TOFFSET ) * FLTP(pB)[FVFO_RHW];
          t3 = ( ( t3 * lscaleT ) + TEXEL_TOFFSET ) * FLTP(pC)[FVFO_RHW];
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

    #if (NUMTEXTUREUNITS > 1)
      // Second texture stage
      if( pRc->state & STATE_REQUIRES_ST_TMU1 )
      {
        float lscaleS, lscaleT;

        s1a = FLTP(pA)[FVFO_TU + pRc->t1CoordIndex];
        t1a = FLTP(pA)[FVFO_TV + pRc->t1CoordIndex];

        s1b = FLTP(pB)[FVFO_TU + pRc->t1CoordIndex];
        t1b = FLTP(pB)[FVFO_TV + pRc->t1CoordIndex];

        s1c = FLTP(pC)[FVFO_TU + pRc->t1CoordIndex];
        t1c = FLTP(pC)[FVFO_TV + pRc->t1CoordIndex];

        lscaleS = pRc->sst.scaleS1;
        lscaleT = pRc->sst.scaleT1;

        // two ways to texture. D3D wraps its textures going around the other direction
        // so we need to adjust S and T in order to get the correct result
        DX6_WRAP( s1, (pRc->wrapT1 & D3DWRAP_U) );
        DX6_WRAP( t1, (pRc->wrapT1 & D3DWRAP_V) );

        // compute the slope of s t and w
        // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
        if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
        {
          s1a = ( ( s1a * lscaleS ) + TEXEL_S1OFFSET ) * FLTP(pA)[FVFO_RHW];
          s1b = ( ( s1b * lscaleS ) + TEXEL_S1OFFSET ) * FLTP(pB)[FVFO_RHW];
          s1c = ( ( s1c * lscaleS ) + TEXEL_S1OFFSET ) * FLTP(pC)[FVFO_RHW];

          t1a = ( ( t1a * lscaleT ) + TEXEL_T1OFFSET ) * FLTP(pA)[FVFO_RHW];
          t1b = ( ( t1b * lscaleT ) + TEXEL_T1OFFSET ) * FLTP(pB)[FVFO_RHW];
          t1c = ( ( t1c * lscaleT ) + TEXEL_T1OFFSET ) * FLTP(pC)[FVFO_RHW];
        }
        // if the texture is not prespective correct then the w is effectively equal to 1
        else
        {
          s1a = ( s1a * lscaleS ) + TEXEL_S1OFFSET;
          s1b = ( s1b * lscaleS ) + TEXEL_S1OFFSET;
          s1c = ( s1c * lscaleS ) + TEXEL_S1OFFSET;

          t1a = ( t1a * lscaleT ) + TEXEL_T1OFFSET;
          t1b = ( t1b * lscaleT ) + TEXEL_T1OFFSET;
          t1c = ( t1c * lscaleT ) + TEXEL_T1OFFSET;
        }
      } // texture
    #endif

    #ifdef CMDFIFO
      SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_TRI, 3, setupFlag, 1 ) );
    #else
      SET( cmdFifo, ghw0->sSetupMode, setupFlag);
    #endif

      //Send 1st vertex
      SETFPD( cmdFifo, ghw0->sVx, FLTP(pA)[FVFO_SX] + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, FLTP(pA)[FVFO_SY] + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, aColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, z1 );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

      // Send 2nd vertex
      SETFPD( cmdFifo, ghw0->sVx, FLTP(pB)[FVFO_SX] + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, FLTP(pB)[FVFO_SY] + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, bColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, z2 );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
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

      // Send 3rd vertex
      SETFPD( cmdFifo, ghw0->sVx, FLTP(pC)[FVFO_SX] + PIXEL_OFFSET );
      SETFPD( cmdFifo, ghw0->sVy, FLTP(pC)[FVFO_SY] + PIXEL_OFFSET );
      if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
        SETPD( cmdFifo, ghw0->sARGB, cColor );
      if(setupFlag & SST_SETUP_Z)
        SETFPD( cmdFifo, ghw0->sVz, z3 );
      if(setupFlag & SST_SETUP_Wfbi)
        SETFPD( cmdFifo, ghw0->sOowfbi, w3 );
      if(setupFlag & SST_SETUP_W0)
        SETFPD( cmdFifo, ghw0->sOow0, FLTP(pC)[FVFO_RHW] );
      if(setupFlag & SST_SETUP_ST0)
      {
        SETFPD( cmdFifo, ghw0->sSow0, s3 );
        SETFPD( cmdFifo, ghw0->sTow0, t3 );
      }
#if (NUMTEXTUREUNITS > 1)
      if(setupFlag & SST_SETUP_ST1)
      {
        SETFPD( cmdFifo, ghw0->sSow1, s1c );
        SETFPD( cmdFifo, ghw0->sTow1, t1c );
      }
#endif
    #ifndef CMDFIFO
      if(_MM(drawGlobal))
        SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
    #endif
    }
  } // every triangle

  CMDFIFO_EPILOG( cmdFifo );
}

#endif // DX >= 6

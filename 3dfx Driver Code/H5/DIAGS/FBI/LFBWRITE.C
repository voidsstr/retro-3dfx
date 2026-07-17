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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** Last Edited: Thu Aug 31 22:47:27 1995 by jdt (John D. Tynefield) on vlsi3
**
** $Revision: 3$ 
** $Date: 10/11/00 8:10:16 PM$ 
**
*/

#include "udiag.h"
#include "sstdiag.h"

#include "fbi.h"
#include "lfbutils.h"
#include "hsimio.h"

#define PRIME_VAL 103

void initScreen(SstRegs *sst)
{
	int x, y;
	FxU32 rval, mask, rands[PRIME_VAL];

	mask = 0xFFFFFFFF;
	if (diago.rgb == 16) mask = 0x00F8FCF8;
	if (diago.rgb == 15) mask = 0x80F8F8F8;
	// init some random values
	for (x=0; x<PRIME_VAL; x++)
	    rands[x] = colRandom32();

	////////////////////////////////////////////////////////////////////
	// Force the entire front/back/ZA buffers to be random valued pixels
	gdbg_info( 2, "Filling in front/back/za buffers (this takes some time)..." );
	csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXFALSE);	// disable video
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_BEGIN(0, CSIM_BUF_3D_FRONT);
	for ( y = 0; y < diago.ymaxscreen; y++ ) {
		for ( x = 0; x < diago.xmaxscreen; x++ ) {
			int temp = y*MAXSCREEN + x;
			rval = rands[(temp+19)%PRIME_VAL] & mask;
			DIAG_FORCE_PIXEL( CSIM_BUF_3D_FRONT, x, y, rval );
		}
	}
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_END(0);
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_BEGIN(0, CSIM_BUF_3D_BACK);
	for ( y = 0; y < diago.ymaxscreen; y++ ) {
		for ( x = 0; x < diago.xmaxscreen; x++ ) {
			int temp = y*MAXSCREEN + x;
			rval = rands[(temp+x)%PRIME_VAL] & mask;
			DIAG_FORCE_PIXEL( CSIM_BUF_3D_BACK, x, y, rval );
		}
	}
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_END(0);

	if (diago.rgb < 32) mask = 0xFFFF;
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_BEGIN(0, CSIM_BUF_3D_AUX1);
	for ( y = 0; y < diago.ymaxscreen; y++ ) {
		for ( x = 0; x < diago.xmaxscreen; x++ ) {
			int temp = y*MAXSCREEN + x;
			rval = rands[(temp+17)%PRIME_VAL] & mask;
			DIAG_FORCE_PIXEL( CSIM_BUF_3D_AUX1, x, y, rval );
		}
	}
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_END(0);
	gdbg_info_more( 2, "...done\n" );
	csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXTRUE);	// enable video
}

void main (int argc, char **argv)
{
    SstRegs        *sst;
    static SstRegs  shadowRegs;
    
    int Pass = 1;  // !! Debugging Variable....
    
    enum TestOpt writeFormat;
    
    // Init the Hardware
    sst = SST_BEGIN( argc, argv );

    if (diago.deviceID == SST_DEVICE_ID_SST96) {
	gdbg_error("lfbwrite:", "Can't run this diag on SST96\n");
	DIAG_PASS( 0 );
	exit(0);
    }

    if(diago.aaEnabled)
      {
	GDBG_INFO(0, "Warning! This sucka diag doesn't run with AA enabled\n");
	DIAG_PASS(0);
	exit(0);
      }

    // Print Out Option Description
    if ( diago.printOpts )
    {
        gdbg_printf( "lfbwrite option description:\n" );
        gdbg_printf( " %d -> Test Fmt       565\n", TEST565 );
        gdbg_printf( " %d -> Test Fmt      x555\n", TESTX555 );
        gdbg_printf( " %d -> Test Fmt      1555\n", TEST1555 );
        gdbg_printf( " %d -> Test Fmt      x888\n", TESTX888 );
        gdbg_printf( " %d -> Test Fmt      8888\n", TEST8888 );
        gdbg_printf( " %d -> Test Fmt   16z+565\n", TEST16N565 );
        gdbg_printf( " %d -> Test Fmt  16z+X555\n", TEST16NX555 );
        gdbg_printf( " %d -> Test Fmt  16z+1555\n", TEST16N1555 );
        gdbg_printf( " %d -> Test Fmt   16z+16z\n", TEST16N16 );
        gdbg_printf( " %d -> Test Fmt       32z\n", TESTZ32 );
        gdbg_printf( " %d -> Test Random Format\n", TESTRANDOM );
        
        DIAG_FAIL();
    }
    
    initScreen(sst);	// init the screen to random data
    SET(sst->nopCMD,1);	// reset HW pixel counters

    // Get Write Format
    writeFormat = diago.option;
    
    // Set up the Fog Table
    // Initialize the Fog Table to be a constant linear function
  {
      FxU32 fogVal1, fogVal2;
      FxU32 fogIndex;
      static FxU32 fogTable[FOG_TABLE_SIZE];

      fogVal1 = fogVal2 = 0;
      for ( fogIndex = 0; fogIndex < FOG_TABLE_SIZE; fogIndex+=2 )
      {
          fogTable[fogIndex] = ( fogVal1 << 8 ) | 0x04;
          fogVal2 = fogVal1 + 0x04;
          fogTable[fogIndex+1] = ( fogVal2 << 8 ) | 0x04;
          fogVal1 += 8;
      }

      // Download Fog Table and Pack on the Fly
      for ( fogIndex = 0; fogIndex < FOG_TABLE_SIZE; fogIndex+=2 )
      {
          shadowRegs.fogTable[fogIndex/2] = (fogTable[fogIndex + 1] << 16) | fogTable[fogIndex];
          SET( sst->fogTable[fogIndex/2], shadowRegs.fogTable[fogIndex/2] );
      }
  }
    
    // !! For now the clipping rectangle will be set to the screen boundaries !!
    // !! Need to discuss with gary what would be the best way to handle this !!
    shadowRegs.clipLeftRight = (0<<16) | diago.xmaxscreen;
    SET(sst->clipLeftRight, shadowRegs.clipLeftRight );
    
    shadowRegs.clipBottomTop = (0<<16) | diago.ymaxscreen;
    SET(sst->clipBottomTop,shadowRegs.clipBottomTop );
    shadowRegs.renderMode = GET(sst->renderMode);
    shadowRegs.stencilOp  = SST_SOP_KEEP<<SST_STENCIL_SFAIL_OP_SHIFT;
    shadowRegs.stencilOp |= SST_SOP_REPLACE<<SST_STENCIL_ZFAIL_OP_SHIFT;
    shadowRegs.stencilOp |= SST_SOP_REPLACE<<SST_STENCIL_ZPASS_OP_SHIFT;
    SET( sst->stencilOp, shadowRegs.stencilOp );

    while (DIAG_STARTPASS())
    {
        int n;
	FxBool pv1,pv2;

        for (n=0; n<100; n++)
        {                       // Do 100 Tests
            FxU32 x1, y1, za1, x2, y2, za2;
            FxU32 r1, g1, b1, a1, r2, g2, b2, a2;
	    FxU32 depth1, depth2;
            fbiColors pixel1, pixel2;
            FxU32 writeValue;
            FxU32 pixelsToWrite;
            
            do
            {
                // Generate A Random XY Coordinate, Depths, and RGBAs, 
                xyRandom(&x1,&y1);              
                y2 = y1;
                x2 = x1 + 1;
                
                pixelsToWrite = iRandom( 0x1 ) + 1;
            } while ( ONSCREEN( (signed long) x1, (signed long) y1 ) &&
                      (( pixelsToWrite == 2 ) && ( x1 % 2 )) );
            
            depth1 = iRandom( 0xFFFF );  // break here Pass == ?, n== ? 
            depth2 = iRandom( 0xFFFF );
            
            r2 = 0xFF ^ (r1 = iRandom( 0xFF ));
            g2 = 0xFF ^ (g1 = iRandom( 0xFF ));
            b2 = 0xFF ^ (b1 = iRandom( 0xFF ));
            a2 = 0xFF ^ (a1 = iRandom( 0xFF ));
            
            // Generate Random Format if Necessary
            if ( writeFormat ==  TESTRANDOM )
              writeFormat = iRandom( NUM_TEST_OPTS - 1 );
            
            // Generate Random LFB Write Parameters
            shadowRegs.lfbMode = rndlfbMode( writeFormat, &pixelsToWrite );
            SET( sst->lfbMode, shadowRegs.lfbMode );
            
            // Generate A Random FBI State
            shadowRegs.fbzMode = rndfbzMode();
	    SET( sst->fbzMode, shadowRegs.fbzMode );    
            
            shadowRegs.fbzColorPath = rndfbzColorPath();
            SET( sst->fbzColorPath, shadowRegs.fbzColorPath );

            // somewhat random stencilMode, func either NEVER or ALWAYS
	    if (diago.rgb == 32) {
		shadowRegs.stencilMode = iRandom(0xFF) << SST_STENCIL_REF_SHIFT;
		if (iRandom(1)) shadowRegs.stencilMode |= SST_STENCIL_ENABLE;
		if (iRandom(1)) shadowRegs.stencilMode |= SST_STENCIL_WMASK;
		if (iRandom(1)) shadowRegs.stencilMode |= SST_STENCIL_FUNC;
		SET( sst->stencilMode, shadowRegs.stencilMode );
	    }

	    // Fog Pass-Thru
	    shadowRegs.fogMode = 0; 
#if 1
	    if (iRandom(1)) shadowRegs.fogMode |= SST_RGB_BLEND_REVERSE;
	    if (iRandom(1)) shadowRegs.fogMode |= SST_RGB_BLEND_SUB;
	    if (iRandom(1)) shadowRegs.fogMode |= SST_A_BLEND_REVERSE;
#endif
            SET( sst->fogMode, shadowRegs.fogMode );
            
	    // Alpha Pass Thru
	    shadowRegs.alphaMode = 0;
#if 1
	    // GMT: at least test out RGB blending 
	    do {
		shadowRegs.alphaMode = iRandom(0xFFFFFFFF);
		shadowRegs.alphaMode &= SST_ENALPHABLEND | SST_RGBSRCFACT|SST_RGBDSTFACT;
		shadowRegs.alphaMode |= SST_A_ONE << SST_ASRCFACT_SHIFT;
		shadowRegs.alphaMode |= SST_A_ZERO << SST_ADSTFACT_SHIFT;
	    } while (!goodAlphaMode(shadowRegs.alphaMode,shadowRegs.fbzMode));
#endif
            SET( sst->alphaMode, shadowRegs.alphaMode );
            
            shadowRegs.fogColor = iRandom( 0xFFFFFFFF );
            SET( sst->fogColor, shadowRegs.fogColor );
            
            shadowRegs.zaColor = iRandom( 0xFFFFFFFF );
            SET( sst->zaColor, shadowRegs.zaColor );
            
            shadowRegs.a = iRandom( 0xFFFFFFFF );
            SET( sst->a, shadowRegs.a );
            
            shadowRegs.w = iRandom( 0xFFFFFFFF );
            SET( sst->w, shadowRegs.w );
            
            shadowRegs.z = iRandom( 0xFFFFFFFF );
            SET( sst->z, shadowRegs.z );
            
            shadowRegs.dzdx = iRandom( 0xFFFFFFFF );
            SET( sst->dzdx, shadowRegs.dzdx );
            
            shadowRegs.dwdx = iRandom( 0xFFFFFFFF );
            SET( sst->dwdx, shadowRegs.dwdx );
            
            shadowRegs.dadx = iRandom( 0xFFFFFFFF );
            SET( sst->dadx, shadowRegs.dadx );
            
            shadowRegs.dzdy = iRandom( 0xFFFFFFFF );
            SET( sst->dzdy, shadowRegs.dzdy );
            
            shadowRegs.dwdy = iRandom( 0xFFFFFFFF );
            SET( sst->dwdy, shadowRegs.dwdy );
            
            shadowRegs.chromaKey = iRandom( 0x00FFFFFF );
            SET( sst->chromaKey, shadowRegs.chromaKey );
            
            shadowRegs.c0 = iRandom( 0x00FFFFFF );
            SET( sst->c0, shadowRegs.c0 );
            
            shadowRegs.c1 = iRandom( 0x00FFFFFF );
            SET( sst->c1, shadowRegs.c1 );

	    shadowRegs.renderMode &= ~SST_RM_ALPHAMODE;
	    shadowRegs.renderMode |= iRandom(2) << SST_RM_ALPHAMODE_SHIFT;
	    SET( sst->renderMode, shadowRegs.renderMode );

            sst_idle_really(sst);

            // Run the pixel through the PixPipeEmulator
            // This will give the depth, r, g, b values that should be in the buffer
            pixel1.red = (short)r1;
            pixel1.green = (short)g1;
            pixel1.blue = (short)b1;
            pixel1.alpha = (short)a1;
	    za1 = depth1;
	    if ((shadowRegs.lfbMode & SST_LFB_FORMAT) == SST_LFB_Z32) {
		za1 = depth1 = iRandom(0xFFFFFFFF);
	    }
                
            pixel2.red = (short)r2;
            pixel2.green = (short)g2;
            pixel2.blue = (short)b2;
            pixel2.alpha = (short)a2;
	    za2 = depth2;

            if ( pixelsToWrite == 1 )
            {
                pv1 = EmulatePixPipe( &pixel1, x1, y1, &za1, &shadowRegs, sst );
            }
            else
            {
                pv1 = EmulatePixPipe( &pixel1, x1, y1, &za1, &shadowRegs, sst );
                pv2 = EmulatePixPipe( &pixel2, x2, y2, &za2, &shadowRegs, sst );
            }
            
            // Based on the lfbMode mangle the correct color/depth input
            mangleColor( &writeValue, &shadowRegs, pixelsToWrite,
                         a1, r1, g1, b1, depth1, a2, r2, g2, b2, depth2 );
            
            // Run the pixel through the simulator
            lfbWrite( x1, y1, writeValue, pixelsToWrite, &shadowRegs, sst );
            
            // Compare the Color/ZA in the Frame Buffer to the Color/ZA From Emulator
            
            // If yOrigin, flip y.
            if ( flipYOrigin( &shadowRegs ) )
            {
                y1 = y2 = (diago.ymaxscreen -1) - y1;
            }
            
            // Wait for card to idle
            sst_idle_really(sst);

	    // if pixel is written in 15bpp mode and either RGB writemask is set
	    // or pixelpipe is disabled, then we apply alphamode override
            if ( pixelsToWrite >= 1 ) {
                testPixelInFB( x1, y1, &shadowRegs, &pixel1 );
                DIAG_TEST_PIXEL( CSIM_BUF_3D_AUX1, x1, y1, za1  );
            }
            if ( pixelsToWrite >= 2 ) {
                testPixelInFB( x2, y2, &shadowRegs, &pixel2 );
                DIAG_TEST_PIXEL( CSIM_BUF_3D_AUX1, x2, y2, za2  );
            }
            
            // Restore the write format in case it is random
            writeFormat = diago.option;
            
            
            gdbg_info( 2, "Pass: %.4d n: %.4d\n", Pass, n ); // !! debugging
        }
        Pass++;// !!debugging
    }
    DIAG_PASS( 0 );
}

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
**
** $Revision: 4$ 
** $Date: 10/11/00 8:10:14 PM$ 
**
*/

#include "udiag.h"
#include "sstdiag.h"

#include "fbi.h"
#include "lfbutils.h"
#include "hsimio.h"

static FxU32 frontBuffer[MAXSCREEN][MAXSCREEN];
static FxU32 backBuffer[MAXSCREEN][MAXSCREEN];
static FxU16 zaBuffer[MAXSCREEN][MAXSCREEN];

#define PRIME_VAL 103

void initScreen(SstRegs *sst)
{
	int x, y;
	FxU32 rval, rands[PRIME_VAL];

	// init some random values
	for (x=0; x<PRIME_VAL; x++)
	    rands[x] = iRandom(0xFFFFFF);

	////////////////////////////////////////////////////////////////////
	// Force the entire front/back/ZA buffers to be random valued pixels
	gdbg_info( 2, "Filling in front/back/za buffers (this takes some time)..." );
	csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXFALSE);	// disable video
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_BEGIN(0, CSIM_BUF_3D_FRONT);
	for ( y = 0; y < diago.ymaxscreen; y++ ) {
		for ( x = 0; x < diago.xmaxscreen; x++ ) {
			int temp = y*MAXSCREEN + x;
			rval = rands[(temp+19)%PRIME_VAL] & 0xF8FCF8;
			frontBuffer[y][x] = rval;
//gdbg_info(401,"xy = %d,%d\n", x,y);
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
			rval = rands[(temp+x)%PRIME_VAL] & 0xF8FCF8;
			backBuffer[y][x] = rval;
			DIAG_FORCE_PIXEL(CSIM_BUF_3D_BACK, x, y, rval );
		}
	}
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_END(0);
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_BEGIN(0, CSIM_BUF_3D_AUX1);
	for ( y = 0; y < diago.ymaxscreen; y++ ) {
		for ( x = 0; x < diago.xmaxscreen; x++ ) {
			int temp = y*MAXSCREEN + x;
			rval = rands[(temp+17)%PRIME_VAL] & 0xFFFF;
			zaBuffer[y][x] = (FxU16)rval;
			DIAG_FORCE_PIXEL(CSIM_BUF_3D_AUX1, x, y, rval );
		}
	}
	if (diago.halInfo->hw)
	    HW_PIXEL_FAST_END(0);
	gdbg_info_more( 2, "...done\n" );
	csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXTRUE);	// enable video
}

void main (int argc, char **argv)
{
	int x, y;
	SstRegs *sst;
	
    // Init the Hardware
    sst = SST_BEGIN( argc, argv );
#ifdef CVG
    if (CSIM_PRIVATE(diago.sstCSIM)->info->sliDetected) {
	GDBG_PRINTF("WARNING: SLI detected, skipping YORIGIN testing\n");
    }
#endif
    while (DIAG_STARTPASS())
    {
		int i,j;
		FxU32 lfbMode;

		initScreen(sst);	// init the screen to random data
		SET(sst->nopCMD,1);	// reset HW pixel counters
		
		for( j = 0; j < 100; j++ )
		{
			int oldMode = lfbMode;
			///////////////////////////////////////////////////////////////////////
			// Generate a random lfbReadMode
			lfbMode = iRandom( 0xFFFFFFFF );
#ifdef CVG
			if (CSIM_PRIVATE(diago.sstCSIM)->info->sliDetected)
			    lfbMode &= ~SST_LFB_YORIGIN;
#endif
	
			// Random ( but legal read mode )
			lfbMode &= ~SST_LFB_READBUFSELECT;
#ifdef CVG
			lfbMode |= ( iRandom( 0x2 ) << SST_LFB_READBUFSELECT_SHIFT );
#else
			if ( iRandom(1) )
			  lfbMode |= SST_LFB_READCOLORBUFFER;
			else 
			  lfbMode |= SST_LFB_READDEPTHABUFFER;
#endif

			SET( sst->lfbMode, lfbMode );
			// Always sync after lfbmode change
			sst_idle_really(sst);
			
			for ( i = 0; i < 10; i++ )
			{
				FxU16 *lfbBaseS = (FxU16 *) SST_LFB_ADDRESS( sst );
				FxU32 leftPixel, rightPixel, lfbValue, testValue;

				///////////////////////////////////////////////////////////////////////
				// Test Randomly selected pixels and compare against known color values.			
				do xyRandom( &x, &y );				
				 while ( x % 2 );
				gdbg_info(4,"reading LFB x,y = %d,%d\n",x,y);
				// Read LFB Value
				lfbValue = GET( lfbBaseS[lfbOffset(x,y)] );

				// YOrigin
				if ( lfbMode & SST_LFB_YORIGIN )
				{
					gdbg_info(5,"---yorigin\n");
					y = ( diago.ymaxscreen - 1 ) - y;
				}

				// Get Buffer Values
				switch( lfbMode & SST_LFB_READBUFSELECT )
				{
#ifdef CVG
					case SST_LFB_READFRONTBUFFER:
						gdbg_info(5,"---frontbuffer\n");
						leftPixel = frontBuffer[y][x];
						rightPixel = frontBuffer[y][x+1];
						break;
					case SST_LFB_READBACKBUFFER:
						gdbg_info(5,"---backbuffer\n");
						leftPixel = backBuffer[y][x];
						rightPixel = backBuffer[y][x+1];
						break;
#else // H3
					case SST_LFB_READCOLORBUFFER:
						gdbg_info(5,"---colorbuffer\n");
						if ( diago.curdrawbuffer == 0 ) {
						  leftPixel = frontBuffer[y][x];
						  rightPixel = frontBuffer[y][x+1]; 
						} else {
						  leftPixel = backBuffer[y][x];
						  rightPixel = backBuffer[y][x+1]; 
						}
						break;
#endif
					case SST_LFB_READDEPTHABUFFER:
						gdbg_info(5,"---zabuffer\n");
						leftPixel = zaBuffer[y][x];
						rightPixel = zaBuffer[y][x+1];
						break;
					default:
						gdbg_info( 0, "Bad read mode.\n" );
				}
				gdbg_info(6,"left = 0x%08x    right = 0x%08x\n",leftPixel,rightPixel);

				// SWAP Words
				if ( lfbMode & SST_LFB_READ_SWAP16 )
				{
					FxU32 temp = leftPixel;
					gdbg_info(5,"---swap16\n");
					leftPixel = rightPixel;
					rightPixel = temp; 
				}

				// Mangle Values
				if ( ! ( (lfbMode & SST_LFB_READBUFSELECT ) == SST_LFB_READDEPTHABUFFER ) )
				{
					FxU32 rLeft = ( leftPixel >> 19 ) & 0x1F;
					FxU32 rRight = ( rightPixel >> 19 ) & 0x1F;
					FxU32 gLeft = ( leftPixel >> 10 ) & 0x3F;
					FxU32 gRight = ( rightPixel >> 10 ) & 0x3F;
					FxU32 bLeft = (leftPixel >> 3)  & 0x1F;
					FxU32 bRight = (rightPixel >> 3) & 0x1F;
					switch( lfbMode & SST_LFB_RGBALANES )
					{
						case SST_LFB_RGBALANES_ARGB:
							gdbg_info(5,"---ARGB\n");
							leftPixel = packBits( 3, rLeft, 5, gLeft, 6, bLeft, 5 );
							rightPixel = packBits( 3, rRight, 5, gRight, 6, bRight, 5 );
							break;
						case SST_LFB_RGBALANES_ABGR:
							gdbg_info(5,"---ABGR\n");
							leftPixel = packBits( 3, bLeft, 5, gLeft, 6, rLeft, 5 );
							rightPixel = packBits( 3, bRight, 5, gRight, 6, rRight, 5 );
							break;
						case SST_LFB_RGBALANES_RGBA:
							gdbg_info(5,"---RGBA\n");
							leftPixel = packBits( 3, rLeft, 5, gLeft, 6, bLeft, 5 );
							rightPixel = packBits( 3, rRight, 5, gRight, 6, bRight, 5 );
						break;
						case SST_LFB_RGBALANES_BGRA:
							gdbg_info(5,"---BGRA\n");
							leftPixel = packBits( 3, bLeft, 5, gLeft, 6, rLeft, 5 );
							rightPixel = packBits( 3, bRight, 5, gRight, 6, rRight, 5 );
						break;
					}
					leftPixel >>= 16;
					rightPixel >>= 16;
				}

				// Swizzle Bytes
				if ( lfbMode & SST_LFB_READ_BYTESWAP )
				{
					FxU32 temp = leftPixel;
					gdbg_info(5,"---byte swap\n");
					leftPixel = ((rightPixel >> 8) & 0xFF) | ((rightPixel&0xFF) << 8);
					rightPixel = ((temp >> 8) & 0xFF) | ((temp&0xFF) << 8);
				}

				// Concatenate
				testValue = ( rightPixel << 16 ) | leftPixel;
				
				// Compare
				DIAG_TESTLFBREAD32(x,y,lfbValue,testValue);
			}
		}
	}
	DIAG_PASS( 0 );
}

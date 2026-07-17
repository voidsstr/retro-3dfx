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
*n* and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 2$ 
** $Date: 10/11/00 8:10:37 PM$ 
**
*/

#include "udiag.h"
#include "sstdiag.h"

#include "fbi.h"
#include "lfbutils.h"

enum { INNER_LOOP_COUNT = 1000 };

void lfbStressWrite( FxU32 x, FxU32 y, FxU32 value,
               FxU32 pixelsToWrite, FxU32 lfbMode,
               SstRegs *sst )
{
    // Create LFB Base Address ( both short and long formats )
    FxU32 *lfbBaseL = (FxU32 *) SST_LFB_ADDRESS( sst );
    FxU16 *lfbBaseS = (FxU16 *) SST_LFB_ADDRESS( sst );
    
    gdbg_info( 20, "lfbWrite x: %d y: %d value: 0x%.8x\n", x, y, value );
    
    switch( lfbMode & SST_LFB_FORMAT )
    {
      case SST_LFB_565:
      case SST_LFB_555:
      case SST_LFB_1555:
      case SST_LFB_ZZ:   
        if ( pixelsToWrite == 2 )
        {
            if ( x % 2 )
	      gdbg_info( 0, "Misaligned 2 pixel write - ( x=%d, y=%d )", x, y );
            SET( lfbBaseS[lfbOffset(x,y)], value );
        }
        else
        {
	  FxU32 address = (FxU32)&(lfbBaseS[lfbOffset(x,y)]);
            
            // Fix Address Based on Word/Byte Swap
            if ( lfbMode & SST_LFB_WRITE_BYTESWAP )
	      address ^= 2;
            if ( lfbMode & SST_LFB_WRITE_SWAP16 )
	      address ^= 2;
            
            // Blech!!!
            SET16( *(FxU16*)address, (unsigned short)value );
        }
        break;
      case SST_LFB_888:
      case SST_LFB_8888:
      case SST_LFB_Z565:
      case SST_LFB_Z555:
      case SST_LFB_Z1555:
      case SST_LFB_Z32:
        SET( lfbBaseL[lfbOffset(x,y)], value );
        break;
      default:
        gdbg_info( 0, "Illegal Write Format %d\n",
                   (lfbMode & SST_LFB_FORMAT) >>
                   SST_LFB_FORMAT_SHIFT ); 
        break;
    }
    
    return;
}

void main (int argc, char **argv)
{
    SstRegs *sst;
	
    enum TestOpt writeFormat;
    
    // Init the Hardware
    sst = SST_BEGIN( argc, argv );

    if (!diago.diff) 
    {
	gdbg_error("stress5","must run with -D option, forcing -D\n");
	diago.diff = 1;
    }    
    if (diago.zeroLodFrac && !diago.hasAuxBuffer) {
	gdbg_error("stress5","must run without -Z option when no zbuffer present, turning off -Z\n");
	diago.zeroLodFrac = 0;
    }

    if (diago.deviceID == SST_DEVICE_ID_SST96) {
	gdbg_error("stress5", "Can't be run with SST-96\n");
    }
    else 
    while (DIAG_STARTPASS())
    {
	FxU32 x1, x2, y1, y2;
	FxU32 pixelsToWrite;
	FxU32 writeValue;
	FxU32 lfbMode;
	int n;

	if (diago.checkEveryTriangle || !diago.diff)
	    DIAG_DIFFSCREEN(diago.xmaxscreen-1,diago.ymaxscreen-1);

	SET( sst->fogColor, iRandom( 0xFFFFFFFF ) );
	SET( sst->zaColor, iRandom(0xFFFFFFFF));
	SET( sst->a, iRandom( 0xFFFFFFFF ) ); 
	SET( sst->w, iRandom( 0xFFFFFFFF ) );
	SET( sst->z, iRandom( 0xFFFFFFFF ) );
	SET( sst->dzdx, iRandom( 0xFFFFFFFF ) );
	SET( sst->dwdx, iRandom( 0xFFFFFFFF ) );
	SET( sst->dadx, iRandom( 0xFFFFFFFF ) );
	SET( sst->dzdy, iRandom( 0xFFFFFFFF ) );
	SET( sst->dwdy, iRandom( 0xFFFFFFFF ) );
	SET( sst->dady, iRandom( 0xFFFFFFFF ) );
	SET( sst->chromaKey, iRandom( 0x00FFFFFF ) );
	SET( sst->c0, iRandom( 0x00FFFFFF ) );
	SET( sst->c1, iRandom( 0x00FFFFFF ) );
	SET( sst->fbzMode, SST_RGBWRMASK |
		( diago.zeroLodFrac ? SST_ZAWRMASK : 0 ) );
	SET( sst->fbzColorPath, 0x0 );
	SET( sst->fogMode, 0x0 );
	SET( sst->alphaMode, 0x0 );

	do {
	    writeFormat = iRandom( NUM_TEST_OPTS - 1 );
	} while ((!diago.hasAuxBuffer) &&
		((writeFormat == TEST1555) || (writeFormat == TEST8888) ||
		(writeFormat == TEST16N565) || (writeFormat == TEST16NX555) ||
		(writeFormat == TEST16N1555) || (writeFormat == TEST16N16) ||
		(writeFormat == TESTZ32)));

	pixelsToWrite = iRandom(1) + 1;
	lfbMode = rndlfbMode( writeFormat, &pixelsToWrite );
	SET( sst->lfbMode, lfbMode );		
			
	for (n=0; n<INNER_LOOP_COUNT; n++) { 
		FxU32 randomNumber = iRandom( 4 );

		// every once in a while change the LFB mode
		if ( randomNumber == 0 ) {
			do {
			    writeFormat = iRandom( NUM_TEST_OPTS - 1 );
			} while ((!diago.hasAuxBuffer) &&
				((writeFormat == TEST1555) || (writeFormat == TEST8888) ||
				(writeFormat == TEST16N565) || (writeFormat == TEST16NX555) ||
				(writeFormat == TEST16N1555) || (writeFormat == TEST16N16) ||
				(writeFormat == TESTZ32)));
			pixelsToWrite = iRandom( 0x1 ) + 1;
			lfbMode = rndlfbMode( writeFormat, &pixelsToWrite );
			SET( sst->lfbMode, lfbMode );
			if (diago.rgb == 32) {
			    SET(sst->stencilMode, iRandom(0x0FFFFFFF));
			    SET(sst->stencilOp, iRandom(0xFFF)&0x777);
			}
		}
		else {
			do {
				xyRandom(&x1, &y1);              
				y2 = y1;
				x2 = x1 + 1;
			} while ( ONSCREEN( (signed long) x1, (signed long) y1 ) &&			
		      					(( pixelsToWrite == 2 ) && ( x1 % 2 )) );
			writeValue = iRandom( 0xFFFFFFFF );
			lfbStressWrite( x1, y1, writeValue, pixelsToWrite, lfbMode, sst );
		}
	}
    }
    DIAG_PASS( 0 );
}

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
** $Revision: 2$
** $Date: 10/11/00 8:10:47 PM$
*/
/* This test is a very simple test of the video backend, using the vidutils.c
file
*/

#include "udiag.h"
#include "sstdiag.h"

void
main (int argc, char **argv)
{
// Questions for Phil: How's the stride value generated.
    int n;
    long cx,cy,x,y, fbzMode,nop;
    SstRegs *sst;
	 FxU32 tmp;
	    SstIORegs *sstio ;
	    FxU32 stride;
	    FxU32 width = 256;
	    FxU32 height = 256;

    sst = SST_BEGIN(argc,argv);
	 sstio = SST_IO_ADDRESS(sst);

	    sst_idle_really(sst);
	    if ( diago.ytiled )
	    SET(sstio->vidProcCfg,SST_OVERLAY_PIXEL_RGB565D|SST_OVERLAY_TILED_EN);
		 else
		 SET(sstio->vidProcCfg,SST_OVERLAY_PIXEL_RGB565D);
		 CSIM_PRIVATE(diago.sstCSIM)->io.vidCurrOverlayStartAddr = GET(sst->colBufferAddr);
		 stride = GET(sst->colBufferStride);
		 stride &= ~SST_BUFFER_MEMORY_TYPE;
		 SET(sstio->vidDesktopOverlayStride,stride<<SST_OVERLAY_STRIDE_SHIFT); 
		  // set desktop to point to front buffer

			sst_idle_really(sst);
			if ( diago.ytiled )
			SET(sstio->vidProcCfg,SST_DESKTOP_PIXEL_RGB565|SST_DESKTOP_TILED_EN);
			else
			SET(sstio->vidProcCfg,SST_DESKTOP_PIXEL_RGB565);
			SET(sstio->vidDesktopStartAddr,GET(sst->colBufferAddr));
			stride = GET(sst->colBufferStride);
			stride &= ~SST_BUFFER_MEMORY_TYPE;
			SET(sstio->vidDesktopOverlayStride,stride<<SST_DESKTOP_STRIDE_SHIFT);
																																													  
    // Setup minimum functionality to actually get a desktop displayed.
	 tmp = GET(sstio->vidProcCfg);
	 tmp &= (~SST_OVERLAY_EN);  // no overlay
	 tmp |= SST_DESKTOP_EN;    // but a desktop.
	 SET(sstio->vidProcCfg,tmp);    
	 tmp = (width << SST_VIDEO_SCREEN_WIDTH_SHIFT ) 
		  | (height << SST_VIDEO_SCREEN_HEIGHT_SHIFT);
	 SET(sstio->vidScreenSize,tmp);    

    vidOut(); // render the display image
  	 sst_idle_really(sst);		// wait for the command to complete
    DIAG_PASS(1);
}

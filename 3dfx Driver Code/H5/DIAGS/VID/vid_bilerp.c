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
** $Date: 10/11/00 8:19:41 PM$
*/
/* This test is a very simple test of the video backend, using the vidutils.c
file
*/

#include "udiag.h"
#include "sstdiag.h"
#include "vidutils.h"
#include "hsimio.h"

void
main (int argc, char **argv)
{
    long x,y;
    SstRegs *sst;
	 FxU32 tmp;
	    SstIORegs *sstio ;
	    FxU32 stride;
	    FxU32 width = 640;
	    FxU32 height = 480;

    sst = SST_BEGIN(argc,argv);
	 sstio = (SstIORegs *)SST_IO_ADDRESS(sst);

	    sst_idle_really(sst);
		 // set the overlay buffer to point to the end of the Color buffer 
	    if ( diago.ytiled )
	       SET(sstio->vidProcCfg,SST_OVERLAY_PIXEL_RGB565D|SST_OVERLAY_TILED_EN);
		 else
		    SET(sstio->vidProcCfg,SST_OVERLAY_PIXEL_RGB565U);
		 CSIM_PRIVATE(diago.sstCSIM)->io.vidCurrOverlayStartAddr 
			  = 0x100000 + GET(sst->colBufferAddr); // one meg into it.
		 stride = GET(sst->colBufferStride);
		 stride &= ~SST_BUFFER_MEMORY_TYPE;
		 SET(sstio->vidDesktopOverlayStride,stride<<SST_OVERLAY_STRIDE_SHIFT); 
		  // set desktop to point to front buffer

			sst_idle_really(sst);
	      tmp = GET(sstio->vidProcCfg);
			if ( diago.ytiled )
			SET(sstio->vidProcCfg,tmp | SST_DESKTOP_PIXEL_RGB565|SST_DESKTOP_TILED_EN);
			else
			SET(sstio->vidProcCfg,tmp | SST_DESKTOP_PIXEL_RGB565);
			SET(sstio->vidDesktopStartAddr,GET(sst->colBufferAddr));
			stride = GET(sst->colBufferStride);
			stride &= ~SST_BUFFER_MEMORY_TYPE;
	      tmp = GET(sstio->vidDesktopOverlayStride);
			SET(sstio->vidDesktopOverlayStride,tmp | (stride<<SST_DESKTOP_STRIDE_SHIFT));
																																													  
	 tmp = GET(sstio->vidProcCfg);
	 tmp |= (SST_OVERLAY_EN);  //  overlay
	 tmp |= SST_DESKTOP_EN;    // and a desktop.
	 tmp |= SST_OVERLAY_CLUT_BYPASS | SST_DESKTOP_CLUT_BYPASS; // turn of clut
	 tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
	 tmp |= SST_OVERLAY_VERT_SCALE_EN;    
	 tmp |= SST_OVERLAY_FILTER_BILINEAR; 
	 tmp |= SST_CURSOR_EN;
	 tmp |= SST_CURSOR_X11;
      tmp |= SST_VIDEO_PROCESSOR_EN;
	 SET(sstio->vidProcCfg,tmp);    
	 tmp = (width << SST_VIDEO_SCREEN_WIDTH_SHIFT ) 
		  | (height << SST_VIDEO_SCREEN_HEIGHT_SHIFT);
	 SET(sstio->vidScreenSize,tmp);    

	 // set video overlay start coords at 11, 51
	 tmp = (11<<SST_OVERLAY_X_SHIFT); 
	 tmp |= (51<<SST_OVERLAY_Y_SHIFT);
	 SET(sstio->vidOverlayStartCoords,tmp);    
	 // set video overlay end coords at 101, 71
	 tmp = ((11+256)<<SST_OVERLAY_X_SHIFT); 
	 tmp   |= ((51+256)<<SST_OVERLAY_Y_SHIFT);
	 SET(sstio->vidOverlayEndScreenCoord,tmp);    

	 // video scaling attributes
	 tmp = (0xff<<(19-7))/2; //Zoom up by a little over half a pixel pixel  
	 SET(sstio->vidOverlayDudx,tmp);    
	 tmp = 1<<18; // starting address at 1/2
	 SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	 tmp = (0xff<<(19-7)); //Zoom up by a little 
	 SET(sstio->vidOverlayDvdy,tmp);    
	 SET(sstio->vidOverlayDvdyOffset,0);    


   // Place data in overlay buffer
	 for(y=0; y<256; y++)
	 {
	 for(x=0; x<256; x++)
	   {
			tmp = ((x*y)>>7)& 0x1ff; 
			if (tmp>255) tmp=255;
			tmp = ((tmp>>3)<<(5+6)) | ((tmp>>2)<< 5) | (tmp>>3);
			if((x%11==0)) tmp |= 0xffff; // white stripes
			if((y%9==0)) tmp ^= 0xffff; // invert data
			CSIM_PIXEL_WR(CSIM_BUF_OVERLAY, x, y, tmp);
	   }
	 }


    // Setup cursor
	SET(sstio->hwCurPatAddr
			  , 0x100000 + GET(sst->colBufferAddr)); // one meg into it.
        SET(sstio->hwCurLoc, (365 <<SST_CURSOR_X_SHIFT) 
                           | (121 << SST_CURSOR_Y_SHIFT)); 
        SET(sstio->hwCurC0, 0x00ffff); // Red
        SET(sstio->hwCurC1, 0xff00ff); // Green
	 for(y=0; y<64; y++)
	 {
	 for(x=0; x<64; x++)
	   {
               tmp= (((y>>2) & x)& 0x8>>3);
               tmp |= (((y ^ (x>>1))&0x10)>>3);
	       CSIM_PIXEL_WR(CSIM_BUF_CURSOR, x, y, tmp);
  
           }
         }



 vidInit(width, height, sst);
    vidOut(); // render the display image
  	vidCleanup(sst);		// wait for the command to complete
	DIAG_PASS(1);
}

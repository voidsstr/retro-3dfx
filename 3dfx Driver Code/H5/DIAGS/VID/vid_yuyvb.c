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
** $Date: 10/11/00 8:19:54 PM$
*/
/* This test is for yuyv format, in bilinear interpolation mode
*/

#include "udiag.h"
#include "sstdiag.h"
#include <fximg.h>
#include "vidutils.h"
#include "hsimio.h"

 // use a 4x4 dither, should probably have the option for 2x2 dither
 static int dithmat[4][4] = {0,8,2,10, 12,4,14,6, 3,11,1,9, 15,7,13,5};
 static int dithmat2[4][4] = {0,8,0,8, 12,4,12,4, 0,8,0,8, 12,4,12,4};
  



void
main (int argc, char **argv)
{
    int j;
    float red, green, blue;
    unsigned long Y[4];
    unsigned long U[4];
    unsigned long V[4];
    int x,y;
	 int newArgc;
	 char **newArgv; // for parsing purposes
    SstRegs *sst;
	 FxU32 tmp;
	    SstIORegs *sstio ;
	    FxU32 stride;
	    FxU32 width = 640;
	    FxU32 height = 480;
		 ImgInfo img;
		 int imgthere=0;

       newArgv = argv;
       newArgc = argc;
		 // parse command line, and get image file
		 while ((--newArgc>0) && (**++newArgv))
		 {
		    if (newArgv[0][0] != '-')
		    continue;
		    if (newArgv[0][1] != 'x')
		    continue;

			 imgReadFile(newArgv[1],&img);
			 imgthere=1;
			 width = img.any.width;
			 height = img.any.height;

		 }
		 if(imgthere==0){ 
			printf("No image specified. usage: xxxx -x filename");
			return;
			}

    sst = SST_BEGIN(argc,argv);
	 sstio = (SstIORegs *)SST_IO_ADDRESS(sst);

	    sst_idle_really(sst);
		 // set the overlay buffer to point to the end of the Color buffer 
	    if ( diago.ytiled )
	       SET(sstio->vidProcCfg,SST_OVERLAY_PIXEL_YUYV422|SST_OVERLAY_TILED_EN);
		 else
		 // use dithered 565 mode
		    SET(sstio->vidProcCfg,SST_OVERLAY_PIXEL_YUYV422);
		 CSIM_PRIVATE(diago.sstCSIM)->io.vidCurrOverlayStartAddr 
			  =  GET(sst->colBufferAddr); // to the color buffer
		 stride = GET(sst->colBufferStride);
		 stride &= ~SST_BUFFER_MEMORY_TYPE;
		 SET(sstio->vidDesktopOverlayStride,stride<<SST_OVERLAY_STRIDE_SHIFT); 
		  // set desktop to point to front buffer

			sst_idle_really(sst);
	      tmp = GET(sstio->vidProcCfg);
			if ( diago.ytiled )
			SET(sstio->vidProcCfg,tmp | SST_DESKTOP_PIXEL_RGB24|SST_DESKTOP_TILED_EN);
			else
			SET(sstio->vidProcCfg,tmp | SST_DESKTOP_PIXEL_RGB24);
			SET(sstio->vidDesktopStartAddr,GET(sst->colBufferAddr));
			SET(sst->colBufferStride, width*3);
			stride = GET(sst->colBufferStride);
			stride &= ~SST_BUFFER_MEMORY_TYPE;
	      tmp = GET(sstio->vidDesktopOverlayStride);
			SET(sstio->vidDesktopOverlayStride,tmp | (stride<<SST_DESKTOP_STRIDE_SHIFT));
																																													  
	 tmp = GET(sstio->vidProcCfg);
	 tmp |= (SST_OVERLAY_EN);  //  overlay
	 tmp |= SST_OVERLAY_CLUT_BYPASS; // turn of clut
	 tmp |= SST_DESKTOP_CLUT_BYPASS; // turn of clut
	 tmp &= ~SST_DESKTOP_EN;    // desktop only.
	 tmp |= SST_OVERLAY_FILTER_BILINEAR;    
	 tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    
	 tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_VIDEO_PROCESSOR_EN;
	 SET(sstio->vidProcCfg,tmp);    
	 tmp = (img.any.width << SST_VIDEO_SCREEN_WIDTH_SHIFT ) 
		  | (img.any.height << SST_VIDEO_SCREEN_HEIGHT_SHIFT);
	 SET(sstio->vidScreenSize,tmp);    

     clutInit(sstio);


	  // video scaling attributes
	 tmp = (0xef<<(19-7))/2; //Zoom up by a little over half a pixel pixel  
	 SET(sstio->vidOverlayDudx,tmp);    
	 tmp = 1<<18; // starting address at 1/2
	 SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	 tmp = (0xcf<<(19-7)); //Zoom up by a little 
	 SET(sstio->vidOverlayDvdy,tmp);    
	 SET(sstio->vidOverlayDvdyOffset,0);    




         x=y=0;
	 for(y=0;y<(int)img.any.height; y++)
		 for(x=0; x<(int)img.any.width; x+=2)
	 {
		 for(j=0; j<2; j++)
               {
		         red = (float)(img.any.data[0+(4*(x+j+y*img.any.width))]);
		         green = (float)(img.any.data[1+(4*(x+j+y*img.any.width))] & 0xff); 
		         blue = (float)(img.any.data[2+(4*(x+j+y*img.any.width))] & 0xff);
               
                 Y[j] =(unsigned long) ((77.0/256.0)*red + (150.0/256.0)*green + (29.0/256.0)*blue);
                 U[j] =(unsigned long)(128-(44.0/256.0)*red-(87.0/256.0)*green+(131.0/256.0)*blue);
                 V[j]  =(unsigned long)(128+(131.0/256.0)*red-(110.0/256.0)*green-(21.0/256.0)*blue);
                } 
			tmp = ((U[0] + U[1])/2)<<24; 
			tmp |= ((V[0] + V[1])/2)<<8; 
			/* first pixel */
                        tmp |= (Y[1] << 16);      
			tmp |= (Y[0] << 0); 
		 CSIM_PIXEL_WR(CSIM_BUF_OVERLAY, x,y,tmp);
	 }

    // set video overlay start coords at 0, 0
	 tmp = (0<<SST_OVERLAY_X_SHIFT);
	 tmp |= (0<<SST_OVERLAY_Y_SHIFT);
	 SET(sstio->vidOverlayStartCoords,tmp);
	 // set video overlay end coords at 256, 256
	 tmp = ((width-1)<<SST_OVERLAY_X_SHIFT);
	 tmp   |= ((height-1)<<SST_OVERLAY_Y_SHIFT);
	 SET(sstio->vidOverlayEndScreenCoord,tmp);
										

    vidInit(width, height, sst);
    vidOut(); // render the display image
  	vidCleanup(sst);		// wait for the command to complete
	DIAG_PASS(1);
}

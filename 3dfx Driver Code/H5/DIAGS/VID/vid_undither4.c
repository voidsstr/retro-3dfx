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
** $Date: 10/11/00 8:19:52 PM$
*/
/* This test is a very simple test of the video backend, using the vidutils.c
file
*/

#include "udiag.h"
#include "sstdiag.h"
#include "vidutils.h"
#include "hsimio.h"
#include <fximg.h>

 // use a 4x4 dither, should probably have the option for 2x2 dither
 static int dithmat[4][4] = {0,8,2,10, 12,4,14,6, 3,11,1,9, 15,7,13,5};
 static int dithmat2[4][4] = {10,6,10,6, 2,14,2,14, 10,6,10,6, 2,14,2,14};
  
 unsigned short *
 sstDither565(unsigned short *out, unsigned int *data, int xsize, int ysize)
{
	 int d,n,x,y;
	 unsigned int t32;
	 unsigned short t16,*d16;
				 
	  if (out==NULL)
     out = (unsigned short *)malloc(xsize*ysize*sizeof(*d16));
		if (getenv("SST_TEX_DITHER2"))
		memcpy(dithmat,dithmat2,sizeof(dithmat2));
		 d16 = out;
		  for (y=0; y<ysize; y++) {
		     for (x=0; x<xsize; x++) {     // for each texel
	   	   t32 = *data++;         // get the 32 bit texel
#ifdef __unix__
      // byte swapping for unix machine
				t32 = ((t32 & 0xff000000) >>24)
					 | ((t32 & 0x00ff0000) >>8)
					 | ((t32 & 0x0000ff00) <<8)
					 | ((t32 & 0x000000ff) <<24);
#endif
				d = dithmat[y&3][x&3];
				n = (t32>>16) & 0xFF;     // get RED channel
				n = (int)(0x1F0/255.0F * n + 0.5F);   // scale
				n += d;          // add in dither
				t16 = (n>>4)<<11;
																						 
				n = (t32>>8) & 0xFF;      // get GREEN channel
				n = (int)(0x3F0/255.0F * n + 0.5F);   // scale
				n += d;          // add in dither
				t16 |= (n>>4)<<5;
			   n = t32 & 0xFF;        // get BLUE channel
			   n = (int)(0x1F0/255.0F * n + 0.5F);   // scale
			   n += d;          // add in dither
			   t16 |= (n>>4)<<0;
																																														  
			   *d16++ = t16;       // store the 16 bit texel
				}
		 }
	  return out;
}
																																																					


// draw a rectangular blit with 32-bit writes
void draw_blt32 (int x, int y, ImgInfo *img, SstRegs *hw)
{
int lfbMode;
	 int dy, sizeX;
	 unsigned long *lfb, *imgdata;
	 unsigned long data;

	 lfbMode = SST_LFB_565;
			
	 sizeX = img->any.width;
	 if ((lfbMode&SST_LFB_FORMAT) == SST_LFB_565) {
			  sizeX >>= 1;
			  //dy = 512;
			  dy = img->any.width*4;
	}
	 else dy = 1024;
										  
	// base address of linear frame buffer is 4 Meg
	 lfb = (unsigned long *) SST_LFB_ADDRESS(hw);
	 // add in x,y offset
		lfb += (y<<10) + x;
	 imgdata = (unsigned long *)img->any.data;
	  SET(hw->lfbMode, lfbMode);
		for (y=0; y<(signed)img->any.height; y++) {
	   	for (x=0; x<sizeX; x++) {
			data = *imgdata;
#ifdef __unix__
         data = ((data & 0xffff0000) >>16)
				  | ((data & 0xffff)<<16);
#endif
			 SET(lfb[x],data);
			  imgdata++;
			}
	  lfb += dy;
																												    }
}

void
main (int argc, char **argv)
{
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
			printf("No image specified. usage: xxxx -x vid_testimage_vid.ppm");
			return;
			}

    sst = SST_BEGIN(argc,argv);
	 sstio = (SstIORegs *)SST_IO_ADDRESS(sst);

	    sst_idle_really(sst);
		 // set the overlay buffer to point to the end of the Color buffer 
	    if ( diago.ytiled )
	       SET(sstio->vidProcCfg,SST_OVERLAY_PIXEL_RGB565D|SST_OVERLAY_TILED_EN);
		 else
		 // use dithered 565 mode
		    SET(sstio->vidProcCfg,SST_OVERLAY_PIXEL_RGB565D);
		 CSIM_PRIVATE(diago.sstCSIM)->io.vidCurrOverlayStartAddr 
			  =  GET(sst->colBufferAddr); // to the color buffer
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
	 tmp &= ~SST_DESKTOP_EN;    // and no desktop.
	 tmp |= SST_OVERLAY_CLUT_BYPASS | SST_DESKTOP_CLUT_BYPASS; // turn of clut
	 tmp |= SST_OVERLAY_FILTER_4X4;    
      tmp |= SST_VIDEO_PROCESSOR_EN;
	 SET(sstio->vidProcCfg,tmp);    
	 tmp = (width << SST_VIDEO_SCREEN_WIDTH_SHIFT ) 
		  | (height << SST_VIDEO_SCREEN_HEIGHT_SHIFT);
	 SET(sstio->vidScreenSize,tmp);    




    x=y=0;
    img.any.data = (unsigned char*) sstDither565(NULL,
			 (unsigned int *) img.any.data,
				 img.any.width, img.any.height);
	 draw_blt32(x,y,&img, sst);  

    // set video overlay start coords at 0, 0
	 tmp = (0<<SST_OVERLAY_X_SHIFT);
	 tmp |= (0<<SST_OVERLAY_Y_SHIFT);
	 SET(sstio->vidOverlayStartCoords,tmp);
	 // set video overlay end coords at 256, 256
	 tmp = ((width-1)<<SST_OVERLAY_X_SHIFT);
	 tmp   |= ((height-1)<<SST_OVERLAY_Y_SHIFT);
	 SET(sstio->vidOverlayEndScreenCoord,tmp);
										
    // set max threshold. Initially very big
    tmp = 0x000f0f0f;
    SET(sstio->vidMaxRGBDelta,tmp);

    vidInit(width, height, sst);
    vidOut(); // render the display image
  	vidCleanup(sst);		// wait for the command to complete
	DIAG_PASS(1);
}

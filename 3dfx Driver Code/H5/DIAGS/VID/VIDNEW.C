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
** $Date: 10/11/00 8:19:56 PM$
*/
/* This is the generic video backend test. It uses fixed patterns for Desktop,
   overlay, and cursor, and by using the -x option followed by a number between
   1 and 16, a different test is run. see the test_matrix.xls file in this 
   directory for which test does what.
*/
/*** Tests added after verif review.
Valid tests: 1, 2, 4, 10, 11, 12, 13, 14, 15, 16, 17, 18
****/

#include "udiag.h"
#include "sstdiag.h"
#include "vidutils.h"
#include "hsimio.h"
 
static int dithmat[4][4] = {0,8,2,10, 12,4,14,6, 3,11,1,9, 15,7,13,5};
static int dithmat2[4][4] = {0,8,0,8, 12,4,12,4, 0,8,0,8, 12,4,12,4};
  
/***************
   Current bugs (6/8/97):
   Tiled mode doesn't seem to work, for either overlay or desktop
   (test 1 looks funny).

   Test3 currently crahses, because the memorySizeIn Bytes < offset.
     Worked around by making the desktop run in lfb mode.

  vidtst7 & on: I'm not sure if the xadj, y adj are working, or are set correctly.
****************/

void
main (int argc, char **argv)
{
    int n;
    long x,y;
    SstRegs *sst;
    FxU32 tmp,tmp2, pixmode;
    SstIORegs *sstio ;
    FxU32 stride, stride2,stride3;
    FxU32 width;
    FxU32 height;
    FxU32 olyWidth, olyXOffset, olyXskew=0;
    FxU32 olyHeight, olyYOffset, olyYskew = 0;
	int newArgc;
	char **newArgv;

	int d;
	unsigned int t32; // temp storage
	int j;
    float red, green, blue, scale;
	unsigned long avgU, avgV;
    unsigned long Y[4];
    unsigned long U[4];
    unsigned long V[4];
	int test=1; // initially, will come in from a -x option.

       newArgv = argv;
       newArgc = argc;
		 // parse command line, and get image file
		 while ((--newArgc>0) && (**++newArgv))
		 {
		    if (newArgv[0][0] != '-')
		    continue;
		    if (newArgv[0][1] != 'x')
		    continue;
			test = atoi(newArgv[1]);
		 }

     scale =1.0; // default

      // generic initialization
      sst = SST_BEGIN(argc,argv);
      sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
      sst_idle_really(sst);

    //vidtst1 init: 
	switch(test)
	{
	case(1): // test 1: overlay at 0,0
      width = (81/8)*8;
      height = 33;
      olyWidth = 75;
      olyHeight = 30;
	  olyXOffset = 0;
	  olyYOffset = 0;

      tmp = 0;
      //tmp |= SST_CURSOR_X11;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
      tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_POINT; 
      tmp |= SST_DESKTOP_PIXEL_PAL8;
      tmp |= SST_OVERLAY_PIXEL_RGB565U;
	  //tmp |= SST_OVERLAY_CLUT_BYPASS;
      tmp |= SST_DESKTOP_TILED_EN;
      tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
		if( test==20) {
	      tmp |= SST_OVERLAY_CLUT_BYPASS;
	      tmp |= SST_DESKTOP_CLUT_BYPASS;
			tmp &= ~SST_CURSOR_EN;

			}
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (33 <<SST_CURSOR_X_SHIFT) 
                           | (20 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0x123456); 
      SET(sstio->hwCurC1, 0xedcba9);
	  
	   // video scaling attributes
	  tmp = 0x70000; 
	  scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x21000>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0xe0000);    
	  SET(sstio->vidOverlayDvdyOffset,0x44000>>1);  
	  
	     SET(sstio->vidChromaMin, 0x000000);
         SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
	  case(2): // cursor at upper left corner 
      width = ((180+16)/8)*8;
		height = 145;
      if(test==30) height=10;
      olyWidth = 177;
      olyHeight = 140;
	  olyXOffset = 11;
	  olyYOffset = 2;

      tmp = 0;
      if (test!=30) tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    
      if(test==30)tmp |= SST_OVERLAY_FILTER_BILINEAR; 
      else tmp |= SST_OVERLAY_FILTER_BILINEAR; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      if(test==2) tmp |= SST_OVERLAY_PIXEL_YUYV422;
		else tmp |= SST_OVERLAY_PIXEL_RGB565U;
	    tmp |= SST_OVERLAY_CLUT_BYPASS;
      //tmp |= SST_DESKTOP_TILED_EN;
      //  ektmp |= SST_OVERLAY_TILED_EN; 
      tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (63 <<SST_CURSOR_X_SHIFT) 
                           | (63 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xf00ff0); 
      SET(sstio->hwCurC1, 0x0ff00f);
	  
	   // video scaling attributes
	  tmp = 0x91111; 
	  scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x99333>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth  * 2.0* scale)+7)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0xff000);    
	  SET(sstio->vidOverlayDvdyOffset,0x55431>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);
      if(test==71) {
	  SET(sstio->vidChromaMin, 0x001021);
          SET(sstio->vidChromaMax, 0x008410);
      }

	 
	  break;
          case(4):
	  case(10): // one wide overlay at 8,0
	  case(11): // test 1: overlay at 1,0
	  case(12): // test 1: overlay at 2,0
	  case(13): // test 1: overlay at 2,0
	  case(14): // test 1: overlay at 2,0
	  case(15): // test 1: overlay at 2,0
	  case(16): // test 1: overlay at 2,0
	  case(17): // test 1: overlay at 2,0
	  case(18): // test 1: overlay at 2,0
      width = (100/8)*8;
      if(test==4) width=256;
      height = 25;
      olyWidth = 1;
      olyHeight = 23;
	  olyXOffset = 8+(test-10);
	  olyYOffset = 0;
      if(test==4) {
            olyWidth=200;
	  olyXOffset = 01;
	  olyYOffset = 10;
          olyHeight = 103;
          height = 123;
      }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      if(test==73) {
      }
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      //tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_BILINEAR; 
      tmp |= SST_DESKTOP_PIXEL_RGB32;
      tmp |= SST_OVERLAY_PIXEL_RGB565U;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (0 <<SST_CURSOR_X_SHIFT) 
                           | (0 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xa55aa5); 
      SET(sstio->hwCurC1, 0x5aa55a);
	  
	   // video scaling attributes
	  tmp = 0x01555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth  * 2.0* scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x032ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x123456);
      SET(sstio->vidChromaMax, 0x789abc);

	 
	  break;
	  case(5): // vidtst 5
          case(54):
      width = (320/8)*8;
      height = 200;
      olyWidth = 320;
      olyHeight = 200;
	  olyXOffset = 0;
	  olyYOffset = 0;
      if(test==54) {
         width = (640/8)*8;
         olyWidth = 640;
      }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      //tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  //tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_2X2; 
      tmp |= SST_DESKTOP_PIXEL_RGB32;
      tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
	  case(6): // vidtst 6
          case(55):
      width = (111/8)*8;
      height = 44;
      olyWidth = 600;
      olyHeight = 400;
	  olyXOffset = 0;
	  olyYOffset = 0;
      if(test==55) {
      width = (1104/8)*8;
      height = 444;
      }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      //tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      //tmp |= SST_OVERLAY_FILTER_2X2; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      //tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
     case(7): // vidtst 7
     case(56):
      width = (140/8)*8;
      height = 31;
      olyWidth = 130;
      olyHeight = 30;
	  olyXOffset = 1;
	  olyYOffset = 3;
	  olyXskew = 3;
	  olyYskew = 1;
	  //olyXskew = 0;
	  //olyYskew = 0;

      if(test==56)
      {
      	width = (540/8)*8;
      	height = 431;
      	olyWidth = 430;
      	olyHeight =330;
	      olyXskew = 2;
	      olyYskew = 2;
      }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
//	  tmp |= SST_DESKTOP_CLUT_BYPASS;
 //     tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
//	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_2X2; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
	  
	  case(117): // vidtst 17 - like 7, with point sampled filtering
      width = (140/8)*8;
      height = 31;
      olyWidth = 130;
      olyHeight = 30;
	  olyXOffset = 1;
	  olyYOffset = 3;
	  olyXskew = 3;
	  olyYskew = 1;

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      //tmp |= SST_OVERLAY_FILTER_2X2; 
      tmp |= SST_OVERLAY_FILTER_POINT; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565U;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   



	  // cursor register setup

	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
	  case(8): // vidtst 8
     case(57):
      width = (140/8)*8;
      height = 31;
      olyWidth = 130;
      olyHeight = 30;
	  olyXOffset = 2;
	  olyYOffset = 1;
	  olyXskew = 2;
	  olyYskew = 1;
     if(test==57){
       width = (840/8)*8;
       height = 631;
       olyWidth = 631;
       olyHeight =505;
	   olyXOffset = 105;
	   olyYOffset = 77;
	   olyXskew = 3;
	   olyYskew = 2;
     }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      //tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
//	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_2X2; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;

	  case(9): // vidtst 9
     case(58):
      width = (140/8)*8;
      height = 31;
      olyWidth = 130;
      olyHeight = 30;
	  olyXOffset = 3;
	  olyYOffset = 2;
	  olyXskew = 3;
	  olyYskew = 2;

     if(test==58){
      width = (360/8)*8;
      height = 222;
      olyWidth = 223;
      olyHeight = 139;
	   olyXOffset = 55;
	   olyYOffset = 67;
	   olyXskew = 0;
	   olyYskew = 3;
     }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
//	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
//	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_2X2; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
	  case(110): // vidtst 10
     case(59):
      width = (140/8)*8;
      height = 31; // ek shoudl be 31;
      olyWidth = 130;
      olyHeight = 30;
	  olyXOffset = 4;
	  olyYOffset = 0;
	  olyXskew = 0;
	  olyYskew = 0;
     if(test==59) {
       width = (240/8)*8;
      height = 151;
      olyWidth = 230;
      olyHeight = 130;
	   olyXOffset = 14;
	   olyYOffset = 10;
	   olyXskew = 1;
	   olyYskew = 1;
     }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  //tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_4X4; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
	   case(111): // vidtst 11
      case(60):
      width = (140/8)*8;
      height = 31;
      olyWidth = 130;
      olyHeight = 30;
	  olyXOffset = 3;
	  olyYOffset = 1;
	  olyXskew = 3;
	  olyYskew = 1;

     if(test==60){
       width = (540/8)*8;
       height = 231;
       olyWidth = 430;
       olyHeight = 130;
	    olyXOffset = 93;
	    olyYOffset = 91;
	    olyXskew = 0;
	    olyYskew = 2;
     }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  //tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  tmp |= SST_DESKTOP_CLUT_SELECT;
	  //tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_4X4; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;

	   case(121) : // vidtst 12
      case(61) :
      width = (140/8)*8;
      height = 31;
      olyWidth = 130;
      olyHeight = 30;
	   olyXOffset = 2;
	   olyYOffset = 2;
	   olyXskew = 2;
	   olyYskew = 2;

      if(test==61){
        width = (440/8)*8;
        height = 331;
        olyWidth = 430;
        olyHeight = 330;
	     olyXOffset = 3;
	     olyYOffset = 3;
	     olyXskew = 3;
	     olyYskew = 3;
      }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      //tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_4X4; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;

	    case(131) : // vidtst 13
       case(62):
      width = (140/8)*8;
      height = 31;
      olyWidth = 130;
      olyHeight = 30;
	   olyXOffset = 1;
	   olyYOffset = 3;
	   olyXskew = 1;
	   olyYskew = 3;

      if(test==62){
        width = (1140/8)*8;
        height = 931;
        olyWidth = 803;
        olyHeight = 730;
	     olyXOffset = 201;
	     olyYOffset = 203;
	     olyXskew = 2;
	     olyYskew = 0;
      }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  //tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_4X4; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565D;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;

	  
	    case(141) : // vidtst 14
       case(63):
      width = (66/8)*8;
      height = 31;
      olyWidth = 60;
      olyHeight = 30;
	  olyXOffset = 1;
	  olyYOffset = 3;
	  olyXskew = 1;
	  olyYskew = 3;

     if(test==63) {
       width = (466/8)*8;
       height = 231;
       olyWidth = 360;
       olyHeight = 130;
	    olyXOffset = 51;
	    olyYOffset = 53;
	    olyXskew = 2;
	    olyYskew = 0;
     }


      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
//	  tmp |= SST_DESKTOP_CLUT_BYPASS;
 //     tmp |= SST_OVERLAY_CLUT_BYPASS;
	 tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_POINT; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_YUV411;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 

      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;

	      case(151) : // vidtst 15
      case(64):
      width = (66/8)*8;
      height = 31;
      olyWidth = 60;
      olyHeight = 30;
	  olyXOffset = 1;
	  olyYOffset = 3;
	  olyXskew = 1;
	  olyYskew = 3;

     if(test==64){
      width = (166/8)*8;
      height = 131;
      olyWidth = 150;
      olyHeight = 120;
	  olyXOffset = 11;
	  olyYOffset = 13;
	  olyXskew = 2;
	  olyYskew = 0;

     }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_POINT; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_YUYV422;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
	      case(161) : // vidtst 16
      case(65):
      width = (66/8)*8;
      height = 31;
      olyWidth = 60;
      olyHeight = 30;
	  olyXOffset = 1;
	  olyYOffset = 3;
	  olyXskew = 1;
	  olyYskew = 3;

     if(test==65){
      width = (266/8)*8;
      height = 231;
      olyWidth = 260;
      olyHeight = 230;
	   olyXOffset = 1;
	   olyYOffset = 3;
	   olyXskew = 2;
	   olyYskew = 0;
     }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
      tmp |= SST_OVERLAY_FILTER_POINT; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_UYVY422;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
      //tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (62 <<SST_CURSOR_X_SHIFT) 
                           | (111 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
	   // video scaling attributes
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
	  break;
      default: 
          printf("test # %d doesn't exist", test);
          return(1);
    }

	   // set video overlay start coords at 11, 51
	 tmp = (olyXOffset<<SST_OVERLAY_X_SHIFT); 
	 tmp |= (olyYOffset<<SST_OVERLAY_Y_SHIFT);
	 tmp |= (olyXskew&0x3)<< SST_OVERLAY_XADJ_SHIFT;
     tmp |= (olyYskew&0x3)<< SST_OVERLAY_YADJ_SHIFT;
	 SET(sstio->vidOverlayStartCoords,tmp);    
	 // set video overlay end coords at 101, 71
	 tmp = ((olyXOffset+olyWidth)<<SST_OVERLAY_X_SHIFT); 
	 tmp   |= ((olyYOffset+olyHeight)<<SST_OVERLAY_Y_SHIFT);
	 SET(sstio->vidOverlayEndScreenCoord,tmp);
	 
	 tmp = ((pixmode & SST_OVERLAY_CLUT_BYPASS)==0); 
	 tmp |=((pixmode & SST_DESKTOP_CLUT_BYPASS)==0);
      // test independant stuff      
      if(tmp != 0) // clut used
      {
         clutInit(sstio); //Initialize the color lookup tables
      }

       

      stride = (width+4)& 0xfffffff8; // word aligned
	  switch(pixmode& (0x7<<SST_DESKTOP_PIXEL_FORMAT_SHIFT))
               {
                 case(SST_DESKTOP_PIXEL_RGB565): 
                   stride = stride*2;
                   break;
                 case(SST_DESKTOP_PIXEL_PAL8): 
                   stride= stride*2;
                   break;
                 case(SST_DESKTOP_PIXEL_RGB24): // nothing to do.
				   stride = stride*3;
                   break;
                 case(SST_DESKTOP_PIXEL_RGB32):  // nothing to do.
				   stride = stride*4;
                   break;
               }
               stride2 = stride;
					if(pixmode &  SST_DESKTOP_TILED_EN)
						 stride2 = (stride + 0x7f) >> 7; // round and shift 
	  SET(sst->colBufferStride, stride2);
	   tmp = 0x100000 + GET(sst->colBufferAddr);
      CSIM_PRIVATE(diago.sstCSIM)->io.vidCurrOverlayStartAddr 
			  = tmp ; // one meg into it.

      printf("%s, %d: Setting overlay buffer location = 0x%x\n",
				 __FILE__, __LINE__, tmp);
	 ;
      SET(sst->leftOverlayBuf, tmp); // one meg into it.
      SET(sst->swapbufferCMD,0); // swap the buffer, so that left and write match

               stride3 = stride;
					if(pixmode &  SST_OVERLAY_TILED_EN)
						 stride3 = (stride + 0x7f) >> 7; // round and shift 
      SET(sstio->vidDesktopOverlayStride,(stride3<<SST_OVERLAY_STRIDE_SHIFT) | (stride2<<SST_DESKTOP_STRIDE_SHIFT));
      SET(sstio->vidDesktopStartAddr,GET(sst->colBufferAddr));
      sst_idle_really(sst);																																											  
	 
      tmp = (width << SST_VIDEO_SCREEN_WIDTH_SHIFT ) 
          | (height << SST_VIDEO_SCREEN_HEIGHT_SHIFT);
      SET(sstio->vidScreenSize,tmp);    
	   SET(sstio->hwCurPatAddr
			  , 0x100000 + GET(sst->colBufferAddr)); // one meg into it.
        
    // set max threshold. Initially very big
    tmp = 0x00100810;
    SET(sstio->vidMaxRGBDelta,tmp);

      // initialize vidproccfg
      SET(sstio->vidProcCfg,pixmode);   
   
   // Place data in overlay buffer
   // Data is a smooth-ramping color with horizontal and vertical bars.
	 for(y=0; y<((int)olyHeight+1); y++)
	 {
	 for(x=0; x<((int)olyWidth+1); x++)
	   {
                
	       tmp = (((x*y*7)>>7)& 0x17f) | ((x<<4) & 0x80); 
	       //if (tmp>255) tmp=255;
	       tmp = (tmp<<16) | ((255-tmp)<< 8) | ((128-tmp) & 0xff);
	       if((x%11==0)) tmp |= 0xffffff; // white stripes
	       if((y%9==0)) tmp ^= 0xffffff; // invert data
			 // ek-final test change- just use random data
			 tmp2 = (tmp2 <<1 ) || (((tmp2>>11) ^ (tmp2>>14)) & 1); //lfsr
			 // turned off tmp = tmp2;
		   red = (float)((tmp>>16)& 0xff);
		   green = (float)((tmp>>8)& 0xff);
		   blue = (float)((tmp>>0)&0xff);
               // convert to current data format
               switch(pixmode& (0x7<<SST_OVERLAY_PIXEL_FORMAT_SHIFT)) {
                 case(SST_OVERLAY_PIXEL_RGB565U): 
                   tmp = (tmp & 0xf80000)>>8 | (tmp & 0xfc00)>>5 | (tmp & 0x1f);
                   SET_PIXEL(CSIM_BUF_OVERLAY, x, y, tmp);
				  
				   break;
                 case(SST_OVERLAY_PIXEL_YUV411):
				   j=x&3;
               
                   Y[j] =(unsigned long) ((77.0/256.0)*red + (150.0/256.0)*green + (29.0/256.0)*blue);
                   U[j] =(unsigned long)(128-(44.0/256.0)*red-(87.0/256.0)*green+(131.0/256.0)*blue);
                   V[j]  =(unsigned long)(128+(131.0/256.0)*red-(110.0/256.0)*green-(21.0/256.0)*blue);
			       avgU = (U[0] + U[1] + U[2] + U[3])/4; 
			       avgV = (V[0] + V[1] + V[2] + V[3])/4;
				   if(j==3)
				   {
		 
      				/**** Attempted fix ported from vid_411 ***/
                  // ek - minor chroma hack
						avgV = (U[0] + U[1] + U[2] + U[3])/4; 
						avgU = (V[0] + V[1] + V[2] + V[3])/4; 
                   /** end of fix*/
		
						/* first, second pixel */
      				tmp = (Y[0] << 0) | ((avgU & 0xc0) | ((avgV & 0xc0) >> 2))<<8;     
						tmp |= (Y[1] << 16) | (((avgU << 2) & 0x0c0) | (avgV & 0x030))<<24;  
		  				SET_PIXEL(CSIM_BUF_OVERLAY, x-3,y,tmp);
         
						/* 3rd pixel, fourth pixel */
         			tmp = (Y[2] << 0) | (((avgU << 4) & 0x0c0) | ((avgV << 2)& 0x030))<<8;     
         			tmp |= (Y[3] << 16) | (((avgU << 6) & 0x0c0) | ((avgV << 4)& 0x030))<<24;
		 				SET_PIXEL(CSIM_BUF_OVERLAY, x+2-3,y,tmp);
	               }
                   break;
                 case(SST_OVERLAY_PIXEL_UYVY422): 
					 j=x&1;
                     Y[j] =(unsigned long) ((77.0/256.0)*red + (150.0/256.0)*green + (29.0/256.0)*blue);
                     U[j] =(unsigned long)(128-(44.0/256.0)*red-(87.0/256.0)*green+(131.0/256.0)*blue);
                     V[j]  =(unsigned long)(128+(131.0/256.0)*red-(110.0/256.0)*green-(21.0/256.0)*blue);
                     if(j==1) // got two values
					 {
					   tmp = ((U[0] + U[1])/2)<<16; 
			           tmp |= (V[0] + V[1])/2; 
			           /* first pixel */
                       tmp |= (Y[1] << 24);      
			           tmp |= (Y[0] << 8);
					   SET_PIXEL(CSIM_BUF_OVERLAY, x-1, y, tmp);
					 }
                   break;
                 case(SST_OVERLAY_PIXEL_YUYV422):
					 j=x&1;
                     Y[j] =(unsigned long) ((77.0/256.0)*red + (150.0/256.0)*green + (29.0/256.0)*blue);
                     U[j] =(unsigned long)(128-(44.0/256.0)*red-(87.0/256.0)*green+(131.0/256.0)*blue);
                     V[j]  =(unsigned long)(128+(131.0/256.0)*red-(110.0/256.0)*green-(21.0/256.0)*blue);
                     if(j==1) // got two values
					 {
					   tmp = ((U[0] + U[1])/2)<<24; 
			           tmp |= ((V[0] + V[1])/2)<<8; 
			           /* first pixel */
                       tmp |= (Y[1] << 16);      
			           tmp |= (Y[0] << 0);
					   SET_PIXEL(CSIM_BUF_OVERLAY, x-1, y, tmp);
					 }
					 

                   break;
                 case(SST_OVERLAY_PIXEL_RGB565D): 
                   // check for 2x2 vs 4x4 dithering.
				   if(pixmode&SST_OVERLAY_FILTER_2X2)
				     d = dithmat2[y&3][x&3];
				   else d=dithmat[y&3][x&3];
				   t32=tmp;
				   n = (t32>>16) & 0xFF;     // get RED channel
			       n = (n-(n>>5))<<1;
				   n += d;          // add in dither
				   tmp  = (n>>4)<<11;
																						 
				   n = (t32>>8) & 0xFF;      // get GREEN channel
			       n = (n-(n>>6))<<2;
				   n += d;          // add in dither
				   tmp |= (n>>4)<<5;
			       n = t32 & 0xFF;        // get BLUE channel
			       n = (n-(n>>5))<<1;
			       n += d;          // add in dither
			       tmp |= (n>>4)<<0;
				   SET_PIXEL(CSIM_BUF_OVERLAY, x, y, tmp);
				   break;
               }
	       
	   }
	 }

   // Place data in desktop buffer
   // Data is a smooth-ramping color with horizontal and vertical bars.
	 for(y=0; y<(int)height; y++)
	 {
	 for(x=0; x<(int)width; x++)
	   {
                
	       tmp = ((x*y)>>7)& 0x1ff; 
	       if (tmp>255) tmp=255;
	       tmp = ((255-tmp)<<16) | ((x>>2)<< 8) | ((x<<3) & 0xff); 
				  // lsbs follow addr.
	       if(((x+y)%11==0)) tmp |= 0xffffff; // white stripes
	       if(((x-y)%9==0)) tmp ^= 0xffffff; // invert data
               // convert to current data format
               switch(pixmode& (0x7<<SST_DESKTOP_PIXEL_FORMAT_SHIFT))
               {
                 case(SST_DESKTOP_PIXEL_RGB565): 
                   tmp = (tmp & 0xf80000)>>8 | (tmp & 0xfc00)>>5 | (tmp & 0x1f);
                   break;
                 case(SST_DESKTOP_PIXEL_PAL8): 
                   tmp = tmp & 0xff; // one color;
                   break;
                 case(SST_DESKTOP_PIXEL_RGB24): // nothing to do. 
                   break;
                 case(SST_DESKTOP_PIXEL_RGB32):  // nothing to do.
                   break;
               }
	       SET_PIXEL(CSIM_BUF_DESKTOP, x, y, tmp);
	   }
	 }


     if((test!=17) & (test!=7))
	 {
    // Setup cursor
	 for(y=0; y<64; y++)
	 {
	 for(x=0; x<64; x++)
	   {
               tmp = (((x>>5) & 1) ^ ((x>>4) & 1) ^ ((x>>3) & 1)
                   ^ ((x>>2) & 1) ^ ((x>>1) & 1) ^ ((x>>0) & 1)
                   ^ ((y>>5) & 1) ^ ((y>>4) & 1) ^ ((y>>3) & 1)
                   ^ ((y>>2) & 1) ^ ((y>>1) & 1) ^ ((y>>0) & 1));
               tmp |= (((x>>5) & 1) ^ ((x>>4) & 1) ^ ((x>>3) & 1)
                   ^ ((x>>2) & 1) ^ ((x>>1) & 1) ^ ((x>>0) & 1)
                   | ((y>>5) & 1) ^ ((y>>4) & 1) ^ ((y>>3) & 1)
                   ^ ((y>>2) & 1) ^ ((y>>1) & 1) ^ ((y>>0) & 1))<<1;

	       SET_PIXEL(CSIM_BUF_CURSOR, x, y, tmp);
  
           }
         }
	 }

    vidInit(width, height,sst);
    vidOut(); // render the display image
  	vidCleanup(sst);		// wait for the command to complete
    DIAG_PASS(1);
}

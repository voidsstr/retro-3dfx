/* Copyright (c) 1995, 3Dfx Interactive, Inc.  All Rights Reserved.
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
** $Date: 10/11/00 8:19:58 PM$ 
*/
/* This is the generic video backend test. It uses fixed patterns for Desktop,
   overlay, and cursor, and by using the -x option followed by a number between
   1 and 16, a different test is run. see the test_matrix.xls file in this 
   directory for which test does what.
*/

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
    FxU32 olyBaseOffset = 0, desktopBaseOffset = 0;
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
	case(1):
	case(20):
	case(50): // big tests
	case(70): // big tests
	case(101): // desktop/overlay base offset
	case(102): // desktop/overlay base offset
      width = (91/8)*8;
      height = 33;
      olyWidth = 75;
      olyHeight = 30;
      if(test==50) {
           width = (411/8)*8;
           height=300;
           olyWidth = 375;
           olyHeight = 255;
      }
      if(test==70) {
           width = (411/8)*8;
           height=300;
           olyWidth = 375;
           olyHeight = 255;
      }
      if(test==101) {
	desktopBaseOffset = 14;
	olyBaseOffset = 12;
      }
      if(test==102) {
	desktopBaseOffset = 15;
	olyBaseOffset = 14;
      }
	  olyXOffset = 11;
	  olyXOffset = 11;
	  olyYOffset = 2;

      tmp = 0;
      tmp |= SST_CURSOR_X11;
      tmp |= SST_CHROMA_EN;
      if(test==70) tmp |= SST_CHROMA_INVERT;
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
      tmp |= SST_CURSOR_EN;
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
	  case(2): 
	  case(30): 
	  case(51): 
	  case(71): 
      width = (190/8)*8;
		height = 45;
      if(test==30) height=10;
      olyWidth = 166;
      olyHeight = 40;
      if(test==30) olyHeight=7;
	  olyXOffset = 11;
	  olyYOffset = 2;
      if(test==30) {olyXOffset=2;
              width = (1280/8)*8;
              olyWidth = 1152;
	      olyXOffset = 111;
	  }
      if(test==51) {
         width = (1280/8)*8;
         olyWidth = 1152;
      }
      if(test==71) {
         width = (800/8)*8;
         olyWidth = 700;
      }

      tmp = 0;
      if (test!=30) tmp |= SST_CURSOR_MICROSOFT;
      if (test!=30) tmp |= SST_CHROMA_EN;
	  if (test!=30) tmp |= SST_CHROMA_INVERT;
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  tmp |= SST_DESKTOP_CLUT_BYPASS;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    
      if(test==30)tmp |= SST_OVERLAY_FILTER_BILINEAR; 
      else tmp |= SST_OVERLAY_FILTER_BILINEAR; 
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      if(test==2)tmp |= SST_OVERLAY_PIXEL_YUV411;
		else tmp |= SST_OVERLAY_PIXEL_RGB565U;
	    tmp |= SST_OVERLAY_CLUT_BYPASS;
      //tmp |= SST_DESKTOP_TILED_EN;
      //  ektmp |= SST_OVERLAY_TILED_EN; 
      tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (111 <<SST_CURSOR_X_SHIFT) 
                           | (60 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xf00ff0); 
      SET(sstio->hwCurC1, 0x0ff00f);
	  
	   // video scaling attributes
	  tmp = 0x91111; 
	  scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x99333>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth  * 2.0* scale)+3+2)<<SST_OVERLAY_FETCH_SIZE_SHIFT; // ek- added two more bits to see if it fixes the problem
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
	  case(3):
	  case(21):
	  case(52):
	  case(72):
  	  case(103):
	  case(104):
	  case(105):
	  case(106):
	  case(1052): // swan
	  case(1053):
      width = (111/8)*8;
      height = 30;
      olyWidth = 80;
      olyHeight = 28;
	  olyXOffset = 0;
	  olyYOffset = 1;
      if(test==52) {
          height = 1024;
          olyHeight = 901;
	  olyYOffset = 101;
      }
		if((test==1052) || (test==1053))  { // swan
		width = (640/8)*8;
      height = 480;
      olyWidth = 280;
      olyHeight = 128;
	  olyXOffset = 20;
	  olyYOffset = 20;
		}
		if(test==1053)
		  height = 5;
      if(test==72) {
          width = (400/8)*8;
          height = 300;
          olyHeight = 330;
          olyWidth = 280;
	  olyXOffset = 22;
      }
      if(test==103) {
	desktopBaseOffset = 12;
	olyBaseOffset = 4;
      }
      if(test==104) {
	desktopBaseOffset = 13;
	olyBaseOffset = 12;
      }
      if(test==105) {
	desktopBaseOffset = 14;
	olyBaseOffset = 12;
      }
      if(test==106) {
	desktopBaseOffset = 15;
	olyBaseOffset = 4;
      }
      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      if(test==72) {
      tmp |= SST_CHROMA_EN;
      tmp |= SST_CHROMA_INVERT;
      }
      tmp |= SST_DESKTOP_EN; 
		if(test!=1053)
		  tmp |= SST_OVERLAY_EN;  
		if(test==1053)
		  tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  tmp |= SST_DESKTOP_CLUT_SELECT;
	  //tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      if(test!=1052) { // swan
      tmp |= SST_OVERLAY_VERT_SCALE_EN;    
		}
      tmp |= SST_OVERLAY_FILTER_BILINEAR; 
      tmp |= SST_DESKTOP_PIXEL_RGB24;
		if(test==1052) { // swan
		tmp |= SST_OVERLAY_PIXEL_RGB565U;
		}
		else {
      tmp |= SST_OVERLAY_PIXEL_YUYV422;
		}
	  //tmp |= SST_OVERLAY_CLUT_BYPASS;
      //tmp |= SST_DESKTOP_TILED_EN;
      //tmp |= SST_OVERLAY_TILED_EN; 
		if(test!=1053)
		  tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (71 <<SST_CURSOR_X_SHIFT) 
                           | (91 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0xa55aa5); 
      SET(sstio->hwCurC1, 0x5aa55a);
	  
	   // video scaling attributes
	  tmp = 0x91111;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x77001>>1; // only 19 bits, 
	  tmp |= ((int)((float)olyWidth  * 2.0* scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x75111);    
	  SET(sstio->vidOverlayDvdyOffset,0x1>>1);  
	  
	  SET(sstio->vidChromaMin, 0x123456);
      SET(sstio->vidChromaMax, 0x789abc);

	 
	  break;
	  case(4): // vidtst 4
	  case(53):
	  case(73):
      width = (351/8)*8;
      height = 25;
      olyWidth = 319;
      olyHeight = 23;
	  olyXOffset = 21;
	  olyYOffset = 0;
      if(test==53) {
        height = 255;
        olyHeight = 239;
      }
      if(test==73) {
          width = (400/8)*8;
          height = 300;
          olyHeight = 330;
          olyWidth = 280;
	  olyXOffset = 22;
      }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      if(test==73) {
      tmp |= SST_CHROMA_EN;
      }
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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
      tmp |= SST_OVERLAY_PIXEL_UYVY422;
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
	  tmp = 0x75555;
	  if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	      scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x2>>1; // only 19 bits
	  tmp |= ((int)((float)olyWidth  * 2.0* scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0x432ff);    
	  SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
	  SET(sstio->vidChromaMin, 0x123456);
      SET(sstio->vidChromaMax, 0x789abc);

	 
	  break;
	  case(5): // vidtst 5
	case(54):
	case(54000):
	case(54001):
	case(54002):
      width = (320/8)*8;
      height = 200;
      olyWidth = 320;
      olyHeight = 200;
	  olyXOffset = 0;
	  olyYOffset = 0;
	  if(test == 54002) {
		 olyXOffset = 1;
		 olyYOffset = 0;
	  }
      if((test==54) || (test==54000) || (test==54001)) {
         width = (640/8)*8;
         olyWidth = 640;
      }
		if(test==54002) {
		  width = (640/8)*8;
		  olyWidth = 639;
      }

      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
      //tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
	  //tmp |= SST_DESKTOP_CLUT_BYPASS;
      tmp |= SST_OVERLAY_CLUT_BYPASS;
	  //tmp |= SST_DESKTOP_CLUT_SELECT;
	  tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;    
		if((test == 54000) || (test==54002))
		  tmp |= SST_OVERLAY_FILTER_BILINEAR; 
		else 
		  tmp |= SST_OVERLAY_FILTER_2X2; 
      tmp |= SST_DESKTOP_PIXEL_RGB32;
		if((test == 54000) || (test==54001) || (test==54002))
		  tmp |= SST_OVERLAY_PIXEL_UYVY422;
		else
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
      olyWidth = 1; // since vidOverlayEndScreenCoord is olyWidth-1
      olyHeight = 400;
	  olyXOffset = 0;
	  olyYOffset = 0;
      if(test==55) {
      width = (1104/8)*8;
      height = 444;
      }
	
      tmp = 0;
      tmp |= SST_CURSOR_MICROSOFT;
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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
	  
	  case(17): // vidtst 17 - like 7, with point sampled filtering
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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
	  case(10): // vidtst 10
	  case(1010): // 1555D overlay on 1555U desktop, 4x4, no scaling, chroma, LUT, cursor, tiled
	  case(1011): // 1555D overlay on 1555U desktop, 2x2, no scaling, chroma, LUT, cursor, tiled
	  case(1012): // 1555D overlay on 1555U desktop, 4x4, no scaling, chroma, LUT, cursor, tiled
	  case(1013): // 1555D overlay on 1555U desktop, 2x2, no scaling, chroma, LUT, cursor, tiled
	  case(1014):
	  case(1015):
	  case(1016):
	  case(1017):
	  case(1018):
	  case(1019):
	  case(1020):
	  case(1021):
	  case(1022):
	  case(1023):
	  case(1024):
	  case(1025):

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
			width = (270/8)*8;
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
		if((test == 1010) || (test == 1011) || (test == 1012) || (test == 1013) ||
			(test == 1014) || (test == 1015) || (test == 1016) || (test == 1017) ||
			(test == 1018) || (test == 1019) || (test == 1020) || (test == 1021) ||
			(test == 1022) || (test == 1023) || (test == 1024) || (test == 1025)) {
		  tmp |= SST_CHROMA_EN;
		}
		if((test == 1018) || (test == 1019) || (test == 1020) || (test == 1021) ||
			(test == 1022) || (test == 1023) || (test == 1024) || (test == 1025)) {
		  tmp |= SST_CHROMA_INVERT;
		}
      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  
		if((test != 1010) && (test != 1011) && (test != 1012) && (test != 1013) &&
			(test != 1014) && (test != 1015) && (test != 1016) && (test != 1017) &&
			(test != 1018) && (test != 1019) && (test != 1020) && (test != 1021) &&
			(test != 1022) && (test != 1023) && (test != 1024) && (test != 1025)) {
		  tmp |= SST_DESKTOP_CLUT_BYPASS;
		  tmp |= SST_OVERLAY_CLUT_BYPASS;
		}
		//tmp |= SST_DESKTOP_CLUT_SELECT;
		//tmp |= SST_OVERLAY_CLUT_SELECT;
      //tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      //tmp |= SST_OVERLAY_VERT_SCALE_EN;
		if((test == 1011) || (test == 1013) || (test == 1015) || (test == 1017) ||
			(test == 1019) || (test == 1021) || (test == 1023) || (test == 1025)) {
		  tmp |= SST_OVERLAY_FILTER_2X2;
		}
		else {
		  tmp |= SST_OVERLAY_FILTER_4X4;
		}
      if((test == 1010) || (test == 1011) || (test == 1012) || (test == 1013) ||
			(test == 1014) || (test == 1015) || (test == 1016) || (test == 1017) ||
			(test == 1018) || (test == 1019) || (test == 1020) || (test == 1021) ||
			(test == 1022) || (test == 1023) || (test == 1024) || (test == 1025))  {
		  tmp |= SST_DESKTOP_PIXEL_RGB1555U;
		  tmp |= SST_OVERLAY_PIXEL_RGB1555D;
		} else {
		  tmp |= SST_DESKTOP_PIXEL_RGB565;
		  tmp |= SST_OVERLAY_PIXEL_RGB565D;
		}
		if((test == 1010) || (test == 1011) || (test == 1012) || (test == 1013) ||
			(test == 1014) || (test == 1015) || (test == 1016) || (test == 1017) ||
			(test == 1018) || (test == 1019) || (test == 1020) || (test == 1021) ||
			(test == 1022) || (test == 1023) || (test == 1024) || (test == 1025)) {
		  tmp |= SST_DESKTOP_TILED_EN;
		  tmp |= SST_OVERLAY_TILED_EN; 
		  tmp |= SST_CURSOR_EN;
		}
		tmp |= SST_VIDEO_PROCESSOR_EN;

		if((test == 1014) || (test == 1015) || (test == 1016) || (test == 1017) ||
			(test == 1022) || (test == 1023) || (test == 1024) || (test == 1025)) {
		  tmp |= SST_USE_ALPHA_BIT;
		}
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
	   case(11): // vidtst 11
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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

	   case(12) : // vidtst 12
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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

	    case(13) : // vidtst 13
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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

	  
	    case(14) : // vidtst 14
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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

	      case(15) : // vidtst 15
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
      width = (186/8)*8;
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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
	      case(16) : // vidtst 16
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
      //tmp |= SST_CHROMA_EN;
	  //tmp |= SST_CHROMA_INVERT;
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


	  /*******************************
		 new tests for 1555 and 32bpp
		 *****************************/

// all the following tests are scaling enabled, thus should not use 4x4 or 2x2 filter
// test 1555U, 565, 32bpp with chroma, alpha bit, cursor and LUT in tile space.
	case(1501): // small 32bpp overlay on 32bpp
	case(3201): // large 32bpp overlay on 32bpp
	case(1511): // small 1555U overlay on 1555U (just add cursor and LUT)
					// no chroma-key, clut bypass, no cursor
	case(1512): // small 1555U overlay on 1555U (turn off cursor and LUT)
					// no chroma-key, clut bypass, no cursor
	case(1513): // small 1555U overlay on 1555U with chroma enabled
               // chroma-key, clut bypass, no cursor
	case(15133): // small 1555U overlay on 1555U with chroma enabled
               // chroma-key, clut bypass, no cursor, bilinear
	case(1514): // small 1555U overlay on 16bpp undithered with chroma enabled
					// chroma-key, clut not bypass, has cursor
	case(1515): // small 1555U overlay on 32bpp undithered with chroma enabled
					// chroma-key, clut not bypass, has cursor, bilinear

	  /*********** test alpha bit *****/

	case(1516): // small 1555U overlay on 1555U with chroma and alpha bit disabled
	case(1517): // small 1555U overlay on 1555U with chroma and alpha bit enabled
	case(1518): // small 16bpp undithered overlay on 1555U with chroma and alpha bit enabled, bilinear
	case(1519): // small 32bpp undithered overlay on 1555U with chroma and alpha bit enabled
// ALL bypass clut and no cursor

	  /***********************/

	case(1521): // small 16bpp overlay on 16bpp
					// no chroma-key,  clut not bypass, has cursor

	  /************ 64MB address space for video out **********/

	case(1601): // small 32bpp overlay on 32bpp with 64MB desktop + 64MB overlay
               // chroma-key, clut not bypass, cursor
	case(1602): // small 32bpp overlay on 32bpp with 64MB desktop + 64MB overlay
               // chroma-key, clut not bypass, cursor
	case(1603): // small 32bpp overlay on 32bpp with 64MB desktop + 64MB overlay
               // chroma-key, clut not bypass, cursor
	case(101603): // small 32bpp overlay on 32bpp with 64MB desktop + 64MB overlay
               // chroma-key, clut not bypass, cursor
	case(1001603): // small 32bpp overlay on 32bpp with 64MB desktop + 64MB overlay
               // chroma-key, clut not bypass, cursor
	case(10001603): // small 32bpp overlay on 32bpp with 64MB desktop + 64MB overlay
               // chroma-key, clut not bypass, cursor
	case(100001603): // small 32bpp overlay on 32bpp with 64MB desktop + 64MB overlay
               // chroma-key, clut not bypass, cursor

	  case (1701):
	  case (1702):
	  case (1703):
	  case (1704):
	  case (1705):
	  case (1706):
	  case (1707):
	  case (1708):
	  case (1709):
	  case (1710):
	  case (1711):
	  case (1712):
	  case (1713):
	  case (1714):
	  case (1715):
	  case (1716):
	  case (1717):
	  case (1718):
	  case (1719):
	  case (1720):
	  case (1721):
	  case (1722):
	  case (1723):
	  case (1724):

      width = (91/8)*8;
      height = 33;
      olyWidth = 75;
      olyHeight = 30;
      if(test==3201) {
           width = (411/8)*8;
           height=300;
           olyWidth = 375;
           olyHeight = 255;
      }
      if(test==1603) {
           width = (3000/8)*8;
           height=30;
           olyWidth = 2600;
           olyHeight = 15;
      }
      if(test==101603) {
           width = (2044/8)*8;
           height=30;
           olyWidth = 260;
           olyHeight = 15;
      }
      if(test==100001603) {
           width = (2046/8)*8;
           height=30;
           olyWidth = 260;
           olyHeight = 15;
      }
      if(test==1001603) {
           width = (2048/8)*8;
           height=30;
           olyWidth = 1600;
           olyHeight = 15;
      }
      if(test==10001603) {
           width = (2048/8)*8;
           height=30;
           olyWidth = 1600;
           olyHeight = 15;
      }

	  olyXOffset = 11;
	  olyXOffset = 11;
	  olyYOffset = 2;

      tmp = 0;
      tmp |= SST_CURSOR_X11;
		if(test!=1511 && test!=1512 && test!=1521 && test!=1707 && test!=1708 && test!=1709 && test!=1710
			&& test!=1711 && test!=1712 && test!=1714 && test!=1716 && test!=1718 && test!=1720 && test!=1722 && test!=1724) tmp |= SST_CHROMA_EN;
      if((test==3201) || (test==1701) || (test==1703) || (test==1705) 
			||  (test==1707) || (test==1709) || (test==1711) || (test==1713)
			||  (test==1715) || (test==1719) || (test==1721) || (test==1723))
		  tmp |= SST_CHROMA_INVERT;

      tmp |= SST_DESKTOP_EN; 
      tmp |= SST_OVERLAY_EN;  



      tmp |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      tmp |= SST_OVERLAY_VERT_SCALE_EN;    

		if ((test==15133) || (test==1515)  || (test==1518)) {
		  tmp |= SST_OVERLAY_FILTER_BILINEAR; 
		} else {
		  tmp |= SST_OVERLAY_FILTER_POINT; 
		}

		if((test==1511) || (test==1512) || (test==1513) || (test==15133) || (test==1701) || (test==1704) || (test==1707) || (test==1710)) {
      tmp |= SST_DESKTOP_PIXEL_RGB1555U;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} else if((test==1702) || (test==1705) || (test==1708) || (test==1711)) {
      tmp |= SST_DESKTOP_PIXEL_RGB1555U;
      tmp |= SST_OVERLAY_PIXEL_RGB1555D;
		} else if((test==1703) || (test==1706) || (test==1709) || (test==1712)) {
      tmp |= SST_DESKTOP_PIXEL_RGB1555U;
      tmp |= SST_OVERLAY_PIXEL_RGB32U;
		} else if(test==1713) {
      tmp |= SST_DESKTOP_PIXEL_PAL8;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} else if(test==1714) {
      tmp |= SST_DESKTOP_PIXEL_RGB565 ;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} else if(test==1715) {
      tmp |= SST_DESKTOP_PIXEL_RGB24;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} else if(test==1716) {
      tmp |= SST_DESKTOP_PIXEL_RGB32;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} else if(test==1717) {
      tmp |= SST_DESKTOP_PIXEL_PAL8;
      tmp |= SST_OVERLAY_PIXEL_RGB1555D;
		} else if(test==1718) {
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB1555D;
		} else if(test==1719) {
      tmp |= SST_DESKTOP_PIXEL_RGB24;
      tmp |= SST_OVERLAY_PIXEL_RGB1555D;
		} else if(test==1720) {
      tmp |= SST_DESKTOP_PIXEL_RGB32;
      tmp |= SST_OVERLAY_PIXEL_RGB1555D;
		} else if(test==1721) {
      tmp |= SST_DESKTOP_PIXEL_PAL8;
      tmp |= SST_OVERLAY_PIXEL_RGB32U;
		} else if(test==1722) {
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB32U;
		} else if(test==1723) {
      tmp |= SST_DESKTOP_PIXEL_RGB24;
      tmp |= SST_OVERLAY_PIXEL_RGB32U;
		} else if(test==1724) {
      tmp |= SST_DESKTOP_PIXEL_RGB32;
      tmp |= SST_OVERLAY_PIXEL_RGB32U;
		} else if(test==1514) {
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} else if(test==1515) {
      tmp |= SST_DESKTOP_PIXEL_RGB32;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} 

		/******** tmp delete */
		else if(test==1516) {
      tmp |= SST_DESKTOP_PIXEL_RGB1555U;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} else if(test==1517) {
      tmp |= SST_DESKTOP_PIXEL_RGB1555U;
      tmp |= SST_OVERLAY_PIXEL_RGB1555U;
		} else if(test==1518) {
      tmp |= SST_DESKTOP_PIXEL_RGB1555U;
      tmp |= SST_OVERLAY_PIXEL_RGB565U;
		} else if(test==1519) {
      tmp |= SST_DESKTOP_PIXEL_RGB1555U;
      tmp |= SST_OVERLAY_PIXEL_RGB32U;
		} 
		/*********/

		else if(test==1521) {
      tmp |= SST_DESKTOP_PIXEL_RGB565;
      tmp |= SST_OVERLAY_PIXEL_RGB565U;
		} else if(test==10001603) {
      tmp |= SST_DESKTOP_PIXEL_RGB24;
      tmp |= SST_OVERLAY_PIXEL_RGB565U;
		} else {
      tmp |= SST_DESKTOP_PIXEL_RGB32;
      tmp |= SST_OVERLAY_PIXEL_RGB32U;
		}


	  //tmp |= SST_OVERLAY_CLUT_BYPASS;
	   if(test!=1602)
		  tmp |= SST_DESKTOP_TILED_EN;
		if(test!=1601)
		  tmp |= SST_OVERLAY_TILED_EN; 
		tmp |= SST_CURSOR_EN;
		tmp |= SST_VIDEO_PROCESSOR_EN;



		/******** tmp delete */
		if ((test==1517)  || (test==1518) || (test==1519) || (test==1704) || (test==1705) || (test==1706) || (test==1710) || (test==1711) || (test==1712)                )
		  tmp |= SST_USE_ALPHA_BIT;
		else
		  tmp &= ~SST_USE_ALPHA_BIT; // turn off alpha bit

		/**********/


		if( (test == 1511)  || (test == 1512) ||  (test == 1513) || (test == 15133) ||
			 (test == 1516)  || (test == 1517) ||  (test == 1518) || (test == 1519) ||
			 (test == 1603)  || (test == 101603) || (test == 1001603) || (test == 10001603) || (test == 100001603)) {
	      tmp |= SST_OVERLAY_CLUT_BYPASS;
	      tmp |= SST_DESKTOP_CLUT_BYPASS;
			tmp &= ~SST_CURSOR_EN;
			}
		if( (test == 1701)  || (test == 1703) ||  (test == 1705) || (test == 1708) ||
			 (test == 1710)  || (test == 1712) ||  (test == 1714) || (test == 1716) ||
			 (test == 1718)  || (test == 1720) ||  (test == 1722) || (test == 1724)) {
	      tmp |= SST_OVERLAY_CLUT_BYPASS;
	      tmp |= SST_DESKTOP_CLUT_BYPASS;
			}

		if( (test == 1702)  || (test == 1704) ||  (test == 1706) || (test == 1707) ||
			 (test == 1709)  || (test == 1711) ||  (test == 1713) || (test == 1715) ||
			 (test == 1717)  || (test == 1719) ||  (test == 1721) || (test == 1723)) {
			tmp &= ~SST_CURSOR_EN;
			}
      pixmode = tmp;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

	  // cursor register setup
	  SET(sstio->hwCurLoc, (33 <<SST_CURSOR_X_SHIFT) 
                           | (20 << SST_CURSOR_Y_SHIFT)); 
	  if(test == 1603)
		 SET(sstio->hwCurLoc, (2580 <<SST_CURSOR_X_SHIFT) | (20 << SST_CURSOR_Y_SHIFT)); 
	  if(test == 101603)
		 SET(sstio->hwCurLoc, (100 <<SST_CURSOR_X_SHIFT) | (20 << SST_CURSOR_Y_SHIFT)); 
	  if(test == 1001603)
		 SET(sstio->hwCurLoc, (1000 <<SST_CURSOR_X_SHIFT) | (20 << SST_CURSOR_Y_SHIFT)); 
      SET(sstio->hwCurC0, 0x123456); 
      SET(sstio->hwCurC1, 0xedcba9);
	  
	   // video scaling attributes
	  tmp = 0x70000; 
	  scale = ((float)tmp)/(float)0x100000; // 0.20 number
	  SET(sstio->vidOverlayDudx,tmp);    
	  tmp = 0x21000>>1; // only 19 bits, 
	  switch(pixmode& (0x7<<SST_OVERLAY_PIXEL_FORMAT_SHIFT)) {
	     case(SST_OVERLAY_PIXEL_RGB32U):
			 tmp |= ((int)((float)olyWidth * 4.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
			 break;
	     default:
			 tmp |= ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
			 break;
	  }
	  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
	  SET(sstio->vidOverlayDvdy,0xe0000);    
	  SET(sstio->vidOverlayDvdyOffset,0x44000>>1);  
	  
	     SET(sstio->vidChromaMin, 0x000000);
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
	 tmp = ((olyXOffset+olyWidth-1)<<SST_OVERLAY_X_SHIFT); 
 // new changes to set end coord to overlaywidth-1
	 tmp   |= ((olyYOffset+olyHeight-1)<<SST_OVERLAY_Y_SHIFT);
	 SET(sstio->vidOverlayEndScreenCoord,tmp);
	 
	 tmp = ((pixmode & SST_OVERLAY_CLUT_BYPASS)==0); 
	 tmp |=((pixmode & SST_DESKTOP_CLUT_BYPASS)==0);
      // test independant stuff      
      if(tmp != 0) // clut used
      {
         clutInit(sstio); //Initialize the color lookup tables
      }

       

      stride = (width+4)& 0xfffffff8; // word aligned

	  /************** temporary change for avenger+ to hardcode the desktop and overlay stride to stride*4 bytes
	  switch(pixmode& (0x7<<SST_DESKTOP_PIXEL_FORMAT_SHIFT))
               {
                 case(SST_DESKTOP_PIXEL_RGB1555U): 
                   stride = stride*2;
                   break;
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
					**********************************************************************************/
				   stride = stride*4; // hardcode the stride
               stride2 = stride;
					if(pixmode &  SST_DESKTOP_TILED_EN)
						 stride2 = (stride + 0x7f) >> 7; // round and shift 
	  SET(sst->colBufferStride, stride2);

	  // set up overlay start address
	  GDBG_INFO(0,"olyBaseOffset = 0x%x\n",olyBaseOffset);
	  tmp = GET(sst->colBufferAddr) + 0x100000 + olyBaseOffset;
	  CSIM_PRIVATE(diago.sstCSIM)->io.vidCurrOverlayStartAddr = tmp ; // one meg into it.	  
	  printf("%s, %d: Setting overlay buffer location = 0x%x\n",
		 __FILE__, __LINE__, tmp);

	  if((test == 1601) || (test == 1602))
		 tmp = 0x03700680; // 64MB address

	  SET(sst->leftOverlayBuf, tmp); // one meg into it.


	  SET(sst->swapbufferCMD,0); // swap the buffer, so that left and write match
	  

	  stride3 = stride;
	  if(pixmode &  SST_OVERLAY_TILED_EN)
	    stride3 = (stride + 0x7f) >> 7; // round and shift 
	  SET(sstio->vidDesktopOverlayStride,(stride3<<SST_OVERLAY_STRIDE_SHIFT) | (stride2<<SST_DESKTOP_STRIDE_SHIFT));
	  
	  // set up desktop start address
	  GDBG_INFO(0,"desktopBaseOffset = 0x%x\n",desktopBaseOffset);


	  if((test == 1601) || (test == 1602))
		 SET(sstio->vidDesktopStartAddr, 0x03000680); // 64MB address
	  else 
		 SET(sstio->vidDesktopStartAddr, GET(sst->colBufferAddr) + desktopBaseOffset);

	  sst_idle_really(sst);

	  tmp = (width << SST_VIDEO_SCREEN_WIDTH_SHIFT ) 
	    | (height << SST_VIDEO_SCREEN_HEIGHT_SHIFT);
	  SET(sstio->vidScreenSize,tmp);    

	  if(test==1052) // swan
		 SET(sstio->hwCurPatAddr, GET(sst->colBufferAddr) + 0x200000); // two meg into it.
	  else if((test == 1601) || (test == 1602))																					  
		 SET(sstio->hwCurPatAddr, 0x03a00680); // 64MB
	  else
		 SET(sstio->hwCurPatAddr, GET(sst->colBufferAddr) + 0x100000); // one meg into it.
        
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
                

		  switch(test) // swan
			 {
			 case(1052): // diags for bringup
				if((x == y) || (x == (int)olyWidth-y))  // a black X
				  tmp = 0;
			   else
				  tmp = 0xffffff;
			 break;

			 default:
			 // original pattern
	       tmp = (((x*y*7)>>7)& 0x17f) | ((x<<4) & 0x80); 
	       //if (tmp>255) tmp=255;
	       tmp = (tmp<<16) | ((255-tmp)<< 8) | ((128-tmp) & 0xff);
	       if((x%11==0)) tmp |= 0xffffff; // white stripes
	       if((y%9==0)) tmp ^= 0xffffff; // invert data
			 // ek-final test change- just use random data
			 tmp2 = (tmp2 <<1 ) || (((tmp2>>11) ^ (tmp2>>14)) & 1); //lfsr
			 // turned off tmp = tmp2;
			 }


		   red = (float)((tmp>>16)& 0xff);
		   green = (float)((tmp>>8)& 0xff);
		   blue = (float)((tmp>>0)&0xff);

               // convert to current data format
               switch(pixmode& (0x7<<SST_OVERLAY_PIXEL_FORMAT_SHIFT)) {
                 case(SST_OVERLAY_PIXEL_RGB32U): 
                   SET_PIXEL(CSIM_BUF_OVERLAY, x, y, tmp);
						 break;
                 case(SST_OVERLAY_PIXEL_RGB1555U): 
                   tmp = (tmp & 0xf80000)>>9 | (tmp & 0xf800)>>6 | (tmp & 0x1f);
 // the B channel I think should be (tmp & f8) >> 3
                   SET_PIXEL(CSIM_BUF_OVERLAY, x, y, tmp);
 // this is overlay -- the alpha bit should not matter
						 break;
                 case(SST_OVERLAY_PIXEL_RGB565U): 
                   tmp = (tmp & 0xf80000)>>8 | (tmp & 0xfc00)>>5 | (tmp & 0x1f);
 // the B channel I think should be (tmp & f8) >> 3
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
					   tmp = ((U[0] + U[1])/2); 
			           tmp |= ((V[0] + V[1])/2)<<16; 
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
					   tmp = ((U[0] + U[1])/2)<<8; 
			           tmp |= ((V[0] + V[1])/2)<<24; 
			           /* first pixel */
                       tmp |= (Y[1] << 16);      
			           tmp |= (Y[0] << 0);
					   SET_PIXEL(CSIM_BUF_OVERLAY, x-1, y, tmp);
					 }
					 

                   break;
                 case(SST_OVERLAY_PIXEL_RGB1555D): 
                   // check for 2x2 vs 4x4 dithering.
				   if(pixmode&SST_OVERLAY_FILTER_2X2)
				     d = dithmat2[y&3][x&3];
				   else d=dithmat[y&3][x&3];
				   t32=tmp;
				   n = (t32>>16) & 0xFF;     // get RED channel
			       n = (n-(n>>5))<<1;
				   n += d;          // add in dither
				   tmp  = (n>>4)<<10;
      // This is overlay -- the alpha bit should not matter
																						 
				   n = (t32>>8) & 0xFF;      // get GREEN channel
			       n = (n-(n>>5))<<1;
				   n += d;          // add in dither
				   tmp |= (n>>4)<<5;

			       n = t32 & 0xFF;        // get BLUE channel
			       n = (n-(n>>5))<<1;
			       n += d;          // add in dither
			       tmp |= (n>>4)<<0;
				   SET_PIXEL(CSIM_BUF_OVERLAY, x, y, tmp);
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
                

		  switch(test) // swan
			 {
			 case(1052): // diags for bringup
			 //case(1603):
			 //case(101603):
			 //case(1001603):
			 //case(10001603):
				if(x <= width/3) // Red
				  tmp = 0x0000ff;
			   else if((x > width/3) && (x <= 2*width/3)) // Green
				  tmp = 0x00ff00;
			   else
				  tmp = 0xff0000; //Blue
printf("%s, %d: SWAN_DEBUG1: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
			 break;

			 case(1053): // diags for DAC linearity
				if(x <= 255)
				  tmp = (x << 16) + (x << 8) + (x << 0);
			   else if((x > 255) && (x < 510))
				  tmp = ((255-(x-255)) << 16) + ((255-(x-255)) << 8) + ((255-(x-255)) << 0);
			   else
				  tmp = 0x000000; 
printf("%s, %d: SWAN_DEBUG2: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
			 break;


			 default:
	       tmp = ((x*y)>>7)& 0x1ff; 
printf("%s, %d: SWAN_DEBUG2a: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
	       if (tmp>255) tmp=255;
	       tmp = ((255-tmp)<<16) | ((x>>2)<< 8) | ((x<<3) & 0xff); 
				  // lsbs follow addr.
printf("%s, %d: SWAN_DEBUG2b: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
	       if(((x+y)%11==0)) tmp |= 0xffffff; // white stripes
printf("%s, %d: SWAN_DEBUG2c: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
	       if(((x-y)%9==0)) tmp ^= 0xffffff; // invert data
printf("%s, %d: SWAN_DEBUG3: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
			 break;
																 }


               // convert to current data format
               switch(pixmode& (0x7<<SST_DESKTOP_PIXEL_FORMAT_SHIFT))
               {
                 case(SST_DESKTOP_PIXEL_RGB1555U): 
                   tmp = (tmp & 0xf80000)>>9 | (tmp & 0xfc00)>>6 | (tmp & 0x1f);
 // the B channel I think should be (tmp & f8) >> 3
																													 
						 /********* tmp delete */
                   if(x%2 == 1) tmp = tmp | 0x8000; // set the alpha bit for every other pixel
						 printf("%s, %d: ALPHA BIT: desktop 1555U pixel (%x, %x) is initialized to = 0x%x\n", __FILE__, __LINE__, x, y, tmp);
	                /***************************************/

printf("%s, %d: SWAN_DEBUG4: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);

                   break;
 // the alpha bit is set to 0 for now
                 case(SST_DESKTOP_PIXEL_RGB565): 
                   tmp = (tmp & 0xf80000)>>8 | (tmp & 0xfc00)>>5 | (tmp & 0x1f);
 // the B channel I think should be (tmp & f8) >> 3
printf("%s, %d: SWAN_DEBUG5: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
                   break;
                 case(SST_DESKTOP_PIXEL_PAL8): 
                   tmp = tmp & 0xff; // one color;
printf("%s, %d: SWAN_DEBUG6: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
                   break;
                 case(SST_DESKTOP_PIXEL_RGB24): // nothing to do. 
printf("%s, %d: SWAN_DEBUG7: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
                   break;
                 case(SST_DESKTOP_PIXEL_RGB32):  // nothing to do.
printf("%s, %d: SWAN_DEBUG8: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
                   break;
               }
printf("%s, %d: SWAN_DEBUG9: tmp is = 0x%x\n", __FILE__, __LINE__, tmp);
	       SET_PIXEL(CSIM_BUF_DESKTOP, x, y, tmp);
						 printf("%s, %d: SWAN_DEBUG: desktop pixel (%x, %x) is initialized to = 0x%x\n", __FILE__, __LINE__, x, y, tmp);
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
	

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
** $Date: 10/11/00 8:20:02 PM$ 
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

void rgb_to_yuv411();
void rgb_to_uyvy422();
void rgb_to_yuyv422();
void rgb_to_rgb565d_2x2();
void rgb_to_rgb565d_4x4();
void rgb_to_rgb565u();
  
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
  long x,y;
  SstRegs *sst;
  FxU32 tmp, tmp2, pixmode;
  SstIORegs *sstio ;
  FxU32 stride, stride2,stride3;
  FxU32 width;
  FxU32 height;
  FxU32 olyWidth, olyXOffset, olyXskew=0;
  FxU32 olyHeight, olyYOffset, olyYskew = 0;
  FxU32 olyBaseOffset = 0, desktopBaseOffset = 0;
  FxU32 r, g, b;

  FxU32 vidProcCfg;
  FxU32 hwCurLoc;
  FxU32 dudxOffset;

  int test=1; // initially, will come in from a -x option.

  float scale = 1.0f; // default

  // generic initialization
  diago.vid = 1;
  sst = SST_BEGIN(argc,argv);
  test = diago.option;
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_X11;
      vidProcCfg |= SST_CHROMA_EN;
      if(test==70) vidProcCfg |= SST_CHROMA_INVERT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;    
      vidProcCfg |= SST_OVERLAY_FILTER_POINT; 
      vidProcCfg |= SST_DESKTOP_PIXEL_PAL8;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565U;
      vidProcCfg |= SST_DESKTOP_TILED_EN;
      vidProcCfg |= SST_OVERLAY_TILED_EN; 
      vidProcCfg |= SST_CURSOR_EN;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      if( test==20) {
	vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
	vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
	vidProcCfg &= ~SST_CURSOR_EN;

      }
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (33 <<SST_CURSOR_X_SHIFT) | (20 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0x123456); 
      SET(sstio->hwCurC1, 0xedcba9);
	  
      // video scaling attributes
      tmp = 0x70000; 
      scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x21000>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      if (test!=30) vidProcCfg |= SST_CURSOR_MICROSOFT;
      if (test!=30) vidProcCfg |= SST_CHROMA_EN;
      if (test!=30) vidProcCfg |= SST_CHROMA_INVERT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;    
      if(test==30)vidProcCfg |= SST_OVERLAY_FILTER_BILINEAR; 
      else vidProcCfg |= SST_OVERLAY_FILTER_BILINEAR; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      if(test==2)vidProcCfg |= SST_OVERLAY_PIXEL_YUV411;
      else vidProcCfg |= SST_OVERLAY_PIXEL_RGB565U;
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_CURSOR_EN;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (111 <<SST_CURSOR_X_SHIFT) | (60 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xf00ff0); 
      SET(sstio->hwCurC1, 0x0ff00f);
	  
      // video scaling attributes
      tmp = 0x91111; 
      scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x99333>>1; // only 19 bits, 
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
      if(test==1052) { // swan
	width = (640/8)*8;
	height = 480;
	olyWidth = 256;
	olyHeight = 256;
	olyXOffset = 20;
	olyYOffset = 20;
      }
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
      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      if(test==72) {
	vidProcCfg |= SST_CHROMA_EN;
	vidProcCfg |= SST_CHROMA_INVERT;
      }
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_DESKTOP_CLUT_SELECT;
      if(test!=1052) { // swan
	vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;    
      }
      vidProcCfg |= SST_OVERLAY_FILTER_BILINEAR; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB24;
      if(test==1052) { // swan
	vidProcCfg |= SST_OVERLAY_PIXEL_RGB565U;
      }
      else {
	vidProcCfg |= SST_OVERLAY_PIXEL_YUYV422;
      }
      vidProcCfg |= SST_CURSOR_EN;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (71 <<SST_CURSOR_X_SHIFT) | (91 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xa55aa5); 
      SET(sstio->hwCurC1, 0x5aa55a);
	  
      // video scaling attributes
      tmp = 0x91111;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x77001>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      if(test==73) {
	vidProcCfg |= SST_CHROMA_EN;
      }
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;    
      vidProcCfg |= SST_OVERLAY_FILTER_BILINEAR; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB32;
      vidProcCfg |= SST_OVERLAY_PIXEL_UYVY422;
      vidProcCfg |= SST_CURSOR_EN;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (0 <<SST_CURSOR_X_SHIFT) | (0 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xa55aa5); 
      SET(sstio->hwCurC1, 0x5aa55a);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
      SET(sstio->vidOverlayDvdy,0x432ff);    
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_FILTER_2X2; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB32;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
      vidProcCfg |= SST_CURSOR_EN;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
      SET(sstio->vidOverlayDvdy,0x432ff);    
      SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
      SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
      break;
    case(6): // vidtst 6
    case(55):
      width = (111/8)*8;
      height = 44;
      olyWidth = 0;
      olyHeight = 400;
      olyXOffset = 0;
      olyYOffset = 0;
      if(test==55) {
	width = (1104/8)*8;
	height = 444;
      }

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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

      if(test==56)
	{
	  width = (540/8)*8;
	  height = 431;
	  olyWidth = 430;
	  olyHeight =330;
	  olyXskew = 2;
	  olyYskew = 2;
	}

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_OVERLAY_FILTER_2X2; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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
      
      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_FILTER_POINT; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565U;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_FILTER_2X2; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_FILTER_2X2; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
      SET(sstio->vidOverlayDvdy,0x432ff);    
      SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
      SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
      break;
    case(10): // vidtst 10
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_FILTER_4X4; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_DESKTOP_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_FILTER_4X4; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_FILTER_4X4; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_FILTER_4X4; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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


      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_FILTER_POINT; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_YUV411;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 

      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_FILTER_POINT; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_YUYV422;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
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

      vidProcCfg = 0;
      vidProcCfg |= SST_CURSOR_MICROSOFT;
      vidProcCfg |= SST_DESKTOP_EN; 
      vidProcCfg |= SST_OVERLAY_EN;  
      vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
      vidProcCfg |= SST_OVERLAY_CLUT_SELECT;
      vidProcCfg |= SST_OVERLAY_FILTER_POINT; 
      vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
      vidProcCfg |= SST_OVERLAY_PIXEL_UYVY422;
      vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
      pixmode = vidProcCfg;
      SET(sstio->vidProcCfg,(pixmode & ~SST_VIDEO_PROCESSOR_EN));   

      // cursor register setup
      hwCurLoc = (62 <<SST_CURSOR_X_SHIFT) | (111 << SST_CURSOR_Y_SHIFT); 
      SET(sstio->hwCurC0, 0xff00ff); 
      SET(sstio->hwCurC1, 0x00ff00);
	  
      // video scaling attributes
      tmp = 0x75555;
      if(pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
	scale = ((float)tmp)/(float)0x100000; // 0.20 number
      SET(sstio->vidOverlayDudx,tmp);    
      dudxOffset = 0x2>>1; // only 19 bits, 
      SET(sstio->vidOverlayDvdy,0x432ff);    
      SET(sstio->vidOverlayDvdyOffset,0x2>>1);  
	  
      SET(sstio->vidChromaMin, 0x000077);
      SET(sstio->vidChromaMax, 0x000077);

	 
      break;
    default: 
      printf("test # %d doesn't exist", test);
      return(1);
    }


  // BEGIN *********** Phil's new code *******************
  width = 640; height = 480;
  if ( olyWidth+olyXOffset > 640 ) {
    olyWidth = 640;
    olyXOffset = 0;
  }
  if ( olyHeight+olyYOffset > 480 ) {
    olyHeight = 480;
    olyYOffset = 0;
  }

  SET(sstio->hwCurLoc, hwCurLoc);
  SET(sstio->vidOverlayDudxOffsetSrcWidth,
      dudxOffset | ((int)((float)olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT);


  // END *********** Phil's new code *******************

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
	 
  // test independant stuff      
  if ( (pixmode & SST_DESKTOP_PIXEL_PAL8) ||
       (pixmode & SST_OVERLAY_CLUT_BYPASS)==0 || 
       (pixmode & SST_DESKTOP_CLUT_BYPASS)==0 )
    clutInit(sstio); //Initialize the color lookup tables
       

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

  // set up overlay start address
  GDBG_INFO(0,"olyBaseOffset = 0x%x\n",olyBaseOffset);
  tmp = GET(sst->colBufferAddr) + 0x180000 + olyBaseOffset;
  CSIM_PRIVATE(diago.sstCSIM)->io.vidCurrOverlayStartAddr = tmp ; // one meg into it.	  
  printf("%s, %d: Setting overlay buffer location = 0x%x\n",
	 __FILE__, __LINE__, tmp);
  SET(sst->leftOverlayBuf, tmp); // one meg into it.


  SET(sst->swapbufferCMD,0); // swap the buffer, so that left and write match
	  
  stride3 = stride;
  if(pixmode &  SST_OVERLAY_TILED_EN)
    stride3 = (stride + 0x7f) >> 7; // round and shift 
  SET(sstio->vidDesktopOverlayStride,(stride3<<SST_OVERLAY_STRIDE_SHIFT) | (stride2<<SST_DESKTOP_STRIDE_SHIFT));
	  
  // set up desktop start address
  GDBG_INFO(0,"desktopBaseOffset = 0x%x\n",desktopBaseOffset);
  SET(sstio->vidDesktopStartAddr, GET(sst->colBufferAddr) + desktopBaseOffset);
  sst_idle_really(sst);

  tmp = (width << SST_VIDEO_SCREEN_WIDTH_SHIFT ) 
    | (height << SST_VIDEO_SCREEN_HEIGHT_SHIFT);
  SET(sstio->vidScreenSize,tmp);    

  if(diago.halInfo->hw) // swan
    SET(sstio->hwCurPatAddr, GET(sst->colBufferAddr) + 0x300000); // two meg into it.
  else
    SET(sstio->hwCurPatAddr, GET(sst->colBufferAddr) + 0x100000); // one meg into it.
        
  // set max threshold. Initially very big
  tmp = 0x00100810;
  SET(sstio->vidMaxRGBDelta,tmp);

  // initialize vidproccfg
  SET(sstio->vidProcCfg,pixmode);   
   
  // Place data in overlay buffer
  // Data is a smooth-ramping color with horizontal and vertical bars.

  if ( diago.imgFilename ) {    // load overlay from file
	switch(pixmode& (0x7<<SST_OVERLAY_PIXEL_FORMAT_SHIFT)) {
	case(SST_OVERLAY_PIXEL_RGB565U): 
	  DIAG_LOADIMAGE(diago.imgFilename,CSIM_BUF_OVERLAY,rgb_to_rgb565u);
	  break;
	case(SST_OVERLAY_PIXEL_YUV411):
	  DIAG_LOADIMAGE(diago.imgFilename,CSIM_BUF_OVERLAY,rgb_to_yuv411);
	  break;
	case(SST_OVERLAY_PIXEL_UYVY422): 
	  DIAG_LOADIMAGE(diago.imgFilename,CSIM_BUF_OVERLAY,rgb_to_uyvy422);
	  break;
	case(SST_OVERLAY_PIXEL_YUYV422):
	  DIAG_LOADIMAGE(diago.imgFilename,CSIM_BUF_OVERLAY,rgb_to_yuyv422);
	  break;
	case(SST_OVERLAY_PIXEL_RGB565D): 
	  if (pixmode&SST_OVERLAY_FILTER_2X2)
	    DIAG_LOADIMAGE(diago.imgFilename,CSIM_BUF_OVERLAY,rgb_to_rgb565d_2x2);
	  else
	    DIAG_LOADIMAGE(diago.imgFilename,CSIM_BUF_OVERLAY,rgb_to_rgb565d_4x4);
	  break;
	}
  } else {
    for( y=0; y<((int)olyHeight+1); y++ ) {
      for( x=0; x<((int)olyWidth+1); x++ ) {

	if ( diago.halInfo->hw ) {
	  if((y % 2) == 0) {
	    if((x % 2) == 0)
	      tmp = 0;
	    else
	      tmp = 0xffffff;
	  } else {
	    if((x % 2) == 0)
	      tmp = 0xffffff;
	    else
	      tmp = 0x0;
	  }
	} else {
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
	
	r = (tmp>>16) & 0xff;
	g = (tmp>>8) & 0xff;
	b = (tmp>>0) & 0xff;

	// convert to current data format
	switch(pixmode& (0x7<<SST_OVERLAY_PIXEL_FORMAT_SHIFT)) {
	case(SST_OVERLAY_PIXEL_RGB565U): 
	  rgb_to_rgb565u(CSIM_BUF_OVERLAY, x, y, r, g, b);
	  break;
	case(SST_OVERLAY_PIXEL_YUV411):
	  rgb_to_yuv411(CSIM_BUF_OVERLAY, x, y, r, g, b);
	  break;
	case(SST_OVERLAY_PIXEL_UYVY422): 
	  rgb_to_uyvy422(CSIM_BUF_OVERLAY, x, y, r, g, b);
	  break;
	case(SST_OVERLAY_PIXEL_YUYV422):
	  rgb_to_yuyv422(CSIM_BUF_OVERLAY, x, y, r, g, b);
	  break;
	case(SST_OVERLAY_PIXEL_RGB565D): 
	  // check for 2x2 vs 4x4 dithering.
	  if (pixmode&SST_OVERLAY_FILTER_2X2)
	    rgb_to_rgb565d_2x2(CSIM_BUF_OVERLAY, x, y, r, g, b);
	  else 
	    rgb_to_rgb565d_4x4(CSIM_BUF_OVERLAY, x, y, r, g, b);
	  break;
	}
	
      }
    }
  }

  // Place data in desktop buffer
  // Data is a smooth-ramping color with horizontal and vertical bars.
  for ( y=0; y<(int)height; y++ ) {
    for ( x=0; x<(int)width; x++ ) {
                
	  if ( diago.halInfo->hw ) {
	    if(x <= width/3) // Blue
	      tmp = 0x0000ff;
	    else if((x > width/3) && (x <= 2*width/3)) // Green
	      tmp = 0x00ff00;
	    else
	      tmp = 0xff0000; // Red
	  } else {
	    tmp = ((x*y)>>7)& 0x1ff; 
	    if (tmp>255) tmp=255;
	    tmp = ((255-tmp)<<16) | ((x>>2)<< 8) | ((x<<3) & 0xff); 
	    // lsbs follow addr.
	    if(((x+y)%11==0)) tmp |= 0xffffff; // white stripes
	    if(((x-y)%9==0)) tmp ^= 0xffffff; // invert data
	  }

	  // convert to current data format
	  switch(pixmode&SST_DESKTOP_PIXEL_FORMAT) {
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
  
  
  if((test!=17) & (test!=7)) {
    // Setup cursor
    for ( y=0; y<64; y++ ) {
      for ( x=0; x<64; x++ ) {
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

void rgb_to_yuv411( int buffer, int x, int y, int r, int g, int b )
{
  static unsigned long Y[4], U[4], V[4];
  unsigned long avgU, avgV;
  int j = x&3;
  float red=(float)r, green=(float)g, blue=(float)b;
  FxU32 tmp;

  Y[j] =(unsigned long) ((77.0/256.0)*red + (150.0/256.0)*green + (29.0/256.0)*blue);
  U[j] =(unsigned long)(128-(44.0/256.0)*red-(87.0/256.0)*green+(131.0/256.0)*blue);
  V[j]  =(unsigned long)(128+(131.0/256.0)*red-(110.0/256.0)*green-(21.0/256.0)*blue);
  if (j==3) {
    
    /**** Attempted fix ported from vid_411 ***/
    // ek - minor chroma hack
    avgV = (U[0] + U[1] + U[2] + U[3])/4; 
    avgU = (V[0] + V[1] + V[2] + V[3])/4; 
    /** end of fix*/
    
    /* first, second pixel */
    tmp = (Y[0] << 0) | ((avgU & 0xc0) | ((avgV & 0xc0) >> 2))<<8;     
    tmp |= (Y[1] << 16) | (((avgU << 2) & 0x0c0) | (avgV & 0x030))<<24;  
    SET_PIXEL(buffer, x-3, y, tmp);
    
    /* 3rd pixel, fourth pixel */
    tmp = (Y[2] << 0) | (((avgU << 4) & 0x0c0) | ((avgV << 2)& 0x030))<<8;     
    tmp |= (Y[3] << 16) | (((avgU << 6) & 0x0c0) | ((avgV << 4)& 0x030))<<24;
    SET_PIXEL(buffer, x+2-3, y, tmp);
  }
}

void rgb_to_uyvy422( int buffer, int x, int y, int r, int g, int b )
{
  static unsigned long Y[4], U[4], V[4];
  int j = x&1;
  float red=(float)r, green=(float)g, blue=(float)b;
  FxU32 tmp;

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
      SET_PIXEL(buffer, x-1, y, tmp);
    }
}

void rgb_to_yuyv422( int buffer, int x, int y, int r, int g, int b )
{
  static unsigned long Y[2], U[2], V[2];
  int j = x&1;
  float red=(float)r, green=(float)g, blue=(float)b;
  FxU32 tmp;

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
      SET_PIXEL(buffer, x-1, y, tmp);
    }
}

void rgb_to_rgb565d_2x2( int buffer, int x, int y, int r, int g, int b )
{
  FxU32 tmp;
  int n, d;
  
  d = dithmat2[y&3][x&3]; // 2x2 dither matrix

  n = r;     // get RED channel
  n = (n-(n>>5))<<1;
  n += d;          // add in dither
  tmp  = (n>>4)<<11;
  
  n = g;      // get GREEN channel
  n = (n-(n>>6))<<2;
  n += d;          // add in dither
  tmp |= (n>>4)<<5;

  n = b;        // get BLUE channel
  n = (n-(n>>5))<<1;
  n += d;          // add in dither
  tmp |= (n>>4)<<0;

  SET_PIXEL(buffer, x, y, tmp);
}

void rgb_to_rgb565d_4x4( int buffer, int x, int y, int r, int g, int b )
{
  FxU32 tmp;
  int n, d;

  d = dithmat[y&3][x&3];    // 4x4 dither matrix

  n = r;     // get RED channel
  n = (n-(n>>5))<<1;
  n += d;          // add in dither
  tmp  = (n>>4)<<11;
  
  n = g;      // get GREEN channel
  n = (n-(n>>6))<<2;
  n += d;          // add in dither
  tmp |= (n>>4)<<5;

  n = b;        // get BLUE channel
  n = (n-(n>>5))<<1;
  n += d;          // add in dither
  tmp |= (n>>4)<<0;

  SET_PIXEL(buffer, x, y, tmp);
}

void rgb_to_rgb565u( int buffer, int x, int y, int r, int g, int b )
{
  FxU32 tmp;

  r >>= 3;
  g >>= 2;
  b >>= 3;
  tmp = (r<<11) | (g<<5) | (b<<0);

  SET_PIXEL(buffer, x, y, tmp);
}

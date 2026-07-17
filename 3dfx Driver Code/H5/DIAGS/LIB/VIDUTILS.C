/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** Last Edited: Mon Nov 17 10:54:28 1997 by psmith (Phil Smith (x2456)) on vlsi2
**
** $Revision: 2$
** $Date: 10/11/00 8:12:14 PM$
**
*/
 /* This file contains routines to generate an H3 display and store it as a PPM file

 modified 5/17/97 to operate like the hardware, since it makes it much easier to 
 get all functions to match. So now, we'll completely process one pixel at a time
*/

/* I still need to add these features, and test all of them:
  In Bilerp mode, YUYV extraction needs to do an interpolate.
	VGA mode on. Currently assume video processor's always on.
	Overlay Stereo Enable
	

    Undithering still showing minor errors compared to Gary's program. Need to debug this.

       YUV411 needs to be added.
*/

#include "udiag.h"
#include "sstdiag.h"
#include "fbi.h"
#include "hsimio.h"
#include "h3regs.h"
#include "h3defs.h"
#include "lfbutils.h"
#include "vidutils.h"
#include <h3.h>
#include <stdlib.h>
#include <string.h>
#include <fximg.h>
#ifndef HAL_HSIM
   void VGA_SET_MODE(int a, int b){}
#endif
/*
   void PCI_IOW8(int addr, int data){}
   void PCI_IOW16(int addr, int data){}
   int PCI_IOR8(int addr){ return(0);}

   void SET_IO16(int addr, int data)
   {
	   PCI_IOW16(addr,data);
   }

   void SET_IO8(int addr, int data)
   {
	   PCI_IOW8(addr,data);
   }

   int  GET_IO8(int addr)
   {
	   return(PCI_IOR8(addr));
   }
*/
//----------------------------------------------------------------------
// force pixel x,y in the framebuffer to color 'col' bypassing simulation
//----------------------------------------------------------------------
// This routine should be in another file, but...
int VID_DEBUG = 100;
void SET_PIXEL(FxI32 buffer, int x, int y, FxU32 col)
{
		 
	 // set the SW sim framebuffer
	 CSIM_PIXEL_WR(buffer,x,y,col);
	 if (diago.halInfo->hsim)      // and the HW sim framebuffer
	   HSIM_PIXEL_WR(buffer,x,y,col);
	 if (diago.halInfo->hw)
	   HW_PIXEL_WR(buffer,x,y,col);
}
																		  
																		  


   // end of temporary section
void vidCleanup(SstRegs *sst)
{
	int ii;
	SstCRegs *sstc = (SstCRegs *)(SST_CMDAGP_ADDRESS(sst));

	/***********************************
	printf ("In vidCleanup\n");
	sst_vsync_active ( sst ) ;          // Still need to refine how many Vsync

	printf ("videoMaxFrame = %x\n", diago.videoMaxFrame);
	for (ii=0;ii<diago.videoMaxFrame;ii++) {
	    	sst_vsync_inactive ( sst ) ;        // we will need to wait for
   		sst_vsync_active ( sst ) ;
	**************************************/

	ulong crc_reg;

// swan -- after invert the vsync (now active low)
	printf ("In vidCleanup\n");
	sst_vsync_inactive ( sst ) ;          // Still need to refine how many Vsync

	printf ("videoMaxFrame = %x\n", diago.videoMaxFrame);
	for (ii=0;ii<diago.videoMaxFrame;ii++) {

	  sst_vsync_active ( sst ) ;        // we will need to wait for

	  /* add CRC register read */
	  crc_reg = GET(sstc->crc2);
	  printf("First CRC read is %x", crc_reg);

	  sst_vsync_inactive ( sst ) ;

	  /* add CRC register read */
	  crc_reg = GET(sstc->crc2);
	  printf("Second CRC read is %x", crc_reg);

	  sst_vsync_active ( sst ) ;        // we will need to wait for

	  /* add CRC register read */
	  crc_reg = GET(sstc->crc2);
	  printf("Third CRC read is %x", crc_reg);

#ifdef H4

	  sst_vsync_inactive ( sst ) ;

	  /* add CRC register read */
	  crc_reg = GET(sstc->crc2);
	  printf("Forth CRC read is %x", crc_reg);

#endif
	}

}

/* Routine that does general initialization of the h3 for video testing
*/
void vidInit(FxU32 width, FxU32 height, SstRegs *sst)
{
  SstIORegs * IORegs;
  int result;
  FxU32 miscInit0;
  FxU32 miscInit1;
  IORegs = (SstIORegs *)SST_IO_ADDRESS(sst);

  printf ("vidInit (Height) = %x\n", height);
    // setup FIFO fill register
    SET(IORegs->vidPixelBufThold, 32 | (32 <<6) | (32 <<12));

    // swan--
	 // TV out interface enabled for replay and tester vector comparison
	 // vid in format reg: chrontel; digital tv; no genlock
    SET(IORegs->vidInFormat, 0x00008000);
	 // serial parallel port : set gp_io[1] =1 so that vmi device not selected;
    SET(IORegs->vidSerialParallelPort, 0x20000002);
	 // bit13:11 -- set delay2 to 5; delay for tv sync out (this is used by the tv model)
	 // bit10:8 -- set delay1 to 4; but delay for tv blank out is not used by the tv model
	 miscInit0 = GET(IORegs->miscInit0);
	 miscInit0 &= 0xffff80ff; // clear the delay bits
	 miscInit0 |= (BIT(13) | BIT(11) | BIT(10)); 
	 SET(IORegs->miscInit0,miscInit0);
    // invert the tv clk out
	 miscInit1 = GET(IORegs->miscInit1);
	 miscInit1 |= BIT(29);
    SET(IORegs->miscInit1,miscInit1);

    if ( ! diago.halInfo->hw )     // only init video timing for csim/hsim
      result =vidTimingInit(width, height,sst); 

}
// routine to initialize the video timing
int vidTimingInit(FxU32 width, FxU32 height, SstRegs *sst)
{
  // This is a consolidation of Rich Goodin's res2crtc code and Kirk's 
	int hact=width/8;
	int hfront= hact/16;
	int hsync = hact/8;
	int hback = hact/16;
  int vact = height;
  int vfront= height/32;
  int vsync = height/32;
  int vback = (height*3)/64;
  int htot, hdispena, hblankst, hblankw, hblankend, hsyncst, hsyncend;
  int vtot, vdispena, vblankst, vblankw, vblankend, vsyncst, vsyncend;
  int extended;
  int iobaseaddr;
  int vinitial, tmp;

  if(hfront < 3) hfront =3;
  if(hsync <2) hsync = 2;
  // ek speed up for initial sims. need to change back for 2x if(hback < 32) hback =32;
  if(hback < 32) hback =32;
  if(vfront <1) vfront =1;
  if(vsync < 1) vsync =1;
  if(vback < 1) vback =1;

  htot = hact+hfront+hback+hsync - 5;
  hdispena = hact-1;
  hblankst = hact-1;
  hblankw = hfront+hback+hsync;
  hblankend = hblankst+hblankw;
  hsyncst = hact+hfront-1;
  hsyncend = hsyncst+hsync;
 
  vtot = vact+vfront+vback+vsync - 2;
  vdispena = vact-1;
  vblankst = vact-1;
  vblankw = vfront+vback+vsync;
  vblankend = vblankst+vblankw;
  vsyncst = vact+vfront-1;
  vsyncend = vsyncst+vsync;

  if((htot>255)||(vtot>1023)) {
    if((htot>511)||(vtot>2048)) {
      printf("ERROR - resolution exceeds extended capability\n");
      exit(-1);
    }
    printf("WARNING - resolution exceeds standard");
    printf(" capability - using extended\n");
    extended = 1;
 } else {
    extended = 1;// always one now- because of numberous fixes
  }
 
  if(hblankw > 64) {
    hblankw = 32;
    //printf("ERROR - horizontal blank width exceeds 6 bits\n");
    //exit(-1);
  }
 
  if(hsync > 32) {
    hsync =16;
    //printf("ERROR - horizontal sync width exceeds 5 bits\n");
    //exit(-1);
  }
 
  if(vblankw > 256) {
    vblankw = 128;
    //printf("ERROR - vertical blank width exceeds 8 bits\n");
    //exit(-1);
  }
 
  if(vsync > 16) {
    vsync =8;
    //printf("ERROR - vertical sync width exceeds 4 bits\n");
   //exit(-1);
  }

 
  // major mystery registers.
  SET_IO8( 0x46e8, 0x016 );   
  SET_IO8( 0x102, 0x1 );   
  SET_IO8( 0x46e8, 0x0e );   
  SET_IO8( 0x3c2, 0xe3 );   
  SET_IO16( 0x3D4, 0x0C11 );   // Unlock timing

     // Horizontal total
   SET_IO8( 0x3D4, 0 );
   SET_IO8( 0x3D5, (FxU8)(0xff &htot) );
  
   // Horizontal display enable end
   SET_IO8( 0x3D4, 1 );
   SET_IO8( 0x3D5, (FxU8)(0xff & hdispena));
  
   // Start horizontal blanking
   SET_IO8( 0x3D4, 2 );
   SET_IO8( 0x3D5, (FxU8)(0xff & hblankst) );
  
   // End horizontal blanking
   SET_IO8( 0x3D4, 3 );
   SET_IO8( 0x3D5, (FxU8)(hblankend&0x1f) );
  
   // Start horizontal sync
   SET_IO8( 0x3D4, 4 );
   SET_IO8( 0x3D5, (FxU8)(0xff & hsyncst));
 
   // End horizontal sync
   SET_IO8( 0x3D4, 5 );
   SET_IO8( 0x3D5, (FxU8)(((hblankend& 0x20)<<2)| hsyncend & 0x1f)); // ek
 
   // Vertical total
   SET_IO8( 0x3D4, 6 );
   SET_IO8( 0x3D5, (FxU8)(vtot&0xff));
 
   // Overflow
   SET_IO8( 0x3D4, 7 );
   SET_IO8( 0x3D5,(FxU8)(((vsyncst&0x200)>>2)|
          ((vdispena&0x200)>>3)|
          ((vtot&0x200)>>4)|
          ((vblankst&0x100)>>5)|
          ((vsyncst&0x100)>>6)|
          ((vdispena&0x100)>>7)|
          ((vtot&0x100)>>8)));
 
   // Start vertical blank
   SET_IO8( 0x3D4, 9 );
   SET_IO8( 0x3D5, (FxU8)((vblankst&0x200)>>4));
 
   // Start vertical sync
   SET_IO8( 0x3D4, 0x10 );
   SET_IO8( 0x3D5, (FxU8)(vsyncst&0xff) );
 
   // End vertical sync
   SET_IO8( 0x3D4, 0x11 );
   SET_IO8( 0x3D5, (FxU8)(vsyncend&0xf)  );
 
   // Vertical display enable end
   SET_IO8( 0x3D4, 0x12 );
   SET_IO8( 0x3D5, (FxU8)(vdispena&0xff) );
 
   // End vertical blanking
   SET_IO8( 0x3D4, 0x16 );
   SET_IO8( 0x3D5, (FxU8)(vblankend&0xff) );
	 


   
    SET_IO8(0x3d4, 0x1c);
    SET_IO8(0x3d5, 0x18);
    iobaseaddr = GET_IO8(0x3d5) & 0xfe;
    SET_IO8(0x3d4, 0x1c);
    SET_IO8(0x3d5, 0x19);

    iobaseaddr |= (GET_IO8(0x3d5)<<8);
    iobaseaddr = iobaseaddr & 0xffff;
	 printf("io base address = 0x%x\n", iobaseaddr);
  if(extended) {
   // set extended mode register
    SET_IO((FxU16)(iobaseaddr + 0x28),(FxU32)(1<<6)); // extended mode register enable
    // load crtc with initial value
	  // Horizontal Extension
	  SET_IO8(0x3d4,0x1a); 
	  SET_IO8(0x3d5,
            (FxU8)(((htot&0x100)>>8)|
            ((hsyncend& 0x20)<<2 )| // ek- two new extensions
				((hblankend& 0x40)>>1 )| // ek-second new extension
            ((hdispena&0x100)>>6)|
            ((hblankst&0x100)>>4)|
            ((hsyncst&0x100)>>2)));
	  // vertical extension
	  SET_IO8(0x3d4,0x1b);
	  tmp = (((vtot&0x400)>>0xa)|
            ((vdispena&0x400)>>8)|
            ((vblankst&0x400)>>6)|
            ((vsyncst&0x400)>>4));
	  SET_IO8(0x3d5,
            (FxU8)(tmp));
  }
 
 
// **4 Turn off overscan
   // Start vertical blanking
   SET_IO8( 0x3D4, 0x15 );
   SET_IO8( 0x3D5, (FxU8)(vblankst&0xff) );
   SET_IO8( 0x3D4, 0x17 );
   SET_IO8( 0x3D5, 0x80 );


if ( diago.halInfo->hsim ) {
  VGA_SET_MODE(0xfe,1); // cleans up SEQ and attributes, but overwrites reg 3c2
									
  SET_IO8( 0x3c2, 0xe3 );   //swan -- invert the vsync
  /* SET_IO8( 0x3c2, 0xe3 );   do not invert the vsync anymore */

}




 /*
   // Re Set the SEQ regs
   SET_IO8( 0x3C4, 0 );
   SET_IO8( 0x3C5, 3 );
 
   SET_IO8( 0x3C4, 1 );
   SET_IO8( 0x3C5, 1 );
 
   SET_IO8( 0x3C4, 2 );
   SET_IO8( 0x3C5, 3 );
 
   SET_IO8( 0x3C4, 3 );
   SET_IO8( 0x3C5, 0 );
 
   SET_IO8( 0x3C4, 4 );
   SET_IO8( 0x3C5, 2 );
*/
 
// **2 active low for hsync and vsync
   SET_IO8( 0x3CC, 0xC3);
   fflush(stdout);
 
// programm intial vertical count using a backdoor
    // get io base address
/*    SET_IO8(0x3d4, 0x1c);
    SET_IO8(0x3d5, 0x18);
    SET_IO8(0x3d4, 0x1c);
    SET_IO8(0x3d5, 0x19);
	 */

    SET_IO((FxU16)(iobaseaddr + 0x28),(FxU32)(1<<6)); // extended mode register enable
    // load crtc with initial value

	 vinitial = vact -2; //  swan-start two line from vblank;
    //vinitial = vact -1; // start one line from vblank;
    SET_IO8(0x3d4, 0x20);
    SET_IO8(0x3d5, (FxU8)(vinitial & 0xff));
    SET_IO8(0x3d4, 0x21);
    SET_IO8(0x3d5, (FxU8)((vinitial>>8) & 0xff));
/* ek-does this work?*/
    sst_idle_really(sst);
    tmp = GET_IO((FxU16)(iobaseaddr + 0x10)); // misc register-resetting video
    SET_IO((FxU16)(iobaseaddr + 0x10), (FxU32)(tmp& ~(0x80))); // 0
    SET_IO((FxU16)(iobaseaddr + 0x10), (FxU32)(tmp| (0x80))); // 1
    SET_IO((FxU16)(iobaseaddr + 0x10), (FxU32)(tmp& ~(0x80))); // 0
    
    
// **5 Enable VGA Graphics mode timing
   GET_IO8( 0x3DA );
   SET_IO8( 0x3C0, 0x30);
   SET_IO8( 0x3C0, 0x1);
 


 
return(0);

 
}
/* Routine to initialize the CLUT. one lut inverts the data, the other one 
   increases contrast */
int clutInit( SstIORegs *IORegs)
{
  FxU32 clut[512];
  int i,j;
  for(i=0; i<256; i++)
  {
     j = 255-i;
     clut[i] = i | ((128-j)& 0xff)<<8 | j<<16;
  }
  for(i=256; i<512; i++)
  {
     j= (i-256)*2 -128;
     if (j<0) j=0;
     if (j>255) j=255;
     clut[i] = j | j<<8 | j<<16;
  }
  i=loadClut(clut, IORegs);
  return(i);
}
int  loadClut(FxU32 *dataIn , SstIORegs *IORegs)
{
   int i,tmp;
#if 0
   GDBG_INFO(0,"loadClut: using memory-mapped access\n");
   for(i=0; i<512; i++)
   {
      SET(IORegs->dacAddr,i);
      tmp = GET(IORegs->dacAddr);

#ifdef H3_A0
      // HACK invert clut to work around hw bug
      if ( i == 0 ) {
	GDBG_INFO(0,"loadClut: HACK inverting clut\n");
	GDBG_INFO(0,"loadClut: HACK forcing clut[0] = ~0\n");
      }
      SET(IORegs->dacData,i==0 ? ~0 : ~dataIn[i]);
#else
      SET(IORegs->dacData,dataIn[i]);
#endif

   }
#else
#include "h3asm.h"
   FxU16 portBase;

   GDBG_INFO(0,"loadClut: using port i/o access\n");
   // get port i/o base address -- HACK PS
   portBase = diago.halInfo->boardInfo[SST_FAKE_ADDRESS_GET_BOARD(IORegs)].physPort & 0xFFFF;
   GDBG_INFO(0,"loadClut: io base address = 0x%x\n", portBase);

   for(i=0; i<512; i++) {
    SET_IO((FxU16)(portBase+DACADDR),i);
    tmp = GET_IO((FxU16)(portBase+DACADDR));	// prevent bursts

#ifdef H3_A0
    // HACK invert clut to work around hw bug
    if ( i == 0 ) {
      GDBG_INFO(0,"loadClut: HACK inverting clut\n");
      GDBG_INFO(0,"loadClut: HACK forcing clut[0] = ~0\n");
    }
    SET_IO((FxU16)(portBase+DACDATA),i==0 ? ~0 : ~dataIn[i]);
#else
    SET_IO((FxU16)(portBase+DACDATA),dataIn[i]);
#endif
   
   }
#endif
   return(0);
   
}
/* video Output pipeline*/ 
/* A very simple routine that displays an image in lfb space
*/
int vidOut(  )
{

    SstIORegs *IORegs = &CSIM_PRIVATE(diago.sstCSIM)->io;
   
	 FxBool result, getOverlayResult;
	int x, y; 
		FxU32 pixnum; 
	FxU32 xoly, yoly;
	FxU32 r, g, b, lutaddr;
	FxU32 ChromaRMin, ChromaGMin, ChromaBMin;
	FxU32 ChromaRMax, ChromaGMax, ChromaBMax;
	FxU32 value, olyvalue;
	FxU32 width, height,pixfmt;
	FxU32 *Screenstore;
	FxBool chromaMatch;
   FxBool alpha_bit;
	FxBool pixType; // 0 = desktop, 1=overlay
	FxU32 cursor, xcursor, ycursor;
	 FxBool inCursor; // inside the cursor area.
	 FxBool cursorOn;
	 FxBool clutBypass;
	 FxBool clutSel;

    
	// Figure out length, width of display (non-overlay)

    
	width = IORegs->vidScreenSize & SST_VIDEO_SCREEN_WIDTH;
	height = (IORegs->vidScreenSize & SST_VIDEO_SCREEN_HEIGHT) >>SST_VIDEO_SCREEN_HEIGHT_SHIFT;

   Screenstore=(FxU32 *)malloc(height*width*sizeof(FxU32));
	
	for(y=0;y<(int)height; y++)
	{
		for(x=0; x<(int)width; x++)
	   {
                  // initially assume desktop
       if(SST_DESKTOP_CLUT_BYPASS & IORegs->vidProcCfg)
            clutBypass = 1;
       else clutBypass =0;
       if(SST_DESKTOP_CLUT_SELECT & IORegs->vidProcCfg)
            clutSel = 1;
       else clutSel =0;
	 
		  pixType=0; // assume desktop
		  chromaMatch = 1; // assume  match
GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch1 = %d\n", __FILE__, __LINE__, chromaMatch);
	     pixnum = y*width+x;
		  if(SST_DESKTOP_EN&IORegs->vidProcCfg ) // Desktop Surface Enable
		  {
	       /* fetch pixels for 2D window */
			 // need to replace this with a video version, I think
          value = CSIM_PIXEL_RD( CSIM_BUF_DESKTOP, x, y );
GDBG_INFO(VID_DEBUG, "%s, %d: desktopvalue1 = %x\n", __FILE__, __LINE__, value);
		    // need to use the pixel format to mask the upper color bits,
			 // before the chroma key compare
          pixfmt = (IORegs->vidProcCfg & SST_DESKTOP_PIXEL_FORMAT) >>SST_DESKTOP_PIXEL_FORMAT_SHIFT ;
		    switch(pixfmt){
			 case(0):    // 8 -bit palletized
						 value &=0xff; 
						 r = value;
						 g = 0; 
						 b = 0;
						 ChromaRMin = IORegs->vidChromaMin & 0xff;
						 ChromaRMax = IORegs->vidChromaMax & 0xff;
						 ChromaGMin = 0;
						 ChromaGMax = 0;
						 ChromaBMin = 0;
						 ChromaBMax = 0;
						 break;
			 case(1):	 //  565 RGB
						 value &=0xffff; 
						 // Red
						 r = (value >> 11) & 0x1f;
						 ChromaRMin = (IORegs->vidChromaMin>>11) & 0x1f;
						 ChromaRMax = (IORegs->vidChromaMax>>11) & 0x1f;
						 g = (value >> 5) & 0x3f;
						 ChromaGMin = (IORegs->vidChromaMin>>5) & 0x3f;
						 ChromaGMax = (IORegs->vidChromaMax>>5) & 0x3f;
						 b = (value >> 0) & 0x1f;
						 ChromaBMin = (IORegs->vidChromaMin>>0) & 0x1f;
						 ChromaBMax = (IORegs->vidChromaMax>>0) & 0x1f;
						 break;
			 case(2):	 //  24-bit RGB
						 value &=0xffffff; 
						 r = (value >> 16) & 0xff;
						 ChromaRMin = (IORegs->vidChromaMin>>16) & 0xff;
						 ChromaRMax = (IORegs->vidChromaMax>>16) & 0xff;
						 g = (value >> 8) & 0xff;
						 ChromaGMin = (IORegs->vidChromaMin>>8) & 0xff;
						 ChromaGMax = (IORegs->vidChromaMax>>8) & 0xff;
						 b = (value >> 0) & 0xff;
						 ChromaBMin = (IORegs->vidChromaMin>>0) & 0xff;
						 ChromaBMax = (IORegs->vidChromaMax>>0) & 0xff;
						 break;
			 case(3):	 //  32-bit RGB
						 value &=0xffffff; 
						 r = (value >> 16) & 0xff;
						 ChromaRMin = (IORegs->vidChromaMin>>16) & 0xff;
						 ChromaRMax = (IORegs->vidChromaMax>>16) & 0xff;
						 g = (value >> 8) & 0xff;
						 ChromaGMin = (IORegs->vidChromaMin>>8) & 0xff;
						 ChromaGMax = (IORegs->vidChromaMax>>8) & 0xff;
						 b = (value >> 0) & 0xff;
						 ChromaBMin = (IORegs->vidChromaMin>>0) & 0xff;
						 ChromaBMax = (IORegs->vidChromaMax>>0) & 0xff;
						 break;
			 case(4):	 //  1555 RGB undithered
						 value &=0xffff; 
						 // Red
						 r = (value >> 10) & 0x1f;
						 ChromaRMin = (IORegs->vidChromaMin>>10) & 0x1f;
						 ChromaRMax = (IORegs->vidChromaMax>>10) & 0x1f;
						 g = (value >> 5) & 0x1f;
						 ChromaGMin = (IORegs->vidChromaMin>>5) & 0x1f;
						 ChromaGMax = (IORegs->vidChromaMax>>5) & 0x1f;
						 b = (value >> 0) & 0x1f;
						 ChromaBMin = (IORegs->vidChromaMin>>0) & 0x1f;
						 ChromaBMax = (IORegs->vidChromaMax>>0) & 0x1f;
						 alpha_bit = (value >> 15) & 0x01;
GDBG_INFO(VID_DEBUG, "%s, %d: desktopvalue2 = %x\n", __FILE__, __LINE__, value);
GDBG_INFO(VID_DEBUG, "%s, %d: alpha_bit2 = %x\n", __FILE__, __LINE__, alpha_bit);
						 break;
		    }



          /* Performan Chroma Key Compare */
  		    if(IORegs->vidProcCfg &SST_CHROMA_EN)// ChromaKeyEnable
		    {
             chromaMatch=1; // assume  match
             GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch2 = %d\n", __FILE__, __LINE__, chromaMatch);

				 /********************
             switch(pixfmt){
             case(4): //  1555 RGB undithered
                if(IORegs->vidProcCfg & SST_USE_ALPHA_BIT) {
                // Before inversion, display overlay if alpha_bit (i.e. bit 15) = 0; 
                // to display overlay, chromaMatch needs to be 1                   
                   chromaMatch = (~alpha_bit) & 0x1;
                   GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch7 = %d\n", __FILE__, __LINE__, chromaMatch);

                }
			       if(IORegs->vidProcCfg & SST_CHROMA_INVERT) {
					    chromaMatch = (~chromaMatch) & 0x1; // ChromaKeyResultInversion
                   GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch8 = %d\n", __FILE__, __LINE__, chromaMatch);

                }
				 **************************/
				 if(IORegs->vidProcCfg & SST_USE_ALPHA_BIT) {
					// Before inversion, display overlay if alpha_bit (i.e. bit 15) = 0; 
					// to display overlay, chromaMatch needs to be 1;                   
					chromaMatch = (~alpha_bit) & 0x1;
GDBG_INFO(VID_DEBUG, "%s, %d: alpha_bit3 = %x\n", __FILE__, __LINE__, alpha_bit);
					GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch7 = %d\n", __FILE__, __LINE__, chromaMatch);
				 } else {
					if((r < ChromaRMin) || (r > ChromaRMax)) {
					  chromaMatch = 0; // chromakey doesn't match
                 GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch3 = %d\n", __FILE__, __LINE__, chromaMatch);
               }
               if((g < ChromaGMin) || (g > ChromaGMax)) {
                 chromaMatch = 0; // chromakey doesn't match
					  GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch4 = %d\n", __FILE__, __LINE__, chromaMatch);
               }
				   if((b < ChromaBMin) || (b > ChromaBMax)) {
				      chromaMatch = 0; // chromakey doesn't match
                  GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch5 = %d\n", __FILE__, __LINE__, chromaMatch);
               }
             }
			    if(IORegs->vidProcCfg & SST_CHROMA_INVERT) {
					chromaMatch = (~chromaMatch) & 0x1; // ChromaKeyResultInversion
               GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch6 = %d\n", __FILE__, __LINE__, chromaMatch);
             }
          }
		    else {
          chromaMatch = 1; // if no chroma, it always matches
          GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch9 = %d\n", __FILE__, __LINE__, chromaMatch);
          }

       }
			 // Perform extract. This converts 16 bit RGB into 24-bit
		    switch(pixfmt){
			 case(1):	 //  565 RGB
		       r = ((value & 0xF800) << 8) >> 16;
		       g = ((value & 0x07E0) << 5) >> 8;
		       b = ((value & 0x001F) << 3);
		       value = (r << 16) | (g << 8) | b;
						 break;
			 case(4):	 //  1555 RGB undithered
		       r = ((value & 0x7c00) << 9) >> 16;
		       g = ((value & 0x03E0) << 6) >> 8;
		       b = ((value & 0x001F) << 3);
		       value = (r << 16) | (g << 8) | b;
						 break;
		    }


		  getOverlayResult = 0; // default to zero 

		  
			 if (width < (int)(IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X)) 
          {
             printf("%s, %d: CSIM ERROR: Overlay end is past screen end: Oly end = %d, screen end = %d\n",
             __FILE__, __LINE__, 
			     (int)(IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X),
              width); 
              exit(1);
          }
		  if( (x >= (int)(IORegs->vidOverlayStartCoords & SST_OVERLAY_X)) 
			  && (y >= (int)((IORegs->vidOverlayStartCoords & SST_OVERLAY_Y) >> SST_OVERLAY_Y_SHIFT )) 
			  && (x <= (int)(IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X)) 
			  && (y <= ((int)(IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_Y)>>SST_OVERLAY_Y_SHIFT) ) 
           && (IORegs->vidProcCfg & SST_OVERLAY_EN) 
			  ) // set to one if chroma not enabled.
        {  
			 
		    /* If overlay is visible, fetch overlay data. This is determined by
		  	  the chroma compare and other control bits */
		  xoly = x  - (IORegs->vidOverlayStartCoords & SST_OVERLAY_X);
			yoly = y - ((IORegs->vidOverlayStartCoords & SST_OVERLAY_Y) 
							 >> SST_OVERLAY_Y_SHIFT ); 
          olyvalue = overlayProcess(  IORegs,  xoly, yoly, x, y);
			  if(chromaMatch) // set to one if chroma not enabled.
			  {
	           pixType=1; // pixel type is overlay
			     value = olyvalue;
                   // set CLUT bits for overlay now.
                  if(SST_OVERLAY_CLUT_BYPASS & IORegs->vidProcCfg)
                      clutBypass = 1;
                  else clutBypass =0;
                  if(SST_OVERLAY_CLUT_SELECT & IORegs->vidProcCfg)
                      clutSel = 1;
                  else clutSel =0;
				} 
GDBG_INFO(VID_DEBUG, "%s, %d: ChromaMatch = %d, value = 0x%x\n",
__FILE__, __LINE__, chromaMatch, value);
		  }

		  /* Color space conversion */
        value = vidOutCSC (value, IORegs, pixType); 
GDBG_INFO(VID_DEBUG, "%s, %d: x,y = %x, %x: colorspace conversion result: %x\n",
__FILE__,__LINE__, x,y, value );

		  /**** ISSUE: VGA bypass comes in here. How do we support this?
				 do we support this? 
        ****/
		  /* Hardware Cursor */
		  // first calculate if we're in the cursor area
		  if(IORegs->vidProcCfg & SST_CURSOR_EN) inCursor=1;
		  else inCursor=0;

        // ek change to use last pixel
		  if(x > (int)(IORegs->hwCurLoc&SST_CURSOR_X)>>SST_CURSOR_X_SHIFT) inCursor = 0; // to the right  
		  if(x<=((int)((int)((IORegs->hwCurLoc&SST_CURSOR_X)>>SST_CURSOR_X_SHIFT)-0x40))) inCursor = 0; // to the right  
		  if(y >(int)((IORegs->hwCurLoc & SST_CURSOR_Y)>> SST_CURSOR_Y_SHIFT)) inCursor = 0; // to the right  
		  if(y <=((int)((int)(IORegs->hwCurLoc & SST_CURSOR_Y)>> SST_CURSOR_Y_SHIFT) - 0x40)) inCursor = 0; // to the right  

        cursorOn=0;
		  if(inCursor)
		    {

                   xcursor = x- (((IORegs->hwCurLoc&SST_CURSOR_X)>> SST_CURSOR_X_SHIFT) - 63);
		   ycursor= y -(((IORegs->hwCurLoc& SST_CURSOR_Y)>> SST_CURSOR_Y_SHIFT) - 63);
          cursor = (CSIM_PIXEL_RD( CSIM_BUF_CURSOR, xcursor, ycursor )&0x3);
			 // swap bits of cursor
			 cursor = ((cursor & 2) >>1) | ((cursor & 1) <<1);
			 if(IORegs->vidProcCfg & SST_CURSOR_X11){
				  // X11 mode
				  if(cursor==2) {
					  cursorOn=1;
					  value = IORegs->hwCurC0;
				  } 
				  else if(cursor==3) {
					  cursorOn=1;
					  value = IORegs->hwCurC1;
				  } 
			 } else
			 {
				  // ms Windows mode
				  if(cursor==0) {
					  cursorOn=1;
					  value = IORegs->hwCurC0;
				  } 
				  else if(cursor==1) {
					  cursorOn=1;
					  value = IORegs->hwCurC1;
				  } else if(cursor==3) { // xor mode
					  value = (~value) & 0xffffff; // cursor not on
				  }
		    }
GDBG_INFO(VID_DEBUG, "%s, %d: x,y = %x, %x: In cursor space. cursor = %x, color: %x, x,y cursor = %x, %x\n",
__FILE__,__LINE__, x,y,cursor, value, xcursor, ycursor );
			 }
		  /* Color Lookup Table, currently a NOP*/
			 // clut code goes here...
          if((clutBypass==0) & (cursorOn ==0))
          {
              r = (value >>16) & 0xff;
              g = (value >>8) & 0xff;
              b = (value >>0) & 0xff;
 
              
              lutaddr=0;
              if(clutSel==1)
                lutaddr=0x100;
			  else lutaddr=0;
              lutaddr |= r;
              r= (CSIM_PRIVATE(diago.sstCSIM)->clut512.rgb[lutaddr] >>16) & 0xff;
              if(clutSel==1)
                lutaddr=0x100;
			  else lutaddr=0;
              lutaddr |= g;
              g= (CSIM_PRIVATE(diago.sstCSIM)->clut512.rgb[lutaddr] >>8) & 0xff;
              if(clutSel==1)
                lutaddr=0x100;
			  else lutaddr=0;
              lutaddr |= b;
              b= (CSIM_PRIVATE(diago.sstCSIM)->clut512.rgb[lutaddr] >>0) & 0xff;
              value = r<<16 | g <<8 | b;
          }
//
//			 // replace gamma corrected code with cursor if cursor is on
//			 if(cursorOn)
//				 value=cursor;
        
GDBG_INFO(VID_DEBUG, "%s, %d: x,y = %x, %x: final result: %x\n",
__FILE__,__LINE__, x,y, value );
	
		  Screenstore[pixnum]=value;
	    }
	}

	// Write to a PPM file.
	{
	  int i;
	  char imgname[128], buf[128];
	  strcpy(buf,diago.pgm_name);
	  for ( i=strlen(diago.pgm_name)-1; i>=0; i-- ) {
	    if ( buf[i] == '.' ) {
	      buf[i] = '\0';
	      break;
	    }
	  }
	  sprintf(imgname,"%s_%d",buf,diago.option);
    result = screenDump(Screenstore,  imgname, // what to do for filename?
			 width, height, IMG_P6);
	}	  
	free(Screenstore);
	 return(result);
}

// even though these are uints, they need to be subtracted
static int undither4x4[4][4] = {
             {4 ,0 , 3, -1},
				 {-2,2 , -3, 1},
				 {2, -2, 3, -1},
				 {-4, 0,-3, 1}
};
// undither 2x2 is just the upper right corner of the 4x4, according
// to gary.
static int undither2x2[4][4] = {
             {4, 0, 4, 0},
				 {-2, 2, -2, 2},
             {4, 0, 4, 0},
				 {-2, 2, -2, 2}
};

static FxU32 xBilinInterp, yBilinInterp;
		  
/* Overlay Pixel processing pipeline*/
FxU32 overlayProcess(  SstIORegs *IORegs, FxU32 x, FxU32 y, FxU32 xdesk, FxU32 ydesk)
{
FxU32 value00 ,value01, value0m2, value0m1; 
FxU32 value10 ,value11;
FxU32 value;
int r, g, b;
FxU32  filtMode, pixfmt;
static FxU32 ufrac, ufrac2, vfrac;
          
           /* overlay data processing: Extracting */
// need to talk to Ken, gary about getting Overlay buffer addr.
				  // extraction incudes converting YUYV and UYUV formats into
				  //YUV, eight bits each.
				  filtMode = IORegs->vidProcCfg & SST_OVERLAY_FILTER_MODE;
				  pixfmt = IORegs->vidProcCfg & SST_OVERLAY_PIXEL_FORMAT;
		     if((filtMode == SST_OVERLAY_FILTER_BILINEAR) 
                       | (filtMode ==SST_OVERLAY_FILTER_POINT)
				           | (filtMode ==SST_OVERLAY_FILTER_4X4 )) { 
				 // first, set the Interp Coordinates
		       if(x == 0) 
				 { 
			      // Set X interp to start 
					xBilinInterp = x<<20;
GDBG_INFO(VID_DEBUG,"%s, %d: xbilin1 = %x\n", __FILE__,__LINE__,xBilinInterp);
               if(IORegs->vidProcCfg & SST_OVERLAY_HORIZ_SCALE_EN) {
					  xBilinInterp += 2*(IORegs->vidOverlayDudxOffsetSrcWidth& 0x7ffff);
 
GDBG_INFO(VID_DEBUG,"%s, %d: xbilin2 = %x\n", __FILE__,__LINE__,xBilinInterp);
					}
					else {
					  xBilinInterp =0;
GDBG_INFO(VID_DEBUG,"%s, %d: xbilin3 = %x\n", __FILE__,__LINE__,xBilinInterp);
					}
			    } else {
				   if(IORegs->vidProcCfg & SST_OVERLAY_HORIZ_SCALE_EN)
 {
                 xBilinInterp += IORegs->vidOverlayDudx& 0xfffff;
GDBG_INFO(VID_DEBUG,"%s, %d: xbilin4 = %x\n", __FILE__,__LINE__,xBilinInterp);
					}
				//	if((IORegs->vidOverlayDudx & 0xfffff)==0)
				  else 
 {
                  xBilinInterp += 0x100000; // step by one
GDBG_INFO(VID_DEBUG,"%s, %d: xbilin5 = %x\n", __FILE__,__LINE__,xBilinInterp);
				  }
				 }
			    if (y == 0 ) 
			    {
			      // Set Y interp to start 
					yBilinInterp = y<<20;
               yBilinInterp += 2*(IORegs->vidOverlayDvdyOffset & 0x7ffff);
			    } else if (x==0) {
               yBilinInterp += IORegs->vidOverlayDvdy& 0xfffff;
				 }
				 }
				 // If HSCALE is on, use the int part of blininterp for x.
				 // If HSCALE is off, don't bilerp.
				 if(IORegs->vidProcCfg & SST_OVERLAY_HORIZ_SCALE_EN)
				 {
					x = xBilinInterp >> 20; 
				 } else
				 {
					// xBilinInterp=0; // not performing a bilerp
				 }

				 // If VSCALE is on, use the int part of blininterp for y.
				 // If VSCALE is off, don't bilerp.
				 if(IORegs->vidProcCfg & SST_OVERLAY_VERT_SCALE_EN)
				 {
					y = yBilinInterp >> 20; 
				 } else
				 {
					yBilinInterp=0; // not performing a bilerp
				 }
			//
			// Color expansion is done for undithered pixels in olyExtractUndither
			// Color expansion is half-done for dithered pixels in olyExtractUndither
			// need to generated the lower bits later inside overlayProcess
         //
         value00 = olyExtractUndither(x,  y,    IORegs,0);
         value01 = olyExtractUndither(x+1,y,    IORegs,1);
         value0m2 = olyExtractUndither(x-2,y,    IORegs,0);
         value0m1 = olyExtractUndither(x-1,y,    IORegs,1);
         //value0m2 = olyExtractUndither(x-2,y,    IORegs);
         //valuem10 = olyExtractUndither(x,  y-1,  IORegs);
         //valuem1m1 = olyExtractUndither(x-1,y-1,  IORegs);
         value10 = olyExtractUndither(x,  y+1,  IORegs,0);
         value11 = olyExtractUndither(x+1,y+1,  IORegs,1);
GDBG_INFO(VID_DEBUG,"%s, %d: xoly =%x, yoly = %x, x,y bilin = %x %x\n", __FILE__,__LINE__,x,y, xBilinInterp, yBilinInterp);
GDBG_INFO(VID_DEBUG,"%s, %d: first line, undithered pixels -1 to 3  %x, %x, %x, %x\n", 
__FILE__,__LINE__,value0m2, value0m1, value00, value01  );
GDBG_INFO(VID_DEBUG,"%s, %d: second line, undithered pixels 0 to 1         ,        , %x, %x\n", 
__FILE__,__LINE__, value10, value11 );
           /* overlay data processing: Bilinear Filtering */

		     if(filtMode == SST_OVERLAY_FILTER_BILINEAR) { 
				if(IORegs->vidProcCfg & SST_VIDEO_2X_MODE_EN)
            {
              printf("%s, %d: 2X video mode doesn't support this feature\n",
                     __FILE__, __LINE__);
              exit(-1);
            }

				 // now use a 6 -bit multiplier to perform the bilerps
		       r = 4*((value00 & 0xFF0000) >> 16);
				 ufrac = 0xff & ((xBilinInterp )>>(20-8));
				  if((pixfmt == SST_OVERLAY_PIXEL_RGB565D)
				    | (pixfmt == SST_OVERLAY_PIXEL_RGB565U)
					 | (pixfmt == SST_OVERLAY_PIXEL_RGB1555U)
				    | (pixfmt == SST_OVERLAY_PIXEL_RGB1555D)
				    | (pixfmt == SST_OVERLAY_PIXEL_RGB32U))
				      ufrac2 = ufrac; // rgb types
				  else if(pixfmt==SST_OVERLAY_PIXEL_YUV411)
				     ufrac2 = 0xff & ((xBilinInterp )>>(20-8+2));// Yuv Types
				  else
				     ufrac2 = 0xff & ((xBilinInterp )>>(20-8+1));// Yuv Types
				 vfrac = (yBilinInterp & 0xfffff)>>(20-8);
				 r=olyBilerp(ufrac, vfrac, value00, value01, value10, value11, 16);
				 g=olyBilerp(ufrac2, vfrac, value00, value01, value10, value11, 8);
				 b=olyBilerp(ufrac2, vfrac, value00, value01, value10, value11, 0);
			  }
           /* overlay data processing: Box Filtering or 4-tap filtering */
			  // extract all components from four pixels, and average like components
		     else if(filtMode == SST_OVERLAY_FILTER_2X2) {
				if(IORegs->vidProcCfg & SST_VIDEO_2X_MODE_EN)
            {
              printf("%s, %d: 2X video mode doesn't support this feature\n",
                     __FILE__, __LINE__);
              exit(-1);
            }
		       r = 4*((value00 & 0xFF0000) >> 16);
		       g = 4*((value00 & 0xFF00) >> 8);
		       b = 4*((value00 & 0xFF) >> 0);

				 // now calculate differences
				 r += redDiff(IORegs, value01, value00, xdesk+1, ydesk);
				 g += greenDiff(IORegs, value01,  value00,xdesk+1, ydesk);
				 b += blueDiff(IORegs, value01,  value00,xdesk+1, ydesk);


				 r += redDiff(IORegs, value10, value00, xdesk, ydesk+1);
				 g += greenDiff(IORegs, value10,  value00,xdesk, ydesk+1);
				 b += blueDiff(IORegs, value10,  value00,xdesk, ydesk+1);


				 r += redDiff(IORegs, value11, value00, xdesk+1, ydesk+1);
				 g += greenDiff(IORegs, value11,  value00,xdesk+1, ydesk+1);
				 b += blueDiff(IORegs, value11,  value00,xdesk+1, ydesk+1);
       
           
				 if(r<0)r=0;
				 if(g<0)g=0;
				 if(b<0)b=0;

				 r = r/4;
				 g = g/4;
				 b = b/4;
				 
				 // Color expansion for dithered pixels, here assume if(filtMode == SST_OVERLAY_FILTER_2X2)
				 // must be dithered pixel format
				 // 565 already expanded to 888, but the bottom bits need to be gened.
	
			 
		       r +=  (r >> 5); // msbs for lsbs.
				 if(pixfmt == SST_OVERLAY_PIXEL_RGB1555D)
					g +=  (g >> 5);
				 else
					g +=  (g >> 6);
  		       b +=  (b >> 5);
				 if(r>255) r=255;
				 if(g>255) g=255;
				 if(b>255) b=255;
			  }
			  else if (filtMode==SST_OVERLAY_FILTER_4X4) {
		       r = 4*((value00 & 0xFF0000) >> 16);
		       g = 4*((value00 & 0xFF00) >> 8);
		       b = 4*((value00 & 0xFF) >> 0);

GDBG_INFO(VID_DEBUG,"debug1: %s, %d: r =%x, g = %x, b = %x after x4\n", __FILE__,__LINE__,r,g,b);

				 // now calculate differences
				 r += redDiff(IORegs, value01, value00, xdesk+1, ydesk);
				 g += greenDiff(IORegs, value01,  value00,xdesk+1, ydesk);
				 b += blueDiff(IORegs, value01,  value00,xdesk+1, ydesk);

GDBG_INFO(VID_DEBUG,"debug2: %s, %d: r =%x, g = %x, b = %x +delta1\n", __FILE__,__LINE__,r,g,b);

				 r += redDiff(IORegs, value0m2, value00, xdesk-2, ydesk);
				 g += greenDiff(IORegs, value0m2,  value00,xdesk-2, ydesk);
				 b += blueDiff(IORegs, value0m2,  value00,xdesk-2, ydesk);

GDBG_INFO(VID_DEBUG,"debug3: %s, %d: r =%x, g = %x, b = %x +delta2\n", __FILE__,__LINE__,r,g,b);

				 r += redDiff(IORegs, value0m1, value00, xdesk-1, ydesk);
				 g += greenDiff(IORegs, value0m1,  value00,xdesk-1,ydesk);
				 b += blueDiff(IORegs, value0m1,  value00,xdesk-1, ydesk);

GDBG_INFO(VID_DEBUG,"debug4: %s, %d: r =%x, g = %x, b = %x +delta3\n", __FILE__,__LINE__,r,g,b);

              if(r<0)r=0;
			     if(g<0)g=0;
			     if(b<0)b=0;

GDBG_INFO(VID_DEBUG,"debug5: %s, %d: r =%x, g = %x, b = %x after clamp\n", __FILE__,__LINE__,r,g,b);


				 r = r/4;
				 g = g/4;
				 b = b/4;

GDBG_INFO(VID_DEBUG,"debug6: %s, %d: r =%x, g = %x, b = %x after divide by 4\n", __FILE__,__LINE__,r,g,b);

				 // Color expansion for dithered pixels, here assume if (filtMode==SST_OVERLAY_FILTER_4X4)
				 // must be dithered pixel format
				 // 565 already expanded to 888, but the bottom bits need to be gened.
	

		       r +=  (r >> 5); // msbs for lsbs.
				 if(pixfmt == SST_OVERLAY_PIXEL_RGB1555D)
					g +=  (g >> 5);
				 else
					g +=  (g >> 6);
  		       b +=  (b >> 5);

GDBG_INFO(VID_DEBUG,"debug7: %s, %d: r =%x, g = %x, b = %x after color expansion\n", __FILE__,__LINE__,r,g,b);

				 if(r>255) r=255;
				 if(g>255) g=255;
				 if(b>255) b=255;

GDBG_INFO(VID_DEBUG,"debug8: %s, %d: r =%x, g = %x, b = %x\n", __FILE__,__LINE__,r,g,b);

			  } else { // point sampling
				      r = (value00 & 0xFF0000) >> 16;
		              g = (value00 & 0xFF00) >> 8;
		              b = (value00 & 0xFF) >> 0;

GDBG_INFO(VID_DEBUG,"debug1a: %s, %d: r =%x, g = %x, b = %x\n", __FILE__,__LINE__,r,g,b);

				  if(pixfmt == SST_OVERLAY_PIXEL_RGB565D) // undithered pixels do not need to be color expanded again
				  {
				
		              r +=  (r >> 5); // msbs for lsbs.
		              g +=  (g >> 6);
  		              b +=  (b >> 5);
				  }

GDBG_INFO(VID_DEBUG,"debug2a: %s, %d: r =%x, g = %x, b = %x\n", __FILE__,__LINE__,r,g,b);

				  if(pixfmt == SST_OVERLAY_PIXEL_RGB1555D) // undithered pixels do not need to be color expanded again
				  {
				
		              r +=  (r >> 5); // msbs for lsbs.
		              g +=  (g >> 5);
  		              b +=  (b >> 5);
				  }

GDBG_INFO(VID_DEBUG,"debug3a: %s, %d: r =%x, g = %x, b = %x\n", __FILE__,__LINE__,r,g,b);

				 if(r>255) r=255;
				 if(g>255) g=255;
				 if(b>255) b=255;

GDBG_INFO(VID_DEBUG,"debug4a: %s, %d: r =%x, g = %x, b = %x\n", __FILE__,__LINE__,r,g,b);

			 }
GDBG_INFO(VID_DEBUG,"%s, %d: filtered result: r=%x, g=%x, b=%x\n",
__FILE__,__LINE__, r, g, b );
			  
		       value = (r << 16) | (g << 8) | b;


			  // munge value if in solaris mode
			  return(value);
}
// Calculate the Red, Green and Blue differences from

FxU32 redDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y)
{
int dr;
FxU8 center, pixel;
int  maxRed;
		       center = (FxU8)((value00 & 0xFF0000) >> 16);
		       pixel = (FxU8)((value & 0xFF0000) >> 16);
				 // check for value too different. If so, then don't use it.
				 dr = (int) (pixel-center);
				 maxRed = (IORegs->vidMaxRGBDelta & 0x3f0000)
                      >>16;
GDBG_INFO(VID_DEBUG,"%s, %d: delta result1: pixel=%x, center=%x, dr=%x, maxRed=%x\n", __FILE__,__LINE__, pixel, center, dr, maxRed);
				 if(maxRed & 0x80) maxRed |= 0xffffff00; // sign extend
GDBG_INFO(VID_DEBUG,"%s, %d: delta result2: pixel=%x, center=%x, dr=%x, maxRed=%x\n", __FILE__,__LINE__, pixel, center, dr, maxRed);
				 if (dr > maxRed || dr < -maxRed) dr=0;
GDBG_INFO(VID_DEBUG,"%s, %d: delta result3: pixel=%x, center=%x, dr=%x, maxRed=%x\n", __FILE__,__LINE__, pixel, center, dr, maxRed);
				 // check for pixel out of range. if so, then don't use it.
   		    if( (x >= (IORegs->vidOverlayStartCoords & SST_OVERLAY_X)) 
			    && (y >= ((IORegs->vidOverlayStartCoords & SST_OVERLAY_Y) >> SST_OVERLAY_Y_SHIFT)) 
			    && (x <= (IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X)) 
			    && (y <= ((IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_Y) >>SST_OVERLAY_Y_SHIFT))) 
				 {
					return(dr);
				 }
				 else return (0);

}



FxU32 greenDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y)
{
int dg;
FxU32 center, pixel;
int maxGreen;
		       center = (value00 & 0xFF00) >> 8;
		       pixel = (value & 0xFF00) >> 8;
				 // check for value too different. If so, then don't use it.
				 dg = (int)(pixel-center);
				 maxGreen = (IORegs->vidMaxRGBDelta & 0x003f00)
                      >>8;
				  if(maxGreen & 0x80) maxGreen |= 0xffffff00; // sign extend
				 if (dg > maxGreen || dg < -maxGreen) dg=0;
				 // check for pixel out of range. if so, then don't use it.
   		    if( (x >= (IORegs->vidOverlayStartCoords & SST_OVERLAY_X)) 
			    && (y >= ((IORegs->vidOverlayStartCoords  & SST_OVERLAY_Y)>> SST_OVERLAY_Y_SHIFT )) 
			    && (x <= (IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X)) 
			    && (y <= ((IORegs->vidOverlayEndScreenCoord  & SST_OVERLAY_Y)>>SST_OVERLAY_Y_SHIFT)) )
				 {
					return(dg);
				 }
				 else return (0);

}

FxU32 blueDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y)
{
int db;
FxU32 center, pixel;
int maxBlue;
		       center = (value00 & 0xFF) >> 0;
		       pixel = (value & 0xFF) >> 0;
				 // check for value too different. If so, then don't use it.
				 db = (int)(pixel-center);
		       maxBlue = (IORegs->vidMaxRGBDelta & 0x00003f)
                      >>0;
			    if(maxBlue & 0x80) maxBlue |= 0xffffff00; // sign extend
				 if (db > maxBlue || db < -maxBlue) db=0;
				 // check for pixel out of range. if so, then don't use it.
   		    if( (x >= (IORegs->vidOverlayStartCoords & SST_OVERLAY_X)) 
			    && (y >= ((IORegs->vidOverlayStartCoords & SST_OVERLAY_Y)>> SST_OVERLAY_Y_SHIFT) 
			    && (x <= (IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X)) 
			    && (y <= (IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_Y) >>SST_OVERLAY_Y_SHIFT)) )
				 {
					return(db);
				 }
				 else return (0);

}


/*******************************
FxU32 redDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y)
{
int dr;
FxU8 center, pixel;
int  maxRed;
		       center = (FxU8)((value00 & 0xFF0000) >> 16);
		       pixel = (FxU8)((value & 0xFF0000) >> 16);
				 // check for value too different. If so, then don't use it.

             maxRed = (IORegs->vidMaxRGBDelta & 0x3f0000) >>16;
				 // unnecessary: if(maxRed & 0x80) maxRed |= 0xffffff00; // sign extend

             if(pixel >= center) {
                dr = (int) (pixel-center);
GDBG_INFO(VID_DEBUG,"%s, %d: delta result1: pixel=%x, center=%x, dr=%x, maxRed=%x\n", __FILE__,__LINE__, pixel, center, dr, maxRed);
				    if (dr > maxRed) dr=0; // check for pixel out of range. if so, then don't use it.
GDBG_INFO(VID_DEBUG,"%s, %d: delta result2: pixel=%x, center=%x, dr=%x, maxRed=%x\n", __FILE__,__LINE__, pixel, center, dr, maxRed);
				 } else {
                dr = (int) (center-pixel);
GDBG_INFO(VID_DEBUG,"%s, %d: delta result3: pixel=%x, center=%x, dr=%x, maxRed=%x\n", __FILE__,__LINE__, pixel, center, dr, maxRed);
				    if (dr > maxRed) dr=0;
GDBG_INFO(VID_DEBUG,"%s, %d: delta result4: pixel=%x, center=%x, dr=%x, maxRed=%x\n", __FILE__,__LINE__, pixel, center, dr, maxRed);
				 }

   		    if( (x >= (IORegs->vidOverlayStartCoords & SST_OVERLAY_X)) 
			    && (y >= ((IORegs->vidOverlayStartCoords & SST_OVERLAY_Y) >> SST_OVERLAY_Y_SHIFT)) 
			    && (x <= (IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X)) 
			    && (y <= ((IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_Y) >>SST_OVERLAY_Y_SHIFT))) 
				 {
					return(dr);
				 }
				 else return (0);

}

FxU32 greenDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y)
{
int dg;
FxU32 center, pixel;
int maxGreen;
		       center = (value00 & 0xFF00) >> 8;
		       pixel = (value & 0xFF00) >> 8;
				 // check for value too different. If so, then don't use it.

				 maxGreen = (IORegs->vidMaxRGBDelta & 0x003f00) >>8;
				 // unnecessary: if(maxGreen & 0x80) maxGreen |= 0xffffff00; // sign extend

             if(pixel >= center) {
				 dg = (int)(pixel-center);
				 if (dg > maxGreen) dg=0;
             } else {
				 dg = (int)(center-pixel);
				 if (dg > maxGreen) dg=0;
             }
				 // check for pixel out of range. if so, then don't use it.


   		    if( (x >= (IORegs->vidOverlayStartCoords & SST_OVERLAY_X)) 
			    && (y >= ((IORegs->vidOverlayStartCoords  & SST_OVERLAY_Y)>> SST_OVERLAY_Y_SHIFT )) 
			    && (x <= (IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X)) 
			    && (y <= ((IORegs->vidOverlayEndScreenCoord  & SST_OVERLAY_Y)>>SST_OVERLAY_Y_SHIFT)) )
				 {
					return(dg);
				 }
				 else return (0);

}

FxU32 blueDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y)
{
int db;
FxU32 center, pixel;
int maxBlue;
		       center = (value00 & 0xFF) >> 0;
		       pixel = (value & 0xFF) >> 0;
				 // check for value too different. If so, then don't use it.

		       maxBlue = (IORegs->vidMaxRGBDelta & 0x00003f) >>0;
			    // unnecessary: if(maxBlue & 0x80) maxBlue |= 0xffffff00; // sign extend

             if(pixel >= center) {
				 db = (int)(pixel-center);
   			 if (db > maxBlue) db=0;
             } else {
				 db = (int)(center-pixel);
   			 if (db > maxBlue) db=0;
             } 
				 // check for pixel out of range. if so, then don't use it.

   		    if( (x >= (IORegs->vidOverlayStartCoords & SST_OVERLAY_X)) 
			    && (y >= ((IORegs->vidOverlayStartCoords & SST_OVERLAY_Y)>> SST_OVERLAY_Y_SHIFT) 
			    && (x <= (IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_X)) 
			    && (y <= (IORegs->vidOverlayEndScreenCoord & SST_OVERLAY_Y) >>SST_OVERLAY_Y_SHIFT)) )
				 {
					return(db);
				 }
				 else return (0);

}


*************************/



// bilinear interpolate the 8-bit component specified by "shift"
int olyBilerp(FxU32 xfrac, FxU32 yfrac, FxU32 value00, FxU32 value01, 
					 FxU32 value10, FxU32 value11, FxU32 shift)
 {
 int color; // or colour, in the isles
 FxU32 color00, color01, color10, color11;
 FxU32 lerp0, lerp1;
 FxU32 oneFrac;

 oneFrac = 1<<8;

 // extract the color (or colour)
	 color00 = (value00>>shift) & 0xff;
	 color01 = (value01>>shift) & 0xff;
	 color10 = (value10>>shift) & 0xff;
	 color11 = (value11>>shift) & 0xff;

	 // horizontal lerps;
	 // issue: is rounding needed? assume no.
         // ek- steve's code does vert, then horiz.
	 lerp0 = (oneFrac * color00 + yfrac * (color10-color00))>>8;
	 lerp1 = (oneFrac * color01 + yfrac * (color11-color01))>>8;
	 color = (oneFrac * lerp0   + xfrac * (lerp1-lerp0))>>8;

GDBG_INFO(VID_DEBUG,"%s, %d: biler colors: %x, %x, %x, %x, fracs = %x, %x, \n",
__FILE__, __LINE__, color00, color01, color10, color11, xfrac,yfrac);
GDBG_INFO(VID_DEBUG,"%s, %d: lerps: %x, %x, out = %x,  \n",
__FILE__, __LINE__, lerp0, lerp1, color);

 
 return(color);
 }
// extract and undither overlay data 
FxU32 olyExtractUndither(FxU32 x, FxU32 y,  SstIORegs *IORegs, int nextpixel)
{
// nextpixel input added to extract the correct u and v in bilinear
int r, g, b, tmp, tmp2;
FxU32 value;
FxU32 value2;
FxU32 pixfmt, filtMode;
FxU32 xfrac, yfrac;

			// create x fraction, y fraction start value
		   xfrac =  (IORegs->vidOverlayStartCoords>>24)&0x3;
			yfrac =	 (IORegs->vidOverlayStartCoords>>26)&0x3;

          if(x&0x80000000) x=0;
		  if(y&0x80000000) y=0;
          value = CSIM_PIXEL_RD( CSIM_BUF_OVERLAY, x, y );
GDBG_INFO(VID_DEBUG,"%s, %d: x,y = %x %x pixvalue =%x\n", __FILE__,__LINE__,x,y,value);
			pixfmt = (IORegs->vidProcCfg & SST_OVERLAY_PIXEL_FORMAT); //overlay 
			filtMode = (IORegs->vidProcCfg & SST_OVERLAY_FILTER_MODE); // filter mode
		   switch(pixfmt){
			 case(SST_OVERLAY_PIXEL_RGB32U):	 //  32 bit RGB undithered
		       r = ((value & 0xFF0000) >> 16);
		       g = ((value & 0x00FF00) >> 8);
		       b = ((value & 0x0000FF));
				 if(r>255) r=255;
				 if(g>255) g=255;
				 if(b>255) b=255;
			    break;
			 case(SST_OVERLAY_PIXEL_RGB1555U):	 //  1555 RGB undithered
		       r = ((value & 0x7C00) << 9) >> 16; // <<2; >>12; <<3
		       g = ((value & 0x03E0) << 6) >> 8;  // <<3; >>8; <<3
		       b = ((value & 0x001F) << 3);
		       r +=  (r >> 5); // msbs for lsbs.
		       g +=  (g >> 5);
  		       b +=  (b >> 5);
				 if(r>255) r=255;
				 if(g>255) g=255;
				 if(b>255) b=255;
             
			    break;
			 case(SST_OVERLAY_PIXEL_RGB565U):	 //  565 RGB
		       r = ((value & 0xF800) << 8) >> 16;
		       g = ((value & 0x07E0) << 5) >> 8;
		       b = ((value & 0x001F) << 3);
		       r +=  (r >> 5); // msbs for lsbs.
		       g +=  (g >> 6);
  		       b +=  (b >> 5);
				 if(r>255) r=255;
				 if(g>255) g=255;
				 if(b>255) b=255;
             
			    break;
          case(SST_OVERLAY_PIXEL_YUV411) :
             // need to fetch two words to do this one.
             // fetch at x&0xfffc, and (x& 0xfffc)+2
             value = CSIM_PIXEL_RD( CSIM_BUF_OVERLAY, x&0xfffc, y ); 
                            // 4-pixel aligned
             value2 = CSIM_PIXEL_RD( CSIM_BUF_OVERLAY, (x&0xfffc)+2, y ); 
GDBG_INFO(VID_DEBUG,"%s, %d: x,y = %x %x 411 pixvalues =%x %x\n", __FILE__,__LINE__,x,y,value, value2);
             switch(x%4)
             {
              case(0): r= (value>>0) & 0xff; break; // Y
              case(1): r= (value>>16) & 0xff; break; // Y
              case(2): r= (value2>>0) & 0xff; break; // Y
              case(3): r= (value2>>16) & 0xff; break; // Y

             }
              // reconstruct u
			  tmp = (value2>>30) & 0x3; // bit 10
			  tmp |= (value2>>(16-4)) & 0xc; // bit 32
			  tmp |= (value>>(30-4)) & 0x30; // bit 54
			  tmp |= (value>>(16-8)) & 0xc0; // bit 76
              g=tmp; // u
              // reconstruct v
           tmp = (value2>>(30-2)) & 0x3; // bit 10
			  tmp |= (value2>>(16-4-2)) & 0xc; // bit 32
			  tmp |= (value>>(30-4-2)) & 0x30; // bit 54
			  tmp |= (value>>(16-8-2)) & 0xc0; // bit 76
              b=tmp; // v
             /* super hack for odd x pixels in bilinear mode 
                here, u and v are the average of the two nearest.*/
				  if( SST_OVERLAY_FILTER_BILINEAR 
                 == filtMode)
              {
                 value = CSIM_PIXEL_RD( CSIM_BUF_OVERLAY, (x+3)&0xfffc, y ); 
                            // 4-pixel aligned
                 value2 = CSIM_PIXEL_RD( CSIM_BUF_OVERLAY, ((x+3)&0xfffc)+2, y ); 
                  // reconstruct u
						if((x%4) != 0) // if nextpixel=0, do nothing, if 1,2,3, then do something
						{
			  tmp = (value2>>30) & 0x3; // bit 10
			  tmp |= (value2>>(16-4)) & 0xc; // bit 32
			  tmp |= (value>>(30-4)) & 0x30; // bit 54
			  tmp |= (value>>(16-8)) & 0xc0; // bit 76
                  // reconstruct v
              tmp2 = (value2>>(30-2)) & 0x3; // bit 10
			  tmp2 |= (value2>>(16-4-2)) & 0xc; // bit 32
			  tmp2 |= (value>>(30-4-2)) & 0x30; // bit 54
			  tmp2 |= (value>>(16-8-2)) & 0xc0; // bit 76
					  if(nextpixel)
					  { g= tmp; b=tmp2; }
               }
					}
                                
			    break;
          case(SST_OVERLAY_PIXEL_YUYV422) :
				 if(x&1) // odd pixel may be problems with bit-odering
				    r= (value>>16) & 0xff; // Y
				 else
				    r= (value>>0) & 0xff; // Y
				 g= (value>>8)& 0xff; // U
				 b= (value>>24)& 0xff; // V
             /* super hack for odd x pixels in bilinear mode 
                here, u and v are the average of the two nearest.*/
				  if( SST_OVERLAY_FILTER_BILINEAR 
                 == (IORegs->vidProcCfg & SST_OVERLAY_FILTER_MODE))
              {
                  value = CSIM_PIXEL_RD( CSIM_BUF_OVERLAY, x+1, y );
						if(nextpixel)
						{
				         g = (value>>8)& 0xff; // U
				         b = (value>>24)& 0xff; // V
						}
              }
			    break;
          case(SST_OVERLAY_PIXEL_UYVY422) :
				 if(x&1) // odd pixel may be problems with bit-odering
				     r= (value>>24) & 0xff; // Y
				 else
				     r= (value>>8) & 0xff; // Y
				 g= (value>>0)& 0xff; // U
				 b= (value>>16)& 0xff; // V
             /* super hack for odd x pixels in bilinear mode 
                here, u and v are the average of the two nearest.*/
				  if( SST_OVERLAY_FILTER_BILINEAR 
                 == (IORegs->vidProcCfg & SST_OVERLAY_FILTER_MODE))
              {
                  value = CSIM_PIXEL_RD( CSIM_BUF_OVERLAY, x+1, y );
						if(nextpixel)
						{
				        g = (value>>0)& 0xff; // U
				        b = (value>>16)& 0xff; // V
                  }
              }
			    break;
			  case(SST_OVERLAY_PIXEL_RGB1555D):	 //   dithered 1555 bit.
		       r = ((value & 0x7C00) << 9) >> 16; // <<2; >>12; <<3
		       g = ((value & 0x03E0) << 6) >> 8;  // <<3; >>8; <<3
		       b = ((value & 0x001F) << 3);
				 // we don't use the copy msb to lsb trick here
				 // ek: assume 2x2 dithering if not specifically 4x4
				xfrac = (xfrac+x) & 0x3;
				yfrac = (yfrac+y) & 0x3;
		       if (filtMode == SST_OVERLAY_FILTER_2X2) 
			    {

				  // swan:
				  // 2x2 dither subtract --
				  // For r/g/b:
				  // in hw, it is ((r << 4) + ditmax) >> 1; 
				  // in csim, since r is already << 3 and ditmax is already divided by 2
              // it seems to be that it is actually doing 
              // (r << 3) + ditmax which is correct.

					r +=undither2x2[yfrac][xfrac]; 
					//g +=(undither2x2[yfrac][xfrac])/2; // for original 565
					g +=(undither2x2[yfrac][xfrac]);
					b +=undither2x2[yfrac][xfrac];
			    }
				 else if (filtMode ==SST_OVERLAY_FILTER_BILINEAR) 
				 {
             //ek another change to match hardware.
             // color expansion done before bilinear, but after 2x2
				 // 565 already expanded to 888, but the bottom bits need to be gened.
		          r +=  (r >> 5); // msbs for lsbs.
                g +=  (g >> 5); // swan: change 6 to 5
  		          b +=  (b >> 5);
				    if(r>255) r=255;
				    if(g>255) g=255;
				    if(b>255) b=255;
             }
			    else { // if (filtMode==SST_OVERLAY_FILTER_4X4)

					
				  // swan:
				  // 4x4 dither subtract --
				  // For r/b:
				  // in hw, it is ((r << 3) + ditmax);
				  // in csim, since r is already << 3 and ditmax is just ditmax (hasnt divided by 2);
              // it seems to be that it is actually doing 
              // (r << 3) + ditmax which is correct.

					//g = g*2; // for original 565 to match hsim

GDBG_INFO(VID_DEBUG,"%s, %d: undither4x4[%x][%x] = %x\n", __FILE__,__LINE__,yfrac,xfrac,undither4x4[yfrac][xfrac]);

					r +=undither4x4[yfrac][xfrac];
					g +=(undither4x4[yfrac][xfrac]);
					b +=undither4x4[yfrac][xfrac];
					//g = g>>1; // for original 565
				   }

			    if(r & 0x80000000) r=0;
 // swan_for_av+: I think this should be ( r & 0x100)
			    if(g & 0x80000000) g=0;
			    if(b & 0x80000000) b=0;
			     break;

			  case(SST_OVERLAY_PIXEL_RGB565D):	 //   dithered 16 bit.
		       r = ((value & 0xF800) << 8) >> 16; // <<1; >>12; <<3 = >>8
		       g = ((value & 0x07E0) << 5) >> 8;  // <<3; >>8; <<2 = >>3
		       b = ((value & 0x001F) << 3);       // <<3
 
				 // we don't use the copy msb to lsb trick here
				 // ek: assume 2x2 dithering if not specifically 4x4
				xfrac = (xfrac+x) & 0x3;
				yfrac = (yfrac+y) & 0x3;
		       if (filtMode == SST_OVERLAY_FILTER_2X2) 
			    {
					
				  // swan:
				  // 2x2 dither subtract --
				  // For r/b:
				  // in hw, it is ((r << 4) + ditmax) >> 1; 
				  // in csim, since r is already << 3 and ditmax is already divided by 2
              // it seems to be that it is actually doing 
              // (r << 3) + ditmax which is correct.
				  // For g:
              // in hw, it is ((g << 4) + ditmax) >> 2;
				  // in csim, since g is already << 2 and ditmax is already divided by 2
              // it seems to be that it is actually doing 
				  // (g << 2) + (ditmax/2) which is correct.

					r +=undither2x2[yfrac][xfrac];
					g +=(undither2x2[yfrac][xfrac])/2;
					b +=undither2x2[yfrac][xfrac];
			    }
             else if (filtMode ==SST_OVERLAY_FILTER_BILINEAR) {
             //ek another change to match hardware.
             // color expansion done before bilinear, but after 2x2
				 // 565 already expanded to 888, but the bottom bits need to be gened.
		          r +=  (r >> 5); // msbs for lsbs.
                if((pixfmt == SST_OVERLAY_PIXEL_RGB1555U) | (pixfmt == SST_OVERLAY_PIXEL_RGB1555D))
                  g +=  (g >> 5);
				    else
		            g +=  (g >> 6);
  		          b +=  (b >> 5);
				    if(r>255) r=255;
				    if(g>255) g=255;
				    if(b>255) b=255;
             }
			    else { // if (filtMode==SST_OVERLAY_FILTER_4X4)

				  // swan:
				  // 4x4 dither subtract --
				  // For r/b:
				  // in hw, it is ((r << 3) + ditmax); 
				  // in csim, since r is already << 3 and ditmax is just ditmax (hasn't divided by 2)
              // it seems to be that it is actually doing 
              // (r << 3) + ditmax which is correct.
				  // For g:
              // in hw, it is ((g << 3) + ditmax) >> 1;
				  // in csim, since g is already << 2 and ditmax is just ditmax (hasn't divided by 2)
              // it seems to be that it is actually doing 
				  // (g << 2) + (ditmax/2) which is correct.

					g = g*2; // to match hsim to become (g << 3)
					r +=undither4x4[yfrac][xfrac];
					g +=(undither4x4[yfrac][xfrac]);
					b +=undither4x4[yfrac][xfrac];
					g = g>>1;
				  
			    }



			    if(r & 0x80000000) r=0;
			    if(g & 0x80000000) g=0;
			    if(b & 0x80000000) b=0;
			     break;
		   }

           /* overlay data processing: Undithering (subtract) */
			  r &= 0xff;
			  g &= 0xff;
			  b &= 0xff;
		     value = (b<<0) | (g<<8) | (r<<16);
				 return(value);
}

/***** Color space conversion routine ***/
FxU32 vidOutCSC ( FxU32 value, SstIORegs *IORegs, FxBool pixType) 
{
       FxU32 pixfmt;
		 int r, g, b ;
		 float rf, gf,bf;
		 int y, u, v;
		 // code needs work
		 if(pixType) pixfmt = (IORegs->vidProcCfg>>SST_OVERLAY_PIXEL_FORMAT_SHIFT) & 0x7; //overlay 
		 else  pixfmt = (IORegs->vidProcCfg>>SST_DESKTOP_PIXEL_FORMAT_SHIFT) & 0x7; //desktop 
		   switch(pixfmt){
		
			 case(SST_DESKTOP_PIXEL_PAL8>>SST_DESKTOP_PIXEL_FORMAT_SHIFT):	 // 8 -bit palletized if desktop; 1555D if overlay

            if(pixType== 0) { // desktop (pal8)
				 value = value & 0xff;
				 value = value | value<<8 | value<<16; // 24-bit value
            } else { // overlay (1555D)
		       r = (value & 0xFF0000) >> 16;
		       g = (value & 0x00FF00) >> 8;
		       b = (value & 0x0000FF) ;
		       value = (r << 16) | (g << 8) | b;
            }

			   break; // csc already done. Pallete is done later.

			 case(SST_DESKTOP_PIXEL_RGB565>>SST_DESKTOP_PIXEL_FORMAT_SHIFT):	 //  565 RGB
				 // 565 already expanded to 888, but the bottom bits need to be gened.
		       r = (value & 0xFF0000) >> 16;
		       g = (value & 0x00FF00) >> 8;
		       b = (value & 0x0000FF) ;
             if(pixType== 0)
             {
		         r +=  (r >> 5); // msbs for lsbs.
		         g +=  (g >> 6);
  		         b +=  (b >> 5);
				   if(r>255) r=255;
				   if(g>255) g=255;
				   if(b>255) b=255;
             }
		       value = (r << 16) | (g << 8) | b;
			    break;



			  case(2):	 //   RGB 24 bit for desktop and 1555U for overlay

            if(pixType == 0) {// desktop
		       r = (value & 0xFF0000) >> 16;
		       g = (value & 0x00FF00) >> 8;
		       b = (value & 0x0000FF) ;
		       // since Suns are big endian, store as BGRA, so when read
		       value = (r << 16) | (g << 8) | b;
            }
            else { // 1555U for overlay

				 // 1555 already expanded to 888, and bottom bits already added in overlayProcess, unlike the desktop
		       r = (value & 0xFF0000) >> 16;
		       g = (value & 0x00FF00) >> 8;
		       b = (value & 0x0000FF) ;

				   if(r>255) r=255;
				   if(g>255) g=255;
				   if(b>255) b=255;
             
		       value = (r << 16) | (g << 8) | b;

            }

			   break;

			  case(3):	 //   RGB 32 bit-same csc as previous
		       r = (value & 0xFF0000) >> 16;
		       g = (value & 0x00FF00) >> 8;
		       b = (value & 0x0000FF) ;
		       value = (r << 16) | (g << 8) | b;
			   break;
			  case(4):	 //   YUV411 for overlay or 1555U for desktop
			  case(5):	 //   YUYV422 // same as 4
			  case(6):	 //   UYUV422 // same as 4

           if((pixType== 0) && (pixfmt == (FxU32) 4)) {// SST_DESKTOP_PIXEL_RGB1555U

				 // 1555 already expanded to 888, but the bottom bits need to be gened.
		       r = (value & 0xFF0000) >> 16;
		       g = (value & 0x00FF00) >> 8;
		       b = (value & 0x0000FF) ;

		         r +=  (r >> 5); // msbs for lsbs.
		         g +=  (g >> 5);
  		         b +=  (b >> 5);
				   if(r>255) r=255;
				   if(g>255) g=255;
				   if(b>255) b=255;
             
		       value = (r << 16) | (g << 8) | b;

             }

             else {

		       y = (value & 0xFF0000) >> 16;
		       u = (value & 0x00FF00) >> 8;
		       v = (value & 0x0000FF) ;

				 rf = (float)0.5+ ((float)(y) + (float)1.40200*((float)v - (float)128.0));
				 r = (int) rf;
				 if(r<0) r=0;
				 if(r>255) r=255;
				 gf =   (float)-0.34414*((float)u - (float)128.0);
				 if (gf<(float)0.0) gf -= (float)0.5; else gf+= (float)0.5; // round
				 g =  y + (int) gf;
				 gf = ((float) -0.71414*((float)v - (float)128.0));
				 if (gf<(float)0.0) gf -= (float)0.5; else gf+= (float)0.5; // round
				 g += (int) gf;
				 if(g<0) g=0;
				 if(g>255) g=255;
				 
				 bf = (float)0.5+ ((float)(y) + (float)1.77200*((float)u - (float)128.0));
				 b = (int) bf;
				 if(b<0) b=0;
				 if(b>255) b=255;
		       value = (r << 16) | (g << 8) | b;
           }

			  break;

			  case(7):	 //   Undithered 16 bit, same as 24 bit. swan -- I think the comment should say dithered 16 bit
		       r = (value & 0xFF0000) >> 16;
		       g = (value & 0x00FF00) >> 8;
		       b = (value & 0x0000FF) ;
		       value = (r << 16) | (g << 8) | b;
			   break;
		   }

	 return(value);
}


static FxBool screenDump(FxU32 *cp, const char *filenameRoot,
			 FxU32 width, FxU32 height, FxU32 imgType)
{

  char filenameExt[10];
  char filename[256];
  FxU32 x,y;
  FxU8 *buf;
  FxU32 col;
  ImgInfo info;
  
  info.any.width = width;
  info.any.height = height;
  info.any.sizeInBytes = width * height * 4;
  info.any.data = (ImgData *)malloc(info.any.sizeInBytes);
  
#if !defined(__unix__)
  strlwr(filenameExt);
#endif
  
  switch (imgType) {
  case IMG_P6:
    strcpy(filenameExt, ".ppm");
    break;
  case IMG_SBI:
    info.sbiInfo.redBits = 5;
    info.sbiInfo.greenBits = 6;
    info.sbiInfo.blueBits = 5;
    info.sbiInfo.yOrigin = 1;
    strcpy(filenameExt, ".sbi");
    break;
  case IMG_TGA32:
    info.tgaInfo.yOrigin = 0;
    strcpy(filenameExt, ".tga");
    break;
  default:
    GDBG_ERROR("screenDump", "invalid image type %d\n",imgType);
    return(-1);
  }
  
  // note we first convert the framebuffer which is in 16-bit RGB format
  // to standard 32-bit ARGB format (in memory) for the image library
  buf = (FxU8 *)info.any.data;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {

      col = cp[x+y*width];

#ifdef HAL_HSIM 
      if ( diago.halInfo->hsim ) 
	VID_WR( (ulong) x+y*width, (ulong) col );
#endif 
      
#ifdef H3_A0
      // invert pixel color again to simulate the presence of the inverting dac of H3_A0
      // so that the colors in the .ppm file will match those on the screen
      if ( x==0 && y==0 )
	GDBG_INFO(0,"screenDump: HACK inverting image\n");
      col = ~col;
#endif

      *buf++ = (FxU8) (col & 0xFF);       // b
      *buf++ = (FxU8) ((col>>8) & 0xFF);  // g
      *buf++ = (FxU8) ((col>>16) & 0xFF); // r
      *buf++ = (FxU8) (0x0);              // a
    }
  }
  sprintf(filename,"%s_vid%s",filenameRoot,filenameExt);
  if (!imgWriteFile(filename, &info, imgType, info.any.data))
    GDBG_ERROR( "imgWriteFile", "file '%s' failed: %s\n", filename, imgGetErrorString() );
  
  free(info.any.data);
  return(0);

}

/*-*-c++-*-*/
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
** $Revision: 4$
** $Date: 10/11/00 8:12:00 PM$
*/

#include <stddef.h>

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "hsimio.h"
#include <fximg.h>

//Amount of additional memory to tack onto buffer allocations
#define BUFFER_SLOP   8192   //2 Pages

//Amount of additional memory to tack onto cmdFifo allocations
#define CMD_FIFO_SLOP 4096   //1 Page

static shutdown = 0;
int diagSwaps;          // number of swaps commands issued (without DONT_SWAP)

void openVectorFiles( CsimPrivate *cpriv, unsigned int vectorGenMask );
void closeVectorFiles( CsimPrivate *cpriv, unsigned int vectorGenMask );
FxU32 generateDeviceNumber(FxU32 busNumber, FxU32 deviceNumber, FxU32 functionNumber);

FxU32 calculateTiledTexturePortAddress(FxU32 textureMode, FxU32 tLOD, FxU32 lod, FxU32 s, FxU32 t, FxU32 wordSelect);
void sstSetupMultichip(SstRegs *sst);
void sstSetupMultichipConfigSpace(SstRegs *sst);

void rgb_to_888( int buffer, int x, int y, int r, int g, int b ) {
  FxU32 col = (r<<16) | (g<<8) | b;
  DIAG_FORCE_PIXEL(buffer,x,y,col);
}


//----------------------------------------------------------------------
// SST-1 specific DIAG_BEGIN routines
//----------------------------------------------------------------------
SstRegs *SST_BEGIN2d(int argc, char **argv)
{
    int stride;
    SstRegs *sst;
    CsimPrivate *cpriv;
    int tstride;
    SstGRegs *sstg;

    static FxU32 _bpp[] = {
        SSTG_PIXFMT_1BPP, SSTG_PIXFMT_8BPP, SSTG_PIXFMT_15BPP, SSTG_PIXFMT_16BPP,
        SSTG_PIXFMT_24BPP, SSTG_PIXFMT_32BPP, 0, 0, SSTG_PIXFMT_422YUV, SSTG_PIXFMT_422UYV
    };
    int dst_is_tiled = 0;
    int src_is_tiled = 0;

    diago.gui = 1;                              // flag as 2D app
    diago.dstFormat = SSTG_PIXFMT_32BPP>>SSTG_SRC_FORMAT_SHIFT;
    sst = SST_BEGIN(argc,argv);                 // parse args
    sstg = SSTG_CHIP(sst);
    cpriv = CSIM_PRIVATE(diago.sstCSIM);

    if (diago.dstFormat < (SSTG_PIXFMT_8BPP>>SSTG_SRC_FORMAT_SHIFT) ||
        diago.dstFormat > (SSTG_PIXFMT_32BPP>>SSTG_SRC_FORMAT_SHIFT)) {
      GDBG_ERROR("SST_BEGIN2D", "invalid destination format\n");
      DIAG_FAIL();
    }

    stride = diago.xmaxscreen*4;
    tstride = CEIL(stride,SST_TILE_WIDTH);
    
    //  diago.ytiled    Src             Dst
    //  ------------    ------          ------
    //       0          Linear          Linear
    //       1          Tiled           Tiled 
    //       2          Linear          Tiled 
    //       3          Tiled           Linear

    if ( diago.ytiled == 1 || diago.ytiled == 2 ) dst_is_tiled = 1;
    if ( diago.ytiled == 1 || diago.ytiled == 3 ) src_is_tiled = 1;
    
    SET(sstg->dstBaseAddr, diagfb.colBufferAddr[0] | 
	(dst_is_tiled ? SSTG_IS_TILED : 0) );     // define a default surface
    SET(sstg->srcBaseAddr, diagfb.colBufferAddr[0] | 
	(src_is_tiled ? SSTG_IS_TILED : 0) );     // Default 
    SET(sstg->dstFormat,_bpp[diago.dstFormat] | (dst_is_tiled ? tstride : stride) );
    SET(sstg->srcFormat,_bpp[diago.dstFormat] | (src_is_tiled ? tstride : stride) );
    
    GDBG_INFO(1,"SST_BEGIN2d: Source is %s, Destination is %s\n",
              src_is_tiled  ? "Tiled" : "Linear",
              dst_is_tiled  ? "Tiled" : "Linear");


    if (cpriv->windows) {
      if ( dst_is_tiled ) {
            cpriv->windows->base = diagfb.colBufferAddr[0];
            cpriv->windows->tiled = 1;
            cpriv->windows->stride = tstride;
            cpriv->windows->endBase = tiledAddress(cpriv->windows->base,
						   cpriv->windows->stride,
						   2,
						   cpriv->windows->width+1,
						   cpriv->windows->height);
        } else {
            cpriv->windows->base = diagfb.colBufferAddr[0];
            cpriv->windows->stride = stride;
            cpriv->windows->endBase = cpriv->windows->base + (cpriv->windows->height+1)*cpriv->windows->stride;
        }
    }

#if !defined(CVG) && !defined(SST2)
    if (diago.halInfo->hw) {
      // setup video for 2d diags
      static FxU32 _fmt[] = {
	0,                        // 1 bpp
	SST_DESKTOP_PIXEL_PAL8,   // 8bpp
	0,                        // 15bpp
	SST_DESKTOP_PIXEL_RGB565, // 16bpp
	SST_DESKTOP_PIXEL_RGB24,  // 24bpp
	SST_DESKTOP_PIXEL_RGB32   // 32bpp
      };

      int stride = diago.xmaxscreen*4;
      int tstride = CEIL(stride,SST_TILE_WIDTH);
 
      sstInitVideoDesktop(sst,
			  1,				// 1=enable desktop surface (DS), 1=disable
			  dst_is_tiled,			// 0=DS linear, 1=tiled
			  _fmt[diago.dstFormat],	// pixel format of DS
			  0,				// bypass clut for DS?
			  0,				// 0=lower 256 CLUT entries, 1=upper 256
			  diagfb.colBufferAddr[0],	// board address of beginning of DS
			  dst_is_tiled ? tstride : stride // stride
			  );

    }
#endif

    setSeed(diago.seed);  // restore user-specified seed

    return sst;
}

SstRegs *SST_BEGIN(int argc, char **argv)
{
    char buf[80];
    int ntrex;
    int rez = GR_RESOLUTION_640x480;    
    CsimPrivate *cpriv;
    SstRegs *sst;
    SstIORegs *sstio;
    FxI32 fifoSize;
    FxI32 counter;
    FxBool useMultiFunctionDevices=FXFALSE;
    FxU32 busNumber, deviceNumber, functionNumber;

    GDBG_INFO(0, "BUILD_DATE: %s\n", BUILD_DATE);

    diago.pixelLimit = 10000;                   // limit diags to 10000 pixels (those that obey)
    diago.errorLimit = 100;                     // for now up this limit
    DIAG_BEGIN(argc,argv);                      // chip independent diag code
#ifndef HAL_HSIM
    if (diago.halInfo->hsim) {
        GDBG_ERROR("SST_BEGIN","-H requested but HSIM was not compiled in\n");
        DIAG_FAIL();
    }
#endif
    // send the command line data to CSIM via environment variables
    if (diago.deviceID != CSIM_DEFAULT_DEVICE_ID) {
        sprintf(buf,"SST_DEVICE_ID=%d",diago.deviceID);
        fxHalPutenv(buf);
    }
    if (diago.fbiRevision != CSIM_DEFAULT_FBI_REV) {
        sprintf(buf,"SST_FBI_REV=%d",diago.fbiRevision);
        fxHalPutenv(buf);
    }
    ntrex = diago.trex;
    if (ntrex < 0) ntrex = -ntrex;              // if need more then default(1)

    if (diago.trexRevision != CSIM_DEFAULT_TMU_REV) {
        sprintf(buf,"SST_TMU_REV=%d",diago.trexRevision);
        fxHalPutenv(buf);
    }
    trx_x2_init_table();
    trx_log2_init_table();
    trx_inv_init_table();                       // jimm's hardware accurate model
      
    //Plug in an appropriate number of virtual csim boards/chips
    switch(diago.chipCount)
      {
      case 3: 
	GDBG_ERROR("SST_BEGIN", "What the f are you smoking running with 3 chips?\n");
	DIAG_FAIL();
	break;

      case 4:
	busNumber      = 3;
	deviceNumber   = 2;
	functionNumber = 0;
	diago.halInfo->boardInfo[2].pciBusNumber = busNumber;
	diago.halInfo->boardInfo[2].pciDeviceNumber = deviceNumber;
	diago.halInfo->boardInfo[2].pciFunctionNumber = functionNumber; 
	diago.halInfo->boardInfo[2].deviceNumber = 
	  generateDeviceNumber(busNumber, deviceNumber, functionNumber);

	busNumber      = 3;
	deviceNumber   = 2;
	functionNumber = 1;
	diago.halInfo->boardInfo[3].pciBusNumber = busNumber;
	diago.halInfo->boardInfo[3].pciDeviceNumber = deviceNumber;
	diago.halInfo->boardInfo[3].pciFunctionNumber = functionNumber; 
	diago.halInfo->boardInfo[3].deviceNumber = 
	  generateDeviceNumber(busNumber, deviceNumber, functionNumber);
	    
	useMultiFunctionDevices=FXTRUE;
      case 2:
	busNumber      = 2;
	deviceNumber   = 1;
	functionNumber = 1;
	diago.halInfo->boardInfo[1].pciBusNumber = busNumber;
	diago.halInfo->boardInfo[1].pciDeviceNumber = deviceNumber;
	diago.halInfo->boardInfo[1].pciFunctionNumber = functionNumber; 
	diago.halInfo->boardInfo[1].deviceNumber = 
	  generateDeviceNumber(busNumber, deviceNumber, functionNumber);

	useMultiFunctionDevices=FXTRUE;
      case 1:
	busNumber      = 2;
	deviceNumber   = 1;
	functionNumber = 0;
	diago.halInfo->boardInfo[0].pciBusNumber = busNumber;
	diago.halInfo->boardInfo[0].pciDeviceNumber = deviceNumber;
	diago.halInfo->boardInfo[0].pciFunctionNumber = functionNumber; 
	diago.halInfo->boardInfo[0].deviceNumber = 
	  generateDeviceNumber(busNumber, deviceNumber, functionNumber);
	break;

      default:
	GDBG_ERROR("SST_BEGIN", "The poop quality of the CSIM limits it to 4 chips\n");
	DIAG_FAIL();
      }

    //Initialize the csim boards/chips
    for(counter=0; counter<diago.chipCount; counter++)
      {
	diago.halInfo->boardInfo[counter].sstCSIM = csimInit(counter);      
	if(useMultiFunctionDevices)
	  csimMakeMultiFunctionDevice(diago.halInfo->boardInfo[counter].sstCSIM);
	CSIM_PRIVATE(diago.halInfo->boardInfo[counter].sstCSIM)->environment.chipCount = diago.chipCount;

	if(counter == 0)
	  diago.sstCSIM = diago.halInfo->boardInfo[counter].sstCSIM;
	else
	  diago.sstChildrenCSIM[counter-1] = diago.halInfo->boardInfo[counter].sstCSIM;
      }

    //Diags are too f'ed up to try to use this error detection
    for(counter=0; counter<diago.chipCount; counter++)
      CSIM_PRIVATE(diago.halInfo->boardInfo[counter].sstCSIM)->environment.detectNopError = FXFALSE;

    
    
    //Now map the boards
    for(counter=0; counter<diago.chipCount; counter++)
      {
	sst = fxHalMapBoard(counter);

	if(sst == NULL)
	  {
	    GDBG_ERROR("SST_BEGIN", "Couldn't map board %d\n", counter);
	    DIAG_FAIL();
	  }
	
	if(counter == 0)
	  diago.sst = sst;
	else
	  diago.sstChildren[counter-1] = sst;
      }

    //Setup some of the config space registers for multi-chip stuff
    sstSetupMultichipConfigSpace(sst);

    sst = diago.sst;    
    cpriv = CSIM_PRIVATE(diago.sstCSIM);

    sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
  
    //If not running multi-chip, give the parent a blank name
    if(diago.chipCount <= 1)
      cpriv->environment.name[0]=0;
    
    //Set the default chipMask to write to all chips
    SET(sst->chipMask, SST_CHIP_MASK_ALL_CHIPS);

    {
      FxU32 s = getSeed();
      SstIORegs *sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
      FxU32 miscInit0 = GET(sstio->miscInit0);
      FxU32 bsReg, wsReg, bsLfb, wsLfb;

      if ( diago.swizzle == -1 ) {
	bsReg = iRandom(20) < 10;
	wsReg = iRandom(20) < 10;
	bsLfb = iRandom(20) < 10;
	wsLfb = iRandom(20) < 10;
      } else {
	bsLfb = diago.swizzle % 10;         // ones digit
	wsLfb = (diago.swizzle/10) % 10;    // tens digit
	bsReg = (diago.swizzle/100) % 10;   // hundreds digit
	wsReg = (diago.swizzle/1000) % 10;  // thousands digit
      }
      GDBG_INFO(0,"SWIZZLING Registers: %s %s %s %s\n",
		bsReg?"bytes":"",
		bsReg&&wsReg?"and":"",
		wsReg?"words":"",
		bsReg||wsReg?"":"none");
      GDBG_INFO(0,"SWIZZLING Raw LFB: %s %s %s %s\n",
		bsLfb?"bytes":"",
		bsLfb&&wsLfb?"and":"",
		wsLfb?"words":"",
		bsLfb||wsLfb?"":"none");
      if (wsReg) 
	miscInit0 |= SST_REGISTER_WORD_SWIZZLE_EN;
      if (wsLfb) 
	miscInit0 |= SST_RAWLFB_WORD_SWIZZLE_EN;
      if (bsReg) 
	miscInit0 |= SST_REGISTER_BYTE_SWIZZLE_EN;
      if (bsLfb) 
	miscInit0 |= SST_RAWLFB_BYTE_SWIZZLE_EN;

      SET(sstio->miscInit0,miscInit0);
      setSeed(s);
    }

    // open vector files 
    if ( diago.vectorGenMask ) openVectorFiles( cpriv, diago.vectorGenMask ); 

    if (!fxHalInitRegisters(sst))
        GDBG_ERROR("SST_BEGIN", "fxHalInitRegisters failed\n");
    if (!fxHalInitGamma(sst, 1.4F))
        GDBG_ERROR("SST_BEGIN", "fxHalInitGamma failed\n");

    // perform some SST-specific configuration 
    // XXX only support these three resolutions right now
    if (diago.width > 2048) {
        GDBG_ERROR("parse_opts","framebuffer is too large, max is 2048\n");
    } else if (diago.width > 1920) {
        diago.xmaxscreen = 2048;
        diago.ymaxscreen = 1536;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_2048x1536;
    } else if (diago.width > 1856) {
        diago.xmaxscreen = 1920;
        diago.ymaxscreen = 1440;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_1920x1440;
    } else if (diago.width > 1792) {
        diago.xmaxscreen = 1856;
        diago.ymaxscreen = 1392;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_1856x1392;
    } else if (diago.width > 1600) {
        diago.xmaxscreen = 1792;
        diago.ymaxscreen = 1344;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_1792x1344;
    } else if (diago.width > 1280) {
        diago.xmaxscreen = 1600;
        diago.ymaxscreen = 1200;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_1600x1200;
    } else if (diago.width > 1024) {
        diago.xmaxscreen = 1280;
        diago.ymaxscreen = 1024;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_1280x1024;
        if ( diago.refreshRate == GR_REFRESH_NONE )
	  diago.refreshRate = GR_REFRESH_75Hz;
    } else if (diago.width > 800) {
        diago.xmaxscreen = 1024;
        diago.ymaxscreen = 768;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_1024x768;
    } else if (diago.width > 640) {
        diago.xmaxscreen = 800;
        diago.ymaxscreen = 600;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_800x600;
    } else {
        diago.xmaxscreen = 640;
        diago.ymaxscreen = 480;
        diago.hasAuxBuffer = 1;
        rez = GR_RESOLUTION_640x480;
    }
    if ( diago.refreshRate == GR_REFRESH_NONE )
	diago.refreshRate = GR_REFRESH_60Hz;

    //Go through and turn off the enhanced video in all the chips
    //Also shut off all the video
    for(counter=0; counter<diago.chipCount; counter++)
      {
	SstIORegs *sstio;
	FxU32 vidScreenSize, data;
	PciRegister cfgVideoCtrl0 = {128, 4, READ_WRITE};
	
	if(counter == 0)
	  sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sst));  
	else
	  sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sstChildren[counter-1]));  

	SET(sstio->vidProcCfg, 0);

	vidScreenSize = GET(sstio->vidScreenSize);   //Disable the fifo for desktop address
	vidScreenSize &= ~(SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE);
	SET(sstio->vidScreenSize, vidScreenSize);

	//pciGetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[counter].deviceNumber, &data);
	//data &= (~SST_CFG_ENHANCED_VIDEO_EN | SST_CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
	//SST_CFG_DIVIDE_VIDEO);
	data=0;
	pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[counter].deviceNumber, &data);	
      }

    if (!fxHalInitVideo(sst, rez, diago.refreshRate, NULL))
        GDBG_ERROR("SST_BEGIN", "fxHalInitVideo failed\n");

    //Initialize the children devices
    for(counter=0; counter<diago.chipCount-1; counter++)
      {
	GDBG_INFO(2, "Initializing child %d", counter);
	if(!fxHalInitRegisters(diago.sstChildren[counter]))
	  GDBG_ERROR("SST_BEGIN", "fxHalInitRegisters failed on child %d\n", counter);
	
	if (!fxHalInitGamma(diago.sstChildren[counter], 1.4F))
	  GDBG_ERROR("SST_BEGIN", "fxHalInitGamma failed on child %d\n", counter);

	if (!fxHalInitVideo(diago.sstChildren[counter], rez, diago.refreshRate, NULL))
	  GDBG_ERROR("SST_BEGIN", "fxHalInitVideo failed on child %d\n", counter);
      }


    //If using a random frame buffer depth, select it
    if(diago.rgb == 0)
      {	    
	switch(iRandom(2))
	  {
	  case 0:
	    diago.rgb = 15;
	    GDBG_INFO(1, "Randomly selected 15bpp\n");
	    break;
	  case 1:
	    diago.rgb = 16;
	    GDBG_INFO(1, "Randomly selected 16bpp\n");
	    break;
	  case 2:
	    diago.rgb = 32;
	    GDBG_INFO(1, "Randomly selected 32bpp\n");
	    break;
	  default:
	    assert(0);
	  }	    
      }
    

    // GMT: switch to using the old yorigin bits
    if (diago.yorigin) {
	FxU32 temp;
        SstIORegs *sstio = (SstIORegs *)SST_IO_ADDRESS(sst);

	// turn off the new method
	temp = GET(sst->renderMode);
	temp &= ~(SST_RM_YORIGIN_SELECT | SST_RM_YORIGIN_TOP);

	if (diago.rgb == 15) temp |= SST_RM_15BPP | SST_RM_ALPHA_MSB;
	if (diago.rgb == 16) temp |= SST_RM_16BPP;
	if (diago.rgb == 32) temp |= SST_RM_32BPP | SST_RM_ALPHA_WMASK |
		SST_RM_RED_WMASK | SST_RM_GREEN_WMASK | SST_RM_BLUE_WMASK;
        SET(sst->renderMode,temp);
        // turn on the old method
        fxHalIdleNoNop(sst);
	temp = GET(sstio->miscInit0) & ~SST_YORIGIN_TOP;
        temp |= (diago.ymaxscreen - 1) << SST_YORIGIN_TOP_SHIFT; 
        SET(sstio->miscInit0, temp);
        fxHalIdleNoNop(sst);
    }
    // else setup the new one
    else {
	FxU32 temp;
	temp = GET(sst->renderMode);
	temp &= ~SST_RM_YORIGIN_TOP;
	temp |= SST_RM_YORIGIN_SELECT;
        temp |= (diago.ymaxscreen - 1) << SST_RM_YORIGIN_TOP_SHIFT; 

	if (diago.rgb == 15) temp |= SST_RM_15BPP | SST_RM_ALPHA_MSB;
	if (diago.rgb == 16) temp |= SST_RM_16BPP;
	if (diago.rgb == 32) temp |= SST_RM_32BPP | SST_RM_ALPHA_WMASK |
		SST_RM_RED_WMASK | SST_RM_GREEN_WMASK | SST_RM_BLUE_WMASK;
        SET(sst->renderMode,temp);
    }

    // XXX hack GMT if bilinear is turned on for a 3D diag
    // then enable the 1 bit LSB tolerance if TMU revision 3
    if ((cpriv->info->tmuRevision == 3) && (diago.bilinear && !diago.gui)) {
        if (diago.halInfo->hsim)
            diago.halInfo->hsim |= BIT(13);     // enable LSB tolerance
        diago.halInfo->csim |= BIT(13); // enable LSB tolerance
    }
    if (diago.halInfo->csim & BIT(13)) {
        if (diago.trex) diago.halInfo->csim |= BIT(14);
        GDBG_PRINTF("WARNING: enabling %d bit LSB tolerance\n",diago.trex ? 4:2);
    }

    // enable triple buffering
    {
      SstIORegs *sstio;
      int tmp;
      sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
      tmp = GET(sstio->dramInit1);
      if (diago.triple) {                 // diag enables triple buffering
        tmp |= SST_TRIPLE_BUFFER_EN;
        SET(sstio->dramInit1,tmp);
      } 
      if ( tmp & SST_TRIPLE_BUFFER_EN )   // csim already configured for triple buffering
        diago.triple = 1;
    }

    // set this to maximum memory, then check memory fifo
    //Setup memory managemnt for framebuffer    
    newMemoryManager(cpriv->memorySizeInBytes, 1024 /*Block size*/, 1024 /*Max blockOwners*/);
    sst_idle_really(sst);

    if (cpriv->environment.recipFlag)
        gdbg_info(0,"WARNING: CSIM_RECIP is set\n");
    if (diago.trexInit0)
        SET(sst->trexInit0,diago.trexInit0);
    if (diago.trexInit1)
        SET(sst->trexInit1,diago.trexInit1);
    // XXX JIM: what is this for????
    if (diago.trexInit0 && diago.trexInit1) {
        int i;
        for (i=0; i<32; i++)
            SET(sst->trexInit1,diago.trexInit1 | (BIT(19)|BIT(20)));
        for (i=0; i<32; i++)
            SET(sst->trexInit1,diago.trexInit1 & ~(BIT(19)|BIT(20)));
    }
    // the following are set for compatibility with old diags, 
    // where the old CSIM set these registers, and some diags counted on it
    SET(sst->fbzMode, SST_RGBWRMASK);
    SET(sst->clipLeftRight, diago.xmaxscreen);
    SET(sst->clipBottomTop, diago.ymaxscreen);

    // if we are not simulating HW or running on HW then check every triangle
    if (!(diago.halInfo->hsim || diago.halInfo->hw)
	&& (diago.dontCheckEveryTriangle == FXFALSE)) {
	diago.checkEveryTriangle = 1;
    }

    if (diago.writeFifo) {
      PlacementStrategy cmdFifoPlacement;

        if (diago.directExec) {
            GDBG_ERROR("SST_BEGIN", "directExec (-e) mode not allowed for H3\n");
            diago.directExec = 0;
            DIAG_FAIL();
        }
        if (diago.disableHoles && diago.directExec) {
            gdbg_printf("WARNING: disableHoles (-h) not allowed in directExec (-e) mode\n");
            diago.disableHoles = 0;
        }

        // place FIFO right at the end of memory
        fifoSize = diago.ringSize * 4;
        if (fifoSize < 0) fifoSize = -fifoSize;

	if(diago.randomCmdFifoPlacement)
	  cmdFifoPlacement=randomBelowSixteenMegPlacement;
	else
	  cmdFifoPlacement=normalPlacement;  //This will place it at the bottom of memory


	//Don't let the command fifo's move around; force them to align on block boundaries
	currentMemoryManager->allowJiggle=FXFALSE;

	//Allocate the first cmdFifo with a little bit of slop
	diago.cmdFifoAddress[0]=(FxU32)allocate(fifoSize + CMD_FIFO_SLOP, "cmdFifo #0", cmdFifoPlacement);

	//Allocate the second cmdFifo if appropriate with a little bit of slop
	if (diago.whichFifo == 2)
	  diago.cmdFifoAddress[1]=(FxU32)allocate(fifoSize + CMD_FIFO_SLOP, "cmdFifo #1", cmdFifoPlacement);

	//Lock all the command FIFOs so they can't be unallocated
	lockByName("cmdFifo #");

	//Mark the end of the command fifos as the end of "trash" memory. This stupid-ass
	//Hack doesn't work when --randomPlacement or --randomCmdFifoPlacement is used.
	//This only affects diags that use these variables.
	diago.minTrashMem = getHighestAddress() + 1;

	//Let things jiggle away again
	currentMemoryManager->allowJiggle=FXTRUE;
	
        // delay actual initialization until later
    } else {
      int dummy;
      dummy = iRandom(1);    // keep random num generator in sync w & w/o CMDFIFO

      diago.minTrashMem=0;
    }

    // allocate frame buffers
    if (diago.gui)
      sstAlloc2DFrameBuffers(sst);
    else
      sstAlloc3DFrameBuffers(sst);

    //Print out where the buffers are
    memoryMap();

    if (!diago.gui && cpriv->windows) {
        // modify the back buffer window
	if (diago.rgb==15) cpriv->windows->pixFormat = SSTG_PIXFMT_15BPP;
	if (diago.rgb==16) cpriv->windows->pixFormat = SSTG_PIXFMT_16BPP;
	if (diago.rgb==32) cpriv->windows->pixFormat = SSTG_PIXFMT_32BPP;
        cpriv->windows->base = diagfb.colBufferAddr[1];
        cpriv->windows->stride = diagfb.colBufferStride[1];
        if ( (cpriv->windows->stride & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ) {
          cpriv->windows->tiled = 1;
          cpriv->windows->stride &= SST_BUFFER_TILE_STRIDE;
          cpriv->windows->endBase = tiledAddress(cpriv->windows->base,
						 cpriv->windows->stride,
						 2,
						 cpriv->windows->width+1,
						 cpriv->windows->height);
        } else {
          cpriv->windows->tiled = 0;
          cpriv->windows->stride &= SST_BUFFER_LINEAR_STRIDE;
          cpriv->windows->endBase = cpriv->windows->base + (cpriv->windows->height+1)*cpriv->windows->stride;
        }

	guiUpdateStatusBar(cpriv->windows);

        // create a front buffer window
        guiNewViewWindow(SST_FAKE_ADDRESS_GET_BOARD(sst),CSIM_VIEW_WINDOW_NAME);

	if (diago.rgb==15) cpriv->windows->pixFormat = SSTG_PIXFMT_15BPP;
	if (diago.rgb==16) cpriv->windows->pixFormat = SSTG_PIXFMT_16BPP;
	if (diago.rgb==32) cpriv->windows->pixFormat = SSTG_PIXFMT_32BPP;
        cpriv->windows->base = diagfb.colBufferAddr[0];
        cpriv->windows->stride = diagfb.colBufferStride[0];
        if ( (cpriv->windows->stride & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ) {
          cpriv->windows->tiled = 1;
          cpriv->windows->stride &= SST_BUFFER_TILE_STRIDE;
          cpriv->windows->endBase = tiledAddress(cpriv->windows->base,
						 cpriv->windows->stride,
						 2,
						 cpriv->windows->width-1,
						 cpriv->windows->height-1)+2;
        } else {
          cpriv->windows->tiled = 0;
          cpriv->windows->stride &= SST_BUFFER_LINEAR_STRIDE;
          cpriv->windows->endBase = cpriv->windows->base + (cpriv->windows->height+1)*cpriv->windows->stride;
        }
	guiUpdateStatusBar(cpriv->windows);

        if ( diago.triple ) {
          // create a triple buffer window
          guiNewViewWindow(SST_FAKE_ADDRESS_GET_BOARD(sst),CSIM_VIEW_WINDOW_NAME);
	  if (diago.rgb==15) cpriv->windows->pixFormat = SSTG_PIXFMT_15BPP;
	  if (diago.rgb==16) cpriv->windows->pixFormat = SSTG_PIXFMT_16BPP;
	  if (diago.rgb==32) cpriv->windows->pixFormat = SSTG_PIXFMT_32BPP;
          cpriv->windows->base = diagfb.colBufferAddr[2];
          cpriv->windows->stride = diagfb.colBufferStride[2];
          if ( (cpriv->windows->stride & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ) {
            cpriv->windows->tiled = 1;
            cpriv->windows->stride &= SST_BUFFER_TILE_STRIDE;
            cpriv->windows->endBase = tiledAddress(cpriv->windows->base,
						   cpriv->windows->stride,
						   2,
						   cpriv->windows->width-1,
						   cpriv->windows->height-1)+2;
          } else {
            cpriv->windows->tiled = 0;
            cpriv->windows->stride &= SST_BUFFER_LINEAR_STRIDE;
            cpriv->windows->endBase = cpriv->windows->base + (cpriv->windows->height+1)*cpriv->windows->stride;
          }
	  guiUpdateStatusBar(cpriv->windows);
        }

        // HACK: store this away so GUI interface can function in a reasonable manner
        cpriv->bufferAddrHack[0] = diagfb.colBufferAddr[0];
        cpriv->bufferStrideHack[0] = diagfb.colBufferStride[0];
        cpriv->bufferAddrHack[1] = diagfb.colBufferAddr[1];
        cpriv->bufferStrideHack[1] = diagfb.colBufferStride[1];
    }
    
    if (diago.imgFilename && !diago.vid) {
        sst_idle_really(sst);
        DIAG_LOADIMAGE(diago.imgFilename,CSIM_BUF_3D_FRONT,rgb_to_888);
    }

    AGPMEMINIT();

    // lastly, init the command fifo
    if (diago.writeFifo) {
      if (diago.agpEnable & 1 && !diago.disableHoles && (diago.whichFifo != 2)) {
        GDBG_ERROR("SST_INIT","Cmdfifo 0 AGP and hole counting are not supported. Disable hole counting.\n");
        DIAG_FAIL();
      }
      if (diago.agpEnable) {
        if (((fifoSize + 0xFFF) & ~0xFFF) *(diago.agpEnable == 3 ? 2 : 1)
            > (signed)cpriv->info->agpSizeInBytes) {
          GDBG_ERROR("SST_INIT","Fifo(s) (%d) too big to fit into AGP memory (%d).\n",
                     ((fifoSize + 0xFFF) & ~0xFFF) *(diago.agpEnable == 3 ? 2 : 1), cpriv->info->agpSizeInBytes);
          DIAG_FAIL();
        }
      }
      if (diago.whichFifo == 2) {
        hb_selectFifo(0);
        if (diago.agpEnable == 1) // 0 in AGP only
          fxHalInitCmdFifo(sst,0,diago.cmdFifoAddress[0],fifoSize,
                         diago.directExec,1,diago.agpEnable & 0x1);
        else
          fxHalInitCmdFifo(sst,0,diago.cmdFifoAddress[0],fifoSize,
                         diago.directExec,diago.disableHoles,diago.agpEnable & 0x1);
        
        fxHalInitCmdFifo(sst,1,diago.cmdFifoAddress[1],fifoSize,
                         // disable holes automatically for cmdfifo 1 if in AGP
                         diago.directExec,diago.disableHoles || diago.agpEnable & 0x2,
                         diago.agpEnable & 0x2);
      }
      else {
	if(diago.whichFifo < 0 || diago.whichFifo > 1)
	  {
	    GDBG_ERROR("SST_BEGIN", "Illegal -g value (whichFifo) of %d; forcing to 0 %s(%d)\n",
		       diago.whichFifo, __FILE__, __LINE__);
	    diago.whichFifo=0;
	  }

        hb_selectFifo(diago.whichFifo);
        fxHalInitCmdFifo(sst,diago.whichFifo,diago.cmdFifoAddress[diago.whichFifo],fifoSize,
                         diago.directExec,diago.disableHoles,diago.agpEnable);
      }
      diago.cmdFifoEnabled = 1;  // CMDFIFO now enabled
    }

    // Provide a default initialization for the all TMUs. Many old diags
    // do not use all TMUs. This initialization keeps unused TMUs mostly
    // quiet.
    {
      int i;
      for (i = 0; i < CSIM_DEFAULT_TMU_NUM; i++) {
	SET(SST_TREX(sst,i)->textureMode,SST_RGB332 | SST_TC_PASS | SST_TCA_PASS);
	SET(SST_TREX(sst,i)->tLOD, SST_TLOD_MINMAX_INT(8,8));
	SET(SST_TREX(sst,i)->texBaseAddr, diagfb.colBufferAddr[0]);
      }
    } 

#if !defined(CVG) && !defined(SST2)
    if ( diago.halInfo->hw && (! diago.gui) ) {
      // setup video for 3d diags
      int tiled = (diagfb.colBufferStride[0] & SST_BUFFER_MEMORY_TILED) ? 1 : 0; 
      int tstride = (diagfb.colBufferStride[0]&SST_BUFFER_TILE_STRIDE)>>SST_BUFFER_STRIDE_SHIFT;
      int stride = (diagfb.colBufferStride[0]&SST_BUFFER_LINEAR_STRIDE)>>SST_BUFFER_STRIDE_SHIFT;
      int i;
      FxU32 format, overlayStride;

      if(diago.rgb == 32)
	{
	  format = SST_OVERLAY_PIXEL_RGB32U;
	  overlayStride = diago.xmaxscreen * 4;
	}
      else if(diago.rgb == 16)
	{
	  if(diago.aaEnabled)
	    format = SST_OVERLAY_PIXEL_RGB565U;
	  else
	    format = SST_OVERLAY_PIXEL_RGB565D;
	  overlayStride = diago.xmaxscreen * 2;
	}
      else if(diago.rgb == 15)
	{
	  format = SST_OVERLAY_PIXEL_RGB1555U;
	  overlayStride = diago.xmaxscreen * 2;
	}
      else
	assert(0);

      sstInitVideoOverlay(sst,
			  1,				// 1=enable Overlay surface (OS), 1=disable
			  0,				// 1=enable OS stereo, 0=disable
			  0,				// 1=enable horizontal scaling, 0=disable
			  0,				// horizontal scale factor (ignored if not scaling)
			  0,				// 1=enable vertical scaling, 0=disable
			  0,				// vertical scale factor (ignored if not scaling)
			  0,				// filter mode
			  tiled,				// 0=OS linear, 1=tiled
			  format,	// pixel format of OS
			  0,				// bypass clut for OS?
			  0,				// 0=lower 256 CLUT entries, 1=upper 256
			  diagfb.colBufferAddr[0],	// board address of beginning of OS
			  tiled ? tstride : stride,	// distance between scanlines of the OS
			  overlayStride);               // overlay width in bytes
      
      // swap thru all active buffers to load vidCurrOverlayStartAddr for initial buffer
      i = diago.saveBeforeSwap;             // save saveBeforeSwap state
      diago.saveBeforeSwap = 0;             // force 0 to skip saving of images/memory

      DIAG_SWAPBUFFER_EX(1,0,0);            // swap
      DIAG_SWAPBUFFER_EX(1,0,0);      
      if (diago.triple)
	DIAG_SWAPBUFFER_EX(1,0,0);      

      diagSwaps = 0;                        // reset swap counter
      diago.saveBeforeSwap = i;             // restore saveBeforeSwap state
    }
#endif

    if(isTwoPixelsPerClockInhibited())
      diago.pixelsPerClock = 1;

    //Set default value of combineMode
    if(diago.pixelsPerClock == 2)
      {
	FxU32 bandHeight, renderMode;

	if(diago.log2BandHeight >= 0)
	  bandHeight = diago.log2BandHeight;
	else
	  bandHeight = iRandom(SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION >> SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION_SHIFT);
	
	if(bandHeight > (SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION >> SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION_SHIFT))
	  {
	    GDBG_ERROR("SST_BEGIN", "Illegal 2 pixel per clock band height\n");
	    DIAG_FAIL();
	  }

	renderMode = GET(sst->renderMode);
	renderMode &= ~SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION;
	renderMode |= bandHeight << SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION_SHIFT;
	SET_FBI(sst->renderMode, renderMode);
	SET_0(sst->renderMode, renderMode);
	SET_1(sst->renderMode, renderMode);

	//Set up for 2 pixels per clock
	SET_FBI(sst->combineMode, SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
	
	//Set up both TMUs for single texturing
	SET_0(sst->combineMode, SST_CM_TC_OTHERSELECT_LOCAL_TRGB |
	      SST_CM_TCA_OTHERSELECT_LOCAL_TA |
	      SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);	
	SET_0(sst->textureMode, SST_TC_ZERO_OTHER | SST_TCA_ZERO_OTHER);

	SET_1(sst->combineMode, SST_CM_TC_OTHERSELECT_LOCAL_TRGB |
	      SST_CM_TCA_OTHERSELECT_LOCAL_TA |
	      SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);	
	SET_1(sst->textureMode, SST_TC_ZERO_OTHER | SST_TCA_ZERO_OTHER);
      }
    else
      {
	//Set up for 1 pixel per clock
	SET_FBI(sst->combineMode, 0);

	//By default, set TMU 0 so that it's using TMU 1's output
	SET_0(sst->combineMode, SST_CM_TC_OTHERSELECT_OTHER_TRGB |
	      SST_CM_TCA_OTHERSELECT_OTHER_TA);

	SET_1(sst->combineMode, 0);
      }
    
    //Set up multichip junk
    sstSetupMultichip(sst);

    //Make sure we're not running with split, compressed textures
    if(diago.tsplit && diago.compressedTextures)
      {
	GDBG_INFO(0, "Warning! Disabling split textures because using compressed textures\n");
	diago.tsplit = 0;
      }

    //Initialize lfbMemoryTileCompare and lfbMemoryTileCtrl
    //Both of these registers are written through lfbMemoryConfig
    {
      SstIORegs *sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
      
      SET(sstio->lfbMemoryConfig, SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(0xffffffff));        //Just 0 out lfbMemoryTileCtrl
      SET(sstio->lfbMemoryConfig, SST_RAW_LFB_WRITE_CONTROL);  //Disable lfbMemoryTileCompare
    }
    
    setSeed(diago.seed);
    return sst;
}

void sstSetupMultichipConfigSpace(SstRegs *sst)
{
  SstIORegs *sstio;

  const PciRegister CFG_INIT_ENABLE = { 65, 3, READ_WRITE };
  const PciRegister CFG_PCI_DECODE  = { 72, 4, READ_WRITE };
  const PciRegister PCI_BASE_ADDRESS_0  = { 0x10, 4, READ_WRITE };
  const PciRegister PCI_BASE_ADDRESS_1  = { 0x14, 4, READ_WRITE };
  const PciRegister PCI_IO_BASE_ADDRESS = { 0x18, 4, READ_WRITE };
  
  FxU32 cfgInitEnable, cfgPciDecode;
  FxU32 pciInit0, tmuGbeInit;

  FxU32 parentMemBase0, parentMemBase1, parentIoBase;

  /*
  //Check to see if parent device is decoding 128MB space
  //If it is, then everything should be set up properly
  pciGetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[0].deviceNumber, &cfgPciDecode);
  if((cfgPciDecode & SST_PCI_MEMBASE0_DECODE) == SST_PCI_MEMBASE0_DECODE_128MB)
    {
      assert((cfgPciDecode & SST_PCI_MEMBASE1_DECODE) == SST_PCI_MEMBASE1_DECODE_128MB);
      assert((cfgPciDecode & SST_PCI_IOBASE0_DECODE) == SST_PCI_IOBASE0_DECODE_256);
      
      return;
    }
  */

  pciGetConfigData(PCI_BASE_ADDRESS_0, diago.halInfo->boardInfo[0].deviceNumber, &parentMemBase0);
  pciGetConfigData(PCI_BASE_ADDRESS_1, diago.halInfo->boardInfo[0].deviceNumber, &parentMemBase1);
  pciGetConfigData(PCI_IO_BASE_ADDRESS, diago.halInfo->boardInfo[0].deviceNumber, &parentIoBase);

  switch(diago.chipCount)
    {
    case 1:
      pciGetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[0].deviceNumber, &cfgInitEnable);
      GDBG_INFO(0, "Chip 0: CFG_INIT_ENABLE = 0x%08x\n", cfgInitEnable);      		       
      pciGetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[0].deviceNumber, &cfgPciDecode);
      GDBG_INFO(0, "Chip 0: CFG_PCI_DECODE = 0x%08x\n", cfgPciDecode);

      cfgInitEnable = 
	SST_ENABLE_HARDWARE_INIT_WRITES |
	SST_ENABLE_PCI_FIFO_WRITES;
      pciSetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[0].deviceNumber, &cfgInitEnable);

      cfgPciDecode = 
	SST_PCI_MEMBASE0_DECODE_128MB | 
	SST_PCI_MEMBASE1_DECODE_128MB |
	SST_PCI_IOBASE0_DECODE_256;
      pciSetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[0].deviceNumber, &cfgPciDecode);
      break;
      
    case 2:
      pciGetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[0].deviceNumber, &cfgInitEnable);
      GDBG_INFO(0, "Chip 0: CFG_INIT_ENABLE = 0x%08x\n", cfgInitEnable);      		       
      pciGetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[1].deviceNumber, &cfgInitEnable);
      GDBG_INFO(0, "Chip 1: CFG_INIT_ENABLE = 0x%08x\n", cfgInitEnable);      		       

      pciGetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[0].deviceNumber, &cfgPciDecode);
      GDBG_INFO(0, "Chip 0: CFG_PCI_DECODE = 0x%08x\n", cfgPciDecode);      		       
      pciGetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[1].deviceNumber, &cfgPciDecode);
      GDBG_INFO(0, "Chip 1: CFG_PCI_DECODE = 0x%08x\n", cfgPciDecode);      		       
      
      cfgInitEnable = 
	SST_ENABLE_HARDWARE_INIT_WRITES |
	SST_ENABLE_PCI_FIFO_WRITES |
	SST_ADDRESS_SNOOP_ENABLE |
	SST_SWAPBUFFER_ALGORITHM |
	SST_SWAP_MASTER;
      pciSetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[0].deviceNumber, &cfgInitEnable);

      cfgInitEnable = 
	SST_ENABLE_HARDWARE_INIT_WRITES |
	SST_ENABLE_PCI_FIFO_WRITES |
	SST_ADDRESS_SNOOP_ENABLE |
	SST_MEMBASE0_SNOOP_ENABLE |
	SST_MEMBASE1_SNOOP_ENABLE |
	SST_ADDRESS_SNOOP_SLAVE |
	(((parentMemBase0 >> (32-10)) << SST_MEMBASE0_SNOOP_SHIFT) & SST_MEMBASE0_SNOOP) |
	SST_SWAPBUFFER_ALGORITHM |
	SST_INIT_REGISTER_SNOOP_ENABLE;
      pciSetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[1].deviceNumber, &cfgInitEnable);

      // Each chip MUST must use 2ws reads and 1ws writes.  Snooping does not
      // work with 0ws writes or 1ws reads...
      // Also, each chip MUST use at least a retry interval of ~12...
      // (0x7 gets added to the timeout interval by the chip automatically...)
      sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sst));
      pciInit0 = GET(sstio->pciInit0);
      SET(sstio->pciInit0, (pciInit0 & ~SST_PCI_RETRY_INTERVAL) |
	  SST_PCI_READ_WS | SST_PCI_WRITE_WS | (5 << SST_PCI_RETRY_INTERVAL_SHIFT));

      tmuGbeInit = GET(sstio->tmuGbeInit);
      SET(sstio->tmuGbeInit, (tmuGbeInit & ~SST_AA_CLK_DELAY) |
	  (0x2 << SST_AA_CLK_DELAY_SHIFT) | SST_AA_CLK_INVERT);

      cfgPciDecode = 
	SST_PCI_MEMBASE0_DECODE_128MB | 
	SST_PCI_MEMBASE1_DECODE_128MB |
	SST_PCI_IOBASE0_DECODE_256 |
	SST_SNOOP_MEMBASE0_DECODE_128MB |
	SST_SNOOP_MEMBASE1_DECODE_128MB;      
      pciSetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[0].deviceNumber, &cfgPciDecode);

      cfgPciDecode = 
	SST_PCI_MEMBASE0_DECODE_128MB | 
	SST_PCI_MEMBASE1_DECODE_128MB |
	SST_PCI_IOBASE0_DECODE_256 |
	SST_SNOOP_MEMBASE0_DECODE_128MB |
	SST_SNOOP_MEMBASE1_DECODE_128MB |
	(((parentMemBase1 >> (32 - 10))<< SST_MEMBASE1_SNOOP_SHIFT) & SST_MEMBASE1_SNOOP);
      pciSetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[1].deviceNumber, &cfgPciDecode);

      pciGetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[0].deviceNumber, &cfgInitEnable);
      GDBG_INFO(0, "Chip 0: CFG_INIT_ENABLE = 0x%08x\n", cfgInitEnable);      		       
      pciGetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[1].deviceNumber, &cfgInitEnable);
      GDBG_INFO(0, "Chip 1: CFG_INIT_ENABLE = 0x%08x\n", cfgInitEnable);      		       

      pciGetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[0].deviceNumber, &cfgPciDecode);
      GDBG_INFO(0, "Chip 0: CFG_PCI_DECODE = 0x%08x\n", cfgPciDecode);      		       
      pciGetConfigData(CFG_PCI_DECODE, diago.halInfo->boardInfo[1].deviceNumber, &cfgPciDecode);
      GDBG_INFO(0, "Chip 1: CFG_PCI_DECODE = 0x%08x\n", cfgPciDecode);      		       
      break;
      
    case 4:
      assert("Not implemented yet" && 0);
      break;
      
    default:
      assert(0);
    }
}

void sstSetupMultichip(SstRegs *sst)
{  
  FxI32 i;
  FxI32 chipIndex;
  FxU32 renderMask, compareMask, scanMask;
  FxU32 renderMaskArray[4], compareMaskArray[4];

  //non-sli, aa case
  PciRegister cfgVideoCtrl0 = {128, 4, READ_WRITE};
  PciRegister cfgVideoCtrl1 = {132, 4, READ_WRITE};
  PciRegister cfgVideoCtrl2 = {136, 4, READ_WRITE};
  const PciRegister CFG_SLI_AA_MISC = {172, 4, READ_WRITE};

  //Should set it up so that the multi-chip lfb junk is properly configured

  //Anti-aliasing default perturbation values
  static FxU32 defaultXOffset[4] = {0x7a, 0x2, 0x7c, 0x4};
  static FxU32 defaultYOffset[4] = {0x7b, 0x4, 0x3, 0x7d};
  //static FxU32 defaultXOffset[4] = {0,0,0,0};
  //static FxU32 defaultYOffset[4] = {0,0,0,0};
  //static FxU32 defaultXOffset[4] = {0x0, 0x3F, 0x3F, 0x0};
  //static FxU32 defaultYOffset[4] = {0x0, 0x0, 0x3F, 0x3F};

  //Download the AA/SLI lfb config space defaults
  for(chipIndex=0; (FxU32)chipIndex<diago.halInfo->boardsFound; chipIndex++)
    {
      halCfgStore32(0x3FC, 1, diago.halInfo->boardInfo[chipIndex].pciBusNumber,
		    diago.halInfo->boardInfo[chipIndex].pciDeviceNumber,
		    diago.halInfo->boardInfo[chipIndex].pciFunctionNumber,
		    offsetof(SstPCIConfigRegs, cfgSliLfbCtrl),
		    SST_SLI_LFB_CPU_WRITE_ENABLE |
		    SST_SLI_LFB_DISPATCH_WRITE_ENABLE |
		    SST_SLI_LFB_READ_ENABLE);		    
      halCfgStore32(0x3FC, 1, diago.halInfo->boardInfo[chipIndex].pciBusNumber,
		    diago.halInfo->boardInfo[chipIndex].pciDeviceNumber,
		    diago.halInfo->boardInfo[chipIndex].pciFunctionNumber,
		    offsetof(SstPCIConfigRegs, cfgAADepthBufferAperture),
		    0);
      halCfgStore32(0x3FC, 1, diago.halInfo->boardInfo[chipIndex].pciBusNumber,
		    diago.halInfo->boardInfo[chipIndex].pciDeviceNumber,
		    diago.halInfo->boardInfo[chipIndex].pciFunctionNumber,
		    offsetof(SstPCIConfigRegs, cfgAALfbCtrl),
		    SST_AA_LFB_CPU_WRITE_ENABLE |
		    SST_AA_LFB_DISPATCH_WRITE_ENABLE |
		    ((diago.aaSampleCount == 4) ? SST_AA_LFB_RD_DIVIDE_BY_FOUR : 0) |
		    SST_AA_LFB_READ_ENABLE);
    }

  if(diago.chipCount > 1)
    {
      FxI32 chipIndex, log2ChipCount;
      FxU32 log2BandHeight;
	
      //Make sure the arguments are acceptable
      if(!((diago.chipCount == 2) || (diago.chipCount == 4)))	   
	{
	  GDBG_ERROR("sstSetupMultichip", "--chipCount must equal 1,2, or 4\n");
	  DIAG_FAIL();
	}
      
      if(diago.chipCount == 2)
	{
	  if(diago.aaEnabled && diago.sliEnabled && (diago.aaSampleCount == 4))
	    {
	      GDBG_ERROR("sstSetupMultichip", "Can't enable 4 sample AA and SLI with 2 chips!\n");
	      DIAG_FAIL();
	    }
	}

      if(!diago.sliEnabled && !diago.aaEnabled)
	{
	  GDBG_ERROR("sstSetupMultichip", "Need to enable AA or SLI for multichip!\n");
	  DIAG_FAIL();
	}

      if(diago.chipCount > 2 && (!diago.sliEnabled))
	{
	  GDBG_ERROR("sstSetupMultichip", "Can't run AA on more than 2 chips without SLI!\n");
	  DIAG_FAIL();
	}

      if((diago.chipCount == 2) && (!diago.sliEnabled) && (diago.aaEnabled) &&
	 (diago.aaSampleCount == 2))
	{
	  GDBG_ERROR("sstSetupMultichip", "Can't run 2 sample AA without SLI with 2 chips!\n");
	  DIAG_FAIL();
	}
      

      if(diago.aaEnabled && diago.sliEnabled &&
	 (diago.chipCount == 2) && (diago.aaSampleCount==4))	
	{
	  GDBG_ERROR("sstSetupMultichip", "Can't run 4 sample AA and SLI with 2 chips!\n");
	  DIAG_FAIL();
	}
	
      switch(diago.sliBandHeight)
	{
	case 1:
	  log2BandHeight=0;
	  break;
	case 2:
	  log2BandHeight=1;
	  break;
	case 4:
	  log2BandHeight=2;
	  break;
	case 8:
	  log2BandHeight=3;
	  break;
	case 16:
	  log2BandHeight=4;
	  break;
	case 32:
	  log2BandHeight=5;
	  break;
	case 64:
	  log2BandHeight=6;
	  break;
	case 128:
	  log2BandHeight=7;
	  break;	    
	default:
	  GDBG_INFO(1, "--sliBandHeight must equal 1,2,4,8,16,32,64 or 128\n");
	  DIAG_FAIL();	    
	}

      GDBG_INFO(1, "%d-way Scan Line Interleave enabled (sliBandHeight = %d)\n", diago.chipCount,
		diago.sliBandHeight);

	
      //Make sure the cprivate shit is up to date
      CSIM_PRIVATE(diago.sstCSIM)->environment.aaEnabled = diago.aaEnabled;
      CSIM_PRIVATE(diago.sstCSIM)->environment.aaSampleCount = diago.aaSampleCount;
      CSIM_PRIVATE(diago.sstCSIM)->environment.sliEnabled = diago.sliEnabled;
      CSIM_PRIVATE(diago.sstCSIM)->environment.sliBandHeight = diago.sliBandHeight;
      for(i=1; i<diago.chipCount; i++)
	{
	  CSIM_PRIVATE(diago.sstChildrenCSIM[i-1])->environment.aaEnabled = diago.aaEnabled;
	  CSIM_PRIVATE(diago.sstChildrenCSIM[i-1])->environment.aaSampleCount = diago.aaSampleCount;
	  CSIM_PRIVATE(diago.sstChildrenCSIM[i-1])->environment.sliEnabled = diago.sliEnabled;
	  CSIM_PRIVATE(diago.sstChildrenCSIM[i-1])->environment.sliBandHeight = diago.sliBandHeight;
	}

      if(diago.aaEnabled)
	{	  
	  for(chipIndex=0; chipIndex<diago.chipCount; chipIndex++)
	    {
	      FxU32 aaCtrl;
	      
	      //Write each chip's aaCtrl register
	      SET(sst->chipMask, 1<<chipIndex);

	      aaCtrl = (defaultXOffset[(chipIndex * 2)%4] << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT) |
		(defaultYOffset[(chipIndex * 2)%4] << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT) |
		(defaultXOffset[(chipIndex * 2 + 1)%4] << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT) |
		(defaultYOffset[(chipIndex * 2 + 1)%4] << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT) |
		SST_AA_CONTROL_AA_ENABLE;
	      
	      GDBG_INFO(0, "chip %d primary: xOffset=0x%x yOffset=0x%x\n", chipIndex, 
			defaultXOffset[(chipIndex * 2)%4],
			defaultYOffset[(chipIndex * 2)%4]);
	      GDBG_INFO(0, "chip %d secondary: xOffset=0x%x yOffset=0x%x\n", chipIndex,
			defaultXOffset[(chipIndex * 2 + 1)%4],
			defaultYOffset[(chipIndex * 2 + 1)%4]);

	      SET(sst->aaCtrl, aaCtrl);
	    }
	  //Re-enable all the chips
	  SET(sst->chipMask, SST_CHIP_MASK_ALL_CHIPS);
	}
      else
	SET(sst->aaCtrl, 0);
      
      if(diago.sliEnabled)
	{
	  FxI32 sliChipCountDivisor;
	  
	  if(diago.aaEnabled && diago.aaSampleCount==4)
	    sliChipCountDivisor = 2;
	  else
	    sliChipCountDivisor = 1;
	  assert(diago.chipCount / sliChipCountDivisor > 1);

	  //Now we need to configure sli (via the sliCtrl register)
	  renderMask = (diago.chipCount / sliChipCountDivisor - 1) << log2BandHeight;
	  scanMask = diago.sliBandHeight - 1;
	    
	  log2ChipCount = 0;
	  while((1<<log2ChipCount) != (diago.chipCount / sliChipCountDivisor))
	    log2ChipCount++;
	    
	  for(chipIndex=0; chipIndex<diago.chipCount; chipIndex++)
	    {
	      compareMask = (chipIndex / sliChipCountDivisor) << log2BandHeight;
		
	      //Write each chip's sliCtrl register
	      SET(sst->chipMask, 1<<chipIndex);
		
	      renderMaskArray[chipIndex] = renderMask;
	      compareMaskArray[chipIndex] = compareMask;
	      SET(sst->sliCtrl, (renderMask << SST_SLI_CONTROL_RENDER_MASK_SHIFT) |
		  (compareMask << SST_SLI_CONTROL_COMPARE_MASK_SHIFT) |
		  (scanMask << SST_SLI_CONTROL_SCAN_MASK_SHIFT) |
		  (log2ChipCount << SST_SLI_CONTROL_LOG2_CHIP_COUNT_SHIFT) |
		  SST_SLI_CONTROL_SLI_ENABLE);
	    }

	  //Re-enable all the chips
	  SET(sst->chipMask, SST_CHIP_MASK_ALL_CHIPS);
	}
      else
	SET(sst->sliCtrl, 0);
    }
  else
    { 
      // Single chip configuration
      if(diago.chipCount != 1)
	{
	  GDBG_ERROR("sstSetupMultichip", "Illegal chipCount=%d\n", diago.chipCount);
	  DIAG_FAIL();
	}
	
      if(diago.sliEnabled)
	{
	  GDBG_ERROR("sstSetupMultichip", "Can't enable SLI with only one chip!\n");
	  DIAG_FAIL();
	}

      if(diago.aaEnabled && diago.aaSampleCount==4)
	{
	  GDBG_ERROR("sstSetupMultichip", "Can't run 4 sample AA with 1 chip!\n");
	  DIAG_FAIL();
	}
	
      //Make sure that sli is disabled
      SET(sst->sliCtrl, 0);

      if(diago.aaEnabled)
	{
	  FxU32 aaCtrl;
	  
	  aaCtrl = (defaultXOffset[0] << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT) |
	    (defaultYOffset[0] << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT) |
	    (defaultXOffset[1] << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT) |
	    (defaultYOffset[1] << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT) |
	    SST_AA_CONTROL_AA_ENABLE;
	      
	  SET(sst->aaCtrl, aaCtrl);
	}
      else
	SET(sst->aaCtrl, 0);
      
      CSIM_PRIVATE(diago.sstCSIM)->environment.aaEnabled = diago.aaEnabled;
      CSIM_PRIVATE(diago.sstCSIM)->environment.aaSampleCount = diago.aaSampleCount;
      CSIM_PRIVATE(diago.sstCSIM)->environment.sliEnabled = FXFALSE;
      CSIM_PRIVATE(diago.sstCSIM)->environment.sliBandHeight = 1;

      assert(diago.chipCount == 1);
      assert(diago.sliBandHeight == 1);
    }

  //////////////////////////////////////////////////////////////////////
  //
  //                     Setup the g'damn video
  //
  //////////////////////////////////////////////////////////////////////
  if(diago.sliEnabled)
    {
      //sli case
      if(diago.aaEnabled)
	{
	  if(diago.chipCount == 2)
	    {
	      //2 chip, 2 way SLI, 2 sample AA
	      FxU32 vidScreenSize, vidProcCfg;
	      FxU32 data;
	      SstIORegs *sstio;
	      
	      //Setup parent chip
	      sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sst));    
	      vidScreenSize = GET(sstio->vidScreenSize);	      
	      SET(sstio->vidScreenSize, vidScreenSize | SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE);
	      
	      //Make sure that video section is fetching from overlay and desktop
	      vidProcCfg = GET(sstio->vidProcCfg);
	      vidProcCfg |= SST_OVERLAY_EN | SST_CHROMA_EN | SST_DESKTOP_EN;
	      SET(sstio->vidProcCfg, vidProcCfg);

	      pciGetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      data = SST_CFG_ENHANCED_VIDEO_EN | 
		SST_CFG_DIVIDE_VIDEO_BY_2 | 
		SST_CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
		(SST_CFG_VIDEO_OTHERMUX_SEL_AAFIFO << SST_CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
		(SST_CFG_VIDEO_OTHERMUX_SEL_PIPE << SST_CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT);
	      pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      
	      data = 
		(renderMaskArray[0] << SST_CFG_SLI_RENDERMASK_FETCH_SHIFT) |
		(compareMaskArray[0] << SST_CFG_SLI_COMPAREMASK_FETCH_SHIFT);
	      pciSetConfigData(cfgVideoCtrl1, diago.halInfo->boardInfo[0].deviceNumber, &data);

	      data = 
		(renderMaskArray[1] << SST_CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
		(compareMaskArray[1] << SST_CFG_SLI_COMPAREMASK_AAFIFO_SHIFT);
	      pciSetConfigData(cfgVideoCtrl2, diago.halInfo->boardInfo[0].deviceNumber, &data);

	      //setup misc shit
	      data = 0x800;
	      pciSetConfigData(CFG_SLI_AA_MISC, diago.halInfo->boardInfo[0].deviceNumber, &data);

	      //
	      //Setup child chip
	      sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sstChildren[0]));    
	      vidScreenSize = GET(sstio->vidScreenSize);	      
	      SET(sstio->vidScreenSize, vidScreenSize | SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE);
	      
	      //Make sure that video section is fetching from the overlay and desktop
	      vidProcCfg = GET(sstio->vidProcCfg);
	      vidProcCfg |= SST_OVERLAY_EN | SST_CHROMA_EN | SST_DESKTOP_EN;
	      SET(sstio->vidProcCfg, vidProcCfg);

	      pciGetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[1].deviceNumber, &data);
	      data = SST_CFG_ENHANCED_VIDEO_EN | 
		SST_CFG_ENHANCED_VIDEO_SLV |
		SST_CFG_DIVIDE_VIDEO_BY_1 | 
		SST_CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY | 
		(SST_CFG_VIDEO_OTHERMUX_SEL_PIPE << SST_CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
		(SST_CFG_VIDEO_OTHERMUX_SEL_PIPE << SST_CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT);
	      pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[1].deviceNumber, &data);
	      
	      data = 
		(renderMaskArray[1] << SST_CFG_SLI_RENDERMASK_FETCH_SHIFT) |
		(compareMaskArray[1] << SST_CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
		(0xff << SST_CFG_SLI_COMPAREMASK_CRT_SHIFT);
	      pciSetConfigData(cfgVideoCtrl1, diago.halInfo->boardInfo[1].deviceNumber, &data);

	      data = 
		(renderMaskArray[1] << SST_CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
		(compareMaskArray[1] << SST_CFG_SLI_COMPAREMASK_AAFIFO_SHIFT);
	      pciSetConfigData(cfgVideoCtrl2, diago.halInfo->boardInfo[1].deviceNumber, &data);

	      //setup misc shit
	      data = 0x82f;
	      pciSetConfigData(CFG_SLI_AA_MISC, diago.halInfo->boardInfo[1].deviceNumber, &data);
	    }
	  else
	    {
	      //4 chip, 2 way SLI, 4 sample AA
	    }
	}
      else
	{
	  //sli only
	  
	  if(diago.chipCount == 2)
	    {
	      //2 way sli, no AA
	      //non-sli, 4 sample aa case (2 chips)
	      FxU32 vidScreenSize, vidProcCfg;
	      FxU32 data;
	      SstIORegs *sstio;
	      
	      //Setup parent chip
	      sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sst));    
	      vidScreenSize = GET(sstio->vidScreenSize);	      
	      SET(sstio->vidScreenSize, vidScreenSize & ~SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE);
	      
	      //Make sure that video section is fetching from overlay
	      vidProcCfg = GET(sstio->vidProcCfg);
	      vidProcCfg |= SST_OVERLAY_EN | SST_CHROMA_EN;
	      vidProcCfg &= ~SST_DESKTOP_EN;
	      SET(sstio->vidProcCfg, vidProcCfg);

	      pciGetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      data = SST_CFG_ENHANCED_VIDEO_EN | 
		SST_CFG_DIVIDE_VIDEO_BY_1 | 
		(SST_CFG_VIDEO_OTHERMUX_SEL_AAFIFO << SST_CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
		(SST_CFG_VIDEO_OTHERMUX_SEL_PIPE << SST_CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT);
	      pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      
	      data = 
		(renderMaskArray[0] << SST_CFG_SLI_RENDERMASK_FETCH_SHIFT) |
		(compareMaskArray[0] << SST_CFG_SLI_COMPAREMASK_FETCH_SHIFT);
	      pciSetConfigData(cfgVideoCtrl1, diago.halInfo->boardInfo[0].deviceNumber, &data);

	      data = 
		(renderMaskArray[1] << SST_CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
		(compareMaskArray[1] << SST_CFG_SLI_COMPAREMASK_AAFIFO_SHIFT);
	      pciSetConfigData(cfgVideoCtrl2, diago.halInfo->boardInfo[0].deviceNumber, &data);

	      //setup misc shit
	      data = 0x800;
	      pciSetConfigData(CFG_SLI_AA_MISC, diago.halInfo->boardInfo[0].deviceNumber, &data);

	      //
	      //Setup child chip
	      sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sstChildren[0]));    
	      vidScreenSize = GET(sstio->vidScreenSize);	      
	      SET(sstio->vidScreenSize, vidScreenSize & ~SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE);
	      
	      //Make sure that video section is fetching from the overlay
	      vidProcCfg = GET(sstio->vidProcCfg);
	      vidProcCfg |= SST_OVERLAY_EN | SST_CHROMA_EN;
	      vidProcCfg &= ~SST_DESKTOP_EN;
	      SET(sstio->vidProcCfg, vidProcCfg);

	      pciGetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[1].deviceNumber, &data);
	      data = SST_CFG_ENHANCED_VIDEO_EN | 
		SST_CFG_ENHANCED_VIDEO_SLV |
		SST_CFG_DIVIDE_VIDEO_BY_1 | 
		(SST_CFG_VIDEO_OTHERMUX_SEL_PIPE << SST_CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
		(SST_CFG_VIDEO_OTHERMUX_SEL_PIPE << SST_CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT);
	      pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[1].deviceNumber, &data);
	      
	      data = 
		(renderMaskArray[1] << SST_CFG_SLI_RENDERMASK_FETCH_SHIFT) |
		(compareMaskArray[1] << SST_CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
		(0xff << SST_CFG_SLI_COMPAREMASK_CRT_SHIFT);
	      pciSetConfigData(cfgVideoCtrl1, diago.halInfo->boardInfo[1].deviceNumber, &data);

	      data = 
		(renderMaskArray[1] << SST_CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
		(compareMaskArray[1] << SST_CFG_SLI_COMPAREMASK_AAFIFO_SHIFT);
	      pciSetConfigData(cfgVideoCtrl2, diago.halInfo->boardInfo[1].deviceNumber, &data);

	      //setup misc shit
	      data = 0x82f;
	      pciSetConfigData(CFG_SLI_AA_MISC, diago.halInfo->boardInfo[1].deviceNumber, &data);
	    }
	  else
	    {
	      //4 way sli, no AA
	      assert(0);
	    }
	}
    }
  else
    {
      //non-sli case
      if(diago.aaEnabled)
	{
	  //non-sli, aa case (single chip)
	  if(diago.aaSampleCount == 2)
	    {
	      //non-sli, 2 sample aa case
	      FxU32 vidScreenSize, vidProcCfg;
	      FxU32 data;
	      SstIORegs *sstio;

	      //Setup parent chip
	      //Set up video section to swap desktop as well as overlay		  
	      sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sst));    
	      vidScreenSize = GET(sstio->vidScreenSize);	      
	      SET(sstio->vidScreenSize, vidScreenSize | SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE);

	      //Make sure that video section is fetching from desktop and overlay
	      vidProcCfg = GET(sstio->vidProcCfg);
	      vidProcCfg |= SST_DESKTOP_EN | SST_OVERLAY_EN | SST_CHROMA_EN;
	      SET(sstio->vidProcCfg, vidProcCfg);

	      pciGetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      data |= SST_CFG_ENHANCED_VIDEO_EN | SST_CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
		SST_CFG_DIVIDE_VIDEO_BY_2;
	      pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      
	      data = 0;
	      pciSetConfigData(cfgVideoCtrl1, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      pciSetConfigData(cfgVideoCtrl2, diago.halInfo->boardInfo[0].deviceNumber, &data);
	    }	      
	  else
	    {
	      //non-sli, 4 sample aa case (2 chips)
	      FxU32 vidScreenSize, vidProcCfg;
	      FxU32 data;
	      SstIORegs *sstio;
	      
	      //Setup parent chip
	      //Set up video section to swap desktop as well as overlay		  
	      sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sst));    
	      vidScreenSize = GET(sstio->vidScreenSize);	      
	      SET(sstio->vidScreenSize, vidScreenSize | SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE);
	      
	      //Make sure that video section is fetching from desktop and overlay
	      vidProcCfg = GET(sstio->vidProcCfg);
	      vidProcCfg |= SST_DESKTOP_EN | SST_OVERLAY_EN | SST_CHROMA_EN;
	      SET(sstio->vidProcCfg, vidProcCfg);

	      pciGetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      data = SST_CFG_ENHANCED_VIDEO_EN | SST_CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
		SST_CFG_DIVIDE_VIDEO_BY_4 | 
		(SST_CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << SST_CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT);
	      pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      
	      data = 0;
	      pciSetConfigData(cfgVideoCtrl1, diago.halInfo->boardInfo[0].deviceNumber, &data);
	      pciSetConfigData(cfgVideoCtrl2, diago.halInfo->boardInfo[0].deviceNumber, &data);

	      //setup misc shit
	      data = 0x800;
	      pciSetConfigData(CFG_SLI_AA_MISC, diago.halInfo->boardInfo[0].deviceNumber, &data);

	      //
	      //Setup child chip
	      //Set up video section to swap desktop as well as overlay		  
	      sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sstChildren[0]));    
	      vidScreenSize = GET(sstio->vidScreenSize);	      
	      SET(sstio->vidScreenSize, vidScreenSize | SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE);
	      
	      //Make sure that video section is fetching from desktop and overlay
	      vidProcCfg = GET(sstio->vidProcCfg);
	      vidProcCfg |= SST_DESKTOP_EN | SST_OVERLAY_EN | SST_CHROMA_EN;
	      SET(sstio->vidProcCfg, vidProcCfg);

	      pciGetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[1].deviceNumber, &data);
	      data = SST_CFG_ENHANCED_VIDEO_EN | SST_CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
		SST_CFG_ENHANCED_VIDEO_SLV |
		SST_CFG_DIVIDE_VIDEO_BY_1 |
		SST_CFG_VIDPLL_SEL;
	      pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[1].deviceNumber, &data);
	      
	      data = (0xff << SST_CFG_SLI_COMPAREMASK_CRT_SHIFT);
	      pciSetConfigData(cfgVideoCtrl1, diago.halInfo->boardInfo[1].deviceNumber, &data);

	      data = 0;
	      pciSetConfigData(cfgVideoCtrl2, diago.halInfo->boardInfo[1].deviceNumber, &data);

	      //setup misc shit
	      data = 0x82f;
	      pciSetConfigData(CFG_SLI_AA_MISC, diago.halInfo->boardInfo[1].deviceNumber, &data);
	    }
	}
      else
	{
	  //non-sli, non-aa case
	  //Base video, don't setup extra shit
	  SstIORegs *sstio;
	  FxU32 vidScreenSize;
	  FxU32 data;
	  
	  sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sst));    
	  vidScreenSize = GET(sstio->vidScreenSize);	      
	  SET(sstio->vidScreenSize, vidScreenSize & (~SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE));
	  
	  data = 0;
	  pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[0].deviceNumber, &data);
	}
    }
}

#ifndef CVG
//
// configure overlay surface
//
void sstInitVideoOverlay(
    SstRegs *sst,     
    FxU32 enable,		// 1=enable Overlay surface (OS), 1=disable
    FxU32 stereo,		// 1=enable OS stereo, 0=disable
    FxU32 horizScaling,		// 1=enable horizontal scaling, 0=disable
    FxU32 dudx,			// horizontal scale factor (ignored if not scaling)
    FxU32 verticalScaling,	// 1=enable vertical scaling, 0=disable
    FxU32 dvdy,			// vertical scale factor (ignored if not scaling)
    FxU32 filterMode,		// duh
    FxU32 tiled,		// 0=OS linear, 1=tiled
    FxU32 pixFmt,		// pixel format of OS
    FxU32 clutBypass,		// bypass clut for OS?
    FxU32 clutSelect,		// 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,		// board address of beginning of OS
    FxU32 stride,		// distance between scanlines of the OS, in
				// units of bytes for linear OS's and tiles for
				// tiled OS's
    FxU32 width)                // overlay width in bytes
{
  SstIORegs *sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
  FxU32 vidProcCfg = GET(sstio->vidProcCfg);
  FxU32 doStride;
  FxU32 fwidth;

  vidProcCfg &= ~(SST_OVERLAY_TILED_EN |
		  SST_OVERLAY_STEREO_EN |  
		  SST_OVERLAY_HORIZ_SCALE_EN |
		  SST_OVERLAY_VERT_SCALE_EN |
		  SST_OVERLAY_TILED_EN |
		  SST_OVERLAY_PIXEL_FORMAT |
		  SST_OVERLAY_CLUT_BYPASS |
		  SST_OVERLAY_CLUT_SELECT);

  if (enable)
    vidProcCfg |= SST_OVERLAY_EN;
  
  if (stereo)
    vidProcCfg |= SST_OVERLAY_STEREO_EN;
  
  if (horizScaling)
    vidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;
  
  if (verticalScaling)
    vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;
  
  if (tiled)
    vidProcCfg |= SST_OVERLAY_TILED_EN;
  
  vidProcCfg |= pixFmt;
  
  if (clutBypass)
    vidProcCfg |= SST_OVERLAY_CLUT_BYPASS;
  
  if (clutSelect)
    vidProcCfg |= SST_OVERLAY_CLUT_SELECT;

  //Need to make sure the desktop and overlay
  //have the same pixel format
  if(diago.aaEnabled)
    {
      switch(pixFmt)
	{
	case SST_OVERLAY_PIXEL_RGB1555U:
	  vidProcCfg |= SST_DESKTOP_PIXEL_RGB1555U;
	  break;
 	case SST_OVERLAY_PIXEL_RGB565U:
	  vidProcCfg |= SST_DESKTOP_PIXEL_RGB565;
	  break;
 	case SST_OVERLAY_PIXEL_RGB32U:
	  vidProcCfg |= SST_DESKTOP_PIXEL_RGB32;
	  break;
	default:
	  assert(0);
	}
    }
  
  SET(sstio->vidProcCfg, vidProcCfg);
  
  // change only the overlay portion of the vidDesktopOverlayStride register
  //
  doStride = GET(sstio->vidDesktopOverlayStride);
  doStride &= ~(SST_OVERLAY_LINEAR_STRIDE | SST_OVERLAY_TILE_STRIDE);

  if (tiled)
    doStride |= ((stride << SST_OVERLAY_STRIDE_SHIFT) & SST_OVERLAY_TILE_STRIDE);
  else
    doStride |= ((stride << SST_OVERLAY_STRIDE_SHIFT) & SST_OVERLAY_LINEAR_STRIDE);

  if(diago.aaEnabled)
    {
      doStride &= ~(SST_DESKTOP_LINEAR_STRIDE | SST_DESKTOP_TILE_STRIDE);
      if (tiled)
	doStride |= ((stride << SST_DESKTOP_STRIDE_SHIFT) & SST_DESKTOP_TILE_STRIDE);
      else
	doStride |= ((stride << SST_DESKTOP_STRIDE_SHIFT) & SST_DESKTOP_LINEAR_STRIDE);
    }
  
  SET(sstio->vidDesktopOverlayStride, doStride);
  
  // set the overlay fetch width
  fwidth = GET(sstio->vidOverlayDudxOffsetSrcWidth);
  fwidth &= ~SST_OVERLAY_FETCH_SIZE;
  fwidth |= width<<SST_OVERLAY_FETCH_SIZE_SHIFT;
  SET(sstio->vidOverlayDudxOffsetSrcWidth,fwidth);

  // set the overlay start address
  SET(sst->leftOverlayBuf,startAddress);
}

// 
// configure desktop surface
//
void
sstInitVideoDesktop(
    SstRegs *sst,
    FxU32 enable,		// 1=enable desktop surface (DS), 1=disable
    FxU32 tiled,		// 0=DS linear, 1=tiled
    FxU32 pixFmt,		// pixel format of DS
    FxU32 clutBypass,		// bypass clut for DS?
    FxU32 clutSelect,		// 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,		// board address of beginning of DS
    FxU32 stride)		// distance between scanlines of the DS, in
				// units of bytes for linear DS's and tiles for
				// tiled DS's
{
  SstIORegs *sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
  FxU32 vidProcCfg = GET(sstio->vidProcCfg);
  FxU32 doStride;

  vidProcCfg &= ~ (SST_DESKTOP_EN |
		   SST_DESKTOP_TILED_EN |
		   SST_DESKTOP_PIXEL_FORMAT |
		   SST_DESKTOP_CLUT_BYPASS |
		   SST_DESKTOP_CLUT_SELECT );
  if (enable)
    vidProcCfg |= SST_DESKTOP_EN;
  
  if (tiled)
    vidProcCfg |= SST_DESKTOP_TILED_EN;
  
  vidProcCfg |= pixFmt;
  
  if (clutBypass)
    vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;
  
  if (clutSelect)
    vidProcCfg |= SST_DESKTOP_CLUT_SELECT;
  
  SET(sstio->vidProcCfg, vidProcCfg);
  
  SET(sstio->vidDesktopStartAddr, 
	 (startAddress & SST_VIDEO_START_ADDR) << SST_VIDEO_START_ADDR_SHIFT);

  // change only the desktop portion of the vidDesktopOverlayStride register
  //
  doStride = GET(sstio->vidDesktopOverlayStride);
  doStride &= ~(SST_DESKTOP_LINEAR_STRIDE | SST_DESKTOP_TILE_STRIDE);
  stride <<= SST_DESKTOP_STRIDE_SHIFT;
  if (tiled)
    stride &= SST_DESKTOP_TILE_STRIDE;
  else
    stride &= SST_DESKTOP_LINEAR_STRIDE;
  doStride |=	stride;
  
  SET(sstio->vidDesktopOverlayStride, doStride);
}
#endif


//
// allocate front/back/aux/triple buffers
//
#define CEIL(x,y)      ( ((x)+(y)-1) / (y) )  // divide x/y, rounding up
#define MAX_TILE_STRIDE_SLOP     2  // stride will be a max of 2 tiles larger than reqd
#define MAX_LINEAR_STRIDE_SLOP  64  // stride will be a max of 64 bytes larger than reqd
#define MAX_BASE_ADDR_SLOP      (SST_TILE_SIZE-1)  // max of one tile

void sstAlloc3DFrameBuffers(SstRegs *sst)
{
  FxU32 saveseed = getSeed();
  PlacementStrategy bufferPlacement;           //Determines how buffers are placed
  int colorTiled, auxTiled, bpp;
  FxU32 tilesInX, tilesInY;
  CsimPrivate *cpriv = CSIM_PRIVATE(diago.sstCSIM);
  
  bpp = diago.rgb==32 ? 4 : 2;	// compute bytes per pixel
  // check that frame buffers will fit in remaining memory
  colorTiled = diago.ytiled == -1 ? iRandom(1) : diago.ytiled;
  auxTiled = diago.ytiled == -1 ? iRandom(1) : diago.ytiled;

  if(diago.randomPlacement)  
    bufferPlacement = randomPlacement;
  else
    bufferPlacement = normalPlacement;

  tilesInX = CEIL(diago.xmaxscreen * bpp, SST_TILE_WIDTH);
  tilesInY = CEIL(diago.ymaxscreen, SST_TILE_HEIGHT) + 1;

  //Allocate the color buffers
  if(colorTiled)
    { //Tiled color buffers
      //Allocate Color Buffer 0 with a little bit of slop
      diagfb.colBufferStride[0] = tilesInX + 1 + iRandom(2); //Randomly increase the width
      diagfb.colBufferAddr[0] = 
	(FxU32)allocate(diagfb.colBufferStride[0] * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			"Color Buffer #0", bufferPlacement);
      if(diago.aaEnabled)
	diagfb.colBufferAddrSecondary[0] = 
	  (FxU32)allocate(diagfb.colBufferStride[0] * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			  "2nd Col Buf#0", bufferPlacement);


      //Allocate Color Buffer 1 with a little bit of slop
      diagfb.colBufferStride[1] = tilesInX + 1 + iRandom(2); //Randomly increase the width
      diagfb.colBufferAddr[1] = 
	(FxU32)allocate(diagfb.colBufferStride[1] * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			"Color Buffer #1", bufferPlacement);      
      if(diago.aaEnabled)
	diagfb.colBufferAddrSecondary[1] = 
	  (FxU32)allocate(diagfb.colBufferStride[1] * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			  "2nd Col Buf#1", bufferPlacement);      


      //Allocate Color Buffer 2 if triple buffering 
      if(diago.triple)
	{
	  diagfb.colBufferStride[2] = tilesInX + 1 + iRandom(2); //Randomly increase the width
	  diagfb.colBufferAddr[2] = 
	    (FxU32)allocate(diagfb.colBufferStride[2] * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			    "Color Buffer #2", bufferPlacement);      
	  if(diago.aaEnabled)
	    diagfb.colBufferAddrSecondary[2] = 
	      (FxU32)allocate(diagfb.colBufferStride[2] * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			      "2nd Col Buf#2", bufferPlacement);      	 
	}

      //Mark color buffers as tiled
      diagfb.colBufferStride[0] |= SST_BUFFER_MEMORY_TILED;      
      diagfb.colBufferStride[1] |= SST_BUFFER_MEMORY_TILED;      
      diagfb.colBufferStride[2] |= SST_BUFFER_MEMORY_TILED;      
    }
  else
    { //Linear color buffers   
      //Allocate Color Buffer 0 with a little bit of slop
      diagfb.colBufferStride[0] = (diago.xmaxscreen + iRandom(1)*16) * bpp;
      diagfb.colBufferAddr[0] = (FxU32)allocate(diagfb.colBufferStride[0] * diago.ymaxscreen + BUFFER_SLOP,
						"Color Buffer #0", bufferPlacement);
      if(diago.aaEnabled)
	diagfb.colBufferAddrSecondary[0] = (FxU32)allocate(diagfb.colBufferStride[0] * diago.ymaxscreen + BUFFER_SLOP,
							   "2nd Col Buf#0", bufferPlacement);
      
      //Allocate Color Buffer 1 with a little bit of slop
      diagfb.colBufferStride[1] = (diago.xmaxscreen + iRandom(1)*16) * bpp;
      diagfb.colBufferAddr[1] = (FxU32)allocate(diagfb.colBufferStride[1] * diago.ymaxscreen + BUFFER_SLOP,
						"Color Buffer #1", bufferPlacement);
      if(diago.aaEnabled)
	diagfb.colBufferAddrSecondary[1] = (FxU32)allocate(diagfb.colBufferStride[1] * diago.ymaxscreen + BUFFER_SLOP,
							   "2nd Col Buf#1", bufferPlacement);
      
      //Allocate Color Buffer 2 if triple buffering with a little bit of slop
      if(diago.triple)
	{
	  diagfb.colBufferStride[2] = (diago.xmaxscreen + iRandom(1)*16) * bpp;
	  diagfb.colBufferAddr[2] = (FxU32)allocate(diagfb.colBufferStride[2] * diago.ymaxscreen + BUFFER_SLOP,
						    "Color Buffer #2", bufferPlacement);
	  if(diago.aaEnabled)
	    diagfb.colBufferAddrSecondary[2] = (FxU32)allocate(diagfb.colBufferStride[2] * diago.ymaxscreen + BUFFER_SLOP,
							       "2nd Col Buf#2", bufferPlacement);

	}

      //Mark color buffers as linear
      diagfb.colBufferStride[0] |= SST_BUFFER_MEMORY_LINEAR;
      diagfb.colBufferStride[1] |= SST_BUFFER_MEMORY_LINEAR;
      diagfb.colBufferStride[2] |= SST_BUFFER_MEMORY_LINEAR;
    }

  //Lock all the color buffers so they can't be unallocated
  lockByName("Color Buffer #");
  lockByName("2nd Col Buf#");

  //Allocate Aux buffer if necessary
  if(diago.hasAuxBuffer)
    {
      if(auxTiled)
	{
	  //Allocate Tiled Aux Buffer with a little bit of slop
	  diagfb.auxBufferStride = tilesInX + 1 + iRandom(2); //Randomly increase the width
	  diagfb.auxBufferAddr = 
	    (FxU32)allocate(diagfb.auxBufferStride * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			    "Aux Buffer", bufferPlacement);
	  if(diago.aaEnabled)
	    diagfb.auxBufferAddrSecondary = 
	      (FxU32)allocate(diagfb.auxBufferStride * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			      "2nd Aux Buf", bufferPlacement);

	  //Mark color buffers as tiled
	  diagfb.auxBufferStride |= SST_BUFFER_MEMORY_TILED;
	}
      else
	{
	  //Allocate Linear Aux Buffer with a little bit of slop
	  diagfb.auxBufferStride = (diago.xmaxscreen + iRandom(1)*16) * bpp;
	  diagfb.auxBufferAddr = (FxU32)allocate(diagfb.auxBufferStride * diago.ymaxscreen + BUFFER_SLOP,
						 "Aux Buffer", bufferPlacement);
	  if(diago.aaEnabled)
	    diagfb.auxBufferAddrSecondary = (FxU32)allocate(diagfb.auxBufferStride * diago.ymaxscreen + BUFFER_SLOP,
							    "2nd Aux Buf", bufferPlacement);


	  //Mark color buffers as linear
	  diagfb.auxBufferStride |= SST_BUFFER_MEMORY_LINEAR;
	}

      //Lock the aux buffer so it can't be unallocated
      lockByName("Aux Buffer");
      lockByName("2nd Aux Buf");
    }

  //Mark the end of the buffers as the start of texture memory. This stupid-ass
  //Hack doesn't work when --randomPlacement or --randomCmdFifoPlacement is used.
  //This only affects diags that use these variables.
  diago.texMemStart = getHighestAddress();

  
  

  SET(sst->colBufferAddr,diagfb.colBufferAddr[0]);
  SET(sst->colBufferStride,diagfb.colBufferStride[0]);
  SET(sst->auxBufferAddr,diagfb.auxBufferAddr);
  SET(sst->auxBufferStride,diagfb.auxBufferStride);

  if(diago.aaEnabled)
    {
      SET(sst->colBufferAddr,diagfb.colBufferAddrSecondary[0] | SST_BUFFER_BASE_SELECT);
      SET(sst->auxBufferAddr,diagfb.auxBufferAddrSecondary | SST_BUFFER_BASE_SELECT);
    }

  GDBG_INFO(1,"virtual Front buffer addr,stride=0x%08x,0x%08x %s\n",
	    diagfb.colBufferAddr[0],diagfb.colBufferStride[0]&(~SST_BUFFER_MEMORY_TYPE),
	    (diagfb.colBufferStride[0] & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ?
	    "tiled" : "linear");
  GDBG_INFO(1,"virtual Back  buffer addr,stride=0x%08x,0x%08x %s\n",
	    diagfb.colBufferAddr[1],diagfb.colBufferStride[1]&(~SST_BUFFER_MEMORY_TYPE),
	    (diagfb.colBufferStride[1] & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ?
	    "tiled" : "linear");
  if ( diago.triple )
    GDBG_INFO(1,"virtual Back  buffer addr,stride=0x%08x,0x%08x %s\n",
	      diagfb.colBufferAddr[2],diagfb.colBufferStride[2]&(~SST_BUFFER_MEMORY_TYPE),
	      (diagfb.colBufferStride[2] & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ?
	      "tiled" : "linear");      
  GDBG_INFO(1,"virtual Aux   buffer addr,stride=0x%08x,0x%08x %s %s\n",
	    diagfb.auxBufferAddr,diagfb.auxBufferStride&(~SST_BUFFER_MEMORY_TYPE),
	    (diagfb.auxBufferStride & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ?
	    "tiled" : "linear", diago.hasAuxBuffer ? "" : "(not active)" );

  if ( diago.ytiled == 2 )
    diago.ytiled = 0;  // for trex/fbi diags, ytiled==2 is just a special linear memory layout

  diagfb.inRegister = 0;
  drawbufferRandom();  // if necessary, reload color buf base/stride regs based on -d flag

  setSeed(saveseed);  // resync random sequence after linear/tiled setup  
}

void sstAlloc2DFrameBuffers(SstRegs *sst)
{
  FxU32 saveseed = getSeed();
  PlacementStrategy bufferPlacement;           //Determines how buffers are placed
  int srcTiled, dstTiled, bpp;
  FxU32 tilesInX, tilesInY;
  CsimPrivate *cpriv = CSIM_PRIVATE(diago.sstCSIM);

  if ( diago.ytiled == -1 )
    diago.ytiled = iRandom(3);

  if(diago.randomPlacement)  
    bufferPlacement = randomPlacement;
  else
    bufferPlacement = normalPlacement;
    
  //  diago.ytiled    Src             Dst
  //  ------------    ------          ------
  //       0          Linear          Linear
  //       1          Tiled           Tiled 
  //       2          Linear          Tiled 
  //       3          Tiled           Linear    
  dstTiled = ( diago.ytiled == 1 || diago.ytiled == 2 ) ? 1 : 0;
  srcTiled = ( diago.ytiled == 1 || diago.ytiled == 3 ) ? 1 : 0;
  
  bpp = 4;         // bytes per pixel
  tilesInX = CEIL(diago.xmaxscreen * bpp, SST_TILE_WIDTH) + 1;
  tilesInY = CEIL(diago.ymaxscreen, SST_TILE_HEIGHT) + 1;

  //Allocate the buffers  
  if ( dstTiled )
    {  //Tiled color buffer with a little bit of slop
      diagfb.colBufferStride[0] = tilesInX + 1 + iRandom(2); //Randomly increase the width
      diagfb.colBufferAddr[0] = 
	(FxU32)allocate(diagfb.colBufferStride[0] * tilesInY * SST_TILE_WIDTH * SST_TILE_HEIGHT + BUFFER_SLOP,
			"Color Buffer #0", bufferPlacement);      

      //Mark as tiled
      diagfb.colBufferStride[0] |= SST_BUFFER_MEMORY_TILED;
    }
  else  
    { //Linear color buffer with a little bit of slop
      diagfb.colBufferStride[0] = (diago.xmaxscreen + iRandom(1)*16) * bpp;
      diagfb.colBufferAddr[0] = (FxU32)allocate(diagfb.colBufferStride[0] * diago.ymaxscreen + BUFFER_SLOP,
						"Color Buffer #0", bufferPlacement);
      //Mark as linear
      diagfb.colBufferStride[0] |= SST_BUFFER_MEMORY_LINEAR;
    }

  //Lock the Color buffer so it can't be unallocated
  lockByName("Color Buffer #");

      
  diagfb.colBufferAddr[1] = 0xDEADBEEF;
  diagfb.colBufferStride[1] = 0xDEADEEF;
  diagfb.colBufferAddr[2] = 0xDEADBEEF;
  diagfb.colBufferStride[2] = 0xDEADBEEF;
  diagfb.auxBufferAddr = 0xDEADBEEF;
  diagfb.auxBufferStride = 0xDEADBEEF;

  SET(sst->colBufferAddr,diagfb.colBufferAddr[0]);
  SET(sst->colBufferStride,diagfb.colBufferStride[0]);

      
  GDBG_INFO(1,"virtual Src/Dst buffer addr,stride=0x%08x,0x%08x %s\n",
	    diagfb.colBufferAddr[0],diagfb.colBufferStride[0]&(~SST_BUFFER_MEMORY_TYPE),
	    (diagfb.colBufferStride[0] & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ?
	    "tiled" : "linear");

  setSeed(saveseed);  // resync random sequence after linear/tiled setup  
}

// get and check all the cmdfifo registers
// we assume the hardware is already idle
void DIAG_CHECKCMDFIFOSTATS(void)
{
  SstCRegs *sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(diago.sst);
  FxU32 hw, sw;

#ifndef CVG
  if (!diago.writeFifo) 
    return;

  if (diago.trexStandAlone) {
    gdbg_printf("WARNING: skipping DIAG_CHECKCMDFIFOSTATS because TREX_STANDALONE is active\n");
    return;
  }
  
  if (!diago.halInfo->csimio) {
    gdbg_printf("WARNING: skipping DIAG_CHECKCMDFIFOSTATS because CSIMIO is not active\n");
    return;
  }
  
  gdbg_info(1,"DIAG_CHECKCMDFIFOSTATS\n");

  // cmdfifo 0

  hw = GET(sstc->cmdFifo0.readPtrH);
  sw = diago.halInfo->csimLastRead;
  DIAG_TESTREG32("cmdFifo0.readPtrH", sw, hw);

  hw = GET(sstc->cmdFifo0.readPtrL);
  sw = diago.halInfo->csimLastRead;
  DIAG_TESTREG32("cmdFifo0.readPtrL", sw, hw);

  hw = GET(sstc->cmdFifo0.depth);
  sw = diago.halInfo->csimLastRead;
  DIAG_TESTREG32("cmdFifo0.depth", sw, hw);

  // cmdfifo 1

  hw = GET(sstc->cmdFifo1.readPtrH);
  sw = diago.halInfo->csimLastRead;
  DIAG_TESTREG32("cmdFifo1.readPtrH", sw, hw);

  hw = GET(sstc->cmdFifo1.readPtrL);
  sw = diago.halInfo->csimLastRead;
  DIAG_TESTREG32("cmdFifo1.readPtrL", sw, hw);

  hw = GET(sstc->cmdFifo1.depth);
  sw = diago.halInfo->csimLastRead;
  DIAG_TESTREG32("cmdFifo1.depth", sw, hw);
#endif

}

// get and check all the pixel counters
// we assume the hardware is already idle
void DIAG_CHECKPIXSTATS(void)
{
    FxU32 sw;
    FxU32 fbiPixelsIn,fbiChromaFail,fbiZfuncFail,fbiAfuncFail,fbiPixelsOut;
    FxU32 fbiStencilFail,fbiTrianglesOut;
    SstRegs *sst = diago.sst;

    if (diago.trexStandAlone) {
        gdbg_printf("WARNING: skipping DIAG_CHECKPIXSTATS because TREX_STANDALONE is active\n");
        return;
    }

    if (!diago.halInfo->csimio) {
        gdbg_printf("WARNING: skipping DIAG_CHECKPIXSTATS because CSIMIO is not active\n");
	return;
    }

    if(diago.chipCount > 1)
      {
	GDBG_INFO(0, "WARNING: skipping DIAG_CHECKPIXSTATS because running in multichip mode\n");
	return;
      }

    if(diago.aaEnabled)
      {
	GDBG_INFO(0, "WARNING: skipping DIAG_CHECKPIXSTATS because using AA\n");
	return;
      }

    gdbg_info(1,"DIAG_CHECKPIXSTATS\n");
    fbiPixelsIn = GET(sst->stats.fbiPixelsIn);
    sw = diago.halInfo->csimLastRead;
    DIAG_TESTREG32("fbiPixelsIn", sw, fbiPixelsIn);

    fbiChromaFail = GET(sst->stats.fbiChromaFail);
    sw = diago.halInfo->csimLastRead;
    DIAG_TESTREG32("fbiChromaFail", sw, fbiChromaFail);

    fbiZfuncFail = GET(sst->stats.fbiZfuncFail);
    sw = diago.halInfo->csimLastRead;
    DIAG_TESTREG32("fbiZfuncFail", sw, fbiZfuncFail);

    fbiAfuncFail = GET(sst->stats.fbiAfuncFail);
    sw = diago.halInfo->csimLastRead;
    DIAG_TESTREG32("fbiAfuncFail", sw, fbiAfuncFail);

    fbiPixelsOut = GET(sst->stats.fbiPixelsOut);
    sw = diago.halInfo->csimLastRead;
    DIAG_TESTREG32("fbiPixelsOut", sw, fbiPixelsOut);

    fbiStencilFail = GET(sst->fbiStencilFail);
    sw = diago.halInfo->csimLastRead;
    DIAG_TESTREG32("fbiStencilFail", sw, fbiStencilFail);

    fbiTrianglesOut = GET(sst->fbiTrianglesOut);
    sw = diago.halInfo->csimLastRead;
    DIAG_TESTREG32("fbiTrianglesOut", sw, fbiTrianglesOut);
}

// cleanup and printout, gets called from DIAG_PASS or DIAG_FAIL
// HACK: reaches into C simulator globals to get stats
void DIAG_END(int fail)
{
    FxU32 fbiPixelsIn,fbiChromaFail,fbiZfuncFail,fbiAfuncFail,fbiPixelsOut;
    FxU32 fbiStencilFail,fbiTrianglesOut;
    FxU32 chipIndex;
    SstRegs *sst = diago.sst;
    CsimPrivate *cpriv = NULL;
    

    if(diago.sstCSIM)
      cpriv = CSIM_PRIVATE(diago.sstCSIM);
    
    //Disable snooping on all chips
    //Disable enhanced video as well
    for(chipIndex=0; chipIndex<(FxU32)diago.chipCount; chipIndex++)
      {
	const PciRegister CFG_INIT_ENABLE = { 65, 3, READ_WRITE };
	const PciRegister cfgVideoCtrl0 = {128, 4, READ_WRITE};
	FxU32 data=0;
	
	if(((chipIndex == 0) && diago.sstCSIM) ||
	   ((chipIndex != 0) && diago.sstChildrenCSIM[chipIndex-1]))
	  {
	    pciGetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[chipIndex].deviceNumber, &data);
	    data &= ~(SST_ADDRESS_SNOOP_ENABLE | SST_MEMBASE0_SNOOP_ENABLE |
		      SST_MEMBASE1_SNOOP_ENABLE | SST_INIT_REGISTER_SNOOP_ENABLE);
	    pciSetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[chipIndex].deviceNumber, &data);
	    
	    data = 0;
	    //pciSetConfigData(cfgVideoCtrl0, diago.halInfo->boardInfo[chipIndex].deviceNumber, &data);
	  }
      }

    if (shutdown) return;       // prevent from executing twice
    if (!fail)                  // call idle one last time
        sst_idle_really(sst);

  // GMT: we have to read these regs before we shut down the CMD FIFO
  if (!fail) {  // don't talk the hardware anymore
    fbiPixelsIn   = GET(sst->stats.fbiPixelsIn);
    fbiChromaFail = GET(sst->stats.fbiChromaFail);
    fbiZfuncFail  = GET(sst->stats.fbiZfuncFail);
    fbiAfuncFail  = GET(sst->stats.fbiAfuncFail);
    fbiPixelsOut  = GET(sst->stats.fbiPixelsOut);
    fbiStencilFail  = GET(sst->fbiStencilFail);
    fbiTrianglesOut  = GET(sst->fbiTrianglesOut);
    if (diago.writeFifo) {
      FxU32 read0,read1,depth0,depth1;
      SstCRegs *sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(diago.sst);
      read0 = GET(sstc->cmdFifo0.readPtrL);
      read1 = GET(sstc->cmdFifo1.readPtrL);
      depth0 = GET(sstc->cmdFifo0.depth);
      depth1 = GET(sstc->cmdFifo1.depth);
      gdbg_info(1,"            CmdFifo0 readptrL :0x%08x\n",read0);
      gdbg_info(1,"            CmdFifo1 readptrL :0x%08x\n",read1);
      gdbg_info(1,"            CmdFifo0 depth    :%6d\n",depth0);
      gdbg_info(1,"            CmdFifo1 depth    :%6d\n",depth1);

    }
    gdbg_info(1,"DIAG_END: SST-1 statistics (remember NOP can reset these)\n");
    gdbg_info(1,"            PixelsIn :%6d\n",fbiPixelsIn);
    gdbg_info(1,"         StencilFail :%6d\n",fbiStencilFail);
    gdbg_info(1,"          ChromaFail :%6d\n",fbiChromaFail);
    gdbg_info(1,"           ZfuncFail :%6d\n",fbiZfuncFail);
    gdbg_info(1,"           AfuncFail :%6d\n",fbiAfuncFail);
    gdbg_info(1,"           PixelsOut :%6d\n",fbiPixelsOut);
    gdbg_info(1,"       Triangles Out :%6d\n",fbiTrianglesOut);
    gdbg_info(1,"       2D Pixels Out :%6d\n",CSIM_PRIVATE(diago.sstCSIM)->pixelsOut2d);
    DIAG_CHECKPIXSTATS();
    DIAG_CHECKCMDFIFOSTATS();
    DIAG_TESTSWAP();                    // check the displayed buffer
  }

    // if the CMD fifo is enabled then shut it down
    if (diago.writeFifo && diago.cmdFifoEnabled) {
        fxHalInitCmdFifo(sst, diago.whichFifo,0,0,0,0,diago.agpEnable);
	diago.cmdFifoEnabled = 0;  // CMDFIFO is now off
    }
    shutdown++;                 // prevent further gsim calls or idle checks

    if (diago.dumpPpm) {                // if movie files requested
      char imgname[128];

      strcpy(imgname,diago.pgm_name); strcat(imgname,"_front");
      DIAG_SAVEIMAGE(imgname,sstRollFwdBufId(CSIM_BUF_3D_FRONT),
                     diago.xmaxscreen,diago.ymaxscreen,IMG_P6);

      //Save other buffers if not a 2d only diag
      if(!diago.gui)
	{
	  strcpy(imgname,diago.pgm_name); strcat(imgname,"_back");
	  DIAG_SAVEIMAGE(imgname,sstRollFwdBufId(CSIM_BUF_3D_BACK),
			 diago.xmaxscreen,diago.ymaxscreen,IMG_P6);
	  if ( diago.triple ) {
	    strcpy(imgname,diago.pgm_name); strcat(imgname,"_triple");
	    DIAG_SAVEIMAGE(imgname,sstRollFwdBufId(CSIM_BUF_3D_TRIPLE),
			   diago.xmaxscreen,diago.ymaxscreen,IMG_P6);
	  }
	  strcpy(imgname,diago.pgm_name); strcat(imgname,"_aux");
	  DIAG_SAVEIMAGE(imgname,sstRollFwdBufId(CSIM_BUF_3D_AUX1),
			 diago.xmaxscreen,diago.ymaxscreen,IMG_P6);
	}
    }

    if (diago.dumpMemory) 
      DIAG_SAVEMEMORY(diago.pgm_name);

    if((cpriv != NULL) && diago.vectorGenMask)
      closeVectorFiles( cpriv, diago.vectorGenMask );

    GDBG_INFO(1,"DIAG_END: random seed = 0x%x\n",getSeed());
}


//----------------------------------------------------------------------
// Checks if base and sizeBytes collides with trash areas
//----------------------------------------------------------------------
int sstTrashMem(FxU32 base,unsigned sizeBytes)
{
  //  gdbg_info(1,"sanity base %x %x %x %x \n",base,diago.minTrashMem,sizeBytes,diago.maxTrashMem);
  return((base < (FxU32)diago.minTrashMem) ||
         (base + sizeBytes) >= (FxU32)diago.maxTrashMem);
}


//----------------------------------------------------------------------
// Open a file for each bit set in the vector gen mask
// Each file will be filled with vectors which can then be 
// used to stimulate and check a verilog module.
//----------------------------------------------------------------------
void openVectorFiles( CsimPrivate *cpriv, 
                      unsigned int vectorGenMask )
{
  int i;
  unsigned int mask = 0x1;
  char *cp, logname[200], finalname[200];
  FILE *fp;

  strcpy(logname,diago.pgm_name);
  for (cp = logname; *cp; cp++) // lower case the name
    *cp = tolower(*cp);
  cp = strstr(logname,".exe");  // strip off trailing .exe
  if (cp) *cp = '\0';

  for ( i = 0; i < VECTORID_MAX; i++, mask <<= 1 ) {
    if ( mask & vectorGenMask ) { 
      
      sprintf(finalname,"%s_%0d.vec", logname, i);
      fp = fopen(finalname,"w");
      if (fp == NULL) {
        gdbg_printf ("ERROR: could not open vector file '%s'\n", finalname );
        DIAG_FAIL();
        exit(2);
      }
      cpriv->vectorf.vectorHandles[i] = fp;
    }
  }
  cpriv->vectorf.dumpVector = dumpVector; 
}
//----------------------------------------------------------------------
// Close all vector files - a bit of overkill, I guess
//----------------------------------------------------------------------
void closeVectorFiles( CsimPrivate *cpriv, unsigned int vectorGenMask )
{
  int i;
  unsigned int mask = 0x1;

  for ( i = 0; i < VECTORID_MAX; i++, mask <<= 1 ) {
    if ( mask & vectorGenMask ) { 
      if ( fclose( cpriv->vectorf.vectorHandles[i] ) ) {
        gdbg_printf ("ERROR: could not close vector file number '%d'\n", i );
      }
    }
  }
}

//----------------------------------------------------------------------
// Print a vector 
//----------------------------------------------------------------------
void dumpVector( CsimPrivate *cpriv,  unsigned char * vectorData, unsigned int vectorType )
{
  unsigned int mask = 0x1;
  //  CacheOutput * cache;
  DecompOutput * decomp;

  switch ( vectorType ) 
    {
    case VECTORID_CACHE : { 
      CacheOutput * cache = ( CacheOutput *) vectorData ; 
      fprintf( cpriv->vectorf.vectorHandles[ vectorType ], "%08x %08x %08x %08x %08x %08x %08x %08x %03x %03x\n",
               cache->bank[0].data[0], cache->bank[0].data[1], 
               cache->bank[1].data[0], cache->bank[1].data[1], 
               cache->bank[2].data[0], cache->bank[2].data[1], 
               cache->bank[3].data[0], cache->bank[3].data[1],
               cache->u, cache->v );
      break;
    }
    case VECTORID_DECOMPRESSOR :
      decomp = ( DecompOutput * ) vectorData;
      fprintf ( cpriv->vectorf.vectorHandles[ vectorType ], "%08x %08x %08x %08x\n",
                decomp->Texel[0], decomp->Texel[1], 
                decomp->Texel[2], decomp->Texel[3] );
      break;

    default:
      gdbg_printf ("illegal vectorType in routine dumpVector : %d \n", vectorType );
      break;
    }
}

//----------------------------------------------------------------------
// Allocates random frame buffer 
//----------------------------------------------------------------------
FxU32 sstFbMemRalloc(unsigned sizeBytes)
{
  static int callCount=0;
  FxU32 address;
  int oldMaxTrashMem; 
  char name[32];

  sprintf(name, "RandBuffer %x", callCount++);
  
  oldMaxTrashMem = diago.maxTrashMem;
  address=allocate(sizeBytes, name, randomPlacement);

  //Need to imitate old semantics of this function
  //The diags that depend on the old, dumb way of
  //allocating framebuffer memory don't consider this
  //memory to have been used
  diago.maxTrashMem = oldMaxTrashMem;

  return(address);
}


//----------------------------------------------------------------------
// convert various color formats to 8888 RGBA
// note that we replicate msb's into the lsb's until we fill up all 8 bits
//----------------------------------------------------------------------
void    
sstYab422to8888(DiagNccTable *ncc, unsigned char *c888, unsigned char yab)
{
    unsigned char y,i,q;
    int r,g,b;

    y = yab >> 4;               // separate out YIQ
    i = (yab >> 2) & 0x3;
    q = (yab >> 0) & 0x3;

    r = ncc->yRGB[y] + ncc->iRGB[i][0] + ncc->qRGB[q][0];
    g = ncc->yRGB[y] + ncc->iRGB[i][1] + ncc->qRGB[q][1];
    b = ncc->yRGB[y] + ncc->iRGB[i][2] + ncc->qRGB[q][2];
//gdbg_printf("YIQ: 0x%x  RGB: %d %d %d\n",yab,r,g,b);

    if (r < 0) r = 0;
    else if (r > 0xFF) r = 0xFF;
    if (g < 0) g = 0;
    else if (g > 0xFF) g = 0xFF;
    if (b < 0) b = 0;
    else if (b > 0xFF) b = 0xFF;

//gdbg_printf("YIQ: 0x%x  RGB: 0x%06x\n",yab,(b<<16) | (g<<8) | r);
    c888[0] = r;
    c888[1] = g;
    c888[2] = b;
    c888[3] = 0xFF;
}

void
sstAi44to8888 (unsigned char c8888[4], unsigned char c44)
{
    c8888[0] =
    c8888[1] =
    c8888[2] = ((c44&0xF)<<4) | (c44&0xF);
    c8888[3] = ((c44&0xF0)>>4) | (c44&0xF0);
}

void
sstRgba332to8888 (unsigned char c8888[4], unsigned char c332)
{
    static unsigned char a3[] = {0x00,0x24,0x49,0x6d,0x92,0xb6,0xdb,0xff};
    static unsigned char a2[] = {0x00,0x55,0xaa,0xff};

    c8888[0] = a3[c332>>5];
    c8888[1] = a3[(c332>>2)&7];
    c8888[2] = a2[c332&3];
    c8888[3] = 0xFF;
}

void
sstRgba565to8888 (unsigned char c8888[4], unsigned short c565)
{
    int t;

    t = c565 & 0xF800;
    c8888[0] = (t>>8) | (t>>13);
    t = c565 & 0x07E0;
    c8888[1] = (t>>3) | (t>>9);
    t = c565 & 0x001F;
    c8888[2] = (t<<3) | (t>>2);
    c8888[3] = 0xFF;
}

void
sstRgba555to8888 (unsigned char c8888[4], unsigned short c555)
{
    int t;

    t = c555 & 0x7C00;
    c8888[0] = (t>>7) | (t>>12);
    t = c555 & 0x03E0;
    c8888[1] = (t>>2) | (t>>7);
    t = c555 & 0x001F;
    c8888[2] = (t<<3) | (t>>2);
    c8888[3] = 0xFF;
}

void
sstRgba1555to8888 (unsigned char c8888[4], unsigned short c1555)
{
    sstRgba555to8888(c8888,c1555);
    c8888[3] = c1555 & 0x8000 ? 0xFF : 0x00;
}

void
sstRgba4444to8888 (unsigned char c8888[4], unsigned short c4444)
{
    int t;

    t = (c4444>>8) & 0xF;
    c8888[0] = t | (t<<4);
    t = (c4444>>4) & 0xF;
    c8888[1] = t | (t<<4);
    t = (c4444>>0) & 0xF;
    c8888[2] = t | (t<<4);
    t = (c4444>>12) & 0xF;
    c8888[3] = t | (t<<4);
}

void
sstRgba6666to8888 (unsigned char c8888[4], unsigned long c6666)
{
    int t;

    t = (c6666>>10) & 0xFC;
    c8888[0] = t | (t>>6);
    t = (c6666>>4) & 0xFC;
    c8888[1] = t | (t>>6);
    t = (c6666<<2) & 0xFC;
    c8888[2] = t | (t>>6);
    t = (c6666>>16) & 0xFC;
    c8888[3] = t | (t>>6);
}

//----------------------------------------------------------------------
// utility dithering routines, emulate SST hardware dithering
//----------------------------------------------------------------------
int sstDit5(int n, int d)
{
    n = ((n<<1) | (n>>7)) - (n>>4);     // normalize
    n += d;                             // add in dither
    return n>>4;                        // return integer
}

int sstDit52(int n, int d)
{
    n = (n - (n>>5))<<1;                // normalize
    n += d;                             // add in dither
    return n>>4;                        // return integer
}

int sstDit6(int n, int d)
{
    n = ((n<<2) | (n>>6)) - (n>>4);     // normalize
    n += d;                             // add in dither
    return n>>4;                        // return integer
}

int sstDit62(int n, int d)
{
    n = (n - (n>>6))<<2;                // normalize
    n += d;                             // add in dither
    return n>>4;                        // return integer
}

// dither to either 565 or 555 depending on mode
unsigned long sst_dit565_2(unsigned long col, int x, int y)
{
    int dm;
    int r,g,b;

    dm = (((y^x)&1)<<3) | ((y&1)<<2);
    if (diago.sstCSIM->renderMode & SST_RM_DITHER_ROTATION) {
	int dsel = (diago.sstCSIM->fogMode & SST_DITHER_ROTATE)>>SST_DITHER_ROTATE_SHIFT;
	if (dsel & 1) dm = ((((y^x)&1)^1)<<3) | (((x&1)^1)<<2);
	if (dsel & 2) dm ^= 0x4;
    }
    r = (col>>16) & 0xFF;
    g = (col>>8) & 0xFF;
    b = col & 0xFF;
    r = sstDit52(r,dm)<<3;      // return to 8.0 format
    if (diago.rgb==16)
	g = sstDit62(g,dm)<<2;
    else
	g = sstDit52(g,dm)<<3;
    b = sstDit52(b,dm)<<3;
    return (col&0x80000000) | (r<<16) | (g<<8) | (b);
}

// dither to either 565 or 555 depending on mode
unsigned long sst_dit565_4(unsigned long col, int x, int y)
{
    int dm = (((y^x)&1)<<3) | ((y&1)<<2) | ((y^x)&2) | ((y&2)>>1);
    int r,g,b;

    if (diago.sstCSIM->renderMode & SST_RM_DITHER_ROTATION) {
	int dsel = (diago.sstCSIM->fogMode & SST_DITHER_ROTATE)>>SST_DITHER_ROTATE_SHIFT;
	if (dsel & 1) dm = ((((y^x)&1)^1)<<3) | (((x&1)^1)<<2) | ((y^x)&2) | ((y&2)>>1);
	if (dsel & 2) dm ^= 0x4;
    }
    r = (col>>16) & 0xFF;
    g = (col>>8) & 0xFF;
    b = col & 0xFF;
    r = sstDit5(r,dm)<<3;       // return to 8.0 format
    if (diago.rgb==16)
	g = sstDit6(g,dm)<<2;
    else
	g = sstDit5(g,dm)<<3;
    b = sstDit5(b,dm)<<3;
    return (col&0x80000000) | (r<<16) | (g<<8) | (b);
}

// formulate an 8888 ARGB result based on current renderMode
FxU32 sst_argb_form_result(FxU32 c, int endit, int x, int y)
{
    switch (diago.rgb) {
	case 15:
	    if ((diago.sstCSIM->renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ZERO)
		c &= 0x00FFFFFF;
	    if ((diago.sstCSIM->renderMode & SST_RM_ALPHAMODE) == SST_RM_ALPHA_ONE)
		c |= 0xFF000000;
	    if (endit & 2) c=sst_dit565_2(c,x,y);
	    if (endit & 4) c=sst_dit565_4(c,x,y);
	
	    c &= 0x80F8F8F8;			// truncate to 565 RGB
	    break;
	case 16:
	    if (endit & 2) c=sst_dit565_2(c,x,y);
	    if (endit & 4) c=sst_dit565_4(c,x,y);
	    c &= 0x00F8FCF8;			// truncate to 565 RGB
	    break;
	case 32:
	    break;
	default:
	    GDBG_ERROR("rgba_form_result","diago.rgb %d\n",diago.rgb);
	    break;
    }
    return c;
}

void GUI_CHECK(void)
{
    if (diago.gui)
        GDBG_ERROR("diag","3d diag routine called in 2d mode\n");
}

//----------------------------------------------------------------------
// force pixel x,y in the framebuffer to color 'col' bypassing simulation
//----------------------------------------------------------------------
void DIAG_FORCE_PIXEL(FxI32 buffer, int x, int y, FxU32 col)
{
  FxI32 sampleIndex, maxSampleIndex;

  if(diago.aaEnabled)
    maxSampleIndex = diago.aaSampleCount;
  else
    maxSampleIndex = 1;

    if ((diago.halInfo->hsim & HSIM_TREX_STANDALONE) || !diago.checkEveryTriangle)
        return;

    if (ONSCREEN(x,y)) {                        // if onscreen
        // optionally convert from 888 RGB to 565 or 555 RGB
        if (buffer == CSIM_BUF_3D_FRONT || buffer == CSIM_BUF_3D_BACK) {
	    if (diago.rgb == 16)
		col = ((col & 0xF80000)>>8) | ((col & 0x00FC00)>>5) | ((col & 0x0000F8)>>3);
	    if (diago.rgb == 15)
		col = (((col>>16)&0x8000) | (col & 0xF80000)>>9) | ((col & 0x00F800)>>6) | ((col & 0x0000F8)>>3);
	}

	for(sampleIndex=0; sampleIndex < maxSampleIndex; sampleIndex++)
	  {
	    // set the SW sim framebuffer
	    csimPixelWrite(sampleIndex, buffer, x, y, col);
	    if (diago.halInfo->hsim)                // and the HW sim framebuffer
	      hsimPixelWrite(sampleIndex, buffer, x, y, col);
	    if (diago.halInfo->hw)
	      HW_PIXEL_WR(buffer,x,y,col);
	  }
    }
}

// same thing as DIAG_FORCE_PIXEL but forces memory contents
void DIAG_FORCE_MEM(FxU32 addr, FxU32 data, int nbytes)
{
  CSIM_MEM_WR(addr,data,nbytes);
  if ( diago.halInfo->hsim )
    HSIM_MEM_WR(addr,data,nbytes); 
  if (diago.halInfo->hw) 
    HW_MEM_WR(0,addr,data,nbytes);
}

// same thing as DIAG_FORCE_PIXEL but for a rectangle
void DIAG_FORCE_RECT(FxI32 buffer, int xs, int y, int w, int h, FxU32 col)
{
    int x;

    if ((diago.halInfo->hsim & HSIM_TREX_STANDALONE) || !diago.checkEveryTriangle)
        return;
    if (xs < 0) xs = 0;
    if (xs + w > diago.xmaxscreen) w = diago.xmaxscreen - xs;
    if (diago.halInfo->hw)
        HW_PIXEL_FAST_BEGIN(0, buffer);
    while (h-- > 0) {                           // for each scanline
	if (ONSCREEN(0,y))			// if Y is on screen
        for (x = xs; x < xs+w; x++) {           // for each pixel
            DIAG_FORCE_PIXEL(buffer,x,y,col);
        }
        y++;                                    // bump up to next scanline
    }
    if (diago.halInfo->hw)
        HW_PIXEL_FAST_END(0);
}

/* lfbOffset
**
** Summary: Compute offset of a pixel in the 3D LFB space given 
**          its x,y coordinates
**
** Important:  this offset must be multiplied by the number of bytes/pixel
**             in order to compute the LFB address to read/write
**
** Arguments: x, y     - screen coordinates from the current y-origin
*/
FxU32 lfbOffset( FxU32 x, FxU32 y )
{
  FxU32 offset = (y << SST_LFB_ADDR_Y_SHIFT) | x;

  return offset;
}

//----------------------------------------------------------------------
// draw pixel x,y in specified color using random LFB mode
// NOTE: currently it only supports 32-bit LFB access
// NOTE: to write correct alpha, it has to set zaColor if the lfb format
//	 does not include alpha
//----------------------------------------------------------------------
static char *lfbFmt_str[] = {"565","555","1555","****","888","8888","****","****"};

// returns 2 if alpha was written, else 1
static int
sst_lfbpixel(SstRegs *sst,int x, int y, unsigned long col, int enpipe)
{
    FxU32 *lfb = (FxU32 *)SST_LFB_ADDRESS(sst);
    int fmt;

    GUI_CHECK();
    fmt = iRandom(1) + SST_LFB_888;
    gdbg_info(10,"  lfbpixel(%d,%d) fmt = %s %s\n",
                x,y,lfbFmt_str[fmt],enpipe ? "ENPIXPIPE" : "" );
    // if 1555 or 8888 mode and LFB format doesn't have alpha and pixel pipeline
    // is enabled, then set zaColor to contain the alpha
    if ((diago.rgb != 16) && (fmt == SST_LFB_888) && (enpipe||(diago.rgb==15)))
	SET(sst->zaColor,col&0xFF000000);
    if (enpipe) fmt |= SST_LFB_ENPIXPIPE;
#ifdef CVG
    if (diago.curdrawbuffer) fmt |= SST_LFB_WRITEBACKBUFFER;
#endif
    // if RGB buffer contains alpha and format doesn't
    SET(sst->lfbMode,fmt);
    SET(lfb[lfbOffset(x,y)],col);
        P6FENCE;
    return (fmt==SST_LFB_8888) || enpipe || diago.rgb==15 ? 2 : 1;
}

//----------------------------------------------------------------------
// draw pixel x,y in specified color, chooses the following options
//      *) LFB write(returns 1) or triangle(returns 0)
//      *) RGB iterator or C1
// if lfbOK != 0 then its OK to use LFB writes, >0 implies ENPIXPIPE
// returns 0 if triangle, 1 if LFB no alpha written, 2 if LFB and alpha written
//----------------------------------------------------------------------
int sst_drawpixel(SstRegs *sst,int x, int y, unsigned long col, int lfbOK)
{
    int srcSel = iRandom(1);

    GUI_CHECK();
    if (diago.deviceID == SST_DEVICE_ID_SST96) {// SST-96
        lfbOK = 0;                              // can't use LFB accesses.
    }

    gdbg_info(10,"  drawpixel(%d,%d) lfbOK=%d fbzSrc=%s\n",
                x,y,lfbOK, srcSel ? "C1" : "RGBA");
    if (lfbOK && iRandom(1) && ((shadowRegisters3D[0][0].renderMode & SST_RM_DITHER_ROTATION) == 0)) {
        return sst_lfbpixel(sst,x,y,col,lfbOK>0);
    }
    if (srcSel) {
        SET(sst->fbzColorPath, SST_RGBSEL_C1 | SST_ASEL_C1);
        SET(sst->c1, col);
    }
    else {
        // set the color
        SET(sst->r, ((col>>16)&0xFF)<<SST_RGBA_FRACBITS);
        SET(sst->g, ((col>>8)&0xFF)<<SST_RGBA_FRACBITS);
        SET(sst->b, ((col>>0)&0xFF)<<SST_RGBA_FRACBITS);
        SET(sst->a, ((col>>24)&0xFF)<<SST_RGBA_FRACBITS);
        SET(sst->fbzColorPath, SST_RGBSEL_RGBA | SST_ASEL_RGBA);
    }
   // set x,y coords
    x <<= SST_XY_FRACBITS;
    y <<= SST_XY_FRACBITS;
    SET(sst->vA.x,x);
    SET(sst->vA.y,y);
    SET(sst->vB.x,x+XY_ONE-1);
    SET(sst->vB.y,y);
    SET(sst->vC.x,x+XY_ONE-1);
    SET(sst->vC.y,y+XY_ONE-1);
    SET(sst->triangleCMD,0);
    return 0;
}

//----------------------------------------------------------------------
// draw a span at x,y in current color
//----------------------------------------------------------------------
void sst_drawspan(SstRegs *sst, int x, int y, int spanlen)
{
    gdbg_info(10,"span @ %d,%d spanlen=%d\n", x,y,spanlen);

    //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
    //if appropriate (i.e. --pixelsPerClock <= 0)
    setPixelsPerClock(sst); 

    GUI_CHECK();
    x <<= SST_XY_FRACBITS;
    y <<= SST_XY_FRACBITS;
    y += XY_ONE>>1;             // add 1/2 to Y

    SET(sst->vA.x,x);
    SET(sst->vA.y,y);
    SET(sst->vB.x,x+(spanlen<<SST_XY_FRACBITS));
    SET(sst->vB.y,y);
    SET(sst->vC.x,x);
    SET(sst->vC.y,y+XY_ONE);
    SET(sst->triangleCMD,0);
}

//----------------------------------------------------------------------
// set the clip rectangle
//----------------------------------------------------------------------
void sst_setclip(SstRegs *sst,
                    unsigned long x1, unsigned long y1,
                    unsigned long x2, unsigned long y2)
{
    SET(sst->clipLeftRight,(x1<<16)|x2);
    SET(sst->clipBottomTop,(y1<<16)|y2);
}


// wait for chip to be idle and check all the HSIM/CSIM fifos
void DIAG_IDLE(void)
{
    sst_idle_really(diago.sst);         // really really wait for idle
#ifdef HAL_HSIM
    if (diago.halInfo->hsim)
        if (TESTBENCH_CHECK_DONE())     // double check hardware all done
            DIAG_FAIL();
#endif
}

//----------------------------------------------------------------------
// load an image into the given buffer
//
// setPixel is a function that will be call for each pixel
//----------------------------------------------------------------------

void DIAG_LOADIMAGE(char *imgFilename, int buffer, void (*setPixel)(int buf, int x, int y, int r, int g, int b) )
{
    int dbg198, saveCET;
    FxU32 x,y,*data;
    ImgInfo info;

    // read the input image
    if (imgReadFile(imgFilename, &info) == FXFALSE) {
        GDBG_ERROR("DIAG_LOADIMAGE","error reading image file %s\n",imgFilename);
        DIAG_FAIL();
    }

    gdbg_info( 2, "Initializing the screen image file %s ... " , imgFilename);
    csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXFALSE);     // disable video
    saveCET = diago.checkEveryTriangle;
    diago.checkEveryTriangle = 1;
    dbg198 = GDBG_GET_DEBUGLEVEL(198);
    GDBG_SET_DEBUGLEVEL(198,GDBG_GET_DEBUGLEVEL(301));
    if (diago.halInfo->hw)
        HW_PIXEL_FAST_BEGIN(0, buffer);
    data = (FxU32 *)info.any.data;

    for ( y = 0; y < info.any.height; y++ )
    for ( x = 0; x < info.any.width; x++ )
    {
      FxU8 r, g, b, a, *bptr;
      bptr = (FxU8 *)data;
      b = *bptr++;
      g = *bptr++;
      r = *bptr++;
      a = *bptr++;
      setPixel(buffer, x, y, r, g, b);
      data++;
    }
    if (diago.halInfo->hw)
        HW_PIXEL_FAST_END(0);

    free(info.any.data);
    diago.checkEveryTriangle = saveCET;
    GDBG_SET_DEBUGLEVEL(198,dbg198);
    gdbg_info_more( 2, "done.\n" );
    csimVideo(CSIM_PRIVATE(diago.sstCSIM),FXTRUE);      // enable video

}

//----------------------------------------------------------------------
// save the contents of a framebuffer as an image file
//
// the image filename will be <filenameRoot>_{csim,hsim,hw}<buffer>
//
//----------------------------------------------------------------------
void DIAG_SAVEIMAGE(const char *filenameRoot, FxU32 buffer, 
                    FxU32 width, FxU32 height, FxU32 imgType)
{
  char filenameExt[10];
  char filename[256];
  FxU32 x,y;
  FxU8 *buf;
  FxU32 col;
  ImgInfo info;
  int dbg199;
  
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
    GDBG_ERROR("csimPicSave", "invalid image type %d\n",imgType);
    return;
  }

  // turn off level 199 debug messages (mem reads) unless level 301 is specified
  dbg199 = GDBG_GET_DEBUGLEVEL(199);
  GDBG_SET_DEBUGLEVEL(199,GDBG_GET_DEBUGLEVEL(301));
  
  // note we first convert the framebuffer which is in 16-bit RGB format
  // to standard 32-bit ARGB format (in memory) for the image library
  if (diago.halInfo->csimio) {
    buf = (FxU8 *)info.any.data;
    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
	col = csimPixelReadCompositeBuffer(buffer,x,y);

	*buf++ = (FxU8) (col & 0xFF);       // b
	*buf++ = (FxU8) ((col>>8) & 0xFF);  // g
	*buf++ = (FxU8) ((col>>16) & 0xFF); // r
	*buf++ = (FxU8) (0x0);              // a
      }
    }
    sprintf(filename,"%s_csim%s",filenameRoot,filenameExt);
    if (!imgWriteFile(filename, &info, imgType, info.any.data))
      GDBG_ERROR( "imgWriteFile", "file '%s' failed: %s\n", filename, imgGetErrorString() );
  }

  // also store the hsim's framebuffer, if available
  if (diago.halInfo->hsim) {
    buf = (FxU8 *)info.any.data;
    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
	col = hsimPixelReadCompositeBuffer(buffer, x, y);

        *buf++ = (FxU8) (col & 0xFF);       // b
        *buf++ = (FxU8) ((col>>8) & 0xFF);  // g
        *buf++ = (FxU8) ((col>>16) & 0xFF); // r
        *buf++ = (FxU8) (0x0);              // a
      }
    }
    sprintf(filename,"%s_hsim%s",filenameRoot,filenameExt);
    if (!imgWriteFile(filename, &info, imgType, info.any.data))
      GDBG_ERROR( "imgWriteFile", "file '%s' failed: %s\n", filename, imgGetErrorString() );
  }
  
  // also store the hw's framebuffer, if available
  if (diago.halInfo->hw) {
    buf = (FxU8 *)info.any.data;
    HW_PIXEL_FAST_BEGIN(0, buffer);
    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
        col = HW_PIXEL_RD(buffer,x,y);
        *buf++ = (FxU8) (col & 0xFF);       // b
        *buf++ = (FxU8) ((col>>8) & 0xFF);  // g
        *buf++ = (FxU8) ((col>>16) & 0xFF); // r
        *buf++ = (FxU8) (0x0);              // a
      }
    }
    HW_PIXEL_FAST_END(0);
    sprintf(filename,"%s_hw%s",filenameRoot,filenameExt);
    if (!imgWriteFile(filename, &info, imgType, info.any.data))
      GDBG_ERROR( "imgWriteFile", "file '%s' failed: %s\n", filename, imgGetErrorString() );
  }
  
  GDBG_SET_DEBUGLEVEL(199,dbg199);  // restore debug level
  free(info.any.data);
}

//----------------------------------------------------------------------
// save the contents of a memory as a file
//
// the filename will be <filenameRoot>_{csim,hsim,hw}
//
//----------------------------------------------------------------------
void DIAG_SAVEMEMORY(const char *filenameRoot)
{
  char filename[256], extension[10];
  CsimPrivate *cp;
  int dbg199;
  FxI32 chipIndex;
  FILE *fp;

  // turn off level 199 debug messages (mem reads) unless level 301 is specified
  dbg199 = GDBG_GET_DEBUGLEVEL(199);
  GDBG_SET_DEBUGLEVEL(199,GDBG_GET_DEBUGLEVEL(301));
  
  for(chipIndex=0; chipIndex<diago.chipCount; chipIndex++)
    {

      if(chipIndex == 0)
	cp = CSIM_PRIVATE(diago.sstCSIM); 
      else
	cp = CSIM_PRIVATE(diago.sstChildrenCSIM[chipIndex-1]);
	  
      if(diago.chipCount == 1)
	strcpy(extension, ".mem");
      else
	sprintf(extension, ".mem%d", chipIndex);      

      // store csim's memory
      if ( diago.halInfo->csimio ) {
	sprintf(filename,"%s_csim%s",filenameRoot, extension);
	GDBG_PRINTF("Writing memory dump file %s\n",filename);
	if ( (fp = fopen(filename,WRITE_ATTRIBS)) == NULL ) {
	  GDBG_ERROR("DIAG_SAVEMEMORY","can't open output file %s\n",filename);
	  DIAG_FAIL();
	}
	if ( fwrite((void *)cp->memory,1,cp->memorySizeInBytes,fp) != (size_t) cp->memorySizeInBytes ) {
	  GDBG_ERROR("DIAG_SAVEMEMORY","error writing output file %s\n",filename);
	  DIAG_FAIL();
	}
	fclose(fp);
      }
    
      // also store the hsim's memory, if available
      if (diago.halInfo->hsim) {
	sprintf(filename,"%s_hsim%s",filenameRoot, extension);
	GDBG_PRINTF("Writing memory dump file %s\n",filename);
	if ( (fp = fopen(filename,WRITE_ATTRIBS)) == NULL ) {
	  GDBG_ERROR("DIAG_SAVEMEMORY","can't open output file %s\n",filename);
	  DIAG_FAIL();
	}
	if ( fwrite(cp->hsim_memory,1,cp->memorySizeInBytes,fp) != (size_t) cp->memorySizeInBytes ) {
	  GDBG_ERROR("DIAG_SAVEMEMORY","error writing output file %s\n",filename);
	  DIAG_FAIL();
	}
	fclose(fp);
      }
  
      // also store the hsim's framebuffer, if available
      if (diago.halInfo->hw) {
	FxU8 *sstRawLfb = (FxU8 *)(diago.halInfo->boardInfo[0].physAddr[1]);
	sprintf(filename,"%s_hw%s",filenameRoot, extension);
	GDBG_PRINTF("Writing memory dump file %s\n",filename);
	if ( (fp = fopen(filename,WRITE_ATTRIBS)) == NULL ) {
	  GDBG_ERROR("DIAG_SAVEMEMORY","can't open output file %s\n",filename);
	  DIAG_FAIL();
	}
	HW_PIXEL_FAST_BEGIN(0, 1);
	if ( fwrite(sstRawLfb,1,cp->memorySizeInBytes,fp) != (size_t) cp->memorySizeInBytes ) {
	  GDBG_ERROR("DIAG_SAVEMEMORY","error writing output file %s\n",filename);
	  DIAG_FAIL();
	}
	HW_PIXEL_FAST_END(0);
	fclose(fp);
      }
    }

  GDBG_SET_DEBUGLEVEL(199,dbg199);  // restore debug level
}

// check and make sure the hardware's status register reports the same
// buffer being displayed as the software simulator
int DIAG_TESTSWAP(void)
{
    int hw,sw;

    gdbg_info(1,"DIAG_TESTSWAP() swaps=%d\n",diagSwaps);
    if (diago.trexStandAlone) {
        gdbg_printf("WARNING: skipping DIAG_TESTSWAP because TREX_STANDALONE is active\n");
        return 0;
    }

    DIAG_IDLE();
#ifdef CVG
    hw = GET(diago.sst->status);
    sw = (diagSwaps & 1) << SST_DISPLAYED_BUFFER_SHIFT;
    gdbg_info(2,"\tgot status: 0x%08x\n",hw);
    hw &= SST_DISPLAYED_BUFFER;
    sw &= SST_DISPLAYED_BUFFER;
    return DIAG_TESTREG32( "status(displayed buffer)", sw, hw);
#else
FXUNUSED(hw);
FXUNUSED(sw);
    return 0;
#endif
}

#ifndef CVG 
//----------------------------------------------------------------------
// Banshee only has one active frame buffer, so to "switch" buffers
// just load the base/stride registers with previously determined 
// front/back buffer values stored in the diagfb structure.  Finally, 
// keep track of which base/stride pair is currently active.
//----------------------------------------------------------------------
void drawbufferSet(FxU32 buffer)
{
  
  // check for legal input values
  if ( buffer != CSIM_BUF_3D_FRONT && buffer != CSIM_BUF_3D_BACK )
    GDBG_ERROR("drawbufferSet","illegal buffer = %d\n",buffer);

  if ( buffer != (unsigned) diagfb.inRegister ) {
    // load base/stride values for specified buffer
    SET(diago.sst->colBufferAddr,diagfb.colBufferAddr[buffer]);
    SET(diago.sst->colBufferStride,diagfb.colBufferStride[buffer]);
    if(diago.aaEnabled)
      SET(diago.sst->colBufferAddr,diagfb.colBufferAddrSecondary[buffer] | SST_BUFFER_BASE_SELECT);
    
    // keep track of which base/stride pair is currently active
    diagfb.inRegister = buffer;
  }
  
}

//----------------------------------------------------------------------
// Banshee only has one active frame buffer, so to "swap" buffers just 
// exchange the predetermined front/back buffer values in the diagfb 
// structure and then toggle the contents of the base/stride registers.
//----------------------------------------------------------------------
void drawbufferSwap()
{
  FxU32 tmpBase, tmpStride;
  FxU32 tmpBaseSecondary;
  
  // swap base/stride values
  tmpBase = diagfb.colBufferAddr[CSIM_BUF_3D_FRONT];
  tmpStride = diagfb.colBufferStride[CSIM_BUF_3D_FRONT];

  //Stuff for when aa is enable
  tmpBaseSecondary = diagfb.colBufferAddrSecondary[CSIM_BUF_3D_FRONT];

  if ( diago.triple ) {
    diagfb.colBufferAddr[0] = diagfb.colBufferAddr[1];          // front <- back
    diagfb.colBufferAddrSecondary[0] = diagfb.colBufferAddrSecondary[1]; 
    diagfb.colBufferStride[0] = diagfb.colBufferStride[1];
    
    diagfb.colBufferAddr[1] = diagfb.colBufferAddr[2];          // back <- triple
    diagfb.colBufferAddrSecondary[1] = diagfb.colBufferAddrSecondary[2];
    diagfb.colBufferStride[1] = diagfb.colBufferStride[2];
    
    diagfb.colBufferAddr[2] = tmpBase;                          // triple <- front
    diagfb.colBufferAddrSecondary[2] = tmpBaseSecondary;
    diagfb.colBufferStride[2] = tmpStride;
  } else {
    diagfb.colBufferAddr[0] = diagfb.colBufferAddr[1];          // front <- back
    diagfb.colBufferAddrSecondary[0] = diagfb.colBufferAddrSecondary[1];
    diagfb.colBufferStride[0] = diagfb.colBufferStride[1];
    
    diagfb.colBufferAddr[1] = tmpBase;                          // back <- front
    diagfb.colBufferAddrSecondary[1] = tmpBaseSecondary;
    diagfb.colBufferStride[1] = tmpStride;
  }
  
  // the update the registers to contain the new base/stride values
  SET(diago.sst->colBufferAddr,diagfb.colBufferAddr[diagfb.inRegister]);
  SET(diago.sst->colBufferStride,diagfb.colBufferStride[diagfb.inRegister]);

  if(diago.aaEnabled)
    SET(diago.sst->colBufferAddr,
	diagfb.colBufferAddrSecondary[diagfb.inRegister] | SST_BUFFER_BASE_SELECT);

}
#endif

// need to keep track of the number of swaps, because HSIM's buffer 0
// never changes upon a swap, and CSIM swaps its buffer pointers
// GMT: currently only DIAG_DIFFSCREEN handles this case
void DIAG_SWAPBUFFER(void)
{
    int s;
    int swapInterval, waitOnVsync, dontSwap;

    s = iRandom(0xFFFFFFFF) & ~(0xFF<<1);       // clear out swapinterval

    waitOnVsync = (s & SST_SWAP_EN_WAIT_ON_VSYNC) ? 1 : 0;
    dontSwap = (s & SST_SWAP_DONT_SWAP) ? 1 : 0;

    if (diago.triple)
      waitOnVsync = 0;                          // must be disabled for triple buffering
    swapInterval = iRandom(iRandom(iRandom(8)));// random swapinterval [0,8]

    DIAG_SWAPBUFFER_EX(swapInterval, waitOnVsync, dontSwap);
}

void DIAG_SWAPBUFFER_EX(int swapInterval, int waitOnVsync, int dontSwap)
{
  int s;
  
  GUI_CHECK();
  
  // save an image of the current back buffer
  if ( !dontSwap && diago.saveBeforeSwap ) {
    char name[128];
    sprintf(name,"%s_swap_%d",diago.pgm_name,diagSwaps);
    if ( diago.dumpPpm )
      DIAG_SAVEIMAGE(name, CSIM_BUF_3D_BACK, diago.xmaxscreen, diago.ymaxscreen, IMG_P6);
    if ( diago.dumpMemory )
      DIAG_SAVEMEMORY(name);
  }
  
  // prepare to display the current back buffer
  SET(diago.sst->leftOverlayBuf,
      diagfb.colBufferAddr[CSIM_BUF_3D_BACK] & SST_BUFFER_BASE_ADDR);

  if(diago.aaEnabled)
    SET(diago.sst->leftDesktopBuf,
	diagfb.colBufferAddrSecondary[CSIM_BUF_3D_BACK] & SST_BUFFER_BASE_ADDR);

  // NOTE: rightOverlayBuf would be set here for stereo operations
  SET(diago.sst->swapBufferPend,0x0);
  
  // perform the swap
  s = iRandom(0xFFFFFFFF);
  s &= ~(SST_SWAP_EN_WAIT_ON_VSYNC|SST_SWAP_BUFFER_INTERVAL|SST_SWAP_DONT_SWAP|SST_SWAP_DESKTOP_EN);

  s |= swapInterval << SST_SWAP_BUFFER_INTERVAL_SHIFT;
  if ( waitOnVsync )
    s |= SST_SWAP_EN_WAIT_ON_VSYNC;
  if ( dontSwap )
    s |= SST_SWAP_DONT_SWAP;
  if(diago.aaEnabled)
    s |= SST_SWAP_DESKTOP_EN;
  
  SET(diago.sst->swapbufferCMD,s);
  
  if ( !dontSwap ) {
    diagSwaps++;
    drawbufferSwap();
  }
  
}    
 
// teset to make sure the current buffer is a particular color
void DIAG_TESTSCREEN_EX(int xmax, int ymax, FxU32 color)
{
    int x,y;
    FxU32 cfb;
    FxI32 sampleIndex, sampleIndexMax;

    gdbg_info(1,"DIAG_TESTSCREEN(%d,%d,0x%x) buffer=%d errors=%d/%d\n",
                        xmax,ymax,color,
                        diago.curdrawbuffer,diago.errorCount,diago.errorLimit);
    if (diago.trexStandAlone) {
        gdbg_printf("WARNING: skipping DIAG_TESTSCREEN because TREX_STANDALONE is active\n");
        return;
    }
    
    if(diago.aaEnabled)
      sampleIndexMax = diago.aaSampleCount;
    else
      sampleIndexMax = 1;

    for(sampleIndex=0; sampleIndex<sampleIndexMax; sampleIndex++)
      {
	if (diago.halInfo->hw)
	  HW_PIXEL_FAST_BEGIN(0, diago.curdrawbuffer);

	for (y=0; y<=ymax; y++)
	  for (x=0; x<=xmax; x++)
	    {
	      if (diago.halInfo->csim) {
		cfb = csimPixelRead(sampleIndex, diago.curdrawbuffer, x, y);
		DIAG_COMPARE_PIXEL("csim:diag_testscreen",diago.curdrawbuffer,x,y,cfb,color);
	      }
	      if (diago.halInfo->hsim) {
		cfb = hsimPixelRead(sampleIndex, diago.curdrawbuffer, x, y);
		DIAG_COMPARE_PIXEL("hsim:diag_testscreen",diago.curdrawbuffer,x,y,cfb,color);
	      }
	      if (diago.halInfo->hw) {
		cfb = HW_PIXEL_RD(diago.curdrawbuffer,x,y);
		DIAG_COMPARE_PIXEL("hw:diag_testscreen",diago.curdrawbuffer,x,y,cfb,color);
	      }
	    }

	if (diago.halInfo->hw)
	  HW_PIXEL_FAST_END(0);
      }
}

// test the entire screen to make sure it is a certain color
// returns 0 on success (or if haven't reached error limit yet)
int DIAG_TESTSCREEN(int xmax, int ymax, FxU32 color)
{
    DIAG_IDLE();
    if (diago.drawbuffer < 0) {         // if drawing to random buffers
        DIAG_TESTSCREEN_EX(xmax,ymax,color);            // test curbuffer
        diago.curdrawbuffer = 1-diago.curdrawbuffer;    // switch buffers
    }                                                   // and test again
    DIAG_TESTSCREEN_EX(xmax,ymax,color);
    return diago.errorCount >= diago.errorLimit;
}

void DIAG_DIFFSCREEN_EX(int xmax, int ymax, int buffer)
{
    int x,y, dbg199;
    FxI32 cfb,hfb, compareMask;
    FxI32 sampleIndex, sampleIndexMax;

    gdbg_info(1,"DIAG_DIFFSCREEN(%d,%d, %d)\tbegin: errors=%d/%d\n",
        xmax,ymax,buffer, diago.errorCount,diago.errorLimit);

    if ( (buffer==CSIM_BUF_3D_AUX1 || buffer==CSIM_BUF_3D_AUX2) && !diago.hasAuxBuffer) {
        gdbg_info(1,"...skipping because no AUX buffer present\n");
        return;
    }
    if (!(diago.halInfo->hsim|diago.halInfo->hw)) {
        gdbg_printf("WARNING: skipping DIAG_DIFFSCREEN because HSIM or HW is not active\n");
        return;
    }
    if (!diago.halInfo->csim) {
        gdbg_printf("WARNING: skipping DIAG_DIFFSCREEN because CSIM is not active\n");
        return;
    }
    if (!diago.halInfo->csimio) {
        gdbg_printf("WARNING: skipping DIAG_DIFFSCREEN because CSIMIO is not active\n");
        return;
    }
    if (diago.trexStandAlone) {
        gdbg_printf("WARNING: skipping DIAG_DIFFSCREEN because TREX_STANDALONE is active\n");
        return;
    }
    if (buffer==0 && diago.videoTest) {
        gdbg_printf("WARNING: skipping DIAG_DIFFSCREEN(0) because video testing\n");
        return;
    }

    // turn off level 199 debug messages (mem reads) unless level 301 is specified
    dbg199 = GDBG_GET_DEBUGLEVEL(199);
    GDBG_SET_DEBUGLEVEL(199,GDBG_GET_DEBUGLEVEL(301));

    // translate from physical to virtual buffer number
    buffer = sstRollFwdBufId(buffer);

    // HACK: use upper 8 bits of goodMask for alpha/Z comparison mask
    compareMask = diago.goodMask;
    if (buffer == CSIM_BUF_3D_AUX1) compareMask >>= 24;

    if(diago.aaEnabled)
      sampleIndexMax = diago.aaSampleCount;
    else
      sampleIndexMax = 1;

    for(sampleIndex=0; sampleIndex<sampleIndexMax; sampleIndex++)
      {
	if(diago.aaEnabled)
	  gdbg_info(1,"DIAG_DIFFSCREEN  sampleIndex=%d\n", sampleIndex);	

	if (diago.halInfo->hw)
	  HW_PIXEL_FAST_BEGIN(0, buffer);

	for (y=0; y<=ymax; y++)
	  for (x=0; x<=xmax; x++)
	    {
	      // NOTE: this isn't quite kosher, we assume HW and HSIM are mutually
	      // exclusive and we compare against only one of them
	      cfb = csimPixelRead(sampleIndex, buffer, x, y);
	      if (diago.halInfo->hw)
		hfb = HW_PIXEL_RD(buffer,x,y);
	      else
		hfb = hsimPixelRead(sampleIndex, buffer, x, y);
	      
	      cfb &= compareMask;
	      hfb &= compareMask;
	      if (cfb != hfb) {
		gdbg_printf("ERROR(diff): pixel[%d,%d] mismatch CSIM:%d(0x%x) %s:%d(0x%x)\n",
			    x,y,cfb,cfb,
			    diago.halInfo->hw ? "HW" : "HSIM",
			    hfb,hfb);
		diago.errorCount++;
		if (diago.errorCount >= diago.errorLimit) {
		  gdbg_printf("WARNING: error count of %d exceeded, aborting DIFFSCREEN...\n",
			      diago.errorLimit);
		  goto qexit;
		}
	      }
	    }      
      qexit:
	if (diago.halInfo->hw)
	  HW_PIXEL_FAST_END(0);
      }
    GDBG_SET_DEBUGLEVEL(199,dbg199);  // restore debug level
    gdbg_info(2,"\t\t\t\t  end: errors=%d/%d\n", diago.errorCount,diago.errorLimit);
}

// diff the entire screen to make sure the HW matches CSIM
// returns 0 on success (or if haven't reached error limit yet)
int DIAG_DIFFSCREEN(int xmax, int ymax)
{
    DIAG_IDLE();
    DIAG_DIFFSCREEN_EX(xmax,ymax,CSIM_BUF_3D_FRONT);         // check original (pre-swap) front buffer
    if (!diago.gui) {
        DIAG_DIFFSCREEN_EX(xmax,ymax,CSIM_BUF_3D_BACK);      // check original (pre-swap) back buffer
        DIAG_DIFFSCREEN_EX(xmax,ymax,CSIM_BUF_3D_AUX1);
        if (diago.triple)
          DIAG_DIFFSCREEN_EX(xmax,ymax,CSIM_BUF_3D_TRIPLE);  // check original (pre-swap) triple buffer
    }
    return diago.errorCount >= diago.errorLimit;
}

// diff the entire memory to make sure the HW matches CSIM
int DIAG_DIFFMEMORY(void)
{
    int i, start, stop,dbg199;
    FxU32 *hMem, *cMem, cfb, hfb, chipIndex;
    CsimPrivate *cp;

    gdbg_info(1,"DIAG_DIFFMEMORY()\tbegin: errors=%d/%d\n", diago.errorCount,diago.errorLimit);	      

    if (!(diago.halInfo->hsim || diago.halInfo->hw)) {
#if defined(HAL_HSIM) || defined(HAL_HW)
        gdbg_printf("WARNING: skipping DIAG_DIFFMEMORY because HSIM or HW is not active\n");
#endif
        return 0;
    }
    if (!diago.halInfo->csim) {
        gdbg_printf("WARNING: skipping DIAG_DIFFMEMORY because CSIM is not active\n");
        return 0;
    }
    if (!diago.halInfo->csimio) {
        gdbg_printf("WARNING: skipping DIAG_DIFFMEMORY because CSIMIO is not active\n");
        return 0;
    }
    if (diago.trexStandAlone) {
        gdbg_printf("WARNING: skipping DIAG_DIFFMEMORY because TREX_STANDALONE is active\n");
        return 0;
    }
    DIAG_IDLE();

    // turn off level 199 debug messages (mem reads) unless level 301 is specified
    dbg199 = GDBG_GET_DEBUGLEVEL(199);
    GDBG_SET_DEBUGLEVEL(199,GDBG_GET_DEBUGLEVEL(301));

    for(chipIndex = 0; chipIndex<CSIM_PRIVATE(diago.sstCSIM)->environment.chipCount; chipIndex++)
      {
	if(chipIndex == 0)
	  cp = CSIM_PRIVATE(diago.sstCSIM);
	else
	  cp = CSIM_PRIVATE(diago.sstChildrenCSIM[chipIndex-1]);
	
	cMem = (FxU32 *)cp->memory;
	
	if (diago.halInfo->hsim) {
	  hMem = (FxU32 *)cp->hsim_memory;
	  
	  start=0;
	  stop =cp->memorySizeInBytes;
	  
	  for (i=start; i<stop; i+=4) {
	    cfb = csimMemoryRead(chipIndex, i, 4);
	    hfb = hsimMemoryRead(chipIndex, i, 4);
	    if (cfb != hfb) {
	      gdbg_printf(
			  "ERROR(diff): mem32 at 0x%x mismatch CSIM:%d(0x%x) HSIM:%d(0x%x) in \"%s\" %s\n",
			  i,cfb,cfb,hfb,hfb, getName(i), cp->environment.name);
	      DIAG_INCERROR();
	    }
	  }
	}
	
	if (diago.halInfo->hw) {
	  
	  // first diff csim and hw memory quick and dirty
	  // if it fails, then step thru again and print error messages
	  // this optimizes the common case, i.e., no errors
	  if ( HW_MEM_CMP2(chipIndex,
			   0,
			   (FxU8*)cp->memory,
			   cp->memorySizeInBytes) != 0 ) {  
	
	    start=0;
	    stop =cp->memorySizeInBytes;
	    
	    HW_PIXEL_FAST_BEGIN(0, 0);
	    for (i=start; i<stop; i+=4) {
	      cfb = csimMemoryRead(chipIndex, i, 4);
	      hfb = HW_MEM_RD(chipIndex, i,4);
	      if (cfb != hfb) {
		gdbg_printf("ERROR(diff): mem32 at 0x%x mismatch CSIM:%d(0x%x) HW:%d(0x%x) in \"%s\"\n",
			    i,cfb,cfb,hfb,hfb, getName(i));
		DIAG_INCERROR();
	      }
	    }
	    HW_PIXEL_FAST_END(0);
	    
	  }
	}
      }

    GDBG_SET_DEBUGLEVEL(199,dbg199);  // restore debug level

    gdbg_info(1,"DIAG_DIFFMEMORY()\tend: errors=%d/%d\n", diago.errorCount,diago.errorLimit);	      

    return diago.errorCount >= diago.errorLimit;
}

//
// read CRC and check against golden value
//
int DIAG_TEST_CRC(FxU32 goldenCrc) 
{
  FxU32 crc = DIAG_READ_CRC_COMPOSITE();
  GDBG_INFO(0,"DIAG_TEST_CRC: read 0x%x, expected 0x%x\n",crc,goldenCrc);
  if ( crc != goldenCrc ) {
    GDBG_ERROR("DIAG_TESTREAD_CRC_COMPOSITE","CRC mismatch, got 0x%x, expected 0x%x\n",crc,goldenCrc);
    return 1;
  } 

  return 0;
}

//
// read CRC and check against golden value
//
FxU32 DIAG_READ_CRC_COMPOSITE()
{
  CsimPrivate *cpriv = CSIM_PRIVATE(diago.sstCSIM);
  FxU32 fbiVideoWidth, fbiVideoHeight;
  FxU32 crc, base, size, bpp, stride;
  FxU32 crcComposite = 0;

  // set special scanout mode
  fbiVideoWidth = 1024;
  fbiVideoHeight = 2048;

  if (diago.halInfo->hw) {

#define CRC_RES 1600
#define CRC_BPP 3

#if CRC_RES == 800
    fbiVideoWidth = 800;
    fbiVideoHeight = 600;
    fxHalInitVideo(diago.sst,GR_RESOLUTION_800x600,GR_REFRESH_60Hz,NULL);
#elif CRC_RES == 1024
    fbiVideoWidth = 1024;
    fbiVideoHeight = 768;
    fxHalInitVideo(diago.sst,GR_RESOLUTION_1024x768,GR_REFRESH_60Hz,NULL);
#elif CRC_RES == 1280
    fbiVideoWidth = 1280;
    fbiVideoHeight = 1024;
    fxHalInitVideo(diago.sst,GR_RESOLUTION_1280x1024,GR_REFRESH_60Hz,NULL);
#elif CRC_RES == 1600
    fbiVideoWidth = 1600;
    fbiVideoHeight = 1200;
    fxHalInitVideo(diago.sst,GR_RESOLUTION_1600x1200,GR_REFRESH_60Hz,NULL);
#else
    fbiVideoWidth = 640;
    fbiVideoHeight = 480;
    fxHalInitVideo(diago.sst,GR_RESOLUTION_640x480,GR_REFRESH_60Hz,NULL);
#endif

#if CRC_BPP == 3
    bpp = 3;
#else
    bpp = 2;
#endif

    stride = (fbiVideoWidth-5)*bpp;
    size = fbiVideoHeight * stride;

    sstInitVideoOverlay(diago.sst,
			0,				// 1=enable Overlay surface (OS), 1=disable
			0,				// 1=enable OS stereo, 0=disable
			0,				// 1=enable horizontal scaling, 0=disable
			0,				// horizontal scale factor (ignored if not scaling)
			0,				// 1=enable vertical scaling, 0=disable
			0,				// vertical scale factor (ignored if not scaling)
			0,				// filter mode
			0,				// 0=OS linear, 1=tiled
			SST_OVERLAY_PIXEL_RGB565D,	// pixel format of OS
			0,				// bypass clut for OS?
			0,				// 0=lower 256 CLUT entries, 1=upper 256
			diagfb.colBufferAddr[0],	// board address of beginning of OS
			256,	// distance between scanlines of the OS
			diago.xmaxscreen*2);          // overlay width in bytes

    // wait for first glitch frame after vga video timing change
    fxHalVsyncNot(diago.sst); // look for vsync falling edge
    fxHalVsync(diago.sst);    // look for vsync rising edge
    
    // wait one more frame for paranoia's sake
    fxHalVsyncNot(diago.sst); // look for vsync falling edge
    fxHalVsync(diago.sst);    // look for vsync rising edge
    
    for ( base=0x0; base<MBYTE(cpriv->info->fbiMemSize); base+=size ) {
      if ( base+size > MBYTE(cpriv->info->fbiMemSize) )
	base = MBYTE(cpriv->info->fbiMemSize) - size;
      
      sstInitVideoDesktop(
			  diago.sst,
			  1,
			  0,
			  bpp == 3 ? SST_DESKTOP_PIXEL_RGB24 : SST_DESKTOP_PIXEL_RGB565,
			  1,
			  0,
			  base,
			  stride
			  );
      
      sst_idle_really(diago.sst);
      
      crc = DIAG_READ_CRC();
      crcComposite = DIAG_CRC_ITER(crcComposite,crc);
      
    }

    // add in pixel statistics

    crcComposite = DIAG_CRC_ITER(crcComposite,GET(diago.sst->stats.fbiPixelsIn));
    crcComposite = DIAG_CRC_ITER(crcComposite,GET(diago.sst->stats.fbiChromaFail));
    crcComposite = DIAG_CRC_ITER(crcComposite,GET(diago.sst->stats.fbiZfuncFail));
    crcComposite = DIAG_CRC_ITER(crcComposite,GET(diago.sst->stats.fbiAfuncFail));
    crcComposite = DIAG_CRC_ITER(crcComposite,GET(diago.sst->stats.fbiPixelsOut));
    crcComposite = DIAG_CRC_ITER(crcComposite,GET(diago.sst->fbiTrianglesOut));

    GDBG_INFO(20,"read crcComposite = 0x%x\n\n",crcComposite);
  }

  return crcComposite;
}  

//
// read CRC and check against golden value
//
FxU32 DIAG_CALC_CRC_COMPOSITE()
{
  CsimPrivate *cpriv = CSIM_PRIVATE(diago.sstCSIM);
  FxU32 fbiVideoWidth, fbiVideoHeight;
  FxU32 crc, base, size, bpp, stride;
  FxU32 crcComposite = 0;
  FxU32 fbiPixelsIn, fbiChromaFail, fbiZfuncFail, fbiAfuncFail, fbiPixelsOut, fbiTrianglesOut;

  // set special scanout mode
#if CRC_RES == 800
  fbiVideoWidth = 800;
  fbiVideoHeight = 600;
#elif CRC_RES == 1024
    fbiVideoWidth = 1024;
    fbiVideoHeight = 768;
#elif CRC_RES == 1280
    fbiVideoWidth = 1280;
    fbiVideoHeight = 1024;
#elif CRC_RES == 1600
  fbiVideoWidth = 1600;
  fbiVideoHeight = 1200;
#else 
  fbiVideoWidth = 640;
  fbiVideoHeight = 480;
#endif  

#if CRC_BPP == 3
    bpp = 3;
#else
    bpp = 2;
#endif

  stride = (fbiVideoWidth-5)*bpp;
  size = fbiVideoHeight * stride;
  
  for ( base=0x0; base<MBYTE(cpriv->info->fbiMemSize); base+=size ) {
    if ( base+size > MBYTE(cpriv->info->fbiMemSize) )
      base = MBYTE(cpriv->info->fbiMemSize) - size;
    crc = DIAG_CALC_CRC(base,fbiVideoWidth,fbiVideoHeight,stride,bpp);
    crcComposite = DIAG_CRC_ITER(crcComposite,crc);
  }

  // add in pixel statistics
  
  GET(diago.sst->stats.fbiPixelsIn);
  fbiPixelsIn = diago.halInfo->csimLastRead;
  
  GET(diago.sst->stats.fbiChromaFail);
  fbiChromaFail = diago.halInfo->csimLastRead;
  
  GET(diago.sst->stats.fbiZfuncFail);
  fbiZfuncFail = diago.halInfo->csimLastRead;
  
  GET(diago.sst->stats.fbiAfuncFail);
  fbiAfuncFail = diago.halInfo->csimLastRead;
  
  GET(diago.sst->stats.fbiPixelsOut);
  fbiPixelsOut = diago.halInfo->csimLastRead;
  
  GET(diago.sst->fbiTrianglesOut);
  fbiTrianglesOut = diago.halInfo->csimLastRead;

  crcComposite = DIAG_CRC_ITER(crcComposite,fbiPixelsIn);
  crcComposite = DIAG_CRC_ITER(crcComposite,fbiChromaFail);
  crcComposite = DIAG_CRC_ITER(crcComposite,fbiZfuncFail);
  crcComposite = DIAG_CRC_ITER(crcComposite,fbiAfuncFail);
  crcComposite = DIAG_CRC_ITER(crcComposite,fbiPixelsOut);
  crcComposite = DIAG_CRC_ITER(crcComposite,fbiTrianglesOut);
  
  GDBG_INFO(20,"calc crcComposite = 0x%x\n\n",crcComposite);
  return crcComposite;
}  

FxU32 DIAG_CRC_ITER(FxU32 crc, FxU32 data)
{
  FxU32 nextCrc;
  FxU32 data_0, data_31_1, crc_30_0, crc_4, crc_28, crc_27, crc_1, crc_0;
  
  crc_0 = (crc>>0) & 0x1;
  crc_1 = (crc>>1) & 0x1;
  crc_4 = (crc>>4) & 0x1;
  crc_27 = (crc>>27) & 0x1;
  crc_28 = (crc>>28) & 0x1;
  
  crc_30_0 = crc & SST_MASK(31);
  
  data_0 = (data>>0) & 0x1;
  
  data_31_1 = (data>>1) & SST_MASK(31);
  
  nextCrc = (crc_30_0 ^ data_31_1) << 1;
  nextCrc |= (crc_4 ^ crc_28 ^ crc_27 ^ crc_1 ^ crc_0 ^ data_0) & 0x1;

#if 0
    printf("crc(4,28,27,1,0) samp(0) nextCrc(0) -> (%d %d %d %d %d) (%d) (%d)\n",
      crc_4,crc_28,crc_27,crc_1,crc_0,samp_0,nextCrc&0x1);
    printf("crc=0x%x, nextCrc=0x%x\n",crc,nextCrc);
#endif

  return nextCrc;
}

//
// compute CRC
//
FxU32 DIAG_CALC_CRC(FxU32 base, FxU32 width, FxU32 height, FxU32 stride, FxU32 bpp)
{
  FxU32 i, samp, crc, nextCrc;
  FxU32 data, r, g, b, r8, g8, b8;
  CsimPrivate *cp = CSIM_PRIVATE(diago.sstCSIM);
  FxU32 x, y, addr;
  FxU32 skew = 5;
  FxU16 *cMem16;
  crc = 0;
#define CRC_FIXED_BITS ( BIT(24) | BIT(25) | BIT(26) )

  cMem16 = (FxU16 *)cp->memory;

  for ( y=0; y<height; y++ ) {

    for ( i=0; i<skew; i++ ) 
      crc = DIAG_CRC_ITER(crc,CRC_FIXED_BITS);

    for ( x=0; x<width-skew; x++ ) {

      addr = base + y*stride + x*bpp;

      // sample = {5'b00000, blank_delay_del1, vsync_delay, hsync_delay,
      // 		 red[7:0], grn[7:0], blu[7:0]}
      // for valid sample --
      // 1. blank = 0
      // 2. vsync = ?
      // 3. hsync = ?  use 3cc 6&7 to determine the ?
      
      if ( bpp == 2 ) {
	// data = csimReadMem16(cp,addr);
	data = cMem16[addr>>1];
	r = (data>>11) & SST_MASK(5);
	g = (data>>5) & SST_MASK(6);
	b = (data>>0) & SST_MASK(5);
	
	r8 = (r<<3) | (r>>2);
	g8 = (g<<2) | (g>>4);
	b8 = (b<<3) | (b>>2);
	
	samp = (r8<<16) | (g8<<8) | (b8<<0);
      } else if ( bpp == 3 ) {
	data = csimReadMem24(cp,0,addr); // linear access
	samp = data;
      } else {
	GDBG_ERROR("DIAG_CALC_CRC","invalid bpp = %d\n",bpp);
	DIAG_FAIL();
      }

      samp |= CRC_FIXED_BITS;
      
      nextCrc = DIAG_CRC_ITER(crc,samp);
      
      crc = nextCrc;
    }
  }
 
  GDBG_INFO(20,"DIAG_CALC_CRC = 0x%x, bpp=%d\n",crc,bpp);
  
  return crc;
}

// 
// read CRC for full memory contents
//
FxU32 DIAG_READ_CRC()
{
  SstCRegs *sstc = (SstCRegs *)(SST_CMDAGP_ADDRESS(diago.sst));
  SstIORegs *sstio = (SstIORegs *)(SST_IO_ADDRESS(diago.sst));
  FxU32 t, crc=0, count=0;
  int i;

#if !defined H3_A0 && !defined H3_A1 && !defined H3_A2
  if (diago.halInfo->hw) {
#if 1
    // start computing crc 
    fxHalVsyncNot(diago.sst); // look for vsync falling edge
    fxHalVsync(diago.sst);    // look for vsync rising edge
    // crc computation completed, latch value into sstc->crc2 register
    fxHalVsyncNot(diago.sst); // look for vsync falling edge
    fxHalVsync(diago.sst);    // look for vsync rising edge

    for ( i=0; i<20; i++ ) {
      
      t = GET(sstc->crc2);    // read crc value
      
      GDBG_INFO(5,"DIAG_READ_CRC: crc = 0x%x\n",t);

      if ( i == 0 || t != crc ) {
	crc = t;
	count = 1;
      } else if ( ++count >= 3 ) {
	break;
      }
      
    }
    
    if ( count < 3 ) {
      GDBG_ERROR("DIAG_READ_CRC","unable to read consistent CRC value\n");
      DIAG_FAIL();
    }
#else 
    FXUNUSED(i);
    FXUNUSED(t);
    fxHalVsyncNot(diago.sst); // look for vsync falling edge
    fxHalVsync(diago.sst);    // look for vsync rising edge
    
    fxHalVsyncNot(diago.sst); // look for vsync falling edge
    fxHalVsync(diago.sst);    // look for vsync rising edge
    crc = GET(sstc->crc2);    // read crc value
#endif
  }
#endif

  GDBG_INFO(20,"DIAG_READ_CRC = 0x%x\n",crc);

  return crc;

}

// read and test a mem location for a certain value
// returns 0 if the same, 1 if different
FxI32 diagTestMemory(FxI32 chipIndex, FxU32 addr, FxU32 cGood, FxI32 nBytes)
{
  int err = 0;
  FxU32 cRead;
  
  // first check the software simulator
  if (diago.halInfo->csim) {
    cRead = csimMemoryRead(chipIndex, addr, nBytes);
    err |= DIAG_COMPARE_MEM("csim",addr,cRead,cGood);
  }
  // then check the hardware simulator
  if ( diago.halInfo->hsim ) {
    cRead = hsimMemoryRead(chipIndex, addr, nBytes);
    err |= DIAG_COMPARE_MEM("hsim",addr,cRead,cGood);
  }
  // then check the real hw
  if (diago.halInfo->hw) {
    cRead = HW_MEM_RD(chipIndex, addr,nBytes);
    err |= DIAG_COMPARE_MEM("hw",addr,cRead,cGood);
  }
  return err;
}

// read and test a pixel for a certain value
// returns 0 if the same, 1 if different
FxI32 diagTestPixel(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxU32 good)
{
    int err = 0;
    FxU32 cfb = good;
    
    if (!ONSCREEN(x,y))                 // if offscreen then forget it
      return 0;
    if ((diago.halInfo->hsim & HSIM_TREX_STANDALONE) || !diago.checkEveryTriangle)
      return 0;

    // first check the software simulator
    if (diago.halInfo->csim) {
      cfb = csimPixelRead(sampleIndex, buffer, x, y);
      err |= DIAG_COMPARE_PIXEL("csim",buffer,x,y,cfb,good);
    }
    // then check the hardware simulator
    if (diago.halInfo->hsim) {      
      cfb = hsimPixelRead(sampleIndex, buffer, x, y);
      err |= DIAG_COMPARE_PIXEL("hsim",buffer,x,y,cfb,good);
    }
    // then check the real hw
    if (diago.halInfo->hw) {
      cfb = HW_PIXEL_RD(buffer,x,y);
      err |= DIAG_COMPARE_PIXEL("hw",buffer,x,y,cfb,good);
    }
    return err;
}



// read and test a pixel for a certain value
// returns 0 if the same, 1 if different
int DIAG_TEST_PIXEL(FxI32 buffer, int x, int y, FxU32 good)
{
  return(diagTestPixel(0, buffer, x, y, good));
}

// read and test a mem location for a certain value
// returns 0 if the same, 1 if different
int DIAG_TEST_MEM(FxU32 addr, FxU32 cGood, int nbytes)
{       
  return(diagTestMemory(0, addr, cGood, nbytes));
}

// test a 32-bit LFB read value where the caller did a true GET()
// this is complicated because the simulator reads both
// the csim and hsim values, returns the hsim and saves the csim in the HAL data
// BUG: this won't use the right compare mask for AUX buffer compares
int DIAG_TESTLFBREAD32(int x, int y, FxU32 cRead, FxU32 cGood)
{       
    int err = 0;
        
    GUI_CHECK();
    if (diago.halInfo->hsim || diago.halInfo->hw) { // first check hardware/simulator
        err |= DIAG_COMPARE_PIXEL("hsim.lfb32",diago.curdrawbuffer,x,y,cRead,cGood);
        cRead =  diago.halInfo->csimLastRead;   // then check software simulator
    }
    return err | DIAG_COMPARE_PIXEL("csim.lfb32",diago.curdrawbuffer,x,y,cRead,cGood);
}

// test a 16-bit LFB read value where the caller did a true GET()
// this is complicated because the simulator reads both
// the csim and hsim values, returns the hsim and saves the csim in the HAL data
// BUG: this won't use the right compare mask for AUX buffer compares
int DIAG_TESTLFBREAD16(int x, int y, FxU32 cRead, FxU32 cGood)
{       
    int err = 0;
        
    GUI_CHECK();
    if (diago.halInfo->hsim || diago.halInfo->hw) { // first check hardware/simulator
        err |= DIAG_COMPARE_PIXEL("hsim.lfb16",diago.curdrawbuffer,x,y,cRead,cGood);
        cRead =  diago.halInfo->csimLastRead;   // then check software simulator
        if (x & 1) cRead >>= 16;                // get high word if odd x
        cRead = ((cRead & 0xF800)<<8) | ((cRead & 0x07E0)<<5) | ((cRead & 0x001F)<<3);
    }
    return err | DIAG_COMPARE_PIXEL("csim.lfb16",diago.curdrawbuffer,x,y,cRead,cGood);
}

//----------------------------------------------------------------------
// Wait until the Graphics engine is idle and the fifo is empty 
//      NOTE: if we are not checking every triangle then just return
//----------------------------------------------------------------------
void sst_idle(SstRegs *sst)
{
    if (diago.checkEveryTriangle)
        sst_idle_really(sst);
}

void sst_idle_really(SstRegs *sst)
{
    FxU32 s;
    SstRegs *actualSst;
    FxI32 chipIndex; 

    if (shutdown)               // once we are shutdown
        return;                 // don't bother anymore
    if (diago.writeFifo && diago.cmdFifoEnabled)
        SET(sst->nopCMD,0);     // just in case we are in CMD FIFO mode
    hb_histFlushAll();          // flush command packet history buffer

    s = getSeed();              // save seed
    setSeed(s);                 // restore seed
    
    for(chipIndex=0; chipIndex<diago.chipCount; chipIndex++)
      {
	actualSst = diago.halInfo->boardInfo[chipIndex].virtAddr[0];
	
	if(chipIndex == 0)
	  assert(sst == actualSst);

	//This used to be set up to randomly idle the 2d or 3d section 
	//of the chip. This didn't work though because sometimes a
	//triangle wouldn't be drawn before checkTriangle was called.
	//Consequently, checkTriangle() would fail like a bastard.
	if (diago.writeFifo && diago.cmdFifoEnabled)
	  {
	    fxHalIdleNoNop(actualSst);  //Idle the 3d section
	    fxHalIdleNoNop2(actualSst); //Idle the 2d section
	  }
	else
	  {
	    fxHalIdle(actualSst);   //Idle the 3d section
	    fxHalIdle2(actualSst);  //Idle the 2d section
	  }
      }
    
#ifdef HAL_HSIM
    // all TREX data should be dumped out by now, so check it
    trexFifoCheckAllCsim();
#endif
}

//----------------------------------------------------------------------
// Wait until vsync -- used for testing video output, when vsync 
//     occurs then previous frame has been checked and the
//     emulation library can generate the next expected frame.
//     Note: this code is based on the "idle" code and might 
//           be doing unnecessary work.
//----------------------------------------------------------------------

void sst_vsync_active(SstRegs *sst)
{
    if (shutdown)               // once we are shutdown
        return;                 // don't bother anymore
    fxHalVsync(sst);
}
void sst_vsync_inactive(SstRegs *sst)
{
    if (shutdown)               // once we are shutdown
        return;                 // don't bother anymore
    fxHalVsyncNot(sst);
}


//----------------------------------------------------------------------
// set diago.curdrawbuffer to either 0 (front) or 1 (back) depending on
// diago.drawbuffer.  If diago.drawbuffer is non-negative we simply copy
// its value, otherwise we generate a random number between 0 and 1
// we return the appropriate fbzMode DRAWBUFFER bit
//----------------------------------------------------------------------
unsigned long drawbufferRandom(void)
{
    diago.curdrawbuffer = diago.drawbuffer;
    if (diago.curdrawbuffer < 0)
        diago.curdrawbuffer = iRandom(1);               // choose random
#ifdef CVG
    return diago.curdrawbuffer ? SST_DRAWBUFFER_BACK : SST_DRAWBUFFER_FRONT;
#else // H3
    // Notes: 
    //  1. This only works because the diags are written to 
    //     unconditionally switch to the buffer returned by this
    //     routine
    //  2. Always return zero, since the return value of 
    //     drawbufferRandom() is often used to set a bit in fbzMode 
    //     (a bit that doesn't exist for Banshee)
    drawbufferSet(diago.curdrawbuffer);
    return 0;
#endif
        
}

//----------------------------------------------------------------------
// routine to test reading and writing registers
// these are typically called from register32test
//----------------------------------------------------------------------
void test32(volatile FxU32 *reg, FxU32 mask, FxU32 val)
{
    char addr[32];
    unsigned long got;

    SET(*reg,val);
    sst_idle_really(diago.sst);

    sprintf(addr,"0x%x",(unsigned int)reg);
    got = GET(*reg) & mask;
    DIAG_TESTREG32(addr,val&mask,got);
}

//----------------------------------------------------------------------
// return floating point representation of W which is 1/oow
// same as wFloat64, but input is 2.30
unsigned long wBufferValue( unsigned long X )
{
    if ( X & 0xC0000000 )               // Check for negative
        return 0;
    return wFloat64(((FxI64)X)<<2);	// convert to 16.32
}

//----------------------------------------------------------------------
// return floating point representation of W which is 1/oow
unsigned long wFloat64(FxI64 oow)
{
    FxU32 X;
    unsigned int result;
    unsigned int exponent;
    

    if (oow < 0) return 0;              // if w is negative return 0
    if (oow > 0xFFFFFFFF) return 0;     // if w >= 1.0 return 0
    if (oow == 0)                        // Check for 0.0
        return diago.rgb<32 ? 0xFFFF : 0xFFFFFF;

    exponent = 0;
    X = FX_LO64(oow);
    while( ! ( 0x80000000 & X ) ) {
        exponent++;
        X <<= 1;
    }
    GDBG_INFO(21,"float exp=%d\n",exponent);
    X <<= 1;                            // Remove the implied one
    if (diago.rgb < 32) {		// for safety keep old code unmodified
        if ( exponent > 15 )
	    return 0xFFFF;		// Clamp if overflow
	X >>= 32-12;			// right justify mantissa
	X ^= 0x0FFF;			// Toggle Mantissa
	X += 1;				// -> 2's complement...
	result = (X + (exponent << 12));
        if ( result > 0xFFFF )
            return 0xFFFF;              // Clamp If Overflow
    }
    else {
        if ( exponent > 31 )
	    return 0xFFFFFF;		// Clamp if overflow
	X >>= 32-19;			// right justify mantissa
	X ^= 0x7FFFF;			// Toggle Mantissa
	X += 1;				// -> 2's complement...
	result = (X + (exponent << 19));
        if ( result > 0xFFFFFF )
            return 0xFFFFFF;		// Clamp If Overflow
    }
    GDBG_INFO(21,"float res = 0x%x\n",result);
    return result;
}


//----------------------------------------------------------------------
// function to determine whether or not a give alpha mode is testable.
// note that depthbuffering and blending with destination alpha are mutually exclusive
//----------------------------------------------------------------------
int goodAlphaMode(FxU32 alphaMode, FxU32 fbzMode)
{
  unsigned char currentMode;

  // GMT: spoof the following checks
  if (diago.rgb != 16) fbzMode |= SST_ENALPHABUFFER;

  // RGB src verification
  currentMode = (unsigned char)((alphaMode & SST_RGBSRCFACT) >> SST_RGBSRCFACT_SHIFT);
  if((currentMode <= 0x0E) && (currentMode >= 0x0a))
      return 0;
  if( ((fbzMode & SST_ENDEPTHBUFFER) && (alphaMode & SST_ENALPHABLEND)) || 
        !diago.hasAuxBuffer || !(fbzMode & SST_ENALPHABUFFER)) {
        if (currentMode == SST_A_DSTALPHA || currentMode == SST_AOM_DSTALPHA ||
                currentMode == SST_A_SATURATE)
            return 0;
  }

  // RGB dst verification
    currentMode = (unsigned char)((alphaMode & SST_RGBDSTFACT) >> SST_RGBDSTFACT_SHIFT);
  if((currentMode <= 0x0F) && (currentMode >= 0x0a))
      return 0;
  if( ((fbzMode & SST_ENDEPTHBUFFER) && (alphaMode & SST_ENALPHABLEND)) ||
        !diago.hasAuxBuffer || !(fbzMode & SST_ENALPHABUFFER)) {
        if (currentMode == SST_A_DSTALPHA || currentMode == SST_AOM_DSTALPHA ||
                currentMode == SST_A_SATURATE)
            return 0;
  }
  // ALPHA verification
  if (diago.rgb == 16) {	// 16-bit 565 RGB mode
    currentMode = (unsigned char)((alphaMode & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT);
    if( (currentMode != SST_A_ZERO) && (currentMode != SST_A_ONE))
      return 0;

    currentMode = (unsigned char)((alphaMode & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT);
    if( (currentMode != SST_A_ZERO) && (currentMode != SST_A_ONE))
        return 0;
    if( (currentMode == SST_A_ONE) && !diago.hasAuxBuffer)
      return 0;
  }
  else if (diago.rgb == 32) { //8888 ARGB mode			
    // check source factor
    currentMode = (unsigned char)((alphaMode & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT);
    if(currentMode >= 0x08) 
	return 0;
    else {
	if (currentMode == SST_A_COLOR || currentMode == SST_AOM_COLOR)
	  return 0;
    }
    // check destination factor
    currentMode = (unsigned char)((alphaMode & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT);
    if(currentMode >= 0x08) 
	return 0;
    else {
	if (currentMode == SST_A_COLOR || currentMode == SST_AOM_COLOR)
	  return 0;
    }
  }
  else //1555 ARGB mode
    {
      //1555 only supports alpha factors of AONE and AZERO
      //It's different than the 16bit case because it always has an alpha channel
      currentMode = (unsigned char)((alphaMode & SST_ASRCFACT) >> SST_ASRCFACT_SHIFT);
      if( (currentMode != SST_A_ZERO) && (currentMode != SST_A_ONE))
	return 0;      
      
      currentMode = (unsigned char)((alphaMode & SST_ADSTFACT) >> SST_ADSTFACT_SHIFT);
      if( (currentMode != SST_A_ZERO) && (currentMode != SST_A_ONE))
        return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
// This determines if the randomly-generated color path is a valid one
//----------------------------------------------------------------------
int goodCcuPath(unsigned long fbzColorPath, int fbiOnly)
{
  // Other RGB select
  if( ((fbzColorPath & SST_RGBSELECT) == SST_RGBSEL_LFB) ||
        (fbiOnly&&((fbzColorPath & SST_RGBSELECT) == SST_RGBSEL_TREXOUT)))
    {
      gdbg_info(220,"  Color path invalidated by: Color RGB select    %x\n",fbzColorPath);
      return 0;
    }

  // Other Alpha select
  if(( (fbzColorPath & SST_ASELECT) == SST_ASEL_LFB) ||
        (fbiOnly&&((fbzColorPath & SST_ASELECT) == SST_ASEL_TREXOUT)))
    {
      gdbg_info(220,"  Color path invalidated by: Alpha RGB select    %x\n",fbzColorPath);
      return 0;
    }

  // Local color RGB select (all valid)

  // Local Alpha select (all valid)

  // RGB mselect
  if (fbiOnly) {
    if(((fbzColorPath & SST_CC_MSELECT) == SST_CC_MATREX) ||
	((fbzColorPath & SST_CC_MSELECT) == SST_CC_MRGBTMU) ||
	((fbzColorPath & SST_CC_MSELECT) == SST_CC_MONE6)) {
      gdbg_info(220,"  Color path invalidated by: Color mselect       %x\n",fbzColorPath);
      return 0;
    }
  }

  // Alpha mselect
  if (fbiOnly) {
    if((fbzColorPath & SST_CCA_MSELECT) == SST_CCA_MATREX) {
      gdbg_info(220,"  Color path invalidated by: Alpha mselect       %x\n",fbzColorPath);
      return 0;
    }
  }

  // Enabled TREX input
  if (fbiOnly) {
    if( fbzColorPath & SST_ENTEXTUREMAP ) {
      gdbg_info(220,"  Color path invalidated by: Enabled TREX        %x\n",fbzColorPath);
      return 0;
    }

    // Enabled SST_LOCALSELECT_OVERRIDE_WITH_ATEX
    if( fbzColorPath & SST_LOCALSELECT_OVERRIDE_WITH_ATEX ) {
      gdbg_info(220,"  Color path invalidated by: SST_LOCALSELECT_OVERRIDE_WITH_ATEX        %x\n",fbzColorPath);
      return 0;
    }
  }
  if (fbiOnly) {
    // RGB add
    if((fbzColorPath & (SST_CC_ADD_CLOCAL|SST_CC_ADD_ALOCAL)) == (SST_CC_ADD_CLOCAL|SST_CC_ADD_ALOCAL))
    {
      gdbg_info(220,"  Color path invalidated by: Invalid add mode    %x\n",fbzColorPath);
      return 0;
    }

    // Alpha add
    if((fbzColorPath & (SST_CCA_ADD_CLOCAL|SST_CCA_ADD_ALOCAL)) == (SST_CCA_ADD_CLOCAL|SST_CCA_ADD_ALOCAL))
    {
      gdbg_info(220,"  Color path invalidated by: Invalid add mode    %x\n",fbzColorPath);
      return 0;
    }
  }
  // If it makes it through all of that...
  return 1;
}

// return 1 if OK, 0 if lfb reads/writes AUX buffer and there isn't one
int lfbAuxCheck(FxU32 lmode)
{
    if (diago.hasAuxBuffer) return 1;
    if ((lmode & SST_LFB_READBUFSELECT)==SST_LFB_READDEPTHABUFFER)
        return 0;
    switch (lmode & SST_LFB_FORMAT) {
        case SST_LFB_565:
        case SST_LFB_555:
        case SST_LFB_888:
                return 1;
        case SST_LFB_1555:
        case SST_LFB_8888:
        case SST_LFB_Z565:
        case SST_LFB_Z555:
        case SST_LFB_Z1555:
        case SST_LFB_ZZ:
        case SST_LFB_Z32:
                return 0;
    }

    assert(0);
    return(-1);
}

// return 1 if OK, 0 if no good
int goodLfbMode(unsigned long lm)
{
  if(diago.chipCount > 1 && (lm & SST_LFB_YORIGIN))
    return(0);
  
    if ( (lm & SST_LFB_READBUFSELECT) != SST_LFB_READCOLORBUFFER && 
         (lm & SST_LFB_READBUFSELECT) != SST_LFB_READDEPTHABUFFER ) return 0;
    if ((lm & SST_LFB_READBUFSELECT) > SST_LFB_READDEPTHABUFFER) return 0;
    switch (lm & SST_LFB_FORMAT) {
        case SST_LFB_565:
        case SST_LFB_555:
        case SST_LFB_1555:
        case SST_LFB_888:
        case SST_LFB_8888:
        case SST_LFB_Z32:
        case SST_LFB_Z565:
        case SST_LFB_Z555:
        case SST_LFB_Z1555:
        case SST_LFB_ZZ:
                return 1;
    }
    // don't allow this - it requires idling the chip before reading LFB
    return 0;
}

// Initialize the Fog Table to have random opacities and slopes
// make sure there are no overflows or underflows
FxU16 sst_random_fog_table_entry(void)
{
    FxI16 fogVal;
    FxI16 fogDelta;

    again:

        fogVal = iRandom( 0xFF );
        fogDelta = iRandom( 0xFF );
        if (fogDelta & 2) {
            if (fogVal - (fogDelta>>2) < 0) goto again;
        }
        if (fogVal + (fogDelta>>2) > 255) goto again;

        return (fogVal << 8) | fogDelta;
}

void sst_random_fog_table(SstRegs *sst, FxU32 fogTable[])
{
    FxU32 fogIndex;

    for ( fogIndex = 0; fogIndex < FOG_TABLE_SIZE; fogIndex++ )  {
        fogTable[fogIndex] = sst_random_fog_table_entry();
    }
    
    // Download Fog Table and Pack on the Fly
    for ( fogIndex = 0; fogIndex < FOG_TABLE_SIZE; fogIndex+=2 )
    {
        SET( sst->fogTable[fogIndex/2],
             (fogTable[fogIndex + 1] << 16) | fogTable[fogIndex] );
    }
}


#ifdef CVG
#define USETEXTUREMOVE(diago)  (0)
#else
#define USETEXTUREMOVE(diago)  (((diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES)==0) && diago.writeFifo && diago.usePacket6)

static FxU32 *texAgpMem = (FxU32*)0xdeadbeef;
static FxU32 texNextFree = 0;
static FxU32 texSizeBytes = 0;
#define DEFAULTTEXSIZEBYTES 524288
// AGP texture memory is reinitialised
// todo: reset condiditions ?/
static void resetAgpTexMem(SstRegs *sst)
{
  FxU32 *agpMem;
  FxU32 size;
  if (!USETEXTUREMOVE(diago)) return;
    
  if  (texAgpMem == (FxU32*)0xdeadbeef ||
       texNextFree > texSizeBytes) {
    size =  CSIM_PRIVATE(diago.sstCSIM)->info->agpSizeInBytes;
    if (size > DEFAULTTEXSIZEBYTES*2)
      texSizeBytes = DEFAULTTEXSIZEBYTES;
    else
      texSizeBytes = size/2;
    agpMem = (FxU32 *)AGPMEMALLOC(sst,texSizeBytes);
    GDBG_INFO(3,"Allocating %d bytes texture mem in AGP\n",texSizeBytes);
    if (agpMem == NULL) {
      DIAG_FAIL();
    }
    texAgpMem = agpMem;
    texNextFree = 0;
  }
}

static void genTextureMoveCmd(SstRegs *sst,FxU32 sizeBytes,FxU32 *vaddr,FxU32 *data) 
{
  int remBytes;
  int retval;
  FxU32 cBytes;
  int tt;
  FxU32 *vva;
  FxU32 *moveData,*pmem,*ddata;
  FxU32 ii;
  FxU32 maxii;
  FxU32 saveSeed;

  remBytes = sizeBytes;
  saveSeed = getSeed(); // save the random seed
  while (remBytes > 0) {
    cBytes = MIN(remBytes,SSTCP_PKT6_SRC_WIDTH & ~0xf);
    GDBG_INFO(110,"cBytes %d remBytes %d addr 0x%x\n",
                cBytes,remBytes,(FxU32)vaddr - SST_TEX_ADDRESS(sst) + (sizeBytes - remBytes));
    retval = -1;
    if (iRandom(1)) {
      
        maxii = (cBytes+3)/4; // clobber
        if (// cBytes >= 4  && // NYI for < 4 bytes
            // cBytes > 1 && 
            texNextFree + maxii*4 <= texSizeBytes) {
        moveData = texAgpMem + texNextFree/4;
        texNextFree += maxii*4;
        GDBG_INFO(110,"move from agp %x texNextfree %x\n",moveData,texNextFree);
        for (pmem=moveData,ddata = data, ii = 0; ii < maxii; ii++,pmem++ ) 
          AGPWRV(*pmem,*ddata++);

	//Napalm is f'ed up and doesn't deal with Packet 6 texture
	//space stuff correctly. It's supposed to add 0x600000,
	//but it doesn't. Consequently, the csim reflects this.
	//The address in the function should actually be
	//(FxU32)vaddr - SST_TEX_ADDRESS(sst) + (sizeBytes - remBytes)
        hb_moveCmd(sst,cBytes,cBytes,cBytes,
                   moveData,
                   (FxU32)vaddr + (sizeBytes - remBytes),
                   cBytes,
                   SSTCP_TEXPORT_SPACE);
        retval = 0;
        data = ddata;
      }
    }
    // if move 
    if (retval == -1) {
      GDBG_INFO(110,"move direct ... cBytes %d\n",cBytes);
      vva = (FxU32 *)((FxU32)vaddr + (sizeBytes - remBytes));
      for (tt=cBytes; tt>0; tt-=4) {
        SET(vva[0],*data);                      // send the texture word
        vva++;
        data++;
      }
    }
    remBytes -= cBytes ;
  }
  setSeed(saveSeed);
}
#endif

//----------------------------------------------------------------------
// local copy of stuff from sstimage.c
//----------------------------------------------------------------------


long sstDownLoadTexture(SstRegs *sst,           // base address of entire SST
                        int trex,               // which TREX chip to load [0,3]
                        long mipmapBaseAddress, // Framebuffer address where the texture should be downloaded
                        int tiled,
                        int tStride,
                        unsigned long textureMode,  // textureMode (8 bit sequential)
                        int ar,                 // aspect ratio
                        int slog,               // log2 of S size
                        int tlog,               // log2 of T size
                        int bpt,                // bytes per texel
                        FxU32 tLOD,             // tLOD for texture
                        unsigned long *data)    // the texture data
{
    long s,t, lodmax;
    long *vaddr;
    FxI32 bitsPerTexel;
    FxU32 largestPossibleLOD;
    FxU32 texturePortBaseAddress;

    //These are variables needed to relocate downloads to tmu0
    FxBool relocatingDownload;
    FxU32 actualTrex, tmu0TexBaseAddr, tmu0TextureMode, tmu0TLOD;

    //Make sure we don't try to download a tiled texture that is bigger than 1024x1024
    if(tiled && (tlog > 10 || slog > 10))
      {
	GDBG_ERROR("sstDownLoadTexture", "Tiled texture too damn big! (%dx%d)\n",
		   1<<slog, 1<<tlog);
	DIAG_FAIL();
      }

    if(bpt == 0)
      bitsPerTexel = 4;
    else
      bitsPerTexel = bpt * 8;

    //Check that textureMode has a proper format in it    
    if(SST_T4BIT_COMPRESSED(textureMode))
      assert(bitsPerTexel == 4);
    else if(SST_T8BIT_COMPRESSED(textureMode))
      assert(bitsPerTexel == 8);
    else if(SST_T8BIT(textureMode))
      assert(bitsPerTexel == 8);
    else if(SST_T16BIT(textureMode))
      assert(bitsPerTexel == 16);
    else if(SST_T32BIT(textureMode))
      assert(bitsPerTexel == 32);

    //GDBG_INFO(0, "sstDownloadTexture(sst=0x%x, trex=%d, mipmapBaseAddress=0x%x, tiled=%d, tStride=%d, textureMode=0x%x, ar=%d, slog=%d, tlog=%d bitsPerTexel=%d, tLOD=0x%x, data=0x%x\n", sst, trex, mipmapBaseAddress, tiled, tStride, textureMode, ar, slog, tlog, bitsPerTexel, tLOD, data);
    
    //Figure out if we need to relocate the download
    if((bitsPerTexel == 32 || (tLOD & SST_TBIG)) && trex != 0)
      {
	GDBG_INFO(5, "sstDownloadTexture relocating download from TMU %d to TMU 0\n", trex);
		  
	relocatingDownload = FXTRUE;
	actualTrex = trex;
	trex = 0;

	assert(actualTrex == 1);
	
	//Copy TMU0's relevant registers for texture downloads
	tmu0TexBaseAddr = shadowRegisters3D[0][1].texBaseAddr;
	tmu0TextureMode = shadowRegisters3D[0][1].textureMode;
	tmu0TLOD        = shadowRegisters3D[0][1].tLOD;

	//Write to TMU0's relevant registers for texture downloads
	SET(SST_TREX(sst, 0)->texBaseAddr, shadowRegisters3D[0][1 + actualTrex].texBaseAddr);
	SET(SST_TREX(sst, 0)->textureMode, shadowRegisters3D[0][1 + actualTrex].textureMode);
	SET(SST_TREX(sst, 0)->tLOD, shadowRegisters3D[0][1 + actualTrex].tLOD);	
      }       
    else
      {
	if(!(tLOD & SST_TBIG))
	  {
	    assert(slog <= 8);
	    assert(tlog <= 8);
	  }
	
	relocatingDownload = FXFALSE;
      }
    
    if(trex == 0)
      //Always use the big texture port for TMU 0 downloads
      texturePortBaseAddress = SST_TEX2_ADDRESS(sst);
    else if(trex == 1)
      texturePortBaseAddress = SST_TEX1_ADDRESS(sst);
    else
      assert(0);

    //Figure out whether or not we're using normal or big textures
    if(diago.bigAssTextures)
      {	
	if(tLOD & SST_TBIG)
	  {
	    largestPossibleLOD = 11; //Using 2048x2048 textures
	  }
	else
	  {
	    largestPossibleLOD = 8;  //Using 256x256 textures	
	  }
      }
    else  //Using 256x256 textures
      {
	if(tLOD & SST_TBIG)
	  {
	    GDBG_ERROR("sstDownLoadTexture", "Shit! SST_TBIG without --bigAssTextures %s(%d)\n",
		       __FILE__, __LINE__);
	    DIAG_FAIL();
	  }
	
	largestPossibleLOD = 8;
      }

    //Make sure that the slog and tlog are ok
    if((FxU32)slog > largestPossibleLOD || (FxU32)tlog > largestPossibleLOD)
      {
	GDBG_ERROR("sstDownLoadTexture", "slog or tlog is too large! slog=%d tlog=%d largest=%d %s(%d)\n",
		   slog, tlog, largestPossibleLOD, __FILE__, __LINE__);
	DIAG_FAIL();
      }

    //Make sure that tmu0 is used for 4 bytes per texel texture downloads
    if((bitsPerTexel == 32) && trex != 0)
      {
	GDBG_ERROR("sstDownloadTexture", "Can't use trex %d with 32 bit per texel textures! %s(%d)\n", 
		   trex, __FILE__, __LINE__);
	DIAG_FAIL();
      }

    //Or if using huge textures
    if(diago.bigAssTextures)
      if((tLOD & SST_TBIG) && trex != 0)
	{
	  GDBG_ERROR("sstDownloadTexture", "Can't use trex %d with big ass textures! %s(%d)\n",
		     trex, __FILE__, __LINE__);
	  DIAG_FAIL();
	}
    
    GDBG_INFO(187, "sstDownloadTexture(trex=%d, tiled=%d, tStride=%d, ar=%d, slog=%d, tlog=%d, bitsPerTexel=%d)\n",
	      trex, tiled, tStride, ar, slog, tlog, bitsPerTexel);


    // NOTE: ar is not necessarily slog-tlog
    if (slog >= tlog) 
      lodmax = largestPossibleLOD-slog;  // s is wider (or square)      
    else        
      lodmax = largestPossibleLOD-tlog; // t is wider
    assert(lodmax >= 0);
    assert(lodmax <= (FxI32)largestPossibleLOD);

    if (ar > 3 || ar < 0)
      {
        GDBG_ERROR("sstDownLoadTexture","invalid aspect ratio: %d by %d\n",
		   1<<slog,1<<tlog);
	DIAG_FAIL();
      }

    if (trex > 1)
      GDBG_ERROR("sstDownLoadTexture","TMU %d is invalid, only 0 and 1 are valid\n",trex);
    else
      {
	char *bitsPerTexelString;
	char bitsPerTexelString4[]="4b";
	char bitsPerTexelString8[]="8b";
	char bitsPerTexelString16[]="16b";
	char bitsPerTexelString32[]="32b";
	char unknown[]="???";
	
	bitsPerTexelString=unknown;
	if(bitsPerTexel==4)
	  bitsPerTexelString=bitsPerTexelString4;
	else if(bitsPerTexel==8)
	  bitsPerTexelString=bitsPerTexelString8;
	else if(bitsPerTexel==16)
	  bitsPerTexelString=bitsPerTexelString16;
	else if(bitsPerTexel==32)
	  bitsPerTexelString=bitsPerTexelString32;
	
	GDBG_INFO(110,"sstDownLoadTexture(trex=%d,mipmapBaseAddress=0x%x,log=%d,%d,%s,0x%x)\n",
		  trex,mipmapBaseAddress,slog,tlog, bitsPerTexelString,data);
      }

    assert((mipmapBaseAddress & (~SST_TEXTURE_FULL_ADDRESS)) == 0);
    mipmapBaseAddress &= SST_TEXTURE_FULL_ADDRESS;

    // Calculate and set texBaseAddr
    if(tiled) 
      {                              
	FxU32 texBaseAddr, depth;
	FxI32 u,v;
	tiledStruct mipmap;
	
	mipmap = sstTiledMipMapOffset2(lodmax, tLOD, textureMode);
	u=mipmap.uoff;
	v=mipmap.voff;

	if(textureMode & SST_COMPRESSED_TEXTURES)
	  {
	    depth = 16;

	    //Convert to microtile
	    if(SST_T4BIT_COMPRESSED(textureMode))
	      {
		if(u > 0)
		  u /= 8;
		else
		  u = (u - 7)/8;
	      }
	    else if(SST_T8BIT_COMPRESSED(textureMode))
	      {
		if(u > 0)
		  u /= 4;
		else
		  u = (u - 3)/4;
	      }
	    else
	      assert(0);
	    
	    if(v > 0)
	      v /= 4;	    	    
	    else
	      v = (v - 3) / 4;
	  }
	else
	  depth = bitsPerTexel / 8;

	texBaseAddr = tiledAddress(mipmapBaseAddress, tStride, depth, -u, -v);
	
	GDBG_INFO(187, "lod=%d u=%d v=%d tStride=%d depth=%d mipmapBaseAddress=0x%x\n",
		  lodmax, u, v, tStride, depth, mipmapBaseAddress);
	GDBG_INFO(187, "texBaseAddr = 0x%x\n", texBaseAddr);
	GDBG_INFO(187, "tiledAddress(0x%x, %d, %d, %d, %d) = 0x%x\n",
		  texBaseAddr, tStride, depth, u, v,
		  tiledAddress(texBaseAddr, tStride, depth, u, v));

	if(texBaseAddr & 0xF)                           // check for 16-byte alignment
	  GDBG_ERROR("sstDownLoadTexture","tiled invalid (unaligned-16) texture base address=0x%x\n", texBaseAddr);


	texBaseAddr = SST_TEXTURE_MUNGE_ADDRESS(texBaseAddr) | 
	  SST_TEXTURE_IS_TILED | (tStride<<SST_TEXTURE_TILESTRIDE_SHIFT);
	SET(SST_TREX(sst,trex)->texBaseAddr,texBaseAddr);
	
	//Write the value to the correct TMU in case something else depends on it later
	if(relocatingDownload)
	  SET(SST_TREX(sst,actualTrex)->texBaseAddr, texBaseAddr);	
	
	GDBG_INFO(110,"texBaseAddr 0x%x\n",texBaseAddr);
      } 
    else  //Linear
      {    
  	FxU32 texBaseAddr;
	
	texBaseAddr = mipmapBaseAddress - sstLinearMipMapOffset2(lodmax, tLOD, textureMode);

	if(texBaseAddr & 0xF)                           // check for 16-byte alignment
	  GDBG_ERROR("sstDownLoadTexture","linear invalid (unaligned-16) texture base address=0x%x\n", texBaseAddr);

	//Munge into proper format
	texBaseAddr = SST_TEXTURE_MUNGE_ADDRESS(texBaseAddr);
	
	SET(SST_TREX(sst,trex)->texBaseAddr, texBaseAddr);
	
	//Write the value to the correct TMU in case something else depends on it later
	if(relocatingDownload)
	  SET(SST_TREX(sst,actualTrex)->texBaseAddr, texBaseAddr);
	
	GDBG_INFO(110,"texBaseAddr munged 0x%x\n", texBaseAddr);
      }
    resetAgpTexMem(sst);
    
    if (diago.halInfo->hsim & HSIM_TREX_BACKDOOR_TEXWRITES) {
      sst_idle_really(sst);
    }
        
    if ( tiled ) {
      //////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////
      //
      //             Tiled download
      //
      //////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////
      FxU8 *data8 = (FxU8 *) data;
      FxU16 *data16 = (FxU16 *) data;
      FxU32 *data32 = (FxU32 *) data;
      int smax = 1<<slog;
      int tmax = 1<<tlog;
      int pack;

      if(textureMode & SST_COMPRESSED_TEXTURES)
	{
	  ///////////////////////////////////////////////////////////////////////
	  //            Compressed textures
	  ///////////////////////////////////////////////////////////////////////
	  FxU32 wordSelect;

	  //Convert s and t into microtile coords, instead of texel coords
	  if(SST_T4BIT_COMPRESSED(textureMode))
	    {
	      //4 bit textures are in 8x4 chunks
	      smax /= 8;
	      tmax /= 4;
	    }
	  else if(SST_T8BIT_COMPRESSED(textureMode))
	    {	      
	      //8 bit textures are in 4x4 chunks
	      smax /= 4;
	      tmax /= 4;
	    }
	  else
	    assert(0);

	  if(smax < 1)
	    smax = 1;
	  if(tmax < 1)
	    tmax = 1;
	  
	  for(t=0; t<tmax; t++)
	    {
	      for(s=0; s<smax; s++)
		{
		  for(wordSelect=0; wordSelect<4; wordSelect++)
		    {
		      vaddr = (long *)(calculateTiledTexturePortAddress(textureMode, tLOD, lodmax, s, t, wordSelect) + 
			texturePortBaseAddress);

		      GDBG_INFO(150,"sstDownLoadTexture: s,t,ws=0x%x,0x%x,0x%x(%d,%d,%d), lod=%d, vaddr=0x%x, data32=0x%x\n",
				s,t,wordSelect,s,t,wordSelect,lodmax,vaddr,*data32);
		      SET(vaddr[0],*data32);
		      data32++;
		    }
		}
	    }
	}
      else
	{
	  ///////////////////////////////////////////////////////////////////////
	  //            Non-compressed textures
	  ///////////////////////////////////////////////////////////////////////

	  pack = (smax*(bitsPerTexel/8)-1) % 4 + 1;

	  // use the largest acceptable transfer size      
	  for ( t=0; t<tmax; t++ ) { 
	    for ( s=0; s<smax; s+=(pack*8/bitsPerTexel) ) 
	      {	      
		vaddr = (long *)(calculateTiledTexturePortAddress(textureMode, tLOD, lodmax, s, t, 0) + texturePortBaseAddress);
	      
		if ( pack == 1 ) {
		  GDBG_INFO(150,"sstDownLoadTexture: s,t=0x%x,0x%x(%d,%d), lod=%d, vaddr=0x%x, data8=0x%x\n",
			    s,t,s,t,lodmax,vaddr,*data8);
		  SET8(vaddr[0],*data8);
		  data8++;
		} else if ( pack == 2 ) {
		  GDBG_INFO(150,"sstDownLoadTexture: s,t=0x%x,0x%x(%d,%d), lod=%d, vaddr=0x%x, data16=0x%x\n",
			    s,t,s,t,lodmax,vaddr,*data16);
		  SET16(vaddr[0],*data16);
		  data16++;
		} else {
		  GDBG_INFO(150,"sstDownLoadTexture: s,t=0x%x,0x%x(%d,%d), lod=%d, vaddr=0x%x, data32=0x%x\n",
			    s,t,s,t,lodmax,vaddr,*data32);
		  SET(vaddr[0],*data32);
		  data32++;
		} 
	      }
	  }
	}
    } else {
      //////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////
      //
      //             Linear download
      //
      //////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////
      FxU8 *data8 = (FxU8 *) data;
      FxU16 *data16 = (FxU16 *) data;
      FxU32 *data32 = (FxU32 *) data;
      int n, size;


      if((tLOD & SST_LOD_TSPLIT) && (tLOD & SST_TBIG) && !(tLOD & SST_LOD_ODD))
	vaddr = (long *)(sstLinearMipMapOffset2(lodmax, tLOD, textureMode) - 
			 sstLinearMipMapOffset2(1, tLOD, textureMode) +	
			 texturePortBaseAddress);
      else
	vaddr = (long *)(sstLinearMipMapOffset2(lodmax, tLOD, textureMode) - 
			 sstLinearMipMapOffset2(0, tLOD, textureMode) +	
			 texturePortBaseAddress);

	
      size = sstLinearMipMapSize2(lodmax, tLOD, textureMode);

      if ( size == 1 ) {
        SET8(vaddr[0],*data8);                  // send the texture byte
      } else if ( size == 2 ) {
        SET16(vaddr[0],*data16);                        // send the texture word
      } else if (USETEXTUREMOVE(diago)) {
	genTextureMoveCmd(sst,size,vaddr,data);
      } else {
        for ( n=0; n<size/4; n++ ) 
	  {
	    SET(vaddr[n],data32[n]);                      // send the texture dword
	  }
      }

    }

    //Restore the original state if the download was relocated
    if(relocatingDownload)
      {		
	GDBG_INFO(5, "Restoring TMU0's download registers\n");
	//Restore TMU0's original register values
	SET(SST_TREX(sst, 0)->texBaseAddr, tmu0TexBaseAddr);
	SET(SST_TREX(sst, 0)->textureMode, tmu0TextureMode);
	SET(SST_TREX(sst, 0)->tLOD, tmu0TLOD);
	GDBG_INFO(5, "Restoring TMU0's download registers done\n");
      }       
    
    return(0);
}

//This calculates the proper address for a tiled texture port write
//given s and t (and wordSelect for compressed writes)
FxU32 calculateTiledTexturePortAddress(FxU32 textureMode, FxU32 tLOD, FxU32 lod, FxU32 s, FxU32 t, FxU32 wordSelect)
{
  FxU32 address=0;

  if(tLOD & SST_TBIG)
    {  //2048 x 2048 textures
      if(SST_T4BIT_COMPRESSED(textureMode))
	address = ((lod<<SST_TEXTURE_BIG_LOD4_COMPRESSED_SHIFT) |    
		   (t<<SST_TEXTURE_BIG_T4_COMPRESSED_SHIFT) |    
		   (s<<SST_TEXTURE_BIG_S4_COMPRESSED_SHIFT) |    
		   (wordSelect<<SST_TEXTURE_BIG_WORDSELECT4_COMPRESSED_SHIFT));
      else if(SST_T8BIT_COMPRESSED(textureMode))
	address = ((lod<<SST_TEXTURE_BIG_LOD8_COMPRESSED_SHIFT) |    
		   (t<<SST_TEXTURE_BIG_T8_COMPRESSED_SHIFT) |    
		   (s<<SST_TEXTURE_BIG_S8_COMPRESSED_SHIFT) |    
		   (wordSelect<<SST_TEXTURE_BIG_WORDSELECT8_COMPRESSED_SHIFT));
      else if(SST_T8BIT(textureMode))
	address = ((lod<<SST_TEXTURE_BIG_LOD8_SHIFT) |
		   (t<<SST_TEXTURE_BIG_T8_SHIFT) |
		   (s<<SST_TEXTURE_BIG_S8_SHIFT));
      else if(SST_T16BIT(textureMode))
	address = ((lod<<SST_TEXTURE_BIG_LOD16_SHIFT) |
		   (t<<SST_TEXTURE_BIG_T16_SHIFT) |
		   (s<<SST_TEXTURE_BIG_S16_SHIFT));
      else if(SST_T32BIT(textureMode))
	address = ((lod<<SST_TEXTURE_BIG_LOD32_SHIFT) |
		   (t<<SST_TEXTURE_BIG_T32_SHIFT) |
		   (s<<SST_TEXTURE_BIG_S32_SHIFT));
      else
	assert(0);
    }
  else
    {  //256 x 256 textures
      if(SST_T4BIT_COMPRESSED(textureMode))
	address = ((lod<<SST_TEXTURE_LOD4_COMPRESSED_SHIFT) |    
		   (t<<SST_TEXTURE_T4_COMPRESSED_SHIFT) |    
		   (s<<SST_TEXTURE_S4_COMPRESSED_SHIFT) |    
		   (wordSelect<<SST_TEXTURE_WORDSELECT4_COMPRESSED_SHIFT));
      else if(SST_T8BIT_COMPRESSED(textureMode))
	address = ((lod<<SST_TEXTURE_LOD8_COMPRESSED_SHIFT) |    
		   (t<<SST_TEXTURE_T8_COMPRESSED_SHIFT) |    
		   (s<<SST_TEXTURE_S8_COMPRESSED_SHIFT) |    
		   (wordSelect<<SST_TEXTURE_WORDSELECT8_COMPRESSED_SHIFT));
      else if(SST_T8BIT(textureMode))
	address = ((lod<<SST_TEXTURE_LOD8_SHIFT) |
		   (t<<SST_TEXTURE_T8_SHIFT) |
		   (s<<SST_TEXTURE_S8_SHIFT));
      else if(SST_T16BIT(textureMode))
	address = ((lod<<SST_TEXTURE_LOD16_SHIFT) |
		   (t<<SST_TEXTURE_T16_SHIFT) |
		   (s<<SST_TEXTURE_S16_SHIFT));
      else if(SST_T32BIT(textureMode))
	address = ((lod<<SST_TEXTURE_LOD32_SHIFT) |
		   (t<<SST_TEXTURE_T32_SHIFT) |
		   (s<<SST_TEXTURE_S32_SHIFT));
      else
	assert(0);

    }

  return(address);
}

//
//  This function selects the appropriate number of pixels per clock 
//  to render via the combineMode, textureMode, fbzColorPath,
//  and renderMode registers. It should only be called when 
//  not using texture mapping
//
void setPixelsPerClock(SstRegs *sst)
{
  static FxI32 triangleCount=0;

  FxU32 oldPixelsPerClock, pixelsPerClock;
  FxU32 combineMode;
  FxU32 randomValue;

  //Only change this state after a random number of triangles
  //has been rendered.
  if(triangleCount > 0)
    {
      triangleCount--;
      return;
    }
  
  triangleCount = iRandom(15);

  setBand(sst);

  assert(isTwoPixelsPerClockInhibited() == 0);
  assert((shadowRegisters3D[0][0].fbzColorPath & SST_ENTEXTUREMAP) == 0);
  assert(isMultiTexturing() == FXFALSE);

  oldPixelsPerClock = (shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ? 2 : 1;
      
  randomValue = iRandom(1);
  if((diago.pixelsPerClock == 2) || (diago.pixelsPerClock != 1 && (randomValue)))
    pixelsPerClock = 2;
  else
    pixelsPerClock = 1;
      
  GDBG_INFO(4, "pixelsPerClock = %d\n", pixelsPerClock);
      
  if(oldPixelsPerClock == 2 && pixelsPerClock == 1)
    {
      //Need to insert 12 nops in TMUs before switching from 2ppc to 1ppc
      FxI32 i;

      for(i=0; i<12; i++)
	SET_0_1(sst->nopCMD, 0);
    }

  //Store the FBI combineMode
  combineMode = shadowRegisters3D[0][0].combineMode;
  combineMode &= ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;      
  if(pixelsPerClock == 2)
    combineMode |= SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;      
  SET_FBI(diago.sst->combineMode, combineMode);     

  //Store TMU 0 and TMU 1's combineMode
  //This is just done for the sake of consistency
  combineMode = shadowRegisters3D[0][1].combineMode;
  combineMode &= ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;      
  if(pixelsPerClock == 2)
    combineMode |= SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;      
  SET_0(diago.sst->combineMode, combineMode);           
  SET_1(diago.sst->combineMode, combineMode);           
}

//
//  This function selects the appropriate number of pixels per clock 
//  to render via the combineMode, textureMode, fbzColorPath,
//  and renderMode registers. This function is intended for use
//  when a diag is single-texturing
void texturingSetPixelsPerClock(SstRegs *sst, FxI32 activeTMU)
{
  FxU32 randomValue, oldPixelsPerClock, pixelsPerClock;
  FxI32 inactiveTMU;

  //When we enter this function, we should always be in 1ppc
  assert(!(shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));
  assert(!(shadowRegisters3D[0][1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));
  assert(!(shadowRegisters3D[0][2].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK));

  assert(activeTMU == 0 || activeTMU == 1);

  setBand(sst);

  //Make sure we don't try to switch to 2ppc while multi-texturing
  if(isTwoPixelsPerClockInhibited())
    return;

  //Make sure some rat bastard diag didn't switch to multi-texturing 
  //while running in 2ppc
  if(shadowRegisters3D[0][1].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK)
    assert(isMultiTexturing() == FXFALSE);

  oldPixelsPerClock = (shadowRegisters3D[0][0].combineMode & SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK) ? 2 : 1;

  randomValue = iRandom(1);
  if(((diago.pixelsPerClock == 2) || (diago.pixelsPerClock != 1 && (randomValue)))
     && (isMultiTexturing() == FXFALSE))
    pixelsPerClock = 2;
  else
    pixelsPerClock = 1;

  if(oldPixelsPerClock == 2 && pixelsPerClock == 1)
    {
      //Need to insert 12 nops in TMUs before switching from 2ppc to 1ppc
      FxI32 i;
      
      for(i=0; i<12; i++)
	SET_0_1(sst->nopCMD, 0);
    }

  GDBG_INFO(4, "pixelsPerClock = %d\n", pixelsPerClock);
  
  //Just stay in 1ppc
  if(pixelsPerClock == 1)
    return;
  
  //In 2ppc, need to copy activeTMU's texture rendering state to the other TMU
  inactiveTMU = 1-activeTMU;

  SET(SST_TREX(sst, inactiveTMU)->textureMode, shadowRegisters3D[0][1+activeTMU].textureMode); 
  SET(SST_TREX(sst, inactiveTMU)->tLOD, shadowRegisters3D[0][1+activeTMU].tLOD); 
  SET(SST_TREX(sst, inactiveTMU)->tDetail, shadowRegisters3D[0][1+activeTMU].tDetail); 
  SET(SST_TREX(sst, inactiveTMU)->texBaseAddr, shadowRegisters3D[0][1+activeTMU].texBaseAddr); 
  SET(SST_TREX(sst, inactiveTMU)->texBaseAddr1, shadowRegisters3D[0][1+activeTMU].texBaseAddr1); 
  SET(SST_TREX(sst, inactiveTMU)->texBaseAddr2, shadowRegisters3D[0][1+activeTMU].texBaseAddr2); 
  SET(SST_TREX(sst, inactiveTMU)->texBaseAddr38, shadowRegisters3D[0][1+activeTMU].texBaseAddr38);   
  SET(SST_TREX(sst, inactiveTMU)->combineMode, shadowRegisters3D[0][1+activeTMU].combineMode); 
  SET(SST_TREX(sst, inactiveTMU)->chromaKey, shadowRegisters3D[0][1+activeTMU].chromaKey); 
  SET(SST_TREX(sst, inactiveTMU)->chromaRange, shadowRegisters3D[0][1+activeTMU].chromaRange);   

  //Make sure it's legal to set 2ppc
  if(csimActiveTMUs2(shadowRegisters3D[0][0].fbzColorPath, shadowRegisters3D[0][0].combineMode,
		     shadowRegisters3D[0][1].textureMode, shadowRegisters3D[0][1].combineMode) > 1)
    return;


  //Set to 2 pixels per clock
  SET_FBI(diago.sst->combineMode, shadowRegisters3D[0][0].combineMode | SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
  SET_0(diago.sst->combineMode, shadowRegisters3D[0][1].combineMode | SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);
  SET_1(diago.sst->combineMode, shadowRegisters3D[0][2].combineMode | SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK);  
}

void setBand(SstRegs *sst)
{
  FxU32 bandHeight, triColumnBand;
  FxU32 fbzColorPath, renderMode;

  //Make sure that every is running the same way
  assert((shadowRegisters3D[0][0].renderMode & SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION) ==
	 (shadowRegisters3D[0][1].renderMode & SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION));
  assert((shadowRegisters3D[0][0].renderMode & SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION) ==
	 (shadowRegisters3D[0][2].renderMode & SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION));

  assert((shadowRegisters3D[0][0].fbzColorPath & SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL) ==
	 (shadowRegisters3D[0][1].fbzColorPath & SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL));
  assert((shadowRegisters3D[0][0].fbzColorPath & SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL) ==
	 (shadowRegisters3D[0][2].fbzColorPath & SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL));

  //Select the 2 pixel per clock band height
  if(diago.log2BandHeight >= 0)
    bandHeight = diago.log2BandHeight;
  else
    bandHeight = iRandom(SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION >> SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION_SHIFT);
  
  renderMode = shadowRegisters3D[0][0].renderMode;
  renderMode &= ~SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION;
  renderMode |= bandHeight << SST_RM_TWO_PIXELS_PER_CLOCK_BAND_SELECTION_SHIFT;
  SET(diago.sst->renderMode, renderMode);

  //Select the Column-of-N to use
  if(diago.triColumnBand >= 0)
    triColumnBand = diago.triColumnBand;
  else
    triColumnBand = iRandom(2);
  
  fbzColorPath = shadowRegisters3D[0][0].fbzColorPath;
  fbzColorPath &= ~SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL;
  fbzColorPath |= (triColumnBand << SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL_SHIFT) & SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL;
  SET(diago.sst->fbzColorPath, fbzColorPath);
}



//
// given a base, tile stride, and pixel depth,
// compute the x,y location corresponding to a given address
//
// x is always rounded in the direction of base, so rem is the same sign as x
//
void sstTiledToXY(FxU32 addr, FxU32 base, FxU32 tStride, FxU32 depth, int *x, int *y, int *rem) 
{
  int a, b, t;
  int tx, ty;
  FxU32 xBase, yBase, tAddr, xAddr, yAddr;

  t = FLOOR(base,SST_TILE_SIZE) * SST_TILE_SIZE;
  b = base - t;
  xBase = (b & SST_BUFFER_BASE_X) >> SST_BUFFER_BASE_X_SHIFT; // base x
  yBase = (b & SST_BUFFER_BASE_Y) >> SST_BUFFER_BASE_Y_SHIFT; // base y

  a = addr - t;
  tAddr = (a & SST_BUFFER_BASE_T) >> SST_BUFFER_BASE_T_SHIFT; // base page
  xAddr = (a & SST_BUFFER_BASE_X) >> SST_BUFFER_BASE_X_SHIFT; // base x
  yAddr = (a & SST_BUFFER_BASE_Y) >> SST_BUFFER_BASE_Y_SHIFT; // base y
  xAddr += (tAddr % tStride)*SST_TILE_WIDTH;
  yAddr += (tAddr / tStride)*SST_TILE_HEIGHT;

  tx = xAddr - xBase;
  ty = yAddr - yBase;

  if ( tx < 0 ) {
    tx = -tx;
    *rem = tx - (tx/depth)*depth;
    tx /= (int) depth;
    tx = -tx;
    *rem = -(*rem);
  } else {
    *rem = tx % depth;
    tx /= (int) depth;
  }
  *x = tx;
  *y = ty;

  GDBG_INFO(170,"sstTiledToXY: a=0x%x, b=0x%x, s=%d, d=%d, x=%d, y=%d, rem=%d\n",
           addr,base,tStride,depth,*x,*y,*rem);
  a = tiledAddress(base,tStride,depth,*x,*y)+(*rem);
  if ( addr != (FxU32)a )
    GDBG_ERROR("sstTiledToXY","a=0x%x, b=0x%x, s=%d, d=%d, x=%d, y=%d, rem=%d, csim=0x%x\n",
               addr,base,tStride,depth,*x,*y,*rem,a);
}

//
// Determine min/max tile rectangle which fits between two linear addresses [amin,amax],
// inclusive
//
void sstBoundingBox(int amin, int amax, int tStride, int depth, 
                    int *base, int *xmax, int *ymax, int *r)
{
  int b = CEIL(amin,16)*16;
  int t = b & SST_BUFFER_BASE_T;
  int nt = (amax - t) / (tStride*SST_TILE_SIZE);
  int bmax = *base = b;
  if ( nt > 0 )
    bmax = t + nt*tStride*SST_TILE_SIZE - 1;

  GDBG_INFO(170,"sstBoundingBox:  a(min,max)=0x%x,0x%x, b(min,max)=0x%x,0x%x, t,tStride=%d,%d\n",
            amin,amax,*base,bmax,t,tStride);
  sstTiledToXY(bmax,*base,tStride,depth,xmax,ymax,r);

}

//
// determine the current buffer id corresponding to
// the 'buf' buffer at time zero (before any swapping)
//
int sstRollFwdBufId(int buf)
{
  int n;
  int lbuf;

  lbuf = buf;

  if ( buf == CSIM_BUF_3D_FRONT || 
       buf == CSIM_BUF_3D_BACK || 
       buf == CSIM_BUF_3D_TRIPLE ) {
    if ( buf == CSIM_BUF_3D_TRIPLE )
      buf = 2;

    if ( diago.triple ) 
      n = (buf + 3 - diagSwaps % 3) % 3;
    else
      n = (buf + 2 - diagSwaps % 2) % 2;
      
    if ( n == 0 )
      lbuf = CSIM_BUF_3D_FRONT;
    else if ( n == 1 )
      lbuf = CSIM_BUF_3D_BACK;
    else if ( n == 2 )
      lbuf = CSIM_BUF_3D_TRIPLE;
  }

  return lbuf;
}

//
// fastfill_bug_workaround
// This function is used to workaround the H3/H4 bug when doing fastfill to
// the Z buffer with SDRAM. The workaround is to fill the Z buffer by using the
// color buffer fill logic.
//

// Dithering depends on the bit in fbzMode. When using SGRAM,
// and you're doing fastfill, you can optionally disable dithering with bit
// 0 of the fastfill cmd. But with SDRAM, only fbzMode matters. The bit 0 in
// the fastfill reg is a don't care. So to disable dithering with SDRAM, it
// has to be done with fbzMode.

void fastfill_bug_workaround(SstRegs *sst, FxU32 fbzMode,
			     FxU32 zaColor, FxU32 c1,
			     FxU32 auxBufferAddr, FxU32 auxBufferStride,
			     FxU32 colBufferAddr, FxU32 colBufferStride,
			     unsigned int fastfill_cmd)
{
  FxU32 temp_c1, temp_fbzMode;

  // Fastfill to the Z buffer is only a function of ZAWRMASK.
  // ENDEPTHBUFFER doesn't matter.

  if (fbzMode & SST_ZAWRMASK) {
    // Need to set RGBWRMASK in case it's not already set.
    temp_fbzMode = (fbzMode & (~SST_ZAWRMASK & ~SST_ENDITHER)) | SST_RGBWRMASK;
    SET(sst->fbzMode, temp_fbzMode);
    SET(sst->colBufferAddr,auxBufferAddr);
    SET(sst->colBufferStride,auxBufferStride);
    // Need to set color1 to the value that will result in zaColor[15:0]
    // being written to memory. This is a function of how 5,6,5 truncation
    // works. See the code in sstFbiPixel. This is kind of reverse
    // truncating zaColor and putting it in c1. Then when c1 is trunc'd
    // you end up with zaColor[15:0] in memory.
    temp_c1 = ((zaColor & 0xf800) << 8) | ((zaColor & 0x7e0) << 5) |
      ((zaColor & 0x1f) << 3);
    GDBG_INFO(199,"fastfill_bug_workaround: Substituting fill of zaColor = %x with fill of color1 = %x\n",zaColor,temp_c1);
    GDBG_INFO(199,"fastfill_bug_workaround: fbzMode originally = %x, temporarily set to = %x\n",fbzMode,temp_fbzMode);
    SET(sst->c1,temp_c1);
    SET(sst->fastfillCMD,fastfill_cmd);
    sst_idle_really(sst);
    // Now do reg writes to restore fbzMode, colBuffer*, and color1.
    SET(sst->fbzMode, fbzMode);
    SET(sst->colBufferAddr,colBufferAddr);
    SET(sst->colBufferStride,colBufferStride);
    SET(sst->c1,c1);
  }
  if (fbzMode & SST_RGBWRMASK) {
    // Clear the SST_ZAWRMASK in case it is set and do the fastfill
    temp_fbzMode = fbzMode & ~SST_ZAWRMASK;
    SET(sst->fbzMode, temp_fbzMode);
    GDBG_INFO(199,"fastfill_bug_workaround: Doing fill of color1 with Z turned off. color1 = %x\n",c1);
    GDBG_INFO(199,"fastfill_bug_workaround: fbzMode originally = %x, temporarily set to = %x\n",fbzMode,temp_fbzMode);
    SET(sst->fastfillCMD,fastfill_cmd);
    sst_idle_really(sst);
    // Restore the fbzMode reg.
    SET(sst->fbzMode, fbzMode);
  }
}

FxU32 sstBitsPerTexel(FxU32 textureMode)
{
  FxU32 result;

  if(SST_T4BIT_COMPRESSED(textureMode))
    result = 4;
  else if(SST_T8BIT_COMPRESSED(textureMode))
    result = 8;
  else if(SST_T8BIT(textureMode))
    result = 8;
  else if(SST_T16BIT(textureMode))
    result = 16;
  else if(SST_T32BIT(textureMode))
    result = 32;
  
  return(result);
}


//Begin evil
//Had to do this goofy shit as a work-around.
static FxBool _inhibitTwoPixelsPerClock = FXFALSE;

void inhibitTwoPixelsPerClock(void)
{
  GDBG_INFO(1, "2 Pixel Per Clock rendering inhibited by diag\n");
  _inhibitTwoPixelsPerClock = FXTRUE;
}

FxBool isTwoPixelsPerClockInhibited(void)
{
  return(_inhibitTwoPixelsPerClock);
}
//End evil

//This takes the real PCI busNumber and deviceNumber and converts
//it into the PCI libraries dumb-ass composite value.
FxU32 generateDeviceNumber(FxU32 busNumber, FxU32 deviceNumber, FxU32 functionNumber)
{
  assert(deviceNumber < 32);
  assert(busNumber < 256);

  //This equation is actually wrong. The PCI library does use the correct 
  //equation either though. Basically, we'll just disregard the functionNumber
  functionNumber;

  return(((functionNumber * 256) + busNumber) * 32 + deviceNumber);
}

/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: H3ACCELERATION.C, 5, 10/11/00 8:38:30 PM, Brent$
** $Log: 
**  5    3dfx      1.3.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    MacOS Dev Tree1.3         04/02/00 Stephane Huaulme more region parsing
**       fixing...
**  3    MacOS Dev Tree1.2         03/25/00 Stephane Huaulme changed region parsing
**       (using same as CP)
**  2    MacOS Dev Tree1.1         02/01/00 Kenneth Dyke    Deal with new HRM
**       struct.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 4     7/12/99 12:07p Kcd
** Fifo idle improvements for Banshee.
** 
** 3     7/09/99 7:18p Kcd
** More paranoid status checks.
** 
** 2     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/


#include <Types.h>
#include <Memory.h>
#include <Errors.h>
#include <Quickdraw.h>
#include <NQDAcceleration.h>
#include <GraphicsAcceleration.h>
#include <Devices.h>
#include <Displays.h>
#include "h3defs.h"
#include "h3gdefs.h"
#include "minihwc.h"
#include "hwcio.h"
#include "H3Acceleration.h"

/* UGH. */


FxU32 h3packetBuffer[H3FIFO_SIZE + 64];
FxU32 *h3packetData, *h3packetHeader;

/* static hwcInfo hInfo; */

/* Fast table lookup for destination bitmaps.  We know that our driver always 
   requests that the V3 gets 256MB of address space.  So, we can use the upper
   4 bits to index into a table to quickly find a batching board for that memory
   region.  No one should have pixmaps that point into that segment that don't
   point into our RAM. 
   
   Note that H5 (Napalm) will probably always request 512MB chunks because of the
   duplicated address spaces.  However, we'll be able to deal with that by simply
   marking two adjacent blocks with the same pointer. */
   
struct h3Info h3_info[16];

hrmHwFifoPtrPtr         _hrmHwFifoPtr;
hrmFifoWrapPtr          _hrmFifoWrap;
hrmGetTargetFifoInfoPtr _hrmGetTargetFifoInfo;
hrmGetTargetBoardInfoExtPtr _hrmGetTargetBoardInfoExt;
hrmGetVersionInfoPtr    _hrmGetVersionInfo;

OSErr InitializeAccelerationHardware(void)
{
  OSErr       myErr;
  FxU32 i = 0, numTargets;
  hrmVersionInfo_t versionInfo;
  
  dprintf("InitializeAccelerationHardware\n");
  
  h3packetData = h3packetBuffer;

  /* First we need to grab the hrm FIFO extensions that we use. */
  _hrmHwFifoPtr = (hrmHwFifoPtrPtr) hrmGetExtension("hrmHwFifoPtr");
  _hrmFifoWrap = (hrmFifoWrapPtr) hrmGetExtension("hrmFifoWrap");
  _hrmGetTargetFifoInfo = (hrmGetTargetFifoInfoPtr) hrmGetExtension("hrmGetTargetFifoInfo");
  _hrmGetTargetBoardInfoExt = (hrmGetTargetBoardInfoExtPtr) hrmGetExtension("hrmGetTargetBoardInfoExt");
  _hrmGetVersionInfo = (hrmGetVersionInfoPtr) hrmGetExtension("hrmGetVersionInfo");
  
  if(!(_hrmFifoWrap && _hrmHwFifoPtr && 
       _hrmGetTargetFifoInfo && _hrmGetTargetBoardInfoExt && _hrmGetVersionInfo)) {
     return -1;
  }
  
  /* Make sure version is what we expect (1.5) */
  _hrmGetVersionInfo(&versionInfo);
  if(!((versionInfo.major == 1) && (versionInfo.minor == 5))) {
    return -1;
  }
  
  /* Query HRM for the number of targets it knows about, and then ask it for the target info. */
  numTargets = hrmGetNumTargets();
  
  dprintf("hrmGetNumTargets() = %d\n",numTargets);
  
  /* For each target, grab board and fifo info */
  for(i = 0; i < numTargets; i++) {
  	hrmBoard_t *board;
  	hwcBoardInfo *bInfo;
  	hrmFifoInfo *fifoInfo;
  	hrmBoardInfo_t boardInfo;
  	
  	board = hrmGetTargetAtIndex(i);
  	dprintf("hrmGetTargetAtIndex(%d) = %08lx\n",i,board);
  	
  	/* These calls are somewhat temporary */
  	fifoInfo = _hrmGetTargetFifoInfo(board);
  	_hrmGetTargetBoardInfoExt(board, &boardInfo);
  	
  	if ( boardInfo.deviceID == 5 )
  	{
  	
      dprintf("info: %08lx %08lx %08lx\n", board, bInfo, fifoInfo);
      dprintf("set: %08lx %08lx\n",fifoInfo->setLfb,fifoInfo->setLfbHost);

	  if(!fifoInfo->setLfb || !fifoInfo->setLfbHost)
	    return -1;

	  /* Now fill in struct since we have all the info we need. */    
      bInfo = &h3_info[boardInfo.pciBaseAddr[1] >> 28].boardInfo;    
    
      /* Fill out hwcBoardInfo struct */
      bInfo->hMon = 0;
      bInfo->hdc = (void *)boardInfo.driverRefNum;
      bInfo->h3Mem = boardInfo.h3Mem;
      bInfo->devRev = boardInfo.deviceRev;
      bInfo->pciInfo.vendorID = boardInfo.vendorID;
      bInfo->pciInfo.deviceID = boardInfo.deviceID;
      bInfo->pciInfo.pciBaseAddr[0] = boardInfo.pciBaseAddr[0];
      bInfo->pciInfo.pciBaseAddr[1] = boardInfo.pciBaseAddr[1];
      bInfo->pciInfo.pciBaseAddr[2] = boardInfo.pciBaseAddr[2];
      bInfo->pciInfo.pciBaseAddr[3] = boardInfo.pciBaseAddr[3];
      bInfo->pciInfo.isMaster = boardInfo.isMaster;
      bInfo->pciInfo.numChips = boardInfo.numChips;
      bInfo->pciInfo.swizzleOffset[0] = boardInfo.swizzleOffset[0];
      bInfo->pciInfo.swizzleOffset[1] = boardInfo.swizzleOffset[1];
      bInfo->pciInfo.swizzleOffset[2] = boardInfo.swizzleOffset[2];
      bInfo->pciInfo.swizzleOffset[3] = boardInfo.swizzleOffset[3];

      bInfo->linearInfo.linearAddress[0] = bInfo->pciInfo.pciBaseAddr[0];
      bInfo->linearInfo.linearAddress[1] = bInfo->pciInfo.pciBaseAddr[1];
      bInfo->linearInfo.linearAddress[2] = bInfo->pciInfo.pciBaseAddr[2];
      bInfo->linearInfo.linearAddress[3] = bInfo->pciInfo.pciBaseAddr[3];
        
      bInfo->regInfo.ioMemBase = bInfo->linearInfo.linearAddress[0] + SST_IO_OFFSET;
      bInfo->regInfo.cmdAGPBase = bInfo->linearInfo.linearAddress[0] + SST_CMDAGP_OFFSET;
      bInfo->regInfo.waxBase = bInfo->linearInfo.linearAddress[0] + SST_2D_OFFSET;
      bInfo->regInfo.sstBase = bInfo->linearInfo.linearAddress[0] + SST_3D_OFFSET;
      bInfo->regInfo.lfbBase = bInfo->linearInfo.linearAddress[0] + SST_LFB_OFFSET;
      bInfo->regInfo.rawLfbBase = bInfo->linearInfo.linearAddress[1];
      bInfo->regInfo.ioPortBase = bInfo->pciInfo.pciBaseAddr[2] & ~0x1;

	  h3_info[bInfo->regInfo.rawLfbBase >> 28].board = board;
      h3_info[bInfo->regInfo.rawLfbBase >> 28].fifo = fifoInfo;
	  h3_info[bInfo->regInfo.rawLfbBase >> 28].bInfo = bInfo;
    
      /* Temp hack to make sure nothing else is blowing up. */
      fifoInfo->exclusiveMode = FXFALSE;
    }
  }
  
  /* As long as we found more than one board, we're okay.  If something
     else failed for some reason, we would have already bailed out. */
  if(i > 0)
  	myErr = 0;
  else
  	myErr = -1;
  	
	return myErr;
}

Int32  H3BlitFinish(long refCon)
{
  hwcBoardInfo *bInfo = ((h3Info *)refCon)->bInfo;
	FxU32 status, depth;

	__sync();
	
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.depth, depth);
	HWC_WAX_LOAD(bInfo->regInfo, status, status);
    
  if(depth != 0) {
    return false;
  }
    
	
	if(status & (SST_BUSY|SST_GUI_BUSY|SST_CMD0_BUSY)) {
		return false;
	}

  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.depth, depth);
	HWC_WAX_LOAD(bInfo->regInfo, status, status);
	if(status & (SST_BUSY|SST_GUI_BUSY|SST_CMD0_BUSY)) {
		return false;
	}

  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.depth, depth);
	HWC_WAX_LOAD(bInfo->regInfo, status, status);
	if(status & (SST_BUSY|SST_GUI_BUSY|SST_CMD0_BUSY)) {
		return false;
	}
	
	return true;
}


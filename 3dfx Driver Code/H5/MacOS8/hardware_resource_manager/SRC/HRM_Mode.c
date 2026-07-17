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
** $Header: HRM_Mode.c, 5, 10/11/00 8:35:11 PM, Brent$
** $Log: 
**  5    3dfx      1.3.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    MacOS Dev Tree1.3         03/07/00 Critical Path   better mem management,
**       more qd coverage, bug fixes...
**  3    MacOS Dev Tree1.2         02/07/00 Kenneth Dyke    Added SLI/AA support
**       functions.
**  2    MacOS Dev Tree1.1         01/31/00 Kenneth Dyke    Added PCI config
**       register extensions.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 6     8/25/99 5:43p Kcd
** Fix to make sure Glide doesn't break when monitors are rearranged.
** 
** 5     8/10/99 12:29p Kcd
** Fixes for full-screen support.
** 
** 4     8/02/99 12:13p Kcd
** New SetPrefs/GetPrefs extensions.
** 
** 3     7/26/99 1:52p Kcd
** Added new extensions to help support Power Management in 2D driver. 
** 
** Also fixed possible FIFO trashing problem when returning from
** fullscreen exclusive modes.
** 
** 2     7/08/99 1:25p Kcd
** Graphics clock speed extension.
** 
** 1     7/02/99 3:20p Kcd
** hrm video mode change support.
**
*/

#include "hrm_priv.h"

#include "hdwr_res_mgr.h"

#include "minihwc.h"
#include "hwcio.h"
#include "hrm_fifo.h"
#include "GraphicsPrivHwc.h"
#include "DCon.h"
#include <string.h>
#include <files.h>
#include <CodeFragments.h>
#include <NameRegistry.h>
#include <DriverServices.h>
#include <Video.h>

#include "hrm_mode.h"

OSErr			hrmPrivateControlCall(
						short				inRefNum,
						FxU32				which,
						hwcRequest_t		*inReq,
						hwcResponse_t		*outRes);

// Function Declarations


FxBool setExclusive(short inRefNum, FxBool exclusive)
{
	OSErr       myErr;
	hwcRequest_t req;

  	req.optData.setExclusiveReq.exclusive = exclusive;
	myErr = hrmPrivateControlCall(
					inRefNum, 
					k3DfxSetExclusive,
					&req,
					NULL);
	
	return (myErr == noErr) ? FXTRUE : FXFALSE;
}

FxBool hrmSetExclusiveMode(hrmBoard_t *board)
{
  VDSwitchInfoRec switchInfo;
  FxU32 switchDepthMode;
  GDHandle gdHandle;
  OSErr err;

  dprintf("hrmSetExclusiveMode(%08lx): %d %08lx\n",board,board->fifoInfo.exclusiveMode,board->boardInfo.hMon);
  
  if(!board->fifoInfo.exclusiveMode && board->boardInfo.hMon) {
    
    /* Tell board to go into exclusive mode. */
    err = setExclusive(board->drvrRefNum, FXTRUE);
    
    dprintf("setExclusive(%d,FXTRUE) returned %d\n",board->drvrRefNum,err);
    
    /* Shut down the 2D command fifo and 2D acceleration for that board. */
    board->fifoInfo.exclusiveMode = FXTRUE;
    
    dprintf("Disabling fifo\n");
    hrmDisableFifo(board);
      
    /* Turn on our exclusive flag so memory manager will fail any new allocations. */
    board->exclusiveMode = FXTRUE;
        
    /* Get the current display mode. */
    gdHandle = _hrmFindGDeviceHandle((short)board->boardInfo.hdc);
    
    dprintf("gdHandle: %08lx\n",gdHandle);
    
    err = DMGetDisplayMode(gdHandle, &board->videoInfo.oldVideoMode);

    dprintf("DMGetDisplayMode returned %d\n",err);
    		
    if(err == noErr) {
	  
	  board->videoInfo.oldModeValid = FXTRUE;
	  
      /* Tell DM to switch to a "magic" display mode that will move the framebuffer
         off of our hardware and into system memory somewhere.  This should prevent
         MacOS from screwing with the display. */
	       
      switchInfo = board->videoInfo.oldVideoMode;
      //switchInfo.csMode = kDepthMode4;
      //switchDepthMode = kDepthMode4;	  
      dprintf("calling DMSetDisplayMode(gd = %08lx, data = %08lx, dm: %d)\n",
        gdHandle,switchInfo.csData,switchDepthMode); 
        
      err = DMSetDisplayMode(gdHandle,
                             switchInfo.csData,
                             &switchDepthMode,
                             (unsigned long)&switchInfo,
                             NULL);
      
      dprintf("DMSetDisplayMode returned %d\n",err);
      
      if(err == noErr) {
        /* Invalidate any remaining memory blocks still around.  This should be done
           earlier, except that the ROM doesn't have a disposal callback in place yet. FIXME. */
        hrmInvalidateMemoryBlocks(board, 0, 0xffffffff);
        
        return FXTRUE;    
      }
      
    }
   
    /* Hmm, something went wrong, so clean up nicely. */
    dprintf("Releasing exclusive mode.\n");
    hrmReleaseExclusiveMode(board);
  }
  return FXFALSE;

}

OSErr hrmSetVideoMode(hrmBoard_t *board, int xRes, int yRes, int refresh)
{
	hwcRequest_t req;

	dprintf("hrmSetVideoMode(%d,%d,%d)\n",xRes,yRes,refresh);
	req.optData.setModeReq.width = xRes;
	req.optData.setModeReq.height = yRes;
	req.optData.setModeReq.refresh = refresh;

	return hrmPrivateControlCall(
					(short)board->boardInfo.hdc, 
					k3DfxSetDisplayMode,
					&req,
					NULL);
}

void hrmReleaseExclusiveMode(hrmBoard_t *board)
{
  unsigned long oldDepthMode;
  OSErr err;
  
  dprintf("hrmReleaseExclusiveMode(%08lx): %d\n",board,board->fifoInfo.exclusiveMode);
  
  if(board->fifoInfo.exclusiveMode) {        
        
    /* Restore old video mode (if we need to). */
    dprintf("old mode valid: %d\n",board->videoInfo.oldModeValid);
    
    /* Set 2D driver back to non-exclusive mode. */
    err = setExclusive(board->drvrRefNum, FXFALSE);

    dprintf("setExclusive(%d,FXFALSE) returned %d\n",board->drvrRefNum,err);
    
    /* Turn off our exclusive flag so memory manager will work again. */
    board->exclusiveMode = FXFALSE;
    
    /* Bring 2D command FIFO back online. */
    dprintf("turning on FIFO\n");
    hrmInitFifo(board, FXTRUE);

    dprintf("turning on acceleration\n");
    
    /* Allow 2D acceleration code to run again. */
    board->fifoInfo.exclusiveMode = FXFALSE;
    
    /* Now restore display... do this AFTER we have re-initialized the FIFO */
    /* So that we don't have the desktop redraw do something stupid like spew */
    /* Data into the old (Glide) command FIFO, which it never bothers to turn off. */
    
    /* Get the current display mode. */
    err = DMGetDisplayMode(_hrmFindGDeviceHandle((short)board->boardInfo.hdc), &board->videoInfo.oldVideoMode);

    if(board->videoInfo.oldModeValid) {
      /* Boot the display manager in the head to force it to redraw and fix the CLUTs */		
      oldDepthMode = board->videoInfo.oldVideoMode.csMode;
      DMSetDisplayMode(_hrmFindGDeviceHandle((short)board->boardInfo.hdc),
                       board->videoInfo.oldVideoMode.csData,
                       &oldDepthMode,
                       (unsigned long)&board->videoInfo.oldVideoMode,NULL);
      
      board->videoInfo.oldModeValid = FXFALSE;
        
    }
    
    dprintf("exclusive mode restoration complete\n");
  }
}

OSErr hrmSetPrefs(hrmBoard_t *board, FxU32 prefs)
{
	hwcRequest_t req;
	OSErr err;

	req.optData.setPrefsReq.prefs = prefs;

	err = hrmPrivateControlCall(
					(short)board->boardInfo.hdc, 
					k3DfxSetPrefs,
					&req,
					NULL);
	if(err == noErr) 
	{
		board->prefs = prefs;
	}
	return err;
}

FxU32 hrmGetPrefs(hrmBoard_t *board)
{
	if(board)
		return(board->prefs);
	return 0;
}

OSErr hrmSetModeFlags(hrmBoard_t *board, unsigned long displayModeID, unsigned long timingFlags)
{
	hwcRequest_t req;
	OSErr rv;

	dprintf("hrmSetModeFlags(%d,%d)\n",displayModeID,timingFlags);

	req.optData.setModeFlagsReq.displayModeID = displayModeID;
	req.optData.setModeFlagsReq.timingFlags = timingFlags;

	rv = hrmPrivateControlCall(
					(short)board->boardInfo.hdc, 
					k3DfxSetModeFlags,
					&req,
					NULL);
					
	dprintf("hrmSetModeFlags(%08lx,%d,%d) = %d\n",board,displayModeID,timingFlags,rv);
	return rv; 
}

OSErr hrmWriteConfigRegister(hrmBoard_t *board, unsigned long deviceID, unsigned long offset, unsigned long value)
{
	hwcRequest_t req;
	OSErr rv;

	dprintf("hrmWriceConfigRegister(%d,%d,%d)\n",deviceID,offset,value);

	req.optData.pciOpReq.Operation = k3DfxPCIOpWrite;
	req.optData.pciOpReq.DeviceId  = deviceID;
	req.optData.pciOpReq.Offset    = offset;
	req.optData.pciOpReq.Value     = value;
			
	rv = hrmPrivateControlCall(
					(short)board->boardInfo.hdc, 
					k3DfxPCIOp,
					&req,
					NULL);
	return rv; 
}

OSErr hrmReadConfigRegister(hrmBoard_t *board, unsigned long deviceID, unsigned long offset, unsigned long *value)
{
	hwcRequest_t req;
	hwcResponse_t res;
	OSErr rv;

	dprintf("hrmReadConfigRegister(%d,%d,%08lx)\n",deviceID,offset,value);

	req.optData.pciOpReq.Operation = k3DfxPCIOpRead;
	req.optData.pciOpReq.DeviceId  = deviceID;
	req.optData.pciOpReq.Offset    = offset;
	req.optData.pciOpReq.Value     = 0;
			
	rv = hrmPrivateControlCall(
					(short)board->boardInfo.hdc, 
					k3DfxPCIOp,
					&req,
					&res);
  
	if(rv == 0) 
	{
		*value = res.optData.pciOpRes.Value;
	}
	return rv; 
}

void hrmGetSlaveRegs(hrmBoard_t *board, FxU32 chipNumber, FxU32 *regs)
{
	hwcRequest_t req;
	hwcResponse_t res;
	OSErr rv;

	memset( &res, 0, sizeof(res));

	req.optData.slaveRegReq.DeviceId  = chipNumber;
			
	rv = hrmPrivateControlCall(
					(short)board->boardInfo.hdc, 
					k3DfxSlaveRegs,
					&req,
					&res);

	regs[0] = res.optData.slaveRegRes.Regs[0];
	regs[1] = res.optData.slaveRegRes.Regs[1];
	regs[2] = res.optData.slaveRegRes.Regs[2];
	regs[3] = res.optData.slaveRegRes.Regs[3];
}

void hrmSLIAA(hrmBoard_t *board, hrmSLIAAChipInfo_t *chipInfo, hrmSLIAAMemInfo_t *memInfo)
{
	hwcRequest_t req;
	OSErr rv;

	req.optData.sliaaReq.chipInfo.numChips      = chipInfo->numChips;
	req.optData.sliaaReq.chipInfo.sliEnable     = chipInfo->sliEnable;
	req.optData.sliaaReq.chipInfo.aaEnable      = chipInfo->aaEnable;
	req.optData.sliaaReq.chipInfo.aaSampleHigh  = chipInfo->aaSampleHigh;
	req.optData.sliaaReq.chipInfo.sliAaAnalog   = chipInfo->sliAaAnalog;
	req.optData.sliaaReq.chipInfo.sli_nlines    = chipInfo->sli_nlines;
	req.optData.sliaaReq.chipInfo.swapAlgorithm = chipInfo->swapAlgorithm;

	req.optData.sliaaReq.memInfo.totalMemory              = memInfo->totalMemory;
	req.optData.sliaaReq.memInfo.tileMark                 = memInfo->tileMark;
	req.optData.sliaaReq.memInfo.tileCmpMark              = memInfo->tileCmpMark;
	req.optData.sliaaReq.memInfo.aaSecondaryColorBufBegin = memInfo->aaSecondaryColorBufBegin;
	req.optData.sliaaReq.memInfo.aaSecondaryDepthBufBegin = memInfo->aaSecondaryDepthBufBegin;
	req.optData.sliaaReq.memInfo.aaSecondaryDepthBufEnd   = memInfo->aaSecondaryDepthBufEnd;
	req.optData.sliaaReq.memInfo.bpp                      = memInfo->bpp;
			
	rv = hrmPrivateControlCall(
					(short)board->boardInfo.hdc, 
					k3DfxSLIAA,
					&req,
					NULL);
}
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
** $Revision: 3$
** $Date: 10/11/00 8:18:53 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

int
setupCmdFifo(SstRegs *sst)
{
  FxU32 fifoSize, fifoStart;

  fifoSize = diago.ringSize * 4;
  //if (fifoSize < 0) fifoSize = -fifoSize;
  fifoStart = 0x0;

  if (diago.whichFifo == 2) {
    hb_selectFifo(0);
    if (diago.agpEnable == 1) // 0 in AGP only
      fxHalInitCmdFifo(sst,0,fifoStart,fifoSize,
		       diago.directExec,1,diago.agpEnable & 0x1);
    else
      fxHalInitCmdFifo(sst,0,fifoStart,fifoSize,
		       diago.directExec,diago.disableHoles,diago.agpEnable & 0x1);
    
    fxHalInitCmdFifo(sst,1,fifoStart+((fifoSize + 0xFFF) & ~0xFFF),fifoSize,
		     // disable holes automatically for cmdfifo 1 if in AGP
		     diago.directExec,diago.disableHoles || diago.agpEnable & 0x2,
		     diago.agpEnable & 0x2);
  }
  else {
    hb_selectFifo(diago.whichFifo);
    fxHalInitCmdFifo(sst,diago.whichFifo,fifoStart,fifoSize,
		     diago.directExec,diago.disableHoles,diago.agpEnable);
  }

  return(1);
}


void
main (int argc, char **argv)
{
    SstRegs *sst;
    SstCRegs *sstc;
    SstIORegs *sstio;
    FxU32 *rawlfb;
    CsimPrivate *cp;

    int n;
    static Triangle t;
    FxU32 dummy;
    FxU32 miscInit1;
    FxU32 agpCommand;
    FxU32 val;

    sst = SST_BEGIN(argc,argv);
    sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
    sstio = (SstIORegs *)(SST_IO_ADDRESS(sst));
    rawlfb = (FxU32 *)(SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET);
    cp = CSIM_PRIVATE(diago.sstCSIM);

    //
    // ensure that PCI cmdfifo is enabled
    //
    
    if ( !diago.writeFifo ) {
      GDBG_ERROR("cfereset","must be run with -W option\n");
      DIAG_FAIL();
    }

    if ( diago.agpEnable ) {
      GDBG_ERROR("cfereset","must NOT be run with AGP CMDFIFO\n");
      DIAG_FAIL();
    }

    //
    // the csim doesn't model cmdfifo reset, hw or hsim is required

    if (!diago.halInfo->hsim && !diago.halInfo->hw) {
      GDBG_ERROR("cfereset","must be run with hw or hsim\n");
    }

    //
    // turn off AGP so moveCMD will hang
    //
#if defined HAL_HSIM || defined HAL_HW
    if (diago.halInfo->hsim || diago.halInfo->hw) {
      GDBG_INFO(0,"Disabling AGP\n");
      agpCommand = PCI_CFG_RD(SST_AGP_COMMAND,0x1) & ~BIT(8);
      PCI_CFG_WR(SST_AGP_COMMAND,agpCommand,0x1);
    }
#else
    FXUNUSED(agpCommand);
#endif

    //
    // reset CMDFIFO once each pass
    //
    
    while (DIAG_STARTPASS()) {			// for each pass
      
      
      //
      // hang CMDFIFO by executing a PKT 6 while AGP is disabled
      //
      {
	FxU32 sizeBytes;
	FxU32 srcWidth;
	FxU32 srcStride;
	FxU32 *moveData;
	FxU32 fbOffset;
	FxU32 dstStride;
	FxU32 space;
	
	sizeBytes = 4;
	srcWidth = 4;
	srcStride = 4;
	moveData = (FxU32 *)cp->info->agpVirtAddr;
	fbOffset = 0x100000;
	dstStride = 4;
	space = SSTCP_LFB_SPACE;
	
	GDBG_INFO(0,"Initiating moveCmd\n");
	hb_moveCmd(sst,sizeBytes,srcWidth,srcStride,moveData,fbOffset,dstStride,space);
      }
      
      //
      // Omit drawing of triangles because csim doesn't model
      // the PKT 6 hang and therefore erroneously executes the
      // triangle commands
      //
#if 0
      //
      // draw some triangles -- these should never be executed
      // because the CMDFIFO will hang on the PKT 6 and reset
      // should clear the triangle commands out of the CMDFIFO
      //
      GDBG_INFO(0,"Drawing triangles before reset\n");
      for (n=0; n<1; n++) {			// do 20 tests
	randomTriangle(&t,diago.tsize,1);	// pick random triangle
	randomRgbaTriangle(&t);			// with random colors
	
	areaTriangle(&t);			// compute the area (before setup)
	setupTriangle(&t,1,0,0);		// setup RGB slopes
	sortTriangle(&t);			// sort it
	printTriangle(2,&t,1,0,0);
	printTriangleSlopes(3,&t,1,0,0);
	gdbg_info(3,"    area = %d\n", t.area);
	
	// NOTE: if sub-pixel parameter adjustment is OFF we may get
	//	 color overflows/underflows etc
	SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());
	drawTriangle(sst,&t,1,0,0);
	
	sst_idle(sst);				// wait for the command to complete
	checkTriangle(&t,1,1,diago.adjust, insideTriangle,1,0,0);
	eraseTriangle(sst,&t,1,0,0);		// erase the triangle
      }
#endif

      //
      // flush hist buffer into cmdfifo
      //
      GDBG_INFO(0,"Flushing hist buffer before reset\n");
      hb_histFlushAll();
      
      //
      // reset the cmdfifo
      //
      GDBG_INFO(0,"Resetting the cmdfifo\n");
      miscInit1 = GET(sstio->miscInit1);
      SET(sstio->miscInit1,miscInit1|BIT(19));
      dummy = GET(rawlfb[0]);      // wait for pending memory operations to clear
      SET(sstio->miscInit1,miscInit1&~BIT(19));
      
      sst_idle_really(sst);			// wait for the reset to complete
      
      //
      // check cmdfifo registers
      // 
      
      // CMDFIFO 0
      GDBG_INFO(0,"Checking CMDFIFO 0 registers after reset\n");
      val = GET(sstc->cmdFifo0.baseAddrL);
      DIAG_TESTREG32("cmdFifo0.baseAddrL", 0x0, val);
      
      val = GET(sstc->cmdFifo0.baseSize);
      DIAG_TESTREG32("cmdFifo0.baseSize", 0x0, val);
      
      val = GET(sstc->cmdFifo0.readPtrL);
      DIAG_TESTREG32("cmdFifo0.readPtrL", 0x0, val);
      
      val = GET(sstc->cmdFifo0.readPtrH);
      DIAG_TESTREG32("cmdFifo0.readPtrH", 0x0, val);
      
      val = GET(sstc->cmdFifo0.aMin);
      DIAG_TESTREG32("cmdFifo0.aMin", 0x4, val);
      
      val = GET(sstc->cmdFifo0.aMax);
      DIAG_TESTREG32("cmdFifo0.aMax", 0x4, val);
      
      val = GET(sstc->cmdFifo0.depth);
      DIAG_TESTREG32("cmdFifo0.depth", 0x0, val);
      
      val = GET(sstc->cmdFifo0.holeCount);
      DIAG_TESTREG32("cmdFifo0.holeCount", 0x0, val);
      
      // CMDFIFO 1
      GDBG_INFO(0,"Checking CMDFIFO 1 registers after reset\n");
      val = GET(sstc->cmdFifo1.baseAddrL);
      DIAG_TESTREG32("cmdFifo1.baseAddrL", 0x0, val);
      
      val = GET(sstc->cmdFifo1.baseSize);
      DIAG_TESTREG32("cmdFifo1.baseSize", 0x0, val);
      
      val = GET(sstc->cmdFifo1.readPtrL);
      DIAG_TESTREG32("cmdFifo1.readPtrL", 0x0, val);
      
      val = GET(sstc->cmdFifo1.readPtrH);
      DIAG_TESTREG32("cmdFifo1.readPtrH", 0x0, val);
      
      val = GET(sstc->cmdFifo1.aMin);
      DIAG_TESTREG32("cmdFifo1.aMin", 0x4, val);
      
      val = GET(sstc->cmdFifo1.aMax);
      DIAG_TESTREG32("cmdFifo1.aMax", 0x4, val);
      
      val = GET(sstc->cmdFifo1.depth);
      DIAG_TESTREG32("cmdFifo1.depth", 0x0, val);
      
      val = GET(sstc->cmdFifo1.holeCount);
      DIAG_TESTREG32("cmdFifo1.holeCount", 0x0, val);
      
      //
      // check that screen is still black
      //
      
      GDBG_INFO(0,"Testing memory contents after reset\n");
      //    DIAG_DIFFMEMORY();
      
      //
      // set up a new cmdfifo
      //
      GDBG_INFO(0,"Setting up cmdfifo after reset\n");
      setupCmdFifo(sst);
      hb_resetAll();
      
      //
      // draw and check some more triangles
      // 
      
      GDBG_INFO(0,"Drawing triangles after reset\n");
      for (n=0; n<2; n++) {			// do 20 tests
	randomTriangle(&t,diago.tsize,1);	// pick random triangle
	randomRgbaTriangle(&t);			// with random colors
	
	areaTriangle(&t);			// compute the area (before setup)
	setupTriangle(&t,1,0,0);		// setup RGB slopes
	sortTriangle(&t);			// sort it
	printTriangle(2,&t,1,0,0);
	printTriangleSlopes(3,&t,1,0,0);
	gdbg_info(3,"    area = %d\n", t.area);

	// NOTE: if sub-pixel parameter adjustment is OFF we may get
	//	 color overflows/underflows etc
	SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom());
	drawTriangle(sst,&t,1,0,0);

	sst_idle(sst);				// wait for the command to complete
	checkTriangle(&t,1,1,diago.adjust, insideTriangle,1,0,0);
	eraseTriangle(sst,&t,1,0,0);		// erase the triangle
    }
      
      //
      // flush hist buffer into cmdfifo
      //
      GDBG_INFO(0,"Flushing hist buffer after reset\n");
      hb_histFlushAll();
      sst_idle_really(sst);
      
      GDBG_INFO(0,"End of pass\n"); 
      
    }
    
    DIAG_PASS(1);					// check for black screen
}

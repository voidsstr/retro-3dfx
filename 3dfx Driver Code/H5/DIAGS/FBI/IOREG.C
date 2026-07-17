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
** $Date: 10/11/00 8:10:11 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "h3asm.h"

//----------------------------------------------------------------------
// routine to test reading and writing registers
// these are typically called from register32test
//----------------------------------------------------------------------
void my_register32check(volatile FxU32 *reg, FxU16 port, 
			FxU32 mask, FxU32 val, 
			int writePort)
{
    char addr[32];
    unsigned long got;
    
    if (writePort)  {
      SET_IO(port,val);
      sst_idle_really(diago.sst);
      sprintf(addr,"0x%x",(unsigned int)reg);
      got = GET(*reg) & mask;
      DIAG_TESTREG32(addr,val&mask,got);
    } else {
      SET(*reg,val);
      sst_idle_really(diago.sst);
      sprintf(addr,"0x%x",(unsigned int)port);
      got = GET_IO(port) & mask;
      DIAG_TESTPORT32(addr,val&mask,got);
    } 

}

void my_register32test( volatile void *reg, FxU16 port, FxU32 maskRead, FxU32 maskWrite)
{
    int i, n;

    // first write via memory-mapped, reading back thru i/o port
    // then write via i/o port, reading back thru memory-mapped space
    for ( n=0; n<=1; n++ ) {
      my_register32check(reg,port,maskRead,maskWrite & iRandom(maskWrite),n);	// random value
      my_register32check(reg,port,maskRead,0,n);			// zero
      my_register32check(reg,port,maskRead,maskWrite,n);		// one
      my_register32check(reg,port,maskRead,maskWrite & iRandom(maskWrite),n);	// random value
      my_register32check(reg,port,maskRead,maskWrite,n);		// one
      my_register32check(reg,port,maskRead,0,n);			// zero
      for (i=0; i<32; i++)			// walking 1
	my_register32check(reg,port,maskRead,maskWrite &(0x00000001<<i),n);
      for (i=0; i<32; i++)			// walking 0
	my_register32check(reg,port,maskRead,maskWrite &(~(0x00000001<<i)),n);
      for (i=0; i<32; i++)			// walking 1
	my_register32check(reg,port,maskRead,maskWrite &(0x80000000>>i),n);
      for (i=0; i<32; i++)			// walking 0
	my_register32check(reg,port,maskRead,maskWrite &(~(0x80000000>>i)),n);
    }
}

int
main (int argc, char **argv)
{
    FxU16 portBase;
    SstRegs *sst;
    SstIORegs *sstio, *portio;

    sst = SST_BEGIN(argc,argv);
    sstio = (SstIORegs *) SST_IO_ADDRESS(sst);
    portio = (SstIORegs *) ((FxU32)SST_PORT_ADDRESS(sst));

    // HACK!!
    portBase = diago.halInfo->boardInfo[SST_FAKE_ADDRESS_GET_BOARD(sst)].physPort & 0xFFFF;
    GDBG_INFO(0,"ioreg: io base address = 0x%x\n",portBase);

    while (DIAG_STARTPASS()) {			// for each pass

	// Init Registers
#if 0
	//	gdbg_info(2,"test32 status\n");
	//	my_register32test(&sstio->status, (FxU16)(portBase+STATUS), 0x00000000, 0xFFFFFFFF);
	gdbg_info(2,"test32 pciInit0\n");
	my_register32test(&sstio->pciInit0, (FxU16)(portBase+PCIINIT0), 0x07FFFB7C, 0xFFFFFFFF);
	gdbg_info(2,"test32 sipMonitor\n");
	my_register32test(&sstio->sipMonitor, (FxU16)(portBase+SIPMONITOR), 0x7FFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 lfbMemoryConfig\n");
	my_register32test(&sstio->lfbMemoryConfig, (FxU16)(portBase+LFBMEMORYCONFIG), 0x007FFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 miscInit0\n");
	my_register32test(&sstio->miscInit0, (FxU16)(portBase+MISCINIT0), 0xFFFDFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 miscInit1\n");
	my_register32test(&sstio->miscInit1, (FxU16)(portBase+MISCINIT1), 0xFF0FFFF9, 0xFFFFFFFF);
	gdbg_info(2,"test32 dramInit0\n");
	my_register32test(&sstio->dramInit0, (FxU16)(portBase+DRAMINIT0), 0x0FFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 dramInit1\n");
	my_register32test(&sstio->dramInit1, (FxU16)(portBase+DRAMINIT1), 0x3FFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 agpInit\n");
	my_register32test(&sstio->agpInit, (FxU16)(portBase+AGPINIT), 0x000007FE, 0xFFFFFFFF);
	gdbg_info(2,"test32 tmuGbeInit\n");
	my_register32test(&sstio->tmuGbeInit, (FxU16)(portBase+TMUGBEINIT), 0x00007FFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vgaInit0\n");
	my_register32test(&sstio->vgaInit0, (FxU16)(portBase+VGAINIT0), 0x007FFFC7, 0xFFFFFFFF);
	gdbg_info(2,"test32 vgaInit1\n");
	my_register32test(&sstio->vgaInit1, (FxU16)(portBase+VGAINIT1), 0x1FFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 dramCommand\n");
	my_register32test(&sstio->dramCommand, (FxU16)(portBase+DRAMCOMMAND), 0xFFFFFF0F, 0xFFFFFFFF);
	gdbg_info(2,"test32 dramData\n");
	my_register32test(&sstio->dramData, (FxU16)(portBase+DRAMDATA), 0xFFFFFFFF, 0xFFFFFFFF);
#endif
	// PLL Registers

	gdbg_info(2,"test32 pllCtrl0\n");
	my_register32test(&sstio->pllCtrl0, (FxU16)(portBase+PLLCTRL0), 0x0001FFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 pllCtrl1\n");
	my_register32test(&sstio->pllCtrl1, (FxU16)(portBase+PLLCTRL1), 0x0001FFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 pllCtrl2\n");
	my_register32test(&sstio->pllCtrl2, (FxU16)(portBase+PLLCTRL2), 0x0001FFFF, 0xFFFFFFFF);

	// DAC Registers

	gdbg_info(2,"test32 dacMode\n");
	my_register32test(&sstio->dacMode, (FxU16)(portBase+DACMODE), 0x0000001F, 0xFFFFFFFF);
#if 0
	gdbg_info(2,"test32 dacAddr\n");
	my_register32test(&sstio->dacAddr, (FxU16)(portBase+DACADDR), 0x000001FF, 0xFFFFFFFF);
	gdbg_info(2,"test32 dacData\n");
	my_register32test(&sstio->dacData, (FxU16)(portBase+DACDATA), 0x00FFFFFF, 0xFFFFFFFF);
#endif
	// Video Registers I

	gdbg_info(2,"test32 vidMaxRGBDelta\n");
	my_register32test(&sstio->vidMaxRGBDelta, (FxU16)(portBase+VIDMAXRGBDELTA), 0x003F3F3F, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidProcCfg\n");
	my_register32test(&sstio->vidProcCfg, (FxU16)(portBase+VIDPROCCFG), 0xFFFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 hwCurPatAddr\n");
	my_register32test(&sstio->hwCurPatAddr, (FxU16)(portBase+HWCURPATADDR), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 hwCurLoc\n");
	my_register32test(&sstio->hwCurLoc, (FxU16)(portBase+HWCURLOC), 0x07FF07FF, 0xFFFFFFFF);
	gdbg_info(2,"test32 hwCurC0\n");
	my_register32test(&sstio->hwCurC0, (FxU16)(portBase+HWCURC0), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 hwCurC1\n");
	my_register32test(&sstio->hwCurC1, (FxU16)(portBase+HWCURC1), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidInFormat\n");
	my_register32test(&sstio->vidInFormat, (FxU16)(portBase+VIDINFORMAT), 0x003FCFFF, 0xFFFFFFFF);

	#if 0
	//I don't what the f this shit is. There's really nothing about the register
	//in the csim/diag source files.
	gdbg_info(2,"test32 vidInStatus\n");
	my_register32test(&sstio->vidInStatus, (FxU16)(portBase+VIDINSTATUS), 0x00000000, 0xFFFFFFFF);
	#endif

#if 0
	gdbg_info(2,"test32 vidSerialParallelPort\n");
	my_register32test(&sstio->vidSerialParallelPort, (FxU16)(portBase+VIDSERIALPARALLELPORT), 0xFFFFFFFF, 0xFFFFFFFF);
#endif

	gdbg_info(2,"test32 vidInXDecimDeltas\n");
	my_register32test(&sstio->vidInXDecimDeltas, (FxU16)(portBase+VIDINXDECIMDELTAS), 0x0FFF0FFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidInDecimInitErrs\n");
	my_register32test(&sstio->vidInDecimInitErrs, (FxU16)(portBase+VIDINDECIMINITERRS), 0x1FFF1FFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidInYDecimDeltas\n");
	my_register32test(&sstio->vidInYDecimDeltas, (FxU16)(portBase+VIDINYDECIMDELTAS), 0x0FFF0FFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidPixelBufThold\n");
	my_register32test(&sstio->vidPixelBufThold, (FxU16)(portBase+VIDPIXELBUFTHOLD), 0x0003FFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidChromaMin\n");
	my_register32test(&sstio->vidChromaMin, (FxU16)(portBase+VIDCHROMAMIN), 0xFFFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidChromaMax\n");
	my_register32test(&sstio->vidChromaMax, (FxU16)(portBase+VIDCHROMAMAX), 0xFFFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidCurrentLine\n");
	my_register32test(&sstio->vidCurrentLine, (FxU16)(portBase+VIDCURRENTLINE), 0x00000000, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidScreenSize\n");
	my_register32test(&sstio->vidScreenSize, (FxU16)(portBase+VIDSCREENSIZE), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidOverlayStartCoords\n");
	my_register32test(&sstio->vidOverlayStartCoords, (FxU16)(portBase+VIDOVERLAYSTARTCOORDS), 0x0FFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidOverlayEndScreenCoord\n");
	my_register32test(&sstio->vidOverlayEndScreenCoord, (FxU16)(portBase+VIDOVERLAYENDSCREENCOORD), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidOverlayDudx\n");
	my_register32test(&sstio->vidOverlayDudx, (FxU16)(portBase+VIDOVERLAYDUDX), 0x000FFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidOverlayDudxOffsetSrcWidth\n");
	my_register32test(&sstio->vidOverlayDudxOffsetSrcWidth, (FxU16)(portBase+VIDOVERLAYDUDXOFFSETSRCWIDTH), 0xFFFFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidOverlayDvdy\n");
	my_register32test(&sstio->vidOverlayDvdy, (FxU16)(portBase+VIDOVERLAYDVDY), 0x000FFFFF, 0xFFFFFFFF);

	// skip VGA Registers
	
	// Video Registers II
	
	gdbg_info(2,"test32 vidOverlayDvdyOffset\n");
	my_register32test(&sstio->vidOverlayDvdyOffset, (FxU16)(portBase+VIDOVERLAYDVDYOFFSET), 0x0007FFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidDesktopStartAddr\n");
	my_register32test(&sstio->vidDesktopStartAddr, (FxU16)(portBase+VIDDESKTOPSTARTADDR), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidDesktopOverlayStride\n");
	my_register32test(&sstio->vidDesktopOverlayStride, (FxU16)(portBase+VIDDESKTOPOVERLAYSTRIDE), 0x7FFF7FFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidInAddr0\n");
	my_register32test(&sstio->vidInAddr0, (FxU16)(portBase+VIDINADDR0), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidInAddr1\n");
	my_register32test(&sstio->vidInAddr1, (FxU16)(portBase+VIDINADDR1), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidInAddr2\n");
	my_register32test(&sstio->vidInAddr2, (FxU16)(portBase+VIDINADDR2), 0x00FFFFFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidInStride\n");
	my_register32test(&sstio->vidInStride, (FxU16)(portBase+VIDINSTRIDE), 0x00007FFF, 0xFFFFFFFF);
	gdbg_info(2,"test32 vidCurrOverlayStartAddr\n");
	my_register32test(&sstio->vidCurrOverlayStartAddr, (FxU16)(portBase+VIDCURROVERLAYSTARTADDR), 0x00000000, 0xFFFFFFFF);

    }
    DIAG_PASS(0);

    return(0);
}




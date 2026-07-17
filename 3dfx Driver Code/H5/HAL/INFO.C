/*-*-c++-*-*/
#include "vxd.h"
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
** $Revision: 3$
** $Date: 10/11/00 8:31:11 PM$
*/

#include <stdlib.h>
#include <stdio.h>

#include <h3.h>

#ifndef WINSIM
#include <fxagp.h>
#endif // WINSIM

#ifdef HAL_CSIM
#include "../csim/csim.h"
#endif

#include "init.h"
#ifdef HAL_HSIM
#include "tstbench.h"
#endif

// get and translate a simple numeric environment variable
static int genv(const char *name, const int defaultVal, const char *msg)
{
    int level=3, val=defaultVal;

#ifndef KERNEL
    if (GETENV(name)) {
        SSCANF(GETENV(name), "%i", &val);
        level = 0;
    }
#endif
    if (val) GDBG_INFO(level,msg,val);
    return val;
}

static void getTmuConfigData( SstRegs *sst, FxDeviceInfo *info )
{
    info->tmuConfig = 0x000c58; // XXX GMT: hardcoded for my board
    info->tmuRevision = 4;      // PS verify this value
}

//---------------------------------------------------------------------------
// get and return TMU information, either from HW, CSIM, or HSIM
// NOTES:
//      assumes that board and registers are initialized
//      destroys part of the framebuffer
//---------------------------------------------------------------------------

static
FxBool fxHalGetTmuInfo( SstRegs *sst, FxU32 bn, FxDeviceInfo *info )
{
    if (halInfo.hw) {
        // first get the HW config and memsize
        //    initSumTables(sst);
        getTmuConfigData(sst,info);
        info->tmuMemSize[0] = 0;

        // count the TMUs and verify that all TMUs are the same revision
	if (( info->deviceID == SST_DEVICE_ID_H4 ) ||
	    ( info->deviceID == SST_DEVICE_ID_H4_OEM ) ||
	    ( info->deviceID > SST_DEVICE_ID_AP) ||
	    ( info->deviceID > SST_DEVICE_ID_AP_OEM)) {
	  info->numberTmus = 2;
	} else if ( info->deviceID == SST_DEVICE_ID_H3 ) {
	  info->numberTmus = 1;
	} else {
	  GDBG_ERROR("fxHalGetTmuInfo","unable to determine num TMUs for deviceID=0x%x\n",
		     info->deviceID);
	  return(FXFALSE);
	}
    }
    else if (!halInfo.hsim)
        info->numberTmus = DEAD;

    // now set the CSIM revision and memsize and compare CSIM and HW
#if HAL_CSIM
    if (halInfo.csim) {
        csimTmuSetMemory(halInfo.boardInfo[bn].sstCSIM );
    }
#endif /* HAL_CSIM */
    GDBG_INFO(201,"fxHalGetTmuInfo:  numberTMus = %d\n", info->numberTmus);

    return(FXTRUE);
}

/*
** fbiMemSize():
**  Returns size (in MBytes) of FBI frame buffer memory (HW)
**  Returns 0 on error
**  NOTE: fbiMemSize() destroys the contents in memory
**
*/
static FxU32 fbiMemSize( SstRegs *sst )
{
  SstIORegs *sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  FxU32 dramInit0 = IGET(sstio->dramInit0);
  FxU32 nbanks, banksize;

  nbanks = ( (dramInit0 & SST_SGRAM_NUM_CHIPSETS) == 0 ) ? 1 : 2;
  if ( (dramInit0 & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_8MBIT ) {
    banksize = 4;
  } else if ( (dramInit0 & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_16MBIT ) {
    banksize = 8;
  } else if ( (dramInit0 & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_32MBIT ) {
    banksize = 16;
  } else if ( (dramInit0 & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_64MBIT ) {
    banksize = 32;
  } else if ( (dramInit0 & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_128MBIT ) {
    banksize = 64;
  } else {
    banksize = 0;
    GDBG_ERROR("fbiMemSize","invalid sgram type = 0x%x\n",
	       (dramInit0&SST_SGRAM_TYPE)<<SST_SGRAM_TYPE_SHIFT);
  }

  GDBG_INFO(187, "fbiMemSize() = %d nbanks * %d banksize = %d\n", 
	    nbanks, banksize, nbanks * banksize);



    return nbanks*banksize;   // XXX get real HW info here, for now return 4 MB
}

//---------------------------------------------------------------------------
// get and return FBI information, either from HW, CSIM, or HSIM
//---------------------------------------------------------------------------
static
FxBool fxHalGetFbiInfo( SstRegs *sst, FxU32 bn, FxDeviceInfo *info)
{
  //    FxU32 init1 = IGET(sst->fbiInit1);  // PS HACK -- moved below 

#if XXX
// GMT: this whole area needs some more work
    if (halInfo.csim) {         // compare HW bits with CSIM
        FxU32 init1 = IGET(sst->fbiInit1);
        // GMT: we might want to init CSIM to match HW
        if ((init1 ^ halInfo.csimLastRead) & SST_SLI_DETECT) {
            GDBG_ERROR("fxHalGetFbiInfo",
                "CSIM(%d) and HW(%d) are configured differently for SLI\n",
                (halInfo.csimLastRead & SST_SLI_DETECT)!=0,
                (init1 & SST_SLI_DETECT)==0);
            return FXFALSE;
        }
    }
#endif
    // first get the HW memsize
    if (halInfo.hw)
        info->fbiMemSize = fbiMemSize(sst);
    

#ifdef HAL_HSIM
    if (halInfo.hsim) {
        void *hmem;
	CsimPrivate *cp = CSIM_PRIVATE(info->sstCSIM);

        HSIM_GET_DEVICE_INFO(info);     // get HSIM config info
        hmem = HSIM_GET_SHMEM();        // get HSIM dram pointer
        GDBG_INFO(3,"hsim memory = 0x%x\n",hmem);
	
	hmem = (void *)((FxU32)hmem + (cp->environment.chipIndex * info->fbiMemSize * 1024 * 1024));
	GDBG_INFO(3, "hsim memory adjusted to 0x%x for chipIndex %d\n",
		  hmem, cp->environment.chipIndex);

        CSIM_PRIVATE(halInfo.boardInfo[bn].sstCSIM)->hsim_memory = hmem;
    }
#endif
#ifdef CVG
    /* Detect board identification and memory speed */
    info->fbiConfig = (IGET(sst->fbiInit3) & SST_FBI_MEM_TYPE) >>
                        SST_FBI_MEM_TYPE_SHIFT;
    info->fbiConfig = genv("SST_FBICFG",info->fbiConfig,"     settting SST_FBICFG = 0x%x\n");

    info->fbiBoardID = (info->fbiConfig >> 2) & 0x1;
    /* fbiMemSpeed is legacy, and is not used by either Obsidian GE or Pro Fab */
    info->fbiMemSpeed = 0;

    /* Detect scanline interleaving */
    info->sstSliDetected = 0;// XXX sst1InitSliDetect(sst);
#endif

#if HAL_CSIM
    // then set the CSIM memsize and compare CSIM and HW
    if (halInfo.csim) {
        csimFbiSetRevision(halInfo.boardInfo[bn].sstCSIM);
        csimFbiSetMemory(halInfo.boardInfo[bn].sstCSIM);
    }
#endif /* HAL_CSIM */

    return FXTRUE;
}

#ifndef CVG

//---------------------------------------------------------------------------
// get and return AGP information, either from HW, CSIM, or HSIM
//---------------------------------------------------------------------------
static
FxBool fxHalGetAgpInfo( SstRegs *sst, FxU32 bn, FxDeviceInfo *info)
{
  CsimPrivate *cp = CSIM_PRIVATE(halInfo.boardInfo[bn].sstCSIM);
  char *buf;

  if (halInfo.hw) {
#ifdef HAL_HW
    FxU32 linear, phys, pciID;

    if ( (buf=GETENV("HAL_AGP")) != NULL && atoi(buf) == 1 ) {      // init agp

      // init AGP interface
      if ( ! fxAGPInit() ) {
	GDBG_ERROR("fxHalGetAgpInfo","%s",fxAGPGetErrorString());
	return FXFALSE;
      }
      
      // get desired agp mem size
      if ( buf = GETENV("HAL_AGP_MEM") )    // agp memory size in bytes
	info->agpSizeInBytes = atoi(buf) * 1024;
      else
	info->agpSizeInBytes = 64 * 1024;
      
      info->agpSizeInBytes &= ~SST_MASK(12);         // page align
      
      // reserve and commit agp memory
      pciID = (info->deviceID<<16)|info->vendorID;
      GDBG_INFO(0,"info: pciID = 0x%x\n",pciID);
      if ( ! fxAGPReserve(info->agpSizeInBytes>>12, &linear, &phys, pciID) ) {
	GDBG_ERROR("fxHalGetAgpInfo","%s",fxAGPGetErrorString());
	return FXFALSE;
      }
      
      if ( ! fxAGPCommit(linear, 0, info->agpSizeInBytes>>12) ) {
	GDBG_ERROR("fxHalGetAgpInfo","%s",fxAGPGetErrorString());
	return FXFALSE;
      }
      
      info->agpMem = (FxU8 *) linear;
      info->agpVirtAddr = (FxU8 *) ((bn+1)<<28);
      info->agpBaseAddrH = 0x0;
      info->agpBaseAddrL = phys;
      info->agpRqDepth = 0x7;
      
    }
#endif
  }
#ifdef HAL_HSIM
  else if (halInfo.hsim) {
    // the relevant members of info have already been filled by fxHalGetFbiInfo()
    info->agpVirtAddr = (bn+1)<<28;
  }
#endif
  else {  // csim

    if ( buf = GETENV("HAL_AGP_MEM") )    // agp memory size in bytes
      info->agpSizeInBytes = atoi(buf) * 1024;
    else
      info->agpSizeInBytes = 64 * 1024;
      
    if ( info->agpSizeInBytes ) {
      info->agpMem = (FxU8 *) malloc( info->agpSizeInBytes * sizeof(FxU8) );
      if ( info->agpMem == NULL ) {
        GDBG_ERROR("fxHalGetAgpInfo","malloc of AGP memory FAILED\n");
        return FXFALSE;
      }
      info->agpVirtAddr = (FxU8 *) ((bn+1)<<28);
      info->agpBaseAddrH = 0x0;
      info->agpBaseAddrL = 0x0;
      info->agpRqDepth = 0x0;
    }

  }
  
  cp->agpVirtAddr = info->agpVirtAddr;  // csim must access agp thru a fake addr
  
  GDBG_INFO(3,"agp memory = 0x%x, Base hi:lo = 0x%01x:%08x, size = %d bytes\n",
            info->agpMem,info->agpBaseAddrH,info->agpBaseAddrL,info->agpSizeInBytes);

  return FXTRUE;
}

#endif
/*
** fxHalFillDeviceInfo():
**  Fill in device information
**  NOTE: This routine destroys current contents in frame buffer memory
**
*/
FxBool fxHalFillDeviceInfo( SstRegs *sst )
{
    FxU32 bn;
    FxDeviceInfo *info;

    GDBG_INFO(1,"fxHalFillDeviceInfo(0x%x)\n",sst);
    if (fxHalVaddrToBoardNumber( sst, &bn ))    // find the board
        info = &halInfo.boardInfo[bn];
    else
        return(FXFALSE);

    if(info->tmuRevision != DEAD)
        return(FXTRUE);
    if(GETENV("SST_NODEVICEINFO")) {
        /* fill device info struct with sane values... */
        GDBG_INFO(2,"fxHalFillDeviceInfo: Filling info Struct with default values...\n");

        info->fbiConfig = genv("SST_FBICFG",0,"     setting SST_FBICFG to 0x%x\n");
        info->numberTmus = 1;
        info->tmuRevision = 4;   // PS verify this value

        info->fbiMemSize = genv("SST_FBIMEM_SIZE",2,"     setting SST_FBIMEM_SIZE to %d\n");
    }
    else {
        int i;  /* retry counter */

        for (i=0; i<5; i++) {
            if(i)
                GDBG_PRINTF("fxHalFillDeviceInfo(): Retry #%d for chip GetInfo()...\n", i);
            if (fxHalGetFbiInfo(sst, bn, info) == FXFALSE)
                continue;
#ifndef CVG
            if (fxHalGetAgpInfo(sst, bn, info) == FXFALSE)
                continue;
#endif
            // get the revision ID of each TMU and verify that they are all the same
            if (fxHalGetTmuInfo(sst, bn, info) == FXFALSE)
                continue;
            break;
        }
        if (i == 5)             /* if we exceeded retry count */
            return FXFALSE;
    }

#ifdef CVG

    GDBG_INFO(2,"FxDeviceInfo: Board ID: Obsidian %s\n",
                info->fbiBoardID ? "PRO" : "GE");
    GDBG_INFO(2,"FxDeviceInfo: FbiConfig:0x%x, TmuConfig:0x%x\n",
                info->fbiConfig, info->tmuConfig);
#endif
    GDBG_INFO(2,"FxDeviceInfo: FBI Revision:%d, TMU Revison:%d, Num TMUs:%d\n",
                info->fbiRevision, info->tmuRevision, info->numberTmus);
    GDBG_INFO(2,"FxDeviceInfo: FBI Memory:%d, TMU[0] Memory:%d",
                info->fbiMemSize, info->tmuMemSize[0]);
    if (info->numberTmus > 1)
        GDBG_INFO_MORE(2,", TMU[1] Memory:%d", info->tmuMemSize[1]);
    if (info->numberTmus > 2)
        GDBG_INFO_MORE(2,", TMU[2] Memory:%d", info->tmuMemSize[2]);
    GDBG_INFO_MORE(2,"\n");

    return(FXTRUE);
}

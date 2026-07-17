/* -*-c++-*- */
/* $Header:
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   sliaa.c
**
** Description: Routines to Initialize SLI AA for Napalm.
**
** $Revision: 26$
** $Date: 10/11/00 8:55:04 PM$
**
** $History: sliaa.c $
** 
** *****************  Version 10  *****************
** User: Cwilcox      Date: 9/09/99    Time: 5:05p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixed lfbMemoryConfig accesses to support 32Mb and 64Mb configs.
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 8/24/99    Time: 5:44p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixed a problem with SLICTRL and 4-way SLI
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 8/20/99    Time: 11:43a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Minor change to disable aa when SLI and AA are disabled.
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 7/30/99    Time: 2:00p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixed a bug with rendermask and SLI copies
** 
** *****************  Version 6  *****************
** User: Andrew       Date: 7/28/99    Time: 3:46p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixes to support SLI on WINSIM
** 
** *****************  Version 5  *****************
** User: Andrew       Date: 7/26/99    Time: 5:53p
** Updated in $/devel/h5/Win9x/dx/minivdd
** fixed some bugs and added code to setup the fb for entering SLI mode
** and leaving sli mode
** 
** *****************  Version 4  *****************
** User: Andrew       Date: 7/20/99    Time: 10:57a
** Updated in $/devel/h5/Win9x/dx/minivdd
** code to init SLICTRL
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
** 
** *****************  Version 2  *****************
** User: Andrew       Date: 7/07/99    Time: 2:43p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Dibengine workaround
** 
** *****************  Version 1  *****************
** User: Andrew       Date: 6/25/99    Time: 10:05a
** Created in $/devel/h5/Win9x/dx/minivdd
** Code to enable/disable SLI/AA
** 
**
*/


#ifdef H3VDD
#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#include "h3irq.h"
#undef  VDDONLY

#include <sliaa.h>
#include <gpio.h>

#ifdef USE_FAKE_PHYS
#include <..\sliaa\devctl.h>
#endif

#include "p6stuff.h"
#include "h3cinit.h"

// Fix for building with Windows 98 DDK (Needs BOOLEAN type)
typedef BYTE BOOLEAN;
#undef WANTVXDWRAPS
#include <mtrr.h>
#endif

#ifdef H3VDD

DWORD our_mem_rdd(DWORD);
void our_mem_wrd(DWORD, DWORD);
void our_memcpy(BYTE * pDest, BYTE * pSrc, DWORD dwSize);
DWORD PCI_Write_Config(DWORD dwBus, DWORD dwDevFunc, DWORD dwOffset, DWORD dwValue);
DWORD PCI_Read_Config(DWORD dwBus, DWORD dwDevFunc, DWORD dwOffset);

#ifdef DEBUG
#define DEBUG_PRINTF Debug_Printf
#else
#define DEBUG_PRINTF()
#endif
#define PCI_IO_WR our_mem_wrd
#define PCI_IO_RD our_mem_rdd
#define PCI_CFG_WR(A, B, C) PCI_Write_Config(pDevTable->dwBus, pDevTable->dwDevFunc, A, B)
#define PCI_CFG_RD(A, B) PCI_Read_Config(pDevTable->dwBus, pDevTable->dwDevFunc, A)
#else
#define DEBUG_PRINT printf
#endif

#ifndef DWORD
typedef unsigned long int DWORD;
#endif

// H3VDD is to distingush that this code is used in the minivdd
#ifdef H3VDD
#endif

// There is a bug in A0 silcon wrt SLI mode.  If a read abort <a unowned read>
// is done by the master or slave when the CMDFIFO is running then the CMDFIFO will
// get hosed.  A unowned read is (A) Master -- read of unowned Tile; (B) Slave -- read of unowned Tile
// or linear memory.  To get around this we run with SLI_LFB_RD_EN disabled.

#define MIN(A,B) ((A) <= (B) ? (A) : (B))

/*----------------------------------------------------------------------
Function name:  H3_DISABLE_SLI_AA

Description:    Disables SLI and AA on Napalm

Information:

Return:    Nothing     
                
----------------------------------------------------------------------*/
void H3_DISABLE_SLI_AA(PCHIPINFO pChipInfo, PSLI_AA_MEMINFO pMemInfo)
{
    DWORD i;

#ifdef H3VDD
   PDEVTABLE pDevTable;
   DWORD h3ChipSetupIoBase;
   DWORD h3ChipSetup3DBase;
   DWORD h3ChipSetupDeviceNum;
#endif

   (void)pMemInfo;

#ifdef H3VDD
    for(pDevTable=(PDEVTABLE)pChipInfo->pDevTable,i=1; i<=pChipInfo->dwChips; i++,pDevTable=pDevTable->pSlave) 
      {
      h3ChipSetupIoBase = pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX];
      h3ChipSetup3DBase = pDevTable->RegBase[HWINFO_SST_3DREGS_INDEX];
      if (SLI_AA_MASTER_DEVICE != pDevTable->dwType)
          h3ChipSetupDeviceNum = i;
#else
    for(i=1; i<=pChipInfo->dwChips; i++) 
      {
      h3ChipSetupIoBase = IOBASE - (0x100 * (i-1));
      if(!vconfigLookup("h3_sliMultiFunction", NULL))
         h3ChipSetupDeviceNum = i;
      else
         PCI_CFG_SET_FNCTN(i-1);
#endif

      PCI_CFG_WR(CFG_INIT_ENABLE,
        (PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) &
         ~(CFG_SNOOP_MEMBASE0 | CFG_SNOOP_EN | CFG_SNOOP_MEMBASE0_EN |
           CFG_SNOOP_MEMBASE1_EN | CFG_SNOOP_SLAVE |
           CFG_SNOOP_FBIINIT_WR_EN | CFG_SWAP_ALGORITHM | CFG_SWAP_QUICK)),
           h3ChipSetupDeviceNum);

      PCI_CFG_WR(CFG_SLI_LFB_CTRL,
        (PCI_CFG_RD(CFG_SLI_LFB_CTRL, h3ChipSetupDeviceNum) &
             ~(CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN | CFG_SLI_RD_EN)),
            h3ChipSetupDeviceNum);

#ifdef H3VDD
      PCI_IO_WR(h3ChipSetup3DBase + SLICTRL, 0x0);
      PCI_IO_WR(h3ChipSetup3DBase + AACTRL, 0x0);
#endif

      PCI_CFG_WR(CFG_AA_LFB_CTRL,
        (PCI_CFG_RD(CFG_AA_LFB_CTRL, h3ChipSetupDeviceNum) &
             ~(CFG_AA_LFB_CPU_WR_EN | CFG_AA_LFB_DPTCH_WR_EN | CFG_AA_LFB_RD_EN)),
            h3ChipSetupDeviceNum);

      PCI_CFG_WR(CFG_SLI_AA_MISC,
             ((PCI_CFG_RD(CFG_SLI_AA_MISC, h3ChipSetupDeviceNum) &
               ~(CFG_VGA_VSYNC_OFFSET | CFG_AA_LFB_RD_SLV_WAIT)) |
              (0x0 << CFG_VGA_VSYNC_OFFSET_PIXELS_SHIFT) |
              (0x0 << CFG_VGA_VSYNC_OFFSET_CHARS_SHIFT) |
              (0x0 << CFG_VGA_VSYNC_OFFSET_HXTRA_SHIFT)),
            h3ChipSetupDeviceNum);

      // Tristate slave Hsync and Vsync's       
      if (1 != i)
         {
         PCI_CFG_WR(CFG_VIDEO_CTRL0, CFG_DAC_VSYNC_TRISTATE | CFG_DAC_HSYNC_TRISTATE, h3ChipSetupDeviceNum);
         }
      else
         {
         PCI_CFG_WR(CFG_VIDEO_CTRL0, 0x0, h3ChipSetupDeviceNum);
         }
   
      PCI_CFG_WR(CFG_VIDEO_CTRL1, 0x0, h3ChipSetupDeviceNum);
      PCI_CFG_WR(CFG_VIDEO_CTRL2, 0x0, h3ChipSetupDeviceNum);

      // This results in a different value then boot and should be verified
      // when we get real hardware      
#ifdef PCI_LATENCY_WHQL
      if (pDevTable->dwPCI)
         {
         if (pChipInfo->dwChips > 1)
            {
            PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
              ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_DISABLE_IO|SST_PCI_DISABLE_MEM|SST_PCI_RETRY_INTERVAL)) |
            (0<<SST_PCI_RETRY_INTERVAL_SHIFT) | SST_PCI_FORCE_FB_HIGH));
            }
         else
            {
            PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
            ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_DISABLE_IO|SST_PCI_DISABLE_MEM|SST_PCI_RETRY_INTERVAL)) |
            (0<<SST_PCI_RETRY_INTERVAL_SHIFT)));
            }
         }   
#endif

      // Kill the slave since the DAC are tied together
      if (i > 1)
         {
         PCI_IO_WR(h3ChipSetupIoBase+DACMODE, SST_DAC_DPMS_ON_VSYNC | SST_DAC_DPMS_ON_HSYNC); 
         PCI_IO_WR(h3ChipSetupIoBase+VIDPROCCFG, (PCI_IO_RD(h3ChipSetupIoBase+VIDPROCCFG) & ~SST_VIDEO_PROCESSOR_EN));
         }   
      }

   return;
}

/*----------------------------------------------------------------------
Function name:  H3_SETUP_SLI_AA

Description:    Sets up SLI and AA on Napalm

Information:

Return:    Nothing     
                
----------------------------------------------------------------------*/
#ifdef H3VDD
void GetStrideAndAperature(DWORD lpDriverData, DWORD * pStrideAndAperature);
#endif
void H3_SETUP_SLI_AA(DWORD dwFunc, PCHIPINFO pChipInfo, PSLI_AA_MEMINFO pMemInfo)
   // nChips: Number of chips in multi-chip configuration (1-4)
   // sliEn: Sli is to be enabled (0,1)
   // aaEn: Anti-aliasing is to be enabled (0,1)
   // aaSampleHigh: 0->Enable 2-sample AA, 1->Enable 4-sample AA
   //               2->Enable 8-sample AA
   // sliAaAnalog: 0->Enable digital SLI/AA, 1->Enable analog Sli/AA
   // sli_nLines: Number of lines owned by each chip in SLI (2-128)
   // fbMem: Amount of memory, in megabytes...
{
    DWORD sli_renderMask, sli_compareMask, sli_scanMask;
    DWORD sli_nLinesLog2, nChipsLog2;
    DWORD i;
    DWORD aaClkOutDel;
    DWORD aaSample8x = (pChipInfo->dwaaSampleHigh == 2);
    DWORD dwStrideAndAperature;
    DWORD dwClkDbl;
#ifdef H3VDD
   PDEVTABLE pDevTable;
   DWORD MEMBASE0; 
   DWORD MEMBASE1; 
   DWORD h3ChipSetupIoBase;
   DWORD h3ChipSetup3DBase;
   DWORD h3ChipSetupDeviceNum;
   DWORD dwFormat;
#endif

#ifndef H3VDD
    DEBUG_PRINTF("H3_SETUP_SLI_AA(): Called at time %d ns...\n", get_hdl_time());
#endif
    DEBUG_PRINTF("   nChips:%d, sliEn:%d, aaEn:%d, aaSampleHigh:%d\n",
      pChipInfo->dwChips, pChipInfo->dwsliEn, pChipInfo->dwaaEn, pChipInfo->dwaaSampleHigh);
    DEBUG_PRINTF("   sliAaAnalog:%d, sli_nLines:%d\n",
      pChipInfo->dwsliAaAnalog, pChipInfo->dwsli_nlines);
#ifndef H3VDD
    fflush(stdout);
#endif


    if (SLI_AA_DISABLE == dwFunc)
      {
      H3_DISABLE_SLI_AA(pChipInfo, pMemInfo);
      return;
      }

#ifdef H3VDD
    GetStrideAndAperature(((PDEVTABLE)pChipInfo->pDevTable)->lpDriverData, &dwStrideAndAperature);
#else
    dwStrideAndAperature = (0x0a << 16);
#endif

    // Some sanity checking...
    if((pChipInfo->dwChips == 0) || (pChipInfo->dwChips == 3) || (pChipInfo->dwChips > 4) ||
       (pChipInfo->dwChips == 1 && (pChipInfo->dwsliEn || pChipInfo->dwaaSampleHigh)) ||
       (pChipInfo->dwChips == 2 && (pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh)) ||
       (aaSample8x && !(pChipInfo->dwChips == 4 && ! pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwsliAaAnalog))) {
          DEBUG_PRINTF("H3_SETUP_SLI_AA() ERROR: Unsupported input params...\n");
#ifndef H3VDD
          fflush(stdout);
          TESTBENCH_FAIL();
#endif
    }

    switch(pChipInfo->dwsli_nlines) {
       case 2: sli_nLinesLog2 = 1; break;
       case 4: sli_nLinesLog2 = 2; break;
       case 8: sli_nLinesLog2 = 3; break;
       case 16: sli_nLinesLog2 = 4; break;
       case 32: sli_nLinesLog2 = 5; break;
       case 64: sli_nLinesLog2 = 6; break;
       case 128: sli_nLinesLog2 = 7; break;
       default:
         DEBUG_PRINTF("H3_SETUP_SLI_AA() ERROR: Unsupported sli_nLines=%d...\n",
            pChipInfo->dwsli_nlines);
#ifndef H3VDD
         fflush(stdout);
         TESTBENCH_FAIL();
#endif
         break;
    }
    switch(pChipInfo->dwChips) {
       case 1: nChipsLog2 = 0; break;
       case 2: nChipsLog2 = 1; break;
       case 4: nChipsLog2 = 2; break;
       case 8: nChipsLog2 = 3; break;
       default: 
          DEBUG_PRINTF("H3_SETUP_SLI_AA() ERROR:  Unsupported nChips=%d...\n",
            pChipInfo->dwChips);
#ifndef H3VDD
          fflush(stdout);
          TESTBENCH_FAIL();
#endif
          break;
    }

    // Each chip has now been setup.  Now connect them all together with SLI...
#ifndef H3VDD
    DEBUG_PRINTF("H3_SETUP_SLI_AA(): Setting up SLI/AA at time %d ns...\n",
       get_hdl_time());
    fflush(stdout);
#endif

#ifdef H3VDD
    MEMBASE0 = ((PDEVTABLE)pChipInfo->pDevTable)->PhysMemBase[0];
    MEMBASE1 = ((PDEVTABLE)pChipInfo->pDevTable)->PhysMemBase[1];
    for(pDevTable=(PDEVTABLE)pChipInfo->pDevTable,i=1; i<=pChipInfo->dwChips; i++,pDevTable=pDevTable->pSlave) {
       h3ChipSetupIoBase = pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX];
       h3ChipSetup3DBase = pDevTable->RegBase[HWINFO_SST_3DREGS_INDEX];
       if (SLI_AA_MASTER_DEVICE != pDevTable->dwType)
          h3ChipSetupDeviceNum = i;
#else
    for(i=1; i<=pChipInfo->dwChips; i++) {
      h3ChipSetupIoBase = IOBASE - (0x100 * (i-1));
       if(!vconfigLookup("h3_sliMultiFunction", NULL))
          h3ChipSetupDeviceNum = i;
       else
          PCI_CFG_SET_FNCTN(i-1);
#endif
       dwClkDbl = PCI_IO_RD(h3ChipSetupIoBase+VIDPROCCFG) & SST_VIDEO_2X_MODE_EN;
       // Each chip MUST must use 2ws reads and 1ws writes.  Snooping does not
       // work with 0ws writes or 1ws reads...
       // Also, each chip MUST use at least a retry interval of ~12...
       // (0x7 gets added to the timeout interval by the chip automatically...)
      // VIA work better without retries
      if (FALSE == ((PDEVTABLE)pChipInfo->pDevTable)->dwVIACoreLogic)
         {
          PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
            ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_RETRY_INTERVAL|SST_PCI_FORCE_FB_HIGH)) |
              SST_PCI_READ_WS | SST_PCI_WRITE_WS | SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM |
              (5<<SST_PCI_RETRY_INTERVAL_SHIFT)));
         }
      else
         {
          PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
            ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_RETRY_INTERVAL|SST_PCI_FORCE_FB_HIGH)) |
              SST_PCI_READ_WS | SST_PCI_WRITE_WS ));
         }

       // This will have to be set properly once hw comes back...For now, use
       // a value that works for simulation...
       aaClkOutDel = 0x2;
       PCI_IO_WR(h3ChipSetupIoBase+TMUGBEINIT,
         ((PCI_IO_RD(h3ChipSetupIoBase+TMUGBEINIT) & ~(SST_AA_CLK_DELAY | SST_AA_CLK_INVERT)) |
          (aaClkOutDel << SST_AA_CLK_DELAY_SHIFT) | SST_AA_CLK_INVERT));

       // Setup buffer swapping to use external sli_syncin/sli_syncout signals
       if(pChipInfo->dwChips > 1) {
#ifdef H3VDD
          PCI_CFG_WR(CFG_INIT_ENABLE,
            (PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) |
             ((pChipInfo->dwCfgSwapAlgorithm & 0x01) << CFG_SWAPBUFFER_ALGORITHM_SHIFT) |
             CFG_SWAP_ALGORITHM | ((i == 1) ? CFG_SWAP_MASTER : 0x0)),
            h3ChipSetupDeviceNum);
#else
          PCI_CFG_WR(CFG_INIT_ENABLE,
            (PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) |
             CFG_SWAP_ALGORITHM | ((i == 1) ? CFG_SWAP_MASTER : 0x0)),
            h3ChipSetupDeviceNum);
#endif
       }

       // Setup snooping...
       if(i == 1 && pChipInfo->dwChips > 1)
          PCI_CFG_WR(CFG_INIT_ENABLE,
            (PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) | CFG_SNOOP_EN),
             h3ChipSetupDeviceNum);
       else if(pChipInfo->dwChips > 1) {
          // For real hardware, MEMBASE0 and MEMBASE1 need to be replaced
          // with the memBaseAddr0 and memBaseAddr1 addresses of the
          // first chip (i.e. the Master chip), respectively.
          // Also, we may not need to run with CFG_SWAP_QUICK with real
          // hardware...
          PCI_CFG_WR(CFG_INIT_ENABLE,
            ((PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) &
               ~CFG_SNOOP_MEMBASE0) | CFG_SNOOP_EN | CFG_SNOOP_MEMBASE0_EN |
              CFG_SNOOP_MEMBASE1_EN | CFG_SNOOP_SLAVE |
              CFG_SNOOP_FBIINIT_WR_EN |
              (((MEMBASE0>>22)&0x3ff) << CFG_SNOOP_MEMBASE0_SHIFT) |
              ((pChipInfo->dwChips > 2) ? CFG_SWAP_QUICK : 0x0)),
            h3ChipSetupDeviceNum);
          PCI_CFG_WR(CFG_PCI_DECODE,
            ((PCI_CFG_RD(CFG_PCI_DECODE, h3ChipSetupDeviceNum) &
               ~CFG_SNOOP_MEMBASE1) |
              (((MEMBASE1>>22)&0x3ff) << CFG_SNOOP_MEMBASE1_SHIFT)),
            h3ChipSetupDeviceNum);
       }

       // Setup cfgSliLfbCtrl
       if(pChipInfo->dwsliEn && (!pChipInfo->dwaaEn || !pChipInfo->dwaaSampleHigh)) {
          // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples spread
          // over 2 chips, then with each pair analog SLI'ed...
          if(pChipInfo->dwChips == 4 && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog) {
             sli_renderMask = ((pChipInfo->dwChips>>1)-1) << sli_nLinesLog2;
             sli_compareMask = ((i-1)>>1) << sli_nLinesLog2;
             sli_scanMask = pChipInfo->dwsli_nlines - 1;
#ifdef RD_ABORT_ERROR
             PCI_CFG_WR(CFG_SLI_LFB_CTRL,
               ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                ((nChipsLog2-1) << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
                CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN),
                h3ChipSetupDeviceNum);
#else
          PCI_CFG_WR(CFG_SLI_LFB_CTRL,
              ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
               (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
               (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
               ((nChipsLog2-1) << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
               CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN | CFG_SLI_RD_EN),
               h3ChipSetupDeviceNum);
#endif

#ifdef H3VDD
          PCI_IO_WR(h3ChipSetup3DBase + SLICTRL,
             ((sli_renderMask << SLICTL_3D_RENDERMASK_SHIFT) |
              (sli_compareMask << SLICTL_3D_COMPAREMASK_SHIFT) |
              (sli_scanMask << SLICTL_3D_SCANMASK_SHIFT) |
              ((nChipsLog2-1) << SLICTL_3D_NUMCHIPS_LOG2_SHIFT) |
              SLICTL_3D_EN));
#endif
          } else {
             sli_renderMask = (pChipInfo->dwChips-1) << sli_nLinesLog2;
             sli_compareMask = (i-1) << sli_nLinesLog2;
             sli_scanMask = pChipInfo->dwsli_nlines - 1;
#ifdef RD_ABORT_ERROR
             PCI_CFG_WR(CFG_SLI_LFB_CTRL,
               ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                (nChipsLog2 << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
                CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN),
                h3ChipSetupDeviceNum);
#else
          PCI_CFG_WR(CFG_SLI_LFB_CTRL,
              ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
               (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
               (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
               (nChipsLog2 << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
               CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN | CFG_SLI_RD_EN),
               h3ChipSetupDeviceNum);
#endif

#ifdef H3VDD
          PCI_IO_WR(h3ChipSetup3DBase + SLICTRL,
             ((sli_renderMask << SLICTL_3D_RENDERMASK_SHIFT) |
              (sli_compareMask << SLICTL_3D_COMPAREMASK_SHIFT) |
              (sli_scanMask << SLICTL_3D_SCANMASK_SHIFT) |
              (nChipsLog2 << SLICTL_3D_NUMCHIPS_LOG2_SHIFT) |
              SLICTL_3D_EN));
#endif
          }
       } else if(!pChipInfo->dwsliEn && pChipInfo->dwaaEn) {
          sli_renderMask = 0x0;
          sli_compareMask = 0x0;
          sli_scanMask = 0x0;
          PCI_CFG_WR(CFG_SLI_LFB_CTRL,
            ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
             (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
             (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
             (0x0 << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT)),
            h3ChipSetupDeviceNum);
#ifdef H3VDD
          PCI_IO_WR(h3ChipSetup3DBase + SLICTRL,
            ((sli_renderMask << SLICTL_3D_RENDERMASK_SHIFT) |
             (sli_compareMask << SLICTL_3D_COMPAREMASK_SHIFT) |
             (sli_scanMask << SLICTL_3D_SCANMASK_SHIFT) |
             (0x0 << SLICTL_3D_NUMCHIPS_LOG2_SHIFT)));
#endif
       } else {
          // pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh
          sli_renderMask = ((pChipInfo->dwChips>>1)-1) << sli_nLinesLog2;
          sli_compareMask = ((i-1)>>1) << sli_nLinesLog2;
          sli_scanMask = pChipInfo->dwsli_nlines - 1;

#ifdef RD_ABORT_ERROR
          PCI_CFG_WR(CFG_SLI_LFB_CTRL,
            ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
             (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
             (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
             ((nChipsLog2-1) << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
             CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN),
            h3ChipSetupDeviceNum);
#else
          PCI_CFG_WR(CFG_SLI_LFB_CTRL,
            ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
             (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
             (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
             ((nChipsLog2-1) << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
             CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN | CFG_SLI_RD_EN),
            h3ChipSetupDeviceNum);
#endif

#ifdef H3VDD
          PCI_IO_WR(h3ChipSetup3DBase + SLICTRL,
            ((sli_renderMask << SLICTL_3D_RENDERMASK_SHIFT) |
             (sli_compareMask << SLICTL_3D_COMPAREMASK_SHIFT) |
             (sli_scanMask << SLICTL_3D_SCANMASK_SHIFT) |
             ((nChipsLog2-1) << SLICTL_3D_NUMCHIPS_LOG2_SHIFT) |
             SLICTL_3D_EN));
#endif
       }
       // Sanity checking...
       if((sli_renderMask > 255) || (sli_compareMask > 255) ||
          (sli_scanMask > 255)) {
             DEBUG_PRINTF("H3_SETUP_SLI_AA() ERROR: sli render/compare/scan masks greater than 255...\n");
#ifndef H3VDD
             fflush(stdout);
             TESTBENCH_FAIL();
#endif
       }

       // Setup cfgSliAaTiledAperture
       if(pChipInfo->dwsliEn && !pChipInfo->dwaaEn) {
          // SLI only...

          // By default, setup beginning of SLI/AA tiled aperture to be evenly
          // split between linear and tiled (based on memory size...)
          // Make 1/2 memory size be the split...
          PCI_IO_WR(h3ChipSetupIoBase+LFBMEMORYCONFIG,
             ((pMemInfo->dwTileMark >> 12) & 0x1fff) |        // tile aperture base bits(12:0)
            (((pMemInfo->dwTileMark >> 25) & 0x0003) << 23) | // tile aperture base bits(14:13)
            dwStrideAndAperature | // number of sgram tiles in X...
				(0x0<<31)); // write to lfbMemoryTileCtrl
#if 0
          PCI_IO_WR(h3ChipSetupIoBase+LFBMEMORYCONFIG,
            ((pMemInfo->dwTileCmpMark >> 12) & 0x7fff) | // tile aperture base bits(14:0)
				(0x1<<15) |	// Use lfbMemoryTileCompare[14:0] for
								// tiled/linear comparison
				(0x1<<31)); // write to lfbMemoryTileCompare
#endif

#ifdef RD_ABORT_ERROR
         {
         DWORD tileBegin; 
         DWORD tileEnd;
          tileBegin = pMemInfo->dwTileMark;
          tileEnd = pMemInfo->dwTotalMemory;

         PCI_CFG_WR(CFG_AA_LFB_CTRL, CFG_AA_LFB_RD_EN, h3ChipSetupDeviceNum);
          PCI_CFG_WR(CFG_AA_ZBUFF_APERTURE,
			 	(((tileBegin >> 12) << CFG_AA_DEPTH_BUFFER_BEG_SHIFT) |
				 ((tileEnd >> 12) << CFG_AA_DEPTH_BUFFER_END_SHIFT)),
             h3ChipSetupDeviceNum);
         }
#endif

       } else {
          // AA is enabled...
          DWORD tileBegin, tileEnd, aaSecondaryBuffersBegin, aaDepthBufferBegin;
			 DWORD aaDepthBufferEnd;

          // By default for simulation, setup the memory map as follows:
          // (for a 64MB memory example)
          //
          // +==========================+ 64 MB
          // +                          +
          // +  Secondary depth buffer  +
          // +                          +
          // +==========================+ 56 MB
          // +                          +
          // +  Secondary color buffers +
          // +                          +
          // +==========================+ 40 MB
          // +                          +
          // +  Primary depth buffer    +
          // +                          +
          // +==========================+ 32 MB
          // +                          +
          // +  Primary color buffers   +
          // +                          +
          // +==========================+ 16 MB  <-- Tile memory starts here...
          // +                          +
          // +  Linear memory space     +
          // +                          +
          // +==========================+ 0
#ifdef H3VDD
          tileBegin = pMemInfo->dwTileMark;
          tileEnd = pMemInfo->dwTotalMemory;
          if(aaSample8x ||
             // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
             // spread over 4 chips, then with each pair analog SLI'ed...
             (pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog)) {
             //
             // Since the hardware will not properly support averaging
             // the subsamples coming from 4 separate chips, we just punt
             // and only return aliased data (color from the first chip)
             // by making all tiled memory look like it is the Z-Buffer...
             aaDepthBufferBegin = tileBegin;
             aaDepthBufferEnd = tileEnd;
          } else {
             aaDepthBufferBegin = pMemInfo->dwaaSecondaryDepthBufBegin;
             aaDepthBufferEnd = pMemInfo->dwaaSecondaryDepthBufEnd;
          }
          if((pChipInfo->dwChips == 2 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh) ||
             (pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog) ||
             (pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && !aaSample8x &&
              pChipInfo->dwsliAaAnalog))
             // Two chips-- 2-sample AA (1 subsampled stored in each chip)
             // Fool the PCI frontend section...the AA reads/writes will still
             // be duplicated to the secondary AA buffers, but the secondary
             // buffer start will point to the primary buffers.  This will,
             // in effect, make it as though we only have one AA buffer...
             // 
             // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples
             // spread over 2 chips, then with each pair analog SLI'ed...
             //
             // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
             // spread over 4 chips, then with each pair analog SLI'ed...
             aaSecondaryBuffersBegin = tileBegin;
          else
             aaSecondaryBuffersBegin = pMemInfo->dwaaSecondaryColorBufBegin;
#else
          tileBegin = (fbMem >> 2) << 20;
          tileEnd = fbMem<<20;
          if(aaSample8x ||
             // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
             // spread over 4 chips, then with each pair analog SLI'ed...
             (nChips == 4 && !sliEn && aaEn && aaSampleHigh && sliAaAnalog)) {
             //
             // Since the hardware will not properly support averaging
             // the subsamples coming from 4 separate chips, we just punt
             // and only return aliased data (color from the first chip)
             // by making all tiled memory look like it is the Z-Buffer...
             aaDepthBufferBegin = tileBegin;
             aaDepthBufferEnd = tileEnd;
          } else {
             aaDepthBufferBegin = (fbMem >> 1) << 20;
             aaDepthBufferEnd = aaDepthBufferBegin + ((fbMem >> 3) << 20);
          }
          if((nChips == 2 && !sliEn && aaEn && !aaSampleHigh) ||
             (nChips == 4 && sliEn && aaEn && !aaSampleHigh && sliAaAnalog) ||
             (nChips == 4 && !sliEn && aaEn && aaSampleHigh && !aaSample8x &&
              sliAaAnalog))
             // Two chips-- 2-sample AA (1 subsampled stored in each chip)
             // Fool the PCI frontend section...the AA reads/writes will still
             // be duplicated to the secondary AA buffers, but the secondary
             // buffer start will point to the primary buffers.  This will,
             // in effect, make it as though we only have one AA buffer...
             // 
             // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples
             // spread over 2 chips, then with each pair analog SLI'ed...
             //
             // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
             // spread over 4 chips, then with each pair analog SLI'ed...
             aaSecondaryBuffersBegin = tileBegin;
          else
             aaSecondaryBuffersBegin = ((fbMem * 5) >> 3) << 20;
#endif
         
          PCI_IO_WR(h3ChipSetupIoBase+LFBMEMORYCONFIG,
             ((tileBegin >> 12) & 0x1fff) |        // tile aperture base bits(12:0)
            (((tileBegin >> 25) & 0x0003) << 23) | // tile aperture base bits(14:13)
            dwStrideAndAperature |
				(0x0<<31));	// write to lfbMemoryTileCtrl

#if 0
          PCI_IO_WR(h3ChipSetupIoBase+LFBMEMORYCONFIG,
            ((tileBegin >> 12) & 0x7fff) | // tile aperture base bits(14:0)
				(0x1<<15) |	// Use lfbMemoryTileCompare[14:0] for
								// tiled/linear comparison
				(0x1<<31));	// write to lfbMemoryTileCompare
#endif

          if (15 == pMemInfo->dwBpp)
            dwFormat = CFG_AA_LFB_RD_FORMAT_15BPP;
          else if (32 == pMemInfo->dwBpp)
            dwFormat = CFG_AA_LFB_RD_FORMAT_32BPP;
          else
            dwFormat = CFG_AA_LFB_RD_FORMAT_16BPP;

           if ((pChipInfo->dwaaSampleHigh ||
              (pChipInfo->dwChips == 2 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh) ||
              (pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog) ||
              (pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog)))
            dwFormat |= CFG_AA_LFB_RD_DIVIDE_BY_4;

          PCI_CFG_WR(CFG_AA_LFB_CTRL,
            ((aaSecondaryBuffersBegin << CFG_AA_BASEADDR_SHIFT) |
             CFG_AA_LFB_CPU_WR_EN | CFG_AA_LFB_DPTCH_WR_EN | CFG_AA_LFB_RD_EN | dwFormat), 
            h3ChipSetupDeviceNum);

          // By default, setup beginning of depth buffer to be at (3/8) * total
          // memory size...
          PCI_CFG_WR(CFG_AA_ZBUFF_APERTURE,
			 	(((aaDepthBufferBegin >> 12) << CFG_AA_DEPTH_BUFFER_BEG_SHIFT) |
				 ((aaDepthBufferEnd >> 12) << CFG_AA_DEPTH_BUFFER_END_SHIFT)),
             h3ChipSetupDeviceNum);
       }

       // Setup vga_vsync_offset field...
       if(pChipInfo->dwChips > 1 && i > 1 && (pChipInfo->dwaaEn || pChipInfo->dwsliEn)) {
          DWORD vsyncOffsetPixels, vsyncOffsetChars, vsyncOffsetHXtra;

          if((pChipInfo->dwsliAaAnalog &&
             // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples
             // spread over 2 chips, then with each pair analog SLI'ed...
              !(pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog &&
                i != 3) &&
             // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
             // spread over 4 chips, then with each pair analog SLI'ed...
             // This covers the aaSample8x case also...
              !(pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog &&
                i != 3 && 0 == dwClkDbl) ) ||

             // Handle four chips, 2-way analog SLI with digital 4-sample AA...
             (pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && !pChipInfo->dwsliAaAnalog &&
              i == 3) ) {

	    // The desired value for the hardware is actually
	    // vsyncOffsetPixels = 7, vsyncOffsetChars = 3, but the
	    // vga_crtc_fast module has a bug in it which causes us to
	    // have to bump the vsyncOffsetChars field when using
            // vsyncOffsetPixels = 7
             vsyncOffsetPixels = 7;
             vsyncOffsetChars = 4;
             vsyncOffsetHXtra = 0;
          } else {
             // Run slave 8 clocks ahead...

				 // Desired value is vsyncOffsetPixels = 7, vsyncOffsetChars = 4
				 // but workaround bug in vga_crtc_fast as explained above...
             vsyncOffsetPixels = 7;
             vsyncOffsetChars = 5;
             vsyncOffsetHXtra = 0;
          }
          PCI_CFG_WR(CFG_SLI_AA_MISC,
             ((PCI_CFG_RD(CFG_SLI_AA_MISC, h3ChipSetupDeviceNum) &
               ~CFG_VGA_VSYNC_OFFSET) |
              (vsyncOffsetPixels << CFG_VGA_VSYNC_OFFSET_PIXELS_SHIFT) |
              (vsyncOffsetChars << CFG_VGA_VSYNC_OFFSET_CHARS_SHIFT) |
              (vsyncOffsetHXtra << CFG_VGA_VSYNC_OFFSET_HXTRA_SHIFT)),
            h3ChipSetupDeviceNum);
       }

       if(pChipInfo->dwChips == 1 && pChipInfo->dwaaEn) {
          // Single chip-- 2-sample AA...
          PCI_CFG_WR(CFG_VIDEO_CTRL0,
            (CFG_ENHANCED_VIDEO_EN | CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
             (CFG_VIDEO_OTHERMUX_SEL_PIPE<<CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
             CFG_DIVIDE_VIDEO_BY_2),
            h3ChipSetupDeviceNum);
          PCI_CFG_WR(CFG_VIDEO_CTRL1,
            ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
             (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
             (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
             (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
            h3ChipSetupDeviceNum);
          PCI_CFG_WR(CFG_VIDEO_CTRL2,
            ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
             (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
            h3ChipSetupDeviceNum);

       } else if(pChipInfo->dwChips == 2 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh &&
         !pChipInfo->dwsliAaAnalog) {
          // Two chips-- 4-sample digital AA...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
             // Second chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 2 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh &&
         pChipInfo->dwsliAaAnalog) {
          // Two chips-- 4-sample analog AA...
          if(i == 1)
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
          else
             // Tristate HSYNC since both chips share a common hsync signal
             // (since we want to support analog SLI also...)
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_DAC_HSYNC_TRISTATE |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
          PCI_CFG_WR(CFG_VIDEO_CTRL1,
            ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
             (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
             (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
             (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
            h3ChipSetupDeviceNum);
          PCI_CFG_WR(CFG_VIDEO_CTRL2,
            ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
             (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
            h3ChipSetupDeviceNum);

       } else if(pChipInfo->dwChips == 2  && pChipInfo->dwsliEn && !pChipInfo->dwaaEn && !pChipInfo->dwsliAaAnalog) {
          // Two chips-- 2-way digital SLI...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                (CFG_VIDEO_OTHERMUX_SEL_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
             // Second chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }
       } else if((pChipInfo->dwChips == 2 || pChipInfo->dwChips == 4) && pChipInfo->dwsliEn && !pChipInfo->dwaaEn &&
         pChipInfo->dwsliAaAnalog) {
          // Two of four chips-- 2/4-way analog SLI...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
             // Second, third, fourth chips...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 2 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh &&
         !pChipInfo->dwsliAaAnalog) {
          // Two chips-- 2-sample AA with 2-way digital SLI...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
             // Second chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 2 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog) {
          // Two -- 2-sample AA with 2/4-way analog SLI...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
             // Second chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog) {
          // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples
          // spread over 2 chips, then with each pair analog SLI'ed...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else if(i == 2 || i == 4) {
             // Second and fourth chips...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_DAC_HSYNC_TRISTATE |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i>>2)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                ((0xff<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
              // Third chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && !aaSample8x &&
          pChipInfo->dwsliAaAnalog) {
          // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
          // spread over 4 chips, then with each pair analog SLI'ed...

          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             if (dwClkDbl)
               {
                PCI_CFG_WR(CFG_VIDEO_CTRL2,
                  ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                   (0xFF << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                  h3ChipSetupDeviceNum);
               }
             else
               {
                PCI_CFG_WR(CFG_VIDEO_CTRL2,
                  ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                   (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                  h3ChipSetupDeviceNum);
               }
          } else if(i == 2 || i == 4) {
             // Second and fourth chips...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_DAC_HSYNC_TRISTATE |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             if (dwClkDbl)
               {
                PCI_CFG_WR(CFG_VIDEO_CTRL2,
                  ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                   (0xFF << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                  h3ChipSetupDeviceNum);
               }
             else
               {
                PCI_CFG_WR(CFG_VIDEO_CTRL2,
                  ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                   (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                  h3ChipSetupDeviceNum);
               }
          } else {
              // Third chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_DAC_HSYNC_TRISTATE |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && aaSample8x &&
          pChipInfo->dwsliAaAnalog) {
          // 8x AA case...Store 2 subsamples in each of the 4 chips...

          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_8),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             if (dwClkDbl)
               {
                PCI_CFG_WR(CFG_VIDEO_CTRL2,
                  ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                   (0xFF << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                  h3ChipSetupDeviceNum);
               }
             else
               {
                PCI_CFG_WR(CFG_VIDEO_CTRL2,
                  ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                   (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                  h3ChipSetupDeviceNum);
               }
          } else if(i == 2 || i == 4) {
             // Second and fourth chips...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                CFG_DAC_HSYNC_TRISTATE |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             if (dwClkDbl)
               {
                PCI_CFG_WR(CFG_VIDEO_CTRL2,
                  ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                   (0xFF << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                  h3ChipSetupDeviceNum);
               }
             else
               {
                PCI_CFG_WR(CFG_VIDEO_CTRL2,
                  ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                   (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                  h3ChipSetupDeviceNum);
               }
          } else {
              // Third chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                CFG_DAC_HSYNC_TRISTATE |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_8),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

        } else if(pChipInfo->dwChips == 2 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh &&
         !pChipInfo->dwsliAaAnalog) {
          // Two chips-- 2-sample digital AA (1 subsampled stored in each chip)
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);

          } else {
             // Second chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 2 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh &&
          pChipInfo->dwsliAaAnalog) {
          // Two chips-- 2-sample analog AA (1 subsampled stored in each chip)
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);

          } else {
             // Second chip...
             // Tristate HSYNC since both chips share a common hsync signal
             // (since we want to support analog SLI also...)
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_DAC_HSYNC_TRISTATE |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);
          }
          PCI_CFG_WR(CFG_VIDEO_CTRL1,
            ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
             (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
             (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
             (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
            h3ChipSetupDeviceNum);
          PCI_CFG_WR(CFG_VIDEO_CTRL2,
            ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
             (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
            h3ChipSetupDeviceNum);

      } else if(pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && !pChipInfo->dwaaEn && !pChipInfo->dwsliAaAnalog) {
          // Four chips-- 4-way digital SLI...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                (CFG_VIDEO_OTHERMUX_SEL_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_SLI_AAFIFO_COMPARE_INV |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
             // Second, third, fourth chips...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh &&
         !pChipInfo->dwsliAaAnalog) {
          // Four chips-- 2-sample AA with 4-way digital SLI...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_SLI_AAFIFO_COMPARE_INV |
                CFG_DIVIDE_VIDEO_BY_2),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
             // Second, third, fourth chips...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                 CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (((i-1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && !pChipInfo->dwsliAaAnalog) {
          // Four chips-- 2-way analog SLI with digital 4-sample AA...

          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else if(i == 2 || i == 4) {
             // Second and fourth chips...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_DAC_HSYNC_TRISTATE |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_1),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i>>2)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                ((0xff<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
              // Third chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO <<
                 CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else if(pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog) {
          // Four chips-- 2-way analog SLI with analog 4-sample AA...
          if(i == 1) {
             // First chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else if(i == 2 || i == 4) {
             // Second and fourth chips...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_DAC_HSYNC_TRISTATE |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                (((i>>2)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                (((i>>2)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          } else {
              // Third chip...
             PCI_CFG_WR(CFG_VIDEO_CTRL0,
               (CFG_ENHANCED_VIDEO_EN |
                CFG_ENHANCED_VIDEO_SLV |
                CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                (CFG_VIDEO_OTHERMUX_SEL_PIPE <<
                 CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                CFG_DIVIDE_VIDEO_BY_4),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL1,
               (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
               h3ChipSetupDeviceNum);
             PCI_CFG_WR(CFG_VIDEO_CTRL2,
               ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
               h3ChipSetupDeviceNum);
          }

       } else {
          DEBUG_PRINTF("H3_SETUP_SLI_AA() ERROR: Unsupported combination of {nChips, sliEn, aaEn, ...}\n");
#ifndef H3VDD
          fflush(stdout);
          TESTBENCH_FAIL();
#endif
       }

       if((pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && i == 4) ||
          (pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog &&
           i == 4)) {
         // Make sure that last chip properly waits for data to be xfered
         // over the PCI bus before driving...
         PCI_CFG_WR(CFG_SLI_AA_MISC,
           ((PCI_CFG_RD(CFG_SLI_AA_MISC, h3ChipSetupDeviceNum) |
             CFG_AA_LFB_RD_SLV_WAIT)),
           h3ChipSetupDeviceNum);
       }

       // Deal with the problem for LFB reads where the data really needs to
       // come from 4 different chips...Since the hardware does not support
       // this, we figure out a way to only return aliased data back back...
       // This is accomplished by having the Master return its lfb data
       //
       // By turning off AA LFB reads, chips 3/4  no longer snoop lfb reads
       // at all.  Then, there is only handshaking between chips 1 & 2 so
       // the Master stays happy...
       //
       // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
       // spread over 4 chips, then with each pair analog SLI'ed...
       // This covers the aaSample8x case also...
       if(pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog &&
          i > 2) {
          PCI_CFG_WR(CFG_AA_LFB_CTRL,
             ((PCI_CFG_RD(CFG_AA_LFB_CTRL, h3ChipSetupDeviceNum) &
              ~CFG_AA_LFB_RD_EN)), h3ChipSetupDeviceNum);
       }

       if(i > 1) {
           // For the slave chips, make the video PLL lock to the Master's 
           // sync_clk_out clock output...
           PCI_CFG_WR(CFG_VIDEO_CTRL0,
              (PCI_CFG_RD(CFG_VIDEO_CTRL0, h3ChipSetupDeviceNum) |
               CFG_VIDPLL_SEL),
              h3ChipSetupDeviceNum);

           // Power down Slave(s) RAMDAC...
           PCI_IO_WR(h3ChipSetupIoBase+MISCINIT1,
             (PCI_IO_RD(h3ChipSetupIoBase+MISCINIT1) | SST_POWERDOWN_DAC));
       } else if ((1 == i) && (4 == pChipInfo->dwChips))
            {
            // Special Case 4 way where master also needs to sync from slave
            PCI_CFG_WR(CFG_VIDEO_CTRL0,
              (PCI_CFG_RD(CFG_VIDEO_CTRL0, h3ChipSetupDeviceNum) |
               CFG_VIDPLL_SEL),
              h3ChipSetupDeviceNum);
            }

       // TO DO:
       // Adjust pci fifo thresholds?

    } // for(i=1; i<=pChipInfo->dwChips; i++) ...
}

#ifdef H3VDD
/*----------------------------------------------------------------------
Function name:  EnableSLIAA

Description:    Enables SLI and AA on Napalm

Information:

Return:    Nothing     
                
----------------------------------------------------------------------*/
extern DWORD dwWin98;
DWORD GetFBAddr(DWORD);
DWORD GetPitch(DWORD);
DWORD GetBPP(DWORD);
void SetVideoMode(VidProcConfig *pVpc, PDEVTABLE pDev);
void InitFifowoDF(DWORD lpDriverData, FxU32 fifoBase, FxU32 fifoSize);
void InitFifoFixup(DWORD lpDriverData);
void InitRegswoDF(DWORD lpMasterDriverData, DWORD lpSlaveDriverData);
void SaveMemConfig(DWORD lpDriverData, PCONF_SAVE pConf_Save, DWORD * regBase, DWORD lfbBase, DWORD SliaaLfbBase);
void RestoreMemConfig(DWORD lpDriverData, PCONF_SAVE pConf_Save, DWORD * regBase, DWORD lfbBase);
extern DWORD DriverData;
#define MAX_RETRYS (10)
void EnableSLIAA(PDEVTABLE pMaster, PSLI_AA_REQUEST pRequest)
{
   PDEVTABLE pSlave;
   DWORD dwOffset;
   DWORD dwPitch;
   DWORD dwSize;
   DWORD dwY;
   DWORD dwDiff;
   DWORD nFixup;
#ifdef DO_MEMCPY
#ifndef WIN_CSIM
   DWORD compareMask;
   DWORD i;
   DWORD NumLines;
   BYTE *pSlaveFB;
#endif
#endif
   DWORD renderMask;
   DWORD N;
#ifdef USE_FAKE_PHYS
   DWORD FakePhysAddr;
#endif
   BYTE *pMasterFB;
   PDEVTABLE pSaveSlave;
   DWORD dwMapSize;
   DWORD dwOldMapSize;
   CHIPINFO ChipInfo;
   SstIORegs * pSrcIORegs;
   SstIORegs * pDstIORegs;
   DWORD k;
   DWORD j;
   DWORD dwGamma;
   DWORD foo;   

   // The 6000 has 4 chips and an external clock
   if (4 == pRequest->ChipInfo.dwChips)
      clock(pMaster);

#ifdef RD_ABORT_ERROR
   pMaster->SLIAARequest = *pRequest;   
#endif
   dwOffset = GetFBAddr(pMaster->lpDriverData);
   dwPitch = GetPitch(pMaster->lpDriverData);
   dwY = pMaster->Vpc.height;
   dwSize = pMaster->Vpc.width * GetBPP(pMaster->lpDriverData);
   dwDiff = dwPitch - dwSize;

   switch (pRequest->ChipInfo.dwsli_nlines) 
      {
      case 2: 
         N = 1; 
         break;

      case 4: 
         N = 2; 
         break;

      case 8: 
         N = 3; 
         break;

      case 16: 
         N = 4; 
         break;

      case 32: 
         N = 5; 
         break;

      case 64: 
         N = 6; 
         break;

      case 128: 
         N = 7; 
         break;

      default: 
         N = 7; 
         break;
      }

#ifdef FAKE_IT
   pRequest->ChipInfo.dwsliEn = 1;
#endif
   pSaveSlave = NULL;
   renderMask = (pRequest->ChipInfo.dwChips - 1) << N;
   dwMapSize = dwPitch * dwY;
   dwOldMapSize = 0x0;
   for (pSlave = pMaster->pSlave; pSlave != NULL; pSlave = pSlave->pSlave)
      {
      pMasterFB = (BYTE *)(pMaster->LfbBase + dwOffset);
      nFixup = FALSE;
      if (0x0 == pSlave->lpDriverData)
         {
         pSlave->lpDriverData = (DWORD)&DriverData;
         nFixup = TRUE;
         }

#ifdef FAKE_IT
      // Enable our resources
      if (!dwWin98)
         {
         PCI_Write_Config(0x00, 0x68, 0x10, 0x0A000000);
         PCI_Write_Config(0x00, 0x68, 0x14, 0x0E000000);
         PCI_Write_Config(0x00, 0x68, 0x18, 0x2000);
         PCI_Write_Config(0x00, 0x68, 0x4, 0x03);
         }
#endif

      // Save Memory Configuration
#ifdef USE_FAKE_PHYS
      if (0x0 == pSlave->SliaaLfbBase)
         NAPDEV_Get_Mem(&FakePhysAddr, &pSlave->SliaaLfbBase);
#endif
      SaveMemConfig(pSlave->lpDriverData, &pSlave->SaveMem, pSlave->RegBase, pSlave->LfbBase,pSlave->SliaaLfbBase);

#ifndef WIN_CSIM
      // Disable Master
      PCI_Write_Config(pMaster->dwBus, pMaster->dwDevFunc, SST_PCI_COMMAND_ID, 0x02);      

      // Enable Slave
      PCI_Write_Config(pSlave->dwBus, pSlave->dwDevFunc, SST_PCI_COMMAND_ID, 0x03);      

      // Now Master and Slave Have same mode
      SetVideoMode(&pMaster->Vpc, pSlave);
      // Disable Slave
      PCI_Write_Config(pSlave->dwBus, pSlave->dwDevFunc, SST_PCI_COMMAND_ID, 0x02);      

      // Enable Master
      PCI_Write_Config(pMaster->dwBus, pMaster->dwDevFunc, SST_PCI_COMMAND_ID, 0x03);      
#endif

      // Setup the CMDFIFO and 2D Engine <even though it is not used>
      InitRegswoDF(pMaster->lpDriverData, pSlave->lpDriverData);

      // At this point we should have promoted to Overlay and so
      // We Need to copy a few registers from the master to the slave
      // vidProcCfg      
      ((SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX])->vidProcCfg =
         ((SstIORegs *)pMaster->RegBase[HWINFO_SST_IOREGS_INDEX])->vidProcCfg;
      // vidScreenSize
      ((SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX])->vidScreenSize =
         ((SstIORegs *)pMaster->RegBase[HWINFO_SST_IOREGS_INDEX])->vidScreenSize;
      
      // vidOverlayDuDxOffsetSrcWidth
      ((SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX])->vidOverlayDudxOffsetSrcWidth =
         ((SstIORegs *)pMaster->RegBase[HWINFO_SST_IOREGS_INDEX])->vidOverlayDudxOffsetSrcWidth;
 
      // vidDesktopOverlayStride
      ((SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX])->vidDesktopOverlayStride =
         ((SstIORegs *)pMaster->RegBase[HWINFO_SST_IOREGS_INDEX])->vidDesktopOverlayStride;
   
      //vidOverlayDudx
      ((SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX])->vidOverlayDudx =
         ((SstIORegs *)pMaster->RegBase[HWINFO_SST_IOREGS_INDEX])->vidOverlayDudx;

#ifdef DO_MEMCPY 
#ifndef WIN_CSIM
      if (pRequest->ChipInfo.dwsliEn)
         {
         // This should make the switch up seem seamless
         if (1 == pSlave->dwUnitNum)
            pSaveSlave = pSlave;
      
         if (0x0 == dwOldMapSize)
            dwOldMapSize = pSlave->nPages<<12;
         else
            dwOldMapSize = dwMapSize + (dwOffset & 4095);

#ifndef WIN_CSIM
         pSlaveFB = SwapPhysFB(pSlave, dwOffset, dwOldMapSize, dwMapSize);
#else
         pSlaveFB = (BYTE *)(pSlave->LfbBase + dwOffset);
#endif

         compareMask = pSlave->dwUnitNum << N;

         for (i=0; i<dwY; i+= NumLines)
            {
            NumLines = MIN(pRequest->ChipInfo.dwsli_nlines, dwY-i);
            if ((i & renderMask) == compareMask)
               {
               for (j=0; j<NumLines; j++)
                  {
                  our_memcpy(pSlaveFB, pMasterFB, dwSize);
                  pSlaveFB += dwPitch;
                  pMasterFB += dwPitch;
                  }
               }
            else
               pMasterFB = pMasterFB + (dwPitch * NumLines);
            }
         }     
#endif      
#endif


      if (nFixup)
         pSlave->lpDriverData = 0x0;

      // Copy the whole DAC
      pSrcIORegs = (SstIORegs *)pMaster->RegBase[HWINFO_SST_IOREGS_INDEX];
      pDstIORegs = (SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX];
      for (j=0; j<512; j++)
         {
         pSrcIORegs->dacAddr = j;
         pDstIORegs->dacAddr = j;
         dwGamma = pSrcIORegs->dacData;
         for (k=0; k<MAX_RETRYS; k++)
            {
            pDstIORegs->dacData = dwGamma;
            foo = pDstIORegs->dacData;
            if (dwGamma == foo)
	            {
	            k=MAX_RETRYS;
	            }
            }
         }
      } 

#ifdef DO_MEMCPY
#ifndef WIN_CSIM
   if (pSaveSlave)
      SwapPhysFB(pSaveSlave, 0x0, dwMapSize, pSaveSlave->nPages<<12);
#endif
#endif

#ifdef DO_MEMCPY
#ifndef WIN_CSIM
   // Need to compress Master
   if (pRequest->ChipInfo.dwsliEn)
      {
      compareMask = 0x0 << N;
      pMasterFB = (BYTE *)(pMaster->LfbBase + dwOffset);
      pSlaveFB = (BYTE *)(pMaster->LfbBase + dwOffset);
      for (i=0; i<dwY; i+= NumLines)
         {
         NumLines = MIN(pRequest->ChipInfo.dwsli_nlines, dwY-i);
         if ((i & renderMask) == compareMask)
            {
            for (j=0; j<NumLines; j++)
               {
               our_memcpy(pSlaveFB, pMasterFB, dwSize);
               pSlaveFB += dwPitch;
               pMasterFB += dwPitch;
               }
            }
         else
            pMasterFB = pMasterFB + (dwPitch * NumLines);
         }   
      }     
#endif
#endif   

   // Need to Resync Master
   InitFifoFixup(pMaster->lpDriverData);

   ChipInfo.dwChips = pRequest->ChipInfo.dwChips;
   ChipInfo.dwsliEn = pRequest->ChipInfo.dwsliEn;
   ChipInfo.dwaaEn = pRequest->ChipInfo.dwaaEn;
   ChipInfo.dwaaSampleHigh = pRequest->ChipInfo.dwaaSampleHigh;
   ChipInfo.dwsliAaAnalog = pRequest->ChipInfo.dwsliAaAnalog;
   ChipInfo.dwsli_nlines = pRequest->ChipInfo.dwsli_nlines;
   ChipInfo.dwCfgSwapAlgorithm = pRequest->ChipInfo.dwCfgSwapAlgorithm;
   ChipInfo.dwMasterID = 0x0;
   ChipInfo.pDevTable = (DWORD)pMaster;

#ifndef FAKE_IT
   H3_SETUP_SLI_AA(SLI_AA_ENABLE, &ChipInfo, &pRequest->MemInfo);
#endif

   return;
}

/*----------------------------------------------------------------------
Function name:  DisableSLIAA

Description:    Disable SLI and AA on Napalm

Information:

Return:    Nothing     
                
----------------------------------------------------------------------*/
void DisableSLIAA(PDEVTABLE pMaster,  PSLI_AA_REQUEST pRequest)
{
   PDEVTABLE pSlave;
   DWORD dwOffset;
   DWORD dwPitch;
   DWORD dwSize;
   DWORD dwY;
   DWORD dwDiff;
#ifdef DO_MEMCPY
#ifndef WIN_CSIM
   int i;
   DWORD j;
   DWORD NumLines;
   BYTE *pSlaveFB;
#endif
#endif
   DWORD nFixup;
   DWORD compareMask;
   DWORD renderMask;
   DWORD N;
   BYTE *pMasterFB;
   PDEVTABLE pSaveSlave;
   DWORD dwMapSize;
   DWORD dwOldMapSize;
   CHIPINFO ChipInfo;
   DWORD dwReg;
   
#ifdef RD_ABORT_ERROR
   pMaster->SLIAARequest = *pRequest;   
#endif
   ChipInfo.dwChips = pRequest->ChipInfo.dwChips;
   ChipInfo.dwsliEn = pRequest->ChipInfo.dwsliEn;
   ChipInfo.dwaaEn = pRequest->ChipInfo.dwaaEn;
   ChipInfo.dwaaSampleHigh = pRequest->ChipInfo.dwaaSampleHigh;
   ChipInfo.dwsliAaAnalog = pRequest->ChipInfo.dwsliAaAnalog;
   ChipInfo.dwsli_nlines = pRequest->ChipInfo.dwsli_nlines;
   ChipInfo.dwCfgSwapAlgorithm = pRequest->ChipInfo.dwCfgSwapAlgorithm;
   ChipInfo.dwMasterID = 0x0;
   ChipInfo.pDevTable = (DWORD)pMaster;

#ifndef FAKE_IT
   H3_SETUP_SLI_AA(SLI_AA_DISABLE, &ChipInfo, &pRequest->MemInfo);
#else
   pRequest->ChipInfo.dwsliEn = 1;
#endif

   dwOffset = GetFBAddr(pMaster->lpDriverData);
   dwPitch = GetPitch(pMaster->lpDriverData);
   dwY = pMaster->Vpc.height;
   dwSize = pMaster->Vpc.width * GetBPP(pMaster->lpDriverData);
   dwDiff = dwPitch - dwSize;

   switch (pRequest->ChipInfo.dwsli_nlines) 
      {
      case 2: 
         N = 1; 
         break;

      case 4: 
         N = 2; 
         break;

      case 8: 
         N = 3; 
         break;

      case 16: 
         N = 4; 
         break;

      case 32: 
         N = 5; 
         break;

      case 64: 
         N = 6; 
         break;

      case 128: 
         N = 7; 
         break;

      default: 
         N = 7; 
         break;
      }

   renderMask = (pRequest->ChipInfo.dwChips - 1) << N;

#ifdef DO_MEMCPY
#ifndef WIN_CSIM
   if (pRequest->ChipInfo.dwsliEn)
      {
      compareMask = 0x0 << N;
      pMasterFB = (BYTE *)(pMaster->LfbBase + dwOffset + dwPitch * dwY);
      pSlaveFB = (BYTE *)(pMaster->LfbBase + dwOffset);
      for (i=0; i<(int)dwY; i+= NumLines)
         {
         NumLines = MIN(pRequest->ChipInfo.dwsli_nlines, dwY-i);
         if ((i & renderMask) == compareMask)
               pSlaveFB += NumLines*dwPitch;
         }

      pSlaveFB -= dwPitch;
      for (i=dwY-NumLines; i>=0; i-= NumLines)
         {
         NumLines = MIN(pRequest->ChipInfo.dwsli_nlines, dwY-i);
         if ((i & renderMask) == compareMask)
            {
            for (j=0; j<NumLines; j++)
               {
               our_memcpy(pMasterFB, pSlaveFB, dwSize);
               pSlaveFB -= dwPitch;
               pMasterFB -= dwPitch;
               }
            }
         else
            pMasterFB = pMasterFB - (dwPitch * NumLines);
         }   
      }     
#endif
#endif

   pSaveSlave = NULL;
   dwMapSize = dwPitch * dwY;
   dwOldMapSize = 0x0;
   for (pSlave = pMaster->pSlave; pSlave != NULL; pSlave = pSlave->pSlave)
      {
      pMasterFB = (BYTE *)(pMaster->LfbBase + dwOffset);
      compareMask = pSlave->dwUnitNum << N;

#ifdef DO_MEMCPY
#ifndef WIN_CSIM
      if (pRequest->ChipInfo.dwsliEn)
         {
         if (1 == pSlave->dwUnitNum)
            pSaveSlave = pSlave;

         if (0x0 == dwOldMapSize)
            dwOldMapSize = pSlave->nPages<<12;
         else
            dwOldMapSize = dwMapSize + (dwOffset & 4095);

#ifndef WIN_CSIM
         pSlaveFB = SwapPhysFB(pSlave, dwOffset, dwOldMapSize, dwMapSize);
#else
         pSlaveFB = (BYTE *)(pSlave->LfbBase + dwOffset);
#endif

         for (i=0; i<(int)dwY; i+= NumLines)
            {
            NumLines = MIN(pRequest->ChipInfo.dwsli_nlines, dwY-i);
            if ((i & renderMask) == compareMask)
               {
               for (j=0; j<NumLines; j++)
                  {
                  our_memcpy(pMasterFB, pSlaveFB, dwSize);
                  pSlaveFB += dwPitch;
                  pMasterFB += dwPitch;
                  }
               }
            else
               pMasterFB = pMasterFB + (dwPitch * NumLines);
            }
         }
#endif
#endif

      nFixup = FALSE;
      if (0x0 == pSlave->lpDriverData)
         {
         pSlave->lpDriverData = (DWORD)&DriverData;
         nFixup = TRUE;
         }

      // Restore Memory Configuration
      RestoreMemConfig(pSlave->lpDriverData, &pSlave->SaveMem, pSlave->RegBase, pSlave->LfbBase);

      // if Slave had a mode then reset it
      if (0x0 != pSlave->Vpc.height)
         {
         // Disable Master
         PCI_Write_Config(pMaster->dwBus, pMaster->dwDevFunc, SST_PCI_COMMAND_ID, 0x02);      

         // Enable Slave
         PCI_Write_Config(pSlave->dwBus, pSlave->dwDevFunc, SST_PCI_COMMAND_ID, 0x03);      

         SetVideoMode(&pSlave->Vpc, pSlave);

         // Disable Slave
         PCI_Write_Config(pSlave->dwBus, pSlave->dwDevFunc, SST_PCI_COMMAND_ID, 0x02);      
         
         // Enable Master
         PCI_Write_Config(pMaster->dwBus, pMaster->dwDevFunc, SST_PCI_COMMAND_ID, 0x03);      
         }

      // Setup the CMDFIFO and 2D Engine <when though it is not used>
      InitRegswoDF(pSlave->lpDriverData, pSlave->lpDriverData);

      // Kill the slave since the DAC are tied together
      dwReg = ((SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX])->vidProcCfg;
      dwReg &= ~SST_VIDEO_PROCESSOR_EN;
      ((SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX])->vidProcCfg = dwReg;
      dwReg = SST_DAC_DPMS_ON_VSYNC | SST_DAC_DPMS_ON_HSYNC;
      ((SstIORegs *)pSlave->RegBase[HWINFO_SST_IOREGS_INDEX])->dacMode = dwReg;            

      if (nFixup)
         pSlave->lpDriverData = 0x0;
      } 

#ifndef WIN_CSIM
   if (pSaveSlave)
      SwapPhysFB(pSaveSlave, 0x0, dwMapSize, pSaveSlave->nPages<<12);
#endif

   return;
}

#ifdef RD_ABORT_ERROR
/*----------------------------------------------------------------------
Function name:  SLI_Read_Enable

Description:    Enable SLI Reads

Information:

Return:    Nothing     
                
----------------------------------------------------------------------*/
void SLI_Read_Enable(PDEVTABLE pMaster)
{
   PDEVTABLE pDev;
   DWORD cfgSliLfbCtrl;
   DWORD cfgAALfbCtrl;

   if (pMaster->SLIAARequest.ChipInfo.dwsliEn)
      {
      for (pDev=pMaster; NULL != pDev; pDev = pDev->pSlave)
         {
         // Turn on SLI Read
         cfgSliLfbCtrl = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, CFG_SLI_LFB_CTRL);
         cfgSliLfbCtrl |= (CFG_SLI_RD_EN);  
         PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_SLI_LFB_CTRL, cfgSliLfbCtrl);

         // Turn off AA if it is not enabled
         if (!pMaster->SLIAARequest.ChipInfo.dwaaEn)
            {
            cfgAALfbCtrl = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, CFG_AA_LFB_CTRL);
            cfgAALfbCtrl &= ~(CFG_AA_LFB_RD_EN);  
            PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_AA_LFB_CTRL, cfgAALfbCtrl);
            }
         }
      }
}

/*----------------------------------------------------------------------
Function name:  SLI_Read_Disable

Description:    Disable SLI Reads

Information:

Return:    Nothing     
                
----------------------------------------------------------------------*/
void SLI_Read_Disable(PDEVTABLE pMaster)
{
   PDEVTABLE pDev;
   DWORD cfgSliLfbCtrl;
   DWORD cfgAALfbCtrl;

   if (pMaster->SLIAARequest.ChipInfo.dwsliEn)
      {
      for (pDev=pMaster; NULL != pDev; pDev = pDev->pSlave)
         {
         // Turn off SLI Read
         cfgSliLfbCtrl = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, CFG_SLI_LFB_CTRL);
         cfgSliLfbCtrl &= ~(CFG_SLI_RD_EN);  
         PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_SLI_LFB_CTRL, cfgSliLfbCtrl);

         // Turn on AA Read
         cfgAALfbCtrl = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, CFG_AA_LFB_CTRL);
         cfgAALfbCtrl |= (CFG_AA_LFB_RD_EN);  
         PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_AA_LFB_CTRL, cfgAALfbCtrl);
         }
      }
}
#endif

/*----------------------------------------------------------------------
Function name:  our_mem_rdd

Description:    Read a DWORD from Memory Mapped Space

Information:

Return:    DWORD
                
----------------------------------------------------------------------*/
DWORD our_mem_rdd(DWORD Addr)
{
   DWORD dwReturn;

   _asm  mov   ebx, Addr
   _asm  mov   eax, [ebx]
   _asm  mov   dwReturn, eax

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  our_mem_wrd

Description:    Write a DWORD from Memory Mapped Space

Information:

Return:    DWORD
                
----------------------------------------------------------------------*/
void our_mem_wrd(DWORD Addr, DWORD Value)
{
   _asm  mov   ebx, Addr
   _asm  mov   eax, Value
   _asm  mov   [ebx], eax
}

/*----------------------------------------------------------------------
Function name:  our_mem_wrd

Description:    Fast Memcpy

Information:

Return:    DWORD
                
----------------------------------------------------------------------*/
void our_memcpy(BYTE * pDest, BYTE * pSrc, DWORD dwSize)
{
   __asm pushad               // Save Registers
   __asm cld                  // Go Forward
   __asm mov edi, pDest       // Get Destination
   __asm mov esi, pSrc        // Get Source
   __asm xor ecx, ecx         // count=0
   __asm mov ebx, dwSize      // Get Size
   __asm shr ebx, 1           // Convert to words
   __asm adc cx, 0            // Remainder?
   __asm rep movsb            // Move a byte if needed
   __asm shr ebx, 1           // Convert to dwords
   __asm mov ecx, ebx         // Count
   __asm rep movsd            // doit
   __asm adc ecx, 0           // any words?
   __asm rep movsw            // move it
   __asm popad                // all done
} 

#endif


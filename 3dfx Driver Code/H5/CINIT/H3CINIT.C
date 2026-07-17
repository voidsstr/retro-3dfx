/* -*-c++-*- */
/* $Header: h3cinit.c, 6, 10/11/00 8:08:11 PM, Brent$ */
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
**
** $Revision: 6$
** $Date: 10/11/00 8:08:11 PM$
**
** $History: h3cinit.c $
** 
** *****************  Version 53  *****************
** User: Atai         Date: 9/01/99    Time: 2:45p
** Updated in $/devel/h5/cinit
** added for dos glide
** 
** *****************  Version 52  *****************
** User: Zelsnack     Date: 11/22/98   Time: 3:43p
** Updated in $/devel/ape/cinit
** Checking in for fun
** 
** *****************  Version 50  *****************
** User: Pault        Date: 11/05/98   Time: 2:04p
** Updated in $/devel/h4/cinit
** Added h4InitPlls function.
** 
** *****************  Version 49  *****************
** User: Jeske        Date: 8/18/98    Time: 5:33p
** Updated in $/devel/h3/cinit
** don't change dramInit1 for diags either... this probably should be the
** case for everyone
** 
** *****************  Version 48  *****************
** User: Psmith       Date: 7/29/98    Time: 8:22a
** Updated in $/devel/h3/cinit
** prevent memory type from being undefined, default to SGRAM
** 
** *****************  Version 47  *****************
** User: Psmith       Date: 7/29/98    Time: 7:42a
** Updated in $/devel/h3/cinit
** added h3InitResetAll() routine to perform a full reset of the chip
** 
** *****************  Version 46  *****************
** User: Andrew       Date: 7/27/98    Time: 1:29p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Scan Line Double Modes
** 
** *****************  Version 45  *****************
** User: Psmith       Date: 7/23/98    Time: 5:14p
** Updated in $/devel/h3/cinit
** added initialization of dramInit1_strap for A-series silicon
** 
** *****************  Version 44  *****************
** User: Psmith       Date: 7/23/98    Time: 6:14a
** Updated in $/devel/h3/cinit
** updated SDRAM init code to only disable 2d block writes
** 
** *****************  Version 43  *****************
** User: Psmith       Date: 7/22/98    Time: 9:11a
** Updated in $/devel/h3/cinit
** added sdram support
** 
** *****************  Version 42  *****************
** User: Mikec        Date: 7/20/98    Time: 11:14a
** Updated in $/devel/h3/cinit
** Made H3_A2 default instead of H3_B0. Also added #if H3VDD around
** vidOverlayDudxOffsetSrcWidth.
** 
** *****************  Version 41  *****************
** User: Psmith       Date: 7/17/98    Time: 12:32p
** Updated in $/devel/h3/cinit
** updated dramInit1 timing parameters for Gateway
** made sgram vendor lookup case insensitive
** 
** *****************  Version 40  *****************
** User: Ken          Date: 7/11/98    Time: 5:51p
** Updated in $/devel/h3/win95/dx/minivdd
** more b0 fixups for gateway
** 
** *****************  Version 39  *****************
** User: Ken          Date: 7/10/98    Time: 9:00p
** Updated in $/devel/h3/win95/dx/minivdd
** memory / clock timing default changes.  default grxclock and memclock
** timings are now settable at compile time.   temporarily, set HR=B0 to
** compile for A2/A3 for minivdd only.  dramInit0 is now not touched in
** the win95 init sequence, unless there's an override in the registry
** 
** *****************  Version 38  *****************
** User: Psmith       Date: 7/10/98    Time: 5:18p
** Updated in $/devel/h3/cinit
** *** changed default compilation mode to H3_B0 ***
** new sgram config for b0
** allow env variables to override sgram config values
** 
** *****************  Version 37  *****************
** User: Ken          Date: 7/02/98    Time: 6:23p
** Updated in $/devel/h3/win95/dx/minivdd
** added magic parameter values to h3InitSGRAM so that windows boot can
** skip the writing of the SGRAM registers
** 
** *****************  Version 36  *****************
** User: Ken          Date: 7/01/98    Time: 6:36p
** Updated in $/devel/h3/win95/dx/minivdd
** added 16bpp low res modes using video overlay stretching
** 
** *****************  Version 35  *****************
** User: Jeske        Date: 6/03/98    Time: 1:51p
** Updated in $/devel/h3/cinit
** put h3InitSetVideoMode() back to taking refresh in Hz, documented the
** argument requirements. 
** 
** *****************  Version 34  *****************
** User: Ken          Date: 6/02/98    Time: 12:27p
** Updated in $/devel/h3/win95/dx/minivdd
** fixed off by one error in LUT initialization
** 
** *****************  Version 33  *****************
** User: Jeske        Date: 6/01/98    Time: 5:48p
** Updated in $/devel/h3/cinit
** 1) convert the ordinal refresh mode to a numerical refresh in HZ, 2)
** support failing the video mode set so we can shutdown gracefully..
** 
** *****************  Version 32  *****************
** User: Psmith       Date: 5/09/98    Time: 11:20a
** Updated in $/devel/h3/cinit
** fixed #if's to compile correctly for H3_A2 
** 
** *****************  Version 31  *****************
** User: Psmith       Date: 5/08/98    Time: 2:50p
** Updated in $/devel/h3/cinit
** added routine to set BlockWrite threshhold
** 
** *****************  Version 30  *****************
** User: Andrew       Date: 5/07/98    Time: 2:42p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a hack to get 1800x1440 to work in Win '9X.  Unknown why display
** pitch needs to be crunched.
** 
** *****************  Version 29  *****************
** User: Peter        Date: 4/17/98    Time: 1:24p
** Updated in $/devel/h3/cinit
** 8.3 fun
** 
** *****************  Version 28  *****************
** User: Ken          Date: 4/16/98    Time: 5:32p
** Updated in $/devel/h3/win95/dx/minivdd
** properly set the vidProcCfg 1x/2x bit
** 
** *****************  Version 27  *****************
** User: Dow          Date: 4/16/98    Time: 12:00p
** Updated in $/devel/h3/cinit
** fixed warnings in h3cinit made warnings=errors for Glide
** 
** *****************  Version 26  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
** 
** *****************  Version 25  *****************
** User: Psmith       Date: 4/09/98    Time: 8:00p
** Updated in $/devel/h3/cinit
** added support for multiple SGRAM vendors
** added support for programming PLLs a wider range of clock frequencies
** 
** *****************  Version 24  *****************
** User: Psmith       Date: 3/30/98    Time: 5:37p
** Updated in $/devel/h3/cinit
** fix for VGA blank with screen off
** 
** *****************  Version 23  *****************
** User: Ken          Date: 3/20/98    Time: 11:30a
** Updated in $/devel/h3/win95/dx/minivdd
** removed non-vdd friendly stuff (ifdefed out proces monitor function,
** until its fixed up)
** 
** *****************  Version 22  *****************
** User: Psmith       Date: 3/19/98    Time: 8:47p
** Updated in $/devel/h3/cinit
** added code to read/display silicon performance values
** 
** *****************  Version 21  *****************
** User: Ken          Date: 3/17/98    Time: 3:55p
** Updated in $/devel/h3/win95/dx/minivdd
** added -DH3_A1  when compiling on A1.    changed h3cinit.c so that until
** the non-MS folks add an H3_A0 flag, when not compiling the H3 VDD, the
** absence of H3_A1 automatically defines H3_A0
** 
** *****************  Version 20  *****************
** User: Ken          Date: 3/17/98    Time: 12:07p
** Updated in $/devel/h3/win95/dx/minivdd
** fixed incorrect #ifdef'ing for H3_A0
** 
** *****************  Version 19  *****************
** User: Ken          Date: 3/17/98    Time: 11:21a
** Updated in $/devel/h3/win95/dx/minivdd
** placed A0 workarounds in #ifdef H3_A0.  updated makefiles to use
** environment variable HR for hardware rev.   if hardware rev is not
** HR=A1, then we build an A0 revision.   use #ifdef H3_A0 for A0
** workarounds
** 
** *****************  Version 18  *****************
** User: Ken          Date: 2/15/98    Time: 11:47a
** Updated in $/devel/h3/win95/dx/minivdd
** added parameter to setvideomode to conditionally initialize clut
** 
** *****************  Version 17  *****************
** User: Psmith       Date: 2/13/98    Time: 7:38p
** Updated in $/devel/h3/cinit
** added 16Mbit SGRAM support
** 
** *****************  Version 16  *****************
** User: Ken          Date: 2/13/98    Time: 4:55p
** Updated in $/devel/h3/win95/dx/minivdd
** added init of tmuGbeInit to turn off  texture caching 
** 
** *****************  Version 15  *****************
** User: Miriam       Date: 2/13/98    Time: 1:17p
** Updated in $/devel/h3/Win95/dx/minivdd
** Can't use texture cache yet.
** 
** *****************  Version 14  *****************
** User: Ken          Date: 2/11/98    Time: 8:13p
** Updated in $/devel/h3/win95/dx/minivdd
** updated to latest h3init.pm, fixes 1280 modes
** 
** *****************  Version 13  *****************
** User: Psmith       Date: 2/10/98    Time: 7:34p
** Updated in $/devel/h3/cinit
** got rid of monitor click !!!
** 
** *****************  Version 12  *****************
** User: Bangell      Date: 2/10/98    Time: 3:50p
** Updated in $/devel/h3/cinit
** allow MEM and GRX clocks to be set to different frequencies
** 
** *****************  Version 11  *****************
** User: Ken          Date: 2/09/98    Time: 8:31a
** Updated in $/devel/h3/win95/dx/minivdd
** updated to andy's latest perl code, now has full mode table
** 
** *****************  Version 10  *****************
** User: Ken          Date: 2/04/98    Time: 1:29p
** Updated in $/devel/h3/win95/dx/minivdd
** turn off VGA screen refresh in h3initSetVideoMode
** 
** *****************  Version 9  *****************
** User: Ken          Date: 1/25/98    Time: 3:08p
** Updated in $/Users/ken/banshee-bringup/directdiags/xor
** added desktop/overlay surface functions
** 
** *****************  Version 8  *****************
** User: Ken          Date: 1/25/98    Time: 1:13p
** Updated in $/Users/ken/banshee-bringup/directdiags/xor
** changed h3InitVideo to h3InitVideoProc, now takes a single 
** parameter which is simply passed into the vidProcCfg register
** 
** *****************  Version 7  *****************
** User: Ken          Date: 1/25/98    Time: 12:27p
** Updated in $/users/ken/banshee-bringup/directdiags/xor
** rollback, new version
** 
** *****************  Version 5  *****************
** User: Ken          Date: 1/24/98    Time: 4:41p
** Updated in $/devel/h3/cinit
** works now
** 
** *****************  Version 4  *****************
** User: Dow          Date: 1/24/98    Time: 9:11a
** Updated in $/devel/h3/cinit
** WIP
** 
** *****************  Version 3  *****************
** User: Dow          Date: 1/23/98    Time: 3:28p
** Updated in $/devel/h3/cinit
** Keywords
**
*/

// Assume H3_A2 board by default (rather than B0).
// this works around the non-MS makefiles which don't define
// the h/w rev on the compile line.  Currently, this means that
// if H3_A1 is not defined, we assume that we're building A0
//
#ifndef H3VDD
# if !defined H3_A1 && !defined H3_B0
#  define H3_A2
# endif // # if !defined H3_A1 && !defined H3_A2
#endif // #ifndef H3VDD

#include <h3cinit.h>
#include <h3.h>

/* 8.3 fun */
#ifdef __DOS32__
#include "h3cini~1.h"
#elif macintosh
#include "h3cinitdd_mac.h"
#else
#include "h3cinitdd.h"
#endif

#include "plltable.h"
#include "memtable.h"

#ifndef H3VDD
#include "stdlib.h"
#endif // #ifndef H3VDD


// LISTEN UP
//
// The macros used in this file need a varible called regBase.
// Because the init registers are aliased both in the io and memory
// map, it's up to the caller and the implementor of h3cinitdd.h to 
// agree on whether you're using the i/o or the memory mapping
//

/*----------------------------------------------------------------------
Function name:  h3InitGetMemSize

Description:    Return the size of memory in MBs.
                
Information:

Return:         FxU32   The size of memory in MBs.
----------------------------------------------------------------------*/
FxU32
h3InitGetMemSize(FxU32 regBase) // init io-register base
{
  FxU32
    partSize,               // size of SGRAM chips in Mbits
    memSize,                // total size of memory in MBytes
    nChips,                 // # of chips of SDRAM/SGRAM
    dramInit0_strap;
  
  // determine memory size from strapping pins (dramInit0 and dramInit1)
  dramInit0_strap = IGET32(dramInit0);
  dramInit0_strap &= SST_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS | SST_MCTL_DRAM_NUMBANKS;

  nChips = ((dramInit0_strap & SST_SGRAM_NUM_CHIPSETS) == 0) ? 4 : 8;
  
  if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_8MBIT )  {
    partSize = 8;
  } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_16MBIT) {
    partSize = 16;
  } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_32MBIT) {
    partSize = 32;
  } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_64MBIT) {
    partSize = 64;
  } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_128MBIT) {
    partSize = 128;
  } else {
    MESSAGE("h3InitGetMemSize: invalid sgram type = 0x%x",
	    (dramInit0_strap & SST_SGRAM_TYPE) << SST_SGRAM_TYPE_SHIFT );
    return 0;
  }

  memSize = (nChips * partSize) / 8;      // in MBytes

  GDBG_INFO(0, "h3InitGetMemSize:  partSize = %dMb  nChips = %d  memSize = %dMB\n",
	    partSize, nChips, memSize);
    
  return (memSize);
}

FxU32                           // return # of MB of memory
h3InitSgram(FxU32 regBase,       // init iegister base
            FxU32 sgramMode,
            FxU32 sgramMask,
            FxU32 sgramColor,
            char *vendorName     // NULL or name of sgram vendor
            )
{
  FxU32
    memType,                  // SGRAM or SDRAM
    partSize,                 // size of SGRAM chips in Mbits
    memSize,                  // total size of memory in MBytes
    nChips,                   // # chips of SDRAM/SGRAM
    dramInit0_strap,    
    dramInit1_strap,    
    dramInit0,
    dramInit1,
    miscInit1;
  FxU32 vendorID;

  // determine memory type: SDRAM or SGRAM
  memType = MEM_TYPE_SGRAM;
  dramInit1_strap = IGET32(dramInit1);
  dramInit1_strap &= SST_MCTL_TYPE_SDRAM;
  if ( dramInit1_strap & SST_MCTL_TYPE_SDRAM )
    memType = MEM_TYPE_SDRAM;

  if(memType == MEM_TYPE_SDRAM)
    GDBG_INFO(0, "h3InitSgram: Using sdram\n");
  else
    GDBG_INFO(0, "h3InitSgram: Using sgram\n");

  // set memory interface delay values and enable refresh
  // these apply to all RAM vendors
  dramInit1 = 0x56c031;

  if ( GETENV("DRAMINIT1") )
    dramInit1 = strtoul(GETENV("DRAMINIT1"),NULL,0);

  dramInit1 &= ~SST_MCTL_TYPE_SDRAM;
  dramInit1 |= dramInit1_strap;

  GDBG_INFO(80, "h3InitSgram: dramInit1 = 0x%x\n",dramInit1);
  ISET32(dramInit1, dramInit1);

  // determine memory size from strapping pins (dramInit0 and dramInit1)
  dramInit0_strap = IGET32(dramInit0);
  dramInit0_strap &= SST_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS | SST_MCTL_DRAM_NUMBANKS;

  nChips = ((dramInit0_strap & SST_SGRAM_NUM_CHIPSETS) == 0) ? 4 : 8;
    
  if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_8MBIT )  {
    partSize = 8;
  } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_16MBIT) {
    partSize = 16;
  } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_32MBIT) {
    partSize = 32;
  } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_64MBIT) {
    partSize = 64;
  } else if ( (dramInit0_strap & SST_SGRAM_TYPE) == SST_SGRAM_TYPE_128MBIT) {
    partSize = 128;
  } else {
    MESSAGE("h3initSgram: invalid sgram type = 0x%x",
	    (dramInit0_strap & SST_SGRAM_TYPE) << SST_SGRAM_TYPE_SHIFT );
    return 0;
  }
    
  memSize = (nChips * partSize) / 8;      // in MBytes

  GDBG_INFO(0, "h3InitSgram:  partSize = %dMb  nChips = %d  memSize = %dMB\n",
	    partSize, nChips, memSize);

  GDBG_INFO(0, "%d MB of total memory\n", memSize);

  // determine memory vendor

  GDBG_INFO(0, "Might want to pick better default dramInit0 value\n");
  if ( memType == MEM_TYPE_SDRAM )        // set defaults
    vendorID = MEM_DEFAULT_SDRAM_16;
  else
    vendorID = partSize == 8 ? MEM_DEFAULT_SGRAM_8 : MEM_DEFAULT_SGRAM_16;

  if ( vendorName ) {
#define TO_UPPER(c)  ( ((c)>='a'&&(c)<='z') ? ((c)-'a'+'A') : (c) )   /* only works for alphabetic characters */
    int i;
    char *v, *t;
    int found;
    // search table for a vendor/size match
    for (i=0; i<MEM_TABLE_SIZE; i++) { 
      for (v=vendorName,t=memTable[i].vendor; *v && *t && TO_UPPER(*v)==*t; v++,t++) ;
      found = (!*v && !*t);
      if ( found && memTable[i].type == memType && memTable[i].size == partSize ) {
        vendorID = i;
        break;

      }
    }
    
    // vendor name not found in table
    if ( i >= MEM_TABLE_SIZE )
      GDBG_ERROR("h3InitSgram","vendor %s not found, defaulting to %s\n",
                 vendorName, memTable[vendorID].vendor);
  }

  GDBG_INFO(80, "h3InitSgram: %d x %d Mbit %s %s parts = %d MBytes\n", 
            nChips, partSize, memTable[vendorID].vendor, 
            memType == MEM_TYPE_SDRAM ? "SDRAM" : "SGRAM", memSize);
  
  dramInit0 = memTable[vendorID].dramInit0;
  
  // write vendor specific SGRAM timings, preserving the SGRAM chipset/size
  // information
  
#if !defined(H3VDD) && !defined(DIAG_BUILD)
  if ( GETENV("SSTH3_DRAMINIT0") )
    dramInit0 = strtoul(GETENV("SSTH3_DRAMINIT0"),NULL,0);
#endif // #ifndef H3VDD

  dramInit0 &= ~(SST_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS | SST_MCTL_DRAM_NUMBANKS);
  dramInit0 |= dramInit0_strap;

#if !defined(H3VDD) && !defined(DIAG_BUILD)
  // don't write dramInit0 in the windows driver by default -- we're using
  // the BIOS's value instead.   It can be overridden in the registry
  // if the user wants to change it or experiment
  //
  GDBG_INFO(80, "h3InitSgram: dramInit0 = 0x%x\n",dramInit0);
  ISET32(dramInit0, dramInit0);
#endif // #ifndef H3VDD

  // Only set the SGRAM registers if the "magic code" isn't sent in.
  // The windows driver will send in the magic code for the main device
  // initialization on pre-B0 silicon, because writing to the SGRAM
  // registers on a soft reboot in certain conditions in pre-B silicon
  // causes the chip to go nuts, so in those cases we just have to live
  // with the BIOS defaults for the SGRAM registers.
  //
  if (!((sgramMode == 0xDEADBEEF) && (sgramMask == 0xF00DCAFE)))
  {
#ifndef H3VDD
      sgramMode = memTable[vendorID].sgramMode;
      if ( GETENV("SSTH3_SGRAMMODE") )
          sgramMode = strtoul(GETENV("SSTH3_SGRAMMODE"),NULL,0);
#else
      sgramMode = memTable[vendorID].sgramMode;
#endif // #ifndef H3VDD

      GDBG_INFO(80, "h3InitSgram: sgramMode = 0x%x\n", sgramMode);

      ISET32(dramData, sgramMode);
      ISET32(dramCommand, 0x10d);

      if ( memType == MEM_TYPE_SGRAM ) {  // only for SGRAM
        ISET32(dramData, sgramMask);
        ISET32(dramCommand, 0x10e);

        ISET32(dramData, sgramColor);
        ISET32(dramCommand, 0x10f);
      }
  }
  
  //
  // disable block writes for SDRAM
  //
  if ( memType == MEM_TYPE_SDRAM ) {
    miscInit1 = IGET32(miscInit1);
    miscInit1 |= SST_DISABLE_2D_BLOCK_WRITE;
    ISET32(miscInit1, miscInit1);
  }
  
  // return # of MBytes of board memory
  return memSize;
  
} // h3initSgram

#ifdef H4
void
h4InitPlls(FxU32 regBase,               // init register base
           FxU32 deviceID,              // H4 or H4_OEM
           FxU32 grxSpeedInMHz)         // desired clock frequency (MHz)
{
  FxU32 maxGrxSpeedInMHz;

  maxGrxSpeedInMHz = MAX_H4_PLL_FREQ;

  // set the graphics clock
  //
  if ( grxSpeedInMHz < MIN_PLL_FREQ || grxSpeedInMHz > maxGrxSpeedInMHz ) {
    MESSAGE("YO!  GRX clock speed of %d MHz is out of range MIN_PLL_FREQ to maxGrxSpeedInMHz\n", grxSpeedInMHz);
  } else {
    GDBG_INFO(80, "Setting Graphics Clock (%d MHz)\n",grxSpeedInMHz);
    ISET32(pllCtrl1, pllTable[grxSpeedInMHz]);
  }

} // h4InitPlls
#else
void
h3InitPlls(FxU32 regBase,               // init iegister base
           FxU32 grxSpeedInMHz,         // desired clock frequency (MHz)
           FxU32 memSpeedInMHz)         // desired clock frequency (MHz)
{

    // set the graphics clock
    //
    if ( grxSpeedInMHz < MIN_PLL_FREQ || grxSpeedInMHz > MAX_PLL_FREQ ) {
      MESSAGE("YO!  Figure out how to set GRX clock to %d MHz\n", grxSpeedInMHz);
    } else {
      GDBG_INFO(80, "Setting Graphics Clock (%d MHz)\n",grxSpeedInMHz);
      ISET32(pllCtrl1, pllTable[grxSpeedInMHz]);
    }

    // set the memory clock
    //
    if ( memSpeedInMHz < MIN_PLL_FREQ || memSpeedInMHz > MAX_PLL_FREQ ) {
      MESSAGE("YO!  Figure out how to set MEM clock to %d MHz\n", memSpeedInMHz);
    } else {
      GDBG_INFO(80, "Setting Memory Clock (%d MHz)\n",memSpeedInMHz);
      ISET32(pllCtrl2, pllTable[memSpeedInMHz]);
    }

} // h3InitPlls
#endif

void 
h3InitVga(
    FxU32 regBase,              // init iegister base
    FxU32 legacyDecode)         // 1=enable VGA decode, 0=disable
{
    FxU32
        vgaInit0 = 0,
        miscInit1;
    
    vgaInit0 |= SST_VGA0_EXTENSIONS;
    vgaInit0 |= SST_WAKEUP_3C3 << SST_VGA0_WAKEUP_SELECT_SHIFT;

    if (legacyDecode)
        vgaInit0 |= SST_VGA0_ENABLE_DECODE << SST_VGA0_LEGACY_DECODE_SHIFT;
    else
        vgaInit0 |= SST_VGA0_DISABLE_DECODE << SST_VGA0_LEGACY_DECODE_SHIFT;

    vgaInit0 |= SST_ENABLE_ALT_READBACK << SST_VGA0_CONFIG_READBACK_SHIFT;

    CHECKFORROOM;
    ISET32(vgaInit0, vgaInit0);
    
    GDBG_INFO(80, "disable VESA mode and VGA VIDEO lock bits\n");
    CHECKFORROOM;
    ISET32(vgaInit1, 0);

    CHECKFORROOM;
    ISET8PHYS(0xc3, 0x1);

    GDBG_INFO(80, "Cleared CLUT Invert Address\n");
    miscInit1 = IGET32(miscInit1);
    miscInit1 |= BIT(0);
    CHECKFORROOM;
    ISET32(miscInit1, miscInit1);
        
} // h3InitVga

void
h3InitVideoProc(FxU32 regBase,
                FxU32 vidProcCfg)       // control bits for video processor
{
    ISET32(vidProcCfg, vidProcCfg);
    
} // h3InitVideo


FxU16 mode_table[][24] =
{ 
#include "modetabl.h"
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        
};

FxU8 vgaattr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 
                  0x00, 0x00, 0x00, 0x00, 0x00, 
                  0x00, 0x00, 0x00, 0x00, 0x00, 
                  0x00, 0x01, 0x00, 0x0F, 0x00};

FxU16 *
h3InitFindVideoMode(FxU32 xRes,
                    FxU32 yRes,
                    FxU32 refresh)
{
    int i = 0;

    while (mode_table[i][0] != 0)
    {
        if ((mode_table[i][0] == xRes) &&
            (mode_table[i][1] == yRes) &&
            (mode_table[i][2] == refresh))
        {
            return &mode_table[i][3];
        }

        i += 1;
    }

    return NULL;
}



FxBool
h3InitSetVideoMode(
  FxU32 regBase,   // regBase of this card
    FxU32 xRes,    // xResolution in pixels
    FxU32 yRes,    // yResolution in pixels
    FxU32 refresh, // refresh rate in Hz
#if defined(H3VDD) && defined(H3_B0)
  FxU32 loadClut, // really a bool, should we load the lookup table
  FxU32 scanlinedouble) // set scanline double bit and double y?
#else
  FxU32 loadClut) // really a bool, should we load the lookup table
#endif
{
    FxU16 i, j;
    FxU8 garbage;
    FxU16 *rs = h3InitFindVideoMode(xRes, yRes, refresh);
    FxU32 vidProcCfg;

    if (rs == NULL)
    {
        GDBG_ERROR("h3InitSetVideoMode", 
                   "mode %d x %d @ %dHz is not supported\n", xRes, yRes, refresh);
        return FXFALSE;
    }
        
    GDBG_INFO(1, "h3InitSetVideoMode(%d, %d, %d)\n", xRes, yRes, refresh);

    //
    // MISC REGISTERS
    // This register gets programmed first since the Mono/ Color
    // selection needs to be made.
    //
    // Sync polarities
    // Also force the programmable clock to be used with bits 3&2
    //

    ISET8PHYS(0xc2, rs[16] | BIT(0));

    //
    // CRTC REGISTERS
    // First Unlock CRTC, then program them 
    //
    
    // Mystical VGA Magic
    ISET16PHYS(0x0d4,  0x11);
    ISET16PHYS(0x0d4,  (rs[0] << 8) | 0x00);
    ISET16PHYS(0x0d4,  (rs[1] << 8) | 0x01);
    ISET16PHYS(0x0d4,  (rs[2] << 8) | 0x02);
    ISET16PHYS(0x0d4,  (rs[3] << 8) | 0x03);
    ISET16PHYS(0x0d4,  (rs[4] << 8) | 0x04);

#ifdef H3_A0
//
// The problem with 800x600 is due to a
// synthesis bug (which A1 does not have)
//
    if (rs[14] & 0x80)
    {
        ISET16PHYS(0xd4, ((rs[5] & 0xe0) << 8) | 0x05);
    }
    else
    {
        ISET16PHYS(0x0d4,  (rs[5] << 8) | 0x05);
    }
#else // #ifdef H3_A0
    ISET16PHYS(0x0d4,  (rs[5] << 8) | 0x05);
#endif // #ifdef H3_A0    

    ISET16PHYS(0x0d4,  (rs[6] << 8) | 0x06);
    ISET16PHYS(0x0d4,  (rs[7] << 8) | 0x07);
    ISET16PHYS(0x0d4,  (rs[8] << 8) | 0x09);
    ISET16PHYS(0x0d4,  (rs[9] << 8) | 0x10);
    ISET16PHYS(0x0d4,  (rs[10] << 8) | 0x11);
    ISET16PHYS(0x0d4,  (rs[11] << 8) | 0x12);
    ISET16PHYS(0x0d4,  (rs[12] << 8) | 0x15);
    ISET16PHYS(0x0d4,  (rs[13] << 8) | 0x16);
#ifdef H3_A0
    ISET16PHYS(0x0d4,  (rs[14] << 8) | 0x101a);
    ISET16PHYS(0x0d4,  (rs[15] << 8) | 0x101b);
#else  // #ifdef H3_A0
    ISET16PHYS(0x0d4,  (rs[14] << 8) | 0x1a);
    ISET16PHYS(0x0d4,  (rs[15] << 8) | 0x1b);
#endif // #ifdef H3_A0
    
    //
    // Enable Sync Outputs
    //
    ISET16PHYS(0x0d4, (0x80 << 8) | 0x17);

    //
    // VIDCLK (32 bit access only!)
    // Set the Video Clock to the correct frequency
    //

    ISET32(pllCtrl0, (rs[19] << 8) | rs[18]);

    //
    // dacMode (32 bit access only!)
    // (sets up 1x mode or 2x mode)
    //
    ISET32(dacMode,  rs[20]);

    //
    // the 1x / 2x bit must also be set in vidProcConfig to properly
    // enable 1x / 2x mode
    //
    vidProcCfg = IGET32(vidProcCfg);
    vidProcCfg &= ~(SST_VIDEO_2X_MODE_EN | SST_HALF_MODE);
    if (rs[20])
        vidProcCfg |= SST_VIDEO_2X_MODE_EN;
#if defined(H3VDD) && defined(H3_B0)
    if (scanlinedouble)
       vidProcCfg |= SST_HALF_MODE;
#endif
    ISET32(vidProcCfg, vidProcCfg);

    //
    // SEQ REGISTERS
    // set run mode in the sequencer (not reset)
    //
    // make sure bit 5 == 0 (i.e., screen on)
    //
    ISET16PHYS(0x0c4, ((rs[17] & ((FxU16) ~BIT(5))) << 8) | 0x1 );
    ISET16PHYS(0x0c4, ( 0x3 << 8) | 0x0 );

    //
    // turn off VGA's screen refresh, as this function only sets extended
    // video modes, and the VGA screen refresh eats up performance
    // (10% difference in screen to screen blits!).   This code is not in
    // the perl, but should stay here unless specifically decided otherwise
    //
    ISET32(vgaInit0, IGET32(vgaInit0)|BIT(12) ); 

    //
    // Make sure attribute index register is initialized
    //
    garbage = IGET8PHYS(0x0da); // return value not used

    for (i = 0; i <= 19; i++)
    {
        ISET8PHYS(0xc0, i);
        ISET8PHYS(0xc0, vgaattr[i]);
    }

    ISET8PHYS(0xc0, 0x34);

    ISET8PHYS(0xda, 0);

    //
    // Initialize VIDEO res & proc mode info...
    //

#if defined(H3VDD) && (H3_B0)
    if (scanlinedouble)
       ISET32( vidScreenSize, (yRes << 13) | (xRes & 0xfff));
    else
       ISET32( vidScreenSize, (yRes << 12) | (xRes & 0xfff));
#else
    ISET32( vidScreenSize, (yRes << 12) | (xRes & 0xfff));
#endif

    ISET32(vidOverlayStartCoords, 0);

    // I think "xFixRes" is obsolete since we're not supporting 1800
    // modes (only 1792 or 1808), so I won't use it here.   -KMW
    //
    ISET32(vidOverlayEndScreenCoord, (((yRes - 1) << 12) |
                                      ((xRes - 1) & 0xfff)));
    //
    // Load CLUTs with an inverted ramp (undo inversion with new hardware!)
    //
    // Put in another routines.
    //
    if (loadClut)
    {
        for( i=0; i <= 0x1ff; i++) {
            ISET32( dacAddr, i);    // assumes CLUT Invert Address,
            // miscInit1[0], is set 
            IGET32(dacAddr);
            j = i & 0xff;
#ifdef H3_A0
            ISET32(dacData, ~((j<<16) | (j<<8) | j));
#else  // #ifdef H3_A0
            ISET32(dacData, (j<<16) | (j<<8) | j);
#endif // #ifdef H3_A0
            IGET32(dacData);
        }
    }

    return FXTRUE; /* success! */

} // h3InitSetVideoMode


//
// Note: use h3InitVideoProc to enable/disable desktop/overlay
// surfaces and set their pixel types, and h3InitVideoDesktopSurface and
// h3InitVideoOverlaySurface to set the width/height/start address/stride
// (position, stretch, filter, etc. for overlay)
// parameters of these surfaces
//

void
h3InitVideoDesktopSurface(
    FxU32 regBase,
    FxU32 enable,               // 1=enable desktop surface (DS), 1=disable
    FxU32 tiled,                // 0=DS linear, 1=tiled
    FxU32 pixFmt,               // pixel format of DS
    FxU32 clutBypass,           // bypass clut for DS?
    FxU32 clutSelect,           // 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,         // board address of beginning of DS
    FxU32 stride)               // distance between scanlines of the DS, in
                                // units of bytes for linear DS's and tiles for
                                // tiled DS's
{
    FxU32 doStride;
    FxU32 vidProcCfg = IGET32(vidProcCfg);

    vidProcCfg &= ~ (SST_DESKTOP_EN |
                     SST_DESKTOP_TILED_EN |
                     SST_DESKTOP_PIXEL_FORMAT |
                     SST_DESKTOP_CLUT_BYPASS |
                     SST_DESKTOP_CLUT_SELECT );
    if (enable)
        vidProcCfg |= SST_DESKTOP_EN;

    if (tiled)
        vidProcCfg |= ~SST_DESKTOP_TILED_EN;

    vidProcCfg |= pixFmt;

    if (clutBypass)
        vidProcCfg |= SST_DESKTOP_CLUT_BYPASS;

    if (clutSelect)
        vidProcCfg |= SST_DESKTOP_CLUT_SELECT;

    ISET32(vidProcCfg, vidProcCfg);

    ISET32(vidDesktopStartAddr, (startAddress & SST_VIDEO_START_ADDR) <<
                                           SST_VIDEO_START_ADDR_SHIFT);

    // change only the desktop portion of the vidDesktopOverlayStride register
    //
    doStride = IGET32(vidDesktopOverlayStride);
    doStride &= ~(SST_DESKTOP_LINEAR_STRIDE | SST_DESKTOP_TILE_STRIDE);
    stride <<= SST_DESKTOP_STRIDE_SHIFT;
    if (tiled)
        stride &= SST_DESKTOP_TILE_STRIDE;
    else
        stride &= SST_DESKTOP_LINEAR_STRIDE;
    doStride |= stride;

    ISET32(vidDesktopOverlayStride, doStride);
}





void
h3InitVideoOverlaySurface(
    FxU32 regBase,
    FxU32 enable,               // 1=enable Overlay surface (OS), 1=disable
    FxU32 stereo,               // 1=enable OS stereo, 0=disable
    FxU32 horizScaling,         // 1=enable horizontal scaling, 0=disable
    FxU32 dudx,                 // horizontal scale factor (ignored if not
                                // scaling)
    FxU32 verticalScaling,      // 1=enable vertical scaling, 0=disable
    FxU32 dvdy,                 // vertical scale factor (ignored if not
                                // scaling)
    FxU32 filterMode,           // duh
    FxU32 tiled,                // 0=OS linear, 1=tiled
    FxU32 pixFmt,               // pixel format of OS
    FxU32 clutBypass,           // bypass clut for OS?
    FxU32 clutSelect,           // 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,         // board address of beginning of OS
    FxU32 stride)               // distance between scanlines of the OS, in
                                // units of bytes for linear OS's and tiles for
                                // tiled OS's
{
    FxU32 doStride;
    FxU32 vidProcCfg = IGET32(vidProcCfg);

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

    if (horizScaling)
    {
        vidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;
        ISET32(vidOverlayDudx, dudx);
    }
#ifdef H3VDD
    // XXX: assumes that stride is in bytes, otherwise this breaks
    // if stride is ever specified in tiles
    //
    ISET32(vidOverlayDudxOffsetSrcWidth, 
           (0 | (stride << SST_OVERLAY_FETCH_SIZE_SHIFT)));
#endif
    if (verticalScaling)
    {
        vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;
        ISET32(vidOverlayDvdy, dvdy);
        ISET32(vidOverlayDvdyOffset, 0);
    }

    ISET32(vidProcCfg, vidProcCfg);

    // change only the overlay portion of the vidDesktopOverlayStride register
    //
    doStride = IGET32(vidDesktopOverlayStride);
    doStride &= ~(SST_OVERLAY_LINEAR_STRIDE | SST_OVERLAY_TILE_STRIDE);
    stride <<= SST_OVERLAY_STRIDE_SHIFT;
    if (tiled)
        stride &= SST_OVERLAY_TILE_STRIDE;
    else
        stride &= SST_OVERLAY_LINEAR_STRIDE;
    doStride |= stride;

    ISET32(vidDesktopOverlayStride, doStride);
    // must set the overlay start address!
}

#ifndef H3VDD

//
// read and display performance monitor values
//
void
h3InitMeasureSiProcess(FxU32 regBase)    // init register base
{
  FxU32 siProcess, nandOsc, norOsc;
  FxU32 pciCntrLoad = 0xfff;

  if(GETENV("SSTH3_SIPROCESS_CNTR")) {
    pciCntrLoad = strtoul(GETENV("SSTH3_SIPROCESS_CNTR"),NULL,0);
    GDBG_INFO(1,"h3InitMeasureSiProcess(): Using PCI Counter preload value of 0x%x...\n", pciCntrLoad);
  }

  ////////////////////////////////
  // Test NAND oscillator tree...
  ////////////////////////////////

  ISET32(sipMonitor,
         (pciCntrLoad<<SST_SIPROCESS_PCI_CNTR_SHIFT) |
         SST_SIPROCESS_OSC_CNTR_RESET_N | SST_SIPROCESS_OSC_NAND_SEL);
  
  // Allow oscillator to run...
  IGET32(sipMonitor);
  ISET32(sipMonitor,
         (pciCntrLoad<<SST_SIPROCESS_PCI_CNTR_SHIFT) |
         SST_SIPROCESS_OSC_CNTR_RUN | SST_SIPROCESS_OSC_NAND_SEL);
  
  // Loop until PCI counter decrements to 0
  do {
    siProcess = IGET32(sipMonitor);
    //GDBG_INFO(0, "NAND siProcess = 0x%08x\n", siProcess);
  } while(siProcess & SST_SIPROCESS_PCI_CNTR);
  
  nandOsc = IGET32(sipMonitor) & SST_SIPROCESS_OSC_CNTR;
  
  GDBG_INFO(1,"h3InitMeasureSiProcess: NAND-tree: %d\n",nandOsc);


  ////////////////////////////////
  // Test NOR oscillator tree...
  ////////////////////////////////
  ISET32(sipMonitor,
         (pciCntrLoad<<SST_SIPROCESS_PCI_CNTR_SHIFT) |
         SST_SIPROCESS_OSC_CNTR_RESET_N | SST_SIPROCESS_OSC_NOR_SEL);
  
  // Allow oscillator to run...
  IGET32(sipMonitor);
  ISET32(sipMonitor,
         (pciCntrLoad<<SST_SIPROCESS_PCI_CNTR_SHIFT) |
         SST_SIPROCESS_OSC_CNTR_RUN | SST_SIPROCESS_OSC_NOR_SEL);
  
  // Loop until PCI counter decrements to 0
  do {
    siProcess = IGET32(sipMonitor);
    //GDBG_INFO(0, "NOR siProcess = 0x%08x\n", siProcess);
  } while(siProcess & SST_SIPROCESS_PCI_CNTR);
  
  norOsc = IGET32(sipMonitor) & SST_SIPROCESS_OSC_CNTR;
  
  GDBG_INFO(1,"h3InitMeasureSiProcess: NOR-tree: %d\n",norOsc);

}

#endif // #ifndef H3VDD

void
h3InitBlockWrite(
    FxU32 regBase,
    FxU32 enable,               // 1=enable block writes, 0=disable
    FxU32 threshhold)           // block write threshhold
{
    FxU32 miscInit1 = IGET32(miscInit1);

#ifdef H3_B0
    // don't allow block writes to be enabled when using SDRAM
    if ( IGET32(dramInit1) & SST_MCTL_TYPE_SDRAM ) {
      GDBG_INFO(0,"h3InitBlockWrite: cannot enable block write when using SDRAM memory\n");
      enable = 0;
    }
#endif

    miscInit1 &= ~(SST_DISABLE_2D_BLOCK_WRITE |
                   SST_BLOCK_WRITE_THRESH);
    
    if (!enable)
        miscInit1 |= SST_DISABLE_2D_BLOCK_WRITE;

    miscInit1 |= (threshhold<<SST_BLOCK_WRITE_THRESH_SHIFT) & SST_BLOCK_WRITE_THRESH;

    ISET32(miscInit1, miscInit1);

    GDBG_INFO(80,"h3InitBlockWrite: %s with threshhold set to %d\n",enable?"enabled":"disabled",
              (miscInit1&SST_BLOCK_WRITE_THRESH)>>SST_BLOCK_WRITE_THRESH_SHIFT);
}

//
// Perform a full hardware reset
//
void 
h3InitResetAll(
    FxU32 regBase)    // init register base address
{
  FxU32 miscInit0 = IGET32(miscInit0);
  FxU32 miscInit1 = IGET32(miscInit1);
  
  GDBG_INFO(80, "h3InitResetAll: initiating reset\n");
  ISET32(miscInit1, miscInit1 | SST_CMDSTREAM_RESET);
  ISET32(miscInit0, miscInit0 | 
         (SST_GRX_RESET | 
          SST_FBI_FIFO_RESET | 
          SST_VIDEO_RESET |
          SST_2D_RESET |
          SST_MEMORY_TIMING_RESET |
          SST_VGA_TIMING_RESET) 
         );
  ISET32(miscInit0, miscInit0 & 
         ~(SST_GRX_RESET | 
           SST_FBI_FIFO_RESET | 
           SST_VIDEO_RESET |
           SST_2D_RESET |
           SST_MEMORY_TIMING_RESET |
           SST_VGA_TIMING_RESET) );
  ISET32(miscInit1, miscInit1 & ~SST_CMDSTREAM_RESET);
  GDBG_INFO(80, "h3InitResetAll: reset completed\n");
}



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
** File name:   h3cinit.c
**
** Description: HW-specific initialization routines for Banshee/Avenger.
**
**
*/

#ifndef H3VDD
#include "stdlib.h"
#else
#define WIN40SERVICES
#include "h3vdd.h"
#include "h3.h"
#endif // #ifndef H3VDD

#ifdef H3VDD
#define VDDONLY
#include "tv.h"
#include "h3g.h"
#endif // #ifdef H3VDD

/* 8.3 fun */
#undef VDDONLY
#ifdef __DOS32__
#include "h3cini~1.h"
#elif defined(macintosh)
#include "h3cinitdd_mac.h"
#include "GraphicsPriv.h"
#include "GraphicsPrivHwc.h"
#include "GraphicsPrivData.h"
#else
#include "h3cinitdd.h"
#endif
#ifdef H3VDD
#define VDDONLY
#endif // #ifdef H3VDD

//#if !defined(macintosh)
#include "devtable.h"
//#endif

#include <h3cinit.h>
#include <h3.h>

// support for H4 pll tables.
#include "h4oempll.h"
#include "h4pll.h"
#include "memtable.h"

// Check for Napalm device, even when "C" simulator!
#ifdef H3VDD

#ifdef WIN_CSIM
#undef IS_NAPALM
#define IS_NAPALM(deviceID) \
       (((deviceID) == SST_VENDOR_DEVICE_ID_H5_6) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_7) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_8) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_9) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_A) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_B) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_C) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_D) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_E) || \
        ((deviceID) == SST_VENDOR_DEVICE_ID_H5_F))
#endif

#endif // #ifdef H3VDD

#if !defined(macintosh)
#include "dfp.h"
#endif

extern FxU16 crtc_table[];

#define __NAPALM_A0__ 0

// LISTEN UP
//
// The macros used in this file need a variable called regBase.
// Because the init registers are aliased both in the io and memory
// map, it's up to the caller and the implementor of h3cinitdd.h to 
// agree on whether you're using the i/o or the memory mapping
//


/*----------------------------------------------------------------------
Function name:  h3InitSgram

Description:    Initialize the dram registers.
                
Information:

Return:         FxU32   The size of memory in MBs.
----------------------------------------------------------------------*/
FxU32                           // return # of MB of memory
h3InitSgram(FxU32 regBase,       // init io-register base
            FxU32 sgramMode,
            FxU32 sgramMask,
            FxU32 sgramColor,
#if defined(H3VDD) || defined(macintosh)
            FxU32 dwVendorDeviceID,
#endif
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

  // set memory interface delay values and enable refresh
  // these apply to all RAM vendors
  dramInit1 = 0x0;

  if(IS_NAPALM(dwVendorDeviceID)) {
#if __NAPALM_A0__ /* A0 boards */
    dramInit1 |= SST_DRAM_REFRESH_EN;
    dramInit1 |= (0x18 << SST_DRAM_REFRESH_VALUE_SHIFT) & SST_DRAM_REFRESH_VALUE;  
    dramInit1 |= SST_SGRAM_USE_INV_SAMPLE;
    dramInit1 |= SST_SGRAM_DEL_CLK_INVERT;
    dramInit1 |= (6<<SST_SGRAM_CLK_ADJ_SHIFT);
    dramInit1 |= (5<<SST_SGRAM_OFLOP_DEL_ADJ_SHIFT);
#else /* boards */
    dramInit1 |= 0x00240031;    
//    dramInit1 |= SST_DRAM_REFRESH_EN;
//    dramInit1 |= (0x18 << SST_DRAM_REFRESH_VALUE_SHIFT) & SST_DRAM_REFRESH_VALUE;  
//    dramInit1 |= SST_SGRAM_USE_INV_SAMPLE; //
//    dramInit1 |= SST_SGRAM_DEL_CLK_INVERT; //
//    dramInit1 |= (6<<SST_SGRAM_CLK_ADJ_SHIFT); //
//    dramInit1 |= (2<<SST_SGRAM_OFLOP_DEL_ADJ_SHIFT);
#endif
  } else {
    dramInit1 |= SST_DRAM_REFRESH_EN;
    dramInit1 |= (0x18 << SST_DRAM_REFRESH_VALUE_SHIFT) & SST_DRAM_REFRESH_VALUE;  
    dramInit1 |= (3<<SST_SGRAM_CLK_ADJ_SHIFT);
    dramInit1 |= (5<<SST_SGRAM_OFLOP_DEL_ADJ_SHIFT);
  }
#if !defined(H3VDD) && !defined(DIAG_BUILD) /* && !defined(MACOS_GDX) */
#if !defined(MACOS_GDX)
  if ( GETENV("SSTH3_DRAMINIT1") )
    dramInit1 = strtoul(GETENV("SSTH3_DRAMINIT1"),NULL,0);
#endif
  dramInit1 &= ~SST_MCTL_TYPE_SDRAM;
  dramInit1 |= dramInit1_strap;
  dramInit1 |= SST_MCTL_TYPE_SDRAM;

  GDBG_INFO(0, "h3InitSgram: dramInit1 = 0x%x\n",dramInit1);
  ISET32(dramInit1, dramInit1);
#endif // #ifndef H3VDD

  /* Determine memory size from strapping pins (dramInit0). */

  dramInit0_strap = IGET32(dramInit0);

  if (IS_NAPALM(dwVendorDeviceID))
  {
    // Napalm memory sizing

    dramInit0_strap &= SST_H5_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS | SST_MCTL_DRAM_NUMBANKS;
    nChips = ((dramInit0_strap & SST_SGRAM_NUM_CHIPSETS) == 0) ? 4 : 8;

    switch (dramInit0_strap & SST_H5_SGRAM_TYPE)
    {
      case SST_SGRAM_TYPE_8MBIT:   partSize = 8;    break;
      case SST_SGRAM_TYPE_16MBIT:  partSize = 16;   break;
      case SST_SGRAM_TYPE_32MBIT:  partSize = 32;   break;
      case SST_SGRAM_TYPE_64MBIT:  partSize = 64;   break;
      case SST_SGRAM_TYPE_128MBIT: partSize = 128;  break;
      default: return 0;  // invalid sgram type!
    }
  }
  else
  {
    // Banshee and Avenger memory sizing

    dramInit0_strap &= SST_H4_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS;
    if ( memType == MEM_TYPE_SDRAM ) 
    {
      nChips = 8;
      partSize = 16;
    
      // avoid minor performance hit when using SDRAM
      dramInit0_strap &= ~SST_SGRAM_NUM_CHIPSETS;
    } 
    else 
    {
      nChips = ((dramInit0_strap & SST_SGRAM_NUM_CHIPSETS) == 0) ? 4 : 8;

      switch (dramInit0_strap & SST_H4_SGRAM_TYPE)
      {
        case SST_SGRAM_TYPE_8MBIT:   partSize = 8;    break;
        case SST_SGRAM_TYPE_16MBIT:  partSize = 16;   break;
        default: return 0;  // invalid sgram type!
      }
    }
  }


  /* Compute memory size in megabytes. */

  memSize = (nChips * partSize) / 8;
  GDBG_INFO(0, "h3InitSgram: partSize = %d, nChips = %d, memSize = %d\n",partSize, nChips, memSize);

  /* Setup dramInit0 based on vendor table. */

  if ( memType == MEM_TYPE_SDRAM )        // set defaults
    vendorID = MEM_DEFAULT_SDRAM_16;
  else
    vendorID = partSize == 8 ? MEM_DEFAULT_SGRAM_8 : MEM_DEFAULT_SGRAM_16;

#define TO_UPPER(c)  ( ((c)>='a'&&(c)<='z') ? ((c)-'a'+'A') : (c) )   /* only works for alphabetic characters */

  if ( vendorName ) 
  {
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
#if !defined(MACOS_GDX)    
    if ( i >= MEM_TABLE_SIZE )
      GDBG_ERROR("h3InitSgram","vendor %s not found, defaulting to %s\n",
                 vendorName, memTable[vendorID].vendor);
#endif                 
  }

  GDBG_INFO(80, "h3InitSgram: %d x %d Mbit %s %s parts = %d MBytes\n", 
            nChips, partSize, memTable[vendorID].vendor, 
            memType == MEM_TYPE_SDRAM ? "SDRAM" : "SGRAM", memSize);
  
#if defined(MACOS_GDX)
  if(IS_NAPALM(dwVendorDeviceID))
#if __NAPALM_A0__
    dramInit0 = 0x00169d25; /* A0 */
#else
//    dramInit0 = 0x001EA9A9; /* A1 */
    dramInit0 = 0x001EA9b9; /* A2  @ 166 Mhz */
#endif
  else
    dramInit0 = 0x0017a9e9;  
#else    
  dramInit0 = memTable[vendorID].dramInit0;
#endif  
  // write vendor specific SGRAM timings, preserving the SGRAM chipset/size
  // information
  
#if !defined(H3VDD) && !defined(DIAG_BUILD) && !defined(MACOS_GDX)
  if ( GETENV("SSTH3_DRAMINIT0") )
    dramInit0 = strtoul(GETENV("SSTH3_DRAMINIT0"),NULL,0);
#endif // #ifndef H3VDD

  if (IS_NAPALM(dwVendorDeviceID))
  {
    dramInit0 &= ~(SST_H5_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS | SST_MCTL_DRAM_NUMBANKS);
  }
  else
  {
    dramInit0 &= ~(SST_H4_SGRAM_TYPE | SST_SGRAM_NUM_CHIPSETS);
  }

  dramInit0 |= dramInit0_strap;

#if !defined(H3VDD) && !defined(DIAG_BUILD) /* && !defined(MACOS_GDX) */
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
#if !defined(H3VDD) && !defined(MACOS_GDX)
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


/*----------------------------------------------------------------------
Function name:  h4InitPlls

Description:    Initialize the Avenger Plls.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
h4InitPlls(FxU32 regBase,               // init register base
           FxU32 deviceID,              // H4 or H4_OEM
           FxU32 grxSpeedInMHz)         // desired clock frequency (MHz)
{
  FxU32 maxGrxSpeedInMHz;

  if (deviceID == SST_DEVICE_ID_H4_OEM) {
    maxGrxSpeedInMHz = MAX_H4_OEM_PLL_FREQ;
  }
  else {
    maxGrxSpeedInMHz = MAX_H4_PLL_FREQ;
  }

  // set the graphics clock
  //
  if ( grxSpeedInMHz < MIN_PLL_FREQ || grxSpeedInMHz > maxGrxSpeedInMHz ) {
    MESSAGE("YO!  GRX clock speed of %d MHz is out of range MIN_PLL_FREQ to maxGrxSpeedInMHz\n", grxSpeedInMHz);
  } else {
    GDBG_INFO(80, "Setting Graphics Clock (%d MHz)\n",grxSpeedInMHz);

    if (deviceID == SST_DEVICE_ID_H4_OEM) {
      ISET32(pllCtrl1, h4oempllTable[grxSpeedInMHz]);
    }
    else {
      ISET32(pllCtrl1, h4pllTable[grxSpeedInMHz]);
    }
  }

} // h4InitPlls

/*----------------------------------------------------------------------
Function name:  h3InitVga

Description:    Initialize the VGA.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
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


/*----------------------------------------------------------------------
Function name:  h3InitVideoProc

Description:    Initialize the Video portion of the HW.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
h3InitVideoProc(FxU32 regBase,
                FxU32 vidProcCfg)       // control bits for video processor
{
    ISET32(vidProcCfg, vidProcCfg);
    
} // h3InitVideo

#if MACOS_GDX
// This is a hack to make the mode tables take up about half the amount of
// space that they normall would.
struct
{
   FxU16 xSize, ySize, refresh;
   FxU8 data[21];
} mode_table[] =
{
#include "modetabl.h"
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

#endif

FxU8 vgaattr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 
                  0x00, 0x00, 0x00, 0x00, 0x00, 
                  0x00, 0x00, 0x00, 0x00, 0x00, 
                  0x00, 0x01, 0x00, 0x0F, 0x00};


/*----------------------------------------------------------------------
Function name:  h3InitFindVideoMode

Description:    Find the video mode in the mode table based on the
                xRes, yRes, and refresh rate.
Information:

Return:         (FxU16 *)   Ptr to the entry in the mode table,
                            NULL if failure.
----------------------------------------------------------------------*/

#if MACOS_GDX
FxU8 *
h3InitFindVideoMode(FxU32 xRes,
                    FxU32 yRes,
                    FxU32 refresh)
{
    int i = 0;

    while (mode_table[i].xSize != 0)
    {
        if ((mode_table[i].xSize == xRes) &&
            (mode_table[i].ySize == yRes) &&
            (mode_table[i].refresh == refresh))
        {
            return &mode_table[i].data[0];
        }

        i += 1;
    }

    return NULL;
}
#endif

/*----------------------------------------------------------------------
Function name:  h3InitSetVideoMode

Description:    Set the video mode.

Information:

Return:         FxBool  FXTRUE  if success or,
                        FXFALSE if failure.
----------------------------------------------------------------------*/
FxBool
h3InitSetVideoMode(
#if defined(H3VDD) || defined(macintosh)
  PDEVTABLE pDev,
#endif
    FxU32 xRes,    // xResolution in pixels
    FxU32 yRes,    // yResolution in pixels
    FxU32 refresh, // refresh rate in Hz
#if defined(H3VDD) || defined(MACOS_GDX)
  FxU32 loadClut, // really a bool, should we load the lookup table
  FxU32 scanlinedouble) // set scanline double bit and double y?
#else
  FxU32 loadClut) // really a bool, should we load the lookup table
#endif
{
    FxU16 i, j;
    FxU8 garbage;
    FxU16 *rs;
    FxU32 vidProcCfg;
    FxU32 regBase = pDev->IoBase;

    rs = &crtc_table[0];		// DYNAMIC MODE TABLE
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
    ISET16PHYS(0x0d4,  (rs[5] << 8) | 0x05);
    ISET16PHYS(0x0d4,  (rs[6] << 8) | 0x06);
    ISET16PHYS(0x0d4,  (rs[7] << 8) | 0x07);
    ISET16PHYS(0x0d4,  (rs[8] << 8) | 0x09);
    ISET16PHYS(0x0d4,  (rs[9] << 8) | 0x10);
    ISET16PHYS(0x0d4,  (rs[10] << 8) | 0x11);
    ISET16PHYS(0x0d4,  (rs[11] << 8) | 0x12);
    ISET16PHYS(0x0d4,  (rs[12] << 8) | 0x15);
    ISET16PHYS(0x0d4,  (rs[13] << 8) | 0x16);
    ISET16PHYS(0x0d4,  (rs[14] << 8) | 0x1a);
    ISET16PHYS(0x0d4,  (rs[15] << 8) | 0x1b);
    
    ISET8PHYS(0xc2, rs[16]);

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
#if defined(H3VDD) || defined(MACOS_GDX)
    if (scanlinedouble)
    {
      vidProcCfg |= SST_HALF_MODE;
      vgaattr[ 0x10 ] = 0x41;
    }
    else
      vgaattr[ 0x10 ] = 0x01;

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

#if defined(H3VDD) || defined(MACOS_GDX)
    if (scanlinedouble)
       ISET32( vidScreenSize, (yRes << 13) | (xRes & 0xfff));
    else
       {
       if (IS_VOODOO3(pDev->dwVendorDeviceID) && (2048 == xRes))
          ISET32( vidScreenSize, (yRes << 12) | ((xRes - 2) & 0xfff));
       else
          ISET32( vidScreenSize, (yRes << 12) | (xRes & 0xfff));
       }
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
    //return FXTRUE;
    
#if !defined(MACOS_GDX)    
    if (loadClut)
    {
        for( i=0; i <= 0x1ff; i++) {
            ISET32( dacAddr, i);    // assumes CLUT Invert Address,
            // miscInit1[0], is set 
            IGET32(dacAddr);
            j = i & 0xff;
            ISET32(dacData, (j<<16) | (j<<8) | j);
            IGET32(dacData);
        }
    }
#endif
    return FXTRUE; /* success! */

} // h3InitSetVideoMode


/*----------------------------------------------------------------------
Function name:  h3InitVideoDesktopSurface

Description:    Set the width/height/start address/stride (position,
                stretch, filter, etc. for overlay) parameters of
                these surfaces.
Information:

Return:         VOID
----------------------------------------------------------------------*/
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
        vidProcCfg |= SST_DESKTOP_TILED_EN;

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


/*----------------------------------------------------------------------
Function name:  h3InitVideoOverlaySurface

Description:    Initialize the video overlay surface.

Information:

Return:         VOID
----------------------------------------------------------------------*/
#if !MACOS_GDX
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
    if (tiled)
	{
	  ISET32(vidOverlayDudxOffsetSrcWidth, (0 | ((stride << 7) << SST_OVERLAY_FETCH_SIZE_SHIFT)));
	}
	else
	{
	  ISET32(vidOverlayDudxOffsetSrcWidth, (0 | (stride << SST_OVERLAY_FETCH_SIZE_SHIFT)));
	}
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
#endif

/*----------------------------------------------------------------------
Function name:  h3InitMeasureSiProcess

Description:    Read and display to the debug output device the
                performance monitor values.
Information:

Return:         VOID
----------------------------------------------------------------------*/
#if !defined(H3VDD) && !defined(MACOS_GDX)

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
  } while(siProcess & SST_SIPROCESS_PCI_CNTR);
  
  norOsc = IGET32(sipMonitor) & SST_SIPROCESS_OSC_CNTR;
  
  GDBG_INFO(1,"h3InitMeasureSiProcess: NOR-tree: %d\n",norOsc);

}

#endif // #ifndef H3VDD


/*----------------------------------------------------------------------
Function name:  h3InitBlockWrite

Description:    Perform a block write.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
h3InitBlockWrite(
    FxU32 regBase,
    FxU32 enable,               // 1=enable block writes, 0=disable
    FxU32 threshhold)           // block write threshhold
{
    FxU32 miscInit1 = IGET32(miscInit1);

    // don't allow block writes to be enabled when using SDRAM
    if ( IGET32(dramInit1) & SST_MCTL_TYPE_SDRAM ) {
      GDBG_INFO(0,"h3InitBlockWrite: cannot enable block write when using SDRAM memory\n");
      enable = 0;
    }

    miscInit1 &= ~(SST_DISABLE_2D_BLOCK_WRITE |
                   SST_BLOCK_WRITE_THRESH);
    
    if (!enable)
        miscInit1 |= SST_DISABLE_2D_BLOCK_WRITE;

    miscInit1 |= (threshhold<<SST_BLOCK_WRITE_THRESH_SHIFT) & SST_BLOCK_WRITE_THRESH;

    ISET32(miscInit1, miscInit1);

    GDBG_INFO(80,"h3InitBlockWrite: %s with threshhold set to %d\n",enable?"enabled":"disabled",
              (miscInit1&SST_BLOCK_WRITE_THRESH)>>SST_BLOCK_WRITE_THRESH_SHIFT);
}


/*----------------------------------------------------------------------
Function name:  h3InitResetAll

Description:    Perform a full HW reset.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
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



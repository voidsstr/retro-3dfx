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
** File name:   sliaa.h
**
** Description: Routines to Initialize SLI AA for Napalm.
**
** $Revision: 10$
** $Date: 10/11/00 8:32:22 PM$
**
** $History: SLIAA.H $
**
** *****************  Version 1  *****************
** User: Jw           Date: 9/09/99    Time: 10:56a
** Created in $/devel/h5/W2K/Src/Video/Miniport/h5
**
** *****************  Version 8  *****************
** User: Andrew       Date: 8/20/99    Time: 11:43a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Minor change to disable aa when SLI and AA are disabled.
**
** *****************  Version 7  *****************
** User: Andrew       Date: 7/28/99    Time: 3:46p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixes to support SLI on WINSIM
**
** *****************  Version 6  *****************
** User: Cwilcox      Date: 7/27/99    Time: 10:41a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added #ifdef SLI_AA to remove non-simulator compiler error.
**
** *****************  Version 5  *****************
** User: Andrew       Date: 7/26/99    Time: 5:52p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added some defines for PCI_DECODE
**
** *****************  Version 4  *****************
** User: Andrew       Date: 7/20/99    Time: 10:53a
** Updated in $/devel/h5/Win9x/dx/minivdd
** added defines for SLICTRL
**
** *****************  Version 3  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
**
** *****************  Version 2  *****************
** User: Andrew       Date: 7/09/99    Time: 8:26a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed unused structure members
**
** *****************  Version 1  *****************
** User: Andrew       Date: 6/25/99    Time: 10:04a
** Created in $/devel/h5/Win9x/dx/minivdd
** Defines and prototypes for sliaa
**
**
*/

#ifndef _SLI_H_

#define _SLI_H_

#ifdef SLI_AA

typedef unsigned long DWORD;

// Here are the functions defines

#define SLI_AA_2_SAMPLE_AA              (0x00)
#define SLI_AA_4_SAMPLE_AA              (0x01)

#define SLI_AA_DIGITAL                  (0x00)
#define SLI_AA_ANALOG                   (0x01)

#define CFG_SWAP_ALGORITHM_VSYNC        (0x00)
#define CFG_SWAP_ALGORITHM_SYNCIN       (0x01)
#define CFG_SWAPBUFFER_ALGORITHM_SHIFT  (25)

#define MAX_CHIPS                       (4)

#define SLI_AA_REGULAR_DEVICE           (0x00)
#define SLI_AA_MASTER_DEVICE            (0x01)
#define SLI_AA_SLAVE_DEVICE             (0x02)
#define HWINFO_SST_MAX_NUM_CHIPS        (0x04)

#if 0
//moved this crap to h3.h
//#define HWINFO_SST_IOREGS_INDEX         (0x00)
//#define HWINFO_SST_CMDFIFOREGS_INDEX    (0x01)
//#define HWINFO_SST_2DREGS_INDEX         (0x02)
//#define HWINFO_SST_3DREGS_INDEX         (0x03)
//#define HWINFO_SST_MAX_CHIP_INDEX       (0x04)
#endif
#define HWINFO_SST_MAX_BASE_INDEX       ((HWINFO_SST_MAX_CHIP_INDEX) * (HWINFO_SST_MAX_NUM_CHIPS))

#if 0
// this crap is crap on NT
typedef struct devtable
{
  DWORD           dwVendorDeviceID;

  DWORD           dwBus;
  DWORD           dwDevFunc;
  DWORD           dwUnitNum;
  DWORD           dwType;
  struct devtable *pSlave;
  DWORD           SliaaLfbBase;
  DWORD           nPages;
  DWORD           dwDDCCount[4];          // One for Each Potential Request
  //DWORD           PCISaveSpace[POWERPCISIZE];

  // Taken from h3vdd.c
  DWORD           IoBase;                 // base of PCI relocatable IO space
  DWORD           PhysMemBase[2];         // physical register address space
  DWORD           RegBase[HWINFO_SST_MAX_BASE_INDEX]; // CPU linear address mapping to PhysMemBase[0]
  DWORD           LfbBase;                // CPU linear address mapping to PhysMemBase[1]

  DWORD           MemSizeInMB;            // # of megabytes of board memory
  DWORD           lfbMemoryConfig[2];     // lfbMemoryConfig
} DEVTABLE, * PDEVTABLE;
#endif

typedef struct chipinfo
{
  DWORD     dwChips;                      // dwChips: Number of chips in multi-chip configuration (1-4)
  DWORD     dwsliEn;                      // dwsliEn: Sli is to be enabled (0,1)
  DWORD     dwaaEn;                       // dwaaEn: Anti-aliasing is to be enabled (0,1)
  DWORD     dwaaSampleHigh;               // dwaaSampleHigh: 0->Enable 2-sample AA, 1->Enable 4-sample AA
  DWORD     dwsliAaAnalog;                // dwsliAaAnalog: 0->Enable digital SLI/AA, 1->Enable analog Sli/AA
  DWORD     dwsli_nlines;                 // dwsli_nLines: Number of lines owned by each chip in SLI (2-128)
  DWORD     dwCfgSwapAlgorithm;           // Swap Buffer Algorithm
  DWORD     dwMasterID;                   // ID number of the Master Chip <1..n>
  //DEVTABLE  *pDevTable;                   // Pointer to Device Table
} CHIPINFO, * PCHIPINFO;

typedef struct meminfo
{
  DWORD     dwTotalMemory;                // In MB
  DWORD     dwTileMark;                   // In MB
  DWORD     dwTileCmpMark;                // In MB
  DWORD     dwaaSecondaryColorBufBegin;   // In MB
  DWORD     dwaaSecondaryDepthBufBegin;   // In MB
  DWORD     dwaaSecondaryDepthBufEnd;     // In MB
} MEMINFO, * PMEMINFO;

extern void DetectNumUnits(PHW_DEVICE_EXTENSION HwDeviceExtension);
extern void InitializeSlaveChipsPCIConfigSpace(PHW_DEVICE_EXTENSION HwDeviceExtension);
extern void MapSlaveChipsAccessRanges(PHW_DEVICE_EXTENSION HwDeviceExtension);
extern void InitializeSlaveChipsInitRegs(PHW_DEVICE_EXTENSION HwDeviceExtension);

extern VP_STATUS GlideMapSlaveChips(PHW_DEVICE_EXTENSION,HANDLE,PVIDEO_PUBLIC_ACCESS_RANGES);
extern VP_STATUS GlideUnmapSlaveChips(PHW_DEVICE_EXTENSION,GLIDE_BASE_INFO*);

extern void H3_DISABLE_SLI_AA(PHW_DEVICE_EXTENSION,PCHIPINFO,PSLI_AA_MEMINFO);
extern void H3_SETUP_SLI_AA(PHW_DEVICE_EXTENSION,DWORD,PCHIPINFO,PSLI_AA_MEMINFO);
extern void EnableSLIAA(PHW_DEVICE_EXTENSION, SLI_AA_REQUEST *);
extern void DisableSLIAA(PHW_DEVICE_EXTENSION, SLI_AA_REQUEST *);

#ifdef RD_ABORT_ERROR
extern void SLI_Read_Enable(PHW_DEVICE_EXTENSION HwDeviceExtension);
extern void SLI_Read_Disable(PHW_DEVICE_EXTENSION HwDeviceExtension);
#endif
/*
   This function sets up the following registers --

   PCIINIT0
   CFG_INIT_ENABLE
   CFG_PCI_DECODE
   CFG_SLI_LFB_CTRL
   CFG_SLI_AA_TILED_APER
   CFG_AA_LFB_CTRL
   CFG_SLI_AA_MISC
   CFG_VIDEO_CTRL0
   CFG_VIDEO_CTRL1
   CFG_VIDEO_CTRL2
   LFBMEMORYCONFIG

   Outside of this function the following 3D Registers need to be setup

   SLICTRL
   AACTRL
   CHIPMASK
   COLBUFFERADDR
   AUXBUFFERADDR

   Outside of this function the following Video Registers need to be setup
   vidDesktopStartAddr
   vidCurrOverlayStartAddr
   vidSerialParallelPort


   Before this function is called it is best to take the primary's primary surface on
   copy it to the secondary's primary surface.

   On, exit a reverse operation where the strips from each non-master chip need
   to be copied to the primary surface.
*/

// The following defines were taken from a many locations from the
// source code used to write the c-simulator

// taken from  h3cfg.h
// config space registers
#define VENDORID        0
#define DEVICEID        2
#define PCICOMMAND      4
#define PCISTATUS       6
#define REVISIONID      8
#define CLASSCODE       9
#define CACHELINESIZE   12
#define LATENCYTIMER    13
#define HEADERTYPE      14
#define BIST            15
#define MEMBASEADDR0    16
#define MEMBASEADDR1    20
#define IOBASEADDR      24
#define RESBASEADDR0    28
#define RESBASEADDR1    32
#define RESBASEADDR2    36
#define CISPOINTER      40
#define SUBVENDORID     44
#define SUBSYSTEMID     46
//
#define ROMBASEADDR     48
#define CAPPTR          52
#define MISCRES0        56
//
#define INTERRUPTLINE   60
#define INTERRUPTPIN    61
#define MINGNT          62
#define MAXLAT          63

#define FABID           64
#define CFGSTATUS       76
#define CFGSCRATCH      80

#define CAPID_SHIFT     0
#define CAPID_MASK      0xFF
#define NEXTPTR_SHIFT   8
#define NEXTPTR_MASK    (0xFF << NEXTPTR_SHIFT)

#define AGPCAPID        84
#define AGPCAPIDNEXT    85
#define AGPCAPIDOTHER   86
#define AGPSTATUS       88
#define AGPCMD          92

#define ACPICAPID       96
#define ACPICAPIDNEXT   97
#define ACPICAPIDPMC    98
#define ACPISTATUS      100

// New for Napalm...
#define CFG_INIT_ENABLE        64
#define CFG_PCI_DECODE         72
#define CFG_VIDEO_CTRL0        128
#define CFG_VIDEO_CTRL1        132
#define CFG_VIDEO_CTRL2        136
#define CFG_SLI_LFB_CTRL       140
#define CFG_AA_ZBUFF_APERTURE  144
#define CFG_AA_LFB_CTRL        148
#define CFG_SLI_AA_MISC        172

// end of config space registers

//#define BIT(n)  (1UL<<(n))
#define SST_MASK(n) (0xFFFFFFFFL >> (32-(n)))

// bit definitions of config space registers
#define PCISTATUS_CAPLIST        BIT(4)

#define AGP_ID                   0x2

#define AGPSTATUS_RATE           0x7
#define AGPSTATUS_RES0           (SST_MASK(1) << 3)
#define AGPSTATUS_4G             BIT(5)
#define AGPSTATUS_RES1           (SST_MASK(3) << 6)
#define AGPSTATUS_SBA            BIT(9)
#define AGPSTATUS_RES2           (SST_MASK(14) << 10)
#define AGPSTATUS_RQDEPTH_SHIFT  24
#define AGPSTATUS_RQDEPTH        (SST_MASK(8) << AGPSTATUS_RQDEPTH_SHIFT)

#define AGPCMD_RATE              0x7
#define AGPCMD_RES0              (SST_MASK(1) << 2)
#define AGPCMD_4G                BIT(5)
#define AGPCMD_RES1              (SST_MASK(2) << 6)
#define AGPCMD_ENABLE            BIT(8)
#define AGPCMD_SBAENABLE         BIT(9)
#define AGPCMD_RES2              (SST_MASK(14) << 10)
#define AGPCMD_RQDEPTH_SHIFT     24
#define AGPCMD_RQDEPTH           (SST_MASK(8) << AGPCMD_RQDEPTH_SHIFT)

// New for Napalm...
// CFG_INIT_ENABLE
#define CFG_UPDATE_MEMBASE_LSBS   BIT(10)
#define CFG_SNOOP_EN              BIT(11)
#define CFG_SNOOP_MEMBASE0_EN     BIT(12)
#define CFG_SNOOP_MEMBASE1_EN     BIT(13)
#define CFG_SNOOP_SLAVE           BIT(14)
#define CFG_SNOOP_MEMBASE0_SHIFT  15
#define CFG_SNOOP_MEMBASE0        (0x3FF<<CFG_SNOOP_MEMBASE0_SHIFT)
#define CFG_SWAP_ALGORITHM        BIT(25)
#define CFG_SWAP_MASTER           BIT(26)
#define CFG_SWAP_QUICK            BIT(27)
#define CFG_MULTI_FUNCTION_DEV    BIT(28)
#define CFG_LFB_RD_CACHE_DISABLE  BIT(29)
#define CFG_SNOOP_FBIINIT_WR_EN   BIT(30)

// CFG_PCI_DECODE
#define CFG_MEMBASE0_DECODE_SHIFT	0
#define CFG_MEMBASE0_DECODE			(0xF<<CFG_MEMBASE0_DECODE_SHIFT)
#define CFG_MEMBASE1_DECODE_SHIFT	4
#define CFG_MEMBASE1_DECODE			(0xF<<CFG_MEMBASE1_DECODE_SHIFT)
#define CFG_IOBASE_DECODE_SHIFT		8
#define CFG_IOBASE_DECODE				(0x3<<CFG_IOBASE_DECODE_SHIFT)
#define CFG_SNOOP_MEMBASE0_DECODE_SHIFT	10
#define CFG_SNOOP_MEMBASE0_DECODE			(0xF<<CFG_SNOOP_MEMBASE0_DECODE_SHIFT)
#define CFG_SNOOP_MEMBASE1_DECODE_SHIFT	14
#define CFG_SNOOP_MEMBASE1_DECODE			(0xF<<CFG_SNOOP_MEMBASE1_DECODE_SHIFT)
#define CFG_SNOOP_MEMBASE1_SHIFT		18
#define CFG_SNOOP_MEMBASE1				(0x3FF<<CFG_SNOOP_MEMBASE1_SHIFT)

#define CFG_MEMBASE0_4MB_DECODE (0x08)
#define CFG_MEMBASE0_8MB_DECODE (0x07)
#define CFG_MEMBASE0_16MB_DECODE (0x06)
#define CFG_MEMBASE0_32MB_DECODE (0x05)
#define CFG_MEMBASE0_64MB_DECODE (0x04)
#define CFG_MEMBASE0_128MB_DECODE (0x00)
#define CFG_MEMBASE0_256MB_DECODE (0x01)
#define CFG_MEMBASE0_512MB_DECODE (0x02)
#define CFG_MEMBASE0_1024MB_DECODE (0x03)

// CFG_VIDEO_CTRL0
#define CFG_ENHANCED_VIDEO_EN				BIT(0)
#define CFG_ENHANCED_VIDEO_SLV			BIT(1)
#define CFG_VIDEO_TV_OUTPUT_EN			BIT(2)
#define CFG_VIDEO_LOCALMUX_SEL			BIT(3)
#define CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY	BIT(3)
#define CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT 4
#define CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT 6
#define CFG_VIDEO_OTHERMUX_SEL_TRUE		(0x3<<CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT)
#define CFG_VIDEO_OTHERMUX_SEL_FALSE	(0x3<<CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT)
#define CFG_VIDEO_OTHERMUX_SEL_PIPE		 0
#define CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO 1
#define CFG_VIDEO_OTHERMUX_SEL_AAFIFO 2
#define CFG_SLI_FETCH_COMPARE_INV		BIT(8)
#define CFG_SLI_CRT_COMPARE_INV			BIT(9)
#define CFG_SLI_AAFIFO_COMPARE_INV		BIT(10)
#define CFG_VIDPLL_SEL                 BIT(11)
#define CFG_DIVIDE_VIDEO_SHIFT			12
#define CFG_DIVIDE_VIDEO					(0x7<<CFG_DIVIDE_VIDEO_SHIFT)
#define CFG_DIVIDE_VIDEO_BY_1				(0x0<<CFG_DIVIDE_VIDEO_SHIFT)
#define CFG_DIVIDE_VIDEO_BY_2				(0x1<<CFG_DIVIDE_VIDEO_SHIFT)
#define CFG_DIVIDE_VIDEO_BY_4				(0x2<<CFG_DIVIDE_VIDEO_SHIFT)
#define CFG_DIVIDE_VIDEO_BY_8				(0x3<<CFG_DIVIDE_VIDEO_SHIFT)
#define CFG_DIVIDE_VIDEO_BY_16			(0x4<<CFG_DIVIDE_VIDEO_SHIFT)
#define CFG_DIVIDE_VIDEO_BY_32			(0x5<<CFG_DIVIDE_VIDEO_SHIFT)
#define CFG_ALWAYS_DRIVE_AA_BUS			BIT(15)
#define CFG_VSYNC_IN_DEL_SHIFT			16
#define CFG_VSYNC_IN_DEL					(0xF<<CFG_VSYNC_IN_DEL_SHIFT)
#define CFG_DAC_VSYNC_TRISTATE			BIT(24)
#define CFG_DAC_HSYNC_TRISTATE			BIT(25)

// CFG_VIDEO_CTRL1
#define CFG_SLI_RENDERMASK_FETCH_SHIFT	 0
#define CFG_SLI_RENDERMASK_FETCH			 (0xFF<<CFG_SLI_RENDERMASK_FETCH_SHIFT)
#define CFG_SLI_COMPAREMASK_FETCH_SHIFT 8
#define CFG_SLI_COMPAREMASK_FETCH		 (0xFF<<CFG_SLI_COMPAREMASK_FETCH_SHIFT)
#define CFG_SLI_RENDERMASK_CRT_SHIFT	 16
#define CFG_SLI_RENDERMASK_CRT			 (0xFF<<CFG_SLI_RENDERMASK_CRT_SHIFT)
#define CFG_SLI_COMPAREMASK_CRT_SHIFT	 24
#define CFG_SLI_COMPAREMASK_CRT			 (0xFF<<CFG_SLI_COMPAREMASK_CRT_SHIFT)

// CFG_VIDEO_CTRL2
#define CFG_SLI_RENDERMASK_AAFIFO_SHIFT  0
#define CFG_SLI_RENDERMASK_AAFIFO		  (0xFF<<CFG_SLI_RENDERMASK_AAFIFO_SHIFT)
#define CFG_SLI_COMPAREMASK_AAFIFO_SHIFT 8
#define CFG_SLI_COMPAREMASK_AAFIFO		 (0xFF<<CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)

// CFG_SLI_LFB_CTRL
#define CFG_SLI_LFB_RENDERMASK_SHIFT	0
#define CFG_SLI_LFB_RENDERMASK			(0xFF<<CFG_SLI_LFB_RENDERMASK_SHIFT)
#define CFG_SLI_LFB_COMPAREMASK_SHIFT	8
#define CFG_SLI_LFB_COMPAREMASK			(0xFF<<CFG_SLI_LFB_COMPAREMASK_SHIFT)
#define CFG_SLI_LFB_SCANMASK_SHIFT		16	
#define CFG_SLI_LFB_SCANMASK				(0xFF<<CFG_SLI_LFB_SCANMASK_SHIFT)
#define CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT 24
#define CFG_SLI_LFB_NUMCHIPS_LOG2		 (0x3<<CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT)
#define CFG_SLI_LFB_CPU_WR_EN				BIT(26)
#define CFG_SLI_LFB_DPTCH_WR_EN			BIT(27)
#define CFG_SLI_RD_EN						BIT(28)

// CFG_AA_ZBUFF_APERTURE
#define CFG_AA_DEPTH_BUFFER_BEG_SHIFT	0
#define CFG_AA_DEPTH_BUFFER_BEG			(0x7FFF<<CFG_AA_DEPTH_BUFFER_BEG_SHIFT)
#define CFG_AA_DEPTH_BUFFER_END_SHIFT	16
#define CFG_AA_DEPTH_BUFFER_END			(0xFFFF<<CFG_AA_DEPTH_BUFFER_END_SHIFT)

// CFG_AA_LFB_CTRL
#define CFG_AA_BASEADDR_SHIFT				0
#define CFG_AA_BASEADDR						(0x3FFFFFF<<CFG_AA_BASEADDR_SHIFT)
#define CFG_AA_LFB_CPU_WR_EN				BIT(26)
#define CFG_AA_LFB_DPTCH_WR_EN			BIT(27)
#define CFG_AA_LFB_RD_EN					BIT(28)
#define CFG_AA_LFB_RD_FORMAT_SHIFT		29
#define CFG_AA_LFB_RD_FORMAT				(0x3<<CFG_AA_LFB_RD_FORMAT_SHIFT)
#define CFG_AA_LFB_RD_FORMAT_16BPP		(0x0<<CFG_AA_LFB_RD_FORMAT_SHIFT)
#define CFG_AA_LFB_RD_FORMAT_15BPP		(0x1<<CFG_AA_LFB_RD_FORMAT_SHIFT)
#define CFG_AA_LFB_RD_FORMAT_32BPP		(0x2<<CFG_AA_LFB_RD_FORMAT_SHIFT)
#define CFG_AA_LFB_RD_DIVIDE_BY_4		BIT(31)

// CFG_SLI_AA_MISC
#define CFG_VGA_VSYNC_OFFSET_SHIFT		0
#define CFG_VGA_VSYNC_OFFSET				(0x1ff<<CFG_VGA_VSYNC_OFFSET_SHIFT)
#define CFG_VGA_VSYNC_OFFSET_PIXELS_SHIFT 0
#define CFG_VGA_VSYNC_OFFSET_CHARS_SHIFT  3
#define CFG_VGA_VSYNC_OFFSET_HXTRA_SHIFT  6
#define CFG_HOTPLUG_SHIFT					9
#define CFG_HOTPLUG_TRISTATE				(0x0<<CFG_HOTPLUG_SHIFT)
#define CFG_HOTPLUG_DRIVE0					(0x2<<CFG_HOTPLUG_SHIFT)
#define CFG_HOTPLUG_DRIVE1					(0x3<<CFG_HOTPLUG_SHIFT)
#define CFG_HOTPLUG_PIN                 BIT(11)
#define CFG_AA_LFB_RD_SLV_WAIT			BIT(12)

// taken from h3asm.h
#define	STATUS		0x00000000
#define	PCIINIT0	0x00000004
#define	SIPMONITOR	0x00000008
#define	LFBMEMORYCONFIG	0x0000000c
#define	MISCINIT0	0x00000010
#define	MISCINIT1	0x00000014
#define	DRAMINIT0	0x00000018
#define	DRAMINIT1	0x0000001c

#define	AGPINIT	0x00000020
#define	TMUGBEINIT	0x00000024
#define	VGAINIT0	0x00000028
#define	VGAINIT1	0x0000002c
#define	DRAMCOMMAND	0x00000030
#define	DRAMDATA	0x00000034
#define	RESERVEDZ_0	0x00000038
#define	RESERVEDZ_1	0x0000003c

#define	PLLCTRL0	0x00000040
#define	PLLCTRL1	0x00000044
#define	PLLCTRL2	0x00000048
#define	DACMODE		0x0000004c
#define	DACADDR		0x00000050
#define	DACDATA		0x00000054
#define	VIDMAXRGBDELTA	0x00000058
#define	VIDPROCCFG	0x0000005c

#define	HWCURPATADDR	0x00000060
#define	HWCURLOC	0x00000064
#define	HWCURC0		0x00000068
#define	HWCURC1		0x0000006c
#define	VIDINFORMAT	0x00000070
#define	VIDINSTATUS	0x00000074
#define	VIDSERIALPARALLELPORT	0x00000078
#define	VIDINXDECIMDELTAS	0x0000007c

#define	VIDINDECIMINITERRS	0x00000080
#define	VIDINYDECIMDELTAS	0x00000084
#define	VIDPIXELBUFTHOLD	0x00000088
#define	VIDCHROMAMIN		0x0000008c
#define	VIDCHROMAMAX		0x00000090
#define	VIDCURRENTLINE		0x00000094
#define	VIDSCREENSIZE		0x00000098
#define	VIDOVERLAYSTARTCOORDS	0x0000009c

#define	VIDOVERLAYENDSCREENCOORD	0x000000a0
#define	VIDOVERLAYDUDX			0x000000a4
#define	VIDOVERLAYDUDXOFFSETSRCWIDTH	0x000000a8
#define	VIDOVERLAYDVDY			0x000000ac
#define	VGAREGISTER_0	0x000000b0
#define	VGAREGISTER_1	0x000000b4
#define	VGAREGISTER_2	0x000000b8
#define	VGAREGISTER_3	0x000000bc

#define	VGAREGISTER_4	0x000000c0
#define	VGAREGISTER_5	0x000000c4
#define	VGAREGISTER_6	0x000000c8
#define	VGAREGISTER_7	0x000000cc
#define	VGAREGISTER_8	0x000000d0
#define	VGAREGISTER_9	0x000000d4
#define	VGAREGISTER_10	0x000000d8
#define	VGAREGISTER_11	0x000000dc

#define	VIDOVERLAYDVDYOFFSET	0x000000e0
#define	VIDDESKTOPSTARTADDR	0x000000e4
#define	VIDDESKTOPOVERLAYSTRIDE	0x000000e8
#define	VIDINADDR0		0x000000ec
#define	VIDINADDR1		0x000000f0
#define	VIDINADDR2		0x000000f4
#define	VIDINSTRIDE		0x000000f8
#define	VIDCURROVERLAYSTARTADDR	0x000000fc

#define	SIZEOF_SSTIO	0x00000100
//Taken from h3defs.h
#define SST_AA_CLK_INVERT				BIT(20)
#define SST_AA_CLK_DELAY_SHIFT		21	
#define SST_AA_CLK_DELAY			(0xF<<SST_AA_CLK_DELAY_SHIFT)



// 3D Stuff
// SLICTL_3D_CTRL
#define SLICTRL (0x20C)
#define AACTRL (0x210)
#define SLICTL_3D_RENDERMASK_SHIFT	0
#define SLICTL_3D_RENDERMASK			(0xFF<<SLICTL_3D_RENDERMASK_SHIFT)
#define SLICTL_3D_COMPAREMASK_SHIFT	8
#define SLICTL_3D_COMPAREMASK			(0xFF<<SLICTL_3D_COMPAREMASK_SHIFT)
#define SLICTL_3D_SCANMASK_SHIFT		16	
#define SLICTL_3D_SCANMASK				(0xFF<<SLICTL_3D_SCANMASK_SHIFT)
#define SLICTL_3D_NUMCHIPS_LOG2_SHIFT 24
#define SLICTL_3D_NUMCHIPS_LOG2		 (0x3<<SLICTL_3D_NUMCHIPS_LOG2_SHIFT)
#define SLICTL_3D_EN				BIT(26)


//////////////////////////////////


// GLOBALDATA Information that we need to save when we enter SLI/AA mode
typedef struct _conf_save {
    ULONG               fifoStart;
    ULONG               fifoSize;
    DWORD               deBitsOffset;
} CONF_SAVE, * PCONF_SAVE;

// #ifdef SLI_AA
#if 0	// This is duplicated in fxioctl.h
typedef struct _sli_aa_chipinfo {
   DWORD dwChips;       // dwChips: Number of chips in multi-chip configuration (1-4)
   DWORD dwsliEn;       // dwsliEn: Sli is to be enabled (0,1)
   DWORD dwaaEn;        // dwaaEn: Anti-aliasing is to be enabled (0,1)
   DWORD dwaaSampleHigh;  // dwaaSampleHigh: 0->Enable 2-sample AA, 1->Enable 4-sample AA
   DWORD dwsliAaAnalog;   // dwsliAaAnalog: 0->Enable digital SLI/AA, 1->Enable analog Sli/AA
   DWORD dwsli_nlines;    // dwsli_nLines: Number of lines owned by each chip in SLI (2-128)
   DWORD dwCfgSwapAlgorithm; // Swap Buffer Algorithm
}  SLI_AA_CHIPINFO, * PSLI_AA_CHIPINFO;

typedef struct _sli_aa_meminfo {
   DWORD dwTotalMemory;       // In MB
   DWORD dwTileMark;          // In MB
   DWORD dwTileCmpMark;       // In MB
   DWORD dwaaSecondaryColorBufBegin;    // In MB
   DWORD dwaaSecondaryDepthBufBegin;    // In MB
   DWORD dwaaSecondaryDepthBufEnd;    // In MB
}  SLI_AA_MEMINFO, * PSLI_AA_MEMINFO;

typedef struct sli_aa_request {
   SLI_AA_CHIPINFO ChipInfo;
   SLI_AA_MEMINFO MemInfo;
} SLI_AA_REQUEST, * PSLI_AA_REQUEST;

#endif
// #endif

// #define UPDATE_IMASK (1)
// #define AGP_WARMUP (2)
// #define GET_DIB_ADDR (3)
// #define VIDEO_SYS_BUF (4)
// #ifdef V3TV
// #define VDD_V3TV_WDMSCALE (5)
// #endif
// #ifdef SLI_AA
#define SLI_AA_ENABLE (6)
#define SLI_AA_DISABLE (7)
// #endif
// #define VDD_EXTRN_PLD (8)

#define USE_INTERNAL_BUS_DATA_FUNCTIONS   1

#if USE_INTERNAL_BUS_DATA_FUNCTIONS
extern ULONG
H3HalSetBusDataByOffset
(
  BUS_DATA_TYPE BusDataType,
  ULONG BusNumber,
  ULONG SlotNumber,
  PVOID Buffer,
  ULONG Offset,
  ULONG Length
);

extern ULONG
H3HalGetBusDataByOffset
(
  BUS_DATA_TYPE BusDataType,
  ULONG BusNumber,
  ULONG SlotNumber,
  PVOID Buffer,
  ULONG Offset,
  ULONG Length
);

#define HAL_GET_BUS_DATA_BY_OFFSET  H3HalGetBusDataByOffset
#define HAL_SET_BUS_DATA_BY_OFFSET  H3HalSetBusDataByOffset
#else
// HalSetBusDataByOffset & HalGetBusDataByOffset are from nt5ddk\inc\ddk\ntddk.h
extern ULONG
HalSetBusDataByOffset
(
  BUS_DATA_TYPE BusDataType,
  ULONG BusNumber,
  ULONG SlotNumber,
  PVOID Buffer,
  ULONG Offset,
  ULONG Length
);

extern ULONG
HalGetBusDataByOffset
(
  BUS_DATA_TYPE BusDataType,
  ULONG BusNumber,
  ULONG SlotNumber,
  PVOID Buffer,
  ULONG Offset,
  ULONG Length
);

#define HAL_GET_BUS_DATA_BY_OFFSET  HalGetBusDataByOffset
#define HAL_SET_BUS_DATA_BY_OFFSET  HalSetBusDataByOffset
#endif

// These functions ignore the error code returned from Hal[GS]etBusDataByOffset
// hope they never fail!
static __inline VOID
PCI_Write_Config(ULONG busNumber, ULONG slotNumber, ULONG startreg, ULONG data)
{
  HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                             busNumber,
                             slotNumber,
                             &data,
                             startreg,
                             sizeof(data));
}

static __inline ULONG
PCI_Read_Config(ULONG busNumber, ULONG slotNumber, ULONG startreg)
{
  ULONG	data;

  HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                             busNumber,
                             slotNumber,
                             &data,
                             startreg,
                             sizeof(data));

  return data;
}

// These two macros actually do memory mapped reads and writes to membase0 init regs
#define PCI_IO_WR(addr, data)         (*(DWORD *)(addr) = (data))
#define PCI_IO_RD(addr)               (*(DWORD *)(addr))

// These two macros do reads and writes to pci configuration space thru the
// Hal[GS]etBusDataByOffset functions
#define PCI_CFG_WR(reg, value, chip)  PCI_Write_Config(HwDeviceExtension->BusNumber, \
                                                       HwDeviceExtension->sliSlotNumber[(chip)].u.AsULONG, \
                                                       (reg), \
                                                       (value))
#define PCI_CFG_RD(reg, chip)         PCI_Read_Config(HwDeviceExtension->BusNumber, \
                                                      HwDeviceExtension->sliSlotNumber[(chip)].u.AsULONG, \
                                                      (reg))

#ifndef PCI_ADDRESS_IO_ADDRESS_MASK
#define PCI_ADDRESS_IO_ADDRESS_MASK         0xfffffffc
#endif

#ifndef PCI_ADDRESS_MEMORY_ADDRESS_MASK
#define PCI_ADDRESS_MEMORY_ADDRESS_MASK     0xfffffff0
#endif

#endif // SLI_AA
#endif // _SLI_H_



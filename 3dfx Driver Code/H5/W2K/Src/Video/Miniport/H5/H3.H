/*++
**
** Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
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


Module Name:

	h3.h

Abstract:

	This module contains the definitions for the H3 miniport driver.

Environment:

	Kernel mode

Revision History

--*/
#include "3dfx.h"
#include "ds_i2c.h"
#include "dfpdisp.h"
#include "ddcdefs.h"

//
// define an assert macro for debugging
//
#if DBG
#define RIP(x)  { VideoDebugPrint((0, x)); ASSERT(FALSE); }
#define H3_ASSERT(x, y) if (!(x)) RIP(y)
#else
#define H3_ASSERT(x, y)
#endif

//
// Meta definitions
//
#define BIT(n)						(1 << n)

//
// These definitions will affect code paths not currently supported for the
// banshee driver, but may eventually be in the code. We leave them
// in the code to help remind us what isn't in there yet...
//
#define VARIANTS_EXIST					0

#define GTF_TIMINGS 1
#define DMT_ENABLED 1

#if defined ( GTF_TIMINGS ) || ( DMT_ENABLED )

#include "dfp.h"
#include <modeprm.h>

#define EDID_BLOCK_MAX_SIZE 128

// DMT stuff begins

DECLARE_HANDLE(HKEY);

#ifndef HKEY_LOCAL_MACHINE	// to avoid compilation warnings
#define HKEY_CLASSES_ROOT		0x80000000
#define HKEY_CURRENT_USER		0x80000001
#define HKEY_LOCAL_MACHINE		0x80000002
#define HKEY_USERS			0x80000003
#define HKEY_PERFORMANCE_DATA		0x80000004
#define HKEY_CURRENT_CONFIG		0x80000005
#define HKEY_DYN_DATA		0x80000006
#endif		// ifndef HKEY_LOCAL_MACHINE

#ifndef REG_SZ		// to avoid compilation warnings
#define REG_NONE                    ( 0 )   // No value type
#define REG_SZ                      ( 1 )   // Unicode nul terminated string
#define REG_EXPAND_SZ               ( 2 )   // Unicode nul terminated string
                                            // (with environment variable references)
#define REG_BINARY                  ( 3 )   // Free form binary
#define REG_DWORD                   ( 4 )   // 32-bit number
#define REG_DWORD_LITTLE_ENDIAN     ( 4 )   // 32-bit number (same as REG_DWORD)
#define REG_DWORD_BIG_ENDIAN        ( 5 )   // 32-bit number
#define REG_LINK                    ( 6 )   // Symbolic Link (unicode)
#define REG_MULTI_SZ                ( 7 )   // Multiple Unicode strings
#define REG_RESOURCE_LIST           ( 8 )   // Resource list in the resource map
#define REG_FULL_RESOURCE_DESCRIPTOR ( 9 )  // Resource list in the hardware description
#define REG_RESOURCE_REQUIREMENTS_LIST ( 10 )
#define REG_QWORD                   ( 11 )  // 64-bit number
#define REG_QWORD_LITTLE_ENDIAN     ( 11 )  // 64-bit number (same as REG_QWORD)
#endif	// #ifndef REG_SZ

// DMT stuff ends

#endif // ( GTF_TIMINGS ) || ( DMT_ENABLED )

//
// Set these flags for debugging and other useful operations,
// including optional capabilities like more modes and suchlike.
//
#define BIOS_VBE_BROKEN                 1
#define DEBUG_MODES                     0
#define DEBUG_REGISTRY                  0
#define DEBUG_SHARING                   0 // for debugging IOCTL_VIDEO_(UN)SHARE_VIDEO_MEMORY
#define VIDEO_720_MODES_SUPPORTED       0
#define ALLOW_ENHANCED_MODES            0
#define ALL_POSSIBLE_MODES_SUPPORTED    0
#define H3_POWER_MANAGEMENT_ENABLED     1 // power management always on
#define SHARE_KERNEL_MAPS_WITH_DISPLAY  1
#define _MAP_FRAMEBUFFER_ONCE           1 // only effective when SHARE_KERNEL_MAPS_WITH_DISPLAY is enabled
#define REDUCED_MEMORY_MAPPINGS         1 // needs to be in sync with display driver's driver.h setting
#define ENABLE_ADDRESS_LIST_ARRAY       0
#define PCI_LATENCY_REQUIRED_TO_PASS    0

//
// Debugging information used in conjunction with the registry
// variable MiniportDebugLevel. If the debug bit corresponding
// to the area is on, then information and errors will be reported.
// This gives us more latitude than levels 0..3 as is found in
// the VideoPort driver.
//
#define DBG_OFF 				0
#define DBG_FAILURES_ONLY		BIT(0)
#define DBG_REPORT_ENTRIES		BIT(1)
#define DBG_UNUSED_00			BIT(2)
#define DBG_UNUSED_01			BIT(3)
#define DBG_UNUSED_02			BIT(4)
#define DBG_UNUSED_03			BIT(5)
#define DBG_UNUSED_04			BIT(6)
#define DBG_UNUSED_05			BIT(7)
#define DBG_PCI_DETECTION		BIT(8)
#define DBG_MAPPINGS			BIT(9)
#define DBG_CHIPS				BIT(10)
#define DBG_REGISTRY			BIT(11)
#define DBG_MODE_DATA			BIT(12)
#define DBG_INITIALIZATION		BIT(13)
#define DBG_MODESET_CODE		BIT(14)
#define DBG_MODESET_MICROSCOPE	BIT(15)
#define DBG_IOCTL_SERVICE_L0	BIT(16)
#define DBG_IOCTL_SERVICE_L1	BIT(17)
#define DBG_IOCTL_SERVICE_ALL	BIT(18)
#define DBG_BIOS_INFO			BIT(19)
#define DBG_UNUSED_07			BIT(20)
#define DBG_UNUSED_08			BIT(21)
#define DBG_UNUSED_09			BIT(22)
#define DBG_UNUSED_10			BIT(23)
#define DBG_UNUSED_11			BIT(24)
#define DBG_GAMMA_STUFF			BIT(25)
#define DBG_UNUSED_13			BIT(26)
#define DBG_UNUSED_14			BIT(27)
#define DBG_UNUSED_15			BIT(28)
#define DBG_UNUSED_16			BIT(29)
#define DBG_UNUSED_17			BIT(30)
#define DBG_UNUSED_18			BIT(31)
#define DBG_KITCHEN_SINK		0xffffffff

//
// If the compile-time options are not set, install them here ...
//
#ifndef MEMORY_CLOCK
#define MEMORY_CLOCK		110
#endif

#ifndef GRAPHICS_CLOCK
#define GRAPHICS_CLOCK		100
#endif

//
// We keep a default clock for Banshee Plus and Avenger separate
#ifndef BANSHEEPLUS_CLOCK
#define BANSHEEPLUS_CLOCK	143
#endif

#ifndef AVENGER_CLOCK
#define AVENGER_CLOCK		143
#endif

//
// Constants dealing with the PLL clocks
//
// These are used to fence clocking so we don't try values that will hurt us
//

#define NO_CLOCK				0xffffffff	// used for invalid entries in the clock tables
#define H3_CLOCK_MINIMUM		49			// no slower
#define H3_CLOCK_MAXIMUM		170 		// no faster
#define H3_DEFAULT_GRX_CLOCK	100
#define H3_DEFAULT_MEM_CLOCK	115
#define MIN_H3_PLL_FREQ			50
#define MAX_H3_PLL_FREQ			170

#define H4_CLOCK_MINIMUM		30			// no slower
#define H4_CLOCK_MAXIMUM		220 		// no faster
#define H4_DEFAULT_CLOCK		141			// Only one clock domain!
#define MIN_H4_PLL_FREQ			30
#define MAX_H4_PLL_FREQ			220

#define H5_CLOCK_MINIMUM		50			// no slower
#define H5_CLOCK_MAXIMUM		220 		// no faster

//
// Number of access ranges used by Banshee
//
#define NUM_H3_ACCESS_RANGES		3		// used to be 4, but we don't call VPGAR with BIOS anymore
#define NUM_H3_PCI_ACCESS_RANGES	1
#define H3_WINDOW_SIZE				0x40000000
#define H3_MULTIPLE_CHIPSET_MASK	0x04000000
#define H3_16MEG_TYPE_MASK			0x08000000

//
// Preset values needed for Banshee init without a BIOS
//
// Constants without SGRAM or SDRAM in the name are appropriate for both memory types
//
#define MISCINIT0_DEFAULT			0x00000000
#define DEFAULT_PCIINIT0			0x0180F840	// Set default for PCI.  Or in WS later for AGP.
#define DEFAULT_MISCINIT0			0x00000000
#define DEFAULT_AGPINIT0			0x0000049E
#define DEFAULT_PLLCTRL1			0x00002805	// 100 MHz MemClock
#define DEFAULT_PLLCTRL2			0x00002805	// 100 MHz MemClock
#define DEFAULT_SGRAMMODE			0x00000037
#define DEFAULT_SGRAM_NOSDRAM		0xffffffff

#define SDRAM_DEFAULT_MISCINIT1		0x00008001
#define SDRAM_DEFAULT_DRAMINIT0		0x0016A165
#define SDRAM_DEFAULT_DRAMINIT1		0x0023C031

#define SGRAM_DEFAULT_MISCINIT1		0x00000001
#define SGRAM_DEFAULT_DRAMINIT0		0x0017A16A
#define SGRAM_DEFAULT_DRAMINIT1		0x0023C031

//
// Init registers
//

//
// DDC and power management settings
//
#if (_WIN32_WINNT >= 0x500)
#define VESA_POWER_FUNCTION 		0x4f10
#define VESA_POWER_ON				0x0000
#define VESA_POWER_STANDBY			0x0100
#define VESA_POWER_SUSPEND			0x0200
#define VESA_POWER_OFF				0x0400
#define VESA_GET_POWER_FUNC 		0x0000
#define VESA_SET_POWER_FUNC 		0x0001
#define VESA_STATUS_SUCCESS 		0x004f

#define QUERY_MONITOR_ID			0x22446688
#define QUERY_NONDDC_MONITOR_ID 	0x11223344
#define QUERY_DFP_ID			    0x22446699
#define QUERY_V3TV_ID				0x33446688
#define QUERY_TVOUT_ID				0x44668899

#endif // (_WIN32_WINNT >= 0x500)

//
// PCI config space stuff
//
#define PCI_VENDOR_ID			0x00
#define PCI_DEVICE_ID			0x02
#define PCI_COMMAND 			0x04
#define PCI_STATUS				0x06
#define PCI_REVISION_ID 		0x08
#define PCI_CACHE_LINE_SIZE 	0x0c
#define PCI_BASE_ADDRESS_0		0x10
#define PCI_BASE_ADDRESS_1		0x14
#define PCI_BASE_ADDRESS_2		0x18
#define PCI_BASE_ADDRESS_3		0x1c
#define PCI_BASE_ADDRESS_4		0x20
#define PCI_BASE_ADDRESS_5		0x14

//
// PCI Command register bits
//
#define PCI_CMD_IOACCESS_ENABLE				BIT(0)
#define PCI_CMD_MEMACCESS_ENABLE			BIT(1)
#define PCI_CMD_MASTER_ENABLE				BIT(2)
#define PCI_CMD_SPECIAL_CYCLE_RECOGNITION	BIT(3)
#define PCI_CMD_MEMWRITE_INVALIDATE_ENABLE	BIT(4)
#define PCI_CMD_PALETTE_SNOOP_ENABLE		BIT(5)
#define PCI_CMD_PARITY_ERROR_RESP_ENABLE	BIT(6)
#define PCI_CMD_WAIT_CYCLE_ENABLE			BIT(7)
#define PCI_CMD_SYSTEM_ERROR_ENABLE			BIT(8)
#define PCI_CMD_MAPPABLE_ACCESS_RANGES		(PCI_CMD_IOACCESS_ENABLE | PCI_CMD_MEMACCESS_ENABLE)

#define SST_AGP_MAJOR_REV			0x00f00000
#define SST_AGP_MAJOR_SHIFT 		20
#define SST_AGP_MINOR_REV			0x000f0000
#define SST_AGP_MINOR_SHIFT 		16
#define SST_AGP_CAPABILITIES		0xff
#define SST_AGP_NEW_CAPABILITIES	BIT(4)
#define SST_AGP_66MHZ_CAPABLE		BIT(5)
#define SST_AGP_CAPABLE				SST_AGP_66MHZ_CAPABLE

#define SST_AGP_CAPABILITIES_SHIFT	0

#define TOP_OF_PCI_MEMORY			0xFFFFFFFF

#define TDFX_VENDOR_ID        0x121a
#define TDFX_VOODOO_ID        0x0001
#define TDFX_VOODOO2_ID       0x0002
#define TDFX_BANSHEE_ID       0x0003
#define TDFX_BANSHEE_PLUS_ID  0x0004
#define TDFX_AVENGER_ID       0x0005
#define TDFX_NAPALM_ID        0x0007
#define TDFX_NAPALM2_ID       0x000B

#define IS_VOODOO3  (0x05 >= HwDeviceExtension->PCIDeviceID)
#define IS_NAPALM   (0x06 <= HwDeviceExtension->PCIDeviceID)

//
// status
//
#define SST_FIFOLEVEL			 0x3F
#define SST_PCIFIFO_FREE		 0x1F
#define SST_PCIFIFO_BUSY		 BIT(5)
#define SST_VRETRACE			 BIT(6)
#define SST_FBI_BUSY			 BIT(7)
#define SST_TMU_BUSY			 BIT(8)
#define SST_TREX_BUSY			 SST_TMU_BUSY
#define SST_BUSY				 BIT(9)
#define SST_GUI_BUSY			 BIT(10)
#define SST_CMD0_BUSY			 BIT(11)
#define SST_CMD1_BUSY			 BIT(12)
#define SST_SWAPBUFPENDING_SHIFT 28
#define SST_SWAPBUFPENDING		 (0x7 << SST_SWAPBUFPENDING_SHIFT)
#define SST_PCIINTERRUPTED		 BIT(31)
//
// pciInit0
//
#define DEFAULT_PCI_LOWTHRESH_VAL         0x8
#define DEFAULT_2CHIP_PCI_LOWTHRESH_VAL   0xA
#define SST_PCI_LOWTHRESH_SHIFT           2
#define SST_PCI_LOWTHRESH                 (0x1F << SST_PCI_LOWTHRESH_SHIFT)
#define SST_PCI_READ_WS                   BIT(8)
#define SST_PCI_WRITE_WS                  BIT(9)
#define SST_PCI_DISABLE_IO                BIT(11)
#define SST_PCI_DISABLE_MEM               BIT(12)
#define SST_PCI_RETRY_INTERVAL_SHIFT      13
#define SST_PCI_RETRY_INTERVAL            (0x1F << SST_PCI_RETRY_INTERVAL_SHIFT)
#define SST_PCI_INTERRUPT_ENABLE          BIT(18)
#define SST_PCI_TIMEOUT_ENABLE            BIT(19)
#define SST_PCI_FORCE_FB_HIGH             BIT(26)

//
// sipMonitor
//

//
// lfbMemoryConfig
//
#define SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT	0
#define SST_RAW_LFB_TILE_BEGIN_PAGE 		(0x1FFF << SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT)
#define SST_RAW_LFB_ADDR_STRIDE_SHIFT		13
#define SST_RAW_LFB_ADDR_STRIDE 			(0x7 << SST_RAW_LFB_ADDR_STRIDE_SHIFT)
#define SST_RAW_LFB_ADDR_STRIDE_1K			(0x0 << SST_RAW_LFB_ADDR_STRIDE_SHIFT)
#define SST_RAW_LFB_ADDR_STRIDE_2K			(0x1 << SST_RAW_LFB_ADDR_STRIDE_SHIFT)
#define SST_RAW_LFB_ADDR_STRIDE_4K			(0x2 << SST_RAW_LFB_ADDR_STRIDE_SHIFT)
#define SST_RAW_LFB_ADDR_STRIDE_8K			(0x3 << SST_RAW_LFB_ADDR_STRIDE_SHIFT)
#define SST_RAW_LFB_ADDR_STRIDE_16K 		(0x4 << SST_RAW_LFB_ADDR_STRIDE_SHIFT)
#define SST_RAW_LFB_ADDR_STRIDE_MAX 		(0x4 << SST_RAW_LFB_ADDR_STRIDE_SHIFT)
#define SST_RAW_LFB_TILE_STRIDE_SHIFT		16
#define SST_RAW_LFB_TILE_STRIDE 			(0x7F << SST_RAW_LFB_TILE_STRIDE_SHIFT)
#define SST_RAW_LFB_ADDR_SHIFT				0
#define SST_RAW_LFB_ADDR					(0x1FFFFFF << SST_RAW_LFB_ADDR_SHIFT)
#define SST_RAW_LFB_CONFIGURATION			0x00001fff

//
// miscInit0
//
#define SST_GRX_RESET					BIT(0)
#define SST_FBI_FIFO_RESET				BIT(1)
#define SST_REGISTER_BYTE_SWIZZLE_EN	BIT(2)
#define SST_REGISTER_WORD_SWIZZLE_EN	BIT(3)
#define SST_VIDEO_RESET 				BIT(4)
#define SST_2D_RESET					BIT(5)
#define SST_MEMORY_TIMING_RESET 		BIT(6)
#define SST_YORIGIN_TOP_SHIFT			(18)
#define SST_YORIGIN_TOP 				(0xFFF << SST_YORIGIN_TOP_SHIFT)
#define SST_RAWLFB_BYTE_SWIZZLE_EN		BIT(15)
#define SST_RAWLFB_WORD_SWIZZLE_EN		BIT(16)

//
// miscInit1
//
#define SST_H3_CLUTINVERT				BIT(0)
#define SST_ALT_REGMAPPING				BIT(5)
#define SST_DISABLE_TEXTURE 			BIT(6)
#define SST_POWERDOWN_CLUT				BIT(7)
#define SST_POWERDOWN_DAC				BIT(8)
#define SST_POWERDOWN_VID_PLL			BIT(9)
#define SST_POWERDOWN_GRX_PLL			BIT(10)
#define SST_POWERDOWN_MEM_PLL			BIT(11)
#define SST_BLOCK_WRITE_THRESH_SHIFT	12
#define SST_BLOCK_WRITE_THRESH			(BIT(12) | BIT(13) | BIT(14))
#define SST_DISABLE_2D_BLOCK_WRITE		BIT(15)
#define SST_DISABLE_3D_BLOCK_WRITE		BIT(16)
#define SST_DISABLE_2D_STALL_ON_3D		BIT(17)
#define SST_DISABLE_3D_STALL_ON_2D		BIT(18)
#define SST_DEFAULT_MASK_MISCINIT1		0x1FF40000
#define SST_MISCINIT1_STRAP_MASK      0x1F000000

//
// dramInit0
//
#define SST_SGRAM_RRD_SHIFT				0
#define SST_SGRAM_RRD					(0x3 << SST_SGRAM_RRD_SHIFT)
#define SST_SGRAM_RCD_SHIFT				2
#define SST_SGRAM_RCD					(0x3 << SST_SGRAM_RCD_SHIFT)
#define SST_SGRAM_RP_SHIFT				4
#define SST_SGRAM_RP					(0x3 << SST_SGRAM_RP_SHIFT)
#define SST_SGRAM_RAS_SHIFT				6
#define SST_SGRAM_RAS					(0xF << SST_SGRAM_RAS_SHIFT)
#define SST_SGRAM_RC_SHIFT				10
#define SST_SGRAM_RC					(0xF << SST_SGRAM_RC_SHIFT)
#define SST_SGRAM_CAS_SHIFT				14
#define SST_SGRAM_CAS					(0x3 << SST_SGRAM_CAS_SHIFT)
#define SST_SGRAM_MRS_SHIFT				16
#define SST_SGRAM_MRS					BIT(16)
#define SST_SGRAM_RD2DQM				BIT(17)
#define SST_SGRAM_BWC					BIT(18)
#define SST_SGRAM_WL					BIT(19)
#define SST_SGRAM_BWL_SHIFT				20
#define SST_SGRAM_BWL					(0x3 << SST_SGRAM_BWL_SHIFT)
#define SST_SGRAM_RL					BIT(22)
#define SST_SGRAM_USE_BST				BIT(23)
#define SST_SGRAM_NO_DEAD				BIT(24)
#define SST_SGRAM_EN_WPB				BIT(25)
#define SST_SGRAM_NUM_CHIPSETS			BIT(26)
#define SST_SGRAM_TYPE_SHIFT			27
//#ifdef H5
#define SST_SGRAM_TYPE_H5               (0x7L<<SST_SGRAM_TYPE_SHIFT)
#define SST_SGRAM_TYPE_H4               (0x1 << SST_SGRAM_TYPE_SHIFT)
#       define SST_SGRAM_TYPE_8MBIT     (0x0L<<SST_SGRAM_TYPE_SHIFT)
#       define SST_SGRAM_TYPE_16MBIT    (0x1L<<SST_SGRAM_TYPE_SHIFT)
#       define SST_SGRAM_TYPE_32MBIT    (0x2L<<SST_SGRAM_TYPE_SHIFT)
#       define SST_SGRAM_TYPE_64MBIT    (0x3L<<SST_SGRAM_TYPE_SHIFT)
#       define SST_SGRAM_TYPE_128MBIT   (0x4L<<SST_SGRAM_TYPE_SHIFT)
#define SST_MCTL_DRAM_NUMBANKS          BIT(30)
//#else
//#define SST_SGRAM_TYPE					(0x1 << SST_SGRAM_TYPE_SHIFT)
//#define SST_SGRAM_TYPE_8MBIT			(0x0 << SST_SGRAM_TYPE_SHIFT)
//#define SST_SGRAM_TYPE_16MBIT			(0x1 << SST_SGRAM_TYPE_SHIFT)
//#endif
#define SST_DRAMINIT0_STRAP_MASK		0xfc000000

//
// dramInit1
//
#define SST_DRAM_REFRESH_EN 			BIT(0)
#define SST_DRAM_REFRESH_VALUE_SHIFT	1
#define SST_DRAM_REFRESH_VALUE			(0x1FF << SST_DRAM_REFRESH_VALUE_SHIFT)
#define SST_VIDEO_OVERRIDE_EN			BIT(10)
#define SST_TRIPLE_BUFFER_EN			BIT(11)
#define SST_DITHER_PASSTHROUGH			BIT(12)
#define SST_SGRAM_CLK_NODELAY			BIT(13)
#define SST_SGRAM_USE_INV_SAMPLE		BIT(14)
#define SST_SGRAM_DEL_CLK_INVERT		BIT(15)
#define SST_SGRAM_CLK_ADJ_SHIFT 		16
#define SST_SGRAM_CLK_ADJ				(0xF << SST_SGRAM_CLK_ADJ_SHIFT)
#define SST_SGRAM_OFLOP_DEL_ADJ_SHIFT	23
#define SST_SGRAM_OFLOP_DEL_ADJ 		(0xF << SST_SGRAM_OFLOP_DEL_ADJ_SHIFT)
#define SST_SGRAM_OFLOP_TRANS_LATCH 	BIT(24)
#define SST_MCTL_SHORT_POWER_ON 		BIT(25)
#define SST_MCTL_NO_AGGRESSIVE			BIT(26)
#define SST_MCTL_PAGEBREAK				BIT(27)
#define SST_MCTL_TRISTATE_OUTPUTS		BIT(28)
#define SST_MCTL_NO_VIN_LOCKING 		BIT(29)
#define SST_MCTL_TYPE_SDRAM 			BIT(30)
#define SST_DRAMINIT1_STRAP_MASK		0xc2000000

//
// agpInit
//
#define SST_AGP_LTOH					BIT(0)
#define SST_AGP_RQSIZE_SHIFT			BIT(1)
#define SST_AGP_RQSIZE					(0x7 << SST_AGP_RQSIZE_SHIFT)
#define SST_AGP_REQFULL_THRESH_SHIFT	4
#define SST_AGP_REQFULL_THRESH			(0x7 << SST_AGP_REQFULL_THRESH_SHIFT)
#define SST_AGP_READFULL_THRESH_SHIFT	7
#define SST_AGP_READFULL_THRESH 		(0xF << SST_AGP_READFULL_THRESH_SHIFT)

//
// tmuGbeInit
//
#define SST_TGI_READLOW_THRESH_SHIFT	0
#define SST_TGI_READLOW_THRESH			(0xF << SST_TGI_READLOW_THRESH_SHIFT)
#define SST_TGI_READHIGH_THRESH_SHIFT	4
#define SST_TGI_READHIGH_THRESH 		(0xF << SST_TGI_READHIGH_THRESH_SHIFT)
#define SST_TGI_PIXELHIGH_THRESH_SHIFT	8
#define SST_TGI_PIXELHIGH_THRESH		(0xF << SST_TGI_PIXELHIGH_THRESH_SHIFT)

//
// PLL and DAC registers
//
#define SST_DACMODE_2X					BIT(0)
#define SST_DACMODE_HOLD_VSYNC			BIT(1)
#define SST_DACMODE_FORCE_VSYNC_1		BIT(2)
#define SST_DACMODE_HOLD_HSYNC			BIT(3)
#define SST_DACMODE_FORCE_HSYNC_1		BIT(4)
#define SST_DACMODE_DPMS_BITS			(SST_DACMODE_HOLD_HSYNC | SST_DACMODE_HOLD_VSYNC)
#define SST_DACMODE_DPMS_ON 			(0)
#define SST_DACMODE_DPMS_STANDBY		(SST_DACMODE_HOLD_HSYNC)
#define SST_DACMODE_DPMS_SUSPEND		(SST_DACMODE_HOLD_VSYNC)
#define SST_DACMODE_DPMS_OFF			(SST_DACMODE_HOLD_HSYNC | SST_DACMODE_HOLD_VSYNC)

#define SST_DAC_DPMS_ON_VSYNC     BIT(1)
#define SST_DAC_DPMS_ON_HSYNC     BIT(3)

//
// vidProcCfg
//
#define SST_VIDEO_PROCESSOR_EN			BIT(0)
#define SST_CURSOR_MODE_SHIFT			1
#define SST_CURSOR_MODE 				(1 << SST_CURSOR_MODE_SHIFT)
#define SST_CURSOR_X11					(1 << SST_CURSOR_MODE_SHIFT)
#define SST_CURSOR_MICROSOFT			(0 << SST_CURSOR_MODE_SHIFT)
#define SST_OVERLAY_STEREO_EN			BIT(2)
#define SST_INTERLACED_EN				BIT(3)
#define SST_HALF_MODE					BIT(4)
#define SST_CHROMA_EN					BIT(5)
#define SST_CHROMA_INVERT				BIT(6)
#define SST_DESKTOP_EN					BIT(7)
#define SST_OVERLAY_EN					BIT(8)
#define SST_VIDEOIN_AS_OVERLAY			BIT(9)
#define SST_DESKTOP_CLUT_BYPASS 		BIT(10)
#define SST_OVERLAY_CLUT_BYPASS 		BIT(11)
#define SST_DESKTOP_CLUT_SELECT 		BIT(12)
#define SST_OVERLAY_CLUT_SELECT 		BIT(13)
#define SST_OVERLAY_HORIZ_SCALE_EN		BIT(14)
#define SST_OVERLAY_VERT_SCALE_EN		BIT(15)
#define SST_OVERLAY_FILTER_MODE_SHIFT	16
#define SST_OVERLAY_FILTER_MODE 		(3 << SST_OVERLAY_FILTER_MODE_SHIFT)
#define SST_OVERLAY_FILTER_POINT		(0 << SST_OVERLAY_FILTER_MODE_SHIFT)
#define SST_OVERLAY_FILTER_2X2			(1 << SST_OVERLAY_FILTER_MODE_SHIFT)
#define SST_OVERLAY_FILTER_4X4			(2 << SST_OVERLAY_FILTER_MODE_SHIFT)
#define SST_OVERLAY_FILTER_BILINEAR 	(3 << SST_OVERLAY_FILTER_MODE_SHIFT)
#define SST_DESKTOP_PIXEL_FORMAT_SHIFT	18
#define SST_DESKTOP_BPP_MASK			(BIT(18) | BIT(19) | BIT(20))
#define SST_DESKTOP_PIXEL_FORMAT		(7 << SST_DESKTOP_PIXEL_FORMAT_SHIFT)
#define SST_DESKTOP_PIXEL_PAL8			(0 << SST_DESKTOP_PIXEL_FORMAT_SHIFT)
#define SST_DESKTOP_PIXEL_RGB565		(1 << SST_DESKTOP_PIXEL_FORMAT_SHIFT)
#define SST_DESKTOP_PIXEL_RGB24 		(2 << SST_DESKTOP_PIXEL_FORMAT_SHIFT)
#define SST_DESKTOP_PIXEL_RGB32 		(3 << SST_DESKTOP_PIXEL_FORMAT_SHIFT)
#define SST_OVERLAY_PIXEL_FORMAT_SHIFT	21
#define SST_OVERLAY_PIXEL_FORMAT		(BIT(21) | BIT(22) | BIT(23))
#define SST_OVERLAY_PIXEL_RGB565U		(1 << SST_OVERLAY_PIXEL_FORMAT_SHIFT)
#define SST_OVERLAY_PIXEL_YUV411		(4 << SST_OVERLAY_PIXEL_FORMAT_SHIFT)
#define SST_OVERLAY_PIXEL_YUYV422		(5 << SST_OVERLAY_PIXEL_FORMAT_SHIFT)
#define SST_OVERLAY_PIXEL_UYVY422		(6 << SST_OVERLAY_PIXEL_FORMAT_SHIFT)
#define SST_OVERLAY_PIXEL_RGB565D		(7 << SST_OVERLAY_PIXEL_FORMAT_SHIFT)
#define SST_DESKTOP_TILED_EN			BIT(24)
#define SST_OVERLAY_TILED_EN			BIT(25)
#define SST_VIDEO_2X_MODE_EN			BIT(26)
#define SST_CURSOR_EN					BIT(27)
#define SST_OVERLAY_EACH_VSYNC			BIT(29)
#define SST_OVERLAY_STRIDE_ADJUST		BIT(30)
#define SST_OVERLAY_DEINTERLACE_EN		BIT(31)

#define SST_NO_CLUT_BYPASS				(~(SST_DESKTOP_CLUT_BYPASS | SST_OVERLAY_CLUT_BYPASS))
#define SST_BYPASS_CLUT 				(SST_DESKTOP_CLUT_BYPASS | SST_OVERLAY_CLUT_BYPASS)

#define SST_CURSOR_X_SHIFT					  0
#define SST_CURSOR_X						  (0x7FF<<SST_CURSOR_X_SHIFT)
#define SST_CURSOR_Y_SHIFT					  16
#define SST_CURSOR_Y						  (0x7FF<<SST_CURSOR_Y_SHIFT)

#define SST_CURSOR_BLUE_SHIFT				  0
#define SST_CURSOR_BLUE 					  (0xFF<<SST_CURSOR_BLUE_SHIFT)
#define SST_CURSOR_RED_SHIFT				  8
#define SST_CURSOR_RED						  (0xFF<<SST_CURSOR_RED_SHIFT)
#define SST_CURSOR_GREEN_SHIFT				  16
#define SST_CURSOR_GREEN					  (0xFF<<SST_CURSOR_GREEN_SHIFT)

#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
//-------------------- vidInFormat
#define SST_VIDEOIN_PIXEL_FORMAT_SHIFT        1
#define SST_VIDEOIN_PIXEL_FORMAT              (7<<SST_VIDEOIN_PIXEL_FORMAT_SHIFT)
#       define SST_VIDEOIN_PIXEL_YUV411       (4<<SST_VIDEOIN_PIXEL_FORMAT_SHIFT)
#       define SST_VIDEOIN_PIXEL_YUYV422      (5<<SST_VIDEOIN_PIXEL_FORMAT_SHIFT)
#       define SST_VIDEOIN_PIXEL_UYUV422      (6<<SST_VIDEOIN_PIXEL_FORMAT_SHIFT)
#define SST_VIDEOIN_DEINTERLACE_EN            BIT(4)
#define SST_VIDEOIN_VSYNC_POLARITY_LOW        BIT(5)
#define SST_VIDEOIN_HSYNC_POLARITY_LOW        BIT(6)
#define SST_VIDEOIN_VACTIVE_POLARITY_LOW      BIT(7)
#define SST_VIDEOIN_G4_FOR_POSEDGE            BIT(8)
#define SST_VIDEOIN_BUFFERING_MODE_SHIFT      9
#define SST_VIDEOIN_BUFFERING_MODE            (3<<SST_VIDEOIN_BUFFERING_MODE_SHIFT)
#       define SST_VIDEOIN_SINGLE_BUFFERING   (0<<SST_VIDEOIN_BUFFERING_MODE_SHIFT)
#       define SST_VIDEOIN_DOUBLE_BUFFERING   (1<<SST_VIDEOIN_BUFFERING_MODE_SHIFT)
#       define SST_VIDEOIN_TRIPLE_BUFFERING   (2<<SST_VIDEOIN_BUFFERING_MODE_SHIFT)
#define SST_VIDEOIN_TILED_EN                  BIT(11)
//#ifdef H4
#define SST_TVOUT_VSYNC_POLARITY_LOW	      BIT(12)
#define SST_TVOUT_HSYNC_POLARITY_LOW	      BIT(13)
//#endif
#define SST_VIDEOIN_INTERFACE_SHIFT           14
#define SST_VIDEOIN_INTERFACE                 (3<<SST_VIDEOIN_INTERFACE_SHIFT)
#       define SST_VIDEOIN_INTERFACE_OFF      (0<<SST_VIDEOIN_INTERFACE_SHIFT)
#       define SST_VIDEOIN_VMI_ENABLE         (1<<SST_VIDEOIN_INTERFACE_SHIFT)
#       define SST_VIDEOIN_TVOUT_ENABLE       (2<<SST_VIDEOIN_INTERFACE_SHIFT)
#define SST_VIDEOIN_GENLOCK_ENABLE            BIT(16)
#define SST_VIDEOIN_NOT_USE_VGA_TIMING        BIT(17)
//#ifdef H4
#define SST_VIDEOIN_GENLOCK_SOURCE_SHIFT      18
#define SST_VIDEOIN_GENLOCK_SOURCE            (1<<SST_VIDEOIN_GENLOCK_SOURCE_SHIFT)
#define SST_VIDEOIN_GENLOCK_SOURCE_TV         (1<<SST_VIDEOIN_GENLOCK_SOURCE_SHIFT)
#define SST_VIDEOIN_GENLOCK_SOURCE_VMI        (0<<SST_VIDEOIN_GENLOCK_SOURCE_SHIFT)
//#endif
#define SST_VIDEOIN_DATA_CAPTURE_EN           BIT(19)
#define SST_VIDEOIN_HORIZ_DECIM_EN            BIT(20)
#define SST_VIDEOIN_VERT_DECIM_EN             BIT(21)
#define SST_VIDEOIN_TV_DATA_SCRAMBLE_DISABLE  BIT(22)

//-------------------- vidSerialParallelPort
#define SST_SERPAR_VMI_EN			BIT(0)
#define SST_SERPAR_VMI_CS_N			BIT(1)
#define SST_SERPAR_VMI_DS_N			BIT(2)
#define SST_SERPAR_VMI_RD_N			BIT(2)
#define SST_SERPAR_VMI_RW_N			BIT(3)
#define SST_SERPAR_VMI_DTACK_N			BIT(4)
#define SST_SERPAR_VMI_RDY			BIT(4)
#define SST_SERPAR_VMI_DATA_OE_N		BIT(5)
#define SST_SERPAR_VMI_DATA_SHIFT		6
#define SST_SERPAR_VMI_DATA			(0xFF<<SST_SERPAR_VMI_DATA_SHIFT)
#define SST_SERPAR_VMI_ADDR_SHIFT		14
#define SST_SERPAR_VMI_ADDR			(0x7<<SST_SERPAR_VMI_ADDR_SHIFT)
#define SST_SERPAR_TVOUT_GPIO_0			BIT(2)
#define SST_SERPAR_TVOUT_GPIO_0_OE_N		BIT(3)
#define SST_SERPAR_TVOUT_GPIO_2			BIT(14)
#define SST_SERPAR_TVOUT_GPIO_2_OE_N		BIT(15)
#define SST_SERPAR_TVOUT_GPIO_1			BIT(16)
#define SST_SERPAR_TVOUT_GPIO_1_OE_N		BIT(17)
#define SST_SERPAR_DDC_EN			BIT(18)
#define SST_SERPAR_DDC_DCK_OUT			BIT(19)
#define SST_SERPAR_DDC_DDA_OUT			BIT(20)
#define SST_SERPAR_DDC_DCK_IN			BIT(21)
#define SST_SERPAR_DDC_DDA_IN			BIT(22)
#define SST_SERPAR_I2C_EN			BIT(23)
#define SST_SERPAR_I2C_SCK_OUT			BIT(24)
#define SST_SERPAR_I2C_DSA_OUT          BIT(25)
#define SST_SERPAR_I2C_SCK_IN			BIT(26)
#define SST_SERPAR_I2C_DSA_IN           BIT(27)
#define SST_SERPAR_VMI_RESET_N			BIT(28)
#define SST_SERPAR_RESET_N			BIT(28)
#define SST_SERPAR_GPIO_1			BIT(29)
#define SST_SERPAR_GPIO_2			BIT(30)
#define SST_SERPAR_TVOUTRESET_N                 BIT(31)

#else

#define SST_VIDEOIN_PIXEL_FORMAT_SHIFT		  1
#define SST_VIDEOIN_PIXEL_FORMAT			  (7<<SST_VIDEOIN_FORMAT_SHIFT)
#		define SST_VIDEOIN_PIXEL_YUV411 	  (4<<SST_VIDEOIN_FORMAT_SHIFT)
#		define SST_VIDEOIN_PIXEL_YUYV422	  (5<<SST_VIDEOIN_FORMAT_SHIFT)
#		define SST_VIDEOIN_PIXEL_UYUV422	  (6<<SST_VIDEOIN_FORMAT_SHIFT)
#define SST_VIDEOIN_DEINTERLACE_EN			  BIT(4)
#define SST_VIDEOIN_VSYNC_POLARITY			  BIT(5)
#define SST_VIDEOIN_HSYNC_POLARITY			  BIT(6)
#define SST_VIDEOIN_VACTIVE_POLARITY		  BIT(7)
#define SST_VIDEOIN_G4_FOR_POSEDGE			  BIT(8)
#define SST_VIDEOIN_BUFFERING_MODE_SHIFT	  9
#define SST_VIDEOIN_BUFFERING_MODE			  (3<<SST_VIDEOIN_BUFFER_MODE_SHIFT)
#		define SST_VIDEOIN_SINGLE_BUFFERING   (0<<SST_VIDEOIN_BUFFER_MODE_SHIFT)
#		define SST_VIDEOIN_DOUBLE_BUFFERING   (1<<SST_VIDEOIN_BUFFER_MODE_SHIFT)
#		define SST_VIDEOIN_TRIPLE_BUFFERING   (2<<SST_VIDEOIN_BUFFER_MODE_SHIFT)
#define SST_VIDEOIN_TILED_EN				  BIT(11)
#define SST_VIDEOIN_INTERFACE_SHIFT 		  14
#define SST_VIDEOIN_INTERFACE				  (7<<SST_VIDEO_INTERFACE_CONFIG_SHIFT)
#		define SST_VIDEOIN_INTERFACE_OFF	  (0<<SST_VIDEO_INTERFACE_CONFIG_SHIFT)
#		define SST_VIDEOIN_INTERFACE_VMI	  (1<<SST_VIDEO_INTERFACE_CONFIG_SHIFT)
#		define SST_VIDEOIN_INTERFACE_GENLOCK  (2<<SST_VIDEO_INTERFACE_CONFIG_SHIFT)
#		define SST_VIDEOIN_INTERFACE_VIDEOOUT (3<<SST_VIDEO_INTERFACE_CONFIG_SHIFT)
#define SST_VIDEOIN_TIMING_POLARITY 		  BIT(17)
#define SST_VIDEOIN_FIELD_DETECTION 		  BIT(18)
#define SST_VIDEOIN_DATA_CAPTURE_EN 		  BIT(19)
#define SST_VIDEOIN_HORIZ_DECIM_EN			  BIT(20)
#define SST_VIDEOIN_VERT_DECIM_EN			  BIT(21)

#define SST_VIDEOIN_LAST_FIELD				  BIT(1)
#define SST_VIDEOIN_LAST_BUFFER_SHIFT		  1
#define SST_VIDEOIN_LAST_BUFFER 			  (3<<SST_VIDEOIN_LAST_BUFFER_SHIFT)

// skipping vidSerialParallelPort register defines

#define SST_VIDEOIN_DECIM_POS_DELTA_SHIFT	  0
#define SST_VIDEOIN_DECIM_POS_DELTA 		  (0xFFF<<SST_VIDEOIN_DECIM_POS_DELTA_SHIFT)
#define SST_VIDEOIN_DECIM_NEG_DELTA_SHIFT	  16
#define SST_VIDEOIN_DECIM_NEG_DELTA 		  (0xFFF<<SST_VIDEOIN_DECIM_NEG_DELTA_SHIFT)

#define SST_VIDEOIN_X_DECIM_ERROR_SHIFT 	  0
#define SST_VIDEOIN_X_DECIM_ERROR			  (0x1FFF<<SST_VIDEOIN_X_DECIM_ERROR_SHIFT)
#define SST_VIDEOIN_Y_DECIM_ERROR_SHIFT 	  16
#define SST_VIDEOIN_Y_DECIM_ERROR			  (0x1FFF<<SST_VIDEOIN_Y_DECIM_ERROR_SHIFT)

#endif //def TVOUT_SUPPORTED

// skipping vidPixelBufThold
// skipping vidChromaKeyMin/Max
// skipping vidCurrentLine

//-------------vidScreenSize

#define SST_VIDEO_SCREEN_WIDTH_SHIFT		  0
#define SST_VIDEO_SCREEN_WIDTH				  (0xFFF<<SST_VIDEO_SCREEN_WIDTH_SHIFT)
#define SST_VIDEO_SCREEN_HEIGHT_SHIFT		  12
#define SST_VIDEO_SCREEN_HEIGHT 			  (0xFFF<<SST_VIDEO_SCREEN_HEIGHT_SHIFT)

//-------------vidOverlayStartCoords, vidOverlayEndCoords
#define SST_OVERLAY_X_SHIFT 				  0
#define SST_OVERLAY_X						  (0xFFF<<SST_OVERLAY_X_SHIFT)
#define SST_OVERLAY_Y_SHIFT 				  12
#define SST_OVERLAY_Y						  (0xFFF<<SST_OVERLAY_Y_SHIFT)
#define SST_OVERLAY_XADJ_SHIFT				  24
#define SST_OVERLAY_XADJ					  (3<<SST_OVERLAY_XADJ_SHIFT)
#define SST_OVERLAY_YADJ_SHIFT				  26
#define SST_OVERLAY_YADJ					  (3<<SST_OVERLAY_YADJ_SHIFT)

//------------vidOverlayDudxOffsetSrcWidth, vidOverlayDvDy
#define SST_OVERLAY_DELTA					  SST_MASK(20)
#define SST_OVERLAY_DELTA_OFFSET			  SST_MASK(19)
#define SST_OVERLAY_FETCH_SIZE_SHIFT		  19
#define SST_OVERLAY_FETCH_SIZE				  (0x1FFF<<SST_OVERLAY_FETCH_SIZE_SHIFT)

#define SST_VIDEO_START_ADDR_SHIFT			  0
#define SST_VIDEO_START_ADDR				  (0xFFFFFF<<SST_VIDEO_START_ADDR_SHIFT)
#define SST_DESKTOP_STRIDE_SHIFT			  0
#define SST_DESKTOP_LINEAR_STRIDE			  (0x3FFF<<SST_DESKTOP_STRIDE_SHIFT)
#define SST_DESKTOP_TILE_STRIDE 			  (0x7F<<SST_DESKTOP_STRIDE_SHIFT)
#define SST_OVERLAY_STRIDE_SHIFT			  16
#define SST_OVERLAY_LINEAR_STRIDE			  (0x3FFF<<SST_OVERLAY_STRIDE_SHIFT)
#define SST_OVERLAY_TILE_STRIDE 			  (0x7F<<SST_OVERLAY_STRIDE_SHIFT)

//------------vidInStride
#define SST_VIDEO_IN_STRIDE_SHIFT			  0
#define SST_VIDEO_IN_LINEAR_STRIDE			  (0x3FFF<<SST_VIDEO_IN_STRIDE_SHIFT)
#define SST_VIDEO_IN_TILE_STRIDE			  (0x7F<<SST_VIDEO_IN_STRIDE_SHIFT)

//----------------- SST *Init*	----------------------------

// vgaInit0
#define SST_VGA0_DISABLE				BIT(0)
#define SST_VGA0_EXTERNAL_TIMING		BIT(1)
#define SST_VGA0_CLUT_SELECT_SHIFT		2
#define SST_VGA0_CLUT_SELECT			(1 << SST_CLUT_SELECT_SHIFT)
#		define SST_CLUT_SELECT_6BIT 	0
#		define SST_CLUT_SELECT_8BIT 	1
#define SST_VGA0_EXTENSIONS 			BIT(6)
#define SST_VGA0_WAKEUP_SELECT_SHIFT	8
#define SST_VGA0_WAKEUP_SELECT			(1 << SST_VGA0_WAKEUP_SELECT_SHIFT)
#		define SST_WAKEUP_46e8			0
#		define SST_WAKEUP_3C3			1
#define SST_VGA0_LEGACY_DECODE_SHIFT	9
#define SST_VGA0_LEGACY_DECODE			(1 << SST_VGA0_LEGACY_DECODE_SHIFT)
#		define SST_VGA0_ENABLE_DECODE	0
#		define SST_VGA0_DISABLE_DECODE	1
#define SST_VGA0_CONFIG_READBACK_SHIFT	10
#define SST_VGA0_CONFIG_READBACK		(1 << SST_VGA0_CONFIG_READBACK_SHIFT)
#		define SST_ENABLE_ALT_READBACK	0
#		define SST_DISABLE_ALT_READBACK 1


// fill in the reset of vgaInit0 sometime
#define SST_VGA0_VGA_BASE_ADDR_SHIFT	14
#define SST_VGA0_VGA_BASE_ADDR			(0xff << SST_VGA0_VGA_BASE_ADDR_SHIFT)

// vgaInit1
// fill this in some day, please

// vga legacy definitions!
#define SST_VGA_SEQ_CLKMODE_SCRN_OFF	BIT(5)

//----------------- fastfillCMD bits --------------------------
#define SST_FASTFILL_DISABLE_DITHER 	BIT(0)

//----------------- nopCMD bits -------------------------------
#define SST_NOP_RESET_PIXEL_STATS		BIT(0)
#define SST_NOP_RESET_TRIANGLE_STATS	BIT(1)

//----------------- swapBufferCMD bits ------------------------
#define SST_SWAP_EN_WAIT_ON_VSYNC		BIT(0)
#define SST_SWAP_BUFFER_INTERVAL_SHIFT	1
#define SST_SWAP_BUFFER_INTERVAL		(0xff << SST_SWAP_BUFFER_INTERVAL_SHIFT)
#define SST_SWAP_DONT_SWAP				BIT(9)

//----------------- SST Configuration Registers ---------------
#define SST_MEMBASE_ADDR				0x10
#define SST_INIT_ENABLE 				0x40
#define SST_BUS_SNOOP0					0x44
#define SST_BUS_SNOOP1					0x48
#define SST_CFG_STATUS					0x4c

//----------------- SST Remapped Registers --------------------
#define SST_DAC_RD_DATA 				0x00000218
#define SST_VIDEO_CHECKSUM				0x0000021c

//----------------- SST initEnable (Config register) bits -----
#define SST_INITWR_EN					BIT(0)
#define SST_PCI_FIFOWR_EN				BIT(1)
#define SST_FBIINIT23_REMAP 			BIT(2)
#define SST_SNOOP0_EN					BIT(4)
#define SST_SNOOP0_IO					BIT(5)
#define SST_SNOOP0_RD					BIT(6)
#define SST_SNOOP1_EN					BIT(7)
#define SST_SNOOP1_IO					BIT(8)
#define SST_SNOOP1_RD					BIT(9)
#define SST_SCANLINE_SLV_OWNPCI 		BIT(10)
#define SST_SCANLINE_SLI_SLV			BIT(11)

//----------------- SST vRetrace bits ---------------------
#define SST_VRETRACE_VSYNC_OFF	0xff

//----------------- SST hSync bits ------------------------
#define SST_HSYNC_ON			0xff
#define SST_HSYNC_OFF_SHIFT 	16
#define SST_HSYNC_OFF			(0x3ff << SST_HSYNCOFF_SHIFT)

//----------------- SST vSync bits ------------------------
#define SST_VSYNC_ON			0xff
#define SST_VSYNC_OFF_SHIFT 	16
#define SST_VSYNC_OFF			(0xfff << SST_VSYNC_OFF_SHIFT)

//----------------- SST backPorch bits --------------------
#define SST_HBACKPORCH			0xff
#define SST_VBACKPORCH_SHIFT	16
#define SST_VBACKPORCH			(0xff << SST_VBACKPORCH_SHIFT)

//----------------- SST videoDimensionS Register -----------
#define SST_VIDEODIMENSIONS_X	0x3ff
#define SST_VIDEODIMENSIONS_Y_SHIFT 16
#define SST_VIDEODIMENSIONS_Y	0x3ff


// Here are the defines for the alternate register mappings
#define r_ALT			r
#define drdx_ALT		g
#define drdy_ALT		b
#define g_ALT			z
#define dgdx_ALT		a
#define dgdy_ALT		s
#define b_ALT			t
#define dbdx_ALT		w

#define dbdy_ALT		drdx
#define z_ALT			dgdx
#define dzdx_ALT		dbdx
#define dzdy_ALT		dzdx
#define a_ALT			dadx
#define dadx_ALT		dsdx
#define dady_ALT		dtdx
#define s_ALT			dwdx

#define dsdx_ALT		drdy
#define dsdy_ALT		dgdy
#define t_ALT			dbdy
#define dtdx_ALT		dzdy
#define dtdy_ALT		dady
#define w_ALT			dsdy
#define dwdx_ALT		dtdy
#define dwdy_ALT		dwdy

#define Fr_ALT			Fr
#define Fdrdx_ALT		Fg
#define Fdrdy_ALT		Fb
#define Fg_ALT			Fz
#define Fdgdx_ALT		Fa
#define Fdgdy_ALT		Fs
#define Fb_ALT			Ft
#define Fdbdx_ALT		Fw

#define Fdbdy_ALT		Fdrdx
#define Fz_ALT			Fdgdx
#define Fdzdx_ALT		Fdbdx
#define Fdzdy_ALT		Fdzdx
#define Fa_ALT			Fdadx
#define Fdadx_ALT		Fdsdx
#define Fdady_ALT		Fdtdx
#define Fs_ALT			Fdwdx

#define Fdsdx_ALT		Fdrdy
#define Fdsdy_ALT		Fdgdy
#define Ft_ALT			Fdbdy
#define Fdtdx_ALT		Fdzdy
#define Fdtdy_ALT		Fdady
#define Fw_ALT			Fdsdy
#define Fdwdx_ALT		Fdtdy
#define Fdwdy_ALT		Fdwdy


//
// Index of Frame buffer in access range array
//
#define MEMBASE_ZERO			0
#define MEMBASE_ONE 			1
#define IOBASE_ZERO 			2
#define ROMBASE 				3
#define LINEAR_FRAME_BUF		4

////////////////////////////////////////////////////////////////////////
// Capabilities flags
//
// These are private flags passed to the H3 display driver.  They're
// put in the high word of the 'AttributeFlags' field of the
// 'VIDEO_MODE_INFORMATION' structure (found in 'ntddvdeo.h') passed
// to the display driver via an 'VIDEO_QUERY_AVAIL_MODES' or
// 'VIDEO_QUERY_CURRENT_MODE' IOCTL.
//
// NOTE: These definitions must match those in the display driver's
//		 'driver.h'!

typedef enum {
	CAPS_SW_POINTER 		= 0x00000001L,	// No hardware pointer; use software pointer instead
	CAPS_AGP_FIFO			= 0x00000002L,	// Use AGP command fifo
	CAPS_VIDEO_ONLY 		= 0x40000000L,	// Used for video-only resolutions
	CAPS_SCAN_LINE_DOUBLED	= 0x80000000L	// Use scan line doubling
} CAPS;

//
// BIOS Section
//
#define BIOS_REMAP_LOCATION				0x00a00000		// map after TMU 1 on V3 in membase0

#ifndef _NTDDK_
typedef enum _MM_SYSTEM_SIZE {
    MmSmallSystem,
    MmMediumSystem,
    MmLargeSystem
} MM_SYSTEMSIZE;
#endif

//
// Strings associated with identifying a banshee, b+, or avenger
//
#define MAX_BIOS_VERSION_LENGTH		(40)
#define BIOS_STRING_1				(0x73726556)
#define BIOS_STRING_2				(0x206E6F69)
#define ROM_LENGTH					(0x8000)
#define TDFX_ROM_SIGNATURE			"3dfx Interactive, Inc"
#define TDFX_ROM_ORIGINAL_VERSION	"VGA BIOS - Version 1.00.01"
#define TDFX_ROM_EARLY_VERSION		L"VGA BIOS - Version 1.00.01-ET"
#define SECONDARY_DEVICE_NO_ROM		L"SECONDARY DEVICE -- NO ROM ALLOWED"
#define NO_ROM_STRING				L"NO ROM DETECTED"
#define DEFAULT_DAC_STRING			L"3dfx Internal DAC"

#define DEFAULT_ADAPTER_STRING		L"3dfx Voodoo Banshee Reference Board"
#define DEFAULT_ADAPTER_TYPE		L"3dfx Voodoo Banshee Reference Board"
#define DEFAULT_CHIP_STRING 		L"3dfx Voodoo Banshee"

#define BPDEFAULT_ADAPTER_STRING	L"3dfx Voodoo Banshee Plus Reference Board"
#define BPDEFAULT_ADAPTER_TYPE		L"3dfx Voodoo Banshee Plus Reference Board"
#define BPDEFAULT_CHIP_STRING 		L"3dfx Voodoo Banshee Plus"

#ifdef H4VEL
#define H4DEFAULT_ADAPTER_STRING	L"3dfx Velocity Reference Board"
#define H4DEFAULT_ADAPTER_TYPE		L"3dfx Velocity Reference Board"
#define H4DEFAULT_CHIP_STRING 		L"3dfx Velocity"
#else
#define H4DEFAULT_ADAPTER_STRING	L"3dfx Voodoo 3 Reference Board"
#define H4DEFAULT_ADAPTER_TYPE		L"3dfx Voodoo 3 Reference Board"
#define H4DEFAULT_CHIP_STRING 		L"3dfx Voodoo 3"
#endif

#ifdef H4VEL
#define H5DEFAULT_ADAPTER_STRING	L"3dfx Velocity"
#define H5DEFAULT_ADAPTER_TYPE		L"3dfx Velocity"
#define H5DEFAULT_CHIP_STRING 		L"3dfx Velocity"
#else
#define H5DEFAULT_ADAPTER_STRING	L"3dfx VSA Series"
#define H5DEFAULT_ADAPTER_TYPE		L"3dfx VSA Series"
#define H5DEFAULT_CHIP_STRING 		L"3dfx VSA Series"
#endif

//
// Supported board definitions.
//

typedef enum _TDFX_BOARDS {
	TDFX_GENERIC_VOODOO_BANSHEE = 0,
	TDFX_VOODOO_BANSHEE,
	TDFX_VOODOO_BANSHEE_PLUS,
	TDFX_VOODOO_3,
	TDFX_MAX_BOARD
} TDFX_BOARDS;

//
// Chip type definitions -- for families of chips
//
// if you change this typedef it will change the size of the second element
// (named Fixed) of the union in the typedef for S3_VIDEO_FREQUENCIES and
// PS3_VIDEO_FREQUENCIES, look at that typedef for a caution about the effect
// this will have on autoinitialization
//

typedef enum _TDFX_CHIPSETS {
	TDFX_BANSHEE = 0,
	TDFX_MAX_CHIPSET
} TDFX_CHIPSET;

//
// Hardware pointer capabilities flags
//

typedef enum _POINTER_CAPABILITY {
	POINTER_BUILT_IN			= 0x01, // A pointer is built in to the hardware
	POINTER_WORKS_ONLY_AT_8BPP	= 0x02, // If set, the hardware pointer works
										//	 only at 8bpp, and only for modes
										//	 1024x768 or less
	POINTER_NEEDS_SCALING		= 0x04, // x-coordinate must be scaled by 2 at
										//	 32bpp
} POINTER_CAPABILITY;

#ifdef CUST_INF_MODERATES
// STBNW JAC 1-8-99
//
// WARNING: The data table in h3modes.c *MUST* be in the same order!!
//
#define MAXNUM_REFRESHRATES_PERMODE	20
typedef struct
{
	PWSTR	regStr;				// The string in the registry we will search for, then get its values (rates).
	ULONG	bpp;
	ULONG	hres;
	ULONG	vres;
	BOOLEAN	bFoundInReg;		// TRUE if mode was found in registry and had valid data.
	BOOLEAN bExcludeRates;		// If TRUE then the rates specify which rates to EXCLUDE; not which rates are valid
	ULONG	refreshRates[MAXNUM_REFRESHRATES_PERMODE];		// e.g. 43, 48, 56, 60, 65, 70, 72, ...
} MODERATES;

extern MODERATES modeRates[];

#endif // CUST_INF_MODERATES

//
// If we have an invalid refresh rate, always use 60 Hz as the
// default frequency
//
#define DEFAULT_REFRESH_RATE		60

//
// Characteristics of each mode
//

typedef struct _H3_VIDEO_MODES {

	USHORT Int10ModeNumberContiguous;
	USHORT Int10ModeNumberNoncontiguous;
	ULONG ScreenStrideContiguous;

	VIDEO_MODE_INFORMATION ModeInformation;

} H3_VIDEO_MODES, *PH3_VIDEO_MODES;

//
// Mode-set specific information.
//

typedef struct _H3_VIDEO_FREQUENCIES {

	ULONG BitsPerPel;
	ULONG ScreenWidth;
	ULONG ScreenHeight;
	ULONG ScreenFrequency;

	// the information required to set the proper refresh rate
	PUCHAR VideoData;
// this field is obsolete, centered DFP timings are generated dynamically.	PUCHAR DfpData;
	UCHAR  isValidTvOutMode;

	PH3_VIDEO_MODES ModeEntry;
	ULONG ModeIndex;
	UCHAR ModeValid;

#ifdef DMT_ENABLED
	UCHAR RegistryVideoData[CRTC_TABLE_SIZE];
	H3_VIDEO_MODES RegistryModeEntry;
#endif // DMT_ENABLED

} H3_VIDEO_FREQUENCIES, *PH3_VIDEO_FREQUENCIES;

//
// Register definitions used with VideoPortRead/Write functions
//
// It's a good idea to write your miniport to allow for easy register
// re-mapping, so we define the offsets for the ports and registers,
// store them in an array and create macro definitions for direct use.
// If we have to move things around (or need to support other offsets),
// the only thing that requires changes are the definitions of the
// original offsets, or at worst the addition of more defs and entries.
//

#define CRTC_ADDRESS_REG_OFFSET 		0x3b4
#define CRTC_DATA_REG_OFFSET			0x3b5
#define SEQUENCER_ADDRESS_REG_OFFSET	0x3c4
#define SEQUENCER_DATA_REG_OFFSET		0x3c5
#define RAMDAC_PIXEL_MASK_OFFSET		0x3c6
#define RAMDAC_READ_INDEX_REG_OFFSET	0x3c7
#define RAMDAC_WRITE_INDEX_REG_OFFSET	0x3c8
#define RAMDAC_DATA_REG_OFFSET			0x3c9
#define GCR_ADDRESS_REG_OFFSET			0x3ce
#define GCR_DATA_REG_OFFSET 			0x3cf

//
// IOBase0 offsets
//
#define STATUS						0x00000000
#define PCIINIT0					0x00000004
#define SIPMONITOR					0x00000008
#define LFBMEMORYCONFIG 			0x0000000c
#define MISCINIT0					0x00000010
#define MISCINIT1					0x00000014
#define DRAMINIT0					0x00000018
#define DRAMINIT1					0x0000001c
#define AGPINIT 					0x00000020
#define TMUGBEINIT					0x00000024
#define VGAINIT0					0x00000028
#define VGAINIT1					0x0000002c
#define DRAMCOMMAND 				0x00000030
#define DRAMDATA					0x00000034
#define RESERVEDZ_0 				0x00000038
#define RESERVEDZ_1 				0x0000003c
#define PLLCTRL0					0x00000040
#define PLLCTRL1					0x00000044
#define PLLCTRL2					0x00000048
#define DACMODE 					0x0000004c
#define DACADDR 					0x00000050
#define DACDATA 					0x00000054
#define VIDMAXRGBDELTA				0x00000058
#define VIDPROCCFG					0x0000005c
#define HWCURPATADDR				0x00000060
#define HWCURLOC					0x00000064
#define HWCURC0 					0x00000068
#define HWCURC1 					0x0000006c
#define VIDINFORMAT 				0x00000070
#define VIDINSTATUS 				0x00000074
#define VIDSERIALPARALLELPORT		0x00000078
#define VIDINXDECIMDELTAS			0x0000007c
#define VIDINDECIMINITERRS			0x00000080
#define VIDINYDECIMDELTAS			0x00000084
#define VIDPIXELBUFTHOLD			0x00000088
#define VIDCHROMAMIN				0x0000008c
#define VIDCHROMAMAX				0x00000090
#define VIDCURRENTLINE				0x00000094
#define VIDSCREENSIZE				0x00000098
#define VIDOVERLAYSTARTCOORDS		0x0000009c
#define VIDOVERLAYENDCOORDS 		0x000000a0
#define VIDOVERLAYDUDX				0x000000a4
#define VIDOVERLAYDUDXOFFSETSRCWIDTH	0x000000a8
#define VIDOVERLAYDVDY				0x000000ac
#define VGAREGISTER_0				0x000000b0
#define VGAREGISTER_1				0x000000b4
#define VGAREGISTER_2				0x000000b8
#define VGAREGISTER_3				0x000000bc
#define VGAREGISTER_4				0x000000c0
#define VGAREGISTER_5				0x000000c4
#define VGAREGISTER_6				0x000000c8
#define VGAREGISTER_7				0x000000cc
#define VGAREGISTER_8				0x000000d0
#define VGAREGISTER_9				0x000000d4
#define VGAREGISTER_10				0x000000d8
#define VGAREGISTER_11				0x000000dc
#define VIDOVERLAYDVDYOFFSET		0x000000e0
#define VIDDESKTOPSTARTADDR 		0x000000e4
#define VIDDESKTOPOVERLAYSTRIDE 	0x000000e8
#define VIDINADDR0					0x000000ec
#define VIDINADDR1					0x000000f0
#define VIDINADDR2					0x000000f4
#define VIDINSTRIDE 				0x000000f8
#define VIDCURROVERLAYSTARTADDR 	0x000000fc

//
// Mem-mapped IO to Membase0 -- here's the structure you
// overlay the registers with!
//
typedef volatile struct _membase0 {
	ULONG status;
	ULONG pciInit0;
	ULONG sipMonitor;
	ULONG lfbMemoryConfig;
	ULONG miscInit0;
	ULONG miscInit1;
	ULONG dramInit0;
	ULONG dramInit1;
	ULONG agpInit;
	ULONG tmuGbeInit;
	ULONG vgaInit0;
	ULONG vgaInit1;
	ULONG dramCommand;
	ULONG dramData;
	ULONG _reserved00;
#ifdef TVOUT_SUPPORTED
	ULONG vidTvOutBlankVCount;
#else
	ULONG _reserved01;
#endif
	ULONG pllCtrl0;
	ULONG pllCtrl1;
	ULONG pllCtrl2;
	ULONG dacMode;
	ULONG dacAddr;
	ULONG dacData;
	ULONG vidMaxRGBDelta;
	ULONG vidProcCfg;
	ULONG hwCurPatAddr;
	ULONG hwCurLoc;
	ULONG hwCurC0;
	ULONG hwCurC1;
	ULONG vidInFormat;
#ifdef TVOUT_SUPPORTED
	ULONG vidTvOutBlankHCount;
#else
	ULONG vidInStatus;
#endif
	ULONG vidSerialParallelPort;
	ULONG vidInXDecimDeltas;
	ULONG vidInDecimInitErrs;
	ULONG vidInYDecimDeltas;
	ULONG vidPixelBufThold;
	ULONG vidChromaMin;
	ULONG vidChromaMax;
	ULONG vidCurrentLine;
	ULONG vidScreenSize;
	ULONG vidOverlayStartCoords;
	ULONG vidOverlayEndScreenCoords;
	ULONG vidOverlayDudx;
	ULONG vidOverlayDudxOffsetSrcWidth;
	ULONG vidOverlayDvdy;
	ULONG vgaRegister[12];
	ULONG vidOverlayDvdyOffset;
	ULONG vidDesktopStartAddr;
	ULONG vidDesktopOverlayStride;
	ULONG vidInAddr0;
	ULONG vidInAddr1;
	ULONG vidInAddr2;
	ULONG vidInStride;
	ULONG vidCurrOverlayStartAddr;
} H3_MEMBASE0, *PH3_MEMBASE0;

typedef H3_MEMBASE0 SstIORegs;

typedef volatile struct cmdfifo
{
  ULONG baseAddrL;
  ULONG baseSize;
  ULONG bump;
  ULONG readPtrL;
  ULONG readPtrH;
  ULONG aMin;
  ULONG unusedA;
  ULONG aMax;
  ULONG unusedB;
  ULONG depth;
  ULONG holeCount;
  ULONG reserved;
} CmdFifo;

typedef volatile struct sstcregs
{
  // AGP
  ULONG agpReqSize;
  ULONG hostAddrLow;
  ULONG hostAddrHigh;
  ULONG graphicsAddr;
  ULONG graphicsStride;
  ULONG moveCMD;
  ULONG reservedL[2];

  // CMD FIFO 0,1
  CmdFifo cmdFifo0;
  CmdFifo cmdFifo1;

  ULONG cmdFifoThresh;
  ULONG cmdHoleInit;
  ULONG reservedO[6];
  ULONG reservedP[8];
  ULONG reservedQ[8];
  ULONG reservedR[8];

  // misc
  ULONG yuvBaseAddr;
  ULONG yuvStride;
  ULONG reservedS[6];
  ULONG crc1;
  ULONG reservedT[3];
  ULONG crc2;
} SstCRegs;

//
// This is the register map for the 2D register set for Banshee
//

typedef volatile struct _h3_2d_registers {
	ULONG status;
	ULONG unused;
	ULONG clip0Min;
	ULONG clip0Max;
	ULONG dstBaseAddr;
	ULONG dstFormat;
	ULONG srcColorKeyMin;
	ULONG srcColorKeyMax;
	ULONG dstColorKeyMin;
	ULONG dstColorKeyMax;
	ULONG bresError0;
	ULONG bresError1;
	ULONG rop;
	ULONG srcBaseAddr;
	ULONG commandEx;
	ULONG lineStipple;
	ULONG lineStyle;
	ULONG pattern0alias;
	ULONG pattern1alias;
	ULONG clip1min;
	ULONG clip1max;
	ULONG srcFormat;
	ULONG srcSize;
	ULONG srcXY;
	ULONG colorBack;
	ULONG colorFore;
	ULONG dstSize;
	ULONG dstXY;
	ULONG command;
	ULONG reserved[3];
	ULONG launch[32];
	ULONG colorPattern[64];
} H3_2D_REGISTERS, *PH3_2D_REGISTERS;

typedef H3_2D_REGISTERS SstGRegs;

//
// This is the register map for the 3D register set for Banshee,
// although we should never need to muck with them...

typedef struct vertex_Rec
{
  ULONG x;      // 12.4 format
  ULONG y;      // 12.4 format
} vtxRec;

typedef volatile struct _h3_3d_registers {
	ULONG   status;
	ULONG   intrCtrl;
  vtxRec  vA;
  vtxRec  vB;
  vtxRec  vC;
	ULONG   r;
	ULONG   g;
	ULONG   b;
	ULONG   z;
	ULONG   s;
	ULONG   t;
	ULONG   a;
	ULONG   w;
	ULONG   drdx;
	ULONG   dgdx;
	ULONG   dbdx;
	ULONG   dzdx;
	ULONG   dadx;
	ULONG   dsdx;
	ULONG   dtdx;
	ULONG   dwdx;
	ULONG   drdy;
	ULONG   dgdy;
	ULONG   dbdy;
	ULONG   dzdy;
	ULONG   dady;
	ULONG   dsdy;
	ULONG   dtdy;
	ULONG   dwdy;
	ULONG   triangleCMD;
	ULONG   reservedA;
  vtxRec  FvA;
  vtxRec  FvB;
  vtxRec  FvC;
	ULONG   Fr;
	ULONG   Fg;
	ULONG   Fb;
	ULONG   Fz;
	ULONG   Fs;
	ULONG   Ft;
	ULONG   Fa;
	ULONG   Fw;
	ULONG   Fdrdx;
	ULONG   Fdgdx;
	ULONG   Fdbdx;
	ULONG   Fdzdx;
	ULONG   Fdadx;
	ULONG   Fdsdx;
	ULONG   Fdtdx;
	ULONG   Fdwdx;
	ULONG   Fdrdy;
	ULONG   Fdgdy;
	ULONG   Fdbdy;
	ULONG   Fdzdy;
	ULONG   Fdady;
	ULONG   Fdsdy;
	ULONG   Fdtdy;
	ULONG   Fdwdy;
	ULONG   FtriangleCMD;
	ULONG   fbzColorPath;
	ULONG   fogMode;
	ULONG   alphaMode;
	ULONG   fbzMode;
	ULONG   lfbMode;
	ULONG   clipLeftRight;
	ULONG   clipTopBottom;
	ULONG   nopCMD;
	ULONG   fastfillCMD;
	ULONG   swapbufferCMD;
	ULONG   fogColor;
	ULONG   zaColor;
	ULONG   chromaKey;
	ULONG   chromaRange;
	ULONG   userIntrCmd;
	ULONG   stipple;
	ULONG   c0;
	ULONG   c1;
  struct
  {
    ULONG   fbiPixelsIn;
    ULONG   fbiChromaFail;
    ULONG   fbiZfuncFail;
    ULONG   fbiAfuncFail;
    ULONG   fbiPixelsOut;
  } stats;
	ULONG   fogTable[32];
  ULONG   renderMode;
  ULONG   stencilMode;
  ULONG   stencilOp;
	ULONG   colBufferAddr;
	ULONG   colBufferStride;
	ULONG   auxBufferAddr;
	ULONG   auxBufferStride;
  ULONG   fbiStencilFail;
	ULONG   clipLeftRight1;
	ULONG   clipTopBottom1;
  ULONG   combineMode;
  ULONG   sliCtrl;
  ULONG   aaCtrl;
  ULONG   chipMask;
  ULONG   leftDesktopBuf;
	ULONG   reservedD[12];
	ULONG   swapBufferPend;
	ULONG   leftOverlayBuf;
	ULONG   rightOverlayBuf;
	ULONG   fbiSwapHistory;
	ULONG   fbiTrianglesOut;
	ULONG   sSetupMode;
	ULONG   sVx;
	ULONG   sVy;
	ULONG   sARGB;
	ULONG   sRed;
	ULONG   sGreen;
	ULONG   sBlue;
	ULONG   sAlpha;
	ULONG   sVz;
	ULONG   sOowfbi;
	ULONG   sOow0;
	ULONG   sSow0;
	ULONG   sTow0;
	ULONG   sOow1;
	ULONG   sSow1;
	ULONG   sTow1;
	ULONG   sDrawTriCMD;
	ULONG   sBeginTriCMD;
	ULONG   reservedG[22];
	ULONG   textureMode;
	ULONG   tLOD;
	ULONG   tDetail;
	ULONG   texBaseAddr;
	ULONG   texBaseAddr1;
	ULONG   texBaseAddr2;
	ULONG   texBaseAddr38;
  ULONG   trexInit0;
	ULONG   trexInit1;
	ULONG   nccTable0[12];
	ULONG   nccTable1[12];
	//ULONG _reserved_7_3d[30];
} H3_3D_REGISTERS, *PH3_3D_REGISTERS;

typedef H3_3D_REGISTERS SstRegs;

#define H5_HP_INT_ENABLE               0x04000000
#define H5_HP_INTERRUPT                0x08000000

#define SST_IO_OFFSET                   0x000000
#define SST_CMDAGP_OFFSET               0x080000
#define SST_2D_OFFSET                   0x100000
#define SST_3D_OFFSET                   0x200000
#define SST_YUV_OFFSET                  0xC00000

#ifdef SLI_AA
#define MEM_DECODE_SIZE                 128*1024*1024   // 128MB
#define IO_DECODE_SIZE                  256

#define SLI_AA_REGULAR_DEVICE           (0x00)
#define SLI_AA_MASTER_DEVICE            (0x01)
#define SLI_AA_SLAVE_DEVICE             (0x02)
#define HWINFO_SST_MAX_NUM_CHIPS        (0x04)

// index in HwDeviceExtension->sliMappedAddress[][these_defines]
#define HWINFO_SST_IOREGS_INDEX         (0x00)
#define HWINFO_SST_CMDFIFOREGS_INDEX    (0x01)
#define HWINFO_SST_2DREGS_INDEX         (0x02)
#define HWINFO_SST_3DREGS_INDEX         (0x03)
#define HWINFO_SST_FB_INDEX             (0x04)
#define HWINFO_SST_IO_INDEX             (0x05)
#define HWINFO_SST_MAX_CHIP_INDEX       (0x06)
#endif

// indices used for sparsely mapping membase0, mapping membase1 and mapping i/o regs
// used for accessing the MappedAddress array in DEVICE_EXTENSION
//
// numerically, I'm keeping MEMBASE_ZERO, MEMBASE_ONE & IOBASE_ZERO identical in case
// someone inadvertentrly uses those values when indexing into HwDeviceExtension->MappedAddress
// if someone insists on using hardcodes indices rather than this enum, that will be a problem
typedef enum
{
  SST_IO_REGS_INDEX     = 0,    // same as MEMBASE_ZERO
  SST_FB_INDEX          = 1,    // same as MEMBASE_ONE
  SST_IO_INDEX          = 2     // same as IOBASE_ZERO
#if REDUCED_MEMORY_MAPPINGS
  , SST_CMDAGP_REGS_INDEX = 3
  , SST_2D_REGS_INDEX     = 4
  , SST_3D_REGS_INDEX     = 5
  , SST_YUV_PLANAR_INDEX  = 6
  , SST_MAX_INDEX         = 7
#endif
} H3_MAPPEDADDRESS_INDICES;

// moved ioctl defines and structures to an include file that can
// be shared by the miniport and display driver
#include "fxioctl.h"

//
// We need this for the ACPI support, to denote that we don't support it. It's also in winerror.h
// Go figure.
#ifndef ERROR_CALL_NOT_IMPLEMENTED
#define ERROR_CALL_NOT_IMPLEMENTED		120L
#endif

//
// Things relating to gammas
//

#define NUM_CLUT_ENTRIES		0x100
#define MAX_CLUT_SIZE			(sizeof(VIDEO_CLUT) + (sizeof(ULONG) * (NUM_CLUT_ENTRIES + 1)))

//
// use this to communicate between DriverEntry and H3FindAdapter
//
typedef struct _entry_info {
	PVOID DriverObject;
	PVOID RegistryPath;
	PVOID HwDeviceExtension;
	ULONG BoardNumber;
	ULONG SlotDeviceNumber;
	ULONG BusNumber;
	ULONG VPConnectsInterrupt;
} DriverEntryInfo, *pDriverEntryInfo;

/* ganked from ntddk.h */

//
// Define function decoration depending on whether a driver, a file system,
// or a kernel component is being built.
//

#if (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)) && !defined (_BLDR_)
#define NTKERNELAPI DECLSPEC_IMPORT
#else
#define NTKERNELAPI
#endif

#if ENABLE_ADDRESS_LIST_ARRAY
typedef struct _EPROCESS *PEPROCESS;
NTKERNELAPI PEPROCESS
IoGetCurrentProcess(VOID);
#endif

#if (_WIN32_WINNT >= 0x500) && defined (AGP_FIFO_CODE)

#define AGP_FIFO_MAX_SIZE 4*1024*1024

// AGP info structure

typedef struct
{
	PHYSICAL_ADDRESS    physAddr;   // Physical address of AGP command fifo
	PVOID               PhysicalReserveContext;
	PVOID               virtualAddr;
	PVOID               VirtualReserveContext;
	ULONG               agpFifoSizeInB;
	
} H3_AGP_INFO;

//
// Define the page size for the Intel 386 as 4096 (0x1000).
//

#define PAGE_SIZE 0x1000

//
// Define the number of trailing zeroes in a page aligned virtual address.
// This is used as the shift count when shifting virtual addresses to
// virtual page numbers.
//

#define PAGE_SHIFT 12L


//++
//
// ULONG
// BYTES_TO_PAGES (
//     IN ULONG Size
//     )
//
// Routine Description:
//
//     The BYTES_TO_PAGES macro takes the size in bytes and calculates the
//     number of pages required to contain the bytes.
//
// Arguments:
//
//     Size - Size in bytes.
//
// Return Value:
//
//     Returns the number of pages required to contain the specified size.
//
//--

#define BYTES_TO_PAGES(Size)  ((ULONG)((ULONG_PTR)(Size) >> PAGE_SHIFT) + \
                               (((ULONG)(Size) & (PAGE_SIZE - 1)) != 0))

#endif

//
// Define device extension structure. This is device dependent/private
// information.
//

#if (_WIN32_WINNT >= 0x0500)
#define ENABLE_UNATTENDED_INSTALL_CHECK   0

#define KMVT        1
#define ENABLE_IRQ        1

//Buffer for KMVT and IRQ
// Note:  If you modify anything in this structure
//        you need to change the same sturcture in
//        driver.h in display driver.
//
typedef struct{
    ULONG dwSize;
    ULONG dwStatus;
    ULONG dwVPAddr0;
    ULONG dwVPAddr1;
    ULONG dwVPAddr2;
    ULONG dwVPStride;
    ULONG dwOVLOffset;
    ULONG dwOldBuf;
    ULONG dwVBILines;

	// V3TV
    ULONG fpVidMem[3];
    ULONG dwVideoLines;			// video data
	ULONG dwVideoLinesOrig;
    ULONG dwVideoSurfacePitch;	// surface pitch
	ULONG dwVBILinesOrig;
    ULONG dwVidInFormat;
    ULONG dwCounter;
	ULONG dwDataWidth;
	ULONG dwDataHeight;

	//PLD 656 variables
    BOOLEAN bVMIPLDinUse;
    BOOLEAN bLastFieldEven;
    BOOLEAN bLastIntrVBI;
    BOOLEAN bVBIcropped;
    ULONG dwLastBufferFilled;
    BOOLEAN  bSawVBIintr;

	// other need info
	BOOLEAN  bDoublePitch;
	ULONG dwPLDVidLines;
	BOOLEAN  bUsingEAV;
	BOOLEAN  bUseReset;

	//V3TV
    //UCHAR     bSkipNextField;
    BOOLEAN	  bInitialized;
    UCHAR	  fTransferFull;
    ULONG	  dwCurrentTransfer;
    ULONG     dwQueuedTransfers;
    ULONG	  dwBusMasterTransfers;
    ULONG	  dwFieldCount;
	BOOLEAN	  bReportForVideo;
	UCHAR	  bFirstFieldBuffer;
	ULONG	  dwIMask;	
	BOOLEAN	  fWDMActive;
	ULONG     decoderWidth;
	ULONG	  dwResetCounter;
} KMVTBUFF;

//value for dwStatus
#define BUFF1_IN_USE    0x0000
#define BUFF2_IN_USE    0x0001
#define BUFF3_IN_USE    0x0002
#define BUFFER_IN_USE_MASK    0x3
#define ONE_BUFFER      0x0100
#define TWO_BUFFERS     0x0200
#define THREE_BUFFERS   0x0300
#define VP_BUFF_MASK    0x0300
#define VP_BUFF_SHIFT   8

#define BOB_ON          0x10000
#define INTERLEAVE_ON   0x20000

#else
#define ENABLE_UNATTENDED_INSTALL_CHECK   1
#define KMVT         0
#define ENABLE_IRQ      0

#endif

#define MAX_ADDRESS_TABLE_SIZE	256
typedef struct _HW_DEVICE_EXTENSION {
	//
	// Address map
	//
	PHYSICAL_ADDRESS PhysicalMemBaseAddr0;
	ULONG MemBase0Length;
	UCHAR MemBase0InIOSpace;
	PHYSICAL_ADDRESS PhysicalMemBaseAddr1;
	ULONG MemBase1Length;
	UCHAR MemBase1InIOSpace;
	PHYSICAL_ADDRESS PhysicalIOBaseAddr;
	ULONG IOBaseLength;
	UCHAR IOBaseInIoSpace;

	VIDEO_ACCESS_RANGE AccessRanges[NUM_H3_ACCESS_RANGES];

#if REDUCED_MEMORY_MAPPINGS
  PVOID MappedAddress[SST_MAX_INDEX];
#else
  PVOID MappedAddress[NUM_H3_ACCESS_RANGES];
#endif

#if ENABLE_ADDRESS_LIST_ARRAY
	ULONG AddressList[MAX_ADDRESS_TABLE_SIZE];
	SHORT AddressAllocCount[MAX_ADDRESS_TABLE_SIZE];
//DanO 06/01/99 The following is used by an illegal import for W2K but we need it for now to make the OpenGL ICD work.
//DanO 06/01/99 #if (_WIN32_WINNT < 0x0500)
	PEPROCESS AddressProcess[MAX_ADDRESS_TABLE_SIZE];
//DanO 06/01/99 #endif
#endif

	//
	// Bus Info
	//
	ULONG Bus;
	ULONG BusNumber;

	//
	// PCI stuff
	//
	PCI_SLOT_NUMBER PCISlot;
	USHORT PCIVendorID;
	USHORT PCIDeviceID;
	USHORT PCISubVendorID;
	USHORT PCISubSystemID;
	ULONG bVGAEnabled;
	ULONG PciSpeed;
	BOOLEAN IsSecondaryDevice;
#if (_WIN32_WINNT >= 0x500) && defined (AGP_FIFO_CODE)
	BOOLEAN IsAGPCard;
#endif

	//
	// chip info and IDs
	//
	ULONG dramInit0;
	ULONG dramInit1;
	ULONG miscInit0;
	ULONG miscInit1;
  ULONG pciInit0;
  ULONG agpInit0;
  ULONG pllCtrl1;
  ULONG sgramMode;
	ULONG ChipID;
	ULONG ChipRevision;
	ULONG SubTypeID;
	ULONG DacID;
	ULONG SDRAMPresent;
	ULONG SDRAMMemorySize;
	ULONG SGRAMMode;
	ULONG SGRAMChips;
	ULONG SGRAMMemorySize;
	ULONG AdapterMemorySize;

	//
	// Bios information (if applicable)
	//
	UCHAR BiosPresent;
	ULONG BiosSize;
	UCHAR BiosVersion[MAX_BIOS_VERSION_LENGTH];
	BOOLEAN OldBiosPresent;

	//
	// Mode and Frequency
	//
	UCHAR FrequencySecondaryIndex;
	BOOLEAN bNeedReset;
	PH3_VIDEO_MODES ActiveModeEntry;
	PH3_VIDEO_FREQUENCIES ActiveFrequencyEntry;
	PH3_VIDEO_FREQUENCIES Int10FrequencyTable;
	PH3_VIDEO_FREQUENCIES FixedFrequencyTable;
	PH3_VIDEO_FREQUENCIES FrequencyTable;
	ULONG BoardNumber;
	ULONG NumberOfAdapters;
	ULONG NumAvailableModes;
	ULONG NumTotalModes;

	// Power Management
	BOOLEAN PowerStateChanged;
	VIDEO_POWER_STATE PowerState;
	VIDEO_POWER_MANAGEMENT PowerManagement;
	ULONG DDC2_save_serial_parallel;

	ULONG Capabilities;

	PVOID DriverObject;
	PVOID RegistryPath;

	// defaults for registry variables
	ULONG MiniportDebugLevel;	// for debugging purposes only
	ULONG UseSoftwareCursor;	// for debugging purposes only, I hope
	ULONG UseNonBIOSModeSet;
	ULONG MemClocking;
	ULONG GraphicsClocking;
	ULONG GammaTable[NUM_CLUT_ENTRIES];
	ULONG GlideGammaTable[NUM_CLUT_ENTRIES];
#if ENABLE_UNATTENDED_INSTALL_CHECK
	ULONG UnattendedInstall;	// required for Gateway configurations <= 32 Mb
#endif

#ifdef SLI_AA
	// SLI/AA info.
  ULONG               numUnits;
  // we probably only need 4 of the following instead of PCI_MAX_FUNCTION
  PCI_SLOT_NUMBER     sliSlotNumber[PCI_MAX_FUNCTION];
  VIDEO_ACCESS_RANGE  sliAccessRanges[PCI_MAX_FUNCTION][NUM_H3_ACCESS_RANGES];
  PVOID               sliMappedAddress[PCI_MAX_FUNCTION][HWINFO_SST_MAX_CHIP_INDEX];

#if 0
	ULONG render32bpp;		// Reg key 32BPP_RENDERING
	ULONG disableSli;		// Reg key DISABLE_SLI
	ULONG AAenable;			// Reg key ENABLE_AA
	ULONG AAsamples;		// Reg key AA_SAMPLES
	ULONG twoPPC;			// Reg key 2PPC
	ULONG twoPPCLog2BandHeight;	// Reg key 2PPC_LOG2_BAND_HEIGHT
	ULONG guardbandClip;	// Reg key GUARDBAND_CLIPPING
	ULONG cbc;				// Reg key CBC - Triangle iterators band control
	ULONG digitalSliAAEnable;	// Reg key DIGITAL_SLI_AA - enable digital SLI/AA
	ULONG sliBandHeight;	// Reg key SLI_BAND_HEIGHT
	ULONG swapAlgorithmType;	// Reg key SWAPBUFFER_ALGO
	// Dither matrix key TBD
	// Add programmable sub-sample jitter.
#endif

	ULONG IsMasterDevice;
#ifdef RD_ABORT_ERROR
  SLI_AA_REQUEST  SLIAARequest;
#endif
  BOOLEAN bVIACoreLogic;
#endif

#ifdef TVOUT_SUPPORTED
// tv Out and LCD stuff
	BOOLEAN tvOutActive;                  // True if desktop is on TV out
	ULONG tvOutData[2];                   // Brooktree regs not readable so we
                                          // need to store state somewhere!
	BOOLEAN tvOutCapable;
	BOOLEAN monitorActive;                // True if output directed to monitor
	ULONG tvPixelClock;
	BOOLEAN nvramCacheInit;                 // boolean for nvram cache initialization
	UCHAR nvramCache[128];                  // reading the real nvram is too slow
    I2C_DEVINFO i2cInfo;                   // I2C bus configuration and state information
    I2C_BUSINFO i2cBusData[I2C_MAXBUSES];
    DFP_DISPLAY dfpDisp;                   // DFP display info and standard mode list
	BOOLEAN bDfpActiveAtBoot;
	BOOLEAN bAllowPALCRT;
	ULONG copyProtectKey;
	I2CKEY ddcKey;
	ULONG biosBoardConfigInfo;
	UCHAR fakeBiosScr2;  // fake copy of BIOS scratch register 2 used on secondary cards in multimon config.
	UCHAR fakeBiosScr3;  // fake copy of BIOS scratch register 3 used on secondary cards in multimon config.
    UCHAR dfpEdidCache[A2_EDID_SIZE];       // This is a copy of the DFP EDI information.
#endif //def TVOUT_SUPPORTED

#if (_WIN32_WINNT >= 0x500) && defined (AGP_FIFO_CODE)
	// AGP stuff

	ULONG AFifo;                // AGP command fifo registry flag

	VIDEO_PORT_AGP_SERVICES AgpServices;
	H3_AGP_INFO	agpinfo;
#endif

#if KMVT || ENABLE_IRQ
    KMVTBUFF * KMVTBuff;
	//V3TV KMVT 
	PVOID   pfnIrqCallback;
	ULONG	dwIRQSources;
	PVOID   lpIRQData;
#endif
#if (_WIN32_WINNT >= 0x500)
    PVOID Kernel_VMIWrite;
    PVOID Kernel_VMIRead;
#endif
	ULONG CustNum;							// customer number

#ifdef GTF_TIMINGS
	BOOLEAN MonitorIsGTF;					// Monitor supports GTF timings
#endif // GTF_TIMINGS

#ifdef DMT_ENABLED
	ULONG RegistryNumModes;
	BOOLEAN RegistryForceGTF;
	H3_VIDEO_FREQUENCIES * RegistryVideoFreqs;
	char DevNodeKeyPath[256];
	TIMING_PARAMS centeredDfpBaseTimings;
	TIMING_PARAMS centeredDfpCurrentTimings;
#endif // DMT_ENABLED

  PWSTR pDriverRegistryPath;
  ULONG dwDFPState;
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

//
// Highest valid DAC color register index.
//

#define VIDEO_MAX_COLOR_REGISTER  0xFF
//
// Desktop surface information used in mode set
//
typedef struct _DESKTOP_SURFACE
{
	ULONG enable;				// 1=enable desktop surface (DS), 1=disable
	ULONG tiled;				// 0=DS linear, 1=tiled
	ULONG pixFmt;				// pixel format of DS (vidProcCfg register)
	ULONG clutBypass;			// bypass clut for DS?
	ULONG clutSelect;			// 0=lower 256 CLUT entries, 1=upper 256
	ULONG stride;				// distance between scanlines of the DS, in
	ULONG scanlinedouble;		// B0 Scanline Double Mode
} DESKTOP_SURFACE, *PDESKTOP_SURFACE;


//
// We use the following constant in calculating the stretch factor for overlays
// by using a 0.20 format for the fraction.
// This gets used for both dudx and dvdy.
//
#define STRETCH_BY_TWO 0x80000

//
// Overlay surface information used in mode set
//
typedef struct _OVERLAY_SURFACE
{
	ULONG enable;				// 1=enable Overlay surface (OS), 1=disable
	ULONG stereo;				// 1=enable OS stereo, 0=disable
	ULONG horizScaling; 		// 1=enable horizontal scaling, 0=disable
	ULONG dudx; 				// horizontal scale factor (ignored if not scaling)
	ULONG verticalScaling;		// 1=enable vertical scaling, 0=disable
	ULONG dvdy; 				// vertical scale factor (ignored if not scaling)
	ULONG filterMode;			// duh
	ULONG tiled;				// 0=OS linear, 1=tiled
	ULONG pixFmt;				// pixel format of OS
	ULONG clutBypass;			// bypass clut for OS?
	ULONG clutSelect;			// 0=lower 256 CLUT entries, 1=upper 256
	ULONG stride;				// distance between scanlines of the OS, in

} OVERLAY_SURFACE, *POVERLAY_SURFACE;


//
// Video Processeor information used in mode set
//
typedef struct _VID_PROC_CONFIG
{
	ULONG changeVideoMode;		// change video mode timings
	ULONG changeDesktop;		// change current desktop surface settings?
	DESKTOP_SURFACE dtSurface;	// new desktop surface data
	ULONG changeOverlay;		// change current overlay surface settings?
	OVERLAY_SURFACE ovSurface;	// new overlay surface data
	ULONG width;
	ULONG height;
} VID_PROC_CONFIG, *PVID_PROC_CONFIG;

//
// Data
//

extern ULONG DebugLevel;

//
// Global Physical Access Ranges.
// Logical access ranges must be stored in the HwDeviceExtension so different
// addresses can be used for different boards. We have two of them now because
// of Microsoft's inability for the setup kernel to permit > 16 Mb of nonpaged
// access ranges.
//

extern VIDEO_ACCESS_RANGE H3SmallAccessRanges[];	// used during unattended installation
extern VIDEO_ACCESS_RANGE H3AccessRanges[];

//
// Memory settings for SGRAMs
//

extern ULONG H3SgramTimings[];

//
// Memory Size array
//

extern ULONG gacjMemorySize[];

//
// Default gamma table for Glide
//
ULONG DefaultGlideGammaTable[];

//
// Modeset frequency tables for both STB and 3Dfx
// We include both versions because we can go either way, and
// the table code goes nicer this way
//

extern H3_VIDEO_FREQUENCIES H4GenericFrequencyTable[];
extern H3_VIDEO_FREQUENCIES H4GenericInt10FrequencyTable[];
extern H3_VIDEO_FREQUENCIES H4VariableInt10FrequencyTable[];

//
// Mode Tables
//

extern H3_VIDEO_MODES H3Modes[];
extern ULONG NumH3VideoModes;

//
// Clock Tables
//
extern ULONG H4oempllTable[];
extern ULONG H4pllTable[];

//
// Function prototypes
//

extern VOID DbgBreakPoint(VOID);

#if (_WIN32_WINNT >= 0x500)

//
// ddc.c
//

VOID
WriteClockLine(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	UCHAR ucData
	);

VOID
WriteDataLine(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	UCHAR ucData
	);

BOOLEAN
ReadClockLine(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

BOOLEAN
ReadDataLine(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

VOID
WaitForVsyncActive(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

BOOLEAN
GetDdcInformation (
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PUCHAR QueryBuffer,
	ULONG BufferSize
	);

BOOLEAN
GetDfpEdidInformation (
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PUCHAR QueryBuffer,
	ULONG BufferSize
	);

#endif

//
// debug.c
//

#if ENABLE_ADDRESS_LIST_ARRAY
PULONG
H3AllocFindAddress(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PTDFX_LINEARADDR_INFO pInfo
	);

VOID
H3RefCountAlloc(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Addr
	);

VOID
H3RefCountFree(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket,
	ULONG Addr
	);

VOID
DumpAddressTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);
#endif

VOID
DumpAccessRanges(
	PVIDEO_ACCESS_RANGE range,
	UCHAR count
	);

VOID
DumpModeInformation(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG modeNumber,
	PH3_VIDEO_MODES ModeEntry
	);

#if DBG
VOID
DumpH3Regs(PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG DebugLevel);

VOID
DumpMTRRs(PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG DebugLevel);
#endif

//
// h3.c
//

VP_STATUS
H3FindAdapter(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVOID HwContext,
	PWSTR ArgumentString,
	PVIDEO_PORT_CONFIG_INFO ConfigInfo,
	PUCHAR Again
	);

VP_STATUS
H3RecordChipType(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

VP_STATUS
H3DetermineMemorySize(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

VP_STATUS
H3MapAccessRanges(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

BOOLEAN
H3Initialize(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

VOID
H3InitializeSecondaryDevice(PHW_DEVICE_EXTENSION  HwDeviceExtension,
                            PH3_MEMBASE0          RegisterMap,
                            PUCHAR                pIO);

BOOLEAN
H3ResetHw(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Columns,
	ULONG Rows
	);

BOOLEAN
H3StartIO(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3SetColorLookup(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VOID
H3UpdateGamma(PHW_DEVICE_EXTENSION  HwDeviceExtension,
              PH3_MEMBASE0          RegisterMap,
              PUCHAR                pIO,
              PULONG                GammaTable);

#if ENABLE_LOG_FILE
VP_STATUS H3WriteLogFile(PVOID pBuffer, ULONG BytesToWrite);
#endif

//
// h3modes.c
//

VP_STATUS
H3ValidateModes(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

VP_STATUS
H3SetCurrentMode(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3QueryAvailableModes(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3QueryNumAvailableModes(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3QueryCurrentMode(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

//
// h3modeset.c
//

ULONG
bppToPixfmt(ULONG bpp);

VOID
LockVgaTimingRegisters(PHW_DEVICE_EXTENSION HwDeviceExtension,
                       PH3_MEMBASE0         RegisterMap);

VOID
UnlockVgaTimingRegisters(PHW_DEVICE_EXTENSION HwDeviceExtension,
                         PH3_MEMBASE0         RegisterMap);

VP_STATUS
H3SetRefreshRate(PHW_DEVICE_EXTENSION   HwDeviceExtension,
                 PH3_MEMBASE0           RegisterMap,
                 PUCHAR                 pIO,
                 PH3_VIDEO_FREQUENCIES  FrequencyEntry);

VOID
H3InitDesktopSurface(PHW_DEVICE_EXTENSION   HwDeviceExtension,
                     PH3_MEMBASE0           RegisterMap,
                     PUCHAR                 pIO,
                     PH3_VIDEO_FREQUENCIES  FrequencyEntry,
                     PVID_PROC_CONFIG       pvidInfo);

VP_STATUS
H3SetModeOverlayDouble(PHW_DEVICE_EXTENSION   HwDeviceExtension,
                       PH3_MEMBASE0           RegisterMap,
                       PUCHAR                 pIO,
                       PH3_VIDEO_FREQUENCIES  FrequencyEntry,
                       PVID_PROC_CONFIG       pvidInfo);

VOID
H3InitOverlaySurface(PHW_DEVICE_EXTENSION   HwDeviceExtension,
                     PH3_MEMBASE0           RegisterMap,
                     PUCHAR                 pIO,
                     PH3_VIDEO_FREQUENCIES  FrequencyEntry,
                     PVID_PROC_CONFIG       pvidInfo);

VP_STATUS
H3InitSetMode(PHW_DEVICE_EXTENSION  HwDeviceExtension,
              PH3_MEMBASE0          RegisterMap,
              PUCHAR                pIO,
              PH3_VIDEO_FREQUENCIES FrequencyEntry,
              PVID_PROC_CONFIG      pvidInfo);

VP_STATUS
H3SetMode(PHW_DEVICE_EXTENSION  HwDeviceExtension,
          PH3_MEMBASE0          RegisterMap,
          PUCHAR                pIO);

VOID
H3SetModeResetTiming(PHW_DEVICE_EXTENSION  HwDeviceExtension);

//
// h3registry.c
//

PVOID
H3CopyUnicodeString(
	PVOID SourceString
	);

VP_STATUS
H3GetGammaTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3SetGammaTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3GetGlideGammaTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3SetGlideGammaTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3RetrieveGammaCallback(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVOID Context,
	PWSTR ValueName,
	PVOID ValueData,
	ULONG ValueLength
	);

VP_STATUS
H3RegistryCallback(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVOID Context,
	PWSTR ValueName,
	PVOID ValueData,
	ULONG ValueLength
	);

VOID
H3GetRegistrySettings(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

VOID
H3CopyDriverRegistryPathToDeviceExtension(PHW_DEVICE_EXTENSION  hwDeviceExtension,
                                          PWSTR                 pDriverRegistryPath);

VP_STATUS
H3QueryRegistryValue(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET pRequestPacket
	);

VP_STATUS
H3GetRegistryValue(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET pRequestPacket
	);

VP_STATUS
H3SetRegistryValue(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET pRequestPacket
	);

//
// pci.c
//

UCHAR
H3PCIConfigReadUchar(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset
	);

BOOLEAN
H3PCIConfigWriteUchar(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset,
	UCHAR Data
	);

ULONG
H3PCIConfigReadUlong(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset
	);

BOOLEAN
H3PCIConfigWriteUlong(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset,
	ULONG Data
	);

USHORT
H3PCIConfigReadUshort(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset
	);

BOOLEAN
H3PCIConfigWriteUshort(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset,
	USHORT Data
	);

VP_STATUS
H3ConfigurePCI(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	);

#if DBG
VOID
DumpPCIConfigSpace(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG slot
	);
#endif

#if (_WIN32_WINNT >= 0x500)

//
// power.c
//

ULONG
H3GetChildDescriptor(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
	PVIDEO_CHILD_TYPE pChildType,
	PUCHAR pChildDescriptor,
	PULONG pHwId,
	PULONG pUnused
	);

VP_STATUS
H3GetPowerState(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG HwDeviceId,
	PVIDEO_POWER_MANAGEMENT VideoPowerManagement
	);

VP_STATUS
H3SetPowerState(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG HwDeviceId,
	PVIDEO_POWER_MANAGEMENT VideoPowerManagement
	);

VP_STATUS
H3GetPowerManagement(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

VP_STATUS
H3SetPowerManagement(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	);

// KMVT.c

#if KMVT || ENABLE_IRQ
VP_STATUS H3AllocatKMBuff(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket);
#endif

#if KMVT
VP_STATUS QueryDriverInterface(
	PHW_DEVICE_EXTENSION hwDeviceExtension,
    PQUERY_INTERFACE QueryInterface);
#endif

#if ENABLE_IRQ
BOOLEAN H3VidInterrupt(
	PHW_DEVICE_EXTENSION HwDeviceExtension);
#endif

VP_STATUS VDDVMIFunctions(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket);

ULONG QueryForVMIPld(  PHW_DEVICE_EXTENSION HwDeviceExtension);

VP_STATUS WDMV3TVScale(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket);

#endif

#ifdef DMT_ENABLED
extern void di_FindDevNode( PHW_DEVICE_EXTENSION HwDeviceExtension );
#ifdef MS_VIEW
extern void * di_MemoryAlloc( PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG size );
#else
extern void * di_MemoryAlloc( ULONG size );
#endif
extern void * di_AllocModeTable( PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG * nummodes );
extern void di_FillModeTable( PHW_DEVICE_EXTENSION HwDeviceExtension );

extern LONG di_RegOpenKey( ULONG mainkey, CHAR * keyname, HKEY *hkeyptr );
extern LONG di_RegCloseKey( HKEY hkey );
extern LONG di_RegQueryValueEx( HKEY hkey, CHAR * pValName, ULONG * reserved, ULONG * pType, CHAR * pData, ULONG * retLength );
extern LONG di_RegSetValueEx( HKEY hkey, CHAR * pValName, ULONG reserved, ULONG Type, CHAR * pData, ULONG Length );
extern LONG di_RegEnumKey( HKEY hkey, ULONG index, CHAR * pKeyName, ULONG length );
extern VIDEO_MODE_INFORMATION BasicModeInfo_8BPP;
extern VIDEO_MODE_INFORMATION BasicModeInfo_16BPP;
extern VIDEO_MODE_INFORMATION BasicModeInfo_32BPP;
extern H3_VIDEO_MODES DefaultMode[];
#endif // DMT_ENABLED

#if defined ( GTF_TIMINGS ) || ( DMT_ENABLED )
extern VOID ds_Calc_CRTC_table( PHW_DEVICE_EXTENSION HwDeviceExtension, TIMING_PARAMS *pVprm, FxU32 actualWidth, FxU32 actualHeight, PUCHAR crtc_table );
extern VOID di_GetGTF_Timing( TIMING_PARAMS *pVprm );
extern BOOLEAN di_IsMonitorGTF( PHW_DEVICE_EXTENSION HwDeviceExtension );
#endif // ( GTF_TIMINGS ) || ( DMT_ENABLED )


//
// Other defs, not in our source files
//
#ifndef _NTDDK_
extern
MM_SYSTEMSIZE
MmQuerySystemSize(
    VOID
    );
#endif

extern VOID
ZwTerminateProcess(
	PVOID Handle,
	ULONG Code
	);


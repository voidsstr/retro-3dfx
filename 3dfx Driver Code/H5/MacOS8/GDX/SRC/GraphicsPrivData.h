/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
*/

#ifndef GRAPHICS_PRIV_DATA_H
#define GRAPHICS_PRIV_DATA_H 1

#include <3dfx.h>

#include <h3cinitdd_mac.h>
#include <h3cinit.h>
#include <h3regs.h>

#include <minihwc.h>
#include <hwcio.h>

#include <hdwr_res_mgr.h>
#include <hrm_fifo.h>
#include <hrm_mem.h>

#include <NameRegistry.h>
#include <Devices.h>

//#include <AGP.h>

/* These really need to be moved into h3defs.h!!! */

#define SST_INTR_HSYNC_RISING_ENABLE		BIT(0)
#define SST_INTR_HSYNC_FALLING_ENABLE		BIT(1)
#define SST_INTR_VSYNC_RISING_ENABLE		BIT(2)
#define SST_INTR_VSYNC_FALLING_ENABLE		BIT(3)
#define SST_INTR_FIFO_FULL_ENABLE			BIT(4)
#define SST_INTR_USER_INTR_ENABLE			BIT(5)
#define SST_INTR_HSYNC_RISING_GENERATED		BIT(6)
#define SST_INTR_HSYNC_FALLING_GENERATED	BIT(7)
#define SST_INTR_VSYNC_RISING_GENERATED		BIT(8)
#define SST_INTR_VSYNC_FALLING_GENERATED	BIT(9)
#define SST_INTR_FIFO_FULL_GENERATED		BIT(10)
#define SST_INTR_USER_INTR_GENERATED		BIT(11)
#define SST_INTR_USER_COMMAND_TAG_SHIFT		12
#define SST_INTR_USER_COMMAND_TAG			(0x7F << SST_INTR_USER_COMMAND_TAG_SHIFT)
#define SST_INTR_HOLE_COUNT_ENABLE			BIT(20)
#define SST_INTR_VMI_ENABLE					BIT(21)
#define SST_INTR_HOLE_COUNT_GENERATED		BIT(22)
#define SST_INTR_VMI_GENERATED				BIT(23)
#define SST_INTR_VGA_GENERATED				BIT(30)
#define SST_INTR_PCI_INTA					BIT(31)

/*
** AvengerHALData
**  This structure contains the necessary items for the Avenger HAL to maintain its
**  state information.
**
**  This is COMPLETELY private to the Avenger implementation of the Graphics HAL.
*/
typedef struct AvengerCLUTEntry
{
	FxU8 a;
	FxU8 r;
	FxU8 g;
	FxU8 b;
} AvengerCLUTEntry;

typedef struct AvengerCLUT
{
	AvengerCLUTEntry entry[512];
} AvengerCLUT;

#define VIDMEM_GRANULARITY			4096
#define VIDMEM_GRANULARITY_MASK	(VIDMEM_GRANULARITY - 1)

typedef struct AvengerHALData AvengerHALData;
struct AvengerHALData
{
  RegEntryID regEntryID[4];    /* The RegEntryID for this device */
	DriverRefNum refNum;
  AbsoluteTime ns260;     /* 260 ns is the absolute time between CLUT accesses */
  DisplayCode displayCode;  /* Class of the connected display */
	FxU32 syncFlags;
	FxI32 monitorType;
	FxU32 senseCode;
	FxU32 redTrigger, greenTrigger;
	FxU32 width, height, refresh;

	Byte  edid[kDDCBlockSize];
	FxU32 edidWasRead;
	FxU32 edidIsValid;
	FxU32 edidIsDVI;
	FxU32 his;
	FxU32 vis;
	FxU32 intTrigger;

  VidProcConfig vpc;
  FxU32 savedVidProcCfg;
  
  /* For using some of the hwc cinit code. */
  FxU32 isMaster;
  FxU32 numChips;
  FxU32 supportsAGP;
  FxU32 isPCI;
  FxU32 pciInit0;
  
  DEVTABLE devTable[4];
  
  FxU32 slaveMemBase0[4];
  FxU32 slaveMemBase1[4];
  FxU32 slaveIOBase0[4];
  FxU32 slaveIOPortBase[4];
  
	hwcBoardInfo bInfo;
  hwcRegInfo slaveRegInfo[4];
  
  /* Other crap */
	FxU32	pciStride;
	FxU32	hwStride;
	FxU32	tileMark;

  /* Misc hardware stuff */
	FxU32 intrCrtl;
	FxI32 cursorX;
	FxI32 cursorY;
	FxU32 cursorVisible;
	FxU32 cursorSet;
	FxU32 curC0;
	FxU32 curC1;
	FxU32 displayMode;
	FxU32 depthMode;
	FxU32 pixelDepth;
	FxU32 grxClock;
	FxU32 halPreferences;
	FxU32 powerState;
	FxU32 powerLevel;
	FxU32 needsRefresh;

  /* CLUTs (duh) */
	AvengerCLUT baseCLUT;

  /* Framebuffer location */
	FxU32 frameBufferOffset;
	FxU32 frameBufferBase;
	FxU32 frameBufferSize;
	FxU32 swizzleOffset;
	FxU32 swizzleOffsets[4];

  /* Cheesy Video Memory allocation */
	//FxU8	memBlockUsed[VIDMEM_GRANULARITY];	/* 4K Allocation granularity.  So sue me. */
	FxU32	memBlockSize;
	FxU32	memBlockMask;

    FxI8 dummyFrameBuffer[8192 + 2048*8]; /* Should be enough to handle a 2K x 2K 32-bit framebuffer. */
  
  /* Command fifo stuff */
	FxU32	nextContextID;
	FxU32	lastContextID;
  struct H3FifoInfo fifo;
  
  #if 0
  AGPAddressPair agpAddress;
  FxBool agpEnabled;
  #endif
  
  /* Register save area for power management sleep/wake */
  SstCRegs cmdRegs;
  SstGRegs waxRegs;
  SstPCIConfigRegs configRegs[4];
  
  /* HRM interactions */
  hrmGetExtensionPtr hrmGetExtension;
  hrmFifoUpdatePtr hrmFifoUpdate;
  hrmDisableFifo2DPtr hrmDisableFifo2D;
  hrmEnableFifo2DPtr hrmEnableFifo2D;
  hrmCreateMemoryAreaPtr hrmCreateMemoryArea;
  hrmDeleteMemoryAreaPtr hrmDeleteMemoryArea;
  hrmInvalidateMemoryBlocksPtr hrmInvalidateMemoryBlocks;
  hrmAllocateBlockPtr hrmAllocateBlock;
  hrmFreeBlockPtr hrmFreeBlock;
  
  hrmBoard_t *hrmBoard;     /* The board struct the HRM knows us by */
  mmArea_t *fixedArea;    /* Fixed heap that we never destroy */
  mmArea_t *dynamicArea;  /* Dynamic heap */
  
  mmBlock_t *frameBufferBlock; /* Block used for our framebuffer */
  
  FxU32 scalerNeeded;
  FxU32 scaler2x;
  
  FxU32  debug;

};

#if 0
AvengerHALData *GraphicsHALGetHALData(void);
#else
#define GraphicsHALGetHALData() &(gAvengerHALData)
extern AvengerHALData gAvengerHALData;    /* Persistant Avenger specific data storage */
#endif

#if GDX_SWIZZLE_HACK
#define SET_FIFO_LFB(d, s)			avengerHALData->fifo.setLfb(&(d),s)
#define SET_FIFO_HOST(d, s) 		avengerHALData->fifo.setLfbHost(&(d),s)
#else
#define	SET_FIFO_LFB(d, s)			__stwbrx(s,(void *)&(d),0)
#define SET_FIFO_HOST(d, s) (d = (s))
#endif

#if PCI_COPYBACK
#define FIFO_CACHE_FLUSH(d)  __dcbf(d,-4)
#else
#define FIFO_CACHE_FLUSH(d)
#endif

#if PCI_BUMP_N_GRIND
#define BUMP_N_GRIND \
{ \
  FIFO_CACHE_FLUSH(avengerHALData->fifo.fifoPtr);\
  P6FENCE;\
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.bump, avengerHALData->fifo.fifoPtr - avengerHALData->fifo.lastBump);\
  P6FENCE;\
  avengerHALData->fifo.lastBump = avengerHALData->fifo.fifoPtr;\
}
#else
#define BUMP_N_GRIND
#endif

/* The Voodoo^2 fifo is 4 byte aligned */
#define FIFO_ALIGN_MASK      0x03

/* We need some slop in the fifo for writing some bookkeeping data
 * since we don't let the fifo autowrap. Its actually a little bit
 * bigger just in case someone does not read this comment.
 *
 * Fullscreen:
 *   1 jmp (1 32-bit word)
 * Windowed:
 *   pci: ret (1 32-bit word)
 *   agp: jmp (2 32-bit words)
 */
#define FIFO_END_ADJUST  (sizeof(FxU32) << 3)

/* The two most commonly defined macros in the known universe */
#define MIN(__x, __y) (((__x) < (__y)) ? (__x) : (__y))
#define MAX(__x, __y) (((__x) < (__y)) ? (__y) : (__x))

#endif  /* GRAPHICS_PRIV_DATA not defined */



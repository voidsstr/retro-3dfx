/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** File name:   GraphicsPrivHwc.h
**
** Description: Structures for the hwc extensions.
**
** $Header: GraphicsPrivHWC.h, 5, 10/11/00 8:38:02 PM, Brent$
**
** $History: GraphicsPrivHwc.h $
** 
** *****************  Version 7  *****************
** User: Kcd          Date: 8/23/99    Time: 2:38p
** Updated in $/devel/h3/MacOS8/shared_headers
** 
** *****************  Version 6  *****************
** User: Kcd          Date: 8/02/99    Time: 12:28p
** Updated in $/devel/h3/MacOS8/shared_headers
** New SetPrefs/GetPrefs stuff.
** 
** *****************  Version 5  *****************
** User: Kcd          Date: 7/30/99    Time: 1:04p
** Updated in $/devel/h3/MacOS8/shared_headers
** Code formatting cleanup.
** 
** *****************  Version 4  *****************
** User: Kcd          Date: 7/26/99    Time: 1:53p
** Updated in $/devel/h3/MacOS8/shared_headers
** A few new blit mode types.
** 
** *****************  Version 3  *****************
** User: Kcd          Date: 7/08/99    Time: 1:27p
** Updated in $/devel/h3/MacOS8/shared_headers
** Graphics clock extension.
** 
** *****************  Version 2  *****************
** User: Kcd          Date: 7/02/99    Time: 3:27p
** Updated in $/devel/h3/MacOS8/shared_headers
** New HRM stuff.
** 
** *****************  Version 1  *****************
** User: Shuaulme     Date: 6/21/99    Time: 5:56p
** Created in $/devel/h3/MacOS8/shared_headers
** 
** *****************  Version 2  *****************
** User: Kcd          Date: 6/10/99    Time: 1:10p
** Updated in $/devel/h3/MacOS8/GDX/src
** New backdoor mode setting routine.
** 
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
** 
**
*/

#ifndef GRAPHICS_PRIV_H
#define GRAPHICS_PRIV_H	1

#include <3Dfx.h>
#include <hdwr_res_mgr.h>

/* FIFO types */
#define HWCEXT_FIFO_INVALID      0x00UL
#define HWCEXT_FIFO_FB           0x01UL /* FIFO in frame buffer */
#define HWCEXT_FIFO_HOST         0x02UL /* FIFO in host memory */
#define HWCEXT_FIFO_AGP          0x03UL /* FIFO in AGP */

/* Scale blit transfer types */
#define HWCEXT_TRANSFER_NORMAL   0x00UL
#define HWCEXT_TRANSFER_16TO32   0x01UL
#define HWCEXT_TRANSFER_32TO16   0x02UL
#define HWCEXT_TRANSFER_8TO32    0x03UL
#define HWCEXT_TRANSFER_8TO16    0x04UL

#define k3DfxNewRequest          0x3DFF  /* Outside the range of our old control codes */

/* Private status calls */
#define k3DfxGetDeviceConfig     0x3DF0
#define k3DfxSetModeFlags        0x3DF1
#define k3DfxSetExclusive        0x3DF2
#define k3DfxSetDisplayMode      0x3DF3
#define k3DfxRegisterHRM         0x3DF4
#define k3DfxSetPrefs            0x3DF5
#define k3DfxPCIOp               0x3DF6
#define k3DfxSlaveRegs           0x3DF7
#define k3DfxFifoFuncs           0x3DF8
#define k3DfxSLIAA               0x3DF9
#define k3DfxGetDriverInfo       0x3DFA

#define k3DfxPCIOpRead           0x0000
#define k3DfxPCIOpWrite          0x0001

#define k3DfxSLIAAEnable         0x0000
#define k3DfxSLIAADisable        0x0001

/* Byte swizzle offsets */
#define SWIZZLE_OFFSET_SWAP_NONE    0x00
#define SWIZZLE_OFFSET_SWAP_BYTES   0x01
#define SWIZZLE_OFFSET_SWAP_WORDS   0x02
#define SWIZZLE_OFFSET_SWAP_BOTH    0x03

#define k3DfxDDCVGA              'DDCv'
#define k3DfxDDCDVI              'DDCd'

struct H3FifoInfo
{
	FxU32 exclusiveMode;
	FxU32 cacheActive;
	
  /* Basic command fifo characteristics. These should be
   * considered logically const after their initialization.
   */

  FxU32* fifoStart;    /* Virtual address of start of fifo */
  FxU32* fifoEnd;      /* Virtual address of fba fifo */
  FxU32  fifoOffset;   /* Offset from hw base to fifo start */
  FxU32  fifoSize;     /* Size in bytes of the fifo */
  FxU32  fifoJmpHdr[2];/* Type0 packet for jmp to fifo start
                          only first DWORD is used for memory
                          fifo--both are used for AGP FIFO
                        */
  
  FxU32* fifoPtr;      /* Current write pointer into fifo */
  FxU32  fifoRead;     /* Last known hw read ptr. 
                        * This is the sli master, if enabled.
                        */

  /* Fifo checking information. In units of usuable bytes until
   * the appropriate condition.
   */
  FxU32  fifoRoom;     /* Space until next fifo check */

  /* These are only valid for the fullscreen case */
  FxI32  roomToReadPtr;/* Bytes until last known hw ptr */
  FxI32  roomToEnd;    /* # of bytes until last usable address before fifoEnd */

  /* FxBool lfbLockCount; /* Have we done an lfb lock? Count of the locks. */
  /* FxBool 
      autoBump;                 /* Are we auto bumping (aka hole counting?) */
  FxU32
    *lastBump,                /* Last ptr where we bumped. */
    *bumpPos;                 /* Nex place to bump */
  FxU32
    bumpSize;                 /* # of DWORDS per bump */

  void  	(*setLfb)(volatile FxU32 *d, FxU32 s);
  void		(*setLfbHost)(volatile FxU32 *d, FxU32 s);
};
typedef struct H3FifoInfo H3FifoInfo;

/* Returned from k3DfxGetDeviceConfig */
typedef struct hwcControlDeviceConfigRes_s
{
  FxU16	
    deviceID,
    vendorID;
  FxU32	
    devRev,
    hwBase,
    lfbBase,
    ioPortBase,
    h3Mem,
    pciStride,
    hwStride,
    tileMark,
    frameBufferOffset,
    prefs;
  FxU32 isMaster;               /* True if device is a Master */
  FxU32 numChips;               /* Number of Chips 1-4 */
  FxU32 supportsAGP;            /* Is this an AGP device? */
  FxU32 swizzleOffsets[4];
} hwcControlDeviceConfigRes_t;

/* Given to k3DfxSetModeFlags */
typedef struct hwcControlSetModeFlagsReq_s
{
  FxU32
    displayModeID,
    timingFlags;
} hwcControlSetModeFlagsReq_t;

typedef struct hwcControlSetExclusiveReq_s
{
	FxU32
		exclusive;
} hwcControlSetExclusiveReq_t;

typedef struct hwcControlSetModeReq_s
{
	FxU32
		width,		      
		height,				
		refresh;
} hwcControlSetModeReq_t;

typedef struct hwcControlRegisterHRMDispatch_s
{
  void *(*hrmGetExtension) (const char *extName);
  hrmBoard_t *hrmBoard;
} hwcControlRegisterHRMDispatch_t;

typedef struct hwcControlSetPrefs_s
{
  FxU32 prefs;
} hwcControlSetPrefs_t;


typedef struct hwcControlDriverInfoRes_s
{
  FxU8 major;
  FxU8 minor;
  FxU8 stage;
  FxU8 rev;
} hwcControlDriverInfoRes_t;


/* nVRAM stuff */

#define kPreferredConfigurationName "3dfx"

#define HALDATA_GRXCLOCK_SHIFT 0
#define HALDATA_GRXCLOCK_MASK (0xFF << HALDATA_GRXCLOCK_SHIFT)

#define HALDATA_TWEAKFLAGS_SHIFT 8
#define HALDATA_TWEAKFLAGS_MASK (0xFFFFFF << HALDATA_TWEAKFLAGS_SHIFT)

#define HALDATA_TWEAKFLAG_FASTPCI      BIT(8)
#define HALDATA_TWEAKFLAG_NOFILLHACK   BIT(9)



typedef struct hwGfxNVram_s
{
  UInt8 reserved;
  UInt8 mappedDisplayModeID;
  UInt8 mappedDepthMode;
  UInt8 mappedDisplayCode;    /* Save the DisplayCode in case the user switches monitors */
  UInt32 halData;
} hwGfxNVram_t;

typedef struct hwcControlPCIOpReq_s {
   FxU32 DeviceId;      /* Device PCI Function Number [0-3] */
   FxU32 Operation;      /* PCI Operation  HWCEXT_PCI_READ | HWCEXT_PCI_WRITE */
   FxU32 Offset;        /* PCI Offset [0-255] */
   FxU32 Value;         /* Used only for Writes */
} hwcControlPCIOpReq_t;

typedef struct hwcControlPCIOpRes_s {
   FxU32 Value;         /* Used only for Reads */
} hwcControlPCIOpRes_t;

#define HWCEXT_MAX_SLAVE_REGS (4)
typedef struct hwcControlSlaveRegReq_s {
   FxU32 DeviceId;      /* Device PCI Function Number [0-3] */
} hwcControlSlaveRegReq_t;

typedef struct hwcControlSlaveRegRes_s {
   FxU32 Regs[HWCEXT_MAX_SLAVE_REGS];
} hwcControlSlaveRegRes_t;

typedef struct hwcControlFifoFuncsRes_s
{
  void  	(*setLfb)(volatile FxU32 *d, FxU32 s);
  void		(*setLfbHost)(volatile FxU32 *d, FxU32 s);
} hwcControlFifoFuncsRes_t;


typedef struct hwcControlSLIAAChipInfo_s {
   FxU32 numChips;       // dwChips: Number of chips in multi-chip configuration (1-4)
   FxU32 sliEnable;      // dwsliEn: Sli is to be enabled (0,1)
   FxU32 aaEnable;       // dwaaEn: Anti-aliasing is to be enabled (0,1)
   FxU32 aaSampleHigh;   // dwaaSampleHigh: 0->Enable 2-sample AA, 1->Enable 4-sample AA
   FxU32 sliAaAnalog;    // dwsliAaAnalog: 0->Enable digital SLI/AA, 1->Enable analog Sli/AA
   FxU32 sli_nlines;     // dwsli_nLines: Number of lines owned by each chip in SLI (2-128)
   FxU32 swapAlgorithm;  // Swap Buffer Algorithm
}  hwcControlSLIAAChipInfo_t;

typedef struct hwcControlSLIAAMemInfo_s {
   FxU32 totalMemory;       // In MB
   FxU32 tileMark;          // In MB
   FxU32 tileCmpMark;       // In MB
   FxU32 aaSecondaryColorBufBegin;    // In MB
   FxU32 aaSecondaryDepthBufBegin;    // In MB
   FxU32 aaSecondaryDepthBufEnd;    // In MB
   FxU32 bpp;                        // Bits Per Pixel
}  hwcControlSLIAAMemInfo_t;

typedef struct hwcControlSLIAAReq_s {
   hwcControlSLIAAChipInfo_t chipInfo;
   hwcControlSLIAAMemInfo_t memInfo;
} hwcControlSLIAAReq_t;

typedef struct hwcRequest_s
{
  FxU32 reserved;
  union reqOptData
  {
    hwcControlSetModeFlagsReq_t       setModeFlagsReq; 
    hwcControlSetExclusiveReq_t       setExclusiveReq;
    hwcControlSetModeReq_t            setModeReq;
    hwcControlRegisterHRMDispatch_t   hrmDispatchReq;
    hwcControlSetPrefs_t              setPrefsReq;
    hwcControlPCIOpReq_t              pciOpReq;
    hwcControlSLIAAReq_t              sliaaReq;
    hwcControlSlaveRegReq_t           slaveRegReq;
    FxU32                             reserved[128]; /* Make space for 128 request words */
  } optData;
} hwcRequest_t;

typedef struct hwcResponse_s
{
  FxU32 reserved;
  union resOptData
  {
    hwcControlDeviceConfigRes_t   deviceConfigRes;
    hwcControlPCIOpRes_t          pciOpRes;
    hwcControlSlaveRegRes_t       slaveRegRes;
    hwcControlFifoFuncsRes_t      fifoFuncsRes;
    hwcControlDriverInfoRes_t     driverInfoRes;
    FxU32                         reserved[128];  /* Make space for 128 response words */
  } optData;
} hwcResponse_t;

typedef struct hwcControl_s
{
  FxU32 which;
  hwcRequest_t *request;
  hwcResponse_t *response;
} hwcControl_t;

#endif

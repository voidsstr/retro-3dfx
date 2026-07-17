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
** File name:   GraphicsHALAvenger.c
**
** Description: HAL layer for MacOS Display Driver.
**
** $Header: GraphicsHALAvenger.c, 36, 10/26/00 6:54:53 AM, Stephane Huaulme$
**
** 10/22/00  1.3f5   - enabled (valid) most resolutions in Timing table
**                     (enabled refresh rates above 85Hz)
** 09/22/00  1.3f4   - fixed sli initialization
** 08/30/00  1.3f3   - fixed 75xx class Macintoshes
**                     blank the screen during sleep
** 08/05/00  1.3f2   - fixed Voodoo4 BIOS
** 08/05/00  1.3f1   - fixed for DDC when no device is available
**                     fixed dummy frame buffer bitsperpixel while in exclusive mode
**                     added version data (from 'vers') to be read by flashing utility
** 07/14/00  1.3f0   - implemented scaling for non native resolution under DVI
** 07/07/00  1.3b5   - disable deep sleep mode (it doesn't work)
** 07/07/00  1.3b4   - fixed 256 color bug at 16x12
** 07/04/00  1.3b3   - fixed legacy G3 bug (Slave mapping problem)
** 05/28/00  1.3b2   - default freq for Napalm is set to 166Mhz
**                     fixed AGP slot detection
** 05/28/00  1.3b1   - default freq for Napalm is set to 155Mhz
** 05/28/00  1.3b0   - voodoo3 beta 13 (version number maintainance)
** 05/27/00  1.3d5   - fixed slow ddc read
**                     fixed display detection in Napalm
**                     added Apple Hardware Key for Napalm
** 05/25/00  1.3d4   - added support for the new PowerManager (kcd)
** 04/19/00  1.3d3   - Fixed a sync polarity from EDID
** 04/19/00  1.3d2   - Fixed a bug that EDID timing translation & a bug in
**                     double DAC rate CRTC regs
** 04/17/00  1.3d1   - Fixed a bug that prevented VGA Misc Reg to be
**                     written (sync polarity)
** 04/11/00  1.3d0   - Updated CRTC generation function (better support for
**                     Napalm)
**
** 04/08/00  1.2f0   - voodoo3 beta 12 candidate
**                   - Dynamic Mode Tables, DVI support (limited)
**
**
*/

#define USE_HRM 1
#define scaled_desktop 1

/* Define this if you are using a broken version of the display manager with a two-monitor system
   and you are trying to debug the power management code. */
#define DISPLAY_MANAGER_POWER_BUG_WORKAROUND 0

#include <3dfx.h>
#include "macos8shim.h"
#define __MACERRORS__

#include "GraphicsPriv.h"
#include "GraphicsPrivHwc.h"
#include "GraphicsPrivData.h"
#include "GraphicsHAL.h"
#include "GraphicsCoreUtils.h"    // for GraphicsUtilMapSenseCodesToDisplayCode()
#include "GraphicsOSS.h"

#include <PCI.h>
#include <Devices.h>
#include <DriverServices.h>
#include <VideoServices.h>
#include <Errors.h>
#include <Kernel.h>
#include <Types.h>
#include <Displays.h>
#include <Video.h>
#include <Power.h>
#include <Math.h>

#include "gdx_debug.h"
#include "gdx_runtime.h"
#include "setmode.h"
#include "h3modeinit.h"
#include "mode.h"

#include "avenger_CLUT.h"
#include "avenger_interrupt_handling.h"
#include "avenger_display_detection.h"
#include "avenger_ddc.h"

#if 0
#define LED_OFF \
do { \
  FxU32 vidSerialParallelPort; \
  HWC_IO_LOAD(avengerHALData->bInfo.regInfo, vidSerialParallelPort, vidSerialParallelPort); \
  vidSerialParallelPort &= ~SST_SERPAR_GPIO_1; \
  HWC_IO_STORE(avengerHALData->bInfo.regInfo, vidSerialParallelPort, vidSerialParallelPort); \
} while(0)

#define LED_ON \
do { \
  FxU32 vidSerialParallelPort; \
  HWC_IO_LOAD(avengerHALData->bInfo.regInfo, vidSerialParallelPort, vidSerialParallelPort); \
  vidSerialParallelPort |= SST_SERPAR_GPIO_1; \
  HWC_IO_STORE(avengerHALData->bInfo.regInfo, vidSerialParallelPort, vidSerialParallelPort); \
} while(0)
#endif

/*
** HALReplacementDriverInfo
**  In the event that the driver is being superseded, the HAL will try and save this information so
**  that the driver's HAL which replaces it can attempt to come up in the same state, if possible.
**  This will help to minimize the visual artifacts that a users sees.
**
**  However, Avenger's behavior is so straight forward, that very little state information needs
**  to be maintained.
*/
typedef struct HALReplacementDriverInfo HALReplacementDriverInfo;
struct HALReplacementDriverInfo
{
  UInt8 graphicsMode;      /* Value of H4Wax's 'graphicsMode' register for current DisplayModeID */
};

/*
** Naming conventions for functions:
**    - GraphicsHALxxx  - functions which all HALs must implement.  These have external scope.
**    - {Avenger}xxx  - functions which are strictly private to a specific HAL.
*/




/* Avenger specific forward declarations. */

extern GDXErr AvengerSetMode(FxU32 width, FxU32 height, FxU32 refresh);
static void   AvengerSetModeSlave(FxU32 chipNumber, FxU32 width, FxU32 height, FxU32 refresh);
static void   AvengerEnableMemory( FxU32 chipNumber );
static void   AvengerDisableMemory( FxU32 chipNumber );
static void   AvengerEnableIO( FxU32 chipNumber );
static void   AvengerDisableIO( FxU32 chipNumber );
static void	  AvengerInitDesktopScaler(FxU16 width,FxU16 height, DepthMode depth );
static void   AvengerDeviceSleep(AvengerHALData *avengerHALData);
static void   AvengerDeviceWakeup(AvengerHALData *avengerHALData);
static FxU32  AvengerIsPCI(void);
static FxBool AvengerCalculatePPLvalue( FxU32 freq, FxU16 * m, FxU16 * n, FxU16 * k);
static void   AvengerInitPCI(hwcRegInfo regInfo );

static FxU32  NapalmMapSlave(FxU32 chipNumber);
static void   NapalmResetSlave(FxU32 chipNumber);
static void   NapalmResetSlaves(void);
extern void   NapalmSLIAA(PDEVTABLE master, hwcControlSLIAAReq_t *req);
extern void   NapalmSetVideoModeSlave(PDEVTABLE pSlave);

pascal OSStatus _XDoDriverPowerManagement(UInt32 message, void *param, UInt32 refCon, RegEntryID *regEntryID);




#if HARDWARE_CURSOR
#define kCursorVRAM 4096            /* Might as well be a page. */
#else
#define kCursorVRAM 0
#endif

#define kFifoVRAM  256*1024          /* Around the same size as the standard 2D fifo */

#define	HasPowerHandlerSupport()		\
	( ( ( Ptr ) AddDevicePowerHandler ) != ( ( Ptr ) kUnresolvedCFragSymbolAddress ) )

#define SlotInfoSize 0x58
char  gSlotInfo[16][SlotInfoSize];

AvengerHALData  gAvengerHALData;    /* Persistant Avenger specific data storage */

static FxU8 logicalToPhysical5to5[32];
static FxU8 logicalToPhysical5to6[32];
static FxU8 logicalToPhysical6to6[64];
static FxU8 redRemap[32];
static FxU8 greenRemap[64];

/* Often-used configuration space registers */
#define kMemBaseAddr0 16
#define kMemBaseAddr1 20
#define kIOBaseAddr0  24
#define kCfgInitEnable 64
#define kCfgPciDecode 72
#define kCfgStatus    76

/* The version of Avenger Graphics driver */
enum
{
#if DEBUG
  kMajorRev = 9,
  kMinorAndBugRev = 9,
  kStage = 0x20,      /* 0x80 = Final, 0x60 = Beta, 0x40 = Alpha, 0x20 = Development */
  kNonRelRev = 0
#else
  kMajorRev = 1,
  kMinorAndBugRev = 3,
  kStage = 0x80,      /* 0x80 = Final, 0x60 = Beta, 0x40 = Alpha, 0x20 = Development */
  kNonRelRev = 5      /* if you change this value, update the header to indicate what changed */
#endif
};



/*
** DriverDescription
**   This structure describes the native driver.  Since this is a Graphics driver, the driver should
**  be loaded by the DisplayMgr's expert loader.  This is indicated by the 'driverOSRuntimeInfo'
**  with the 'kDriverIsUnderExpertControl' flag set.
*/
DriverDescription TheDriverDescription =
{
  /* driverDescSignature */
  kTheDescriptionSignature,            /* Signature of DriverDescription */

  /* driverDescVersion */
  kInitialDriverDescriptor,            /* Version of this data structure */

  /* driverType */
  {
#if NOROM
#  if H3
    "\ppci121a,2",
#  elif H4 /* H4 */
#    if GDX_AGP_SUPPORT
    "\ppci121a,34",                  /* device name must match in Devicetree */
#    else
//    "\ppci121a,36",                  /* device name must match in Devicetree */
//    "\ppci121a,39",    // pci dvi board
    "\ppci121a,40",      // agp dvi board
#    endif        
#  else
    "\ppci121a,2",
#  endif
#else /* !DEBUG */
#  if H3
    "\p3dfx,Banshee",                  /* device name must match in Devicetree */
#  elif H4
    "\p3dfx,Voodoo3",                  /* device name must match in Devicetree */
#  elif H5
#    if V5_5000
    "\p3dfx,Voodoo5",                  /* device name must match in Devicetree */
#    else
    "\p3dfx,Voodoo4",                  /* device name must match in Devicetree */
#    endif    
#  endif
#endif /* !DEBUG */
    kMajorRev, kMinorAndBugRev, kStage, kNonRelRev,  /* Major, Minor, Stage, Rev */
  },


  /* driverOSRuntimeInfo                // OS Runtime Requirements of Driver */
    {
      kDriverIsOpenedUponLoad + kDriverIsUnderExpertControl,  /* Runtime Options */
#if H5      
  #if V5_5000
    "\p.Display_Video_Voodoo5",
  #else
    "\p.Display_Video_Voodoo4",
  #endif
#elif H3
  "\p.Display_Video_Banshee",
#else
  "\p.Display_Video_Voodoo3",
#endif
    },


 /* driverServices                  // Apple Service API Membership               */

    1,                              /* Number of Services Supported               */

    kServiceCategoryNdrvDriver,     /* We support the 'ndrv' category             */

    kNdrvTypeIsVideo,               /* Type1                                      */

    1,0,0,0                         /* majorRev, minorAndBugRev, stage, nonRelRev */

};

pascal OSErr myInitialize(const CFragInitBlock *	initBlock);

pascal OSErr myInitialize(const CFragInitBlock *	initBlock)
{
  OSErr err = 0;

#if DEBUG  
  #if DEBUG_USE_PRINTF
    gdx_debug_init();
  #else
    SysDebug();
  #endif
#endif  
  
  return err;
}





/*
**=====================================================================================================
**
** GraphicsHALInitPrivateData()
**  Allocate and initialize the HAL private data
**
**    -> regEntryID  The NameRegistry ID for this device.
**
**    <> replacingDriver
**    On input, this indicates whether the Core got a 'kInitializeCommand' or a 'kReplaceCommand'.
**    These commands are similar, but with subtle differences.  A 'kInitializeCommand' is issued
**    if no version of this driver has been previously loaded, whereas a 'kReplaceCommand' is
**    issued if a previous version of the driver has been loaded, but subsequently superseded.
**
**    If 'false', then a 'kInitializeCommand' had been recieved by the Core, and the HAL should
**    do a full hardware initialization.
**
**    If 'true', then a 'kReplaceCommand' had been received by the Core, and the HAL can attempt
**    to configure itself to its state prior to it being superceded.
**
**    On output, this allows the HAL to override the Core's default behavior if it chooses to do so.
**
**    If 'false' then the HAL is signinaling the Core that it is unable to re-configure itself to
**    its state prior to being superseded, and the core will continue as if a 'kInitializeCommand'
**    had occurred.
**
**    If 'true', then the HAL was able to re-configure itself in the event of being replaced,
**    and the Core will proceeded accordingly.
**
**=====================================================================================================
*/
#define kNumAssignedAddresses 4

GDXErr GraphicsHALInitPrivateData( DriverRefNum refNum, const RegEntryID *regEntryID, Boolean *replacingDriver)
{
#define FN_NAME "GraphicsHALInitPrivateData"

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  GDXErr err = kGDXErrUnableToAllocateHALData;  /* Assume failure */

  PCIAssignedAddress assignedAddresses[kNumAssignedAddresses];    /* There should be three "phys-addr size" pairs */
  UInt32 applAddress[kNumAssignedAddresses];                      /* There should be three logical addresses */
  UInt32 baseRegister0Index;            /* Entry in APPL,address for Base Register 0 */
  UInt32 baseRegister1Index;            /* Entry in APPL,address for Base Register 1 */
  UInt32 baseRegister2Index;            /* Entry in APPL,address for Base Register 2 */
  UInt32 baseRegister3Index;            /* Entry in APPL,address for Base Register 3 */
  UInt16 commandRegister;              /* Enable Avenger's memory if everything ok */
  OSErr osErr;

  Nanoseconds nanoseconds;                          /* Convert 260ns into Absolute Time */
  void *hardwareBaseAddress;
  UInt32 halPreferences;                            /* Used for retrieving HAL prefs from NVRAM. */
  FxU16 tempReg;
  FxU32 i;
  FxU8 deviceRev;

  FxU32 versionFlag[] = { 'vers',
                          kMajorRev<<24 | kMinorAndBugRev<<16 | kStage<<8 | kNonRelRev,
#if H3
                          'bans'
#elif H4
                          '  V3'
#elif H5
  #if V5_5000
                          '  V5'
  #else
                          '  V4'
  #endif
#endif    
                          };


  LOG_ENTRY(1);
  
  GDBG_PRINTF("GraphicsHALInitPrivateData()\n");

  /* this code is here to make sure that versionFlags data is in the ROM.
     it is used to identify the ROM version by inspecting the binary (before flashing)
  */
  logicalToPhysical5to5[0] = (FxU8) versionFlag[0];
  
  /* Build 'magic' remapping tables.  */
  /* FrameBuffer:    rrrrrggggggbbbbb */
  /* MacOS:          arrrrrgggggbbbbb */
  for(i = 0; i < 32; i++) {
    FxU32 index = (i << 3) + (i >> 2);              /* CLUT entry to be written */
    logicalToPhysical5to5[i] = index;
  }
  for(i = 0; i < 64; i++) {
    logicalToPhysical6to6[i] = (i << 2) + (i >> 4);
  }
  for(i = 0; i < 32; i++) {
    logicalToPhysical5to6[i] = logicalToPhysical6to6[(i << 1) + (i >> 4)];
    greenRemap[i] = logicalToPhysical5to6[i];
    greenRemap[i+32] = logicalToPhysical5to6[i];
  }
  for(i = 0; i < 16; i++) {
    redRemap[i] = logicalToPhysical5to5[(i<<1) + (i>>3)];
    redRemap[i+16] = logicalToPhysical5to5[(i<<1) + (i>>3)];
  }

  /* Save the regEntryID, */
  RegistryEntryIDCopy(regEntryID, &avengerHALData->regEntryID[0]);

  avengerHALData->refNum = refNum;

  /* Avenger is on a PCI card.  Avenger contains the H4Wax graphics controller, and the
  ** H4Clut CLUT.  In this sample code, it is assumed that the Avenger PCI card uses 2
  ** Base Registers.  This illustrates how the "assigned-address" property must be examined to
  ** determine which "AAPL,address" 32 bit entry corresponds with each Base Register.
  **
  **      Base Register 0 (offset 0x10 from the configuration registers) is the base address for
  **      the hardware registers.
  **
  **      Base Register 1 (offset 0x14 from the configuration registers) is the base address for
  **      the VRAM.
  **
  ** Avenger's Open Firmware startup code created a "reg" property that described
  **      1) the configuration registers
  **      2) Base Register 0
  **      3) Base Register 1
  ** Open Firmware creates an "assigned-address" property that describes Base Register 0 and
  ** Base Register 1
  */

  /* Get the kPCIAssignedAddressProperty.  (Note that a multiplier of 2x is used since since
     there should be 4 assigned-addresses */
  err = GraphicsOSSGetProperty(&avengerHALData->regEntryID[0], kPCIAssignedAddressProperty,
      assignedAddresses, sizeof(PCIAssignedAddress)*kNumAssignedAddresses);

  /* If there is an error, that means Open Firmware was unable to allocate the memory space that was
     requested.  Hence, Avenger is unusable at this time and the driver should quit. */
  if (err) {
    //DebugStr("\pGraphicsOSSGetProperty(kPCIAssignedAddressProperty) barfed");
    goto ErrorExit;
  }
  /* Since the entries of the "assigned-address" property does not necessarily match the order of the
  ** entries in the "reg" property, examine the rrrrrrrr bits in each physHi to determine which
  ** "phys-addr size" pair matches each Base Register.
  ** (Base Register 0 (offset at 0x10 from the configuration registers) has rrrrrrrr = 0x10 etc...)
  */
  for(i = 0; i < kNumAssignedAddresses; i++) {
    if(assignedAddresses[i].registerNumber == 0x10) {
      baseRegister0Index = i;
    } else if(assignedAddresses[i].registerNumber == 0x14) {
      baseRegister1Index = i;
    } else if(assignedAddresses[i].registerNumber == 0x18) {
      baseRegister2Index = i;
    } else if(assignedAddresses[i].registerNumber == 0x30) {
      baseRegister3Index = i;
    }
  }

  /* The order of the "phys-addr size" pairs in the "assigned-address" property match the order
  ** of the logical addresses in the "AAPL,address" property.  Go find the logical addresses.
  */
  err = GraphicsOSSGetProperty(&avengerHALData->regEntryID[0], "AAPL,address", applAddress, 4 * kNumAssignedAddresses);

  if (err)              /* Should NEVER be an error if gotten this far */
  {
    goto ErrorExit;
  }
  /* Have successfully found all base addresses.
  ** Enable Avenger's memory space.  Always do a read-modify-write when hitting configuration
  ** registers.
  */
  avengerHALData->hrmGetExtension = 0;
  avengerHALData->hrmFifoUpdate = 0;
  avengerHALData->powerState = 3;

  /* non native DVI resolution support */
  avengerHALData->scalerNeeded = false;
  avengerHALData->scaler2x = false;

  /* edid info */
  avengerHALData->edidWasRead = false;
  avengerHALData->edidIsValid = false;
  
  AvengerEnableMemory(0);
  AvengerEnableIO(0);
#if H5
  /* Enable 32MB memory decode for registers and 256MB memory decode for memory. */  
  err = ExpMgrConfigWriteByte(&avengerHALData->regEntryID[0], (LogicalAddress) kCfgPciDecode, 
                           SST_PCI_MEMBASE0_DECODE_32MB |
	                       SST_PCI_MEMBASE1_DECODE_256MB |
	                       SST_PCI_IOBASE0_DECODE_256);  
#endif

  /* Set up HWC PCI info. */
  avengerHALData->bInfo.pciInfo.initialized = FXTRUE;
  ExpMgrConfigReadWord(&avengerHALData->regEntryID[0], (void *)0, &tempReg);
  avengerHALData->bInfo.pciInfo.vendorID = tempReg;
  ExpMgrConfigReadWord(&avengerHALData->regEntryID[0], (void *)2, &tempReg);
  avengerHALData->bInfo.pciInfo.deviceID = tempReg;
  avengerHALData->bInfo.pciInfo.pciBaseAddr[0] = applAddress[baseRegister0Index];
  avengerHALData->bInfo.pciInfo.pciBaseAddr[1] = applAddress[baseRegister1Index];
  avengerHALData->bInfo.pciInfo.pciBaseAddr[2] = applAddress[baseRegister2Index];
  avengerHALData->bInfo.pciInfo.pciBaseAddr[3] = applAddress[baseRegister3Index];

  LOG_PRINTF1(0,"pciBaseAddr[0]: %08lx\n",avengerHALData->bInfo.pciInfo.pciBaseAddr[0]);
  LOG_PRINTF1(0,"pciBaseAddr[1]: %08lx\n",avengerHALData->bInfo.pciInfo.pciBaseAddr[1]);
  
  /* Set up HWC Board info */
  avengerHALData->bInfo.regInfo.initialized = FXTRUE;
  avengerHALData->bInfo.regInfo.ioMemBase = applAddress[baseRegister0Index] + SST_IO_OFFSET;
  avengerHALData->bInfo.regInfo.cmdAGPBase = applAddress[baseRegister0Index] + SST_CMDAGP_OFFSET;
  avengerHALData->bInfo.regInfo.waxBase = applAddress[baseRegister0Index] + SST_2D_OFFSET;
  avengerHALData->bInfo.regInfo.sstBase = applAddress[baseRegister0Index] + SST_3D_OFFSET;
  avengerHALData->bInfo.regInfo.lfbBase = applAddress[baseRegister0Index] + SST_LFB_OFFSET;
  avengerHALData->bInfo.regInfo.rawLfbBase = applAddress[baseRegister1Index];
  avengerHALData->bInfo.regInfo.ioPortBase = applAddress[baseRegister2Index];
  ExpMgrConfigReadByte(&avengerHALData->regEntryID[0], (LogicalAddress) 0x8, &deviceRev);
  avengerHALData->bInfo.devRev = deviceRev;

  /* FIXME */
  avengerHALData->isMaster = FXTRUE;
  avengerHALData->numChips = 1;
  avengerHALData->supportsAGP = FXFALSE;
  avengerHALData->isPCI = AvengerIsPCI();
  avengerHALData->powerLevel = kPMDevicePowerLevel_On;
  
  /* Hack for h3cinit and sliaa code */
  avengerHALData->devTable[0].IoBase = avengerHALData->bInfo.regInfo.ioPortBase;
  avengerHALData->devTable[0].dwVendorDeviceID =avengerHALData->bInfo.pciInfo.deviceID;
  avengerHALData->devTable[0].dwType = SLI_AA_MASTER_DEVICE;
  avengerHALData->devTable[0].dwUnitNum = 0;
  avengerHALData->devTable[0].dwDevFunc = 0;
  avengerHALData->devTable[0].dwBus = 0;
  avengerHALData->devTable[0].dwPCI = avengerHALData->isPCI;
  avengerHALData->devTable[0].lpDriverData = NULL;
  avengerHALData->devTable[0].RegBase[HWINFO_SST_IOREGS_INDEX] = avengerHALData->bInfo.regInfo.ioMemBase;
  avengerHALData->devTable[0].RegBase[HWINFO_SST_CMDFIFOREGS_INDEX] = avengerHALData->bInfo.regInfo.cmdAGPBase;
  avengerHALData->devTable[0].RegBase[HWINFO_SST_2DREGS_INDEX] = avengerHALData->bInfo.regInfo.waxBase;
  avengerHALData->devTable[0].RegBase[HWINFO_SST_3DREGS_INDEX] = avengerHALData->bInfo.regInfo.sstBase;
  avengerHALData->devTable[0].PhysMemBase[0] = applAddress[baseRegister0Index];
  avengerHALData->devTable[0].PhysMemBase[1] = applAddress[baseRegister1Index];
  avengerHALData->devTable[0].PhysMemBase[2] = applAddress[baseRegister2Index];
  avengerHALData->devTable[0].LfbBase = applAddress[baseRegister1Index]; // Not sure on this yet...
  avengerHALData->devTable[0].pSlave = NULL;
  
#if V5_5000
  if(NapalmMapSlave(1)) {
    avengerHALData->numChips = 2;  
  }

  for(i = 1; i < avengerHALData->numChips; i++) {
    avengerHALData->slaveRegInfo[i].ioMemBase = avengerHALData->slaveMemBase0[i] + SST_IO_OFFSET;
    avengerHALData->slaveRegInfo[i].cmdAGPBase = avengerHALData->slaveMemBase0[i] + SST_CMDAGP_OFFSET;
    avengerHALData->slaveRegInfo[i].waxBase = avengerHALData->slaveMemBase0[i] + SST_2D_OFFSET;
    avengerHALData->slaveRegInfo[i].sstBase = avengerHALData->slaveMemBase0[i] + SST_3D_OFFSET;
    avengerHALData->slaveRegInfo[i].lfbBase = 0;
    avengerHALData->slaveRegInfo[i].rawLfbBase = avengerHALData->slaveMemBase1[i];
    avengerHALData->slaveRegInfo[i].ioPortBase = avengerHALData->slaveIOPortBase[i];
    
    /* Create DEVTABLE for slave chip so we can share SLI/AA setup code. */
    avengerHALData->devTable[i].IoBase = avengerHALData->slaveRegInfo[i].ioPortBase;
    avengerHALData->devTable[i].dwVendorDeviceID = avengerHALData->bInfo.pciInfo.deviceID;
    avengerHALData->devTable[i].dwType = SLI_AA_SLAVE_DEVICE;
    avengerHALData->devTable[i].dwUnitNum = i;
    avengerHALData->devTable[i].dwDevFunc = i;
    avengerHALData->devTable[i].dwBus = 0;
    avengerHALData->devTable[i].dwPCI = avengerHALData->isPCI;
    avengerHALData->devTable[i].lpDriverData = NULL;
    avengerHALData->devTable[i].RegBase[HWINFO_SST_IOREGS_INDEX] = avengerHALData->slaveRegInfo[i].ioMemBase;
    avengerHALData->devTable[i].RegBase[HWINFO_SST_CMDFIFOREGS_INDEX] = avengerHALData->slaveRegInfo[i].cmdAGPBase;
    avengerHALData->devTable[i].RegBase[HWINFO_SST_2DREGS_INDEX] = avengerHALData->slaveRegInfo[i].waxBase;
    avengerHALData->devTable[i].RegBase[HWINFO_SST_3DREGS_INDEX] = avengerHALData->slaveRegInfo[i].sstBase;
    avengerHALData->devTable[i].PhysMemBase[0] = avengerHALData->slaveMemBase0[i];
    avengerHALData->devTable[i].PhysMemBase[1] = avengerHALData->slaveMemBase1[i];
    avengerHALData->devTable[i].PhysMemBase[2] = avengerHALData->slaveIOPortBase[i];
    avengerHALData->devTable[i].LfbBase = avengerHALData->slaveMemBase1[i];
    avengerHALData->devTable[i].pSlave = NULL;
    avengerHALData->devTable[i-1].pSlave = &avengerHALData->devTable[i];
  }

#if DEBUG
  {
    FxU32 theVendorDevice;
    
    theVendorDevice = 0;
    ExpMgrConfigReadLong(&avengerHALData->regEntryID[1], 0, &theVendorDevice);
    LOG_PRINTF1(0,"Vendor / Device: 0x%08x\n",theVendorDevice);

    LOG_PRINTF1(0,"numChips = %d\n",avengerHALData->numChips);
  }
#endif

#endif
  
  /* FIXME - Well, sortof.  It turns out we can't support anything but 256MB
     allocations on MacOS, so these are pretty much always going to be the same
     no matter what board we have.  Fortunately there are no plans to build a
     board with >32MB of RAM per chip. */
  if(IS_NAPALM(avengerHALData->bInfo.pciInfo.deviceID)) {
    avengerHALData->swizzleOffsets[0] = 0x00000000;
    avengerHALData->swizzleOffsets[1] = 0x04000000;
    avengerHALData->swizzleOffsets[2] = 0x08000000;
    avengerHALData->swizzleOffsets[3] = 0x0c000000;
  }
      
  /* Set up default command fifo location */
  avengerHALData->bInfo.fifoInfo.agpFifo = FXFALSE;
  avengerHALData->bInfo.fifoInfo.fifoStart = 0;
  avengerHALData->bInfo.fifoInfo.fifoLength = kFifoVRAM;

  if (*replacingDriver) {
    /* Important Implementation Note:  The exact behavior of what should happen during driver
    ** replacement is largely dependent of the implementation of the driver, and the reason for
    ** it being replaced.
    ** Under the ideal circumstances, a driver should be able to be replaced without having to
    ** shut down the raster or reprogram the hardware.  This will prevent visible flashes on the
    ** display during the replacement process.
    **
    ** When a driver is being updated to incorporate new features (as opposed to bug fixes), then
    ** replacement can usually occur without flashes.  This is because by default, the
    ** Driver Loader Library (DLL) does not let QuickDraw know that a new driver got loaded, so
    ** QuickDraw doesn't issue commands to gray the screen, etc. which result in 'flashes.'
    **
    ** However, this default behavior is only acceptable if the new driver comes up in the same
    ** bit depth and has the same 'base address' and 'rowbytes' as the driver it replaced.  If
    ** this is not the case, then the DLL needs to be informed that a full initialization of all
    ** the QuickDraw variables is required.  The DLL can be informed by setting uncommenting
    ** the following lines:
    **
    ** UInt32 needFullInit  = 1;
    ** err = GraphicsOSSSaveProperty(&sixty6HALData->regEntryID, "needFullInit",
    **      &needFullInit, sizeof(needFullInit), kOSSPropertyVolatile);
    */

    /* This driver is replacing a previous version.  Therefore, to try and minimize the visual
    ** effects of this driver being loaded, retrieve whatever state information the superseded
    ** driver left behind.
    **
    ** Note:  For Avenger, this is largely just a place holder.  Your real hardware probably
    ** has more extensive requirements.
    */
    HALReplacementDriverInfo replacementDriverInfo;

    err = GraphicsOSSGetProperty(&avengerHALData->regEntryID[0], "HALReplacementInfo",
        &replacementDriverInfo, sizeof(HALReplacementDriverInfo));

    if (!err) {
      /* avengerHALData->graphicsMode = replacementDriverInfo.graphicsMode; */

      /* Have now grabbed all the old state information.  The new driver knows what bugs the old
      ** driver had.  Examine the state information to see if it is necessary to do a full hw

      ** initialization.  For example, one displayModeID was known to be bad...the hw was
      ** programmed incorrectly for that resolution.
      ** if (brokenDisplayModeID == avengerHALData->displayModeID)
      **    *replacingDriver = false;
      */

      *replacingDriver = false;
    } else {
      /* An error occurred while attempting to retrieve the previous state information, so
         report that a full initialization is required. */

      *replacingDriver = false;
      err = kGDXErrNoError;
    }
  }

  /* Always try to delete the HALReplacementDriverInfo in the NameRegistry, so it doesn't cause
     confusion later on. */
  (void) GraphicsOSSDeleteProperty(&avengerHALData->regEntryID[0], "HALReplacementInfo");

  /*
  ** Avenger might want to look at the HAL specfic data stored in NVRAM.
  ** The last 4 bytes (each device is allowed 8 bytes of NVRAM) is private to each HAL.
  ** GraphicsOSSGetHALPref() automatically fetches the last 4 bytes for the HAL.
  */
#if !H3
  err = GraphicsOSSGetHALPref(&avengerHALData->regEntryID[0], &halPreferences);
  if (!err) {
    /*
    ** Examine the halPreferences stored in NVRAM.  The HAL portion of the NVRAM is private to the
    ** HAL (the Core never examines or messes with it) and can have any meaning defined by the HAL.
    ** If the HAL wishes to save data, it can use the routine GraphicsOSSSetHALPref().
    ** NOTE: the HAL should do a read-modify-write if it wishes to change the data.  This allows
    ** disk based drivers that replace the ROM based drivers to define new HAL data.  Ensures that
    ** the ROM based driver (always run at startup before the disk based driver) doesn't smash
    ** the data that the disk based driver saved.
    ** Grab clock speed info.  0 means "default", which we ought to get upon reset.
    */
    LOG_PRINTF1(0,"Got old halPreferences: %08lx\n",halPreferences);
    avengerHALData->halPreferences = halPreferences;
    avengerHALData->grxClock = (halPreferences & HALDATA_GRXCLOCK_MASK) >> HALDATA_GRXCLOCK_SHIFT;
    
    if(avengerHALData->grxClock == 0) {
#if H5    
      avengerHALData->grxClock = 166; /* Set default speed. */      
#elif V3_3000 
      avengerHALData->grxClock = 166; /* Set default speed. */
#else 
      avengerHALData->grxClock = 143; /* Set default speed. */
#endif
    }
  } else {
    FxU32 halPreferences;
    LOG_PRINTF(0,"Creatinng new halPreferences\n");

    /* Hmm... couldn't get prefs so we set up the new defaults. */
#if H5    
    avengerHALData->grxClock = 166; /* Set default speed. */      
#elif V3_3000 
    avengerHALData->grxClock = 166; /* Set default speed. */
#else 
    avengerHALData->grxClock = 143; /* Set default speed. */
#endif
    avengerHALData->halPreferences = (avengerHALData->grxClock << HALDATA_GRXCLOCK_SHIFT);
  }
    
  /* Store prefs back to nvram as a "hint". */
  GraphicsOSSSetHALPref(&avengerHALData->regEntryID[0], avengerHALData->halPreferences);
#endif
#if DISPLAY_MANAGER_POWER_BUG_WORKAROUND
  if( HasPowerHandlerSupport() ) {
    err = AddDevicePowerHandler( &avengerHALData->regEntryID[0], 
	                             _XDoDriverPowerManagement, 
	                             (UInt32)avengerHALData, 
	                             NULL );
  }
#endif
  err = kGDXErrNoError;               /* Everything ok */

  /* test for Apple Key */
#if H5
  {
    FxU32 theStrapinfo1;

    HWC_IO_STORE( avengerHALData->bInfo.regInfo, strapInfo, 1 );
    HWC_IO_LOAD( avengerHALData->bInfo.regInfo, strapInfo, theStrapinfo1 );
    
    if ( !(theStrapinfo1 & BIT(22)) )
      err = kGDXErrDriverCannotRun;
  }
#endif

ErrorExit:
  LOG_EXIT(1,err);
  return (err);
#undef FN_NAME
}





/*
**=====================================================================================================
**
** GraphicsHALKillPrivateData()
**  Dispose of the HAL's private data
**
**=====================================================================================================
*/
void GraphicsHALKillPrivateData()
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  RegistryEntryIDDispose(&avengerHALData->regEntryID[0]);

}





/*
**=====================================================================================================
**
** GraphicsHALOpen()
**  It is possible for the driver to be opened and closed many times.
**   The HAL private data has been setup, so just initialize the hardware into
**  the state that is should be in on startup.  This should be the complete initialization
**  to get the HW into the desired state for the amount of VRAM that is in the system and
**  any other hardware specfic stuff that the HAL cares about. The sense lines, for example, should
**  be able to be read and tweaked to determine the type of the connected monitor.
**  No programming to set up the raster for a givin 'DisplayModeID' or 'DepthMode' is necessary at
**  this point.
**
**    -> spaceID      The AddressSpaceID for this device.
**    -> replacingDriver  'true' if the HAL should behave as if the driver is being replaced,
**              'false' otherwise.
**
**=====================================================================================================
*/
GDXErr GraphicsHALOpen(const AddressSpaceID spaceID, Boolean replacingDriver)
{
#define FN_NAME "GraphicsHALOpen"
#define FN_LEVEL 0

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 ioPortAddress = avengerHALData->bInfo.regInfo.ioPortBase,
        pciInit0;
  const FxU32 sgramMode     = 0x37,
              sgramMask     = 0xFFFFFFFF,
              sgramColor     = 0x00000000;
  FxU32 lfbMemoryConfig;

  GDXErr err = kGDXErrNoError;

  LOG_ENTRY(1);
  
  AvengerEnableMemory(0);
  AvengerEnableIO(0);

#if GDX_AGP_SUPPORT && 0
  avengerHALData->agpEnabled = FXFALSE;  
  /* It would be nice to do this automatically rather than needing a seperate binary. */
  /* I just have to figure out how to look up the addresses of these routines dynamically. */
  err = AGPNewMemory(&avengerHALData->agpAddress, 1024*1024, false);
  if(err == noErr) {
    err = AGPCommitMemory(&avengerHALData->agpAddress, false);
    if(err == noErr) {
      avengerHALData->agpEnabled = FXTRUE;
    } else {
      AGPDisposeMemory(&avengerHALData->agpAddress);
    }
  }
  err = noErr;
#endif

  AvengerInitPCI( avengerHALData->bInfo.regInfo );

  /* Reset hardware, program PLL */
  h3InitResetAll(ioPortAddress);
#if defined(H4) || defined(H5)
  h4InitPlls(ioPortAddress,  SST_DEVICE_ID_H4, avengerHALData->grxClock);
#else
  h3InitPlls(ioPortAddress, 100, 100);
#endif

  AvengerEnableMemory(0);
  AvengerEnableIO(0);

  /* read back the memory size, since we don't know it under DOS  (see hwcInit) - dwj */
  bInfo->h3Mem = h3InitSgram(ioPortAddress, sgramMode, sgramMask, sgramColor, avengerHALData->bInfo.pciInfo.deviceID, NULL);
  //bInfo->h3Mem = 32; //h3InitGetMemSize(ioPortAddress);

  /* Set up memory allocation stuff */
  avengerHALData->memBlockSize = VIDMEM_GRANULARITY;
  avengerHALData->memBlockMask = VIDMEM_GRANULARITY_MASK;

  /* No tiled memory yet... */
  avengerHALData->pciStride = 0;
  avengerHALData->hwStride = 0;
//  avengerHALData->tileMark = bInfo->h3Mem * 1024 * 1024;
  avengerHALData->tileMark = - 1;
  avengerHALData->swizzleOffset = 0;
  
  /* Do initial VGA setup - make sure legacy decode is disabled. */
  h3InitVga(ioPortAddress, FXFALSE);

  /* Start Tiled mode at 4MB boundary instead of default. -- FIXME, doesn't work for modes >1024x768 very well */
  lfbMemoryConfig = SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE((avengerHALData->tileMark >> 12)) | SST_RAW_LFB_ADDR_STRIDE_1K | 0xa0000;
  HWC_IO_STORE(bInfo->regInfo, lfbMemoryConfig, lfbMemoryConfig);

#if H5
	{
		FxU32 miscInit1;
		HWC_IO_LOAD(bInfo->regInfo,miscInit1,miscInit1);
		miscInit1 &= ~( SST_BYTE_SWIZZLE_SELECT | SST_BYTE_SWIZZLE_ENABLE);
		miscInit1 |=  SST_BYTE_SWIZZLE_64MB | SST_BYTE_SWIZZLE_ENABLE;
		HWC_IO_STORE(bInfo->regInfo,miscInit1,miscInit1);
	}
#endif	

  /*
  ** Make sure VBL interrupts are enabled.  Well, sortof.
  ** I don't actually enable interrupts until we have been programmed with a
  ** valid display mode...  this may not be necessary though since VSYNC interrupts
  ** probably won't happen until then anyway.
  */
#if ENABLE_INTERRUPTS  
  avengerHALData->intrCrtl = SST_INTR_PCI_INTA;
  HWC_SST_STORE(bInfo->regInfo,intrCtrl,avengerHALData->intrCrtl);
  HWC_SST_STORE(bInfo->regInfo,status,0);
  HWC_IO_LOAD(avengerHALData->bInfo.regInfo, pciInit0, pciInit0);
  pciInit0 |= SST_PCI_INTERRUPT_ENABLE;
  HWC_IO_STORE(avengerHALData->bInfo.regInfo, pciInit0, pciInit0);
  HWC_IO_STORE(avengerHALData->bInfo.regInfo, status, 0);
#endif

#if V5_5000 && 1
  if(avengerHALData->numChips > 1) {
    FxU32 i, ioPortBaseSlave;
        
    for(i = 1; i < avengerHALData->numChips; i++) {
      NapalmResetSlave(i);
    }
  }
#endif

  /*
  ** No real hardware initialization needs to occur, since things should (according to the
  ** arbitrary ERSs) come in in a polite state.  Additionally, things were left in a polite
  ** state in the event of a previous GraphicsHALClose().
  */
ErrorExit:
  LOG_EXIT(1,err);
  return(err);
#undef FN_NAME
#undef FN_LEVEL
}

/*
**=====================================================================================================
**
** GraphicsHALClose(const AddressSpaceID spaceID)
**  Upon close, there are no major requirements, since the majority of the work will be handled
**  elsewhere.
**
**
**=====================================================================================================
*/
GDXErr GraphicsHALClose(const AddressSpaceID spaceID)
{
#define FN_NAME "GraphicsHALClose"
  GDXErr err = kGDXErrNoError;
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  
  LOG_ENTRY(1);
#if GDX_AGP_SUPPORT && 0 
  if(avengerHALData->agpEnabled) {
    AGPDecommitMemory(&avengerHALData->agpAddress);
    AGPDisposeMemory(&avengerHALData->agpAddress);
    avengerHALData->agpEnabled = FXFALSE;
  }
#endif
  
ErrorExit:
  LOG_EXIT(1,err);
  return err;
#undef FN_NAME
}

/*
**=====================================================================================================
**
** GraphicsHALTerminate()
**
**    -> superseded
**    'true' if current driver is going to be superseded by another driver, 'false' otherwise.
**    If 'true', the current driver can choose to save any state that the replacement driver
**    may need..if it wants to keep raster going.
**
**    'false' no driver is going to replace it.  In that event, it should stop the raster and
**    leave hardware in a 'polite' state.
**
**=====================================================================================================
*/
GDXErr GraphicsHALTerminate(Boolean superseded)
{
#define FN_NAME "GraphicsHALTerminate"
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  GDXErr err = kGDXErrNoError;                    /* Assume success */

  LOG_ENTRY(1);
  
  if (superseded) {
    /* Driver is being superseded, so save state information for the replacement driver. */

    HALReplacementDriverInfo replacementDriverInfo;

    /* replacementDriverInfo.graphicsMode = avengerHALData->graphicsMode; */

    err = GraphicsOSSSaveProperty(&avengerHALData->regEntryID[0], "HALReplacementInfo",
              &replacementDriverInfo, sizeof(HALReplacementDriverInfo), kOSSPropertyVolatile);
#if 0
    if (err)
      *h4Wax->graphicsMode = 0xFF;  /* Causes Comso's state machine to stop and blanks display */
#endif
  } else {
#if 0
    /* Driver is going away for good, so blank turn off state machines. */
    *h4Wax->graphicsMode = 0xFF;    /* Causes Comso's state machine to stop and blanks display */
#endif
  }
  LOG_EXIT(1,err);
  return err;
#undef FN_NAME  
}


/*
**=====================================================================================================
**
** GraphicsHALGetVBLInterruptRoutines()
**  The OSS encapsulates how interrupts are handled by the system.  This routine supplies
**  that OSS with the HAL's interrupt routines that follow the OSS conventions.  Hopefully,
**  if the OS changes, only the OSS will need to change.
**
**    <- installVBLInterrupts
**    'true'  if the HAL's interrupt scheme can match the OSS's scheme. i.e. the HAL lets the OSS
**    handle most of the interrupt functions.
**    'false' if the HAL's interrupt scheme is radically different than the OSS's scheme.  The
**    HAL is responsible for knowing how the OS handles interrupts.  Obviously, this is the
**    escape mechanism for a poor OSS design.  The HAL needs a radically different interrupt handling
**    design so the OSS will not handle any interrupt services.  If this is false, all other
**    paramters are ignored
**
**    <- chainDefault
**    If 'halVBLEnabler' or 'halVBLDisabler' = NULL, this is ignored by the OSS for the respective
**    function since the default enabler/disabler supplied by the OS is used.
**    If 'chainDefault = true', if the halVBLEnabler or halVBLDisabler != NULL, the OSS will call
**    the default OS enabler/disbler after the HAL's enabler/disabler is called
**    If 'chainDefault = false', if the halVBLEnabler or halVBLDisabler != NULL, the OSS will NOT call
**    the default OS enabler/disbler after the HAL's enabler/disabler is called.  The HAL assumes
**    the responsibility for enabling/disabling the interrupt source.  (Dangerous!)
**
**    <- halVBLHandler
**    The HAL's VBL handler which should clear the internal interrupt source.
**
**    <- halVBLEnabler
**    If 'halVBLEnabler = NULL', the default OS enabler will be called and the HAL can ignore things.
**    If 'halVBLEnabler != NULL' and 'hainDefault = true', the HAL needs to enable the internal
**    interrupt source and the OSS calls the default OS enable routine to enable external interrupts.
**    If 'halVBLEnabler != NULL' and 'chainDefault = false', the HAL needs to enable the internal
**    and external interrupt source.  (Dangerous!)
**
**    <- halVBLDisabler
**    If 'halVBLDisabler = NULL', the default OS enabler will be called and the HAL can ignore things.
**    If 'halVBLDisabler != NULL' and 'chainDefault = true', the HAL can choose to disable the internal
**    interrupt source and the OSS calls the default OS disable routine to disable external interrupts.
**    If 'halVBLDisabler != NULL' and 'chainDefault = false', the HAL can choose to disable the internal
**    and must disable the external interrupt source.  (Dangerous!)
**
**    <- vblRefCon
**    If the HAL needs some data for the interrupt routines, allocate some structure that
**    is pointed to by vblRefCon.
**
**
**  Avenger always has the internal interrupt source enabled so it knows when vbls occur.  Hence,
**  halVBLEnabler and halVBLDisabler = NULL.  The default OS enable/disable routines are used to
**  control what interrupts the OS sees.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetVBLInterruptRoutines(Boolean *installVBLInterrupts, Boolean *chainDefault,
    VBLHandler **halVBLHandler, VBLEnabler **halVBLEnabler, VBLDisabler **halVBLDisabler,
    void **vblRefCon)
{
#define FN_NAME "GraphicsHALGetVBLInterruptRoutines"
  LOG_ENTRY(50);
  /*
  ** Avenger is always going to have its internal interrupt source enabled for VBLs.  Hence,
  ** 'halVBLEnabler' and 'halVBLDisabler' are NULL.  The internal interrupt will be
  ** allowed/stopped to propagate by opening/closing the external interrupt 'gateway' via the
  ** default OS enabler/disabler.
  */
  *installVBLInterrupts = true;   /* This HAL supports the OSS's interrupt scheme. */
  *chainDefault = true;       /* Ignored by OSS since HAL's enabler/disablers are NULL */
  *halVBLHandler = AvengerHandleVBLInterrupts;
  *halVBLEnabler = AvengerEnableVBLInterrupts;
  *halVBLDisabler = AvengerDisableVBLInterrupts;
  *vblRefCon = NULL;          /* No private refCon needed. */

  LOG_EXIT(50,0);

  return kGDXErrNoError;
#undef FN_NAME  
}


/*
**=====================================================================================================
**
** GraphicsHALGrayCLUT()
**  This routine sets all the CLUT entries to 50% gray (with gamma correction).
**  This is useful so that the pixel depth can be subsequently changed without
**  introducing screen anonmalies.
**  The 50% gray value will be obtained by using the midpoint value of the supplied
**  gamma table.
**
**  NOTE:  this assumes that the gamma correction data size is 1 byte,
**  as stated in the core.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGrayCLUT(const GammaTbl *gamma)
{
#define FN_NAME "GraphicsHALGrayCLUT"

  /* Gray all 256 entries...it doesn't matter what the current 'depthMode' is. */

  GDXErr err = kGDXErrUnknownError;         /* Assume failure */

  UInt8 *midPointRed;                 /* Midpoint of red correction data */
  UInt8 *midPointGreen;               /*     "    "  green    "       "  */
  UInt8 *midPointBlue;                /*     "    "  blue     "       "  */
  SInt16 channelCount = gamma->gChanCnt;
  SInt16 entriesPerChannel = gamma->gDataCnt;
  FxU32 theColor;

  Boolean vblInterruptsEnabled;

  UInt32 i;

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 regBase = avengerHALData->bInfo.regInfo.ioPortBase;
  AbsoluteTime ns260 = avengerHALData->ns260;     /* 260ns in absolute time */

  LOG_ENTRY(5);

  /*
  ** Get the midpoint of the red correction data.  This is found by starting at the begining of the
  ** correction data, which can be found at '&gFormulaData[0]', adding the 'gFormulaSize'.
  ** Then go halfway into the table as determined by 'entriesPerChannel / 2'
  */
  midPointRed = (UInt8 *) ((UInt32) &gamma->gFormulaData[0] + gamma->gFormulaSize +
      (entriesPerChannel / 2));

  /*
  ** If there is only 1 channel of correction data, it means the same correction is applied to the
  ** red, green, and blue channels.  If there are 3 channels then each color has its own correction
  ** data.
  */
  if (1 == channelCount) {
    midPointGreen = midPointRed;
    midPointBlue = midPointRed;
  } else {
    midPointGreen = midPointRed + entriesPerChannel;
    midPointBlue = midPointRed + (entriesPerChannel * 2);
  }

  theColor = (*midPointRed << RED_SHIFT) | (*midPointGreen << GREEN_SHIFT) | (*midPointBlue << BLUE_SHIFT);
  AvengerProgramCLUTColor( theColor );

  err = kGDXErrNoError;

ErrorExit:
  LOG_EXIT(5,err);
  return (err);
#undef FN_NAME
}

/*
**=====================================================================================================
**
** GraphicsHALSetCLUT()
**  This routine will program the CLUT with the specified array of 'ColorSpecs'.
**  Two such arrays are provided, the original, and a second that has been luminance mapped (if
**  appropriate) and gamma corrected. It is up to the HAL implementation to decide which array should
**  be applied to the hardware. Most hardware will use the corrected version.
**
**  It is important to note that the positions of the entries refers to logical positions, not physical
**  ones. In 4-bits-per-pixel mode, for example, the entry positions could range from 0, 1, 3,, 15,
**  even though the physical positions may not have this number sequence.
**
**  No range checking is required, because the caller has allready done so.
**
**    -> originalCSTable
**    This is a pointer to the array of ColorSpecs provided by the caller. This is only provided in
**    the event that the hardware should not use the correctedCSTable. If any adjuments need to made
**    to it, then they should be done to a copy. Don't throw away the const!
**
**    -> correctedCSTable
**    This is essentially a copy of originalCSTable, except that it has been luminance mapped (if
**    appropriate) and gamma corrected. Most hardware will use this information to set the CLUT.
**    Though it is unlikely that you will need to change this information, it is not marked as
**    'const' in case you need to build a special version from the originalCSTable.  In that event,
**    you can alter the array as you see fit.  (Note:  regardless of the size of the originalCSTable,
**    correctedCSTable points to an array of ColorSpecs with 256 entries.
**
**    -> startPosition
**    (0 based) Starting point in the array of ColorSpecs
**
**    -> numberOfEntries
**    (0 based) This is the number of entries to be set.
**
**    -> sequential
**    If 'false', then the 'value' field of the ColorSpec should be inspected to see what logical
**    position should be set. If 'true', then the array index indicates what logical position should
**    be set.
**
**    -> depthMode
**    The relative bit depth. This is provided so that the HAL can decide how to map the logical
**    entry positions to the physical entry postions.
**
**=====================================================================================================
*/
GDXErr GraphicsHALSetCLUT(const ColorSpec *originalCSTable, ColorSpec *correctedCSTable,
    SInt16 startPosition, SInt16 numberOfEntries, Boolean sequential, DepthMode depthMode)
{
#define FN_NAME "GraphicsHALSetCLUT"
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  AbsoluteTime ns260 = avengerHALData->ns260;       /* 260ns in absolute time */

  UInt32 startAddress;
  UInt32 entryOffset;
  UInt32 physicalAddress;
  UInt32 physicalAddressG;
  UInt32 logicalAddress;

  UInt32 i;                 /* Loop control variable */

  GDXErr err = kGDXErrUnknownError;     /* Assume failure. */

  LOG_ENTRY(5);
  
  err = AvengerMapDepthModeToCLUTAttributes(depthMode, &startAddress, &entryOffset);
  if (err)
    goto ErrorExit;

  // Program the CLUT entries.  For our hardware, use the correctedCSTable
  for (i = startPosition ; i <= (startPosition + numberOfEntries); i++) {
    FxU32 data;

    if (sequential)
      logicalAddress = i;
    else
      logicalAddress = correctedCSTable[i].value;

    if(depthMode == kDepthMode2) {
      physicalAddress = startAddress + logicalToPhysical5to5[logicalAddress];
#if H5
      physicalAddressG = startAddress + logicalToPhysical5to5[logicalAddress];
#else
      physicalAddressG = startAddress + logicalToPhysical5to6[logicalAddress];
#endif      
    } else {
      physicalAddress = startAddress + logicalAddress;
      physicalAddressG = physicalAddress;
    }

    avengerHALData->baseCLUT.entry[physicalAddress].r = correctedCSTable[i].rgb.red;
    avengerHALData->baseCLUT.entry[physicalAddressG].g = correctedCSTable[i].rgb.green;
    avengerHALData->baseCLUT.entry[physicalAddress].b = correctedCSTable[i].rgb.blue;
  }

  /* Rebuild hacked 565 clut to sorta emulate 1555 (really 0455) */
  for(i = 0; i < 32; i++) {
#if !H5
    avengerHALData->baseCLUT.entry[256 + logicalToPhysical5to5[i]].r = avengerHALData->baseCLUT.entry[redRemap[i]].r;
#else    
    avengerHALData->baseCLUT.entry[256 + logicalToPhysical5to5[i]].r = avengerHALData->baseCLUT.entry[logicalToPhysical5to5[i]].r;
    avengerHALData->baseCLUT.entry[256 + logicalToPhysical5to5[i]].g = avengerHALData->baseCLUT.entry[logicalToPhysical5to5[i]].g;
#endif    
    avengerHALData->baseCLUT.entry[256 + logicalToPhysical5to5[i]].b = avengerHALData->baseCLUT.entry[logicalToPhysical5to5[i]].b;
  }
#if !H5
  for(i = 0; i < 64; i++)
  {
    avengerHALData->baseCLUT.entry[256 + logicalToPhysical6to6[i]].g = avengerHALData->baseCLUT.entry[greenRemap[i]].g;
  }
#endif

  /* We now always program the clut.  If we're in exclusive mode this has been gated in the core driver code. */
  AvengerProgramCLUT();

  err = kGDXErrNoError;

ErrorExit:
  LOG_EXIT(5,err);
  return (err);
#undef FN_NAME  
}


/*
**=====================================================================================================
**
** GraphicsHALGetCLUT()
**  This routine will fill out the specified array of ColorSpecs with the contents of the CLUT.
**  The 'RGBColor' structure in each 'ColorSpec' uses 16-bits for each channel (red, green, and blue),
**  whereas most CLUTs only use 8-bits. Therefore, when filling in the 'RGBColor' structure, the most
**  significant byte for each channel should be filled with the 8-bits extracted from its respective
**  channel in the CLUT. Moreover, to maintain the same behavior as the reference model, the 8-bits
**  from the CLUT should also be written to the least significant byte for each RGBColor.
**
**  It is important to note that the positions of the entries refer to logical positions, not physical
**  ones. At 4 bpp, for example, the entry positions could range from 0, 1, 2,, 15, even though the
**  physical positions may not have this number sequence.
**
**  No range checking is required, because the caller has already done so.
**
**    <-> csTable
**    Pointer to array of 'ColorSpecs' provided by the caller to be filled with the CLUT contents.
**
**    -> startPosition  (0 based) Starting point in the array to fill.
**    -> numberOfEntries  (0 based) The number of entries to be get.
**
**    -> sequential
**    If 'false', then the value field of the 'ColorSpec' should be inspected to see what logical
**    position should be retrieved. If 'true', then the array index indicates what logical position
**    should be read.
**
**    -> depthMode
**    The relative bit depth. This is provided so that the HAL can decide how to map the logical
**    entry positions to the physical entry postions.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetCLUT(ColorSpec *csTable, SInt16 startPosition, SInt16 numberOfEntries,
    Boolean sequential, DepthMode depthMode)
{
#define FN_NAME "GraphicsHALGetCLUT"

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;

  AbsoluteTime ns260 = avengerHALData->ns260;       /* 260ns in absolute time */

  UInt32 startAddress;
  UInt32 entryOffset;
  UInt32 physicalAddress;
  UInt32 physicalAddressG;
  UInt32 logicalAddress;

  UInt32 i;

  GDXErr err = kGDXErrUnknownError;

  LOG_ENTRY(5);
  
  err = AvengerMapDepthModeToCLUTAttributes(depthMode, &startAddress, &entryOffset);
  if (err)
    goto ErrorExit;

  /* Read the CLUT entries. */
  for (i = startPosition ; i <= (startPosition + numberOfEntries); i++) {
    FxU32 data;

    if (sequential)
      logicalAddress = i;
    else
      logicalAddress = csTable[i].value;

    if(depthMode == kDepthMode2) {
      physicalAddress = startAddress + logicalToPhysical5to5[logicalAddress];
      physicalAddressG = startAddress + logicalToPhysical5to6[logicalAddress];
    } else {
      physicalAddress = startAddress + logicalAddress;
      physicalAddressG = physicalAddress;
    }

    csTable[i].rgb.red = avengerHALData->baseCLUT.entry[physicalAddress].r;
    csTable[i].rgb.red |= (csTable[i].rgb.red << 8);    /* Copy it to most sig. byte  (rrrr) */
    csTable[i].rgb.green = avengerHALData->baseCLUT.entry[physicalAddressG].g;
    csTable[i].rgb.green |= (csTable[i].rgb.green << 8);    /* Copy it to most sig. byte  (rrrr) */
    csTable[i].rgb.blue = avengerHALData->baseCLUT.entry[physicalAddress].b;
    csTable[i].rgb.blue |= (csTable[i].rgb.blue << 8);    /* Copy it to most sig. byte  (rrrr) */
  }

  err = kGDXErrNoError;

ErrorExit:
  LOG_EXIT(5,err);
#undef FN_NAME
  return (err);
}


/*
**=====================================================================================================
**
** GraphicsHALGetPages()
**  This routine reports the number of graphics pages supported for the specified 'DisplayModeID' at
**  the specified 'DepthMode'.
**  No attempt should be made to determine whether or not a display capable of being driven with
**  a raster of type 'DisplayModeID' is physically connected.
**
**    -> displaymodeID  The DisplayModeID for which the page count is desired.
**    -> depthMode    The relative bit depth for which the page count is desired.
**
**    <- pageCount
**    # of pages supported at the specified 'DisplayModeID' and 'DepthMode'.  In the event of an
**    error, 'pageCount' is undefined.  This is a counting number, so it is NOT zero based.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetPages(DisplayModeID displayModeID, DepthMode depthMode, SInt16 *pageCount)
{
#define FN_NAME "GraphicsHALGetPages"
  GDXErr err = kGDXErrUnknownError;             /* Assume failure. */
  Boolean  modePossible = false;

  LOG_ENTRY(10);
  
  /*
  ** For the Avenger graphics hardware, the 'pageCount' is always 1 for any supported
  ** 'DisplayModeID'  Therefore, the validity of the  the specified 'DisplayModeID' and 'DepthMode'
  ** will be tested by calling GraphicsHALModePossible(), passing in '0' for the page #.
  */
  err = GraphicsHALModePossible(displayModeID, depthMode, 0, &modePossible);
  if (err || !modePossible)
  {
    /* Opps...caller specified an invalid 'DisplayModeID' or 'DepthMode' */
    err = kGDXErrInvalidParameters;
    goto ErrorExit;
  }

  /* Avenger graphics architecture only supports 1 page. */

  *pageCount = 1;

  err = kGDXErrNoError;                  // Everything okay

ErrorExit:

  LOG_EXIT(10,err);
  
  return (err);
#undef FN_NAME  
}



/*
**=====================================================================================================
**
** GraphicsHALGetBaseAddress()
**  This returns the base address of a specified page in the current mode.
**  This allows video pages to be written to even when not displayed
**
**      ->  page      Page number ( 0 based ).  Return the base address for this page.
**      <-  baseAddress   Base address of desired page.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetBaseAddress(SInt16 page, char **baseAddress)
{
#define FN_NAME "GraphicsHALGetBaseAddress"
  GDXErr err = kGDXErrUnknownError;     /* Assume failure. */
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  LOG_ENTRY(10);
  
  /* Avenger only supports 1 page.  Hence, only page = 0 is valid. */
  /* All other values will return an error. */

  if ( 0 != page ) {
    /* Caller asked for baseAddress of invalid page */
    err = kGDXErrInvalidParameters;
    goto ErrorExit;
  }
  
  /* For now I always stick the display at the beginning of VRAM. */
  /* *baseAddress = (char *) (avengerHALData->bInfo.regInfo.rawLfbBase + kCursorVRAM + kFifoVRAM); */
  if(avengerHALData->fifo.exclusiveMode)
    *baseAddress = (char *)&avengerHALData->dummyFrameBuffer[0];
  else
    *baseAddress = (char *) (avengerHALData->frameBufferBase + avengerHALData->swizzleOffset);

  LOG_PRINTF2(10, "GraphicsHALGetBaseAddress(%d, %08lx)\n",page,*baseAddress);
  
  err = kGDXErrNoError;                 /* Everything okay */

ErrorExit:
  LOG_EXIT(10,err);
#undef FN_NAME  
  return (err);
}


/*
**=====================================================================================================
**
** GraphicsHALGetSync()
**  Multipurpose call to
**  1) Report the capabilities of the frame buffer in controlling the sync lines and if HW can
**  "sync" on Red, Green or Blue
**  2) Report the current status of the sync lines and if HW is "syncing" on Red, Green or Blue
**
**  If the display supported the VESA Device Power Management Standard (DPMS), it would respond
**  to HSync and VSync in the following manner:
**  The VESA Standards is:
**
**      State         Vert Sync   Hor Sync    Video
**      -----     --------    ---------   ------
**      Active          Pulses        Pulses        Active
**      Standby     Pulses      No Pulses       Blanked
**      Idle      No Pulses       Pulses      Blanked
**      Off       No Pulses   No Pulses   Blanked
**
**
**      ->  getHardwareSyncCapability 'true' if reporting HW capability
**                      'false' if reporting current status
**
**  For this routine, the relevant fields of the 'VDSyncInfoRec' structure are as follows:
**      <-  sync    Report HW capability or current state.
**
**        If 'getHardwareSyncCapability = true' then report the cabability of the HW
**        When reporting the capability of the HW, set the appropriate bits of csMode:
**        kDisableHorizontalSyncBit   Set if HW can disable Horizontal Sync
**        kDisableVerticalSyncBit     Set if HW can disable Vertical Sync
**        kDisableCompositeSyncBit    Set if HW can disable Composite Sync
**        kSyncOnRedEnableBit       Set if HW can sync on Red
**        kSyncOnGreenEnableBit     Set if HW can sync on Green
**        kSyncOnBlueEnableBit      Set if HW can sync on Blue
**        kNoSeparateSyncControlBit   Set if HW CANNOT enable/disable H,V,C sync independently
**                        Means that HW ONLY supports the VESA "OFF" state
**
**        If 'getHardwareSyncCapability  = false' then report the current state of sync lines and
**        if HW is "syncing" on Red, Green or Blue.
**        Reporting the "current state of the sync lines" means: "Report the State of the Display"
**        To report the current state of the display, the 'kDisableHorizontalSyncBit' and
**        the 'kDisableVerticalSyncBit' have the following interpretation:
**
**        State         kDisableVerticalSyncBit   kDisableHorizontalSyncBit   Video
**        -----     -----------------------   -------------------------   ------
**        Active              0                 0               Active
**        Standby         0               1               Blanked
**        Idle          1                 0           Blanked
**        Off           1             1           Blanked
**
**        To report if HW is "syncing" on Red, Green or Blue:
**        kSyncOnRedEnableBit       Set if HW is "syncing" on Blue
**        kSyncOnGreenEnableBit     Set if HW is "syncing" on Green
**        kSyncOnBlueEnableBit      Set if HW is "syncing" on Blue
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetSync(Boolean getHardwareSyncCapability, VDSyncInfoRec *sync)
{
#define FN_NAME "GraphicsHALGetSync"
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  GDXErr err = kGDXErrUnknownError;               /* Assume failure. */

  LOG_ENTRY(20);
  
  sync->csFlags = 0;
  if (getHardwareSyncCapability) {
    /* Report capability of the hardware.  We can control H and V sync independantly. */
    sync->csMode = 1 << kDisableHorizontalSyncBit |
                   1 << kDisableVerticalSyncBit;
  } else {
    /* Report the current status of the various syncs */
    sync->csMode = avengerHALData->syncFlags;
  }

  err = kGDXErrNoError;                     /* Everything okay */

ErrorExit:
  LOG_EXIT(20,err);
  return (err);
#undef FN_NAME
}


/*
**=====================================================================================================
** GraphicsHALSetSync()
**  If the display supported the VESA Device Power Management Standard (DPMS), it would respond
**  to HSync and VSync in the following manner:
**  The VESA Standards are:
**
**  State         Vert Sync   Hor Sync    Video
**  -----     --------    ---------   ------
**  Active          Pulses        Pulses        Active
**  Standby     Pulses      No Pulses       Blanked
**  Idle      No Pulses       Pulses      Blanked
**  Off       No Pulses   No Pulses   Blanked
**
**
**  For this routine, the relevant fields of the 'VDSyncInfoRec' structure are as follows:
**      ->  syncBitField    bit field of the sync bits that need to be disabled/enabled
**
**        kDisableHorizontalSyncBit   Set if HW should disable Horizontal Sync (No Pulses)
**        kDisableVerticalSyncBit     Set if HW should disable Vertical Sync (No Pulses)
**        kDisableCompositeSyncBit    Set if HW should disable Composite Sync (No Pulses)
**        kSyncOnRedEnableBit       Set if HW should sync on Red
**        kSyncOnGreenEnableBit     Set if HW should sync on Green
**        kSyncOnBlueEnableBit      Set if HW should sync on Blue
**
**
**      ->  syncBitFieldValid   Mask of the bits that are valid in the csMode bit field
**
**=====================================================================================================
*/
GDXErr GraphicsHALSetSync(UInt8 syncBitField, UInt8 syncBitFieldValid)
{
#define FN_NAME "GraphicsHALSetSync"
#define FN_LEVEL 0
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 dacMode;
  FxBool blank = FXFALSE;

  GDXErr err = kGDXErrUnknownError;               /* Assume failure */

  LOG_ENTRY(FN_LEVEL);
  
#define DAC_DPMS_VSYNC  BIT(1)
#define DAC_VSYNC_HIGH  BIT(2)
#define DAC_DPMS_HSYNC  BIT(3)
#define DAC_HSYNC_HIGH  BIT(4)

  /* No valid bits are set, just exit with no error. */
  if (0 == syncBitFieldValid) {
    err = kGDXErrNoError;
    goto ErrorExit;
  }

  HWC_IO_LOAD( bInfo->regInfo, dacMode, dacMode );

  /* Deal with hSync */
  if (syncBitFieldValid & kHorizontalSyncMask) {
    dacMode &= ~(DAC_DPMS_HSYNC|DAC_HSYNC_HIGH);
    if(syncBitField & kHorizontalSyncMask) {
      dacMode |= DAC_DPMS_HSYNC;
      blank = FXTRUE;
      avengerHALData->syncFlags |= kHorizontalSyncMask;
    } else {
      avengerHALData->syncFlags &= ~kHorizontalSyncMask;
    }
  }

  /* Deal with vSync */
  if (syncBitFieldValid & kVerticalSyncMask) {
    dacMode &= ~(DAC_DPMS_VSYNC|DAC_VSYNC_HIGH);
    if(syncBitField & kVerticalSyncMask) {
      dacMode |= DAC_DPMS_VSYNC;
      blank = FXTRUE;
      avengerHALData->syncFlags |= kVerticalSyncMask;
    } else {
      avengerHALData->syncFlags &= ~kVerticalSyncMask;
    }
  }

  HWC_IO_STORE( bInfo->regInfo, dacMode, dacMode);

  if( blank )
  {
    FxU32 theVidProcCfg;
    
  	LOG_PRINTF(FN_LEVEL, "we need to blank...\n" );
    HWC_IO_LOAD( bInfo->regInfo, vidProcCfg, theVidProcCfg);
    if ( theVidProcCfg )
      avengerHALData->savedVidProcCfg = theVidProcCfg;
  	LOG_PRINTF1(FN_LEVEL, "saved VidProcCfg = 0x%08x\n", avengerHALData->savedVidProcCfg );
    HWC_IO_STORE( bInfo->regInfo, vidProcCfg, 0);  // blank the screen
  }
  else
  {
  	LOG_PRINTF(FN_LEVEL, "we need to blank...\n" );
    HWC_IO_STORE( bInfo->regInfo, vidProcCfg, avengerHALData->savedVidProcCfg);  // unblank the screen
  	LOG_PRINTF1(FN_LEVEL, "restored VidProcCfg = 0x%08x\n", avengerHALData->savedVidProcCfg );
  }

  err = kGDXErrNoError;

ErrorExit:
  LOG_EXIT(FN_LEVEL,err);
  
  return (err);
#undef FN_NAME  
#undef FN_LEVEL  
}


/*
**=====================================================================================================
**
** GraphicsHALGetModeTiming()
**  This is used to to gather scan timing information.  Look at 'displayModeID' and return the
**  appropriate info.  The 'displayModeID' must be valid for the display type attached.
**
**      ->  displayModeID Describes the dispaly resolution and scan timing
**
**      <-  timingFormat  Format of the info in 'timingData' field, only 'kDeclROMtables' is valid.
**
**      <-  timingFlags   Whether the display mode with these scan timings is required or optional.
**                If a 'displayModeID' is not thought to be valid for given display, set
**                timingFlags to 0 (invalid and unsafe)...the DisplayMgr will ask
**                display modules if the mode is valid.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetModeTiming(DisplayModeID displayModeID, UInt32 *timingFormat, UInt32 *timingFlags)
{
#define FN_NAME "GraphicsHALGetModeTiming"
#define FN_LEVEL 20
  FxU32 i;
  
  enum
  {
    valid = (1 << kModeValid),
    validAndSafe = (1 << kModeValid | 1 << kModeSafe),
    validAndSafeAndDefault = ( (1 << kModeValid) | (1 << kModeSafe) | (1 << kModeDefault) )
  };

  typedef struct DisplayModeTimingTable DisplayModeTimingTable;
  struct DisplayModeTimingTable
  {
    DisplayModeID displayModeID;
    UInt32 timingFlags;
  };

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  DisplayCode displayCode = avengerHALData->displayCode;  /* Class of connected display */

  GDXErr err = kGDXErrNoError;          /* Never fails.  'timingFlags = 0' if don't think */
                                        /* display supports the 'displayModeID' */

  LOG_ENTRY(FN_LEVEL);
  
  *timingFlags = 0;       /* Default to invalid and unsafe.  If HAL doesn't know if a monitor */
                          /* supports a 'displayModeID', this will allow the Display Mgr to   */
                          /* ask a display module.                                            */


  *timingFormat = kDeclROMtables; /* Default to kDeclROMtables -- the only valid timingFormat */

  /* Prior to doing anything else, do some paranoid error checking, and make sure that H4Wax */
  /* can drive the indicated 'displayModeID.'  This can be accomplished by calling the       */
  /* GraphicsHALModePossible() routine.                                                      */
 
  /* if ( displayCode == kDisplayCodeVGA ) */
  for(i = 0; i < kMaxDisplayModeIDs; i++)
  {
    if ( displayModeIDMap[i].displayModeID == displayModeID )
    {
      *timingFlags = displayModeIDMap[i].timingFlags;
      break;
    }
  }

  if ( avengerHALData->displayCode == kDisplayCodeDVI )
  {
    if ( avengerHALData->his == 1600 )
    {
      switch ( displayModeID ) {
        case kDisplay640x480At60Hz:
//        case kDisplay800x500At60Hz:
        case kDisplay800x512At60Hz:
        case kDisplay800x600At60Hz:
        case kDisplay1024x768At60Hz:
        case kDisplay1280x1024At60Hz:
        case kDisplay1600x1024At60Hz:
          *timingFlags = (1 << kModeValid | 1 << kModeSafe);
          break;
        
        default:
          break;
      }
    }
    else if ( avengerHALData->his == 1024 )
    {
      switch ( displayModeID ) {
      	case kDisplay640x480At60Hz:
      	case kDisplay800x600At60Hz:
        case kDisplay1024x768At60Hz:
          *timingFlags = (1 << kModeValid | 1 << kModeSafe);
          break;
        
        default:
          break;
      }
    }
  }
  
ErrorExit:
#if DEBUG
  if ( err == 0 )
  {
    char * theTimingFlags;
    
    if ( *timingFlags == valid )
      theTimingFlags = "valid";
    else if ( *timingFlags == validAndSafe )
      theTimingFlags = "valid and safe";
    else if ( *timingFlags == validAndSafeAndDefault )
      theTimingFlags = "valid and safe and default";
    else
      theTimingFlags = "unknown";
    
  	LOG_PRINTF1(FN_LEVEL, "displayModeID = %d\n", displayModeID );
  	LOG_PRINTF2(FN_LEVEL, "timingFlags = %s (%d)\n", theTimingFlags, *timingFlags );
  }
  else
  {
  	LOG_PRINTF1(FN_LEVEL, "### FAILED ###   displayModeID = %d\n", displayModeID );
  }
#endif
  LOG_EXIT(FN_LEVEL,err);

  return (err);
#undef FN_NAME  
#undef FN_LEVEL
}


/*
**=====================================================================================================
**
** GraphicsHALGetNextResolution()
**  This call will take a 'previousDisplayModeID' and return the next supported display mode.
**  The Core takes care of most of the work.  The HAL just needs to look at the
**  'previousDisplayModeID' and return the next supported d'isplayModeID' and the max depthMode
**  that the HW supports.
**  The Core has already checked to see if previousDisplayModeID = -1, in which case it has
**  returned the current resolution's information.
**  The HAL returns all DisplayModeIDs that it supports.  Pay No attention to the monitor behind
**  the sense lines.
**
**      ->  previousDisplayModeID
**      'previousDiplayModeID = kDisplayModeIDCurrent' never happens.  Core will handle that case.
**      If 'previousDiplayModeID = kDisplayModeIDFindFirstResolution', get the first supported
**      resolution for the monitor.
**      Otherwise, 'previousDiplayModeID' contains the previous displayModeID (hence its name)
**      from the previous call.
**
**      <-  displayModeID ID of the next display mode following 'csPreviousDiplayModeID'
**      Set to 'kDisplayModeIDNoMoreResolutions' once all supported display modes have been reported.
**
**      <-  maxDepthMode  Maximum relative bit depth for the 'displayModeID'
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetNextResolution(DisplayModeID previousDisplayModeID,
    DisplayModeID *displayModeID, DepthMode *maxDepthMode)
{
#define FN_NAME "GraphicsHALGetNextResolution"
#define FN_LEVEL 20
  GDXErr err = kGDXErrUnknownError;               /* Assume failure */

  UInt32 i;                           /* Loop iterator */
  UInt32 displayModeIDIndex;          /* Index for the returned DisplayModeID */

  LOG_ENTRY(FN_LEVEL);
  
  /* If 'kDisplayModeIDFindFirstResolution == previousDisplayModeID', return the first resolution */
  /* that the HW supports.                                                                        */

  if (kDisplayModeIDFindFirstResolution == previousDisplayModeID) {
    /* Index to first supported resolution */
  	LOG_PRINTF(FN_LEVEL, "requesting the first resolution available\n" );
    displayModeIDIndex = 0;
  } else {
    /* Find the previousDisplayModeID in 'theResolutionTable'. */
    i = 0;
    while ( (i < kMaxDisplayModeIDs) &&  (displayModeIDMap[i].displayModeID != previousDisplayModeID) )
      i++;

    /* Check if i has exceeded kMaxResolutions.  This means the 'previousDisplayModeID' was not valid. */

    if (kMaxDisplayModeIDs == i) {
      err = kGDXErrInvalidParameters;
      goto ErrorExit;
    }

    displayModeIDIndex = ++i;   /* Index to next supported resolution. (or no more) */
  }

  /* DisplayModeIndex now points to the next resolution. */
  /* Make sure 'displayModeIndex != kMaxResolutions'... if it does, all resolutions have been reported. */
  

  while (displayModeIDIndex < kMaxDisplayModeIDs) {
    DisplayModeID theModeID = displayModeIDMap[displayModeIDIndex].displayModeID;
    Boolean theModePossible;
    
    GraphicsHALModePossible(theModeID, kDepthMode3, 0, &theModePossible);
    
    if ( theModePossible )
    {
      *displayModeID = displayModeIDMap[displayModeIDIndex].displayModeID;
      err = GraphicsHALGetMaxDepthMode(*displayModeID, maxDepthMode);
      if ( err == 0 )
        break;
    }
    
    displayModeIDIndex++;
  }

  if (displayModeIDIndex >= kMaxDisplayModeIDs) {
    *displayModeID = kDisplayModeIDNoMoreResolutions;   /* No more supported resolutions */
    err = kGDXErrNoError;                               /* Everything okay */
  }

ErrorExit:
#if DEBUG
  if ( err == 0 )
  {
  	LOG_PRINTF2(FN_LEVEL, "prevModeID = %d, nextModeID = %d\n", previousDisplayModeID, *displayModeID );
  	LOG_PRINTF1(FN_LEVEL, "maxdepth = %d\n", *maxDepthMode );
  }
  else
  {
    LOG_PRINTF1(FN_LEVEL, "### FAILED ###   prevModeID = %d\n", previousDisplayModeID );
  }
#endif
  LOG_EXIT(FN_LEVEL,err);
  return (err);
#undef FN_NAME  
#undef FN_LEVEL  
}


/*
**=====================================================================================================
**
** GraphicsHALGetVideoParams()
**  The HAL only needs to return the absolute bits per pixel and the rowBytes for a
**  given depthMode.  The rowBytes, on input, contains the number of horizontal pixels
**  for the displayModeID.
**
**    -> depthMode  The relative bit depth for which the info is desired.
**    <- bitsPerPixel Absolute bit depth for the given depthMode.
**    <> rowBytes   On input, rowbytes contains the horizontal pixels for the dispalyModeID.
**            On output, width between successive rows of video memory for the given depthMode.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetVideoParams(DisplayModeID displayModeID, DepthMode depthMode,
    UInt32 *bitsPerPixel, SInt16 *rowBytes)
{
#define FN_NAME "GraphicsHALGetVideoParams"
#define FN_LEVEL 0

  static FxU8 _bitsMap[3] = { 8, 16, 32 };

  GDXErr err = kGDXErrUnknownError;       /* Assume failure */
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  LOG_ENTRY(FN_LEVEL); 
  
  *bitsPerPixel = _bitsMap[depthMode - kDepthMode1];

  /* Magic */
  if(avengerHALData->fifo.exclusiveMode) {
    *rowBytes = 8;
    err = kGDXErrNoError;
  }
  else if(depthMode <= kDepthMode3) {
    static FxU8 _shiftMap[3] = { 0, 1, 2 };
    *rowBytes = *rowBytes << _shiftMap[depthMode - kDepthMode1];
    err = kGDXErrNoError;
  } else {
    err = kGDXErrInvalidParameters;
  }

#if DEBUG
  if ( err == 0 )
  {
  	LOG_PRINTF2(FN_LEVEL, "displayModeID = %d, depthMode = %d\n", displayModeID, depthMode );
  	LOG_PRINTF2(FN_LEVEL, "bitsPerPixel = %d, rowByte = 0x%08x\n", *bitsPerPixel, *rowBytes );
  }
  else
  {
    LOG_PRINTF2(FN_LEVEL, "### FAILED ###   displayModeID = %d, depthMode = %d\n", displayModeID, depthMode );
  }
#endif
  LOG_EXIT(FN_LEVEL,err);
  return (err);
#undef FN_NAME
#undef FN_LEVEL
}


/*
**=====================================================================================================
**
** GraphicsHALGetMaxDepthMode()
** This takes a 'displayModeID' and returns the maximum depthMode that is supported by the
** hardware for that 'displayModeID'  NO check is made to determine if the 'displayModeID' is
** valid for the connected monitor.  The HAL should return an error if the 'displayModeID' is
** not supported or there is not enough VRAM to support the 'displayModeID'
**
**      ->  displayModeID   Get the information for this display mode
**      <-  maxDepthMode    Maximum relative bit depth for the DisplayModeID
**
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetMaxDepthMode(DisplayModeID displayModeID, DepthMode *maxDepthMode)
{
  *maxDepthMode = kDepthMode3;
  return kGDXErrNoError;
}


/*
**=====================================================================================================
**
** GraphicsHALMapDepthModeToBPP()
**  This routine maps a relative pixel depth (DepthMode) to an absolute pixel depth (bits per pixel)
**
**    -> depthMode    The relative pixel depth
**
**    <- bitsPerPixel   Corresponding abosolute pixel depth.
**
**=====================================================================================================
*/
GDXErr GraphicsHALMapDepthModeToBPP(DepthMode depthMode, UInt32 *bitsPerPixel)
{
#define FN_NAME "GraphicsHALMapDepthModeToBPP"
#define FN_LEVEL 10

  GDXErr err = kGDXErrUnknownError;     /* Assume failure. */
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  static FxU8 _map[3] = { 8, 16, 32 };
  FxU32 maxDepthMode;

  LOG_ENTRY(FN_LEVEL);  
  LOG_PRINTF1(FN_LEVEL,"depthMode = (%d)\n",depthMode);
  
  if(depthMode > kDepthMode3) {
    err = kGDXErrUnableToMapDepthModeToBPP;
    goto ErrorExit;
  }
  *bitsPerPixel = _map[depthMode - kDepthMode1];

  err = kGDXErrNoError;

ErrorExit:
  LOG_EXIT(FN_LEVEL,err);
  return (err);
#undef FN_NAME
#undef FN_LEVEL
}


/*
**=====================================================================================================
**
** GraphicsHALModePossible()
**  This routine checks to see if the frame buffer is cabable of driving the given 'displayModeID' at
**  the indicated 'depthMode' and 'page'. This DOES NOT check to see that the 'displayModeID' is valid
**  for the display type that is physically connected to the frame buffer.
**
**  IMPORTANT NOTE: The GDXErr return value DOES NOT indicate whether the mode is possible or not.
**  It only signifies whether or not the value returned in 'modePossible' was correctly determined.  In
**  the event of an error, 'modePossible' does not contain valid information.
**
**    -> displaymodeID    The desired DisplayModeID.
**    -> depthMode      The desired relative bit depth.
**    -> page         The desired page.
**
**    <- modePossible
**    This will be 'true' if the frame buffer can support the desired items, 'false' otherwise. In
**    the event of an error, 'modePossible' is undefined.
**
**=====================================================================================================
*/
GDXErr GraphicsHALModePossible(DisplayModeID displayModeID, DepthMode depthMode, SInt16 page,
    Boolean *modePossible)
{
#define FN_NAME "GraphicsHALModePossible"
#define FN_LEVEL 20

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  DepthMode maxDepthMode;           /* The max depthMode for the DisplayModeID */
  GDXErr err = kGDXErrNoError;      /* If displayModeID, depthMode or page isn't supported */
                                    /* just return modePossible = false */

  *modePossible = false;            /* Assume mode not possible */


  LOG_ENTRY(10);


  /*
  ** Check if the display can handle the mode...
  */

  if ( avengerHALData->displayCode == kDisplayCodeDVI )
  {
    if ( avengerHALData->his == 1600 )
    {
      switch ( displayModeID ) {
        case kDisplay640x480At60Hz:
//        case kDisplay800x500At60Hz:
        case kDisplay800x512At60Hz:
        case kDisplay800x600At60Hz:
        case kDisplay1024x768At60Hz:
        case kDisplay1280x1024At60Hz:
        case kDisplay1600x1024At60Hz:
          break;
        
        default:
          goto ErrorExit;
      }
    }
    else if ( avengerHALData->his == 1024 )
    {
      switch ( displayModeID ) {
      	case kDisplay640x480At60Hz:
      	case kDisplay800x600At60Hz:
        case kDisplay1024x768At60Hz:
          break;
        
        default:
          goto ErrorExit;
      }
    }
    
    if ( depthMode == kDepthMode1 )
      goto ErrorExit;
  }
  
  /*
  ** Find the maximum depthMode for the displayModeID
  ** If the displayModeID is not supportted by hardware, GetMaxDepthMode() returns
  ** kGDXErrDisplayModeIDUnsupported
  */
  err =  GraphicsHALGetMaxDepthMode(displayModeID, &maxDepthMode);

  if (err)
    goto ErrorExit;

  if (maxDepthMode >= depthMode)
    *modePossible = true;

  err = kGDXErrNoError;


ErrorExit:
  LOG_EXIT(10,err);
#if DEBUG
  if ( err == 0 )
  {
    LOG_PRINTF2(FN_LEVEL,"displayModeID = %d, depthMode = %d\n",displayModeID,depthMode);
    LOG_PRINTF2(FN_LEVEL,"page = %d, possible = %s\n", page, *modePossible ? "true" : "false" );
  }
  else
  {
    LOG_PRINTF2(FN_LEVEL,"displayModeID = %d, depthMode = %d\n",displayModeID,depthMode);
    LOG_PRINTF1(FN_LEVEL,"page = %d\n",page); 
    LOG_PRINTF(FN_LEVEL," ### FAILED ###\n" );
  }
#endif
  return (err);
#undef FN_NAME  
#undef FN_LEVEL
}





GDXErr GraphicsHALGetDDCBlock(UInt32 blockNumber, ResType blockType, UInt32 flags, Byte *data)
{
#define FN_NAME "GraphicsHALGetDDCBlock"
#define FN_LEVEL 0
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  GDXErr err = kGDXErrUnknownError;

  LOG_ENTRY(FN_LEVEL);

  LOG_PRINTF2(FN_LEVEL,"blockNumber = %08lx, blockType = %08lx\n", blockNumber,blockType);
  LOG_PRINTF2(FN_LEVEL,"flags = %08lx, data = %08lx\n",flags,data);

  if(!((blockType == kDDCBlockTypeEDID) && (blockNumber == 1)))
  {
    LOG_PRINTF(FN_LEVEL,"Unknown blockType/blockNumber.  Bailing out.\n");
    return err;
  }

  if((flags & kDDCForceReadMask) || !avengerHALData->edidWasRead)
  {
    if(AvengerReadDDC( k3DfxDDCDVI, 0xA0 /*+ blockNumber */, avengerHALData->edid, kDDCBlockSize)
        && GetEDIDVideoInputType( avengerHALData->edid ) )
    {
      LOG_PRINTF(FN_LEVEL,"AvengerReadDDC for DVI port succeeded.\n");
      avengerHALData->edidIsValid = true;
      avengerHALData->edidIsDVI = true;
      avengerHALData->his = GetEDIDDetailedTimingHActive( (Byte*) avengerHALData->edid, 1 );
      avengerHALData->vis = GetEDIDDetailedTimingVActive( (Byte*) avengerHALData->edid, 1 );
    }
    else if(AvengerReadDDC( k3DfxDDCVGA, 0xA0 /*+ blockNumber */, avengerHALData->edid, kDDCBlockSize))
    {
      LOG_PRINTF(FN_LEVEL,"AvengerReadDDC for VGA port  succeeded.\n");
      avengerHALData->edidIsValid = true;
      avengerHALData->edidIsDVI = false;
    }
    else
    {
      LOG_PRINTF(FN_LEVEL,"AvengerReadDDC failed.\n");
      avengerHALData->edidIsValid = false;
    }
    avengerHALData->edidWasRead = true;

  }

  /* Copy cached data if it's valid and we have a valid pointer. */
  if(avengerHALData->edidIsValid && data)
  {
    int i;
    for(i = 0; i < kDDCBlockSize; i++)
    {
      data[i] = avengerHALData->edid[i];
    }

    LOG_PRINTF(FN_LEVEL,"EDID copied to the data pointer\n");
    err = kGDXErrNoError;
  }

  LOG_EXIT(FN_LEVEL,err);
  return err;
#undef FN_NAME  
#undef FN_LEVEL
}





GDXErr GraphicsHALDDCMonitorAvailable(Boolean *ddcAvailable)
{
#define FN_NAME "GraphicsHALDDCMonitorAvailable"
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  LOG_ENTRY(20);
  
  /* Try to read DDC info if we don't think we have a DDC monitor. This is
     supposed to fail fairly quickly if there's a monitor attached but it's
     not DDC capable.  It's possible we should cache the fact that we have
     a monitor attached but it's not DDC capable. */

  if(!avengerHALData->edidIsValid) {
    GraphicsHALGetDDCBlock(1, kDDCBlockTypeEDID, 0, 0);
  }

  /* Now check for having a real DDC monitor. */
  *ddcAvailable = avengerHALData->edidIsValid;

  LOG_EXIT(20,0);
  return (kGDXErrNoError);
#undef FN_NAME
}

/*
**=====================================================================================================
**
** GraphicsHALDetermineDisplayCode()
**  This routine is called whenever it is necessary to determine the type of display that is
**  connected to the frame buffer controller.
**  When this routine is called, the following actions should occur:
**
**    - Perform required steps to determine what display is connected (e.g., read sense lines)
**    - Update the HAL's state information regarding the type of display connected (if HAL
**      implementation maintains that state information)
**
**  In the event that the HAL is does not the specific type of display attached, it should set
**  '*displayCode = kDisplayCodeUnknown'
**
**      <- displayCode  DisplayCode for the attached display.
**
**=====================================================================================================
*/
GDXErr GraphicsHALDetermineDisplayCode(DisplayCode *displayCode)
{
#define FN_NAME "GraphicsHALDetermineDisplayCode"
#define FN_LEVEL 0
  GDXErr err = kGDXErrNoError;          /* Assume sucess */
  Boolean ddcAvailable;
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  avengerHALData->monitorType = AvengerGetMonitorType();
  GraphicsHALDDCMonitorAvailable( &ddcAvailable );

  LOG_ENTRY(FN_LEVEL);
  
  /* The monitor type was determined very early on, so we don't have to read it here. */
  if(avengerHALData->monitorType > 0 || ddcAvailable) { /* Mono or VGA */
    if ( avengerHALData->edidIsDVI )
      *displayCode = kDisplayCodeDVI;
//      *displayCode = kDisplayCodeVGA;
    else
      *displayCode = kDisplayCodeVGA;
  } else {
    *displayCode = kDisplayCodeNoDisplay;
  }

  avengerHALData->displayCode = *displayCode;   /* Update HAL information */

ErrorExit:
#if DEBUG
  if ( err == 0 )
  {
    char * displayCodeName;
    switch( avengerHALData->displayCode )
    {
      case kDisplayCodeVGA: displayCodeName = "kDisplayCodeVGA"; break;
      case kDisplayCodeNoDisplay: displayCodeName = "kDisplayCodeNoDisplay"; break;
      case kDisplayCodeDVI: displayCodeName = "kDisplayCodeDVI"; break;
      default: displayCodeName = "unknown"; break;
    }
    LOG_PRINTF2(FN_LEVEL,"%s (%d)\n", displayCodeName, avengerHALData->displayCode );
  }
  else 
  {
    LOG_PRINTF(FN_LEVEL,"GraphicsHALDetermineDisplayCode(): ### failed ###\n" );
  }
#endif
  LOG_EXIT(FN_LEVEL,err);
  return (err);
#undef FN_NAME  
#undef FN_LEVEL
}

/*
**=====================================================================================================
**
** GraphicsHALGetSenseCodes()
**  This routine is called whenever the state of the sense codes need to be reported.
**  NOTE:  This should only report the the sense code information.  No attempt should be made to
**  determine what type of display is attached here.  Moreover, the sense codes should be
**  determined EVERY time this call is made, and not make use of any previously saved values.
**
**    <- rawSenseCode
**    For 'standard' sense code hardware, this value is found by instructing the frame buffer
**    controller NOT to actively drive any of the monitor sense lines, and then reading the
**    state of the the monitor sense lines 2, 1, and 0.  (2 is the MSB, 0 the LSB)
**
**    <- extendedSenseCode
**    For 'standard' sense code hardware, the extended sense code algorithm is as follows:
**    (Note:  as described here, sense line 'A' corresponds to '2', 'B' to '1', and 'C' to '0')
**      - Drive sense line 'A' low and read the values of 'B' and 'C'.
**      - Drive sense line 'B' low and read the values of 'A' and 'C'.
**      - Drive sense line 'C' low and read the values of 'A' and 'B'.
**
**    In this way, a six-bit number of the form BC/AC/AB is generated.
**
**    <- standardInterpretation
**    If 'standard' sense code hardware is implemented (or the values are coerced to appear
**    'standard' then set this to 'true'.  Otherwise, set it to 'false', and the interpretation for
**    'rawSenseCode' and 'extendedSenseCode' will be considered private.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetSenseCodes(RawSenseCode *rawSenseCode, ExtendedSenseCode *extendedSenseCode,
    Boolean *standardInterpretation)
{
#define FN_NAME "GraphicsHALGetSenseCodes"
#define FN_LEVEL 20
  GDXErr err = kGDXErrNoError;              /* Assume sucess */

  LOG_ENTRY(FN_LEVEL);
  *extendedSenseCode = 0;
  *standardInterpretation = false;            /* Avenger has nonstandard sense lines */
  *rawSenseCode = AvengerReadSenseLines();

ErrorExit:
#if DEBUG
  if ( err == 0 )
  {
    LOG_PRINTF2(FN_LEVEL, "rawSenseCode = 0x%04x, extended = 0x%04x\n", *rawSenseCode, *extendedSenseCode);
    LOG_PRINTF1(FN_LEVEL+10, "standard interpretation = %s\n", *standardInterpretation ? "true" : "false" );
  }
  else 
  {
    LOG_PRINTF(FN_LEVEL, "### failed ###\n" );
  }
#endif

  LOG_EXIT(FN_LEVEL,err);
  return (err);

#undef FN_LEVEL  
#undef FN_NAME  
}


/*
**=====================================================================================================
**
** GraphicsHALGetDefaultDisplayModeID()
**  The 'displayCode' is passed in and the HAL returns the default 'displayModeID' and the
**  'depthMode'.  This routine gets called when a new monitor is connected to the computer.  The
**  HAL knows how much VRAM is available and whether it can handle a given 'displayModeID' for a
**  monitor.
**  For example, the 'kIndexedMultiScanBand3' has a default 'displayModeID' of kDisplay1024x768At7500Hz.
**  If there is not enough VRAM available, the HAL is unable to switch into that resolution, hence
**  the HAL will switch into the next best resolution.
**  The HAL will also return the max 'depthMode' that can be supported for the resolution.  This
**  enables a switch from a 13" monitor at millions of colors to be switched with a 21" monitor
**  and, if there is enough VRAM, the depthMode will still be at millions of colors
**
**    -> displayCode      The connected display
**    <- displaymodeID    The default DisplayModeID for the connected display type
**    <- depthMode      The default depthMode
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetDefaultDisplayModeID(DisplayCode displayCode, DisplayModeID *displayModeID,
    DepthMode *depthMode)
{
#define FN_NAME "GraphicsHALGetDefaultDisplayModeID"
#define FN_LEVEL 20

  /* Define a new type which is a table of default 'DisplayModeIDs' for the connected monitor. */
  typedef struct DefaultResolutionTable DefaultResolutionTable;
  struct DefaultResolutionTable
  {
    DisplayCode displayCode;
    DisplayModeID displayModeID;
  };

  DefaultResolutionTable defaultResolutionTable[] =
  {
    { kDisplayCodeUnknown,       kDisplay640x480At60Hz },
    { kDisplayCodeVGA,           kDisplay640x480At60Hz }
  };
  
  #define kMaxDefaultTableEntries ( sizeof(defaultResolutionTable) / sizeof(DefaultResolutionTable) )

  UInt32 i;                   /* Loop iterator */
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  GDXErr err = kGDXErrMonitorUnsupported;     /* Assume failure */

  /* Scan the defaultResolutionTable to find the the connected monitor */
  LOG_ENTRY(FN_LEVEL);

  if ( displayCode != kDisplayCodeDVI )
  {
    /* the display is not a DVI display */
    
    for (i = 0; i < kMaxDefaultTableEntries; i++)
    {
      if (defaultResolutionTable[i].displayCode == displayCode)
      {
        *displayModeID = defaultResolutionTable[i].displayModeID;
        err = kGDXErrNoError;
        break;          /* Found displayCode */
      }
    }
  }
  else
  {
    
    LOG_PRINTF2(FN_LEVEL,"hor image size %d, ver image size %d\n", avengerHALData->his, avengerHALData->vis);

    *displayModeID = kDisplay1024x768At60Hz;
    
    if ( avengerHALData->his == 800 && avengerHALData->vis == 600 )
      *displayModeID = kDisplay800x600At60Hz;
    else if ( avengerHALData->his == 1024 && avengerHALData->vis == 768 )
      *displayModeID = kDisplay1024x768At60Hz;
    else if ( avengerHALData->his == 1280 && avengerHALData->vis == 1024 )
      *displayModeID = kDisplay1280x1024At60Hz;
    else if ( avengerHALData->his == 1600 && avengerHALData->vis == 1024 )
      *displayModeID = kDisplay1600x1024At60Hz;

    LOG_PRINTF1(FN_LEVEL,"displayModeID = %d\n", *displayModeID);

    err = kGDXErrNoError;
  }

  if (err)                      /* Didn't find displayCode */
    goto ErrorExit;

  err = GraphicsHALGetMaxDepthMode(*displayModeID, depthMode);


ErrorExit:
#if DEBUG
  if ( err == 0 )
  {
    LOG_PRINTF2(FN_LEVEL, "displayCode = %d, displayModeID = %d\n", displayCode, *displayModeID );
    LOG_PRINTF1(FN_LEVEL, "depth = %d\n", *depthMode  );
  }
  else 
  {
    LOG_PRINTF1(FN_LEVEL, "### failed ### displayCode = %d \n", displayCode );
  }
#endif
  LOG_EXIT(FN_LEVEL,err);
  return (err);

#undef FN_LEVEL  
#undef FN_NAME  
}





/*
**=====================================================================================================
**
** GraphicsHALProgramHardware()
**  This routine attempts to program the graphics hardware to the desired 'displayModeID', 'depthMode',
**  and 'page'.  It is not required to specifically check to see if the inputs are valid, since it
**  can assume that the checking has been done elsewhere.
**
**    -> displaymodeID    The desired DisplayModeID.
**    -> depthMode      The desired relative bit depth.
**    -> page         The desired page.
**
**    <- directColor
**    This is 'true' if the desired depthMode results in the hardware being in a direct color mode,
**    otherwise it is 'false'. In the event on an error, it is undefined.
**
**    <- baseAddress
**    The resulting base address of the frame buffers ram. In the event of an error, it is undefined.
**
**=====================================================================================================
*/
GDXErr GraphicsHALProgramHardware(DisplayModeID displayModeID, DepthMode depthMode, SInt16 page,
    Boolean *directColor, char **baseAddress)
{
#define FN_NAME "GraphicsHALProgramHardware"
#define FN_LEVEL 2
  /* Define a new type which maps a 'DisplayModeID' to the value that will represent
     the appropriate 'GraphicsMode' for the hardware. */
  FxU32 width, height, refresh, bytesPerPixel, pixelFormat, stride;
  FxU32 clutSelect, vidProcCfg, swizzle, miscInit0, tmuGbeInit;
  FxU32 memUsed, i, swizzleOffset;
  FxU32 newDisplayMode = 0;

  /* Define a new type which maps a 'DepthMode' to the value that will represent
     the appropriate bit depth for the hardware. */

  typedef struct DepthModeToBitDepthMap DepthModeToBitDepthMap;
  struct DepthModeToBitDepthMap
  {
    FxU32 swizzleOffsetIndex;
    FxU32 pixelFormat;
    FxU16 bytesPerPixel;
    FxU16 clutSelect;
    FxU32 swizzle;
    void  (*setLfb)(volatile FxU32 *d, FxU32 s);
    void  (*setLfbHost)(volatile FxU32 *d, FxU32 s);
  };

  enum {kDMMapSize = 3};

  DepthModeToBitDepthMap depthModeMap[kDMMapSize] =
  {
    {0, SST_DESKTOP_PIXEL_PAL8,   1, 0, 0, __swizzleWrite32_8, __swizzleWrite32_32},
#if H5
    {3, SST_DESKTOP_PIXEL_RGB1555U, 2, 0, 0, __swizzleWrite32_16, __swizzleWrite16_16},
#else    
    {0, SST_DESKTOP_PIXEL_RGB565, 2, 1, SST_RAWLFB_BYTE_SWIZZLE_EN | SST_RAWLFB_WORD_SWIZZLE_EN, __swizzleWrite32_16, __swizzleWrite16_16},
#endif    
    {1,  SST_DESKTOP_PIXEL_RGB32,  4, 0, SST_RAWLFB_BYTE_SWIZZLE_EN, __swizzleWrite32_32, __swizzleWrite32_8},
  };

  GDXErr err = kGDXErrDisplayModeIDUnsupported;

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;

  LOG_ENTRY(FN_LEVEL);
  

  /* convert the mode to width/height/refresh values */
  for (i = 0 ; i < kMaxDisplayModeIDs ; i++) {
    if (displayModeIDMap[i].displayModeID == displayModeID) {
      width = displayModeIDMap[i].horizontalPixels;
      height = displayModeIDMap[i].verticalLines;
      refresh = displayModeIDMap[i].refreshRate >> 16;
      err = kGDXErrNoError;
      break;
    }
  }

  if (err)
    goto ErrorExit;

  LOG_PRINTF2(FN_LEVEL, "ProgramHardware width = %d, height = %d)\n",width,height);
  LOG_PRINTF2(FN_LEVEL, "ProgramHardware refresh = %d, depthMode = %d)\n",refresh,depthMode - kDepthMode1);
  
  /* Make sure that we know what to program for the requested 'depthMode' */
  if(depthMode < kDepthMode1 || depthMode > kDepthMode3) {
    err = kGDXErrDepthModeUnsupported;
    goto ErrorExit;
  }

  pixelFormat = depthModeMap[depthMode - kDepthMode1].pixelFormat;
  bytesPerPixel = depthModeMap[depthMode - kDepthMode1].bytesPerPixel;
  clutSelect = depthModeMap[depthMode - kDepthMode1].clutSelect;
  swizzle = depthModeMap[depthMode - kDepthMode1].swizzle;
  swizzleOffset = avengerHALData->swizzleOffsets[depthModeMap[depthMode - kDepthMode1].swizzleOffsetIndex];  

  /* If we get a mode change request when we are still in exclusive mode, then we
     simply record what the requested display/depth mode was and return a pointer to our
     dummy framebuffer. */      
  if(avengerHALData->fifo.exclusiveMode) {
    avengerHALData->displayMode = displayModeID;
    avengerHALData->depthMode = depthMode;
    avengerHALData->pixelDepth = bytesPerPixel;
    avengerHALData->swizzleOffset = 0;
    
    /* Free our framebuffer with the HRM */
    if(avengerHALData->frameBufferBlock) {
      avengerHALData->hrmFreeBlock(avengerHALData->frameBufferBlock);
      avengerHALData->frameBufferBlock = 0; 
    }
    
    if(baseAddress) {
      *baseAddress = (char *)&avengerHALData->dummyFrameBuffer[0];
    }
    
	if(directColor) {
	  if (depthMode > kDepthMode1)
	    *directColor = true;            /* > 8 bpp, so directColor == true */
	  else
	    *directColor = false;           /* == 8 bpp, so using indexed (CLUT) color */
	}
	return kGDXErrNoError;
  }


  /* Place framebuffer at the top of memory */
  memUsed = ((width * bytesPerPixel * height + kCursorVRAM)
            + avengerHALData->memBlockMask) & ~avengerHALData->memBlockMask;
  avengerHALData->frameBufferOffset = (bInfo->h3Mem * 1024 * 1024) - memUsed + kCursorVRAM;
  avengerHALData->frameBufferBase = avengerHALData->bInfo.regInfo.rawLfbBase + avengerHALData->frameBufferOffset;
  avengerHALData->frameBufferSize = memUsed;
#if H5  
  avengerHALData->swizzleOffset = swizzleOffset;
#endif  
  /* avengerHALData->bInfo.fifoInfo.fifoStart = avengerHALData->frameBufferOffset - kCursorVRAM - kFifoVRAM; */
  if(baseAddress) {
    *baseAddress = (char *)avengerHALData->frameBufferBase + avengerHALData->swizzleOffset; // Frame buffer base for all DisplayModeIDs
  }
  LOG_PRINTF3(FN_LEVEL, "frameBufferOffset = 0x%08x, Base = 0x%08x, Size = 0x%08x)\n",avengerHALData->frameBufferOffset,avengerHALData->frameBufferBase, avengerHALData->frameBufferSize);
  LOG_PRINTF1(FN_LEVEL, "bInfo->h3Mem = %d)\n",bInfo->h3Mem);


  if(AvengerSetMode( width, height, refresh) != kGDXErrNoError) {
    err = kGDXErrDisplayModeIDUnsupported;
    goto ErrorExit;
  }

  /* Have to set this AFTER we have programmed the display. */
  avengerHALData->displayMode = displayModeID;
  avengerHALData->depthMode = depthMode;
  avengerHALData->pixelDepth = bytesPerPixel;

#if 1
  /* update the HRM memory manager */
  if(avengerHALData->dynamicArea) {
    /* First, free and invalidate our old framebuffer block if we have one. */
    if(avengerHALData->frameBufferBlock) {
      avengerHALData->hrmFreeBlock(avengerHALData->frameBufferBlock);
      avengerHALData->frameBufferBlock = 0; 
      /* Now Invalidate any memory blocks in the same range as our framebuffer */
      avengerHALData->hrmInvalidateMemoryBlocks(avengerHALData->hrmBoard,
                                                avengerHALData->frameBufferBase,
                                                avengerHALData->frameBufferSize);
    }
    
    /* Now we should be able to allocate our framebuffer and get it at the same location */
    avengerHALData->frameBufferBlock = avengerHALData->hrmAllocateBlock(avengerHALData->hrmBoard,
                                       avengerHALData->frameBufferSize,
                                       HRM_MEMF_DESKTOP|HRM_MEMF_REVERSE,0);
    if(avengerHALData->frameBufferBlock) {
      LOG_PRINTF2(FN_LEVEL, "allocated at: %08lx should be: %08lx\n",
        avengerHALData->frameBufferBlock->start,
        avengerHALData->frameBufferBase - kCursorVRAM);
    } else {
      LOG_PRINTF(FN_LEVEL, "couldn't allocate framebuffer from HRM, WTF??\n");
    }
  }
#endif  


  /* Program gfx controller */
  h3InitVideoDesktopSurface(avengerHALData->bInfo.regInfo.ioPortBase,
    1,0,pixelFormat,0,clutSelect,
    avengerHALData->frameBufferOffset,width * bytesPerPixel);
#if scaled_desktop
  if ( !avengerHALData->fifo.exclusiveMode )
  {
    AvengerInitDesktopScaler(width, height, depthMode );
  }
#endif 

  /* Enable LFB swizzle stuff */
  LOG_PRINTF(FN_LEVEL, "adjusting swizzle settings\n");
#if H5  
	{
		FxU32 miscInit1;
		HWC_IO_LOAD(bInfo->regInfo,miscInit1,miscInit1);
		miscInit1 &= ~( SST_BYTE_SWIZZLE_SELECT | SST_BYTE_SWIZZLE_ENABLE);
		miscInit1 |=  SST_BYTE_SWIZZLE_64MB | SST_BYTE_SWIZZLE_ENABLE;
		HWC_IO_STORE(bInfo->regInfo,miscInit1,miscInit1);
	}
#else
  HWC_IO_LOAD(bInfo->regInfo,miscInit0,miscInit0);
  miscInit0 &= ~( SST_RAWLFB_BYTE_SWIZZLE_EN | SST_RAWLFB_WORD_SWIZZLE_EN);
  miscInit0 |=  swizzle;
  HWC_IO_STORE(bInfo->regInfo,miscInit0,miscInit0);  
#endif

  if(directColor) {
    if (depthMode > kDepthMode1)
      *directColor = true;            /* > 8 bpp, so directColor == true */
    else
      *directColor = false;           /* == 8 bpp, so using indexed (CLUT) color */
  }

#if H5
  avengerHALData->fifo.setLfb = __swizzleWrite32_8;
  avengerHALData->fifo.setLfbHost = __swizzleWrite32_32;
#else  
  avengerHALData->fifo.setLfb = depthModeMap[depthMode - kDepthMode1].setLfb;
  avengerHALData->fifo.setLfbHost = depthModeMap[depthMode - kDepthMode1].setLfbHost;
#endif


  /* update fifo function pointers */
  LOG_PRINTF(FN_LEVEL, "updating fifo function pointers\n");
  if(avengerHALData->hrmFifoUpdate) {
    hrmFifoUpdate_t fifoUpdate;
    fifoUpdate.board = avengerHALData->hrmBoard;
    fifoUpdate.setLfb = avengerHALData->fifo.setLfb;
    fifoUpdate.setLfbHost = avengerHALData->fifo.setLfbHost;

    avengerHALData->hrmFifoUpdate(&fifoUpdate);
  }

  LOG_PRINTF(FN_LEVEL, "done...\n");
  err = kGDXErrNoError;

ErrorExit:
  LOG_EXIT(5,err);
  return (err);

#undef FN_NAME  
#undef FN_LEVEL
}
/*
**=====================================================================================================
**
** GraphicsHALDrawHardwareCursor()
**  This routine sets the cursor's X and Y coordinates and its visible state.  If the cursor was set
**  successfully by a previous call to GraphicsHALSetHardwareCursor(), then the HAL must program the
**  hardware with the given X, Y and visible state.  If the previous call to
**  GraphicsHALSetHardwareCursor() failed, then an error should be returned.
**
**    -> x    X coordinate
**    -> y    Y coordinate
**    -> visible  'true' if the cursor must be visible.
**
**=====================================================================================================
*/
GDXErr  GraphicsHALDrawHardwareCursor(SInt32 x, SInt32 y, Boolean visible)
{
#define FN_NAME "GraphicsHALDrawHardwareCursor"
#define FN_LEVEL 10

#if HARDWARE_CURSOR
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 vidProcCfg;

  LOG_ENTRY(FN_LEVEL);
  
  /* Don't enable cursor if we're in exclusive mode or the cursor image is not valid. */
  if(avengerHALData->fifo.exclusiveMode || !avengerHALData->cursorSet)
    return kGDXErrCannotRenderCursorImage;
    
  /* HACK -- Just see if I can program the 2D hardware to enable the cursor. */
  avengerHALData->cursorX = x;
  avengerHALData->cursorY = y;
  avengerHALData->cursorVisible = visible;

  /* Set X, Y location. */
  HWC_IO_STORE(bInfo->regInfo,hwCurLoc,(x+63) | ((y+63) << 16));

  /* Enable/disable cursor */
  HWC_IO_LOAD(bInfo->regInfo,vidProcCfg,vidProcCfg);
  vidProcCfg &= ~(SST_CURSOR_EN | SST_CURSOR_MODE);
  if(visible && !avengerHALData->fifo.exclusiveMode)
    vidProcCfg |= SST_CURSOR_EN;
  HWC_IO_STORE(bInfo->regInfo,vidProcCfg,vidProcCfg);
  return (kGDXErrNoError);
#else
  return (kGDXErrUnsupportedFunctionality);
#endif

#undef FN_NAME
#undef FN_LEVEL
}





#pragma options align=mac68k

#define kNumHardwareCursorColors 2

struct HWCursorColorTable {
  SInt32      ctSeed;
  SInt16      ctFlags;
  SInt16      ctSize;
  ColorSpec    ctTable[kNumHardwareCursorColors];
};
typedef struct HWCursorColorTable HWCursorColorTable;

#pragma options align=reset

/*
**=====================================================================================================
**
** GraphicsHALSetHardwareCursor()
**  This routine is called to setup the hardware cursor and determine if whether the hardware can
**  support it.  The HAL should remember whether this call was successful for subsequent
**  GetHardwareCursorDrawState() or DrawHardwareCursor() calls, but should NOT change the cursor's
**  X or Y coordinates, nor its visible state.
**
**    -> gamma
**    Current gamma table to correct cursor colors with, if the HAL can apply gamma correction.
**
**    -> luminanceMapping
**    This will be true if the Core had luminance mapping enabled AND it was in an indexed color
**    mode.  If 'true', the HAL should luminance map the cursor CLUT EVEN if the hardware cursor is
**    a super-duper cursor capable of direct color. This is because the hardware cursor should look
**    like the software cursor it is replacing.
**
**    -> cursorRef
**    Opaque data to be handed to VSLPrepareCursorForHardwareCursor().
**
**=====================================================================================================
*/
GDXErr GraphicsHALSetHardwareCursor(const GammaTbl *gamma, Boolean luminanceMapping, void *cursorRef)
{
#define FN_NAME "GraphicsHALSetHardwareCursor"
#define FN_LEVEL 10

#if HARDWARE_CURSOR
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  HardwareCursorInfoRec    hardwareCursorInfo;
  HWCursorColorTable      colorMap;
  FxU32 colorEncodings[] = { 0, 1 }, i;
  FxU8 cursorData[1024];
  HardwareCursorDescriptorRec  hardwareCursorDescription;

  LOG_ENTRY(FN_LEVEL);
  /* Bail out when in our magic display mode */
  if( avengerHALData->fifo.exclusiveMode || avengerHALData->scalerNeeded || avengerHALData->scaler2x )
    return kGDXErrCannotRenderCursorImage;

  /* Set up our description */
  hardwareCursorDescription.majorVersion = kHardwareCursorDescriptorMajorVersion;
  hardwareCursorDescription.minorVersion = kHardwareCursorDescriptorMinorVersion;
  hardwareCursorDescription.height = 64;
  hardwareCursorDescription.width = 64;
  hardwareCursorDescription.bitDepth = 2;
  hardwareCursorDescription.maskBitDepth = 0;
  hardwareCursorDescription.numColors = 2;
  hardwareCursorDescription.colorEncodings = (FxU32 *)&colorEncodings;
  hardwareCursorDescription.flags = 0;
  hardwareCursorDescription.supportedSpecialEncodings = kTransparentEncodedPixel | kInvertingEncodedPixel;
  hardwareCursorDescription.specialEncodings[0] = 2;
  hardwareCursorDescription.specialEncodings[1] = 3;
  for (i = 2; i < 16; i++)
    hardwareCursorDescription.specialEncodings[i] = 0;

  /* Clear out Info */
  hardwareCursorInfo.majorVersion = 0;
  hardwareCursorInfo.minorVersion = 0;
  hardwareCursorInfo.cursorHeight = 0;
  hardwareCursorInfo.cursorWidth = 0;
  hardwareCursorInfo.colorMap = (CTabPtr) &colorMap;
  hardwareCursorInfo.hardwareCursor = (char *)cursorData;
  for (i = 0; i < 6; i++)
    hardwareCursorInfo.reserved[i] = 0;

  avengerHALData->cursorSet = VSLPrepareCursorForHardwareCursor(cursorRef, &hardwareCursorDescription,
    &hardwareCursorInfo);

  if(avengerHALData->cursorSet) {
    FxU32 x, y;
    FxU8 *dst0, *dst1;
    FxU16 *src = (FxU16 *)cursorData;
    FxU32 *dst = (FxU32 *)((FxU32)avengerHALData->bInfo.regInfo.rawLfbBase + avengerHALData->frameBufferOffset - kCursorVRAM);
#if !H5
    FxU32 miscInit0, miscInitSave;
#endif
    if (luminanceMapping) {
      /*
      ** Convert the RGB colors into luminance mapped gray scale.
      ** For those of us familiar with color space theory,
      **
      **    Luminance = .299Red + .587Green + .114Blue
      **    ("Video Demystified" by Keith Jack, page 28)
      **
      ** Conveniently, on the PowerPC architechture, floating point math is FASTER
      ** than integer math, so we will do outright floating point and not even
      ** think about playing games with Fixed Point math.
      */
      double redPortion;          /* Luminance portion from red component   */
      double greenPortion;        /* Luminance portion from green component */
      double bluePortion;         /* Luminance portion from blue component  */
      double luminance;           /* Resulting luminosity                   */

      for (i = 0 ; i < kNumHardwareCursorColors ; i++) {

        redPortion = 0.299 * colorMap.ctTable[i].rgb.red;
        greenPortion = 0.587 * colorMap.ctTable[i].rgb.green;
        bluePortion = 0.114 * colorMap.ctTable[i].rgb.blue;

        luminance = redPortion + greenPortion + bluePortion;

        /* I'm paranoid. */
        if(luminance < 0.0) luminance = 0.0;
        if(luminance > 255.0) luminance = 255.0;

        colorMap.ctTable[i].rgb.red = luminance;
        colorMap.ctTable[i].rgb.green = luminance;
        colorMap.ctTable[i].rgb.blue = luminance;
      }
    }

    avengerHALData->curC0 = colorMap.ctTable[0].rgb.red << RED_SHIFT |
                colorMap.ctTable[0].rgb.green << GREEN_SHIFT |
                colorMap.ctTable[0].rgb.blue << BLUE_SHIFT;
    avengerHALData->curC1 = colorMap.ctTable[1].rgb.red << RED_SHIFT |
                colorMap.ctTable[1].rgb.green << GREEN_SHIFT |
                colorMap.ctTable[1].rgb.blue << BLUE_SHIFT;

    /* Set up new colors. */
    HWC_IO_STORE(bInfo->regInfo,hwCurC0,avengerHALData->curC0);
    HWC_IO_STORE(bInfo->regInfo,hwCurC1,avengerHALData->curC1);

    /* Make sure pattern register is set correctly */
    HWC_IO_STORE(bInfo->regInfo,hwCurPatAddr,avengerHALData->frameBufferOffset - kCursorVRAM);

    /* Clear existing cursor to transparent */
    for(y = 0; y < 64; y++) {
      *dst++ = 0xFFFFFFFF;
      *dst++ = 0xFFFFFFFF;
      *dst++ = 0x0;
      *dst++ = 0x0;
    }

    /* Temporarily Disable LFB swizzle stuff */
#if !H5    
    HWC_IO_LOAD(bInfo->regInfo,miscInit0,miscInitSave);
    miscInit0 = miscInitSave & ~( SST_RAWLFB_BYTE_SWIZZLE_EN | SST_RAWLFB_WORD_SWIZZLE_EN);
    HWC_IO_STORE(bInfo->regInfo,miscInit0,miscInit0);
#endif

    /* I have to convert from two-bit chunky to two-bit planar format. */
    for(y = 0; y < hardwareCursorInfo.cursorHeight; y++) {
      /* Set up destinations for upper & lower 64-bit chunks of each line. */
      dst0 = (FxU8 *)((FxU32)avengerHALData->bInfo.regInfo.rawLfbBase + avengerHALData->frameBufferOffset - kCursorVRAM + 16*y + 0);
      dst1 = (FxU8 *)((FxU32)avengerHALData->bInfo.regInfo.rawLfbBase + avengerHALData->frameBufferOffset - kCursorVRAM + 16*y + 8);

      for(x = 0; x < hardwareCursorInfo.cursorWidth / 8; x++) {
        FxU16 srcData = *src++;
        FxU8 dstData0 = 0, dstData1 = 0;

        /* Shift data out into planes */
        for(i = 0; i < 8; i++) {
          dstData0 <<= 1;
          dstData0 |= (srcData & 0x8000) >> 15;
          dstData1 <<= 1;
          dstData1 |= (srcData & 0x4000) >> 14;
          srcData <<= 2;
        }
        /* Store planar data. */
        *dst0++ = dstData0;
        *dst1++ = dstData1;
      }
    }

    /* Restore swizzle mode */
#if !H5
    HWC_IO_STORE(bInfo->regInfo,miscInit0,miscInitSave);
#endif
  }
  return (avengerHALData->cursorSet ? kGDXErrNoError : kGDXErrCannotRenderCursorImage);
#else
  return (kGDXErrUnsupportedFunctionality);
#endif

#undef FN_NAME
#undef FN_LEVEL
}


/*
**=====================================================================================================
**
** GraphicsHALSupportsHardwareCursor()
**  This call is used to determine if the HAL supports a hardware cursor.
**
**    <- supportsHardwareCursor 'true' if supports a hardware cursor, 'false' otherwise.
**
**=====================================================================================================
*/
GDXErr GraphicsHALSupportsHardwareCursor(Boolean *supportsHardwareCursor)
{
#if HARDWARE_CURSOR
  *supportsHardwareCursor = true;
#else
  *supportsHardwareCursor = false;
#endif
  return (kGDXErrNoError);
}


/*
**=====================================================================================================
**
** GraphicsHALGetHardwareCursorDrawState()
**  This routine is used to determine the state of the hardware cursor.  After HAL initialization
**  the cursors visible state and set state should be false. After a mode change the cursor should be
**  made invisible but the set state should remain unchanged.
**
**    <- csCursorX    X coordinate from last DrawHardwareCursor call
**    <- csCursorY    Y coordinate from last DrawHardwareCursor call
**    <- csCursorVisible  'true' if the cursor is visible
**    <- csCursorSet    'true' if last GraphicsHALSetHardwareCursor() call was successful.
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetHardwareCursorDrawState(SInt32  * cursorX, SInt32  * cursorY,
    UInt32  *cursorVisible, UInt32  *cursorSet)
{
#if HARDWARE_CURSOR
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;

  *cursorX = avengerHALData->cursorX;
  *cursorY = avengerHALData->cursorY;
  *cursorVisible = avengerHALData->cursorVisible;
  *cursorSet = avengerHALData->cursorSet;

  return (kGDXErrNoError);
#else
  return (kGDXErrUnsupportedFunctionality);
#endif
}


/*
**=====================================================================================================
**
** GraphicsHALSetPowerState()
**  The graphics hw might have the ability to to go into some kind of power saving mode.  Just
**  pass the call to the HAL
**
**  For this routine, the relevant fields indicated by 'VDPowerStateRec' are:
**      ->  powerState    desired power mode: kAVPowerOff, kAVPowerStandby, kAVPowerSuspend,
**                kAVPowerOn
**
**      <- powerFlags   kPowerStateNeedsRefresh bit set if hw needs to be refreshed after
**                coming out of the designated power state.
**
**=====================================================================================================
*/
extern void AvengerEnableVBLInterrupts(void *vblRefCon);
extern unsigned char AvengerDisableVBLInterrupts(void *vblRefCon);

GDXErr GraphicsHALSetPowerState(VDPowerStateRec *powerState)
{
#define FN_NAME "GraphicsHALSetPowerState"
#define FN_LEVEL 2
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;

  FxU32 miscInit1, dacMode, displayMode;
  LOG_ENTRY(FN_LEVEL);

  powerState->powerFlags = 0;

  /* Big nasty state machine. */
  switch(powerState->powerState) {
    case kAVPowerOn:
      LOG_PRINTF(FN_LEVEL,"kAVPowerOn\n");
      /* Restore hsync/vsync signals.   */
      HWC_IO_LOAD( bInfo->regInfo, dacMode, dacMode );
      dacMode &= ~(DAC_DPMS_HSYNC|DAC_HSYNC_HIGH|DAC_DPMS_VSYNC|DAC_VSYNC_HIGH);
      HWC_IO_STORE( bInfo->regInfo, dacMode, dacMode );
      /* turn on DVI output */
      if( avengerHALData->displayCode == kDisplayCodeDVI )
      {
        FxU32 value;
        HWC_IO_LOAD(bInfo->regInfo, vidInFormat, value);
        value |= 0x00408000; /* TV out enable and descramble colors */
        HWC_IO_STORE(bInfo->regInfo, vidInFormat, value);
      }
      break;
    
    case kAVPowerStandby:
      LOG_PRINTF(FN_LEVEL,"kAVPowerStandby\n");
      /* Force HSync low */
      HWC_IO_LOAD( bInfo->regInfo, dacMode, dacMode );
      dacMode &= ~(DAC_DPMS_HSYNC|DAC_HSYNC_HIGH|DAC_DPMS_VSYNC|DAC_VSYNC_HIGH);
      dacMode |= DAC_DPMS_HSYNC;
      HWC_IO_STORE( bInfo->regInfo, dacMode, dacMode );    
      /* turn on DVI output */
      if( avengerHALData->displayCode == kDisplayCodeDVI )
      {
        FxU32 value;
        HWC_IO_LOAD(bInfo->regInfo, vidInFormat, value);
        value &= ~0x00408000; /* TV out enable and descramble colors */
        HWC_IO_STORE(bInfo->regInfo, vidInFormat, value);
      }
      break;
    
    case kAVPowerSuspend:
      /* Force VSync low */
      LOG_PRINTF(FN_LEVEL,"kAVPowerSuspend\n");
      HWC_IO_LOAD( bInfo->regInfo, dacMode, dacMode );
      dacMode &= ~(DAC_DPMS_HSYNC|DAC_HSYNC_HIGH|DAC_DPMS_VSYNC|DAC_VSYNC_HIGH);
      dacMode |= DAC_DPMS_VSYNC;
      HWC_IO_STORE( bInfo->regInfo, dacMode, dacMode );    
      if( avengerHALData->displayCode == kDisplayCodeDVI )
      {
        FxU32 value;
        HWC_IO_LOAD(bInfo->regInfo, vidInFormat, value);
        value &= ~0x00408000; /* TV out enable and descramble colors */
        HWC_IO_STORE(bInfo->regInfo, vidInFormat, value);
      }
      break;
    
    case kAVPowerOff:
      /* Force HSync and VSync low */
      LOG_PRINTF(FN_LEVEL,"kAVPowerOff\n");
//      HWC_IO_LOAD( bInfo->regInfo, miscInit1, miscInit1 );
//      miscInit1 |= SST_POWERDOWN_DAC;
//      HWC_IO_STORE( bInfo->regInfo, miscInit1, miscInit1 );    
//      AvengerProgramCLUTColor(0);
      HWC_IO_LOAD( bInfo->regInfo, dacMode, dacMode );
      dacMode |= DAC_DPMS_HSYNC|DAC_HSYNC_HIGH|DAC_DPMS_VSYNC|DAC_VSYNC_HIGH;
      HWC_IO_STORE( bInfo->regInfo, dacMode, dacMode );    
      if( avengerHALData->displayCode == kDisplayCodeDVI )
      {
        FxU32 value;
        HWC_IO_LOAD(bInfo->regInfo, vidInFormat, value);
        value &= ~0x00408000; /* TV out enable and descramble colors */
        HWC_IO_STORE(bInfo->regInfo, vidInFormat, value);
      }
      break;
    
    case kHardwareSleep:
      LOG_PRINTF(FN_LEVEL,"kHardwareSleep\n");
      /* Putting hardware to sleep */
      AvengerDeviceSleep(avengerHALData);
    break;
    
    case kHardwareWake: 
    case kHardwareWakeFromSuspend:
    {
      extern GDXErr GraphicsCoreGrayPage(const VDPageInfo *pageInfo);
      VDPageInfo pageInfo;

      /* Waking up hardware */
      LOG_PRINTF(FN_LEVEL,"kHardwareWake\n");
      AvengerDeviceWakeup(avengerHALData);
      LOG_PRINTF(FN_LEVEL,"GraphicsHALProgramHardware\n");
      displayMode = avengerHALData->displayMode;
      avengerHALData->displayMode = 0;
      GraphicsHALProgramHardware(displayMode, avengerHALData->depthMode, 0, NULL, NULL);
      LOG_PRINTF(FN_LEVEL,"AvengerProgramCLUT\n");
      AvengerProgramCLUT();
      pageInfo.csMode = 0;
      pageInfo.csData = 0;
      pageInfo.csPage = 0;
      pageInfo.csBaseAddr = 0;
      /* Temp */
      LOG_PRINTF(FN_LEVEL,"DPMS off\n");
      HWC_IO_LOAD( bInfo->regInfo, dacMode, dacMode );
      dacMode &= ~(DAC_DPMS_HSYNC|DAC_HSYNC_HIGH|DAC_DPMS_VSYNC|DAC_VSYNC_HIGH);
      HWC_IO_STORE( bInfo->regInfo, dacMode, dacMode );
//      HWC_IO_LOAD( bInfo->regInfo, miscInit1, miscInit1 );
//      miscInit1 &= ~SST_POWERDOWN_DAC;
//      HWC_IO_STORE( bInfo->regInfo, miscInit1, miscInit1 );    

      GraphicsCoreGrayPage(&pageInfo);
    }
  }
  
  /* We always report that we succeeded, even if we didn't. */
  avengerHALData->powerState = powerState->powerState;
  powerState->powerFlags = kPowerStateNeedsRefreshMask | kPowerStateSleepAwareMask | kPowerStateSleepCanPowerOffMask;
  LOG_EXIT(FN_LEVEL,0);
  return (kGDXErrNoError);

#undef FN_NAME  
#undef FN_LEVEL
}


/*
**=====================================================================================================
**
** GraphicsHALGetPowerState()
**  The graphics hw might have the ability to to go into some kind of power saving mode.  Just
**  pass the call to the HAL
**
**  For this routine, the relevant fields indicated by 'VDPowerStateRec' are:
**      <- powerState   current power mode: kAVPowerOff, kAVPowerStandby, kAVPowerSuspend,
**                kAVPowerOn
**
**      <- powerFlags   kPowerStateNeedsRefresh bit set if hw needs to be refreshed after
**                coming out of power state
**
**=====================================================================================================
*/
GDXErr GraphicsHALGetPowerState(VDPowerStateRec *powerState)
{
#define FN_NAME "GraphicsHALGetPowerState"
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  LOG_ENTRY(2);
  powerState->powerState = avengerHALData->powerState;
//  powerState->powerFlags = kPowerStateNeedsRefreshMask | kPowerStateSleepAwareMask | kPowerStateSleepCanPowerOffMask;
  powerState->powerFlags = kPowerStateNeedsRefreshMask | kPowerStateSleepAwareMask;
  LOG_EXIT(2,0);
  return (kGDXErrNoError);
#undef FN_NAME  
}


/*
**=====================================================================================================
**
** GraphicsHALPrivateControl()
**  Routine accepts private control calls.  The core passes all control codes that it doesn't
**  understand to this routine.  If the HAL knows what to do with the code, it hands it off
**  accordingly.
**
**    -> genericPtr     Points to the data structure that the HAL needs for this control
**                call.  Should be cast to appropriate data type if internal routine is
**                invoked.
**    -> privateControlCode The private csCode that the HAL might know what to do with.
**
**
**=====================================================================================================
*/
OSErr GraphicsHALPrivateControl(void * genericPtr, SInt16 privateControlCode)
{
#define FN_NAME "GraphicsHALPrivateControl"
#define FN_LEVEL 0
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  hwcControl_t *control = (hwcControl_t *)genericPtr;
  
  hwcRequest_t *req;
  hwcResponse_t *res;
  FxU32 code;
  
  GDXErr err = kGDXErrUnknownError;     /* Assume failure. */
  OSErr returnErr = controlErr;         /* Assume cs code is invalid */

  LOG_ENTRY(FN_LEVEL);

  if(privateControlCode != k3DfxNewRequest)
    goto ErrorExit;

  code = control->which;
  req = control->request;
  res = control->response;
    

  switch (code) {
    case k3DfxGetDeviceConfig: /* Get vendor/device ids. */
      LOG_PRINTF2(FN_LEVEL+5,"k3DfxGetDeviceConfig: %08lx %08lx\n",req,res);
      res->optData.deviceConfigRes.vendorID =          avengerHALData->bInfo.pciInfo.vendorID;
      res->optData.deviceConfigRes.deviceID =          avengerHALData->bInfo.pciInfo.deviceID;
      res->optData.deviceConfigRes.devRev =            avengerHALData->bInfo.devRev;
      res->optData.deviceConfigRes.ioPortBase =        avengerHALData->bInfo.regInfo.ioPortBase;
      res->optData.deviceConfigRes.hwBase =            avengerHALData->bInfo.regInfo.ioMemBase;
      res->optData.deviceConfigRes.lfbBase =           avengerHALData->bInfo.regInfo.rawLfbBase;
      res->optData.deviceConfigRes.h3Mem =             avengerHALData->bInfo.h3Mem;
      res->optData.deviceConfigRes.pciStride =         avengerHALData->pciStride;
      res->optData.deviceConfigRes.hwStride =          avengerHALData->hwStride;
      res->optData.deviceConfigRes.tileMark =          avengerHALData->tileMark;
      res->optData.deviceConfigRes.frameBufferOffset = avengerHALData->frameBufferOffset;
      res->optData.deviceConfigRes.prefs =             avengerHALData->halPreferences;
      res->optData.deviceConfigRes.isMaster =          avengerHALData->isMaster;
      res->optData.deviceConfigRes.numChips =          avengerHALData->numChips;
      res->optData.deviceConfigRes.supportsAGP =       avengerHALData->supportsAGP;
      res->optData.deviceConfigRes.swizzleOffsets[0] = avengerHALData->swizzleOffsets[0];
      res->optData.deviceConfigRes.swizzleOffsets[1] = avengerHALData->swizzleOffsets[1];
      res->optData.deviceConfigRes.swizzleOffsets[2] = avengerHALData->swizzleOffsets[2];
      res->optData.deviceConfigRes.swizzleOffsets[3] = avengerHALData->swizzleOffsets[3];
      returnErr = noErr;
      break;
#if 0
    case k3DfxSetModeFlags:
      LOG_PRINTF2(FN_LEVEL+5,"k3DfxSetModeFlags: %08lx %08lx\n",req,res);
      {
        FxU32 i;
        returnErr = kGDXErrDisplayModeIDUnsupported;
        for(i = 0; i < kMaxDisplayModeIDs; i++) {
          if(displayModeIDMap[i].displayModeID == req->optData.setModeFlagsReq.displayModeID) {
            displayModeIDMap[i].timingFlags = req->optData.setModeFlagsReq.timingFlags;
            returnErr = noErr;
          }
        }       
      }
      break;
#endif
    case k3DfxSetExclusive:
      LOG_PRINTF2(FN_LEVEL+5,"k3DfxSetExclusive: %08lx %08lx\n",req,res);
      LOG_PRINTF2(FN_LEVEL+5,"SetExclusive: %d  old = %d\n",req->optData.setExclusiveReq.exclusive,
      avengerHALData->fifo.exclusiveMode);
      
      if(req->optData.setExclusiveReq.exclusive) {
        if(avengerHALData->fifo.exclusiveMode == FXFALSE) {
          avengerHALData->fifo.exclusiveMode = FXTRUE;
          returnErr = noErr;
        } else  {
          returnErr = kGDXErrUnknownError;
        }
      } else {
        if(avengerHALData->fifo.exclusiveMode == FXTRUE) {
          avengerHALData->fifo.exclusiveMode = FXFALSE;
          NapalmResetSlaves();
          returnErr = noErr;
        } else {
          returnErr = kGDXErrUnknownError;
        }
      }
      /* Blank/Unblank the hardware cursor. */
      GraphicsHALDrawHardwareCursor(avengerHALData->cursorX,
                      avengerHALData->cursorY,
                      avengerHALData->cursorVisible);
      break;
 
    case k3DfxSetDisplayMode:
      LOG_PRINTF2(FN_LEVEL+5,"k3DfxSetDisplayMode: %08lx %08lx\n",req,res);
      LOG_PRINTF3(FN_LEVEL+5,"SetMode(%d,%d,%d)\n",
                                 req->optData.setModeReq.width,
                                 req->optData.setModeReq.height,
                                 req->optData.setModeReq.refresh);
      returnErr = AvengerSetMode(req->optData.setModeReq.width,
                                 req->optData.setModeReq.height,
                                 req->optData.setModeReq.refresh);
      avengerHALData->displayMode = 0;
      break;

    case k3DfxRegisterHRM:
      LOG_PRINTF2(FN_LEVEL+5,"k3DfxRegisterHRM: %08lx %08lx\n",req,res);    
      avengerHALData->hrmGetExtension = req->optData.hrmDispatchReq.hrmGetExtension;
      if(avengerHALData->hrmGetExtension) {
        /* Get the other extensions we need */
        avengerHALData->hrmFifoUpdate = avengerHALData->hrmGetExtension("hrmFifoUpdate");
        avengerHALData->hrmDisableFifo2D = avengerHALData->hrmGetExtension("hrmDisableFifo2D");
        avengerHALData->hrmEnableFifo2D = avengerHALData->hrmGetExtension("hrmEnableFifo2D");
        avengerHALData->hrmCreateMemoryArea = avengerHALData->hrmGetExtension("hrmCreateMemoryArea");
        avengerHALData->hrmDeleteMemoryArea = avengerHALData->hrmGetExtension("hrmDeleteMemoryArea");
        avengerHALData->hrmInvalidateMemoryBlocks = avengerHALData->hrmGetExtension("hrmInvalidateMemoryBlocks");
        avengerHALData->hrmAllocateBlock = avengerHALData->hrmGetExtension("hrmAllocateBlock");
        avengerHALData->hrmFreeBlock = avengerHALData->hrmGetExtension("hrmFreeBlock");
        avengerHALData->hrmBoard = req->optData.hrmDispatchReq.hrmBoard;
        
        if(avengerHALData->hrmFifoUpdate && avengerHALData->hrmDisableFifo2D && 
           avengerHALData->hrmEnableFifo2D && avengerHALData->hrmCreateMemoryArea &&
           avengerHALData->hrmDeleteMemoryArea && avengerHALData->hrmInvalidateMemoryBlocks &&
           avengerHALData->hrmAllocateBlock && avengerHALData->hrmFreeBlock) {
          
          /* Register current FIFO write functions */
          hrmFifoUpdate_t fifoUpdate;
          
          fifoUpdate.board = avengerHALData->hrmBoard;
          fifoUpdate.setLfb = avengerHALData->fifo.setLfb;
          fifoUpdate.setLfbHost = avengerHALData->fifo.setLfbHost;

          avengerHALData->hrmFifoUpdate(&fifoUpdate);
           
          /* Create memory area */
          avengerHALData->dynamicArea = avengerHALData->hrmCreateMemoryArea(avengerHALData->hrmBoard,
              avengerHALData->bInfo.regInfo.rawLfbBase,
              avengerHALData->bInfo.h3Mem * 1024 * 1024,
              HRM_MEMF_DESKTOP | HRM_MEMF_LINEAR);
          
          if(avengerHALData->dynamicArea && !avengerHALData->fifo.exclusiveMode) {
            /* "Allocate" our desktop memory chunk */
            avengerHALData->frameBufferBlock = avengerHALData->hrmAllocateBlock(avengerHALData->hrmBoard,
                                                                                   avengerHALData->frameBufferSize,
                                                                                   HRM_MEMF_DESKTOP|HRM_MEMF_REVERSE,0); 
            if(avengerHALData->frameBufferBlock) {
              LOG_PRINTF2(FN_LEVEL+5,"allocated at: %08lx should be: %08lx\n",
                avengerHALData->frameBufferBlock->start,
                avengerHALData->frameBufferBase - kCursorVRAM);
            } else {
              LOG_PRINTF(FN_LEVEL+5,"couldn't allocate framebuffer from HRM, WTF??\n");
            }
            
          }
                
          returnErr = noErr;
        }
          
      } else {
        /* HRM has gone away, so clear our other function pointers. */
        avengerHALData->hrmFifoUpdate = 0;
        avengerHALData->hrmDisableFifo2D = 0;
        avengerHALData->hrmEnableFifo2D = 0;
        avengerHALData->hrmCreateMemoryArea = 0;
        avengerHALData->hrmDeleteMemoryArea = 0;
        avengerHALData->hrmInvalidateMemoryBlocks = 0;
        avengerHALData->hrmAllocateBlock = 0;
        avengerHALData->hrmFreeBlock = 0;
        
        /* Dispose of our memory areas (actually, just NULL them out, the HRM nukes them for us. */
        avengerHALData->fixedArea = 0;
        avengerHALData->dynamicArea = 0;
        avengerHALData->hrmBoard = 0;
        avengerHALData->frameBufferBlock = 0;
      }
      returnErr = noErr;
      break;

    case k3DfxSetPrefs:
      LOG_PRINTF2(FN_LEVEL+5,"k3DfxSetPrefs: %08lx %08lx\n",req,res);
      {
        FxU32 grxClock;

        avengerHALData->halPreferences = req->optData.setPrefsReq.prefs;
        
        /* Do a little sanity checking */
        grxClock = (avengerHALData->halPreferences & HALDATA_GRXCLOCK_MASK) >> HALDATA_GRXCLOCK_SHIFT;
        if(grxClock <= 30 || grxClock > 220) {
          grxClock = 0;
        }
        avengerHALData->halPreferences &= ~HALDATA_GRXCLOCK_MASK;
        avengerHALData->halPreferences |= (grxClock << HALDATA_GRXCLOCK_SHIFT);

        err = GraphicsOSSSetHALPref(&avengerHALData->regEntryID[0], avengerHALData->halPreferences);
        if(!err) {
          returnErr = noErr;
        }
        LOG_PRINTF1(FN_LEVEL+5,"SetHALPref err: %d\n",err);        
      }
      
    case k3DfxFifoFuncs:
      LOG_PRINTF2(FN_LEVEL+5,"k3DfxFifoFuncs: %08lx %08lx\n",req,res);
      res->optData.fifoFuncsRes.setLfb = avengerHALData->fifo.setLfb;
      res->optData.fifoFuncsRes.setLfbHost = avengerHALData->fifo.setLfbHost;     
      returnErr = noErr;
      break;
      
    case k3DfxPCIOp:
    {
      RegEntryID *regEntry;
            
      if(req->optData.pciOpReq.DeviceId >= avengerHALData->numChips) {
        returnErr = badUnitErr;
        goto ErrorExit;
      }
      
      regEntry = &avengerHALData->regEntryID[req->optData.pciOpReq.DeviceId];
      
      switch(req->optData.pciOpReq.Operation) {
        case k3DfxPCIOpRead:
        err = ExpMgrConfigReadLong(regEntry, (LogicalAddress) req->optData.pciOpReq.Offset, 
                                   &res->optData.pciOpRes.Value);
        returnErr = err;                           
        break;
        
        case k3DfxPCIOpWrite:
        err = ExpMgrConfigWriteLong(regEntry, (LogicalAddress) req->optData.pciOpReq.Offset, 
                                    req->optData.pciOpReq.Value);
        returnErr = err;                           
        break;
      }
      break;
    }
    
    case k3DfxSlaveRegs:
    {
      res->optData.slaveRegRes.Regs[0] = avengerHALData->devTable[req->optData.slaveRegReq.DeviceId].RegBase[0];    
      res->optData.slaveRegRes.Regs[1] = avengerHALData->devTable[req->optData.slaveRegReq.DeviceId].RegBase[1];    
      res->optData.slaveRegRes.Regs[2] = avengerHALData->devTable[req->optData.slaveRegReq.DeviceId].RegBase[2];    
      res->optData.slaveRegRes.Regs[3] = avengerHALData->devTable[req->optData.slaveRegReq.DeviceId].RegBase[3];    
      returnErr = noErr;
      break;
    }
#if V5_5000   
    case k3DfxSLIAA:
    {
      NapalmSLIAA(&avengerHALData->devTable[0],&req->optData.sliaaReq);
      break;
    }
#endif

    case k3DfxGetDriverInfo:
      LOG_PRINTF2(FN_LEVEL+5,"k3DfxGetDriverInfo: %08lx %08lx\n",req,res);
      res->optData.driverInfoRes.major = kMajorRev;
      res->optData.driverInfoRes.minor = kMinorAndBugRev;
      res->optData.driverInfoRes.stage = kStage;
      res->optData.driverInfoRes.rev = kNonRelRev;
      returnErr = noErr;
      break;
   
    default:
      goto ErrorExit;             /* csCode not supported */
  }

ErrorExit:
  LOG_EXIT(FN_LEVEL,returnErr);
  return (returnErr);

#undef FN_NAME  
#undef FN_LEVEL  
}


/*
**=====================================================================================================
**
** GraphicsHALPrivateStatus()
**  Routine accepts private status calls.  The core passes all status codes that it doesn't
**  understand to this routine.  If the HAL knows what to do with the code, it hands it off
**  accordingly.
**
**    -> genericPtr     Points to the data structure that the HAL needs for this status
**                call.  Should be cast to appropriate data type if internal routine is
**                invoked.
**    -> privateStatusCode  The private csCode that the HAL might know what to do with.
**
**
**=====================================================================================================
*/
OSErr GraphicsHALPrivateStatus(void * genericPtr, SInt16 privateStatusCode)
{
  GDXErr err = kGDXErrUnknownError;       /* Assume failure. */
  OSErr returnErr = statusErr;            /* Assume cs code is invalid */

  switch (privateStatusCode)
  {
    default:
      goto ErrorExit;             /* csCode not supported */
  }

ErrorExit:
  return (returnErr);
}


Boolean GraphicsHALIsExclusive(void)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  return (avengerHALData->fifo.exclusiveMode);
}








/* Custom backdoor routine for programming display hardware to a specific width/height/refresh
   without going through MacOS interface. */
GDXErr AvengerSetMode(FxU32 width, FxU32 height, FxU32 refresh)
{
#define FN_NAME "AvengerSetMode"
#define FN_LEVEL 0
  /* Define a new type which maps a 'DisplayModeID' to the value that will represent
     the appropriate 'GraphicsMode' for the hardware. */
  FxU32 vidProcCfg, swizzle, miscInit0, tmuGbeInit;
  FxU32 lfbMemoryConfig, syncPolarity;
  FxU8 scanLineDoubled = 0;
  GDXErr err = kGDXErrUnknownError;

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 forceResolutionChange = !avengerHALData->displayMode || width != avengerHALData->width || height != avengerHALData->height || refresh != avengerHALData->refresh || avengerHALData->fifo.exclusiveMode;
  
  LOG_ENTRY(FN_LEVEL);

  /* Temp kludge. */
  if(width == 400 && height == 300) {
    width = 800;
    height = 600;
  } else if(width == 512 && height == 384) {
    width = 1024;
    height = 768;
  }

  /* Bail on anything else */
  if(width < 640) {
    LOG_PRINTF2(FN_LEVEL, "this resolution is not supported (%d,%d)", width, height );
    return kGDXErrDisplayModeIDUnsupported;
  }
  
  /* Stomp on saved display mode ID so we reprogram hardware properly when the real display mode is restored. */
  /* avengerHALData->displayMode = 0; */

  /* Force tile mark to top of memory for now. */
  lfbMemoryConfig = SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE((avengerHALData->tileMark >> 12)) | SST_RAW_LFB_ADDR_STRIDE_1K | 0xa0000;
  HWC_IO_STORE(bInfo->regInfo, lfbMemoryConfig, lfbMemoryConfig);

  if(forceResolutionChange)
  {
    VidProcConfig * vpc = &avengerHALData->vpc;

    vpc->width = width;
    vpc->height = height;
    vpc->refresh = refresh;
      
    /* Program VGA registers for this display mode */
    if( avengerHALData->displayCode == kDisplayCodeDVI )
    {
      FxU32 value;
      
      HWC_IO_LOAD(bInfo->regInfo, vidInFormat, value);
      value |= 0x00408000; /* TV out enable and descramble colors */
      HWC_IO_STORE(bInfo->regInfo, vidInFormat, value);
    
      HWC_IO_LOAD(bInfo->regInfo, tmuGbeInit, value);
      value |= 0xF << 16; /* TV out delay adjustment */
      HWC_IO_STORE(bInfo->regInfo, tmuGbeInit, value);
    
      
      vpc->TimingParams.width = GetEDIDDetailedTimingHActive( avengerHALData->edid, 1 );
      vpc->TimingParams.height = GetEDIDDetailedTimingVActive( avengerHALData->edid, 1 );
      vpc->TimingParams.refresh = GetEDIDStdTimingRefresh( avengerHALData->edid, 1 );

      vpc->TimingParams.HTotal = vpc->TimingParams.width + GetEDIDDetailedTimingHBlank( avengerHALData->edid, 1 );
      vpc->TimingParams.HSyncStart = vpc->TimingParams.width + GetEDIDDetailedTimingHSyncOffset( avengerHALData->edid, 1 );
      vpc->TimingParams.HSyncEnd = vpc->TimingParams.HSyncStart + GetEDIDDetailedTimingHSyncWidth( avengerHALData->edid, 1 );

      vpc->TimingParams.VTotal = vpc->TimingParams.height + GetEDIDDetailedTimingVBlank( avengerHALData->edid, 1 );
      vpc->TimingParams.VSyncStart = vpc->TimingParams.height + GetEDIDDetailedTimingVSyncOffset( avengerHALData->edid, 1 );
      vpc->TimingParams.VSyncEnd = vpc->TimingParams.VSyncStart + GetEDIDDetailedTimingVSyncWidth( avengerHALData->edid, 1 );

      vpc->TimingParams.CRTCflags = 0;
      if ( (GetEDIDDetailedTimingFlags( avengerHALData->edid, 1 ) >> 3 & 0x3) == 3 ) /* digital syncs */
      {
        vpc->TimingParams.CRTCflags = ((GetEDIDDetailedTimingFlags( avengerHALData->edid, 1 ) & 0x02) ^ 0x02) << 1;
        vpc->TimingParams.CRTCflags |= ((GetEDIDDetailedTimingFlags( avengerHALData->edid, 1 ) & 0x04) ^ 0x04) << 1;
      }

      vpc->TimingParams.PixelClock = GetEDIDDetailedTimingPixelClock( avengerHALData->edid, 1 ) * 10000;
      vpc->TimingParams.CharWidth = 8;
      vpc->TimingParams.UseGTF = 0;
      vpc->TimingParams.UseAltTiming = 0;
      vpc->TimingParams.Checksum = 0;
      
//      avengerHALData->scaler2x = (2*width) <= vpc->TimingParams.width && (2*height) <= vpc->TimingParams.height;
#if scaled_desktop
      if ( !avengerHALData->fifo.exclusiveMode && !avengerHALData->scaler2x )
      {
        vpc->width = vpc->TimingParams.width;
        vpc->height = vpc->TimingParams.height;
      }
      
      if ( avengerHALData->scaler2x )
      {
        LOG_PRINTF(FN_LEVEL, "2x scaling...\n" );
        scanLineDoubled = 1;
        vpc->TimingParams.CRTCflags |= 0x01;
        vpc->width = 2 * width;
        vpc->height = 2 * height;
      }
#endif

    }
    else
    { /* this is only needed until we implement the complete dynamic mode table generation */
      
      vpc->TimingParams.width = vpc->width;
      vpc->TimingParams.height = vpc->height;
      vpc->TimingParams.refresh = vpc->refresh;

      if(FindMode( &vpc->TimingParams ))
      {
        err = kGDXErrDisplayModeIDUnsupported;
        goto ErrorExit;
      }
    }
    
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "width (request)", vpc->width );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "height (request)", vpc->height );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "refresh (request)", vpc->refresh );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "width", vpc->TimingParams.width );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "height", vpc->TimingParams.height );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "refresh", vpc->TimingParams.refresh );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "HTotal", vpc->TimingParams.HTotal );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "HSyncStart", vpc->TimingParams.HSyncStart );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "HSyncEnd", vpc->TimingParams.HSyncEnd );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "VTotal", vpc->TimingParams.VTotal );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "VSyncStart", vpc->TimingParams.VSyncStart );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "VSyncEnd", vpc->TimingParams.VSyncEnd );
    LOG_PRINTF2(FN_LEVEL, "%32s = 0x%02x\n", "CRTCflags", vpc->TimingParams.CRTCflags );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "PixelClock", vpc->TimingParams.PixelClock );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "CharWidth", vpc->TimingParams.CharWidth );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "UseGTF", vpc->TimingParams.UseGTF );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "UseAltTiming", vpc->TimingParams.UseAltTiming );
    LOG_PRINTF2(FN_LEVEL, "%32s = %d\n", "Checksum", vpc->TimingParams.Checksum );
    
    ds_Calc_CRTC_table( vpc, &avengerHALData->devTable[0] );
       
    LOG_PRINTF(FN_LEVEL, "CRTC table :\n" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 0, crtc_table[0], "byHTotal" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 1, crtc_table[1], "byHorDispEnEnd" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 2, crtc_table[2], "byHBlankStart" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 3, crtc_table[3], "byHBlankEnd" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 4, crtc_table[4], "byHSyncStart" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 5, crtc_table[5], "byHSyncEnd" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 6, crtc_table[6], "byVTotal" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 7, crtc_table[7], "byOverflow" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 8, crtc_table[8], "byMaxLineScan" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 9, crtc_table[9], "byVSyncStart" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 10, crtc_table[10], "byVSyncEnd" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 11, crtc_table[11], "byVertDispEnEnd" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 12, crtc_table[12], "byVBlankStart" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 13, crtc_table[13], "byVBlankEnd" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 14, crtc_table[14], "byHExtensions" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 15, crtc_table[15], "byVExtensions" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 16, crtc_table[16], "byMiscOutput" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 17, crtc_table[17], "bySeqDotClk" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 18, crtc_table[18], "wPllCtrl0 & 0xff" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 19, crtc_table[19], "wPllCtrl0 >> 8" );
    LOG_PRINTF3(FN_LEVEL, "crtc_table[%02d] = 0x%02x, (%s)\n", 20, crtc_table[20], "byDacMode" );

    if(h3InitSetVideoMode( &avengerHALData->devTable[0], vpc->width,vpc->height,refresh,FXFALSE,scanLineDoubled) == FXFALSE)
    {
      err = kGDXErrDisplayModeIDUnsupported;
      LOG_PRINTF(FN_LEVEL, "##### ERROR : DisplayModeID Unsupported\n" );
      goto ErrorExit;
    }

    /* Disable desktop and overlay surfaces */
    HWC_IO_LOAD(bInfo->regInfo, vidProcCfg, vidProcCfg);
    vidProcCfg &= ~(SST_DESKTOP_EN|SST_OVERLAY_EN);
    HWC_IO_STORE(bInfo->regInfo, vidProcCfg, vidProcCfg);


    /* Reset graphics controller since we possibly messed with the screen size */
    HWC_IO_LOAD(bInfo->regInfo, vidProcCfg, vidProcCfg);
    vidProcCfg &= ~(SST_VIDEO_PROCESSOR_EN); 
    HWC_IO_STORE(bInfo->regInfo, vidProcCfg, vidProcCfg);
    vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
    HWC_IO_STORE(bInfo->regInfo, vidProcCfg, vidProcCfg);
  }

  /* Disable LFB swizzle stuff */
#if !H5  
  HWC_IO_LOAD(bInfo->regInfo,miscInit0,miscInit0);
  miscInit0 &= ~( SST_RAWLFB_BYTE_SWIZZLE_EN | SST_RAWLFB_WORD_SWIZZLE_EN);
  HWC_IO_STORE(bInfo->regInfo,miscInit0,miscInit0);
#endif

  avengerHALData->fifo.setLfb = __swizzleWrite32_8;
  avengerHALData->fifo.setLfbHost = __swizzleWrite32_32;

#if ENABLE_INTERRUPTS 
  /* Enable interrupts now that we are sure the hardware is programmed to a valid display mode. */
  syncPolarity = ((*(volatile FxU8 *)(bInfo->regInfo.ioPortBase + 0xcc)) & 0x80);
  if(syncPolarity) { 
    /* Negative sync, trigger on falling edge */
    avengerHALData->intrCrtl = SST_INTR_PCI_INTA | SST_INTR_VSYNC_FALLING_ENABLE | SST_INTR_USER_INTR_ENABLE;
  } else {
    /* Positive sync, trigger on rising edge */
    avengerHALData->intrCrtl = SST_INTR_PCI_INTA | SST_INTR_VSYNC_RISING_ENABLE | SST_INTR_USER_INTR_ENABLE;
  }
  
  HWC_SST_STORE(bInfo->regInfo,intrCtrl,avengerHALData->intrCrtl);
#endif

  avengerHALData->width = width;
  avengerHALData->height = height;
  avengerHALData->refresh = refresh;

  err = kGDXErrNoError;

ErrorExit:
  LOG_EXIT(FN_LEVEL,err);
  return (err);
#undef FN_NAME  
#undef FN_LEVEL
}





/* Custom backdoor routine for programming display hardware to a specific width/height/refresh
   without going through MacOS interface. */
void AvengerSetModeSlave(FxU32 chipNumber, FxU32 width, FxU32 height, FxU32 refresh)
{
#define FN_NAME "AvengerSetModeSlave"
#define FN_LEVEL 0
  /* Define a new type which maps a 'DisplayModeID' to the value that will represent
     the appropriate 'GraphicsMode' for the hardware. */
  FxU32 vidProcCfg, swizzle, miscInit0, tmuGbeInit;
  FxU32 lfbMemoryConfig, syncPolarity;
  GDXErr err = kGDXErrUnknownError;

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  hwcRegInfo *rInfo = &avengerHALData->slaveRegInfo[chipNumber];
  
  LOG_ENTRY(FN_LEVEL);
  
  /* Stomp on saved display mode ID so we reprogram hardware properly when the real display mode is restored. */
  /* avengerHALData->displayMode = 0; */

  /* Force tile mark to top of memory for now. */
  lfbMemoryConfig = SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE((avengerHALData->tileMark >> 12)) | SST_RAW_LFB_ADDR_STRIDE_1K | 0xa0000;
  HWC_IO_STORE((*rInfo), lfbMemoryConfig, lfbMemoryConfig);

#if 1 /* overwrite the crtc regs with the old h3InitFindVideoMode() */
          LOG_PRINTF(FN_LEVEL, "Slave...\n" );
      {
        short i = 0;
        while ( i++ < CRTC_TABLE_SIZE )
          LOG_PRINTF2(FN_LEVEL, "crtc[%d] = 0x%02x\n", i, crtc_table[i]);
      }
#endif

  /* We have to disable IO decode on the Master and enable it on the slave. */
  AvengerDisableIO(0);
  AvengerEnableIO(chipNumber);

  /* CRTC regs used by the Master are still in crtc regs. */
  /* Program VGA registers for this display mode */
  if(h3InitSetVideoMode(&avengerHALData->devTable[chipNumber],
                        width,height,refresh,FXFALSE,FXFALSE) == FXFALSE) {
    err = kGDXErrDisplayModeIDUnsupported;
    AvengerDisableIO(chipNumber);
    AvengerEnableIO(0);
    goto ErrorExit;
  }
  
  /* Re-enable master IO */
  AvengerDisableIO(chipNumber);
  AvengerEnableIO(0);
  
  /* Disable desktop and overlay surfaces */
  HWC_IO_LOAD((*rInfo), vidProcCfg, vidProcCfg);
  vidProcCfg &= ~(SST_DESKTOP_EN|SST_OVERLAY_EN);
  HWC_IO_STORE((*rInfo), vidProcCfg, vidProcCfg);
  
  /* Make sure desktop size is set */
  HWC_IO_STORE((*rInfo), vidScreenSize, width | (height << SST_VIDEO_SCREEN_HEIGHT_SHIFT));

  /* Reset graphics controller since we possibly messed with the screen size */
  HWC_IO_LOAD((*rInfo), vidProcCfg, vidProcCfg);
  vidProcCfg &= ~(SST_VIDEO_PROCESSOR_EN); 
  HWC_IO_STORE((*rInfo), vidProcCfg, vidProcCfg);
  vidProcCfg |= SST_VIDEO_PROCESSOR_EN;
  HWC_IO_STORE((*rInfo), vidProcCfg, vidProcCfg);
  
ErrorExit:
  LOG_EXIT(FN_LEVEL,err);
#undef FN_NAME
#undef FN_LEVEL
}






FxBool
AvengerCalculatePPLvalue(
  FxU32 freq,   /* frequency */
  FxU16 * m,
  FxU16 * n,
  FxU16 * k)
{
#define FN_NAME "AvengerCalculatePPLvalue"
#define FN_LEVEL 1
  FxU16 bestn,bestm,bestk;
  FxU16 mt,kt;
  double test,dfreq,newoverflow,oldoverflow,rndtest;

  LOG_ENTRY(FN_LEVEL);

#if DEBUG
  {
    FxU32 freq_i = freq / 100;
    FxU32 freq_d = freq - ( freq_i * 100 );
    
    LOG_PRINTF2(FN_LEVEL, "freq = %d.%02d\n", freq_i, freq_d );
  }
#endif

  dfreq = (double) freq / 100.0;
  oldoverflow = 1000.0;  /* big number */
  bestn = bestm = bestk = 0;

  if( freq > 15000 )
    kt=1;
  else if ( freq > 6500)
    kt=2;
  else
    kt=3;

  /* Find the correct pllTable value for the pixel clock */
  for( mt=1 ; mt<64; mt++ )
  {
	// m should not start at 0 (Found empirically that m=0 will cause the
	// equation to be 14.31818*(n+2)/(1*2^k) So m adds 1 but not 2 here.)
	// As per Yancy's email on Feb. 8, 1999, use only m values >=10
    if ( (freq > 3600) && (mt>10) )
      break;
    if ( (freq > 20000) && (mt>5))
      break;

    test = dfreq * ((double) mt + 2.0) * 
      ((double) (8>>(3-kt))) / 14.31818;

    if( test>257.0 )
      continue;

    rndtest = round(test);

    newoverflow = dfreq - 14.31818 * rndtest/(((double) mt + 2.0) * ((double) (8>>(3-kt))));
    if(newoverflow < 0.0)
		newoverflow *= -1.0;

	// The first values found will give better results than the older values
	if(newoverflow < oldoverflow)
	{
      bestm = mt;
      bestn = (FxU16)(rndtest - 2.0);
      bestk = kt;
      oldoverflow = newoverflow;
    }
  }
    
  *m = bestm;
  *n = bestn;
  *k = bestk;

#if DEBUG
  {
    float ffreq = (14.31818f * ( ((float) *n) + 2.0f)) / 
                         ( (((float) *m) + 2.0f) * ((float) (8>>(3-*k))) );
    
    LOG_PRINTF1(FN_LEVEL, "freq = %.02f\n", ffreq );
  }
#endif



  LOG_EXIT(FN_LEVEL, 1);
  return FXTRUE;
    
#undef FN_NAME
#undef FN_LEVEL
}






static void AvengerDeviceSleep(AvengerHALData *avengerHALData)
{
#define FN_NAME "AvengerDeviceSleep"
#define FN_LEVEL 0
  FxU32 chipIndex, i;
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;

  LOG_ENTRY(5);

  // Save off 2D (WAX) registers on master
  // Note, the following registers are not saved:
  //   pattern0alias, pattern1alias, command
  HWC_WAX_LOAD(bInfo->regInfo, clip0min, avengerHALData->waxRegs.clip0min);
  HWC_WAX_LOAD(bInfo->regInfo, clip0max, avengerHALData->waxRegs.clip0max);
  HWC_WAX_LOAD(bInfo->regInfo, dstBaseAddr, avengerHALData->waxRegs.dstBaseAddr);
  HWC_WAX_LOAD(bInfo->regInfo, dstFormat, avengerHALData->waxRegs.dstFormat);
  HWC_WAX_LOAD(bInfo->regInfo, srcColorkeyMin, avengerHALData->waxRegs.srcColorkeyMin);
  HWC_WAX_LOAD(bInfo->regInfo, srcColorkeyMax, avengerHALData->waxRegs.srcColorkeyMax);
  HWC_WAX_LOAD(bInfo->regInfo, dstColorkeyMin, avengerHALData->waxRegs.dstColorkeyMin);
  HWC_WAX_LOAD(bInfo->regInfo, dstColorkeyMax, avengerHALData->waxRegs.dstColorkeyMax);
  HWC_WAX_LOAD(bInfo->regInfo, bresError0, avengerHALData->waxRegs.bresError0);
  HWC_WAX_LOAD(bInfo->regInfo, bresError1, avengerHALData->waxRegs.bresError1);
  HWC_WAX_LOAD(bInfo->regInfo, rop, avengerHALData->waxRegs.rop);
  HWC_WAX_LOAD(bInfo->regInfo, srcBaseAddr, avengerHALData->waxRegs.srcBaseAddr);
  HWC_WAX_LOAD(bInfo->regInfo, commandEx, avengerHALData->waxRegs.commandEx);
  HWC_WAX_LOAD(bInfo->regInfo, lineStipple, avengerHALData->waxRegs.lineStipple);
  HWC_WAX_LOAD(bInfo->regInfo, lineStyle, avengerHALData->waxRegs.lineStyle);
  HWC_WAX_LOAD(bInfo->regInfo, clip1min, avengerHALData->waxRegs.clip1min);
  HWC_WAX_LOAD(bInfo->regInfo, clip1max, avengerHALData->waxRegs.clip1max);
  HWC_WAX_LOAD(bInfo->regInfo, srcFormat, avengerHALData->waxRegs.srcFormat);
  HWC_WAX_LOAD(bInfo->regInfo, srcSize, avengerHALData->waxRegs.srcSize);
  HWC_WAX_LOAD(bInfo->regInfo, srcXY, avengerHALData->waxRegs.srcXY);
  HWC_WAX_LOAD(bInfo->regInfo, colorBack, avengerHALData->waxRegs.colorBack);
  HWC_WAX_LOAD(bInfo->regInfo, colorFore, avengerHALData->waxRegs.colorFore);
  HWC_WAX_LOAD(bInfo->regInfo, dstSize, avengerHALData->waxRegs.dstSize);
  HWC_WAX_LOAD(bInfo->regInfo, dstXY, avengerHALData->waxRegs.dstXY);
  for(i = 0; i < 64; i++) {
    HWC_WAX_LOAD(bInfo->regInfo, colorPattern[i], avengerHALData->waxRegs.colorPattern[i]);
  }
  
  // Save off Command FIFO registers on master
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.baseAddrL, avengerHALData->cmdRegs.cmdFifo0.baseAddrL);
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.baseSize, avengerHALData->cmdRegs.cmdFifo0.baseSize);
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.readPtrL, avengerHALData->cmdRegs.cmdFifo0.readPtrL);
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.readPtrH, avengerHALData->cmdRegs.cmdFifo0.readPtrH);
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.aMin, avengerHALData->cmdRegs.cmdFifo0.aMin);
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.aMax, avengerHALData->cmdRegs.cmdFifo0.aMax);
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.depth, avengerHALData->cmdRegs.cmdFifo0.depth);
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.holeCount, avengerHALData->cmdRegs.cmdFifo0.holeCount);
  HWC_CAGP_LOAD(bInfo->regInfo, cmdFifoThresh, avengerHALData->cmdRegs.cmdFifoThresh);

  // Save off extra PCI config space registers on all chips.  In theory the OS should grab the memory
  // base addresses for the slaves, but it may not.  So, we save them all anyway.
  for(chipIndex = 0; chipIndex < avengerHALData->numChips; chipIndex++) {
    ExpMgrConfigReadLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kMemBaseAddr0, (FxU32 *)&avengerHALData->configRegs[chipIndex].memBaseAddr0);
    LOG_PRINTF2(FN_LEVEL,"chip = %d, memBaseAddr0 = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].memBaseAddr0);
    ExpMgrConfigReadLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kMemBaseAddr1, (FxU32 *)&avengerHALData->configRegs[chipIndex].memBaseAddr1);
    LOG_PRINTF2(FN_LEVEL,"chip = %d, memBaseAddr1 = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].memBaseAddr1);
    ExpMgrConfigReadLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kIOBaseAddr0, (FxU32 *)&avengerHALData->configRegs[chipIndex].ioBaseAddr);
    LOG_PRINTF2(FN_LEVEL,"chip = %d, ioBaseAddr = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].ioBaseAddr);
#if H5  
    ExpMgrConfigReadLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kCfgInitEnable, (FxU32 *)&avengerHALData->configRegs[chipIndex].cfgInitEnable_FabID);
    LOG_PRINTF2(FN_LEVEL,"chip = %d, cfgInitEnable_FabID = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].cfgInitEnable_FabID);
    ExpMgrConfigReadLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kCfgPciDecode, (FxU32 *)&avengerHALData->configRegs[chipIndex].cfgPciDecode);
    LOG_PRINTF2(FN_LEVEL,"chip = %d, cfgPciDecode = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].cfgPciDecode);
#endif
  }

  
  /* Make a note that the cursor is invalid until we get a new one. */
  avengerHALData->cursorSet = FXFALSE;
  
  // kludge.
#if DEBUG
  gdx_scc_shutdown();
#endif  
#undef FN_NAME
#undef FN_LEVEL
}

static void AvengerDeviceWakeup(AvengerHALData *avengerHALData)
{
#define FN_NAME "AvengerDeviceWakeup"
#define FN_LEVEL 0
  FxU32 chipIndex, pciInit0, lfbMemoryConfig, i;
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 ioPortAddress = avengerHALData->bInfo.regInfo.ioPortBase;
  const FxU32
    sgramMode     = 0x37,
    sgramMask     = 0xFFFFFFFF,
    sgramColor    = 0x00000000;

  LOG_ENTRY(5);

  // Restore PCI config space registers on all chips.  The docs say that this should be done for us, but it some
  // cases it didn't seem to work right.  Since we know what they are supposed to be we restore them anyway. 
  for(chipIndex = 0; chipIndex < avengerHALData->numChips; chipIndex++) {
    LOG_PRINTF2(FN_LEVEL,"chip = %d, memBaseAddr0 = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].memBaseAddr0);
    ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kMemBaseAddr0, avengerHALData->configRegs[chipIndex].memBaseAddr0);
    LOG_PRINTF2(FN_LEVEL,"chip = %d, memBaseAddr1 = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].memBaseAddr1);
    ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kMemBaseAddr1, avengerHALData->configRegs[chipIndex].memBaseAddr1);
    LOG_PRINTF2(FN_LEVEL,"chip = %d, ioBaseAddr = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].ioBaseAddr);
    ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kIOBaseAddr0, avengerHALData->configRegs[chipIndex].ioBaseAddr);
#if H5    
    LOG_PRINTF2(FN_LEVEL,"chip = %d, cfgInitEnable_FabID = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].cfgInitEnable_FabID);
    ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kCfgInitEnable, avengerHALData->configRegs[chipIndex].cfgInitEnable_FabID);
    LOG_PRINTF2(FN_LEVEL,"chip = %d, cfgPciDecode = 0x%08x\n", chipIndex, avengerHALData->configRegs[chipIndex].cfgPciDecode);
    ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipIndex], (LogicalAddress)kCfgPciDecode, avengerHALData->configRegs[chipIndex].cfgPciDecode);
#endif    
  }

  LOG_PRINTF(5,"Enabling memory and IO\n");
  /* Enable memory and IO accesses */
  AvengerEnableMemory(0);
  AvengerEnableIO(0);

  LOG_PRINTF(5,"Configuring pciInit0\n");
  /* Configure some PCI stuff */
  AvengerInitPCI( avengerHALData->bInfo.regInfo );

  LOG_PRINTF(5,"Resetting HW\n");
  /* Reset hardware, program PLL */
  h3InitResetAll(ioPortAddress);
  LOG_PRINTF(5,"Programming PLL\n");
#if defined(H4) || defined(H5)
  h4InitPlls(ioPortAddress,  SST_DEVICE_ID_H4, avengerHALData->grxClock);
#else
  h3InitPlls(ioPortAddress, 100, 100);
#endif

  LOG_PRINTF(5,"Configuring memory\n");

  /* read back the memory size, since we don't know it under DOS  (see hwcInit) - dwj */
  h3InitSgram(ioPortAddress, sgramMode, sgramMask, sgramColor, avengerHALData->bInfo.pciInfo.deviceID, NULL);

  LOG_PRINTF(5,"Enabling VGA\n");
  /* Do initial VGA setup - make sure legacy decode is disabled. */
  h3InitVga(ioPortAddress, FXFALSE);

#if H5
  {
    FxU32 miscInit1;
    HWC_IO_LOAD(bInfo->regInfo,miscInit1,miscInit1);
    miscInit1 &= ~( SST_BYTE_SWIZZLE_SELECT | SST_BYTE_SWIZZLE_ENABLE);
    miscInit1 |=  SST_BYTE_SWIZZLE_64MB | SST_BYTE_SWIZZLE_ENABLE;
    HWC_IO_STORE(bInfo->regInfo,miscInit1,miscInit1);
  }
#endif	

  LOG_PRINTF(5,"Configuring lfbMemoryConfig\n");

  /* Start Tiled mode at 4MB boundary instead of default. -- FIXME, doesn't work for modes >1024x768 very well */
  lfbMemoryConfig = SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE((avengerHALData->tileMark >> 12)) | SST_RAW_LFB_ADDR_STRIDE_1K | 0xa0000;
  HWC_IO_STORE(bInfo->regInfo, lfbMemoryConfig, lfbMemoryConfig);

  LOG_PRINTF(5,"Enabling interrupts\n");
  /*
  ** Make sure VBL interrupts are enabled.  Well, sortof.
  ** I don't actually enable interrupts until we have been programmed with a
  ** valid display mode...  this may not be necessary though since VSYNC interrupts
  ** probably won't happen until then anyway.
  */
#if ENABLE_INTERRUPTS
  avengerHALData->intrCrtl = SST_INTR_PCI_INTA;
  HWC_SST_STORE(bInfo->regInfo,intrCtrl,avengerHALData->intrCrtl);
  HWC_SST_STORE(bInfo->regInfo,status,0);
  HWC_IO_LOAD(avengerHALData->bInfo.regInfo, pciInit0, pciInit0);
  pciInit0 |= SST_PCI_INTERRUPT_ENABLE;
  HWC_IO_STORE(avengerHALData->bInfo.regInfo, pciInit0, pciInit0);
  HWC_IO_STORE(avengerHALData->bInfo.regInfo, status, 0);
#endif

#if V5_5000
  if(avengerHALData->numChips > 1) {
    FxU32 i, ioPortBaseSlave;
        
    for(i = 1; i < avengerHALData->numChips; i++) {
      NapalmResetSlave(i);
    }
  }
#endif
  
  // Restore 2D registers on master
  HWC_WAX_STORE(bInfo->regInfo, clip0min, avengerHALData->waxRegs.clip0min);
  HWC_WAX_STORE(bInfo->regInfo, clip0max, avengerHALData->waxRegs.clip0max);
  HWC_WAX_STORE(bInfo->regInfo, dstBaseAddr, avengerHALData->waxRegs.dstBaseAddr);
  HWC_WAX_STORE(bInfo->regInfo, dstFormat, avengerHALData->waxRegs.dstFormat);
  HWC_WAX_STORE(bInfo->regInfo, srcColorkeyMin, avengerHALData->waxRegs.srcColorkeyMin);
  HWC_WAX_STORE(bInfo->regInfo, srcColorkeyMax, avengerHALData->waxRegs.srcColorkeyMax);
  HWC_WAX_STORE(bInfo->regInfo, dstColorkeyMin, avengerHALData->waxRegs.dstColorkeyMin);
  HWC_WAX_STORE(bInfo->regInfo, dstColorkeyMax, avengerHALData->waxRegs.dstColorkeyMax);
  HWC_WAX_STORE(bInfo->regInfo, bresError0, avengerHALData->waxRegs.bresError0);
  HWC_WAX_STORE(bInfo->regInfo, bresError1, avengerHALData->waxRegs.bresError1);
  HWC_WAX_STORE(bInfo->regInfo, rop, avengerHALData->waxRegs.rop);
  HWC_WAX_STORE(bInfo->regInfo, srcBaseAddr, avengerHALData->waxRegs.srcBaseAddr);
  HWC_WAX_STORE(bInfo->regInfo, commandEx, avengerHALData->waxRegs.commandEx);
  HWC_WAX_STORE(bInfo->regInfo, lineStipple, avengerHALData->waxRegs.lineStipple);
  HWC_WAX_STORE(bInfo->regInfo, lineStyle, avengerHALData->waxRegs.lineStyle);
  HWC_WAX_STORE(bInfo->regInfo, clip1min, avengerHALData->waxRegs.clip1min);
  HWC_WAX_STORE(bInfo->regInfo, clip1max, avengerHALData->waxRegs.clip1max);
  HWC_WAX_STORE(bInfo->regInfo, srcFormat, avengerHALData->waxRegs.srcFormat);
  HWC_WAX_STORE(bInfo->regInfo, srcSize, avengerHALData->waxRegs.srcSize);
  HWC_WAX_STORE(bInfo->regInfo, srcXY, avengerHALData->waxRegs.srcXY);
  HWC_WAX_STORE(bInfo->regInfo, colorBack, avengerHALData->waxRegs.colorBack);
  HWC_WAX_STORE(bInfo->regInfo, colorFore, avengerHALData->waxRegs.colorFore);
  HWC_WAX_STORE(bInfo->regInfo, dstSize, avengerHALData->waxRegs.dstSize);
  HWC_WAX_STORE(bInfo->regInfo, dstXY, avengerHALData->waxRegs.dstXY);
  for(i = 0; i < 64; i++) {
    HWC_WAX_STORE(bInfo->regInfo, colorPattern[i], avengerHALData->waxRegs.colorPattern[i]);
  }

  // Restore Command FIFO registers on master
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.baseAddrL, avengerHALData->cmdRegs.cmdFifo0.baseAddrL);
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.baseSize, avengerHALData->cmdRegs.cmdFifo0.baseSize);
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.readPtrL, avengerHALData->cmdRegs.cmdFifo0.readPtrL);
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.readPtrH, avengerHALData->cmdRegs.cmdFifo0.readPtrH);
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.aMin, avengerHALData->cmdRegs.cmdFifo0.aMin);
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.aMax, avengerHALData->cmdRegs.cmdFifo0.aMax);
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.depth, avengerHALData->cmdRegs.cmdFifo0.depth);
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.holeCount, avengerHALData->cmdRegs.cmdFifo0.holeCount);
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifoThresh, avengerHALData->cmdRegs.cmdFifoThresh);
#undef FN_NAME
#undef FN_LEVEL
}

static void AvengerEnableMemory( FxU32 chipNumber )
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  FxU16 commandRegister;
  OSErr osErr;

  osErr = ExpMgrConfigReadWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, &commandRegister);
  commandRegister |= 2;     /* Enable memory space */
  osErr = ExpMgrConfigWriteWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, commandRegister);
}

static void AvengerDisableMemory( FxU32 chipNumber )
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  OSErr osErr;
  FxU16 commandRegister;

  osErr = ExpMgrConfigReadWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, &commandRegister);
  commandRegister &= ~2;      /* Disable memory space */
  osErr = ExpMgrConfigWriteWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, commandRegister);
}





static void AvengerEnableIO( FxU32 chipNumber )
{
#define FN_NAME "AvengerEnableIO"
#define FN_LEVEL 0
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  OSErr osErr;
  FxU16 commandRegister;

  osErr = ExpMgrConfigReadWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, &commandRegister);
  commandRegister |= 1;     /* Enable IO space */
  osErr = ExpMgrConfigWriteWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, commandRegister);

  osErr = ExpMgrConfigReadWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, &commandRegister);
  LOG_PRINTF1(FN_LEVEL,"commandRegister = 0x%08x\n", commandRegister);
#undef FN_NAME  
#undef FN_LEVEL  
}





static void AvengerDisableIO( FxU32 chipNumber )
{
#define FN_NAME "AvengerEnableIO"
#define FN_LEVEL 0
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  OSErr osErr;
  FxU16 commandRegister;

  osErr = ExpMgrConfigReadWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, &commandRegister);
  commandRegister &= ~1;      /* Disable IO space */
  osErr = ExpMgrConfigWriteWord(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) 0x4, commandRegister);
#undef FN_NAME  
#undef FN_LEVEL  
}





static FxU32 AvengerIsPCI(void)
{
#define kAGPMasterPropertyStr "AGP_Master"

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  FxU32 theValueSize;
  FxU32 isPCI;
  
  isPCI = RegistryPropertyGetSize( &avengerHALData->regEntryID[0], kAGPMasterPropertyStr, &theValueSize) != noErr;

  return isPCI;
}





/*
**=====================================================================================================
**
** AvengerInitDesktopScaler()
**
**
**=====================================================================================================
*/
static void AvengerInitDesktopScaler(FxU16 width,FxU16 height, DepthMode depth )
{
#define FN_NAME "AvengerInitDesktopScaler"
#define FN_LEVEL 0

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
//  FxU32 regBase = avengerHALData->bInfo.regInfo.ioPortBase;
  FxU32 theVidProcCfg;
  FxU32 theDstWidth;
  FxU32 theDstHeight;
  FxU32 theTiled = 0; // linear
  FxU32 theStride;
  
  LOG_ENTRY(FN_LEVEL);
  avengerHALData->scalerNeeded = false;
  
  HWC_IO_LOAD( avengerHALData->bInfo.regInfo, vidProcCfg, theVidProcCfg );
  HWC_IO_LOAD( avengerHALData->bInfo.regInfo, vidScreenSize, theDstWidth );
  theDstHeight = theDstWidth;
  theDstWidth = ( theDstWidth & SST_VIDEO_SCREEN_WIDTH) >> SST_VIDEO_SCREEN_WIDTH_SHIFT;
  theDstHeight = ( theDstHeight & SST_VIDEO_SCREEN_HEIGHT) >> SST_VIDEO_SCREEN_HEIGHT_SHIFT;
  HWC_IO_LOAD( avengerHALData->bInfo.regInfo, vidDesktopOverlayStride, theStride );
  theStride = ( theStride & SST_DESKTOP_LINEAR_STRIDE) >> SST_DESKTOP_STRIDE_SHIFT;

  LOG_PRINTF2(FN_LEVEL,"width = %d, height = %d\n", width, height);
  LOG_PRINTF2(FN_LEVEL,"dstWidth = %d, dstHeight = %d\n", theDstWidth, theDstHeight);

  if ( avengerHALData->scaler2x ) {    
//    theVidProcCfg |= SST_VIDEO_2X_MODE_EN;
//    HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidProcCfg, theVidProcCfg);
    return;    
  }
  /*
      first let's check if we need any horizontal scaling and if so, calculate the appropriate params
  */
  if (width < theDstWidth)
  {
    FxU32 theDudx;
    FxU32 theDudxOffsetSrcWidth;
    float fScale;
        
    theVidProcCfg |= SST_OVERLAY_HORIZ_SCALE_EN;
    fScale = (double)width / (double)theDstWidth;                               /* scalingFactor = fScale; */
    LOG_PRINTF1(FN_LEVEL,"horizontal scaling factor = %f\n", fScale);
    fScale *= 1 << 20;
    LOG_PRINTF1(FN_LEVEL,"horizontal scaling factor = %f\n", fScale);
    theDudx = (UInt32)(fScale);
    if ( theTiled )
      theDudxOffsetSrcWidth = ((theStride << 7) << SST_OVERLAY_FETCH_SIZE_SHIFT)|(DWORD) 0;
    else
      theDudxOffsetSrcWidth = (theStride << SST_OVERLAY_FETCH_SIZE_SHIFT)|(DWORD) 0; /* Dudx offset = 0 */

    LOG_PRINTF2(FN_LEVEL,"theDudx = %d (0x%08x)\n", theDudx, theDudx);
    HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidOverlayDudx, theDudx);
    HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidOverlayDudxOffsetSrcWidth, theDudxOffsetSrcWidth);
    avengerHALData->scalerNeeded = true;
   }
   else
   {
    theVidProcCfg &= ~SST_OVERLAY_HORIZ_SCALE_EN;
   }
    

  /*
      first let's check if we need any horizontal scaling and if so, calculate the appropriate params
  */
  if (height < theDstHeight)
  {
    FxU32 theDvdy;
    float fScale;
       
    theVidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;	
    fScale = (double)height / (double)theDstHeight;  /* scalingFactor = fScale; */
    LOG_PRINTF1(FN_LEVEL,"vertical scaling factor = %f\n", fScale);
    fScale *= 1 << 20;
    LOG_PRINTF1(FN_LEVEL,"vertical scaling factor = %f\n", fScale);
    theDvdy = (UInt32)(fScale);
    LOG_PRINTF2(FN_LEVEL,"theDvdy = %d (0x%08x)\n", theDvdy, theDvdy);
    HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidOverlayDvdy, theDvdy);
    HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidOverlayDvdyOffset, 0);
    avengerHALData->scalerNeeded = true;
  }
   else
   {
    theVidProcCfg &= ~SST_OVERLAY_VERT_SCALE_EN;
   }



  if ( avengerHALData->scalerNeeded )
  {
    theVidProcCfg &= ~(SST_DESKTOP_TILED_EN | SST_OVERLAY_TILED_EN | SST_CHROMA_EN | SST_USE_ALPHA_BIT | SST_CHROMA_INVERT | SST_HALF_MODE | SST_INTERLACED_EN);
  
    /*
        setup the overlay stride
    */
    {
      FxU32 theVidDesktopOverlayStride;
        
      HWC_IO_LOAD( avengerHALData->bInfo.regInfo, vidDesktopOverlayStride, theVidDesktopOverlayStride);
      theVidDesktopOverlayStride &= ~SST_OVERLAY_LINEAR_STRIDE;
      theVidDesktopOverlayStride |= (theStride << SST_OVERLAY_STRIDE_SHIFT) & SST_OVERLAY_LINEAR_STRIDE;

      HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidDesktopOverlayStride, theVidDesktopOverlayStride);
    }

    /*
        setup the overlay screen coordinates (full screen)
    */
    {
      FxU32 thePosition = 0;
        
      HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidOverlayStartCoords, thePosition);
      thePosition = ((theDstWidth - 1) << SST_OVERLAY_X_SHIFT) & SST_OVERLAY_X;
      thePosition |= ((theDstHeight - 1) << SST_OVERLAY_Y_SHIFT) & SST_OVERLAY_Y;        
      HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidOverlayEndScreenCoord, thePosition);
    }

    theVidProcCfg &= ~SST_OVERLAY_PIXEL_FORMAT;
    switch( depth )
    {
      case kDepthMode2:
        theVidProcCfg |= SST_OVERLAY_PIXEL_RGB1555U;
        break;
        
      case kDepthMode3:
        theVidProcCfg |= SST_OVERLAY_PIXEL_RGB32U;
        break;      
    }
    
    theVidProcCfg |= SST_OVERLAY_EN | SST_OVERLAY_FILTER_BILINEAR | SST_OVERLAY_CLUT_BYPASS;


    /* 
       set the new params to the video processor config register,
       setup the framebuffer base address variable and jumpstart the overlay circuitry
    */
  
    HWC_IO_STORE( avengerHALData->bInfo.regInfo, vidProcCfg, theVidProcCfg);

    HWC_SST_STORE( avengerHALData->bInfo.regInfo, leftOverlayBuf, avengerHALData->frameBufferOffset);
    HWC_SST_STORE( avengerHALData->bInfo.regInfo, rightOverlayBuf, avengerHALData->frameBufferOffset);
    HWC_SST_STORE( avengerHALData->bInfo.regInfo, swapbufferCMD, 0x00);
    HWC_SST_STORE( avengerHALData->bInfo.regInfo, swapbufferCMD, 0x00);

  }
  
  
#undef FN_NAME  
#undef FN_LEVEL  
}







/*
**=====================================================================================================
**
** AvengerInitPCI()
**
**
**=====================================================================================================
*/
static void AvengerInitPCI( hwcRegInfo regInfo )
{
#define FN_NAME "AvengerInitPCI"
#define FN_LEVEL 0
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  FxU32 pciInit0;
  
  /* Configure some PCI stuff */
  HWC_IO_LOAD(regInfo, pciInit0, pciInit0);
  LOG_PRINTF1(FN_LEVEL,"pciinit0 0x%08x (original)\n", pciInit0);
  if(avengerHALData->isPCI) {
    LOG_PRINTF(FN_LEVEL,"pciinit0 -> PCI operation\n");
    if(avengerHALData->numChips > 1) {
      pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (10 << SST_PCI_LOWTHRESH_SHIFT) | SST_PCI_FORCE_FB_HIGH;
    } else {
      pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (8 << SST_PCI_LOWTHRESH_SHIFT);
    }
    pciInit0 &= ~(SST_PCI_DISABLE_IO|SST_PCI_DISABLE_MEM|SST_PCI_RETRY_INTERVAL);
    pciInit0 |= SST_PCI_READ_WS | SST_PCI_WRITE_WS | SST_PCI_DISABLE_MEM | SST_PCI_DISABLE_IO;
  } else {
    LOG_PRINTF(FN_LEVEL,"pciinit0 -> AGP operation\n");
    if(avengerHALData->numChips > 1) {
      pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (10 << SST_PCI_LOWTHRESH_SHIFT);
    } else {
      pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (8 << SST_PCI_LOWTHRESH_SHIFT);
    }
    pciInit0 |= SST_PCI_DISABLE_IO|SST_PCI_DISABLE_MEM;
    pciInit0 &= ~SST_PCI_FORCE_FB_HIGH;
  }
  HWC_IO_STORE(regInfo, pciInit0, pciInit0);
  LOG_PRINTF1(FN_LEVEL,"pciinit0 = 0x%08x\n", pciInit0);
  
#undef FN_NAME  
#undef FN_LEVEL  
}


#if V5_5000
static unsigned long _strlen(const char *c);
static void _strcpy(char *dst, const char *src);
static void _strcat(char *dst, const char *src);

typedef struct PCIRegPropertyEntry
{
  unsigned long phys_hi;
  unsigned long phys_mid;
  unsigned long phys_lo;
  unsigned long size_hi;
  unsigned long size_lo;
} PCIRegPropertyEntry;

#define kPhysHISpaceMask 0x03000000
#define kPhysHISpaceConfig 0x00000000
#define kPhysHIBusMask   0x00ff0000
#define kPhysHIBusShift  16
#define kPhysHIDeviceMask 0x0000f800
#define kPhysHIDeviceShift 11
#define kPhysHIFunctionMask 0x00000700
#define kPhysHIFunctionShift 8
#define kPhysHIRegisterMask 0x000000ff
#define kPhysHIRegisterShift 0


static FxU32 NapalmMapSlave(FxU32 chipNumber)
{
#define FN_NAME "NapalmMapSlave"
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  RegEntryID masterEntryID;
  RegEntryID slaveEntryID;
  RegEntryID pciBusEntryID;
  PCIRegPropertyEntry regProperty[5];
  PCIRegPropertyEntry slaveRegProperty;
	
  RegPropertyNameBuf propName;
  RegPropertyValueSize propSize;
  static char slaveName[256];
  static char parentName[256];
  static char slotName[32];
  
  FxU32 slaveNameLen;
    
  static const char nameProperty[] = "name";

  FxU32 entryVendorID, entryDeviceID, i, regConfigNum;
  FxBool haveVendorID, haveDeviceID, foundBoard, success;	

  OSErr err = noErr;
  Boolean done = true, propDone;
  foundBoard = success = FXFALSE;
	
  LOG_ENTRY(5);
	
  masterEntryID = avengerHALData->regEntryID[0];
  RegistryEntryIDInit(&avengerHALData->regEntryID[chipNumber]);
    
  propSize = sizeof(slaveName)-2;
  err = RegistryPropertyGet(&masterEntryID,"name", &slaveName[1], &propSize);
  if(err == noErr) {
    _strcat(&slaveName[1],",Slave");
    slaveNameLen = _strlen(&slaveName[1]);
    slaveName[0] = ':';
    slaveName[slaveNameLen+1] = '0' + chipNumber;
    slaveName[slaveNameLen+2] = 0;
    
    err = RegistryPropertyGetSize(&masterEntryID,"reg", &propSize);
    if(err == noErr) {
      err = RegistryPropertyGet(&masterEntryID,"reg", regProperty, &propSize);
      if(err == 0) {
        /* Find the right entry in the reg property */
        for(i = 0; i < (propSize / sizeof(PCIRegPropertyEntry)); i++) {
          if((regProperty[i].phys_hi & kPhysHISpaceMask) == kPhysHISpaceConfig) {
            regConfigNum = i;
          }
        }
    
        /* Find parent RegEntry, which should be a PCI bus */
        err = RegistryCStrEntryToName(&masterEntryID,&pciBusEntryID,parentName,&done);
        if(err == noErr) {
    
          /* Look for existing slave registry entry, nuke it if it's there. */
          err = RegistryCStrEntryLookup(&pciBusEntryID,&slaveName[0],&slaveEntryID);
          if(err == noErr ) {
            err = RegistryEntryDelete(&slaveEntryID);
	      }
	    }

	    /* Create new slave chip registry entry */
	    err = RegistryCStrEntryCreate(&pciBusEntryID,&slaveName[1],&slaveEntryID);
	    if(err == noErr) {
	      

          err = RegistryPropertyGetSize(&masterEntryID,"AAPL,slot-name", &propSize);
          if(err == noErr) {
	          LOG_PRINTF1(5,"found AAPL,slot-name: %d\n",propSize);
            err = RegistryPropertyGet(&masterEntryID,"AAPL,slot-name", slotName, &propSize);
            if(err == 0) {
	          LOG_PRINTF1(5,"AAPL,slot-name: %s\n",slotName);
	          err = RegistryPropertyCreate(&slaveEntryID, "AAPL,slot-name", &slotName, propSize);
	        }
          }


	      /* Create new "reg" property for slave */
	      slaveRegProperty = regProperty[regConfigNum];
	      slaveRegProperty.phys_hi &= ~kPhysHIFunctionMask;
	      slaveRegProperty.phys_hi |= 1 << kPhysHIFunctionShift;
	      propSize = sizeof(slaveRegProperty);
	      err = RegistryPropertyCreate(&slaveEntryID, "reg", &slaveRegProperty, propSize);
	      if(err == noErr) {
	        FxU32 cfgPciDecode, cfgInitEnable;
	        FxU32 theVendorDevice;
	        FxU32 theSlaveIDProperty;
	        
	        RegistryEntryIDCopy(&slaveEntryID,&avengerHALData->regEntryID[chipNumber]);
	        
	        /* verify that we have indeed a valid regEntryID */
	        theVendorDevice = 0;
	        err = ExpMgrConfigReadLong(&avengerHALData->regEntryID[chipNumber], 0, &theVendorDevice);
	        LOG_PRINTF1(0,"Vendor / Device: 0x%08x\n",theVendorDevice);
	        if(err == noErr) {
	          LOG_PRINTF(0,"slave mapping is valid!\n");
	          success = FXTRUE;
	        }
	        else
	        {
	          long theNextSlotInfoOffset = 0x14;
              
              #define theMasterEntry avengerHALData->regEntryID[0]
              #define theSlaveEntry avengerHALData->regEntryID[chipNumber]
          
              long *theSlaveSlotInfo = (long *)gSlotInfo[chipNumber];
              long *theMasterSlotInfo;
	          long theCount = 16;
          
	          LOG_PRINTF(0,"slave mapping *NOT* is valid!\n");
	          LOG_PRINTF(0,"attempting repair!!!\n");

              theMasterSlotInfo = (long*) 0x2b6;
              theMasterSlotInfo = (long*) *theMasterSlotInfo;
              LOG_PRINTF1(0,"*0x2b6 = 0x%08x\n", theMasterSlotInfo );
              theMasterSlotInfo = (long*) theMasterSlotInfo[ 0x8d ];
              LOG_PRINTF1(0,"*( *0x2b6 + 0x234 ) = 0x%08x\n", theMasterSlotInfo );
              theMasterSlotInfo = (long*) theMasterSlotInfo[ 1 ];
              LOG_PRINTF1(0,"theMasterSlotInfo = 0x%08x\n", theMasterSlotInfo );

              if ( ( theMasterSlotInfo[ theNextSlotInfoOffset ] >> 24 ) == -1 )
                theNextSlotInfoOffset++;  // beige G3 (OpenFirmware v2)
              LOG_PRINTF1(0,"theNextSlotInfoOffset = 0x%x\n", theNextSlotInfoOffset );
                 
              while ( theMasterSlotInfo && ( theMasterSlotInfo[0] != theMasterEntry.contents[1] ) && ( theCount-- > 0 	) )
              {
                theMasterSlotInfo = (long*) theMasterSlotInfo[ theNextSlotInfoOffset ];
                LOG_PRINTF1(0,"next MasterSlotInfo = 0x%08x\n", theMasterSlotInfo );
                LOG_PRINTF1(0,"theCount = %d\n", theCount );
              }

              if ( theMasterSlotInfo && ( theCount > 0 ) )
              {
                BlockMove( theMasterSlotInfo, theSlaveSlotInfo, SlotInfoSize );
                theSlaveSlotInfo[ theNextSlotInfoOffset ] = theMasterSlotInfo[ theNextSlotInfoOffset ];
                theMasterSlotInfo[ theNextSlotInfoOffset ] = (long) theSlaveSlotInfo;
            
                theSlaveSlotInfo[ 0 ] = theSlaveEntry.contents[1];
                theSlaveSlotInfo[ 4 ] = 1;
                theSlaveSlotInfo[ 5 ] |= 0x100;
              }

#if DEBUG
	          LOG_PRINTF(0,"theMasterSlotInfo:\n");
              {
                long i;                
                for ( i = 0 ; i < SlotInfoSize / 4 ; i += 2 )
                {
                  LOG_PRINTF2(0,"0x%08x 0x%08x \n", theMasterSlotInfo[i], theMasterSlotInfo[i+1]);
                }
              } 
	          LOG_PRINTF(0,"theSlaveSlotInfo:\n");
              {
                long i;                
                for ( i = 0 ; i < SlotInfoSize / 4 ; i += 2 )
                {
                  LOG_PRINTF2(0,"0x%08x 0x%08x \n", theSlaveSlotInfo[i], theSlaveSlotInfo[i+1]);
                }
              } 
#endif
//	           ExpMgrConfigReadLong( &theSlaveEntry, 0, &theVendorDevice );
//               cls_printf( "theSlaveEntry: theVendorDevice = 0x%08x\n", theVendorDevice );

	          theVendorDevice = 0;
	          err = ExpMgrConfigReadLong(&avengerHALData->regEntryID[chipNumber], 0, &theVendorDevice);
	          LOG_PRINTF1(0,"Vendor / Device: 0x%08x\n",theVendorDevice);
	          if(err == noErr) {
	            LOG_PRINTF(0,"slave mapping is valid!\n");
	            success = FXTRUE;
	          }
	          else
	          {
	            LOG_PRINTF(0,"slave mapping *STILL NOT* is valid!\n");
	            goto ErrorExit;
              }	          
	        }

	        /* Now do the messy work of mapping in the slave chip's registers. */
	        
	        /* Make sure we can play with init bits */
	        cfgInitEnable = (SST_ENABLE_HARDWARE_INIT_WRITES |
	                         SST_ENABLE_PCI_FIFO_WRITES |
	                         SST_ENABLE_BASE_ADDR_WRITES
	                        ) << 8;
	                 
	        cfgInitEnable |= ((avengerHALData->bInfo.pciInfo.pciBaseAddr[0] >> (32 - 10)) & 0x3FF) << (SST_MEMBASE0_SNOOP_SHIFT + 8);
            /* Set config register */	        
	        ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) kCfgInitEnable, cfgInitEnable);

	        /* First, set up PCI config decode stuff */
	        cfgPciDecode = SST_PCI_MEMBASE0_DECODE_32MB |
	                       SST_PCI_MEMBASE1_DECODE_64MB |
	                       SST_PCI_IOBASE0_DECODE_256 |
	                       SST_SNOOP_MEMBASE0_DECODE_32MB |
	                       SST_SNOOP_MEMBASE1_DECODE_256MB;
	                       
	        /* Also set up memBase1 snoop address */
	        cfgPciDecode |= ((avengerHALData->bInfo.pciInfo.pciBaseAddr[1] >> (32 - 10)) & 0x3FF) << SST_MEMBASE1_SNOOP_SHIFT;
            
            /* Set config register */	        
	        ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) kCfgPciDecode, cfgPciDecode);
	        
	        /* Now that the hardware knows how much memory to decode, map in the 3 memory ranges we care about */
	        /* 256MB for for memBase0 & slaves is divided up into 8 32MB chunks like this:
	        ** | chip0memBase0 | chip1 memBase0 | chip2 memBase0 | chip3 memBase0 | chip [1,2,3] memBase1 (64MB) | (64MB unused) |
	        ** Note: All slave chips share the same overlapping memBase1 location because they MUST be programmed to decode the 
	        ** same amount of memory as the master decodes for memBase1, otherwise certain things (such as LFB accesses) break
	        ** in strange ways that only make sense if you understand what order the HW does things in.
	        */
	        ExpMgrConfigReadLong(&avengerHALData->regEntryID[0], (LogicalAddress) kIOBaseAddr0,
	          &avengerHALData->bInfo.pciInfo.pciBaseAddr[2]);
	        LOG_PRINTF1(0,"master slaveIOBase0:  %08lx\n",avengerHALData->bInfo.pciInfo.pciBaseAddr[2]);

	        avengerHALData->slaveMemBase0[chipNumber] = avengerHALData->bInfo.pciInfo.pciBaseAddr[0] + 32*1024*1024*chipNumber;
	        avengerHALData->slaveMemBase1[chipNumber] = avengerHALData->bInfo.pciInfo.pciBaseAddr[0] + 128*1024*1024;
	        avengerHALData->slaveIOBase0[chipNumber]  = avengerHALData->bInfo.pciInfo.pciBaseAddr[2] & ~0xF;
	        
	        LOG_PRINTF1(5,"slaveMemBase0: %08lx\n",avengerHALData->slaveMemBase0[chipNumber]);
	        LOG_PRINTF1(5,"slaveMemBase1: %08lx\n",avengerHALData->slaveMemBase1[chipNumber]);
	        LOG_PRINTF1(5,"slaveIOBase0:  %08lx\n",avengerHALData->slaveIOBase0[chipNumber]);
	        
	        /* This is really a virtual address, but we calculate it here too */
	        avengerHALData->slaveIOPortBase[chipNumber] = avengerHALData->bInfo.regInfo.ioPortBase;
	        LOG_PRINTF1(5,"slaveIOPort: %08lx\n",avengerHALData->slaveIOPortBase[chipNumber]);

	        
	        /* Set config registers */
	        ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) kMemBaseAddr0, 
	          avengerHALData->slaveMemBase0[chipNumber]);
	        ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) kMemBaseAddr1, 
	          avengerHALData->slaveMemBase1[chipNumber]);
	        ExpMgrConfigWriteLong(&avengerHALData->regEntryID[chipNumber], (LogicalAddress) kIOBaseAddr0,
	          avengerHALData->slaveIOBase0[chipNumber]);


	        theSlaveIDProperty = chipNumber;
	        err = RegistryPropertyCreate(&slaveEntryID, "slave-id", &theSlaveIDProperty, 4);
	        if(err == noErr) {
	          LOG_PRINTF(0,"slave-id was added\n");
	        }
	        
	      }
	      else
	      {
	        RegistryEntryDelete(&slaveEntryID);
	      }
	      RegistryEntryIDDispose(&slaveEntryID);
        }
      }
    }
  }
  
ErrorExit:
  LOG_EXIT(5,(success == FXTRUE) ? 0:1);
  
  return success;  
#undef FN_NAME
}
#endif





static void NapalmResetSlave(FxU32 chipNumber)
{
#define FN_NAME "NapalmResetSlave"
#define FN_LEVEL 0
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  FxU32 ioPortBaseSlave, pciInit0;
    const FxU32
      sgramMode     = 0x37,
      sgramMask     = 0xFFFFFFFF,
      sgramColor     = 0x00000000;

  LOG_ENTRY(FN_LEVEL);
        
  ioPortBaseSlave = avengerHALData->slaveRegInfo[chipNumber].ioPortBase;
  
  AvengerEnableMemory(chipNumber);
  AvengerDisableIO(0);
  AvengerEnableIO(chipNumber);
  LOG_PRINTF(FN_LEVEL,"memory and IO enabled...\n");
  LOG_PRINTF1(FN_LEVEL,"ioMemBase = 0x%08x.\n", avengerHALData->slaveRegInfo[chipNumber].ioMemBase);

  AvengerInitPCI( avengerHALData->slaveRegInfo[chipNumber] );
  
  h3InitResetAll(ioPortBaseSlave);

  h4InitPlls(ioPortBaseSlave, SST_DEVICE_ID_H4, avengerHALData->grxClock);
  h3InitSgram(ioPortBaseSlave, sgramMode, sgramMask, sgramColor, avengerHALData->bInfo.pciInfo.deviceID, NULL);
  h3InitVga(ioPortBaseSlave, FXFALSE);
  HWC_SST_STORE(avengerHALData->slaveRegInfo[chipNumber],intrCtrl,SST_INTR_PCI_INTA);
  HWC_SST_STORE(avengerHALData->slaveRegInfo[chipNumber],status,0);

  /* Have to enable proper byte swizzle aperture stuff on the slave as well as on the master. */
#if H5
  {
    FxU32 miscInit1;
    HWC_IO_LOAD(avengerHALData->slaveRegInfo[chipNumber],miscInit1,miscInit1);
    miscInit1 &= ~( SST_BYTE_SWIZZLE_SELECT | SST_BYTE_SWIZZLE_ENABLE);
    miscInit1 |=  SST_BYTE_SWIZZLE_64MB | SST_BYTE_SWIZZLE_ENABLE;
    HWC_IO_STORE(avengerHALData->slaveRegInfo[chipNumber],miscInit1,miscInit1);
  }
#endif	

  LOG_PRINTF(FN_LEVEL,"reset completed!...\n");

  AvengerDisableIO(chipNumber);
  AvengerEnableIO(0);
#undef FN_NAME  
#undef FN_LEVEL  
}

static void NapalmResetSlaves()
{  
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  FxU32 slave;
  for(slave = 1; slave < avengerHALData->numChips; slave++) {
    NapalmResetSlave(slave);
  }
}

#define SST_1ST_CAP 0x34
#define SST_AGP_CAP 0x02

void PCI_Write_Config(FxU32 dwBus, FxU32 dwDevFunc, FxU32 dwOffset, FxU32 dwValue);
FxU32 PCI_Read_Config(FxU32 dwBus, FxU32 dwDevFunc, FxU32 dwOffset);

void NapalmSetVideoModeSlave(PDEVTABLE pSlave)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();  
  AvengerSetModeSlave(pSlave->dwDevFunc, avengerHALData->width, avengerHALData->height, avengerHALData->refresh);
}





void PCI_Write_Config(FxU32 dwBus, FxU32 dwDevFunc, FxU32 dwOffset, FxU32 dwValue)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();  
  ExpMgrConfigWriteLong(&avengerHALData->regEntryID[dwDevFunc], (LogicalAddress)dwOffset, dwValue);  
}





FxU32 PCI_Read_Config(FxU32 dwBus, FxU32 dwDevFunc, FxU32 dwOffset)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();  
  FxU32 result;
  ExpMgrConfigReadLong(&avengerHALData->regEntryID[dwDevFunc], (LogicalAddress)dwOffset, &result);  
  return result;
}






	        
static unsigned long _strlen(const char *c)
{
  const char *end = c;
  
  while(*end) {
    end++;
  }
  return ((unsigned long)end - (unsigned long)c);
}





static void _strcpy(char *dst, const char *src)
{
  while((*dst++ = *src++)) {};
}





static void _strcat(char *dst, const char *src)
{
  char *end = dst;
  while(*end) {
    end++;
  }
  _strcpy(end, src);
}





#if DISPLAY_MANAGER_POWER_BUG_WORKAROUND
// Note: This is not supposed to be used by display drivers.  It was here to work around some
// OS bugs that prevent multiple cards from working right with power management.
pascal OSStatus _XDoDriverPowerManagement(UInt32 message, void *param, UInt32 refCon, RegEntryID *regEntryID)
{
#define FN_NAME "AvengerPowerManagement"
	OSStatus		err;
	AvengerHALData *avengerHALData = (AvengerHALData *)refCon;
	
	// Always return kPowerMgtMessageNotHandled for messages we don't respond to.
	err = kPowerMgtMessageNotHandled;
	
	switch( message )
	{
		case kSleepRequest:
			// Complete pending I/O during kSleepRequest, but don't suspend 
			// further I/O requests until kSleepDemand.
			// CPU interrupts are still enabled!		
			err = noErr;
			GraphicsOSSSetHALPref(&avengerHALData->regEntryID[0], 0xbbbb0000 | avengerHALData->halPreferences);
			break;
			
		
		case kSleepDemand:
			// Now it is okay to suspend further I/O. Do complete any pending
			// I/O and then turn off device interrupts and save device-specific
			// registers and PCI config space (beyond the base 64 bytes which
			// Apple SSW saves), as well as any other volatile memory for the
			// device.
			// CPU interrupts are NOT enabled!
			//GraphicsOSSSetHALPref(&avengerHALData->regEntryID[0], 0xcccc0000 | avengerHALData->halPreferences);
			AvengerDeviceSleep((AvengerHALData *)refCon);
			//GraphicsOSSSetHALPref(&avengerHALData->regEntryID[0], 0xdddd0000 | avengerHALData->halPreferences);
			err = noErr;
			break;
			
		
		case kWakeToDoze:	
			// IMPORTANT!
			// The kWakeToDoze message is sent on a partial wake. Most devices won't do anything
			// different than when they receive a kSleepWakeUp message. However, they MUST at least
			// do that if they don't support partial wakeup. The message cannot simply be ignored.		
		case kSleepWakeUp:
			err = noErr;
			return err;
			break;
					
		case kDozeToFullWakeUp:
			// This message only needs to do anything interesting if the device supports
			// partial wakeup (and handled kWakeToDoze differently than a normal kSleepWakeUp).
			break;
				
		case kSleepRevoke:
			// Reverse any steps taken during kSleepRequest handling so that we may resume
			// full functionality.
			break;
				
		case kSetPowerLevel:
			// kGetPowerLevel and kSetPowerLevel are sent as a result of software (including
			// our very driver) calling SetDevicePowerLevel or GetDevicePowerLevel. It
			// is recommended that all power state changes come through here to allow the possibility
			// of improved power state monitoring by the Power Manager. For example, during handling
			// of the kSleepDemand message we might call:
			//
			//    err = SetDevicePowerLevel (&gOurDeviceRegEntry, kPMDevicePowerLevel_Off);
			//
			// and handle the actual details in our kSetPowerLevel case statement.
			//
			// Note that, currently, these are the only selectors that can be called at any time and 
			// not just during the sleep/wake process. Please see Power.h for the definition of the power 
			// levels (kPMDevicePowerLevel_On, kPMDevicePowerLevel_D1, etc.).
			switch (*(PowerLevel *)param)
			{
				case kPMDevicePowerLevel_Off:
				    avengerHALData->powerLevel = kPMDevicePowerLevel_Off;
					// turn things off
					break;
					
				case kPMDevicePowerLevel_On:
					// turn things on
					avengerHALData->powerLevel = kPMDevicePowerLevel_On;
					break;
			}
			err = noErr;
			break;
			
		case kGetPowerLevel:
			// determine current power level and return to caller, e.g.:
			*(PowerLevel *)param = avengerHALData->powerLevel;
			err = noErr;
			break;
			
		
		case kGetPowerInfo:
			// The Power Mgr will call this before sleeping to determine if we support
			// removing power from our PCI device during sleep.
			{
				DevicePowerInfo *		powerInfoPtr;
				
				powerInfoPtr = ( DevicePowerInfo * ) param;
				powerInfoPtr->flags |= kDevicePCIPowerOffAllowed;
				err = noErr;
			}
			break;
		
		
		case kDeviceInitiatedWake:
			// The kDeviceInitiatedWake message is sent by the Power Mgr to see how 
			// the machine was awakened. It may perform certain actions based on the result 
			// returned, perhaps to support partial wakeup.
			*(UInt32 *)param = kDeviceDidNotWakeMachine;		
			err = noErr;
			break;
		
		default:
			break;
	}
	return( err );

#undef FN_NAME
}
#endif

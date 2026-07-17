/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** $Header: HRM_Exports.c, 12, 10/11/00 8:35:04 PM, Brent$
** $Log: 
**  12   3dfx      1.4.1.2.1.3 10/11/00 Brent           Forced check in to enforce
**       branching.
**  11   3dfx      1.4.1.2.1.2 09/07/00 Stephane Huaulme latest critical path
**       source drop
**  10   3dfx      1.4.1.2.1.1 07/11/00 Critical Path   new source drop
**  9    3dfx      1.4.1.2.1.0 06/30/00 Critical Path   new source drop
**  8    3dfx      1.4.1.2     06/03/00 Stephane Huaulme added per board prefs
**       settings
**  7    3dfx      1.4.1.1     05/27/00 Critical Path   new source drop
**  6    3dfx      1.4.1.0     05/11/00 Stephane Huaulme reved version number
**  5    MacOS Dev Tree1.4         03/07/00 Critical Path   better mem management,
**       more qd coverage, bug fixes...
**  4    MacOS Dev Tree1.3         02/07/00 Kenneth Dyke    Added SLI/AA
**       extensions.
**  3    MacOS Dev Tree1.2         02/01/00 Kenneth Dyke    Return driver ref num
**       rather than GDHandle.  Fixed CR/LF effage.
**  2    MacOS Dev Tree1.1         01/31/00 Kenneth Dyke    Added PCI config
**       register extensions.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 8     8/25/99 5:43p Kcd
** Fix to make sure Glide doesn't break when monitors are rearranged.
** 
** 7     8/23/99 2:36p Kcd
** Surface, memory management, and AGP support.
** 
** 6     8/02/99 12:13p Kcd
** New SetPrefs/GetPrefs extensions.
** 
** 5     7/26/99 1:52p Kcd
** Added new extensions to help support Power Management in 2D driver. 
** 
** Also fixed possible FIFO trashing problem when returning from
** fullscreen exclusive modes.
** 
** 4     7/19/99 4:04p Kcd
** Don't abort board search if we find a board with no driver.
** 
** 3     7/08/99 1:24p Kcd
** Banshee & graphics clock support.
** 
** 2     7/02/99 3:24p Kcd
** New extension stuff.
**
*/

#include "hrm_priv.h"
#include "hdwr_res_mgr.h"
#include "hrm_fifo.h"
#include "hrm_mode.h"
#include "hrm_Loader.h"
#include "hrm_overlay_surface.h"
#include "DynamicPatches.h"
#include "hrm_prefs.h"
#include "hrm_arbitration.h"

#include "GraphicsPrivHwc.h"

#include "hdwr_res_mgr.h"
#include "DCon.h"
#include <string.h>
#include <time.h>
#include <files.h>
#include <CodeFragments.h>
#include <NameRegistry.h>
#include <DriverServices.h>
#include <Video.h>
#include <Folders.h>
#include <ctype.h>
#include <Gestalt.h>


// MBW -- XXX -- backwards compatibility
#define OLD_DRIVER_COMPAT 0

/*
________________________________________________________ Private Definitions ___
*/

#define kVendorID 0x121a
#define kDeviceID_Banshee 3
#define kDeviceID_Voodoo3 5
#define kDeviceID_Voodoo4 9

#define kVendorIDStr "vendor-id"
#define kDeviceIDStr "device-id"
 
pascal OSErr      hrmInitialize(
                          const CFragInitBlock *inInitBlock);

pascal void       hrmTerminate( void );

OSErr             hrmMakePersistent(
                          const CFragInitBlock *inInitBlock);

OSErr             hrmInitializeGlobals( void );

short             hrmRegisterPCIBoard(short inRefNum, RegEntryID entryID);

void              hrmDeRegisterAllTargets( void );

unsigned long     hrmiGetPropertyUInt32(
                          RegEntryID *       inEntry,
                          char *             inPropertyName );

OSErr			hrmPrivateControlCall(
						short				inRefNum,
						FxU32				which,
						hwcRequest_t		*inReq,
						hwcResponse_t		*outRes);
						
extern "C" void hrmAddExtensions( hrmExtensionNode_t *hrmExtensionNode );
extern "C" void hrmRemoveExtensions( hrmExtensionNode_t *hrmExtensionNode );

extern "C" short hrmGetDeviceConfig(hrmBoard_t *board, hrmDeviceConfig_t *config);

extern "C" void hrmGetVersionInfo(hrmVersionInfo_t * inVersion);

extern "C" void hrmSetModePrefs(hrmBoard_t *board);

extern "C" OSErr __initialize(const CFragInitBlock *);
extern "C" void  __terminate(void);

	// MBW -- XXX -- backwards compatibility
	// The code inside the following #ifdef block implements backwards compatibility with
	// the circa b11 voodoo3 drivers.  Once we have the updated drivers, it can safely be deleted.
#if OLD_DRIVER_COMPAT
extern "C" hwcBoardInfo *hrmGetTargetBoardInfo(hrmBoard_t *board);
#endif

/*
___________________________________________________________ global variables ___
*/

hrmBoard_t        gHdwrData[ kHrmMaxNumberOfTargets ];
static long       gCount = 0;
static long       gTargetCount = 0;
CFragConnectionID gSysCFragConnectionID; 
static List_t     gExtensionList;

/*
_________________________________________________________ primary extensions ___
*/

/* Note: All extensions that are statically linked to the HRM should be placed
         into this table to keep things simple. 
*/
static hrmExtensionTableEntry_t hrmPrimaryExtensions[] = {
	/* Extension management, etc. */
	{ "hrmGetVersionInfo", &hrmGetVersionInfo },
	{ "hrmAddExtensions", &hrmAddExtensions },
	{ "hrmRemoveExtensions", &hrmRemoveExtensions },
	{ "hrmGetDeviceConfig", &hrmGetDeviceConfig },
	  
	/* FIFO extensions */
	{ "hrmGetTargetFifoInfo", &hrmGetTargetFifoInfo },
	{ "hrmFifoWrap", &hrmFifoWrap },
	{ "hrmHwFifoPtr", &hrmHwFifoPtr },
	{ "hrmFifoUpdate", &hrmFifoUpdate },
	{ "hrmAllocWinContext", &hrmAllocWinContext },
	{ "hrmFreeWinContext", &hrmFreeWinContext },
	{ "hrmExecuteWinFifo", &hrmExecuteWinFifo },
	{ "hrmGetAGPInfo", &hrmGetAGPInfo },

    /* PCI Configuration */
    { "hrmWriteConfigRegister", &hrmWriteConfigRegister },
    { "hrmReadConfigRegister", &hrmReadConfigRegister },
    { "hrmGetSlaveRegs", &hrmGetSlaveRegs },
    
	/* Private 2D driver calls */
	{ "hrmDisableFifo2D", &hrmDisableFifo2D },
	{ "hrmEnableFifo2D", &hrmEnableFifo2D },

	/* Screen mode extensions */
	{ "hrmSetExclusiveMode", &hrmSetExclusiveMode },
	{ "hrmReleaseExclusiveMode", &hrmReleaseExclusiveMode },
	{ "hrmSetVideoMode", &hrmSetVideoMode },
	{ "hrmGetTargetBoardInfoExt", &hrmGetTargetBoardInfoExt },
    { "hrmSLIAA", &hrmSLIAA },
    
	/* Misc */
	{ "hrmGetPrefs", &hrmGetPrefs },
	{ "hrmSetPrefs", &hrmSetPrefs },
	{ "hrmSetModeFlags", &hrmSetModeFlags },
	{ "hrmSetModePrefs", &hrmSetModePrefs },
	  
	/* Memory management */
	{ "hrmCreateMemoryArea", &hrmCreateMemoryArea },
	{ "hrmDeleteMemoryArea", &hrmDeleteMemoryArea },
	{ "hrmInvalidateMemoryBlocks", &hrmInvalidateMemoryBlocks },
	{ "hrmAllocateBlock", &hrmAllocateBlock },
	{ "hrmFreeBlock", &hrmFreeBlock },   

	// Acceleration Loading
	{ "hrmLoadAccel", &hrmLoadAccel },
	{ "hrmUnloadAccel", &hrmUnloadAccel },

	// Dynamic Patches
	{ "hrmInitPatchManager", &hrm_InitPatchManager },
	{ "hrmRegisterPatches", &hrm_RegisterPatches },
	{ "hrmDeregisterPatches", &hrm_DeregisterPatches },

	// Overlay Plane
	{ "hrmSetVideoOverlay", &hrmSetVideoOverlay },

	// MBW -- XXX -- backwards compatibility
	// The code inside the following #ifdef block implements backwards compatibility with
	// the circa b11 voodoo3 drivers.  Once we have the updated drivers, it can safely be deleted.
#if OLD_DRIVER_COMPAT
	{ "hrmGetTargetBoardInfo", &hrmGetTargetBoardInfo },
#endif

};

static hrmExtensionNode_t hrmPrimaryExtensionNode = {
  { 0, 0 }, /* Node */
  sizeof(hrmPrimaryExtensions) / sizeof(hrmExtensionTableEntry_t),
  hrmPrimaryExtensions
};

   
/*
__________________________________________________________ hrmGetVersionInfo ___
*/

void
hrmGetVersionInfo(
  hrmVersionInfo_t *   outVersion)
{
  outVersion->major = 1;
  outVersion->minor = 6;
}

OSErr			
hrmPrivateControlCall(
						short				inRefNum,
						FxU32				which,
						hwcRequest_t		*inReq,
						hwcResponse_t		*outRes)
{
	OSErr			result;
	hwcControl_t	theData;
	ParamBlockRec	theParamBlock;
	
    theParamBlock.cntrlParam.ioCompletion = 0;
    theParamBlock.cntrlParam.ioVRefNum = 0;
    theParamBlock.cntrlParam.ioCRefNum = inRefNum;
    *((void **) &(theParamBlock.cntrlParam.csParam)) = &theData;
    theParamBlock.cntrlParam.csCode = k3DfxNewRequest;
	
	theData.which = which;
	theData.request = inReq;
	theData.response = outRes;

	result = (OSErr) PBControlSync(&theParamBlock);
	
	// MBW -- XXX -- backwards compatibility
	// The code inside the following #ifdef block implements backwards compatibility with
	// the circa b11 voodoo3 drivers.  Once we have the updated drivers, it can safely be deleted.
#if OLD_DRIVER_COMPAT
	/* Private status calls */
	enum
	{
		k3DfxOldGetDeviceConfig = 0x3DF0,
		k3DfxOldAllocWinContext,
		k3DfxOldReleaseWinContext,
		k3DfxOldSetModeFlags,
		k3DfxOldAllocVidMem,
		k3DfxOldFreeVidMem,
		k3DfxOldSetExclusive,
		k3DfxOldGetAGPInfo,
		k3DfxOldGetFifoInfo,
		k3DfxOldSetMode,
		k3DfxOldRegisterHRM,
		k3DfxOldSetPrefs
	};
	
	/* Given to k3DfxAllocWinContext */
	typedef struct hwcControlAllocContextReq_s 
	{
	  FxU32	
	    protocolRev,
	    appType;
	} hwcControlAllocContextReq_t;

	/* Given to k3DfxReleaseWinContext */
	typedef struct hwcControlReleaseContextReq_s
	{
		FxU32
			contextID;
	} hwcControlReleaseContextReq_t;

	/* Given to k3DfxExecuteFifo */
	typedef struct hwcControlExecuteFifoReq_s
	{
	  /* NB: These fields must be first to support old drivers that do not
	   * support multiple fifo types.  
	   */
	  FxU32
	    fifoPtr,        /* Linear address of command stream to execute */
	    fifoSize,       /* Size of command stream (DWORDS) */
	    statePtr,       /* Linear address of persistent state buffer */
	    stateSize;      /* Size of persistent state buffer (DWORDS) */
	  
	  FxU32
	    fifoType;       /* One of HWCEXT_FIFO_XXXX */

	  /* These are only necessary if fifoType != HWCEXT_FIFO_HOST */
	  FxU32
	    fifoOffset,     /* HW relative address of command stream */
	    stateOffset,    /* HW relative address of state buffer */
	    sentinalOffset, /* HW relative address of buffer to write serialNumber */
	    serialNumber;   /* Client specified value to write to indicate that the
	                       * current command stream has been committed by the hw.
	                       */
	} hwcControlExecuteFifoReq_t;

	/* Given to k3DfxAllocVidMem */
	typedef struct hwcControlAllocVidMemReq_s
	{
		FxU32 size;
	} hwcControlAllocVidMemReq_t;

	/* Given to k3DfxFreeVidMem */
	typedef struct hwcControlFreeVidMemReq_s
	{
		FxU32	surface;
		FxU32 size;
	} hwcControlFreeVidMemReq_t;

	/* Returned from k3DfxAllocWinContext */
	typedef struct hwcControlAllocContextRes_s
	{
	  FxU32	
	    contextID,
	    depthMode;
	} hwcControlAllocContextRes_t;

	/* Returned from k3DfxAllocVidMem */
	typedef struct hwcControlAllocVidMemRes_s
	{
		FxU32	surface;
		FxU32 size;
	} hwcControlAllocVidMemRes_t;

	typedef struct hwcControlGetFifoInfoRes_s
	{
		H3FifoInfo *fifoInfo;
	} hwcControlGetFifoInfoRes_t;

	typedef struct hwcContrelGetAGPInfoRes_s
	{
	  FxU32
	    agpLogicalAddress,
	    agpPhysicalAddress,
	    agpSize;
	} hwcControlGetAGPInfoRes_t;

	typedef struct
	{
		struct
		{
			FxU32 contextID;
			union reqOptData
			{
				hwcControlAllocContextReq_t       allocContextReq;
				hwcControlReleaseContextReq_t     releaseContextReq;
				hwcControlSetModeFlagsReq_t       setModeFlagsReq; 
				hwcControlExecuteFifoReq_t        executeFifoReq;
				hwcControlAllocVidMemReq_t        allocVidMemReq;
				hwcControlFreeVidMemReq_t         freeVidMemReq;
				hwcControlSetExclusiveReq_t       setExclusiveReq;
				hwcControlSetModeReq_t            setModeReq;
				hwcControlRegisterHRMDispatch_t   hrmDispatchReq;
				hwcControlSetPrefs_t              setPrefsReq;
			} optData;
		} req;
		struct
		{
			union resOptData
			{
				hwcControlDeviceConfigRes_t   deviceConfigRes;	// the new version of this struct is actually longer,
																// but it should still work properly.
				hwcControlAllocContextRes_t   allocContextRes;
				hwcControlAllocVidMemRes_t    allocVidMemRes;
				hwcControlGetFifoInfoRes_t    getFifoInfoRes;
				hwcControlGetAGPInfoRes_t     getAGPInfoRes;
			} optData;
		} res;
	} hwcOldControl_t;
	
	
	if(result != noErr)
	{
		// We are talking to an old (voodoo3 circa b11) driver.
		short which2 = 0;
		hwcOldControl_t  theOldData;
		
		switch(which)
		{
			case k3DfxGetDeviceConfig:
				which2 = k3DfxOldGetDeviceConfig;
				// No request data
			break;
			
			case k3DfxRegisterHRM:
				which2 = k3DfxOldRegisterHRM;
				theOldData.req.optData.hrmDispatchReq = inReq->optData.hrmDispatchReq;
			break;
			
			case k3DfxFifoFuncs:
				which2 = k3DfxOldGetFifoInfo;
				// No request data
			break;
			
			case k3DfxSetExclusive:
				which2 = k3DfxOldSetExclusive;
				theOldData.req.optData.setExclusiveReq = inReq->optData.setExclusiveReq;
			break;
			
			case k3DfxSetDisplayMode:
				which2 = k3DfxOldSetMode;
				theOldData.req.optData.setModeReq = inReq->optData.setModeReq;
			break;
			
			case k3DfxSetPrefs:
				which2 = k3DfxOldSetPrefs;
				theOldData.req.optData.setPrefsReq = inReq->optData.setPrefsReq;
			break;
			
			case k3DfxSetModeFlags:
				which2 = k3DfxOldSetModeFlags;
				theOldData.req.optData.setModeFlagsReq = inReq->optData.setModeFlagsReq;
			break;
		}
		
		if(which != 0)
		{
			// This is a call we know how to deal with.  Call the old driver the way it expects.
			theParamBlock.cntrlParam.ioCompletion = 0;
			theParamBlock.cntrlParam.ioVRefNum = 0;
			theParamBlock.cntrlParam.ioCRefNum = inRefNum;
			*((void **) &(theParamBlock.cntrlParam.csParam)) = &theOldData;
			theParamBlock.cntrlParam.csCode = which2;

			result = (OSErr) PBControlSync(&theParamBlock);
		
			if(result == 0)
			{
				switch(which)
				{
					case k3DfxGetDeviceConfig:
						// Pass along the valid information
						outRes->optData.deviceConfigRes = theOldData.res.optData.deviceConfigRes;
						
						// Mock up the information the old driver didn't give us
						outRes->optData.deviceConfigRes.isMaster = 0;
						outRes->optData.deviceConfigRes.numChips = 1;
						outRes->optData.deviceConfigRes.supportsAGP = 0;
						outRes->optData.deviceConfigRes.swizzleOffsets[0] = 0;
						outRes->optData.deviceConfigRes.swizzleOffsets[1] = 0;
						outRes->optData.deviceConfigRes.swizzleOffsets[2] = 0;
						outRes->optData.deviceConfigRes.swizzleOffsets[3] = 0;
					break;
					
					case k3DfxRegisterHRM:
						// No response data
					break;
					
					case k3DfxFifoFuncs:
						outRes->optData.fifoFuncsRes.setLfb = theOldData.res.optData.getFifoInfoRes.fifoInfo->setLfb;
						outRes->optData.fifoFuncsRes.setLfbHost = theOldData.res.optData.getFifoInfoRes.fifoInfo->setLfbHost;
						
					break;

					case k3DfxSetExclusive:
						// No response data
					break;

					case k3DfxSetDisplayMode:
						// No response data
					break;

					case k3DfxSetPrefs:
						// No response data
					break;

					case k3DfxSetModeFlags:
						// No response data
					break;
				}
			}
		}
	}
	
#endif
	
	return(result);
}

/*
______________________________________________________________ hrmInitialize ___
*/

pascal OSErr
hrmInitialize(
  const CFragInitBlock *inInitBlock)
{
  OSErr                 theSuccess;
  
  // rcf
//  Debugger();
  
  __initialize( inInitBlock );
  
  dopen( "hrdw_res_mgr.log" );

#if !__UPDATE__
  dprintf("______________________________________________________________ hrmInitialize ___\n");
  dprintf("hrmInitialize(): ... %d\n");
  dprintf("hrmInitialize(): funcPtr = 0x%08x\n", hrmInitialize);

  
  theSuccess = hrmInitializeGlobals();
	
	// Only make persistent if we found a valid target board
	if (hrmGetNumTargets())
	{
		theSuccess = hrmMakePersistent( inInitBlock );
	}
#else
  dprintf("____________________________________________________ hrmInitialize __ Update ___\n");
  dprintf("hrmInitialize(): ... %d\n");
  dprintf("hrmInitialize(): funcPtr = 0x%08x\n", hrmInitialize);

	if (hrmGetNumTargets())
	{
		theSuccess = hrmMakePersistent( inInitBlock );
	}
  
  theSuccess = noErr;
  
#endif

  return( theSuccess );
}


/*
_______________________________________________________________ hrmTerminate ___

This function will never be called.

*/

pascal void
hrmTerminate()
{

#if !__UPDATE__
  dprintf("_______________________________________________________________ hrmTerminate ___\n");
#else
  dprintf("______________________________________________________ hrmTerminate __ Update___\n");
#endif

  hrmDeRegisterAllTargets();
  
  __terminate();
  
}


/*
__________________________________________________________ hrmMakePersistent ___

We instantiate ourselves (from the system) to increase the instantiation count
by one and thus garantee that we will stick around.

*/

OSErr
hrmMakePersistent(
  const CFragInitBlock *inInitBlock)
{
  THz                  theCurrentZone;
  Ptr                  theMainPtr;
  OSErr                theSuccess = -1;
  Str255               theError;
  
  theCurrentZone = GetZone ();
  SetZone (SystemZone ());
		
  switch( inInitBlock->fragLocator.where ) {
    case kMemoryCFragLocator :
      dprintf("hrmMakePersistent(): CFrag is in mem.\n");
      theSuccess = GetMemFragment(  inInitBlock->fragLocator.u.inMem.address,
                                    inInitBlock->fragLocator.u.inMem.length,
                                    inInitBlock->libName, kLoadCFrag,
                                    &gSysCFragConnectionID, &theMainPtr,
                                    theError);
       break;
      
    case kDataForkCFragLocator:
    case kResourceCFragLocator:
      dprintf("hrmMakePersistent(): CFrag is in a file.\n");
      theSuccess = GetDiskFragment( inInitBlock->fragLocator.u.onDisk.fileSpec,
                                    inInitBlock->fragLocator.u.onDisk.offset,
                                    inInitBlock->fragLocator.u.onDisk.length,
                                    inInitBlock->libName, kLoadCFrag,
                                    &gSysCFragConnectionID, &theMainPtr,
                                    theError);
      break;
      
    default:
      dprintf("hrmMakePersistent(): ### cannot reload the CFrag because"
                                               " we don't know where it is\n");
      dprintf("hrmMakePersistent(): where = %d", inInitBlock->fragLocator.where );
      break;
  }

  dprintf("hrmMakePersistent(): re-instantiation %s\n", (theSuccess == 0) ? "was successfull" : "FAILED");

  SetZone (theCurrentZone);
  
  return theSuccess;
}


/*
_______________________________________________________ hrmInitializeGlobals ___

*/

OSErr
hrmInitializeGlobals()
{
	RegEntryID         theEntry; 
	RegEntryIter       theEntryCookie; 
	RegEntryIterationOp theEntryIterOp; 
	Boolean            theDone;
	long               theVendorID = kVendorID;
	long               theDeviceID;
	/* short              theNumberOfTargets; */
	OSStatus           theSuccess = noErr;

	dprintf("hrmInitializeGlobals(): ...\n");

	/* Initialize extensions */
	NewList(&gExtensionList);
	hrmAddExtensions(&hrmPrimaryExtensionNode);

	/* Initialize preferences */
	hrm_InitPrefs();

	// Search the Name Registry for boards
	RegistryEntryIDInit( &theEntry );
	theSuccess = RegistryEntryIterateCreate( &theEntryCookie );
	if (theSuccess == noErr)
	{
		theEntryIterOp = kRegIterDescendants;

		do {
			// Search for boards whose Vendor ID matches 3dfx's
			theSuccess = RegistryEntrySearch( &theEntryCookie, theEntryIterOp,
					&theEntry, &theDone, kVendorIDStr, &theVendorID, 4);

			if (!theDone && (theSuccess == noErr))
			{
				dprintf("hrmInitializeGlobals(): we found a pci device from the "
						"correct vendor (0x%04x)\n", theVendorID);

				// Now, look to see if the device ID indicates a Voodoo3 or Voodoo4 board.
				theDeviceID = hrmiGetPropertyUInt32( &theEntry, kDeviceIDStr);
				if ( theDeviceID == kDeviceID_Banshee || theDeviceID == kDeviceID_Voodoo3 ||
						theDeviceID == kDeviceID_Voodoo4) 
				{
					short      theRefNum;
					unsigned long theSize = sizeof(theRefNum);

					dprintf("hrmInitializeGlobals(): we found an appropriate target "
							"device (0x%04x)\n", theDeviceID);

					theSuccess = RegistryPropertyGet( &theEntry, "driver-ref", &theRefNum, &theSize );

					if ( theSuccess == noErr )
					{
						// Register this board, and put in our list of boards
						dprintf("hrmInitializeGlobals(): the board is a valid target (%d)\n", theRefNum );
						theSuccess = hrmRegisterPCIBoard( theRefNum, theEntry);
					} else
					{
						dprintf("hrmInitializeGlobals(): ### the driver for this card is not open\n");
						dprintf("hrmInitializeGlobals(): ### we cannot use it!\n");
						/* Make sure we don't stop looking for other boards... this can happen
						if one board doesn't happen to have a monitor attached. */ 
						theSuccess = noErr;
					}

				} else
				{
					dprintf("hrmInitializeGlobals(): device is not a valid target!\n");
					RegistryEntryIDDispose( &theEntry );
				}
			}
			theEntryIterOp = kRegIterContinue;
		}  while( !theDone && theSuccess == noErr);

		RegistryEntryIDDispose( &theEntry );
	}

	return noErr;
}


/*
________________________________________________________ hrmRegisterPCIBoard ___

*/

GDHandle _hrmFindGDeviceHandle(short inRefNum)
{
  GDHandle screenDeviceHandle;
  
  screenDeviceHandle = DMGetFirstScreenDevice(dmOnlyActiveDisplays);
  while(screenDeviceHandle) {             
    /* First, verify that it's a screen device */
    if(TestDeviceAttribute(screenDeviceHandle,screenDevice)) {
      /* Then verify it actually has a driver, so we know gdRefNum
         is valid. */ 
      if(!TestDeviceAttribute(screenDeviceHandle,noDriver)) {
        if((*screenDeviceHandle)->gdRefNum == inRefNum)
          return screenDeviceHandle;
          
      }
    }
    screenDeviceHandle = DMGetNextScreenDevice(screenDeviceHandle,dmOnlyActiveDisplays);
  }
  
  return (GDHandle)0;
}


/********************************************************************************
	hrmRegisterPCIBoard
		short inRefNum
		RegEntryID entryID
		
	
*/
short hrmRegisterPCIBoard(short inRefNum, RegEntryID entryID)
{
	hwcRequest_t   			req;
	hwcResponse_t   		res;
	OSErr           		err = -1;
	THz            			theCurrentZone;
	char 					slotName[256];
	RegPropertyValueSize	propSize = 256;
	long					gestaltResult;
	
	dprintf("hrmRegisterPCIBoard(): inDrvrRefNum %d\n", inRefNum );
	if (!inRefNum)
	{
		dprintf("hrmRegisterPCIBoard(): ### Invalid Driver Ref Number!\n" );
		return err;
	}

	if ( gTargetCount >= kHrmMaxNumberOfTargets )
	{
		dprintf("hrmRegisterPCIBoard(): Too many targets!\n");
		return err;
	}

	// Get the device configuration from the driver
	err = hrmPrivateControlCall(inRefNum, k3DfxGetDeviceConfig, &req, &res);
	if (err)
	{
		dprintf("hrmRegisterPCIBoard(): ### OS driver did not return registration data (%d)\n", err);
		return err;
	}
	hwcControlDeviceConfigRes_t * theDeviceConfig = &res.optData.deviceConfigRes;

	// Fill in our gHdwrData struct with info about the board
	gHdwrData[ gTargetCount ].drvrRefNum = inRefNum;
	gHdwrData[ gTargetCount ].prefsIdx = gTargetCount;

	gHdwrData[ gTargetCount ].boardInfo.hMon = (void *)_hrmFindGDeviceHandle(inRefNum);
	dprintf("hMon: %08lx\n",gHdwrData[ gTargetCount ].boardInfo.hMon);

	gHdwrData[ gTargetCount ].boardInfo.hdc = (void *)inRefNum;
	gHdwrData[ gTargetCount ].boardInfo.devRev = theDeviceConfig->devRev;
	gHdwrData[ gTargetCount ].boardInfo.h3Mem = theDeviceConfig->h3Mem;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.vendorID = theDeviceConfig->vendorID;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.deviceID = theDeviceConfig->deviceID;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.pciBaseAddr[0] = theDeviceConfig->hwBase;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.pciBaseAddr[1] = theDeviceConfig->lfbBase;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.pciBaseAddr[2] = theDeviceConfig->ioPortBase;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.pciBaseAddr[2] = theDeviceConfig->ioPortBase;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.isMaster = theDeviceConfig->isMaster;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.numChips = theDeviceConfig->numChips;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.swizzleOffset[0] = theDeviceConfig->swizzleOffsets[0];
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.swizzleOffset[1] = theDeviceConfig->swizzleOffsets[1];
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.swizzleOffset[2] = theDeviceConfig->swizzleOffsets[2];
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.swizzleOffset[3] = theDeviceConfig->swizzleOffsets[3];
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.initialized = FXTRUE;

	gHdwrData[ gTargetCount ].boardInfo.linearInfo.linearAddress[0] = theDeviceConfig->hwBase;
	gHdwrData[ gTargetCount ].boardInfo.linearInfo.linearAddress[1] = theDeviceConfig->lfbBase;
	gHdwrData[ gTargetCount ].boardInfo.linearInfo.linearAddress[2] = theDeviceConfig->ioPortBase;
	gHdwrData[ gTargetCount ].boardInfo.linearInfo.linearAddress[3] = 0;
	gHdwrData[ gTargetCount ].boardInfo.linearInfo.initialized = FXTRUE;

	gHdwrData[ gTargetCount ].boardInfo.regInfo.ioMemBase = theDeviceConfig->hwBase + SST_IO_OFFSET;
	gHdwrData[ gTargetCount ].boardInfo.regInfo.cmdAGPBase = theDeviceConfig->hwBase + SST_CMDAGP_OFFSET;
	gHdwrData[ gTargetCount ].boardInfo.regInfo.waxBase = theDeviceConfig->hwBase + SST_2D_OFFSET;
	gHdwrData[ gTargetCount ].boardInfo.regInfo.sstBase = theDeviceConfig->hwBase + SST_3D_OFFSET;
	gHdwrData[ gTargetCount ].boardInfo.regInfo.lfbBase = theDeviceConfig->hwBase + SST_LFB_OFFSET;
	gHdwrData[ gTargetCount ].boardInfo.regInfo.rawLfbBase = theDeviceConfig->lfbBase;
	gHdwrData[ gTargetCount ].boardInfo.regInfo.ioPortBase = theDeviceConfig->ioPortBase & ~0x1;
	gHdwrData[ gTargetCount ].boardInfo.regInfo.initialized = FXTRUE;

	gHdwrData[ gTargetCount ].supportsAGP = theDeviceConfig->supportsAGP;
	gHdwrData[ gTargetCount ].boardInfo.pciInfo.is66MHz = false;

	// Is this a G3 computer?
	if (!(err = Gestalt(gestaltNativeCPUtype, &gestaltResult)) && gestaltResult == gestaltCPU750)
	{
		// Look at the slot-name property, and see if it's "J13". This is the 66MHz slot in
		// the blue-and-white G3.
		err = RegistryPropertyGet(&entryID, "AAPL,slot-name", &slotName, &propSize);
		if (!err && propSize == 4 && slotName[0] == 'J' && slotName[1] == '1' && slotName[2] == '2')
		{
			gHdwrData[gTargetCount].boardInfo.pciInfo.is66MHz = true;
		}
	}

	// MBW -- XXX -- TAKE THIS OUT -- the new V3 driver doesn't seem to return valid information here.
	//		gHdwrData[ gTargetCount ].supportsAGP = FXTRUE;

	gHdwrData[ gTargetCount ].prefs = theDeviceConfig->prefs;

	dprintf("Graphics Clock: %d Mhz\n",theDeviceConfig->prefs & 0xff);
	dprintf("initializing 2D command FIFO\n");

	/* Initialize memory manager */
	NewList(&gHdwrData[ gTargetCount ].memAreas);

	/* Register callback routine with driver */
	req.optData.hrmDispatchReq.hrmGetExtension = hrmGetExtension;
	req.optData.hrmDispatchReq.hrmBoard = &gHdwrData[ gTargetCount ];
	err = hrmPrivateControlCall(inRefNum, k3DfxRegisterHRM, &req, &res);

	gHdwrData[ gTargetCount ].agpEnabled = FXFALSE;
	gHdwrData[ gTargetCount ].agpAllocated = FXFALSE;

	/* If this fails, it's okay. */
	if (gHdwrData[ gTargetCount ].supportsAGP && ((long)AGPNewMemory != kUnresolvedCFragSymbolAddress))
	{
		/* Okay, this board claims to support AGP and the system AGP library is around. */
		/* Allocate some memory for it. */
		FxU32 value;

		err = AGPGetStatus(kAGPQueryBaseAddress,&value);
		dprintf("succ: %d  baseAddress: %08lx\n",err,value);
		err = AGPGetStatus(kAGPQueryEnabled,&value);
		dprintf("succ: %d  enabled: %08lx\n",err,value);
		err = AGPGetStatus(kAGPQueryMaxAGPMemory,&value);
		dprintf("succ: %d  maxMem: %08lx\n",err,value);
		err = AGPGetStatus(kAGPQueryFreeAGPMemory,&value);
		dprintf("succ: %d  freeMem: %08lx\n",err,value);

		dprintf("board %08lx supports AGP\n");
		gHdwrData[ gTargetCount ].agpEnabled = FXTRUE;

		theCurrentZone = GetZone ();
		SetZone (SystemZone ());

		gHdwrData[ gTargetCount ].agpAddress.systemLogicalAddress = 0;
		gHdwrData[ gTargetCount ].agpAddress.agpLogicalAddress = 0;

		err = AGPNewMemory(&gHdwrData[ gTargetCount ].agpAddress, 1024*1024, false);
		if(err == noErr) 
		{
			dprintf("AGPNewMemory succeeded\n");

			// MBW -- XXX -- If we enable the AGP FIFO, we need to commit with true, 
			// and we will manage the caches.
			err = AGPCommitMemory(&gHdwrData[ gTargetCount ].agpAddress, false);
			if(err == noErr) 
			{
				dprintf("AGPCommitMemory succeeded\n");
				gHdwrData[ gTargetCount ].agpEnabled = FXTRUE;
				gHdwrData[ gTargetCount ].agpAllocated = FXTRUE;
				gHdwrData[ gTargetCount ].agpSize = 1024*1024;
				#if 0
				{
					FxU32 start, end, i, j;
					vector unsigned long zero = (vector unsigned long)(0);

					start = clock();
					for (i = 0; i < 1024; i++) 
					{
						vector unsigned long *dst = (vector unsigned long *)gHdwrData[ gTargetCount ].agpAddress.systemLogicalAddress;
						for (j = 0; j < 1024*1024; j += sizeof(zero))
							*dst++ = zero;
					}
					end = clock();
					dprintf("mb/second: %f\n",(float)(1024.0 / ((end - start) / (double)CLOCKS_PER_SEC)));
				}
				{
					FxU32 start, end, i, j;

					start = clock();
					for (i = 0; i < 1024; i++)
					{
						double *dst = (double *)gHdwrData[ gTargetCount ].agpAddress.systemLogicalAddress;
						for (j = 0; j < 1024*1024; j += sizeof(double))
							*dst++ = 0;
					}
					end = clock();
					dprintf("mb/second: %f\n",(float)(1024.0 / ((end - start) / (double)CLOCKS_PER_SEC)));
				}
				{
					FxU32 start, end, i, j;

					start = clock();
					for (i = 0; i < 1024; i++)
					{
						long *dst = (long *)gHdwrData[ gTargetCount ].agpAddress.systemLogicalAddress;
						for (j = 0; j < 1024*1024; j += sizeof(long))
							*dst++ = 0;
					}
					end = clock();
					dprintf("mb/second: %f\n",(float)(1024.0 / ((end - start) / (double)CLOCKS_PER_SEC)));
				}
				#endif

			} else
			{
				AGPDisposeMemory(&gHdwrData[ gTargetCount ].agpAddress);
				dprintf("AGPCommitMemory failed\n",err);
			}
		} else
		{
			dprintf("AGPNewMemory failed: %d\n",err);
		}

		SetZone (theCurrentZone);		  
		err = noErr;
	}

	/* Set up the default 2D Command FIFO location and size. */
	/* Initialize the 2D command FIFO.  The allocation is done there now. */
	gHdwrData[ gTargetCount ].fifoInfo.exclusiveMode = FXFALSE;
	gHdwrData[ gTargetCount ].exclusiveMode = FXFALSE;

	hrmInitFifo(&gHdwrData[ gTargetCount ], FXTRUE);
	dprintf("command fifo set up.\n");

	/* Set up the arbitration variables for this board */
	gHdwrData[ gTargetCount ].arbiterDepth = 0;
	gHdwrData[ gTargetCount ].arbiterQueue.qFlags = 0;
	gHdwrData[ gTargetCount ].arbiterQueue.qHead = 0;
	gHdwrData[ gTargetCount ].arbiterQueue.qTail = 0;

	#if 0        
	err = mmInitializeMemoryManager( &gHdwrData[ gTargetCount ] );
	#endif
	hrmSetModePrefs( &gHdwrData[ gTargetCount ] );

	dprintf("memory manager set up.\n");

	gTargetCount++;
	return 0;
}

/*
____________________________________________________ hrmDeRegisterAllTargets ___

*/

void              hrmDeRegisterAllTargets( void )
{
	long i;
	hwcRequest_t       req;
	
	for(i = 0; i < gTargetCount; i++ ) 
	{
    	/* De-Register callback routine with driver */
    	req.optData.hrmDispatchReq.hrmGetExtension = 0;
    	
		hrmPrivateControlCall((short)gHdwrData[ i].boardInfo.hdc, k3DfxRegisterHRM, &req, NULL);
	}
}


/*
______________________________________________________ hrmiGetPropertyUInt32 ___

*/

unsigned long
hrmiGetPropertyUInt32(
  RegEntryID *       inEntry,
  char *             inPropertyName )
{
  RegPropertyNameBuf thePropertyName;
  RegPropertyIter    thePropertyCookie; 
  Boolean            theDone;
  unsigned long      theSize = 4;
  unsigned long      theResult = 0;
  OSStatus           theErr = noErr;


  theErr = RegistryPropertyIterateCreate( inEntry, &thePropertyCookie);
  if ( theErr == noErr) {
			
    do {
      theErr = RegistryPropertyIterate( &thePropertyCookie,
                                                   thePropertyName, &theDone);

      if ( !theDone && (theErr == noErr)) {
        if ( strcmp( thePropertyName, inPropertyName ) == 0) {


          theErr = RegistryPropertyGet( inEntry, inPropertyName,
							&theResult, &theSize);
          if ( theErr == noErr ) theDone = true;
        }
      }
    }  while( !theDone && theErr == noErr);
  }
	
  RegistryPropertyIterateDispose( &thePropertyCookie );

  return theResult;
}

/*
__________________________________________________________ hrmIdentifyTarget ___

*/

hrmBoard_t *      hrmIdentifyTarget(
                          short                inRefNum)
{
	int i;
	
	for(i = 0; i < gTargetCount; i++) {
		if(gHdwrData[ i ].boardInfo.hdc == (void *)inRefNum) {
		  return &gHdwrData[ i ];
		}
	}
	
	return NULL;
}

/*
___________________________________________________________ hrmGetNumTargets ___

*/

long hrmGetNumTargets( void )
{
  return gTargetCount;
}

/*
___________________________________________________________ hrmGetNumTargets ___

*/

hrmBoard_t *hrmGetTargetAtIndex( long index )
{
	if( index < 0 || index >= gTargetCount)
		return NULL;
	
	return &gHdwrData[ index ];
}
 

/*
____________________________________________________________ hrmGetExtension ___

*/

void *hrmGetExtension(const char *extensionName)
{
	Node_t *node;
	long i;
	
	/* Walk list of extension nodes. */
  for(node = gExtensionList.head; node->succ; node = node->succ) {
  
    /* Get a more convenient pointer */
    hrmExtensionNode_t *hrmExtNode = (hrmExtensionNode_t *)node;

    for(i = 0; i < hrmExtNode->count; i++) {
      
      if(!strcmp(hrmExtNode->table[i].extName, extensionName))
        return hrmExtNode->table[i].extValue;
    }   
  }
  /* Extension not found */
  return NULL;
}


/*
___________________________________________________________ hrmAddExtensions ___

*/

void hrmAddExtensions(hrmExtensionNode_t *hrmExtensionNode) {
  AddTail(&gExtensionList,&hrmExtensionNode->node);
}


/*
________________________________________________________ hrmRemoveExtensions ___

*/

void hrmRemoveExtensions(hrmExtensionNode_t *hrmExtensionNode) {
  Remove(&hrmExtensionNode->node);
  /* Bad things happen if you remove something twice, so this will cause
     an immediate crash rather than corrupt a valid list and create a hard
     to find bug. */
  hrmExtensionNode->node.succ = 0;
  hrmExtensionNode->node.prev = 0;
}


/*
_________________________________________________________ hrmGetDeviceConfig ___

*/

short hrmGetDeviceConfig(hrmBoard_t *board, hrmDeviceConfig_t *config)
{
	hwcResponse_t      res;
	OSErr              theSuccess = -1;
  			
	theSuccess = hrmPrivateControlCall(board->drvrRefNum, k3DfxGetDeviceConfig, NULL, &res);
    
	if(theSuccess == noErr) 
	{
		hwcControlDeviceConfigRes_t * theDeviceConfig = &res.optData.deviceConfigRes;
		config->lfbBase = theDeviceConfig->lfbBase;
		config->pciStride = theDeviceConfig->pciStride;
		config->hwStride = theDeviceConfig->hwStride;
		config->tileMark = theDeviceConfig->tileMark;
	}

	config->device = (GDHandle) board->boardInfo.hMon;

	return theSuccess;
}
/*
typedef struct hrmBoardInfo_s
{
  unsigned long size;
  unsigned long screenDeviceHandle;
  unsigned long h3Mem;
  unsigned long deviceRev;
  unsigned long vendorID;
  unsigned long deviceID;
  unsigned long pciBaseAddr[4];
  unsigned long devNum;
  unsigned long isMaster;
  unsigned long numChips;
  unsigned long swizzleOffset[4];
} hrmBoardInfo_t;
*/
void hrmGetTargetBoardInfoExt(hrmBoard_t *board, hrmBoardInfo_t *boardInfo)
{
	if(board && boardInfo) 
	{
		if(boardInfo->size >= sizeof(hrmBoardInfo_t)) 
		{
			/* At least big enough for version 1 */
			boardInfo->driverRefNum = (FxI32)board->boardInfo.hdc;
			boardInfo->h3Mem = board->boardInfo.h3Mem;
			boardInfo->deviceRev = board->boardInfo.devRev;
			boardInfo->vendorID = board->boardInfo.pciInfo.vendorID;
			boardInfo->deviceID = board->boardInfo.pciInfo.deviceID;
			boardInfo->pciBaseAddr[0] = board->boardInfo.pciInfo.pciBaseAddr[0];
			boardInfo->pciBaseAddr[1] = board->boardInfo.pciInfo.pciBaseAddr[1];
			boardInfo->pciBaseAddr[2] = board->boardInfo.pciInfo.pciBaseAddr[2];
			boardInfo->pciBaseAddr[3] = 0;
			boardInfo->devNum = 0;
			boardInfo->isMaster = board->boardInfo.pciInfo.isMaster;
			boardInfo->numChips = board->boardInfo.pciInfo.numChips;
			boardInfo->swizzleOffset[0] = board->boardInfo.pciInfo.swizzleOffset[0];      
			boardInfo->swizzleOffset[1] = board->boardInfo.pciInfo.swizzleOffset[1];      
			boardInfo->swizzleOffset[2] = board->boardInfo.pciInfo.swizzleOffset[2];      
			boardInfo->swizzleOffset[3] = board->boardInfo.pciInfo.swizzleOffset[3];      
			boardInfo->is66MHz =board->boardInfo.pciInfo.is66MHz;  
		}
	} 
}  

	// MBW -- XXX -- backwards compatibility
	// The code inside the following #ifdef block implements backwards compatibility with
	// the circa b11 voodoo3 drivers.  Once we have the updated drivers, it can safely be deleted.
#if OLD_DRIVER_COMPAT
hwcBoardInfo *hrmGetTargetBoardInfo(hrmBoard_t *board)
{
//	if(board) 
//	{
//		return(&board->boardInfo);
//	}
	return NULL;
}  
#endif


static int sst1InitFgets(char *string, short refNum);
static int sst1InitFgetc(short refNum);
static int sst1InitParseFieldDac(char *);
static int sst1InitParseFieldCfg(hrmBoard_t *board, char *string);
static void sst1InitToLower(char *string);

#define kMaxEnvVarLen 100
#define kMaxEnvValLen 256
typedef struct {
    char envVariable[kMaxEnvVarLen];
    char envValue[kMaxEnvValLen];
    void *nextVar;
} sst1InitEnvVarStruct;

sst1InitEnvVarStruct *envVarsBase = (sst1InitEnvVarStruct *) NULL;

void hrmSetModePrefs(hrmBoard_t *board)
{
  static FxBool retVal = FXFALSE;

  int inCfg;
  FILE *file = (FILE *) NULL;
  char buffer[1024], filename[256];
  short iniRefNum;
  
	filename[0] = '\0';
	
	dprintf("hrmSetModePrefs(%08lx)\n",board);
	
	{
		FSSpec iniSpec = {
			0, 0,
			"\pVoodoo3ModePrefs"
		};
		Boolean foundP = false;
		
		/* Check the mac's version of the 'search path' */
		if (!foundP) {
			OSType folderList[] = { kPreferencesFolderType, kExtensionFolderType };
			int i;

			for(i = 0; i < sizeof(folderList) / sizeof(folderList[0]); i++) {
				short vRefNum;
				long dirId;
				
				if (FindFolder(kOnSystemDisk, folderList[i], false, 
											 &vRefNum, &dirId) == noErr) {
				
					CInfoPBRec thePB;
					
					thePB.hFileInfo.ioCompletion = NULL;
					thePB.hFileInfo.ioNamePtr = iniSpec.name;
					thePB.hFileInfo.ioVRefNum = vRefNum;
					thePB.hFileInfo.ioDirID = dirId;
					
					thePB.hFileInfo.ioFDirIndex = 0;
					
					foundP = ((PBGetCatInfoSync(&thePB) == noErr) &&
										((thePB.hFileInfo.ioFlAttrib & (0x01 << 4)) == 0));
					if (foundP) {
						iniSpec.vRefNum = vRefNum;
						iniSpec.parID = dirId;
						dprintf("found prefs file\n");
						/* Now open the damned thing, using the MacOS API du jour. */
						if(FSpOpenDF(&iniSpec, fsRdPerm, &iniRefNum) == noErr) {
						  dprintf("opened prefs file\n");
						  while(sst1InitFgets(buffer, iniRefNum)) {
						    buffer[strlen(buffer)-1] = (char) NULL;
						    dprintf("buffer: %s\n",buffer);
						    if(!strcmp(buffer, "[VOODOO3MODES]")) {
						      dprintf("in config\n");
						      inCfg = 1;
						      continue;
						    } else if(buffer[0] == '[') {
						      dprintf("out of config\n");
						      inCfg = 0;
						      continue;
						    }
						
						    if(inCfg) {
						      sst1InitParseFieldCfg(board, buffer);
						    } 						  
						  }
						  FSClose(iniRefNum);
						}
						break;
					}
				}
			}
		}
	}

__errExit:
;
}

void hrmArbiterGet(hrmBoard_t *board)
{
	board->arbiterDepth++;
}

void hrmArbiterRelease(hrmBoard_t *board)
{
	if(board->arbiterDepth == 1)
	{
		// The board mutex level just dropped to 0.  Execute any queued procs.
		while(board->arbiterQueue.qHead != nil)
		{
			// Queue is not empty.
			hrmArbiterQElem *elem = (hrmArbiterQElem*)(board->arbiterQueue.qHead);
			Dequeue(board->arbiterQueue.qHead, &(board->arbiterQueue));
			
			// Execute the queued task.
			elem->delayedTask(elem->userData);
		}
		
		board->arbiterDepth--;
	}
	else if(board->arbiterDepth <= 0)
	{
		// This is bad.  Someone isn't balancing calls. 
		board->arbiterDepth = 0;

		// MBW -- XXX -- TAKE THIS OUT before shipping
		//DebugStr("\pERROR: board mutex level went negative.");
	}
	else
	{
		board->arbiterDepth--;
	}
}

void hrmArbiterQueue(hrmBoard_t *board, hrmArbiterQElem *elem)
{
	// Always queue the element.
	Enqueue((QElemPtr)elem, &(board->arbiterQueue));

	if(board->arbiterDepth == 0)
	{
		// The board isn't busy.  Flush the queue.
		hrmArbiterGet(board);
		hrmArbiterRelease(board);
	}
}


static void sst1InitFixFilename(char *dst, char *src)
{
    while(*src) {
        *dst++ = *src;
        if(*src == '\\')
            *dst++ = *src;
        src++;
    }
    *dst = (char) NULL;
}


static int sst1InitFgets(char *string, short refNum)
{
    int validChars = 0;
    char *ptr = string;
    int charRead;

    while(0 != ((charRead = sst1InitFgetc(refNum)))) {
        *ptr++ = (char) charRead;
        validChars++;
        if(charRead == '\n' || charRead == '\r') {
            *ptr++ = (char) NULL;
            break;
        }
    }
    return(validChars);
}

static int sst1InitFgetc(short refNum)
{
    static int column = 0;
    static int validChars = 0;
    int charRead, charReadL;
    int inComment;
    long count;
    OSErr err;
    char c;
    
    inComment = 0;
    while(1) {
        count = 1;
        err = FSRead(refNum, &count, &c);
        if(!err) {
          charRead = c;
        } else {
          charRead = -1;
        }
        if(inComment == 1) {
            if(charRead <= 0)
                return(0);
            else if(charRead == '\n' || charRead == '\r')
                inComment = 0;
            column = 0;
            validChars = 0;
            continue;
        } else if(column == 0 && charRead == '#') {
            /* Comment line */
            inComment = 1;
            column = 0;
            validChars = 0;
        } else if(charRead <= 0) {
                return(0);
        } else {
            if(charRead == '\n' || charRead == '\r') {
                if(validChars > 0) {
                    validChars = 0;
                    column = 0;
                    return(charRead);
                } else
                    continue;
            } else {
                //if(isspace(charRead))
                //    continue;
                validChars++;
                column++;
                charReadL = (islower(charRead)) ? toupper(charRead) : charRead;
                return(charReadL);
            }
        }
    }
}

static int sst1InitParseFieldCfg(hrmBoard_t *board, char *string)
{
    int count, displayModeID, timingFlags;
    
    count = sscanf(string,"%d %d",&displayModeID,&timingFlags);
    dprintf("count: %d %d %d\n",count,displayModeID,timingFlags);
    
    if(count == 2) {
      hrmSetModeFlags(board,displayModeID,timingFlags);
    }
    return 1;
}

static void sst1InitToLower(char *string)
{
    char *ptr = string;

    while(*ptr) {
        *ptr = (isupper(*ptr)) ? tolower(*ptr) : *ptr;
        ptr++;
    }
}

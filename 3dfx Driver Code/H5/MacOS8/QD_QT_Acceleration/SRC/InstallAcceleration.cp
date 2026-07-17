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
** $Header: /devel/h3/MacOS8/H3_2D_Acceleration/src/GraphicsAcceleration.c 2     7/02/99 3:33p Kcd $
** $Log: /devel/h3/MacOS8/H3_2D_Acceleration/src/GraphicsAcceleration.c $
** 
** 2     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

// Defines
#define WANT_DCON	0
#include "DConLoader.h"

// Includes
#define __MACERRORS__
#include <Quickdraw.h>
#include <Power.h>
#include <Displays.h>
#include <Profiler.h>

#include "NQDAcceleration.h"
#include "Utilities.h"

#include "minihwc.h"
#include "hwcio.h"

#include "hrm_Loader.h"
#include "DynamicPatches.h"
#include "StdTextPatch.h"
#include "DrawPictPatch.h"

#include "QuickTimeCodecs.h"

#include "GWorldSupport.h"

// Enums

// Globals
Boolean  accelerationInstalled = false;
ProcInfoType __procinfo =  kPascalStackBased;
HRMPrefsTable			*gPreferences = nil;

SleepQRec	gSleepQueueRec;

// Function Declarations
extern pascal OSErr  __initialize(const CFragInitBlock *inInitBlock);
extern pascal OSErr  __terminate();

pascal OSErr	InitializeFn(const CFragInitBlock *inInitBlock);
pascal OSErr	TerminateFn();
OSErr  			UninstallAcceleration ();
void 			InstallNQDHook(NQDGetBlitProcPtr proc, NQDFinishBlitProcPtr finishProc, UInt32 index);
void 			RemoveNQDHook(NQDGetBlitProcPtr proc, UInt32 index);
long 			WakeUpNotifyProc(long message, SleepQRecPtr qRecPtr);
void 			DisplayNotificationProc(void *userData, short theMessage, void *notifyData);
void			PrefsChangedHook(void);

OSErr	NewGWorldPatch(GWorldPtr *offscreenGWorld, short PixelDepth,
							   const Rect *boundsRect, CTabHandle cTable,
							   GDHandle aGDevice, GWorldFlags flags);

long  MenuSelectPatch(Point startPt);


extern "C" Boolean	InstallAcceleration ();
extern "C" void 	ApplicationMain();

// Routine Descriptors
RoutineDescriptor WakeUpNotifyProcRD = BUILD_ROUTINE_DESCRIPTOR(uppSleepQProcInfo, WakeUpNotifyProc);
RoutineDescriptor DisplayNotificationProcRD = BUILD_ROUTINE_DESCRIPTOR(uppDMExtendedNotificationProcInfo,
						DisplayNotificationProc);
		
// Function Definitions

/********************************************************************************
	InitializeFn
		const CFragInitBlock *inInitBlock
		
	
*/
pascal OSErr  InitializeFn(const CFragInitBlock *inInitBlock)
{
	OSErr  theErr;

	theErr =  __initialize( inInitBlock );
	if(theErr != 0)
		return theErr;

#ifdef VOODOO4
	dopen("2d_accel_v4.log");
#else
	dopen("2d_accel_v3.log");
#endif

	dprintf("InitializeGraphicsAcceleration 1\n");

	return (noErr);
}

/********************************************************************************
	TerminateFn
		
	
*/
pascal OSErr  TerminateFn()
{
	dprintf("TerminateFn\n");

	/* uninstall acceleration */
	UninstallAcceleration ();

	__terminate();

	return (noErr);
}

/********************************************************************************
	InstallAcceleration
		
	
*/
Boolean  InstallAcceleration()
{
	OSErr  theErr;

	if (!accelerationInstalled)
	{
// rcf
//Debugger();
		
		// Get preferences struct from the HRM
		gPreferences = hrm_GetPrefsTable();
		
		// Register for notification when the prefs change.

//		this is failing when v5 but no v3 is installed
//		hrm_RegisterPrefsChanged(PrefsChangedHook, kPrefsFlag2D | kPrefsFlagQT);
		
		/* initialize hardware */
		if ((theErr = InitializeAccelerationHardware ()) != noErr)
			return (false);

#ifdef VOODOO4
		dprintf("installing V4 acceleration\n");
#else
		dprintf("installing V3 acceleration\n");
#endif
 		/* install acceleration hooks */
		InstallNQDHook(GetAcceleratedBitBlitProc, H3BlitFinish, kBitBlitIndex);
		InstallNQDHook(GetAcceleratedBitBlitProc, H3BlitFinish, kRgnBlitIndex);
		InstallNQDHook(GetAcceleratedBitBlitProc, H3BlitFinish, kScaleBlitIndex);
		
		InstallNQDHook(GetAcceleratedPatBlitProc, H3BlitFinish, kPatBlitIndex);
		InstallNQDHook(GetAcceleratedPatBlitProc, H3BlitFinish, kPatRgnBlitIndex);
		
		InstallNQDHook(GetAcceleratedLineBlitProc, H3LineBlitFinish, kLineBlitIndex);
		InstallNQDHook(GetAcceleratedSlabBlitProc, H3SlabBlitFinish, kSlabBlitIndex);

#if INSTALL_ADVANCED_2D_ACCEL
		// Initialize the font manager and the pict cache
		gFonts.Init();
		DrawPictPatchInit();

		// Register our patches with the HRM's patch manager
		PatchGroup	patches;				
		patches.Init();
		patches.stdBitsPatch = StdBitsPatch;
		patches.stdTextPatch = StdTextPatch;
		patches.stdTextMeasPatch = StdTextMeasPatch;
		patches.drawPicturePatch = DrawPictPatch;
		patches.newGWorldPatch = NewGWorldPatch;
		patches.menuSelectPatch = MenuSelectPatch;
		InitDynamicPatches(&patches);
#endif
		
		// Register our QuickTime acceleration components
#if INSTALL_QUICKTIME
		InstallQuickTimeAccel();
#endif
		
		// Install our sleep/wake proc
		gSleepQueueRec.sleepQLink = 0;
		gSleepQueueRec.sleepQType = sleepQType;
		gSleepQueueRec.sleepQProc = &WakeUpNotifyProcRD;
		gSleepQueueRec.sleepQFlags = 0;
		SleepQInstall(&gSleepQueueRec);
		
		// Install a Display Manager Callback proc
		DMRegisterExtendedNotifyProc(&DisplayNotificationProcRD, 0, 0, 0);
		
#if INSTALL_ADVANCED_2D_ACCEL
		// Install the GWorld Support
		for (UInt32 index = 0; index < 16; ++index)
		{
			// Is there a board at this memory address?
			if (h3_info[index].boardFlags & 1)
			{
				theErr = InstallGWorldSupport(&h3_info[index]);
			}
		}
#endif

		accelerationInstalled = true;
	}
	
	return (true);
}

/********************************************************************************
	UninstallAcceleration
		
	Called when the fragment is terminated. Note that this function could also be
	called directly, to remove acceleration without closing the fragment.
*/
OSErr  UninstallAcceleration()
{
	if (accelerationInstalled)
	{ 
		/* remove acceleration hooks */
 		RemoveNQDHook(GetAcceleratedBitBlitProc, kBitBlitIndex);
 		RemoveNQDHook(GetAcceleratedBitBlitProc, kRgnBlitIndex);
  		RemoveNQDHook(GetAcceleratedBitBlitProc, kScaleBlitIndex);
		
 		RemoveNQDHook(GetAcceleratedPatBlitProc, kPatBlitIndex);
 		RemoveNQDHook(GetAcceleratedPatBlitProc, kPatRgnBlitIndex);
 		
 		RemoveNQDHook(GetAcceleratedLineBlitProc, kLineBlitIndex);
 		RemoveNQDHook(GetAcceleratedSlabBlitProc, kSlabBlitIndex);

		// Deallocate all our scratch space blocks
		for (UInt32 index = 0; index < 16; ++index)
		{
			// Is there a board at this memory address?
			if (h3_info[index].boardFlags & 1)
			{
				if (h3_info[index].scratchSpace)
				{
					hrmFreeBlock(h3_info[index].scratchSpace);
				}

#if INSTALL_ADVANCED_2D_ACCEL
				// Uninstall the GWorldSupport
				UninstallGWorldSupport(&h3_info[index]);
#endif
			}
		}
		
#if INSTALL_ADVANCED_2D_ACCEL
		// Deinstall our dynamic patches
		RemoveDynamicPatches();
		
		// Close the font manager port.
		gFonts.Terminate();
		
		// Terminate the picture cache
		DrawPictPatchTerminate();
#endif

#if INSTALL_QUICKTIME
		// Unregister our QuickTime acceleration components
		RemoveQuickTimeAccel();
#endif
		
		// Remove our Sleep Queue entry
		SleepQRemove(&gSleepQueueRec);
		
		// Remove our Display Manager proc
		DMRemoveExtendedNotifyProc(&DisplayNotificationProcRD, 0, 0, 0);
		
		// Unregister from HRM notification of prefs changes
//		hrm_UnregisterPrefsChanged(PrefsChangedHook);
		
		accelerationInstalled = false;
	}

	return (noErr);
}

/********************************************************************************
	HackNQDMisc
		long selector
		long *paramBlock	
		
	The 3.3 Universal Interfaces no longer include the NQDMisc function, so we include it here.	
*/
UniversalProcPtr NQDMiscTrap : 0x1D0C;
pascal long HackNQDMisc(long selector, long *paramBlock)
{
	// Call the NQDMisc trap from the A-trap table
	return CallUniversalProc(NQDMiscTrap, kPascalStackBased | RESULT_SIZE(kFourByteCode) |
			STACK_ROUTINE_PARAMETER(1, kFourByteCode) | STACK_ROUTINE_PARAMETER(2, kFourByteCode),
			selector, paramBlock);
}


/********************************************************************************
	InstallNQDHook
		NQDGetBlitProcPtr proc
		UInt32 index
		
	
*/
void InstallNQDHook(NQDGetBlitProcPtr proc, NQDFinishBlitProcPtr finishProc, UInt32 index)
{
 	NQDGetBlitProcParamBlock  paramBlock;
  
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) proc;
    paramBlock.finishProc = finishProc;
    paramBlock.index = index;
    HackNQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);
}

/********************************************************************************
	RemoveNQDHook
		NQDGetBlitProcPtr proc
		UInt32 index
		
	
*/
void RemoveNQDHook(NQDGetBlitProcPtr proc, UInt32 index)
{
 	NQDGetBlitProcParamBlock  paramBlock;

    paramBlock.getBlitProc = (NQDGetBlitProcPtr) proc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = index;
    HackNQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);
}


/********************************************************************************
	WakeUpNotifyProc
		long message
		SleepQRecPtr qRecPtr
		
	This is a Power Manager callback routine, which gets notified of system sleep
	and wake events. It's main purpose is to reset the board state
*/
long WakeUpNotifyProc(long message, SleepQRecPtr qRecPtr)
{
#pragma unused (qRecPtr)

	if (message == sleepWakeUp || message == dozeWakeUp)
	{
		// Reset the register state for each board in the system
		for (UInt32 index = 0; index < 16; ++index)
		{
			if (h3_info[index].boardFlags & 1)
			{
				hrmBoardInfo_t	*bInfo = h3_info[index].bInfo;
				
				// Set the clip0 registers to a wide-open clip range
				((UInt32 *) bInfo->pciBaseAddr[0])[reg_clip0min + 2] = 0;
				((UInt32 *) bInfo->pciBaseAddr[0])[reg_clip0max + 2] = 0xFFFFFFFF;
				
				// Set the commandExtra register to 0
				((UInt32 *) bInfo->pciBaseAddr[0])[reg_commandEx + 2] = 0;
			}
		}
	}
	
	return 0;
}

/********************************************************************************
	DisplayNotificationProc
		void *userData
		short theMessage
		void *notifyData
		
	
*/
void DisplayNotificationProc(void *userData, short theMessage, void *notifyData)
{
#pragma unused (userData, theMessage, notifyData)
	h3Info		*h3InfoPtr;
	GDHandle	gd;

	// Reset the register state for each board in the system
	for (UInt32 index = 0; index < 16; ++index)
	{
		if (h3_info[index].boardFlags & 1)
		{
			// Set the clip0 registers to a wide-open clip range
			h3_info[index].regs2D->clip0min = 0;
			h3_info[index].regs2D->clip0max = 0xFFFFFFFF;
			
			// Set the commandExtra register to 0
			h3_info[index].regs2D->commandEx = 0;
			
// this configuration setting does not belong in here, but if it's
// not performed, bad things happen. So for the case when the user
// has already upgraded the extension, but not the ROM yet, we
// must do this.
// For now, since we don't need it any other way, we can just leave
// it as is. 
			h3_info[index].regsIO->lfbMemoryConfig = 0x018A5FFF;
		}
	}
	
	// Search for the gDevice that matches each board
	for (gd = GetDeviceList(); gd != 0; gd = GetNextDevice(gd))
	{
		// Is the gDevice active?
		if (gd[0][0].gdFlags & 0x8000)
		{
			PixMap *gdPMapPtr = *gd[0][0].gdPMap;
			
			// Does this device map to a 3dfx board?
			if (gdPMapPtr && (h3InfoPtr = FindH3Info(gdPMapPtr->baseAddr)))
			{
				// Set the depth field to the depth of the screen on this board.
				// This is important for Voodoo3 cards, as it determine the swap
				// mode for writes to memory.
				h3InfoPtr->depth = gdPMapPtr->pixelSize;
			}
		}
	}
}

/********************************************************************************
	PrefsChangedHook
*/
void PrefsChangedHook(void)
{
	// We don't really need to do anything here, do we?
}

#pragma mark -
#pragma mark ¥ÊNewGWorld Patch Hack

#include "QDOffscreen.h"
#include "Traps.h"


UInt32 gNewGWorldAddr[2];

typedef OSErr (*newGWorldProc)(GWorldPtr *offscreenGWorld, short PixelDepth,
							   const Rect *boundsRect, CTabHandle cTable,
							   GDHandle aGDevice, GWorldFlags flags);

bool gInsideMenuSelect = false;

/********************************************************************************
	NewGWorldPatch
		GWorldPtr *offscreenGWorld
		short PixelDepth
		const Rect *boundsRect
		CTabHandle cTable
		GDHandle aGDevice
		GWorldFlags flags
		
	
*/
OSErr	NewGWorldPatch(GWorldPtr *offscreenGWorld, short PixelDepth,
							   const Rect *boundsRect, CTabHandle cTable,
							   GDHandle aGDevice, GWorldFlags flags)
{
	/* MBW -- XXX -- This hack doesn't work in some cases.  
		Specifically, when the menu manager has created offscreen GWorlds to stash
		drawn menus and you move the menubar to another screen and make the depths
		of the new and old screens different.  I'm removing it until I can figure
		out how to get around this problem.
	*/
//	if (gInsideMenuSelect)
//		flags |= useDistantHdwrMem;
		
	return PatchNewGWorld::PatchFn(offscreenGWorld, PixelDepth, boundsRect, cTable, aGDevice, flags);
}

long  MenuSelectPatch(Point startPt)
{
	gInsideMenuSelect = true;
	
	long result = PatchMenuSelect::PatchFn(startPt);
	
	gInsideMenuSelect = false;
	
	return(result);
}


#pragma mark -
#pragma mark ¥ÊDebugging App
/********************************************************************************
	ApplicationMain
		
	
*/
void ApplicationMain()
{
	InitGraf(&qd.thePort);
	InitMenus();
	InitFonts();
	InitWindows();

//Debugger();
	
	void (*hrmUnloadAccel)() = (void  (*)()) hrmGetExtension("hrmUnloadAccel");
	hrmUnloadAccel();
	
	InstallAcceleration();
	
#if __profile__
	ProfilerInit(collectDetailed, bestTimeBase, 200, 20);
#endif	
	// 
	SetMenuBar(GetNewMBar(128));
	
	EventRecord ev;
	while (1)
	{
		if (WaitNextEvent(everyEvent, &ev, 60, 0))
		{
			if (ev.what == mouseDown)
			{
				WindowPtr	windowPtr;
				short where = FindWindow(ev.where, &windowPtr);
				if (where == inMenuBar)
				{
					if (MenuSelect(ev.where) == ((129L << 16) | 1))
						break;
					HiliteMenu(0);
				}
			}
			
			if (ev.what == keyDown)
			{
				if (((ev.message & 0xFF) == 'q') && (ev.modifiers & cmdKey))
					break;
			}
		}
	}
	
#if __profile__
	ProfilerDump("\p3DFX Debug App Profile");
	ProfilerTerm();
#endif
}




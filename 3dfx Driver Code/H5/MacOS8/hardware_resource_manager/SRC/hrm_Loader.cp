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
** $Header: /devel/h3/MacOS8/H3_2D_Acceleration/src/system_extension.c 2     7/02/99 3:32p Kcd $
** $Log: /devel/h3/MacOS8/H3_2D_Acceleration/src/system_extension.c $
** 
** 2     7/02/99 3:32p Kcd
** Added icon display at startup.
**
*/

// Includes
#include <Memory.h>
#include <Resources.h>
#include <CodeFragments.h>
#include <Types.h>
#include <Start.h>
#include <MixedMode.h>

#include "hrm_Loader.h"

// Types
typedef struct ConnectionIDTable
{
	CFragConnectionID	connID;
	short				resID;
	Handle				handle;
	Str255				resName;
	Boolean				valid;
} ConnectionIDTable;

// Globals
FSSpec					gInitFile;
UInt32					gNumFragments;
ConnectionIDTable		gConnIDTable[20];
Handle					gLoaderTempHandle = 0;

// Function Declarations
extern "C" void   	LoaderEntry();
void				InstallAccel(Boolean openInitFile);
void				UnloadAccel();
void 				ExtensionsDoneProc(UInt32 messsage, void *unused, ExtensionElementPtr element);

// Routine Descriptors

RoutineDescriptor ExtensionsDoneProcRD = 
		BUILD_ROUTINE_DESCRIPTOR(uppExtensionNotificationProcInfo, ExtensionsDoneProc);

/********************************************************************************
	hrmLoadAccel
		Boolean	openInitFile
	
*/
UInt32 hrmLoadAccel(Boolean	openInitFile)
{
	Handle	acclResHandle;
	short	resID;
	ResType	resType;

//	DebugStr("\phrmLoadAccel");

	// Load the accel resources into the system zone
	THz theCurrentZone = GetZone();
	SetZone(SystemZone ());

	// Load in all the accl resources
	for (UInt32	resIndex = 1; acclResHandle = Get1IndResource('accl', resIndex); ++resIndex)
	{
		// Get some info on the resource
		HLock(acclResHandle);
		GetResInfo(acclResHandle, &resID, &resType, gConnIDTable[gNumFragments].resName);
		DetachResource(acclResHandle);
		
		// Set up a connection table entry for this resource
		gConnIDTable[gNumFragments].connID = 0;
		gConnIDTable[gNumFragments].resID = resID;
		gConnIDTable[gNumFragments].handle = acclResHandle;
		gConnIDTable[gNumFragments].valid = true;
		++gNumFragments;
	}

	if (gNumFragments > 0)
	{
		// Set up our delayed proc to load all our accel resources
		InstallExtensionNotificationProc(&ExtensionsDoneProcRD);
		
		// Due to the way the System heap zone grows for extensions, we may end up not having enough
		// memory available at accel load time (after extensions have finished loading). This block
		// is held between init time and extensions done time, to 'force' the system to not reallocate
		// the memory we asked for in the 'sysz' resource to other inits.
		gLoaderTempHandle = NewHandleSys(512 * 1024);
	}

	SetZone(theCurrentZone);
	return(gNumFragments > 0);
}

/********************************************************************************
	InstallAccel
		Boolean openInitFile
	
*/
void	InstallAccel(Boolean openInitFile)
{
	short	initFileRef = 0;
	Boolean	(*mainAddr)();
	Str255	fragError;
	CFragConnectionID connID;
	
//	DebugStr("\pInstallAccel");

	THz theCurrentZone = GetZone();
	SetZone(SystemZone ());

	// Deallocate the loader temp handle
	DisposeHandle(gLoaderTempHandle);

	// Open our resource file
	if (openInitFile)
	{
		initFileRef = FSpOpenResFile(&gInitFile, fsRdPerm);
		UseResFile(initFileRef);
	}
	
	// For each accl resource we loaded at INIT time...
	for (UInt32	tableIndex = 0; tableIndex < gNumFragments; ++tableIndex)
	{		
		// Just continue if this resource is already installed 
		if (gConnIDTable[tableIndex].connID)
			continue;
			
		Boolean	accelRsrcInstalled = false;
			
		// Load the fragment 
		if (!GetMemFragment((void *) *gConnIDTable[tableIndex].handle, 
				GetHandleSize(gConnIDTable[tableIndex].handle), 
				gConnIDTable[tableIndex].resName,
				kPrivateCFragCopy, &connID, (char **) &mainAddr, fragError))
		{
			// Call the fragment's main address
			if (mainAddr())
			{
				gConnIDTable[tableIndex].connID = connID;
				accelRsrcInstalled = true;
			} else
			{
				CloseConnection(&connID);
				gConnIDTable[tableIndex].connID = 0;
			}
		}
		
		if (!accelRsrcInstalled)
		{
			DisposeHandle(gConnIDTable[tableIndex].handle);
			gConnIDTable[tableIndex].handle = 0;
		}
			
	}
	
	// Close the resource file
	if (initFileRef)
		CloseResFile(initFileRef);
	SetZone(theCurrentZone);
}

/********************************************************************************
	hrmUnloadAccel
		
	
*/
void hrmUnloadAccel()
{
	THz theCurrentZone = GetZone();
	SetZone(SystemZone ());

	for (UInt32 index = 0; index < gNumFragments; ++index)
	{
		if (gConnIDTable[index].connID)
			CloseConnection(&gConnIDTable[index].connID);
		gConnIDTable[index].connID = 0;
	}

	SetZone(theCurrentZone);
}


/********************************************************************************
	ExtensionsDoneProc
		UInt32 messsage
		void *unused
		ExtensionElementPtr element
		
	
*/
void ExtensionsDoneProc(UInt32 message, void *unused, ExtensionElementPtr element)
{
#pragma unused (unused, element)
	if (message != extNotificationAfterLast)
		return;
		
	InstallAccel(false);
}

#pragma mark -
#pragma mark ¥ Scratch
#if 0



/********************************************************************************
	LoaderEntry
		
	
*/
void LoaderEntry()
{
	long 	response;
	UInt32	version = 0x01000000;
	UInt32	productID = 0x0;
	UInt32	initIconID = kFailureIcon;
	OSErr	err;

	THz theCurrentZone = GetZone();
	SetZone(SystemZone ());

	// Make sure the computer supports the minimum stuff we need to survive
	if (Gestalt(gestaltNameRegistryVersion, &response))
		return;
		
	// Get our product and version ID resources
	VersRecHndl versionRes = (VersRecHndl) Get1Resource('vers', 1);
	if (versionRes && *versionRes)
		version = *(UInt32 *) &versionRes[0][0].numericVersion;
	UInt32	**productIDRes = (UInt32 **) Get1Resource('prod', 128);
	if (productIDRes && *productIDRes)
		productID = **productIDRes;
		
	
	// Save the location of our init file
	FCBPBRec	pb;
	pb.ioRefNum = CurResFile();
	pb.ioNamePtr = gInitFile.name;
	pb.ioFCBIndx = 0;
	pb.ioCompletion = 0;
	if (PBGetFCBInfoSync(&pb))
		goto error;
	gInitFile.vRefNum = pb.ioFCBVRefNum;
	gInitFile.parID = pb.ioFCBParID;
	
	// Set up our delayed proc to load all our accel resources
	if (InstallExtensionNotificationProc(&ExtensionsDoneProcRD))
		goto error;

	// Detach ourselves so the loader sticks around
	Handle thisResourceHand = (Handle) Get1Resource('INIT', 128);
	if (thisResourceHand)
		DetachResource(thisResourceHand);

	// Set up our loader control table
	gLoaderControl.tableVersion = kLoaderControlTableVersion;
	gLoaderControl.productID = productID;
	gLoaderControl.productVersion = version;
	gLoaderControl.next = 0;
	gLoaderControl.loadAccel = &AttemptLoadAccel;
	gLoaderControl.unloadAccel = &UnloadAccel;
	
	// Check for the existance of an already-installed loader with our version #
	// and product ID
	if (!Gestalt(k3DFXGetLoaderTable, &response))
	{
		LoaderControlTable *table = (LoaderControlTable *) response;
		
		// Step through the list until we get to the end
		while (table->next != 0)
			table = table->next;
			
		// Add ourselves here
		table->next = &gLoaderControl;
	} else
	{
		// No table. Make a table, and add ourselves to it.
		// Note that we can still live if this fails, but nobody can find us
		NewGestaltValue(k3DFXGetLoaderTable, (long) &gLoaderControl);
	}
	
	
error:
	// Check for existance of a display, and the existance of our icon resource
	DisplayIDType displayID;
	GDHandle display = DMGetFirstScreenDevice(true);
	err = DMGetDisplayIDByGDevice(display, &displayID, false);
	if (!err && (displayID != kDummyDeviceID) && Get1Resource('icl8', initIconID))
		ShowInitIcon(initIconID, true);

	SetZone(theCurrentZone);
}


#endif
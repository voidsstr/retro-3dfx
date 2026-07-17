/******************************************************************************
	File:		
	Copyright:	© 1995-2000 Critical Path Software Inc.  All Rights Reserved.
	Author:		Andrew Mellinger
	Purpose:
	History:	
 ******************************************************************************/
	
	/* Headers */
/******************************************************************************/
#include "hrm_prefs.h"

#include <Errors.h>
#include <Resources.h>
#include <Folders.h>
#include <Script.h>
#include "DCON.h"


	/* Constants */
/******************************************************************************/
#define	kNotificationElemCount		16
#define kPrefsFileName				"\p3dfx prefs"

#define kPrefsResType				'pref'
typedef enum
{
	kResID2D					= 130,
	kResID3D,
	kResIDQT
} PrefsIDs;

	/* Typedefs */
/******************************************************************************/
typedef struct
{
	PrefsChangedFn	fn;
	UInt32			prefsFlags;
} ChangedFnElement;

typedef void (*PrefsUpdateFn)(void *old);

	/* Globals */
	/* File Globals */
/******************************************************************************/
HRMPrefsTable						gPrefsTable;
HRMPrefs2D							gPrefs2D[kMaxNumberOfBoards];
HRMPrefs3D							gPrefs3D[kMaxNumberOfBoards];
HRMPrefsQT							gPrefsQT[kMaxNumberOfBoards];

ChangedFnElement					gNotifyList[kNotificationElemCount];
SInt16								gRefNum;

	/* Prototypes */
/******************************************************************************/
static bool LoadPrefs(UInt32 prefsID, PrefsUpdateFn updateFn);
static void SavePrefs(void *newPrefs, UInt32 prefsSize, UInt32 prefsID);

static void InitPrefs2D();
static void InitPrefs3D();
static void InitPrefsQT();

static void UpdatePrefs2D(void *old);
static void UpdatePrefs3D(void *old);
static void UpdatePrefsQT(void *old);

static OSStatus	OpenPrefsFile();
static void ClosePrefsFile();

	/* Methods */
	/* Functions */
/******************************************************************************/
OSStatus hrm_InitPrefs()
{
	gRefNum = -1;

	// Set up the pointers in our preference table.
	for (UInt32 i = 0; i < kMaxNumberOfBoards; ++i)
	{
		gPrefsTable.m2D[i]		= &gPrefs2D[i];
		gPrefsTable.m3D[i]		= &gPrefs3D[i];
		gPrefsTable.mQT[i]		= &gPrefsQT[i];
	}

	for (UInt32 i = 0; i < kNotificationElemCount; ++i)
	{
		gNotifyList[i].fn			= 0;
		gNotifyList[i].prefsFlags	= 0;
	}

	// Init to defaults;
	InitPrefs2D();
	InitPrefs3D();
	InitPrefsQT();
	
	// Open the prefs file.
	OpenPrefsFile();
	
	// Load each set of prefs based on version.
	LoadPrefs(kResID2D, UpdatePrefs2D);
	LoadPrefs(kResID3D, UpdatePrefs3D);
	LoadPrefs(kResIDQT, UpdatePrefsQT);

	// Close the prefs file
	ClosePrefsFile();

	return noErr;
}

/*----------------------------------------------------------------------------*\
	==> GetPrefsTable <==
	
	This routine returns a global pointer to a table of pointers to different
	memory structures.
\*----------------------------------------------------------------------------*/
HRMPrefsTable *hrm_GetPrefsTable()
{
	return &gPrefsTable;
}

/*----------------------------------------------------------------------------*\
	==> RegisterPrefsChanged <==
	
	Clients can register a callback routine to be called when the preferences
	are changed by a client, like the UI.
\*----------------------------------------------------------------------------*/
OSStatus hrm_RegisterPrefsChanged(PrefsChangedFn fn, UInt32 flags)
{
	UInt32	i;

	dprintf( "hrm_RegisterPrefsChanged(): fn 0x%08x, flags = 0x%08x\n", fn, flags );
	if (fn == 0 || flags == 0)
		return paramErr;

	for (i = 0; i < kNotificationElemCount; ++i)
	{
		if (gNotifyList[i].fn == 0)
		{
			gNotifyList[i].fn			= fn;
			gNotifyList[i].prefsFlags	= flags;
			return noErr;
		}
	}

	// If we got here, we didn't have any free elements.
	return memFullErr;
}

/*----------------------------------------------------------------------------*\
	==> UnregisterPrefsChanged <==
	
	In case you don't want to get notified when prefs changed.
\*----------------------------------------------------------------------------*/
OSStatus hrm_UnregisterPrefsChanged(PrefsChangedFn fn)
{
	UInt32	i;

	for (i = 0; i < kNotificationElemCount; ++i)
	{
		if (gNotifyList[i].fn == fn)
		{
			gNotifyList[i].fn			= 0;
			gNotifyList[i].prefsFlags	= 0;
			return noErr;
		}
	}

	// If we got here, we couldn't find the element to remove.
	return qErr;
}

/*----------------------------------------------------------------------------*\
	==> ChangePrefs <==
	
	This call is to be used by the Control Panel or other UI to set which
	options/abilities the driver will use.
	
	Usage:  Create a Prefs Table with pointers to allocated structures
	that you wish to change.  Set any pointer to zero in which you do not
	wish to change.  Sumbit this table and flags indicating which structures
	were updated to the prefs manager.
	
	The prefs manager will call every registered PrefsChanged function that 
	wanted events on these kind of preference changes.
	
	Any older version of struct can be submitted to the prefs manager and 
	will be updated to the latest structure.
\*----------------------------------------------------------------------------*/
OSStatus hrm_ChangePrefs(HRMPrefsTable *newPrefsTable, UInt32 flags)
{
	dprintf( "hrm_ChangePrefs(): prefsTable @ 0x%08x, flags = 0x%08x\n", newPrefsTable, flags );
	if (newPrefsTable == 0)
		return paramErr;

	// Open the preferences file, in case we need to make changes
	OpenPrefsFile();
	
	// Push the data into the prefs.
	if (flags & kPrefsFlag2D && newPrefsTable->m2D != 0)
	{
dprintf( "hrm_InitPrefs(): gPrefsTable = 0x%08x, gPrefsTable.m2D = 0x%08x, gPrefsTable.m2D = 0x%08x\n",newPrefsTable , newPrefsTable->m2D , newPrefsTable->m2D[0]);
		UpdatePrefs2D(newPrefsTable->m2D[0]);
		SavePrefs(newPrefsTable->m2D[0], sizeof(HRMPrefs2D) * kMaxNumberOfBoards, kResID2D);
	}
	
	if (flags & kPrefsFlag3D && newPrefsTable->m3D != 0)
	{
		UpdatePrefs3D(newPrefsTable->m3D[0]);
		SavePrefs(newPrefsTable->m3D[0], sizeof(HRMPrefs3D) * kMaxNumberOfBoards, kResID3D);
	}

	if (flags & kPrefsFlagQT && newPrefsTable->mQT != 0)
	{
		UpdatePrefsQT(newPrefsTable->mQT[0]);
		SavePrefs(newPrefsTable->mQT[0], sizeof(HRMPrefsQT) * kMaxNumberOfBoards, kResIDQT);
	}

	// Open the preferences file, in case we need to make changes
	ClosePrefsFile();

	dprintf( "hrm_ChangePrefs(): notify registered clients...\n" );
	// Call each function that wants to know about these events.
	for (UInt32 i = 0; i < kNotificationElemCount; ++i)
	{
		if (gNotifyList[i].fn && (gNotifyList[i].prefsFlags & flags) != 0)
		{
			dprintf( "hrm_ChangePrefs(): notifying...\n" );
			gNotifyList[i].fn();
		}
	}

	dprintf( "hrm_ChangePrefs(): done...\n" );
	return noErr;
}

#pragma mark -
/*----------------------------------------------------------------------------*\
	==> LoadPrefs <==
	
	This routine loads the appropriate resource into memory.  If the version
	number isn't current it calls the update routine.  If the version number
	is current, then it simply block moves the data in.
\*----------------------------------------------------------------------------*/
static bool LoadPrefs(UInt32 prefsID, PrefsUpdateFn updateFn)
{
	Handle		resHndl;

	resHndl = Get1Resource(kPrefsResType, prefsID);
	if (resHndl != 0)
	{
		// Check to see if we need to be updated
		HLock(resHndl);
		updateFn((void *)*resHndl);
		HUnlock(resHndl);

		// Since we found some prefernces, copy them over.
		return true;
	}
	
	// We didn't find any ones on disk.
	return false;
}


/*----------------------------------------------------------------------------*\
	==> SavePrefs <==
	
	This routine saves the appropriate structure out to disk.  It overwrites
	whatever used to be there.
\*----------------------------------------------------------------------------*/
static void SavePrefs(void *newPrefs, UInt32 prefsSize, UInt32 prefsID)
{
	Handle		resHndl;

	dprintf( "SavePrefs(): newPrefs @ 0x%08x, size = %d, id = %d\n", newPrefs, prefsSize, prefsID);

	resHndl = Get1Resource(kPrefsResType, prefsID);
	if (resHndl != 0)
	{
		// Check the version for sanity sake.  Should be the same.
		if (GetHandleSize(resHndl) != prefsSize)
		{
			// Set the size, and we'll overwrite
			dprintf( "SavePrefs(): data handle has been grown to size = \n", prefsSize);
			SetHandleSize(resHndl, prefsSize);
		}

		// Save off our new values into our updated block.
		dprintf( "SavePrefs(): copy data ... \n");
		BlockMoveData(newPrefs, *resHndl, prefsSize);
		ChangedResource( resHndl );
	}
	else
	{
		// Create a new handle and add it.
		dprintf( "SavePrefs(): creating new data resource ... \n");
		resHndl = NewHandleSys(prefsSize);
		if (resHndl != 0)
		{
			BlockMoveData(newPrefs, *resHndl, prefsSize);
			AddResource(resHndl, kPrefsResType, prefsID, 0);
			if (ResError() != noErr)
			{
				dprintf( "SavePrefs(): ### ERROR ### new data resource failed!!! \n");
				// Do something.
			}
		}
	}
}

#pragma mark -
/*----------------------------------------------------------------------------*/
static void InitPrefs2D()
{
	for (UInt32 i = 0; i < kMaxNumberOfBoards; ++i)
	{
		gPrefs2D[i].header.version = kHRMPrefsVersionCurrent;
		gPrefs2D[i].header.size = sizeof(gPrefs2D);
		gPrefs2D[i].disableFlags = 0;
	}
}

/*----------------------------------------------------------------------------*/
static void InitPrefs3D()
{
	for (UInt32 i = 0; i < kMaxNumberOfBoards; ++i)
	{
		gPrefs3D[i].header.version = kHRMPrefsVersionCurrent;
		gPrefs3D[i].header.size = sizeof(gPrefs3D);
	}
}

/*----------------------------------------------------------------------------*/
static void InitPrefsQT()
{
	for (UInt32 i = 0; i < kMaxNumberOfBoards; ++i)
	{
		gPrefsQT[i].header.version = kHRMPrefsVersionCurrent;
		gPrefsQT[i].header.size = sizeof(gPrefsQT);
	}
}

#pragma mark -
/*----------------------------------------------------------------------------*/
static void UpdatePrefs2D(void *old)
{
	HRMPrefs2D *oldPrefs = (HRMPrefs2D *)old;

	dprintf( "UpdatePrefs2D(): ...\n" );
	// If they are the latest version, then no update needed
	switch(oldPrefs->header.version)
	{
		case kHRMPrefsVersionCurrent:
			BlockMoveData(oldPrefs, &gPrefs2D, sizeof(HRMPrefs2D) * kMaxNumberOfBoards);
		break;
		
		default:
			dprintf( "UpdatePrefs2D(): ### ERROR ###, unknown 2d prefs format\n" );
		break;
	}
}

/*----------------------------------------------------------------------------*/
static void UpdatePrefs3D(void *old)
{
	HRMPrefs3D *oldPrefs = (HRMPrefs3D *)old;

	dprintf( "UpdatePrefs3D(): ...\n" );
	// If they are the latest version, then no update needed
	switch(oldPrefs->header.version)
	{
		case kHRMPrefsVersionCurrent:
			BlockMoveData(oldPrefs, &gPrefs3D, sizeof(HRMPrefs3D) * kMaxNumberOfBoards);
		break;
		
		default:
			dprintf( "UpdatePrefs3D(): ### ERROR ###, unknown 3d prefs format\n" );
		break;
	}
}

/*----------------------------------------------------------------------------*/
static void UpdatePrefsQT(void *old)
{
	HRMPrefsQT *oldPrefs = (HRMPrefsQT *)old;

	dprintf( "UpdatePrefsQT(): ...\n" );
	// If they are the latest version, then no update needed
	switch(oldPrefs->header.version)
	{
		case kHRMPrefsVersionCurrent:
			BlockMoveData(oldPrefs, &gPrefsQT, sizeof(HRMPrefsQT) * kMaxNumberOfBoards);
		break;

		default:
			dprintf( "UpdatePrefsQT(): ### ERROR ###, unknown QT prefs format\n" );
		break;
	}
}

#pragma mark -
/*----------------------------------------------------------------------------*/
static OSStatus OpenPrefsFile()
{
	OSErr				err = noErr;
	SInt16				prefsVol = -1;
	SInt32				prefsDir;
	FSSpec				spec;

	// Get the location of the preferernces folder
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, 
		kCreateFolder, &prefsVol, &prefsDir);
	if (err != noErr)
		return err;

	// Make a spec to it so we can open the thing.
	err = FSMakeFSSpec(prefsVol, prefsDir, kPrefsFileName, &spec);
	if (err == fnfErr)
	{
		FSpCreateResFile(&spec, '3DFX', 'pref', smSystemScript);	
	}
	else if (err != noErr)
	{
		return err;
	}

	gRefNum = FSpOpenResFile(&spec, fsRdWrPerm);
	if(gRefNum == -1)
		return ResError();
	
	return(err);
}

/*----------------------------------------------------------------------------*/
static void ClosePrefsFile()
{
	if(gRefNum != -1)
	{
		UpdateResFile(gRefNum);
		CloseResFile(gRefNum);
		gRefNum = -1;
	}
}

/******************************************************************************/
/*----------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

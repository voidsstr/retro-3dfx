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
**
*/

#include "URequestVideo.h"

// Internal includes
#include <Dialogs.h>
#include <ROMDefs.h>
#include <Devices.h>
#include <Errors.h>
#include <Gestalt.h>
#include <Memory.h>
#include <Palettes.h>
#include <Displays.h>
#include <ConditionalMacros.h>	//DL
#include <CodeFragments.h>	//DL

#define abs(x) ( ( (x) < 0 ) ? -(x) : (x) )

//--------------------------------------------------------------
//
// Internal defines, structs, typedefs, and routine declarations
//
//--------------------------------------------------------------
#define		KMonoDev			0						// false (handy definitions for gdDevType settings)
#define		kColorDev			1						// true
#define		char_Enter			0x03					// for our filter proc
#define		char_Return			0x0D					//
#define		iRevertItem			1						// User buttons
#define		iConfirmItem		2						//
#define		kSecondsToConfirm	8						// seconds before confirm dialog is taken down
#define		rConfirmSwtchAlrt	2735					// ID of alert dialog

struct DepthInfo
{
	VDSwitchInfoRec			depthSwitchInfo;			// This is the switch mode to choose this timing/depth
	VPBlock					depthVPBlock;				// VPBlock (including size, depth and format)
};
typedef struct DepthInfo DepthInfo;

struct ListIteratorDataRec
{
	unsigned long			displayModeFlags;			// 
	VDSwitchInfoRec			displayModeSwitchInfo;		//
	VDResolutionInfoRec		displayModeResolutionInfo;	//
	VDTimingInfoRec			displayModeTimingInfo;		// Contains timing flags and such
	unsigned long			depthBlockCount;			// How many depths available for a particular timing
	DepthInfo				*depthBlocks;				// Array of DepthInfo
	Str255					displayModeName;			// name of the timing mode
};
typedef struct ListIteratorDataRec ListIteratorDataRec;


//--------------------------------------------------------------
//
// Implementation of sample code
//
//--------------------------------------------------------------
OSErr RVSetVideoRequest (VideoRequestRecPtr requestRecPtr)
{
	Boolean			displayMgrPresent;
	unsigned long	displayMgrVersion;
	OSErr			err = paramErr;
	Boolean			isColor;
	long			value = 0;

	Gestalt(gestaltDisplayMgrVers, (long*)&displayMgrVersion);
	Gestalt(gestaltDisplayMgrAttr,&value);
	displayMgrPresent=value&(1<<gestaltDisplayMgrPresent);
	if (displayMgrPresent)
	{
		if (requestRecPtr->displayMode && requestRecPtr->depthMode)
		{
			if (requestRecPtr->availBitDepth == 1)	// Based on avail bit depth, 
				isColor = KMonoDev;					// set the device to a mono device, or
			else isColor = kColorDev;				// set the device to a color device
			SetDeviceAttribute(requestRecPtr->screenDevice,gdDevType,isColor);		
			
			if (displayMgrVersion >= 0x00020000)
			{
				// only call DMSetDisplayMode if we have DM2.0 is installed
				err = DMSetDisplayMode(	requestRecPtr->screenDevice,	// GDevice
						requestRecPtr->displayMode,						// DM1.0 uses this
						&requestRecPtr->depthMode,						// DM1.0 uses this
						(unsigned long) &(requestRecPtr->switchInfo),	// DM2.0 uses this rather than displayMode/depthMode combo
						nil);
				if (kDMDriverNotDisplayMgrAwareErr == err)
				{
					// DM not supported by driver, so all we can do is set the bit depth
					err = SetDepth (requestRecPtr->screenDevice, requestRecPtr->depthMode, gdDevType, isColor);
				}
			}
		}
	}
	return (err);
}

// This extern should be removed once this function is formally defined in Displays.h
extern pascal OSErr DMUseScreenPrefs(Boolean usePrefs, Handle displayState)
 THREEWORDINLINE(0x303C, 0x03EC, 0xABEB);

OSErr RVSetVideoAsScreenPrefs (void)
{
	Handle		displaystate;
	Boolean		displayMgrPresent;
	long		value = 0;
	OSErr		theErr = -1;		// some generic error

	Gestalt(gestaltDisplayMgrAttr,&value);
	displayMgrPresent=value&(1<<gestaltDisplayMgrPresent);
	if (displayMgrPresent)
	{
		DMBeginConfigureDisplays (&displaystate);	// Tell the world it is about to change
		DMUseScreenPrefs (true, displaystate);		// Make the change
		DMEndConfigureDisplays (displaystate);		// Tell the world the change is over
		
		theErr = noErr;	// we (maybe) set the world back to a known setting
	}
	return (theErr);	
}

OSErr RVGetCurrentVideoSetting (VideoRequestRecPtr requestRecPtr)
{
	unsigned long		displayMgrVersion;
	OSErr				error = paramErr;
	VDSwitchInfoRec		switchInfo;

	requestRecPtr->availBitDepth			= 0;	// init to default - you can do it if it is important to you
	requestRecPtr->availHorizontal			= 0;
	requestRecPtr->availVertical			= 0;
	requestRecPtr->availFlags				= 0;
	requestRecPtr->displayMode				= -1; 
	requestRecPtr->depthMode				= -1;
	requestRecPtr->switchInfo.csMode		= 0;
	requestRecPtr->switchInfo.csData		= 0;
	requestRecPtr->switchInfo.csPage		= 0;
	requestRecPtr->switchInfo.csBaseAddr	= 0;
	requestRecPtr->switchInfo.csReserved	= 0;
	
	Gestalt(gestaltDisplayMgrVers, (long*)&displayMgrVersion);
	if (requestRecPtr->screenDevice)
	{
//DL -	must also make sure that Display Library is installed, otherwise
//		calling DMGetDisplayMode() will branch to nil.
//DL		if (displayMgrVersion >= 0x00020000)
#if GENERATINGCFM
		if (displayMgrVersion >= 0x00020000 && (Ptr) DMGetDisplayMode != (Ptr) kUnresolvedCFragSymbolAddress)
#else
		if (displayMgrVersion >= 0x00020000)
#endif
		{	// get the info the DM 2.0 way
			error = DMGetDisplayMode(requestRecPtr->screenDevice, &switchInfo);
			if (noErr == error)
			{
				requestRecPtr->depthMode			= switchInfo.csMode;
				requestRecPtr->displayMode			= switchInfo.csData; 
				requestRecPtr->switchInfo.csMode	= switchInfo.csMode;
				requestRecPtr->switchInfo.csData	= switchInfo.csData;
			}
		}
	}
	return (error);
}

pascal Boolean ConfirmAlertFilter(DialogRef theDialog, EventRecord *theEvent, short *itemHit)
{
	char charCode;
	Boolean enterORreturn;
	Boolean returnValue = false;
	
	WindowRef dialogWindow = GetDialogWindow(theDialog);	//DL

	if (0 == GetWRefCon(dialogWindow))
		SetWRefCon (dialogWindow,TickCount());
	else
	{
		if (GetWRefCon(dialogWindow) + kSecondsToConfirm * 60 < TickCount())
		{
			returnValue = true;
			theEvent->what = nullEvent;
			*itemHit = 1;
		}
		else
		{
			if (theEvent->what == keyDown)
			{
				charCode = (char)theEvent->message & charCodeMask;
				enterORreturn = (charCode == (char)char_Return) || (charCode == (char)char_Enter);
				if (enterORreturn)
				{
					theEvent->what = nullEvent;
					returnValue = true;
					*itemHit = iRevertItem;
					if (enterORreturn && (0 != (theEvent->modifiers & optionKey)))
					{
						*itemHit = iConfirmItem;
					}
				}
			}
		}
	}
	return (returnValue);
}

OSErr RVConfirmVideoRequest (VideoRequestRecPtr requestRecPtr)
{
	short			alertReturn;		// Alert() return value
	ModalFilterUPP	confirmFilterUPP;	// got to have us one of them new fangled UPP thingies
	OSErr			theErr = noErr;
	
	if (requestRecPtr->availFlags & 1<<kModeValidNotSafeBit)
	{	// new mode is valid but not safe, so ask user to confirm
		SetCursor(&qd.arrow);										// have to show the arrow

		confirmFilterUPP = NewModalFilterProc (ConfirmAlertFilter);	// create a new modal filter proc UPP
		alertReturn = Alert(rConfirmSwtchAlrt, confirmFilterUPP);	// alert the user
		DisposeRoutineDescriptor (confirmFilterUPP);				// of course there is no DisposeModalFilterProc...
		
		if (alertReturn != iConfirmItem)
			theErr = -1;							// tell the caller to switch back to a known setting
	}
	return (theErr);									// the mode was safe, so do nothing
}


OSErr RVRequestVideoSetting (VideoRequestRecPtr requestRecPtr)
{
	Boolean							displayMgrPresent;
	short							iCount = 0;					// just a counter of GDevices we have seen
	DMDisplayModeListIteratorUPP	myModeIteratorProc = nil;	// for DM2.0 searches
	Boolean							suppliedGDevice;	
	DisplayIDType					theDisplayID;				// for DM2.0 searches
	DMListIndexType					theDisplayModeCount;		// for DM2.0 searches
	DMListType						theDisplayModeList;			// for DM2.0 searches
	long							value = 0;
	GDHandle						walkDevice = nil;			// for everybody
	OSErr							gestaltErr;
	OSErr							returnErr = -1;				// set to some generic error
	unsigned long					displayMgrVersion;
	Boolean 						hasDM2 = false;
	
	gestaltErr = Gestalt(gestaltDisplayMgrVers, (long*)&displayMgrVersion);
	if (	gestaltErr == noErr &&
#if GENERATINGCFM
			(Ptr) DMNewDisplayModeList != (Ptr) kUnresolvedCFragSymbolAddress &&
			(Ptr) DMDisposeList != (Ptr) kUnresolvedCFragSymbolAddress &&
#endif
			displayMgrVersion >= 0x00020000	)
	{
		hasDM2 = true;				// Check for the presence of DM 2.0 before calling DMNewDisplayModeList() and DMDisposeList()
	}

	Gestalt(gestaltDisplayMgrAttr,&value);
	displayMgrPresent=value&(1<<gestaltDisplayMgrPresent);
	displayMgrPresent=displayMgrPresent && hasDM2;
	
	if (displayMgrPresent)		// only jump in if we have DM 2.0
	{
		// init the needed data before we start
		if (requestRecPtr->screenDevice)							// user wants a specifc device?
		{
			walkDevice = requestRecPtr->screenDevice;
			suppliedGDevice = true;
		}
		else
		{
			walkDevice = DMGetFirstScreenDevice (dmOnlyActiveDisplays);			// for everybody
			suppliedGDevice = false;
		}
		
		myModeIteratorProc = NewDMDisplayModeListIteratorProc(ModeListIterator);	// for DM2.0 searches
	
		// Note that we are hosed if somebody changes the gdevice list behind our backs while we are iterating....
		// ...now do the loop if we can start
		if( walkDevice && myModeIteratorProc)
		{
			do // start the search
			{
				iCount++;		// GDevice we are looking at (just a counter)
				if( noErr == DMGetDisplayIDByGDevice( walkDevice, &theDisplayID, false ) )	// DM1.0 does not need this, but it fits in the loop
				{
					theDisplayModeCount = 0;	// for DM2.0 searches
					if (noErr == DMNewDisplayModeList(theDisplayID, 0, 0, &theDisplayModeCount, &theDisplayModeList) )
					{
						// search NuBus & PCI the new kool way through Display Manager 2.0
						GetRequestTheDM2Way (requestRecPtr, walkDevice, myModeIteratorProc, theDisplayModeCount, &theDisplayModeList);
						DMDisposeList(theDisplayModeList);	// now toss the lists for this gdevice and go on to the next one
					}
				}
			} while ( !suppliedGDevice && nil != (walkDevice = DMGetNextScreenDevice ( walkDevice, dmOnlyActiveDisplays )) );	// go until no more gdevices
		}
		
		if( myModeIteratorProc )
			DisposeRoutineDescriptor(myModeIteratorProc);
		
		//¥¥¥ WE MAY NOT HAVE FOUND A REQUEST FROM GETREQUESTTHEDM2WAY() in whicxh case the requestRec.displayMode & requestRec.depthMode fields will be nil
		returnErr = noErr;	// we were able to try to look for a match
	}
	return (returnErr);
}

pascal void ModeListIterator(void *userData, DMListIndexType, DMDisplayModeListEntryPtr displaymodeInfo)
{
	unsigned long			depthCount;
	short					iCount;
	ListIteratorDataRec		*myIterateData		= (ListIteratorDataRec*) userData;
	DepthInfo				*myDepthInfo;
	
	long					length = 0;
	
	// set display data in a round about way
	// Set the basics
	myIterateData->displayModeFlags				= displaymodeInfo->displayModeFlags;		// for mode flags
	myIterateData->displayModeSwitchInfo		= *displaymodeInfo->displayModeSwitchInfo;	// not needed - depth info has this per depth
	myIterateData->displayModeResolutionInfo	= *(VDResolutionInfoRec *)displaymodeInfo->displayModeResolutionInfo;	// refresh rate, pixels/lines at max depth 
		
	//myIterateData->displayModeResolutionInfo.csRefreshRate = GlobalRefreshRate;

	myIterateData->displayModeTimingInfo		= *displaymodeInfo->displayModeTimingInfo;	// for iming flags
	
	BlockMoveData ((StringPtr)displaymodeInfo->displayModeName,(StringPtr)&myIterateData->displayModeName, ((StringPtr)displaymodeInfo->displayModeName)[0]+1);	// the name of the mode
	
	// now get the DMDepthInfo into memory we own
	depthCount = displaymodeInfo->displayModeDepthBlockInfo->depthBlockCount;
	myDepthInfo = (DepthInfo*)NewPtrClear(depthCount * sizeof(DepthInfo));

	// set the info for the caller
	myIterateData->depthBlockCount = depthCount;
	myIterateData->depthBlocks = myDepthInfo;

	// and fill out all the entries
	if (depthCount) for (iCount=0; iCount < depthCount; iCount++)
	{
		myDepthInfo[iCount].depthSwitchInfo = 
			*displaymodeInfo->displayModeDepthBlockInfo->depthVPBlock[iCount].depthSwitchInfo;
		myDepthInfo[iCount].depthVPBlock = 
			*displaymodeInfo->displayModeDepthBlockInfo->depthVPBlock[iCount].depthVPBlock;
	}
}

void GetRequestTheDM2Way (	VideoRequestRecPtr requestRecPtr,
							GDHandle walkDevice,
							DMDisplayModeListIteratorUPP myModeIteratorProc,
							DMListIndexType theDisplayModeCount,
							DMListType *theDisplayModeList)
{
	short					jCount;
	short					kCount;
	ListIteratorDataRec		searchData;

	searchData.depthBlocks = nil;
	// get the mode lists for this GDevice
	for (jCount=0; jCount<theDisplayModeCount; jCount++)		// get info on all the resolution timings
	{
		DMGetIndexedDisplayModeFromList(*theDisplayModeList, jCount, 0, myModeIteratorProc, &searchData);
		
		// for all the depths for this resolution timing (mode)...
		if (searchData.depthBlockCount) for (kCount = 0; kCount < searchData.depthBlockCount; kCount++)
		{
			// only if the mode is valid and is safe or we override it with the kAllValidModesBit request flag
			/*if	(	searchData.displayModeTimingInfo.csTimingFlags & 1<<kModeValid && 
					(	searchData.displayModeTimingInfo.csTimingFlags & 1<<kModeSafe ||
						requestRecPtr->requestFlags & 1<<kAllValidModesBit
					)
				)
			{*/
				if (FindBestMatch (	requestRecPtr,
									searchData.depthBlocks[kCount].depthVPBlock.vpPixelSize,
									searchData.depthBlocks[kCount].depthVPBlock.vpBounds.right,
									searchData.depthBlocks[kCount].depthVPBlock.vpBounds.bottom,
									searchData.displayModeResolutionInfo.csRefreshRate))
				{
					requestRecPtr->screenDevice = walkDevice;
					requestRecPtr->availBitDepth = searchData.depthBlocks[kCount].depthVPBlock.vpPixelSize;
					requestRecPtr->availHorizontal = searchData.depthBlocks[kCount].depthVPBlock.vpBounds.right;
					requestRecPtr->availVertical = searchData.depthBlocks[kCount].depthVPBlock.vpBounds.bottom;
					
					// now set the important info for DM to set the display
					requestRecPtr->depthMode = searchData.depthBlocks[kCount].depthSwitchInfo.csMode;
					requestRecPtr->displayMode = searchData.depthBlocks[kCount].depthSwitchInfo.csData;
					requestRecPtr->switchInfo = searchData.depthBlocks[kCount].depthSwitchInfo;
					if (searchData.displayModeTimingInfo.csTimingFlags & 1<<kModeSafe)
						requestRecPtr->availFlags = 0;							// mode safe
					else requestRecPtr->availFlags = 1<<kModeValidNotSafeBit;	// mode valid but not safe, requires user validation of mode switch
	
				}
			/*}*/

		}
	
		if (searchData.depthBlocks)
		{
			DisposePtr ((Ptr)searchData.depthBlocks);	// toss for this timing mode of this gdevice
			searchData.depthBlocks = nil;				// init it just so we know
		}
	}
}

Boolean FindBestMatch (VideoRequestRecPtr requestRecPtr, short bitDepth, unsigned long horizontal, unsigned long vertical, Fixed myRefreshRate)
{
	// ¥¥ do the big comparison ¥¥
	// first time only if	(no mode yet) and
	//						(bounds are greater/equal or kMaximizeRes not set) and
	//						(depth is less/equal or kShallowDepth not set) and
	//						(request match or kAbsoluteRequest not set)
	if	(	nil == requestRecPtr->displayMode
			&&
			(	(horizontal >= requestRecPtr->reqHorizontal &&
				vertical >= requestRecPtr->reqVertical)
				||														
				!(requestRecPtr->requestFlags & 1<<kMaximizeResBit)	
			)
			&&
			(	bitDepth <= requestRecPtr->reqBitDepth ||	
				!(requestRecPtr->requestFlags & 1<<kShallowDepthBit)		
			)
			&&
			(	(horizontal == requestRecPtr->reqHorizontal &&	
				vertical == requestRecPtr->reqVertical &&
				bitDepth == requestRecPtr->reqBitDepth)
				||
				!(requestRecPtr->requestFlags & 1<<kAbsoluteRequestBit)	
			)
			&&
			(
				myRefreshRate == requestRecPtr->refreshRate
			)
		)
		{
			// go ahead and set the new values
			return (true);
		}
	else	// can we do better than last time?
	{
		// if	(kBitDepthPriority set and avail not equal req) and
		//		((depth is greater avail and depth is less/equal req) or kShallowDepth not set) and
		//		(avail depth less reqested and new greater avail) or
		//		(request match or kAbsoluteRequest not set)
		if	(	(	requestRecPtr->requestFlags & 1<<kBitDepthPriorityBit && 
					requestRecPtr->availBitDepth != requestRecPtr->reqBitDepth
				)
				&&
				(	(	bitDepth > requestRecPtr->availBitDepth &&
						bitDepth <= requestRecPtr->reqBitDepth
					)
					||
					!(requestRecPtr->requestFlags & 1<<kShallowDepthBit)	
				)
				&&
				(	requestRecPtr->availBitDepth < requestRecPtr->reqBitDepth &&
					bitDepth > requestRecPtr->availBitDepth	
				)
				&&
				(	(horizontal == requestRecPtr->reqHorizontal &&	
					vertical == requestRecPtr->reqVertical &&
					bitDepth == requestRecPtr->reqBitDepth)
					||
					!(requestRecPtr->requestFlags & 1<<kAbsoluteRequestBit)	
				)
				&&
				(
					myRefreshRate == requestRecPtr->refreshRate
				)
			)
		{
			// go ahead and set the new values
			return (true);
		}
		else
		{
			// match resolution: minimize Æh & Æv
			if	(	abs((requestRecPtr->reqHorizontal - horizontal)) <=
					abs((requestRecPtr->reqHorizontal - requestRecPtr->availHorizontal)) &&
					abs((requestRecPtr->reqVertical - vertical)) <=
					abs((requestRecPtr->reqVertical - requestRecPtr->availVertical))
				)
			{
				// now we have a smaller or equal delta
				//	if (h or v greater/equal to request or kMaximizeRes not set) 
				if (	(horizontal >= requestRecPtr->reqHorizontal &&
						vertical >= requestRecPtr->reqVertical)
						||
						!(requestRecPtr->requestFlags & 1<<kMaximizeResBit)
					)
				{
					// if	(depth is equal or kBitDepthPriority not set) and
					//		(depth is less/equal or kShallowDepth not set) and
					//		([h or v not equal] or [avail depth less reqested and new greater avail] or depth equal avail) and
					//		(request match or kAbsoluteRequest not set)
					if	(	(	requestRecPtr->availBitDepth == bitDepth ||			
								!(requestRecPtr->requestFlags & 1<<kBitDepthPriorityBit)
							)
							&&
							(	bitDepth <= requestRecPtr->reqBitDepth ||	
								!(requestRecPtr->requestFlags & 1<<kShallowDepthBit)		
							)
							&&
							(	(requestRecPtr->availHorizontal != horizontal ||
								requestRecPtr->availVertical != vertical)
								||
								(requestRecPtr->availBitDepth < requestRecPtr->reqBitDepth &&
								bitDepth > requestRecPtr->availBitDepth)
								||
								(bitDepth == requestRecPtr->reqBitDepth)
							)
							&&
							(	(horizontal == requestRecPtr->reqHorizontal &&	
								vertical == requestRecPtr->reqVertical &&
								bitDepth == requestRecPtr->reqBitDepth)
								||
								!(requestRecPtr->requestFlags & 1<<kAbsoluteRequestBit)	
							)
							&&
							(
								myRefreshRate == requestRecPtr->refreshRate
							)
						)
					{
						// go ahead and set the new values
						return (true);
					}
				}
			}
		}
	}
	return (false);
}

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

#ifndef __UREQUESTVIDEO_H__
#define __UREQUESTVIDEO_H__

 
#include <QuickDraw.h>
#include <Video.h>
#include <Displays.h>


#ifdef __cplusplus
extern "C"	{

// requestFlags bit values in VideoRequestRec (example use: 1<<kAbsoluteRequestBit)
enum {
	kBitDepthPriorityBit		= 0,	// Bit depth setting has priority over resolution
	kAbsoluteRequestBit			= 1,	// Available setting must match request
	kShallowDepthBit			= 2,	// Match bit depth less than or equal to request
	kMaximizeResBit				= 3,	// Match screen resolution greater than or equal to request
	kAllValidModesBit			= 4		// Match display with valid timing modes (may include modes which are not marked as safe)
};

// availFlags bit values in VideoRequestRec (example use: 1<<kModeValidNotSafeBit)
enum {
	kModeValidNotSafeBit		= 0		//  Available timing mode is valid but not safe (requires user confirmation of switch)
};

// video request structure
struct VideoRequestRec	{
	GDHandle		screenDevice;		// <in/out>	nil will force search of best device, otherwise search this device only
	short			reqBitDepth;		// <in>		requested bit depth
	short			availBitDepth;		// <out>	available bit depth
	unsigned long	reqHorizontal;		// <in>		requested horizontal resolution
	unsigned long	reqVertical;		// <in>		requested vertical resolution
	unsigned long	availHorizontal;	// <out>	available horizontal resolution
	unsigned long	availVertical;		// <out>	available vertical resolution
	unsigned long	requestFlags;		// <in>		request flags
	unsigned long	availFlags;			// <out>	available mode flags
	unsigned long	displayMode;		// <out>	mode used to set the screen resolution
	unsigned long	depthMode;			// <out>	mode used to set the depth
	Fixed 			refreshRate;		// <out>	Vertical Refresh Rate in Hz
	VDSwitchInfoRec	switchInfo;			// <out>	DM2.0 uses this rather than displayMode/depthMode combo
};
typedef struct VideoRequestRec VideoRequestRec;
typedef struct VideoRequestRec *VideoRequestRecPtr;



void GetRequestTheDM2Way (		VideoRequestRecPtr requestRecPtr,
								GDHandle walkDevice,
								DMDisplayModeListIteratorUPP myModeIteratorProc,
								DMListIndexType theDisplayModeCount,
								DMListType *theDisplayModeList);

pascal void ModeListIterator (	void *userData,
								DMListIndexType itemIndex,
								DMDisplayModeListEntryPtr displaymodeInfo);

Boolean FindBestMatch (			VideoRequestRecPtr requestRecPtr,
								short bitDepth,
								unsigned long horizontal,
								unsigned long vertical,
								Fixed myRefreshRate);

void GravitateMonitors (void);

pascal Boolean ConfirmAlertFilter (DialogRef dlg, EventRecord *evt, short *itemHit);


// Routine defines
OSErr RVRequestVideoSetting(VideoRequestRecPtr requestRecPtr);
OSErr RVGetCurrentVideoSetting(VideoRequestRecPtr requestRecPtr);
OSErr RVSetVideoRequest (VideoRequestRecPtr requestRecPtr);
OSErr RVConfirmVideoRequest (VideoRequestRecPtr requestRecPtr);
OSErr RVSetVideoAsScreenPrefs (void);

#ifdef __cplusplus
}
#endif
#endif

#endif	/*  __UREQUESTVIDEO_H__  */

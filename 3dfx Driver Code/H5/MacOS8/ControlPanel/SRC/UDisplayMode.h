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

#ifndef __UDISPLAYMODE_H__
#define __UDISPLAYMODE_H__

#include <Dialogs.h>
#include <Devices.h>
#include <Displays.h>
#include <Errors.h>
#include <FixMath.h>
#include <fp.h>
#include <Gestalt.h>
#include <Memory.h>
#include <Palettes.h>
#include <PLStringFuncs.h>
#include <QuickDraw.h>
#include <ROMDefs.h>
#include <Slots.h>
#include <StdIO.h>
#include <Video.h>
#include <TextUtils.h>
#include <Strings.h>

#include <stdlib.h>


#ifdef __cplusplus
extern "C"	{

typedef struct Node_s Node_t;

/* Ken old favorite Amiga style doubly-linked list stuff */

struct Node_s
{
	Node_t *succ;
	Node_t *prev;
};

typedef struct List_s List_t;

/* I know someone is going to look at these next two definitions (List_s & NewList)
 * and scratch their head, so here's the deal:
 *
 * The list header saves space by embedding & overlapping the head and tail sentinel
 * nodes.  Normally we'd have something like this:
 *
 *     		Head		Node		Node		Tail
 * succ		----------->----------->-----------> 0
 * prev      0  <-----------<-----------<-----------
 *
 * The List_s structure just overlaps the 0's from the head & tail nodes.
 *
 * That's it!  The only thing complicated from other systems is that nodes with 
 * 0 succ or 0 prev are not valid nodes.
 */
 
struct List_s
{
	Node_t *head;
	Node_t *tail;
	Node_t *tailPred;
};




// rcf These were'nt here, and it seems as if they should be...
void NewList(List_t *list);
void AddHead(List_t *list, Node_t *node);
void AddTail(List_t *list, Node_t *node);
Node_t *Remove(Node_t *node);
Node_t *RemHead(List_t *list);
Node_t *RemTail(List_t *list);
void Insert(Node_t *before, Node_t *after);


/* Okay, enough of that. */



struct CPDisplayDesc_s
{
	SInt16						resIndex;
	SInt16						refIndex;
	SInt16						depthIndex;
	
	unsigned long				Horizontal;
	unsigned long				Vertical;
	Fixed						RefreshRate;
	unsigned short				Depth;
	
	VDTimingInfoRec				displayModeTimingInfo;
	unsigned long				displayModeFlags;
	unsigned long				mycsData;									
	unsigned short				mycsMode;
	short 						mygdRefNum;	
};
typedef struct CPDisplayDesc_s	CPDisplayDesc_t;


struct CPDisplay_s
{
	Node_t				node;
	CPDisplayDesc_t	    desc;

};
typedef struct CPDisplay_s	CPDisplay_t;

struct DepthInfo {
	VDSwitchInfoRec			depthSwitchInfo;			// This is the switch mode to choose this timing/depth
	VPBlock					depthVPBlock;				// VPBlock (including size, depth and format)
};
typedef struct DepthInfo DepthInfo;

struct ListIteratorDataRec {
	unsigned long			displayModeFlags;			// 
	VDSwitchInfoRec			displayModeSwitchInfo;		//
	VDResolutionInfoRec		displayModeResolutionInfo;	//
	VDTimingInfoRec			displayModeTimingInfo;		// Contains timing flags and such
	unsigned long			depthBlockCount;			// How many depths available for a particular timing
	DepthInfo				*depthBlocks;				// Array of DepthInfo
	Str255					displayModeName;			// name of the timing mode
};
typedef struct ListIteratorDataRec ListIteratorDataRec;




List_t * MakeDisplayModeList(short currentRefNum);


void AddAvailableDisplayMode2List (GDHandle walkDevice,
								DMDisplayModeListIteratorUPP myModeIteratorProc,
								DMListIndexType theDisplayModeCount,
								DMListType *theDisplayModeList,
								short currentRefNum);



#ifdef __cplusplus
}
#endif
#endif

#endif	/*  __UDISPLAYMODE_H__  */

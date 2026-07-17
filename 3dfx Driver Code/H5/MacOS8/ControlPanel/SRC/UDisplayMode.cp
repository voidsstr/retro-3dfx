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

#include "CDisplayView.h"
#include "UDisplayMode.h"
#include "URequestVideo.h"

void NewList(List_t *list)
{
	list->head = (Node_t *)&list->tail;
	list->tail = 0;
	list->tailPred = (Node_t *)list;
}

void AddHead(List_t *list, Node_t *node)
{
	Node_t *head = list->head;
	
	node->succ = head;
	node->prev = head->prev;
	head->prev = node;
	list->head = node;
}

void AddTail(List_t *list, Node_t *node)
{
	Node_t *tail = list->tailPred;
	
	node->succ = tail->succ;
	node->prev = tail;
	list->tailPred = node;
	tail->succ = node;
}

Node_t *Remove(Node_t *node)
{
	node->succ->prev = node->prev;
	node->prev->succ = node->succ;
	return node;
}

Node_t *RemHead(List_t *list)
{
	if(list->head->succ)
		return Remove(list->head);
	return 0;
}

Node_t *RemTail(List_t *list)
{
	if(list->tailPred->prev)
		return Remove(list->head);
	return 0;
}

static List_t CPDisplayList = { 0, 0, 0 };


List_t * MakeDisplayModeList(short currentRefNum)
{
	Boolean							displayMgrPresent;
	short							iCount = 0;					// just a counter of GDevices we have seen
	DMDisplayModeListIteratorUPP	myModeIteratorProc = nil;	// for DM2.0 searches
	SpBlock							spBlock;
	Boolean							suppliedGDevice;	
	DisplayIDType					theDisplayID;				// for DM2.0 searches
	DMListIndexType					theDisplayModeCount;		// for DM2.0 searches
	DMListType						theDisplayModeList;			// for DM2.0 searches
	long							value = 0;
	GDHandle						walkDevice = nil;			// for everybody
	
	/* initialize the list */
	NewList(&CPDisplayList);
	
	Gestalt(gestaltDisplayMgrAttr,&value);
	displayMgrPresent=value&(1<<gestaltDisplayMgrPresent);
	displayMgrPresent=displayMgrPresent && (SVersion(&spBlock)==noErr);	// need slot manager
	if (displayMgrPresent)												// and Display Manager
	{	
		walkDevice = DMGetFirstScreenDevice (dmOnlyActiveDisplays);			// for everybody
		suppliedGDevice = false;
		
		myModeIteratorProc = NewDMDisplayModeListIteratorProc(ModeListIterator);	// for DM2.0 searches
	
		// Note that we are hosed if somebody changes the gdevice list behind our backs while we are iterating....
		// ...now do the loop if we can start
		if( walkDevice && myModeIteratorProc) do // start the search
		{
			iCount++;		// GDevice we are looking at (just a counter)
			if( noErr == DMGetDisplayIDByGDevice( walkDevice, &theDisplayID, false ) )	// DM1.0 does not need this, but it fits in the loop
			{
				theDisplayModeCount = 0;	// for DM2.0 searches
				if (noErr == DMNewDisplayModeList(theDisplayID, 0, 0, &theDisplayModeCount, &theDisplayModeList) )
				{
					// search video devices the new kool way through Display Manager 2.0
					AddAvailableDisplayMode2List ( walkDevice, myModeIteratorProc, theDisplayModeCount, &theDisplayModeList, currentRefNum);
					DMDisposeList(theDisplayModeList);	// now toss the lists for this gdevice and go on to the next one
				}
			}
			
		} while ( !suppliedGDevice && nil != (walkDevice = DMGetNextScreenDevice ( walkDevice, dmOnlyActiveDisplays )) );	// go until no more gdevices
		if( myModeIteratorProc )
			DisposeRoutineDescriptor(myModeIteratorProc);
		
	}
	
	return (&CPDisplayList);
}




void AddAvailableDisplayMode2List (GDHandle walkDevice,
							DMDisplayModeListIteratorUPP myModeIteratorProc,
							DMListIndexType theDisplayModeCount,
							DMListType *theDisplayModeList,
							short currentRefNum)
{
	short					jCount;
	short					kCount;
	ListIteratorDataRec		searchData;
	CPDisplay_t				*cpdisplay;
	CPDisplay_t				*lastcpdisplay = nil;
	short					resPopup, refPopup, depthPopup;
	
		
	searchData.depthBlocks = nil;
	// get the mode lists for this GDevice
	for (jCount=0; jCount<theDisplayModeCount; jCount++)		// get info on all the resolution timings
	{
		DMGetIndexedDisplayModeFromList(*theDisplayModeList, jCount, 0, myModeIteratorProc, &searchData);		
		
		/* Look for all the available depths */
		if (searchData.depthBlockCount) for (kCount = 0; kCount < searchData.depthBlockCount; kCount++)
		{
				cpdisplay = (CPDisplay_t *)NewPtr(sizeof(*cpdisplay));
				if (!cpdisplay)
					break;
				
				/* Timing Flags : Valid, Safe, Shown, ...*/
				cpdisplay->desc.displayModeTimingInfo = searchData.displayModeTimingInfo;
				cpdisplay->desc.mycsData = searchData.depthBlocks[0].depthSwitchInfo.csData;
				
				/* Mode Flags : Recommended list of timing or not ...*/
				cpdisplay->desc.displayModeFlags =  searchData.displayModeFlags;
				cpdisplay->desc.Horizontal = 
							searchData.depthBlocks[kCount].depthVPBlock.vpBounds.right;
				cpdisplay->desc.Vertical =
							searchData.depthBlocks[kCount].depthVPBlock.vpBounds.bottom;
				cpdisplay->desc.Depth =
							searchData.depthBlocks[kCount].depthVPBlock.vpPixelSize;
				cpdisplay->desc.RefreshRate = 
							searchData.displayModeResolutionInfo.csRefreshRate;
				cpdisplay->desc.mycsMode = 
							searchData.depthBlocks[kCount].depthSwitchInfo.csMode;
				
				cpdisplay->desc.mygdRefNum = (*walkDevice)->gdRefNum;
				
				AddTail (&CPDisplayList, &cpdisplay->node);
		}
		
		
		
		if (searchData.depthBlocks)
		{
			DisposePtr ((Ptr)searchData.depthBlocks);	// toss for this timing mode of this gdevice
			searchData.depthBlocks = nil;				// init it just so we know
		}
		
	}
}

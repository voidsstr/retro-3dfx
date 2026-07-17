/******************************************************************************
	File:		
	Copyright:	© 1999  3Dfx Interactive, Inc.  All Rights Reserved.
	Author:		Andrew Mellinger
	Purpose:
	History:	
 ******************************************************************************/
	
	/* Headers */
/******************************************************************************/
#include "GWorldSupport.h"
#include "hrm_mem.h"
#include "HRM_Priv.h"

	/* Constants */
/******************************************************************************/

	/* Typedefs */
	/* Globals */
	/* File Globals */
/******************************************************************************/
bool			gGWorldSupportInited = false;
QHdr			gTrackerQueue;

	/* Prototypes */
/******************************************************************************/
OSErr		AllocMemFn(UInt32 type, UInt32 size, Ptr *buffer, UInt32 *rowBytes, UInt32 refCon);
OSErr		ReleaseMemFn(Ptr buffer, UInt32 refCon);
void		GWorldSupportNotifyProc(mmBlock_t *blockPt, UInt32 code);

	/* Methods */
	/* Functions */
/*----------------------------------------------------------------------------*\
	==> InstallGWorldSupport <==
\*----------------------------------------------------------------------------*/
OSErr InstallGWorldSupport(h3Info *h3InfoDst)
{
	long							status = noErr;
	AcceleratorMemoryParamBlock 	paramBlock;

	if (!gGWorldSupportInited)
	{
		gTrackerQueue.qFlags = 0;
		gTrackerQueue.qHead = 0;
		gTrackerQueue.qTail = 0;
		gGWorldSupportInited = true;
	}

	paramBlock.gdRefNum = h3InfoDst->board->drvrRefNum;
	paramBlock.getMemProc = (AllocAcceleratorMemProcPtr) AllocMemFn;
	paramBlock.releaseMemProc = (ReleaseAcceleratorMemProcPtr) ReleaseMemFn;
	paramBlock.refCon = (UInt32) h3InfoDst;
	paramBlock.reserved = 0;

	status = HackNQDMisc(kRegisterHdwrMemory, (Int32 *) &paramBlock);

	return (status);
}

/*----------------------------------------------------------------------------*\
	==> UninstallGWorldSupport <==

	We need to remove all the important blocks, and then tell QuickDraw we
	aren't doing GWorld support anymore.
\*----------------------------------------------------------------------------*/
OSErr UninstallGWorldSupport(h3Info *h3InfoDst)
{
	AcceleratorMemoryParamBlock		paramBlock;
	long status	= noErr;

	GWorldTracker	*trackerPt = (GWorldTracker	*) gTrackerQueue.qHead;
	GWorldTracker	*nextTrackerPt;
	
	while (trackerPt)
	{
		// Hold on to the next pointer in case we remove this guy.
		nextTrackerPt = (GWorldTracker*) trackerPt->qLink;
		if (trackerPt->h3InfoDst == h3InfoDst)
		{
			Dequeue((QElemPtr)trackerPt, &gTrackerQueue);
			status = HackNQDMisc(kReleaseAcceleratorMemory, (Int32 *) trackerPt->blockPt->start);
			DisposePtr((Ptr)trackerPt);
		}

		trackerPt = nextTrackerPt;
	}

	// Tell QuickDraw we don't support this anymore.
	paramBlock.gdRefNum = h3InfoDst->board->drvrRefNum;
	paramBlock.getMemProc = (AllocAcceleratorMemProcPtr) AllocMemFn;
	paramBlock.releaseMemProc = (ReleaseAcceleratorMemProcPtr) ReleaseMemFn;
	paramBlock.refCon = (UInt32) h3InfoDst;
	paramBlock.reserved = 0;
	
	HackNQDMisc(kUnregisterHdwrMemory, (Int32 *) &paramBlock);
	
	return (noErr);
}

/*----------------------------------------------------------------------------*\
	==> AllocMemFn <==

	type : 		gets all the GWorld flags rather than just the 
				local or distant flag.
	
	size :		minimum size needed for supplied rowBytes.
	
	rowBytes : 	initially set to exactly the width required.
				CHANGE to indicated optimal alignment.
	
	refCon :	we stored the device in the refCon (in the install
				routine). This sample doesn't use it but if you
				had more than one of your cards installed it
				would be handy to know where the alloction needed occur.
					
\*----------------------------------------------------------------------------*/
OSErr AllocMemFn(UInt32 type, UInt32 size, Ptr *bufferPt, UInt32 *rowBytes, UInt32 refCon)
{
	OSErr			err = noErr;
	h3Info *		h3InfoDst = (h3Info *) refCon;

	// as a simple test we will only handle VRAM requests
	if (type & useDistantHdwrMem)
	{
		// if we're disabled, get out.
		if(gPreferences->m2D[h3InfoDst->prefsIdx]->disableFlags & kDisableVRAMGWorlds)
			return memFullErr;

		// Round rowBytes up to a multiple of 4 and adjust size accordingly.
		// NOTE: In 8-bit mode, whoever calls us passes a rowBytes that's a little too small.
		// We take care of this here by adding 7 instead of 3 when rounding up.
		UInt32 height = size / *rowBytes;
		*rowBytes = (*rowBytes + 7) & ~3;
		size = height * (*rowBytes);

		// To reduce memory fragmentation we may want to allocate a block
		// of a bunch up front then just use them up.  However this takes
		// a little more memory management.
		GWorldTracker *trackerPt = (GWorldTracker*)NewPtrSys(sizeof(GWorldTracker));
		if (trackerPt == 0)
			return memFullErr;

		Enqueue((QElemPtr)trackerPt, &gTrackerQueue);
		
		// Allocate space on the board.  Tell the board that this is 
		// important memory but can go away.
//		trackerPt->blockPt = hrmAllocateBlock(h3InfoDst->board, size, HRM_MEMF_LINEAR | HRM_MEM_PRI_CACHE2);
		trackerPt->blockPt = hrmAllocateBlock(h3InfoDst->board, size, HRM_MEMF_LINEAR, HRM_MEM_PRI_CACHE2);

		strcpy(trackerPt->blockPt->comment, "Offscreen GWorld");

		// Fix up the scratch space memory record
		trackerPt->blockPt->userData = trackerPt;
		trackerPt->blockPt->notifyProc = GWorldSupportNotifyProc;

		// Give back an address in the same aperture as the screen.
		{
			UInt32 temp = trackerPt->blockPt->start;
			
			temp &= 0xF3FFFFFF;
			
			// This doesn't work.
			// temp |= 0x0C000000 & h3InfoDst->board->frameBufferBaseAddress;
			
			// This does.
			switch(h3InfoDst->depth)
			{
				case 32:	temp |= 0x04000000;		break;
				case 16:	temp |= 0x0C000000;		break;
				case  8:	temp |= 0x00000000;		break;
			}
				
			
			trackerPt->buffer = temp;
		}
		
		*bufferPt = (Ptr)trackerPt->buffer;
		
		trackerPt->rowBytes = *rowBytes;
	}
	else
	{
		// invalid device request - reasonable error ?
		err = cDevErr;
	}
	
	return err;
}

/*----------------------------------------------------------------------------*\
	==> ReleaseMemFunction <==
	
	Quickdraw is done with the memory.  Walk out data list to find the memBlock
	to free.
\*----------------------------------------------------------------------------*/
OSErr ReleaseMemFn(Ptr buffer, UInt32 refCon)
{
#pragma unused (refCon)
	GWorldTracker *trackerPt = (GWorldTracker *)gTrackerQueue.qHead;
	
	while (trackerPt)
	{
		if (trackerPt->buffer == (UInt32) buffer)
		{
			hrmFreeBlock(trackerPt->blockPt);
			Dequeue((QElemPtr)trackerPt, &gTrackerQueue);
			DisposePtr((Ptr)trackerPt);
			return noErr;
		}

		trackerPt = (GWorldTracker*) trackerPt->qLink;
	}

	return qErr;
}

/*----------------------------------------------------------------------------*\
	==> GWorldSupportNotifyProc <==

	This routine is called by the HRM when something happens.  The userdata
	here is the one placed in the memBlock_t.
\*----------------------------------------------------------------------------*/
void GWorldSupportNotifyProc(mmBlock_t *blockPt, UInt32 code)
{
	long			status				= noErr;
	GWorldTracker	*tracker2DeletePt	= (GWorldTracker*)blockPt->userData;

	if (code == HRM_MEM_NOTIFY_DISPOSE)
	{
		status = HackNQDMisc(kReleaseAcceleratorMemory, (Int32 *) blockPt->start);

		// Get rid of our element.
		GWorldTracker	*trackerPt = (GWorldTracker *)gTrackerQueue.qHead;
		while (trackerPt)
		{
			if (trackerPt == tracker2DeletePt)
			{
				Dequeue((QElemPtr)trackerPt, &gTrackerQueue);
				DisposePtr((Ptr)trackerPt);
				break;
			}

			trackerPt = (GWorldTracker*) trackerPt->qLink;
		}
	}
	else if (code == HRM_MEM_NOTIFY_MOVED)
	{
		RelocateAcceleratorMemoryPB		pb;
		
		// Tell QD it moved.
		pb.oldAddress = (Ptr)tracker2DeletePt->buffer;
		pb.newAddress = (Ptr)tracker2DeletePt->blockPt->start;
		HackNQDMisc(kRelocateAcceleratorMemory, (Int32 *) &pb);

		// Update our internal pointer.
		tracker2DeletePt->buffer = tracker2DeletePt->blockPt->start;
	}
}

/******************************************************************************/
/*----------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

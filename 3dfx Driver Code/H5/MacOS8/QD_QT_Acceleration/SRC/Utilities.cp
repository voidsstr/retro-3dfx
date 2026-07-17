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
** $Header: H3Acceleration.c, 1, 9/11/99 3:59:40 PM, StarTeam VTS Administrator$
** $Log: 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 4     7/12/99 12:07p Kcd
** Fifo idle improvements for Banshee.
** 
** 3     7/09/99 7:18p Kcd
** More paranoid status checks.
** 
** 2     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

// Includes
//#define WANT_DCON	1
//#include "DConLoader.h"

#define __MACERRORS__
#include <Types.h>
#include <Memory.h>
#include <Errors.h>
#include <Quickdraw.h>
#include <NQDAcceleration.h>
#include <Devices.h>
#include <Displays.h>

#include "InstallAcceleration.h"
#include "h3defs.h"
#include "h3gdefs.h"
#include "minihwc.h"
#include "hwcio.h"
#include "Utilities.h"

#include "FifoClasses.h"

#include "HRM_Priv.h"

#include "hrm_arbitration.h"

// Globals

// Set to TRUE when a new NQD operation is entered
bool gNewNQDOperation;

// Set to true if we are running on a machine with Altivec.
bool gAltivecAvailable = false;

/* Fast table lookup for destination bitmaps.  We know that our driver always 
   requests that the V3 gets 256MB of address space.  So, we can use the upper
   4 bits to index into a table to quickly find a batching board for that memory
   region.  No one should have pixmaps that point into that segment that don't
   point into our RAM. 
   
   Note that H5 (Napalm) will probably always request 512MB chunks because of the
   duplicated address spaces.  However, we'll be able to deal with that by simply
   marking two adjacent blocks with the same pointer */
struct h3Info h3_info[16];

bool gCommandExNeedsRestore = false;

// Here for reference
//const UInt32 kROPSource  = 0x000000CC << SSTG_ROP0_SHIFT;
//const UInt32 kROPDest	   = 0x000000AA << SSTG_ROP0_SHIFT;
//const UInt32 kROPPattern = 0x000000F0 << SSTG_ROP0_SHIFT;

// This just cleans up the table a bit.
#define rop(x)\
	((x) & (0x000000FF << SSTG_ROP0_SHIFT))
	
const UInt32 gModeToRop[16] =
{
	rop(	 kROPSource					),		// srcCopy		0xCC
	rop(	 kROPSource & kROPDest		),		// srcOr		0x88
	rop(	~kROPSource ^ kROPDest		),		// srcXor		0x99
	rop(	~kROPSource | kROPDest		),		// srcBic		0xBB
	
	rop(	~kROPSource					),		// notSrcCopy	0x33
	rop(	~kROPSource & kROPDest		),		// notSrcOr		0x22
	rop(	 kROPSource ^ kROPDest		),		// notSrcXor	0x66
	rop(	 kROPSource | kROPDest		),		// notSrcBic	0xEE
	
	rop(	 kROPPattern				),		// patCopy		0xF0
	rop(	 kROPPattern & kROPDest		),		// patOr		0xA0
	rop(	~kROPPattern ^ kROPDest		),		// patXor		0x5A
	rop(	~kROPPattern | kROPDest		),		// patBic		0xAF

	rop(	~kROPPattern				),		// notPatCopy	0x0F
	rop(	~kROPPattern & kROPDest		),		// notPatOr		0x0A
	rop(	 kROPPattern ^ kROPDest		),		// notPatXor	0xA5
	rop(	 kROPPattern | kROPDest		)		// notPatBic	0xFA
};

const UInt32 gModeToRop8[16] =
{
	rop(	 kROPSource					),		// srcCopy		0xCC
	rop(	 kROPSource | kROPDest		),		// srcOr		0xEE
	rop(	 kROPSource ^ kROPDest		),		// srcXor		0x66
	rop(	~kROPSource & kROPDest		),		// srcBic		0x22

	rop(	~kROPSource					),		// notSrcCopy	0x33
	rop(	~kROPSource | kROPDest		),		// notSrcOr		0xBB
	rop(	~kROPSource ^ kROPDest		),		// notSrcXor	0x99
	rop(	 kROPSource & kROPDest		),		// notSrcBic	0x88

	rop(	 kROPPattern				),		// patCopy		0xF0
	rop(	 kROPPattern | kROPDest		),		// patOr		0xFA
	rop(	 kROPPattern ^ kROPDest		),		// patXor		0x5A
	rop(	~kROPPattern & kROPDest		),		// patBic		0x0A

	rop(	~kROPPattern				),		// notPatCopy	0x0F
	rop(	~kROPPattern | kROPDest		),		// notPatOr		0xAF
	rop(	~kROPPattern ^ kROPDest		),		// notPatXor	0xA5
	rop(	 kROPPattern & kROPDest		)		// notpatBic	0xA0
};

const UInt32 gModeToRop1[16] =
{
	rop(	 kROPSource				),						// srcCopy		
	rop(	 kROPSource 			) | SSTG_TRANSPARENT,	// srcOr		
	rop(	 kROPSource ^ kROPDest	),						// srcXor		
	rop(	 kROPSource				) | SSTG_TRANSPARENT,	// srcBic		

	rop(	 kROPSource				),						// notSrcCopy	
	rop(	 kROPSource				) | SSTG_TRANSPARENT,	// notSrcOr		
	rop(	~kROPSource ^ kROPDest	),						// notSrcXor	
	rop(	 kROPSource				) | SSTG_TRANSPARENT,	// notSrcBic	

	rop(	 kROPPattern			),						// patCopy		
	rop(	 kROPPattern 			) | SSTG_TRANSPARENT,	// patOr		
	rop(	 kROPPattern ^ kROPDest	),						// patXor		
	rop(	 kROPPattern			) | SSTG_TRANSPARENT,	// patBic		

	rop(	 kROPPattern			),						// notPatCopy	
	rop(	 kROPPattern			) | SSTG_TRANSPARENT,	// notPatOr		
	rop(	~kROPPattern ^ kROPDest	),						// notPatXor	
	rop(	 kROPPattern			) | SSTG_TRANSPARENT,	// notPatBic	
};

/*
	If an entry in the following table is true, the transfer mode needs the 
	source data negated before it gets to the blitter.  This is done
	so that the proper part of the data is made transparent by SSTG_TRANSPARENT.
*/
const UInt32 gModeToRop1InvertXfer[16] =
{
	0x00000000,	// srcCopy		
	0x00000000,	// srcOr		
	0x00000000,	// srcXor		
	0x00000000,	// srcBic		

	0x00000000,	// notSrcCopy	
	0xFFFFFFFF,	// notSrcOr		
	0x00000000,	// notSrcXor	
	0xFFFFFFFF,	// notSrcBic	

	0x00000000,	// patCopy		
	0x00000000,	// patOr		
	0x00000000,	// patXor		
	0x00000000,	// patBic		

	0x00000000,	// notPatCopy	
	0xFFFFFFFF,	// notPatOr		
	0x00000000,	// notPatXor	
	0xFFFFFFFF,	// notPatBic	
};

/* This table is used as follows:
	0:
		reg_colorFore gets drawVars->foreColor
		reg_colorBack gets drawVars->backColor
	1:
		reg_colorFore gets drawVars->backColor
		reg_colorBack gets drawVars->foreColor
	2:
		reg_colorFore gets 0xFFFFFFFF
		reg_colorBack gets 0x00000000
*/
const SInt32 gModeToRop1SwapColors[16] =
{
	0,	// srcCopy		
	0,	// srcOr		
	2,	// srcXor		
	1,	// srcBic		

	1,	// notSrcCopy	
	0,	// notSrcOr		
	2,	// notSrcXor	
	1,	// notSrcBic	

	0,	// patCopy		
	0,	// patOr		
	2,	// patXor		
	1,	// patBic		

	1,	// notPatCopy	
	0,	// notPatOr		
	2,	// notPatXor	
	1,	// notPatBic	
};

#undef rop

/********************************************************************************
	InitializeAccelerationHardware
		void
		
	Querys the HRM for installed Voodoo boards at startup.
*/
OSErr InitializeAccelerationHardware(void)
{
	bool	targetsInitted = false;

	dprintf("InitializeAccelerationHardware\n");

	// Check to see whether we can use Altivec.
	{
		long response;
		if(Gestalt(gestaltPowerPCProcessorFeatures, &response) == noErr)
		{
			if(response & (1 << gestaltPowerPCHasVectorInstructions))
			{
				gAltivecAvailable = true;
			}
		}
	}

	/* Query HRM for the number of targets it knows about, and then ask it for the target info. */
	UInt32 numTargets = hrmGetNumTargets();
	dprintf("hrmGetNumTargets() = %d\n",numTargets);

	// Declare each board invalid
	for (SInt32 index = 0; index < 16; ++index)
	{
		h3_info[index].boardFlags = 0;
	}

	/* For each target, grab board and fifo info */
	for (SInt32 boardNum = 0; boardNum < numTargets; boardNum++)
	{
		hrmBoard_t		*board    = hrmGetTargetAtIndex(boardNum);
		hrmFifoInfo		*fifoInfo = hrmGetTargetFifoInfo(board);
		hrmBoardInfo_t	*bInfo    = (hrmBoardInfo_t *) NewPtr(sizeof(hrmBoardInfo_t));
		
		if (!bInfo)
			continue;
					
		bInfo->size = sizeof(hrmBoardInfo_t);
		hrmGetTargetBoardInfoExt(board, bInfo);
dprintf("bInfo->deviceID = 0x%08x\n", bInfo->deviceID);
		dprintf("hrmGetTargetAtIndex(%d) = %08lx\n", boardNum, board);
		dprintf("info: %08lx %08lx %08lx\n", board, bInfo, fifoInfo);
		
#ifdef VOODOO4
		if(!IS_NAPALM(bInfo->deviceID))
			continue;
#else
		if((bInfo->deviceID != SST_DEVICE_ID_H4) && (bInfo->deviceID != SST_DEVICE_ID_H4_OEM))
			continue;
#endif

		/* Now fill in struct since we have all the info we need. */
		UInt32 boardIndex = bInfo->pciBaseAddr[1] >> 28;
		h3_info[boardIndex].board = board;
		h3_info[boardIndex].bInfo = bInfo;		
		h3_info[boardIndex].fifo = fifoInfo;
		h3_info[boardIndex].regsIO =     (VoodooIORegs *) (bInfo->pciBaseAddr[0] + SST_IO_OFFSET);
		h3_info[boardIndex].regsFifo = (VoodooFifoRegs *) (bInfo->pciBaseAddr[0] + SST_CMDAGP_OFFSET);
		h3_info[boardIndex].regs2D =     (Voodoo2DRegs *) (bInfo->pciBaseAddr[0] + SST_2D_OFFSET);
		h3_info[boardIndex].regs3D =     (Voodoo3DRegs *) (bInfo->pciBaseAddr[0] + SST_3D_OFFSET);
		h3_info[boardIndex].is66MHz = bInfo->is66MHz;

		// Preferences index
		h3_info[boardIndex].prefsIdx = board->prefsIdx;
		
		// FIFO arbitration
		h3_info[boardIndex].hasArbitration = false;
		h3_info[boardIndex].holdArbitration = false;

		/* Temp hack to make sure nothing else is blowing up. */
		fifoInfo->exclusiveMode = FXFALSE;
	
		// Set the clip0 registers to a wide-open clip range
		h3_info[boardIndex].regs2D->clip0min = 0;
		h3_info[boardIndex].regs2D->clip0max = 0xFFFFFFFF;
		
		// Set the commandExtra register to 0
		h3_info[boardIndex].regs2D->commandEx = 0;
		
		// Initialize our QuickDraw scratch space for this board, and declare the board valid
		h3_info[boardIndex].boardFlags = 1;
		h3_info[boardIndex].scratchSpace = 0;
		
#if AGP_FIFO
		// Check whether this board is using an AGP FIFO
		if(board->agpAllocated)
			h3_info[boardIndex].agpFIFO = true;
		else
			h3_info[boardIndex].agpFIFO = false;
#endif

		// There is at least one valid target
		targetsInitted = true;
	}

	/* As long as we found more than one board, we're okay.  If something
	else failed for some reason, we would have already bailed out. */
	if(targetsInitted)
		return noErr;
	else
		return -1;
}


#include "HRM_Priv.h"
/********************************************************************************
	hrmFifoWrap2
		h3Info *h3info
		
	Wraps the command FIFO. This version does not wait for the fifo wrap to complete
	before exiting.
*/
void hrmFifoWrap2(h3Info *h3info)
{
	hrmBoard_t *board = h3info->board;
	UInt32	hardwareReadPtr;
	Fifo2DRegs		blitter(h3info);
	
	// Check for previous wrap in progress
	do {
		hardwareReadPtr = h3info->regsFifo->cmdFifo0.readPtrL;
	} while (hardwareReadPtr > (UInt32) board->fifoInfo.fifoPtr);
	
	// Write the jump packet to the fifo
	//*board->fifoInfo.fifoPtr = board->fifoInfo.fifoJmpHdr[0];
	blitter.storeNonPixelData(board->fifoInfo.fifoPtr, board->fifoInfo.fifoJmpHdr[0]);

	__dcbst(board->fifoInfo.fifoPtr, 0);
	__sync();
	h3info->regsFifo->cmdFifo0.bump = 1;
	
	// Reset the fifo variables to the start of the fifo
	board->fifoInfo.fifoPtr = board->fifoInfo.fifoStart;
 	board->fifoInfo.lastBump = board->fifoInfo.fifoPtr;
	board->fifoInfo.fifoRoom = (hardwareReadPtr - ((UInt32) board->fifoInfo.fifoPtr & 0x00FFFFFF) >> 2);
}


/********************************************************************************
	H3BlitFinish
		long refCon
		
	Called by NQD at the end of a drawing operation. This function synchronizes the
	cpu with the Voodoo fifo by waiting for the fifo to clear.
*/
Int32  H3BlitFinish(long refCon)
{
	h3Info			*board = (h3Info *) refCon;
	
	// Blits that read from a target board and write to host memory do not have 
	// a valid board value in the refCon.
	if (!board)
		return true;
		
	hrmFifoInfo		*fifo  = board->fifo;
	Fifo2DRegs		blitter((h3Info *)refCon);
	
#if FIFO_CACHE_SPECIAL_CASE
	if(fifo->fifoPtr > fifo->lastBump)
	{
		// Someone was using a special case in the FifoClasses.  Finish up.
		blitter.endAlignFifo();
	}
#endif

	if(gCommandExNeedsRestore)
	{
		// If someone has been using the commandEx register, set it back to zero.
		gCommandExNeedsRestore = false;
		blitter.go1reg(reg_commandEx, 0);
	}
	
	// If we are using the deferred bump model, bump the fifo now.
	bumpFifoDeferred(board);
	
	__sync();
	
	ReleaseArbitration(board);

	while (board->regsFifo->cmdFifo0.depth) ;
	
	while (board->regs2D->status & (SST_BUSY | SST_GUI_BUSY | SST_CMD0_BUSY)) ;
	
	return true;
}

/********************************************************************************
	H3LineBlitFinish
		long refCon
		
	Called by NQD at the end of a line operation. This function synchronizes the
	cpu with the Voodoo fifo by waiting for the fifo to clear.
*/
Int32  H3LineBlitFinish(long refCon)
{
	h3Info			*board = (h3Info *) refCon;
	
	hrmFifoInfo		*fifo  = board->fifo;
	Fifo2DRegs		blitter((h3Info *)refCon);
	
#if FIFO_CACHE_SPECIAL_CASE
	if(fifo->fifoPtr > fifo->lastBump)
	{
		// Someone was using a special case in the FifoClasses.  Finish up.
		blitter.endAlignFifo();
	}
#endif

	if(gCommandExNeedsRestore)
	{
		// If someone has been using the commandEx register, set it back to zero.
		gCommandExNeedsRestore = false;
		blitter.go1reg(reg_commandEx, 0);
	}
	
	// If we are using the deferred bump model, bump the fifo now.
	bumpFifoDeferred(board);
	
	__sync();
	
	ReleaseArbitration(board);

	while (board->regsFifo->cmdFifo0.depth) ;

	return true;
}

/********************************************************************************
	ScratchSpaceNotifyProc
		void *userData
		UInt32 code
		
	This is a HRM Memory Manager callback proc to notify us when our scratch space
	memory block has been purged.
*/
void ScratchSpaceNotifyProc(struct mmBlock_s *block, UInt32 code)
{
	h3Info *h3info = (h3Info *) block->userData;
	
	if (code == HRM_MEM_NOTIFY_DISPOSE)
		h3info->scratchSpace = 0;
}

#pragma mark -
#pragma mark Misc. Utils

// For filling in the comment field in memory blocks.
void strcpy(char *dst, char *src)
{
	while((*dst++ = *src++) != 0)
		;
}

void strcat(char *dst, char *src)
{
	while(*dst != 0)
		dst++;
		
	while((*dst++ = *src++) != 0)
		;
}


/*----------------------------------------------------------------------------*\
	==> UStateLocalBuffer::UStateLocalBuffer <==
\*----------------------------------------------------------------------------*/
UStateLocalBuffer::UStateLocalBuffer(UInt32 numLongs)
{
	mBasePtr = (UInt32)NewPtrSys((numLongs * 4) + 3);
}

UStateLocalBuffer::~UStateLocalBuffer()
{
	if (mBasePtr != 0)
		DisposePtr((Ptr)mBasePtr);
}

UInt32 *UStateLocalBuffer::Get()
{
	if (mBasePtr == 0)
		return 0;\

	return (UInt32*)((mBasePtr + 3)	& ~3);
}

#pragma mark -

void GetArbitration(h3Info *board)
{
	if(!board->holdArbitration)
	{
		if(!board->hasArbitration)
		{
			board->hasArbitration = true;
			hrmArbiterGet(board->board);
		}
	}
}

void ReleaseArbitration(h3Info *board)
{
	if(!board->holdArbitration)
	{
		if(board->hasArbitration)
		{
			board->hasArbitration = false;
			hrmArbiterRelease(board->board);
		}
	}
}
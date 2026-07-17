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
** $Header: HRM_FIFO.c, 12, 10/11/00 7:35:06 PM, Brent$
** $Log: 
**  12   3dfx      1.2.1.2.1.5 10/11/00 Brent           Forced check in to enforce
**       branching.
**  11   3dfx      1.2.1.2.1.4 09/29/00 Critical Path   fixes for V3 address space
**       (cacheable) etc...
**  10   3dfx      1.2.1.2.1.3 08/15/00 Critical Path   new source drop
**  9    3dfx      1.2.1.2.1.2 06/30/00 Critical Path   new source drop
**  8    3dfx      1.2.1.2.1.1 06/23/00 Critical Path   you guessed it, more
**       hrmFifoWrap changes
**  7    3dfx      1.2.1.2.1.0 06/22/00 Stephane Huaulme go back to old
**       hrmFifoWrap()
**  6    3dfx      1.2.1.2     06/15/00 Critical Path   new fifo wrap proc
**  5    3dfx      1.2.1.1     05/27/00 Critical Path   new source drop
**  4    3dfx      1.2.1.0     05/11/00 Stephane Huaulme v3 / napalm cohabitation
**  3    MacOS Dev Tree1.2         03/07/00 Critical Path   better mem management,
**       more qd coverage, bug fixes...
**  2    MacOS Dev Tree1.1         02/07/00 Kenneth Dyke    Fixed line endings.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 5     8/23/99 2:36p Kcd
** Surface, memory management, and AGP support.
** 
** 4     7/26/99 1:52p Kcd
** Added new extensions to help support Power Management in 2D driver. 
** 
** Also fixed possible FIFO trashing problem when returning from
** fullscreen exclusive modes.
** 
** 3     7/19/99 4:05p Kcd
** Fix annoying warning.
** 
** 2     7/08/99 4:10p Kcd
** Removed fifo reset code in idle function.
** 
** 1     7/02/99 3:19p Kcd
** HRM fifo management.
**
*/


#include "hrm_priv.h"

#include "hdwr_res_mgr.h"
#include "hrm_mem.h"

#include "minihwc.h"
#include "hwcio.h"
#include "hrm_fifo.h"
#include "GraphicsPrivHwc.h"
#include "DCon.h"
#include <string.h>
#include <files.h>
#include <CodeFragments.h>
#include <NameRegistry.h>
#include <DriverServices.h>
#include <Video.h>
#include <cstddef>

OSErr			hrmPrivateControlCall(
						short				inRefNum,
						FxU32				which,
						hwcRequest_t		*inReq,
						hwcResponse_t		*outRes);

/* This really needs to be cleaned up and moved to a header file at some point. */
#define kFifoVRAM    (262144+4096)

#define P6FENCE __sync()

#if GDX_SWIZZLE_HACK
#define SET_FIFO_LFB(d, s)			do { dprintf("setLfb: %08lx\n",s); board->fifoInfo.setLfb(&(d),s); } while(0)
#define SET_FIFO_HOST(d, s) 		board->fifoInfo.setLfbHost(&(d),s)
#else
#define	SET_FIFO_LFB(d, s)			__stwbrx(s,(void *)&(d),0)
#define SET_FIFO_HOST(d, s) (d = (s))
#endif

#if PCI_COPYBACK
#define FIFO_CACHE_FLUSH(d)  __dcbf(d,-4)
#else
#define FIFO_CACHE_FLUSH(d)
#endif

#if PCI_BUMP_N_GRIND
#define BUMP_N_GRIND \
{ \
  FIFO_CACHE_FLUSH(board->fifoInfo.fifoPtr);\
  P6FENCE;\
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.bump, board->fifoInfo.fifoPtr - board->fifoInfo.lastBump);\
  P6FENCE;\
  board->fifoInfo.lastBump = board->fifoInfo.fifoPtr;\
}	
#else
#define BUMP_N_GRIND
#endif

/* Full byte swap */
void  __swizzleWrite32_8(volatile FxU32 *d, FxU32 s)
{
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbf((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	__stwbrx(s,(void *)d, 0);
}

/* Swap words */
void  __swizzleWrite32_16(volatile FxU32 *d, FxU32 s)
{
	FxU32 temp = (s >> 16) | (s << 16);
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbf((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	*d = temp;
}

/* Swap bytes within words */
void  __swizzleWrite16_16(volatile FxU32 *d, FxU32 s)
{
	FxU32 temp = (s >> 16) | (s << 16);
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbf((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	__stwbrx(temp,(void *)d, 0);
}

/* Pass through */
void  __swizzleWrite32_32(volatile FxU32 *d, FxU32 s)
{
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbf((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	*d = s;
}


/* The Voodoo^3 fifo is 4 byte aligned */
#define FIFO_ALIGN_MASK      0x03

/* We need some slop in the fifo for writing some bookkeeping data
 * since we don't let the fifo autowrap. Its actually a little bit
 * bigger just in case someone does not read this comment.
 * 
 * Fullscreen:
 *   1 jmp (1 32-bit word)
 * Windowed:
 *   pci: ret (1 32-bit word)
 *   agp: jmp (2 32-bit words) 
 */
#define FIFO_END_ADJUST  (sizeof(FxU32) << 3)

/* The two most commonly defined macros in the known universe */
#define MIN(__x, __y) (((__x) < (__y)) ? (__x) : (__y))
#define MAX(__x, __y) (((__x) < (__y)) ? (__y) : (__x))

short hrmFifoUpdate( hrmFifoUpdate_t *fifoUpdate );

#if 0
short
fifoDispatch_Fifo(
  hrmCommand_t *       ioCmd)
{
  short                theSuccess = -1;
  
  dprintf("fifoDispatchFifo: %08lx\n",ioCmd);
  
  switch( ioCmd->methodSelector ) {
    case HRM_FIFO_UPDATE:
      theSuccess = hrmFifoUpdate( &ioCmd->u.fifoUpdate );
      break;
      
    default:
      dprintf("fifoDispatch_Fifo(): invalid method selector = %d, ( ioCmd @ 0x%08x )\n", ioCmd->methodSelector, ioCmd);
      break;
  }
  
  return theSuccess;   
}
#endif

short hrmFifoUpdate( hrmFifoUpdate_t *fifoUpdate )
{
  short theSuccess = -1;
  hrmBoard_t *board = fifoUpdate->board;
  
  dprintf("fifoUpdate board: %08lx  set: %08lx %08lx\n",board,fifoUpdate->setLfb,fifoUpdate->setLfbHost);
  
  if(board)	{
  	board->fifoInfo.setLfb = fifoUpdate->setLfb;
  	board->fifoInfo.setLfbHost = fifoUpdate->setLfbHost;
	board->lastContextID = 0xFFFFFFFF;
  	theSuccess = 0;
  }
   
  return theSuccess;
}
 
hrmFifoInfo *hrmGetTargetFifoInfo(hrmBoard_t *board)
{
  if(board) {
    return &board->fifoInfo;
  }
  return NULL;
}

void strcpy(char *dst, char *src);
void strcpy(char *dst, char *src)
{
	while((*dst++ = *src++) != 0)
		;
}

/********************************************************************************
	hrmInitFifo
		hrmBoard_t *board
		FxBool doGetFifoInfo
		
	Fifo management code, cribbed from minihwc.c.
*/
void hrmInitFifo(hrmBoard_t *board, FxBool doGetFifoInfo)
{
	hwcBoardInfo *bInfo = &board->boardInfo;
	FxU32 miscInit1;
	OSErr       myErr;
	hwcResponse_t res;
    
	
#if PCI_BUMP_N_GRIND
	FxBool   disableHoles = FXTRUE,
#else
	FxBool   disableHoles = FXFALSE,
#endif    	
	agpEnable = FXFALSE;
	
	// MBW -- XXX -- Enable this in sync with AGP_FIFO in FifoClasses.h.
	// If this board is AGP, it should have had 1 megabyte of AGP memory allocated in 
	// hrmRegisterPCIBoard().
//	if(board->agpAllocated)
//		agpEnable = FXTRUE;
	
	board->boardInfo.fifoInfo.agpFifo = agpEnable;
	
	if(doGetFifoInfo)
	{	
		if(agpEnable)
		{
			// MBW -- XXX -- This is NOT correct
			board->fifoInfo.setLfb = __swizzleWrite32_8;
			board->fifoInfo.setLfbHost = __swizzleWrite32_8;
		}
		else
		{
		/* Get command FIFO info for this board */
		/* This had better not fail or we're hosed. */
			myErr = hrmPrivateControlCall( (short)board->boardInfo.hdc, k3DfxFifoFuncs, NULL, &res);
		
		if(myErr == noErr)
		{
			board->fifoInfo.setLfb = res.optData.fifoFuncsRes.setLfb;
			board->fifoInfo.setLfbHost = res.optData.fifoFuncsRes.setLfbHost;
			dprintf("setLfb: %08lx  setLfbHost: %08lx\n",board->fifoInfo.setLfb,board->fifoInfo.setLfbHost);
		}
	}		
	}		
	
	if(agpEnable)
	{
		// Set up the board to use the AGP FIFO
		
		// This is the fifo start address from the board's point of view
		board->boardInfo.fifoInfo.fifoStart = (UInt32)board->agpAddress.agpLogicalAddress;
		
		// This is the real length of the fifo
		board->boardInfo.fifoInfo.fifoLength = kFifoVRAM - 4096;

		// This is the fifo start address from the system's point of view
		board->fifoInfo.fifoStart = (FxU32*)((UInt32)board->agpAddress.systemLogicalAddress);
	}
	else
	{
	/* Allocate FIFO memory block.  This must not fail. */
	/* We can prevent the failure by invalidating all allocations when */
	/* the 2D driver goes into fullscreen exclusive mode.  Then the 2D driver */
	/* Will simply "allocate" the entire memory area, thus preventing anyone else */
	/* from grabbing any memory while fullscreen is in effect.  Then when we come out of */
	/* fullscreen mode, we should be the first non-desktop allocation that happens, so */
	/* we should always get the first chunk of video memory for the 2D fifo. */
	board->fifoMemBlock = hrmAllocateBlock(board, kFifoVRAM, HRM_MEMF_LINEAR | HRM_MEMF_PAGE_ALIGN, HRM_MEM_PRI_ABSOLUTE);
	strcpy(board->fifoMemBlock->comment, "Command FIFO");
  
	/* The 4K offset is a hack.  This makes sure we don't bus error when the code tries to 
	flush the previous cache line (which could potentially be outside the bounds of our
		board's memory address space). */
		board->boardInfo.fifoInfo.fifoStart = (board->fifoMemBlock->start - 
				board->boardInfo.regInfo.rawLfbBase + 4096);
	board->boardInfo.fifoInfo.fifoLength = board->fifoMemBlock->size - 4096;
	dprintf("board->fifoMemBlock: %08lx\n",board->fifoMemBlock);
	if(board->fifoMemBlock)
	{
		dprintf("fifo start: %08lx  size: %08lx\n",board->fifoMemBlock->start,board->fifoMemBlock->size);
	}
		
		UInt32 cachedAreaStart;
		UInt32 cachedAreaSize;
		UInt32 newCachedAreaStart;
		
		// Only offset the FIFO when running on a Voodoo 4
		if (IS_NAPALM(bInfo->pciInfo.deviceID))
		{
			// Cached area is 32 megabytes, located in the upper half of the big-endian aperture.
			cachedAreaStart = (UInt32) board->fifoInfo.fifoStart & 0xFE000000;
			cachedAreaSize = 0x02000000;
			board->fifoInfo.fifoStart = (FxU32 *)(board->boardInfo.regInfo.rawLfbBase + 
					board->boardInfo.fifoInfo.fifoStart + 0x06000000);
			newCachedAreaStart = (UInt32) board->fifoInfo.fifoStart & 0xFE000000;
		}
		else
		{
			// Cached area is 256K, located 4K from the base of the VRAM area
			cachedAreaStart = (UInt32) board->fifoInfo.fifoStart - 4096;
			cachedAreaSize = 0x00040000;
			board->fifoInfo.fifoStart = (FxU32 *)(board->boardInfo.regInfo.rawLfbBase + 
					board->boardInfo.fifoInfo.fifoStart);
			newCachedAreaStart = (UInt32) board->fifoInfo.fifoStart - 4096;
		}
		
#if PCI_COPYBACK
		// Uncache the previous fifo block, if there was one.
		if (board->fifoInfo.cacheActive) 
		{
			FlushProcessorCache(kCurrentAddressSpaceID, (void *) cachedAreaStart, cachedAreaSize);
			SetProcessorCacheMode(kCurrentAddressSpaceID,  (void *) cachedAreaStart, cachedAreaSize, kProcessorCacheModeInhibited);
		}
		
		/* Enable copyback cache for fast(er) PCI transfers */
		SetProcessorCacheMode(kCurrentAddressSpaceID, (void *) newCachedAreaStart, cachedAreaSize, kProcessorCacheModeCopyBack);
		board->fifoInfo.cacheActive = FXTRUE;
#endif		

		// Reset command fifo
		HWC_IO_LOAD(bInfo->regInfo, miscInit1, miscInit1);
		miscInit1 |= SST_CMDSTREAM_RESET;
		HWC_IO_STORE(bInfo->regInfo, miscInit1, miscInit1);
		miscInit1 &= ~SST_CMDSTREAM_RESET;
		HWC_IO_STORE(bInfo->regInfo, miscInit1, miscInit1);
	}

	// What exactly does this mean for an AGP fifo?
	board->fifoInfo.fifoOffset = board->boardInfo.fifoInfo.fifoStart;

	// Init command transport values
	board->fifoInfo.fifoEnd = (FxU32 *)((FxU32)board->fifoInfo.fifoStart +
			board->boardInfo.fifoInfo.fifoLength - FIFO_END_ADJUST);
	board->fifoInfo.fifoSize = board->boardInfo.fifoInfo.fifoLength;
	board->fifoInfo.fifoPtr =  board->fifoInfo.fifoStart;
	board->fifoInfo.lastBump = board->fifoInfo.fifoPtr;
	board->fifoInfo.roomToEnd = (board->fifoInfo.fifoSize - FIFO_END_ADJUST) / sizeof(FxU32);
	board->fifoInfo.fifoRoom = board->fifoInfo.roomToEnd - 1 ; // - sizeof( FxU32 );
	board->fifoInfo.fifoJmpHdr[0] = ( SSTCP_PKT0_JMP_LOCAL | (board->fifoInfo.fifoOffset << (SSTCP_PKT0_ADDR_SHIFT - 2)));

	dprintf("fifoStart(HW): %08lx\n",board->fifoInfo.fifoOffset);
	dprintf("fifoStart(SW): %08lx\n",board->fifoInfo.fifoStart);
	dprintf("fifoEnd(SW):   %08lx\n",board->fifoInfo.fifoEnd);
	dprintf("fifoSize(SW):  %08lx\n",board->fifoInfo.fifoSize);
	dprintf("fifoPtr(SW):   %08lx\n",board->fifoInfo.fifoPtr);
	dprintf("roomToEnd:     %08lx\n",board->fifoInfo.roomToEnd);
	dprintf("fifoRoom:      %08lx\n",board->fifoInfo.fifoRoom);

	/* disable the CMD fifo */
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.baseSize, 0);  
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.baseAddrL,
			bInfo->fifoInfo.fifoStart>>12);
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.readPtrL, bInfo->fifoInfo.fifoStart);
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.readPtrH, 0);
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.aMin, bInfo->fifoInfo.fifoStart-4);
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.aMax, bInfo->fifoInfo.fifoStart-4);
  
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.depth, 0);
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.holeCount, 0);

	/* Fifo LWM /HWM/ THRESHOLD */
	if (bInfo->pciInfo.deviceID == 0x3)
	{ /* banshee */
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifoThresh, (0x09 << SST_HIGHWATER_SHIFT) | 0x2);
	} else
	{
	    HWC_CAGP_STORE(bInfo->regInfo, cmdFifoThresh, (0xf << SST_HIGHWATER_SHIFT) | 0x8);
	}
	
	// Enable the FIFO 
	HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.baseSize,
			((bInfo->fifoInfo.fifoLength >> 12) - 1) | SST_EN_CMDFIFO |
			(disableHoles ? SST_CMDFIFO_DISABLE_HOLES : 0) |
			(agpEnable ?  SST_CMDFIFO_AGP : 0));
	
}

/********************************************************************************
	hrmIdleFifo
		hrmBoard_t *board
		
	Bumps the command fifo if there are any unbumped writes pending, and waits for
	the fifo to flush and the chip to idle.
*/
FxU32 hrmIdleFifo(hrmBoard_t *board)
{
  hwcBoardInfo *bInfo = &board->boardInfo;
  FxU32 status, cmd, count = 0, rv = 1;
#if DEBUG
  FxU32 miscInit1;
#endif  
  BUMP_N_GRIND;	
  
  //dprintf("AvengerIdleFifo\n");
  do {
  	count++;
		HWC_IO_LOAD(bInfo->regInfo, status, status);
		HWC_CAGP_LOAD(bInfo->regInfo, cmdFifo0.unusedB, cmd);
#if 0
		if (count > 5000000) {
      dprintf("busy timeout!\n");
      ExitToShell();
    }
    #endif
  } while (status & (SST_BUSY /*|SST_GUI_BUSY*/));
 
  return rv;
  //dprintf("AvengerIdleFifo Complete\n");	
}

/********************************************************************************
	hrmDisableFifo
		hrmBoard_t *board
		
	Deinitializes the command FIFO.
*/
void hrmDisableFifo(hrmBoard_t *board)
{
  hwcBoardInfo *bInfo = &board->boardInfo;
  FxU32 miscInit1;
  
  hrmIdleFifo(board);
  
  //dprintf("AvengerDisableFifo()\n");

#if PCI_COPYBACK
	UInt32 cachedAreaStart;
	UInt32 cachedAreaSize;
	if (bInfo->pciInfo.deviceID == 9)
	{
		cachedAreaStart = (UInt32) board->fifoInfo.fifoStart & 0xFE000000;
		cachedAreaSize = 0x02000000;
	} else
	{
		cachedAreaStart = (UInt32) board->fifoInfo.fifoStart - 4096;
		cachedAreaSize = board->fifoInfo.fifoSize + 4096;
	}
	
	if(board->fifoInfo.cacheActive) 
	{
		FlushProcessorCache(kCurrentAddressSpaceID, (void *) cachedAreaStart, cachedAreaSize);
		SetProcessorCacheMode(kCurrentAddressSpaceID, (void *) cachedAreaStart, cachedAreaSize, kProcessorCacheModeInhibited);
    board->fifoInfo.cacheActive = FXFALSE;
  }
#endif
  
  // Reset command fifo
  HWC_IO_LOAD(bInfo->regInfo,miscInit1,miscInit1);
  miscInit1 |= SST_CMDSTREAM_RESET;
  HWC_IO_STORE(bInfo->regInfo,miscInit1,miscInit1);
  miscInit1 &= ~SST_CMDSTREAM_RESET;
  HWC_IO_STORE(bInfo->regInfo,miscInit1,miscInit1);
  
  /* disable the CMD fifo */
  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.baseSize, 0);  
  
  /* Free memory */
  hrmFreeBlock(board->fifoMemBlock);
  board->fifoMemBlock = 0;
}

FxU32 hrmHwFifoPtr(hrmBoard_t *board)
{
	hwcBoardInfo *bInfo = &board->boardInfo;
	FxU32 rVal = 0;
	FxU32 status, readPtrL1, readPtrL2;

	do {
  		HWC_CAGP_LOAD(bInfo->regInfo,cmdFifo0.readPtrL,readPtrL1);
  		HWC_IO_LOAD(bInfo->regInfo,status,status);
  		HWC_CAGP_LOAD(bInfo->regInfo,cmdFifo0.readPtrL,readPtrL2);
	} while (readPtrL1 != readPtrL2);
  
	rVal = ((FxU32) board->fifoInfo.fifoStart) + readPtrL2 - 
			(FxU32) board->fifoInfo.fifoOffset;
  
  return rVal;
} /* AvengerHwFifoPtr */

/********************************************************************************
	hrmFifoWrap
		hrmBoard_t *board
		
	Wraps the command FIFO. This version does not wait for the fifo wrap to complete
	before exiting.
*/
#if 0
FxU32 *hrmFifoWrap(hrmBoard_t *board)
{
	hwcBoardInfo 	*bInfo = &board->boardInfo;
	UInt32			hardwareReadPtr;
	
	// Check for previous wrap in progress
	do {
		hardwareReadPtr = hrmHwFifoPtr(board);
	} while (hardwareReadPtr > (UInt32) board->fifoInfo.fifoPtr);
	P6FENCE;
	
	// Write the jump packet to the fifo
	if(board->boardInfo.fifoInfo.agpFifo)
	{
		SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, board->fifoInfo.fifoJmpHdr[0]);
		if (!((UInt32) board->fifoInfo.fifoPtr & 31))
			__dcbst(board->fifoInfo.fifoPtr, -4);
		SET_FIFO_LFB(*board->fifoInfo.fifoPtr, board->fifoInfo.fifoJmpHdr[0]);
		__dcbst(board->fifoInfo.fifoPtr, 0);
		__sync();
		HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.bump, 2);
	} else
	{
		SET_FIFO_LFB(*board->fifoInfo.fifoPtr, board->fifoInfo.fifoJmpHdr[0]);
		__dcbst(board->fifoInfo.fifoPtr, 0);
		__sync();
		HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.bump, 1);
	}
	
	// Reset the fifo variables to the start of the fifo
	board->fifoInfo.fifoPtr = board->fifoInfo.fifoStart;
 	board->fifoInfo.lastBump = board->fifoInfo.fifoPtr;
	board->fifoInfo.fifoRoom = (hardwareReadPtr - ((UInt32) board->fifoInfo.fifoPtr & 0x00FFFFFF) >> 2);

	return (FxU32 *) hardwareReadPtr;
}
#else
FxU32 *hrmFifoWrap(hrmBoard_t *board)
{
  hwcBoardInfo *bInfo = &board->boardInfo;
  FxU32 timeOut = 0;
  
	FxU32 *rdPtr, *rdPtr2;
	
	// Check for a previous wrap in progress, and if one is happening, wait for it to
	// complete.
	do {
		rdPtr = (FxU32 *)hrmHwFifoPtr(board);
		dprintf("fifo: %08lx > %08lx\n",rdPtr,board->fifoInfo.fifoPtr);
	} while ( (rdPtr > board->fifoInfo.fifoPtr) || (rdPtr == board->fifoInfo.fifoStart));
	
	// Write the jump packet into the fifo, which will perform the wrap when it executes.
	P6FENCE;
	if (IS_NAPALM(board->boardInfo.pciInfo.deviceID))
	{
		*board->fifoInfo.fifoPtr++ = board->fifoInfo.fifoJmpHdr[0];
		if(board->boardInfo.fifoInfo.agpFifo)
		{
			FIFO_CACHE_FLUSH(board->fifoInfo.fifoPtr);
			*board->fifoInfo.fifoPtr++ = board->fifoInfo.fifoJmpHdr[1];
		}
	} 
	else
	{
		SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, board->fifoInfo.fifoJmpHdr[0]);	
		if(board->boardInfo.fifoInfo.agpFifo)
		{
			FIFO_CACHE_FLUSH(board->fifoInfo.fifoPtr);
			SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, board->fifoInfo.fifoJmpHdr[1]);	
		}
	}
	// Make sure all fifo data is flushed....
	BUMP_N_GRIND;
	P6FENCE;

	// Hack.. there seems to be some problem with not waiting for the wrap, so
	// until I have time to track down what that is, I'm going to just wait for it.
	do { 
		rdPtr2 = (FxU32 *) hrmHwFifoPtr(board);
		//dprintf("waiting for jump: %08lx -> %08lx\n",rdPtr2,board->fifoInfo.fifoStart);
	} while ( rdPtr2 != board->fifoInfo.fifoStart);
 	
 	board->fifoInfo.fifoPtr = (FxU32 *)board->fifoInfo.fifoStart;
 	board->fifoInfo.lastBump = board->fifoInfo.fifoPtr;
	board->fifoInfo.fifoRoom = board->fifoInfo.fifoEnd - board->fifoInfo.fifoPtr;

	//dprintf("wraproom: %d\n",board->fifoInfoRoom);
	return rdPtr;
}
#endif

void hrmFifoMakeRoom(hrmBoard_t *board, const FxU32 blockSize)
{
  hwcBoardInfo *bInfo = &board->boardInfo;
  FxU32 wrapAddr = 0x00UL;
  FxU32 shouldWrap = FXFALSE, didWrap = FXFALSE;
  FxU32 *rdPtr = (FxU32 *)0xdeadbeef;
  
	/* This code is based on the 2D driver's FIFO management code.  It's far simpler than 
	 * what Glide uses in the fullscreen case. */
	 
	
	/* If desired write won't fit in fifo, then wrap it */
	if((board->fifoInfo.fifoPtr + blockSize) > board->fifoInfo.fifoEnd)
	{
		//dprintf("wrapping fifo: %d\n",blockSize);
		rdPtr = hrmFifoWrap(board);
		shouldWrap = FXTRUE;
		//dprintf("fifo wrapped\n");
	}
	
	while(board->fifoInfo.fifoRoom < blockSize)
	{
		rdPtr = (FxU32 *)hrmHwFifoPtr(board);
		if(rdPtr == board->fifoInfo.fifoStart && shouldWrap)
			didWrap = FXTRUE;
		if(rdPtr <= board->fifoInfo.fifoPtr)
			board->fifoInfo.fifoRoom = board->fifoInfo.fifoEnd - board->fifoInfo.fifoPtr;
		else
			board->fifoInfo.fifoRoom = 0; //((FxU32)board->fifoInfoEnd - (FxU32)board->fifoInfoPtr);
			//board->fifoInfoRoom = rdPtr - board->fifoInfoPtr - 1;
		dprintf("room: %d\n",board->fifoInfo.fifoRoom);
	}
	board->fifoInfo.fifoRoom -= blockSize;
}

short hrmEnableFifo2D( short inRefNum )
{
  hrmBoard_t *board = hrmIdentifyTarget( inRefNum );
  short theSuccess = -1;
  dprintf("hrmEnableFifo2D(%d)\n",inRefNum);
  if(board) {
    dprintf("board: %08lx exclusive: %d\n",board,board->fifoInfo.exclusiveMode);
    if(board->fifoInfo.exclusiveMode) {
      hrmInitFifo(board, FXFALSE);
      board->fifoInfo.exclusiveMode = FXFALSE;
      theSuccess = 0;
    }
  }
  return theSuccess;
}

short hrmDisableFifo2D( short inRefNum )
{
  hrmBoard_t *board = hrmIdentifyTarget( inRefNum );
  short theSuccess = -1;
  dprintf("hrmDisableFifo2D(%d)\n",inRefNum);
  if(board) {
    dprintf("board: %08lx exclusive: %d\n",board,board->fifoInfo.exclusiveMode);
    if(!board->fifoInfo.exclusiveMode) {
      board->fifoInfo.exclusiveMode = FXTRUE;
      hrmDisableFifo(board);
      theSuccess = 0;
    }
  }
  dprintf("success: %d\n",theSuccess);
  return theSuccess;
}

unsigned long hrmAllocWinContext(hrmBoard_t *board)
{
  return ++board->nextContextID;
}

void hrmFreeWinContext(hrmBoard_t *board, unsigned long contextID)
{
  board = board;
  contextID = contextID;
}

short hrmExecuteWinFifo(hrmBoard_t *board, hrmExecuteFifoRequest_t *fifoReq)
{
  hwcBoardInfo *bInfo = &board->boardInfo;
  OSErr retVal = noErr;

  FxU32
    stateJmp[2],
    stateRet[2],
    cmdJmp[2],
    cmdRet[2];
  FxU32
    jmpWords;
  FxBool
    doStateP = fifoReq->contextID != board->lastContextID;
  
  /* Have to fail this */
  if(board->fifoInfo.exclusiveMode) {
    return -1;  
  }
  
  hrmArbiterGet(board);

  switch(fifoReq->fifoType)
  {
    case HWCEXT_FIFO_FB:
    {
      jmpWords = 1;

      if(doStateP) {
        stateJmp[0] = ((fifoReq->stateOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                        SSTCP_PKT0_JSR |
                        SSTCP_PKT0);
        stateRet[0] = (SSTCP_PKT0_RET |
                       SSTCP_PKT0);
      }

      cmdJmp[0] = ((fifoReq->fifoOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                    SSTCP_PKT0_JSR |
                    SSTCP_PKT0);
      cmdRet[0] = (SSTCP_PKT0_RET |
                   SSTCP_PKT0);

      goto __jmpWrite;
    }
    break;

   case HWCEXT_FIFO_AGP:
    /* AGP Support would go here */

  __jmpWrite:
    {
      FxU32 i, *clientPtr;

      P6FENCE;

      if(doStateP)  {
        /* State return to our fifo */
        clientPtr = (FxU32 *)(fifoReq->statePtr + (fifoReq->stateSize << 0x02UL));
        for(i = 0; i < jmpWords; i++)
          SET_FIFO_LFB(*clientPtr++, stateRet[i]);

        P6FENCE;

        /* Jump to state routine from our fifo */
        hrmFifoMakeRoom(board,jmpWords);
        for(i = 0; i < jmpWords; i++)
          SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, stateJmp[i]);

      }

      /* Command return to our fifo */
      clientPtr = (FxU32 *)(fifoReq->fifoPtr + (fifoReq->fifoSize << 0x02UL));
      for(i = 0; i < jmpWords; i++)
        SET_FIFO_LFB(*clientPtr++,cmdRet[i]);

      P6FENCE;

      /* Jump to commands */
      hrmFifoMakeRoom(board,jmpWords);
      for(i = 0; i < jmpWords; i++)
        SET_FIFO_LFB(*board->fifoInfo.fifoPtr++,cmdJmp[i]);

      /* Write the serial # for this fifo execution */
      hrmFifoMakeRoom(board,3);
      SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, (SSTCP_PKT5_LFB |
                                            (0x00UL << SSTCP_PKT5_BYTEN_W2_SHIFT) |
                                            (0x00UL << SSTCP_PKT5_BYTEN_WN_SHIFT) |
                                            (0x01UL << SSTCP_PKT5_NWORDS_SHIFT) |
                                            SSTCP_PKT5));
      SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, fifoReq->sentinalOffset);
      SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, fifoReq->serialNumber);
    }
    break;

  case HWCEXT_FIFO_HOST:
    {
      FxU32 *cmdBuf, *cmdBufEnd;
      FxU32 numCmds;
      FxU32 index = 0;
      FxU32 miscInit0, swizzleMode;
      
      HWC_IO_LOAD(bInfo->regInfo,miscInit0,miscInit0);
      
      swizzleMode = (miscInit0 >> 30) & 0x3;
      
      if(doStateP) {
        hrmFifoMakeRoom(board,fifoReq->stateSize );

        numCmds = fifoReq->stateSize;
        cmdBuf = (FxU32 *)fifoReq->statePtr;
        cmdBufEnd = cmdBuf + numCmds;

        /* For Voodoo3 and earlier this is a lot uglier */
        if(board->boardInfo.pciInfo.deviceID < 7) {
	        switch(swizzleMode) {
	          case 0: /* 8-bit display mode */
	          while(cmdBuf < cmdBufEnd) {
	            __swizzleWrite32_32(&(*board->fifoInfo.fifoPtr++),*cmdBuf);
	            cmdBuf++;
	          }
	          break;
	          
	          case 1: /* 32-bit display mode */
	          while(cmdBuf < cmdBufEnd) {
	            __swizzleWrite32_8(&(*board->fifoInfo.fifoPtr++),*cmdBuf);
	            cmdBuf++;
	          }
	          break;
	          
	          case 3: /* 16-bit display mode */
	          while(cmdBuf < cmdBufEnd) {
	            __swizzleWrite16_16(&(*board->fifoInfo.fifoPtr++),*cmdBuf);
	            cmdBuf++;
	          }
	          break;
	          
	          default: /* oops! */
				hrmArbiterRelease(board);
	            return -1; 
	        }
	    } else {
          while(cmdBuf < cmdBufEnd) {
            SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, *cmdBuf);
            cmdBuf++;
          }
        }

        //if(avengerHALData->fifoPtr > avengerHALData->fifoEnd) {
        //  dprintf("SHIT!!!\n");
        //}
        BUMP_N_GRIND;
      
        /* Sanity check */
        /* hrmIdleFifo(board); */
        /* dprintf("idleState: %08lx -> %08lx\n",hrmHwFifoPtr(board),board->fifoInfo.fifoPtr); */

        board->lastContextID = fifoReq->contextID;
      }

      /* Do command stream */
      hrmFifoMakeRoom(board,fifoReq->fifoSize );
      numCmds = fifoReq->fifoSize;

      cmdBuf = (FxU32 *)fifoReq->fifoPtr;
      cmdBufEnd = cmdBuf + numCmds;
      index = 0;
      
      if(board->boardInfo.pciInfo.deviceID < 7) {
	      switch(swizzleMode) {
	        case 0: /* 8-bit display mode */
	        while(cmdBuf < cmdBufEnd) {
	          __swizzleWrite32_32(&(*board->fifoInfo.fifoPtr++),*cmdBuf);
	          cmdBuf++;
	        }
	        break;
	        
	        case 1: /* 32-bit display mode */
	        while(cmdBuf < cmdBufEnd) {
	          __swizzleWrite32_8(&(*board->fifoInfo.fifoPtr++),*cmdBuf);
	          cmdBuf++;
	        }
	        break;
	        
	        case 3: /* 16-bit display mode */
	        while(cmdBuf < cmdBufEnd) {
	          __swizzleWrite16_16(&(*board->fifoInfo.fifoPtr++),*cmdBuf);
	          cmdBuf++;
	        }
	        break;
	        
	        default: /* oops! */
			  hrmArbiterRelease(board);
	          return -1;
	      }
	  } else {
        while(cmdBuf < cmdBufEnd) {
          SET_FIFO_LFB(*board->fifoInfo.fifoPtr++, *cmdBuf);
          cmdBuf++;
        }
      }

      BUMP_N_GRIND;

      /* Sanity check */
      /* hrmIdleFifo(board); */
      /* dprintf("idleCMD: %08lx -> %08lx\n",hrmHwFifoPtr(board),board->fifoInfo.fifoPtr); */

    }
    break;

  default:
    retVal = -1;
    break;

  }

  hrmArbiterRelease(board);

  return retVal;
}

short hrmGetAGPInfo(hrmBoard_t *board, FxU32 *agpLAddr, FxU32 *agpPAddr, FxU32 *agpSize)
{
  OSErr       myErr = -1;
#if 0
  CntrlParam  paramBlock;
  hwcControl_t control;

  /* Get command FIFO info for this board */
  paramBlock.ioCRefNum = (short)board->boardInfo.hdc;
  paramBlock.csCode = k3DfxGetAGPInfo; /* driver-specific status request */
  *(hwcControl_t **)(&paramBlock.csParam[0]) = &control;
	
  /* If this fails, it's okay. */
  myErr = PBControl((ParmBlkPtr)&paramBlock, false);
  if(myErr == noErr) {
    *agpLAddr = control.res.optData.getAGPInfoRes.agpLogicalAddress;
    *agpPAddr = control.res.optData.getAGPInfoRes.agpPhysicalAddress;
    *agpSize = control.res.optData.getAGPInfoRes.agpSize;
  }	
#else
  if(board->agpEnabled && board->agpAllocated) {
    *agpLAddr = (FxU32)board->agpAddress.systemLogicalAddress;
    *agpPAddr = (FxU32)board->agpAddress.agpLogicalAddress;
    *agpSize = board->agpSize;
    dprintf("hrmGetAGPInfo: %08lx %08lx %d\n",*agpLAddr,*agpPAddr,*agpSize);
    myErr = noErr;
  } else {
    dprintf("hrmGetAGPInfo: AGP not enabled for board: %08lx\n",board);
  }
#endif  
  return myErr;	
}

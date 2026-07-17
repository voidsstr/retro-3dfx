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
** $Header: HRM_Memory_Manager.c, 7, 10/11/00 8:35:01 PM, Brent$
** $Log: 
**  7    3dfx      1.2.1.1.1.1 10/11/00 Brent           Forced check in to enforce
**       branching.
**  6    3dfx      1.2.1.1.1.0 06/30/00 Critical Path   new source drop
**  5    3dfx      1.2.1.1     06/09/00 Stephane Huaulme command fifo fixes (from
**       critical path)
**  4    3dfx      1.2.1.0     05/27/00 Critical Path   new source drop
**  3    MacOS Dev Tree1.2         03/07/00 Critical Path   better mem management,
**       more qd coverage, bug fixes...
**  2    MacOS Dev Tree1.1         02/07/00 Kenneth Dyke    Fixed line endings.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 3     8/23/99 2:36p Kcd
** Surface, memory management, and AGP support.
** 
** 2     7/02/99 3:20p Kcd
** Leave memory space for cursor & fifo.
**
*/

#include <cstddef>

#include "hrm_lists.h"
#include "hrm_priv.h"
#include "hdwr_res_mgr.h"
#include "hrm_mem.h"

#include "hrm_fifo.h"
#include "hwcio.h"

#define kCursorVRAM  4096


#include "DCon.h"

#pragma mark Notes
#pragma mark -
/*********************************************************************************
	Notes:
	
	This code maintains the following invariants:
	
	For each mmArea_t:
	- All blocks in the area (both used and free) are contained in the mapList
	- The mapList is sorted by address
	- The mapList covers the entire area (there are no holes)
	- All free blocks in the area are also in the freeList
	- The freeList is sorted by address
	- The mapList will never contain two contiguous free blocks (_hrmFreeBlock will
		always fully consolidate free space, as will the heap compaction code)
	
	For each mmBlock_t:
	- If a block is allocated: 
		- its area field points to the area it belongs to
		- it is not in the free list
	- If a block is free:
		- its area is NULL
		- it is in the free list
	
	Other assumptions:
	- The allocation flags HRM_MEMF_PAGE_ALIGN and HRM_MEMF_RELOCATABLE are
		mutually exclusive.  (i.e. the heap compaction algorithm doesn't
		maintain 4k alignment when moving blocks.)
*********************************************************************************/

/*
________________________________________________________ Private Definitions ___
*/
                          
static bool _hrmCheckRoomForBlock(UInt32 blockStart, UInt32 blockSize, UInt32 size, UInt32 type);
static mmBlock_t *_hrmInvalidateMemoryBlocks(hrmBoard_t *board, FxU32 start, FxU32 size);
static bool _hrmVerifyArea(mmArea_t *area);

static inline mmBlock_t *node2block(Node_t *node)
{
	return((mmBlock_t*)(((UInt32)node) - offsetof(mmBlock_t, node)));
}

static inline mmBlock_t *freeListNode2block(Node_t *node)
{
	return((mmBlock_t*)(((UInt32)node) - offsetof(mmBlock_t, freeListNode)));
}

mmArea_t *hrmCreateMemoryArea(hrmBoard_t *board, FxU32 start, FxU32 size, FxU32 type)
{
	mmArea_t *newArea;
	mmBlock_t *newBlock;

	dprintf("hrmCreateMemoryArea(board = %08lx, start = %08lx, size = %08lx, type = %08lx\n",
	board,start,size,type);

	if(newArea = (mmArea_t *) NewPtrSys( sizeof( mmArea_t ) )) 
	{ 

		newArea->start = start;
		newArea->size = size;
		newArea->type = type;
		NewList(&newArea->freeList);
		NewList(&newArea->mapList);

		if(newBlock = (mmBlock_t *) NewPtrSys( sizeof( mmBlock_t ))) 
		{

			newBlock->start = start;
			newBlock->size = size;
			newBlock->userData = 0;
			newBlock->notifyProc = NULL;
			newBlock->comment[0] = '\0';
			newBlock->priority = HRM_MEM_PRI_ABSOLUTE;
			newBlock->flags = 0;
			newBlock->area = 0;
			newBlock->freeListNode.succ = NULL;
			newBlock->freeListNode.prev = NULL;

			AddTail(&newArea->mapList, &newBlock->node);
			AddTail(&newArea->freeList, &newBlock->freeListNode);

			AddTail(&board->memAreas, &newArea->node);
		} 
		else 
		{
			DisposePtr((Ptr)newArea);
		}
	}
	return newArea;
}

void hrmDeleteMemoryArea(mmArea_t *area)
{
	mmBlock_t *oldBlock;

	/* Unlink memory area */
	Remove(&area->node);

	/* Remove and dispose all memory blocks */
	while(oldBlock = node2block(RemHead(&area->mapList))) 
	{
		// If the block is allocated and has a notify proc,
		if(oldBlock->area && oldBlock->notifyProc) 
		{
			// call the notify proc (i.e. purge the block)
			oldBlock->notifyProc(oldBlock, HRM_MEM_NOTIFY_DISPOSE);
		}
		DisposePtr((Ptr)oldBlock);
	}

	/* Now dispose the area itself */
	DisposePtr((Ptr)area);
}

static  void _hrmAddBlockToFreeList(mmArea_t *area, mmBlock_t *block)
{
	Node_t *node;

	// Starting with the previous block, walk the map until we hit a free block.  
	node = block->node.prev;
	
	while(node->prev != NULL) 
	{
		mmBlock_t *prevBlock = node2block(node);

		// Go until we find a free block
		if(prevBlock->area == NULL)
		{
			// This is the closest previous free block.
			Insert(&prevBlock->freeListNode, &block->freeListNode);
			block = NULL;
			break;
		}
		
		node = node->prev;
	}
	
	// If no free block with a lower address was found, put the newly freed block at the head of the free list.
	if(block != NULL)
	{
		AddHead(&area->freeList, &block->freeListNode);
	}

}

/*********************
	_hrmSplitBlock()
		Split the specified number of bytes off an existing free block.  
*********************/
inline static mmBlock_t *_hrmSplitBlock(mmBlock_t *freeBlock, UInt32 size, bool splitLow)
{
	mmBlock_t *newBlock = (mmBlock_t *)NewPtrSys(sizeof(mmBlock_t));
	
	if(newBlock != NULL)
	{
		if(splitLow)
		{
			// Take the lowest-addressed part of the free block
			newBlock->start = freeBlock->start;
			newBlock->size = size;
			freeBlock->size -= size;
			freeBlock->start += size;
		
			// Insert the new block in the map before the block we just split
			Insert(freeBlock->node.prev, &newBlock->node);
		}
		else
		{
			// Take the highest-addressed part of the free block
			newBlock->start = freeBlock->start + freeBlock->size - size;
			newBlock->size = size;
			freeBlock->size -= size;
		
			// Insert the new block in the map after the block we just split
			Insert(&freeBlock->node, &newBlock->node);
		}
	}
	
	return(newBlock);
}

/*********************
	_hrmJoinBlocks()
		Join two contiguous blocks, disposing of the structure associated with one of them.  
*********************/
static void _hrmJoinBlocks(mmBlock_t *block, mmBlock_t *deadBlock)
{	
	
	if(deadBlock == node2block(block->node.succ))
	{
		// The dying block is after the existing block
		block->size += deadBlock->size;
	}
	else if(deadBlock == node2block(block->node.prev))
	{
		// The dying block is before the existing block
		block->start -= deadBlock->size;
		block->size += deadBlock->size;
	}
	else
	{
		// The blocks are not contiguous.  This is really bad.
		dprintf("_hrmJoinBlocks: called with non-contiguous blocks!\n");
		return;
	}
	
	Remove(&deadBlock->node);
	if(deadBlock->area == NULL)
	{
		Remove(&deadBlock->freeListNode);
	}
	
	DisposePtr((Ptr)deadBlock);
}

inline static void _fifoStore(hrmBoard_t *board, UInt32 val)
{
	__dcbf(board->fifoInfo.fifoPtr, -4);
	board->fifoInfo.setLfb(board->fifoInfo.fifoPtr++, val);
}

inline static void _fifoBump(hrmBoard_t *board)
{
	__dcbf(board->fifoInfo.fifoPtr, -4);
	__sync();
	HWC_CAGP_STORE(board->boardInfo.regInfo, cmdFifo0.bump, board->fifoInfo.fifoPtr - board->fifoInfo.lastBump);
	__sync();
	board->fifoInfo.lastBump = board->fifoInfo.fifoPtr;
}

static void _hrmMoveBlock(hrmBoard_t *board, UInt32 to, UInt32 from, UInt32 size)
{
#if 1
	// Move a block using the blitter.
	/*
	hrmFifoMakeRoom(board, ???);
	
	#define SET_FIFO_LFB(d, s)			board->fifoInfo.setLfb(&(d),s)

	#define FIFO_CACHE_FLUSH(d)  __dcbf(d,-4)

	#define BUMP_N_GRIND \
	{ \
	  FIFO_CACHE_FLUSH(board->fifoInfo.fifoPtr);\
	  P6FENCE;\
	  HWC_CAGP_STORE(bInfo->regInfo, cmdFifo0.bump, board->fifoInfo.fifoPtr - board->fifoInfo.lastBump);\
	  P6FENCE;\
	  board->fifoInfo.lastBump = board->fifoInfo.fifoPtr;\
	}
	*/
	#define MASK_clip0min			(1UL << 3)
	#define MASK_clip0max			(1UL << 4)
	#define MASK_dstBaseAddr		(1UL << 5)
	#define MASK_dstFormat			(1UL << 6)
	#define MASK_srcColorkeyMin		(1UL << 7)
	#define MASK_srcColorkeyMax		(1UL << 8)
	#define MASK_dstColorkeyMin		(1UL << 9)
	#define MASK_dstColorkeyMax		(1UL << 10)
	#define MASK_bresError0			(1UL << 11)
	#define MASK_bresError1			(1UL << 12)
	#define MASK_rop				(1UL << 13)
	#define MASK_srcBaseAddr		(1UL << 14)
	#define MASK_commandEx			(1UL << 15)
	#define MASK_lineStipple		(1UL << 16)
	#define MASK_lineStyle			(1UL << 17)
	#define MASK_pattern0alias		(1UL << 18)
	#define MASK_pattern1alias		(1UL << 19)
	#define MASK_clip1min			(1UL << 20)
	#define MASK_clip1max			(1UL << 21)
	#define MASK_srcFormat			(1UL << 22)
	#define MASK_srcSize			(1UL << 23)
	#define MASK_srcXY				(1UL << 24)
	#define MASK_colorBack			(1UL << 25)
	#define MASK_colorFore			(1UL << 26)
	#define MASK_dstSize			(1UL << 27)
	#define MASK_dstXY				(1UL << 28)
	#define MASK_command			(1UL << 29)

	// This function is in hrm_fifo.c, but doesn't appear to have a prototype in the headers.
	void hrmFifoMakeRoom(hrmBoard_t *board, const FxU32 blockSize);

	const UInt32 maxWidth = 1024;
	const UInt32 maxHeight = 1024;
	UInt32 blockSize;
	UInt32 blockHeight;
	UInt32 blockWidth;
	bool moveBackwards = (from < to);
		
	if(moveBackwards)
	{
		// Run the whole thing backwards
		to += size;
		from += size;
	}

	// Move large blocks as much as possible
	while(size != 0)
	{
		blockHeight = size / maxWidth;
		
		if(blockHeight != 0)
		{
			// This will be a block of one or more maximum width lines
			blockWidth = maxWidth;
			
			// The largest block we will move is maxWidth x maxHeight
			if(blockHeight > maxHeight)
				blockHeight = maxHeight;
		}
		else
		{
			// One-line block at the end
			blockHeight = 1;
			blockWidth = size;
		}
		
		blockSize = blockWidth * blockHeight;

		if(moveBackwards)
		{
			to -= blockSize;
			from -= blockSize;
		}

		hrmFifoMakeRoom(board, 9);
		
		_fifoStore(board,
				2 |     // type 2 packet
				MASK_dstBaseAddr |
				MASK_dstFormat |
				MASK_srcBaseAddr |
				MASK_srcFormat |
				MASK_srcXY |
				MASK_dstSize |
				MASK_dstXY |
				MASK_command);
		
		// dstBaseAddr
		_fifoStore(board, to & 0x03FFFFFF);

		// dstFormat
		_fifoStore(board, SSTG_PIXFMT_8BPP | maxWidth);
		
		// srcBaseAddr
		_fifoStore(board, from & 0x03FFFFFF);
		
		// srcFormat
		_fifoStore(board, SSTG_PIXFMT_8BPP | maxWidth);
		
		// srcXY
		_fifoStore(board, moveBackwards?((blockWidth - 1) | ((blockHeight - 1) << 16)):(0));
		
		// dstSize
		_fifoStore(board, blockWidth | (blockHeight << 16));
		
		// dstXY
		_fifoStore(board, moveBackwards?((blockWidth - 1) | ((blockHeight - 1) << 16)):(0));
		
		// command register
		_fifoStore(board, 
			(moveBackwards?(SSTG_XDIR | SSTG_YDIR):(0)) | 
			SSTG_BLT |
			SSTG_ROP_SRC << SSTG_ROP0_SHIFT |
			SSTG_GO);
			
		_fifoBump(board);
		
		if(!moveBackwards)
		{
			to += blockSize;
			from += blockSize;
		}

		size -= blockSize;
	}
	
#else
	// MBW -- XXX -- TAKE THIS OUT -- move the block manually. 
	
	// MBW -- XXX -- This is cheating.  This function appears to be internal to hrm_fifo.c.
	FxU32 hrmIdleFifo(hrmBoard_t *board);
	
	hrmIdleFifo(board);
	BlockMoveDataUncached((void*)from, (void*)to, size);
#endif
}

static bool _hrmCompactArea(hrmBoard_t *board, mmArea_t *area)
{
	bool result = false;
	
	mmBlock_t *freeBlock;
	mmBlock_t *iter = NULL;
	UInt32 newAddr;
	
	// MBW -- XXX -- If the fifo isn't usable, return false here.
	
	hrmArbiterGet(board);
	
	// Scan backwards through the free list
	for(	freeBlock = freeListNode2block(area->freeList.tailPred);
			freeBlock->freeListNode.prev;
			) 
	{
		if(freeBlock->node.prev->prev == NULL)
		{
			// freeBlock is the first block in the heap.  We're done.
			break;
		}
		
		if(iter == NULL)
		{
			// Start with the block right before freeBlock
			iter = node2block(freeBlock->node.prev);
	
			// Special cases for the block directly before freeBlock
			if(iter->area == NULL)
			{
				// Preceeding block is free -- consolidate.
				_hrmJoinBlocks(freeBlock, iter);
				
				// Restart the loop, continuing to look at freeBlock.
				iter = NULL;
				continue;
			}
			else if(iter->flags & HRM_BLKF_RELOCATABLE)
			{
				// Preceeding block is relocatable -- slide it up.
				result = true;
				
				// call the "pre-move" callback
				if(iter->notifyProc) 
				{
					iter->notifyProc(iter, HRM_MEM_NOTIFY_PREMOVE);
				}       
				
				// Figure out the new address of the allocated block
				newAddr = freeBlock->start + freeBlock->size - iter->size;
				
				// move the data
				_hrmMoveBlock(board, newAddr, iter->start, iter->size);
				
				// update the addresses stored in the block records
				freeBlock->start = iter->start;
				iter->start = newAddr;
				
				// exchange the block records' positions in the map
				// NOTE: This may leave multiple contiguous free blocks in the map.  This will be
				// taken care of the next time through the outer loop.
				Remove(&iter->node);
				Insert(&freeBlock->node, &iter->node);
				
				// Call the "post-move" callback
				if(iter->notifyProc) 
				{
					iter->notifyProc(iter, HRM_MEM_NOTIFY_MOVED);
				}
				
				// Restart the loop, continuing to look at freeBlock.
				iter = NULL;
				continue;
			}
			else
			{
				// Non-trivial case -- move on to the loop below
				iter = node2block(iter->node.prev);
			}
		}
		
		// Starting with this free block, scan backwards through the map
		// looking for relocatable blocks that will fit here.	
		for(	;
				iter->node.prev;
				iter = node2block(iter->node.prev))
		{
			if(iter->area == NULL)
			{
				// This block is free.  We'll get to it later.
				continue;
			}
			
			if((iter->flags & HRM_BLKF_RELOCATABLE) == 0)
			{
				// This block is not relocatable -- leave it alone.
				continue;
			}
			
			// If we got here, this is a relocatable block.
			if(iter->size > freeBlock->size)
			{
				// This block is bigger than the current free block -- leave it alone.
				continue;
			}
			
			// If we got here, iter points to a block that could be moved down into freeBlock.
			break;
		}
		
		if(iter->node.prev != NULL)
		{
			// If we got here, we want to move this block up.
			mmBlock_t *newBlock = NULL;
			
			result = true;

			
			if(iter->size != freeBlock->size)
			{
				// Split the free block.

				// call the "pre-move" callback
				if(iter->notifyProc) 
				{
					iter->notifyProc(iter, HRM_MEM_NOTIFY_PREMOVE);
				}       
			
				newBlock = _hrmSplitBlock(freeBlock, iter->size, false);
				if(newBlock == NULL)
				{
					dprintf("couldn't create new block!\n");
					return(result); /* Just bail out, the system is probably close to death anyway */
				}
				
				// Set it up as a free block
				newBlock->userData = 0;
				newBlock->notifyProc = NULL;
				newBlock->area = NULL;
				newBlock->comment[0] = '\0';
				newBlock->priority = 0;
				newBlock->flags = 0;
				newBlock->freeListNode.succ = NULL;
				newBlock->freeListNode.prev = NULL;
				
				// Figure out the new address of the allocated block
				newAddr = newBlock->start;
				
				// move the data
				_hrmMoveBlock(board, newAddr, iter->start, iter->size);
				
				// update the addresses stored in the block records
				newBlock->start = iter->start;
				iter->start = newAddr;
				
				// exchange the positions of newBlock and iter in the map
				// NOTE: This may leave multiple contiguous free blocks in the map.  This will be
				// taken care of when the outside loop walking the free list reaches one
				// of the two.
				Remove(&newBlock->node);
				Insert(&iter->node, &newBlock->node);

				Remove(&iter->node);
				Insert(&freeBlock->node, &iter->node);
				
				// and add the new free block to the free list.
				_hrmAddBlockToFreeList(area, newBlock);

				// Call the "post-move" callback
				if(iter->notifyProc) 
				{
					iter->notifyProc(iter, HRM_MEM_NOTIFY_MOVED);
				}
				
				// Next time through, start where we moved the block _from_
				iter = newBlock;
			}
			else
			{
				// If the blocks are the same size, just exchange them.

				// call the "pre-move" callback
				if(iter->notifyProc) 
				{
					iter->notifyProc(iter, HRM_MEM_NOTIFY_PREMOVE);
				}       
			
				// Since this may rearrange the free list, save the node after
				// the current free block. 
				Node_t *nextFree = freeBlock->freeListNode.succ;
				
				// Take the free block out of the free list
				Remove(&freeBlock->freeListNode);
				
				// Figure out the new address of the allocated block
				newAddr = freeBlock->start;
				
				// move the data
				_hrmMoveBlock(board, newAddr, iter->start, iter->size);
				
				// update the addresses stored in the block records
				freeBlock->start = iter->start;
				iter->start = newAddr;

				// exchange the positions of freeBlock and iter in the map
				// NOTE: This may leave multiple contiguous free blocks in the map.  This will be
				// taken care of when the outside loop walking the free list reaches one
				// of the two.
				Node_t *oldIter = iter->node.prev;
				
				Remove(&iter->node);
				Insert(&freeBlock->node, &iter->node);
				
				Remove(&freeBlock->node);
				Insert(oldIter, &freeBlock->node);

				// Add freeBlock back into the free list in the right place.
				_hrmAddBlockToFreeList(area, freeBlock);
				
				// Call the "post-move" callback
				if(iter->notifyProc) 
				{
					iter->notifyProc(iter, HRM_MEM_NOTIFY_MOVED);
				}
				
				// Continue from the next free block and reset the iterator.
				freeBlock = freeListNode2block(nextFree->prev);
				iter = NULL;
			}
		}
		else
		{
			// If we got here, nothing more can be done with this free block.  Move on to the next one.
			freeBlock = freeListNode2block(freeBlock->freeListNode.prev);
			iter = NULL;
		}
	}
	
	hrmArbiterRelease(board);
	
	return(result);
}


static mmBlock_t* _hrmFreeBlock(mmArea_t *area, mmBlock_t *block)
{
	mmBlock_t *result = block;
	
	mmBlock_t *prevBlock = (block->node.prev->prev)?(node2block(block->node.prev)):(NULL);
	mmBlock_t *nextBlock = (block->node.succ->succ)?(node2block(block->node.succ)):(NULL);
	
	/* Minor (but important) change.  Freed blocks have a NULL area pointer so we can 
		identify blocks that have already been freed for some reason and not free them
		twice. */
	block->area = 0;
	
	// Consolidate free blocks
	
	if( ((prevBlock != NULL) && (prevBlock->area == NULL)) &&
		((nextBlock != NULL) && (nextBlock->area == NULL)) )
	{
		// The blocks on either side of this one are free.  Consolidate all three.
		Remove(&block->node);
		Remove(&nextBlock->node);
		Remove(&nextBlock->freeListNode);
		
		prevBlock->size += block->size + nextBlock->size;
		
		DisposePtr((Ptr)block);
		DisposePtr((Ptr)nextBlock);
		result = prevBlock;
	}
	else if((nextBlock != NULL) && (nextBlock->area == NULL))
	{
		// The block following this one is free
		Remove(&block->node);
		
		nextBlock->size  += block->size;
		nextBlock->start -= block->size;
		
		DisposePtr((Ptr)block);
		result = nextBlock;
	}
	else if((prevBlock != NULL) && (prevBlock->area == NULL))
	{
		// The block preceeding this one is free
		Remove(&block->node);
		
		prevBlock->size  += block->size;
		
		DisposePtr((Ptr)block);
		result = prevBlock;
	}
	else
	{
		// No consolidation is possible.
		// Add the newly freed block to the free list.
		_hrmAddBlockToFreeList(area, block);
		
	}
	
	return(result);
}


inline static bool _hrmCheckRoomForBlock(UInt32 blockStart, UInt32 blockSize, UInt32 size, bool forward, bool pageAlign)
{
	// Shortcut
	if(blockSize < size)
		return(false);
		
	if(pageAlign)
	{
		UInt32 alignSize;

		// Make sure we have enough room to do the MMU page alignment.
		if(forward)
		{
			alignSize = blockStart & 4095;
			if(alignSize != 0)
			{
				alignSize = 4096 - alignSize;
			}
		}
		else
		{
			alignSize = ((blockStart + blockSize) & 4095);
		}

		if(blockSize + alignSize < size)
		{
			// There isn't room to align inside this free block.  Don't use it.
			return(false);
		}
	}
	
	return(true);
}

/*********************
	_hrmSplitBlockForAlloc()
		Split a block in the way needed by the block allocation code.  This function takes care
		of MMU page size alignment if necessary.  
*********************/
inline static mmBlock_t *_hrmSplitBlockForAlloc(mmBlock_t *freeBlock, UInt32 size, bool forward, bool pageAlign)
{
	mmBlock_t *newBlock = NULL;
	
	if(pageAlign)
	{
		UInt32 alignSize;

		// Make sure we have enough room to do the MMU page alignment.
		if(forward)
		{
			alignSize = freeBlock->start & 4095;
			if(alignSize != 0)
			{
				alignSize = 4096 - alignSize;
			}
		}
		else
		{
			alignSize = ((freeBlock->start + freeBlock->size) & 4095);
		}
		
		if(alignSize != 0)
		{
			// Split off a block (which will remain free) for alignment purposes.
			dprintf("splitting block for alignment: %08lx\n",freeBlock);
			if((newBlock = _hrmSplitBlock(freeBlock, size, forward)) == NULL)
			{
				dprintf("couldn't create new block!\n");
				return NULL; /* Just bail out, the system is probably close to death anyway */
			}

			// Insert the new block into the free list before (or after) the original.
			Insert(forward?freeBlock->freeListNode.prev:&freeBlock->freeListNode, &newBlock->freeListNode);
			
			// This new block will remain free.
			newBlock->userData = 0;
			newBlock->notifyProc = NULL;
			newBlock->area = NULL;
			newBlock->comment[0] = '\0';
			newBlock->priority = 0;
			newBlock->flags = 0;
			newBlock->freeListNode.succ = NULL;
			newBlock->freeListNode.prev = NULL;
			
			newBlock = NULL;
		}
	}

	/* If it's not a perfect size match, then we have to split it. */
	if(freeBlock->size > size) 
	{
		dprintf("splitting block: %08lx\n",freeBlock);
		if((newBlock = _hrmSplitBlock(freeBlock, size, forward)) != NULL)
		{
			dprintf("new block: %08lx\n",newBlock);			
		} 
		else 
		{
			dprintf("couldn't create new block!\n");
			return NULL; /* Just bail out, the system is probably close to death anyway */
		}
	} 
	else 
	{
		// Just use the whole block as-is
		newBlock = freeBlock;
		
		// Remove the block from the free list
		Remove(&newBlock->freeListNode);
	}
	
	return(newBlock);
}

void hrmInvalidateMemoryBlocks(hrmBoard_t *board, FxU32 start, FxU32 size)
{
	(void)_hrmInvalidateMemoryBlocks(board, start, size);
}

static mmBlock_t *_hrmInvalidateMemoryBlocks(hrmBoard_t *board, FxU32 start, FxU32 size)
{
	/* Invalidate (dispose) all memory blocks in a given range */

	mmArea_t *theArea;
	mmBlock_t *block, *result = NULL;

	dprintf("hrmInvalidateMemoryBlocks(board = %08lx, start = %08lx, size = %08lx)\n",
	board, start, size);

	theArea = (mmArea_t *)board->memAreas.head;
	while(theArea->node.succ) 
	{
		/* Walk the used list, looking for overlapping blocks */
		block = node2block(theArea->mapList.head);
		dprintf("block: %08lx succ: %08lx\n",block,block->node.succ);
		while(block->node.succ) 
		{
			/* Check for !non-overlap (it's easier) */
			dprintf("block start: %08lx size: %08lx\n",block->start,block->size);
			
			// If this block overlaps the purge area
			if(!((block->start >= (start + size)) || (start >= (block->start + block->size)))) 
			{
				// If this is not a free block
				if(block->area != NULL)
				{
					// Call the block's purge notify proc
					if(block->notifyProc) 
					{
						block->notifyProc(block, HRM_MEM_NOTIFY_DISPOSE);
					}       
					
					// and free the block.
					// NOTE:  This also consolidates free blocks on either side of the freed block
					//    and returns the block containing the memory space of the freed block
					//    (which may _not_ be the same block record as before, due to the consolidation)
					block = _hrmFreeBlock(theArea, block);
				}
				
				result = block;
			} 

			// Always continue to the next block
			block = node2block(block->node.succ);
		}

		theArea = (mmArea_t *)theArea->node.succ;
	}
	
	return(result);
}

mmBlock_t *hrmAllocateBlock(hrmBoard_t *board, FxU32 size, FxU32 type, FxU32 priority)
{
	mmArea_t *area;
	mmBlock_t *bestBlock = NULL;
	UInt32 flags = 0;
	bool forward = ((type & HRM_MEMF_REVERSE) == 0);
	bool pageAlign = ((type & HRM_MEMF_PAGE_ALIGN) != 0);
	bool heapCompacted;
		
	/* Don't let the user do something stupid... */
	if(size == 0 || board->exclusiveMode) 
	{
		return NULL;
	}

	// MBW -- XXX -- Remove this when all clients (including the driver) become priority-aware.
	if(type & (HRM_MEMF_DESKTOP | HRM_MEMF_FIXED))
	{
		// This allocation was done by a client that isn't aware of priorities.
		// The 'priority' argument is most likely garbage.

		priority = HRM_MEM_PRI_ABSOLUTE;
	}

	if(type & HRM_MEMF_RELOCATABLE)
	{
		// keep track of which blocks are relocatable
		flags |= HRM_BLKF_RELOCATABLE;
		
		// Always allocate relocatable blocks high, close to the frame buffer.  This separates them from
		// non-relocatable blocks for better heap compaction.  It also takes advantage of the fact that
		// they can move to accomidate changing frame buffer sizes instead of being purged.
		forward = false;
	}

	if(pageAlign)
	{
		/* Round block to PowerPC MMU page size */
	size = (size + 4095) & ~4095;
	}
	else
	{
		/* Round block to PowerPC L1 cache line size */
		size = (size + 31) & ~31;
	}

	/* Find an area that matches the type requirements needed */
	area = (mmArea_t *)board->memAreas.head;
	dprintf("hrmAllocateMemoryBlock(board = %08lx, size = %d, type = %08lx\n",board,size,type);

	while(area->node.succ) 
	{

		dprintf("checking area: %08lx\n",area);

		/* If all required type bits match, then we're all set. */
		if((area->type & type & 0xFFFF) == (type & 0xFFFF)) 
		{
			mmBlock_t *block;

			/* First pass, look for a big enough free block */
			dprintf("type match\n");

			heapCompacted = false;
			
			while(1)
			{
			// Loop forwards or backwards through the list, as requested.
			for(	block = freeListNode2block(forward ? area->freeList.head : area->freeList.tailPred);
					forward ? block->freeListNode.succ : block->freeListNode.prev;
					block = freeListNode2block(forward ? block->freeListNode.succ : block->freeListNode.prev)) 
			{
				if(block->size == size) 
				{
						if(pageAlign)
						{
							// Deal with the edge case where the block is exactly the right size but is misaligned.
							if((block->start & 4095) != 0)
							{
								// There isn't room to align inside this free block.  Don't use it.
								continue;
							}
						}
						
					bestBlock = block;
					
					// Since we can't find a better match than this, we might as well bail out.
					break;
				}
					else if(_hrmCheckRoomForBlock(block->start, block->size, size, forward, pageAlign))
				{
					if(bestBlock == NULL)
					{
						// We don't yet have a suitable free block.  This is the one.
						bestBlock = block;
					}
					// This block is big enough, but we'll keep going to see if we find an exact size match.
				}
			}

				// If we found a block, we're done.
				if(bestBlock != NULL)
					break;
					
				if(!heapCompacted)
				{
					heapCompacted = true;
					if(!_hrmCompactArea(board, area))
					{
						// The compaction didn't consolidate any free space.  There's no point in trying the first pass again.
						break;
					}
				}
				else
				{
					// Second time through with no luck.  Move on to the second pass.
					break;
				}
			}
			
			/* Second pass, look for a block or blocks we can purge to make room */
			/* We select based on the following criteria:
				prefer the lowest possible maximum priority
					within same max priority, prefer the lowest count of purged blocks
						within same count of purged blocks, prefer the lowest (highest) address
			*/
			if(bestBlock == NULL)
			{
				UInt32 bestPriority = HRM_MEM_PRI_ABSOLUTE;
				UInt32 bestCount = 0;
				UInt32 bestSize = 0;
				
				// Loop forwards or backwards through the used list, as requested.
				for(	block = node2block(forward ? area->mapList.head : area->mapList.tailPred);
						forward ? block->node.succ : block->node.prev;
						block = node2block(forward ? block->node.succ : block->node.prev)) 
				{
					if(_hrmCheckRoomForBlock(block->start, 
									(area->start + area->size) - block->start, 
									size, 
									forward, 
									pageAlign))
					{
						// There's room between the start of this block and the end of this heap.
						UInt32 thisPriority = 0;
						UInt32 thisCount = 0;
						UInt32 thisSize = 0;
						mmBlock_t *iter;
						
						// Get the maximum priority of all blocks that will have to be purged if we use this block.
						for(	iter = block;
								iter->node.succ;
								iter = node2block(iter->node.succ))
						{
							// If this block wouldn't need to be purged for this allocation, we're done.
							if(_hrmCheckRoomForBlock(block->start, 
									(iter->start) - block->start, 
									size, 
									forward, 
									pageAlign))
								break;
							
							if(iter->area != NULL)
							{
								// This block is not free
								thisCount++;
								thisSize += iter->size;
								if(iter->priority > thisPriority)
									thisPriority = iter->priority;
							}
							
							// Shortcut:  If the max priority gets too high, we're done.
							if(thisPriority >= priority)
								break;
						}
						
						// There are one or more blocks with higher priority than this allocation in the way.
						if(thisPriority >= priority)
							continue;
						
						if(bestBlock == NULL)
						{
							// If this is the first possible match, keep track of it.
							bestBlock = block;
						}
						else if(thisPriority < bestPriority)
						{
							// The max priority that would be purged by this allocation is lower than what we had.
							bestBlock = block;
						}
						else if(thisPriority == bestPriority)
						{
							// Check secondary criteria
							if(bestCount > thisCount)
							{
								// This allocation would purge less blocks than what we had.  
								bestBlock = block;
							}
							else if(bestCount == thisCount)
							{
								// Preference by address sorting is implicit in the order of our search.
							}
						}
						
						if(bestBlock == block)
						{
							// Save the statistics about this block.
							bestPriority = thisPriority;
							bestCount = thisCount;
							bestSize = thisSize;
										
						}
					}					
				}
				
				if(bestBlock != NULL)
				{
					// This block needs to be purged.
					bestBlock = _hrmInvalidateMemoryBlocks(board, bestBlock->start, size);
				}
			}
			
			/* If we have a valid block, then use it */
			if(bestBlock) 
			{
				mmBlock_t *newBlock;
				dprintf("using block: %08lx\n",bestBlock);

				newBlock = _hrmSplitBlockForAlloc(bestBlock, size, forward, pageAlign);
				
				/* If it's not a perfect size match, then we have to split it. */
				if(newBlock != NULL) 
				{
				// Set up the block info as an allocated block
				newBlock->userData = 0;
				newBlock->notifyProc = NULL;
				newBlock->area = area;
				newBlock->comment[0] = '\0';
				newBlock->priority = priority;
				newBlock->flags = flags;
				newBlock->freeListNode.succ = NULL;
				newBlock->freeListNode.prev = NULL;
				}

				dprintf("returning block: %08lx\n",newBlock);
				return newBlock;
			} 
		}
		/* Go try next memory area */
		dprintf("no blocks found... trying next area\n");
		area = (mmArea_t *)area->node.succ;
	}

	dprintf("no free blocks found\n");
	/* Couldn't find a block */
	return NULL;
}


void hrmFreeBlock(mmBlock_t *block)
{
	mmArea_t *area;
	dprintf("hrmFreeBlock(block = %08lx), area = %08lx\n", block, block->area);
	
	if(area = block->area) 
	{
		_hrmFreeBlock(area, block);
	} 
	else 
	{
		dprintf("block %08lx freed twice!\n",block);
	}
}

/*******************************************
	_hrmVerifyArea()
	
	This function checks the internal consistency of the data structures for an area
	in all of the paranoid and pedantic ways I can think of.  It is intended for use
	while debugging the code in this file.  No calls to this function should be left
	in shipping versions of the code.
	
*******************************************/
static bool _hrmVerifyArea(mmArea_t *area)
{
	bool result = true;
	
	Node_t *node, *freeNode;
	mmBlock_t *block, *otherBlock;

	freeNode = area->freeList.head;
	
	for(node = area->mapList.head; node->succ != NULL; node = node->succ)
	{
		if(node->succ->prev != node)
		{
			DebugStr("\pBad Map List (forward)");
			return(false);
		}

		if(node->prev->succ != node)
		{
			DebugStr("\pBad Map List (reverse)");
			return(false);
		}
		
		block = node2block(node);
		
		if(node->succ->succ != NULL)
		{
			// This is not the end of the map
			otherBlock = (mmBlock_t*)(node->succ);
			if(otherBlock->start != (block->start + block->size))
			{
				DebugStr("\pBad Map (forward)");
				return(false);
			}
		}
		else
		{
			// This is the end of the map
			if((area->start + area->size) != (block->start + block->size))
			{
				DebugStr("\pBad Map (end)");
				return(false);
			}
		}

		if(block->node.prev->prev != NULL)
		{
			// This is not the start of the map
			otherBlock = (mmBlock_t*)(node->prev);
			if( otherBlock->start + otherBlock->size != block->start)
			{
				DebugStr("\pBad Map (backward)");
				return(false);
			}
		}
		else
		{
			// This is the start of the map
			if(area->start != block->start)
			{
				DebugStr("\pBad Map (start)");
				return(false);
			}
		}		

		// Only do this for free blocks
		if(block->area == NULL)
		{
			if(freeListNode2block(freeNode) != block)
			{
				DebugStr("\pMap and free list don't match");
				return(false);
			}
			
			freeNode = freeNode->succ;
		}

	}
	
	if(freeNode->succ != NULL)
	{
		DebugStr("\pEnd of the free list isn't right");
		return(false);
	}

	return(result);
}

#ifndef __HRM_MEM_H__
#define __HRM_MEM_H__ 1

#include "hrm_lists.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mmArea_s
{
  Node_t node;
  FxU32  start;
  FxU32  size;
  FxU32  type;
  List_t freeList;		/* List of free memory blocks in this area */
  List_t mapList;		/* List of all memory blocks (free & used) in address order */
} mmArea_t;

typedef struct mmBlock_s
{
  Node_t node;          /* Reserved for internal HRM use only */
  FxU32  start;
  FxU32  size;
  
  mmArea_t *area;    
  
  /* User notification stuff */
  void *userData;
  void (*notifyProc)(struct mmBlock_s *block, unsigned long code);
  char comment[32];
  UInt32 priority;		// block priority (ignored for free blocks)
  UInt32 flags;			// miscellaneous info (ignored for free blocks)

  Node_t freeListNode;          /* Reserved for internal HRM use only */
} mmBlock_t;

enum
{
	// Flags for hrmCreateMemoryArea/hrmAllocateBlock
	HRM_MEMF_LINEAR			= 0x00000001,   // Block in linear memory
	HRM_MEMF_TILED			= 0x00000002,   // Block in tiled memory
	HRM_MEMF_DESKTOP		= 0x00000004,   // This block will always contain the desktop
	HRM_MEMF_FIXED			= 0x00010000,   // Block will never be invalidated
											/*	NOTE:  This flag is now deprecated.  Use the priority
													HRM_MEM_PRI_ABSOLUTE instead.
											*/
	HRM_MEMF_REVERSE		= 0x00020000,   // Allocate from top of heap
	HRM_MEMF_RELOCATABLE	= 0x00040000,	// Block can move when heap is compacted
	HRM_MEMF_PAGE_ALIGN		= 0x00080000,	// Align and pad this page so its boundaries match the hardware page size
											// (useful when a page will be marked cacheable, only the fifo uses this for now)
	
	// Flags for the "flags" field in the block structure
	HRM_BLKF_RELOCATABLE	= 0x00000001,	// Block can move when heap is compacted

	// Selectors passed to blocks' notifyProcs
	HRM_MEM_NOTIFY_DISPOSE	= 'dead',		// Block is about to go away
	HRM_MEM_NOTIFY_PREMOVE	= 'mvng',		// Block is about to move
	HRM_MEM_NOTIFY_MOVED	= 'mved',		// Block is done moving
	
	// Block priorities:
	HRM_MEM_PRI_ABSOLUTE	= 0xFFFFFFFF,	// Block can't ever be priority purged
	
	HRM_MEM_PRI_UNIQUE		= 0xF0000000,	// Block contains data that can't be reconstructed
	
	HRM_MEM_PRI_IMPORTANT	= 0xA0000000,	// If block is purged, something will stop working (QT, 3D, etc.)
	
	HRM_MEM_PRI_CACHE2		= 0x80000000,	// Like CACHE, but reconstructing the data is more expensive
	
	HRM_MEM_PRI_CACHE		= 0x10000000,	// Block contains cached data that can be
											//    reconstructed easily if the block is purged.
											
	HRM_MEM_PRI_SCRATCH		= 0x00000000	// Lowest possible priority (just used as scratch, 
											//    and can be reallocated whenever it's needed.)
};

typedef mmArea_t *(*hrmCreateMemoryAreaPtr)(hrmBoard_t *board, FxU32 start, FxU32 size, FxU32 type);
typedef void (*hrmDeleteMemoryAreaPtr)(mmArea_t *area);
typedef void (*hrmInvalidateMemoryBlocksPtr)(hrmBoard_t *board, FxU32 start, FxU32 size);
typedef mmBlock_t *(*hrmAllocateBlockPtr)(hrmBoard_t *board, FxU32 size, FxU32 type, FxU32 priority);
typedef void (*hrmFreeBlockPtr)(mmBlock_t *block);

// Prototypes
mmArea_t *hrmCreateMemoryArea(hrmBoard_t *board, FxU32 start, FxU32 size, FxU32 type);
void hrmDeleteMemoryArea(mmArea_t *area);
void hrmInvalidateMemoryBlocks(hrmBoard_t *board, FxU32 start, FxU32 size);
mmBlock_t *hrmAllocateBlock(hrmBoard_t *board, FxU32 size, FxU32 type, FxU32 priority);
void hrmFreeBlock(mmBlock_t *block);


#ifdef __cplusplus
}
#endif

#endif

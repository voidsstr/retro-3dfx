/* $Header: memmgr16.h, 2, 10/11/00 8:51:26 PM, Brent$ */
/*
** Copyright (c) 1998-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   memmgr16.h
**
** Description: Off-screen memory manager header file, definitions
**              and function prototypes.
**
** $Revision: 2$
** $Date: 10/11/00 8:51:26 PM$
**
** $History: memmgr16.h $
** 
** *****************  Version 23  *****************
** User: Cwilcox      Date: 2/18/99    Time: 12:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Final removal of tiled/linear promotion.
** 
** *****************  Version 22  *****************
** User: Cwilcox      Date: 2/10/99    Time: 3:52p
** Updated in $/devel/h3/Win95/dx/dd16
** Linear versus tiled promotion removal.
** 
** *****************  Version 21  *****************
** User: Michael      Date: 12/29/98   Time: 1:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** 
*/

#define PAGE_4K   4096
#define MAX_NODETABLE_ENTRIES  70
#define NODES_PER_PAGE (PAGE_4K / sizeof (struct tag_node))

#define   FAIL        0
#define   SUCCESS     1

#define   MAX_HEAPS   2  // linear & null (invalid)

//
// pNode->dwFlags
//
#define   MM_FREE                    0x1
#define   MM_DDRAW_SPACE             0x8
#define   MM_HEADER                  0x10

//
// used by getnode/freenode only!
//
#define   MM_NODE_PTR_IS_USED        0x80000000L

//
// mmAlloc return code
//
#define   MM_OK             0x1
#define   MM_OUT_OF_MEMORY  0x2
#define   MM_OUT_OF_NODES   0x4
#define   MM_INVALID_HEAPID 0x8
#define   MM_INVALID_ALLOCSIZE_STRUC 0x10

//
// DriverData.mmFlags 
//
#define   MM_FONT_CACHE_INVALID      0x1  //Set when font cache kicked out
#define   MM_IS_INITIALIZED          0x80000000 //Set when MM gets Init'ed

//
// allocation strategies
//
#define   MM_ALLOC_FROM_TOP_OF_HEAP  0x1
#define   MM_ALLOC_FROM_END_OF_HEAP  0x2
#define   MM_ALLOC_FIXED             0x4

//
// heap selection
//

typedef enum tag_mmHeapType {
    linear_heap1, any_heap, invalid_heap
} MM_HEAPTYPE;

//
// force allocated block to start on alignment boundary
//
typedef enum tag_mmAlignType {
    byte_align, word_align, dword_align, qword_align, page_align
} MM_ALIGNTYPE;

//
// allocation request block
//
typedef struct tag_mmAllocSize
{
    DWORD dwSize;        // size of this structure
    DWORD dwRequestSize; // size in bytes
    DWORD dwFlags;       // allocate from top of heap or bottom of heap
    FARPROC fpHostifyCallBackFunc;

    // put this field last, since enum is an integer value, not dword
    MM_ALIGNTYPE mmAlignType;  // alignment type for starting address
    MM_HEAPTYPE  mmHeapType;   // heap index for multiple heap support
}
MM_ALLOCSIZE, FAR *LPMM_ALLOCSIZE;

//
// node structure
//
typedef struct tag_node
{
    struct tag_node FAR *fpLeftNode;       // pointer to left node in heap
    struct tag_node FAR *fpRightNode;      // pointer to right node in heap
    struct tag_node FAR *fpLeftUsedNode;   // keep track of used nodes
    struct tag_node FAR *fpRightUsedNode;	
    struct tag_node FAR *fpTOHptr;// node with lowest offset (Top Of Heap)
    DWORD fpVidMemStart;          // starting offset in heap
    DWORD fpVidMemEnd;            // ending offset in heap
    DWORD dwBlockSize;            // size of allocated block
    DWORD dwFlags;                // bit flags
    FARPROC fpHostifyCallBackFunc;
    DWORD fpLinearAddress;        // linear address returned to caller
    DWORD	dwRefCount;             // use for tiled heap management
    DWORD	dwReserved1;            // #nodes allocated in header node,
}
MM_NODE, FAR *LPMM_NODE;

//
// heap table structure
//
typedef struct tag_mmHeapTable
{
    LPMM_NODE   pHeap;
    LPMM_NODE   pUsedList;
}
MM_HEAPTABLE, FAR *LPMM_HEAPTABLE;

//
// ALLOCFUNC is a function pointer to FxAllocMem/Align() returning LPMM_NODE
//
typedef LPMM_NODE ( 
    (* ALLOCFUNC) (LPMM_NODE, LPMM_NODE, LPMM_ALLOCSIZE, DWORD, DWORD)
    );

//
// function prototypes
//
DWORD mmAlloc(LPMM_ALLOCSIZE, DWORD *);
BOOL _loadds WINAPI mmEvict(DWORD, DWORD, DWORD);
BOOL  mmInit(void);
DWORD mmFree(DWORD);
BOOL _loadds WINAPI mmReclaim(DWORD);

LPMM_NODE FxAllocMem(LPMM_NODE, LPMM_NODE, LPMM_ALLOCSIZE, DWORD, DWORD);
LPMM_NODE FxAllocMemAlign(LPMM_NODE, LPMM_NODE, LPMM_ALLOCSIZE, DWORD, DWORD);
LPMM_NODE CoalesceFreeSpace(LPMM_NODE, LPMM_NODE);
LPMM_NODE InitHeap(DWORD, DWORD, MM_HEAPTYPE);
LPMM_NODE InitUsedList(void);
MM_HEAPTYPE GetHeapID(DWORD);

LPMM_NODE getnode(void);
BOOL freenode(LPMM_NODE);
void FreeAllNodeTablePages(void);

#ifdef MM_INTEGRITY
void CheckHeapList(LPMM_NODE);
void CheckHeapSize(LPMM_NODE);
void CheckNodeTable(void);
void CheckFreeNode(LPMM_NODE, LPMM_NODE);
void CheckAddressOverlap(LPMM_NODE);
void FindLargestFreeBlock(LPMM_NODE);
BOOL IsNodeInTable(LPMM_NODE);
#endif



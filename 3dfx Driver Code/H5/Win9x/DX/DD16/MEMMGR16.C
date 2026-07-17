/* -*-c++-*- */
/* $Header: memmgr16.c, 3, 10/11/00 8:51:25 PM, Brent$ */
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
** File name:   memmgr16.c
**
** Description: Implements the off-screen memory manager for the
**              display driver.
**
** $Revision: 3$
** $Date: 10/11/00 8:51:25 PM$
**
** $History: memmgr16.c $
** 
** *****************  Version 54  *****************
** User: Cwilcox      Date: 8/23/99    Time: 3:16p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added secondary heap to avoid tile mark problems.
** 
** *****************  Version 52  *****************
** User: Cwilcox      Date: 5/12/99    Time: 1:07p
** Updated in $/devel/h3/Win95/dx/dd16
** Implemented tiled/linear switching in 16-bit code.
** 
** *****************  Version 51  *****************
** User: Cwilcox      Date: 5/03/99    Time: 4:32p
** Updated in $/devel/h3/Win95/dx/dd16
** Changes to allow tiled/linear switching.
** 
** *****************  Version 50  *****************
** User: Cwilcox      Date: 4/22/99    Time: 4:52p
** Updated in $/devel/h3/Win95/dx/dd16
** Make STB_MMGR_TWK changes permanent, removed #ifdefs.
** 
** *****************  Version 49  *****************
** User: Stb_mimhoff  Date: 3/11/99    Time: 9:39a
** Updated in $/devel/h3/win95/dx/dd16
** Fix for PRS 4978 - Verdict hang in DDraw Surface Allocation stress test
** 
** *****************  Version 48  *****************
** User: Cwilcox      Date: 2/18/99    Time: 12:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Final removal of tiled/linear promotion.
** 
** *****************  Version 47  *****************
** User: Cwilcox      Date: 2/10/99    Time: 3:51p
** Updated in $/devel/h3/Win95/dx/dd16
** Linear versus tiled promotion removal.
** 
** *****************  Version 46  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 7:02a
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 45  *****************
** User: Michael      Date: 12/29/98   Time: 1:20p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 44  *****************
** User: Michael      Date: 12/29/98   Time: 7:29a
** Updated in $/devel/h3/Win95/dx/dd16
** Backout change from version 43.  MartyA reported that this was believed
** to have caused a GPF in Urban Assult.
** 
** *****************  Version 43  *****************
** User: Michael      Date: 12/24/98   Time: 10:18a
** Updated in $/devel/h3/Win95/dx/dd16
** ChrisE's (igx) optimization for last node used by memory manager.
** Bypass redundant overhead.  Also, add standard VSS header.
** 
*/
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

#include <memory.h>
#include "header.h"
#include "memmgr16.h"

#ifndef PERF_NEWMM


//PingZ 1/17/98
//Replace the getnode and freenode functions.
//The new approach is use a free node stack to maintain the freed nodes so that getting and
//freeing a node is a simple action of popping and pushing on the stack instead of doing a linear
//search from the entire node list.  This optimizatio cut the total Realize bitmap time by about 1/3.

DWORD *gpFreeNodeStack = NULL; //array that contains free MM_NODEs
DWORD gdwTotalFreeMMNodes = 0; //total number of free MM_NODEs, i.e., top of the stack gpFreeNodeStack

// MACROs ----------------------------------------------------------------
//
// define MACRO to align address
//
#define ROUNDUP(me, mask)   (((me) + (mask)) & (~(mask)))
#define ROUNDDN(me, mask)   ((me) & (~(mask)))

//
// define MACRO to test for out of range inputs
//
#define INRANGE(val, lo, hi)   ((val) >= (lo) && (val) <= (hi))

//
// define MACRO to remove neighbouring links from a node
//
#define RemoveNode(pNode) \
    pNode->fpLeftNode->fpRightNode = pNode->fpRightNode; \
    pNode->fpRightNode->fpLeftNode = pNode->fpLeftNode

//
// define MACRO to insert node to right of used list header,
// used list's right always point to the most recently used node
//
#ifdef MM_INTEGRITY
 #ifdef DEBUG
  #define InsertULNode(pNode) \
    pUsedList->fpRightUsedNode->fpLeftUsedNode = pNode; \
    pNode->fpLeftUsedNode = pUsedList; \
    pNode->fpRightUsedNode = pUsedList->fpRightUsedNode; \
    pUsedList->fpRightUsedNode = pNode; \
    pUsedList->dwReserved1++; \
    if (MaxUsedNodes < (WORD)pUsedList->dwReserved1) \
       MaxUsedNodes = (WORD)pUsedList->dwReserved1
 #else
  #define InsertULNode(pNode) \
    pUsedList->fpRightUsedNode->fpLeftUsedNode = pNode; \
    pNode->fpLeftUsedNode = pUsedList; \
    pNode->fpRightUsedNode = pUsedList->fpRightUsedNode; \
    pUsedList->fpRightUsedNode = pNode; \
    pUsedList->dwReserved1++   // increment node count
 #endif
#else
 #define InsertULNode(pNode) \
    pUsedList->fpRightUsedNode->fpLeftUsedNode = pNode; \
    pNode->fpLeftUsedNode = pUsedList; \
    pNode->fpRightUsedNode = pUsedList->fpRightUsedNode; \
    pUsedList->fpRightUsedNode = pNode
#endif


#ifdef DEBUG
#define DbgBrk()   _asm {int 3}
#else
#define DbgBrk()   ;
#endif

#ifdef DEBUG
WORD MemMgrOn = 1;
WORD MaxHeapNodes = 1;  //leap of faith, assume we can allocate
WORD MaxUsedNodes = 0;
WORD MaxNodePages = 1;  //leap of faith, assume we can allocate
#endif

#ifdef IFB
BOOL bInfiniteHeap= FALSE;     // almost infinite heap, no list management
#endif

//
//-- Global Statics -------------------------------------------------------
//
DWORD dwMinBlockSizeTab[page_align - byte_align + 1] = 
{ 
    (DWORD) sizeof(BYTE)       // byte_align
    ,(DWORD) sizeof(WORD)      // word_align
    ,(DWORD) sizeof(DWORD)     // dword_align
    ,(DWORD) 16L               // qword_align (16 bytes)
    ,(DWORD) 4096L             // page_align  (4096 bytes)
};

DWORD dwMinBlockSizeMaskDef;   // default mask
LPMM_NODE pAnchor;             // node returned by CoalesceFreeSpace

MM_HEAPTYPE mmDefHeapID;       // default heap to use
MM_HEAPTABLE HeapTable[MAX_HEAPS] = { NULL };

LPMM_NODE NodeTable[MAX_NODETABLE_ENTRIES] = { 0L }; // List of GlobalAlloc 16:16 ptrs

ALLOCFUNC AllocFuncTab[page_align - byte_align + 1] = 
{
     (ALLOCFUNC)FxAllocMem
    ,(ALLOCFUNC)FxAllocMemAlign
    ,(ALLOCFUNC)FxAllocMemAlign
    ,(ALLOCFUNC)FxAllocMemAlign
    ,(ALLOCFUNC)FxAllocMemAlign
};

#ifdef MM_INTEGRITY
WORD   nHeaders;               // number of permenantly allocated nodes
#endif

//
//-- External references --------------------------------------------------
//
#ifdef SSB
extern void DiscardAllSSB(void);
#endif

/*----------------------------------------------------------------------
Function name:  InitMMNodeStack

Description:    Allocates MAX_NODETABLE_ENTRIES number of system memory pages
                and put all MM_NODE addresses into free node stack
Information:

Return:         SUCCESS / FAIL
----------------------------------------------------------------------*/
BOOL InitMMNodeStack(void)
{
   UINT nPages, nNodes;
   LPMM_NODE pNode;
   HGLOBAL hNode;

   gdwTotalFreeMMNodes = 0;


   //Make sure we alloc gpFreeNodeStack only once because we never free gpFreeNodeStack memory
   if(gpFreeNodeStack == NULL)
   {    
     hNode = GlobalAlloc (GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, NODES_PER_PAGE * MAX_NODETABLE_ENTRIES * 4);
     
     if (hNode == 0L)
       return FAIL;

     gpFreeNodeStack = (DWORD *)GlobalLock (hNode);

     if(gpFreeNodeStack == NULL)
       return FAIL;
   }

   for (nPages = 0; nPages < MAX_NODETABLE_ENTRIES; nPages++)
   {
      hNode = GlobalAlloc (GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, PAGE_4K);

      // Successfully allocated?
      if (hNode == 0L)
      {
         DPF(DBGLVL_NORMAL,
         ">>> (getnode) fail to allocate a new 4K page <<<");
         return FAIL;     // ERROR: couldn't allocate
      }

      // pNode address points to head of node table
      pNode = (LPMM_NODE)GlobalLock (hNode);
      GlobalPageLock ((HGLOBAL)((DWORD)pNode >> 16));

      NodeTable[nPages] = pNode;

      //Initialize free node stack
      for (nNodes = 0; nNodes < NODES_PER_PAGE; nNodes++, pNode++)
        gpFreeNodeStack[gdwTotalFreeMMNodes++] = (DWORD)pNode;
   }

   return SUCCESS;
}   

/*----------------------------------------------------------------------
Function name:  getnode

Description:    Return the location of a free node within the node
                table.  
Information:    It gets a freed node from the free node stack

Return:         LPMM_NODE   16:16 address of the node or,
                            NULL for failure.
----------------------------------------------------------------------*/
LPMM_NODE getnode(void)
{
  LPMM_NODE pNextFreeNode;

  if(gdwTotalFreeMMNodes == 0)
  {
    DPF(DBGLVL_NORMAL,
       ">>> (getnode) Exceeded max. nodes limit <<<");
    return NULL;
  } 

  if((pNextFreeNode = (LPMM_NODE)gpFreeNodeStack[--gdwTotalFreeMMNodes]) != NULL)
  {
    gpFreeNodeStack[gdwTotalFreeMMNodes] = (DWORD)NULL; 
    pNextFreeNode->dwFlags = MM_NODE_PTR_IS_USED; // mark node as used
    return pNextFreeNode;
  }
  else
    return NULL;
}

/*----------------------------------------------------------------------
Function name:  freenode

Description:    Return the location of a free node.
Information:    A freed node is pushed on to the free node stack
                Input
                  pNodeFree 16:16 address of the node to free

Return:         BOOL    SUCCESS or FAIL
----------------------------------------------------------------------*/
BOOL freenode(LPMM_NODE pNodeFree)
{
  if(gdwTotalFreeMMNodes >= MAX_NODETABLE_ENTRIES * NODES_PER_PAGE)
  {
    DPF(DBGLVL_NORMAL,
       ">>> (getnode) Exceeded max. free nodes limit, nothing to free <<<");
    return FAIL;
  }

  memset( pNodeFree, 0, sizeof(MM_NODE) );

  gpFreeNodeStack[gdwTotalFreeMMNodes++] = (DWORD)pNodeFree;

  return SUCCESS;
}   


/*----------------------------------------------------------------------
Function name:  FreeAllNodeTablePages

Description:    Frees all system memory allocated for node table
                management.

Information:

Return:         VOID
----------------------------------------------------------------------*/
VOID FreeAllNodeTablePages()
{
   UINT Cntr1;
   LPMM_NODE pNode;
   HGLOBAL hNode;

   for (Cntr1=0; Cntr1 < MAX_NODETABLE_ENTRIES; Cntr1++)
      {
      if ((NodeTable[Cntr1]) == 0L)
         break;                     // Not allocated, we're done

      // Free the node table page
      pNode = NodeTable[Cntr1];
      hNode = (HGLOBAL)GlobalHandle ((WORD) ((DWORD)pNode >> 16));
      GlobalPageUnlock ((HGLOBAL)((DWORD)pNode >> 16));
      GlobalUnlock (hNode);
      GlobalFree (hNode);
      NodeTable[Cntr1] = 0L;
      }
}



/*----------------------------------------------------------------------
Function name:  GetHeapID

Description:    Find out which heap is the address in.

Information:
                Input
                   fpVidMemStart  linear frame buffer address

Return:         MM_HEAPTYPE  heap ID of linear_heap1 or invalid_heap.
----------------------------------------------------------------------*/
MM_HEAPTYPE GetHeapID(DWORD fpVidMemStart)
{
    DWORD dwStart, dwEnd;

    // setup linear heap1 range
    dwStart = _FF(ddLinearHeapStart) + _FF(lfbBase);
    dwEnd = dwStart + _FF(ddLinearHeapSize) + _FF(ddTiledHeapSize) + _FF(ddSecondaryHeapSize) - 1;

    if ( INRANGE(fpVidMemStart, dwStart, dwEnd) )
    {
       return (linear_heap1);
    }
    else 

    {
      return (invalid_heap);
    }
}


/*----------------------------------------------------------------------
Function name:  InitHeap

Description:    Initialize heap list.

Information:
                Input
                   dwHeapStart linear start offset of heap
                   dwHeapSize  size of heap in bytes
                   heapID      heap id

Return:         LPMM_NODE   node pointer to heap list if success,
                            null if fail
----------------------------------------------------------------------*/
LPMM_NODE InitHeap(DWORD dwHeapStart, DWORD dwHeapSize, MM_HEAPTYPE heapID)
{
    LPMM_NODE pTmp, pHeap;
    DWORD dwAlignMask;

    if (0L == dwHeapSize)
    {
       return (NULL);
    }

    pHeap = getnode();               // all fields in node are set to zero
    pTmp = getnode();                // allocate free heap
    if (pHeap && pTmp)
    {
       // align the default heap on 4K boundary, align other heaps on default
       // alignment
       dwAlignMask = (heapID == mmDefHeapID)? 4095L : dwMinBlockSizeMaskDef;
          
#ifdef MM_INTEGRITY
       nHeaders++;
#endif
       pHeap->fpLeftNode = 
       pHeap->fpRightNode = pTmp;    // circular list
       pHeap->fpTOHptr = pTmp;       // node w/ lowest offset (Top Of Heap)
       pTmp->fpLeftNode =
       pTmp->fpRightNode = pHeap;

       // initialize free heap, force starting address to dword boundary
       pHeap->fpVidMemStart = 
          ROUNDUP((dwHeapStart + _FF(lfbBase)), dwAlignMask);
       pHeap->fpVidMemEnd = 
          dwHeapStart + _FF(lfbBase) + dwHeapSize - 1;
       pHeap->dwBlockSize = pHeap->fpVidMemEnd - pHeap->fpVidMemStart + 1;
#ifdef DEBUG
       if (0 == pHeap->dwBlockSize)
       {
          DPF(DBGLVL_NORMAL, ">>> (InitHeap) heap has zero block size <<<");
          DbgBrk();
          freenode(pHeap);
          freenode(pTmp);
          return (NULL);
       }
#endif
       pHeap->dwReserved1 = 1;       // number of nodes in list
       pHeap->dwFlags |= MM_HEADER;
 
       pTmp->fpVidMemStart = 0L;     // first node
       pTmp->fpVidMemEnd = pHeap->fpVidMemEnd - pHeap->fpVidMemStart;
       pTmp->dwBlockSize = pHeap->dwBlockSize;
       pTmp->dwFlags |= MM_FREE;

       DPF(DBGLVL_MEMMGR, "(InitHeap) pHeap: %lx", pHeap);
       DPF(DBGLVL_MEMMGR, "(InitHeap) 1st FreeNode: %lx", pTmp);
       DPF(DBGLVL_MEMMGR, "(InitHeap) free heap start,end,size(%lx, %lx, %lx)"
          ,pHeap->fpVidMemStart + pTmp->fpVidMemStart
          ,pHeap->fpVidMemStart + pTmp->fpVidMemEnd
          ,pHeap->dwBlockSize);
       DPF(DBGLVL_MEMMGR,
          "(InitHeap) DriverData.HeapStart/End/Size: %lx,%lx,%lx"
          ,dwHeapStart + _FF(lfbBase)
          ,dwHeapStart + _FF(lfbBase) + dwHeapSize - 1
          ,dwHeapSize);

       return (pHeap);
    }
    else {
       if (pHeap)
       {
          freenode(pHeap);
       }
       if (pTmp)
       {
          freenode(pTmp);
       }
       return (NULL);
    }
}


/*----------------------------------------------------------------------
Function name:  InitUsedList

Description:    Initialize used list.

Information:

Return:         LPMM_NODE   node pointer to used heap list if success,
                            null if fail
----------------------------------------------------------------------*/
LPMM_NODE InitUsedList(void)
{
    LPMM_NODE pUsedList;

    if (pUsedList = getnode()) /* all fields in node are set to zero */
    {
#ifdef MM_INTEGRITY
       nHeaders++;
#endif
       pUsedList->fpLeftUsedNode = pUsedList;
       pUsedList->fpRightUsedNode = pUsedList; /* circular list */
       pUsedList->dwFlags |= MM_HEADER;

       return (pUsedList);
    }
    else {
       return (NULL);
    }
}


/*----------------------------------------------------------------------
Function name:  mmInit

Description:    Initialize the Banshee off-screen memory manager
                data structures and allocates space for fix buffers.  
Information:
    The memory manager allocates linear off-screen memory for GDI
    objects, including device bitmaps, fonts, brushes, etc. It
    allocates space for the permanent buffers, including command
    fifo, stretch blt buffer, cursor masks, and VGA. It also manages
    the use of off-screen memory by DDRAW, ensuring that GDI.

Return:         BOOL    SUCCESS or FAIL
----------------------------------------------------------------------*/
BOOL mmInit(void)
{
#ifdef IFB
    bInfiniteHeap = TRUE;
    DPF(DBGLVL_NORMAL, "!!! Infinite Heap test is on !!!");
#endif

#ifdef MM_INTEGRITY
   if (sizeof (MM_NODE) & 0x1)
      {
 #ifdef DEBUG
      DPF(DBGLVL_NORMAL,
          ">>> (mmInit) sizeof(MM_NODE) is not EVEN, disabling Mem Mgr <<<");
      MemMgrOn = 0;
 #else
      DPF(DBGLVL_NORMAL, ">>> (mmInit) sizeof(MM_NODE) is not EVEN <<<");
 #endif
      }
#endif

#ifdef DEBUG
    if (!MemMgrOn)
      {
      UINT Cntr1;

      for (Cntr1=0; Cntr1 < MAX_HEAPS; Cntr1++)
         {
         HeapTable[Cntr1].pHeap = NULL;
         HeapTable[Cntr1].pUsedList = NULL;
         }
      return (FAIL);
      }
#endif

    // Check to see if we've already been initialized
    if (_FF(mmFlags) & MM_IS_INITIALIZED)
       {
       // All GDI objects must get hostified (device bitmaps) or
       //   invalidated (Text & SSB) here.
       //** DoAllHostify() for Device Bitmaps done in Enable()
       _FF(mmFlags) |= MM_FONT_CACHE_INVALID;

#ifdef SSB
       //** DiscardAllSSB(); moved to Enable1(), see defect 1854
#endif

       // Itegrity check - check for all free nodes here

       // Free all the node table pages
       FreeAllNodeTablePages();
       }

    // Put address of DD32 memory manager callbacks in shared data area
    _FF(mmEvictAddr) = (DWORD) mmEvict;
    _FF(mmReclaimAddr) = (DWORD) mmReclaim;

#ifdef MM_INTEGRITY
       nHeaders= 0;
#endif

    if(InitMMNodeStack() == FAIL)
    {
       DPF(DBGLVL_NORMAL, ">>> InitMMNodeStack() failed <<<");
       return (FAIL);
    }

    // initialize min. alloc size
    dwMinBlockSizeMaskDef = dwMinBlockSizeTab[dword_align] - 1;

    HeapTable[linear_heap1].pUsedList = InitUsedList();

    HeapTable[linear_heap1].pHeap = InitHeap(_FF(ddLinearHeapStart), _FF(ddLinearHeapSize) + _FF(ddTiledHeapSize) + _FF(ddSecondaryHeapSize), linear_heap1);

    //
    // check for the largest heap initialization, all we need is one heap
    // to function
    //

    mmDefHeapID = linear_heap1;
    if (HeapTable[mmDefHeapID].pHeap && HeapTable[mmDefHeapID].pUsedList)
    {
       _FF(mmFlags) |= MM_IS_INITIALIZED; // successfully initialized
       return (SUCCESS);
    }
    else
    {
       DPF(DBGLVL_NORMAL, ">>> (mmInit) Fail initialization. <<<");
       return (FAIL);
    }
}


/*----------------------------------------------------------------------
Function name:  mmAlloc

Description:    Allocate space in off-screen memory.  The allocated
                space will always be rounded up to the next
                dwMinBlockSize boundary.
Information:
                Input
                   pAllocSize     pointer to request block
                   pRetCode       pointer to dword that holds the
                                  return code

Return:         DWORD   fpVidMemStart  linear address of allocated block
                        null           failed
----------------------------------------------------------------------*/
DWORD mmAlloc(LPMM_ALLOCSIZE pAllocSize, DWORD * pRetCode)
{
    BOOL  bNodeFound;
    int   nHeaps;
    DWORD dwMinBlockSizeMask, dwRequestSize;
    LPMM_NODE pNode, pHeap, pUsedList;
    MM_ALIGNTYPE mmAlignType;
    MM_HEAPTYPE  mmHeapID;
    DWORD dwUnfitBytes = (DWORD)-1;
    LPMM_NODE pTemp = NULL;

#ifdef IFB
    if (bInfiniteHeap)
    {
       pHeap = HeapTable[mmDefHeapID].pHeap;
       if (pAllocSize->dwRequestSize <= pHeap->dwBlockSize)
       {
          *pRetCode = MM_OK;
          return (pHeap->fpVidMemStart);
       }
       else
       {
          DPF(DBGLVL_NORMAL,
             ">>> (mmAlloc) out of heap space, r(%lx), h(%lx) <<<",
             pAllocSize->dwRequestSize, pHeap->dwBlockSize);
          *pRetCode = MM_OUT_OF_MEMORY;
          return (FALSE);
       }
    }
#endif

#ifdef DEBUG
    if (pAllocSize->dwSize != sizeof(MM_ALLOCSIZE))
    {
       *pRetCode = MM_INVALID_ALLOCSIZE_STRUC;
       return (FAIL);
    }
#endif

    mmAlignType = pAllocSize->mmAlignType;
    if ( !INRANGE(mmAlignType, byte_align, page_align) )
    {
       mmAlignType = dword_align; // see mmInit's default align mask setting
    }
    dwMinBlockSizeMask = (dwMinBlockSizeTab[mmAlignType] - 1);

    // ensure size meets minimum block size
    dwRequestSize = 
       ROUNDUP(pAllocSize->dwRequestSize, dwMinBlockSizeMask);

    mmHeapID = (any_heap == pAllocSize->mmHeapType) ?
       mmDefHeapID : pAllocSize->mmHeapType;

    //
    // find the heap that meets the request
    //
    for (bNodeFound = FALSE, nHeaps = 0; nHeaps < MAX_HEAPS ; nHeaps++)
    {
          pHeap = HeapTable[mmHeapID].pHeap;
          pUsedList = HeapTable[mmHeapID].pUsedList;

          if (pHeap && pUsedList)
          {  //
             // is there enough total free space in the heap?
             //
             if (pHeap->dwBlockSize >= dwRequestSize)
             {  
//PingZ 1/10/98 Instead of picking the first free block that can hold the requested
//bitmap, we walk through the entire list to find the smallest block that fit.  This 
//greately reduces the chances of fragmenting memory.
                for (pNode = pHeap->fpRightNode; pHeap != pNode;
                     pNode = pNode->fpRightNode)
                {
                   if ( (MM_FREE & pNode->dwFlags) && 
                        (pNode->dwBlockSize >= dwRequestSize) ) 
                   {
                      if((pNode->dwBlockSize - dwRequestSize) < dwUnfitBytes)
                      {
                         dwUnfitBytes = pNode->dwBlockSize - dwRequestSize;
                         pTemp = pNode;
                      }
                   }
                }  

                if(pTemp)
                {
                   pNode = pTemp;
                   bNodeFound = TRUE;
                   nHeaps = MAX_HEAPS;  // terminate outer for loop
                }
             }  // endif: is total heap big enough?
#ifdef DEBUG
             if (nHeaps < MAX_HEAPS)
             {
 #ifdef MM_INTEGRITY
                FindLargestFreeBlock(pHeap);
 #endif
                DPF(DBGLVL_MEMMGR, "... no vacancy in heap %d ...", mmHeapID);
                DPF(DBGLVL_MEMMGR,
                   "... dwBlockSize %lx, requesting %lx, try next heap ...",
                   pHeap->dwBlockSize, dwRequestSize);
             }
#endif
          }  // endif: is heap pointers valid?

       mmHeapID++;                      // try another heap
       mmHeapID &= (MAX_HEAPS - 1);     // ensure it is in range

    }  // walk all heaps

    if (bNodeFound)
    {  //
       // allocate from the free node that we found
       //
       pNode = (* AllocFuncTab[mmAlignType])
          (pHeap, pNode, pAllocSize, dwRequestSize, dwMinBlockSizeMask);

       if (NULL != pNode)
       {
          InsertULNode(pNode);
          *pRetCode = MM_OK;

#ifdef MM_INTEGRITY
          CheckAddressOverlap(pHeap);
          DPF(DBGLVL_MEMMGR,
             "(mmAlloc) [%d] %lx->b(%lx),e(%lx),s(%lx); %ld nodes"
#else
          DPF(DBGLVL_MEMMGR, "(mmAlloc) [%d] %lx->b(%lx),e(%lx),s(%lx)"
#endif
             ,mmHeapID - 1
             ,pNode
             ,pNode->fpLinearAddress
             ,pHeap->fpVidMemStart + pNode->fpVidMemEnd
             ,pNode->dwBlockSize
#ifdef MM_INTEGRITY
             ,pHeap->dwReserved1
#endif
             );

#ifdef DEBUG
          // ensure allocated address is aligned
          if (pNode->fpLinearAddress & dwMinBlockSizeMask)
          {
             DPF(DBGLVL_NORMAL,
                ">>> mmAlloc : %lx is not aligned to %ld bytes <<<",
                pNode->fpLinearAddress, dwMinBlockSizeMask + 1);
          }
#endif
          return (pNode->fpLinearAddress);
       }
       else 
       {
          DPF(DBGLVL_NORMAL, ">>> mmAlloc : out of nodes <<<");

          *pRetCode = MM_OUT_OF_NODES;
          return (FAIL); // AllocMem failed, return error
       }
    }
    else {   // no free space found
       DPF(DBGLVL_MEMMGR, ">>> mmAlloc : out of memory <<<");

       *pRetCode = MM_OUT_OF_MEMORY;
       return (FAIL);
    }
}


/*----------------------------------------------------------------------
Function name:  mmFree

Description:    Frees an allocated GDI space, unlinks hook from the
                used list, and marks node as free.  Also, perform
                coalesce on the heap list.  Do not explicitly free up
                the node in this function.  A node is freed as a
                result of coalescing two nodes into one. 
Information:
                Input
                    fpVidMemStart  linear address returned by
                                   mmAlloc() return code

Return:         DWORD   size of newly coalesced node or,
                        failure if used list is empty or cannot find
                        node.
----------------------------------------------------------------------*/
DWORD mmFree(DWORD fpVidMemStart_arg)
{
    LPMM_NODE pNode, pHeap, pUsedList;
    MM_HEAPTYPE heapID;

#ifdef IFB
    if (bInfiniteHeap)
    {
       pHeap = HeapTable[mmDefHeapID].pHeap;
       return (pHeap->dwBlockSize);
       
    }
#endif

    heapID = GetHeapID(fpVidMemStart_arg);
    pHeap = HeapTable[heapID].pHeap;
    pUsedList = HeapTable[heapID].pUsedList;

    // unlikely to happen, just in case...
    if (!pHeap || !pUsedList)
    {
       DPF(DBGLVL_MEMMGR, ">>> mmFree : mmInit() failed. <<<");
       return (FAIL);
    }

#ifdef MM_INTEGRITY
    CheckNodeTable();
#endif

    if (0L == fpVidMemStart_arg)
    {
       DPF(DBGLVL_NORMAL,
          ">>> mmFree : invalid argument :%lx <<<", fpVidMemStart_arg);
       return (FAIL);
    }

    // if used list is empty, fall out of loop and return failure
    for ( pNode = pUsedList->fpRightUsedNode; pUsedList != pNode; 
          pNode = pNode->fpRightUsedNode )
    {
       if (pNode->fpLinearAddress == fpVidMemStart_arg)
       {
          // link up adjacent used nodes
          pNode->fpLeftUsedNode->fpRightUsedNode = pNode->fpRightUsedNode;
          pNode->fpRightUsedNode->fpLeftUsedNode = pNode->fpLeftUsedNode;
          pNode->fpLeftUsedNode =
          pNode->fpRightUsedNode = NULL;// unlink node from used list
          pNode->dwFlags &= ~MM_DDRAW_SPACE;
          pNode->dwFlags |= MM_FREE;    // mark node as free
          pNode->dwRefCount = 0L;       // clear reference count
          pHeap->dwBlockSize += pNode->dwBlockSize;
#ifdef DEBUG
          pNode->dwReserved1 = 0xBEEFBEEF; // easy mark for free node
#endif
          DPF(DBGLVL_MEMMGR, "(mmFree) %lx->b(%lx),e(%lx),s(%lx)"
             ,pNode
             ,pNode->fpLinearAddress
             ,pHeap->fpVidMemStart + pNode->fpVidMemEnd
             ,pNode->dwBlockSize);

          // combine free space on either side of pNode
          pAnchor = CoalesceFreeSpace(pHeap, pNode);

#ifdef DEBUG
          if (pAnchor == pHeap)
          {
             (">>> mmFree: pAnchor == pHeap, tell me it ain't so, Joe <<<");
          }
#endif
#ifdef MM_INTEGRITY                     // integrity check here
          pUsedList->dwReserved1--;     // one less node to deal with
          CheckHeapSize(pHeap);
#endif

          DPF(DBGLVL_MEMMGR,
             "(mmFree) coalesced block %lx->b(%lx),e(%lx),s(%lx)"
             ,pAnchor
             ,pHeap->fpVidMemStart + pAnchor->fpVidMemStart
             ,pHeap->fpVidMemStart + pAnchor->fpVidMemEnd
             ,pAnchor->dwBlockSize);

          return (pAnchor->dwBlockSize);
       } 
    } // walk used list

    DPF(DBGLVL_NORMAL, ">>> mmFree : node not found <<<");
    return (FAIL); // node not found or nothing to free
}


/*----------------------------------------------------------------------
Function name:  mmEvict

Description:    Called from DDRAW's memMgr_allocSurface() routine.
                DDRAW is going to allocate in the space starting
                at hwPtrStart, we must move objects to host memory
                or other off-screen area if this space is occupied
                so that the content will not be overwritten by DDRAW.
Information:
                Input
                  hwPtrStart     start address (ranges 0 to max
                                 video memory)
                  hwPtrEnd       end addressReturn
                  dwBlockSize    size of DDRAW space in bytes

Return:         BOOL    SUCCESS or FAIL
----------------------------------------------------------------------*/
BOOL _loadds WINAPI mmEvict(DWORD hwPtrStart, DWORD hwPtrEnd,
    DWORD dwBlockSize)
{
    DWORD          dwTemp;
    DWORD          dwRefCountOld;
    DWORD          fpVidMemStart, fpVidMemEnd;   // start/end offset in heap
    LPMM_NODE      pNew, pNode, pStart; // pointer to nodes
    LPMM_NODE      pHeap, pUsedList;
    MM_ALLOCSIZE   AllocSize;
    MM_HEAPTYPE    heapID;

    heapID = GetHeapID(hwPtrStart + _FF(lfbBase));
    pHeap = HeapTable[heapID].pHeap;
    pUsedList = HeapTable[heapID].pUsedList;

    if (!pHeap || !pUsedList)
    {
       DPF(DBGLVL_NORMAL,
          ">>> mmEvict: null pHeap or pUsedList due to bad heapID <<<");
       return (FAIL);
    }

    // compute start/end address relative to start of heap
    switch (heapID)
    {
       case linear_heap1:
          fpVidMemStart = hwPtrStart - _FF(ddLinearHeapStart);
          fpVidMemEnd = hwPtrEnd - _FF(ddLinearHeapStart);
          break;

       default:
          DPF(DBGLVL_NORMAL,
             ">>> mmEvict: bad hwPtrStart(%lx) gives invalid heapID <<<",
             hwPtrStart);
          return(FAIL);
    }

#ifdef DEBUG
    if ( (hwPtrEnd + _FF(lfbBase)) > pHeap->fpVidMemEnd )
    {
       DPF(DBGLVL_NORMAL,
          ">>> (mmEvict): bad input: hwPrtEnd(%lx) > heap end(%lx) <<<",
          hwPtrEnd, pHeap->fpVidMemEnd);
       return (FAIL);
    }
#endif

#ifdef MM_INTEGRITY
    CheckHeapList(pHeap);
#endif

    // walk the heap and move GDI space to host memory
    pNode = pHeap->fpTOHptr;      // start at top of heap: lowest address
    do
    {
       if ( (pHeap != pNode) &&
            INRANGE(fpVidMemStart, pNode->fpVidMemStart, pNode->fpVidMemEnd)
          )
       {
          pStart = pNode;               // save this
          dwRefCountOld = 0L;

#ifdef DEBUG
          if ( (pStart->dwRefCount > 0L) &&
               !(MM_DDRAW_SPACE & pStart->dwFlags)
             )
          {
             DPF(DBGLVL_NORMAL,
                ">>> (mmEvict) Invalid ref. count in non DDRAW space. <<<");
             DbgBrk();
          }
#endif

     
          if (pStart->dwRefCount > 0L)  // other tiled DDRAW objects in node?
          {
             if (fpVidMemEnd <= pStart->fpVidMemEnd)
             {
                pStart->dwRefCount++;   // fits w/i same node

                DPF(DBGLVL_MEMMGR, "(mmEvict) bump: c[%lx] %lx->b(%lx),e(%lx)"
                   ,pStart->dwRefCount
                   ,pStart
                   ,pHeap->fpVidMemStart + pStart->fpVidMemStart
                   ,pHeap->fpVidMemStart + pStart->fpVidMemEnd);
                DPF(DBGLVL_MEMMGR, "(mmEvict) entry: b(%lx), e(%lx)"
                   ,pHeap->fpVidMemStart + fpVidMemStart
                   ,pHeap->fpVidMemStart + fpVidMemEnd);

                return (SUCCESS);
             }
             else                 // need to grow the DDRAW node
             {
                DPF(DBGLVL_MEMMGR, "(mmEvict) grow the node...");

                dwRefCountOld = pStart->dwRefCount;
             }
          }
          
          do // evict objects (one to many nodes) to make room for DDRAW
          {
             if ( !((MM_HEADER | MM_FREE) & pNode->dwFlags) )
             {
                if (pNode->fpHostifyCallBackFunc != (FARPROC)NULL)
                {
                   (*pNode->fpHostifyCallBackFunc)(pHeap->fpVidMemStart +
                        pNode->fpVidMemStart);

                   DPF(DBGLVL_MEMMGR, "(mmEvict) call hostify");
                }
                else  // no callback, we'll overwrite it regrettably
                {
                   mmFree(pHeap->fpVidMemStart + pNode->fpVidMemStart);

                   DPF(DBGLVL_MEMMGR,
                      "(mmEvict) callbackFunc is null, free anyway!");
                }

                pNode = pAnchor->fpRightNode;
             }
             else {
                pNode = pNode->fpRightNode;
             }

             if (pNode == pHeap->fpTOHptr)
                break;

          } while (fpVidMemEnd >= pNode->fpVidMemStart);

          // PRS 4978
          // pStart is the node we started with; in some instances, this node
          // gets coalesced with a node to the left, freeing the pStart node,
          // which causes pStart->* to be set to 0, so, we need to detect
          // this case and reset pStart - I think it should get set the the
          // left node of the current node, because that will be the one that
          // the original pStart was folded into; no valid blocks will ever
          // have a fpVidMemEnd of 0, so that how I detect this case
          if (pStart->fpVidMemEnd == 0)
              pStart = pNode->fpLeftNode;

          AllocSize.dwSize = sizeof(MM_ALLOCSIZE);

          // blocksize + size on left when DDRAW starts in middle of free node

          dwTemp = dwBlockSize + (fpVidMemStart - pStart->fpVidMemStart);

          // size we reserve for DDRAW cannot exceed free node's size

          AllocSize.dwRequestSize = (dwTemp > pStart->dwBlockSize)?
             pStart->dwBlockSize : dwTemp;

          AllocSize.dwFlags = MM_ALLOC_FROM_TOP_OF_HEAP;
          AllocSize.mmAlignType = byte_align; // don't change DDRAW alignment
          AllocSize.mmHeapType = heapID;
          AllocSize.fpHostifyCallBackFunc = NULL;
          pNew = 
             FxAllocMem(pHeap,pStart,&AllocSize,AllocSize.dwRequestSize,0L);
#ifdef DEBUG
          // PRS 4978
          // While running verdict, we run into cases where the DirectDraw 
          // heap mangler allocates offscreen memory larger than what we think
          // we can give; therefore, after we've freed all blocks after the
          // start of the block, we still don't have enough free space; 
          // however, continuing on having only freed what was available
          // appears to work - at least verdict doesn't complain :) this might
          // be a bigger issue that needs studied
#ifdef INVALID
          if ( (NULL == pNew) || (pNew->dwBlockSize < dwBlockSize) )
#else
          if (NULL == pNew)
#endif
          {
             DbgBrk();      // should never fail
             return (FAIL); // if we do, fail gracefully
          }
#endif
          pNew->dwFlags |= MM_DDRAW_SPACE; // mark node as used
          pNew->dwRefCount = dwRefCountOld + 1;
          InsertULNode(pNew);

          DPF(DBGLVL_MEMMGR,
             "(mmEvict) allocated: c[%lx] %lx->b(%lx),e(%lx),s(%lx)"
             ,pNew->dwRefCount
             ,pNew
             ,pHeap->fpVidMemStart + pNew->fpVidMemStart
             ,pHeap->fpVidMemStart + pNew->fpVidMemEnd
             ,pNew->dwBlockSize);
          DPF(DBGLVL_MEMMGR, "(mmEvict) entry: b(%lx), s(%lx)",
             pHeap->fpVidMemStart + fpVidMemStart, dwBlockSize);

          return (SUCCESS);

       } // find first node where DDRAW starts to occupy

       pNode = pNode->fpRightNode;

    } // walk heap
    while (pHeap->fpTOHptr != pNode);

    DPF(DBGLVL_NORMAL, ">>> mmEvict : node at %lx not found <<<", 
       pHeap->fpVidMemStart + fpVidMemStart);

    return (FAIL);
}


/*----------------------------------------------------------------------
Function name:  mmReclaim

Description:    Called from DDRAW's memMgr_freeSurface() routine.
                The space is marked free and can be used as both
                GDI space or DDRAW space.
Information:
                Input
                  hwPtrStart     start address (ranges 0 to max
                                 video memory)

Return:         BOOL    SUCCESS or FAIL
----------------------------------------------------------------------*/
BOOL _loadds WINAPI mmReclaim(DWORD hwPtrStart)
{
    DWORD       fpVidMemStart;    // linear offset in heap
    LPMM_NODE   pNode, pHeap, pUsedList;
    MM_HEAPTYPE heapID;

    heapID = GetHeapID(hwPtrStart + _FF(lfbBase));
    pHeap = HeapTable[heapID].pHeap;
    pUsedList = HeapTable[heapID].pUsedList;

    if (!pHeap || !pUsedList)
    {
       DPF(DBGLVL_NORMAL
          ,">>> mmReclaim: null pHeap or pUsedList due to bad heapID <<<");
       return (FAIL);
    }

    // compute start/end address relative to start of heap
    switch (heapID)
    {
       case linear_heap1:
          fpVidMemStart = hwPtrStart - _FF(ddLinearHeapStart);
          break;

       default:
          DPF(DBGLVL_NORMAL,
             ">>> mmReclaim: bad hwPtrStart(%lx) gives invalid heapID <<<",
             hwPtrStart);
          return(FAIL);
    }

    // if used list is empty, fall out of loop and return failure
    for ( pNode = pUsedList->fpRightUsedNode; pUsedList != pNode; 
          pNode = pNode->fpRightUsedNode )
    {
       if ( INRANGE(fpVidMemStart, pNode->fpVidMemStart, pNode->fpVidMemEnd) )
       {
          if ( (MM_DDRAW_SPACE & pNode->dwFlags)
             )
          {
             pNode->dwRefCount--;
#ifdef DEBUG
             if (pNode->dwRefCount < 0L)
             {
                DPF(DBGLVL_NORMAL, ">>> (mmReclaim) reference < 0 <<<");
             }
#endif
             DPF(DBGLVL_MEMMGR, "(mmReclaim) c[%lx] %lx->b(%lx),e(%lx),s(%lx)"
                ,pNode->dwRefCount
                ,pNode
                ,pHeap->fpVidMemStart + pNode->fpVidMemStart
                ,pHeap->fpVidMemStart + pNode->fpVidMemEnd
                ,pNode->dwBlockSize);

             if (0L == pNode->dwRefCount)
             {
                DPF(DBGLVL_MEMMGR, "(mmReclaim) DDRAW free: %lx", pNode);
                mmFree(pHeap->fpVidMemStart + pNode->fpVidMemStart);
             }
             return (SUCCESS);
          }
          else
          {
             DPF(DBGLVL_NORMAL,
                ">>> (mmReclaim) try to free a non DDRAW object <<<");
             return (FAIL); // try to free a non DDRAW object
          }
       } // find first node where DDRAW starts to occupy
    } // walk used list

    DPF(DBGLVL_NORMAL, ">>> (mmReclaim) node at %lx not found <<<", 
       pHeap->fpVidMemStart + fpVidMemStart);

    return (FAIL); // cannot find node
}


/*----------------------------------------------------------------------
Function name:  CoalesceFreeSpace

Description:    Combine free spaces in adjacent nodes into a
                larger contiguous space.
Information:
                Input
                  pHeap pointer to linear/tiled heap
                  pNode pointer to the node to work on

Return:         LPMM_NODE   pointer to node that contains the
                            coalesced space.
----------------------------------------------------------------------*/
//
// define MACRO to ensure used list pointers are null at coalescing,
// we should not coalesce if the node is still linked to used list
//
#ifdef DEBUG
 #define CheckUsedPtr(pNode) \
    if ( (NULL != pNode->fpLeftUsedNode) || \
         (NULL != pNode->fpRightUsedNode) \
       ) \
    { \
       DbgBrk();  /* should never get here! */ \
    }
#else
 #define CheckUsedPtr(pNode)
#endif

//
// define MACRO to coalesce free space between pNode and node on its right
//
#define CoalesceRightNode(pNode, pRight) \
    pNode->fpVidMemEnd = pRight->fpVidMemEnd; \
    pNode->dwBlockSize += pRight->dwBlockSize; \
    pNode->fpRightNode = pRight->fpRightNode; \
    pRight->fpRightNode->fpLeftNode = pNode


LPMM_NODE CoalesceFreeSpace(LPMM_NODE pHeap, LPMM_NODE pNode)
{    
    LPMM_NODE  pLeft, pRight, pNew;  // pointer to nodes
#ifdef DEBUG
    DWORD dwOldHeapSize = pHeap->dwBlockSize;
#endif

    pLeft = pNode->fpLeftNode;
    pRight = pNode->fpRightNode;
    pNew = pNode;  // the newly combined node

    //
    // merge pNode and its right free node
    //
    if ( (pRight != pHeap) && 
         (MM_FREE & pRight->dwFlags) &&
         (pNode->fpVidMemEnd + 1 == pRight->fpVidMemStart)
       )
    {
       CheckUsedPtr(pRight);
       CoalesceRightNode(pNode, pRight);
       freenode(pRight);       // free a node

#ifdef MM_INTEGRITY
       pHeap->dwReserved1--;   // one less node to deal with
       CheckFreeNode(pHeap, pRight);
       CheckAddressOverlap(pHeap);
#endif
    }

    //
    // merge pLeft and its right free node
    //
    if ( (pLeft != pHeap) && 
         (MM_FREE & pLeft->dwFlags) &&
         (pLeft->fpVidMemEnd + 1 == pNode->fpVidMemStart)
       )
    {
       CheckUsedPtr(pLeft);
       CoalesceRightNode(pLeft, pNode);
       freenode(pNode);        // free a node
       pNew = pLeft;

#ifdef MM_INTEGRITY
       pHeap->dwReserved1--;   // one less node to deal with
       CheckFreeNode(pHeap, pNode);
       CheckAddressOverlap(pHeap);
#endif
    }

    //
    // coalesce left and right free nodes on either side of pHeap
    //
    pLeft = pHeap->fpLeftNode;
    pRight = pHeap->fpRightNode;
    if ( (MM_FREE & pLeft->dwFlags) &&
         (MM_FREE & pRight->dwFlags) &&
         (pLeft->fpVidMemStart != pRight->fpVidMemStart)
       )
    {
       if (pLeft->fpVidMemEnd + 1 == pRight->fpVidMemStart)
       {
          // move heap pointer to left of pLeft
          RemoveNode(pHeap);
          pHeap->fpRightNode = pLeft;
          pHeap->fpLeftNode = pLeft->fpLeftNode;
          pHeap->fpLeftNode->fpRightNode = pHeap;
          pLeft->fpLeftNode = pHeap;
          
          CheckUsedPtr(pLeft);
          CheckUsedPtr(pRight);
          CoalesceRightNode(pLeft, pRight);
          freenode(pRight);    // free a node
          pNew = pLeft;

#ifdef MM_INTEGRITY
          pHeap->dwReserved1--;// one less node to deal with
          CheckFreeNode(pHeap, pRight);
          CheckAddressOverlap(pHeap);
#endif
       }
       else {
          if (pRight->fpVidMemEnd + 1 == pLeft->fpVidMemStart)
          {
             CheckUsedPtr(pLeft);
             CheckUsedPtr(pRight);
             CoalesceRightNode(pRight, pLeft);
             freenode(pLeft);    // free a node
             pNew = pRight;

#ifdef MM_INTEGRITY
             pHeap->dwReserved1--;// one less node to deal with
             CheckFreeNode(pHeap, pLeft);
             CheckAddressOverlap(pHeap);
#endif
          }
       }
    }  // coalesce left and right free node on either side of pHeap

#ifdef DEBUG
    if (pHeap->dwBlockSize > dwOldHeapSize)
    {
       DPF(DBGLVL_NORMAL, ">>> (mmCoalesceFreeSpace): heap has grown! <<<");
       DbgBrk();
    }
    else {
        if (pHeap->dwBlockSize < dwOldHeapSize)
        {
          DPF(DBGLVL_NORMAL, ">>> (mmCoalesceFreeSpace): heap has shrunk! <<<");
          DbgBrk();
        }
    }
#endif

    pNew->dwFlags |= MM_FREE;  // mark node as free
    return (pNew);
}


/*----------------------------------------------------------------------
Function name:  FxAllocMem

Description:    mmAlloc's main work horse.  If request size is the
                same as the free node  size, mark node as used and
                return.  If request size is less then the free node
                size, only take what's asked for (rounded up to
                the nearest granularity). 
Information:
Input
  pHeap       pointer to heap list
  pNode       pointer to free node
  pAllocSize  pointer to request block
  dwRequestSize  request size rounded up to dwMinBlockSize
  dwMinBlockSizeMask alignment mask
  
Return:         LPMM_NODE   pointer to allocated memory
----------------------------------------------------------------------*/
LPMM_NODE FxAllocMem(LPMM_NODE pHeap, LPMM_NODE pNode,
    LPMM_ALLOCSIZE pAllocSize, DWORD dwRequestSize, DWORD dwMinBlockSizeMask)
{
    DWORD dwUnusedSize;  // size remaining in free block
    LPMM_NODE   pNew;    // pointer to nodes

#ifdef DEBUG
    if (0 != dwMinBlockSizeMask)
    {
       DPF(DBGLVL_NORMAL,
          ">>> (FxAllocMem): should call FxAllocMemAlign instead <<<");
       DbgBrk();
       return (NULL);    // fail the alloc call
    }

    if (dwRequestSize > pNode->dwBlockSize)
    {
       DPF(DBGLVL_NORMAL, ">>> (FxAllocMem): should never get here! <<<");
       DbgBrk();
       return (NULL);    // fail the alloc call
    }
#endif

    pNew = pNode;
    dwUnusedSize = pNode->dwBlockSize - dwRequestSize;

#ifdef MM_INTEGRITY
    if ( dwUnusedSize && (dwUnusedSize < (dwMinBlockSizeMask+1)) )
    {
       // this free block can never be used, sigh
       DPF(DBGLVL_MEMMGR,
          "- - (FxAllocMem): dwUnusedSize (%lx) < dwMinBlockSize (%lx) - -",
          dwUnusedSize, dwMinBlockSizeMask+1);
    }
#endif

    //
    // if dwUnusedSize is zero, perfect fit, return pointer to node
    // if dwUnusedSize > zero, carve free memory into two pieces
    //
    if (dwUnusedSize > 0L)
    {
       pNew = getnode();
       if (NULL == pNew)
          return (NULL); // out of nodes, fail the alloc call

       pNew->dwBlockSize = dwRequestSize;
#ifdef DEBUG
       if ( pNode->dwBlockSize != (pNew->dwBlockSize + dwUnusedSize) )
       {
          DPF(DBGLVL_NORMAL,
             ">>> (FxAllocMem): block size(%lx) != split size(%lx) <<<",
             pNode->dwBlockSize, pNew->dwBlockSize + dwUnusedSize);
       }
#endif
       pNode->dwBlockSize = dwUnusedSize;  // set unused size

       if (MM_ALLOC_FROM_TOP_OF_HEAP & pAllocSize->dwFlags)
       {
          pNew->fpVidMemStart = pNode->fpVidMemStart;
          pNew->fpVidMemEnd = pNode->fpVidMemStart + dwRequestSize - 1;
          pNode->fpVidMemStart = pNew->fpVidMemEnd + 1;

          // ensure fpTOHptr points to node w/ lowest starting address
          if (pHeap->fpTOHptr == pNode)
             pHeap->fpTOHptr = pNew;

          // link new node to the left of current node
          pNode->fpLeftNode->fpRightNode = pNew;
          pNew->fpLeftNode = pNode->fpLeftNode;
          pNew->fpRightNode = pNode;
          pNode->fpLeftNode = pNew;

          // move header to the right of pNew for better distribution
          RemoveNode(pHeap);
          pHeap->fpLeftNode = pNew;
          pHeap->fpRightNode = pNode;
          pNew->fpRightNode = pHeap;
          pNode->fpLeftNode = pHeap;
       }
       else  // default is to allocate from bottom of heap
       {
          pNew->fpVidMemStart = pNode->fpVidMemStart + dwUnusedSize;
          pNew->fpVidMemEnd = pNode->fpVidMemEnd;
          pNode->fpVidMemEnd = pNew->fpVidMemStart - 1;

          // link new node to the right of current node
          pNode->fpRightNode->fpLeftNode = pNew;
          pNew->fpLeftNode = pNode;
          pNew->fpRightNode = pNode->fpRightNode;
          pNode->fpRightNode = pNew;

          if (pNode->fpLeftNode != pHeap)
          {
             // move header to the left of pNode for better distribution
             RemoveNode(pHeap);
             pHeap->fpLeftNode = pNode->fpLeftNode;
             pHeap->fpRightNode = pNode;
             pNode->fpLeftNode->fpRightNode = pHeap;
             pNode->fpLeftNode = pHeap;
          }
       }

#ifdef MM_INTEGRITY
       pHeap->dwReserved1++;   // add one node to heap list
 #ifdef DEBUG
       if (MaxHeapNodes < (WORD)pHeap->dwReserved1)
          MaxHeapNodes = (WORD)pHeap->dwReserved1;
 #endif
       CheckHeapList(pHeap);
#endif
    }  // done adding new node to heap

#ifdef DEBUG
    if (pNew->fpVidMemStart & dwMinBlockSizeMask)
    {
       DPF(DBGLVL_NORMAL, ">>> (FxAllocMem): unaligned start addr. <<<");
    }
#endif

    pHeap->dwBlockSize -= dwRequestSize;
    pNew->dwFlags &= ~MM_FREE; // mark node as used
    pNew->fpHostifyCallBackFunc = pAllocSize->fpHostifyCallBackFunc;
    pNew->fpLinearAddress = pHeap->fpVidMemStart + pNew->fpVidMemStart;
    return (pNew);
}


/*----------------------------------------------------------------------
Function name:  FxAllocMemAligned

Description:    mmAlloc's main work horse.  If request size is the
                same as the free node size, mark node as used and
                return.  If request size is less then the free node
                size, only take what's asked for (rounded up to
                the nearest granularity). 

Information:
Input
  pHeap       pointer to heap list
  pNode       pointer to free node
  pAllocSize  pointer to request block
  dwRequestSize  request size rounded up to dwMinBlockSize
  dwMinBlockSizeMask alignment mask
  
Return:         LPMM_NODE   pointer to allocated node
----------------------------------------------------------------------*/
LPMM_NODE FxAllocMemAlign(LPMM_NODE pHeap, LPMM_NODE pNode,
    LPMM_ALLOCSIZE pAllocSize, DWORD dwRequestSize, DWORD dwMinBlockSizeMask)
{
    DWORD dwUnusedSize;  // size remaining in free block
    DWORD dwSlopSize;    // size wasted due to alignment
    DWORD fpNewNodeStart, fpNewNodeEnd;
    LPMM_NODE   pNew;    // pointer to nodes

#ifdef DEBUG
    if (0 == dwMinBlockSizeMask)
    {
       DPF(DBGLVL_NORMAL,
          ">>> (FxAllocMem): should call FxAllocMem instead <<<");
       DbgBrk();
    }

    if (dwRequestSize > pNode->dwBlockSize)
    {
       DPF(DBGLVL_NORMAL, ">>> (FxAllocMem): should never get here! <<<");
       DbgBrk();
       return (NULL);    // fail the alloc call
    }
#endif
    //
    // align start address, ensure the node has room after alignment
    //
    if (MM_ALLOC_FROM_TOP_OF_HEAP & pAllocSize->dwFlags)
    {
       fpNewNodeStart = 
          ROUNDUP(pNode->fpVidMemStart, dwMinBlockSizeMask);
       fpNewNodeEnd = fpNewNodeStart + dwRequestSize - 1;       

       if (fpNewNodeEnd > pNode->fpVidMemEnd)
       {
          DPF(DBGLVL_NORMAL,
             ">>> (FxAllocMem): node cannot fit aligned object <<<");
          return (NULL);
       }
       dwUnusedSize = pNode->fpVidMemEnd - fpNewNodeEnd;
       dwSlopSize = fpNewNodeStart - pNode->fpVidMemStart;
    }
    else  // default is to allocate from bottom of heap
    {
       fpNewNodeStart = 
          ROUNDDN( (pNode->fpVidMemEnd + 1 - dwRequestSize),
             dwMinBlockSizeMask);

       if (fpNewNodeStart < pNode->fpVidMemStart)
       {
          DPF(DBGLVL_NORMAL,
             ">>> (FxAllocMem): node cannot fit aligned object <<<");
          return (NULL);
       }
       fpNewNodeEnd = pNode->fpVidMemEnd;
       dwUnusedSize = fpNewNodeStart - pNode->fpVidMemStart;
       dwSlopSize = (pNode->fpVidMemEnd +1 - dwRequestSize) - fpNewNodeStart;
    }

#ifdef MM_INTEGRITY
    if ( dwUnusedSize && (dwUnusedSize < (dwMinBlockSizeMask+1)) )
    {
       // this free block can never be used, sigh
       DPF(DBGLVL_MEMMGR,
          "- - (FxAllocMem): dwUnusedSize (%lx) < dwMinBlockSize (%lx) - -",
          dwUnusedSize, dwMinBlockSizeMask+1);
    }
#endif
#ifdef DEBUG
    if (fpNewNodeStart & dwMinBlockSizeMask)
    {
       DPF(DBGLVL_NORMAL, ">>> (FxAllocMem): unaligned start addr. <<<");
    }
#endif

    pNew = pNode;

    //
    // if dwUnusedSize is zero, perfect fit, return pointer to node
    // if dwUnusedSize > zero, carve free memory into two pieces
    //
    if (dwUnusedSize > 0L)
    {
       pNew = getnode();
       if (NULL == pNew)
          return (NULL); // out of nodes, fail the alloc call

       pNew->dwBlockSize = dwRequestSize + dwSlopSize;
#ifdef DEBUG
       if ( pNode->dwBlockSize != (pNew->dwBlockSize + dwUnusedSize) )
       {
          DPF(DBGLVL_NORMAL,
             ">>> (FxAllocMem): block size(%lx) != split size(%lx) <<<",
             pNode->dwBlockSize, pNew->dwBlockSize + dwUnusedSize);
       }
#endif
       pNew->fpVidMemEnd = fpNewNodeEnd;
       pNode->dwBlockSize = dwUnusedSize;  // set unused size

       if (MM_ALLOC_FROM_TOP_OF_HEAP & pAllocSize->dwFlags)
       {
          // include padding for alignment
          pNew->fpVidMemStart = pNode->fpVidMemStart;
          pNode->fpVidMemStart = fpNewNodeEnd + 1;

          // ensure fpTOHptr points to node w/ lowest starting address
          if (pHeap->fpTOHptr == pNode)
             pHeap->fpTOHptr = pNew;

          // link new node to the left of current node
          pNode->fpLeftNode->fpRightNode = pNew;
          pNew->fpLeftNode = pNode->fpLeftNode;
          pNew->fpRightNode = pNode;
          pNode->fpLeftNode = pNew;

          // move header to the right of pNew for better distribution
          RemoveNode(pHeap);
          pHeap->fpLeftNode = pNew;
          pHeap->fpRightNode = pNode;
          pNew->fpRightNode = pHeap;
          pNode->fpLeftNode = pHeap;
       }
       else  // default is to allocate from bottom of heap
       {
          pNode->fpVidMemEnd = fpNewNodeStart - 1;
          pNew->fpVidMemStart = fpNewNodeStart;

          // link new node to the right of current node
          pNode->fpRightNode->fpLeftNode = pNew;
          pNew->fpLeftNode = pNode;
          pNew->fpRightNode = pNode->fpRightNode;
          pNode->fpRightNode = pNew;

          if (pNode->fpLeftNode != pHeap)
          {
             // move header to the left of pNode for better distribution
             RemoveNode(pHeap);
             pHeap->fpLeftNode = pNode->fpLeftNode;
             pHeap->fpRightNode = pNode;
             pNode->fpLeftNode->fpRightNode = pHeap;
             pNode->fpLeftNode = pHeap;
          }
       }

#ifdef MM_INTEGRITY
       pHeap->dwReserved1++;   // add one node to heap list
 #ifdef DEBUG
       if (MaxHeapNodes < (WORD)pHeap->dwReserved1)
          MaxHeapNodes = (WORD)pHeap->dwReserved1;
 #endif
       CheckHeapList(pHeap);
#endif
    }  // done adding new node to heap

    pHeap->dwBlockSize -= (dwRequestSize + dwSlopSize);
    pNew->dwFlags &= ~MM_FREE; // mark node as used
    pNew->fpHostifyCallBackFunc = pAllocSize->fpHostifyCallBackFunc;
    pNew->fpLinearAddress = pHeap->fpVidMemStart + fpNewNodeStart;
    return (pNew);
}


#ifdef MM_INTEGRITY
/*----------------------------------------------------------------------
Function name:  FindLargestFreeBlock

Description:    Find the largest free block and print it out.

Information:    Used for debugging when "#ifdef MM_INTEGRITY".

Return:         VOID
----------------------------------------------------------------------*/
void FindLargestFreeBlock(LPMM_NODE pHeap)
{
    DWORD       dwMaxFreeSpace;
    LPMM_NODE   pNode, pBigNode;

    dwMaxFreeSpace = 0L;
    pNode = pHeap->fpTOHptr;
    pBigNode = pNode;

    // walk the heap list
    do
    {
       if (!(MM_HEADER & pNode->dwFlags))
       {
          if ( (MM_FREE & pNode->dwFlags) &&
               (pNode->dwBlockSize > dwMaxFreeSpace)
             )
          {
             dwMaxFreeSpace = pNode->dwBlockSize;
             pBigNode = pNode;
          }
       }  // skip header node

       pNode = pNode->fpRightNode;

    }  // walk heap list
    while (pHeap->fpTOHptr != pNode);

    DPF(DBGLVL_MEMMGR,
       "... Largest free block (%lx) @ %lx ...", dwMaxFreeSpace, pBigNode);
}


/*----------------------------------------------------------------------
Function name:  CheckHeapList

Description:    Check the following items in the linear heap
                (pointed to by pHeap).
                    - free nodes do not link to used list
                    - nodes are sorted in ascending order
                    - start offset is less than end offset in
                      each allocation
                    - node size is equal to node's end offset -
                      begin offset + 1
                    - count number of allocated nodes

Information:    Used for debugging when "#ifdef MM_INTEGRITY".

Return:         VOID
----------------------------------------------------------------------*/
void CheckHeapList(LPMM_NODE pHeap)
{
    DWORD       dwNodes, dwHeapNodes, dwPage;  // count the number of nodes
    DWORD       fpVidMemStart;
    LPMM_NODE   pNode;

    pNode = pHeap->fpTOHptr;
    fpVidMemStart = pNode->fpVidMemStart;
    if (0L != fpVidMemStart)
    {
       DPF(DBGLVL_NORMAL,
          ">>> (HeapCheck) fpTOHptr->fpVidMemStart is non-zero. <<<");
    }

    // walk the heap list
    do
    {
       if (!(MM_HEADER & pNode->dwFlags))
       {
          if (MM_FREE & pNode->dwFlags)
          {
             //
             // check for incorrect reference to used list
             //
             if ( (NULL != pNode->fpLeftUsedNode) || 
                  (NULL != pNode->fpLeftUsedNode)
                )
             {
                DPF(DBGLVL_NORMAL,
                   ">>> (HeapCheck) free node still linked to used list <<<"); 
             }
          }

          //
          // check ordering of starting addresses
          //
          if (pNode->fpVidMemStart < fpVidMemStart)
          {
             DPF(DBGLVL_NORMAL, ">>> (HeapCheck) out of order node <<<"); 
          }

          //
          // ensure start offset is less than end offset
          //
          if (pNode->fpVidMemStart >= pNode->fpVidMemEnd)
          {
             DPF(DBGLVL_NORMAL, ">>> (HeapCheck) start(%lx) >= end(%lx) <<<",
                pNode->fpVidMemStart, pNode->fpVidMemEnd); 
          }

          //
          // there should not be any slop size in node
          //
          if ( pNode->dwBlockSize != 
               (pNode->fpVidMemEnd - pNode->fpVidMemStart + 1)
             )
          {
             DPF(DBGLVL_NORMAL,
                ">>> (HeapCheck) unaccounted for size in node(%lx) <<<",
                pNode);
          }

       }  // skip header node

       pNode = pNode->fpRightNode;

    }  // walk heap list
    while (pHeap->fpTOHptr != pNode);

    //
    // count the number of nodes used
    //
    for (dwNodes = 0L, dwPage = 0L; dwPage < MAX_NODETABLE_ENTRIES; dwPage++)
    {
       UINT i;

       if (pNode = NodeTable[dwPage])   // if pNode is not null
       {
          for (i = 0; i < NODES_PER_PAGE; i++, pNode++)
          {
             if (MM_NODE_PTR_IS_USED & pNode->dwFlags)
                dwNodes++;
          }
       }
    }

    dwNodes -= nHeaders; // pHeap and pUsedList are never freed
    dwHeapNodes = 0L;

    if (HeapTable[linear_heap1].pHeap)
       dwHeapNodes += (HeapTable[linear_heap1].pHeap)->dwReserved1;

    if (dwNodes != dwHeapNodes)
    {
       DPF(DBGLVL_NORMAL,
          ">>> (HeapCheck) node count delta : heap(%lx),me(%lx) <<<",
          dwNodes, dwHeapNodes);
    }
}


/*----------------------------------------------------------------------
Function name:  CheckHeapSize

Description:    Ensure the total heap size does not changed due
                to mmAlloc and mmFree.
                
Information:    Used for debugging when "#ifdef MM_INTEGRITY".

Return:         VOID
----------------------------------------------------------------------*/
void CheckHeapSize(LPMM_NODE pHeap)
{
    DWORD dwFreeSize = 0L;
    DWORD dwUsedSize = 0L;
    DWORD dwHeapSize;
    LPMM_NODE   pNode;

    // walk the heap list
    for (pNode = pHeap->fpRightNode; pHeap != pNode;
         pNode = pNode->fpRightNode)
    {
       if (MM_FREE & pNode->dwFlags)
       {
          dwFreeSize += pNode->dwBlockSize;
       }
       else
       {
          dwUsedSize += pNode->dwBlockSize;
       }
    }  // walk the heap

    dwHeapSize = pHeap->fpVidMemEnd - pHeap->fpVidMemStart + 1;
    if ( dwHeapSize != (dwFreeSize + dwUsedSize) )
    {
       DPF(DBGLVL_NORMAL,
          ">>> (CheckSize) ddLinear(%lx) != free(%lx) + used(%lx) <<<",
          dwHeapSize, dwFreeSize, dwUsedSize);
    }
}


/*----------------------------------------------------------------------
Function name:  CheckNodeTable

Description:    Ensure the allocated node tables are mapped into
                memory.
                
Information:    Used for debugging when "#ifdef MM_INTEGRITY".

Return:         VOID
----------------------------------------------------------------------*/
void CheckNodeTable()
{
   UINT Cntr1;
   DWORD *pNode;
   DWORD mmNodeTmp, *pmmNodeTmp;
   WORD AllocedPages, ActualPages;

   // Initialize some locals
   pmmNodeTmp = &mmNodeTmp;
   AllocedPages = 0;
   ActualPages = 0;

   // How many nodes should be mapped into memory?
   for (Cntr1=0; Cntr1 < MAX_NODETABLE_ENTRIES; Cntr1++)
      {
      if (NodeTable[Cntr1] != 0L)
         {
         AllocedPages++;
         }
      }

   // Now physically test to see if they're mapped
   for (Cntr1=0; Cntr1 < MAX_NODETABLE_ENTRIES; Cntr1++)
      {
      if (((LPMM_NODE)pNode = NodeTable[Cntr1]) != 0L)
         {
         *pmmNodeTmp = *pNode;      // read failure if mem not mapped
         *pNode = 0xDEADBEEF;       // write failure if mem not mapped
         *pNode = *pmmNodeTmp;      // restore good data         
         ActualPages++;
         }
      }

   if (AllocedPages != ActualPages)
      {
      // We should never get here because we'll GPF above!
      DPF(DBGLVL_NORMAL,
          ">>> (CheckNode) ERROR:  AllocedPages(%d) != ActualPages(%d). <<<",
          AllocedPages, ActualPages);
      }
}


/*----------------------------------------------------------------------
Function name:  CheckFreeTable

Description:    Ensure the freed node passed in is not referenced
                by any list.
                
Information:    Used for debugging when "#ifdef MM_INTEGRITY".

Return:         VOID
----------------------------------------------------------------------*/
void CheckFreeNode(LPMM_NODE pHeap, LPMM_NODE pFreedNode)
{
    LPMM_NODE   pNode, pUsedList;
    MM_HEAPTYPE heapID;

    // walk the heap list
    for (pNode = pHeap->fpRightNode; pHeap != pNode;
         pNode = pNode->fpRightNode)
    {
       if ( (pNode == pFreedNode) ||
            (pNode->fpLeftNode == pFreedNode) ||
            (pNode->fpRightNode == pFreedNode) ||
            (pNode->fpLeftUsedNode == pFreedNode) ||
            (pNode->fpRightUsedNode == pFreedNode)
          )
       {
          DPF(DBGLVL_NORMAL,
             ">>> (FreeNode) %lx cannot be referenced. <<<", pFreedNode);
       }
    } // walk heap

    //
    // check header references
    //
    heapID = GetHeapID(pHeap->fpVidMemStart);
    pUsedList = HeapTable[heapID].pUsedList;

    if (NULL == pUsedList)
    {
       DPF(DBGLVL_NORMAL,
          ">>> (CheckFreeNode) Null pUsedList <<<");
    }
    else
    {
        if ( (pHeap->fpLeftNode == pFreedNode) ||
             (pHeap->fpRightNode == pFreedNode) ||
             (pUsedList->fpLeftUsedNode == pFreedNode) ||
             (pUsedList->fpRightUsedNode == pFreedNode)
           )
        {
            DPF(DBGLVL_NORMAL,
                ">>> (CheckFreeNode) %lx cannot be referenced in headers <<<",
                pFreedNode);
        }
    }
}


/*----------------------------------------------------------------------
Function name:  IsNodeInTable

Description:    Verify that the node to be freed is in the NodeTable.
                
Information:    Used for debugging when "#ifdef MM_INTEGRITY".

Return:         BOOL    SUCCESS or FAIL
----------------------------------------------------------------------*/
BOOL IsNodeInTable(LPMM_NODE pNodeFree)
{
    UINT nPages, nNodes;
    LPMM_NODE   pNode;

    // Search all allocated 4K page
    for (nPages = 0; nPages < MAX_NODETABLE_ENTRIES; nPages++)
    {
       // Is it an allocated 4K page?
       if ( (pNode = NodeTable[nPages]) != 0L)
       {
          // Search all entries on this 4K page
          for (nNodes = 0; nNodes < NODES_PER_PAGE; nNodes++, pNode++)
          {
             if (pNodeFree == pNode)
             {
                if (pNodeFree->dwFlags & MM_NODE_PTR_IS_USED)
                   return (SUCCESS);
                else
                   return (FAIL);
             }
          } // node not found in this page
       }  // page empty
    }  // walk next page

    return (FAIL);
}


/*----------------------------------------------------------------------
Function name:  CheckAddressOverlap

Description:    Test for overlapping video memory addresses in list.
                
Information:    Used for debugging when "#ifdef MM_INTEGRITY".

Return:         VOID
----------------------------------------------------------------------*/
void CheckAddressOverlap(LPMM_NODE pHeap)
{
    DWORD       fpVidMemEnd;
    LPMM_NODE   pNode, pStart;

    pStart = pHeap->fpTOHptr;
    fpVidMemEnd = pStart->fpVidMemEnd;

    // walk the heap list
    for (pNode = pStart->fpRightNode; pStart != pNode;
         pNode = pNode->fpRightNode)
       {
       if (!(MM_HEADER & pNode->dwFlags))
          {
          //
          // check starting addresses with previous ending address
          //
          if (pNode->fpVidMemStart < fpVidMemEnd)
             {
             DPF(DBGLVL_NORMAL,
                ">>> (CheckOverlap) address overlap (%lx) <<<", pNode); 
             DbgBrk();
             }

          //
          // check starting/ending address alignment
          //
          if (pNode->fpVidMemStart != (fpVidMemEnd + 1))
             {
             DPF(DBGLVL_NORMAL,
                ">>> (CheckOverlap) address alignment (%lx) <<<", pNode); 
             DbgBrk();
             }

          fpVidMemEnd = pNode->fpVidMemEnd;
          }  // skip header node
       }  // walk heap list
}
#endif // MM_INTEGRITY

#endif // PERF_NEWMM


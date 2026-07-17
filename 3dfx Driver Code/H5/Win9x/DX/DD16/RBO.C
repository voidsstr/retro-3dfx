/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** File name: rbo.c
**
** Description: Windows device bitmap caching functions
**
** $Revision: 6$
** $Date: 10/11/00 8:51:49 PM$
**
** $Log:
** $
*/

#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif
#ifdef  PERF_NEWMM
#include "header.h"
#define MSVC 1
#include "robject.h"
#include "devbits.h"
#include "dqueue.h"

// COUNT defines a set of debug outs that collect statistics about the bitmap cache
// CACHEMETER defines a set of debug outs that dump out info about the state of the cache
// CHECK defines a set of debug outs that checks the contents of the bitmap cache
// ALWAYS forces ALL memory allocation requests to succeed, even if there isn't enough VRAM
// TWEAK_PARAMS defines a set of variables that may be read from the registry to tune our cache

//#define COUNT
//#define CACHMETER
//#define CHECK
//#define ALWAYS
//#define TWEAK_PARAMS
//#define BIG_CHECK
//#undef DEBUG

#ifdef DEBUG_LIST
#define INLINE
#else
#define INLINE __inline
#endif

#define USE_DIBENG 0        // Define this when we want to make DIBENGINE calls!
                            // Use only in case of emergency -- we should never need to call
                            // the DIBENGINE to realize a bitmap.
#define EXCLUDE_CM_BLTS 0   // Set to 1 if you wish to throw bitmaps that are used in 
                            // color->mono blts out of the cache
#define ALLOC_BEST_FIT 1    // Search for the best fit available memory block when allocating.
#define SAVE_FREED_CACHE_BLOCK 1    // Set this to keep the block that was just freed around as the chances
                                    // are the next allocation call is for a block of the same size.

//---------------------------------------------------------------------
// Prototypes:  System Functions
//---------------------------------------------------------------------
extern DWORD FAR PASCAL DIB_RealizeObjectExt(LPVOID      lpDestDev,
                                             WORD        wStyle,
                                             LPVOID      lpInObj,
                                             LPVOID      lpOutObj,
                                             LPVOID      lpTextXForm,
                                             LPVOID      lpDriverPDevice );

//---------------------------------------------------------------------
// Prototypes:  External Functions
//---------------------------------------------------------------------
        
extern BigCopy(WORD SrcSel, DWORD SrcOffset, WORD DestSel, DWORD DestOffset, WORD Count);
extern DoBitCopy(DWORD, WORD, DWORD, WORD, DWORD, DWORD, WORD, WORD);
extern FastDWordCopy(DWORD, WORD, DWORD, WORD, DWORD, WORD);

#ifdef SSB
void _loadds WINAPI InvalidateSSB(PVRAMNODE pNode);
#endif

//---------------------------------------------------------------------
// Prototypes:  Local Functions
//---------------------------------------------------------------------
BOOL FAR    InitCache( );
BOOL        ReInitCache( );

BOOL _loadds WINAPI mmEvict(DWORD, DWORD, DWORD);
BOOL _loadds WINAPI mmReclaim(DWORD);

#ifdef CHECK
BOOL FAR IsNodeInList( PVRAMNODE List, PVRAMNODE pNd );
BOOL FAR IsListIncluded( PVRAMNODE List );
#endif

#ifdef COUNT
WORD FAR ListNodeStat( PVRAMNODE List );
#endif

#if SAVE_FREED_CACHE_BLOCK
WORD NEAR CoalesceFreeBlock(PVRAMNODE pNode);
#endif

//---------------------------------------------------------------------
// External Data
//---------------------------------------------------------------------
extern WORD PASCAL wFlatDataSel;

            // **** Nodes is the allocation of all nodes of all types ****
VRAMNODE __near node_mem[NUMNODES];
void __near *Nodes = node_mem;

            // DeviceBitmapInfo structures, used to store info like Offscreen Address,
            // pitch etc. Refer to devbits.h for more details.
DQUEUE DevBitFree;
DQUEUE DevBitInUse = {&DevBitInUse, &DevBitInUse, NULL};

DEVQUEUE DevBitPool[NUMBER_DEVBIT];

            // A node in NodePool may be in no other list!

PVRAMNODE NodePool = NULL;

            // FreeVRAM is the list of free VRAM blocks
            // A node in FreeVRAM will also be included in the SortedList

PVRAMNODE FreeVRAM = NULL;

            // BitmapCache is the list of bitmaps in the cache
            // A node in BitmapCache will also be included in the SortedList

PVRAMNODE BitmapCache = NULL;

            // SortedList is the list of all bitmaps and free elements
            // in our cache sorted by address. We use this to merge free elements in 
            // the cache back together

PVRAMNODE SortedList = NULL;

            // A node pointer used to reference a block of memory that was just freed. 
            // Instead of immediately merging the freed block we keep it around as the
            // chances are the next block allocation will require a block of this size.
PVRAMNODE RecentlyFreed = NULL;

DWORD       LastFailedAllocSize;
DWORD       MinimumAllocSize;       //Smallest allowed allocation
DWORD       MaximumAllocSize;       //Largest allowed allocation

            //---------------------------------------------------------
            // TotalCacheSize and CacheStartOffset are globals so we
            // can reinitialize the cache after a screen switch.
            //---------------------------------------------------------
DWORD       TotalCacheSize;         //   = 0;
DWORD       CacheStartOffset;       // = 0;
            //---------------------------------------------------------
            // FreeMemory is the number of free bytes in the cache.
            //---------------------------------------------------------
DWORD       FreeMemory;             //       = 0;
            //---------------------------------------------------------
            // CacheInitialized is just a flag to tell us if we have
            // done a complete initialization of the cache.
            //---------------------------------------------------------
WORD        CacheInitialized = 0;
            //---------------------------------------------------------
            // CacheDisabled set indicates the we cannot currently
            // cache a bitmap.  It may be because DirectDrawActive
            // or DriverInBackground.
            //---------------------------------------------------------
BOOL PASCAL CacheDisabled      = TRUE;

DWORD       BytesPerPixel = 0;

VOID FAR * PASCAL lpBeginAccess;
VOID FAR * PASCAL lpEndAccess;

// 
// If a particular byte alignment is required, set it here. The value of
// BYTE_ALIGNMENT is the number of bytes of alignment required for a bitmap
// in the offscreen cache.
//
// #define BYTE_ALIGNMENT 32
#define BYTE_ALIGNMENT 4
#define CACHE_ALIGNMENT_MASK (BYTE_ALIGNMENT-1)

void INLINE ListCreate(PVRAMNODE L, BYTE type, BYTE index);
PVRAMNODE INLINE ListReturnHead(PVRAMNODE L);
PVRAMNODE INLINE ListReturnTail(PVRAMNODE L);
void INLINE ListInsertHead(PVRAMNODE L, PVRAMNODE x);
PVRAMNODE INLINE ListDeleteHead(PVRAMNODE L);
void INLINE ListInsertTail(PVRAMNODE L, PVRAMNODE n);
void INLINE ListDelete(PVRAMNODE L, PVRAMNODE x);
PVRAMNODE INLINE ListDeleteTail(PVRAMNODE L);
void INLINE ListInsertBefore(PVRAMNODE L, PVRAMNODE x,    PVRAMNODE y);
void INLINE ListInsertAfter(PVRAMNODE L, PVRAMNODE x, PVRAMNODE y);


void NEAR Cacheify(PVRAMNODE pNode, LPDIBENGINE lpOrigDib);
PVRAMNODE InsertBitmapIntoCache(LPDIBENGINE lpBitmap);
BOOL NEAR CheckBitmapSize(WORD x, WORD y, DWORD BitmapSize);
void NEAR Bitmapify(PVRAMNODE pNode);
void NEAR CPUCopyVidMemToSysMem(PVRAMNODE pNode);
void FreeOffscreenMemory(PVRAMNODE pNode);
PVRAMNODE AllocOffscreenMemory(DWORD size);
DWORD FAR PASCAL GetCacheInitValues(char *Key);

// The following two macros detect whether or not a node in a list points to the
// NIL element of the list. The 'NIL' element terminates the list. Since the lists
// are circular, the sentinel node in the list is considered to be 'NIL'. This 
// greatly simplifies the list manipulation code.

#define IS_NIL(L, n) (L == n)       // True if 'n' is the NIL node in list 'L'
#define IS_NOT_NIL(L, n) (L != n)   // True if 'n' is not the NIL node in list 'L'

/* #define ListDelete(L, x, i) \
    i = L->ListIndex;       \
    x->Prev[i]->Next[i] = x->Next[i];   \
    x->Next[i]->Prev[i] = x->Prev[i]
*/



// **********************************************************************************
//
//                          VRAMNODE Management
//
// The following three routines act as a simple storage allocator for the VRAMNODE
// structures. Unlike more classical memory allocation schemes where the header for
// a block of memory is allocated from the memory itself, here we allocate headers
// from primary storage to point to the offscreen memory. This, however, creates a
// small memory allocation problem for us -- we never know how many of these VRAMNODE
// structures that we will need at any given time. Rather than create another heap
// storage alloctor, we simply create a fixed number of these VRAMNODES, link them
// together in a list (NodePool), and then simply grab one from the list when we 
// need one, and put one back onto the list when we are finished with it. Yes, this 
// amounts to a storage allocator within another storage allocator.
//
// The only real drawback with this scheme is that it is possible for us to run out 
// of VRAMNODEs before we run out of offscreen video memory. 
//
// The following routines manage the list of unused VRAMNODEs with the doubly linked
// list routines used elsewhere in this module. In fact, it would be very easy to 
// code them to use only singly linked lists. This would be slightly more efficient,
// but I didn't want to confuse the issue with too many list manipulation routines.


//---------------------------------------------------------------------
// Function:      InitNodePoool
//
// Description:   Create the pool of available VRAM structures. 
//
//
// Parameters:    none
//
//
// Returns:       nothing -- it can't fail.
//---------------------------------------------------------------------

void InitNodePool()
{
    int i;
    PVRAMNODE Node = Nodes;

    // Use the first node for the head of the list

    NodePool = &Node[0];   
    ListCreate(NodePool, FLAG_SENTINEL, FREE_NODE_LIST);
    
    // Insert nodes into our free list

    for (i = 1 ; i < NUMNODES; i++) {
        Node[i].NodeFlags = FREE_NODE;
        ListInsertTail(NodePool, Node + i);
    }    
}

//---------------------------------------------------------------------
// Function:      InitDeviceBitmap
//
// Description:   Create the pool of available DEVQUEUE structures. 
//
//
// Parameters:    none
//
//
// Returns:       nothing -- it can't fail.
//---------------------------------------------------------------------

void InitDeviceBitMap(void)
{    
    PDQUEUE pDqueue;
    int i;

    // Initialize Free Header of Device Bit Map Descriptors
    DevBitFree.pLeft = &DevBitFree;
    DevBitFree.pRight = &DevBitFree;

    // Initialize In Use Header of Device Bit Map Descriptors
    DevBitInUse.pLeft = &DevBitInUse;
    DevBitInUse.pRight = &DevBitInUse;

    // Initialize Descriptors and Free List
    for (i=0; i<NUMBER_DEVBIT; i++)
    {
        DevBitPool[i].DevInfo.deBitsOffset = 0x0;
        DevBitPool[i].DevInfo.wDeltaScan = 0x0;
        DevBitPool[i].DevInfo.wWidthBytes = 0x0;        
        pDqueue = (PDQUEUE)&DevBitPool[i];
        DINSERT(pDqueue, DevBitFree);
     }
}


//---------------------------------------------------------------------
// Function:      AllocNode
//
// Description:   Allocate a VRAMNODE structure for our use
//
//
// Parameters:    type -- one of the following values:
//                FREE_NODE -- An element in the NodePool
//                FLAG_FREE_VRAM -- An element on the list of available VRAM
//                FLAG_ALLOC_VRAM_TEMP -- An element on the list of allocated VRAM
//                FLAG_GDI_BITMAP -- An element on the list of allocated VRAM
//
// Returns:       A pointer to the allocated node, or NIL if no more nodes
//                are available
//---------------------------------------------------------------------

PVRAMNODE AllocNode(BYTE type)
{
    PVRAMNODE pNode;

    // Remove the first node from the head of the list
    pNode = ListDeleteHead(NodePool);

    // If the node returned is NIL, don't assign a type to it
    if (IS_NOT_NIL(NodePool, pNode))
        pNode->NodeFlags = type;

    // Return the found node
    return pNode;
}


//---------------------------------------------------------------------
// Function:      FreeNode
//
// Description:   Free a VRAMNODE structure for future use
//
//
// Parameters:    pNode -- a pointer to the node to free
//                
//
// Returns:       nothing.
//
//---------------------------------------------------------------------
// 

void FreeNode(PVRAMNODE pNode)
{
    pNode->NodeFlags = FREE_NODE;                   // mark as a free node

    ListInsertHead(NodePool, pNode);                // insert back into list
}

    

// **********************************************************************************



//---------------------------------------------------------------------
// Function:      RealizeBitmapObject
//
// Description:   Realizes Bitmaps and performs Device Bitmap Caching
//
//                RealizeBitmapObject is called twice by GDI to realize a bitmap.
//                During the first pass, lpOutObj == 0. This signifies that GDI
//                expects the driver to tell it how much memory to allocate for the
//                bitmap object. During this pass, RealizeBitmapObject calls CanCache,
//                which allocates memory in our device bitmap cache and saves off a 
//                pointer to this memory for the pass 2 call. Whether the bitmap can
//                be cached in offscreen VRAM or not, the driver returns the size
//                of the DIBENGINE header+the size of the bitmap.
//
//                During the second pass, RealizeBitmap object fills in the DibEngine
//                bitmap header, and if offscreen VRAM was allocated during pass 1 for
//                this bitmap, it causes the bitmap to reference the block of offscreen
//                VRAM.
//
// Parameters:    lpDestDev -- The driver PDevice
//                wStyle -- 5 
//                lpInObj --  pointer to the memory bitmap header
//                lpOutObj -- pointer to the header of the device dependent
//                            bitmap to be realized by the driver
//                lpTextXForm --
//
//
// Returns:       The size of the DIBENGINE header + bitmap on pass 1
//                TRUE on pass 2
//                
//---------------------------------------------------------------------

DWORD __cdecl RealizeBitmapObject( LPVOID      lpDestDev,
                            WORD         wStyle,
                            LPBITMAP    lpInObj,
                            LPDIBENGINE lpOutObj,
                            LPVOID      lpTextXForm,
                            LPVOID       lpPDevice )
{
    DWORD   RetValue;
    LPDIBENGINE lpPDev = (LPDIBENGINE) lpPDevice;
    WORD    tmp;
    WORD    WidthBytes;

#ifdef  TWEAK_PARAMS
    if (DevBMCache == 0) {
        return RetValue;
    }
#endif

    if ( ! lpOutObj ) {
#if !USE_DIBENG
        RetValue = lpInObj->bmWidthBytes;
        RetValue += 3l;
        RetValue &= 0xfffcl;
        RetValue *= lpInObj->bmHeight;
        RetValue += sizeof(DIBENGINE);
#else
        // Always give the DibEngine a chance to realize the bitmap.
        RetValue = DIB_RealizeObjectExt( lpDestDev,
                                     wStyle,
                                     lpInObj,
                                     lpOutObj,
                                     lpTextXForm,
                                     lpPDevice );
#endif
      
        return RetValue;
    }
    else {
    // Always give the DibEngine a chance to realize the bitmap.
#if USE_DIBENG
        RetValue = DIB_RealizeObjectExt( lpDestDev,
                                     wStyle,
                                     lpInObj,
                                     lpOutObj,
                                     lpTextXForm,
                                     lpPDevice );
#else
        // Hey! let's just fill out those DIB Engine fields ourselves!
        lpOutObj->deFlags = lpPDev->deFlags & (FIVE6FIVE | MINIDRIVER | PALETTIZED | SELECTEDDIB);

        lpOutObj->deBitsPixel = (BYTE) _FF(bpp);
        lpOutObj->dePlanes = 1;
        lpOutObj->deWidth = lpInObj->bmWidth;
        lpOutObj->deHeight = lpInObj->bmHeight;
        WidthBytes = lpInObj->bmWidthBytes;
        lpOutObj->deBitmapInfo = lpPDev->deBitmapInfo;
        tmp = (WORD) ((DWORD)lpOutObj & 0xffff) + sizeof(DIBENGINE);
        lpOutObj->deBitsOffset = (DWORD) tmp;
        tmp = (WORD) ((DWORD)lpOutObj >> 16);
        lpOutObj->deBitsSelector = tmp;
        lpOutObj->deReserved1 = 0;
        lpOutObj->delpPDevice = 0;
        lpOutObj->deBeginAccess = 0;
        lpOutObj->deEndAccess = 0;
        lpOutObj->deDriverReserved = 0;
        lpOutObj->deVersion = VER_DIBENG;
        lpOutObj->deType = TYPE_DIBENG;

        lpOutObj->deWidthBytes = WidthBytes;
        lpOutObj->deDeltaScan = (WidthBytes + 3) & ~3;

        RetValue = lpOutObj->deDeltaScan;
#endif
        if ( ((LPDIBENGINE)lpDestDev)->deFlags & (BUSY|DISABLED))
            return RetValue;

        if ( lpInObj->bmBitsPixel != _FF(bpp) )
            return RetValue;
                    
        // This is pass 2 of RealizeBitmapObject. 
        InsertBitmapIntoCache(lpOutObj);

        return RetValue;
    }

}


//---------------------------------------------------------------------
// Function:      DeleteBitmapObject
//
// Description:   Releases a previously cached bitmap.
//
//
// Parameters:    lpDevBM - Pointer to device bitmap to free.
//
//
// Returns:       TRUE   - We can cache the bitmap.
//                FALSE  - We can not cache the bitmap.
//---------------------------------------------------------------------
VOID __cdecl DeleteBitmapObject( LPDIBENGINE lpDevBM )
{
    WORD sorted = SortedList->ListIndex;   
    PVRAMNODE   pNode;
    WORD        Path = 0;
    PDEVQUEUE   pDevqueue;

    if ( lpDevBM->deFlags & VRAM ) {
        pDevqueue = (PDEVQUEUE) lpDevBM->deDriverReserved;
        pNode = (PVRAMNODE) pDevqueue->DevInfo.pNode;
        //---------------------------------------------------------------
        // We are releasing a bitmap in VRAM.
        //---------------------------------------------------------------

        FreeOffscreenMemory(pNode);
        
        DDELETE(pDevqueue);
        DINSERT((PDQUEUE)pDevqueue, DevBitFree);

        lpDevBM->deFlags &= ~(VRAM | OFFSCREEN);
        lpDevBM->deDriverReserved = 0;
        lpDevBM->deReserved1 = 0;
    }
}


BOOL NEAR CheckBitmapSize(WORD x, WORD y, DWORD BitmapSize)
{

#if EXCLUDE_CM_BLTS
    //******************************
    // Special BitBlt (C->M) cases 
    //******************************
    if (x == 186)                               // A real Keeper!
       return FALSE;                    
       
    if (x == 181  &&  y == 84) 
       return FALSE;
       
    if (x == 179  &&  y == 82) 
       return FALSE;
       
    //if (x == 369  &&  y == 79) 
    //   return FALSE;
    
    if (x == 35  &&  y == 35) 
       return FALSE;
#endif  //EXCLUDE_CM_BLTS
        
#if 1   
    // The standard rejections 
    if ( ( (x == 8) && (y == 8) ) || 
         ( (x == 1) && (y == 1) ) ||
         ( (x == 256) && (y == 16) ) || 
         (x == 624) || (x == 369) || 
         (x > 1300) || (y > 768) )
        return FALSE;
#else
    // Use this rejection to keep out the smaller bitmaps
    if ( ( x > 4096 ) || ( y > 4096 ) ||
         ( ( x * y ) <= 1024 ) )
         return FALSE;
#endif                
    
    return TRUE;    
}

//---------------------------------------------------------------------
// Function:      InsertBitmapIntoCache
//
// Description:   
//---------------------------------------------------------------------

PVRAMNODE InsertBitmapIntoCache(LPDIBENGINE lpBitmap)
{
    DWORD BitmapSize;
    PVRAMNODE pAlloc;
    DWORD Pitch;

    Pitch = lpBitmap->deWidth * BytesPerPixel;
    Pitch = (Pitch + CACHE_ALIGNMENT_MASK) & ~CACHE_ALIGNMENT_MASK;
    
    BitmapSize = (DWORD) lpBitmap->deHeight * Pitch;
        
    if (!CheckBitmapSize(lpBitmap->deWidth, lpBitmap->deHeight, BitmapSize))
        return NULL;
    
    pAlloc = AllocOffscreenMemory(BitmapSize);

    if (pAlloc)
        Cacheify(pAlloc, lpBitmap);

    return pAlloc;                        

}

//---------------------------------------------------------------------
// Function:      EvictBmp
//
// Description:   
//---------------------------------------------------------------------
void _loadds EvictBmp(LPDIBENGINE lpPDevice)
{
    PDEVQUEUE pDevqueue = (PDEVQUEUE)lpPDevice->deDriverReserved;
    PVRAMNODE pNode = (PVRAMNODE) pDevqueue->DevInfo.pNode;
    Bitmapify(pNode);
    FreeOffscreenMemory(pNode);
    // Free Up DeviceBit Map Descriptor
    DDELETE(pDevqueue);
    //Add to Free List
    DINSERT((PDQUEUE)pDevqueue, DevBitFree)    
    
}

//---------------------------------------------------------------------
// Function:      Bitmapify
//
// Description:   Convert a device bitmap back to a system memory bitmap
//
// Parameters:    pNode - the node to convert back to system memory
//
//
// Returns:       
//---------------------------------------------------------------------
void NEAR Bitmapify(PVRAMNODE pNode)
{
    LPDIBENGINE    lpOrigDib;
    FPDEVQUEUE     lpDevq;

    // Don't do the following unless this is an element in the GDI bitmap cache
    if (!(pNode->NodeFlags & FLAG_GDI_BITMAP))
        return;

    // First, make certain the node is actually allocated in the Bitmap Cache
    // and that it references a DIBEngine bitmap
//dad 2/07/97
//  The second test in the following "if" construct fixes track #1758. If you
//  change desktop backgrounds and close down Windows the lpOrigDib pointer for
//  one of the nodes in the cache will be invalid - and the driver crashes. This
//  may be a Windows device bitmap bug. The pointer becomes invalid w/o
//  the bitmap being destroyed. Things are not happening in the expected order
//  when Windows is shutting down. Therefore we check for a valid pointer before
//  before we use it.
//dad 2/07/97
//steveh 11-9-99
//  Andy Sobczyk discovered a situation where GDI will ask the driver to realize
//  the same bitmap twice without a delete in between. The driver then goes ahead
//  and allocates 2 blocks of memory. The problem is, when we are evicting bitmaps
//  the 1st will go smoothly however when we come to the 2nd block we find we have 
//  already "bitmapified" the corresponding DIBENGINE struct in system mem, i.e. reset 
//  deDriverReserved to 0, remove VRAM bit from deFlags etc. 
//  We check for this situation by testing if deDriverReserved is 0, and if it is
//  simply free up the allocated VRAM, all the other work has already been done.
//  If we don't do this we get a GPF when attempting to load the NULL deDriverReserved.
//  steveh 11-9-99

    if (!IsBadReadPtr( pNode->lpOrigDib, sizeof(DIBENGINE) ) &&
        ( pNode->lpOrigDib->deType == TYPE_DIBENG ) &&
        ( pNode->lpOrigDib->deDriverReserved != 0 ) ) {

        // Point to the original DIBEngine bitmap structure
        lpOrigDib = pNode->lpOrigDib;

        // Copy bits.
        CPUCopyVidMemToSysMem(pNode);
        
        lpDevq = (PDEVQUEUE) lpOrigDib->deDriverReserved;
       
        // Switch to the system memory pointer.
        lpOrigDib->deBitsOffset = lpDevq->DevInfo.deBitsOffset;
        lpOrigDib->deBitsSelector = lpDevq->DevInfo.wBitsSel;
        lpOrigDib->deDeltaScan = lpDevq->DevInfo.wDeltaScan;
        lpOrigDib->deWidthBytes = lpDevq->DevInfo.wWidthBytes;
        
        lpOrigDib->deFlags &= ~(VRAM | OFFSCREEN);
        lpOrigDib->deReserved1 = 0;
        lpOrigDib->deDriverReserved = 0;
        lpOrigDib->deBeginAccess = 0;
        lpOrigDib->deEndAccess = 0;
    }
}

void NEAR CPUCopyVidMemToSysMem(PVRAMNODE pNode)
{
    LPDIBENGINE     lpOrigDib;
    FPDEVQUEUE      lpDevq;
    WORD            SysMemorySel;
    DWORD           SysMemoryOffset;
    DWORD           SysDeltaScan;
    WORD            VidMemorySel;
    DWORD           VidMemoryOffset;
    DWORD           VidDeltaScan;
    WORD            Height;
    WORD            Count;
    
    // Point to the original DIBEngine bitmap structure
    lpOrigDib = pNode->lpOrigDib;
    Count = lpOrigDib->deWidthBytes;
    VidDeltaScan = lpOrigDib->deDeltaScan;    

    lpDevq = (PDEVQUEUE) lpOrigDib->deDriverReserved;
    
    // Use the fast copy if the width is the same as the deltascan and is a DWORD
    // multiple
        
    if ( (lpDevq->DevInfo.wWidthBytes == VidDeltaScan) && (VidDeltaScan % 4 == 0) )
    {
        FastDWordCopy( lpOrigDib->deBitsOffset, lpOrigDib->deBitsSelector,
                       lpDevq->DevInfo.deBitsOffset, lpDevq->DevInfo.wBitsSel,
                       VidDeltaScan, lpOrigDib->deHeight);    
    }
    else
    {                
        VidMemorySel = wFlatDataSel;
        VidMemoryOffset = pNode->dwStart;

        SysMemorySel = lpDevq->DevInfo.wBitsSel;
        SysMemoryOffset = lpDevq->DevInfo.deBitsOffset;
        SysDeltaScan = lpDevq->DevInfo.wDeltaScan;        
        
        Height = lpOrigDib->deHeight;

        for (;Height > 0; Height--)
        {
            BigCopy(VidMemorySel, VidMemoryOffset, SysMemorySel, SysMemoryOffset, Count);
            VidMemoryOffset += VidDeltaScan;
            SysMemoryOffset += SysDeltaScan;
        }
    }    
}    

//---------------------------------------------------------------------
// Function:      Cacheify
//
// Description:   Modify the DIBENGINE header of a bitmap so that it points
//                to a newly allocated block of offscreen memory
//
// Parameters:    pNode - the block of memory just allocated
//                lpOrigDib - the bitmap to be cached.
//
//
// Returns:       Nothing
//---------------------------------------------------------------------
void NEAR Cacheify(PVRAMNODE pNode, LPDIBENGINE lpOrigDib)
{
    LPBYTE         lpSysMem;
    DWORD          p;
    DWORD Pitch;
    PDEVQUEUE      pDevqueue; 
        
    // Get a Descriptor
    pDevqueue = (PDEVQUEUE)DevBitFree.pRight;       
    
    // Remove it from the free list
    DDELETE(pDevqueue);
    
    // Insert in the Used List
    DINSERT((PDQUEUE)pDevqueue, DevBitInUse);
    
    // Save a pointer to the DEVQUEUE in the pNode. This is needed in cachedepopulate
    // while stepping though the pNodes to evict the bmps and free up the associated data.
    pNode->pDqueue = (PDQUEUE)pDevqueue;   
    
    // Setup Device BitMap Information
    pDevqueue->DevInfo.deBitsOffset = lpOrigDib->deBitsOffset;
    pDevqueue->DevInfo.wBitsSel = lpOrigDib->deBitsSelector;
    pDevqueue->DevInfo.wDeltaScan = (WORD)lpOrigDib->deDeltaScan;
    pDevqueue->DevInfo.wWidthBytes = lpOrigDib->deWidthBytes;
    pDevqueue->DevInfo.pNode = (DWORD) pNode;    
    
    Pitch = lpOrigDib->deWidth * BytesPerPixel;

    lpOrigDib->deDeltaScan = (Pitch + CACHE_ALIGNMENT_MASK) & ~CACHE_ALIGNMENT_MASK; // ****
    p = ((DWORD)(lpOrigDib->deBitsSelector) << 16L) | ((WORD)(lpOrigDib->deBitsOffset));
    lpSysMem = (LPBYTE) p;


    // Point the bitmap to the device bitmap
    lpOrigDib->deBitsSelector = wFlatDataSel;
    lpOrigDib->deBitsOffset = pNode->dwStart;
    lpOrigDib->deDriverReserved = (DWORD)pDevqueue;
    lpOrigDib->deFlags |= (VRAM | OFFSCREEN);
    if (lpOrigDib->deBitsPixel == 16)
        lpOrigDib->deFlags |= FIVE6FIVE;
    lpOrigDib->deBeginAccess = (void*)BeginAccess;
    lpOrigDib->deEndAccess = (void*)EndAccess;
    
    // Setup our VRAM node structure
    pNode->lpOrigDib  = lpOrigDib;
    pNode->NodeFlags &= FLAG_PREDEFINED_VRAM;   // Keep the predefined VRAM bit
    pNode->NodeFlags |= FLAG_GDI_BITMAP;    
}

//---------------------------------------------------------------------
// Function:      AllocOffscreenMemory
//
// Description:   Attempts to allocate a block of offscreen memory.
//
// Parameters:    size - number of bytes to allocate
//
//
// Returns:       pNode of allocated memory, or NULL
//---------------------------------------------------------------------

PVRAMNODE AllocOffscreenMemory(DWORD BitmapSize)
{
    PVRAMNODE pNode = FreeVRAM->Next[FREE_VRAM_LIST];   //pNode is the first node in the list
    PVRAMNODE pAlloc;
    PVRAMNODE pNodeBestFit;
    DWORD   dwBestFit = 0xFFFFFFFF;     

    if ( !(_FF(ddMiscFlags) & DDMF_ENABLE_DEVICEBITMAPS) )
        return NULL;
       
#if SAVE_FREED_CACHE_BLOCK       
    // Check to see if the size of memory we require has been recently freed
    if (RecentlyFreed != NULL)
    {
        if (RecentlyFreed->dwSize == BitmapSize)
        {
            pNode = RecentlyFreed;
            ListDelete(FreeVRAM, pNode);        // Unlink pNode from the free list
            pNode->NodeFlags = FLAG_ALLOC_VRAM_TEMP;

            ListInsertTail(BitmapCache, pNode); // Put the node in the bitmap cache
            pNode->lpOrigDib = (LPDIBENGINE) NULL;
            RecentlyFreed = NULL;
            return pNode;
        }
        else
        {
            if ( CoalesceFreeBlock(RecentlyFreed) )
            {
                RecentlyFreed = NULL;
                pNode = FreeVRAM->Next[FREE_VRAM_LIST];
            }
        }
    }
#endif
     
#if ALLOC_BEST_FIT 
    while ( IS_NOT_NIL(FreeVRAM, pNode) )
    {        
        if ( pNode->dwSize == BitmapSize )
        {
            // We've found a node that fits exactly, no need to continue 
            // the search
            pNodeBestFit = pNode;
            dwBestFit = BitmapSize;
            break;
        }
        
        if ( (pNode->dwSize > BitmapSize) && (pNode->dwSize < dwBestFit) &&
             !(pNode->NodeFlags & FLAG_RECENTLY_FREED) )
        {
            // We've found a node that will accomodate the bitmap and is a 
            // closer fit than the previously found nodes.
            dwBestFit = pNode->dwSize;
            pNodeBestFit = pNode;
        }
        
        pNode = pNode->Next[FREE_VRAM_LIST];
    }

    if ( dwBestFit == 0xFFFFFFFF )
    {
        // No block was large enough
        LastFailedAllocSize = BitmapSize;        
        return NULL;
    }
        
    pNode = pNodeBestFit;   // Optimization - if it's worth it this can be
                            // removed by changing all references to pNode for
                            // the rest of this function.
#else

    // Walk the list of available blocks of VRAM, looking for the first 
    // node in the list that is large enough to satisfy the request
    // Note that the sentinel node in the list will always appear to satisfy
    // the request, and hence this loop is guaranteed to terminate.
    
    while ( pNode->dwSize < BitmapSize ) {
        pNode = pNode->Next[FREE_VRAM_LIST];
    }
                                
    // We did not find a large enough node, so we must fail the request.
    // Note: This condition can also be met if there are no available blocks
    // of VRAM in the free list. This condition CAN happen. However, it isn't
    // a problem, we simply fail the request.

    if ( IS_NIL(FreeVRAM, pNode) ) {

        // No block was large enough

         LastFailedAllocSize = BitmapSize;
         return NULL;
    }

#endif // ALLOC_BEST_FIT

    // If this node is the exact size or we are out of free nodes
    // to do a split, return the node we just found.

    if ( pNode->dwSize == BitmapSize ) {

        ListDelete(FreeVRAM, pNode);        // Unlink pNode from the free list

        pNode->NodeFlags = FLAG_ALLOC_VRAM_TEMP;

        ListInsertTail(BitmapCache, pNode); // Put the node in the bitmap cache
        pNode->lpOrigDib = (LPDIBENGINE) NULL;
        return pNode;
    }

    // We have to split pNode. Allocate a new node (pAlloc) that will point to the
    // newly allocated block of VRAM.
    // Don't split pNode if there are no free nodes left. This can 
    // result in some wasted VRAM. But we tolerate this to simplify the code.
    
    pAlloc = AllocNode(FLAG_ALLOC_VRAM_TEMP);   //pAlloc will point to the new block of VRAM

    if ( IS_NIL(NodePool, pAlloc)) {
        // I believe we have run out of Nodes
#ifdef  DEBUG
        _asm{int 3};
#endif
        return NULL;
    }

    // The new block of memory is allocated from the FRONT of the found block of 
    // memory, so we insert pAlloc BEFORE pNode on the Sorted List.

    ListInsertBefore(SortedList, pNode, pAlloc);

    // pAlloc is the new allocation of VRAM. It's allocated from the front of pNode
    pAlloc->lpOrigDib = NULL;
    pAlloc->dwSize = BitmapSize;
    pAlloc->dwStart = pNode->dwStart;

    // pNode is the block in the free list that has been split. We adjust it's size
    // downward, as well as it's starting address.
    pNode->dwSize -= BitmapSize;
    pNode->dwStart += BitmapSize;

    // insert the new node onto the allocated list
    ListInsertTail(BitmapCache, pAlloc);
    pAlloc->lpOrigDib = (LPDIBENGINE) NULL;

    return pAlloc;
}


//---------------------------------------------------------------------
// Function:      FreeOffscreenMemory
//
// Description:   Attempts to merge pNode with it's successor and 
//                predecessor in VRAM on the Free list. If the node
//                can't be combined with either it's predecessor or 
//                successor, it's put onto the free list itself. To 
//                accomplish this, all nodes, either free or allocated,
//                are stored in sorted order in SortedList.
//
// Parameters:    pNode - node to merge 
//
//
// Returns:       nothing
//---------------------------------------------------------------------
#if SAVE_FREED_CACHE_BLOCK

void FreeOffscreenMemory(PVRAMNODE pNode)
{
    PVRAMNODE   pPrevNode;
    PVRAMNODE   pNextNode;
    PVRAMNODE   pTempNode;
    WORD        Merged = 0;     // incremented if pNode is merged with either block
    
    pNode->lpOrigDib = NULL;
    ListDelete(BitmapCache, pNode);     // remove from bitmap cache
        
    pNode->NodeFlags = FLAG_FREE_VRAM | FLAG_RECENTLY_FREED;    
    ListInsertHead(FreeVRAM, pNode);    // Insert in the free list
              
    if (RecentlyFreed == NULL)
    {
        // We don't have a recently freed node at the moment            
        RecentlyFreed = pNode;
    }
    else
    {       
        // The recently freed spot is filled, merge the current recently freed block
        // into the free node pool and replace it with the node we've just freed
        // a few lines of code ago.
        pTempNode = pNode;
        pNode = RecentlyFreed;
        RecentlyFreed = pTempNode;
    
        pPrevNode = pNode->Prev[SORTED_VRAM_LIST];  // predecessor
        pNextNode = pNode->Next[SORTED_VRAM_LIST];  // successor
            
        // Merge with next node if it is free. This operation will destroy the
        // node pointed to by pNode, combining it with it's successor.

        // Note that NodeFlags & FLAG_FREE_VRAM implies that a node is on the FREE VRAM list.


        if ( ((pNextNode->NodeFlags & (FLAG_FREE_VRAM | FLAG_RECENTLY_FREED | FLAG_PREDEFINED_VRAM)) == FLAG_FREE_VRAM) 
            && IS_NOT_NIL(SortedList, pNextNode) ) 
        {       
                // Expand next node.
                pNextNode->dwSize     += pNode->dwSize;
                pNextNode->dwStart    -= pNode->dwSize;

            ListDelete(FreeVRAM, pNode);
            ListDelete(SortedList, pNode);  // remove from sorted list

            FreeNode( pNode );              // return pNode to our list of unused node headers
        
            // Set pNode to the node we merged with.
            // (The node pointed to by our parameter pNode no longer exists.)
            pNode = pNextNode;

            Merged++;         // indicate that at least one merge has happend.
        }

        // See if this node can be merged with the previous. If successful, the 
        // node pointed to by pNode will be destroyed and combined with it's predecessor.

        if ( ((pPrevNode->NodeFlags & (FLAG_FREE_VRAM | FLAG_RECENTLY_FREED | FLAG_PREDEFINED_VRAM)) == FLAG_FREE_VRAM) 
             && IS_NOT_NIL(SortedList, pPrevNode) ) {       

            // Expand prev node. to include this node
            pPrevNode->dwSize += pNode->dwSize;

            ListDelete(FreeVRAM, pNode);
            ListDelete(SortedList, pNode);  // remove from sorted list

            FreeNode( pNode );      //destroy node header

            Merged++;                 // indicate that we've merged.
        }

        // If the block could not be merged with either it's predecessor or 
        // successor, put it on the free list
        if (!Merged) {

            pNode->NodeFlags = FLAG_FREE_VRAM;

            pNode->lpOrigDib = NULL;

        }    
    }
    LastFailedAllocSize = 0;                // We've done a merge, no telling what will fit...
}

WORD NEAR CoalesceFreeBlock(PVRAMNODE pNode)
{
    PVRAMNODE   pPrevNode;
    PVRAMNODE   pNextNode;
    WORD        Merged = 0; 
    
    pPrevNode = pNode->Prev[SORTED_VRAM_LIST];  // predecessor
    pNextNode = pNode->Next[SORTED_VRAM_LIST];  // successor
            
    // Merge with next node if it is free. This operation will destroy the
    // node pointed to by pNode, combining it with it's successor.

    // Note that NodeFlags & FLAG_FREE_VRAM implies that a node is on the FREE VRAM list.

    if ( ((pNextNode->NodeFlags & (FLAG_FREE_VRAM | FLAG_RECENTLY_FREED | FLAG_PREDEFINED_VRAM)) == FLAG_FREE_VRAM) 
         && IS_NOT_NIL(SortedList, pNextNode) ) 
    {       

        // Expand next node.
        pNextNode->dwSize     += pNode->dwSize;
        pNextNode->dwStart    -= pNode->dwSize;

        ListDelete(FreeVRAM, pNode);
        ListDelete(SortedList, pNode);  // remove from sorted list
        
        FreeNode( pNode );              // return pNode to our list of unused node headers
        
        // Set pNode to the node we merged with.
        // (The node pointed to by our parameter pNode no longer exists.)
        pNode = pNextNode;

        Merged++;         // indicate that at least one merge has happend.
    }

    // See if this node can be merged with the previous. If successful, the 
    // node pointed to by pNode will be destroyed and combined with it's predecessor.

    if ( ((pPrevNode->NodeFlags & (FLAG_FREE_VRAM | FLAG_RECENTLY_FREED | FLAG_PREDEFINED_VRAM)) == FLAG_FREE_VRAM) 
         && IS_NOT_NIL(SortedList, pPrevNode) ) {       

        // Expand prev node. to include this node
        pPrevNode->dwSize += pNode->dwSize;

        ListDelete(FreeVRAM, pNode);
        ListDelete(SortedList, pNode);  // remove from sorted list

        FreeNode( pNode );      //destroy node header

        Merged++;                 // indicate that we've merged.
    }
    return Merged;    
}


#else

void NEAR FreeOffscreenMemory(PVRAMNODE pNode)
{
    PVRAMNODE   pPrevNode;
    PVRAMNODE   pNextNode;
    WORD        Merged = 0;     // incremented if pNode is merged with either block
    WORD        i;
        
    pPrevNode = pNode->Prev[SORTED_VRAM_LIST];  // predecessor
    pNextNode = pNode->Next[SORTED_VRAM_LIST];  // successor

    // Merge with next node if it is free. This operation will destroy the
    // node pointed to by pNode, combining it with it's successor.

    // Note that NodeFlags & FLAG_FREE_VRAM implies that a node is on the FREE VRAM list.
    
    if ( (pNextNode->NodeFlags & FLAG_FREE_VRAM) && IS_NOT_NIL(SortedList, pNextNode) && 
        !(pNextNode->NodeFlags & FLAG_PREDEFINED_VRAM)) {

        // Expand next node.
        pNextNode->dwSize     += pNode->dwSize;
        pNextNode->dwStart    -= pNode->dwSize;

        ListDelete(BitmapCache, pNode); // remove from bitmap cache

        ListDelete(SortedList, pNode);  // remove from sorted list

        FreeNode( pNode );              // return pNode to our list of unused node headers

        // Set pNode to the node we merged with.
        // (The node pointed to by our parameter pNode no longer exists.)
        pNode = pNextNode;

        Merged++;         // indicate that at least one merge has happend.
    }

    // See if this node can be merged with the previous. If successful, the 
    // node pointed to by pNode will be destroyed and combined with it's predecessor.

    if ( (pPrevNode->NodeFlags & FLAG_FREE_VRAM) && IS_NOT_NIL(SortedList, pPrevNode) && 
        !(pPrevNode->NodeFlags & FLAG_PREDEFINED_VRAM)) {

        // Expand prev node. to include this node
        pPrevNode->dwSize += pNode->dwSize;

        ListDelete(BitmapCache, pNode); // remove from bitmap cache

        ListDelete(SortedList, pNode);  // remove from sorted list

        FreeNode( pNode );      //destroy node header

        Merged++;                 // indicate that we've merged.
    }

    // If the block could not be merged with either it's predecessor or 
    // successor, put it on the free list
    if (!Merged) {

        pNode->NodeFlags = FLAG_FREE_VRAM;

        pNode->lpOrigDib = NULL;
        ListDelete(BitmapCache, pNode);     // remove from bitmap cache

        ListInsertHead(FreeVRAM, pNode);    // Insert in the free list

    }

    LastFailedAllocSize = 0;                // We've done a merge, no telling what will fit...
}
#endif

//---------------------------------------------------------------------
// Function:      InitCache
//
// Description:   Sets up the off-screen bitmap cache based on the
//                starting offset and size passed in.
//
//                Called by the enable code.
//
// Parameters:    lpOffscreenInitParams - pointer to offscreen bounding rectangle
//
//
// Returns:       TRUE   - Successful init.
//                FALSE  - Failure.
//---------------------------------------------------------------------
BOOL FAR InitCache()
{
    DWORD		CacheSize;
    DWORD		CacheStart;
    PVRAMNODE   pNode;

    if (CacheInitialized)
        // If we have already been initialized (i.e. this is a reenable
        // call) just restore the cache to an empty state.
        return ReInitCache();
                                                                                                                      
    BytesPerPixel = _FF(bpp) >> 3;
    
    // Put address of DD32 memory manager callbacks in shared data area
    _FF(mmEvictAddr) = (DWORD) mmEvict;
    _FF(mmReclaimAddr) = (DWORD) mmReclaim;
    
    // Starting offset of cache
    // CacheStart = ScreenSize;
    CacheStart = _FF(ddLinearHeapStart) + _FF(lfbBase);       

//    CacheSize = _FF(ddLinearHeapSize) + _FF(ddTiledHeapSize) + _FF(ddSecondaryHeapSize);  
    CacheSize = _FF(ddLinearHeapSize) + _FF(ddTiledHeapSize);  

    MinimumAllocSize = 0;       // smallest allowable bitmap in the cache
    MaximumAllocSize = 0;       // largest allowable bitmap in the cache
    LastFailedAllocSize = 0;

#ifdef TWEAK_PARAMS
    MinimumAllocSize = GetCacheInitValues("MinAllocSize");
    MaximumAllocSize = GetCacheInitValues("MaxAllocSize");

    STB_DBG_DRVDPF( "TotalCacheSize = FreeMemory = CacheSize = %lu", CacheSize);
    STB_DBG_DRVDPF( "MinimumAllocSize = %ld", MinimumAllocSize);
    STB_DBG_DRVDPF( "MaximumAllocSize = %ld", MaximumAllocSize);
#else

    MinimumAllocSize = 256*(_FF(bpp) >>3);
    MaximumAllocSize = 0xBFFFFL*(_FF(bpp) >>3);
#endif  //TWEAK_PARAMS


    //------------------------------------------------------------------
    // Init some globals.
    //------------------------------------------------------------------
    TotalCacheSize = FreeMemory = CacheSize;

#ifdef TWEAK_PARAMS
    if (MinimumAllocSize == 0L)
        MinimumAllocSize = 64L*(ScreenDepth>>3);

    if (MaximumAllocSize == 0L)
        MaximumAllocSize = 0x40000L*(ScreenDepth>>3);
#endif //TWEAK_PARAMS

    CacheStartOffset = CacheStart;

    // Create our node storage allocator

    InitNodePool();
    
    // Allocate empty lists
    FreeVRAM = AllocNode(FLAG_SENTINEL);           // Free VRAM list
    ListCreate(FreeVRAM, FLAG_SENTINEL, FREE_VRAM_LIST);       // Create List
    FreeVRAM->dwSize = 0x10000000;                      // Size of the header is always big 
                                                        // enough to satisfy a memory request

    BitmapCache = AllocNode(FLAG_SENTINEL);       // The bitmap cache
    ListCreate(BitmapCache, FLAG_SENTINEL, ALLOC_VRAM_LIST);  // Create list
    BitmapCache->dwSize = 0x10000000;
        
    SortedList = AllocNode(FLAG_SENTINEL);       // List of all VRAM in address sorted order
    ListCreate(SortedList, FLAG_SENTINEL, SORTED_VRAM_LIST); // Create List
        
    pNode = AllocNode(FLAG_FREE_VRAM);              // Get a node for the free list that
                                                    // represents all of VRAM
    // Init first real node head to one free chunk encompassing the
    // entire cache.
    pNode->lpOrigDib           = NULL;
    pNode->dwStart             = CacheStart;
    pNode->dwSize              = CacheSize;
    ListInsertHead(FreeVRAM, pNode);
    ListInsertHead(SortedList, pNode);
    
    CacheDisabled = FALSE;
    RecentlyFreed = NULL;
    CacheInitialized = TRUE;
   
    return TRUE;
}

//---------------------------------------------------------------------
// Function:      ReInitCache
//
// Description:   Sets up the off-screen bitmap cache based on the
//                starting offset and size previously passed in.
//
// Parameters:    
//                
//
//
// Returns:       TRUE   - Successful init.
//                FALSE  - Failure.
//---------------------------------------------------------------------
BOOL ReInitCache( )
{
    PVRAMNODE   pNode;

    //------------------------------------------------------------------
    // Init some globals.
    //------------------------------------------------------------------
    TotalCacheSize = _FF(ddLinearHeapSize) + _FF(ddTiledHeapSize); 
    FreeMemory = TotalCacheSize;
    BytesPerPixel = _FF(bpp) >> 3;    

    // Create our node storage allocator

    InitNodePool();
    
#ifdef TWEAK_PARAMS
    MinimumAllocSize = GetCacheInitValues("MinAllocSize");
    MaximumAllocSize = GetCacheInitValues("MaxAllocSize");
#endif
    LastFailedAllocSize = 0;                // We've aged the cache, more stuff might fit

    // Allocate empty lists
    FreeVRAM = AllocNode(FLAG_SENTINEL);           // Free VRAM list
    ListCreate(FreeVRAM, FLAG_SENTINEL, FREE_VRAM_LIST);       // Create List
    FreeVRAM->dwSize = 0x10000000;

    BitmapCache = AllocNode(FLAG_SENTINEL);       // The bitmap cache
    ListCreate(BitmapCache, FLAG_SENTINEL, ALLOC_VRAM_LIST);  // Create list
    BitmapCache->dwSize = 0x10000000;
    
    SortedList = AllocNode(FLAG_SENTINEL);       // List of all VRAM in address sorted order
    ListCreate(SortedList, FLAG_SENTINEL, SORTED_VRAM_LIST); // Create List

    pNode = AllocNode(FLAG_FREE_VRAM);              // Get a node for the free list that
                                                    // represents all of VRAM

    // Init first real node head to one free chunk encompassing the
    // entire cache.
    pNode->lpOrigDib           = NULL;
    CacheStartOffset = _FF(ddLinearHeapStart) + _FF(lfbBase); 
    pNode->dwStart             = CacheStartOffset;
    pNode->dwSize              = TotalCacheSize;
    ListInsertHead(FreeVRAM, pNode);
    ListInsertHead(SortedList, pNode);
    
    // The head of the free list contains values such that it always 
    // appears to satisfy any allocation (so will don't have to check 
    // for the end of the list within our search loop).

    CacheDisabled = FALSE;
    RecentlyFreed = NULL;

    return TRUE;
}


//---------------------------------------------------------------------
// Function:      CacheDepopulate
//
// Description:   Saves each bitmap in the cache to system memory.
//                Bitmaps are removed from our bitmap cache and turned
//                back into system memory bitmaps. We can do this because
//                memory for the bitmap is allocated at the time the bitmap
//                is created. A pointer to the system memory for the bitmap
//                is saved in the VRAMPOINTER structure for that node.
//
//
// Parameters:    none
//
//
// Returns:       nothing
//---------------------------------------------------------------------
void __loadds FAR CacheDepopulate( )
{
		PVRAMNODE pNode;
        PVRAMNODE pNextNode;
        PDQUEUE   pDqueue;
        
        // Check the cache has been initialized
        if (BitmapCache != NULL)
        {
           
            pNode = BitmapCache->Next[ALLOC_VRAM_LIST];   //pNode is the first node in the list
        
	        while (IS_NOT_NIL(BitmapCache, pNode)) {
	            pNextNode = pNode->Next[ALLOC_VRAM_LIST];
#ifdef SSB                
                if (pNode->NodeFlags & FLAG_GDI_SSB)    // Is this a node used in by SSB?
                {
                    InvalidateSSB(pNode);
                }
                else
#endif                 
                {       
                    pDqueue = pNode->pDqueue;                            
		    	    Bitmapify(pNode);
                    
                    if (!IsBadReadPtr( pNode->lpOrigDib, sizeof(DIBENGINE) ) &&
                                     ( pNode->lpOrigDib->deType == TYPE_DIBENG ) )  // Make sure the pointer is still valid
    		    	    pNode->lpOrigDib->deReserved1 = 1;    // Flag this bitmap as recacheable
                        
                    FreeOffscreenMemory(pNode);
                    DDELETE(pDqueue);
                    //Add to Free List
                    DINSERT(pDqueue, DevBitFree);                                            
                }                    
                pNode = pNextNode;
            }

		}
}   /* CacheDepopulate */

//---------------------------------------------------------------------
// Function:      ListCreate
//
// Description:   Create the sentinel node in a list. This function must
//                be called before any node can be inserted into the list.
//
//
// Parameters:    
//                PVRAMNODE L -- pointer to a node that will contain the 
//                               list sentinel node.
//
//                WORD type -- Type of element in the list
//
//                WORD index -- Which set of prev and next pointers this
//                              list uses. This is used by the list manipulation
//                              routines to pick the appropriate prev and next
//                              pointers
//
//
// Returns:       nothing -- it can't fail.
//---------------------------------------------------------------------

void INLINE ListCreate(PVRAMNODE L, BYTE type, BYTE index)
{
    L->Next[index] =  L;
    L->Prev[index] =  L;
    L->NodeFlags = type;
    L->ListIndex = index;
}

//---------------------------------------------------------------------
// Function:      ListReturnHead
//
// Description:   return the first node in the list
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//
// Returns:       The first node in the list, or a pointer to 'L', the 
//                sentinel node in the list if the list is empty
//---------------------------------------------------------------------

PVRAMNODE INLINE ListReturnHead(PVRAMNODE L)
{
    WORD i = L->ListIndex;
    return L->Next[i];
}

//---------------------------------------------------------------------
// Function:      ListReturnTail
//
// Description:   return the last node in the list
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//
// Returns:       The last node in the list, or a pointer to 'L', the 
//                sentinel node in the list if the list is empty
//---------------------------------------------------------------------

PVRAMNODE INLINE ListReturnTail(PVRAMNODE L)
{
    WORD i = L->ListIndex;
    return L->Prev[i];
}

//---------------------------------------------------------------------
// Function:      ListInsertHead
//
// Description:   insert node x into the first position in the list
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//                PVRAMNODE x -- pointer to the node to insert
//
// Returns:       none
//---------------------------------------------------------------------

void INLINE ListInsertHead(PVRAMNODE L, PVRAMNODE x)
{
    WORD i = L->ListIndex;

    x->Next[i] = L->Next[i];
    L->Next[i]->Prev[i] = x;
    L->Next[i] = x;
    x->Prev[i] = L;
}

//---------------------------------------------------------------------
// Function:      ListDeleteHead
//
// Description:   remove the first node from list L and return it
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//
// Returns:       the first node in the list, or NIL if the list is
//                empty
//---------------------------------------------------------------------

PVRAMNODE INLINE ListDeleteHead(PVRAMNODE L)
{
    PVRAMNODE n;
    WORD i = L->ListIndex;
    
    n = L->Next[i];             // point to the first node
    L->Next[i] = n->Next[i];    // make 2nd node the first in list
    n->Next[i]->Prev[i] = L;    // 2nd node's predecessor is now L

    return n;
}


//---------------------------------------------------------------------
// Function:      ListInsertTail
//
// Description:   insert node x into the last position in the list
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//                PVRAMNODE x -- pointer to the node to insert
//
// Returns:       none
//---------------------------------------------------------------------

void INLINE ListInsertTail(PVRAMNODE L, PVRAMNODE n)
{
    WORD i = L->ListIndex;

    n->Prev[i] = L->Prev[i];
    L->Prev[i]->Next[i] = n;
    L->Prev[i] = n;
    n->Next[i] = L;

}

//---------------------------------------------------------------------
// Function:      ListDelete
//
// Description:   splice node x our of list L
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//                PVRAMNODE x -- pointer to the node to delete
//
// Returns:       none
//---------------------------------------------------------------------

void INLINE ListDelete(PVRAMNODE L, PVRAMNODE x)
{
    WORD i = L->ListIndex;

    x->Prev[i]->Next[i] = x->Next[i];
    x->Next[i]->Prev[i] = x->Prev[i];
}

//---------------------------------------------------------------------
// Function:      ListDeleteTail
//
// Description:   remove the last node from list L and return it
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//
// Returns:       the last node in the list, or NIL if the list is
//                empty
//---------------------------------------------------------------------

PVRAMNODE INLINE ListDeleteTail(PVRAMNODE L)
{
    WORD i = L->ListIndex;
    PVRAMNODE n = L->Prev[i];
    
    L->Prev[i] = n->Prev[i];    // make 2nd node the first in list
    n->Prev[i]->Next[i] = L;    // 2nd node's predecessor is now L
    return n;
}


//---------------------------------------------------------------------
// Function:      ListInsertBefore
//
// Description:   insert node y into the list before node x
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//                PVRAMNODE x -- pointer to a node in the list
//                PVRAMNODE y -- pointer to insert ahead of node x
//
// Returns:       none
//---------------------------------------------------------------------

void INLINE ListInsertBefore(PVRAMNODE L, PVRAMNODE x,    PVRAMNODE y)
{
    WORD i = L->ListIndex;

    y->Next[i] = x;
    y->Prev[i] = x->Prev[i];
    x->Prev[i]->Next[i] = y;
    x->Prev[i] = y;
}

//---------------------------------------------------------------------
// Function:      ListInsertAfter
//
// Description:   insert node y into the list after node x
//
// Parameters:    
//                PVRAMNODE L -- pointer to the list
//                PVRAMNODE x -- pointer to a node in the list
//                PVRAMNODE y -- pointer to insert behind node x
//
// Returns:       none
//---------------------------------------------------------------------

void INLINE ListInsertAfter(PVRAMNODE L, PVRAMNODE x, PVRAMNODE y)
{
    WORD i = L->ListIndex;

    y->Prev[i] = x;
    y->Next[i] = x->Next[i];
    x->Next[i]->Prev[i] = y;
    x->Next[i] = y;
}

//---------------------------------------------------------------------
// Function:      mmEvict and mmRestore
//
// Description: The 32-bit DDRAW driver calls these from 
//              memMgr_allocSurface when it wants to allocate a piece
//              of linear memory. This is done with the understanding
//              that the linear heap is being used to cache GDI bitmaps
//              while DDRAW is running in NON-EXCLUSIVE mode (they are not 
//              called when running in EXCLUSIVE mode).
//              However, with this 2D memory manager, devicebitmap caching
//              is completely disabled while DDRAW is active (by calling
//              DisableDeviceBitmaps from within DDCreateDriverObject).
//              Bitmap caching is re-enabled when DDRAW is shut down.
//              For this reason these functions don't need to do
//              anything (as there is nothing to evict)
//---------------------------------------------------------------------

BOOL _loadds WINAPI mmEvict(DWORD hwPtrStart, DWORD hwPtrEnd, DWORD dwBlockSize)
{
    return TRUE;
}

BOOL _loadds WINAPI mmReclaim(DWORD hwPtrStart)
{
    return TRUE;
}


//**********************************************************************
//                      DEBUG Code

#ifdef TWEAK_PARAMS
// GetCacheInitValue
//
// 

DWORD FAR PASCAL GetCacheInitValues(char *Key)
{
    HKEY    hkey;
    BYTE value[9];
    DWORD len=9;
    LPDWORD DummyPtr;
    DWORD flag_val;
    int i;

    flag_val = 0;

    // Open the cache key
    if( !RegOpenKey( HKEY_LOCAL_MACHINE, "SOFTWARE\\STBSystems\\Cache", &hkey ) )
    {
        // Query the flags value
        if ( !RegQueryValueEx(hkey,Key,NULL,NULL,value,&len) )
        {
            len = lstrlen(value);
            for (i = 0; i < len; i++) {
                flag_val <<= 4;
                switch (value[i]) {
                case '0':
                    flag_val += 0;
                    break;
                case '1':
                    flag_val += 1;
                    break;
                case '2':
                    flag_val += 2;
                    break;
                case '3':
                    flag_val += 3;
                    break;
                case '4':
                    flag_val += 4;
                    break;
                case '5':
                    flag_val += 5;
                    break;
                case '6':
                    flag_val += 6;
                    break;
                case '7':
                    flag_val += 7;
                    break;
                case '8':
                    flag_val += 8;
                    break;
                case '9':
                    flag_val += 9;
                    break;
                case 'a':
                case 'A':
                    flag_val += 10;
                    break;
                case 'b':
                case 'B':
                    flag_val += 11;
                    break;
                case 'c':
                case 'C':
                    flag_val += 12;
                    break;
                case 'd':
                case 'D':
                    flag_val += 13;
                    break;
                case 'e':
                case 'E':
                    flag_val += 14;
                    break;
                case 'f':
                case 'F':
                    flag_val += 15;
                    break;
                default:
                    break;
                }
            }
        }

        RegCloseKey(hkey);

    }

    return flag_val;

} 
#endif


#ifdef CACHMETER
void PrintNode(PVRAMNODE x)
{                       
    STB_DBG_DRVDPF("%x: ", x);
    switch(x->NodeID) {
    case FREE_NODE:
        STB_DBG_DRVDPF("FREE_NODE ");
        break;
    case VRAM_FREE_NODE:
        STB_DBG_DRVDPF("VRAM_FREE_NODE ");
        break;
    case VRAM_ALLOC_NODE:
        STB_DBG_DRVDPF("VRAM_ALLOC_NODE ");
        break;
    case SENTINEL_FREE:
        STB_DBG_DRVDPF("SENTINEL_FREE ");
        break;
    case SENTINEL_VRAM_ALLOC:
        STB_DBG_DRVDPF("SENTINEL_VRAM_ALLOC ");
        break;
    case SENTINEL_VRAM_FREE:
        STB_DBG_DRVDPF("SENTINEL_VRAM_FREE ");
        break;
    case SENTINEL_VRAM_SORTED:
        STB_DBG_DRVDPF("SENTINEL_VRAM_SORTED ");
        break;
    }
    STB_DBG_DRVDPF("%d ", x->ListIndex);
    STB_DBG_DRVDPF("dwSize %lx dwStart: %ld lpOrigDib:%lx", x->dwSize, x->dwStart, x->lpOrigDib);
}

void PrintList(PVRAMNODE L)
{
    WORD i = L->ListIndex;
    PVRAMNODE p;

    PrintNode(L);
    p = L->Next[i];
    while (IS_NOT_NIL(L, p)) {
        PrintNode(p);
        p = p->Next[i];
    }
}

#endif

#ifdef CHECK
//---------------------------------------------------------------------
// Function:      IsNodeInList
//
// Description:   Look for the specified node in the specified list.
//
// Parameters:    List - A pointer to the list of interest.
//                pNd -  A pointer to the node to look for.
//
// Returns:       TRUE   - The node of interest is in the specified list.
//                FALSE  - The node of interest is not in the specified list.
//---------------------------------------------------------------------
BOOL FAR IsNodeInList( PVRAMNODE List, PVRAMNODE pNd )
{

    WORD i = List->ListIndex;   // Which set of Next/Prev pointers to use in the node
    PVRAMNODE ParseList = List->Next[i];

    while ( ParseList != pNd )  // walk the list while we have no matching bitmap.
    {
        if ( IS_NOT_NIL(List, ParseList) )  // Is the next node the NIL node?
            ParseList = ParseList->Next[i];     // No, then check out the next node
        else    //  Yes, there is no matching bitmap.
            return FALSE;
    }
    return TRUE;
}

//---------------------------------------------------------------------
// Function:      IsListIncluded
//
// Description:   Check if the specified list is included in the "SortedList" list.
//
// Parameters:    List - A pointer to the list of interest.
//
// Returns:       TRUE   - The list of interest is included.
//                FALSE  - The list of interest is not included.
//---------------------------------------------------------------------
BOOL FAR IsListIncluded( PVRAMNODE List )
{

    WORD i = List->ListIndex;   // Which set of Next/Prev pointers to use in the node
    PVRAMNODE ParseTargetList = List->Next[i];

    while ( IS_NOT_NIL(List, ParseTargetList) ) // parse the list of interst while the next node is not the NIL node.
    {
        if( !IsNodeInList(SortedList, ParseTargetList) ) // check each node in reference to the SortedList list.
        {
            STB_DBG_DRVDPF("Node (%lx) is not in the SortedList list", ParseTargetList);
            return FALSE;
        }
        ParseTargetList = ParseTargetList->Next[i];
    }
    return TRUE;
}
#endif  //CHECK

#ifdef  COUNT
//---------------------------------------------------------------------
// Function:      ListNodeStat
//
// Description:   Counts the number of nodes in the specified list and
//                updates the minimum and maximum node number as needed.
//
// Parameters:    List - A pointer to the list of interest.
//
// Returns:       The number of nodes in the specified list.
//---------------------------------------------------------------------
WORD FAR ListNodeStat( PVRAMNODE List )
{

    WORD i = List->ListIndex;   // Which set of Next/Prev pointers to use in the node
    PVRAMNODE ParseList = List->Next[i];
    int  NodeCount = 0; // The number of nodes in the list

    while ( IS_NOT_NIL(List, ParseList) )   // walk the list while next node is not the NIL node.
    {
        NodeCount++;
        ParseList = ParseList->Next[i];     
    }
    
    if( List == SortedList )
    {
        if( NodeCount < MinNumNodesSL )
            MinNumNodesSL = NodeCount;

        if( MaxNumNodesSL < NodeCount )
            MaxNumNodesSL = NodeCount;

        AccNodesSL += NodeCount;
    }

    if( List == FreeVRAM )
    {
        if( NodeCount < MinNumNodesFV )
            MinNumNodesFV = NodeCount;

        if( MaxNumNodesFV < NodeCount )
            MaxNumNodesFV = NodeCount;
        AccNodesFV += NodeCount;
    }

    if( List == BitmapCache )
    {
        if( NodeCount < MinNumNodesBC )
            MinNumNodesBC = NodeCount;

        if( MaxNumNodesBC < NodeCount )
            MaxNumNodesBC = NodeCount;
        AccNodesBC += NodeCount;
     }

    return NodeCount;
}

#endif   //COUNT

#endif  // PERF_NEWMM
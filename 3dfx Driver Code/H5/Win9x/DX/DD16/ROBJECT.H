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
** File name: robject.h
**
** Description: Structures for device bitmap caching
**
** $Revision: 3$
** $Date: 10/11/00 8:51:51 PM$
**
** $Log:
** $
*/


#ifdef 	PERF_NEWMM

#include "dqueue.h"

typedef struct {
   unsigned short      left;                    
   unsigned short      top;                     
   unsigned short      right;                   
   unsigned short      bottom;                  
   unsigned long       frameBuffer;             
   unsigned long       frameBufferLimit;        
} OFFSCREENINITPARAMS;                          


// VRAMNODES are used to track elements in the device bitmap cache. 
// A VRAM Node may exist in two lists at the same time, hence two 
// sets of next and previous pointers. VRAM nodes may be in either the
// BitmapCache list if they are presently representing allocated nodes,
// or the FreeVRAM list if they represent free nodes. Obviously, no node can
// be on both of these lists simultaneously. In addition, all nodes, whether 
// allocated or free are stored in a linked list in sorted order of address.
// This makes mergeing freed blocks of VRAM back together simpler.
//
// VRAMNODES are stored as circular linked lists. The First node in each list
// is a dummy sentinel node. In the FreeVRAM list, the sentinel node dwSize is
// set such that the sentinel node will always be large enough to satisfy the 
// memory allocation request. This is used to simplify the termination condition
// of the loop in CanCache.
// 
// List->Next[i] points to the first real node in any list. List->Prev[i] 
// points to the last real node in the list. 

struct _VRAMNODE
{
    BYTE                ListIndex;		// which set of pointers this list uses (only used in List headers)
    BYTE                NodeFlags; 		// What kind of node is this?
    struct _VRAMNODE    *Prev[2];		// Pointers to previous elements in the list
    struct _VRAMNODE    *Next[2];		// Pointers to next elements in the list
    DWORD				   dwSize;			// size of the bitmap in bytes
    DWORD               dwStart;		   // Address in VRAM of the bitmap
    LPDIBENGINE			lpOrigDib;		// pointer to the original dib engine structure
    PDQUEUE             pDqueue;       // Needed in CacheDepopulate to ref the associated PDQUEUE
//    DWORD               Age;			// Age of the bitmap
//    BYTE                Accessed;		// 0 if the bitmap has not been accessed, 1 if it has
};

//---------------------------------------------------------------------
// STB Defines
//---------------------------------------------------------------------

typedef struct _VRAMNODE VRAMNODE;
typedef struct _VRAMNODE *PVRAMNODE;

#define NUMNODES            450  // The number of pNode structures available for use. 
                                 // On a 16MB board at 16bpp you are more likely to run
                                 // out of pNodes before running out of actual cache memory.
                                 // However it is not a good idea to have a huge number of these
                                 // as they take up space in the DATA section of the driver.
                                 // 450 is more than enough for 2D Winbench 99.
                                 // IMPORTANT: If you are going to increase this also increase 
                                 // NUMBER_DEVBIT in devbits.h
                                
#define END_OF_SYS_LIST      ((LPDIBENGINE)-1)

#define NUM_RECENTLY_FREED 1
#define NUM_PREDEFINED_SIZES  4
#define NUM_PREDEFINED_16X16  40
#define NUM_PREDEFINED_56X48  14
#define NUM_PREDEFINED_12X12  14
#define NUM_PREDEFINED_24X24  72


/* typedef enum _tagPredefinedSizes {
   SIZE_16X16 = 0,
   SIZE_56X48
} PREDEFINED_SIZES; */

//#define SENTINEL_FREE         0x3333
//#define SENTINEL_VRAM_ALLOC   0x4444
//#define SENTINEL_VRAM_FREE    0x5555
//#define SENTINEL_VRAM_SORTED  0x6666

#define FREE_NODE_LIST        0     //A node in the free list uses the first set of pointers
#define FREE_VRAM_LIST        0     //A node in the free vram list uses the first set of pointers
#define ALLOC_VRAM_LIST       0     //A node in the allocated vram list uses the first set of pointers
#define PREDEFINED_VRAM_LIST  0     //A node in the predefined vram list uses the first set of pointers
#define SORTED_VRAM_LIST      1     //A node in the sorted vram list uses the second set of pointers

//#define FREE_NODE             0x1111
//#define VRAM_FREE_NODE        0x2222
//#define VRAM_ALLOC_NODE       0x9999

// NodeFlags Flag bit values
// NodeFlags == 0 implies that the node has no VRAM associated with it

#define FREE_NODE             0x0
#define FLAG_GDI_BITMAP       0x1       // Node exists in the GDI bitmap cache 
                                        // Note that a DOS box or Direct Draw can 
                                        // invalidate the node, causing it to be freed.
#define FLAG_ALLOC_VRAM_TEMP  0x2       // Node has been temporarily allocated 
                                        // Note that a DOS box or Direct Draw can 
                                        // invalidate the node, causing it to be freed.
#define FLAG_ALLOC_VRAM_PERM  0x4       // Permanent offscreen memory allocation. (not implemented yet)
#define FLAG_FREE_VRAM        0x8       // This Node exists in the free VRAM list
#define FLAG_RECENTLY_FREED   0x10      // This Node has been recently freed and should not be
                                        // into the free VRAM list right away as there is a good chance
                                        // the next alloc request will be for a block the same size
#define FLAG_PREDEFINED_VRAM  0x20      // This is one of the predefined blocks of memory. When freeing
                                        // these should never be merged into the free pool.
#define FLAG_GDI_SSB          0x40      // This is a block of memory which was allocated by SaveScreenBitmap to
                                        // temporarily store a section of the screen in the offscreen cache.                                        

#define FLAG_SENTINEL         0x80      // This Node is a sentinel for the head of a list

#define  MAX_BITMAP_WIDTH     2047      /* S3 hw limitation */
#define  MAX_BITMAP_HEIGHT    2047      /* S3 hw limitation */
#endif	//Not LRU
/*
END C code definitions.  BEGIN MASM definitions. NOTE: This is not currently used / needs to be updated! ~

ifdef   PERF_NEWMM
;***************************************
;  Bitmap Caching data structures
;***************************************
VRAMNODE	Struc
ListIndex           db      0
NodeFlags           db      0
Prev0               dw      0
Prev1               dw      0
Next0               dw      0
Next1               dw      0
dwSize              dd      0
dwStart             dd      0
lpOrigDib           dd      0
;Age                 dd      0
;Accessed            db      0

VRAMNODE	EndS

FREE_NODE               equ 00h
FLAG_GDI_BITMAP         equ 01h
FLAG_ALLOC_VRAM_TEMP    equ 02h
FLAG_ALLOC_VRAM_PERM    equ 04h
FLAG_FREE_VRAM          equ 08h
FLAG_SENTINEL           equ 80h

Endif	; PERF_NEWMM
;*/


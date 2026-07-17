/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Revision: 5$
** $Date: 10/11/00 8:57:19 PM$
**
*/

#ifndef __FXGLOBAL_H__
#define __FXGLOBAL_H__

#ifndef WINNT
#include <stdio.h>
#include "hw.h"
#include "ddvpe32.h"
#include "i2cfx.h"
#include "ddglobal.h"
#include "fifomgr.h"
#include "cmddefs.h"
#endif

#define cpuP6 6

#if defined( DEBUG )
extern ULONG renderTypes;
#endif

#if defined( TEX_CLAMP )
  #define kFPExpMask        0x7F800000UL
  #define kFPClampThreshold 0x20UL

  #define FP_FLOAT_CLAMP( fval )  fpClamp( fval )

extern __inline float fpClamp( float fval )
{
  FxU32 temp = ((FxU32)(((*(const FxU32*)(&(fval))) & kFPExpMask) >> 0x17UL));

  if(temp < kFPClampThreshold)
    return 0.0f;
  else
    return fval;
}
#else
  #define FP_FLOAT_CLAMP( fval ) fval
#endif

#define HEAP_SHARED          0x04000000UL   // put heap in shared memory
#define HEAP_INIT_SIZE       0x2000         //

#define GLOBAL_MEM_HEAP_PTR  ((HANDLE)_DS(hSharedHeap))
#define GLOBAL_MEM_INIT      (_FX(memInit))

// memory manager

#define LINEAR_HEAP0_ID    0L  //  linear heap     linear heap
#define TILED_HEAP0_ID     1L  //  tiled BB        tiled BB
#define TILED_HEAP1_ID     2L  //  tiled ZB        tiled ZB
#define TILED_HEAP2_ID     3L  //  tiled TB        tiled TB
#define LINEAR_HEAP1_ID    4L  //  linear heap     tiled FB (AA)
#define LINEAR_HEAP2_ID    5L  //  linear heap     tiled BB (AA)
#define LINEAR_HEAP3_ID    6L  //  linear heap     tiled ZB (AA)
#define LINEAR_HEAP4_ID    7L  //  linear heap     tiled TB (AA)

#define HEAP_INVALID       0x0000FFFFL
#define HEAP_MASK          0x0000FFFFL

// Flags stored in the upper word of heapID
						 
#define LFBPTR_TRANSLATED  0x40000000L

#define MEM_IN_TILED        1
#define MEM_IN_LINEAR       2

// global data for both ddraw and d3d (mostly related to fifo management)
typedef struct _FXGLOBAL
{
  unsigned long cpuType;

  // This is only needed for the Comdex reset fix.
  FxU32 resetLimit;
  FxU32 resetCount;
  FxU32 resetIdle;

  FxU32 forceReset;

  HANDLE        memHeap;                      // Heap Handle
  DWORD         memInit;                      // MemInit Semaphore

  // Keep track of outstanding locks
  DWORD    openLockCount;
  DWORD    openLockIndex;

  // Define multiple heaps for texture memory because textures can not
  // span 2M boundary. This is the number and heap ID for each of them
  #define D3D_MAX_NUM_TMUS 2
  DWORD   numTextureHeaps;
  DWORD   numTmus;
  DWORD   textureHeapStart[D3D_MAX_NUM_TMUS];
  DWORD   textureHeapLength[D3D_MAX_NUM_TMUS];
  DWORD   packerFixAddress;
  DWORD   flags;

  #define TRILINEAR_ON2TMUS     BIT(0)
  #define TRILINEAR_SPLIT2TMUS  BIT(1)
  #define FORCE_TRILINEAR       BIT(2)
  #define FORCE_BILINEAR        BIT(3)

  FxU32          *master_ptr;             /* Scanline Interleave Slave Addr  */
  FxU32          *slave_ptr;             /* Scanline Interleave Slave Addr  */
  FxBool          scanline_interleaved;

} fxGlobal;

extern fxGlobal _fxGlobal;

#define GETMEMTYPE(ptr)  \
  ((ptr) & 0x80000000) ? MEM_IN_TILED: MEM_IN_LINEAR

#define IS_TILED(ptr) \
  ((ptr) & 0x80000000) ? 1: 0

#define TWO_MEG_MEMORY  0x200000
#define FOUR_MEG_MEMORY 0x400000
#ifdef WINNT
#define _FX(Field)  (((fxGlobal *)(_FF(pfxglobal)))->Field)
#else
#define _FX(Field)  (((fxGlobal *)(_FF(pfxglobal)))->Field)
#endif
#define TRILINEAR_ON_2_TMUS    (_FX(flags) & TRILINEAR_ON2TMUS)
#define TRILINEAR_SPLIT_2_TMUS (_FX(flags) & TRILINEAR_SPLIT2TMUS)
#define NUM_AVAILABLE_TMUS     (_FX(numTmus))
#define TXTR_HEAP_SIZE(tmu)    (_FX(textureHeapLength[tmu]))

//memory manager calls
#ifdef WINNT
#if USE_NT5_DDMEMMGR
extern HRESULT memMgr_allocSurface(PDEV         *ppdev,
                                   DWORD        type,
                                   DWORD        width,
                                   DWORD        height,
                                   DWORD        tWidth,
                                   DWORD        tHeight,
                                   DWORD        *fpVidMem,
                                   DWORD        *hwVidMem,
                                   DWORD        *lpPitch,
                                   DWORD        *tileFlag,
                                   DWORD        *heapID,
                                   VIDEOMEMORY  **ppvmHeap);
extern void    memMgr_freeSurface (PDEV         *ppdev,
                                   DWORD        type,
                                   DWORD        fpVidMem,
                                   DWORD        hwVidMem,
                                   DWORD        tileFlag,
                                   DWORD        heapID,
                                   VIDEOMEMORY  *pvmHeap);
#endif
#else
extern DWORD memMgr_allocSurface(
  LPDDRAWI_DIRECTDRAW_GBL lpDD, // direct draw vidmemalloc need this
  DWORD type,       // type of surface (can use the standard DD surface flags) [IN]
  DWORD width,      // width in linear space [IN]
  DWORD height,     // height in linear space [IN]
  DWORD tWidth,     // width in tiled space [IN]
  DWORD tHeight,    // width in tiled space [IN]
  DWORD *fpVidMem,  // host lfb start address of allocation [OUT]
  DWORD *hwVidMem,  // hw vidmem address [OUT]
  DWORD *lpPitch,   // pitch [OUT]
  DWORD *tileFlag,  // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
  DWORD *heapID );  // Ddraw heap ID[OUT]

extern void memMgr_freeSurface(
  LPDDRAWI_DIRECTDRAW_GBL lpDD, // direct draw vidmemfree need this
  DWORD type,                   // type of surface (can use the standard DD surface flags) [IN]
  DWORD fpVidMem,               // host lfb start address of allocation [IN]
  DWORD hwVidMem,               // hw start address of allocation [IN]
  DWORD tileFlag,               // MEM_IN_TILED or MEM_IN_LINEAR [IN]
  DWORD heapID );               // Ddraw heap ID[IN]
#endif



#endif  /* __FXGLOBAL_H__ */

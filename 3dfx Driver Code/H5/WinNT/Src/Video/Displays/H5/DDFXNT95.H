/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Revision: 9$
** $Date: 10/11/00 8:57:05 PM$
**
*/

#ifndef __DDFXNT95_H__
#define __DDFXNT95_H__


// dx95type.h
#define NODXMALLOC
#include "dx95type.h"
#ifdef WINNT
#define DXMALLOC(n)   EngAllocMem(0, n, '3HxD')
#define DXMALLOCZ(n)  EngAllocMem(FL_ZERO_MEMORY, n, '3HxD')
#define DXFREE(p)     EngFreeMem((void *)p)
#else
#define DXMALLOC(n)   HeapAlloc(GLOBAL_MEM_HEAP_PTR, 0, n)
#define DXMALLOCZ(n)  HeapAlloc(GLOBAL_MEM_HEAP_PTR, HEAP_ZERO_MEMORY, n)
#define DXFREE(ptr)   HeapFree(GLOBAL_MEM_HEAP_PTR, 0, (void*) ptr)

#include "ddvpe32.h"
#include "i2cfx.h"
#endif

#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
#define Z_ACCESS_OPT          1
#endif

#include "ddglobal.h"
#include "fxglobal.h"


#ifdef WINNT

#define TDFX_VENDOR_ID        0x121a
#define TDFX_AVENGER_ID       0x0005
#define TDFX_NAPALM_ID        0x0007
#define TDFX_NAPALM2_ID       0x000B

#define IS_VOODOO3  (0x05 >= ppdev->usDeviceID)
#define IS_NAPALM   (0x06 <= ppdev->usDeviceID)

#ifndef SST_USE_APLHA_BIT
// Bit definition changed and common source makes this necessary
#define SST_USE_ALPHA_BIT                     BIT(3)
#endif
#ifndef SST_INTERLACED_EN
// Bit definition changed and common source makes this necessary
#define SST_INTERLACED_EN                     BIT(3)
#endif

#if (_WIN32_WINNT >= 0x0500)
#pragma message("ddraw version = " MakeString(Stringize, DIRECTDRAW_VERSION))
#pragma message("d3d   version = " MakeString(Stringize, DIRECT3D_VERSION))
#endif

  #include "cmddefs.h"
  #include "fifomgr.h"

#else
#include "fifomgr.h"
#endif


#ifdef WINNT
  #if _WIN32_WINNT >= 0x0500
    #ifdef USE_D3D_CODE
      #define ENABLE_3D     1
    #else
      #define ENABLE_3D     0
    #endif
    #define ENABLE_VIDEOPORT    1   // Enable VPE for W2K
  #else
    #define ENABLE_3D       0
    #define ENABLE_VIDEOPORT    0
  #endif
  #if ENABLE_RECONFIG_VIDMEM
    #define ENABLE_TILED_HEAP   1
  #else
    #define ENABLE_TILED_HEAP   0
  #endif
#else
#define ENABLE_3D           1
#define ENABLE_VIDEOPORT    1    //Enable VPE feature == 1, disable == 0
#define ENABLE_TILED_HEAP   1
#endif


#if ENABLE_3D
#ifdef WINNT
typedef PDD_SURFACE_INT               LPDDRAWI_DDRAWSURFACE_INT;
#define D3DHAL_TSS_MAXSTAGES          D3DNTHAL_TSS_MAXSTAGES

typedef struct _D3DNTHAL_DP2RENDERSTATE       *LPD3DHAL_DP2RENDERSTATE;
typedef struct _D3DNTHAL_DP2TEXTURESTAGESTATE *LPD3DHAL_DP2TEXTURESTAGESTATE;
typedef struct _D3DNTHAL_DP2WINFO             *LPD3DHAL_DP2WINFO;
typedef struct _D3DNTHAL_DP2TRIANGLESTRIP     *LPD3DHAL_DP2TRIANGLESTRIP;
typedef struct _D3DNTHAL_DP2STARTVERTEX       *LPD3DHAL_DP2STARTVERTEX;
typedef struct _D3DNTHAL_DP2TRIANGLEFAN_IMM   *LPD3DHAL_DP2TRIANGLEFAN_IMM;
typedef struct _D3DNTHAL_DP2TRIANGLEFAN       *LPD3DHAL_DP2TRIANGLEFAN;
typedef struct _D3DNTHAL_DP2POINTS            *LPD3DHAL_DP2POINTS;
typedef struct _D3DNTHAL_DP2LINELIST          *LPD3DHAL_DP2LINELIST;
typedef struct _D3DNTHAL_DP2INDEXEDLINELIST   *LPD3DHAL_DP2INDEXEDLINELIST;
typedef struct _D3DNTHAL_DP2INDEXEDLINESTRIP  *LPD3DHAL_DP2INDEXEDLINESTRIP;
typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLELIST   *LPD3DHAL_DP2INDEXEDTRIANGLELIST;
typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLELIST2  *LPD3DHAL_DP2INDEXEDTRIANGLELIST2;

// BobJ 12/07/1999 - In order to get Vertex Buffer support working in both
// Win9X and Win2K, we need to correct the mapping goof that 'Da Boyz from Redmon'
// made in thier making differnt structures for the Execute Buffer support
#define     DDHAL_DDEXEBUFCALLBACKS         DD_D3DBUFCALLBACKS
typedef struct _DD_D3DBUFCALLBACKS          *LPDDHAL_DDEXEBUFCALLBACKS;

#define NEW_CLIP_FOR_GB       1
#define SLI_AA_2D             1
#endif

// Set RC_LINKED_LIST = 1 to use the linked list implementation for D3D contexts
//                    = 0 to use the array implementation for D3D contexts
#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
// W2K
#define RC_LINKED_LIST    1
#else
// Win9x
#define RC_LINKED_LIST    1
#endif

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
// Set HL_LINKED_LIST = 1 to use the linked list implementation for DX7 handle lists
//                    = 0 to use the array implementation for DX7 handle lists
#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
// W2K
#define HL_LINKED_LIST    1
#else
// Win9x
#define HL_LINKED_LIST    1
#endif

#include "d7d3d.h"
#endif

#include "d3global.h"

#ifdef WINNT
#include "d6fvf.h"
#include "d3contxt.h"
#include "d6global.h"
#include "d3txtr.h"
#include "d3tri.h"
#endif
#endif


#ifdef WINNT
#define _FF(field)  ppdev->DriverData.field
#else
#ifdef GLOBALDATA_IN_PDEV
#undef _FF
#define _FF(field)  ppdev->DriverData.field
#endif
#undef _FF
#define _FF(field) (ppdev->field)
#endif


#ifndef WINNT
typedef VIDMEM    VIDEOMEMORY;
typedef LPVIDMEM  LPVIDEOMEMORY;

DECLARE_HANDLE(DHPDEV);

typedef unsigned long   _1616PTR;

// DIBENGINE structure
typedef struct _DIBENGINE
{
  WORD        deType;
  WORD        deWidth;
  WORD        deHeight;
  WORD        deWidthBytes;
  BYTE        dePlanes;
  BYTE        deBitsPixel;
  DWORD       deReserved1;
  DWORD       deDeltaScan;
  _1616PTR    delpPDevice;
  DWORD       deBitsOffset;
  WORD        deBitsSelector;
  WORD        deFlags;
  WORD        deVersion;
  _1616PTR    deBitmapInfo;
  _1616PTR    deBeginAccess;
  _1616PTR    deEndAccess;
  DWORD       deDriverReserved;
} DIBENGINE;

typedef struct _PDEV
{
  DIBENGINE   dibEngine;
#ifdef GLOBALDATA_IN_PDEV
  GLOBALDATA  DriverData;
#endif
} PDEV;
#endif  // ifndef WINNT


#ifndef WINNT
extern HINSTANCE  hInstance;
extern HANDLE     hSharedHeap;
#endif

#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
//-----------------------------------------------------------------------------
//
// LfbPtrToHwPtr
//
// Translate a lfbPtr in tiled memory to a hwPtr
//
//-----------------------------------------------------------------------------

static __inline DWORD
LfbPtrToHwPtr(PDEV *ppdev, DWORD lfbPtr)
{
  DWORD tileOffset;
  DWORD tileInY;
  DWORD tileInX;
  DWORD hwPtr;


  ASSERTDD(0 != _FF(ddTiledHeapSize), "no tiled heap!");
  ASSERTDD(lfbPtr >= _FF(ddTiledHeapStart), "lfbPtr not in tiled memory!");

  tileOffset = lfbPtr - _FF(ddTiledHeapStart);

  tileInY = (tileOffset / _FF(ddTilePitch)) / SST_TILE_HEIGHT;
  tileInX = (tileOffset % _FF(ddTilePitch)) / SST_TILE_WIDTH;

  hwPtr = ((tileInY * _FF(ddTileStride) + tileInX) * SST_TILE_SIZE) + _FF(ddTiledHeapStart);

  DISPDBG((6, "LfbPtrToHwPtr: lfbPtr=%8lXh, hwPtr=%8lXh  (tileOffset=%8lXh, tileInX=%8lXh, tileInY=%8lXh)",
           lfbPtr, hwPtr, tileOffset, tileInX, tileInY));

  return hwPtr;
} // LfbPtrToHwPtr

//-----------------------------------------------------------------------------
//
// HwPtrToLfbPtr
//
// Translate a hwPtr in tiled memory to a lfbPtr
//
//-----------------------------------------------------------------------------

static __inline DWORD
HwPtrToLfbPtr(PDEV *ppdev, DWORD hwPtr)
{
  DWORD tileOffset;
  DWORD tileInY;
  DWORD tileInX;
  DWORD lfbPtr;


  ASSERTDD(0 != _FF(ddTiledHeapSize), "no tiled heap!");
  ASSERTDD(hwPtr >= _FF(ddTiledHeapStart), "hwPtr not in tiled memory!");

  tileOffset = hwPtr - _FF(ddTiledHeapStart);

  tileInY = (tileOffset / SST_TILE_SIZE) / _FF(ddTileStride);
  tileInX = (tileOffset / SST_TILE_SIZE) % _FF(ddTileStride);

  lfbPtr = ((tileInY * SST_TILE_HEIGHT) * _FF(ddTilePitch)) + (tileInX * SST_TILE_WIDTH) + _FF(ddTiledHeapStart);

  DISPDBG((6, "HwPtrToLfbPtr: hwPtr=%8lXh, lfbPtr=%8lXh  (tileOffset=%8lXh, tileInX=%8lXh, tileInY=%8lXh)",
           hwPtr, lfbPtr, tileOffset, tileInX, tileInY));

  return lfbPtr;
} // HwPtrToLfbPtr

#ifdef SLI_AA
//-----------------------------------------------------------------------------
//
// SLI_LfbPtrToHwPtr
//
// Translate a lfbPtr in tiled memory to a hwPtr
//
//-----------------------------------------------------------------------------

static __inline DWORD
SLI_LfbPtrToHwPtr(PDEV *ppdev, DWORD lfbPtr)
{
  DWORD tileOffset;
  DWORD tileInY;
  DWORD tileInX;
  DWORD hwPtr;


  ASSERTDD(0 != _FF(ddTiledHeapSize), "no tiled heap!");
  ASSERTDD(lfbPtr >= _FF(ddTiledHeapStart), "lfbPtr not in tiled memory!");

  tileOffset = lfbPtr - _FF(ddTiledHeapStart);

  tileInY = (tileOffset / _FF(ddTilePitch)) / SST_TILE_HEIGHT;
  tileInX = (tileOffset % _FF(ddTilePitch)) / SST_TILE_WIDTH;

  // yOffset calculation
  hwPtr  = ((tileInY * _FF(ddTileStride)) * SST_TILE_SIZE) / _FF(dwNumUnits);
  // add on xOffset
  hwPtr += tileInX * SST_TILE_SIZE;
  hwPtr += _FF(ddTiledHeapStart);

  DISPDBG((6, "SLI_LfbPtrToHwPtr: lfbPtr=%8lXh, hwPtr=%8lXh  (tileOffset=%8lXh, tileInX=%8lXh, tileInY=%8lXh)",
           lfbPtr, hwPtr, tileOffset, tileInX, tileInY));

  return hwPtr;
} // SLI_LfbPtrToHwPtr

//-----------------------------------------------------------------------------
//
// SLI_HwPtrToLfbPtr
//
// Translate a hwPtr in tiled memory to a lfbPtr
//
//-----------------------------------------------------------------------------

static __inline DWORD
SLI_HwPtrToLfbPtr(PDEV *ppdev, DWORD hwPtr)
{
  DWORD tileOffset;
  DWORD tileInY;
  DWORD tileInX;
  DWORD lfbPtr;


  ASSERTDD(0 != _FF(ddTiledHeapSize), "no tiled heap!");
  ASSERTDD(hwPtr >= _FF(ddTiledHeapStart), "hwPtr not in tiled memory!");

  tileOffset = hwPtr - _FF(ddTiledHeapStart);

  tileInY = (tileOffset / SST_TILE_SIZE) / _FF(ddTileStride);
  tileInX = (tileOffset / SST_TILE_SIZE) % _FF(ddTileStride);

  // yOffset calculation
  lfbPtr  = ((tileInY * SST_TILE_HEIGHT) * _FF(ddTilePitch)) * _FF(dwNumUnits);
  lfbPtr += (tileInX * SST_TILE_WIDTH);
  lfbPtr += _FF(ddTiledHeapStart);

  DISPDBG((6, "SLI_HwPtrToLfbPtr: hwPtr=%8lXh, lfbPtr=%8lXh  (tileOffset=%8lXh, tileInX=%8lXh, tileInY=%8lXh)",
           hwPtr, lfbPtr, tileOffset, tileInX, tileInY));

  return lfbPtr;
} // SLI_HwPtrToLfbPtr
#endif
#endif

#endif /* __DDFXNT95_H__ */

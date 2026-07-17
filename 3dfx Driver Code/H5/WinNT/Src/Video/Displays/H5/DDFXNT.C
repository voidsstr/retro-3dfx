/*
** Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
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
** $Revision: 4$
** $Date: 10/11/00 8:56:42 PM$
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"
#include "regkeys.h"

/**************************************************************************
* D E F I N E S
***************************************************************************/

//#define ENABLE_REG_IOCTL_TEST

/**************************************************************************
* F U N C T I O N   P R O T O T Y P E S
***************************************************************************/

#if 0
VOID vGetDisplayDuration(PDEV* ppdev);
#endif
#if defined(ENABLE_REG_IOCTL_TEST)
static VOID TestRegIoctl(PDEV*);
#endif

static BOOL GetSLIAAInfo(PDEV* ppdev);

int __cdecl atoi ( const char *pszString );

#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
GLIDESTATE * hwcGetGlideStateStructureForLFBMapping( PDEV * ppdev, PVOID pvLFBMapping );
VOID hwcSetContextDWORDBit( HANDLE hProcess, DWORD dwBitMask );
#endif

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

/******************************Public*Routine******************************\
* DWORD DdMapMemory
*
* This is a new DDI call specific to Windows NT that is used to map
* or unmap all the application modifiable portions of the frame buffer
* into the specified process's address space.
*
\**************************************************************************/

DWORD DdMapMemory(
PDD_MAPMEMORYDATA lpMapMemory)
{
    PDEV*                           ppdev;
    VIDEO_SHARE_MEMORY              ShareMemory;
    VIDEO_SHARE_MEMORY_INFORMATION  ShareMemoryInformation;
    DWORD                           ReturnedDataLength;
#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
    VIDEO_SHARE_MEMORY              sUnMapShareMemory;
    GLIDESTATE                      *psGlideState;
#endif

    ppdev = (PDEV*) lpMapMemory->lpDD->dhpdev;

    if (lpMapMemory->bMap)
    {
        ShareMemory.ProcessHandle = lpMapMemory->hProcess;

        // 'RequestedVirtualAddress' isn't actually used for the SHARE IOCTL:

        ShareMemory.RequestedVirtualAddress = 0;

        // We map in starting at the top of the frame buffer:

        ShareMemory.ViewOffset = 0;

        // We map down to the end of the frame buffer.
        //
        // Note: There is a 64k granularity on the mapping (meaning that
        //       we have to round up to 64k).
        //
        // Note: If there is any portion of the frame buffer that must
        //       not be modified by an application, that portion of memory
        //       MUST NOT be mapped in by this call.  This would include
        //       any data that, if modified by a malicious application,
        //       would cause the driver to crash.  This could include, for
        //       example, any DSP code that is kept in off-screen memory.

        // jdw - fix this (miniport?). 64k round could expose non-modifiable stuff.

#if ENABLE_RECONFIG_VIDMEM
        ShareMemory.ViewSize
            = ROUND_UP_TO_64K((ppdev->cyScreen) * ppdev->lDelta + ppdev->ulScreenOffset);
#else
        ShareMemory.ViewSize
            = ROUND_UP_TO_64K((ppdev->cyMemory + ppdev->cyDDMemoryExtra)
                              * ppdev->lDelta + ppdev->ulScreenOffset);
#endif

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
                               &ShareMemory,
                               sizeof(VIDEO_SHARE_MEMORY),
                               &ShareMemoryInformation,
                               sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
                               &ReturnedDataLength))
        {
            DISPDBG((0, "Failed IOCTL_VIDEO_SHARE_MEMORY"));

            lpMapMemory->ddRVal = DDERR_GENERIC;
            return(DDHAL_DRIVER_HANDLED);
        }

        lpMapMemory->fpProcess = (DWORD) ShareMemoryInformation.VirtualAddress;
    }
    else
    {
#ifdef ENABLE_V3_W2K_GLIDE_CHANGES
        sUnMapShareMemory.ProcessHandle           = lpMapMemory->hProcess;
        sUnMapShareMemory.ViewOffset              = 0;
        sUnMapShareMemory.ViewSize                = 0;
        sUnMapShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;

        // check to see if the current mapping belongs to a GLIDE application, if so then don't 
        // unmap the memory, simply set a bit in the lost-context flag and store the data to 
        // be used for unmapping later
        if (psGlideState = hwcGetGlideStateStructureForLFBMapping(ppdev,
                                                                  (PVOID)lpMapMemory->fpProcess))
        {
            // set a bit in the lost-context DWORD to indicate to glide that it's memory
            // mappings should be unmapped
            hwcSetContextDWORDBit( psGlideState->glideProcessHandle, 0x80000000 );
            
            // set the flag that indicates the glide mapping should be undone
            psGlideState->dwUnMapMemoryLFB++;
            
            // copy the unmapping structure into the GLIDE state structure
            memcpy( &psGlideState->sUnMapShareMemory, &sUnMapShareMemory, sizeof( VIDEO_SHARE_MEMORY ) );
            
            // spin down the command FIFO to ensure that all of the command FIFOS owned by GLIDE are
            // processed and not pending processing
            while(H3_GP_BUSY(ppdev, ppdev->pjH3Base));
        }
        else
        {
            // we need to set the context dword to indicate to GLIDE that it has lost the context
            hwcSetContextDWORD();
            
            // unmap the memory because GLIDE does not own it
            if (EngDeviceIoControl(ppdev->hDriver,
                                   IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
                                   &sUnMapShareMemory,
                                   sizeof(VIDEO_SHARE_MEMORY),
                                   NULL,
                                   0,
                                   &ReturnedDataLength))
            {
                RIP("Failed IOCTL_VIDEO_UNSHARE_MEMORY");
            }
        }
#else
        ShareMemory.ProcessHandle           = lpMapMemory->hProcess;
        ShareMemory.ViewOffset              = 0;
        ShareMemory.ViewSize                = 0;
        ShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
                               &ShareMemory,
                               sizeof(VIDEO_SHARE_MEMORY),
                               NULL,
                               0,
                               &ReturnedDataLength))
        {
            RIP("Failed IOCTL_VIDEO_UNSHARE_MEMORY");
        }
#endif
    }

    lpMapMemory->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

/******************************Public*Routine******************************\
* VOID DrvDisableDirectDraw
*
* This function is called by GDI when the last active DirectDraw program
* is quit and DirectDraw will no longer be active.
*
\**************************************************************************/

VOID DrvDisableDirectDraw(
DHPDEV      dhpdev)
{
    PDEV* ppdev;

    ppdev = (PDEV*) dhpdev;

    FXBUSYWAIT(ppdev);

    // DirectDraw is done with the display, so we can go back to using
    // all of off-screen memory ourselves

#if !USE_NT5_DDMEMMGR
    // convert DDraw memory allocation from permanent to reserved
    bOhCommit(ppdev, ppdev->pohDirectDraw, FALSE);
#endif
}

/******************************Public*Routine******************************\
* VOID vAssertModeDirectDraw
*
* This function is called by enable.c when entering or leaving the
* DOS full-screen character mode.
*
\**************************************************************************/

VOID vAssertModeDirectDraw(
PDEV*   ppdev,
BOOL    bEnable)
{
}

/******************************Public*Routine******************************\
* BOOL bEnableDirectDraw
*
* This function is called by enable.c when the mode is first initialized,
* right after the miniport does the mode-set.
*
\**************************************************************************/

BOOL bEnableDirectDraw(
PDEV*   ppdev)
{
#if !USE_NT5_DDMEMMGR
    OH      *poh;
#endif
    DWORD   height;
    char    *pDDEnabled;

    pDDEnabled = GETENV(DDRAW_ENABLED);
    if (NULL != pDDEnabled)
    {
      if (0 == atoi(pDDEnabled))
      {
        // read Misc Output reg and save bit 7 (vsync polarity) in ddMiscFlags
        _FF(ddMiscFlags) &= ~DDMF_VSYNC_POLARITY_MASK;
        _FF(ddMiscFlags) |= (inp(ppdev->pjIoBase + 0xCC) & 0x80) >> (7 - DDMF_VSYNC_POLARITY_BIT);

        // DirectDraw is disabled for use on this card
        ppdev->flStatus &= ~STAT_DIRECTDRAW;

        return TRUE;
      }
    }

#if defined(ENABLE_REG_IOCTL_TEST)
    TestRegIoctl(ppdev);
#endif

    //allocate memory for ddglobal
#ifdef MEMCHECK
    (LPVOID)_FF(pddglobal) = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(DDGLOBAL), '3HxD', &_FF(pddglobal));
#else
    (LPVOID)_FF(pddglobal) = DXMALLOCZ(sizeof(DDGLOBAL));
#endif
    if (NULL == (LPVOID)_FF(pddglobal))
      return FALSE;

    //allocate memory for fxglobal
#ifdef MEMCHECK
    (LPVOID)_FF(pfxglobal) = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(fxGlobal), '3HxD', &_FF(pfxglobal));
#else
    (LPVOID)_FF(pfxglobal) = DXMALLOCZ(sizeof(fxGlobal));
#endif
    if (NULL == (LPVOID)_FF(pfxglobal))
      return FALSE;

#if ENABLE_3D
    // allocate memory for d3global
#ifdef MEMCHECK
    (LPVOID)_FF(pd3global) = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(d3Global), '3HxD', &_FF(pd3global));
#else
    (LPVOID)_FF(pd3global) = DXMALLOCZ(sizeof(d3Global));
#endif
    if (NULL == (LPVOID)_FF(pd3global))
      return FALSE;
#endif

#if 0
    // Accurately measure the refresh rate for later:
    vGetDisplayDuration(ppdev);
#endif

    // copy rgb bitmasks from pdev into pdev->DriverData
    _FF(dwRBitMask) = ppdev->flRed;
    _FF(dwGBitMask) = ppdev->flGreen;
    _FF(dwBBitMask) = ppdev->flBlue;

    // more pdev->DriverData initialization

#if defined(H3_A0) || defined(H3_A1) || defined(H3_A2) || defined(H3_A3)
    // rev A silicon has a problem with scanline doubling so
    // low res modes (512x384x16 and below) use the overlay to display the mode
    // so set dd3DInOverlay to TRUE for those modes so ddraw page flipping
    // will flip the overlay
    // for higher res modes, set dd3DInOverlay to FALSE so normal flipping is done
    if (512 >= ppdev->cxScreen)
      _FF(dd3DInOverlay) = TRUE;
    else
#endif
      _FF(dd3DInOverlay) = FALSE;
    _FF(lastOverlayAddress) = INVALID_ADDRESS;
    _FF(pitch) = ppdev->lDelta;
    _DD(WaitOnVsync) = 1;

    // read Misc Output reg and save bit 7 (vsync polarity) in ddMiscFlags
    _FF(ddMiscFlags) &= ~DDMF_VSYNC_POLARITY_MASK;
    _FF(ddMiscFlags) |= (inp(ppdev->pjIoBase + 0xCC) & 0x80) >> (7 - DDMF_VSYNC_POLARITY_BIT);

#define STRETCHBLTSIZE    (8*1024)
#define SWAPCOUNTSIZE     (4)
#define DDSLOPSIZE        (STRETCHBLTSIZE+SWAPCOUNTSIZE)
    // allocate permananent STRETCHBLTSIZE heap for ddraw blt code

    // compute how many scanlines are needed to eat up at least STRETCHBLTSIZE
    // of memory
    height = (DDSLOPSIZE + ppdev->cxMemory * ppdev->cjPelSize - 1)
             / (ppdev->cxMemory * ppdev->cjPelSize);
#if USE_NT5_DDMEMMGR
    if ((DWORD)(ppdev->cyMemory - (ppdev->cyScreen + ppdev->cyText)) > height)
    {
      DISPDBG((1, "DirectDraw slop gets %lXh x %lXh surface at (%lXh, %lXh)",
              ppdev->cxMemory, height, 0,  ppdev->cyScreen + ppdev->cyText));

      ppdev->cyDDSlopHeight = height;

      _DS(ddSwapCount) = ppdev->ulScreenOffset +
                         (ppdev->cyScreen + ppdev->cyText) * ppdev->lDelta;
      _DS(stretchBltStart) = _DS(ddSwapCount) + SWAPCOUNTSIZE;
      _DS(stretchBltSize) = ppdev->cxMemory * ppdev->cjPelSize * height - SWAPCOUNTSIZE;
    }
#else
    poh = pohAllocate(ppdev,NULL,ppdev->cxMemory,height,FLOH_MAKE_PERMANENT);
    if (NULL != poh)
    {
      DISPDBG((1, "DirectDraw slop gets %lXh x %lXh surface at (%lXh, %lXh)",
              poh->cx, poh->cy, poh->x,  poh->y));

      _DS(ddSwapCount) = ppdev->ulScreenOffset +
                         (poh->y * ppdev->lDelta) +
                         (poh->x * ppdev->cjPelSize);
      _DS(stretchBltStart) = _DS(ddSwapCount) + SWAPCOUNTSIZE;
      _DS(stretchBltSize)  = poh->cx * ppdev->cjPelSize * poh->cy - SWAPCOUNTSIZE;
    }
#endif
    else
    {
      // skip ddraw support if we can't allocate the stretchBlt heap
      _DS(stretchBltStart) = 0;
      _DS(stretchBltSize)  = 0;
      ppdev->flStatus &= ~STAT_DIRECTDRAW;
      return TRUE;
    }

    // reserve all of non-permanent offscreen memory
    // for ddraw
#if USE_NT5_DDMEMMGR
    if ((ppdev->cyMemory - (ppdev->cyScreen + ppdev->cyText + ppdev->cyDDSlopHeight)) > 0)
    {
      ppdev->cyDDHeap = ppdev->cyMemory - (ppdev->cyScreen + ppdev->cyText + ppdev->cyDDSlopHeight);

      DISPDBG((1, "DirectDraw reserves %lXh x %lXh surface at (%lXh, %lXh)",
              ppdev->cxMemory, ppdev->cyDDHeap,
              0, ppdev->cyScreen + ppdev->cyText + ppdev->cyDDSlopHeight));

    }
    else
      ppdev->cyDDHeap = 0;

//#define PAINT_RECTS
#ifdef PAINT_RECTS
    {
      extern void PaintDFBRect(PDEV *, DWORD, DWORD, DWORD);
      extern DWORD InitFillColor;

      if (ppdev->cyDDHeap)
        PaintDFBRect(ppdev,
                     H3_PACKXY(0,(ppdev->cyScreen+ppdev->cyText+ppdev->cyDDSlopHeight)),
                     H3_PACKXY_FAST(ppdev->cxMemory,ppdev->cyDDHeap),
                     InitFillColor);
      if (ppdev->cyDDMemoryExtra)
        PaintDFBRect(ppdev,
                     H3_PACKXY(0,ppdev->cyMemory),
                     H3_PACKXY_FAST(ppdev->cxMemory,ppdev->cyDDMemoryExtra),
                     InitFillColor);

  }
#endif
#else
    if ((0 < ppdev->heap.cxMax) && (0 < ppdev->heap.cyMax))
    {
      bMoveAllDfbsFromOffscreenToDibs(ppdev);
      ppdev->pohDirectDraw = pohAllocate(ppdev,
                                         NULL,
                                         ppdev->heap.cxMax,
                                         ppdev->heap.cyMax,
                                         FLOH_RESERVE);
    }
    else
    {
      ppdev->pohDirectDraw = NULL;
    }

    if (NULL != ppdev->pohDirectDraw)
    {
      DISPDBG((1, "DirectDraw reserves %lXh x %lXh surface at (%lXh, %lXh)",
              ppdev->pohDirectDraw->cx, ppdev->pohDirectDraw->cy,
              ppdev->pohDirectDraw->x, ppdev->pohDirectDraw->y));

    }
#endif

#ifdef SLI_AA
    GetSLIAAInfo(ppdev);
#else
    _FF(dwNumUnits) = 1;
#endif
    // DirectDraw is enabled for use on this card
    ppdev->flStatus |= STAT_DIRECTDRAW;

    return(TRUE);
}

/******************************Public*Routine******************************\
* VOID vDisableDirectDraw
*
* This function is called by enable.c when the driver is shutting down.
*
\**************************************************************************/

VOID vDisableDirectDraw(
PDEV*   ppdev)
{
  //free memory for ddglobal
  if (_FF(pddglobal))
  {
#ifdef MEMCHECK
    ENGFREEMEM(_FF(pddglobal));
#else
    DXFREE((LPVOID)_FF(pddglobal));
#endif
    _FF(pddglobal) = 0;
  }

  //free memory for fxglobal
  if (_FF(pfxglobal))
  {
#ifdef MEMCHECK
    ENGFREEMEM(_FF(pfxglobal));
#else
    DXFREE((LPVOID)_FF(pfxglobal));
#endif
    _FF(pfxglobal) = 0;
  }

#if ENABLE_3D
  if (_FF(pd3global))
  {
    // release allocations made inD3DHalCreateDriver()

    // free _D3(contexts)
    if (_D3(contexts))
      DXFREE(_D3(contexts));

    // free _D3(contextMap)
    if (_D3(contextMap))
      DXFREE(_D3(contextMap));

    // free _D3(txtrDesc)
    if (_D3(txtrDesc))
      DXFREE(_D3(txtrDesc));

    // free _D3(txtrHndl)
    if (_D3(txtrHndl))
      DXFREE(_D3(txtrHndl));

    // free _D3(buffer)
    if (_D3(buffer))
      DXFREE(_D3(buffer));

    //free memory for d3global
#ifdef MEMCHECK
    ENGFREEMEM(_FF(pd3global));
#else
    DXFREE((LPVOID)_FF(pd3global));
#endif
    _FF(pd3global) = 0;
  }
#endif

#if !USE_NT5_DDMEMMGR
  if (NULL != ppdev->pohDirectDraw)
  {
    pohFree(ppdev, ppdev->pohDirectDraw);
    ppdev->pohDirectDraw = NULL;
  }
#endif
}

#if (_WIN32_WINNT >= 0x0500)
/****************************************************************************
*
* FUNCTION:     DdGetDriverInfo()
*
* DESCRIPTION:
*
****************************************************************************/

DWORD __stdcall
DdGetDriverInfo ( LPDDHAL_GETDRIVERINFODATA lpInput )
{
  DWORD dwSize;
  PDEV  *ppdev = lpInput->dhpdev;


#ifdef DBG
#define CHECKSIZE(x) if (lpInput->dwExpectedSize != sizeof(x)) \
            DISPDBG((0, "GetDriverInfo: #x structure size mismatch"));
#else
#define CHECKSIZE(x) ((void)0)
#endif

  lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;

#if ENABLE_3D
  if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DCallbacks2))
  {
    extern DWORD __stdcall mySetRenderTarget32 ( LPD3DHAL_SETRENDERTARGETDATA psrtd );

    D3DHAL_CALLBACKS2 D3DCallbacks2;


    DISPDBG((0, "Get D3D Callbacks2"));

    memset(&D3DCallbacks2, 0, sizeof(D3DCallbacks2));

    dwSize = min(lpInput->dwExpectedSize, sizeof(D3DHAL_CALLBACKS2));
    lpInput->dwActualSize = sizeof(D3DHAL_CALLBACKS2);

    CHECKSIZE(D3DHAL_CALLBACKS2);

    D3DCallbacks2.dwSize = dwSize;
    D3DCallbacks2.dwFlags = 0
                          | D3DHAL2_CB32_SETRENDERTARGET
                          ;

    D3DCallbacks2.SetRenderTarget =  mySetRenderTarget32;

    memcpy(lpInput->lpvData, &D3DCallbacks2, dwSize);
    lpInput->ddRVal = DD_OK;
  }
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DParseUnknownCommandCallback))
  {
    DISPDBG((0, "Get D3DParseUnknownCommandCallback"));

    ppdev->pD3DParseUnknownCommand = lpInput->lpvData;

    ASSERTDD((ppdev->pD3DParseUnknownCommand),
             "D3D DX6 ParseUnknownCommand callback == NULL");

    lpInput->ddRVal = DD_OK;
  }
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DCallbacks3) )
  {
    // extern DWORD __stdcall ddiClear2 ( LPD3DHAL_CLEAR2DATA pcd );
    extern DWORD __stdcall ddiValidateTextureStageState( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pcd );
    extern DWORD __stdcall ddiDrawPrimitives2( LPD3DHAL_DRAWPRIMITIVES2DATA pcd );

    D3DHAL_CALLBACKS3 D3DCallbacks3;


    DISPDBG((0, "Get D3D Callbacks3"));

    memset(&D3DCallbacks3, 0, sizeof(D3DCallbacks3));

    dwSize = min(lpInput->dwExpectedSize, sizeof(D3DHAL_CALLBACKS3));
    lpInput->dwActualSize = sizeof(D3DHAL_CALLBACKS3);

    CHECKSIZE( D3DHAL_CALLBACKS3 );

    D3DCallbacks3.dwSize = dwSize;
    D3DCallbacks3.dwFlags = 0
                          //| D3DHAL3_CB32_CLEAR2
                          | D3DHAL3_CB32_VALIDATETEXTURESTAGESTATE
                          | D3DHAL3_CB32_DRAWPRIMITIVES2
                          ;

    //D3DCallbacks3.Clear2                    = ddiClear2;
    D3DCallbacks3.Clear2                    = NULL;
    D3DCallbacks3.lpvReserved               = NULL;
    D3DCallbacks3.ValidateTextureStageState = ddiValidateTextureStageState;
    D3DCallbacks3.DrawPrimitives2           = ddiDrawPrimitives2;

    memcpy(lpInput->lpvData, &D3DCallbacks3, dwSize);
    lpInput->ddRVal = DD_OK;
  }
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DExtendedCaps))
  {
    D3DNTHAL_D3DEXTENDEDCAPS D3DExtendedCaps;


    DISPDBG((0, "Get D3D Extended caps"));

    memset(&D3DExtendedCaps, 0, sizeof(D3DExtendedCaps));
    dwSize = min(lpInput->dwExpectedSize, sizeof(D3DNTHAL_D3DEXTENDEDCAPS));
    lpInput->dwActualSize = sizeof(D3DNTHAL_D3DEXTENDEDCAPS);

    CHECKSIZE(D3DNTHAL_D3DEXTENDEDCAPS);

    D3DExtendedCaps.dwSize = dwSize;
    D3DExtendedCaps.dwMinTextureWidth  = 1;
    D3DExtendedCaps.dwMaxTextureWidth  = 256;
    D3DExtendedCaps.dwMinTextureHeight = 1;
    D3DExtendedCaps.dwMaxTextureHeight = 256;

    D3DExtendedCaps.dwMaxTextureRepeat        = 32768;
    D3DExtendedCaps.dwMaxTextureAspectRatio   = 256;
    D3DExtendedCaps.dwMaxAnisotropy           = 1;
    D3DExtendedCaps.dvGuardBandLeft           = 0.f;
    D3DExtendedCaps.dvGuardBandTop            = 0.f;
    D3DExtendedCaps.dvGuardBandRight          = 0.f;
    D3DExtendedCaps.dvGuardBandBottom         = 0.f;
    D3DExtendedCaps.dvExtentsAdjust           = 0.f;
    D3DExtendedCaps.dwStencilCaps             = 0;
    D3DExtendedCaps.dwFVFCaps                 = NUMTEXTUREUNITS;
    D3DExtendedCaps.dwTextureOpCaps           = D3DTEXOPCAPS_DISABLE |
                                                D3DTEXOPCAPS_SELECTARG1 |
                                                D3DTEXOPCAPS_SELECTARG2 |
                                                D3DTEXOPCAPS_MODULATE |
                                                //D3DTEXOPCAPS_MODULATE2X |
                                                //D3DTEXOPCAPS_MODULATE4X |
                                                D3DTEXOPCAPS_ADD |
                                                //D3DTEXOPCAPS_ADDSIGNED |
                                                //D3DTEXOPCAPS_ADDSIGNED2X |
                                                D3DTEXOPCAPS_SUBTRACT |
                                                //D3DTEXOPCAPS_ADDSMOOTH |
                                                D3DTEXOPCAPS_BLENDDIFFUSEALPHA |
                                                D3DTEXOPCAPS_BLENDTEXTUREALPHA |
                                                //D3DTEXOPCAPS_BLENDFACTORALPHA |
                                                //D3DTEXOPCAPS_BLENDTEXTUREALPHAPM |
                                                //D3DTEXOPCAPS_BLENDCURRENTALPHA |
                                                //D3DTEXOPCAPS_PREMODULATE |
                                                //D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR |
                                                //D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA |
                                                //D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
                                                //D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
                                                //D3DTEXOPCAPS_BUMPENVMAP |
                                                //D3DTEXOPCAPS_BUMPENVMAPLUMINANCE |
                                                //D3DTEXOPCAPS_BUMPMAPLIGHT |
                                                0;
    D3DExtendedCaps.wMaxTextureBlendStages    = NUMTEXTUREUNITS + 1;
    D3DExtendedCaps.wMaxSimultaneousTextures  = NUMTEXTUREUNITS;

    memcpy(lpInput->lpvData, &D3DExtendedCaps, dwSize);
    lpInput->ddRVal = DD_OK;
  }
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_ZPixelFormats))
  {
    DDPIXELFORMAT ddZBufPixelFormat;
    DWORD         dwNumZPixelFormats;


    DISPDBG((0, "Get Z Pixel Formats"));
    memset(&ddZBufPixelFormat, 0, sizeof(ddZBufPixelFormat));
    dwSize = min(lpInput->dwExpectedSize, sizeof(DDPIXELFORMAT));

    lpInput->dwActualSize = sizeof(DDPIXELFORMAT) + sizeof(DWORD);

    // We only fill one 16-bit Z Buffer format since that is all
    // what the Virge supports. Drivers have to report here all
    // Z Buffer formats supported only if they support the Clear2
    // callback. We implement it here for illustration purposes.

    dwNumZPixelFormats = 1;

    ddZBufPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddZBufPixelFormat.dwFlags = DDPF_ZBUFFER;
    ddZBufPixelFormat.dwFourCC = 0;
    ddZBufPixelFormat.dwZBufferBitDepth = 16;
    ddZBufPixelFormat.dwStencilBitDepth = 0;
    ddZBufPixelFormat.dwZBitMask = 0xFFFF;
    ddZBufPixelFormat.dwStencilBitMask = 0x0000;
    ddZBufPixelFormat.dwRGBZBitMask = 0;

    memcpy(lpInput->lpvData, &dwNumZPixelFormats, sizeof(DWORD));
    memcpy((LPVOID)((LPBYTE)(lpInput->lpvData) + sizeof(DWORD)),
           &ddZBufPixelFormat, dwSize);

    lpInput->ddRVal = DD_OK;
  }
#if 0
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_NTPrivateDriverCaps) )
  {
    DD_NTPRIVATEDRIVERCAPS ddPrivateCaps;


    memset(&ddPrivateCaps, 0, sizeof(ddPrivateCaps));

    dwSize = min(lpInput->dwExpectedSize, sizeof(ddPrivateCaps));
    lpInput->dwActualSize = sizeof(ddPrivateCaps);

    ddPrivateCaps.dwSize = dwSize;
    ddPrivateCaps.dwPrivateCaps = 0
                                | DDHAL_PRIVATECAP_ATOMICSURFACECREATION
                                //| DDHAL_PRIVATECAP_NOTIFYPRIMARYCREATION
                                ;

    memcpy(lpInput->lpvData, &ddPrivateCaps, dwSize);
    lpInput->ddRVal = DD_OK;
	}
#endif
  else
#endif // ENABLE_3D
  if (IsEqualIID(&lpInput->guidInfo, &GUID_NTCallbacks))
  {
#if USE_NT5_DDMEMMGR
    extern DWORD __stdcall DdFreeDriverMemory ( PDD_FREEDRIVERMEMORYDATA pfdmd );
#endif
    extern DWORD __stdcall DdSetExclusiveMode( PDD_SETEXCLUSIVEMODEDATA psemd );
    extern DWORD __stdcall DdFlipToGDISurface( PDD_FLIPTOGDISURFACEDATA pftgs );

    DD_NTCALLBACKS NtCallbacks;


    DISPDBG((0, "Get NT Callbacks"));
    memset(&NtCallbacks, 0, sizeof(NtCallbacks));
    dwSize = min(lpInput->dwExpectedSize, sizeof(DD_NTCALLBACKS));

    lpInput->dwActualSize = sizeof(DD_NTCALLBACKS);

    NtCallbacks.dwSize  = dwSize;
    NtCallbacks.dwFlags = 0
#if USE_NT5_DDMEMMGR
                        | DDHAL_NTCB32_FREEDRIVERMEMORY
#endif
                        | DDHAL_NTCB32_SETEXCLUSIVEMODE
                        | DDHAL_NTCB32_FLIPTOGDISURFACE
                        ;
#if USE_NT5_DDMEMMGR
    NtCallbacks.FreeDriverMemory = DdFreeDriverMemory;
#endif
    NtCallbacks.SetExclusiveMode = DdSetExclusiveMode;
    NtCallbacks.FlipToGDISurface = DdFlipToGDISurface;

    memcpy(lpInput->lpvData, &NtCallbacks, dwSize);

    lpInput->ddRVal = DD_OK;
  }

  return DDHAL_DRIVER_HANDLED;
}

#if USE_NT5_DDMEMMGR
/******************************Public*Routine******************************\
* DWORD DdFreeDriverMemory
*
* This function called by DirectDraw when it's running low on memory in
* our heap.  You only need to implement this function if you use the
* DirectDraw 'HeapVidMemAllocAligned' function in your driver, and you
* can boot those allocations out of memory to make room for DirectDraw.
*
* We implement this function in the S3 driver because we have DirectDraw
* entirely manage our off-screen heap, and we use HeapVidMemAllocAligned
* to put GDI device-bitmaps in off-screen memory.  DirectDraw applications
* have a higher priority for getting stuff into video memory, though, and
* so this function is used to boot those GDI surfaces out of memory in
* order to make room for DirectDraw.
*
\**************************************************************************/

DWORD __stdcall
DdFreeDriverMemory( PDD_FREEDRIVERMEMORYDATA pfdmd )
{
    PDEV*   ppdev;

    ppdev = (PDEV*) pfdmd->lpDD->dhpdev;

    pfdmd->ddRVal = DDERR_OUTOFMEMORY;

    // If we successfully freed up some memory, set the return value to
    // 'DD_OK'.  DirectDraw will try again to do its allocation, and
    // will call us again if there's still not enough room.  (It will
    // call us until either there's enough room for its alocation to
    // succeed, or until we return something other than DD_OK.)

    if (bMoveOldestOffscreenDfbToDib(ppdev))
    {
        pfdmd->ddRVal = DD_OK;
    }

    return(DDHAL_DRIVER_HANDLED);
}
#endif

/****************************************************************************
*
* FUNCTION:     DdSetExclusiveMode()
*
* DESCRIPTION:
*
****************************************************************************/

DWORD __stdcall
DdSetExclusiveMode ( PDD_SETEXCLUSIVEMODEDATA psemd )
{
  DD_ENTRY_SETUP(psemd->lpDD);


  if (psemd->dwEnterExcl)
  {
    DISPDBG((DEBUG_APIENTRY, "DdSetExclusiveMode (entering)"));
    _DS(ddExclusiveMode) = TRUE;
	hwcSetContextDWORD();
  }
  else
  {
    DISPDBG((DEBUG_APIENTRY, "DdSetExclusiveMode (leaving)"));
    _DS(ddExclusiveMode) = FALSE;

#if 0
    if(_FF(dd3DInOverlay) == TRUE)
    {
      demote_3DToNoneOverlay();
    }
    if(_DS(ddPrimaryInTile) == TRUE )
    {
      demote_PrimaryToLinear();
    }

    _FF(ddVisibleOverlaySurf) = 0;
    RepaintDesktop();
#endif
  }
  psemd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
} /*DdSetExclusiveMode */

/***************************************************************************
*
* FUNCTION:     DdFlipToGDISurface
*
* DESCRIPTION:
*
****************************************************************************/

DWORD __stdcall
DdFlipToGDISurface( PDD_FLIPTOGDISURFACEDATA pftgs )
{
  DD_ENTRY_SETUP(pftgs->lpDD);

  if( pftgs->dwToGDI )
  {
    DWORD addr;
    CMDFIFO_PROLOG(hwPtr);
    addr = ppdev->ulScreenOffset;

    FXBUSYWAIT(ppdev);

    SETDW(ghwIO->vidDesktopStartAddr, addr);

    CMDFIFO_CHECKROOM(hwPtr, 2);
    SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF));
    SETPD(hwPtr, ghw0->swapbufferCMD, 1);
    BUMP(2);

    DISPDBG((DEBUG_APIENTRY, "DdFlipToGDISurface (to GDI)"));
    CMDFIFO_EPILOG(hwPtr);
  }
  else
  {
    DISPDBG((DEBUG_APIENTRY, "DdFlipToGDISurface (away from GDI)"));
  }

  pftgs->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
}
#endif // _WIN32_WINNT >= 0x0500

/****************************************************************************
*
* FUNCTION:     ddgetenv()
*
* DESCRIPTION:  calls miniport to retrieve a string from the registry
*
* WARNING:  This function is not reentrant!!!
*           This function originates from the CVG code and there was
*           apparently an oversight that the returned string needs to be
*           stored somewhere.  For some reason the CVG code decided to
*           return a pointer to a local stack variable which seems like
*           trouble waiting to happen.  Since ddraw code is being shared
*           on Banshee between win9x and NT (such that I can't change the
*           interface to this function) I've opted to return a pointer
*           to a static variable.  Which means if you don't use the
*           returned string before calling this function again, you lose
*           the previous setting.
*
****************************************************************************/

char *
ddgetenv ( PDEV *ppdev, const char *varname )
{
  static char regStr[TDFX_MAX_DATA_LENGTH];

  TDFX_QUERY_VALUE_INFO queryValueInfo;
  DWORD                 numBytes;


  // call ioctl to have miniport read data from registry
  queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;
  if (! EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_QUERY_REGISTRY_VALUE,
                           (PVOID)varname,
                           strlen(varname) + 1,
                           &queryValueInfo,
                           sizeof(queryValueInfo),
                           &numBytes))
  {
    if (REG_SZ == queryValueInfo.Type)
    {
      strcpy(regStr, queryValueInfo.Data);
      return regStr;
    }
    // if key isn't a string, return as if key not found
  }

  // miniport didn't find key
  return NULL;
}

/****************************************************************************
*
* FUNCTION:		 atoi()
*
* DESCRIPTION:	Converts ASCIIZ string to int
*
****************************************************************************/

int __cdecl
atoi ( const char *pszString )
{
  char  ch;
  int   retVal;
  BOOL  negative = FALSE;


  retVal = 0;

  // skip leading white space
  while (((char)' '  == *pszString) ||
         ((char)'\t' == *pszString) ||
         ((char)'\n' == *pszString))
    pszString++;

  // keep track of negative values
  if ((char)'-' == *pszString)
  {
    negative = TRUE;
    pszString++;
  }
  else if ((char)'+' == *pszString)
    pszString++;

  // convert the string
  // first non-numeric character causes loop to exit
  // there's no error checking for overflow of retVal
  for (;;)
  {
    ch = *pszString++;
    if (ch < (char)'0')
      break;
    if (ch > (char)'9')
      break;

    retVal *= 10;
    ch -= (char)'0';
    retVal += ch;
  }

  if (negative)
    retVal = -retVal;

  return retVal;
}

#if 0
/******************************Public*Routine******************************\
* VOID vGetDisplayDuration
*
* Get the length, in EngQueryPerformanceCounter() ticks, of a refresh cycle.
*
* If we could trust the miniport to return back an accurate value for
* the refresh rate, we could use that.  Unfortunately, our miniport doesn't
* ensure that it's an accurate value.
*
\**************************************************************************/

#define NUM_VBLANKS_TO_MEASURE      1
#define NUM_MEASUREMENTS_TO_TAKE    8

VOID vGetDisplayDuration(PDEV* ppdev)
{
    BYTE*       pjH3Base;
    LONG        i;
    LONG        j;
    LONGLONG    li;
    LONGLONG    liFrequency;
    LONGLONG    liMin;
    LONGLONG    aliMeasurement[NUM_MEASUREMENTS_TO_TAKE + 1];

    pjH3Base = ppdev->pjH3Base;	// 2D base

    memset(&ppdev->flipRecord, 0, sizeof(ppdev->flipRecord));

    // Warm up EngQUeryPerformanceCounter to make sure it's in the working
    // set:

    EngQueryPerformanceCounter(&li);

#if !defined( CSIM ) && !defined( A0_SILICON )
    // Unfortunately, since NT is a proper multitasking system, we can't
    // just disable interrupts to take an accurate reading.  We also can't
    // do anything so goofy as dynamically change our thread's priority to
    // real-time.
    //
    // So we just do a bunch of short measurements and take the minimum.
    //
    // It would be 'okay' if we got a result that's longer than the actual
    // VBlank cycle time -- nothing bad would happen except that the app
    // would run a little slower.  We don't want to get a result that's
    // shorter than the actual VBlank cycle time -- that could cause us
    // to start drawing over a frame before the Flip has occured.

    while (VBLANK_IS_ACTIVE(pjH3Base))
        ;
    while (!(VBLANK_IS_ACTIVE(pjH3Base)))
        ;

    for (i = 0; i < NUM_MEASUREMENTS_TO_TAKE; i++)
    {
        // We're at the start of the VBlank active cycle!

        EngQueryPerformanceCounter(&aliMeasurement[i]);

        // Okay, so life in a multi-tasking environment isn't all that
        // simple.  What if we had taken a context switch just before
        // the above EngQueryPerformanceCounter call, and now were half
        // way through the VBlank inactive cycle?  Then we would measure
        // only half a VBlank cycle, which is obviously bad.  The worst
        // thing we can do is get a time shorter than the actual VBlank
        // cycle time.
        //
        // So we solve this by making sure we're in the VBlank active
        // time before and after we query the time.  If it's not, we'll
        // sync up to the next VBlank (it's okay to measure this period --
        // it will be guaranteed to be longer than the VBlank cycle and
        // will likely be thrown out when we select the minimum sample).
        // There's a chance that we'll take a context switch and return
        // just before the end of the active VBlank time -- meaning that
        // the actual measured time would be less than the true amount --
        // but since the VBlank is active less than 1% of the time, this
        // means that we would have a maximum of 1% error approximately
        // 1% of the times we take a context switch.  An acceptable risk.
        //
        // This next line will cause us wait if we're no longer in the
        // VBlank active cycle as we should be at this point:

        while (!(VBLANK_IS_ACTIVE(pjH3Base)))
            ;

        for (j = 0; j < NUM_VBLANKS_TO_MEASURE; j++)
        {
            while (VBLANK_IS_ACTIVE(pjH3Base))
                ;
            while (!(VBLANK_IS_ACTIVE(pjH3Base)))
                ;
        }
    }

    EngQueryPerformanceCounter(&aliMeasurement[NUM_MEASUREMENTS_TO_TAKE]);

    // Use the minimum:

    liMin = aliMeasurement[1] - aliMeasurement[0];

    DISPDBG((1, "Refresh count: %li - %li", 1, (ULONG) liMin));

    for (i = 2; i <= NUM_MEASUREMENTS_TO_TAKE; i++)
    {
        li = aliMeasurement[i] - aliMeasurement[i - 1];

        DISPDBG((1, "               %li - %li", i, (ULONG) li));

        if (li < liMin)
            liMin = li;
    }

    // Round the result:

    ppdev->flipRecord.liFlipDuration
        = (DWORD) (liMin + (NUM_VBLANKS_TO_MEASURE / 2)) / NUM_VBLANKS_TO_MEASURE;
    ppdev->flipRecord.bFlipFlag  = FALSE;
    ppdev->flipRecord.fpFlipFrom = 0;

    // We need the refresh rate in Hz to query the S3 miniport about the
    // streams parameters:

    EngQueryPerformanceFrequency(&liFrequency);

    ppdev->ulRefreshRate
        = (ULONG) ((liFrequency + (ppdev->flipRecord.liFlipDuration / 2))
                    / ppdev->flipRecord.liFlipDuration);

    DISPDBG((1, "Frequency: %li Hz", ppdev->ulRefreshRate));
#else
#ifdef A0_SILICON
    EngQueryPerformanceFrequency(&liFrequency);
    // assumes ppdev->ulRefreshRate was filled in by bAssertModeHardware
    ppdev->flipRecord.liFlipDuration = liFrequency / ppdev->ulRefreshRate;
#else
    // the following FlipDuration is appropriate for 60Hz on a PII 300
    // refreshRate = liFrequency/flipDuration = 1234DEh/4D87h = 3Ch = 60Hz
    ppdev->flipRecord.liFlipDuration = (LONGLONG) 0x4D87;	// hardcode for csim
#endif
    ppdev->flipRecord.bFlipFlag  = FALSE;
    ppdev->flipRecord.fpFlipFrom = 0;
#endif
}
#endif

#if defined(ENABLE_REG_IOCTL_TEST)
/****************************************************************************
*
* FUNCTION:     TestRegIoctl()
*
* DESCRIPTION:
*
****************************************************************************/

static VOID
TestRegIoctl ( PDEV *ppdev )
{
  static const char   TestREG_SZData[] = "H3 NT Test String";
  static const DWORD  TestREG_DWORDData = 0xA5A5A5A5;
  static const BYTE   TestREG_BINARYData[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
  static const char   TestREG_SZValue[] = "SSTH3_TEST_REG_SZ";
  static const char   TestREG_DWORDValue[] = "H3_TEST_REG_DWORD";
  static const char   TestREG_BINARYValue[] = "H3_TEST_REG_BINARY";

  TDFX_SET_VALUE_INFO setValueInfo;
  TDFX_QUERY_VALUE_INFO queryValueInfo;
  DWORD numBytes;
  DWORD retval;


  // initialize setValueInfo for string data
  strcpy(setValueInfo.Data, TestREG_SZData);
  setValueInfo.DataLength = strlen(TestREG_SZData) + 1;
  strcpy(setValueInfo.ValueName, TestREG_SZValue);
  setValueInfo.ValueNameLength = strlen(TestREG_SZValue) + 1;
  setValueInfo.Type = REG_SZ;

  // call ioctl to have miniport write data to registry
  retval = EngDeviceIoControl(ppdev->hDriver,
                              IOCTL_3DFX_SET_REGISTRY_VALUE,
                              &setValueInfo,
                              sizeof(setValueInfo),
                              NULL,
                              0,
                              &numBytes);
  if (retval)
  {
    DISPDBG((0, "set REG_SZ ioctl failed, returned %08lXh", retval));
  }

  // initialize setValueInfo for dword data
  *(DWORD *)setValueInfo.Data = TestREG_DWORDData;
  setValueInfo.DataLength = sizeof(TestREG_DWORDData);
  strcpy(setValueInfo.ValueName, TestREG_DWORDValue);
  setValueInfo.ValueNameLength = strlen(TestREG_DWORDValue) + 1;
  setValueInfo.Type = REG_DWORD;

  // call ioctl to have miniport write data to registry
  retval = EngDeviceIoControl(ppdev->hDriver,
                              IOCTL_3DFX_SET_REGISTRY_VALUE,
                              &setValueInfo,
                              sizeof(setValueInfo),
                              NULL,
                              0,
                              &numBytes);
  if (retval)
  {
    DISPDBG((0, "set REG_DWORD ioctl failed, returned %08lXh", retval));
  }

  // initialize setValueInfo for binary data
  memcpy(setValueInfo.Data, TestREG_BINARYData, sizeof(TestREG_BINARYData));
  setValueInfo.DataLength = sizeof(TestREG_BINARYData);
  strcpy(setValueInfo.ValueName, TestREG_BINARYValue);
  setValueInfo.ValueNameLength = strlen(TestREG_BINARYValue) + 1;
  setValueInfo.Type = REG_BINARY;

  // call ioctl to have miniport write data to registry
  retval = EngDeviceIoControl(ppdev->hDriver,
                              IOCTL_3DFX_SET_REGISTRY_VALUE,
                              &setValueInfo,
                              sizeof(setValueInfo),
                              NULL,
                              0,
                              &numBytes);
  if (retval)
  {
    DISPDBG((0, "set REG_BINARY ioctl failed, returned %08lXh", retval));
  }


  // read string data back from registry
  // call ioctl to have miniport read data from registry
  queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;
  retval = EngDeviceIoControl(ppdev->hDriver,
                              IOCTL_3DFX_QUERY_REGISTRY_VALUE,
                              (PVOID)TestREG_SZValue,
                              strlen(TestREG_SZValue) + 1,
                              &queryValueInfo,
                              sizeof(queryValueInfo),
                              &numBytes);
  if (retval)
  {
    DISPDBG((0, "query REG_SZ ioctl failed, returned %08lXh", retval));
  }
  else if (REG_SZ != queryValueInfo.Type)
  {
    DISPDBG((0, "query REG_SZ failed, returned type of %ld",
             queryValueInfo.Type));
  }
  else
  {
    if (memcmp(queryValueInfo.Data, TestREG_SZData, strlen(TestREG_SZData) + 1))
    {
      DISPDBG((0, "query REG_SZ failed, returned string %s",
               queryValueInfo.Data));
    }
  }

  // read dword data back from registry
  // call ioctl to have miniport read data from registry
  queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;
  retval = EngDeviceIoControl(ppdev->hDriver,
                              IOCTL_3DFX_QUERY_REGISTRY_VALUE,
                              (PVOID)TestREG_DWORDValue,
                              strlen(TestREG_DWORDValue) + 1,
                              &queryValueInfo,
                              sizeof(queryValueInfo),
                              &numBytes);
  if (retval)
  {
    DISPDBG((0, "query REG_DWORD ioctl failed, returned %08lXh", retval));
  }
  else if (REG_DWORD != queryValueInfo.Type)
  {
    DISPDBG((0, "query REG_DWORD failed, returned type of %ld",
             queryValueInfo.Type));
  }
  else
  {
    if (*(DWORD *)queryValueInfo.Data != TestREG_DWORDData)
    {
      DISPDBG((0, "query REG_DWORD failed, returned dword %08lXh",
               *(DWORD *)queryValueInfo.Data));
    }
  }

  // read binary data back from registry
  // call ioctl to have miniport read data from registry
  queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;
  retval = EngDeviceIoControl(ppdev->hDriver,
                              IOCTL_3DFX_QUERY_REGISTRY_VALUE,
                              (PVOID)TestREG_BINARYValue,
                              strlen(TestREG_BINARYValue) + 1,
                              &queryValueInfo,
                              sizeof(queryValueInfo),
                              &numBytes);
  if (retval)
  {
    DISPDBG((0, "query REG_BINARY ioctl failed, returned %08lXh", retval));
  }
  else if (REG_BINARY != queryValueInfo.Type)
  {
    DISPDBG((0, "query REG_BINARY failed, returned type of %ld",
             queryValueInfo.Type));
  }
  else
  {
    if (memcmp(queryValueInfo.Data, TestREG_BINARYData, sizeof(TestREG_BINARYData)))
    {
      DISPDBG((0, "query REG_BINARY failed, returned data "
                  "%02Xh %02Xh %02Xh %02Xh "
                  "%02Xh %02Xh %02Xh %02Xh "
                  "%02Xh %02Xh %02Xh %02Xh "
                  "%02Xh %02Xh %02Xh %02Xh "
                  "%02Xh %02Xh",
               *(BYTE *)queryValueInfo.Data[0],
               *(BYTE *)queryValueInfo.Data[1],
               *(BYTE *)queryValueInfo.Data[2],
               *(BYTE *)queryValueInfo.Data[3],
               *(BYTE *)queryValueInfo.Data[4],
               *(BYTE *)queryValueInfo.Data[5],
               *(BYTE *)queryValueInfo.Data[6],
               *(BYTE *)queryValueInfo.Data[7],
               *(BYTE *)queryValueInfo.Data[8],
               *(BYTE *)queryValueInfo.Data[9],
               *(BYTE *)queryValueInfo.Data[10],
               *(BYTE *)queryValueInfo.Data[11],
               *(BYTE *)queryValueInfo.Data[12],
               *(BYTE *)queryValueInfo.Data[13],
               *(BYTE *)queryValueInfo.Data[14],
               *(BYTE *)queryValueInfo.Data[15],
               *(BYTE *)queryValueInfo.Data[16],
               *(BYTE *)queryValueInfo.Data[17]));
    }
  }
}
#endif

#ifdef SLI_AA
/******************************Public*Routine******************************\
* BOOL GetSLIAAInfo
\**************************************************************************/
static BOOL GetSLIAAInfo(PDEV* ppdev)
{
  TDFX_SLI_AA_INFO  SLIAAInfo;
  DWORD             numBytes;


  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_3DFX_SLI_AA_INFO,
                         NULL,
                         0,
                         &SLIAAInfo,
                         sizeof(SLIAAInfo),
                         &numBytes))
  {
    DISPDBG((0, "GetSLIAAInfo - Can't get sli/aa info!"));
    _FF(dwNumUnits) = 1;
    return(FALSE);
  }

  _FF(dwNumUnits) = SLIAAInfo.numUnits;
  memcpy(&_FF(regBase[0]),
         &SLIAAInfo.sliMappedAddress[0][0],
         SLIAAInfo.numUnits * HWINFO_SST_MAX_CHIP_INDEX * sizeof(SLIAAInfo.sliMappedAddress[0][0]));

  return(TRUE);
}
#endif


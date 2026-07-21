/* $Header: ddinit.c, 28, 10/13/00 2:30:19 AM, Johnny Trainor $ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File Name: 	DDINIT.C
**
** Description: Direct Draw surface related functions.
**
** $Revision: 28$
** $Date: 10/13/00 2:30:19 AM$
**
*/

/*******************************************************************************
*
* EXPORTED FUNCTIONS:
*
* DrvGetDirectDrawInfo      --- Initialize DirectDraw capabilities.
* DrvEnableDirectDraw       --- Initialize DirectDraw callbacks.
*
* INTERNAL FUNCTIONS:
*
* fxinit                    --- Initialize DirectDraw information in pdevice.
* NTSpecificInit            --- Initialize Windows NT specific information.
*
*******************************************************************************/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"
#include "regkeys.h"

// STB Begin Changes
#ifdef INCSTBPERF
#ifdef WINNT
#if (_WIN32_WINNT < 0x0500)
#include "..\..\..\..\build\stbperf.inc"
#endif
#else
#include "..\build\stbperf.inc"
#endif
#endif
// STB End Changes

/**************************************************************************
* D E F I N E S
***************************************************************************/

// I'm leaving the PALETTE_CAPS define because I'm not convinced there is
// any reason to implement the palette callbacks on NT but they are aleady
// implemented for win9x - RL
//
// The OVERLAY_CAPS define is left here for a simple method of disabling
// the overlay support.  Due to the way we are implementing creation of
// overlay surfaces, disabling overlay support is almost the only way to
// debug color conversion blts - RL

#ifdef WINNT
#define ENABLE_PALETTE_CAPS       0
#ifdef CSIM
#define ENABLE_OVERLAY_CAPS       0
#else
#define ENABLE_OVERLAY_CAPS       1
#endif
#else
#define ENABLE_PALETTE_CAPS       1
#define ENABLE_OVERLAY_CAPS       1
#endif

//#define STB_DISABLEHWDOWNSCALE    1   //used for test only

// fix for PRS 14329, disable system to video memory blt caps
//#define PERF_DISABLE_CANBLTSYSMEM   1
// but that costs us a 2.4 point performance hit in 3DWB2000
// at 1024x768x16@85Hz in 2-way sli mode on a PIII-733 with a V5 5500 AGP
// so add a registry key that allows the user to disable the CANBLTSYSMEM
// caps so they can play Descent3 on w2k, yee haw

// The problem with Descent3 is that they create DXT1 mipmap chains but then no calls are made into
// the driver to load those textures with any data, but rather the surfaces are immediately destroyed.
// It seems that most of the textures Descent3 attempts to use are DXT1 but since none ever remain
// allocated the game ends up rendering with no textures and most of the screen shows up white.
// 
// The w2k checked build spits out the following messages when the DXT1 surfaces are attempted to be
// loaded:
// 
// Direct3D: (ERROR) :Blt failure
// Direct3D: (ERROR) :CopySurface returned error
// Direct3D: (ERROR) :Failed to create video memory surface
// GDI: NtGdiDdBlt: Invalid source surface or source rectangle
// 
// and no Lock, Blt or TEXBLT call is made to the driver.
// 
// According to the W2K DDK, section 3.11.3 Using Compressed Texture Surfaces
// 
// The semantics of the DirectDraw DDCAPS_CANBLTSYSMEM capability bit imply that the display driver be
// called for all backing surface to display memory blits. Consequently, the driver may be called for
// backing surface to display memory blits from DXT surfaces to non-DXT surfaces. The only requirement
// in this case is that the driver return DDHAL_DRIVER_NOTHANDLED if it cannot perform the decompression.
// This will cause DirectDraw to propagate a DDERR_UNSUPPORTED error code to the application. It is
// acceptable to implement decompression for backing surface to display memory blits in your driver, but
// this is not required.
// 
// 
// This seems to imply that to properly support CANBLTSYSMEM, we need to add functionality such that we
// can convert any texture format surface we support to any other texture format surface we support.
// And probably add the DXTn/FXT1 fourcc codes to our FourCC array.  Ugh!  As far as I know, we never
// wanted to do that.

/**************************************************************************
* F U N C T I O N   P R O T O T Y P E S
***************************************************************************/

#ifdef WINNT
int __cdecl atoi(const char *);
static VOID NTSpecificInit(PDEV         *ppdev,
                           DDHALINFO    *pHalInfo,
                           DWORD        *pdwNumHeaps,
                           VIDEOMEMORY  *pvmList,
                           DWORD        *pdwNumFourCC,
                           DWORD        *pdwFourCC);
#endif

/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  DrvGetDirectDrawInfo

Description:    Initialize DirectDraw capabilities (Windows NT style).

Return:         TRUE  - success
				FALSE - failure
----------------------------------------------------------------------*/

BOOL __stdcall
DrvGetDirectDrawInfo(DHPDEV       dhpdev,
                     DDHALINFO    *pHalInfo,
                     DWORD        *pdwNumHeaps,
                     VIDEOMEMORY  *pvmList,
                     DWORD        *pdwNumFourCC,
                     DWORD        *pdwFourCC)
{
  NT9XDEVICEDATA *ppdev = (NT9XDEVICEDATA *)dhpdev;

#ifdef WINNT
  *pdwNumFourCC = 0;
  *pdwNumHeaps  = 0;

  // We may not support DirectDraw on this card
  if (!(ppdev->flStatus & STAT_DIRECTDRAW))
    return FALSE;
#endif

  /* Initialize DDHALINFO structure. */

#ifndef WINNT
  memset(pHalInfo, 0, sizeof(DDHALINFO));
#endif

  pHalInfo->dwSize = sizeof(DDHALINFO);

  pHalInfo->ddCaps.ddsCaps.dwCaps = 0
                                  | DDSCAPS_FLIP
                                  | DDSCAPS_OFFSCREENPLAIN
                                  | DDSCAPS_BACKBUFFER
                                  | DDSCAPS_COMPLEX
                                  | DDSCAPS_FRONTBUFFER
                                  | DDSCAPS_VIDEOMEMORY
#if ENABLE_OVERLAY_CAPS
                                  | DDSCAPS_OVERLAY
#endif
#if ENABLE_PALETTE_CAPS
                                  | DDSCAPS_PALETTE
#endif
                                  | DDSCAPS_PRIMARYSURFACE
#if ENABLE_3D
                                  | DDSCAPS_TEXTURE
                                  | DDSCAPS_3DDEVICE
                                  | DDSCAPS_ZBUFFER
                                  | DDSCAPS_MIPMAP
#if defined(WINNT) && defined(TnL_HAL) && defined(VERT_BUFF)
                                  | DDSCAPS_EXECUTEBUFFER  
#endif // WINNT && TnL_HAL && VERTBUF
#endif // ENABLE_3D
#if ENABLE_VIDEOPORT
#ifndef WINNT

                                  | DDSCAPS_VIDEOPORT
#endif
#endif
                                  ;
#ifndef WINNT
  pHalInfo->lpDDCallbacks        = &(_FF(DDCallbacks));
  pHalInfo->lpDDSurfaceCallbacks = &(_FF(DDSurfaceCallbacks));
  pHalInfo->lpDDPaletteCallbacks = &(_FF(DDPaletteCallbacks));

#if   defined(TnL_HAL) && defined(VERT_BUFF)
  if (_FF(DDExebufCallbacks.dwFlags))
  {
        pHalInfo->lpDDExeBufCallbacks  = &_FF(DDExebufCallbacks);
        pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_EXECUTEBUFFER;
  }
  else
        pHalInfo->lpDDExeBufCallbacks = NULL;
#endif  

  pHalInfo->ddCaps.dwNumFourCCCodes = 0;
  pHalInfo->lpdwFourCC              = NULL;

  pHalInfo->hInstance = (DWORD) hInstance;
  pHalInfo->lpPDevice = (LPVOID)_FF(lpPDevice);

  pHalInfo->dwFlags = 0
                    // | DDHALINFO_MODEXILLEGAL // Fixes Bug!
                    | DDHALINFO_GETDRIVERINFOSET
                    ;
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
/* DX8 GetDriverInfo2 callback      */    DDHALINFO_GETDRIVERINFO2          |
#endif
                                          0;

  // @RBISSELL, We can't display ModeX modes on the TV in Win9x
  if (_FF(dwTvoActive))
     pHalInfo->dwFlags |= DDHALINFO_MODEXILLEGAL;


  pHalInfo->GetDriverInfo = DdGetDriverInfo;
#endif

  pHalInfo->ddCaps.dwCaps =       0
                                  | DDCAPS_BLT
                                  //| DDCAPS_BLTQUEUE
                                  | DDCAPS_BLTFOURCC
                                  | DDCAPS_BLTSTRETCH
#ifndef WINNT
// specifying DDCAPS_GDI on NT causes NOHARDWARE support!
                                  | DDCAPS_GDI
#endif
#if ENABLE_OVERLAY_CAPS
                                  | DDCAPS_OVERLAY
                                  | DDCAPS_OVERLAYCANTCLIP
                                  | DDCAPS_OVERLAYFOURCC
                                  | DDCAPS_OVERLAYSTRETCH
#endif
#if ENABLE_PALETTE_CAPS
                                  | DDCAPS_PALETTE
#endif
                                  | DDCAPS_READSCANLINE
                                  | DDCAPS_COLORKEY
                                  | DDCAPS_COLORKEYHWASSIST
                                  | DDCAPS_BLTCOLORFILL
                                  //| DDCAPS_ALPHA
                                  //| DDCAPS_CANCLIP
                                  //| DDCAPS_CANCLIPSTRETCHED
#ifndef PERF_DISABLE_CANBLTSYSMEM
                                  | DDCAPS_CANBLTSYSMEM
#endif

#if ENABLE_3D
                                  | DDCAPS_3D
                                  | DDCAPS_ZBLTS
                                  | DDCAPS_BLTDEPTHFILL
#endif
                                  ;

#ifndef PERF_DISABLE_CANBLTSYSMEM
  pHalInfo->ddCaps.dwSVBCaps =    0
                                  | DDCAPS_BLT
                                  //| DDCAPS_BLTQUEUE
                                  //| DDCAPS_BLTFOURCC
                                  | DDCAPS_BLTSTRETCH
#ifndef WINNT
// specifying DDCAPS_GDI on NT causes NOHARDWARE support!
                                  | DDCAPS_GDI
#endif
                                  | DDCAPS_COLORKEY
                                  | DDCAPS_COLORKEYHWASSIST
                                  //| DDCAPS_BLTCOLORFILL
                                  //| DDCAPS_ALPHA
                                  //| DDCAPS_CANCLIP
                                  //| DDCAPS_CANCLIPSTRETCHED
#if 0
                                  | DDCAPS_CANBLTSYSMEM

#if ENABLE_3D
                                  | DDCAPS_3D
                                  | DDCAPS_ZBLTS
                                  | DDCAPS_BLTDEPTHFILL
#endif
#endif
                                  ;
#endif

#ifndef WINNT
  pHalInfo->ddCaps.dwCaps2 =      0
                                  | DDCAPS2_NOPAGELOCKREQUIRED      // important optimization. banshee does not need memory
                                                                    // page locked down by direct draw.

                                  | DDCAPS2_CANFLIPODDEVEN          // Support flip with ODD/EVEN flags, DX-6 AGUS
                                  | DDCAPS2_WIDESURFACES            // Allow allocation of surfaces wider than primary.
#ifdef STBPERF_USE_FLIPNOVSYNC
                                  | DDCAPS2_FLIPNOVSYNC           // Support flip without VSYNC, DX-6 AGUS - enabled 7/20/99 mls
#endif
#if ENABLE_VIDEOPORT
                                  | DDCAPS2_VIDEOPORT
                                  | DDCAPS2_AUTOFLIPOVERLAY
                                  | DDCAPS2_CANBOBINTERLEAVED       // Disabling caused C-CUBE weave to fail
                                  | DDCAPS2_CANBOBNONINTERLEAVED
                                  | DDCAPS2_CANBOBHARDWARE
                                  //| DDCAPS2_COLORCONTROLOVERLAY
#endif
                                  | DDCAPS2_COPYFOURCC
                                  ;
#endif

  pHalInfo->ddCaps.dwFXCaps =     0
                                  | DDFXCAPS_BLTSHRINKX
                                  | DDFXCAPS_BLTSHRINKY
                                  | DDFXCAPS_BLTSTRETCHX
                                  | DDFXCAPS_BLTSTRETCHY
                                  //| DDFXCAPS_BLTARITHSTRETCHY
                                  //| DDFXCAPS_BLTARITHSTRETCHYN
                                  //| DDFXCAPS_BLTSHRINKXN
                                  //| DDFXCAPS_BLTSHRINKYN
                                  //| DDFXCAPS_BLTSTRETCHXN
                                  //| DDFXCAPS_BLTSTRETCHYN
#if ENABLE_OVERLAY_CAPS
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
                                  | DDFXCAPS_OVERLAYSHRINKX          	// VPE requires to shrink video window!
                                  | DDFXCAPS_OVERLAYSHRINKY          	// VPE requires to shrink video window!
#endif
                                  | DDFXCAPS_OVERLAYSTRETCHX
                                  | DDFXCAPS_OVERLAYSTRETCHY
                                  //| DDFXCAPS_OVERLAYARITHSTRETCHY
                                  //| DDFXCAPS_OVERLAYARITHSTRETCHYN
                                  //| DDFXCAPS_OVERLAYSHRINKXN
                                  //| DDFXCAPS_OVERLAYSHRINKYN
                                  //| DDFXCAPS_OVERLAYSTRETCHXN
                                  //| DDFXCAPS_OVERLAYSTRETCHYN
#endif
                                  ;

#ifndef PERF_DISABLE_CANBLTSYSMEM
  pHalInfo->ddCaps.dwSVBFXCaps =  0
                                  | DDFXCAPS_BLTSTRETCHX
                                  | DDFXCAPS_BLTSTRETCHY
                                  //| DDFXCAPS_BLTARITHSTRETCHY
                                  //| DDFXCAPS_BLTARITHSTRETCHYN
                                  //| DDFXCAPS_BLTSHRINKX
                                  //| DDFXCAPS_BLTSHRINKY
                                  //| DDFXCAPS_BLTSHRINKXN
                                  //| DDFXCAPS_BLTSHRINKYN
                                  //| DDFXCAPS_BLTSTRETCHXN
                                  //| DDFXCAPS_BLTSTRETCHYN
                                  ;
#endif

#if ENABLE_PALETTE_CAPS
  pHalInfo->ddCaps.dwPalCaps =    0
                                  | DDPCAPS_8BIT
                                  | DDPCAPS_ALLOW256
                                  ;
#endif

  pHalInfo->ddCaps.dwCKeyCaps =   0
                                  | DDCKEYCAPS_SRCBLT
                                  | DDCKEYCAPS_SRCBLTCLRSPACE

#if defined(WINNT) && (_WIN32_WINNT == 0x0400)
// NT bug, NT doesn't report dest colorkey values to DdBlt correctly
#else
                                  | DDCKEYCAPS_DESTBLT
                                  | DDCKEYCAPS_DESTBLTCLRSPACE
#endif

#if ENABLE_OVERLAY_CAPS
                                  | DDCKEYCAPS_DESTOVERLAY
                                  | DDCKEYCAPS_DESTOVERLAYCLRSPACE  //RGB only
                                  | DDCKEYCAPS_DESTOVERLAYONEACTIVE

                                  //| DDCKEYCAPS_DESTOVERLAYCLRSPACE
                                  //| DDCKEYCAPS_SRCOVERLAY
                                  //| DDCKEYCAPS_SRCOVERLAYCLRSPACE
#endif
                                  ;

#ifndef PERF_DISABLE_CANBLTSYSMEM
  pHalInfo->ddCaps.dwSVBCKeyCaps = 0

#if defined(WINNT) && (_WIN32_WINNT == 0x0400)
// NT bug, NT doesn't report dest colorkey values to DdBlt correctly
#else
                                  | DDCKEYCAPS_DESTBLT
                                  | DDCKEYCAPS_DESTBLTCLRSPACE
#endif							
                                  | DDCKEYCAPS_SRCBLT
                                  | DDCKEYCAPS_SRCBLTCLRSPACE
                                  ;
#endif

#if (_WIN32_WINNT >= 0x0500) || !defined WINNT
#if ENABLE_VIDEOPORT
  pHalInfo->ddCaps.dwMaxVideoPorts =  1;      //maximum number of usable video ports
  pHalInfo->ddCaps.dwCurrVideoPorts = 0;      //current number of video ports used
#else
  pHalInfo->ddCaps.dwMaxVideoPorts =  0;      //maximum number of usable video ports
  pHalInfo->ddCaps.dwCurrVideoPorts = 0;      //current number of video ports used
#endif
#endif

#if ENABLE_OVERLAY_CAPS
  pHalInfo->ddCaps.dwMaxVisibleOverlays  = 1;
  pHalInfo->ddCaps.dwCurrVisibleOverlays = 0;

//***VPE requires this MinOverlayStretch to be set to 100 to be able to shrink the video window properly***
//---------------------------------------------------------------------------------------------------------
//pHalInfo->ddCaps.dwMinOverlayStretch   =  100;      // min = 1/10:1 ?
//---------------------------------------------------------------------------------------------------------

#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
#ifdef  STB_DISABLEHWDOWNSCALE
  //LPost remove this when hw downscaling works.
  //This is for the Dell Demo
  pHalInfo->ddCaps.dwMinOverlayStretch   =  1000;    // min = 1:1
#else
  pHalInfo->ddCaps.dwMinOverlayStretch   =  100;     // min = 1:10
#endif

#else
  pHalInfo->ddCaps.dwMinOverlayStretch   =  1000;    // max = 1:1
#endif
  pHalInfo->ddCaps.dwMaxOverlayStretch   = 10000;    // max = 10:1 ?
  pHalInfo->vmiData.dwOverlayAlign       = 64;       // quadword alignment?
#endif

#ifdef WINNT
  NTSpecificInit(ppdev,
                 pHalInfo,
                 pdwNumHeaps,
                 pvmList,
                 pdwNumFourCC,
                 pdwFourCC);
#else
  // support all 16 two operand rops
  pHalInfo->ddCaps.dwRops[0] = 0x00020001;  // 0    & DSon
  pHalInfo->ddCaps.dwRops[1] = 0x00080004;  // DSna & Sn
  pHalInfo->ddCaps.dwRops[2] = 0x00200010;  // SDna & Dn
  pHalInfo->ddCaps.dwRops[3] = 0x00800040;  // DSx  & DSan
  pHalInfo->ddCaps.dwRops[4] = 0x02000100;  // DSa  & DSxn
  pHalInfo->ddCaps.dwRops[5] = 0x08000400;  // D    & DSno
  pHalInfo->ddCaps.dwRops[6] = 0x20001000;  // S    & SDno
  pHalInfo->ddCaps.dwRops[7] = 0x80004000;  // DSo  & 1

  pHalInfo->ddCaps.dwSVBRops[0] = 0x00020001;
  pHalInfo->ddCaps.dwSVBRops[1] = 0x00080004;
  pHalInfo->ddCaps.dwSVBRops[2] = 0x00200010;
  pHalInfo->ddCaps.dwSVBRops[3] = 0x00800040;
  pHalInfo->ddCaps.dwSVBRops[4] = 0x02000100;
  pHalInfo->ddCaps.dwSVBRops[5] = 0x08000400;
  pHalInfo->ddCaps.dwSVBRops[6] = 0x20001000;
  pHalInfo->ddCaps.dwSVBRops[7] = 0x80004000;
#endif

  ppdev->flip_pending = 0;

#if ENABLE_3D
  pHalInfo->ddCaps.dwZBufferBitDepths = DDBD_16;
  if (IS_NAPALM)
  {
    pHalInfo->ddCaps.dwZBufferBitDepths |= DDBD_24;
  }

#endif

#ifndef WINNT
  // 16bit driver will fill this out
  pHalInfo->dwModeIndex = DDUNSUPPORTEDMODE;
#endif

  if (! fxinit(ppdev, FALSE))
    return FALSE;

#if ENABLE_3D
#if   defined(TnL_HAL) && defined(VERT_BUFF)
  if (!D3DHALCreateDriver(ppdev,
                          (LPD3DHAL_GLOBALDRIVERDATA*)&(pHalInfo->lpD3DGlobalDriverData),
                          (LPD3DHAL_CALLBACKS*)&(pHalInfo->lpD3DHALCallbacks),
                          (DDHAL_DDEXEBUFCALLBACKS*)&_FF(DDExebufCallbacks)))
    return FALSE;
#else
  if (!D3DHALCreateDriver(ppdev,
                          (LPD3DHAL_GLOBALDRIVERDATA*)&(pHalInfo->lpD3DGlobalDriverData),
                          (LPD3DHAL_CALLBACKS*)&(pHalInfo->lpD3DHALCallbacks)))
    return FALSE;
#endif
#endif

#ifndef WINNT
    _FF(dwRelaxedOverlayOwnerMode) = 0;
#endif

  return TRUE;

} // DrvGetDirectDrawInfo


/*----------------------------------------------------------------------
retro3dfx: logging shims around the surface-creation callbacks. Every
CreateSurface/CanCreateSurface failure (any DDERR, from any of the ~15 exit
sites) gets one ring line — this is the last silent way a D3D app's resource
loading can abort with clean driver state (the warm-rerun degradation).
----------------------------------------------------------------------*/
#if ENABLE_LOG_FILE
static DWORD __stdcall retroDdCreateSurfaceLogged(LPDDHAL_CREATESURFACEDATA pcsd)
{
  DWORD ret = DdCreateSurface(pcsd);
  if (DD_OK != pcsd->ddRVal)
  {
    PDEV *ppdev = (PDEV *)pcsd->lpDD->dhpdev;
    retroLogForce(ppdev, "retro3dfx CREATESURF-FAIL: ddRVal=%08lXh caps=%08lXh %ldx%ld cnt=%ld\r\n",
                  (DWORD)pcsd->ddRVal,
                  pcsd->lpDDSurfaceDesc ? pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps : 0,
                  pcsd->lpDDSurfaceDesc ? (LONG)pcsd->lpDDSurfaceDesc->dwWidth : 0,
                  pcsd->lpDDSurfaceDesc ? (LONG)pcsd->lpDDSurfaceDesc->dwHeight : 0,
                  (LONG)pcsd->dwSCnt);
  }
  return ret;
}
static DWORD __stdcall retroDdCanCreateSurfaceLogged(LPDDHAL_CANCREATESURFACEDATA pccsd)
{
  DWORD ret = DdCanCreateSurface(pccsd);
  if (DD_OK != pccsd->ddRVal)
  {
    PDEV *ppdev = (PDEV *)pccsd->lpDD->dhpdev;
    retroLogForce(ppdev, "retro3dfx CANCREATE-FAIL: ddRVal=%08lXh caps=%08lXh\r\n",
                  (DWORD)pccsd->ddRVal,
                  pccsd->lpDDSurfaceDesc ? pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps : 0);
  }
  return ret;
}
#else
#define retroDdCreateSurfaceLogged    DdCreateSurface
#define retroDdCanCreateSurfaceLogged DdCanCreateSurface
#endif

/*----------------------------------------------------------------------
Function name:  DrvEnableDirectDraw

Description:    Initialize DirectDraw callbacks (Windows NT style).

Return:         TRUE  - success
				FALSE - failure
----------------------------------------------------------------------*/
BOOL __stdcall
DrvEnableDirectDraw(DHPDEV                    dhpdev,
                    DDHAL_DDCALLBACKS         *pCallBacks,
                    DDHAL_DDSURFACECALLBACKS  *pSurfaceCallBacks,
                    DDHAL_DDPALETTECALLBACKS  *pPaletteCallBacks)
{
#ifdef WINNT
  PDEV  *ppdev = (PDEV *)dhpdev;
#endif


  // fill in DirectDraw object callbacks
#ifndef WINNT
  memset(pCallBacks, 0, sizeof(DDHAL_DDCALLBACKS));
  pCallBacks->dwSize  = sizeof(DDHAL_DDCALLBACKS);
#endif

  pCallBacks->dwFlags = 0
                      | DDHAL_CB32_CREATESURFACE
                      | DDHAL_CB32_WAITFORVERTICALBLANK
                      | DDHAL_CB32_CANCREATESURFACE
#if ENABLE_PALETTE_CAPS
                      | DDHAL_CB32_CREATEPALETTE
#endif
                      | DDHAL_CB32_GETSCANLINE
#ifdef WINNT
                      | DDHAL_CB32_MAPMEMORY
#else
                      | DDHAL_CB32_SETEXCLUSIVEMODE
                      | DDHAL_CB32_FLIPTOGDISURFACE
#endif
                      ;
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && 0
  pCallBacks->SetColorKey          = DdSetDrvColorKey;
  pCallBacks->dwFlags             |= DDHAL_CB32_SETCOLORKEY;
#endif
#ifndef WINNT
//This member is not used and obsolete under DX-6
#if (DX < 6)
  pCallBacks->SetColorKey          = DdSetDrvColorKey;
  pCallBacks->dwFlags             |= DDHAL_CB32_SETCOLORKEY;
#endif
#endif

  pCallBacks->CreateSurface        = retroDdCreateSurfaceLogged;
  pCallBacks->WaitForVerticalBlank = DdWaitForVerticalBlank;
  pCallBacks->CanCreateSurface     = retroDdCanCreateSurfaceLogged;
#if ENABLE_PALETTE_CAPS
  pCallBacks->CreatePalette        = DdCreatePalette;
#endif
  pCallBacks->GetScanLine          = DdGetScanLine;

#ifdef WINNT
  pCallBacks->MapMemory            = DdMapMemory;
#else
  pCallBacks->SetExclusiveMode     = DdSetExclusiveMode;
  pCallBacks->FlipToGDISurface     = DdFlipToGDISurface;
#endif


  // fill in DirectDrawSurface object callbacks
#ifndef WINNT
  memset(pSurfaceCallBacks, 0, sizeof(DDHAL_DDSURFACECALLBACKS));
  pSurfaceCallBacks->dwSize  = sizeof(DDHAL_DDSURFACECALLBACKS);
#endif

  pSurfaceCallBacks->dwFlags = 0
                             | DDHAL_SURFCB32_DESTROYSURFACE
                             | DDHAL_SURFCB32_FLIP
                             | DDHAL_SURFCB32_LOCK
                             | DDHAL_SURFCB32_UNLOCK
                             | DDHAL_SURFCB32_BLT
#if ENABLE_OVERLAY_CAPS
                             | DDHAL_SURFCB32_SETCOLORKEY
#endif
                             | DDHAL_SURFCB32_ADDATTACHEDSURFACE
                             | DDHAL_SURFCB32_GETBLTSTATUS
                             | DDHAL_SURFCB32_GETFLIPSTATUS
#if ENABLE_OVERLAY_CAPS
                             | DDHAL_SURFCB32_UPDATEOVERLAY
                             | DDHAL_SURFCB32_SETOVERLAYPOSITION
#endif
#if ENABLE_PALETTE_CAPS
                             | DDHAL_SURFCB32_SETPALETTE
#endif
                             ;

  pSurfaceCallBacks->DestroySurface     = DdDestroySurface;
  pSurfaceCallBacks->Flip               = DdFlip;
  pSurfaceCallBacks->Lock               = DdLock;
  pSurfaceCallBacks->Unlock             = DdUnlock;
  pSurfaceCallBacks->Blt                = DdBlt;
#if ENABLE_OVERLAY_CAPS
  pSurfaceCallBacks->SetColorKey        = DdSetSurfaceColorKey;
#endif
  pSurfaceCallBacks->AddAttachedSurface = DdAddAttachedSurface;
  pSurfaceCallBacks->GetBltStatus       = DdGetBltStatus;
  pSurfaceCallBacks->GetFlipStatus      = DdGetFlipStatus;
#if ENABLE_OVERLAY_CAPS
  pSurfaceCallBacks->UpdateOverlay      = UpdateOverlay32;
  pSurfaceCallBacks->SetOverlayPosition = SetOverlayPosition32;
#endif
#if ENABLE_PALETTE_CAPS
  pSurfaceCallBacks->SetPalette         = DdSetPalette;
#endif


  // fill in DirectDrawPalette object callbacks
#ifndef WINNT
  memset(pPaletteCallBacks, 0, sizeof(DDHAL_DDPALETTECALLBACKS));
  pPaletteCallBacks->dwSize  = sizeof(DDHAL_DDPALETTECALLBACKS);
#endif

#if ENABLE_PALETTE_CAPS
  pPaletteCallBacks->dwFlags = 0
                             | DDHAL_PALCB32_DESTROYPALETTE
                             | DDHAL_PALCB32_SETENTRIES
                             ;

  pPaletteCallBacks->DestroyPalette = DdDestroyPalette;
  pPaletteCallBacks->SetEntries     = DdSetEntries;
#endif

#ifdef WINNT
#if !USE_NT5_DDMEMMGR
  // convert DDraw memory allocation from reserved to permanent
  bOhCommit(ppdev, ppdev->pohDirectDraw, TRUE);
#endif
#endif

  return TRUE;

} // DrvEnableDirectDraw


/*******************************************************************/
/*                     PRIVATE FUNCTIONS                           */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  fxinit

Description:    Initialize DirectDraw information in pdevice.

Return:         FXTRUE  - success
                FXFALSE - failure
----------------------------------------------------------------------*/

BOOL __stdcall fxinit
(
  NT9XDEVICEDATA *ppdev,
  BOOL setMode
)
{
  FxBool  fxStatus = FXTRUE;
  FxU32   width, height;
  DWORD   numberTmus;
  char    *pEnvStr;


#ifndef WINNT
#ifdef WIN_CSIM
  _DD(sst3DRegs) = (SstRegs*)(_FF(regFakeBase) + SST_3D_OFFSET);
#else
  _DD(sst3DRegs) = (SstRegs*)(_FF(regBase[HWINFO_SST_3DREGS_INDEX]));
#endif

#endif
  _DD(overlaySurfaceCnt) = 0;
  _DD(fxCaps) |= FXCAPS_PALETTIZED_TEXTURES;

#ifdef WINNT
  width  = _DD(fbiWidth)  = ppdev->cxScreen;
  height = _DD(fbiHeight) = ppdev->cyScreen;
#else
  width = _DD(fbiWidth) = _FF(HALInfo).vmiData.dwDisplayWidth;
  height = _DD(fbiHeight) = _FF(HALInfo).vmiData.dwDisplayHeight;
#endif

  _DD(bufferUsage) = 0;
  _FF(ddVisibleOverlaySurf) = 0;

#ifdef DEBUG
  // System Environment
  pEnvStr = GETENV(DDDEBUGLEVEL);
  if (NULL == pEnvStr)
    _DD(DD_DebugLevel) = 0;
  else
    _DD(DD_DebugLevel) = atoi(pEnvStr);
#endif  // DEBUG

  pEnvStr = GETENV(SWAPINTERVAL);
  if (NULL == pEnvStr)
    _DD(WaitOnVsync) = 1;
  else
    _DD(WaitOnVsync) = atoi(pEnvStr);

#if defined(SLI_AA) && (!defined(WINNT) || (_WIN32_WINNT >= 0x0500))

  // Initialize SLI and AA variables.

  _DD(ddSLIAAConfiguration) = 0;
  _DD(ddSLIAAAnalog)        = 0;
  _DD(ddAAModeEnabled)      = 0;
  _DD(ddAAModeRequested)    = 0;
  _DD(ddAAPrimaryStart)     = 0;
  _DD(ddAAZbufferStart)     = 0;
  _DD(ddAANumberSamples)    = 0;
  _DD(ddSLIModeEnabled)     = 0;
  _DD(ddSLIModeRequested)   = 0;
  _DD(ddSLINumberScanlines) = 0;

#endif // SLI_AA

  _DD(dd3DInOverlay)        = 0;

  // Initialize surface flipping variables.

  _DD(ddSurfaceFlippedFrom) = 0;
  _DD(ddSurfaceFlippedTo)   = 0;
  _DD(ddAcceleratorUsed)    = 0;

  pEnvStr = GETENV(OVERLAYMODE);
  if (NULL == pEnvStr)
    _DD(overlayFilter) = 0;
  else
    _DD(overlayFilter) = atoi(pEnvStr);

  // Reusing overlayfilter to now hold both overlay filter options as well as Alphadithermode
  pEnvStr = GETENV(ALPHADITHERMODE);
  if (NULL != pEnvStr)
    _DD(overlayFilter) |= (atoi(pEnvStr)<<2);

  if (_FF(dwRBitMask) == 0xF800)
    _DD(fbiColorModel) = FX_COLORMODEL_RGB;
  else
    _DD(fbiColorModel) = FX_COLORMODEL_BGR;

#if ENABLE_3D

  // This code should be code inspected in conjunction with the
  // Direct3D team.  It appears to be mostly used for texture
  // allocation on Voodoo2.  I already removed some of the most
  // obvious obsolete code and unreferenced variables, but the
  // cleanup is not complete. -CGW-

  // only for tmu 0
  _DD(startTREXMem) = SST_TEX_ADDRESS(ghw0);
  _DD(totalSizeTREXMem) = 0;//info.tmuMemSize[0] << 20;

  // Initially no locks outstanding
  _FX(openLockCount) = 0;
  _FX(openLockIndex) = 0;

  // 16 bit driver needs this to initialize the heaps
  _FX(numTmus) = 1;                 // banshee only has one
  for (numberTmus=0; numberTmus < _FX(numTmus); ++numberTmus)

  if (_FX(numTmus) == 2)
    _FX(flags) = TRILINEAR_SPLIT2TMUS;
  else
    _FX(flags) = 0;

  {
#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
    DWORD baseAddress     = 0;
#else
    DWORD baseAddress     = _DS(LFBBASE);
#endif
    _FX(numTextureHeaps)  = 0;

    // Banshee currently only supports one TMU
    for (numberTmus=0; numberTmus < _FX(numTmus); ++numberTmus)
    {
      ++_FX(numTextureHeaps);
      _FX(textureHeapStart[numberTmus])  = baseAddress;
      _FX(textureHeapLength[numberTmus]) = 0x200000; // WRONG - need to fix
    }
  } // tmu init data
#endif

#ifdef Z_ACCESS_OPT
  // Default the Z Clear Optimization as on.
  pEnvStr = GETENV(Z_CLEAR_OPTIMIZATION);
  if (NULL == pEnvStr)
    _DD(ddEnableZClearOpt) = 1;
  else
    _DD(ddEnableZClearOpt) = atoi(pEnvStr);

  _DD(ddFlipsWithoutZClear) = 0;
#endif

  return FXTRUE;

} // fxinit

/**************************************************************************
* S T A T I C   F U N C T I O N S
***************************************************************************/

#ifdef WINNT

#define DDRAWHEAP_DBGLVL  0

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
/*----------------------------------------------------------------------
Function name:  ComputeOptimalSLIConfig

Description:    

Return:         
----------------------------------------------------------------------*/

static VOID
ComputeOptimalSliConfig(PDEV *ppdev)
{
  ULONG   sliGdiDesktopSize, optSliFrontBuffer, optSliBackBuffer, optSliZBuffer, optSliThirdBuffer;
  ULONG   ulTileCtrl;
  double  numerator, denominator, tileCtrl;
  BOOL    bFound;
  ULONG   org_optSliFrontBuffer;


  // the optimal sli config is
  sliGdiDesktopSize = _FF(gdiDesktopSize) / _FF(dwNumUnits);

  optSliBackBuffer = _FF(TotalVRAM) - sliGdiDesktopSize;
  DISPDBG((DDRAWHEAP_DBGLVL, "    optimal SLI BB hwPtr=%8lXh", optSliBackBuffer));

  optSliZBuffer     = optSliBackBuffer - sliGdiDesktopSize - _FF(ddTileStride) * SST_TILE_SIZE;
  optSliZBuffer    += SST_TILE_SIZE;
  DISPDBG((DDRAWHEAP_DBGLVL, "    optimal SLI ZB hwPtr=%8lXh", optSliZBuffer));

  optSliThirdBuffer  = optSliZBuffer - SST_TILE_SIZE - sliGdiDesktopSize;
  DISPDBG((DDRAWHEAP_DBGLVL, "    optimal SLI TB hwPtr=%8lXh", optSliThirdBuffer));

  optSliFrontBuffer = optSliThirdBuffer - sliGdiDesktopSize;
  DISPDBG((DDRAWHEAP_DBGLVL, "    optimal SLI FB hwPtr=%8lXh", optSliFrontBuffer));

  org_optSliFrontBuffer = optSliFrontBuffer;

  // The equation below comes from SLI_HwPtrToLfbPtr and has be rearranged to
  // find the tileMark (or tileCtrl as it's called here)
  // the xOffset from SLI_HwPtrToLfbPtr is assumed to be zero

  // we can't move the primary's lfb address, so compute the tileMark that
  // would give us the closest we can get to the above optSliFrontBuffer hwPtr
  numerator = SST_TILE_SIZE * (double)_FF(ddTileStride) * (double)_FF(ddPrimarySurfaceData).lfbPtr;
  numerator -= (double)_FF(dwNumUnits) * (double)_FF(ddTilePitch) * SST_TILE_HEIGHT * (double)optSliFrontBuffer;

  denominator  = SST_TILE_SIZE * (double)_FF(ddTileStride);
  denominator -= (double)_FF(dwNumUnits) * (double)_FF(ddTilePitch) * SST_TILE_HEIGHT;

  tileCtrl = numerator / denominator;
  ulTileCtrl = (ULONG)tileCtrl;
  DISPDBG((DDRAWHEAP_DBGLVL, "    optimal tileCtrl = %8lXh", ulTileCtrl));

  // tileCtrl must be aligned on a 4kB boundary
  if (ulTileCtrl & 0xFFF)
  {
    ulTileCtrl &= ~0xFFF;
    DISPDBG((DDRAWHEAP_DBGLVL, "    4kB aligned tileCtrl = %8lXh", ulTileCtrl));

    bFound = FALSE;

    // brute force loop to find a tileMark for sli mode that
    // meets the following requirements:
    // - tileCtrl & tileCompare are 4kB aligned
    // - the hwPtr is tile aligned and less than or equal to the org_optSliFrontBuffer
    // - the lfbPtr of the gdi desktop doesn't change (although the hwPtr can and most likely will change)
    while (ulTileCtrl >= _FF(ddTiledHeapStart))
    {
      _FF(ddTileMark) = ulTileCtrl;
      DISPDBG((DDRAWHEAP_DBGLVL+10, "    trying ulTileCtrl=%8lXh", ulTileCtrl));

      optSliFrontBuffer = SLI_LfbPtrToHwPtr(ppdev, _FF(ddPrimarySurfaceData).lfbPtr);
      DISPDBG((DDRAWHEAP_DBGLVL+10, "      new optimal SLI FB hwPtr=%8lXh, lfbPtr=%8lXh",
               optSliFrontBuffer, SLI_HwPtrToLfbPtr(ppdev, optSliFrontBuffer)));

      // don't bother checking this if the new optimal sliFrontBuffer isn't at
      // least to the org_optSliFrontBuffer position
      if (optSliFrontBuffer <= org_optSliFrontBuffer)
      {
        if (SLI_HwPtrToLfbPtr(ppdev, optSliFrontBuffer) == _FF(ddPrimarySurfaceData).lfbPtr)
        {
          bFound = TRUE;

          optSliThirdBuffer = optSliFrontBuffer + sliGdiDesktopSize;
          DISPDBG((DDRAWHEAP_DBGLVL+10, "      new optimal SLI TB hwPtr=%8lXh, lfbPtr=%8lXh",
                   optSliThirdBuffer, SLI_HwPtrToLfbPtr(ppdev, optSliThirdBuffer)));
    
          optSliZBuffer = optSliThirdBuffer + sliGdiDesktopSize + SST_TILE_SIZE;
          DISPDBG((DDRAWHEAP_DBGLVL+10, "      new optimal SLI ZB hwPtr=%8lXh, lfbPtr=%8lXh",
                   optSliZBuffer, SLI_HwPtrToLfbPtr(ppdev, optSliZBuffer)));
          
          optSliBackBuffer = optSliZBuffer - SST_TILE_SIZE + _FF(ddTileStride) * SST_TILE_SIZE + sliGdiDesktopSize;
          DISPDBG((DDRAWHEAP_DBGLVL+10, "      new optimal SLI BB hwPtr=%8lXh, lfbPtr=%8lXh",
                   optSliBackBuffer, SLI_HwPtrToLfbPtr(ppdev, optSliBackBuffer)));
    
          break;
        }
      }

      ulTileCtrl -= SST_TILE_SIZE;
    }
    _FF(ddTileMark) = _FF(ddTiledHeapStart);
  }
  else
  {
    bFound = TRUE;
  }

  if (! bFound)
  {
    DISPDBG((DDRAWHEAP_DBGLVL, "    unable to find a tileCtrl to match optimal sli setup"));
    DISPDBG((DDRAWHEAP_DBGLVL, "    using non-optimal setup"));

    // non optimized locations
    _FF(sliFrontBuffer) = SLI_LfbPtrToHwPtr(ppdev, _FF(ddPrimarySurfaceData).lfbPtr);
    DISPDBG((DDRAWHEAP_DBGLVL, "  non optimized FB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliFrontBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliFrontBuffer))));
    
    _FF(sliTileCtrl)    = _FF(ddTileMark);
    _FF(sliTileCompare) = _FF(sliFrontBuffer);

    // Compute addresses of sli buffers for n-way SLI (AA disabled)
    _FF(sliBackBuffer) = _FF(TotalVRAM) - sliGdiDesktopSize;
    DISPDBG((DDRAWHEAP_DBGLVL, "  non optimized BB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliBackBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliBackBuffer))));
    
    _FF(sliZBuffer) = _FF(sliBackBuffer) - sliGdiDesktopSize - _FF(ddTileStride) * SST_TILE_SIZE + SST_TILE_SIZE;
    DISPDBG((DDRAWHEAP_DBGLVL, "  non optimized ZB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliZBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliZBuffer))));
    
    _FF(sliThirdBuffer) = _FF(sliZBuffer) - SST_TILE_SIZE - sliGdiDesktopSize;
    DISPDBG((DDRAWHEAP_DBGLVL, "  non optimized TB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliThirdBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliThirdBuffer))));

    DISPDBG((DDRAWHEAP_DBGLVL, "    non optimized extra sli heap size = %8lXh",
             _FF(sliFrontBuffer) - _FF(ddTiledHeapStart)));
  }
  else
  {
#if DBG
    ULONG nonOptSliExtraHeapSize, optSliExtraHeapSize;
#endif

    DISPDBG((DDRAWHEAP_DBGLVL, "    using ulTileCtrl=%8lXh", ulTileCtrl));

    _FF(sliTileCtrl)    = ulTileCtrl;
    _FF(sliTileCompare) = optSliFrontBuffer & ~0xFFF;

#if DBG
    // non optimized sliFrontBuffer location
    _FF(sliFrontBuffer) = SLI_LfbPtrToHwPtr(ppdev, _FF(ddPrimarySurfaceData).lfbPtr);
    DISPDBG((DDRAWHEAP_DBGLVL, "    non optimized FB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliFrontBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliFrontBuffer))));
    nonOptSliExtraHeapSize = _FF(sliFrontBuffer) - _FF(ddTiledHeapStart);
    DISPDBG((DDRAWHEAP_DBGLVL, "    non optimized extra sli heap size = %8lXh", nonOptSliExtraHeapSize));

    // set ddTileMark to sliTileCtrl so the following SLI_HwPtrToLfbPtr's will
    // work as if in sli mode
    _FF(ddTileMark) = _FF(sliTileCtrl);
#endif

    // optimized locations
    _FF(sliFrontBuffer) = optSliFrontBuffer;
    DISPDBG((DDRAWHEAP_DBGLVL, "  optimized FB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliFrontBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliFrontBuffer))));
    
    _FF(sliBackBuffer)  = optSliBackBuffer;
    DISPDBG((DDRAWHEAP_DBGLVL, "  optimized BB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliBackBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliBackBuffer))));
    
    _FF(sliZBuffer)     = optSliZBuffer;
    DISPDBG((DDRAWHEAP_DBGLVL, "  optimized ZB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliZBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliZBuffer))));
    
    _FF(sliThirdBuffer) = optSliThirdBuffer;
    DISPDBG((DDRAWHEAP_DBGLVL, "  optimized TB (SLI) hwPtr=%8lXh, lfbPtr=%8lXh",
             _FF(sliThirdBuffer), SLI_HwPtrToLfbPtr(ppdev, _FF(sliThirdBuffer))));

#if DBG
    optSliExtraHeapSize = _FF(sliTileCompare) - _FF(ddTiledHeapStart);
    DISPDBG((DDRAWHEAP_DBGLVL, "    optimized extra sli heap size = %8lXh", optSliExtraHeapSize));

    DISPDBG((DDRAWHEAP_DBGLVL, "    extra heap size = %8lXh", optSliExtraHeapSize - nonOptSliExtraHeapSize));

    // restore the normal ddTileMark value
    _FF(ddTileMark) = _FF(ddTiledHeapStart);
#endif
  }
}
#endif

/*----------------------------------------------------------------------
Function name:  ReportNTDDrawHeaps

Description:    Initialize Windows NT ddraw heap information.

                NOTE: This initialization is performed on W9X by
                16-bit display driver code instead.

Return:         NONE
----------------------------------------------------------------------*/

#if USE_NT5_DDMEMMGR

static VOID
ReportNTDDrawHeaps(PDEV *ppdev, DWORD *pdwNumHeaps, VIDEOMEMORY *pvmList)
{
  DWORD     cHeaps;
#if ENABLE_RECONFIG_VIDMEM
  DWORD     heapStart;
#endif


  cHeaps = 0;
  ppdev->pvmList = pvmList;

#if ENABLE_RECONFIG_VIDMEM
  // make a linear heap
  if (0 < _FF(ddLinearHeapSize))
  {
    cHeaps++;

    if (NULL != pvmList)
    {
      pvmList[LINEAR_HEAP0_ID].dwFlags           = VIDMEM_ISLINEAR;
      pvmList[LINEAR_HEAP0_ID].fpStart           = _FF(ddLinearHeapStart);
      pvmList[LINEAR_HEAP0_ID].fpEnd             = pvmList[LINEAR_HEAP0_ID].fpStart + _FF(ddLinearHeapSize) - 1;
      pvmList[LINEAR_HEAP0_ID].ddsCaps.dwCaps    = 0;
      pvmList[LINEAR_HEAP0_ID].ddsCapsAlt.dwCaps = 0;

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh",
               pvmList[LINEAR_HEAP0_ID].fpStart, pvmList[LINEAR_HEAP0_ID].fpEnd));
    }
  }
  // make secondary aa heaps
  if (0 < _FF(ddSecondaryHeapSize))
  {
    cHeaps += 4;

    if (NULL != pvmList)
    {
      heapStart  = _FF(ddSecondaryHeapStart);

      // heap for aa third buffer
      // even alignment
      pvmList[LINEAR_HEAP4_ID].dwFlags           = VIDMEM_ISLINEAR;
      pvmList[LINEAR_HEAP4_ID].fpStart           = heapStart;
      _DS(ddLinearHeap4Start)                    = heapStart;
      pvmList[LINEAR_HEAP4_ID].fpEnd             = pvmList[LINEAR_HEAP4_ID].fpStart + _FF(gdiDesktopSize) - 1;
      pvmList[LINEAR_HEAP4_ID].ddsCaps.dwCaps    = 0;
      pvmList[LINEAR_HEAP4_ID].ddsCapsAlt.dwCaps = 0;

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh (AA Third Buffer Heap)",
               pvmList[LINEAR_HEAP4_ID].fpStart, pvmList[LINEAR_HEAP4_ID].fpEnd));

      heapStart += _FF(gdiDesktopSize);

      // heap for aa z buffer
      // odd alignment
      heapStart += SST_TILE_SIZE;
      pvmList[LINEAR_HEAP3_ID].dwFlags           = VIDMEM_ISLINEAR;
      pvmList[LINEAR_HEAP3_ID].fpStart           = heapStart;
      _DS(ddLinearHeap3Start)                    = heapStart;
      pvmList[LINEAR_HEAP3_ID].fpEnd             = pvmList[LINEAR_HEAP3_ID].fpStart + _FF(gdiDesktopSize) - 1;
      pvmList[LINEAR_HEAP3_ID].ddsCaps.dwCaps    = 0;
      pvmList[LINEAR_HEAP3_ID].ddsCapsAlt.dwCaps = 0;

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh (AA Z Buffer Heap)",
               pvmList[LINEAR_HEAP3_ID].fpStart, pvmList[LINEAR_HEAP3_ID].fpEnd));

      heapStart -= SST_TILE_SIZE;
      heapStart += _FF(gdiDesktopSize) + (_FF(ddTileStride) * SST_TILE_SIZE * _FF(dwNumUnits));

      // heap for aa backbuffer
      // even alignment
      pvmList[LINEAR_HEAP2_ID].dwFlags           = VIDMEM_ISLINEAR;
      pvmList[LINEAR_HEAP2_ID].fpStart           = heapStart;
      _DS(ddLinearHeap2Start)                    = heapStart;
      pvmList[LINEAR_HEAP2_ID].fpEnd             = pvmList[LINEAR_HEAP2_ID].fpStart + _FF(gdiDesktopSize) - 1;
      pvmList[LINEAR_HEAP2_ID].ddsCaps.dwCaps    = 0;
      pvmList[LINEAR_HEAP2_ID].ddsCapsAlt.dwCaps = 0;

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh (AA Back Buffer Heap)",
               pvmList[LINEAR_HEAP2_ID].fpStart, pvmList[LINEAR_HEAP2_ID].fpEnd));

      heapStart += _FF(gdiDesktopSize);

      // heap for aa primary buffer
      // even alignment
      pvmList[LINEAR_HEAP1_ID].dwFlags           = VIDMEM_ISLINEAR;
      pvmList[LINEAR_HEAP1_ID].fpStart           = heapStart;
      _DS(ddLinearHeap1Start)                    = heapStart;
      pvmList[LINEAR_HEAP1_ID].fpEnd             = pvmList[LINEAR_HEAP1_ID].fpStart + _FF(gdiDesktopSize) - 1;
      pvmList[LINEAR_HEAP1_ID].ddsCaps.dwCaps    = 0;
      pvmList[LINEAR_HEAP1_ID].ddsCapsAlt.dwCaps = 0;

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh (AA Front Buffer Heap)",
               pvmList[LINEAR_HEAP1_ID].fpStart, pvmList[LINEAR_HEAP1_ID].fpEnd));
    }
  }
  // make rectangular heap(s)
  if (0 < _FF(ddTiledHeapSize))
  {
    cHeaps += _FF(ddNumColorBuff);

    _FF(ddPrimarySurfaceData).lfbPtr = (DWORD)(ppdev->pjScreen - ppdev->pjScreenBase);
    _FF(ddPrimarySurfaceData).hwPtr  = ppdev->ulScreenOffset;
    _FF(ddPrimarySurfaceData).lPitch = ppdev->lDelta;

    if (NULL != pvmList)
    {
      heapStart  = _FF(ddTiledHeapStart);

      switch (_FF(ddNumColorBuff))
      {
        case 3:
          // heap for third buffer when triple buffering
          // even alignment
          pvmList[TILED_HEAP2_ID].dwFlags           = VIDMEM_ISRECTANGULAR;
          pvmList[TILED_HEAP2_ID].fpStart           = HwPtrToLfbPtr(ppdev, heapStart);
          _DS(ddTiledHeap2Start)                    = heapStart;
          pvmList[TILED_HEAP2_ID].dwWidth           = _FF(ddTileStride) * SST_TILE_WIDTH;
          pvmList[TILED_HEAP2_ID].dwHeight          = _FF(ddTileHeight) * SST_TILE_HEIGHT;
          // don't allow texture or overlay surfaces in tiled heaps
          pvmList[TILED_HEAP2_ID].ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_TEXTURE;
          pvmList[TILED_HEAP2_ID].ddsCapsAlt.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_TEXTURE;

          DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw tiled heap reported as fpStart=%8lXh, dwWidth=%8lXh, dwHeight=%lXh, hwStart=%8lXh (Third Buffer Heap)",
                   pvmList[TILED_HEAP2_ID].fpStart, pvmList[TILED_HEAP2_ID].dwWidth, pvmList[TILED_HEAP2_ID].dwHeight, heapStart));

          heapStart += _FF(gdiDesktopSize);

        case 2:
          // heap for z buffer
          // odd alignment
          heapStart += SST_TILE_SIZE;

          pvmList[TILED_HEAP1_ID].dwFlags           = VIDMEM_ISRECTANGULAR;
          pvmList[TILED_HEAP1_ID].fpStart           = HwPtrToLfbPtr(ppdev, heapStart);
          _DS(ddTiledHeap1Start)                    = heapStart;
          pvmList[TILED_HEAP1_ID].dwWidth           = _FF(ddTileStride) * SST_TILE_WIDTH;
          pvmList[TILED_HEAP1_ID].dwHeight          = _FF(ddTileHeight) * SST_TILE_HEIGHT;
          // don't allow texture or overlay surfaces in tiled heaps
          pvmList[TILED_HEAP1_ID].ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_TEXTURE;
          pvmList[TILED_HEAP1_ID].ddsCapsAlt.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_TEXTURE;

          DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw tiled heap reported as fpStart=%8lXh, dwWidth=%8lXh, dwHeight=%lXh, hwStart=%8lXh (Z Buffer Heap)",
                   pvmList[TILED_HEAP1_ID].fpStart, pvmList[TILED_HEAP1_ID].dwWidth, pvmList[TILED_HEAP1_ID].dwHeight, heapStart));

          heapStart -= SST_TILE_SIZE;
          heapStart += _FF(gdiDesktopSize) + (_FF(ddTileStride) * SST_TILE_SIZE * _FF(dwNumUnits));

        case 1:
          // heap for backbuffer
          // even alignment
          pvmList[TILED_HEAP0_ID].dwFlags           = VIDMEM_ISRECTANGULAR;
          pvmList[TILED_HEAP0_ID].fpStart           = HwPtrToLfbPtr(ppdev, heapStart);
          _DS(ddTiledHeap0Start)                    = heapStart;
          pvmList[TILED_HEAP0_ID].dwWidth           = _FF(ddTileStride) * SST_TILE_WIDTH;
          pvmList[TILED_HEAP0_ID].dwHeight          = _FF(ddTileHeight) * SST_TILE_HEIGHT;
          // don't allow texture or overlay surfaces in tiled heaps
          pvmList[TILED_HEAP0_ID].ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_TEXTURE;
          pvmList[TILED_HEAP0_ID].ddsCapsAlt.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_TEXTURE;

          DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw tiled heap reported as fpStart=%8lXh, dwWidth=%8lXh, dwHeight=%lXh, hwStart=%8lXh (Back Buffer Heap)",
                   pvmList[TILED_HEAP0_ID].fpStart, pvmList[TILED_HEAP0_ID].dwWidth, pvmList[TILED_HEAP0_ID].dwHeight, heapStart));

          DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw primary surface at     fpStart=%8lXh, dwWidth=%8lXh, dwHeight=%lXh, hwStart=%8lXh (Front Buffer)",
                   _FF(ddPrimarySurfaceData).lfbPtr, _FF(ddTileStride) * SST_TILE_WIDTH, _FF(ddTileHeight) * SST_TILE_HEIGHT, _FF(ddPrimarySurfaceData).hwPtr));
      }

      if ((IS_NAPALM) && (_FF(dwNumUnits) > 1))
      {
        // Compute addresses of aa buffers for 2-way SLI/2 sample AA mode
        switch (_FF(ddNumColorBuff))
        {
          case 3: _FF(secondaryThirdBuffer)  = _FF(ddTiledHeap2Start);                     // third buffer hwptr
                  _FF(secondaryThirdBuffer) += _FF(ddTiledHeapSize) + _FF(gdiDesktopSize); // add tiled memory size
                  _FF(secondaryThirdBuffer) -= _FF(ddTiledHeapStart);                      // subtract tiled mark
                  _FF(secondaryThirdBuffer) /= _FF(dwNumUnits);                            // divide by number of chips
                  _FF(secondaryThirdBuffer) += _FF(ddTiledHeapStart);                      // add tiled mark
                  DISPDBG((DDRAWHEAP_DBGLVL, "  TB (AA)=%8lXh", _FF(secondaryThirdBuffer)));
          case 2: _FF(secondaryZBuffer)      = _FF(ddTiledHeap1Start) - SST_TILE_SIZE;     // zbuffer hwptr
                  _FF(secondaryZBuffer)     += _FF(ddTiledHeapSize) + _FF(gdiDesktopSize); // add tiled memory size
                  _FF(secondaryZBuffer)     -= _FF(ddTiledHeapStart);                      // subtract tiled mark
                  _FF(secondaryZBuffer)     /= _FF(dwNumUnits);                            // divide by number of chips
                  _FF(secondaryZBuffer)     += _FF(ddTiledHeapStart) + SST_TILE_SIZE;      // add tiled mark
                  DISPDBG((DDRAWHEAP_DBGLVL, "  ZB (AA)=%8lXh", _FF(secondaryZBuffer)));
          case 1: _FF(secondaryBackBuffer)   = _FF(ddTiledHeap0Start);                     // back buffer hwptr
                  _FF(secondaryBackBuffer)  += _FF(ddTiledHeapSize) + _FF(gdiDesktopSize); // add tiled memory size
                  _FF(secondaryBackBuffer)  -= _FF(ddTiledHeapStart);                      // subtract tiled mark
                  _FF(secondaryBackBuffer)  /= _FF(dwNumUnits);                            // divide by number of chips
                  _FF(secondaryBackBuffer)  += _FF(ddTiledHeapStart);                      // add tiled mark
                  DISPDBG((DDRAWHEAP_DBGLVL, "  BB (AA)=%8lXh", _FF(secondaryBackBuffer)));
          case 0: _FF(secondaryFrontBuffer)  = _FF(gdiDesktopStart) & ~SSTG_IS_TILED;      // front buffer hwptr
                  _FF(secondaryFrontBuffer) += _FF(ddTiledHeapSize) + _FF(gdiDesktopSize); // add tiled memory size
                  _FF(secondaryFrontBuffer) -= _FF(ddTiledHeapStart);                      // subtract tiled mark
                  _FF(secondaryFrontBuffer) /= _FF(dwNumUnits);                            // divide by number of chips
                  _FF(secondaryFrontBuffer) += _FF(ddTiledHeapStart);                      // add tiled mark
                  DISPDBG((DDRAWHEAP_DBGLVL, "  FB (AA)=%8lXh", _FF(secondaryFrontBuffer)));
        }
      }
    }

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
    if ((IS_NAPALM) &&
        // need to have 2 or more chips to do sli
        (2 <= _FF(dwNumUnits)) &&
        // need numColorBuffers * dwNumUnits to be >= 4 to do this
        // so for a 2-chip board we need two colorBuffers
        // and for a 4-chip board we need at least one colorBuffer
        (4 <= (_FF(ddNumColorBuff) * _FF(dwNumUnits))) &&
        // Compute_SLIAA_Config in ddfxnt.c doesn't allow sli or aa modes
        // when either the SST_HALF_MODE or SST_VIDEO_2X_MODE_EN bits in
        // vidProcCfg are set, so may as well skip those modes here as well
        (! ((SST_HALF_MODE | SST_VIDEO_2X_MODE_EN) & GET(ghwIO->vidProcCfg))))
    {
      DISPDBG((DDRAWHEAP_DBGLVL, ""));
  
      // default to extraSliLinearHeap enabled
      // but allow registry key to override
      if (GetRegDWORD(ppdev, "UseSliExtraLinearHeap", &_FF(bUseSliExtraLinearHeap)))
      {
        if (0 == _FF(bUseSliExtraLinearHeap))
        {
          DISPDBG((DDRAWHEAP_DBGLVL, "  extraSliLinearHeap disabled in registry"));
        }
        else
        {
          DISPDBG((DDRAWHEAP_DBGLVL, "  extraSliLinearHeap enabled in registry"));
        }
      }
      else
      {
  #if 0
        DISPDBG((DDRAWHEAP_DBGLVL, "  defaulting to extraSliLinearHeap disabled"));
        _FF(bUseSliExtraLinearHeap) = 0;
  #else
        DISPDBG((DDRAWHEAP_DBGLVL, "  defaulting to extraSliLinearHeap enabled"));
        _FF(bUseSliExtraLinearHeap) = 1;
  #endif
      }
  
      if (_FF(bUseSliExtraLinearHeap))
      {
        cHeaps++;
        
        if (NULL != pvmList)
        {
          ComputeOptimalSliConfig(ppdev);

          // report extra linear heap for sli mode (AA disabled)
          pvmList[cHeaps - 1].dwFlags           = VIDMEM_ISLINEAR;
          pvmList[cHeaps - 1].fpStart           = _FF(ddTiledHeapStart);
          pvmList[cHeaps - 1].fpEnd             = _FF(sliTileCompare) - 1;
          pvmList[cHeaps - 1].ddsCaps.dwCaps    = 0;
          pvmList[cHeaps - 1].ddsCapsAlt.dwCaps = 0;
          
          DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw sli extra linear heap reported as fpStart=%8lXh, fpEnd=%8lXh",
                   pvmList[cHeaps - 1].fpStart, pvmList[cHeaps - 1].fpEnd));
        }
      }
    }
    else
    {
      _FF(bUseSliExtraLinearHeap) = 0;
    }
#endif
  }
#else
  // make one linear heap
  if (0 < (ppdev->cyDDHeap + ppdev->cyDDMemoryExtra))
  {
    cHeaps++;

    if (NULL != pvmList)
    {
      pvmList->dwFlags           = VIDMEM_ISLINEAR;
      pvmList->fpStart           = ppdev->ulScreenOffset
                                 + ((ppdev->cyScreen + ppdev->cyText + ppdev->cyDDSlopHeight) * ppdev->lDelta)
                                 ;
      pvmList->fpEnd             = pvmList->fpStart
                                 + ((ppdev->cyDDMemoryExtra + ppdev->cyDDHeap) * ppdev->lDelta)
                                 - 1;
      pvmList->ddsCaps.dwCaps    = 0;
      pvmList->ddsCapsAlt.dwCaps = 0;
    }
  }
#endif

  ppdev->cHeaps = cHeaps;
  *pdwNumHeaps = cHeaps;
}

#else

// NT4 version
static VOID
ReportNTDDrawHeaps(PDEV *ppdev, DWORD *pdwNumHeaps, VIDEOMEMORY *pvmList)
{
  OH        *poh;


  poh = ppdev->pohDirectDraw;
  if (poh != NULL)
  {
    *pdwNumHeaps = 1;
    if (0 != ppdev->cyDDMemoryExtra)
    {
      // if there are two ddraw heaps
      // and if we can combine them into a single heap
      // then report only one heap
      if ((0 == poh->x) && (poh->cxReserved == ppdev->cxMemory) &&
          ((poh->y + poh->cyReserved) == ppdev->cyMemory))
      {
        DISPDBG((1, "DDraw heaps will be combined to a single heap"));
      }
      // otherwise report two heaps
      else
        *pdwNumHeaps = 2;
    }

    // Fill in the list of off-screen rectangles if we've been asked
    // to do so:

    if (pvmList != NULL)
    {
      // if the heap width isn't the screen pitch then we can't say it's linear!
      ASSERTDD2((poh->cxReserved * ppdev->cjPelSize) == ppdev->lDelta);

      // if there are two ddraw heaps
      // and if we can combine them into a single heap
      // then do so
      if ((0 != ppdev->cyDDMemoryExtra) &&
          (0 == poh->x) && (poh->cxReserved == ppdev->cxMemory) &&
          ((poh->y + poh->cyReserved) == ppdev->cyMemory))
      {
        DISPDBG((1, "DDraw heaps being combined to a single heap"));
        DISPDBG((1, "DirectDraw gets %li x %li surface at (%li, %li)",
                poh->cxReserved, poh->cyReserved + ppdev->cyDDMemoryExtra, poh->x, poh->y));


        // unlike Win9x, linear heaps for NT do not specify fpEnd as inclusive
        // therefore do not set fpEnd = fpStart + heapSize - 1
        // on NT fpEnd must be set to fpStart + heapSize
        pvmList->dwFlags = VIDMEM_ISLINEAR;
        pvmList->fpStart = ppdev->ulScreenOffset +
                           (poh->y * ppdev->lDelta);
        pvmList->fpEnd   = pvmList->fpStart
                         + ppdev->lDelta * (poh->cyReserved + ppdev->cyDDMemoryExtra);
        pvmList->ddsCaps.dwCaps    = 0;
        pvmList->ddsCapsAlt.dwCaps = 0;
      }
      // either there is only one ddraw heap or
      // there are two heaps that can't be combined
      else
      {
        DISPDBG((1, "DirectDraw gets %li x %li surface at (%li, %li)",
                poh->cxReserved, poh->cyReserved, poh->x, poh->y));

        // unlike Win9x, linear heaps for NT do not specify fpEnd as inclusive
        // therefore do not set fpEnd = fpStart + heapSize - 1
        // on NT fpEnd must be set to fpStart + heapSize
        pvmList->dwFlags           = VIDMEM_ISLINEAR;
        pvmList->fpStart           = ppdev->ulScreenOffset
                                   + (poh->y * ppdev->lDelta)
                                   + (poh->x * ppdev->cjPelSize);
        pvmList->fpEnd             = pvmList->fpStart
                                   + ppdev->lDelta * poh->cyReserved;
        pvmList->ddsCaps.dwCaps    = 0;
        pvmList->ddsCapsAlt.dwCaps = 0;

        if (0 != ppdev->cyDDMemoryExtra)
        {
          DISPDBG((1, "DirectDraw gets 2nd heap: %li x %li surface at (%li, %li)",
                  ppdev->cxMemory, ppdev->cyDDMemoryExtra, 0, ppdev->cyMemory));

          // unlike Win9x, linear heaps for NT do not specify fpEnd as inclusive
          // therefore do not set fpEnd = fpStart + heapSize - 1
          // on NT fpEnd must be set to fpStart + heapSize
          pvmList[1].dwFlags           = VIDMEM_ISLINEAR;
          pvmList[1].fpStart           = ppdev->ulScreenOffset
                                       + (ppdev->cyMemory * ppdev->lDelta);
          pvmList[1].fpEnd             = pvmList[1].fpStart
                                       + (ppdev->lDelta * ppdev->cyDDMemoryExtra);
          pvmList[1].ddsCaps.dwCaps    = 0;
          pvmList[1].ddsCapsAlt.dwCaps = 0;
        }
      }
    }
  }
}

#endif

/*----------------------------------------------------------------------
Function name:  NTSpecificInit

Description:    Initialize Windows NT specific information.

                NOTE: This initialization is performed on W9X by
                16-bit display driver code instead.

Return:         NONE
----------------------------------------------------------------------*/

static VOID
NTSpecificInit(PDEV         *ppdev,
               DDHALINFO    *pHalInfo,
               DWORD        *pdwNumHeaps,
               VIDEOMEMORY  *pvmList,
               DWORD        *pdwNumFourCC,
               DWORD        *pdwFourCC)
{
#ifndef PERF_DISABLE_CANBLTSYSMEM
  char  *pEnvStr;
  BOOL  bCanBltSysMem;
#endif


  // Current primary surface attributes.  Since HalInfo is zero-initialized
  // by GDI, we only have to fill in the fields which should be non-zero:

  // If we use command fifo we may need to set pHalInfo->vmiData.fpPrimary

#if ENABLE_RECONFIG_VIDMEM
  pHalInfo->vmiData.fpPrimary       = ppdev->pjScreen - ppdev->pjScreenBase;
#else
  pHalInfo->vmiData.fpPrimary       = ppdev->ulScreenOffset;
#endif
  pHalInfo->vmiData.pvPrimary       = ppdev->pjScreen;
  pHalInfo->vmiData.dwDisplayWidth  = ppdev->cxScreen;
  pHalInfo->vmiData.dwDisplayHeight = ppdev->cyScreen;
  pHalInfo->vmiData.lDisplayPitch   = ppdev->lDelta;

  pHalInfo->vmiData.ddpfDisplay.dwSize  = sizeof(DDPIXELFORMAT);
  pHalInfo->vmiData.ddpfDisplay.dwFlags = DDPF_RGB;

  pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = ppdev->cBitsPerPel;

  if (ppdev->iBitmapFormat == BMF_8BPP)
  {
    pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
  }

  // These masks will be zero at 8bpp:

  pHalInfo->vmiData.ddpfDisplay.dwRBitMask = ppdev->flRed;
  pHalInfo->vmiData.ddpfDisplay.dwGBitMask = ppdev->flGreen;
  pHalInfo->vmiData.ddpfDisplay.dwBBitMask = ppdev->flBlue;

  // report ddraw heap
  ReportNTDDrawHeaps(ppdev, pdwNumHeaps, pvmList);

  // We have to tell DirectDraw our preferred off-screen alignment.
  // dword alignment must be guaranteed for off-screen surfaces:

  pHalInfo->vmiData.dwOffscreenAlign = 4;

#if ENABLE_3D
  pHalInfo->vmiData.dwZBufferAlign = 16;
  pHalInfo->vmiData.dwTextureAlign = 16;
#endif

#if (_WIN32_WINNT >= 0x0500)
  pHalInfo->ddCaps.dwCaps2 = 0
                           | DDCAPS2_COPYFOURCC
                           | DDCAPS2_CANFLIPODDEVEN          // Support flip with ODD/EVEN flags, DX-6 AGUS
                           | DDCAPS2_WIDESURFACES            // Allow allocation of surfaces wider than primary.
#if ENABLE_VIDEOPORT
                           | DDCAPS2_VIDEOPORT
                           | DDCAPS2_AUTOFLIPOVERLAY
                           | DDCAPS2_CANBOBINTERLEAVED       // Disabling caused C-CUBE weave to fail
                           | DDCAPS2_CANBOBNONINTERLEAVED
                           | DDCAPS2_CANBOBHARDWARE
#endif
#ifdef STBPERF_USE_FLIPNOVSYNC
                           | DDCAPS2_FLIPNOVSYNC             // Support flip without VSYNC
#endif
                           ;

  pHalInfo->dwFlags = 0
                    | DDHALINFO_GETDRIVERINFOSET
                    ;
  pHalInfo->GetDriverInfo = DdGetDriverInfo;
#endif

  if (ppdev->iBitmapFormat == BMF_8BPP)
  {
    // banshee doesn't support fourcc blts in 8bpp modes
    pHalInfo->ddCaps.dwCaps &= ~DDCAPS_BLTFOURCC;
#if ENABLE_OVERLAY_CAPS
    *pdwNumFourCC = 2;
    if (NULL != pdwFourCC)
    {
      pdwFourCC[0] = FOURCC_YUY2;
      pdwFourCC[1] = FOURCC_UYVY;
    }
#endif
  }
  else
  {
    *pdwNumFourCC = 2;
    if (NULL != pdwFourCC)
    {
      pdwFourCC[0] = FOURCC_YUY2;
      pdwFourCC[1] = FOURCC_UYVY;
    }
  }

#if (_WIN32_WINNT < 0x0500)
// NT bug, NT doesn't report rops to DdBlt correctly
// it always says the rop is SRCCOPY
  // support all 16 two operand rops
  pHalInfo->ddCaps.dwRops[0] = 0;
  pHalInfo->ddCaps.dwRops[1] = 0;
  pHalInfo->ddCaps.dwRops[2] = 0;
  pHalInfo->ddCaps.dwRops[3] = 0;
  pHalInfo->ddCaps.dwRops[4] = 0;
  pHalInfo->ddCaps.dwRops[5] = 0;
  pHalInfo->ddCaps.dwRops[6] = 0x00001000;  // S
  pHalInfo->ddCaps.dwRops[7] = 0;

  pHalInfo->ddCaps.dwSVBRops[0] = 0;
  pHalInfo->ddCaps.dwSVBRops[1] = 0;
  pHalInfo->ddCaps.dwSVBRops[2] = 0;
  pHalInfo->ddCaps.dwSVBRops[3] = 0;
  pHalInfo->ddCaps.dwSVBRops[4] = 0;
  pHalInfo->ddCaps.dwSVBRops[5] = 0;
  pHalInfo->ddCaps.dwSVBRops[6] = 0x00001000;
  pHalInfo->ddCaps.dwSVBRops[7] = 0;
#else
  // support all 16 two operand rops
  pHalInfo->ddCaps.dwRops[0] = 0x00020001;  // 0    & DSon
  pHalInfo->ddCaps.dwRops[1] = 0x00080004;  // DSna & Sn
  pHalInfo->ddCaps.dwRops[2] = 0x00200010;  // SDna & Dn
  pHalInfo->ddCaps.dwRops[3] = 0x00800040;  // DSx  & DSan
  pHalInfo->ddCaps.dwRops[4] = 0x02000100;  // DSa  & DSxn
  pHalInfo->ddCaps.dwRops[5] = 0x08000400;  // D    & DSno
  pHalInfo->ddCaps.dwRops[6] = 0x20001000;  // S    & SDno
  pHalInfo->ddCaps.dwRops[7] = 0x80004000;  // DSo  & 1

#ifndef PERF_DISABLE_CANBLTSYSMEM
  pEnvStr = GETENV(CANBLTSYSMEM);
  if (NULL == pEnvStr)
  {
    // to fix PRS 14329
    // default to system to video blt caps disabled on napalm
    // do this in spite of the performance loss it introduces
    // QA is apparently too inept to add the registry value themselves
    if (IS_NAPALM)
      bCanBltSysMem = 0;
    else // IS_VOODOO3
      bCanBltSysMem = 1;
  }
  else
    bCanBltSysMem = atoi(pEnvStr);

  if (bCanBltSysMem)
  {
    // fill in our rops and leave the sytem-to-video caps alone
    pHalInfo->ddCaps.dwSVBRops[0] = 0x00020001;
    pHalInfo->ddCaps.dwSVBRops[1] = 0x00080004;
    pHalInfo->ddCaps.dwSVBRops[2] = 0x00200010;
    pHalInfo->ddCaps.dwSVBRops[3] = 0x00800040;
    pHalInfo->ddCaps.dwSVBRops[4] = 0x02000100;
    pHalInfo->ddCaps.dwSVBRops[5] = 0x08000400;
    pHalInfo->ddCaps.dwSVBRops[6] = 0x20001000;
    pHalInfo->ddCaps.dwSVBRops[7] = 0x80004000;
  }
  else
  {
    // clear the system-to-video caps
    pHalInfo->ddCaps.dwCaps &= ~DDCAPS_CANBLTSYSMEM;
    pHalInfo->ddCaps.dwSVBCaps = 0;
    pHalInfo->ddCaps.dwSVBFXCaps = 0;
    pHalInfo->ddCaps.dwSVBCKeyCaps = 0;
  }
#endif
#endif

} // NTSpecificInit
#endif


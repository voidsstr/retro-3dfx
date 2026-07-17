/* $Header: ddinit.c, 31, 10/12/00 12:48:11 AM, Jonny Cochrane$ */
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
** $Revision: 31$
** $Date: 10/12/00 12:48:11 AM$
**
*/

/*******************************************************************************
*
* EXPORTED FUNCTIONS:
*
* DrvGetDirectDrawInfo      --- Initialize DirectDraw capabilities.
* DrvGetDirect3DInfo        --- Initialize Direct X 8 capabilities.
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
  *pdwNumFourCC                         = 0;
  *pdwNumHeaps                          = 0;

  // We may not support DirectDraw on this card
  if (!(ppdev->flStatus & STAT_DIRECTDRAW))
    return FALSE;
#endif

  /* Initialize DDHALINFO structure. */

#ifndef WINNT
  memset(pHalInfo, 0, sizeof(DDHALINFO));
#endif

  pHalInfo->dwSize                      = sizeof(DDHALINFO);

  pHalInfo->ddCaps.ddsCaps.dwCaps       =
                                          DDSCAPS_FLIP                      |
                                          DDSCAPS_OFFSCREENPLAIN            |
                                          DDSCAPS_BACKBUFFER                |
                                          DDSCAPS_COMPLEX                   |
                                          DDSCAPS_FRONTBUFFER               |
                                          DDSCAPS_VIDEOMEMORY               |
#if ENABLE_OVERLAY_CAPS
                                          DDSCAPS_OVERLAY                   |
#endif
#if ENABLE_PALETTE_CAPS
                                          DDSCAPS_PALETTE                   |
#endif
                                          DDSCAPS_PRIMARYSURFACE            |
#if ENABLE_3D
                                          DDSCAPS_TEXTURE                   |
                                          DDSCAPS_3DDEVICE                  |
                                          DDSCAPS_ZBUFFER                   |
                                          DDSCAPS_MIPMAP                    |
#if defined(WINNT) && defined(TnL_HAL) 
                                          DDSCAPS_EXECUTEBUFFER             |
#endif // WINNT && TnL_HAL && VERTBUF
#endif // ENABLE_3D
#if ENABLE_VIDEOPORT
#ifndef WINNT

                                          DDSCAPS_VIDEOPORT                 |
#endif
#endif
                                          0;
#ifndef WINNT
  pHalInfo->lpDDCallbacks               = &(_FF(DDCallbacks));
  pHalInfo->lpDDSurfaceCallbacks        = &(_FF(DDSurfaceCallbacks));
  pHalInfo->lpDDPaletteCallbacks        = &(_FF(DDPaletteCallbacks));

#if   defined(TnL_HAL) 
  if (_FF(DDExebufCallbacks.dwFlags))
  {
        pHalInfo->lpDDExeBufCallbacks   = &_FF(DDExebufCallbacks);
        pHalInfo->ddCaps.ddsCaps.dwCaps |=DDSCAPS_EXECUTEBUFFER;
  }
  else
        pHalInfo->lpDDExeBufCallbacks   = NULL;
#endif  

  pHalInfo->ddCaps.dwNumFourCCCodes     = 0;
  pHalInfo->lpdwFourCC                  = NULL;

  pHalInfo->hInstance                   = _FF(lpPDevice) >> 16;
  pHalInfo->lpPDevice                   = (LPVOID)_FF(lpPDevice);

  pHalInfo->dwFlags                     =
// Fixes Bug!                             DDHALINFO_MODEXILLEGAL            |
                                          DDHALINFO_GETDRIVERINFOSET        |
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
/* DX8 GetDriverInfo2 callback      */    DDHALINFO_GETDRIVERINFO2          |
#endif
                                          0;

  // @RBISSELL, We can't display ModeX modes on the TV in Win9x
  if (_FF(dwTvoActive))
     pHalInfo->dwFlags                  |=DDHALINFO_MODEXILLEGAL;


  pHalInfo->GetDriverInfo               = DdGetDriverInfo;
#endif

  pHalInfo->ddCaps.dwCaps               =
                                          DDCAPS_BLT                        |
//                                        DDCAPS_BLTQUEUE                   |
                                          DDCAPS_BLTFOURCC                  |
                                          DDCAPS_BLTSTRETCH                 |
#ifndef WINNT
/* Specifying DDCAPS_GDI on NT      */    DDCAPS_GDI                        |
/* causes NOHARDWARE support!       */
#endif
#if ENABLE_OVERLAY_CAPS
                                          DDCAPS_OVERLAY                    |
                                          DDCAPS_OVERLAYCANTCLIP            |
                                          DDCAPS_OVERLAYFOURCC              |
                                          DDCAPS_OVERLAYSTRETCH             |
#endif
#if ENABLE_PALETTE_CAPS
                                          DDCAPS_PALETTE                    |
#endif
                                          DDCAPS_READSCANLINE               |
                                          DDCAPS_COLORKEY                   |
                                          DDCAPS_COLORKEYHWASSIST           |
                                          DDCAPS_BLTCOLORFILL               |
//                                        DDCAPS_ALPHA                      |
//                                        DDCAPS_CANCLIP                    |
//                                        DDCAPS_CANCLIPSTRETCHED           |
#ifndef PERF_DISABLE_CANBLTSYSMEM
                                          DDCAPS_CANBLTSYSMEM               |
#endif

#if ENABLE_3D
                                          DDCAPS_3D                         |
                                          DDCAPS_ZBLTS                      |
                                          DDCAPS_BLTDEPTHFILL               |
#endif
                                          0;

  pHalInfo->ddCaps.dwSVBCaps            =
                                          DDCAPS_BLT                        |
//                                        DDCAPS_BLTQUEUE                   |
//                                        DDCAPS_BLTFOURCC                  |
                                          DDCAPS_BLTSTRETCH                 |
#ifndef WINNT
/* Specifying DDCAPS_GDI on NT      */    DDCAPS_GDI                        |
/* causes NOHARDWARE support!       */
#endif
                                          DDCAPS_COLORKEY                   |
                                          DDCAPS_COLORKEYHWASSIST           |
                                          DDCAPS_BLTCOLORFILL               |
//                                        DDCAPS_ALPHA                      |
//                                        DDCAPS_CANCLIP                    |
//                                        DDCAPS_CANCLIPSTRETCHED           |
#ifndef PERF_DISABLE_CANBLTSYSMEM
                                          DDCAPS_CANBLTSYSMEM               |
#endif

#if ENABLE_3D
                                          DDCAPS_3D                         |
                                          DDCAPS_ZBLTS                      |
                                          DDCAPS_BLTDEPTHFILL               |
#endif
                                          0;

#ifndef WINNT
  pHalInfo->ddCaps.dwCaps2              = 
/* Important optimization. Banshee  */    DDCAPS2_NOPAGELOCKREQUIRED        |
/* does not need memory page locked */
/* down by direct draw.             */

/* Support flip with ODD/EVEN flags */    DDCAPS2_CANFLIPODDEVEN            |
/* Allow allocation of surfaces     */    DDCAPS2_WIDESURFACES              |
/* wider than primary.              */
#ifdef STBPERF_USE_FLIPNOVSYNC
/* Support flip without VSYNC       */    DDCAPS2_FLIPNOVSYNC               |
#endif

#ifdef STEREO
/* jcochrane - caps for alternating */    (_FF(ddStereoWrapperLoaded) ? DDCAPS2_STEREO : 0) |
/* field stereo glasses support     */
#endif

#if ENABLE_VIDEOPORT
                                          DDCAPS2_VIDEOPORT                 |
                                          DDCAPS2_AUTOFLIPOVERLAY           |
/* Disabling caused C-CUBE weave fail */  DDCAPS2_CANBOBINTERLEAVED         |   
                                          DDCAPS2_CANBOBNONINTERLEAVED      |
                                          DDCAPS2_CANBOBHARDWARE            |
//                                        DDCAPS2_COLORCONTROLOVERLAY       |
#endif
                                          DDCAPS2_COPYFOURCC                |
                                          0;

#ifdef STEREO

pHalInfo->ddCaps.dwSVCaps = (_FF(ddStereoWrapperLoaded) ? DDSVCAPS_STEREOSEQUENTIAL:0);



#endif


#endif

  pHalInfo->ddCaps.dwFXCaps             =
                                          DDFXCAPS_BLTSHRINKX               |
                                          DDFXCAPS_BLTSHRINKY               |
                                          DDFXCAPS_BLTSTRETCHX              |
                                          DDFXCAPS_BLTSTRETCHY              |
//                                        DDFXCAPS_BLTARITHSTRETCHY         |
//                                        DDFXCAPS_BLTARITHSTRETCHYN        |
//                                        DDFXCAPS_BLTSHRINKXN              |
//                                        DDFXCAPS_BLTSHRINKYN              |
//                                        DDFXCAPS_BLTSTRETCHXN             |
//                                        DDFXCAPS_BLTSTRETCHYN             |
#if ENABLE_OVERLAY_CAPS
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
                                          DDFXCAPS_OVERLAYSHRINKX           |   // VPE requires to shrink video window!
                                          DDFXCAPS_OVERLAYSHRINKY           |   // VPE requires to shrink video window!
#endif
                                          DDFXCAPS_OVERLAYSTRETCHX          |
                                          DDFXCAPS_OVERLAYSTRETCHY          |
//                                        DDFXCAPS_OVERLAYARITHSTRETCHY     |
//                                        DDFXCAPS_OVERLAYARITHSTRETCHYN    |
//                                        DDFXCAPS_OVERLAYSHRINKXN          |
//                                        DDFXCAPS_OVERLAYSHRINKYN          |
//                                        DDFXCAPS_OVERLAYSTRETCHXN         |
//                                        DDFXCAPS_OVERLAYSTRETCHYN         |
#endif
                                          0;

  pHalInfo->ddCaps.dwSVBFXCaps          =
                                          DDFXCAPS_BLTSTRETCHX              |
                                          DDFXCAPS_BLTSTRETCHY              |
//                                        DDFXCAPS_BLTARITHSTRETCHY         |
//                                        DDFXCAPS_BLTARITHSTRETCHYN        |
//                                        DDFXCAPS_BLTSHRINKX               |
//                                        DDFXCAPS_BLTSHRINKY               |
//                                        DDFXCAPS_BLTSHRINKXN              |
//                                        DDFXCAPS_BLTSHRINKYN              |
//                                        DDFXCAPS_BLTSTRETCHXN             |
//                                        DDFXCAPS_BLTSTRETCHYN             |
                                          0;

#if ENABLE_PALETTE_CAPS
  pHalInfo->ddCaps.dwPalCaps            =
                                          DDPCAPS_8BIT                      |
                                          DDPCAPS_ALLOW256                  |
                                          0;
#endif

  pHalInfo->ddCaps.dwCKeyCaps           =
                                          DDCKEYCAPS_SRCBLT                 |
                                          DDCKEYCAPS_SRCBLTCLRSPACE         |

#if defined(WINNT) && (_WIN32_WINNT == 0x0400)
// NT bug, NT doesn't report dest colorkey values to DdBlt correctly
#else
                                          DDCKEYCAPS_DESTBLT                |
                                          DDCKEYCAPS_DESTBLTCLRSPACE        |
#endif

#if ENABLE_OVERLAY_CAPS
                                          DDCKEYCAPS_DESTOVERLAY            |
/* RGB only */                            DDCKEYCAPS_DESTOVERLAYCLRSPACE    |   
                                          DDCKEYCAPS_DESTOVERLAYONEACTIVE   |

//                                        DDCKEYCAPS_DESTOVERLAYCLRSPACE    |
//                                        DDCKEYCAPS_SRCOVERLAY             |
//                                        DDCKEYCAPS_SRCOVERLAYCLRSPACE     |
#endif
                                          0;

  pHalInfo->ddCaps.dwSVBCKeyCaps        =

#if defined(WINNT) && (_WIN32_WINNT == 0x0400)
// NT bug, NT doesn't report dest colorkey values to DdBlt correctly
#else
                                          DDCKEYCAPS_DESTBLT                |
                                          DDCKEYCAPS_DESTBLTCLRSPACE        |
#endif
                                          DDCKEYCAPS_SRCBLT                 |
                                          DDCKEYCAPS_SRCBLTCLRSPACE         |
                                          0;

#if (_WIN32_WINNT >= 0x0500) || !defined WINNT
#if ENABLE_VIDEOPORT
  pHalInfo->ddCaps.dwMaxVideoPorts      = 1;    //maximum number of usable video ports
  pHalInfo->ddCaps.dwCurrVideoPorts     = 0;    //current number of video ports used
#else
  pHalInfo->ddCaps.dwMaxVideoPorts      = 0;    //maximum number of usable video ports
  pHalInfo->ddCaps.dwCurrVideoPorts     = 0;    //current number of video ports used
#endif
#endif

#if ENABLE_OVERLAY_CAPS
  pHalInfo->ddCaps.dwMaxVisibleOverlays = 1;
  pHalInfo->ddCaps.dwCurrVisibleOverlays= 0;

//***VPE requires this MinOverlayStretch to be set to 100 to be able to shrink the video window properly***
//---------------------------------------------------------------------------------------------------------
//pHalInfo->ddCaps.dwMinOverlayStretch    = 100;      // min = 1/10:1 ?
//---------------------------------------------------------------------------------------------------------

#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
#ifdef  STB_DISABLEHWDOWNSCALE
  //LPost remove this when hw downscaling works.
  //This is for the Dell Demo
  pHalInfo->ddCaps.dwMinOverlayStretch  = 1000;     // min = 1:1
#else
  pHalInfo->ddCaps.dwMinOverlayStretch  = 100;      // min = 1:10
#endif

#else
  pHalInfo->ddCaps.dwMinOverlayStretch  = 1000;     // max = 1:1
#endif
  pHalInfo->ddCaps.dwMaxOverlayStretch  = 10000;    // max = 10:1 ?
  pHalInfo->vmiData.dwOverlayAlign      = 64;       // quadword alignment?
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
  pHalInfo->ddCaps.dwRops[0]            = 0x00020001;   // 0    & DSon
  pHalInfo->ddCaps.dwRops[1]            = 0x00080004;   // DSna & Sn
  pHalInfo->ddCaps.dwRops[2]            = 0x00200010;   // SDna & Dn
  pHalInfo->ddCaps.dwRops[3]            = 0x00800040;   // DSx  & DSan
  pHalInfo->ddCaps.dwRops[4]            = 0x02000100;   // DSa  & DSxn
  pHalInfo->ddCaps.dwRops[5]            = 0x08000400;   // D    & DSno
  pHalInfo->ddCaps.dwRops[6]            = 0x20001000;   // S    & SDno
  pHalInfo->ddCaps.dwRops[7]            = 0x80004000;   // DSo  & 1

  pHalInfo->ddCaps.dwSVBRops[0]         = 0x00020001;
  pHalInfo->ddCaps.dwSVBRops[1]         = 0x00080004;
  pHalInfo->ddCaps.dwSVBRops[2]         = 0x00200010;
  pHalInfo->ddCaps.dwSVBRops[3]         = 0x00800040;
  pHalInfo->ddCaps.dwSVBRops[4]         = 0x02000100;
  pHalInfo->ddCaps.dwSVBRops[5]         = 0x08000400;
  pHalInfo->ddCaps.dwSVBRops[6]         = 0x20001000;
  pHalInfo->ddCaps.dwSVBRops[7]         = 0x80004000;
#endif

  ppdev->flip_pending                   = 0;

#if ENABLE_3D
  pHalInfo->ddCaps.dwZBufferBitDepths   = DDBD_16;
  if (IS_NAPALM)
  {
    pHalInfo->ddCaps.dwZBufferBitDepths |=DDBD_24;
  }

#endif

#ifndef WINNT
  // 16bit driver will fill this out
  pHalInfo->dwModeIndex = DDUNSUPPORTEDMODE;
#endif

  if (! fxinit(ppdev, FALSE))
    return FALSE;

#if ENABLE_3D
#if   defined(TnL_HAL) 
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
    _FF(dwRelaxedOverlayOwnerMode)      = 0;
#endif

  return TRUE;

} // DrvGetDirectDrawInfo


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
  pCallBacks->dwSize                    = sizeof(DDHAL_DDCALLBACKS);
#endif

  pCallBacks->dwFlags                   =
                                          DDHAL_CB32_CREATESURFACE          |
                                          DDHAL_CB32_WAITFORVERTICALBLANK   |
                                          DDHAL_CB32_CANCREATESURFACE       |
#if ENABLE_PALETTE_CAPS
                                          DDHAL_CB32_CREATEPALETTE          |
#endif
                                          DDHAL_CB32_GETSCANLINE            |
#ifdef WINNT
                                          DDHAL_CB32_MAPMEMORY              |
#else
                                          DDHAL_CB32_SETEXCLUSIVEMODE       |
                                          DDHAL_CB32_FLIPTOGDISURFACE       |
#endif
                                          0;
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && 0
  pCallBacks->SetColorKey               = DdSetDrvColorKey;
  pCallBacks->dwFlags                   |=DDHAL_CB32_SETCOLORKEY;
#endif
#ifndef WINNT
//This member is not used and obsolete under DX-6
#if (DX < 6)
  pCallBacks->SetColorKey               = DdSetDrvColorKey;
  pCallBacks->dwFlags                   |=DDHAL_CB32_SETCOLORKEY;
#endif
#endif

  pCallBacks->CreateSurface             = DdCreateSurface;
  pCallBacks->WaitForVerticalBlank      = DdWaitForVerticalBlank;
  pCallBacks->CanCreateSurface          = DdCanCreateSurface;
#if ENABLE_PALETTE_CAPS
  pCallBacks->CreatePalette             = DdCreatePalette;
#endif
  pCallBacks->GetScanLine               = DdGetScanLine;

#ifdef WINNT
  pCallBacks->MapMemory                 = DdMapMemory;
#else
  pCallBacks->SetExclusiveMode          = DdSetExclusiveMode;
  pCallBacks->FlipToGDISurface          = DdFlipToGDISurface;
#endif


  // fill in DirectDrawSurface object callbacks
#ifndef WINNT
  memset(pSurfaceCallBacks, 0, sizeof(DDHAL_DDSURFACECALLBACKS));
  pSurfaceCallBacks->dwSize             = sizeof(DDHAL_DDSURFACECALLBACKS);
#endif

  pSurfaceCallBacks->dwFlags            =
                                          DDHAL_SURFCB32_DESTROYSURFACE     |
                                          DDHAL_SURFCB32_FLIP               |
                                          DDHAL_SURFCB32_LOCK               |
                                          DDHAL_SURFCB32_UNLOCK             |
                                          DDHAL_SURFCB32_BLT                |
#if ENABLE_OVERLAY_CAPS
                                          DDHAL_SURFCB32_SETCOLORKEY        |
#endif
                                          DDHAL_SURFCB32_ADDATTACHEDSURFACE |
                                          DDHAL_SURFCB32_GETBLTSTATUS       |
                                          DDHAL_SURFCB32_GETFLIPSTATUS      |
#if ENABLE_OVERLAY_CAPS
                                          DDHAL_SURFCB32_UPDATEOVERLAY      |
                                          DDHAL_SURFCB32_SETOVERLAYPOSITION |
#endif
#if ENABLE_PALETTE_CAPS
                                          DDHAL_SURFCB32_SETPALETTE         |
#endif
                                          0;

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
  pPaletteCallBacks->dwSize             = sizeof(DDHAL_DDPALETTECALLBACKS);
#endif

#if ENABLE_PALETTE_CAPS
  pPaletteCallBacks->dwFlags            =
                                          DDHAL_PALCB32_DESTROYPALETTE      |
                                          DDHAL_PALCB32_SETENTRIES          |
                                          0;

  pPaletteCallBacks->DestroyPalette     = DdDestroyPalette;
  pPaletteCallBacks->SetEntries         = DdSetEntries;
#endif

#ifdef WINNT
#if !USE_NT5_DDMEMMGR
  // convert DDraw memory allocation from reserved to permanent
  bOhCommit(ppdev, ppdev->pohDirectDraw, TRUE);
#endif
#endif

  /* Clear out the HotKey_Data array, without this overlay's displayed after  */	
  /* the Screenshot has been enabled will be horribly slow because the        */	
  /* capture routine will fire..                                              */	
  /* This is done before we do an Enter_3DApplication because there is where  */	
  /* the array is filled.                                                     */	
  {
  extern HOTKEY_DATA  HotKey_Data[Number_HotKeys] ;
  int counter ;
  for (counter = 0 ; counter < Number_HotKeys ; counter ++)
     HotKey_Data[counter].pHotKeyData = NULL ;
  }

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

#ifndef WINNT
#ifdef WIN_CSIM
  _DD(sst3DRegs) = (SstRegs*)(_FF(regFakeBase) + SST_3D_OFFSET);
#else
  _DD(sst3DRegs) = (SstRegs*)(_FF(regBase[HWINFO_SST_3DREGS_INDEX]));
#endif

#endif
  _DD(overlaySurfaceCnt)                = 0;
  _DD(fxCaps)                           |=FXCAPS_PALETTIZED_TEXTURES;

#ifdef WINNT
  width  = _DD(fbiWidth)                = ppdev->cxScreen;
  height = _DD(fbiHeight)               = ppdev->cyScreen;
#else
  width  = _DD(fbiWidth)                = _FF(HALInfo).vmiData.dwDisplayWidth;
  height = _DD(fbiHeight)               = _FF(HALInfo).vmiData.dwDisplayHeight;
#endif

  _DD(bufferUsage)                      = 0;
  _FF(ddVisibleOverlaySurf)             = 0;

#ifdef DEBUG
  // System Environment
  if (NULL == GETENV(DDDEBUGLEVEL))
    _DD(DD_DebugLevel)                  = 0;
  else
    _DD(DD_DebugLevel)                  = atoi(GETENV(DDDEBUGLEVEL));
#endif  // DEBUG

  if (NULL == GETENV(SWAPINTERVAL))
    _DD(WaitOnVsync)                    = 1;
  else
    _DD(WaitOnVsync)                    = atoi(GETENV(SWAPINTERVAL));

#if defined(SLI_AA) && (!defined(WINNT) || (_WIN32_WINNT >= 0x0500))

  // Initialize SLI and AA variables.

  _DD(ddSLIAAConfiguration)             = 0;
  _DD(ddSLIAAAnalog)                    = 0;
  _DD(ddAAModeEnabled)                  = 0;
  _DD(ddAAModeRequested)                = 0;
  _DD(ddAAPrimaryStart)                 = 0;
  _DD(ddAAZbufferStart)                 = 0;
  _DD(ddAANumberSamples)                = 0;
  _DD(ddSLIModeEnabled)                 = 0;
  _DD(ddSLIModeRequested)               = 0;
  _DD(ddSLINumberScanlines)             = 0;
  _DD(ddSLINumberWays)                  = 0;

#endif // SLI_AA

  _DD(dd3DInOverlay)                    = 0;

  // Initialize surface flipping variables.

  _DD(ddSurfaceFlippedFrom)             = 0;
  _DD(ddSurfaceFlippedTo)               = 0;
  _DD(ddAcceleratorUsed)                = 0;

  if (NULL == GETENV(OVERLAYMODE))
    _DD(overlayFilter)                  = 0;
  else
    _DD(overlayFilter)                  = atoi(GETENV(OVERLAYMODE));

  // Reusing overlayfilter to now hold both overlay filter options as well as Alphadithermode
  if (NULL != GETENV(ALPHADITHERMODE))
    _DD(overlayFilter)                  |=(atoi(GETENV(ALPHADITHERMODE))<<2);

  if (_FF(dwRBitMask) == 0xF800)
    _DD(fbiColorModel)                  = FX_COLORMODEL_RGB;
  else
    _DD(fbiColorModel)                  = FX_COLORMODEL_BGR;

#if ENABLE_3D

  // This code should be code inspected in conjunction with the
  // Direct3D team.  It appears to be mostly used for texture
  // allocation on Voodoo2.  I already removed some of the most
  // obvious obsolete code and unreferenced variables, but the
  // cleanup is not complete. -CGW-

  // only for tmu 0
  _DD(startTREXMem)                     = SST_TEX_ADDRESS(ghw0);
  _DD(totalSizeTREXMem)                 = 0;    //info.tmuMemSize[0] << 20;


  // Initially no locks outstanding
  _FX(openLockCount)                    = 0;
  _FX(openLockIndex)                    = 0;
  // 16 bit driver needs this to initialize the heaps
  _FX(numTmus)                          = 1;    // banshee only has one
  for (numberTmus=0; numberTmus < _FX(numTmus); ++numberTmus)

  if (_FX(numTmus) == 2)
    _FX(flags)                          = TRILINEAR_SPLIT2TMUS;
  else
    _FX(flags)                          = 0;

  {
#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
    DWORD baseAddress                   = 0;
#else
    DWORD baseAddress                   = _DS(LFBBASE);
#endif
    _FX(numTextureHeaps)                = 0;

    // Banshee currently only supports one TMU
    for (numberTmus=0; numberTmus < _FX(numTmus); ++numberTmus)
    {
      ++_FX(numTextureHeaps);
      _FX(textureHeapStart[numberTmus]) = baseAddress;
      _FX(textureHeapLength[numberTmus])= 0x200000;     // WRONG - need to fix
    }
  } // tmu init data
#endif

#ifdef Z_ACCESS_OPT
  // Default the Z Clear Optimization as on.
  if (NULL == GETENV(Z_CLEAR_OPTIMIZATION))
    _DD(ddEnableZClearOpt)              = 1;
  else
    _DD(ddEnableZClearOpt)              = atoi(GETENV(Z_CLEAR_OPTIMIZATION));

  _DD(ddFlipsWithoutZClear)             = 0;
#endif

  return FXTRUE;

} // fxinit

/**************************************************************************
* S T A T I C   F U N C T I O N S
***************************************************************************/

#ifdef WINNT

/*----------------------------------------------------------------------
Function name:  ReportNTDDrawHeaps

Description:    Initialize Windows NT ddraw heap information.

                NOTE: This initialization is performed on W9X by
                16-bit display driver code instead.

Return:         NONE
----------------------------------------------------------------------*/

#if USE_NT5_DDMEMMGR

#define DDRAWHEAP_DBGLVL  0

static VOID
ReportNTDDrawHeaps(PDEV *ppdev, DWORD *pdwNumHeaps, VIDEOMEMORY *pvmList)
{
  DWORD     cHeaps;


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
      DWORD   heapStart;

      heapStart  = _FF(ddSecondaryHeapStart);

      // heap for aa third buffer
      // even alignment
      pvmList[LINEAR_HEAP4_ID].dwFlags           = VIDMEM_ISLINEAR;
      pvmList[LINEAR_HEAP4_ID].fpStart           = heapStart;
      _DS(ddLinearHeap4Start)                    = heapStart;
      pvmList[LINEAR_HEAP4_ID].fpEnd             = pvmList[LINEAR_HEAP4_ID].fpStart + _FF(gdiDesktopSize) - 1;
      pvmList[LINEAR_HEAP4_ID].ddsCaps.dwCaps    = 0;
      pvmList[LINEAR_HEAP4_ID].ddsCapsAlt.dwCaps = 0;

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh",
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

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh",
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

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh",
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

      DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw linear heap reported as fpStart=%8lXh, fpEnd=%8lXh",
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
      DWORD   i;
      DWORD   heapStart;

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

          DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw tiled heap reported as fpStart=%8lXh, dwWidth=%8lXh, dwHeight=%lXh, hwStart=%8lXh",
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

          DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw tiled heap reported as fpStart=%8lXh, dwWidth=%8lXh, dwHeight=%lXh, hwStart=%8lXh",
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

          DISPDBG((DDRAWHEAP_DBGLVL, "DirectDraw tiled heap reported as fpStart=%8lXh, dwWidth=%8lXh, dwHeight=%lXh, hwStart=%8lXh",
                   pvmList[TILED_HEAP0_ID].fpStart, pvmList[TILED_HEAP0_ID].dwWidth, pvmList[TILED_HEAP0_ID].dwHeight, heapStart));
      }

      // Compute addresses of secondary buffers for SLI mode.
      if ((IS_NAPALM) && (_FF(dwNumUnits) > 1))
      {
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
  // Current primary surface attributes.  Since HalInfo is zero-initialized
  // by GDI, we only have to fill in the fields which should be non-zero:

  // If we use command fifo we may need to set pHalInfo->vmiData.fpPrimary

#if ENABLE_RECONFIG_VIDMEM
  pHalInfo->vmiData.fpPrimary           = ppdev->pjScreen - ppdev->pjScreenBase;
#else
  pHalInfo->vmiData.fpPrimary           = ppdev->ulScreenOffset;
#endif
  pHalInfo->vmiData.pvPrimary           = ppdev->pjScreen;
  pHalInfo->vmiData.dwDisplayWidth      = ppdev->cxScreen;
  pHalInfo->vmiData.dwDisplayHeight     = ppdev->cyScreen;
  pHalInfo->vmiData.lDisplayPitch       = ppdev->lDelta;

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

  pHalInfo->vmiData.dwOffscreenAlign    = 4;

#if ENABLE_3D
  pHalInfo->vmiData.dwZBufferAlign      = 16;
  pHalInfo->vmiData.dwTextureAlign      = 16;
#endif

#if (_WIN32_WINNT >= 0x0500)
  pHalInfo->ddCaps.dwCaps2              =
                                          DDCAPS2_COPYFOURCC                |
/* Support flip with ODD/EVEN flags */    DDCAPS2_CANFLIPODDEVEN            |
/* Allow allocation of surfaces     */    DDCAPS2_WIDESURFACES              |
/* wider than primary.              */
#if ENABLE_VIDEOPORT
                                          DDCAPS2_VIDEOPORT                 |
                                          DDCAPS2_AUTOFLIPOVERLAY           |
/* Disabling caused C-CUBE          */    DDCAPS2_CANBOBINTERLEAVED         |
/* weave to fail                    */
                                          DDCAPS2_CANBOBNONINTERLEAVED      |
                                          DDCAPS2_CANBOBHARDWARE            |
#endif
#ifdef STBPERF_USE_FLIPNOVSYNC
/* Support flip without VSYNC       */    DDCAPS2_FLIPNOVSYNC               |
#endif
                                          0;

  pHalInfo->dwFlags                     =
                                          DDHALINFO_GETDRIVERINFOSET        |
                                          0;
  pHalInfo->GetDriverInfo               = DdGetDriverInfo;
#endif

  if (ppdev->iBitmapFormat == BMF_8BPP)
  {
    // banshee doesn't support fourcc blts in 8bpp modes
    pHalInfo->ddCaps.dwCaps             &=~DDCAPS_BLTFOURCC;
#if ENABLE_OVERLAY_CAPS
    *pdwNumFourCC                       = 2;
    if (NULL != pdwFourCC)
    {
      pdwFourCC[0]                      = FOURCC_YUY2;
      pdwFourCC[1]                      = FOURCC_UYVY;
    }
#endif
  }
  else
  {
    *pdwNumFourCC                       = 2;
    if (NULL != pdwFourCC)
    {
      pdwFourCC[0]                      = FOURCC_YUY2;
      pdwFourCC[1]                      = FOURCC_UYVY;
    }
  }

#if (_WIN32_WINNT < 0x0500)
// NT bug, NT doesn't report rops to DdBlt correctly
// it always says the rop is SRCCOPY
  // support all 16 two operand rops
  pHalInfo->ddCaps.dwRops[0]            = 0;
  pHalInfo->ddCaps.dwRops[1]            = 0;
  pHalInfo->ddCaps.dwRops[2]            = 0;
  pHalInfo->ddCaps.dwRops[3]            = 0;
  pHalInfo->ddCaps.dwRops[4]            = 0;
  pHalInfo->ddCaps.dwRops[5]            = 0;
  pHalInfo->ddCaps.dwRops[6]            = 0x00001000;   // S    & SDno
  pHalInfo->ddCaps.dwRops[7]            = 0;

  pHalInfo->ddCaps.dwSVBRops[0]         = 0;
  pHalInfo->ddCaps.dwSVBRops[1]         = 0;
  pHalInfo->ddCaps.dwSVBRops[2]         = 0;
  pHalInfo->ddCaps.dwSVBRops[3]         = 0;
  pHalInfo->ddCaps.dwSVBRops[4]         = 0;
  pHalInfo->ddCaps.dwSVBRops[5]         = 0;
  pHalInfo->ddCaps.dwSVBRops[6]         = 0x00001000;
  pHalInfo->ddCaps.dwSVBRops[7]         = 0;
#else
  // support all 16 two operand rops
  pHalInfo->ddCaps.dwRops[0]            = 0x00020001;   // 0    & DSon
  pHalInfo->ddCaps.dwRops[1]            = 0x00080004;   // DSna & Sn
  pHalInfo->ddCaps.dwRops[2]            = 0x00200010;   // SDna & Dn
  pHalInfo->ddCaps.dwRops[3]            = 0x00800040;   // DSx  & DSan
  pHalInfo->ddCaps.dwRops[4]            = 0x02000100;   // DSa  & DSxn
  pHalInfo->ddCaps.dwRops[5]            = 0x08000400;   // D    & DSno
  pHalInfo->ddCaps.dwRops[6]            = 0x20001000;   // S    & SDno
  pHalInfo->ddCaps.dwRops[7]            = 0x80004000;   // DSo  & 1

  pHalInfo->ddCaps.dwSVBRops[0]         = 0x00020001;
  pHalInfo->ddCaps.dwSVBRops[1]         = 0x00080004;
  pHalInfo->ddCaps.dwSVBRops[2]         = 0x00200010;
  pHalInfo->ddCaps.dwSVBRops[3]         = 0x00800040;
  pHalInfo->ddCaps.dwSVBRops[4]         = 0x02000100;
  pHalInfo->ddCaps.dwSVBRops[5]         = 0x08000400;
  pHalInfo->ddCaps.dwSVBRops[6]         = 0x20001000;
  pHalInfo->ddCaps.dwSVBRops[7]         = 0x80004000;
#endif

} // NTSpecificInit
#endif


/* $Header: ddinit32.c, 18, 10/19/00 2:00:30 AM, Jonny Cochrane$ */
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
** File Name:  ddinit32.c
**
** Description: Direct Draw GetDriverInfo support.
**
** $Revision: 18$
** $Date: 10/19/00 2:00:30 AM$
**
*/

/*******************************************************************************
*
* DIRECTDRAW FUNCTIONS:
*
* DdGetDriverInfo               --- Returns extended callbacks and capabilities.
* DdGetAvailDriverMemory        --- Returns driver managed memory.
*
* EXPORTED FUNCTIONS:
*
* ddgetenv                      --- Get environment variable from registry.
*
* INTERNAL FUNCTIONS:
*
* AhNuts_GetTheDevNodeKeyMyself --- Generate the DevNodeKey.
*
*******************************************************************************/


#include "precomp.h"
#include "ddglobal.h"
#include "ddinit.h"

// Include for new DX 8 macros
#if( DIRECTDRAW_VERSION >= 0x0800 )
#include <d3dhal.h>
#include <d3dhalex.h>
#endif

#include "ddkernel.h" // for guid
#include "header.h"
#include "fxglobal.h"
#include "runtime.h"
#include "ddvpe32.h"
#define Not_VxD
#include <vmm.h>
#include <configmg.h>

#include "VMIPLD.h"

/*
 *
 * NOTE:  All routines are called with the Win16 lock taken.   This is
 * to prevent anyone from calling the display driver and doing something
 * that could confict with this 32-bit driver.
 *
 * This means that all shared 16-32 memory is safe to use at any time inside
 * either driver.
 */

extern void _stdcall Connect( LPVOID );

/*
 * shared memory area!   16-bit display driver gets this by calling GetDataPtr
 * in the 16-bit DLL
 */

extern HINSTANCE        hInstance;

void BuildD3DCaps(NT9XDEVICEDATA *ppdev, D3DDEVICEDESC_V1 *pCaps);
extern void BuildD3DExtendedCaps(NT9XDEVICEDATA *ppdev, D3DHAL_D3DEXTENDEDCAPS *pCaps);

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
extern void BuildD3DCaps8(NT9XDEVICEDATA *ppdev, D3DCAPS8 *pCaps);
#endif


/*
 * Global variables
 */

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
extern DWORD g_nDX8TextureFormats;
extern DDSURFACEDESC g_DX8TextureFormats[];
#endif
/*----------------------------------------------------------------------
Function name: VRwriteSubID

Description:   write the subdevice id so the VR app can know the board type

Return:        VOID
----------------------------------------------------------------------*/
void VRwriteSubID(DWORD dwSubSystemID)
{
   HKEY hcpl;
   // Try creating/opening the registry key
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\3dfx Interactive\\VisualReality", 0, KEY_WRITE, &hcpl) != ERROR_SUCCESS )
   {
      DWORD dwDisp;
      if (RegCreateKeyEx ( HKEY_LOCAL_MACHINE, "SOFTWARE\\3dfx Interactive\\VisualReality", 0, "",
            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hcpl, &dwDisp ) != ERROR_SUCCESS)
      {
         return;
      }
   }
   RegSetValueEx (hcpl, "V3TV_ID", 0, REG_DWORD, (BYTE*)&dwSubSystemID, sizeof(dwSubSystemID));
   RegCloseKey (hcpl);
}

#ifndef WINNT

/*----------------------------------------------------------------------
Function name:  DdGetAvailDriverMemory

Description:    Reports total and free memory privatley managed by driver.

Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED    -
				DDHAL_DRIVER_NOTHANDLED	-
----------------------------------------------------------------------*/
DWORD __stdcall DdGetAvailDriverMemory(LPDDHAL_GETAVAILDRIVERMEMORYDATA lpData)
{
  DD_ENTRY_SETUP(lpData->lpDD);

  // Napalm/Voodoo3 display drivers do not manage memory.

  lpData->dwTotal = 0;
  lpData->dwFree  = 0;

  if (IS_NAPALM)
  {
    // Subtract extra linear memory unless it is available.

    if (!(_DD(ddSLIModeRequested) && !_DD(ddAAModeRequested)))
    {
      lpData->dwTotal = (unsigned long) -((long)_FF(ddExtraMemorySize));
      lpData->dwFree  = (unsigned long) -((long)_FF(ddExtraMemorySize));
    }
  }

  lpData->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // DdGetAvailDriverMemory

#endif //WINNT

/*----------------------------------------------------------------------
Function name: DdGetDriverInfo

Description:   Initialize Direct Draw callbacks and data

Return:        DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall DdGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpInput)
{
    NT9XDEVICEDATA * ppdev = (NT9XDEVICEDATA * )lpInput->dwContext;
    DWORD dwSize;
    extern DWORD __stdcall ddiDrawOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA);
    extern DWORD __stdcall ddiDrawOneIndexedPrimitive(LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA);
    extern DWORD __stdcall ddiDrawPrimitives(LPD3DHAL_DRAWPRIMITIVESDATA);
    extern DWORD __stdcall ddiClearSD(LPD3DHAL_CLEARDATA pcd);
    extern DWORD __stdcall ddiClearSG(LPD3DHAL_CLEARDATA pcd);
    extern DWORD __stdcall ddiClear2SD(LPD3DHAL_CLEAR2DATA pcd);
    extern DWORD __stdcall ddiClear2SG(LPD3DHAL_CLEAR2DATA pcd);

#if ENABLE_VIDEOPORT
    extern DWORD __stdcall vpCanCreateVideoPort32(LPDDHAL_CANCREATEVPORTDATA);
    extern DWORD __stdcall vpCreateVideoPort32(LPDDHAL_CREATEVPORTDATA);
    extern DWORD __stdcall vpFlipVideoPort32(LPDDHAL_FLIPVPORTDATA);
    extern DWORD __stdcall vpGetVideoPortBandwidth32(LPDDHAL_GETVPORTBANDWIDTHDATA);
    extern DWORD __stdcall vpGetVideoPortInputFormats32(LPDDHAL_GETVPORTINPUTFORMATDATA);
    extern DWORD __stdcall vpGetVideoPortOutputFormats32(LPDDHAL_GETVPORTOUTPUTFORMATDATA);
    extern DWORD __stdcall vpGetVideoPortField32(LPDDHAL_GETVPORTFIELDDATA);
    extern DWORD __stdcall vpGetVideoPortLine32(LPDDHAL_GETVPORTLINEDATA);
    extern DWORD __stdcall vpGetVideoPortConnectInfo32(LPDDHAL_GETVPORTCONNECTDATA);
    extern DWORD __stdcall vpDestroyVideoPort32(LPDDHAL_DESTROYVPORTDATA);
    extern DWORD __stdcall vpGetVideoPortFlipstatus32(LPDDHAL_GETVPORTFLIPSTATUSDATA);
    extern DWORD __stdcall vpUpdateVideoPort32(LPDDHAL_UPDATEVPORTDATA);
    extern DWORD __stdcall vpWaitForVideoPortSync32(LPDDHAL_WAITFORVPORTSYNCDATA);
    extern DWORD __stdcall vpGetVideoSignalStatus32(LPDDHAL_GETVPORTSIGNALDATA);
    extern DWORD __stdcall vpColorControl32(LPDDHAL_VPORTCOLORDATA);
    extern DWORD __stdcall ovlColorControl32(LPDDHAL_COLORCONTROLDATA);
    extern DWORD __stdcall kSyncSurfaceData32(LPDDHAL_SYNCSURFACEDATA);
    extern DWORD __stdcall kSyncVideoportData32(LPDDHAL_SYNCVIDEOPORTDATA);
#endif

#define CHECKSIZE(x) ((void)0)

#if ENABLE_VIDEOPORT
    if (IsEqualIID(&lpInput->guidInfo, &GUID_VideoPortCallbacks))
    {
        DDHAL_DDVIDEOPORTCALLBACKS vpCallbacks;
        memset(&vpCallbacks, 0, sizeof(vpCallbacks));

   // use this as a trigger to query and set initial state of pld
#ifndef WINNT
#define MAXSTRLEN_V3TV 32
      {  //V3TV VMIPLD support
         DWORD val = VMIPLD_MODE_USE_GUID;
         _DD(bVideoPortActive)              = FALSE;
         _DD(bUseWDMScaling)                = TRUE;
         _DD(bIgnoreWDM)                    = FALSE;
         _DD(bVMIPLDinUse)                  = FALSE; 
         _DD(bVMIPLDpresent)                = FALSE;
         _DD(dwVMIPLDmode)                  = val;
#ifdef WINNT
//       DD_PLD656_Init(ppdev);
//       _DD(bVMIPLDpresent)                = DD_VMI_Available(ppdev);
#else
         _DD(bVMIPLDpresent)                = _FF(PLDRevisionID);
#endif
         if (_DD(bVMIPLDpresent))
         {

            if (val == VMIPLD_MODE_PASSTHRU)
            {
               DD_VMI_SetCommand( ppdev, VMI_PASSTHRU); // passthru, no vbi int, no crop vbi
               _DD(bVMIPLDinUse)            = FALSE;
            }
            else  // (val == VMIPLD_MODE_656)
            {
               DD_VMI_SetCommand( ppdev, VMI_VBICROP); // not passthru, no vbi int, crop vbi
               _DD(bVMIPLDinUse)            = TRUE;
            }
            DD_VMI_SetVbiMax(ppdev, 0x1f);
            DD_VMI_SetVidMax(ppdev, 0x3ff);
         }
         VRwriteSubID(_FF(SSID));
      }
#endif

        dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDVIDEOPORTCALLBACKS));
        lpInput->dwActualSize               = sizeof(DDHAL_DDVIDEOPORTCALLBACKS);
        vpCallbacks.dwSize                  = dwSize;

        vpCallbacks.dwFlags                 = 0 /* Video port callback flags */|
                                              DDHAL_VPORT32_CANCREATEVIDEOPORT |
                                              DDHAL_VPORT32_CREATEVIDEOPORT    |
                                              DDHAL_VPORT32_FLIP               |
                                              DDHAL_VPORT32_GETBANDWIDTH       |
                                              DDHAL_VPORT32_GETINPUTFORMATS    |
                                              DDHAL_VPORT32_GETOUTPUTFORMATS   |
                                              DDHAL_VPORT32_GETFIELD           |
//                                            DDHAL_VPORT32_GETLINE            |
                                              DDHAL_VPORT32_GETCONNECT         |
                                              DDHAL_VPORT32_DESTROY            |
                                              DDHAL_VPORT32_GETFLIPSTATUS      |
                                              DDHAL_VPORT32_UPDATE             |
                                              DDHAL_VPORT32_WAITFORSYNC        |
                                              DDHAL_VPORT32_GETSIGNALSTATUS    |
//                                            DDHAL_VPORT32_COLORCONTROL       |
                                              0;

        vpCallbacks.CanCreateVideoPort      = vpCanCreateVideoPort32;
        vpCallbacks.CreateVideoPort         = vpCreateVideoPort32;
        vpCallbacks.FlipVideoPort           = vpFlipVideoPort32;
        vpCallbacks.GetVideoPortBandwidth   = vpGetVideoPortBandwidth32;
        vpCallbacks.GetVideoPortInputFormats= vpGetVideoPortInputFormats32;
        vpCallbacks.GetVideoPortOutputFormats=vpGetVideoPortOutputFormats32;
        vpCallbacks.GetVideoPortField       = vpGetVideoPortField32;
//      vpCallbacks.GetVideoPortLine        = vpGetVideoPortLine32;
        vpCallbacks.GetVideoPortConnectInfo = vpGetVideoPortConnectInfo32;
        vpCallbacks.DestroyVideoPort        = vpDestroyVideoPort32;
        vpCallbacks.GetVideoPortFlipStatus  = vpGetVideoPortFlipstatus32;
        vpCallbacks.UpdateVideoPort         = vpUpdateVideoPort32;
        vpCallbacks.WaitForVideoPortSync    = vpWaitForVideoPortSync32;
        vpCallbacks.GetVideoSignalStatus    = vpGetVideoSignalStatus32;
//      vpCallbacks.ColorControl            = vpColorControl32;

        memcpy(lpInput->lpvData, &vpCallbacks, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_VideoPortCaps))
    {
        DDVIDEOPORTCAPS vpCaps;
        memset(&vpCaps, 0, sizeof(vpCaps));

        dwSize = min(lpInput->dwExpectedSize, sizeof(DDVIDEOPORTCAPS));
        lpInput->dwActualSize               = sizeof(DDVIDEOPORTCAPS);
        vpCaps.dwSize                       = dwSize; // size of the DDVIDEOPORTCAPS structure
        vpCaps.dwFlags                      = 0 /* Fields which contain data */|
                                              DDVPD_WIDTH                      |
                                              DDVPD_HEIGHT                     |
                                              DDVPD_ID                         |
                                              DDVPD_CAPS                       |
                                              DDVPD_FX                         |
                                              DDVPD_AUTOFLIP                   |
//                                            DDVPD_ALIGN                      |
                                              0;

        vpCaps.dwMaxWidth                   = H3_MAX_VID_IN_X; // max width of the video port field
        vpCaps.dwMaxVBIWidth                = H3_MAX_VID_IN_X; // max width of the VBI data
        vpCaps.dwMaxHeight                  = H3_MAX_VID_IN_Y; // max height of the video port field
        vpCaps.dwVideoPortID                = H3_PORT_ID;      // Video port ID (0 - (dwMaxVideoPorts -1))
        vpCaps.dwCaps                       = 0 /* Video port capabilities */  |
                                              DDVPCAPS_AUTOFLIP                |
                                              DDVPCAPS_INTERLACED              |
                                              DDVPCAPS_NONINTERLACED           |
                                              DDVPCAPS_READBACKFIELD           |
//                                            DDVPCAPS_READBACKLINE            |
//                                            DDVPCAPS_SHAREABLE               |
//                                            DDVPCAPS_SKIPEVENFIELDS          | //H3 does not have hardware caps to skip field
//                                            DDVPCAPS_SKIPODDFIELDS           |
//                                            DDVPCAPS_SYNCMASTER              | //Genlock CRTC VSync with video in VSync
//                                            DDVPCAPS_COLORCONTROL            | //Color control operation cap on the Video Port
//                                            DDVPCAPS_SYSTEMMEMORY            |
                                              DDVPCAPS_VBISURFACE              |
                                              DDVPCAPS_OVERSAMPLEDVBI          |
                                              DDVPCAPS_VBIANDVIDEOINDEPENDENT  |
                                              0;

        vpCaps.dwFX                         = 0  /* More video port caps */    |

                                              DDVPFX_CROPTOPDATA               |
//                                            DDVPFX_CROPX                     |
//                                            DDVPFX_CROPY                     |
                                              DDVPFX_INTERLEAVE                |
//                                            DDVPFX_MIRRORLEFTRIGHT           |
//                                            DDVPFX_MIRRORUPDOWN              |
                                              DDVPFX_PRESHRINKX                |
                                              DDVPFX_PRESHRINKY                |
//                                            DDVPFX_PRESHRINKXB               |
//                                            DDVPFX_PRESHRINKYB               |
//                                            DDVPFX_PRESHRINKXS               |
//                                            DDVPFX_PRESHRINKYS               |
//                                            DDVPFX_PRESTRETCHX               |
//                                            DDVPFX_PRESTRETCHY               |
//                                            DDVPFX_PRESTRETCHXN              |
//                                            DDVPFX_PRESTRETCHYN              |

                                              DDVPFX_VBICONVERT                |
                                              DDVPFX_VBINOSCALE                |
                                              DDVPFX_IGNOREVBIXCROP            |
                                              DDVPFX_VBINOINTERLEAVE           |
                                              0;

        vpCaps.dwNumAutoFlipSurfaces        = 3; // Number of autoflippable surfaces
        vpCaps.dwAlignVideoPortBoundary     = 0; // Byte restriction of placement within the surface
        vpCaps.dwAlignVideoPortPrescaleWidth= 0; // Byte restriction of width after prescaling
        vpCaps.dwAlignVideoPortCropBoundary = 0; // Byte restriction of left cropping
        vpCaps.dwAlignVideoPortCropWidth    = 0; // Byte restriction of cropping width
        vpCaps.dwNumVBIAutoFlipSurfaces     = 3; // Number of VBI autoflippable surfaces
        memcpy(lpInput->lpvData, &vpCaps, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_ColorControlCallbacks))
    {
        DDHAL_DDCOLORCONTROLCALLBACKS ccCallbacks;

        memset(&ccCallbacks, 0, sizeof(ccCallbacks));
        dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDCOLORCONTROLCALLBACKS));
        lpInput->dwActualSize               = sizeof(DDHAL_DDCOLORCONTROLCALLBACKS);
        ccCallbacks.dwSize                  = dwSize; // size of the DDHAL_DDCOLORCONTROLCALLBACKS structure
        ccCallbacks.dwFlags                 = DDHAL_COLOR_COLORCONTROL;
        ccCallbacks.ColorControl            = ovlColorControl32;
        memcpy(lpInput->lpvData, &ccCallbacks, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_KernelCallbacks))
    {
         DDHAL_DDKERNELCALLBACKS kCallbacks;

         memset(&kCallbacks, 0, sizeof(kCallbacks));
         dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDKERNELCALLBACKS));
         lpInput->dwActualSize              = sizeof(DDHAL_DDKERNELCALLBACKS);
         kCallbacks.dwSize                  = dwSize; // size of the DDHAL_DDKERNELCALLBACKS structure
         kCallbacks.dwFlags                 = 0 /* Kernel callback flags */    |
#ifdef KMVP
                                              DDHAL_KERNEL_SYNCSURFACEDATA     |
                                              DDHAL_KERNEL_SYNCVIDEOPORTDATA   |
#endif
                                              0;
#ifdef KMVP
         kCallbacks.SyncSurfaceData         = kSyncSurfaceData32;
         kCallbacks.SyncVideoPortData       = kSyncVideoportData32;
#endif
         memcpy(lpInput->lpvData, &kCallbacks, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_KernelCaps))
    {
        DDKERNELCAPS kCaps;

        memset(&kCaps, 0, sizeof(kCaps));
        dwSize = min(lpInput->dwExpectedSize, sizeof(DDKERNELCAPS));
        lpInput->dwActualSize               = sizeof(DDKERNELCAPS);
        kCaps.dwSize                        = dwSize; // size of the DDKERNELCAPS structure
        kCaps.dwCaps                        = DDKERNELCAPS_FLIPOVERLAY; //Support vddFlipOverlay
        kCaps.dwIRQCaps                     = 0;
        if (_DD(bVMIPLDpresent))
        {
            kCaps.dwCaps                   |= 0 /* Modify kernel caps */       |
                                              DDKERNELCAPS_SKIPFIELDS          | //Skip odd or even fields by using hw or vddSkipNextField
                                              DDKERNELCAPS_AUTOFLIP            | //Support vddFlipVideoPort and vddFlipOverlay
                                              DDKERNELCAPS_SETSTATE            | //Support vddSetState to switch between BOB & WEAVE
                                              DDKERNELCAPS_LOCK                | //Support vddLock fb access w/ contention w/ blitter
                                              DDKERNELCAPS_FLIPVIDEOPORT       | //Support vddFlipVideoPort
//                                            DDKERNELCAPS_I2C                 | //This flag is NOT defined anywhere
//                                            DDKERNELCAPS_GPIO                | //This flag is NOT defined anywhere
                                              DDKERNELCAPS_CAPTURE_SYSMEM      | //Kernel support capture to system memory
//                                            DDKERNELCAPS_CAPTURE_NONLOCALVIDMEM | //Kernel support capture to non local video memory
                                              DDKERNELCAPS_FIELDPOLARITY       | //Can report polarity of video field
//                                            DDKERNELCAPS_CAPTURE_INVERTED    | //Can capture and invert the DIB
//                                            DDKERNELCAPS_TRANSFER            | //Support data xfer between system memory and fb
                                              0;
            kCaps.dwIRQCaps                |= 0 /* Modify IRQ caps */          |
                                              DDIRQ_DISPLAY_VSYNC              | //Device generates display VSync
                                              DDIRQ_VPORT0_VSYNC               | //Device generates video VSync on the Video Port 0
                                              0;
        }
        memcpy(lpInput->lpvData, &kCaps, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else 
#endif    
    
#ifndef WINNT
    if (IsEqualIID(&lpInput->guidInfo, &GUID_MiscellaneousCallbacks))
    {
        DDHAL_DDMISCELLANEOUSCALLBACKS ddMiscCallbacks;

        memset(&ddMiscCallbacks, 0, sizeof(ddMiscCallbacks));

        ddMiscCallbacks.dwSize              = sizeof(ddMiscCallbacks);
        ddMiscCallbacks.dwFlags             = DDHAL_MISCCB32_GETAVAILDRIVERMEMORY;
        ddMiscCallbacks.GetAvailDriverMemory= DdGetAvailDriverMemory;

        dwSize = min(lpInput->dwExpectedSize, sizeof(ddMiscCallbacks));
        lpInput->dwActualSize               = sizeof(ddMiscCallbacks);
        memcpy(lpInput->lpvData, &ddMiscCallbacks, dwSize);

        lpInput->ddRVal = DD_OK;
    }
    else
#endif //WINNT

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  if (IsEqualIID(&lpInput->guidInfo, &GUID_Miscellaneous2Callbacks))
  {
    extern DWORD __stdcall ddiGetDriverState( LPDDHAL_GETDRIVERSTATEDATA pgdsd );
    extern DWORD __stdcall ddiCreateSurfaceEx( LPDDHAL_CREATESURFACEEXDATA pcsxd );
    extern DWORD __stdcall ddiDestroyDDLocal( LPDDHAL_DESTROYDDLOCALDATA pdddd );

    DDHAL_DDMISCELLANEOUS2CALLBACKS Misc2Callbacks;

    DISPDBG((0, "Get Misc2 Callbacks"));

    memset(&Misc2Callbacks, 0, sizeof(Misc2Callbacks));

    dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS));
    lpInput->dwActualSize = sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS);

    CHECKSIZE(DDHAL_DDMISCELLANEOUS2CALLBACKS);

    Misc2Callbacks.dwSize                   = dwSize;
    Misc2Callbacks.dwFlags                  = 0 /* Misc. callback flags */     |
                                              DDHAL_MISC2CB32_GETDRIVERSTATE   |
                                              DDHAL_MISC2CB32_CREATESURFACEEX  |
                                              DDHAL_MISC2CB32_DESTROYDDLOCAL   |
                                              0;

    Misc2Callbacks.GetDriverState           = ddiGetDriverState;
    Misc2Callbacks.CreateSurfaceEx          = ddiCreateSurfaceEx;
    Misc2Callbacks.DestroyDDLocal           = ddiDestroyDDLocal;

    memcpy(lpInput->lpvData, &Misc2Callbacks, dwSize);
    lpInput->ddRVal = DD_OK;
  }
  else 
#else
    if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DCallbacks2) )
    {
      D3DHAL_CALLBACKS2 D3DCallbacks2;
      memset(&D3DCallbacks2, 0, sizeof(D3DCallbacks2));

      dwSize = min(lpInput->dwExpectedSize, sizeof(D3DHAL_CALLBACKS2));
      lpInput->dwActualSize                 = sizeof(D3DHAL_CALLBACKS2);
      CHECKSIZE(D3DHAL_CALLBACKS2);

      D3DCallbacks2.dwSize                  = dwSize;
      D3DCallbacks2.dwFlags                 = 0 /* D3D callback flags */       |
//                                            D3DHAL2_CB32_SETRENDERTARGET     |
                                              D3DHAL2_CB32_DRAWONEPRIMITIVE    |
                                              D3DHAL2_CB32_DRAWONEINDEXEDPRIMITIVE |
                                              D3DHAL2_CB32_DRAWPRIMITIVES      |
                                              D3DHAL2_CB32_CLEAR               |
                                              0;

//    D3DCallbacks2.SetRenderTarget         = mySetRenderTarget32;
      D3DCallbacks2.DrawOnePrimitive        = ddiDrawOnePrimitive;
      D3DCallbacks2.DrawOneIndexedPrimitive = ddiDrawOneIndexedPrimitive;
      D3DCallbacks2.DrawPrimitives          = ddiDrawPrimitives;
      if (_FF(ddMiscFlags) & DDMF_MEMTYPE_SDRAM)
        D3DCallbacks2.Clear                 = ddiClearSD;
      else 
        D3DCallbacks2.Clear                 = ddiClearSG;

      memcpy(lpInput->lpvData, &D3DCallbacks2, dwSize);
      lpInput->ddRVal = DD_OK;
    }
   else
#endif
     if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DExtendedCaps) )
    {
      D3DHAL_D3DEXTENDEDCAPS D3DExtendedCaps;

      BuildD3DExtendedCaps(ppdev, &D3DExtendedCaps);
      dwSize = min(lpInput->dwExpectedSize, sizeof(D3DHAL_D3DEXTENDEDCAPS));
      lpInput->dwActualSize                 = sizeof(D3DHAL_D3DEXTENDEDCAPS);
      memcpy(lpInput->lpvData, &D3DExtendedCaps, dwSize);
      lpInput->ddRVal = DD_OK;

    }
#if( DIRECTDRAW_VERSION >= 0x0600 )
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DCallbacks3) )
  {
    extern DWORD  __stdcall ddiValidateTextureStageState( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pcd );
    extern DWORD  __stdcall ddiDrawPrimitives2( LPD3DHAL_DRAWPRIMITIVES2DATA pcd );

    D3DHAL_CALLBACKS3 D3DCallbacks3;
    memset(&D3DCallbacks3, 0, sizeof(D3DCallbacks3));

    dwSize = min(lpInput->dwExpectedSize, sizeof(D3DHAL_CALLBACKS3));
    lpInput->dwActualSize                   = sizeof(D3DHAL_CALLBACKS3);
    CHECKSIZE( D3DHAL_CALLBACKS3 );

    D3DCallbacks3.dwSize                    = dwSize;
    D3DCallbacks3.dwFlags                   = 0 /* Additional D3D flags */     |
                                              D3DHAL3_CB32_VALIDATETEXTURESTAGESTATE |
                                              D3DHAL3_CB32_DRAWPRIMITIVES2     |
                                              0;

    D3DCallbacks3.ValidateTextureStageState = ddiValidateTextureStageState;
    D3DCallbacks3.DrawPrimitives2           = ddiDrawPrimitives2;

    if (IS_NAPALM)
    {
     D3DCallbacks3.dwFlags                 |= D3DHAL3_CB32_CLEAR2;

      if (_FF(ddMiscFlags) & DDMF_MEMTYPE_SDRAM)
        D3DCallbacks3.Clear2                = ddiClear2SD;
      else 
        D3DCallbacks3.Clear2                = ddiClear2SG;
    }

    memcpy(lpInput->lpvData, &D3DCallbacks3, dwSize);
    lpInput->ddRVal = DD_OK;
  }
#if( DIRECTDRAW_VERSION >= 0x0800 )
    // Check for calls to GetDriverInfo2
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_GetDriverInfo2) )
    {
        // Make sure this is actually a call to GetDriverInfo2 
        // ( and not a call to DDStereoMode!)
        if (D3DGDI_IS_GDI2(lpInput))
        {
            // Yes, its a call to GetDriverInfo2, fetch the
            // DD_GETDRIVERINFO2DATA data structure.
            DD_GETDRIVERINFO2DATA* pGDI2 = D3DGDI_GET_GDI2_DATA(lpInput);
            DD_DXVERSION *pDXVer;
            DD_GETFORMATCOUNTDATA *pGFCD;
            DD_GETFORMATDATA *pGFD;
            size_t copySize;
            D3DCAPS8 D3DCaps8;

            // What type of request is this?
            switch (pGDI2->dwType)
            {
                case D3DGDI2_TYPE_DXVERSION:

                    // This is a way for a driver on NT to find out the DX-Runtime 
                    // version. (This functionality exists on 9x but not on NT). 
                    // This information is provided to a new driver (i.e. one that 
                    // exposes GETDRIVERINFO2) for DX7 applications and DX8 
                    // applications. And you should get x0000800 for dwDXVersion; 
                    // or more accurately, you should DD_RUNTIME_VERSION which is 
                    // defined in ddrawi.h.

                    pDXVer = (DD_DXVERSION *) pGDI2;
                    _FF(ddRunTimeVersion) = pDXVer->dwDXVersion;

                    lpInput->dwActualSize = sizeof(DD_DXVERSION);
                    lpInput->ddRVal       = DD_OK;                

                    break;

                case D3DGDI2_TYPE_GETFORMATCOUNT:

                    // Its a request for the number of texture formats
                    // we support. Get the extended data structure so
                    // we can fill in the format count field.

                    pGFCD = (DD_GETFORMATCOUNTDATA *) pGDI2;
                    pGFCD->dwFormatCount = g_nDX8TextureFormats;

                    lpInput->dwActualSize = sizeof(DD_GETFORMATCOUNTDATA);
                    lpInput->ddRVal       = DD_OK;

                    break;

                case D3DGDI2_TYPE_GETFORMAT:

                    // Its a request for a particular format we support.
                    // Get the extended data structure so we can fill in
                    // the format field.

                    pGFD = (DD_GETFORMATDATA *) pGDI2;
                
                    // Initialize the surface description and copy over
                    // the pixel format from out pixel format table.

                    memcpy(&pGFD->format, &g_DX8TextureFormats[pGFD->dwFormatIndex].ddpfPixelFormat,
                           sizeof(pGFD->format));

                    lpInput->dwActualSize = sizeof(DD_GETFORMATDATA);
                    lpInput->ddRVal       = DD_OK;

                    break;

               case D3DGDI2_TYPE_GETD3DCAPS8:

                    // The runtime is requesting the DX8 D3D caps build them
                
                    BuildD3DCaps8(ppdev, &D3DCaps8);

                    // It should be noted that the dwExpectedSize field
                    // of DD_GETDRIVERINFODATA is not used for
                    // GetDriverInfo2 calls and should be ignored.

                    copySize = min(sizeof(D3DCAPS8), pGDI2->dwExpectedSize);
                    memcpy(lpInput->lpvData, &D3DCaps8, copySize);

                    lpInput->dwActualSize = copySize;
                    lpInput->ddRVal       = DD_OK;

                    break;

                default:

                    // Default behavior for any other type.

                    break;
            }
        }
    }
#endif
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_ZPixelFormats))
  {
    DDPIXELFORMAT ddZBufPixelFormat[3];
    DWORD         dwNumZPixelFormats;

    DISPDBG((0, "Get Z Pixel Formats"));
    memset(&ddZBufPixelFormat, 0, sizeof(ddZBufPixelFormat));

    // 16bpp Z Buffer
    ddZBufPixelFormat[0].dwSize            = sizeof(DDPIXELFORMAT);
    ddZBufPixelFormat[0].dwFlags           = DDPF_ZBUFFER;
    ddZBufPixelFormat[0].dwFourCC          = 0;
    ddZBufPixelFormat[0].dwZBufferBitDepth = 16;
    ddZBufPixelFormat[0].dwStencilBitDepth = 0;
    ddZBufPixelFormat[0].dwZBitMask        = 0xFFFF;
    ddZBufPixelFormat[0].dwStencilBitMask  = 0x0000;
    ddZBufPixelFormat[0].dwRGBZBitMask     = 0;

    if (IS_NAPALM)
    {
      // 24bpp zbuffer, no stencil buffer
      ddZBufPixelFormat[1].dwSize            = sizeof(DDPIXELFORMAT);
      ddZBufPixelFormat[1].dwFlags           = DDPF_ZBUFFER;
      ddZBufPixelFormat[1].dwFourCC          = 0;
      ddZBufPixelFormat[1].dwZBufferBitDepth = 24;
      ddZBufPixelFormat[1].dwStencilBitDepth = 0;
      ddZBufPixelFormat[1].dwZBitMask        = 0x00FFFFFF;
      ddZBufPixelFormat[1].dwStencilBitMask  = 0x00000000;
      ddZBufPixelFormat[1].dwRGBZBitMask     = 0;

      // 24bpp Z Buffer, 8 bit Stencil
      ddZBufPixelFormat[2].dwSize            = sizeof(DDPIXELFORMAT);
      ddZBufPixelFormat[2].dwFlags           = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
      ddZBufPixelFormat[2].dwFourCC          = 0;
      ddZBufPixelFormat[2].dwZBufferBitDepth = 32; // Zbuffer + stencil buffer depth!
      ddZBufPixelFormat[2].dwStencilBitDepth = 8;
      ddZBufPixelFormat[2].dwZBitMask        = 0x00FFFFFF;
      ddZBufPixelFormat[2].dwStencilBitMask  = 0xFF000000;
      ddZBufPixelFormat[2].dwRGBZBitMask     = 0;

      dwNumZPixelFormats = 3;
      dwSize = min(lpInput->dwExpectedSize, 3 * sizeof(DDPIXELFORMAT));
      lpInput->dwActualSize = 3 * sizeof(DDPIXELFORMAT) + sizeof(DWORD);
    }
    else
    {
      dwNumZPixelFormats = 1;
      dwSize = min(lpInput->dwExpectedSize, 1 * sizeof(DDPIXELFORMAT));
      lpInput->dwActualSize = 1 * sizeof(DDPIXELFORMAT) + sizeof(DWORD);
    }

    memcpy(lpInput->lpvData, &dwNumZPixelFormats, sizeof(DWORD));
    memcpy((LPVOID)((LPBYTE)(lpInput->lpvData) + sizeof(DWORD)),
           &ddZBufPixelFormat, dwSize);

    lpInput->ddRVal = DD_OK;
  }
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DParseUnknownCommandCallback) )
  {
    extern PFND3DPARSEUNKNOWNCOMMAND dp2Callback;

    dp2Callback = (PFND3DPARSEUNKNOWNCOMMAND)lpInput->lpvData;
    lpInput->ddRVal = DD_OK;
  }
#endif

#ifdef STEREO
    else if( IsEqualIID(&lpInput->guidInfo, &GUID_DDMoreSurfaceCaps) )
    {
    DDMORESURFACECAPS   DDMoreSurfaceCaps;
    DDSCAPSEX           ddsCapsEx, ddsCapsExAlt;

    memset(&DDMoreSurfaceCaps, 0, sizeof(DDMoreSurfaceCaps));

    memset(&ddsCapsEx,      0, sizeof(ddsCapsEx));
    memset(&ddsCapsExAlt,   0, sizeof(ddsCapsExAlt));

    DDMoreSurfaceCaps.dwSize = lpInput->dwExpectedSize;

    if(_FF(StereoMode) && _FF(ddStereoWrapperLoaded))
            DDMoreSurfaceCaps.ddsCapsMore.dwCaps2 = DDSCAPS2_STEREOSURFACELEFT;

    lpInput->dwActualSize = lpInput->dwExpectedSize;

    dwSize = min(sizeof(DDMoreSurfaceCaps), lpInput->dwExpectedSize);

    memcpy(lpInput->lpvData, &DDMoreSurfaceCaps, dwSize);

    while(dwSize < lpInput->dwExpectedSize)
    {
        memcpy( (PBYTE)lpInput->lpvData+dwSize, &ddsCapsEx, sizeof(DDSCAPSEX));

        dwSize += sizeof(DDSCAPSEX);

        memcpy( (PBYTE)lpInput->lpvData+dwSize, &ddsCapsExAlt, sizeof(DDSCAPSEX));

        dwSize += sizeof(DDSCAPSEX);
    }

    lpInput->ddRVal = DD_OK;
    }


#endif //STEREO


    return DDHAL_DRIVER_HANDLED;

}// DdGetDriverInfo


/*----------------------------------------------------------------------
Function name:  AhNuts_GetTheDevNodeKeyMyself

Description:    Generate the DevNodeKey.

Return:         static CONFIGRET
                CR_SUCCESS - registry successfully navigated, key copied
            CR_FAILURE  - unable to navigate registry
----------------------------------------------------------------------*/
static CONFIGRET
AhNuts_GetTheDevNodeKeyMyself(DEVNODE   dnDevNode,
                              PFARVOID  Buffer,
                              ULONG     BufferLen)
{
  HKEY  hkey;
  char  Key[MAX_VMM_REG_KEY_LEN];
  DWORD type;
  DWORD length;


  // make sure the Buffer is large enough
  if (strlen("System\\CurrentControlSet\\Services\\Class\\Display\\XXXX")+1 > BufferLen)
    return CR_FAILURE;

  // convert dnDevNode to a string (in hex)
  // and tack it onto "Config Manager\Enum\"
  strcpy(Key, "Config Manager\\Enum\\");
  ddxtoa(dnDevNode, Key+strlen(Key));

  // open the HKEY_DYN_DATA\Config Manager\Enum\"DevNode" key
  if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_DYN_DATA,
                                    Key,
                                    0,
                                    KEY_QUERY_VALUE | KEY_READ,
                                    &hkey))
    return CR_FAILURE;

  // read the HardWareKey value
  // and tack in onto "Enum\\"
  strcpy(Key, "Enum\\");
  length = sizeof(Key)-strlen(Key);
  if (ERROR_SUCCESS != RegQueryValueEx(hkey,
                                       "HardWareKey",
                                       0,
                                       &type,
                                       Key+strlen(Key),
                                       &length))
  {
    RegCloseKey(hkey);
    return CR_FAILURE;
  }

  if (REG_SZ != type)
  {
    RegCloseKey(hkey);
    return CR_FAILURE;
  }

  // close HKEY_DYN_DATA\Config Manager\Enum\"DevNode" key
  RegCloseKey(hkey);

  // open the HKEY_LOCAL_MACHINE\Enum\"HardWareKey" key
  if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    Key,
                                    0,
                                    KEY_QUERY_VALUE | KEY_READ,
                                    &hkey))
    return CR_FAILURE;

  // read the Driver value
  length = sizeof(Key);
  if (ERROR_SUCCESS != RegQueryValueEx(hkey,
                                       "Driver",
                                       0,
                                       &type,
                                       Key,
                                       &length))
  {
    RegCloseKey(hkey);
    return CR_FAILURE;
  }

  if (REG_SZ != type)
  {
    RegCloseKey(hkey);
    return CR_FAILURE;
  }

  // close HKEY_LOCAL_MACHINE\Enum\"HardWareKey" key
  RegCloseKey(hkey);

  // finally generate the DevNodeKey to return to the caller
  strcpy(Buffer, "System\\CurrentControlSet\\Services\\Class\\");
  strcat(Buffer, Key);

  return CR_SUCCESS;

} // AhNuts_GetTheDevNodeKeyMyself


/*----------------------------------------------------------------------
Function name:  ddgetenv

Description:    Get environment variable from registry.

Return:         char * - success, pointer to environment variable
            NULL   - failure
----------------------------------------------------------------------*/

#define  PRSIZE  50      // GetProfileString return buffer

char *ddgetenv(NT9XDEVICEDATA *ppdev, const char *varname )
{
  char    rstr[ PRSIZE ];
  char    nstr[]="\0";
  char    *lpnull;
  char    *lprstr;
  DWORD   DevNode;
  char    DevNodeKey[MAX_VMM_REG_KEY_LEN];

  lprstr = rstr;
  lpnull = nstr;

  // First check in [3dfx] section of WIN.INI
  if( GetProfileString( "3Dfx", varname, lpnull, lprstr, PRSIZE ) )
  {
    // returning a pointer to a local var seems rather dangerous
    return( lprstr );
  }

  DevNode = _FF(DevNode);

  // convert devnode to a registry key
  // when successful, this thing returns a string something like
  // "System\CurrentControlSet\Services\Class\DISPLAY\XXXX"
  //
  // fix for PRS bug 1681
  // It appears that the CM_ functions aren't fully implemented on the
  // original win95 (aka OSR1, windows 95a (gold))
  // so we have to go thru the registry and generate the DevNodeKey
  // on our own
  if ((CR_SUCCESS == CM_Get_DevNode_Key(DevNode,
                                        NULL,
                                        (PFARVOID)DevNodeKey,
                                        sizeof(DevNodeKey),
                                        CM_REGISTRY_SOFTWARE)) ||
      (CR_SUCCESS == AhNuts_GetTheDevNodeKeyMyself(DevNode,
                                                   DevNodeKey,
                                                   sizeof(DevNodeKey))))
  {
    typedef struct _H3_SEARCH_TYPE
    {
      HKEY  hkey;
      char  *pszSubkey;
    } H3_SEARCH_TYPE;

    static const H3_SEARCH_TYPE H3SearchOrder[] =
    {
      { HKEY_CURRENT_USER,  "\\D3D" },
      { HKEY_CURRENT_USER,  NULL    },
      { HKEY_LOCAL_MACHINE, "\\D3D" },
      { HKEY_LOCAL_MACHINE, NULL    },
    };
    const H3_SEARCH_TYPE *pSearchLoc;
    HKEY  hkey;
    DWORD type;
    DWORD length;
    ULONG DevNodeKeyLength;


    // save original length of DevNodeKey
    DevNodeKeyLength = strlen(DevNodeKey);

    // loop over possible registry locations
    for (pSearchLoc = &H3SearchOrder[0];
         pSearchLoc < &H3SearchOrder[sizeof(H3SearchOrder)/sizeof(H3SearchOrder[0])];
         pSearchLoc++)
    {
      // if we have a non NULL subkey
      // tack the subkey to the end of the DevNodeKey
      if (NULL != pSearchLoc->pszSubkey)
        strcat(DevNodeKey, pSearchLoc->pszSubkey);

      // attempt to open the key
      if (ERROR_SUCCESS == RegOpenKeyEx(pSearchLoc->hkey,
                                        DevNodeKey,
                                        0,
                                        KEY_QUERY_VALUE | KEY_READ,
                                        &hkey))
      {
        // the key exists so attempt to read the value of varname
        length = sizeof(rstr);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             varname,
                                             0,
                                             &type,
                                             lprstr,
                                             &length))
        {
          // the value exists so check it's type
          // if it's a string then return it
          // otherwise loop to the next search location
          //
          // we could put a switch statement here
          // and convert other types to strings
          if (REG_SZ == type)
          {
            RegCloseKey(hkey);
            // returning a pointer to a local var seems rather dangerous
            return lprstr;
          }
        }

        RegCloseKey(hkey);
      }

      // restore the original DevNodeKey, in case we tacked on a subkey above
      DevNodeKey[DevNodeKeyLength] = '\0';
    }
  }

  // Return NULL if not found
  return( NULL );

} // ddgetenv



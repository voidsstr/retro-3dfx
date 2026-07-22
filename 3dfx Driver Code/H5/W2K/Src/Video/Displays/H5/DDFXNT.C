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
** File Name:  DDFXNT.C
**
** Description:
**
** $Revision: 58$
** $Date: 10/31/00 1:58:20 AM$
**
** $Log: 
**  58   3dfx      1.38.2.10.1.710/31/00 Johnny Trainor  Updated so we no longer
**       use surface local pointers.
**  57   3dfx      1.38.2.10.1.610/20/00 Johnny Trainor  We now use
**       BuildD3DExtendedCaps to get our extended capabilities. Added support for
**       the GetDriverInfo2 case under DX8.
**  56   3dfx      1.38.2.10.1.510/13/00 Johnny Trainor  Changed occurances of
**       D3DNTHAL_D3DEXTENDEDCAPS to the new Dx8 version D3DHAL_D3DEXTENDEDCAPS.
**       Added a conditional include to file dx7todx8.h. 
**  55   3dfx      1.38.2.10.1.410/12/00 Reid Campbell   Added API version for 3dfx
**       Tools to check
**  54   3dfx      1.38.2.10.1.310/11/00 Brent           Forced check in to enforce
**       branching.
**  53   3dfx      1.38.2.10.1.208/11/00 Russ Lind       merge of various w9x
**       changes into w2k
**       including
**         LODBIASPERCHIP
**         LODDITHER
**         MAXPENDINGBUFFERS
**         TEXTURE_REPEAT_NOT_SCALED
**         COLUMNBANDCONTROL_DEFAULT
**         SLI_ABOVE_1280
**         fix for z buffer problems in Shadow of the Empire
**         flat shaded specular fix
**         part of the Daytona/Napalm2 changes
**       also fix for PRS 15212 - Star Trek Armada corruption in menus, gotta punt
**       gdi calls on 4500 for 2 sample aa
**  52   3dfx      1.38.2.10.1.108/10/00 Russ Lind       initialize
**       _FF(ModeUses2PixPerClkRender) to enable 2 pixels per clock on napalm to
**       get 2 pixels per clock working on w2k
**  51   3dfx      1.38.2.10.1.007/06/00 Edwin Wong      Do not call
**       Clear_SLIAA_Buffers() for single buffer apps., e.g. DirectX Caps Viewer's
**       Texture Formats.
**  50   3dfx      1.38.2.10   06/09/00 Edwin Wong      Use backend to support 2
**       sample AA when we failed to allocate buffers for 4 sample AA.
**  49   3dfx      1.38.2.9    06/06/00 Russ Lind       changed a #define name
**  48   3dfx      1.38.2.8    06/06/00 Russ Lind       merge from w9x, mods to 4
**       chip support
**  47   3dfx      1.38.2.7    06/02/00 Russ Lind       modified dynamic memory
**       allocation code so MEMCHECK tracking can be enabled
**  46   3dfx      1.38.2.6    05/31/00 Russ Lind       Fix for PRS 14389 & 14419,
**       add function to allow the driver to pull out of
**       AA mode when memMgr_allocSecondary fails.  See BailOutOfAA for additional
**       comments.
**  45   3dfx      1.38.2.5    05/26/00 Russ Lind       changes for optimized
**       tileCtrl/tileCompare settings in sli mode
**  44   3dfx      1.38.2.4    05/25/00 Russ Lind       changes to support extra
**       linear heap in sli mode
**  43   3dfx      1.38.2.3    05/12/00 Edwin Wong      Fix 14260: Enable
**       Clear_SLIAA_Buffers, always clear gdidesktop.
**  42   3dfx      1.38.2.2    05/09/00 Russ Lind       added LoadGamma to compute
**       and load the D3D gamma ramp in Enter_3DApplication and RestoreGamma to
**       restore the desktop gamma ramp in Exit_3DApplication
**  41   3dfx      1.38.2.1    05/05/00 StarTeam VTS Administrator Russ Lind ==
**       5/3/00 4:25 PM
**       reenable sli/aa on w2k
**  40   3dfx      1.38.2.0    05/05/00 StarTeam VTS Administrator Russ Lind 5/2/00
**       6:01 PM
**       merge GuardBand settings, Promote_PrimaryToOverlay & KILL_ANALOG stuff
**       from win9x
**       disable ClearSLIAABuffers
**  39   3dfx      1.38        04/26/00 Russ Lind       add code in DdMapMemory to
**       track linear address mapped to a process (replaces AddressList code in
**       miniport)
**  38   3dfx      1.37        04/10/00 Russ Lind       merge of changes for new
**       setup for 2 chip 2-sample AA
**  37   3dfx      1.36        04/07/00 Russ Lind       merge slibandheight calc
**       from win9x.  add code to reinit the cmdfifo if the miniport fails to
**       enable sli/aa and set gdiDesktopStart to the modified hw address of the
**       primary in Promote_DeviceToSLIAA and Demote_DeviceFromSLIAA
**  36   3dfx      1.35        03/29/00 Russ Lind       reload dac in
**       Promote_DeviceToSLIAA so slaves will snoop it
**  35   3dfx      1.34        03/23/00 Russ Lind       port of glide changes from
**       V3_W2K branch, changes are #ifdef'd and currently disabled
**  34   3dfx      1.33        03/21/00 Russ Lind       merge of more win9x changes
**       from ddfxs32.c
**  33   3dfx      1.32        03/14/00 Russ Lind       merge is recent w9x changes
**       from ddfxs32.c
**  32   3dfx      1.31        03/08/00 Russ Lind       fix for PRS 13283, remove
**       the screw up I added to DdMapMemory in rev 29
**  31   3dfx      1.30        03/07/00 Russ Lind       disable sli/aa on w2k
**       retail/free builds
**  30   3dfx      1.29        03/06/00 Russ Lind       in Promote_DeviceToSLIAA &
**       Demote_DeviceFromSLIAA, always disable and re-enable the cmdfifo
**  29   3dfx      1.28        03/03/00 Russ Lind       update ViewSize in
**       DdMapMemory to account for sli mode
**       merge in a bunch of sli/aa setup from win9x's ddfxs32.c
**       temporarily disable app requested aa support until we can figure out why 4
**       sample aa hangs
**  28   3dfx      1.27        02/29/00 Russ Lind       corrected roomToEnd
**       calculations in Promote_DeviceToSLIAA and Demote_DeviceFromSLIAA
**  27   3dfx      1.26        02/25/00 Russ Lind       sync up with win9x changes
**       to default to bandheight of 32 and digital
**  26   3dfx      1.25        02/23/00 Russ Lind       merge in recent win9x
**       changes to Compute_SLIAA_Config
**  25   3dfx      1.24        02/23/00 Christopher Wilcox Ported fix for PRS 12659
**       from W9x to W2k.
**  24   3dfx      1.23        02/23/00 Christopher Wilcox Removed
**       CachedWaitOnVsync variable.
** 
**  23   3dfx      1.22        02/17/00 Russ Lind       moved CmdFifo0Disable call
**       in Promote_DeviceToSLIAA up a couple lines
**       added Modify_SLI_Read function
**  22   3dfx      1.21        02/16/00 Russ Lind       pull in _DD(dwMaxHeight)
**       calculation from win9x
**  21   3dfx      1.20        02/11/00 Russ Lind       In Promote_DeviceToSLIAA,
**       if sli is being enabled, disable the cmdfifo, then call the miniport to
**       enable sli, then reenable the cmdfifo.  In Demote_DeviceFromSLIAA, if sli
**       was enabled, disable the cmdfifo, then call the miniport to disable sli,
**       then reenable the cmdfifo
**  20   3dfx      1.19        02/09/00 Lauren Post     V3TV Win2k Enablement Fixes
**  19   3dfx      1.18        02/04/00 Russ Lind       pull in win9x changes to
**       fix PRS 12589
**  18   3dfx      1.17        02/03/00 Christopher Wilcox Move dd3DInOverlay from
**       _FF to _DD, to match W9x changes.
** 
**  17   3dfx      1.16        01/26/00 Russ Lind       mods in
**       Compute_SLIAA_Config to handle app requested AA
**  16   3dfx      1.15        01/20/00 Russ Lind       correction to ZPixelFormats
**       handling in DdGetDriverInfo, fixes majority of stencil buffer problems on
**       w2k
**       also sync up Enter/Exit_3DApplication & Promote_PrimaryToOverlay with
**       win9x changes
**  15   3dfx      1.14        01/18/00 Russ Lind       Added some modifications
**       for AA
**       Pass Bits Per Pixel since we need this to program cfgAAlfbCtrl
**  14   3dfx      1.13        01/06/00 Russ Lind       pull in changes made to rev
**       9 of ddfxs32.c in win9x branch
**  13   3dfx      1.12        12/14/99 Christopher Wilcox Changed to using
**       SLI_AA_CONFIGURATION registry variable to control SLI and AA
**       configuration.
**  12   3dfx      1.11        12/02/99 Christopher Wilcox Synchronized with
**       scanline doubling changes to ddfxs32.c.
** 
**  11   3dfx      1.10        11/30/99 Christopher Wilcox Use scanline doubling
**       only for low resolution modes.
** 
**  10   3dfx      1.9         11/09/99 Russ Lind       merge of win9x sli/aa
**       memory management changes
**  9    3dfx      1.8         11/08/99 Russ Lind       set dvMaxVertexW in
**       D3DExtendedCaps handler of DdGetDriverInfo
**       change T&L cap from NONLOCALVIEWER to LOCALVIEWER
**  8    3dfx      1.7         10/27/99 Russ Lind       added TnL_HAL caps in
**       D3DExtendedCaps GUID handler in DdGetDriverInfo
**  7    3dfx      1.6         10/25/99 Russ Lind       fix for dct250 MipFilter
**       Point - Full NonSquare Width & Height and MipFilter Linear - Full
**       NonSquare Width & Height failures, set dwMaxTextureAspectRatio to 8 in
**       D3DExtendedCaps
**  6    3dfx      1.5         10/15/99 Russ Lind       fix for PRS 8988 - Low
**       Resolution 320x200 thru 512x584 modes are broken with DX7 Samples
**       in Promote_PrimaryToOverlay, when SST_HALF_MODE is set in vidProcCfg,
**       enable vertical overlay scaling and set up to stretch by two
**  5    3dfx      1.4         10/05/99 Russ Lind       added call to miniport to
**       get sli/aa info
**  4    3dfx      1.3         09/24/99 Russ Lind       for csim builds, read
**       pllCtrl1 from the hw rather than the simulator to compute the memclk
**  3    3dfx      1.2         09/16/99 Russ Lind       init _FF(dwNumUnits) in
**       bEnableDirectDraw
**  2    3dfx      1.1         09/14/99 Russ Lind       update to starteam keywords
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"
#include "regkeys.h"
#if ENABLE_VIDEOPORT
#include "ddkernel.h"
#endif
#if (_WIN32_WINNT >= 0x0500)
#include "vmipld.h"
#endif
#ifdef SLI_AA
#include "ddsli2d.h"
#endif

#if (DXDDKVERSION == 7)
#include "dx7todx8.h"
#endif

// Include for new DX 8 macros
#if( DIRECTDRAW_VERSION >= 0x0800 )
#include <d3dhal.h>
#include <d3dhalex.h>
#endif


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

extern HANDLE getCurrentProcessId(PDEV *);

extern BOOL HWSetPalette ( PDEV *, int, int, PVIDEO_CLUTDATA, GAMMA_STATE );

void BuildD3DCaps(NT9XDEVICEDATA *ppdev, D3DDEVICEDESC_V1 *pCaps);
extern void BuildD3DExtendedCaps(NT9XDEVICEDATA *ppdev, D3DHAL_D3DEXTENDEDCAPS *pCaps);

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
extern void BuildD3DCaps8(NT9XDEVICEDATA *ppdev, D3DCAPS8 *pCaps);
#endif

/**************************************************************************
* G L O B A L   V A R I A B L E S
***************************************************************************/

LFBMAPPING LfbMappings[MAX_ADDRESS_TABLE_SIZE];

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
// Note these should probably be changed from globals at a later stage
// if they cause problems with multimonitor/multithreaded support.
extern DWORD g_nDX8TextureFormats;
extern DDSURFACEDESC g_DX8TextureFormats[];
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
    DWORD   i;
    HANDLE  curPID;


    ppdev = (PDEV*) lpMapMemory->lpDD->dhpdev;

    if (lpMapMemory->bMap)
    {
        curPID = getCurrentProcessId(ppdev);
        if ((HANDLE)-1 == curPID)
        {
          DISPDBG((0, "DdMapMemory - unable to get current process id"));

          lpMapMemory->fpProcess = 0;
          lpMapMemory->ddRVal = DDERR_GENERIC;
          return DDHAL_DRIVER_HANDLED;
        }
  
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
#if 1
        ShareMemory.ViewSize = ROUND_UP_TO_64K(2 *ppdev->cjBank);
#else
        ShareMemory.ViewSize
            = ROUND_UP_TO_64K((ppdev->cyScreen) * ppdev->lDelta +
                              (ppdev->pjScreen - ppdev->pjScreenBase));
#endif
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

        DISPDBG((1, "DdMapMemory(mapping) - PID=%8lXh, LfbAddr=%8lXh, ViewSize=%8lXh",
                 curPID, ShareMemoryInformation.VirtualAddress, ShareMemoryInformation.SharedViewSize));

        for (i = 0; i < MAX_ADDRESS_TABLE_SIZE; i++)
        {
          // see if we already have a mapping for this ProcessID
          // overwrite the previous one if we do
          if (curPID == LfbMappings[i].ProcessID)
          {
            DISPDBG((1, "  memory already mapped for this process, index=%ld, PID=%8lXh, newLFBAddr=%8lXh, prevLFBAddr",
                     i, curPID, ShareMemoryInformation.VirtualAddress, LfbMappings[i].LFBAddr));

            // overwrite old mapping
            // this means we lose the old mapping, and when DdMapMemory is called to unmap that
            // address we won't clear it in the LfbMappings array, hope this doesn't cause a
            // problem
            LfbMappings[i].ProcessID = curPID;
            LfbMappings[i].LFBAddr = (DWORD)ShareMemoryInformation.VirtualAddress;
            break;
          }
          // otherwise find an empty slot in the LfbMappings array
          else if (0 == LfbMappings[i].ProcessID)
          {
            LfbMappings[i].ProcessID = curPID;
            LfbMappings[i].LFBAddr = (DWORD)ShareMemoryInformation.VirtualAddress;

            DISPDBG((1, "  adding mapping at index=%ld, PID=%8lXh, LFBAddr=%8lXh",
                     i, LfbMappings[i].ProcessID, LfbMappings[i].LFBAddr));

            break;
          }
        }
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
            RETRO_GP_SPIN(ppdev);  /* retro3dfx: bounded FIFO spin-down */
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
        curPID = getCurrentProcessId(ppdev);

        ShareMemory.ProcessHandle           = lpMapMemory->hProcess;
        ShareMemory.ViewOffset              = 0;
        ShareMemory.ViewSize                = 0;
        ShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;

        DISPDBG((1, "DdMapMemory(unmapping) - PID(from OS)=%8lXh, curPID=%8lXh, LfbAddr=%8lXh",
                 lpMapMemory->hProcess, curPID, lpMapMemory->fpProcess));

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

        for (i = 0; i < MAX_ADDRESS_TABLE_SIZE; i++)
        {
          if ((curPID == LfbMappings[i].ProcessID) &&
              (lpMapMemory->fpProcess == LfbMappings[i].LFBAddr))
          {
            DISPDBG((1, "  removing mapping at index=%ld, PID=%8lXh, LFBAddr=%8lXh",
                     i, LfbMappings[i].ProcessID, LfbMappings[i].LFBAddr));

            LfbMappings[i].ProcessID = 0;
            LfbMappings[i].LFBAddr = 0;
            break;
          }
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
#if (_WIN32_WINNT >= 0x0500)
DisableDDraw:
#endif
        // read Misc Output reg and save bit 7 (vsync polarity) in ddMiscFlags
        _FF(ddMiscFlags) &= ~DDMF_VSYNC_POLARITY_MASK;
        _FF(ddMiscFlags) |= (inp(ppdev->pjIoBase + 0xCC) & 0x80) >> (7 - DDMF_VSYNC_POLARITY_BIT);

        // DirectDraw is disabled for use on this card
        ppdev->flStatus &= ~STAT_DIRECTDRAW;

        return TRUE;
      }
    }

#if (_WIN32_WINNT >= 0x0500)
    // temporarily use the height local var to read CapabilityOverride key from registry

    // MS uses a registry setting called CapabilityOverride to decide in the OS
    // whether ddraw and/or d3d support is enabled.
    // Currently, the default setting is d3d disabled and ddraw is enabled
    // Unfortunately, with this setting, MS does not call DdGetDriverInfo with
    // the NTPrivateDriverCaps GUID, thus we can not set the flag indicating we want
    // to be notified when primary surface's are created and thus we can't initialize
    // our SURFACEDATA pointer for primary surfaces.  This causes access violations
    // in the driver when a ddraw app is run.  So we must disable ddraw ourselves
    // to avoid the access violation.
    if (GetRegDWORD(ppdev, "CapabilityOverride", &height))
    {
      // bit 1 => if set, ddraw support is disabled by the OS
      // bit 2 => if set, d3d support is disabled by the OS
      if (0x6 & height)
      {
        DISPDBG((0, "  CapabilityOverride=%lXh which disables DDraw and/or D3D", height));

        // MS ain't gonna like this, but this enables DDraw and D3D
        // but only after the system is rebooted or the mode is changed
        // the key ends up being converted from REG_DWORD to REG_BINARY
        height &= ~0x6;

        // regardless of whether or not the following write is successful
        // we must disable ddraw support in this mode
        // By the time this function is called, MS has already decided that
        // ddraw and/or d3d are disabled in the current mode, so if we let
        // ddraw be enabled now, the access violation will occur when a
        // ddraw app is executed.
        // ddraw can only be enabled after a reboot or after a mode change
        // and if CapabilityOverride is zero
        SetRegDWORD(ppdev, "CapabilityOverride", height);
        goto DisableDDraw;
      }
    }
#endif

#if defined(ENABLE_REG_IOCTL_TEST)
    TestRegIoctl(ppdev);
#endif

    //allocate memory for ddglobal
    (LPVOID)_FF(pddglobal) = DDMALLOCZ(sizeof(DDGLOBAL), &_FF(pddglobal));
    if (NULL == (LPVOID)_FF(pddglobal))
      return FALSE;

    //allocate memory for fxglobal
    (LPVOID)_FF(pfxglobal) = DDMALLOCZ(sizeof(fxGlobal), &_FF(pfxglobal));
    if (NULL == (LPVOID)_FF(pfxglobal))
      return FALSE;

#if ENABLE_3D
    // allocate memory for d3global
    (LPVOID)_FF(pd3global) = DDMALLOCZ(sizeof(d3Global), &_FF(pd3global));
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

    {
      ULONG pll, k, m, n;

#if defined(CSIM)
      pll = ((SstIORegs *)ppdev->CSIM_pjBase)->pllCtrl1;
#else
      pll = GET(ghwIO->pllCtrl1);
#endif
      k =  (pll & SST_PLL_K) >> SST_PLL_K_SHIFT;
      m =  (pll & SST_PLL_M) >> SST_PLL_M_SHIFT;
      n =  (pll & SST_PLL_N) >> SST_PLL_N_SHIFT;

      // from avenger_spec.doc, chapter 9
      // memclk = 14.31818 * (n + 2) / (m + 2) / (2 ^ k)
      _FF(memclk) = (1431818 * (n + 2) / (m + 2) / (1 << k)) / 100000;
      DISPDBG((0, "  pllCtrl1 = %lXh, memclk = %ld", pll, _FF(memclk)));
    }

#ifdef SLI_AA
    GetSLIAAInfo(ppdev);
#else
    _FF(dwNumUnits) = 1;
#endif

    if (IS_NAPALM)
      _FF(ModeUses2PixPerClkRender) = SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;

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
    DDFREE((LPVOID)_FF(pddglobal));
    _FF(pddglobal) = 0;
  }

  //free memory for fxglobal
  if (_FF(pfxglobal))
  {
    DDFREE((LPVOID)_FF(pfxglobal));
    _FF(pfxglobal) = 0;
  }

#if ENABLE_3D
  if (_FF(pd3global))
  {
    // release allocations made inD3DHalCreateDriver()

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
// for DX7, we'll copy the context info between pdev's in DrvResetPDEV
#else
#if RC_LINKED_LIST
    while (_D3(pContexts))
    {
      CONTEXT_FREE(ppdev, (DWORD)_D3(pContexts));
    }
#else
    // free _D3(contexts)
    if (_D3(contexts))
      D3DFREE(_D3(contexts));

    // free _D3(contextMap)
    if (_D3(contextMap))
      D3DFREE(_D3(contextMap));
#endif  // RC_LINKED_LIST
#endif  // DX7

    // free _D3(txtrDesc)
    if (_D3(txtrDesc))
      D3DFREE(_D3(txtrDesc));

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
// for DX7, the txtrHndl array are in the HNDLLIST's
#else
    // free _D3(txtrHndl)
    if (_D3(txtrHndl))
      D3DFREE(_D3(txtrHndl));
#endif

    // free _D3(buffer)
    if (_D3(buffer))
      D3DFREE(_D3(buffer));

    //free memory for d3global
    DDFREE((LPVOID)_FF(pd3global));
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
                       DISPDBG((0, "GetDriverInfo: %s structure size mismatch", #x));
#else
#define CHECKSIZE(x) ((void)0)
#endif

  lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;

#if ENABLE_3D
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

    Misc2Callbacks.dwSize = dwSize;
    Misc2Callbacks.dwFlags = 0
                           | DDHAL_MISC2CB32_GETDRIVERSTATE
                           | DDHAL_MISC2CB32_CREATESURFACEEX
                           | DDHAL_MISC2CB32_DESTROYDDLOCAL
                           ;

    Misc2Callbacks.GetDriverState  = ddiGetDriverState;
    Misc2Callbacks.CreateSurfaceEx = ddiCreateSurfaceEx;
    Misc2Callbacks.DestroyDDLocal  = ddiDestroyDDLocal;

    memcpy(lpInput->lpvData, &Misc2Callbacks, dwSize);
    lpInput->ddRVal = DD_OK;
  }
#else
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
#endif
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DParseUnknownCommandCallback))
  {
    DISPDBG((0, "Get D3DParseUnknownCommandCallback"));

    ppdev->pD3DParseUnknownCommand = lpInput->lpvData;

    ASSERTDD((ppdev->pD3DParseUnknownCommand),
             "D3D DX6 ParseUnknownCommand callback == NULL");

    lpInput->ddRVal = DD_OK;
  }
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_MiscellaneousCallbacks))
  {
    extern DWORD __stdcall DdGetAvailDriverMemory( PDD_GETAVAILDRIVERMEMORYDATA pgadm );

    DD_MISCELLANEOUSCALLBACKS  MiscCallbacks;


    DISPDBG((0, "Get Miscellaneous Callbacks"));
    memset(&MiscCallbacks, 0, sizeof(MiscCallbacks));
    dwSize = min(lpInput->dwExpectedSize, sizeof(DD_MISCELLANEOUSCALLBACKS));

    lpInput->dwActualSize = sizeof(DD_MISCELLANEOUSCALLBACKS);

    MiscCallbacks.dwSize  = dwSize;
    MiscCallbacks.dwFlags = 0
                          | DDHAL_MISCCB32_GETAVAILDRIVERMEMORY
                          ;
    MiscCallbacks.GetAvailDriverMemory = DdGetAvailDriverMemory;

    memcpy(lpInput->lpvData, &MiscCallbacks, dwSize);

    lpInput->ddRVal = DD_OK;
  }
#endif
   else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DExtendedCaps) )
    {
      D3DHAL_D3DEXTENDEDCAPS D3DExtendedCaps;

      BuildD3DExtendedCaps(ppdev, &D3DExtendedCaps);
      dwSize = min(lpInput->dwExpectedSize, sizeof(D3DHAL_D3DEXTENDEDCAPS));
      lpInput->dwActualSize                 = sizeof(D3DHAL_D3DEXTENDEDCAPS);
      memcpy(lpInput->lpvData, &D3DExtendedCaps, dwSize);
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
      ddZBufPixelFormat[2].dwZBufferBitDepth = 32;
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
#if USE_NT5_DDMEMMGR
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_NTPrivateDriverCaps) )
  {
    DD_NTPRIVATEDRIVERCAPS ddPrivateCaps;


    DISPDBG((0, "Get NTPrivateDriverCaps"));

    memset(&ddPrivateCaps, 0, sizeof(ddPrivateCaps));

    dwSize = min(lpInput->dwExpectedSize, sizeof(ddPrivateCaps));
    lpInput->dwActualSize = sizeof(ddPrivateCaps);

    ddPrivateCaps.dwSize = dwSize;
    ddPrivateCaps.dwPrivateCaps = 0
// hack for MS's mispelled define in the bld 1963 DDK
// (and probably other versions of the W2K DDK as well)
#ifdef DDHAL_PRIVATECAP_AUTOMICSURFACECREATION
                                | DDHAL_PRIVATECAP_AUTOMICSURFACECREATION
#else
                                | DDHAL_PRIVATECAP_ATOMICSURFACECREATION
#endif
                                | DDHAL_PRIVATECAP_NOTIFYPRIMARYCREATION
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
#if ENABLE_VIDEOPORT
  else if (IsEqualIID(&lpInput->guidInfo, &GUID_VideoPortCallbacks))
  {
        DDHAL_DDVIDEOPORTCALLBACKS vpCallbacks;
        memset(&vpCallbacks, 0, sizeof(vpCallbacks));

    	_DD(bVideoPortActive) = FALSE;
		_DD(bUseWDMScaling) = TRUE;
		_DD(bIgnoreWDM) = FALSE;
	
		{  //V3TV VMIPLD support
           DWORD val = VMIPLD_MODE_USE_GUID;
 		  _DD(bVMIPLDinUse) = FALSE;
		  _DD(bVMIPLDpresent) = FALSE;
          DD_PLD656_Init(ppdev);
	  	  _DD(bVMIPLDpresent) = DD_VMI_Available(ppdev); 

          if (_DD(bVMIPLDpresent))
          {
            if (val == VMIPLD_MODE_PASSTHRU)
            {
               DD_VMI_SetCommand( ppdev, VMI_PASSTHRU); // passthru, no vbi int, no crop vbi
               _DD(bVMIPLDinUse) = FALSE;
            }
            else  // (val == VMIPLD_MODE_656)
            {
               DD_VMI_SetCommand( ppdev, VMI_VBICROP); // not passthru, no vbi int, crop vbi
               _DD(bVMIPLDinUse) = TRUE;
            }
            DD_VMI_SetVbiMax(ppdev, 0x1f);
            DD_VMI_SetVidMax(ppdev, 0x3ff);
          }
		}

        dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDVIDEOPORTCALLBACKS));
        lpInput->dwActualSize = sizeof(DDHAL_DDVIDEOPORTCALLBACKS);
        vpCallbacks.dwSize = dwSize;

        vpCallbacks.dwFlags =  DDHAL_VPORT32_CANCREATEVIDEOPORT |
                               DDHAL_VPORT32_CREATEVIDEOPORT    |
                               DDHAL_VPORT32_FLIP               |
                               DDHAL_VPORT32_GETBANDWIDTH       |
                               DDHAL_VPORT32_GETINPUTFORMATS    |
                               DDHAL_VPORT32_GETOUTPUTFORMATS   |
                               DDHAL_VPORT32_GETFIELD           |
                             //DDHAL_VPORT32_GETLINE            |
                               DDHAL_VPORT32_GETCONNECT         |
                               DDHAL_VPORT32_DESTROY            |
                               DDHAL_VPORT32_GETFLIPSTATUS      |
                               DDHAL_VPORT32_UPDATE             |
                               DDHAL_VPORT32_WAITFORSYNC        |
                               DDHAL_VPORT32_GETSIGNALSTATUS    |
                             //DDHAL_VPORT32_COLORCONTROL       |
                               0;

        vpCallbacks.CanCreateVideoPort = vpCanCreateVideoPort32;
        vpCallbacks.CreateVideoPort = vpCreateVideoPort32;
        vpCallbacks.FlipVideoPort = vpFlipVideoPort32;
        vpCallbacks.GetVideoPortBandwidth = vpGetVideoPortBandwidth32;
        vpCallbacks.GetVideoPortInputFormats = vpGetVideoPortInputFormats32;
        vpCallbacks.GetVideoPortOutputFormats = vpGetVideoPortOutputFormats32;
        vpCallbacks.GetVideoPortField = vpGetVideoPortField32;
        //vpCallbacks.GetVideoPortLine = vpGetVideoPortLine32;
        vpCallbacks.GetVideoPortConnectInfo = vpGetVideoPortConnectInfo32;
        vpCallbacks.DestroyVideoPort = vpDestroyVideoPort32;
        vpCallbacks.GetVideoPortFlipStatus = vpGetVideoPortFlipstatus32;
        vpCallbacks.UpdateVideoPort = vpUpdateVideoPort32;
        vpCallbacks.WaitForVideoPortSync = vpWaitForVideoPortSync32;
        vpCallbacks.GetVideoSignalStatus = vpGetVideoSignalStatus32;
        //vpCallbacks.ColorControl = vpColorControl32;

        memcpy(lpInput->lpvData, &vpCallbacks, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_VideoPortCaps))
    {
        DDVIDEOPORTCAPS vpCaps;
        memset(&vpCaps, 0, sizeof(vpCaps));

        dwSize = min(lpInput->dwExpectedSize, sizeof(DDVIDEOPORTCAPS));
        lpInput->dwActualSize = sizeof(DDVIDEOPORTCAPS);
        vpCaps.dwSize = dwSize;           // size of the DDVIDEOPORTCAPS structure
        vpCaps.dwFlags = DDVPD_WIDTH   |  // indicates which fields contain data
                         DDVPD_HEIGHT  |
                         DDVPD_ID      |
                         DDVPD_CAPS    |
                         DDVPD_FX      |
                         DDVPD_AUTOFLIP|
                       //DDVPD_ALIGN   |
                         0;

        vpCaps.dwMaxWidth = H3_MAX_VID_IN_X; // max width of the video port field
        vpCaps.dwMaxVBIWidth = H3_MAX_VID_IN_X;   // max width of the VBI data
        vpCaps.dwMaxHeight = H3_MAX_VID_IN_Y;// max height of the video port field
        vpCaps.dwVideoPortID = H3_PORT_ID;   // Video port ID (0 - (dwMaxVideoPorts -1))
        vpCaps.dwCaps = DDVPCAPS_AUTOFLIP |  // Video port capabilities
                        DDVPCAPS_INTERLACED |
                        DDVPCAPS_NONINTERLACED |
                        DDVPCAPS_READBACKFIELD |
                      //DDVPCAPS_READBACKLINE  |
                      //DDVPCAPS_SHAREABLE     |
                      //DDVPCAPS_SKIPEVENFIELDS|  //H3 does not have hardware caps to skip field
                      //DDVPCAPS_SKIPODDFIELDS |
                      //DDVPCAPS_SYNCMASTER    |  //Genlock CRTC VSync with video in VSync
                      //DDVPCAPS_COLORCONTROL  |  //Color control operation cap on the Video Port
                      //DDVPCAPS_SYSTEMMEMORY  |
                        DDVPCAPS_VBISURFACE    |
                        DDVPCAPS_OVERSAMPLEDVBI|
                        DDVPCAPS_VBIANDVIDEOINDEPENDENT |
                        0;

        vpCaps.dwFX =   DDVPFX_CROPTOPDATA |   // More video port capabilities
                        //DDVPFX_CROPX |
                        //DDVPFX_CROPY |
                        DDVPFX_INTERLEAVE |
                        //DDVPFX_MIRRORLEFTRIGHT |
                        //DDVPFX_MIRRORUPDOWN    |
                        DDVPFX_PRESHRINKX |
                        DDVPFX_PRESHRINKY |
                        //DDVPFX_PRESHRINKXB |
                        //DDVPFX_PRESHRINKYB |
                        //DDVPFX_PRESHRINKXS |
                        //DDVPFX_PRESHRINKYS |
                        //DDVPFX_PRESTRETCHX |
                        //DDVPFX_PRESTRETCHY |
                        //DDVPFX_PRESTRETCHXN|
                        //DDVPFX_PRESTRETCHYN|
                          DDVPFX_VBICONVERT  |
                          DDVPFX_VBINOSCALE  |
                          DDVPFX_IGNOREVBIXCROP |
                          DDVPFX_VBINOINTERLEAVE|
                        0;

        vpCaps.dwNumAutoFlipSurfaces = 3;    // Number of autoflippable surfaces
        vpCaps.dwAlignVideoPortBoundary = 0; // Byte restriction of placement within the surface
        vpCaps.dwAlignVideoPortPrescaleWidth = 0;// Byte restriction of width after prescaling
        vpCaps.dwAlignVideoPortCropBoundary = 0; // Byte restriction of left cropping
        vpCaps.dwAlignVideoPortCropWidth = 0;    // Byte restriction of cropping width
        vpCaps.dwNumVBIAutoFlipSurfaces = 3;     // Number of VBI autoflippable surfaces
        memcpy(lpInput->lpvData, &vpCaps, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_KernelCallbacks))
    {
         DDHAL_DDKERNELCALLBACKS kCallbacks;

         memset(&kCallbacks, 0, sizeof(kCallbacks));
         dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDKERNELCALLBACKS));
         lpInput->dwActualSize = sizeof(DDHAL_DDKERNELCALLBACKS);
         kCallbacks.dwSize = dwSize;               // size of the DDHAL_DDKERNELCALLBACKS structure
         kCallbacks.dwFlags =
#if defined KMVP || (_WIN32_WINNT >= 0x0500)
                              DDHAL_KERNEL_SYNCSURFACEDATA |
                              DDHAL_KERNEL_SYNCVIDEOPORTDATA |
#endif
                              0;
#if defined KMVP || (_WIN32_WINNT >= 0x0500)
         kCallbacks.SyncSurfaceData = kSyncSurfaceData32;
         kCallbacks.SyncVideoPortData = kSyncVideoportData32;
#endif
         memcpy(lpInput->lpvData, &kCallbacks, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_KernelCaps))
    {
        DDKERNELCAPS kCaps;

        memset(&kCaps, 0, sizeof(kCaps));
        dwSize = min(lpInput->dwExpectedSize, sizeof(DDKERNELCAPS));
        lpInput->dwActualSize = sizeof(DDKERNELCAPS);
        kCaps.dwSize = dwSize;                 // size of the DDKERNELCAPS structure
        kCaps.dwCaps =  DDKERNELCAPS_SKIPFIELDS |        //Skip odd or even fields by using hw or vddSkipNextField
                        DDKERNELCAPS_AUTOFLIP |        //Support vddFlipVideoPort and vddFlipOverlay
                        DDKERNELCAPS_SETSTATE |        //Support vddSetState to switch between BOB & WEAVE
                        DDKERNELCAPS_LOCK     |        //Support vddLock fb access w/ contention w/ blitter
                        DDKERNELCAPS_FLIPVIDEOPORT |     //Support vddFlipVideoPort
                        //DDKERNELCAPS_FLIPOVERLAY |        //Support vddFlipOverlay
                        DDKERNELCAPS_CAPTURE_SYSMEM |     //Kernel support capture to system memory
                        //DDKERNELCAPS_CAPTURE_NONLOCALVIDMEM | //Kernel support capture to non local video memory
                        DDKERNELCAPS_FIELDPOLARITY |    //Can report polarity of video field
                        0;
        kCaps.dwIRQCaps = DDIRQ_DISPLAY_VSYNC |        //Device generates display VSync
                          DDIRQ_VPORT0_VSYNC  |        //Device generates video VSync on the Video Port 0
                        0;

        memcpy(lpInput->lpvData, &kCaps, dwSize);
        lpInput->ddRVal = DD_OK;
    }

#endif

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
#if ENABLE_LOG_FILE
    retroLogForce(ppdev, "retro3dfx EXCL-ENTER: tiledHeapSz=%ld primInTile=%ld 3dCnt=%ld\r\n",
                  _FF(ddTiledHeapSize), (LONG)_DS(ddPrimaryInTile), _FF(dd3DSurfaceCount));
#endif
    _DS(ddExclusiveMode) = TRUE;
    hwcSetContextDWORD();
    // remove all GDI device bitmaps from video memory here
    bMoveAllDfbsFromOffscreenToDibs(ppdev);
  }
  else
  {
    DISPDBG((DEBUG_APIENTRY, "DdSetExclusiveMode (leaving)"));
#if ENABLE_LOG_FILE
    retroLogForce(ppdev, "retro3dfx EXCL-LEAVE: 3dCnt=%ld\r\n", _FF(dd3DSurfaceCount));
#endif
    _DS(ddExclusiveMode) = FALSE;
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

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
/****************************************************************************
*
* FUNCTION:     DdGetAvailDriverMemory()
*
* DESCRIPTION:
*
****************************************************************************/

DWORD __stdcall
DdGetAvailDriverMemory(LPDDHAL_GETAVAILDRIVERMEMORYDATA pgadm)
{
  DD_ENTRY_SETUP(pgadm->lpDD);


  DDPRINT(DDDBGLVL, ">> DdGetAvailDriverMemory (ddsCaps.dwCaps=%8lXh", pgadm->DDSCaps.dwCaps);

  pgadm->dwTotal = 0;
  pgadm->dwFree  = 0;

  if ((IS_NAPALM) && _FF(bUseSliExtraLinearHeap))
  {
    memMgr_checkHeapStatus(ppdev);
  
    if (DDSCAPS_NONLOCALVIDMEM & pgadm->DDSCaps.dwCaps)
    {
      pgadm->ddRVal = DDERR_CURRENTLYNOTAVAIL;
  
      DDPRINT(DDDBGLVL, "  nonlocalvidmem not handled");
      DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pgadm->ddRVal);
  
      return DDHAL_DRIVER_NOTHANDLED;
    }
    else
    {
      // if we're in sli mode with aa disabled
      if ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
          (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)))
      {
        if (DDSCAPS_TEXTURE & pgadm->DDSCaps.dwCaps)
        {
          // don't do anything
        }
        else // this catches ((0 == DDSCap.dwCaps) || ((DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM) & pgadm->DDSCaps.dwCaps))
        {
        }
  
        DDPRINT(DDDBGLVL, "  (in sli mode) returning dwTotal=%ld, dwFree=%ld", pgadm->dwTotal, pgadm->dwFree);
      }
      // for all other sli/aa modes or for single chip mode
      // subtract sli extra linear memory and sli tiled heap
      else
      {
        if (DDSCAPS_TEXTURE & pgadm->DDSCaps.dwCaps)
        {
          pgadm->dwTotal = (unsigned long) -((long)(_FF(sliTileCompare) - _FF(ddTiledHeapStart)));
          //pgadm->dwFree  = (unsigned long) -((long)(_FF(sliTileCompare) - _FF(ddTiledHeapStart)));
        }
        else // this catches ((0 == DDSCap.dwCaps) || ((DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM) & pgadm->DDSCaps.dwCaps))
        {
          pgadm->dwTotal = (unsigned long) -((long)(_FF(sliTileCompare) - _FF(ddTiledHeapStart)));
          //pgadm->dwFree  = (unsigned long) -((long)(_FF(sliTileCompare) - _FF(ddTiledHeapStart)));
        }
  
        DDPRINT(DDDBGLVL, "  (not in sli mode) returning dwTotal=%ld, dwFree=%ld", pgadm->dwTotal, pgadm->dwFree);
      }
    }
  }

  pgadm->ddRVal = DD_OK;

  DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pgadm->ddRVal);

  return DDHAL_DRIVER_HANDLED;
} // DdGetAvailDriverMemory
#endif

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
* FUNCTION:     GetRegDWORD()
*
* DESCRIPTION:  calls miniport to retrieve a REG_DWORD value from the registry
*
****************************************************************************/

BOOL
GetRegDWORD( PDEV *ppdev, const char *varname, ULONG *pValue )
{
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
    memcpy(pValue, queryValueInfo.Data, sizeof(ULONG));
    return TRUE;
  }

  // miniport didn't find key
  return FALSE;
}

/****************************************************************************
*
* FUNCTION:     SetRegDWORD()
*
* DESCRIPTION:  calls miniport to write a REG_DWORD value to the registry
*
****************************************************************************/

BOOL
SetRegDWORD( PDEV *ppdev, const char *varname, ULONG Value )
{
  TDFX_SET_VALUE_INFO   setValueInfo;
  DWORD                 numBytes;


  // initialize setValueInfo
  setValueInfo.DataLength = sizeof(ULONG);
  strcpy(setValueInfo.ValueName, varname);
  setValueInfo.ValueNameLength = strlen(varname) + 1;
  setValueInfo.Type = REG_DWORD;
  memcpy(setValueInfo.Data, &Value, sizeof(ULONG));

  // call ioctl to have miniport write data to registry
  if (! EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_SET_REGISTRY_VALUE,
                           &setValueInfo,
                           sizeof(setValueInfo),
                           NULL,
                           0,
                           &numBytes))
  {
    return TRUE;
  }

  // miniport didn't write key
  return FALSE;
}

/****************************************************************************
*
* FUNCTION:     SetRegSZ()
*
* DESCRIPTION:  calls miniport to write a REG_SZ value to the registry
*
****************************************************************************/

BOOL
SetRegSZ( PDEV *ppdev, const char *varname, const char *pValue )
{
  TDFX_SET_VALUE_INFO   setValueInfo;
  DWORD                 numBytes;
  DWORD                 dataLenght;


  // initialize setValueInfo
  setValueInfo.DataLength = dataLenght = strlen(pValue) + 1;
  strcpy(setValueInfo.ValueName, varname);
  setValueInfo.ValueNameLength = strlen(varname) + 1;
  setValueInfo.Type = REG_SZ;
  memcpy(setValueInfo.Data, pValue, dataLenght);

  // call ioctl to have miniport write data to registry
  if (! EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_SET_REGISTRY_VALUE,
                           &setValueInfo,
                           sizeof(setValueInfo),
                           NULL,
                           0,
                           &numBytes))
  {
    return TRUE;
  }

  // miniport didn't write key
  return FALSE;
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

/*----------------------------------------------------------------------
Function name: ddatof

Description:

Return:        double
----------------------------------------------------------------------*/
double ddatof( const char *s )
{
   float left, right, sign, mult;
   int i;

   for( i=0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
     ;

   if (s[i] == '+' || s[i] == '-')
     sign = (s[i++] == '+') ? 1.0f : -1.0f;
   else
     sign = 1.0f;

   for (left = 0.0f; s[i] >= '0' && s[i] <= '9'; i++)
     left = (left * 10.0f) + (s[i] - '0');

   if (s[i++] != '.')
     return( sign * left );

   for (mult = 0.1f, right = 0.0f; s[i] >= '0' && s[i] <= '9'; i++, mult /= 10.0f)
     right = right + (mult * (s[i] - '0'));

   return( sign * (left + right) );

   // return( atof(s) );     // Tests the runtime library
}// ddatof

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

/*----------------------------------------------------------------------
Function name: LoadGamma

Description:   Load D3D gamma ramp

Return:        nothing
----------------------------------------------------------------------*/

#define DEFAULT_D3D_GAMMA   1.30f
#define MIN_D3D_GAMMA       0.43f

void
LoadGamma(NT9XDEVICEDATA *ppdev)
{
  LPSTR   lpStr;
  float   rgamma,ggamma,bgamma;
  float   rg1, gg1, bg1;
  float   fval;
  int     ir, ig, ib;
  int     i;


  if ((2 == ppdev->cjPelSize) || ((IS_NAPALM) && (4 == ppdev->cjPelSize)))
  {
    // grab red gamma value from registry
    lpStr = GETENV(RED_NAME);
    if (NULL != lpStr)
      rgamma = (float)ddatof(lpStr);
    else
      rgamma = DEFAULT_D3D_GAMMA;

    if (rgamma < MIN_D3D_GAMMA)
      rgamma = MIN_D3D_GAMMA;

    // grab green gamma value from registry
    lpStr = GETENV(GREEN_NAME);
    if (NULL != lpStr)
      ggamma = (float)ddatof(lpStr);
    else
      ggamma = DEFAULT_D3D_GAMMA;

    if (ggamma < MIN_D3D_GAMMA)
      ggamma = MIN_D3D_GAMMA;

    // grab blue gamma value from registry
    lpStr = GETENV(BLUE_NAME);
    if (NULL != lpStr)
      bgamma = (float)ddatof(lpStr);
    else
      bgamma = DEFAULT_D3D_GAMMA;

    if (bgamma < MIN_D3D_GAMMA)
      bgamma = MIN_D3D_GAMMA;

    rg1 = 1.0f/rgamma;
    gg1 = 1.0f/ggamma;
    bg1 = 1.0f/bgamma;

    for (i = 0; i < 256; i++)
    {
      fval = (float)log((float)i/255.0f);

      ir = (int)(exp(fval * rg1) * 255.0 + 0.5f);
      ig = (int)(exp(fval * gg1) * 255.0 + 0.5f);
      ib = (int)(exp(fval * bg1) * 255.0 + 0.5f);

      ppdev->TransientGammaTable[i] = (ir << 16) | (ig << 8) | ib;
    }

#ifdef DBG
    //DISPDBG((3, " RedGamma=%f, GreenGamma=%f, BlueGamma=%f", rgamma, ggamma, bgamma);
    DISPDBG((3, "D3D Gamma Table"));
    for (i = 0; i < 256; i += 8)
    {
      DISPDBG((3,"  %8lXh  %8lXh  %8lXh  %8lXh  %8lXh  %8lXh  %8lXh  %8lXh",
               ppdev->TransientGammaTable[i],
               ppdev->TransientGammaTable[i+1],
               ppdev->TransientGammaTable[i+2],
               ppdev->TransientGammaTable[i+3],
               ppdev->TransientGammaTable[i+4],
               ppdev->TransientGammaTable[i+5],
               ppdev->TransientGammaTable[i+6],
               ppdev->TransientGammaTable[i+7]));
    }
#endif

    HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_TRANSIENT);
  }
} // LoadGamma

/*----------------------------------------------------------------------
Function name: RestoreGamma

Description:   Restore desktop gamma ramp

Return:        nothing
----------------------------------------------------------------------*/

#define RestoreGamma(ppdev)   HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_DESKTOP);

/*----------------------------------------------------------------------
Function name: Enter_3DApplication

Description:   Entering a 3D, exclusive-mode, full-screen application.

Return:        DD_OK
----------------------------------------------------------------------*/

DWORD Enter_3DApplication(NT9XDEVICEDATA * ppdev)
{
#if ENABLE_LOG_FILE
  retroLogForce(ppdev, "retro3dfx ENTER-3DAPP: scr=%ldx%ldx%ld primInTile=%ld tiledHeapSz=%ld\r\n",
                (LONG)ppdev->cxScreen, (LONG)ppdev->cyScreen,
                (LONG)(ppdev->cjPelSize * 8), (LONG)_DS(ddPrimaryInTile),
                _FF(ddTiledHeapSize));
#endif

  /* Disable video display. */
  if (IS_NAPALM)
  {
    DWORD vidProcCfg = GET(ghwIO->vidProcCfg);
    vidProcCfg &= ~SST_VIDEO_PROCESSOR_EN;
    SETDW(ghwIO->vidProcCfg, vidProcCfg);
  }

#ifdef SLI_AA
  /* Enable SLI if multiple chips are present, and DISABLE_SLI is not  */
  /* set in the registry.  Enable AA if the application requests by    */
  /* setting the anti-aliasing hint during surface creation, or if     */
  /* ENABLE_AA is set in the registry.  In both cases, AA will only be */
  /* enabled if enough buffers can be allocated.  -CGW-                */
  if (IS_NAPALM)
    Promote_DeviceToSLIAA(ppdev);
#endif // SLI_AA

#if ENABLE_TILED_HEAP && !defined(CSIM)
  /* Voodoo3/Napalm have separate mechanisms for flipping the desktop  */
  /* and video overlay surfaces.  Desktop flipping can't be pipelined, */
  /* so the video overlay is borrowed to display the primary flipping  */
  /* chain for 3D, exclusive-mode, full-screen applications.  -CGW-    */
  Promote_PrimaryToOverlay(ppdev);
#endif // ENABLE_TILED_HEAP

#if 1
// W2K shouldn't need this
// we generally get colorfill blt calls to DdBlt for the primary and
// backbuffers from the OS before the app does anything
#ifdef SLI_AA_2D
#ifdef ENABLE_TILED_HEAP
  /* Napalm needs to clear the secondary AA buffers, when enabling AA  */
  /* rendering, and the primary surface when enabling SLI mode. -CGW-  */
  if (IS_NAPALM && 
      (SINGLE_CHIP_NOSLI_AA_DISABLED != _DD(ddSLIAAConfiguration)))
  {
    Clear_SLIAA_Buffers (ppdev);
  }
#endif // ENABLE_TILED_HEAP
#endif
#endif

#ifdef AGP_CMDFIFO
  /* Voodoo3/Napalm promote to using the AGP command FIFO, to improve  */
  /* the performance of 3D, exclusive-mode, full-screen applications.  */
  /* Voodoo3 disables this function because of hardware problems, but  */
  /* it should be enabled on Napalm.  -CGW-                            */
  if (IS_NAPALM)
    Promote_CmdFifoToAGP(ppdev);
#endif // AGP_CMDFIFO

  // load d3d gamma
  LoadGamma(ppdev);

  // Dirty the D3D context, to force D3D to setup registers
  // for both chips, since we may have entered SLI mode

  _D3(lastContext) = 0;

  DUMP_H3_REGS(ppdev, 3);

  return DD_OK;
}

/*----------------------------------------------------------------------
Function name: Exit_3DApplication

Description:   Exiting a 3D, exclusive-mode, full-screen application.

Return:        DD_OK
----------------------------------------------------------------------*/

DWORD Exit_3DApplication(NT9XDEVICEDATA * ppdev)
{
#if ENABLE_LOG_FILE
  retroLogForce(ppdev, "retro3dfx EXIT-3DAPP: 3dCnt=%ld\r\n", _FF(dd3DSurfaceCount));
#endif

  /* Disable video display. */
  if (IS_NAPALM)
  {
    DWORD vidProcCfg = GET(ghwIO->vidProcCfg);
    vidProcCfg &= ~SST_VIDEO_PROCESSOR_EN;
    SETDW(ghwIO->vidProcCfg, vidProcCfg);
  }

#ifdef SLI_AA
  /* Disable SLI and AA setup. */
  if (IS_NAPALM)
    Demote_DeviceFromSLIAA(ppdev);
#endif // SLI_AA

#if ENABLE_TILED_HEAP && !defined(CSIM)
  /* Demote from video overlay. */
  Demote_PrimaryFromOverlay(ppdev);
#endif // ENABLE_TILED_HEAP

#ifdef AGP_CMDFIFO
  /* Disable AGP command FIFO. */
  if (IS_NAPALM)
    Demote_CmdFifoFromAGP(ppdev);
#endif

  // restore desktop gamma
  RestoreGamma(ppdev);

  // Dirty the D3D context, to force D3D to setup registers
  // for both chips, since we may have entered SLI mode

  _D3(lastContext) = 0;

  DUMP_H3_REGS(ppdev, 3);

  return DD_OK;
}

#if ENABLE_TILED_HEAP && !defined(CSIM)
// for whatever reason the demote code in DestroySurface is wrapped
// in #if ENABLE_TILED_HEAP and the promote code in the win9x portion of CreateSurface
// is wrapped in #if ENABLE_3D, so I'll wrap the code here in ENABLE_TILED_HEAP
// since I don't want to mess with the DestroySurface code

/*----------------------------------------------------------------------
Function name: Promote_PrimaryToOverlay

Description:   Display primary using overlay.

Return:        DD_OK
----------------------------------------------------------------------*/

DWORD Promote_PrimaryToOverlay(NT9XDEVICEDATA * ppdev)
{
  DWORD vidProcCfg, vidOverlayStartAddr, vidOverlayEndAddr, vidDesktopOverlayStride;
  DWORD vidScreenSize;
  FXSURFACEDATA *surfaceData;

  // this isn't all that useful anymore
  // particularly on napalm where sli requires overlay promotion
#if 0
  DWORD enableOverlayPromotion;

  // default to overlayPromotion enabled
  // but allow registry key to override
  if (GetRegDWORD(ppdev, "OverlayPromotion", &enableOverlayPromotion))
  {
    if (0 == enableOverlayPromotion)
    {
      DISPDBG((0, "  OverlayPromotion disabled in registry, skipping promotion of primary"));
      return DD_OK;
    }
  }
#if 0
  else
  {
    DISPDBG((0, "  defaulting to OverlayPromotion disabled, skipping promotion of primary"));
    return DD_OK;
  }
#endif
#endif

  // vidProcCfg
  vidProcCfg = GET(ghwIO->vidProcCfg);
  vidProcCfg &= ~(SST_DESKTOP_EN                |  // Disable desktop
                  SST_OVERLAY_STEREO_EN         |  // Disable stereo
                  SST_INTERLACED_EN             |  // No interlace output
                  SST_CHROMA_EN                 |  // Disable chromakey
                  SST_CHROMA_INVERT             |  // Disable chromakey invert
                  SST_VIDEOIN_AS_OVERLAY        |  // Disable autoflipping overlay
                  SST_OVERLAY_CLUT_BYPASS       |  // No overlay clut bypass
                  SST_OVERLAY_CLUT_SELECT       |  // No overlay clut select
                  SST_OVERLAY_HORIZ_SCALE_EN    |  // Disable horizontal scaling
                  SST_OVERLAY_VERT_SCALE_EN     |  // Disable vertical scaling
                  SST_OVERLAY_FILTER_MODE       |  // Disable bilinear scaling
                  SST_OVERLAY_PIXEL_FORMAT      |  // Clear pixel format
                  SST_OVERLAY_TILED_EN          |  // Select overlay linear space
                  SST_OVERLAY_DEINTERLACE_EN    |  // Disable backend deinterlace
                  0 );
	
  vidProcCfg |=  (SST_OVERLAY_EN                |  // Enable overlay
                  SST_OVERLAY_FILTER_2X2);         // Enable overlay filtering

  // From Scott Sellers
  // vidProcCfg[23:21] should be set to RGB565U for AA modes
  // since the desktop and the overlay need to have the same mode
  if (IS_NAPALM && (4 == ppdev->cjPelSize))
    vidProcCfg |=  SST_OVERLAY_PIXEL_RGB32U;       // Select 888 format
  else if (_DD(ddAAModeRequested))
    vidProcCfg |=  SST_OVERLAY_PIXEL_RGB565U;      // Select 565U format
  else
    vidProcCfg |=  SST_OVERLAY_PIXEL_RGB565D;      // Select 565 format
  if (ppdev->ulScreenOffset & SSTG_IS_TILED)
  {
    vidProcCfg |= SST_OVERLAY_TILED_EN;
  }
  if (SST_HALF_MODE & vidProcCfg)
  {
    #define STRETCH_BY_TWO  0x00080000

    vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;
    SETDW(ghwIO->vidOverlayDvdy, STRETCH_BY_TWO);
    SETDW(ghwIO->vidOverlayDvdyOffset, 0);
  }

  // Antialiasing specific code
  if (IS_NAPALM)
  {
    if (_DD(ddAAModeRequested))
    {
      vidProcCfg |= (SST_DESKTOP_EN | SST_CHROMA_EN);
      vidScreenSize = GET(ghwIO->vidScreenSize);
      vidScreenSize |= SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE;
      SETDW(ghwIO->vidScreenSize, vidScreenSize);
    }

    // In Enhanced Video Mode AA or SLI then we need to
    // set this register to have hsync in the middle of the line
    // Scott says to set this to the max!!!!!
    if (_DD(ddAAModeRequested) || _DD(ddSLIModeRequested) || (_DD(ddAANumberSamples) && (_FF(dwNumUnits) > 1)))
      SETDW(ghwIO->vidOverlayDudx, (ppdev->cxScreen >> 1));
//      SETDW(ghwIO->vidOverlayDudx, 0x7FF);
  }
   
  vidProcCfg |= SST_VIDEO_PROCESSOR_EN; // Enable video
  SETDW(ghwIO->vidProcCfg,vidProcCfg);

  // vidDesktopOverlayStride
  vidDesktopOverlayStride = GET(ghwIO->vidDesktopOverlayStride);
  vidDesktopOverlayStride &= ~SST_OVERLAY_LINEAR_STRIDE;
  if (ppdev->ulScreenOffset & SSTG_IS_TILED)
  {
    vidDesktopOverlayStride |= _DS(ddTileStride) << SST_OVERLAY_STRIDE_SHIFT;
    vidDesktopOverlayStride &= ~SST_OVERLAY_OVERFLOW_DUDX_WIDTH;
    vidDesktopOverlayStride |= ((ppdev->cjPelSize * ppdev->cxScreen) & SST_OVERLAY_OVERFLOW_DUDX_BIT) << SST_OVERLAY_OVERFLOW_RELATIVE_SHIFT;
  }
  else
  {
    vidDesktopOverlayStride |= (ppdev->lDelta) << SST_OVERLAY_STRIDE_SHIFT;
  }
  SETDW(ghwIO->vidDesktopOverlayStride, vidDesktopOverlayStride);

  // vidOverlayStartAddr,vidOverlayEndAddr
  GETOVERLAYADDR( 0, 0, vidOverlayStartAddr);
  SETDW(ghwIO->vidOverlayStartCoords, vidOverlayStartAddr);
  if (SST_HALF_MODE & vidProcCfg)
  {
    GETOVERLAYADDR( (ppdev->cxScreen - 1), (2 * ppdev->cyScreen - 1), vidOverlayEndAddr);
  }
  else
  {
    GETOVERLAYADDR( (ppdev->cxScreen - 1), (ppdev->cyScreen - 1), vidOverlayEndAddr);
  }
  SETDW(ghwIO->vidOverlayEndScreenCoord, vidOverlayEndAddr);

  // vidOverlayDudxOffsetSrcWidth
  SETDW(ghwIO->vidOverlayDudxOffsetSrcWidth, ((ppdev->cjPelSize * ppdev->cxScreen) << SST_OVERLAY_FETCH_SIZE_SHIFT));

  // vidMaxRGBDelta
  SETDW(ghwIO->vidMaxRGBDelta,0x1F0F1F);

  // Setup overlay filter mode.
  vidProcCfg = GET(ghwIO->vidProcCfg) & ~SST_OVERLAY_FILTER_MODE;   // clear filter mode bits
  switch (_DD(overlayFilter) & 0x3)
  {
    default:
    case 0:	// Registry key not set or set to zero, use optimal
    case 1:	// Registry key optimal, use 2X2 filter if < 1024x768, else use 4x4 filter
      if(ppdev->cxScreen < 1024)
        vidProcCfg |= SST_OVERLAY_FILTER_2X2;     // set 2x2 mode
      else    	
        vidProcCfg |= SST_OVERLAY_FILTER_4X4;     // set 4x4 mode
      break;
    case 2:	// Registry key normal - Use 4x4 video filter always
      vidProcCfg   |= SST_OVERLAY_FILTER_4X4;     // set 4x4 mode
      break;
    case 3: // Registry key high, use 2X2 video filter always, except when pixel doubled
      if(SST_VIDEO_2X_MODE_EN & vidProcCfg)
        vidProcCfg |= SST_OVERLAY_FILTER_4X4;     // set 4x4 mode
      else    	
        vidProcCfg |= SST_OVERLAY_FILTER_2X2;     // set 2x2 mode
      break;
  }

  // From Scott Sellers
  // vidProcCfg[17:16] should be set to 0 for AA Modes
  // and 0x0 or 0x2 for SLI modes
  if (IS_NAPALM)
  {
    // If AA Mode then we can do no filtering      
    if (_DD(ddAAModeRequested))
    {
      vidProcCfg = vidProcCfg & ~(SST_OVERLAY_FILTER_MODE);
    }

    // If SLI Mode then Filter Can be 4x4 Dither or Point
    if (_DD(ddSLIModeRequested) || (_DD(ddAANumberSamples) && (_FF(dwNumUnits) > 1)))
    {
      DWORD filterMode;
      filterMode = vidProcCfg & (SST_OVERLAY_FILTER_MODE);
      if ((SST_OVERLAY_FILTER_2X2 == filterMode) || (SST_OVERLAY_FILTER_BILINEAR == filterMode))
      {
        vidProcCfg = vidProcCfg & ~(SST_OVERLAY_FILTER_MODE);            
      }            
    }
  }

  SETDW(ghwIO->vidProcCfg,vidProcCfg);

  // Swap to overlay
  surfaceData = (FXSURFACEDATA*) &(_FF(ddPrimarySurfaceData));

  // When AA is enabled, display from both buffers.
  if ((IS_NAPALM) && (_DD(ddAAModeRequested)))
  {
    SETDW(ghw0->leftOverlayBuf,  (surfaceData->hwPtr & ~SSTG_IS_TILED));
    SETDW(ghw0->leftDesktopBuf,  (surfaceData->AAhwPtr & ~SSTG_IS_TILED));
    SETDW(ghw0->swapbufferCMD, SST_SWAP_DESKTOP_EN);
  }
  else
  {
    SETDW(ghw0->leftOverlayBuf,  (surfaceData->hwPtr & ~SSTG_IS_TILED));
    SETDW(ghw0->swapbufferCMD, 0x0);
  }

  _FF(ddVisibleOverlaySurf) = GETPRIMARY;
  _FF(lastOverlayAddress) = INVALID_ADDRESS;
  _DD(dd3DInOverlay) = 1;

  return DD_OK;

} // Promote_PrimaryToOverlay

/*----------------------------------------------------------------------
Function name: Demote_PrimaryFromOverlay

Description:   Switch primary from overlay to desktop.

Return:        DD_OK
----------------------------------------------------------------------*/

DWORD Demote_PrimaryFromOverlay(NT9XDEVICEDATA * ppdev)
{
  DWORD vidProcCfg;
  DWORD vidScreenSize;

  // vidDesktopStartAddr
  FXBUSYWAIT(ppdev);
  SETDW(ghwIO->vidDesktopStartAddr, _DS(gdiDesktopStart));

  // vidProcCfg
  vidProcCfg = GET(ghwIO->vidProcCfg);
  vidProcCfg &= ~(SST_OVERLAY_EN                |  // Disable overlay
                  SST_OVERLAY_STEREO_EN         |  // Disable stereo
                  SST_INTERLACED_EN             |  // No interlace output
                  SST_CHROMA_EN                 |  // Disable chromakey
                  SST_CHROMA_INVERT             |  // Disable chromakey invert
                  SST_VIDEOIN_AS_OVERLAY        |  // Disable autoflipping overlay
                  SST_OVERLAY_CLUT_BYPASS       |  // No overlay clut bypass
                  SST_OVERLAY_CLUT_SELECT       |  // No overlay clut select
                  SST_OVERLAY_HORIZ_SCALE_EN    |  // Disable horizontal scaling
                  SST_OVERLAY_VERT_SCALE_EN     |  // Disable vertical scaling
                  SST_OVERLAY_FILTER_MODE       |  // Disable bilinear scaling
                  SST_OVERLAY_PIXEL_FORMAT      |  // Clear pixel format
                  SST_OVERLAY_TILED_EN          |  // Select overlay linear space
                  SST_OVERLAY_DEINTERLACE_EN    |  // Disable backend deinterlace
                  0 );
	
  vidProcCfg |= SST_DESKTOP_EN |                   // Enable desktop
                SST_VIDEO_PROCESSOR_EN;            // Enable video

  SETDW(ghwIO->vidProcCfg,vidProcCfg);

  // vidScreenSize
  vidScreenSize = GET(ghwIO->vidScreenSize);
  vidScreenSize &= ~SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE;
  SETDW(ghwIO->vidScreenSize, vidScreenSize);

  if (_FF(ddVisibleOverlaySurf) == GETPRIMARY)
  {
    _FF(ddVisibleOverlaySurf) = 0;
  }

  _DD(dd3DInOverlay) = 0;

  return DD_OK;

} // Demote_PrimaryFromOverlay
#endif  // ENABLE_TILED_HEAP

#ifdef SLI_AA

#if 1
// W2K shouldn't need this
// we generally get colorfill blt calls to DdBlt for the primary and
// backbuffers from the OS before the app does anything

/*----------------------------------------------------------------------
Function name: Clear_SLIAA_Buffers

Description:   Clears primary/secondary surfaces for SLI/AA transitions.

Return:        DD_OK
----------------------------------------------------------------------*/

void Clear_SLIAA_Buffers (NT9XDEVICEDATA * ppdev)
{
  SOLIDCOLORPARAMS SolidColorParams; 
  DWORD dstPixelFormat;

  SolidColorParams.fillData       = 0;
  SolidColorParams.fpVidMem       = 0;
  SolidColorParams.Pitch          = 0;
  SolidColorParams.linearPitch    = 0;
  SolidColorParams.dstLeft        = 0;
  SolidColorParams.dstTop         = 0;
  SolidColorParams.dstRight       = ppdev->cxScreen;
  SolidColorParams.dstBottom      = ppdev->cyScreen;
  SolidColorParams.BytesPerPel    = GETPRIMARYBYTEDEPTH;
  SolidColorParams.isTiled        = TRUE;
  SolidColorParams.isZClear       = FALSE;
  SolidColorParams.bltRop         = (SSTG_ROP_SRC << 16) | (SSTG_ROP_SRC << 8) | (SSTG_ROP_SRC);

  GETPIXELFORMAT(SolidColorParams.BytesPerPel, dstPixelFormat);
  BLTFMT(_DS(ddTileStride), dstPixelFormat, SolidColorParams.bltDstFormat);

  if (_DD(ddSLIModeEnabled))
    SolidColorParams.dwSliSurface = TRUE;
  else
    SolidColorParams.dwSliSurface = FALSE;

  SolidColorParams.bltDstBaseAddr = _FF(gdiDesktopStart) | SSTG_IS_TILED;
  DdSli2DSolidColor(ppdev, &SolidColorParams);

  // Clear secondary buffers, if AA enabled.

  if (_DD(ddAAModeEnabled))
  {
    if (_DD(ddSLIModeEnabled))
    {
      SolidColorParams.bltDstBaseAddr = _FF(secondaryThirdBuffer) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
 
      SolidColorParams.bltDstBaseAddr = _FF(secondaryZBuffer) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);

      SolidColorParams.bltDstBaseAddr = _FF(secondaryBackBuffer) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);

      SolidColorParams.bltDstBaseAddr = _FF(secondaryFrontBuffer) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
    }
    else
    {
      SolidColorParams.bltDstBaseAddr = _FF(ddLinearHeap4Start) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
 
      SolidColorParams.bltDstBaseAddr = _FF(ddLinearHeap3Start) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);

      SolidColorParams.bltDstBaseAddr = _FF(ddLinearHeap2Start) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);

      SolidColorParams.bltDstBaseAddr = _FF(ddLinearHeap1Start) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
    }
  }

} // Clear_SLIAA_Buffers
#endif

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

/*----------------------------------------------------------------------
Function name: RetrieveFromRegistry

Description:   Common code to read a integer value from the registry

Return:        default, Min, max or registry value
----------------------------------------------------------------------*/

DWORD RetrieveFromRegistry(NT9XDEVICEDATA *ppdev,
                           const char     *pStr,
                           DWORD          dwDefault,
                           DWORD          dwMin,
                           DWORD          dwMax)
{
  DWORD dwReturn = dwDefault;
  LPSTR lpStr;


  lpStr = GETENV(pStr);
  if (NULL != lpStr)
  {
    dwReturn = atoi(lpStr);
    if (dwReturn < dwMin)
      dwReturn = dwMin;
    if (dwReturn > dwMax)
      dwReturn = dwMax;
  }

  return dwReturn;
}

/*----------------------------------------------------------------------
Function name: GetLog2

Description:   Function to Log 2

Return:        1
----------------------------------------------------------------------*/

DWORD GetLog2(DWORD dwNum)
{
   DWORD dwReturn;

   for (dwReturn=0, dwNum>>=1; dwNum > 0; dwNum>>=1, dwReturn++)
      ;

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  Promote_DeviceToSLIAA

Description:    Enable SLI or AA mode.
----------------------------------------------------------------------*/

DWORD Promote_DeviceToSLIAA(NT9XDEVICEDATA * ppdev)
{
  SLI_AA_REQUEST  Sli_AA_Request;
  DWORD           vidProcCfg;
  DWORD           numBytes;
  DWORD           dwLog2GroupHeight;
  DWORD           dwOverRide;
  DWORD           dwSliBandHeight;


  // If SLI or AA requested, and this is a Napalm
  if (_DD(ddAAModeRequested) || _DD(ddSLIModeRequested) || _DD(ddAANumberSamples))
  {
    // disable cmdfifo on master
    H3_GP_WAIT(ppdev, ppdev->pjBase);   // only need to wait on master cmdfifo
    CmdFifo0Disable(ppdev);

    _DD(ddAAModeEnabled)  = _DD(ddAAModeRequested);
    _DD(ddSLIModeEnabled) = _DD(ddSLIModeRequested);

    if (! (SINGLE_CHIP_NOSLI_AA_DISABLED == _DD(ddSLIAAConfiguration)))
    {
      _FF(ddMultiChipConfig) = TRUE;
    }

    // Disable hardware cursors.
#if 0
    _FF(gdiFlags) |= SDATA_GDIFLAGS_SLI_AA_MASTER;
    SwitchToHostCursor(ppdev);

    // Switch to a Host Centered Cursor
    // Save FB Offsets
    _DD(ddOldHostcursorAndStart) = _FF(HostcursorAndStart);
    _DD(ddOldHostcursorXorStart) = _FF(HostcursorXorStart);
    _DD(ddOldHostcursorSrcStart) = _FF(HostcursorSrcStart);
    _DD(ddOldHostcursorExclusionStart) = _FF(HostcursorExclusionStart);

    // Get Host Based Ones
    _FF(HostcursorAndStart) = _DD(ddHostcursorAndStart);
    _FF(HostcursorXorStart) = _DD(ddHostcursorXorStart);
    _FF(HostcursorSrcStart) = _DD(ddHostcursorSrcStart);
    _FF(HostcursorExclusionStart) = _DD(ddHostcursorExclusionStart);

    //Copy Data
    memcpy((void *)_DD(ddHostcursorAndStart), (void *)_FF(HostcursorAndStart), 4096);
    memcpy((void *)_DD(ddHostcursorXorStart), (void *)_FF(HostcursorXorStart), 4096);
    memcpy((void *)_DD(ddHostcursorSrcStart), (void *)_FF(HostcursorSrcStart), 4096);
    memcpy((void *)_DD(ddHostcursorExclusionStart), (void *)_FF(HostcursorExclusionStart), 4096);
#endif

    // Disable overlay filtering.
    vidProcCfg = GET(ghwIO->vidProcCfg);
    SETDW(ghwIO->vidProcCfg, vidProcCfg & ~SST_OVERLAY_FILTER_MODE);

    // Request SLI or AA enable
    Sli_AA_Request.ChipInfo.dwaaEn  = _DD(ddAAModeEnabled);
    Sli_AA_Request.ChipInfo.dwsliEn = _DD(ddSLIModeEnabled);

    // Read the following from registry.

    // Entry Is Opposite of Our Logic
    Sli_AA_Request.ChipInfo.dwsliAaAnalog      = _DD(ddSLIAAAnalog);

    // For everything below 768, lets use the SLI Band Height of 16
    // For everything above or equal to 768, lets use the SLI Band Height of 32
    dwSliBandHeight = (ppdev->cyScreen >= 768 ) ? 32 : 16;
  
    Sli_AA_Request.ChipInfo.dwsli_nlines       = RetrieveFromRegistry(ppdev, SLI_BAND_HEIGHT_NAME, dwSliBandHeight, 2, 128);
    if (Sli_AA_Request.ChipInfo.dwsli_nlines > 32)
    {
      dwOverRide = RetrieveFromRegistry(ppdev, SLI_BAND_HEIGHT_OVERRIDE_NAME, 0, 0, 1);
      if (!dwOverRide)
        Sli_AA_Request.ChipInfo.dwsli_nlines = 32;
    }

    Sli_AA_Request.ChipInfo.dwCfgSwapAlgorithm = RetrieveFromRegistry(ppdev, SWAPBUFFER_ALGO_NAME, 1, 0, 1);
    _DD(ddSLINumberScanlines) = Sli_AA_Request.ChipInfo.dwsli_nlines;
    _DD(dwlog2BandHeight) = GetLog2(_DD(ddSLINumberScanlines));
    _DD(dwlog2NumChips)   = GetLog2(_FF(dwNumUnits));
    dwLog2GroupHeight = _DD(dwlog2BandHeight) + _DD(dwlog2NumChips);
    _DD(dwMaxHeight)  = ((ppdev->cyScreen + (1 << dwLog2GroupHeight) - 1) >> dwLog2GroupHeight) << _DD(dwlog2BandHeight);
    // Tile Align Max Linear Height
    _DD(dwMaxLinearHeight) = (_DD(dwMaxHeight) + SST_TILE_HEIGHT - 1) & ~(SST_TILE_HEIGHT - 1);

    // Fill out the memory configuration.
    Sli_AA_Request.MemInfo.dwTotalMemory = _FF(TotalVRAM);
    Sli_AA_Request.MemInfo.dwTileMark    = _FF(ddTiledHeapStart);
    Sli_AA_Request.MemInfo.dwTileCmpMark = _FF(ddTiledHeapStart);

    // Fill out SLI mode information.
    Sli_AA_Request.ChipInfo.dwChips = _FF(dwNumUnits);

    // Fill out AA mode information.
    Sli_AA_Request.ChipInfo.dwaaSampleHigh = _DD(ddAANumberSamples) >> 2;

    // If AA Sample is non-zero then tell backend to do AA
    if (_DD(ddAANumberSamples))
    {
      Sli_AA_Request.ChipInfo.dwaaEn = 0x1;
    }

    Sli_AA_Request.MemInfo.dwaaSecondaryColorBufBegin = _DD(ddAAPrimaryStart);
    Sli_AA_Request.MemInfo.dwaaSecondaryDepthBufBegin = _DD(ddAAZbufferStart);
    Sli_AA_Request.MemInfo.dwaaSecondaryDepthBufEnd   = _DD(ddAAZbufferStart) + _FF(gdiDesktopSize) - 1;
    Sli_AA_Request.MemInfo.dwBpp                      = ppdev->cBitsPerPel;

#if ENABLE_LOG_FILE
    /* retro3dfx: flight-record the exact multi-chip request (D3D SLI banding
       investigation) — compare against the known-good Glide escape path. */
    retroLogForce(ppdev, "retro3dfx PROMOTE-SLIAA: cfg=%ld sliEn=%ld aaEn=%ld smpHi=%ld analog=%ld nlines=%ld chips=%ld tileMark=%08lXh bpp=%ld scr=%ldx%ld\r\n",
                  _DD(ddSLIAAConfiguration),
                  Sli_AA_Request.ChipInfo.dwsliEn, Sli_AA_Request.ChipInfo.dwaaEn,
                  Sli_AA_Request.ChipInfo.dwaaSampleHigh, Sli_AA_Request.ChipInfo.dwsliAaAnalog,
                  Sli_AA_Request.ChipInfo.dwsli_nlines, Sli_AA_Request.ChipInfo.dwChips,
                  Sli_AA_Request.MemInfo.dwTileMark, Sli_AA_Request.MemInfo.dwBpp,
                  (LONG)ppdev->cxScreen, (LONG)ppdev->cyScreen);
#endif

    // call the miniport
    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_SLI_AA_ENABLE,
                           &Sli_AA_Request,
                           sizeof(Sli_AA_Request),
                           NULL,
                           0,
                           &numBytes))
    {
      DISPDBG((0, "Failed IOCTL_3DFX_SLI_AA_ENABLE"));

      /* Set initial fifo state. hw read and sw write pointers at
       * start of the fifo.
       */
      ppdev->fifoData.fifoPtr = (ULONG *) ppdev->fifoData.fifoStart;
      ppdev->fifoData.fifoLastRead = (ULONG) ppdev->fifoData.fifoOffset;
  
      /* Adjust room values.
       * RoomToEnd needs enough room for the jmp packet since we
       * never allow the hw to auto-wrap. RoomToRead needs to be
       * adjusted so that we never acutally write onto the read ptr.
       *
       * fifoRoom is generally the min of roomToEnd and roomToRead,
       * but we 'know' here that roomToRead < roomToEnd.
       */
      ppdev->fifoData.roomToEnd     = ppdev->fifoData.fifoSize;
      ppdev->fifoData.fifoRoom      =
      ppdev->fifoData.roomToReadPtr = ppdev->fifoData.roomToEnd;
  
      // reenable cmdfifo on master, or we'll be hosed
      CmdFifo0Init(ppdev,
                   ppdev->fifoData.fifoOffset,
                   ppdev->fifoData.fifoSize,
                   0,
                   0);

      return 0;
    }

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
    if ((_FF(bUseSliExtraLinearHeap)) &&
        ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
         (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
    {
      DWORD lfbMemoryConfig;

      lfbMemoryConfig = GET(ghwIO->lfbMemoryConfig);
      DISPDBG((0, "  lfbMemoryConfig  = %8lXh", lfbMemoryConfig));

      _FF(ddTileMark) = _FF(sliTileCtrl);
      lfbMemoryConfig &= ~SST_RAW_LFB_TILE_BEGIN_PAGE;
      lfbMemoryConfig |= SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(_FF(ddTileMark) >> 12L);
      SETDW(ghwIO->lfbMemoryConfig, lfbMemoryConfig);
      DISPDBG((0, "  sli lfbMemoryTileCtrl    = %8lXh", GET(ghwIO->lfbMemoryConfig)));

      // write the tile compare register
      // set bit 15 to tell the hw to use tile compare for the beginning
      // of tiled space
      SETDW(ghwIO->lfbMemoryConfig, ((_FF(sliTileCompare) >> 12) |
                                     SST_RAW_LFB_WRITE_CONTROL   |
                                     LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE));
#if DBG
      // set bit 30 to indicate we're only modifying bit 29
      // set bit 29 to read lfbMemoryTileCompare
      SETDW(ghwIO->lfbMemoryConfig, (SST_RAW_LFB_UPDATE_CONTROL | SST_RAW_LFB_READ_CONTROL));
      DISPDBG((0, "  sli lfbMemoryTileCompare = %8lXh", GET(ghwIO->lfbMemoryConfig)));
#endif
      // set bit 30 to indicate we're only modifying bit 29
      // don't set bit 29 so the next read of lfbMemoryConfig will read lfbMemoryTileCtrl
      SETDW(ghwIO->lfbMemoryConfig, SST_RAW_LFB_UPDATE_CONTROL);
    }
#endif

    /* Set initial fifo state. hw read and sw write pointers at
     * start of the fifo.
     */
    ppdev->fifoData.fifoPtr = (ULONG *) ppdev->fifoData.fifoStart;
    ppdev->fifoData.fifoLastRead = (ULONG) ppdev->fifoData.fifoOffset;

    /* Adjust room values.
     * RoomToEnd needs enough room for the jmp packet since we
     * never allow the hw to auto-wrap. RoomToRead needs to be
     * adjusted so that we never acutally write onto the read ptr.
     *
     * fifoRoom is generally the min of roomToEnd and roomToRead,
     * but we 'know' here that roomToRead < roomToEnd.
     */
    ppdev->fifoData.roomToEnd     = ppdev->fifoData.fifoSize;
    ppdev->fifoData.fifoRoom      =
    ppdev->fifoData.roomToReadPtr = ppdev->fifoData.roomToEnd;

    // reenable cmdfifo on master, slaves are snooping membase0
    CmdFifo0Init(ppdev,
                 ppdev->fifoData.fifoOffset,
                 ppdev->fifoData.fifoSize,
                 0,
                 0);

    // reload dac so slaves will snoop it
    HWSetPalette(ppdev, 0, 256, (PVIDEO_CLUTDATA)ppdev->pPal, GAMMA_DESKTOP);

    if (_DD(ddSLIModeEnabled))
    {
      // Compute the primary hardware offset in SLI mode.
      _FF(ddPrimarySurfaceData).hwPtr  = SLI_LfbPtrToHwPtr(ppdev, _FF(ddPrimarySurfaceData).lfbPtr);
      _FF(ddPrimarySurfaceData).hwPtr |= SSTG_IS_TILED;
      _FF(gdiDesktopStart) = _FF(ddPrimarySurfaceData).hwPtr;
      DISPDBG((0, "  Promote_ToSLIAA -> primary HwPtr=%8lXh", _FF(ddPrimarySurfaceData)));
#if ENABLE_LOG_FILE
      retroLogForce(ppdev, "retro3dfx PROMOTE-SLIAA OK: primary lfb=%08lXh hwPtr=%08lXh maxH=%ld maxLinH=%ld\r\n",
                    _FF(ddPrimarySurfaceData).lfbPtr, _FF(ddPrimarySurfaceData).hwPtr,
                    _DD(dwMaxHeight), _DD(dwMaxLinearHeight));
#endif

#ifdef RD_ABORT_ERROR
      _FF(dwSLIMode) = DISABLE_SLI_READ;
#endif
    }
  }

  return 1;
} // Promote_DeviceToSLIAA

/*----------------------------------------------------------------------
Function name:  Demote_DeviceFromSLIAA

Description:    Disable SLI or AA mode.
----------------------------------------------------------------------*/

DWORD Demote_DeviceFromSLIAA(NT9XDEVICEDATA * ppdev)
{
  SLI_AA_REQUEST  Sli_AA_Request;
  DWORD           numBytes;

#if ENABLE_LOG_FILE
  retroLogForce(ppdev, "retro3dfx DEMOTE-SLIAA: aaEn=%ld sliEn=%ld smp=%ld cfg=%ld\r\n",
                _DD(ddAAModeEnabled), _DD(ddSLIModeEnabled),
                _DD(ddAANumberSamples), _DD(ddSLIAAConfiguration));
#endif

  if (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled) || _DD(ddAANumberSamples))
  {
    // disable cmdfifo on master & slaves
    FXBUSYWAIT(ppdev);  // need to wait on all cmdfifo's
    CmdFifo0Disable(ppdev);

    // Use identical enable values to promotion.
    Sli_AA_Request.ChipInfo.dwaaEn  = _DD(ddAAModeEnabled);
    Sli_AA_Request.ChipInfo.dwsliEn = _DD(ddSLIModeEnabled);

    // Use number of scanlines stored during promotion.
    Sli_AA_Request.ChipInfo.dwsli_nlines = _DD(ddSLINumberScanlines);

    // Fill out SLI mode information.
    Sli_AA_Request.ChipInfo.dwChips = _FF(dwNumUnits);

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
    // do this before pulling out of sli mode
    // so the slaves will snoop it
    if ((_FF(bUseSliExtraLinearHeap)) &&
        ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
         (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
    {
      DWORD lfbMemoryConfig;

      lfbMemoryConfig = GET(ghwIO->lfbMemoryConfig);
      DISPDBG((0, "  sli lfbMemoryConfig  = %8lXh", lfbMemoryConfig));

      _FF(ddTileMark) = _FF(ddTiledHeapStart);
      lfbMemoryConfig &= ~SST_RAW_LFB_TILE_BEGIN_PAGE;
      lfbMemoryConfig |= SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(_FF(ddTileMark) >> 12L);
      SETDW(ghwIO->lfbMemoryConfig, lfbMemoryConfig);
      DISPDBG((0, "  lfbMemoryTileCtrl    = %8lXh", GET(ghwIO->lfbMemoryConfig)));

      // write the tile compare register
      // clear bit 15 to tell the hw to use tile ctrl for the beginning
      // of tiled space
      SETDW(ghwIO->lfbMemoryConfig, ((0 >> 12) |
                                     SST_RAW_LFB_WRITE_CONTROL));
#if DBG
      // set bit 30 to indicate we're only modifying bit 29
      // set bit 29 to read lfbMemoryTileCompare
      SETDW(ghwIO->lfbMemoryConfig, (SST_RAW_LFB_UPDATE_CONTROL | SST_RAW_LFB_READ_CONTROL));
      DISPDBG((0, "  lfbMemoryTileCompare = %8lXh", GET(ghwIO->lfbMemoryConfig)));
#endif
      // set bit 30 to indicate we're only modifying bit 29
      // don't set bit 29 so the next read of lfbMemoryConfig will read lfbMemoryTileCtrl
      SETDW(ghwIO->lfbMemoryConfig, SST_RAW_LFB_UPDATE_CONTROL);
    }
#endif

    // call the miniport
    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_SLI_AA_DISABLE,
                           &Sli_AA_Request,
                           sizeof(Sli_AA_Request),
                           NULL,
                           0,
                           &numBytes))
    {
      DISPDBG((0, "Failed IOCTL_3DFX_SLI_AA_DISABLE"));
    }

#if 0
    // Restore Old Pointers
    _FF(HostcursorAndStart) = _DD(ddOldHostcursorAndStart);
    _FF(HostcursorXorStart) = _DD(ddOldHostcursorXorStart);
    _FF(HostcursorSrcStart) = _DD(ddOldHostcursorSrcStart);
    _FF(HostcursorExclusionStart) = _DD(ddOldHostcursorExclusionStart);

    // Restore Old Buffers
    memcpy((void *)_FF(HostcursorAndStart), (void *)_DD(ddHostcursorAndStart), 4096);
    memcpy((void *)_FF(HostcursorXorStart), (void *)_DD(ddHostcursorXorStart), 4096);
    memcpy((void *)_FF(HostcursorSrcStart), (void *)_DD(ddHostcursorSrcStart), 4096);
    memcpy((void *)_FF(HostcursorExclusionStart), (void *)_DD(ddHostcursorExclusionStart), 4096);

    _FF(gdiFlags) &= ~SDATA_GDIFLAGS_SLI_AA_MASTER;
    SwitchToHostCursor(ppdev);
#endif

    /* Set initial fifo state. hw read and sw write pointers at
     * start of the fifo.
     */
    ppdev->fifoData.fifoPtr = (ULONG *) ppdev->fifoData.fifoStart;
    ppdev->fifoData.fifoLastRead = (ULONG) ppdev->fifoData.fifoOffset;

    /* Adjust room values.
     * RoomToEnd needs enough room for the jmp packet since we
     * never allow the hw to auto-wrap. RoomToRead needs to be
     * adjusted so that we never acutally write onto the read ptr.
     *
     * fifoRoom is generally the min of roomToEnd and roomToRead,
     * but we 'know' here that roomToRead < roomToEnd.
     */
    ppdev->fifoData.roomToEnd     = ppdev->fifoData.fifoSize;
    ppdev->fifoData.fifoRoom      =
    ppdev->fifoData.roomToReadPtr = ppdev->fifoData.roomToEnd;

    // reenable cmdfifo on master
    CmdFifo0Init(ppdev,
                 ppdev->fifoData.fifoOffset,
                 ppdev->fifoData.fifoSize,
                 0,
                 0);

    if (_DD(ddSLIModeEnabled))
    {
      // Restore the primary hardware offset in SLI mode.
      _FF(ddPrimarySurfaceData).hwPtr  = LfbPtrToHwPtr(ppdev, _FF(ddPrimarySurfaceData).lfbPtr);
      _FF(ddPrimarySurfaceData).hwPtr |= SSTG_IS_TILED;
      _FF(gdiDesktopStart) = _FF(ddPrimarySurfaceData).hwPtr;
      DISPDBG((0, "  Demote_FromSLIAA -> primary HwPtr=%8lXh", _FF(ddPrimarySurfaceData)));
    }
  }

  // Disable SLI and AA modes
  _FF(ddMultiChipConfig) = FALSE;
  _DD(ddSLIModeEnabled)  = FALSE;
  _DD(ddAAModeEnabled)   = FALSE;

  // Disable everything related to sli and aa modes
  _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
  _DD(ddAAModeRequested)  = 0;
  _DD(ddAANumberSamples)  = 0;
  _DD(ddSLIModeRequested) = 0;

  return 1;
} // Demote_DeviceFromSLIAA

/*----------------------------------------------------------------------

Function name:  BailOutOfAA

Description:    allow driver to back out of AA mode if necessary

Fix for PRS 14389 and PRS 14419

PRS 14389 - FLYFS creates a 24bpp zbuffer after switching to 640x480x16
full screen.  This ends up in linear memory and the AA zbuffer allocation
ends up failing.  Our memory layout doesn't really support allocation of
tiled 24bpp z buffers in 16bpp modes.

PRS 14419 - Enemy Engaged creates a flipping chain and z buffer after
switching to full screen mode.  Then a short time later, it creates
a 256x256 render target and a second z buffer which is also 256x256.
The AA allocation for the second AA z buffer fails.  Our memory layout
doesn't really support multiple z buffers in AA or SLI modes.

So this is something of a workaround that allows the driver to back out
of AA mode (after AA mode has already been entered) when a
memMgr_allocSecondary call fails.

----------------------------------------------------------------------*/

void
BailOutOfAA(NT9XDEVICEDATA *ppdev)
{
  DWORD SLIAAConfig = _DD(ddSLIAAConfiguration);
  DWORD vidProcCfg;


  DISPDBG((0, "Bailing out of AA mode!"));

  // most of Exit_3DApplication(ppdev);
  /* Disable video display. */
  vidProcCfg = GET(ghwIO->vidProcCfg);
  vidProcCfg &= ~SST_VIDEO_PROCESSOR_EN;
  SETDW(ghwIO->vidProcCfg, vidProcCfg);
  
#ifdef SLI_AA
  /* Disable SLI and AA setup. */
  Demote_DeviceFromSLIAA(ppdev);
#endif // SLI_AA
  
#if ENABLE_TILED_HEAP && !defined(CSIM)
  /* Demote from video overlay. */
  Demote_PrimaryFromOverlay(ppdev);
#endif // ENABLE_TILED_HEAP
  
#ifdef AGP_CMDFIFO
  /* Disable AGP command FIFO. */
  Demote_CmdFifoFromAGP(ppdev);
#endif

  // Setup SLI and AA configuration

  // we're shuting off AA
  // so fall thru to cases with AA_DISABLED but with the same SLI_nWAY
  switch (SLIAAConfig)
  {
    case SINGLE_CHIP_NOSLI_AA_2SAMPLE:
    case DUAL_CHIP_NOSLI_AA_4SAMPLE:
    case SINGLE_CHIP_NOSLI_AA_DISABLED:

      DISPDBG((0, "Switching to SINGLE_CHIP_NOSLI_AA_DISABLED"));

      // free up AA surfaces allocated from Linear Heaps 1 thru 4
      // this would be a lot easier if we had the damn pDDLcl here!
      {
        extern HNDLLIST *g_pHndlList;
        HNDLLIST  *pHndlList = (HNDLLIST *)&g_pHndlList;
      
        while (pHndlList->pNext) 
        {
          pHndlList = pHndlList->pNext;

          if (NULL != pHndlList->ppTxtrHndlList)
          {
            DWORD   i;
			
            for (i = 1; i <= (DWORD)pHndlList->ppTxtrHndlList[0]; i++)
            {
			  TXTRHNDL	*pTxtrHndl = pHndlList->ppTxtrHndlList[i];

			  if ((NULL != pTxtrHndl))	 
              {
				FXSURFACEDATA		*pSurfData = pTxtrHndl->surfData;

				if ((NULL != pSurfData) && (0 != pSurfData->AAlfbPtr))
                {
                  memMgr_freeSurface(ppdev,
			                         pTxtrHndl->dwCaps,           	// [IN] DirectDraw surface capabilities
                                     pSurfData->AAlfbPtr,           // [IN] lfb address of surface
                                     pSurfData->AAhwPtr,            // [IN] hardware offset of surface
                                     GETMEMTYPE(pSurfData->AAhwPtr),// [IN] MEM_IN_TILED or MEM_IN_LINEAR
                                     pSurfData->AAheapID,           // [IN] DirectDraw heap number
                                     pSurfData->AApvmHeap);
      
                  pSurfData->AAhwPtr  = 0;
                  pSurfData->AAlfbPtr = 0;
                  pSurfData->AAlPitch = 0;
                  pSurfData->AAheapID = HEAP_INVALID;
                  pSurfData->AApvmHeap = NULL;
                }
              }
            }
          }
        }
      }

      // Use backend to support 2 sample AA when we failed to allocate buffers
      // for 4 sample AA.
      if (DUAL_CHIP_NOSLI_AA_4SAMPLE == SLIAAConfig)
      {
        _DD(ddSLIAAConfiguration) = DUAL_CHIP_SLI_2WAY_AA_2SAMPLE;
        _DD(ddAANumberSamples)  = 2;
      }
      else
      {
        _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
        _DD(ddAANumberSamples)  = 0;
      }
      _DD(ddAAModeRequested)  = 0;
      _DD(ddSLIModeRequested) = 0;
      break;

    case DUAL_CHIP_SLI_2WAY_AA_2SAMPLE:
    case QUAD_CHIP_SLI_2WAY_AA_4SAMPLE:
    case DUAL_CHIP_SLI_2WAY_AA_DISABLED:

      DISPDBG((0, "Switching to DUAL_CHIP_SLI_2WAY_AA_DISABLED"));

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
      // we can't use the sli extra linear heap in this case
      _FF(bUseSliExtraLinearHeap) = 0;
#endif
      _DD(ddSLIAAConfiguration) = DUAL_CHIP_SLI_2WAY_AA_DISABLED;
      _DD(ddAAModeRequested)  = 0;
      _DD(ddAANumberSamples)  = 0;
      _DD(ddSLIModeRequested) = 1;
      break;

    case QUAD_CHIP_SLI_4WAY_AA_2SAMPLE:
    case QUAD_CHIP_SLI_4WAY_AA_DISABLED:

      DISPDBG((0, "Switching to QUAD_CHIP_SLI_4WAY_AA_DISABLED"));

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
      // we can't use the sli extra linear heap in this case
      _FF(bUseSliExtraLinearHeap) = 0;
#endif
      _DD(ddSLIAAConfiguration) = QUAD_CHIP_SLI_4WAY_AA_DISABLED;
      _DD(ddAAModeRequested)  = 0;
      _DD(ddAANumberSamples)  = 0;
      _DD(ddSLIModeRequested) = 1;
      _DD(ddSLIAAAnalog) = 1;
      break;
  }

  // most of Enter_3DApplication(ppdev);
  /* Disable video display. */
  if (IS_NAPALM)
  {
    DWORD vidProcCfg = GET(ghwIO->vidProcCfg);
    vidProcCfg &= ~SST_VIDEO_PROCESSOR_EN;
    SETDW(ghwIO->vidProcCfg, vidProcCfg);
  }

#ifdef SLI_AA
  /* Enable SLI if multiple chips are present, and DISABLE_SLI is not  */
  /* set in the registry.  Enable AA if the application requests by    */
  /* setting the anti-aliasing hint during surface creation, or if     */
  /* ENABLE_AA is set in the registry.  In both cases, AA will only be */
  /* enabled if enough buffers can be allocated.  -CGW-                */
  Promote_DeviceToSLIAA(ppdev);
#endif // SLI_AA

#if ENABLE_TILED_HEAP && !defined(CSIM)
  /* Voodoo3/Napalm have separate mechanisms for flipping the desktop  */
  /* and video overlay surfaces.  Desktop flipping can't be pipelined, */
  /* so the video overlay is borrowed to display the primary flipping  */
  /* chain for 3D, exclusive-mode, full-screen applications.  -CGW-    */
  Promote_PrimaryToOverlay(ppdev);
#endif // ENABLE_TILED_HEAP

#ifdef AGP_CMDFIFO
  /* Voodoo3/Napalm promote to using the AGP command FIFO, to improve  */
  /* the performance of 3D, exclusive-mode, full-screen applications.  */
  /* Voodoo3 disables this function because of hardware problems, but  */
  /* it should be enabled on Napalm.  -CGW-                            */
  Promote_CmdFifoToAGP(ppdev);
#endif // AGP_CMDFIFO
}

#ifdef RD_ABORT_ERROR
/*----------------------------------------------------------------------
Function name:  Modify_SLI_Read

Description:    Determine SLI and AA configuration.
----------------------------------------------------------------------*/

void
Modify_SLI_Read(NT9XDEVICEDATA *ppdev, DWORD dwRequest)
{
  DWORD numBytes;


  if (dwRequest == _FF(dwSLIMode))
    return;

  _FF(dwSLIMode) = dwRequest;

  // call the miniport
  if (EngDeviceIoControl(ppdev->hDriver,
                         dwRequest,
                         NULL,
                         0,
                         NULL,
                         0,
                         &numBytes))
  {
    DISPDBG((0, "Failed IOCTL_3DFX_%s_SLI_READ", (dwRequest == ENABLE_SLI_READ) ? "ENABLE" : "DISABLE"));
  }
}
#endif

/*----------------------------------------------------------------------
Function name:  Compute_SLIAA_Config

Description:    Determine SLI and AA configuration.
----------------------------------------------------------------------*/

void Compute_SLIAA_Config(NT9XDEVICEDATA * ppdev, FxU32 numBuffers)
{
  SstIORegs * pIORegs;
  DWORD vidProcCfg;
  DWORD m;
  DWORD n;
  DWORD k;
  DWORD pclock;
  DWORD i;
  DWORD kpow;
  DWORD pixelclock;

#ifndef SLI_ABOVE_1280
  DWORD dwKillAnalog = (4 != _FF(dwNumUnits)) ? 1 : 0;
#else 
	// Settings for analog and digital SLI operation
	// dwSLICompatibilitySettings = 0 - force single chip mode above 1280x1024.
	// dwSLICompatibilitySettings = 1 - (don't kill) allow analog mode at all resolutions.
	// dwSLICompatibilitySettings = 2 - use digital SLI at and above 1280x1024.
	
	// We may need to change the default value of dwSLICompatibilitySettings for different chip configurations.
	DWORD dwSLICompatibilitySettings = RetrieveFromRegistry(ppdev, SLI_COMPATIBILITY_SETTINGS, 0, 0, 2);
#endif // SLI_ABOVE_1280


  // Read SLI and AA configuration from registry.

  switch (_FF(dwNumUnits))
  {
    // Setup default value based on number of chips.  We default to
    // maximum performance if registry value is missing.

    case 4:  // default is 4-way SLI, AA disabled for quad-chip config
             _DD(ddSLIAAConfiguration) =  RetrieveFromRegistry(ppdev, SLI_AA_CONFIG_NAME, 5, 0, 8);
             break;
    case 2:  // default is 2-way SLI, AA disabled for dual-chip config
             _DD(ddSLIAAConfiguration) =  RetrieveFromRegistry(ppdev, SLI_AA_CONFIG_NAME, 2, 0, 8);
             break;
    default: // default is SLI disabled, AA disabled for single-chip config
             _DD(ddSLIAAConfiguration) =  RetrieveFromRegistry(ppdev, SLI_AA_CONFIG_NAME, 0, 0, 8);
             break;
  }

#ifdef SLI_ABOVE_1280
	if(ANALOG_SLI_ABOVE_1280 == dwSLICompatibilitySettings)
		_DD(ddSLIAAAnalog) = !RetrieveFromRegistry(ppdev, DIGITAL_SLI_AA_NAME, 1, 0, 1);
	else
		_DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
#else 
  // Allow analog on 6000 or 4 chip board
  if (dwKillAnalog)
    _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
  else
    _DD(ddSLIAAAnalog) = !RetrieveFromRegistry(ppdev, DIGITAL_SLI_AA_NAME, 1, 0, 1);
#endif //SLI_ABOVE_1280

  // Sanity testing of config stored in registry.

  switch (_FF(dwNumUnits))
  {
    case 4:  if ((_DD(ddSLIAAConfiguration) == DUAL_CHIP_SLI_2WAY_AA_DISABLED) ||
                 (_DD(ddSLIAAConfiguration) == DUAL_CHIP_SLI_2WAY_AA_2SAMPLE)  ||            
                 (_DD(ddSLIAAConfiguration) == DUAL_CHIP_NOSLI_AA_4SAMPLE) ||
                 (SINGLE_CHIP_NOSLI_AA_2SAMPLE == _DD(ddSLIAAConfiguration)))
             {
               _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
             }
             break;
    case 2:  if ((_DD(ddSLIAAConfiguration) == QUAD_CHIP_SLI_4WAY_AA_DISABLED) ||
                 (_DD(ddSLIAAConfiguration) == QUAD_CHIP_SLI_4WAY_AA_2SAMPLE)  ||            
                 (_DD(ddSLIAAConfiguration) == QUAD_CHIP_SLI_2WAY_AA_4SAMPLE)  ||
                 (_DD(ddSLIAAConfiguration) == QUAD_CHIP_NOSLI_AA_8SAMPLE)     ||
                 (SINGLE_CHIP_NOSLI_AA_2SAMPLE == _DD(ddSLIAAConfiguration)))
             {
               _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
             }
             break;
    default: if ((_DD(ddSLIAAConfiguration) != SINGLE_CHIP_NOSLI_AA_DISABLED)  &&
                 (_DD(ddSLIAAConfiguration) != SINGLE_CHIP_NOSLI_AA_2SAMPLE))
             {
               // Registry specifies value for dual-chip or quad-chip, instead of single-chip.

               _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
             }
             break;
  }

  // Handle app requested AA
  if (DDSCAPS2_HINTANTIALIASING == _DD(ddAAModeRequested))
  {
    // give the app the best AA we can based on the number of chips
    switch (_FF(dwNumUnits))
    {
      case 1:
        // best 1 chip AA is 2 sample AA
        _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_2SAMPLE;
        break;
      case 2:
        // best 2 chip AA is 4 sample AA with SLI disabled
        _DD(ddSLIAAConfiguration) = DUAL_CHIP_NOSLI_AA_4SAMPLE;
        break;
      case 4:
        // best 4 chip AA is 4 sample AA with 2-way SLI
        _DD(ddSLIAAConfiguration) = QUAD_CHIP_NOSLI_AA_8SAMPLE;
        break;
    }
  }

  // Disallow SLI and AA when scanline doubling.

  vidProcCfg = GET(ghwIO->vidProcCfg);
  if (vidProcCfg & SST_HALF_MODE)
  {
    _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
  }    

  // If 2X mode then switch to from digital to analog
  // since digital cannot handle 2X mode
  if (vidProcCfg & SST_VIDEO_2X_MODE_EN)
  {
#ifdef SLI_ABOVE_1280
    switch (dwSLICompatibilitySettings)
    {
      case SINGLE_CHIP__ABOVE_1280:
        _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
        break;
      case ANALOG_SLI_ABOVE_1280:
        // if we only have one unit then there is no choice between analog and digital
        if (1 == _FF(dwNumUnits))
          _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
        else
          _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
        break;
      case DIGITAL_SLI_ABOVE_1280:
        _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
        break;
    }		
#else
    // Don't roll down to Analog as this is bad
    if (dwKillAnalog)
    {
      _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
    }
    else
    {
      // if we only have one unit then there is no choice between analog and digital
      if (1 == _FF(dwNumUnits))
        _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
      else
        _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
    }
#endif //SLI_ABOVE_1280
  }   

  // Determine the pixel clock
  pIORegs = (SstIORegs *)_FF(regBase[HWINFO_SST_IOREGS_INDEX]);
  pixelclock = pIORegs->pllCtrl0;
  n = ((pixelclock & 0xFF00) >> 8)+2;
  m = ((pixelclock & 0xFC) >> 2)+2;
  k = pixelclock & 0x03;

  kpow = 1;
  for (i=0; i<k; i++)
    kpow<<=1;

  // This should work to 4 GigaHertz
  pclock = 14318180*n/m/kpow;                    

  // This will need to be revisited when we do 4-way cards
  if (2 == _FF(dwNumUnits))
  {
    // Since Digital is less then analog fall back to analog
    if (SLI_AA_VIDEO_FORMAT_DIGITAL == _DD(ddSLIAAAnalog))
    {
      if (DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))
      {
        if (pclock > DUAL_CHIP_DIGITAL_SLI_CUTOFF)
        {
#ifdef SLI_ABOVE_1280
          switch (dwSLICompatibilitySettings)
          {
            case SINGLE_CHIP__ABOVE_1280:
              _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
              break;
            case ANALOG_SLI_ABOVE_1280:
              _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
              break;
            case DIGITAL_SLI_ABOVE_1280:
              _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
              break;
          }	
#else
          if (dwKillAnalog)
            _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
          else
            _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
#endif //SLI_ABOVE_1280
        }
      }
      else 
      {
        if (pclock > DUAL_CHIP_DIGITAL_AA_CUTOFF)
        {
#ifdef SLI_ABOVE_1280
          switch (dwSLICompatibilitySettings)
          {
            case SINGLE_CHIP__ABOVE_1280:
              _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
              break;
            case ANALOG_SLI_ABOVE_1280:
              _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
              break;
            case DIGITAL_SLI_ABOVE_1280:
              _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
              break;
          }	
#else
          if (dwKillAnalog)
            _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
          else
            _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
#endif //SLI_ABOVE_1280
        }
      }
    }

    // Analog does have a limit and let's make sure we don't exceed it
    if (SLI_AA_VIDEO_FORMAT_ANALOG == _DD(ddSLIAAAnalog))
    {
      if (DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))
      {
        if (pclock > DUAL_CHIP_ANALOG_SLI_CUTOFF)
          _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
      }
      else 
      {
        if (pclock > DUAL_CHIP_ANALOG_AA_CUTOFF)
          _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
      }
    }
  }

  // Disable SLI and AA for single-buffered applications.

  if (numBuffers < 2)
  {
    _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
  }

  // Silently fail AA when no secondary buffers are available in !SLI modes.

  if (!RetrieveFromRegistry(ppdev, AA_ENABLE_OUTOFMEMORY_NAME, 0, 0, 1))
  {
    if ((_DD(ddSLIAAConfiguration) == SINGLE_CHIP_NOSLI_AA_2SAMPLE) && (_FF(ddSecondaryHeapSize) == 0))
    {
      _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
    }
    if ((_DD(ddSLIAAConfiguration) == DUAL_CHIP_NOSLI_AA_4SAMPLE) && (_FF(ddSecondaryHeapSize) == 0))
    {
      _DD(ddSLIAAConfiguration) = DUAL_CHIP_SLI_2WAY_AA_2SAMPLE;
    }
    if ((_DD(ddSLIAAConfiguration) == QUAD_CHIP_NOSLI_AA_8SAMPLE) && (_FF(ddSecondaryHeapSize) == 0))
    {
      _DD(ddSLIAAConfiguration) = QUAD_CHIP_SLI_2WAY_AA_4SAMPLE;
    }
  }

  // Setup SLI and AA configuration.

  switch (_DD(ddSLIAAConfiguration))
  {
    case SINGLE_CHIP_NOSLI_AA_DISABLED:  _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 0;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         break;
    case SINGLE_CHIP_NOSLI_AA_2SAMPLE:   _DD(ddAAModeRequested)  = 1;
                                         _DD(ddAANumberSamples)  = 2;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         break;
    case DUAL_CHIP_SLI_2WAY_AA_DISABLED: _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 0;
                                         _DD(ddSLIModeRequested) = 1;
                                         _DD(ddSLINumberWays)    = 2;
                                         break;
    case DUAL_CHIP_SLI_2WAY_AA_2SAMPLE:  _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 2;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         break;
    case DUAL_CHIP_NOSLI_AA_4SAMPLE:     _DD(ddAAModeRequested)  = 1;
                                         _DD(ddAANumberSamples)  = 4;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         break;
    case QUAD_CHIP_SLI_4WAY_AA_DISABLED: _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 0;
                                         _DD(ddSLIModeRequested) = 1;
                                         _DD(ddSLINumberWays)    = 4;
                                         _DD(ddSLIAAAnalog) = 1;
                                         break;
    case QUAD_CHIP_SLI_4WAY_AA_2SAMPLE:  _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 2;
                                         _DD(ddSLIModeRequested) = 1;
                                         _DD(ddSLINumberWays)    = 2;
                                         _DD(ddSLIAAAnalog) = 1;
                                         break;
    case QUAD_CHIP_SLI_2WAY_AA_4SAMPLE:  _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 4;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         _DD(ddSLIAAAnalog) = 1;
                                         break;
    case QUAD_CHIP_NOSLI_AA_8SAMPLE:     _DD(ddAAModeRequested)  = 1;
                                         _DD(ddAANumberSamples)  = 8;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         _DD(ddSLIAAAnalog) = 1;
                                         break;
  }

#if ENABLE_LOG_FILE
  retroLogForce(ppdev, "retro3dfx COMPUTE-SLIAA: cfg=%ld numBufs=%ld -> aaReq=%ld aaSmp=%ld sliReq=%ld sliWays=%ld analog=%ld 3dCnt=%ld\r\n",
                _DD(ddSLIAAConfiguration), numBuffers,
                _DD(ddAAModeRequested), _DD(ddAANumberSamples),
                _DD(ddSLIModeRequested), _DD(ddSLINumberWays), _DD(ddSLIAAAnalog),
                _FF(dd3DSurfaceCount));
#endif
} // Compute_SLIAA_Config
#endif


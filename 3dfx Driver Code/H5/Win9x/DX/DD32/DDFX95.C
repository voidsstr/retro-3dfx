/* $Header: ddfx95.c, 4, 10/11/00 8:51:58 PM, Brent$ */
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
** File Name: 	DDFX95.C
**
** Description: DirectDraw DLL entry point and driver initialization.
**
** $Revision: 4$
** $Date: 10/11/00 8:51:58 PM$
**
*/

/*******************************************************************************
*
* EXPORTED FUNCTIONS:
*
* DllMain                   --- DirectDraw 32-bit DLL main entry point.
* DriverInit                --- DirectDraw 32-bit driver initialization.
* buildDDHALInfo32          --- Enumerate Global data, D3DHal callbacks and DriverInfo.
*
*******************************************************************************/

/*
 *
 * NOTE:  All routines are called with the Win16 lock taken.   This is
 * to prevent anyone from calling the display driver and doing something
 * that could confict with this 32-bit driver.
 *
 * This means that all shared 16-32 memory is safe to use at any time inside
 * either driver.
 */

/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include "precomp.h"

#ifdef K6_2
#include "k6_2.h"
#endif


/***************************************************************************
* F U N C T I O N   P R O T O T Y P E S
****************************************************************************/

static BOOL buildDDHALInfo32(NT9XDEVICEDATA *ppdev);

NT9XDEVICEDATA * pDevices[NUM_DEVICES] =
   {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,}; 

/***************************************************************************
* G L O B A L   V A R I A B L E S
****************************************************************************/

HINSTANCE   hInstance;

/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name: DllMain

Description:   DirectDraw 32-bit DLL main entry point.

Return:        TRUE  - DLL attached to calling process
			   FALSE - DLL unable to attach to calling process
----------------------------------------------------------------------*/

BOOL WINAPI
DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
#if CREATE_SHARED_HEAP
  static long   lProcessAttachCnt = 0;
#endif

  hInstance = hModule;

  switch (dwReason)
  {
    case DLL_PROCESS_ATTACH:
      //DISPDBG((DEBUG_DDDETAILS, "Attach: DllMain=%08lx, pid=%08lx",
      //         DllMain, GetCurrentProcessId()));

      DisableThreadLibraryCalls(hModule);
      break;

    case DLL_PROCESS_DETACH:
      break;

#ifdef DEBUG
    /*
     * we don't ever want to see thread attach/detach
     */
    case DLL_THREAD_ATTACH:
      break;

    case DLL_THREAD_DETACH:
      break;
#endif

    default:
      break;
  }

  return TRUE;

} // DllMain


/*----------------------------------------------------------------------
Function name: DriverInit

Description:   DirectDraw 32-bit driver initialization.

Return:        DWORD 

               0   - unable to initialize the driver
			   ptr - driver data - ppdev; 
			         shared memory region between 16/32-bit address space
----------------------------------------------------------------------*/

DWORD _stdcall
DriverInit(DWORD ptr)
{
  NT9XDEVICEDATA *ppdev;
  int i;

  ASSERTDD2(ptr);

#ifdef GBLDATA_IN_PDEV
  ppdev = (PDEV *)ptr;
  ASSERTDD2(! IsBadWritePtr((LPVOID)ppdev,sizeof(PDEV)));
  ASSERTDD2(! IsBadReadPtr((LPVOID)ppdev,sizeof(PDEV)));
  if (! buildDDHALInfo32((PDEV *)ptr))
    return 0;
#else
  ppdev = (NT9XDEVICEDATA *)ptr;
#ifdef SLI_AA
   // If we are a slave then fail
   if (SLI_AA_SLAVE_DEVICE == _FF(dwType))
      return 0;
#endif
  for (i=0; i<NUM_DEVICES; i++)
      {

      // Already got it?
      if (ppdev == pDevices[i])
         break;

      // Found a spare spot then save it
      if (NULL == pDevices[i])
         {
         pDevices[i] = ppdev;
         break;
         }
      }
  ASSERTDD2(! IsBadWritePtr((LPVOID)ppdev,sizeof(GLOBALDATA)));
  ASSERTDD2(! IsBadReadPtr((LPVOID)ppdev,sizeof(GLOBALDATA)));

  if (0 == _DS(hSharedHeap))
      (HANDLE)_DS(hSharedHeap) = HeapCreate(HEAP_SHARED, 0x2000, 0);

  if (! buildDDHALInfo32(ppdev))
    return 0;

#endif  // ifdef GBLDATA_IN_PDEV

#ifdef K6_2
  // set K6-2 functions pointers so we may enable/disable
  // 3DNow! optimization via registry each time driver inited
  ADJUST3DNOWPOINTERS(ppdev);
#endif

  DISPDBG((ppdev, DEBUG_APIENTRY, "DriverInit called, ptr=%08lx", ptr));

  return (DWORD)ptr;

} // DriverInit


/*----------------------------------------------------------------------
Function name: buildDDHALInfo32

Description:   Enumerate Global data, D3DHal callbacks and DriverInfo

Return:        FALSE for failure, TRUE for success
----------------------------------------------------------------------*/

static BOOL buildDDHALInfo32(NT9XDEVICEDATA *ppdev)
{
  //allocate memory for ddglobal
  if (NULL == (LPVOID)_FF(pddglobal))
  {
    (LPVOID)_FF(pddglobal) = DXMALLOCZ(sizeof(DDGLOBAL));
    if (NULL == (LPVOID)_FF(pddglobal))
      return FALSE;
#ifdef SLI_AA
    ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorAndStart =(DWORD)DXMALLOCZ(32*32*4*5);
    if (0x0 == ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorAndStart)
      return FALSE;  
    ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorXorStart = ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorAndStart + 4096;
    ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorSrcStart = ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorAndStart + 8192;
    ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorExclusionStart = ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorAndStart + 12288;
    ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorExclusionSave = ((DDGLOBAL *)_FF(pddglobal))->ddHostcursorAndStart + 16384;
#endif
  }

  //allocate memory for fxglobal
  if (NULL == (LPVOID)_FF(pfxglobal))
  {
    (LPVOID)_FF(pfxglobal) = DXMALLOCZ(sizeof(fxGlobal));
    if (NULL == (LPVOID)_FF(pfxglobal))
      return FALSE;
  }

#if ENABLE_3D
  //allocate memory for d3global
  if (NULL == (LPVOID)_FF(pd3global))
  {
    (LPVOID)_FF(pd3global) = DXMALLOCZ(sizeof(d3Global));
    if (NULL == (LPVOID)_FF(pd3global))
      return FALSE;
  }
#endif


  // if we wanted to emulate NT, we'd call DrvGetDirectDrawInfo twice
  // the first time with pvmList and pdwFourCC set to NULL, after the
  // first call we'd use the returned dwNumHeaps and dwNumFourCC to
  // allocate two memory chunks, one for the heap array and one for
  // the fourcc array then call DrvGetDirectDrawInfo again with these
  // pointers filled in but this isn't necessary on win9x since the 16
  // bit side will fill in the heap array and fourcc array
  // So we just call DrvGetDirectDrawInfo once on winx9x

  if (FALSE == DrvGetDirectDrawInfo((DHPDEV)ppdev,
                                    &(_FF(HALInfo)),
                                    &(_FF(HALInfo).vmiData.dwNumHeaps),
                                    NULL,
                                    &(_FF(HALInfo).ddCaps.dwNumFourCCCodes),
                                    NULL))
    return FALSE;

  if (FALSE == DrvEnableDirectDraw((DHPDEV)ppdev,
                                   &(_FF(DDCallbacks)),
                                   &(_FF(DDSurfaceCallbacks)),
                                   &(_FF(DDPaletteCallbacks))))
    return FALSE;

  return TRUE;

} // buildDDHALInfo32

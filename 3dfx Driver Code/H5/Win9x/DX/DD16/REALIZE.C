/* -*-c++-*- */
/* $Header: realize.c, 4, 10/11/00 8:51:35 PM, Brent$ */
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
** File name:   realize.c
**
** Description: RealizeObject and supporting functions.
**
** $Revision: 4$
** $Date: 10/11/00 8:51:35 PM$
**
** $History: realize.c $
** 
** *****************  Version 31  *****************
** User: Cwilcox      Date: 4/22/99    Time: 4:54p
** Updated in $/devel/h3/Win95/dx/dd16
** Make STB_MMGR_TWK changes permanent, removed #ifdefs.
** 
** *****************  Version 30  *****************
** User: Cwilcox      Date: 4/21/99    Time: 5:05p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed obsolete 16-bit linear memory manager.
** 
** *****************  Version 29  *****************
** User: Stb_pzheng   Date: 3/04/99    Time: 8:52a
** Updated in $/devel/h3/win95/dx/dd16
** Added call to wait engine idle in EvictBmp - fix for PRS #4750.   Tests
** show that this change does not introduce any 2D performance hit.
** 
** *****************  Version 28  *****************
** User: Stb_pzheng   Date: 2/09/99    Time: 10:14a
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 27  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 7:07a
** Updated in $/devel/h3/win95/dx/dd16
** 
** *****************  Version 26  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:39a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 25  *****************
** User: Andrew       Date: 1/09/99    Time: 11:30a
** Updated in $/devel/h3/Win95/dx/dd16
** Added back in the DoHostStage2 and changed DoHostStage1 to old version
** 
** *****************  Version 24  *****************
** User: Cshaw        Date: 1/07/99    Time: 10:16a
** Updated in $/devel/h3/Win95/dx/dd16
** Moved the "nullAlll" and "nullDoCreateBitmap" declairations to
** beginning of the file so the driver will build with the env var,
** "nd=1".
** 
** *****************  Version 23  *****************
** User: Michael      Date: 12/30/98   Time: 9:30a
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 22  *****************
** User: Andrew       Date: 11/25/98   Time: 1:44p
** Updated in $/devel/h3/Win95/dx/dd16
** Change DoHostStage1 to clean up the pDevInfo even if lpPDevice is
** hosed.  Also added a case which should fix deletion of Nodes which both
** point at the same lpPDevice.
** 
** *****************  Version 21  *****************
** User: Michael      Date: 11/19/98   Time: 12:10p
** Updated in $/devel/h3/Win95/dx/dd16
** For RealizeObject realize path, let dibengine handle if Dest Dev is
** busy or disabled.  Fixes the Fountain stop ship issue.  Driver was
** causing a GPF when playing video clip in full screen under Compton's
** Encyclopedia '98 (PRS 3395).
** 
** *****************  Version 20  *****************
** User: Andrew       Date: 9/23/98    Time: 11:12p
** Updated in $/devel/h3/Win95/dx/dd16
** removed dohoststage2 and modified dohoststage1
** 
** *****************  Version 19  *****************
** User: Andrew       Date: 8/19/98    Time: 3:54p
** Updated in $/devel/h3/Win95/dx/dd16
** Change bug in commented out section of DoHostStage1.
** 
** *****************  Version 18  *****************
** User: Michael      Date: 8/19/98    Time: 12:09p
** Updated in $/devel/h3/Win95/dx/dd16
** Rearranged RealizeObject.  Get rid of levels of indirection and
** redundant call levels.  Bring in code from functions.  Gains 1-2
** Winmarks on WB98.
** 
** *****************  Version 17  *****************
** User: Ken          Date: 7/23/98    Time: 4:04p
** Updated in $/devel/h3/win95/dx/dd16
** added agp command fifo.   not added to NT build.  currently not
** functional on non-win98 systems without running a special enable apg
** script first, see Ken for that.   agp command fifo is enabled by
** setting the environment variable ACF=1 , all other settings disable it.
** Only turned on interatively in debugger in InitFifo call.   
** 
** *****************  Version 16  *****************
** User: Michael      Date: 7/10/98    Time: 5:41p
** Updated in $/devel/h3/Win95/dx/dd16
** MarkL's (IGX) fix for Track 2053 & 2054.  It appears based on his
** research that device BMs must be DWORD aligned because DIBENG expects
** them that way.  I actually saw a WB98 improvement here.  (alignment?)
** 
**/
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

#include "header.h"
#include "devbits.h"
#define Not_VxD
#include "minivdd.h"
#include "dqueue.h"
#include "memmgr16.h"

#define Not_VxD
#include <vmm.h>
#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)

#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)


// This is our Display Info provided by the main VDD
// this should be move into DriverInfo
DISPLAYINFO DisplayInfo;

extern void FXWAITFORIDLE();

#ifdef PERF_NEWMM
extern DWORD /*__near*/ __cdecl RealizeBitmapObject( LPVOID      lpDestDev,
                            WORD         wStyle,
                            LPBITMAP    lpInObj,
                            LPDIBENGINE lpOutObj,
                            LPVOID      lpTextXForm,
                            LPVOID       lpPDevice ); 

extern VOID /* __near */ __cdecl DeleteBitmapObject( LPDIBENGINE lpDevBM );
#endif

// This is information on our device bitmap filter
DEVFILTER DevFilter;

REGINFO RegInfo[] = {
   {(WORD FAR *)&DevFilter.nDevBitsEnabled, TRUE, STR_ENABLE},
   {(WORD FAR *)&DevFilter.nFilterEnabled, TRUE, STR_FILTERENABLE},
   {(WORD FAR *)&DevFilter.nMinX, MIN_DEVBIT_X, STR_MIN_X},
   {(WORD FAR *)&DevFilter.nMinY, MIN_DEVBIT_Y, STR_MIN_Y},
   {(WORD FAR *)&DevFilter.nMaxX, MAX_DEVBIT_X, STR_MAX_X},
   {(WORD FAR *)&DevFilter.nMaxY, MAX_DEVBIT_Y, STR_MAX_Y},
   };

// External DIB Functions
DWORD FAR PASCAL DIB_RealizeObjectExt(LPDIBENGINE lpDestDev, WORD wStyle, LPVOID lpInObj, LPVOID lpOutObj, LPTEXTXFORM lpTextXForm, LPDIBENGINE lpDisplayDev);

WORD PASCAL wFlatDataSel;

#ifndef PERF_NEWMM
DQUEUE DevBitFree;
DQUEUE DevBitInUse = {&DevBitInUse, &DevBitInUse, NULL};

DEVQUEUE DevBitPool[NUMBER_DEVBIT];
#endif

// These our local functions
void GetBinary(DWORD dwDevNodeHandle, WORD FAR * lpValue, char FAR * lpStr, WORD nDefault);
extern DoBitCopy(DWORD, WORD, DWORD, WORD, DWORD, DWORD, WORD, WORD);

void DeleteObject(LPDIBENGINE lpObject);

// Local Defines
#undef ANDY_DEBUG

#ifdef NULLDRIVER
extern FxU32 nullAll;
FxU32 nullDoCreateBitmap = 0;
#endif // #ifdef NULLDRIVER

/*----------------------------------------------------------------------
Function name:  RealizeObject

Description:    Creates or deletes a physical object.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         DWORD   the size in bytes of the physical object or,
                        return value of an object
----------------------------------------------------------------------*/
DWORD FAR PASCAL _loadds RealizeObject(LPDIBENGINE lpDestDev, WORD wStyle, LPVOID lpInObj, LPVOID lpOutObj, LPTEXTXFORM lpTextXForm)
{
    if (wStyle == -5)
        {
#ifdef PERF_NEWMM
        DeleteBitmapObject( (LPDIBENGINE) lpOutObj );
#else                    
        PDEVQUEUE pDevqueue;

        if (NULL != lpOutObj)
            {
            pDevqueue = (PDEVQUEUE)((LPDIBENGINE)lpOutObj)->deDriverReserved;
            if (NULL != pDevqueue)
                {
                // Set pointer to zero
                (PDEVQUEUE)((LPDIBENGINE)lpOutObj)->deDriverReserved = 0x0;

                // Free Up Memory Allocator
                // Is this a Offscreen|VRAM or a Hostified BitMap
                if (DV_OFFSCREEN == pDevqueue->DevInfo.wStatus)
                    mmFree( pDevqueue->DevInfo.dwAddr );
                else
                    GlobalFree(pDevqueue->DevInfo.wHlpPDevice);

                // Free Up DeviceBit Map Descriptor
                DDELETE(pDevqueue);

                //Add to Free List
                DINSERT((PDQUEUE)pDevqueue, DevBitFree)
                }
            }
#endif // PERF_NEWMM            
        }
    else
    if (wStyle == 5)
        {
#ifdef PERF_NEWMM
        return RealizeBitmapObject( lpDestDev, wStyle, lpInObj, lpOutObj, lpTextXForm, (LPVOID) _FF(lpPDevice));        
#else        
        LPDIBENGINE lpDevBit;
        DWORD dwReturn;
        DWORD dwMem;
        PDEVQUEUE pDevqueue;
        WORD wWidth;
        WORD wHeight;
        MM_ALLOCSIZE AllocSize;
        DWORD dwRetCode;

#ifdef STBPERF_RBO_SKIPDIBENGINE
// PingZ.  This code bypass the call to DIB_RealizeObjectExt
        DWORD WidthBytes;

        if (NULL == lpOutObj)
        {
          dwReturn = ((LPBITMAP)lpInObj)->bmWidthBytes;
          dwReturn += 3l; 
          dwReturn &= 0xfffcl;
          dwReturn *= ((LPBITMAP)lpInObj)->bmHeight;
          dwReturn += sizeof(DIBENGINE);

          return dwReturn;
        }

// Fill lpOutObj ourselves without calling DIBENGINE to do it
        ((LPDIBENGINE)lpOutObj)->deFlags = _FF(lpPDevice)->deFlags & (FIVE6FIVE | MINIDRIVER | PALETTIZED | SELECTEDDIB);

        ((LPDIBENGINE)lpOutObj)->deBitsPixel = _FF(bpp);
        ((LPDIBENGINE)lpOutObj)->dePlanes = 1;
        ((LPDIBENGINE)lpOutObj)->deWidth = ((LPBITMAP)lpInObj)->bmWidth;
        ((LPDIBENGINE)lpOutObj)->deHeight = ((LPBITMAP)lpInObj)->bmHeight;
        WidthBytes = ((LPBITMAP)lpInObj)->bmWidthBytes;
        ((LPDIBENGINE)lpOutObj)->deBitmapInfo = _FF(lpPDevice)->deBitmapInfo;
        ((LPDIBENGINE)lpOutObj)->deBitsOffset = ((DWORD)lpOutObj & 0xffff) + sizeof(DIBENGINE);
        ((LPDIBENGINE)lpOutObj)->deBitsSelector = (WORD) ((DWORD)lpOutObj >> 16);
        ((LPDIBENGINE)lpOutObj)->deReserved1 = 0;
        ((LPDIBENGINE)lpOutObj)->delpPDevice = 0;
        ((LPDIBENGINE)lpOutObj)->deBeginAccess = 0;
        ((LPDIBENGINE)lpOutObj)->deEndAccess = 0;
        ((LPDIBENGINE)lpOutObj)->deDriverReserved = 0;
        ((LPDIBENGINE)lpOutObj)->deVersion = VER_DIBENG;
        ((LPDIBENGINE)lpOutObj)->deType = TYPE_DIBENG;

        ((LPDIBENGINE)lpOutObj)->deWidthBytes = WidthBytes;
        ((LPDIBENGINE)lpOutObj)->deDeltaScan = (WidthBytes + 3) & ~3;

        dwReturn = ((LPDIBENGINE)lpOutObj)->deDeltaScan;
#else
        dwReturn = DIB_RealizeObjectExt(lpDestDev, wStyle, lpInObj, lpOutObj, lpTextXForm, _FF(lpPDevice));
#endif
        // return if busy or disabled.  Fixes 3395.
        // It is possible that we get realizeObject calls when
        // we've gone through Disable1.  This will GPF at mmAlloc
        // below since Disable1 performs a FreeAllNodeTablePages.
        // This test should not be done in the free object case above.
        // mmFree will be unable to free since FreeAllNodeTablePages
        // has been called.  This is OK.
        if (lpDestDev->deFlags & (BUSY|DISABLED))
            return dwReturn;

#ifndef STBPERF_RBO_SKIPDIBENGINE
//No longer need to check this any more since it's been ckecked earlier
        // Is this just size??
        if (NULL == lpOutObj)
            return dwReturn;
#endif
        lpDevBit = lpOutObj;
        // Check to ensure we do not already have handle
        // DeleteObject(lpDevBit);
        lpDevBit->deDriverReserved = 0x0;

#ifdef NULLDRIVER
        if (nullAll || nullDoCreateBitmap)
            return dwReturn;
#endif // #ifdef NULLDRIVER   

        // If DeviceBitmap is not enabled or we have no free descriptors then return
        if ( !(_FF(ddMiscFlags) & DDMF_ENABLE_DEVICEBITMAPS) ||
                (DevBitFree.pLeft == &DevBitFree) )
            return dwReturn;

        wWidth = lpDevBit->deWidth;
        wHeight = lpDevBit->deHeight;
        // Are we discarding runts and big boys ??

//PingZ 1/10/98  Do not cache these bitmaps as they are used for CM blit only by WB.  
//Also,with the fix for memory fragmentation problem we should allow bitmaps that 
//are bigger than 1024x768 to be cached.  

        if ( (_FF(ddMiscFlags) & DDMF_ENABLE_DEVICEBITMAPS) && 
             (((wWidth == 8) && (wHeight == 8)) || 
             ((wWidth == 1) && (wHeight == 1)) ||
             (wWidth == 624) || (wWidth == 369) || 
             (wWidth > 1300) || (wHeight > 768)))

            return dwReturn;

        // Make sure that the screen and the offscreen Device Bitmap have the same pixel depth
        if (lpDevBit->deBitsPixel != _FF(bpp))
            {
#ifdef DEBUG
            // Please send email to andrew@3dfx.com if you hit this int 3h
            // and reveil your secret.  Inquiring minds want to know....
            // I don't think that this is possible to happen.....
            DPF(DBGLVL_NORMAL, "Device Bitmap != Screen Format\n", lpDevBit->deBitsPixel, _FF(bpp));
            {_asm int 03h}
#endif
            return dwReturn;
            }

        // No... now we want to allocate the space that we need
        AllocSize.dwSize = sizeof(MM_ALLOCSIZE);
        AllocSize.dwRequestSize = ((DWORD)lpDevBit->deWidthBytes * (DWORD)wHeight);
        AllocSize.dwFlags = MM_ALLOC_FROM_END_OF_HEAP;
        // Device Bitmaps must be Dword aligned because DIBENG expects them 
        // to be so.  This fixes alignment problems in track bugs 2053 and 2054.
        AllocSize.mmAlignType = dword_align;
        AllocSize.mmHeapType = any_heap;
        AllocSize.fpHostifyCallBackFunc = (FARPROC)DoHostStage1;
        dwMem = mmAlloc(&AllocSize, &dwRetCode);

        if (MM_OK != dwRetCode)
            return dwReturn;

        // Get a Descriptor
        pDevqueue = (PDEVQUEUE)DevBitFree.pRight;

        // Remove if from the Free List
        DDELETE(pDevqueue)

        // Insert in Use List
        DINSERT((PDQUEUE)pDevqueue, DevBitInUse)

        // Setup Device Bit Map Information
        pDevqueue->DevInfo.dwAddr = dwMem;

        // Save this for the hostify
        pDevqueue->DevInfo.deBitsOffset = lpDevBit->deBitsOffset;
        pDevqueue->DevInfo.wHostPitch = (WORD)lpDevBit->deDeltaScan;
        pDevqueue->DevInfo.wDevPitch = lpDevBit->deWidthBytes;
        pDevqueue->DevInfo.wHeight = wHeight;
        pDevqueue->DevInfo.wHlpPDevice = GlobalPtrHandle(lpDevBit);
        pDevqueue->DevInfo.wStatus = DV_OFFSCREEN;

#ifdef ANDY_DEBUG
        DPF(DBGLVL_NORMAL, "Good Stuff flags %04x Sel=%04x Off=%08lx PHandle=%04x lpDevBit=%08lx lpDevBit=%08lx lpBitMap=%08lx %08lx %08lx\n",
        lpDevBit->deFlags, lpDevBit->deBitsSelector, lpDevBit->deBitsOffset, Devqueue.wHlpPDevice,
        lpDevBit, GlobalLock(Devqueue.wHlpPDevice), MAKELP(SELECTOROF(GlobalLock(Devqueue.wHlpPDevice)), (WORD)lpDevBit->deBitsOffset),
        lpDevBit->deBeginAccess, lpDevBit->deEndAccess);
#endif

        // Setup For DIBENG
        (DWORD)lpDevBit->deBeginAccess = (DWORD)BeginAccess;
        (DWORD)lpDevBit->deEndAccess = (DWORD)EndAccess;
        lpDevBit->deFlags |= (VRAM | OFFSCREEN);
        lpDevBit->deBitsOffset = dwMem;
        lpDevBit->deBitsSelector = wFlatDataSel;
        // Hey this is linear
        // that makes our deDeltaScan = lpDevBit->deWidthBytes
        lpDevBit->deDeltaScan = lpDevBit->deWidthBytes;

        // Give myself a link back to the DevBitInUse List
        lpDevBit->deDriverReserved = (DWORD)pDevqueue;

        return dwReturn;
#endif // PERF_NEWMM        
        }

    return DIB_RealizeObjectExt(lpDestDev, wStyle, lpInObj, lpOutObj, lpTextXForm, _FF(lpPDevice));
}


/*----------------------------------------------------------------------
Function name:  SelectBitmap

Description:    This function selects a bitmap into the specificed
                device, replacing the previous bitmap.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         WORD    1 is always returned
----------------------------------------------------------------------*/
#undef SelectBitmap
WORD FAR PASCAL SelectBitmap(LPDIBENGINE lpDevice, LPBITMAP lpPrevBitmap, LPBITMAP lpBitMap, DWORD fFlags)
{
   return 1;
}


/*----------------------------------------------------------------------
Function name:  InitDeviceBitmapFilter

Description:    This function is called once per mode switch to
                initialize the filter function.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void InitDeviceBitmapFilter(void)
{
   int i;

   if (DisplayInfo.diDevNodeHandle)
      {
      // No greater then screen size?
      RegInfo[4].nDefault = DisplayInfo.diXRes;
      RegInfo[5].nDefault = DisplayInfo.diYRes;

      for (i=0; i<sizeof(RegInfo)/sizeof(REGINFO); i++)
         GetBinary(DisplayInfo.diDevNodeHandle, RegInfo[i].lpValue, RegInfo[i].lpStr, RegInfo[i].nDefault);
      }
   else
      {
      DevFilter.nDevBitsEnabled = FALSE;
      DevFilter.nFilterEnabled = TRUE;
      DevFilter.nMinX = MIN_DEVBIT_X;
      DevFilter.nMinY = MIN_DEVBIT_Y;
      DevFilter.nMaxX = DisplayInfo.diXRes;
      DevFilter.nMaxY = DisplayInfo.diYRes;
      }
}


/*----------------------------------------------------------------------
Function name:  GetBinary

Description:    This function is used to retrieve information
                from the registry.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void GetBinary(DWORD dwDevNodeHandle, WORD FAR * lpValue, char FAR * lpStr, WORD nDefault)
{
   ULONG cbValue = sizeof(WORD);

   //Read the attribute from the registry if it is available
	if (CM_Read_Registry_Value(dwDevNodeHandle,
						"DEFAULT",
						lpStr,
						REG_BINARY,
						(LPBYTE)lpValue,
						&cbValue,
						CM_REGISTRY_SOFTWARE) != CR_SUCCESS)
		*lpValue = nDefault;
}


/*----------------------------------------------------------------------
Function name:  InitDeviceBitMap

Description:    This function is used to initialize device bitmaps.

Information:

Return:         VOID
----------------------------------------------------------------------*/
WORD nFirstTime = TRUE;

#ifndef PERF_NEWMM   // The InitDeviceBitmap function for used with the new MM 
                     // is in RBO.C
void InitDeviceBitMap(void)
{
   PDQUEUE pDqueue;
   int i;

   if (nFirstTime)
      {
      // Initialize Free Header of Device Bit Map Descriptors
      DevBitFree.pLeft = &DevBitFree;
      DevBitFree.pRight = &DevBitFree;

      // Initialize In Use Header of Device Bit Map Descriptors
      DevBitInUse.pLeft = &DevBitInUse;
      DevBitInUse.pRight = &DevBitInUse;

      // Initialize Descriptors and Free List
      for (i=0; i<NUMBER_DEVBIT; i++)
         {
         DevBitPool[i].DevInfo.dwAddr = 0x0;
         DevBitPool[i].DevInfo.deBitsOffset = 0x0;
         DevBitPool[i].DevInfo.wHlpPDevice = 0x0;
         DevBitPool[i].DevInfo.wHostPitch = 0x0;
         DevBitPool[i].DevInfo.wDevPitch = 0x0;
         DevBitPool[i].DevInfo.wHeight = 0x0;
         DevBitPool[i].DevInfo.wStatus = DV_NOTINUSE;
         pDqueue = (PDQUEUE)&DevBitPool[i];
         DINSERT(pDqueue, DevBitFree);
         }
      nFirstTime = FALSE;
   }
}
#endif // PERF_NEWMM


/*----------------------------------------------------------------------
Function name:  DoAllHost

Description:    This function is used to move device bitmaps
                to the Host.
Information:

Return:         VOID
----------------------------------------------------------------------*/
#ifndef PERF_NEWMM   // The new MM handles bitmap eviction differently
void DoAllHost(void)
{
   PDEVQUEUE pDevqueue;
   DWORD dwAddr;

   for (pDevqueue = (PDEVQUEUE)DevBitInUse.pRight; pDevqueue != (PDEVQUEUE)&DevBitInUse; )
      {

      // If it is in offscreen memory then move it to the host
      if (DV_OFFSCREEN == pDevqueue->DevInfo.wStatus)
         {
#ifdef ANDY_DEBUG
         DPF(DBGLVL_NORMAL, "Address to Hostify %08lx\n", pDevInfo->dwAddr);
#endif
         dwAddr = pDevqueue->DevInfo.dwAddr;
         DRTRAVERSE(pDevqueue);
         DoHostStage1(dwAddr);
         }
      else
         {
         DRTRAVERSE(pDevqueue);
         }
      }
}
#endif // PERF_NEWMM

#ifndef PERF_NEWMM   // The new MM uses the EvictBmp function in RBO.C
/*----------------------------------------------------------------------
Function name:  EvictBmp  //PingZ 1/20/98

Description: This function dumps the bitmap bits held by lpPDevice to system
             memory and removed its presence from the list of cached bitmaps   
      
Information: It is a simplified version of DoHostStage1 in that lpPDevice is
             already given, so there is no need to search for it from the cache
             list, and also no need to lock any system memory pointer.
Return:      VOID   
----------------------------------------------------------------------*/
void _loadds EvictBmp(LPDIBENGINE lpPDevice)
{
   PDEVQUEUE pDevqueue;
   LPSTR lpBitMap;
   DWORD dwSrcAdj;
   DWORD dwDestAdj;
   DWORD dwAddr;

   if (NULL == lpPDevice)
      return;

   pDevqueue = (PDEVQUEUE)lpPDevice->deDriverReserved;

   if (NULL == pDevqueue)
      return;


   lpBitMap = (LPSTR)MAKELP(SELECTOROF(lpPDevice), (WORD)pDevqueue->DevInfo.deBitsOffset);
   dwAddr = lpPDevice->deBitsOffset;


   // Copy BitMap to Host
   dwSrcAdj = lpPDevice->deDeltaScan - lpPDevice->deWidthBytes;
   dwDestAdj = pDevqueue->DevInfo.wHostPitch - lpPDevice->deWidthBytes;

   // Need to spin down the engine first.  PRS #4750
   FXWAITFORIDLE();

   DoBitCopy(lpPDevice->deBitsOffset, lpPDevice->deBitsSelector, (DWORD)OFFSETOF(lpBitMap), SELECTOROF(lpBitMap), dwSrcAdj, dwDestAdj, lpPDevice->deWidthBytes, lpPDevice->deHeight);

   // Fix it up so it looks like I have never been here
   (DWORD)lpPDevice->deBeginAccess = (DWORD)NULL;
   (DWORD)lpPDevice->deEndAccess = (DWORD)NULL;
   lpPDevice->deFlags &= ~(VRAM | OFFSCREEN);
   lpPDevice->deBitsOffset = OFFSETOF(lpBitMap);
   lpPDevice->deBitsSelector = SELECTOROF(lpBitMap);
   lpPDevice->deDeltaScan = pDevqueue->DevInfo.wHostPitch;
   // Clear Link so that we will Skip Mem Dealloc
   lpPDevice->deDriverReserved = 0x0;

   // Reinit
   pDevqueue->DevInfo.deBitsOffset = 0x0;
   pDevqueue->DevInfo.wHostPitch = 0x0;
   pDevqueue->DevInfo.wHeight = 0x0;
   pDevqueue->DevInfo.wHlpPDevice = 0x0;
   pDevqueue->DevInfo.wStatus = DV_NOTINUSE;

   // Free Up DeviceBit Map Descriptor
   DDELETE(pDevqueue);

   //Add to Free List
   DINSERT((PDQUEUE)pDevqueue, DevBitFree)

   pDevqueue->DevInfo.dwAddr = 0x0;
         // Free Up Memory Allocated
      mmFree(dwAddr);

   DRTRAVERSE(pDevqueue);
}


// InsertBitmapIntoCache is used by the new bitblt codr to recache certain 
// devicebitmaps.
#ifdef PERF_NEW16BITBLT
BOOL InsertBitmapIntoCache(LPDIBENGINE lpDevBit)
{
    MM_ALLOCSIZE    AllocSize;
    PDEVQUEUE       pDevqueue;
    DWORD           dwMem, dwRetCode;
    WORD            wWidth, wHeight;
    
    wWidth = lpDevBit->deWidth;
    wHeight = lpDevBit->deHeight;
        
    if ( (_FF(ddMiscFlags) & DDMF_ENABLE_DEVICEBITMAPS) && 
            (((wWidth == 8) && (wHeight == 8)) || 
            ((wWidth == 1) && (wHeight == 1)) ||
            (wWidth == 624) || (wWidth == 369) || 
            (wWidth > 1300) || (wHeight > 768)))
        return FALSE;
            
    AllocSize.dwSize = sizeof(MM_ALLOCSIZE);
    AllocSize.dwRequestSize = ((DWORD)lpDevBit->deWidthBytes * (DWORD)wHeight);
    AllocSize.dwFlags = MM_ALLOC_FROM_END_OF_HEAP;
    // Device Bitmaps must be Dword aligned because DIBENG expects them 
    // to be so.  This fixes alignment problems in track bugs 2053 and 2054.
    AllocSize.mmAlignType = dword_align;
    AllocSize.mmHeapType = any_heap;
    AllocSize.fpHostifyCallBackFunc = (FARPROC)DoHostStage1;
    dwMem = mmAlloc(&AllocSize, &dwRetCode);

    if (MM_OK != dwRetCode)
        return FALSE;

    // Get a Descriptor
    pDevqueue = (PDEVQUEUE)DevBitFree.pRight;

    // Remove if from the Free List
    DDELETE(pDevqueue)

    // Insert in Use List
    DINSERT((PDQUEUE)pDevqueue, DevBitInUse)

    // Setup Device Bit Map Information
    pDevqueue->DevInfo.dwAddr = dwMem;

    // Save this for the hostify
    pDevqueue->DevInfo.deBitsOffset = lpDevBit->deBitsOffset;
    pDevqueue->DevInfo.wHostPitch = (WORD)lpDevBit->deDeltaScan;
    pDevqueue->DevInfo.wDevPitch = lpDevBit->deWidthBytes;
    pDevqueue->DevInfo.wHeight = wHeight;
    pDevqueue->DevInfo.wHlpPDevice = GlobalPtrHandle(lpDevBit);
    pDevqueue->DevInfo.wStatus = DV_OFFSCREEN;

    // Setup For DIBENG
    (DWORD)lpDevBit->deBeginAccess = (DWORD)BeginAccess;
    (DWORD)lpDevBit->deEndAccess = (DWORD)EndAccess;
    lpDevBit->deFlags |= (VRAM | OFFSCREEN);
    lpDevBit->deBitsOffset = dwMem;
    lpDevBit->deBitsSelector = wFlatDataSel;
    // Hey this is linear
    // that makes our deDeltaScan = lpDevBit->deWidthBytes
    lpDevBit->deDeltaScan = lpDevBit->deWidthBytes;

    // Give myself a link back to the DevBitInUse List
    lpDevBit->deDriverReserved = (DWORD)pDevqueue;

    return TRUE;
}
#endif  // PERF_NEW16BITBLT

#endif  // PERF_NEWMM

/*----------------------------------------------------------------------
Function name:  DoHostStage1

Description:    This function is used to do Stage 1 move.

Information:
    I would like to move all of the bitmap to the host area I
    allocated in DoCreateBitMap.  Unfortuntately, when I tried
    to lock the Handle to the pDevice it would not always work
    due to unknown reasons.  I think that every task has its own
    LDT and if right LDT is not in scope this is not good.  Could
    there be aliasing?  Hell, I don't know.  Where is the doc on
    this?  In the interest of safety, I will wait until Windows
    puts the lpPDevice into scope to use it.  To fix this, I
    allocate memory myself.

Return:         VOID
----------------------------------------------------------------------*/
#ifndef PERF_NEWMM
void _loadds WINAPI DoHostStage1(DWORD dwAddr)
{
   PDEVQUEUE pDevqueue;
   LPSTR lpBitMap;
   DWORD dwSrcAdj;
   DWORD dwDestAdj;

#ifdef ANDY_DEBUG
   DPF(DBGLVL_NORMAL, "Do Host %08lx\n", dwAddr);
#endif

   for (pDevqueue = (PDEVQUEUE)DevBitInUse.pRight; pDevqueue != (PDEVQUEUE)&DevBitInUse;)
      {
      if (dwAddr == pDevqueue->DevInfo.dwAddr)
         {
#ifdef SKIP_STAGE2

         LPDIBENGINE lpPDevice;

         // look's like there is....
         // Let's hope this works and there is no aliasing
         lpPDevice = (LPDIBENGINE)GlobalLock(pDevqueue->DevInfo.wHlpPDevice);

#ifdef STBPERF_DEVBITFIX
//PingZ 2/5/99 calling IsBadReadPtr is much safer than simply checking if the lpPDevice is NULL
//because it catches cases when lpPDevice is not NULL but points to a different memory.
         if( !IsBadReadPtr(lpPDevice, sizeof(DIBENGINE)) )
#else

         if (NULL != lpPDevice)
#endif
            {
            // Both of these checks are necessry to make sure that the DeviceBitmap has been
            // changed without tell us:
            // a.) First case -- DeviceBitmap has been deleted
            // b.) Second case -- DeviceBitmap has been deleted and is now being reused and we have already moved it  
            if ((TYPE_DIBENG == lpPDevice->deType) && (0x0 != lpPDevice->deDriverReserved)) 
               {
               lpBitMap = (LPSTR)MAKELP(SELECTOROF(lpPDevice), (WORD)pDevqueue->DevInfo.deBitsOffset);

               // Copy BitMap to Host
               dwSrcAdj = lpPDevice->deDeltaScan - lpPDevice->deWidthBytes;
               dwDestAdj = pDevqueue->DevInfo.wHostPitch - lpPDevice->deWidthBytes;
               DoBitCopy(lpPDevice->deBitsOffset, lpPDevice->deBitsSelector, (DWORD)OFFSETOF(lpBitMap), SELECTOROF(lpBitMap), dwSrcAdj, dwDestAdj, lpPDevice->deWidthBytes, lpPDevice->deHeight);

               // Fix it up so it looks like I have never been here
               (DWORD)lpPDevice->deBeginAccess = (DWORD)NULL;
               (DWORD)lpPDevice->deEndAccess = (DWORD)NULL;
               lpPDevice->deFlags &= ~(VRAM | OFFSCREEN);
               lpPDevice->deBitsOffset = OFFSETOF(lpBitMap);
               lpPDevice->deBitsSelector = SELECTOROF(lpBitMap);
               lpPDevice->deDeltaScan = pDevqueue->DevInfo.wHostPitch;

               // Clear Link so that we will Skip Mem Dealloc
               lpPDevice->deDriverReserved = 0x0;
               }
            // Unlock our boys to be free to roam the memory space of PC's near you
            GlobalUnlock(pDevqueue->DevInfo.wHlpPDevice);
            }

         // Reinit
         pDevqueue->DevInfo.deBitsOffset = 0x0;
         pDevqueue->DevInfo.wHostPitch = 0x0;
         pDevqueue->DevInfo.wHeight = 0x0;
         pDevqueue->DevInfo.wHlpPDevice = 0x0;
         pDevqueue->DevInfo.wStatus = DV_NOTINUSE;

         // Free Up DeviceBit Map Descriptor
         DDELETE(pDevqueue);

         //Add to Free List
         DINSERT((PDQUEUE)pDevqueue, DevBitFree)

         pDevqueue->DevInfo.dwAddr = 0x0;
         // Free Up Memory Allocated
         mmFree(dwAddr);

         break;
         }
      DRTRAVERSE(pDevqueue);
      }
#else  // SKIP_STAGE2

         pDevqueue->DevInfo.wHlpPDevice = GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, (DWORD)pDevqueue->DevInfo.wHostPitch * (DWORD)pDevqueue->DevInfo.wHeight);
         lpBitMap = (LPSTR)GlobalLock(pDevqueue->DevInfo.wHlpPDevice);
         // Copy BitMap to Host
         dwSrcAdj =  0x0;
         dwDestAdj = pDevqueue->DevInfo.wHostPitch - pDevqueue->DevInfo.wDevPitch;
         DoBitCopy(dwAddr, wFlatDataSel, (DWORD)OFFSETOF(lpBitMap), SELECTOROF(lpBitMap), dwSrcAdj, dwDestAdj, pDevqueue->DevInfo.wDevPitch, pDevqueue->DevInfo.wHeight);
         GlobalUnlock(pDevqueue->DevInfo.wHlpPDevice);
         pDevqueue->DevInfo.wStatus = DV_ONHOST;
#ifdef ANDY_DEBUG
         DPF(DBGLVL_NORMAL, "Address Hostified %08lx %04x %08lx \n", pDevqueue->DevInfo.dwAddr, pDevqueue->DevInfo.wStatus, lpBitMap);
#endif

         //pDevqueue->DevInfo.dwAddr = 0x0;
         // Free Up Memory Allocated
         mmFree(dwAddr);

         break;
         }
      DRTRAVERSE(pDevqueue);
      }
#endif

}
#endif // PERF_NEWMM

#ifndef SKIP_STAGE2
/*----------------------------------------------------------------------
Function name:  DoHostStage2

Description:    Move DeviceBitMaps into PDEVICE when OS asks to use it

Information:
    This function is used to do Stage 2 move.  At this point I have move
    the bitmap to the host but I would like to stop managing it.
    To do this I need to copy, update the pointers, update the structures then
    I am all throught with it.

Return:         VOID
----------------------------------------------------------------------*/
void _loadds DoHostStage2(LPDIBENGINE lpPDevice)
{
   LPSTR lpToBitMap;
   LPSTR lpFromBitMap;
   PDEVQUEUE pDevqueue;

   pDevqueue = (PDEVQUEUE)lpPDevice->deDriverReserved;
   if (NULL != pDevqueue)
      {
#ifdef ANDY_DEBUG
         DPF(DBGLVL_NORMAL, "Stage II %08lx %04x\n", pDevqueue->DevInfo.dwAddr, pDevqueue->DevInfo.wStatus);
#endif
      if (DV_ONHOST == pDevqueue->DevInfo.wStatus)
         {
#ifdef ANDY_DEBUG
         DPF(DBGLVL_NORMAL, "Found one\n");
#endif
         lpFromBitMap = (LPSTR)GlobalLock(pDevqueue->DevInfo.wHlpPDevice);
         lpToBitMap = (LPSTR)MAKELP(SELECTOROF(lpPDevice), (WORD)pDevqueue->DevInfo.deBitsOffset);

#ifdef ANDY_DEBUG
         DPF(DBGLVL_NORMAL, "Stage II Addr=%08lx From=%08lx To=%08lx\n", pDevqueue->DevInfo.dwAddr, lpFromBitMap, lpToBitMap);
#endif
         // Copy BitMap to Host
         DoBitCopy((DWORD)OFFSETOF(lpFromBitMap), SELECTOROF(lpFromBitMap), (DWORD)OFFSETOF(lpToBitMap), SELECTOROF(lpToBitMap), 0x0, 0x0, pDevqueue->DevInfo.wHostPitch, lpPDevice->deHeight);

         // Fix it up so it looks like I have never been here
         (DWORD)lpPDevice->deBeginAccess = (DWORD)NULL;
         (DWORD)lpPDevice->deEndAccess = (DWORD)NULL;
         lpPDevice->deFlags &= ~(VRAM | OFFSCREEN);
         lpPDevice->deBitsOffset = OFFSETOF(lpToBitMap);
         lpPDevice->deBitsSelector = SELECTOROF(lpToBitMap);
         lpPDevice->deDeltaScan = pDevqueue->DevInfo.wHostPitch;

         // Clear Link so that we will Skip Mem Dealloc
         lpPDevice->deDriverReserved = 0x0;

         // Unlock our boys to be free to roam the memory space of PC's near you
         GlobalUnlock(pDevqueue->DevInfo.wHlpPDevice);

         // Free Up the Host memory
         GlobalFree(pDevqueue->DevInfo.wHlpPDevice);

         // Reinit
         pDevqueue->DevInfo.deBitsOffset = 0x0;
         pDevqueue->DevInfo.wHostPitch = 0x0;
         pDevqueue->DevInfo.wDevPitch = 0x0;
         pDevqueue->DevInfo.wHeight = 0x0;
         pDevqueue->DevInfo.wHlpPDevice = 0x0;
         pDevqueue->DevInfo.wStatus = DV_NOTINUSE;

         // Free Up DeviceBit Map Descriptor
         DDELETE(pDevqueue);

         //Add to Free List
         DINSERT((PDQUEUE)pDevqueue, DevBitFree)
         }
   }
}
#endif

/*----------------------------------------------------------------------
Function name:  EnableDeviceBitmaps

Description:    Runtime enable/disable of the device bitmaps.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void
EnableDeviceBitmaps()
{
    if (TRUE == DevFilter.nDevBitsEnabled)
    {
        _FF(ddMiscFlags) |= DDMF_ENABLE_DEVICEBITMAPS;
    }
    else // if filter says disable device bitmaps, do so.
    {
        _FF(ddMiscFlags) &= ~DDMF_ENABLE_DEVICEBITMAPS;
    }

}


/*----------------------------------------------------------------------
Function name:  DisableDeviceBitmaps

Description:    Runtime disable of the device bitmaps.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void
DisableDeviceBitmaps()
{
    _FF(ddMiscFlags) &= ~DDMF_ENABLE_DEVICEBITMAPS;
}


/*----------------------------------------------------------------------
Function name:  DeleteObject

Description:    Delete a previously created physical object.

Information:

Return:         VOID
----------------------------------------------------------------------*/
#ifndef PERF_NEWMM
void DeleteObject(LPDIBENGINE lpDevBit)
{
   PDEVQUEUE pDevqueue;
   WORD wHlpPDevice = GlobalPtrHandle(lpDevBit);

   for (pDevqueue = (PDEVQUEUE)DevBitInUse.pRight; pDevqueue != (PDEVQUEUE)&DevBitInUse; )
      {

      if (wHlpPDevice == pDevqueue->DevInfo.wHlpPDevice)
         {
         if (lpDevBit != NULL)
            {
#ifdef DEBUG
            if (pDevqueue != (PDEVQUEUE)lpDevBit->deDriverReserved)
               _asm int 03;
#endif
            }
         // Free Up Memory Allocator
         // Is this a Offscreen|VRAM or a Hostified BitMap
         if (DV_OFFSCREEN == pDevqueue->DevInfo.wStatus)
            mmFree( pDevqueue->DevInfo.dwAddr );

#ifdef DEBUG
         else
            _asm int 03;
#endif

         // Free Up DeviceBit Map Descriptor
         DDELETE(pDevqueue);

         //Add to Free List
         DINSERT((PDQUEUE)pDevqueue, DevBitFree)

         return;
         }
      DRTRAVERSE(pDevqueue);
      }
}
#endif // PERF_NEWMM


#define DEBUG_FIX	// as nothing
#define MYCMDFIFO	lph3agp->PRIMARY_CMDFIFO

#pragma warning (disable: 4704)
#include "..\minivdd\agpcf.c"
#pragma warning (default: 4704)



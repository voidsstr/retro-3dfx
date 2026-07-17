/* -*-c++-*- */
/* $Header: drv2.c, 2, 10/11/00 8:51:03 PM, Brent$ */
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** Portions Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
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
**
** File name:   drv2.c
**
** Description: Multiple adapter support functions
**
** $Revision: 2$
** $Date: 10/11/00 8:51:03 PM$
**
** $History: drv2.c $
** 
** *****************  Version 10  *****************
** User: Edwin        Date: 6/03/99    Time: 9:38a
** Updated in $/devel/h5/Win9x/dx/dd16
** Declare external reference for tvoutSetStdInternal() so we will compile
** with -WX.
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 5/10/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed PhysScreenAddr to RealregBase
** 
** *****************  Version 7  *****************
** User: Stuartb      Date: 3/12/99    Time: 3:38p
** Updated in $/devel/h3/Win95/dx/dd16
** Move call to tvoutSetStdInternal() to before call to
** HWEnable(ENABLE_MEM) so I/O is enabled for secondary adapter.
** 
** *****************  Version 6  *****************
** User: Stuartb      Date: 3/11/99    Time: 2:46p
** Updated in $/devel/h3/Win95/dx/dd16
** Added call to tvoutSetStdInternal to kick tvout on boot of secondary
** adapter.
** 
** *****************  Version 5  *****************
** User: Xingc        Date: 3/11/99    Time: 10:19a
** Updated in $/devel/h3/Win95/dx/dd16
** Move PRE_MODE_CHANGE VDDCall into setmode.c
** 
** *****************  Version 4  *****************
** User: Xingc        Date: 3/10/99    Time: 8:57p
** Updated in $/devel/h3/Win95/dx/dd16
** Add PRE_MODE_CHANGE call in ValidateMode()
** 
** *****************  Version 3  *****************
** User: Michael      Date: 12/28/98   Time: 11:24a
** Updated in $/devel/h3/Win95/dx/dd16
** Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
** files where this was missing.
** 
**
*/

#ifndef _WIN32
#define NOUSER
#define NOGDI
#define NOGDIOBJ
#define NOGDICAPMASKS
#include <windows.h>

#define NOEXTDEVMODEPROPSHEET       // no property sheet structs in print.h
#define LPDOCINFO void * // avoids a compile error in print.h
#include <print.h>                  // to get DEVMODE struct.

#define NOPTRC
#define PTTYPE POINT
#include <gdidefs.inc>
#include <dibeng.inc>
#include <minivdd.h>
/* Icon/Cursor header */
typedef struct
{
   int     xHotSpot, yHotSpot;
   int     cx, cy;
   int     cbWidth;
   BYTE    Planes, BitsPixel;
} CURSORSHAPE;
#endif // _WIN32
#include "valmode.inc"

#ifdef TRACE
	#define DPF Msg
#else
#ifndef THUNK32
    #define DPF         1 ? (void)0 : (void)
    #define BREAK()
#endif // #ifndef THUNK32
#endif

/***************************************************************************
 *
 * globals
 *
 ***************************************************************************/
DWORD dwDeviceHandle=0;
DISPLAYINFO DisplayInfo;
DWORD dwDevNode = 0;
/***************************************************************************
 *
 * internal functions.
 *
 ***************************************************************************/
DWORD VDDCall(DWORD myEAX, DWORD VDDmagicNumber, DWORD myECX, DWORD myEDX, LPVOID esdi);
#if 0
DWORD VDDCall(DWORD DeviceHandle, DWORD function, DWORD flags, LPVOID buffer, DWORD buffer_size);
#endif
#if 0
UINT FAR PASCAL _loadds CreatePhysSel(DWORD PhysAddress, DWORD PhysSize);
void FreeSel(UINT Sel);
DWORD PhysToLinear(DWORD PhysAddress, DWORD Size);
#endif
#define VDDOpenDevice(sz)   VDDCall(VDD_OPEN, 0, 0, 0, sz)
#define VDDCloseDevice(h)   VDDCall(VDD_CLOSE, h, 0, 0, NULL)
#define HWEnable(f)  ((dwDeviceHandle==1) ? 0 : VDDCall(VDD_ENABLE, dwDeviceHandle, 0, f, NULL))
/***************************************************************************
 *
 * external functions.
 *
 ***************************************************************************/
extern UINT  FAR PASCAL Enable1        (LPVOID, UINT, LPSTR, LPSTR, LPVOID);
extern UINT  FAR PASCAL ReEnable1      (LPVOID, LPVOID);
extern UINT  FAR PASCAL Disable1       (DIBENGINE FAR *);
extern LONG  FAR PASCAL Control1       (DIBENGINE FAR *, UINT, LPVOID, LPVOID);
extern UINT  FAR PASCAL SetPalette1    (UINT start, UINT count, DWORD FAR *lpPalette);
extern UINT  FAR PASCAL ValidateMode1  (DISPVALMODE FAR *);
extern UINT  FAR PASCAL ValidateDesk1  (DISPVALMODE FAR *);
extern int   tvoutSetStdInternal(void);

BOOL  FAR PASCAL CanHWRunAsSecondary (void);
/***************************************************************************
 *
 * DEBUG stuff
 *
 ***************************************************************************/

#ifdef DEBUG
extern void FAR __cdecl DPF(LPSTR szFormat, ...);
#define BREAK() DebugBreak();
#else
#define DPF         1 ? (void)0 : (void)
#define BREAK()
#define BREAK()
#endif
/*----------------------------------------------------------------------
Function name:  Enabled

Description:    Called by GDI to enable the device and set the
                video mode.
Information:

Return:         UINT   Size or structure filled in or,
                        TRUE if success or,
                        FALSE if failure.
----------------------------------------------------------------------*/
#pragma optimize("gle", off)

UINT FAR PASCAL _loadds Enable(
                              LPVOID  lpDevice,
                              UINT        style,
                              LPSTR       lpDeviceName,
                              LPSTR       lpOutput,
                              LPVOID      lpStuff)
{
   BOOL    f;
   UINT size;
   DIBENGINE FAR *  pdevice;
   pdevice=(DIBENGINE FAR *)lpDevice;

        //
        // open the device if we have not done it yet.
        //

   if (dwDeviceHandle==0)
   {
      if (lpDeviceName)
      {
         dwDeviceHandle = VDDOpenDevice(lpDeviceName);
         DPF("we got a device name '%s' h=%08lX",lpDeviceName,dwDeviceHandle);
         if (dwDeviceHandle == 0 || dwDeviceHandle == 0xFFFFFFFF)
         {
            DPF("fail on VDDOpenDevice call");
            return FALSE;
         }
         else
         {
            DPF("Enable as external driver.");
         }
      }
      else
      {
         dwDeviceHandle = 1;
         DPF("Enable as the DISPLAY driver.");
      }
   }
   //_fmemset(&DisplayInfo, 0, sizeof(DisplayInfo));
   DisplayInfo.diDevNodeHandle = 0x0;
   VDDCall(VDD_GET_DISPLAY_CONFIG, dwDeviceHandle, sizeof(DisplayInfo), 0,
           &DisplayInfo);
   dwDevNode = DisplayInfo.diDevNodeHandle;
#ifdef DEBUG
   DPF("DISPLAY:Settings %dx%dx%d %d-%dHz [%dx%d]",
       (int)DisplayInfo.diXRes,
       (int)DisplayInfo.diYRes,
       (int)DisplayInfo.diBpp,
       (int)DisplayInfo.diRefreshRateMin,
       (int)DisplayInfo.diRefreshRateMax,
       (int)DisplayInfo.diXDesktopSize,
       (int)DisplayInfo.diYDesktopSize);
        //
        //  or use the DEVMODE passed to CreateDC
        //
   if (lpStuff && dwDeviceHandle > 1)
   {
      DEVMODE FAR *pdm = (DEVMODE FAR *)lpStuff;

      DPF("we got passed a DEVNODE %dx%dx%d @ %dHz.",
          (int)pdm->dmPelsWidth,
          (int)pdm->dmPelsHeight,
          (int)pdm->dmBitsPerPel,
          (int)pdm->dmDisplayFrequency);

      if (pdm->dmFields & DM_BITSPERPEL)
      {
         DisplayInfo.diBpp = (BYTE)pdm->dmBitsPerPel;
      }

      if (pdm->dmFields & (DM_PELSWIDTH|DM_PELSHEIGHT))
      {
         DisplayInfo.diXRes         = (UINT)pdm->dmPelsWidth;
         DisplayInfo.diYRes         = (UINT)pdm->dmPelsHeight;
         DisplayInfo.diXDesktopSize = pdm->dmPelsWidth;
         DisplayInfo.diYDesktopSize = pdm->dmPelsHeight;
      }

      if (pdm->dmFields & DM_DISPLAYFREQUENCY)
      {
         DisplayInfo.diRefreshRateMin = (UINT)pdm->dmDisplayFrequency;
         DisplayInfo.diRefreshRateMax = (UINT)pdm->dmDisplayFrequency;
      }
      DPF("External:Settings %dx%dx%d %d-%dHz [%dx%d]",
          (int)DisplayInfo.diXRes,
          (int)DisplayInfo.diYRes,
          (int)DisplayInfo.diBpp,
          (int)DisplayInfo.diRefreshRateMin,
          (int)DisplayInfo.diRefreshRateMax,
          (int)DisplayInfo.diXDesktopSize,
          (int)DisplayInfo.diYDesktopSize);
   }

        //
        // now validate the desktop
        //
   if (DisplayInfo.diXDesktopSize < (DWORD)DisplayInfo.diXRes ||
       DisplayInfo.diYDesktopSize < (DWORD)DisplayInfo.diYRes)
   {
      DisplayInfo.diXDesktopSize = DisplayInfo.diXRes;
      DisplayInfo.diYDesktopSize = DisplayInfo.diYRes;
   }
#endif //DEBUG
        //
        // If we are on the primary device, just vector off to the regular Enable()
        //
   if (dwDeviceHandle == 1)
      return Enable1(lpDevice,style,lpDeviceName,lpOutput,lpStuff);
        //
        // to enable this HW, call to see if the chipset is supported.
        //
   if (HWEnable(ENABLE_ALL) == ENABLE_ERROR)
   {
      DPF("HWEnable failed");
      return FALSE;
   }
   f = CanHWRunAsSecondary();

   if (f)
   {
      DPF("ok on CanHWRunAsSecondary() call");
      size=Enable1(lpDevice,style,lpDeviceName,lpOutput,lpStuff);
      if (size == 0)
      {
         DPF("Enable: failed on %d call,InquireInfo=%d,EnableDevice=%d ",
                                                             style,InquireInfo,EnableDevice);
         HWEnable(ENABLE_NONE);
         return FALSE;
      }
      if (style == InquireInfo)
      {
         HWEnable(ENABLE_NONE);
      }
      else
      {
#if 0
//
//#ifdef DEBUG // For now, we are going to leave this enabled!!!
// Check to see if any VGA resources are still allocated. If so, output a gross debug
// message!

         BYTE far *lpA000;
         BYTE far *lpB000;
         BYTE far *lpB800;

         lpA000 = MAKELP(CreatePhysSel(0xa0000l, 32l*1024l),0);
         lpB000 = MAKELP(CreatePhysSel(0xb0000l, 32l*1024l),0);
         lpB800 = MAKELP(CreatePhysSel(0xb8000l, 32l*1024l),0);

         if (*lpA000 != 0xff) 
         {
            OutputDebugString("ERROR: A000 memory mapped in!\n\r");
#ifdef DEBUG // Put this in DEBUG for Beta 2
         _asm int 1
#endif
         }
         if (*lpB000 != 0xff) 
         {
            OutputDebugString("ERROR: B000 memory mapped in!\n\r");
#ifdef DEBUG // Put this in DEBUG for Beta 2
         _asm int 1
#endif
         }

         if (*lpB800 != 0xff) 
         {
            OutputDebugString("ERROR: B800 memory mapped in!\n\r");
#ifdef DEBUG // Put this in DEBUG for Beta 2
         _asm int 1
#endif
         }

         FreeSel(SELECTOROF(lpA000));
         FreeSel(SELECTOROF(lpB000));
         FreeSel(SELECTOROF(lpB800));
#endif
//#endif //DEBUG
#ifndef WIN_CSIM
	     tvoutSetStdInternal();
#endif
         HWEnable(ENABLE_MEM);   // leave memory access on if style == EnableDevice
      }
      return size;
   }
   else
   {
      DPF("failed on CanHWRunAsSecondary() call");
      HWEnable(ENABLE_NONE);
      return FALSE;
   }

}


/*----------------------------------------------------------------------
Function name:  ReEnable

Description:    Called by GDI to ReEnable the device and set
                the video mode
Information:

Return:         UINT   Value returned by ReEnable1 call or,
                        FALSE if HWEnable failed.
----------------------------------------------------------------------*/
#pragma optimize("", on)

UINT FAR PASCAL _loadds ReEnable(
                                LPVOID  lpDevice,
                                LPVOID  lpGDIINFO)
{
   UINT  f=FALSE;
   DPF("ReEnable");
   if (dwDeviceHandle == 1) return ReEnable1(lpDevice,lpGDIINFO);

   if (HWEnable(ENABLE_ALL) != ENABLE_ERROR)
   {
      f=ReEnable1(lpDevice,lpGDIINFO);
   }
   HWEnable(ENABLE_MEM);
   return f;
}


/*----------------------------------------------------------------------
Function name:  Disable

Description:    Called by GDI to Disable the device.

Information:

Return:         UINT   Value returned by Disable1 call or,
                        FALSE if failure.
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds Disable(DIBENGINE FAR *pde)
{
   UINT  f=FALSE;
   DPF("Disable");
   if (dwDeviceHandle == 1) return Disable1(pde);
   if (HWEnable(ENABLE_ALL) != ENABLE_ERROR)
   {
      f=Disable1(pde);
   }
   HWEnable(ENABLE_NONE);
   VDDCloseDevice(dwDeviceHandle);
   dwDeviceHandle = 0;
   return f;
}


/*----------------------------------------------------------------------
Function name:  ValidateMode

Description:    Called by GDI to verify whether the screen size
                is supported
Information:
    We have to make sure screen size <= desktop size
    Actually system verifies it before calling here, I am not sure if
    we have to verify again.
    This code assumes that the driver will NOT be loaded and
    ValidateMode called on the secondary driver.

Return:         UINT   Value returned by ValidateMode1 call or,
                        VALMODE_YES if dwDeviceHandle == 0 or,
                        0 if failure.
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds ValidateMode(DISPVALMODE FAR *pdvm)
{
   UINT  result=0;
   DWORD dw;
   DPF("ValidateMode");
   if (dwDeviceHandle == 0)
      return VALMODE_YES;

   if ((dw = HWEnable(ENABLE_ALL)) != ENABLE_ERROR)
   {
      result=ValidateMode1(pdvm);
      HWEnable(dw);
   }


   return result;
}

#define NO_VALIDATEDESK
#ifndef NO_VALIDATEDESK
/*----------------------------------------------------------------------
Function name:  ValidateDesk

Description:    Called by GDI to determine whether the driver/board
                can enable the given desktop size.
Information:

Return:         UINT   VALMODE_YES (0) or reason code
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds ValidateDesk(DISPVALMODE FAR *pdvm)
{
   UINT  result=0;
   DWORD dw;

   DPF("ValidateDesk");
   if (dwDeviceHandle == 0)
      return ValidateDesk1(pdvm);
   if ((dw = HWEnable(ENABLE_ALL)) != ENABLE_ERROR)
   {
      result=ValidateDesk1(pdvm);
      HWEnable(dw);
   }
   return result;
}
#endif // NO_VALIDATEDESK


/*----------------------------------------------------------------------
Function name:  SetPalette

Description:    Called by GDI to set the palette.

Information:

Return:         UINT   Value returned by SetPalette1 call or,
                       0 if failure.
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds SetPalette(
                                  UINT        start,
                                  UINT        count,
                                  DWORD FAR  *lpPalette)
{
   UINT rc=0;
   DWORD dw;

   DPF("SetPalette");
   if (dwDeviceHandle == 1) return SetPalette1(start, count, lpPalette);

   if ((dw = HWEnable(ENABLE_VGA)) != ENABLE_ERROR)
   {
      rc=SetPalette1(start, count, lpPalette);
      HWEnable(dw);
   }
   return rc;
}


/*----------------------------------------------------------------------
Function name:  Control

Description:    This is what GDI calls when a app calls Escape or
                ExtEscape.  If you don't handle a escape make sure to
                pass it to the DIBENG.

Information:

Return:         LONG   Value returned by Control1 call or,
                       0 if failure.
----------------------------------------------------------------------*/
LONG FAR PASCAL _loadds Control(
                               DIBENGINE FAR * lpDevice,
                               UINT     function,
                               LPVOID      lpInput,
                               LPVOID      lpOutput)
{
   LONG rc=0;
   DWORD   dw;

   if (dwDeviceHandle == 1) return Control1(lpDevice,function,lpInput,lpOutput);

   if ((dw = HWEnable(ENABLE_VGA)) != ENABLE_ERROR)
   {
      rc=Control1(lpDevice,function,lpInput,lpOutput);
      HWEnable(dw);
   }
   return rc;
}


/*----------------------------------------------------------------------
Function name:  VDDCall

Description:    Makes an INT 2F call into the display driver's VxD.

Information:    Function is not presently used.

Return:         DWORD   Result of call if success or,
                        -1L if failure.
----------------------------------------------------------------------*/
#pragma optimize("", on)

#if 0
#pragma optimize("gle", off)
DWORD VDDCall(DWORD dev, DWORD function, DWORD flags, LPVOID buffer, DWORD buffer_size)
{
   static DWORD   VDDEntryPoint=0;
   DWORD   result=0xFFFFFFFF;

   if (VDDEntryPoint == 0)
   {
      _asm
      {
         xor     di,di           ;set these to zero before calling
         mov     es,di           ;
         mov     ax,1684h        ;INT 2FH: Get VxD API Entry Point
         mov     bx,0ah          ;this is device code forVDD
         int     2fh             ;call the multiplex interrupt
         mov     word ptr VDDEntryPoint[0],di    ;
         mov     word ptr VDDEntryPoint[2],es    ;save the returned data
      }

      if (VDDEntryPoint == 0)
         return result;
   }
   _asm
   {
      _emit 66h _asm push si                       ; push esi
      _emit 66h _asm push di                       ; push edi
      _emit 66h _asm mov ax,word ptr function      ;eax = function
      _emit 66h _asm mov bx,word ptr dev           ;ebx = device
      _emit 66h _asm mov cx,word ptr buffer_size   ;ecx = buffer_size
      _emit 66h _asm mov dx,word ptr flags         ;edx = flags
      _emit 66h _asm xor di,di                     ; HIWORD(edi)=0
        les     di,buffer
        mov     si,es                               ;si=es
        call    dword ptr VDDEntryPoint             ;call the VDD's PM API
        cmp     ax,word ptr function
        je      fail
      _emit 66h _asm mov word ptr result,ax
fail: _emit 66h _asm pop di                        ; pop edi
      _emit 66h _asm pop si                        ; pop esi
   }

   return result;
}


/*----------------------------------------------------------------------
Function name:  DFP

Description:    Wrapper for debugger output.

Information:    Used only for debugging.

Return:         VOID
----------------------------------------------------------------------*/
#pragma optimize("", on)
#ifdef DEBUG

void FAR __cdecl DPF(LPSTR szFormat, ...)
{
   static int (WINAPI *fpwvsprintf)(LPSTR lpszOut, LPCSTR lpszFmt, const void FAR* lpParams);
   char str[256];

   if (fpwvsprintf == NULL)
   {
      fpwvsprintf = (LPVOID) GetProcAddress(GetModuleHandle("USER"),"wvsprintf");
      if (fpwvsprintf == NULL)
         return;
   }

#ifdef DRVNAME
   lstrcpy(str, DRVNAME ": ");
   fpwvsprintf(str+lstrlen(str), szFormat, (LPVOID)(&szFormat+1));
#else
   fpwvsprintf(str, szFormat, (LPVOID)(&szFormat+1));
#endif

   lstrcat(str, "\r\n");
   OutputDebugString(str);
}

#endif


/*----------------------------------------------------------------------
Function name:  CreateSel

Description:    Calls INT 31h to create a selector.

Information:    

Return:         UINT    Value of the selector.
----------------------------------------------------------------------*/
#pragma optimize("", off)
static UINT CreateSel(DWORD base, DWORD limit)
{
    UINT Sel;

    Sel = AllocSelector(SELECTOROF((LPVOID)&dwDeviceHandle));

    if (Sel == 0)
        return 0;

    SetSelectorBase(Sel, base);
                
    // SetSelectorLimit(FlatSel, -1);
    _asm    mov     ax,0008h            ; DPMI set limit
    _asm    mov     bx,Sel
    _asm    mov     dx,word ptr limit[0]
    _asm    mov     cx,word ptr limit[2]
    _asm    int     31h

    return Sel;
}
#pragma optimize("", on)


/*----------------------------------------------------------------------
Function name:  FreeSel

Description:    Frees a previous allocated selector.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
static void FreeSel(UINT Sel)
{
    if (Sel)
    {
        SetSelectorLimit(Sel, 0);
        FreeSelector(Sel);
    }
}


/*----------------------------------------------------------------------
Function name:  CreatePhysSel

Description:    Calls CreateSel to create a physical selector.

Information:    

Return:         UINT    Value returned from CreateSel call
----------------------------------------------------------------------*/
static UINT FAR PASCAL _loadds CreatePhysSel(DWORD PhysAddress, DWORD PhysSize)
{
    return CreateSel(PhysToLinear(PhysAddress, PhysSize), PhysSize-1);
}


/*----------------------------------------------------------------------
Function name:  GetFlatSel

Description:    Calls CreateSel to create a Flat selector.

Information:    

Return:         UINT    Value returned from CreateSel call
----------------------------------------------------------------------*/
static UINT GetFlatSel(UINT FlatSel)
{
    if (FlatSel != 0)
        return FlatSel;

    FlatSel = CreateSel(0, 0xFFFFFFFF);
    return FlatSel;
}


/*----------------------------------------------------------------------
Function name:  FreeFlatSel

Description:    Calls FreeSel to free a previously created a
                Flat selector.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
static void FreeFlatSel(UINT FlatSel)
{
    if (FlatSel)
    {
        FreeSel(FlatSel);
        FlatSel = 0;
    }
}


/*----------------------------------------------------------------------
Function name:  PhysToLinear

Description:    Calls INT 31h to do a physical to linear tranlation.

Information:    

Return:         DWORD   The linear address
----------------------------------------------------------------------*/
#pragma optimize("", off)
static DWORD PhysToLinear(DWORD PhysAddress, DWORD PhysSize)
{
    DWORD LinearAddress;

    PhysSize = PhysSize-1;      // we want limit, not size for DPMI

    _asm
    {
        mov     cx, word ptr PhysAddress[0]
        mov     bx, word ptr PhysAddress[2]
        mov     di, word ptr PhysSize[0]
        mov     si, word ptr PhysSize[2]
        mov     ax, 0800h               ; DPMI phys to linear
        int     31h
        mov     word ptr LinearAddress[0], cx
        mov     word ptr LinearAddress[2], bx
    }

    return LinearAddress;
}
#endif


/*----------------------------------------------------------------------
Function name:  CanHWRunAsSecondary 

Description:    Query if we support a secondary adapter

Information:    

Return:         BOOL    TRUE is always returned
----------------------------------------------------------------------*/
// Hell, yes... is this not the point of all this stuff
BOOL  FAR PASCAL CanHWRunAsSecondary (void)
{
   return TRUE;
}

/* -*-c++-*- */
/* $Header: cursor.c, 9, 10/11/00 8:50:51 PM, Brent$ */
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   cursor.c
**
** Description: Implements the hardware cursor functions.
**
** $Revision: 9$
** $Date: 10/11/00 8:50:51 PM$
**
** $History: cursor.c $
** 
** *****************  Version 69  *****************
** User: Andrew       Date: 8/23/99    Time: 6:59p
** Updated in $/devel/h5/Win9x/dx/dd16
** code changes for Hostbased cursor
** 
** *****************  Version 68  *****************
** User: Andrew       Date: 8/04/99    Time: 12:59p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added Source Clip
** 
** *****************  Version 67  *****************
** User: Andrew       Date: 7/27/99    Time: 4:36p
** Updated in $/devel/h5/Win9x/dx/dd16
** Removed ifndef WIN_CSIM from around SW Cursor
** 
** *****************  Version 66  *****************
** User: Edwin        Date: 6/29/99    Time: 3:57p
** Updated in $/devel/h5/Win9x/dx/dd16
** Remove obsolete Banshee ifdefs.
** 
** *****************  Version 65  *****************
** User: Andrew       Date: 6/25/99    Time: 4:30p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added function to Switch the Cursor to the Host Base Cursor
** 
** *****************  Version 64  *****************
** User: Andrew       Date: 6/17/99    Time: 2:22p
** Updated in $/devel/h5/Win9x/dx/dd16
** removed HAL_CSIM
** 
** *****************  Version 63  *****************
** User: Andrew       Date: 6/16/99    Time: 4:45p
** Updated in $/devel/h5/Win9x/dx/dd16
** Removed unsed variable Offset and changed to use SKIP_FLAGS
** 
** *****************  Version 62  *****************
** User: Andrew       Date: 6/04/99    Time: 4:09p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added code to support Host Base Cursor
** 
** *****************  Version 60  *****************
** User: Andrew       Date: 5/19/99    Time: 3:58p
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to Always use Real FB when in CSIM
** 
** *****************  Version 59  *****************
** User: Andrew       Date: 5/13/99    Time: 4:11p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed ifdef WIN_CSIM since lfbBase is real lfb Base
** 
** *****************  Version 58  *****************
** User: Andrew       Date: 5/12/99    Time: 10:44a
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to use real lfb when updating the cursor when in WIN_CSIM
** mode
** 
** *****************  Version 57  *****************
** User: Andrew       Date: 5/10/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed PhysScreenAddr to RealregBase
** 
** *****************  Version 56  *****************
** User: Andrew       Date: 5/06/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Modified to work with Trapping "C" Simulator
** 
** *****************  Version 55  *****************
** User: Andrew       Date: 3/03/99    Time: 2:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Moved Back to Version 52 since Version 53 had a bug with certain
** applications and restorecursor.  The original fix was believed to get
** better performance but it is unknown if it does.
** 
** *****************  Version 53  *****************
** User: Stb_srogers  Date: 2/22/99    Time: 7:32a
** Updated in $/devel/h3/win95/dx/dd16
** Changes outward appearance of 2048 to 2046
** 
** *****************  Version 52  *****************
** User: Andrew       Date: 2/11/99    Time: 4:50p
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to support Mono-Chrome SW cursor and kick it on at
** 2048,1536.
** 
** *****************  Version 51  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:35a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 50  *****************
** User: Michael      Date: 12/24/98   Time: 1:21p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB Unified Header Initiative.  
** 
** *****************  Version 49  *****************
** User: Andrew       Date: 10/26/98   Time: 12:57p
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed a problem with GetCursorFormat where 4BPP would not set return
** value.  This would cause problems with S3 & Multimonitor.
** 
** *****************  Version 48  *****************
** User: Andrew       Date: 10/06/98   Time: 8:54a
** Updated in $/devel/h3/Win95/dx/dd16
** Changed the way we draw cursor to support XOR cursors
** 
** *****************  Version 47  *****************
** User: Andrew       Date: 9/11/98    Time: 10:08a
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to handle 4BPP cursors and to fix a problem with large
** negative X & Ys.
** 
** *****************  Version 46  *****************
** User: Andrew       Date: 9/10/98    Time: 12:12p
** Updated in $/devel/h3/Win95/dx/dd16
** Added a fix to Handle Cursor Format BPP != Screen Format BPP
** 
** *****************  Version 45  *****************
** User: Andrew       Date: 9/02/98    Time: 12:07p
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed a bug with unreal and 8 bpp
** 
** *****************  Version 44  *****************
** User: Michael      Date: 8/27/98    Time: 9:39p
** Updated in $/devel/h3/Win95/dx/dd16
** Add a hack to pass WHQL PC98, GDI-Windows, AMovie test.  The app
** (Active Movie?)  passes in bad data in POINTERINFO to SetCursor.  We
** test for bogus or corrupted data and just disregard it.  Fixes PRS #s
** 1784 & 2337.
** 
** *****************  Version 43  *****************
** User: Andrew       Date: 8/25/98    Time: 12:33a
** Updated in $/devel/h3/Win95/dx/dd16
** Removed SaveCursorExclude from Init and added check to see if cursor
** BPP = screen BPP before using Sw cursor
** 
** *****************  Version 42  *****************
** User: Andrew       Date: 8/21/98    Time: 6:12p
** Updated in $/devel/h3/Win95/dx/dd16
** Added Check in SWEnableCursor to check if cursor was enabled before
** erasing.  Fixes problem with flip 3d
** 
** *****************  Version 41  *****************
** User: Andrew       Date: 8/18/98    Time: 10:20p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed HotspotX to be precalculated on LastCursorPos since Save &
** Restore must sync on the actual values.  Changed so that HotSpotX/Y is
** recalculated on values used by SaveCursorExclude/DrawCursor to remove
** dropped Cursor erase problem.
** 
** *****************  Version 40  *****************
** User: Andrew       Date: 8/16/98    Time: 9:42p
** Updated in $/devel/h3/Win95/dx/dd16
** Added Standard Header
*/

/***************************************************************************
 *
 *  Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       cursor.c
 *  Content:    based on sample Windows95 display driver
 *
 *      contains the hardware cursor functions
 *
 *  public functions:
 *      InitCursor
 *      DoMouseTrails
 *
 ***************************************************************************/
#include "header.h"
#include "fifomgr.h"
#include "cursor.h"
#define Not_VxD
#include "minivdd.h"
#include <string.h>
#include <modelist.h>
#include <entrleav.h>


POINTERINFO SaveCursor;

typedef void (FNENABLECURSOR)(int);
typedef BOOL (FNMOVECURSOR)(SHORT, SHORT);
typedef BOOL (FNSETCURSOR)(CURSORSHAPE FAR *);

DWORD dwCursorLoc;      // Linear Address of Offscreen Cursor
FNENABLECURSOR * pEnableCursor;
FNMOVECURSOR * pMoveCursor;
FNSETCURSOR * pSetCursor;
FNSAVECURSOREXCLUDE * pSaveCursorExclude;
FNRESTORECURSOREXCLUDE * pRestoreCursorExclude;
FNDRAWCURSOR * pDrawCursor;

int SetUpCursorFunctions(CURSORSHAPE FAR * lpCursor);
void DoCursor(DWORD Offset, DWORD FAR * lpAnd);
void DoCursorDblX(DWORD Offset, DWORD FAR * lpAnd);
void DoBigCursor(DWORD Offset, DWORD FAR * lpAnd);

// DIB engine Cursor Routines
FNMOVECURSOR DIBMoveCursor;
FNSETCURSOR DIBSetCursor;
FNENABLECURSOR DIBEnableCursor;
// Software Cursor Routines
FNMOVECURSOR SWMoveCursor;
FNSETCURSOR SWSetCursor;
FNENABLECURSOR SWEnableCursor;
// Banshee Cursor Routines
FNMOVECURSOR BansheeMoveCursor;
FNSETCURSOR BansheeSetCursor;
FNENABLECURSOR BansheeEnableCursor;

// Extra Function for when we are in SLI/AA mode
FNSAVECURSOREXCLUDE SaveCursorExclude;
FNRESTORECURSOREXCLUDE RestoreCursorExclude;
FNDRAWCURSOR DrawCursor;

#ifdef SLI_AA
FNENABLECURSOR HostEnableCursor;
FNSETCURSOR HostSetCursor;
FNSAVECURSOREXCLUDE HostSaveCursorExclude;
FNRESTORECURSOREXCLUDE HostRestoreCursorExclude;
FNDRAWCURSOR HostDrawCursor;
#endif


// External Pointer to Register
#ifdef WIN_CSIM
static SstIORegs     *lph3IORegs;
#else
extern SstIORegs *lph3IORegs;
#endif
int ShowCursor(int nX, int nY);
void GetBinary(DWORD dwDevNodeHandle, WORD FAR * lpValue, char FAR * lpStr, WORD nDefault);

DWORD movecursorcount = 0;
DWORD setcursorcount = 0;
int nDIBCursor;

extern void FXWAITFORIDLE();
extern DISPLAYINFO DisplayInfo;
extern int bScanlineDouble;    // in setmode.c

#define HWCURSOR        0x0100 //;driver is using a HWCURSOR right now.

#define SSTCP_PKT4_NOPCMD  ((0x48L << SSTCP_REGBASE_SHIFT))

#if 0
#define CMDFIFO_BUILD_PK4( mask, regName, chip )\
    ( ((mask) << SSTCP_PKT4_MASK_SHIFT) | (chip << 11L)\
    | (regName) | SSTCP_PKT4 )
#endif

#define CMDFIFO_BUILD_PK4 0x8244L

#ifdef CMDFIFO
#define WAXBUG_3DNOPFIX \
  { \
    SETPH( cmdFifo, CMDFIFO_BUILD_PK4); \
    SET(cmdFifo, ghw3D->nopCMD, 0); \
  }
#else
#define WAXBUG_3DNOPFIX \
  { \
    SstRegs * ghw3D = (SstRegs*)(_FF(regBase[HWINFO_SST_3DREGS_INDEX]));\
    SET(cmdFifo, ghw3D->nopCMD, 0); \
  }
#endif

#ifdef RD_ABORT_ERROR
void Modify_SLI_Read(DWORD dwRequest);
#define SET_SLI_READ(Request) {\
   if (_FF(dwSLIMode) != H3VDD_SLI_READ_NOT_IN_USE) \
      {\
      Modify_SLI_Read(Request);\
      }\
   }
#else
#define SET_SLI_READ(Request)
#endif

/*----------------------------------------------------------------------
Function name:  InitCursor

Description:    Called to allocate cursor space on every mode switch
                and to setup the correct cursor routines to use.
Information:

Return:         VOID
----------------------------------------------------------------------*/
extern WORD FirstEnable;
#ifdef SLI_AA
DWORD GetFBAddr(int X, int Y, WORD wBytesPerPixel);
SHORT GetBPP(void);
#endif
void InitCursor(void)
{
    GetBinary(DisplayInfo.diDevNodeHandle, &nDIBCursor, "DIBCursor", 0x0);

#ifdef WIN_CSIM
    lph3IORegs = (SstIORegs *)_FF(regRealBase);
    dwCursorLoc = _FF(cursorStart) + _FF(lfbRealBase);
#else
    dwCursorLoc = _FF(cursorStart) + _FF(lfbBase);
#endif

    if (0x0 != dwCursorLoc)
    {
        SETDW(lph3IORegs->hwCurPatAddr, _FF(cursorStart));
    }

    if ((!_FF(fMouseTrailsOn)) &&
        (0x0 != dwCursorLoc))
    {
        pMoveCursor = BansheeMoveCursor;
        pSetCursor = BansheeSetCursor;
        pEnableCursor = BansheeEnableCursor;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR);
        _FF(lpPDevice)->deFlags |= HWCURSOR;
    }
    else if (_FF(fMouseTrailsOn) || (nDIBCursor))
    {
        pMoveCursor = DIBMoveCursor;
        pSetCursor = DIBSetCursor;
        pEnableCursor = DIBEnableCursor;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_SW_CURSOR);
        _FF(gdiFlags) |= (SDATA_GDIFLAGS_DIB_CURSOR);
        _FF(lpPDevice)->deFlags &= ~HWCURSOR;
    }
   else
    {
      pMoveCursor = SWMoveCursor;
      pSetCursor = SWSetCursor;
      pEnableCursor = SWEnableCursor;
      _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_DIB_CURSOR);
      _FF(gdiFlags) |= SDATA_GDIFLAGS_SW_CURSOR;
      _FF(lpPDevice)->deFlags &= ~HWCURSOR;
    }

    pSaveCursorExclude = SaveCursorExclude;
    pRestoreCursorExclude = RestoreCursorExclude;
    pDrawCursor = DrawCursor;

    //
    // Disable Cursor and Move to Upper Left of Screen
    //
    _FF(LastCursorPosY)=0;
    _FF(LastCursorPosX)=0;
    _FF(HotspotX)=0;
    _FF(HotspotY)=0;

    if (FirstEnable)
      {
      SaveCursor.csColor = 0x0;
      _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_ENABLED);
      }

    _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED);
  
    if( !(SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
    {
        return;
    }
#if 0
    SaveCursorExclude(0,0);
#endif
    pEnableCursor(0x0);
    pMoveCursor(0x0, 0x0);
   
    ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
#ifdef SLI_AA
   _FF(CursorFlipCount) = _FF(FlipCount) = 0L;
   _FF(CursorSurface) = _FF(CurrentSurface) = GetFBAddr(0, 0, GetBPP());   
#endif
}

#undef DIBCURSOR

/*----------------------------------------------------------------------
Function name:  DoMouseTrails

Description:    Handles the drawing of the mouse trails.
                Called from the Windows Control function.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void DoMouseTrails(WORD wTrails)
{
#ifdef DIBCURSOR
    DWORD fOldTrails = _FF(fMouseTrailsOn);

    //
    // Are trails being enabled
    //
    if (wTrails)
    {
        _FF(fMouseTrailsOn) = TRUE;
    }
    else
    {
        _FF(fMouseTrailsOn) = FALSE;
    }
    //
    // Has the state of trails changed
    //
    if (fOldTrails != _FF(fMouseTrailsOn))
    {
        if( !(SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
        {
            return ;
        }
        SetUpCursorFunctions((CURSORSHAPE FAR *)&SaveCursor);
        ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
    }
#else
   return ;
#endif
}

/*----------------------------------------------------------------------
Function name:  SetCursor

Description:    Sets the shape of the cursor.  Determines whether to
                use the HW or SW cursor functions.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure due to BUSY condition
----------------------------------------------------------------------*/
BOOL FAR PASCAL _loadds SetCursor(CURSORSHAPE FAR *lpCursor)
{
   SHORT x;
   SHORT y;


   if (_FF(lpPDevice)->deFlags & BUSY)
      return(FALSE);
     
   if( !(SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
   {
       return FALSE;
   }

   pEnableCursor(0x0);
   _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_ENABLED);
   if (lpCursor == NULL)
   {
       ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
       return TRUE;
   }

   DPF(DBGLVL_CURSOR, " Enter SetCursor");


   // cde 030398
   // The routine referenced by pMoveCursor doesn't care about the
   // hotspot, it expects an absolute x,y location to move the cursor
   // to; the MoveCursor routine just below normally takes care of
   // calculating the correct x,y based on the given x,y and the hotspot;
   // However, with this new cursor, the hotspot and cursor could change
   // on the screen before a MoveCursor is called; this causes the new
   // cursor to dance around when it changes; So, we'll first subtract
   // out the old hotspot from the current location, the calculate
   // a new correct offset based on the new hotspot

   _FF(HotspotX) = (UINT)lpCursor->xHotSpot;
   _FF(HotspotY) = (UINT)lpCursor->yHotSpot;

   // Do Save Under after hotspot updated
   if (SetUpCursorFunctions(lpCursor) == -1)
   {
      // If bogus or corrupted data found in the POINTERINFO
      // structure, then just return.  This is to work around
      // a WHQL app bug in WHQL PC98, GDI - Windows, AMovie test.
      // See PRS #s 1784 & 2337.
      ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
      return (TRUE);
   }

   _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_ENABLED;
   if( !( _FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR) )
   {
       pMoveCursor(_FF(CursorPosX),  _FF(CursorPosY));
       pSetCursor((CURSORSHAPE FAR *)&SaveCursor);
   }
   else
   {

       pSetCursor((CURSORSHAPE FAR *)&SaveCursor);
//       pMoveCursor(_FF(CursorPosX),  _FF(CursorPosY));
       
       //
       // Duplicate Cursor Move Code to avoid playing with flags
       //  
       
       pRestoreCursorExclude((int)_FF(LastCursorPosX), (int)_FF(LastCursorPosY));
       // Get Cursor Position so SWMove will not cause us to lose one
       // This may cause us to lose a update but should prevent losing a cursor
       x = _FF(CursorPosX) -  _FF(HotspotX);  
       y = _FF(CursorPosY) -  _FF(HotspotY);  
       pSaveCursorExclude(x, y);
       pDrawCursor(x, y); 
   }
   DPF(DBGLVL_CURSOR, " Exit SetCursor");

   ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 

   return TRUE;
}

/*----------------------------------------------------------------------
Function name:  MoveCursor

Description:    Moves the screen location of the cursor.  Used for both
                HW and SW cursor.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure
----------------------------------------------------------------------*/
BOOL FAR PASCAL _loadds MoveCursor(SHORT x, SHORT y)
{
   if (!(_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR))
      {
      if (bScanlineDouble & CURSOR_DBL_X)
           x <<= 1;     // fixup cursor positions for special modes

      if (bScanlineDouble & CURSOR_DBL_Y)
           y <<= 1;
      }

   _FF(CursorPosX) = x;
   _FF(CursorPosY) = y;

   return (pMoveCursor)(x, y);

}

/*----------------------------------------------------------------------
Function name:  SetUpCursorFunctions

Description:    Determines whether to use HW or SW cursor functions.

Information:

Return:         int      1 = Success
                        -1 = Failure
----------------------------------------------------------------------*/
int From4BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwSreenFormat);
int From8BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwSreenFormat);
int To8BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwCursorFormat);
int SetUpCursorFunctions(CURSORSHAPE FAR * lpCursor)
{
   DWORD dwFormat;
   DWORD transition=0;
   LPPOINTERINFO lpPointer = (LPPOINTERINFO)lpCursor;


   //
   // Save Cursor
   //
   switch (lpPointer->csColor)
   {
      case 0x101:
         *(PPOINTERINFO1)&SaveCursor = *(LPPOINTERINFO1)lpPointer;
         break;
   
      case 0x401:
         *(PPOINTERINFO4)&SaveCursor = *(LPPOINTERINFO4)lpPointer;
         break;
 
      case 0x801:
         *(PPOINTERINFO8)&SaveCursor = *(LPPOINTERINFO8)lpPointer;
         break;

      case 0x1001:
         *(PPOINTERINFO16)&SaveCursor = *(LPPOINTERINFO16)lpPointer;
         break;

      case 0x1801:
         *(PPOINTERINFO24)&SaveCursor = *(LPPOINTERINFO24)lpPointer;
         break;

      case 0x2001:
         SaveCursor = *lpPointer;
         break;

      // Save Cursor Incase we have to restore it
      default:
         // If bogus or corrupted data found in the POINTERINFO
         // structure, then return error.  It is just a guess that
         // hot spot will never be greater than 0x41.  This is to
         // work around a WHQL app bug in WHQL PC98, GDI - Windows,
         // AMovie test.  See PRS #s 1784 & 2337.
         if ((UINT)lpCursor->xHotSpot > 0x41)
            return -1;

         _fmemcpy(&SaveCursor, lpPointer, 
            sizeof(POINTERINFO) - sizeof(lpPointer->csXORBits) +
               (lpPointer->csColor >> 8) * lpPointer->csWidthBytes * 32);
         break;
    }

    transition = (WORD) ( _FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR ) ;

    transition >>= 1;

   if (_FF(fMouseTrailsOn) || (_FF(HotspotX) < 0) || (_FF(HotspotY) < 0) || (nDIBCursor) ||
       ((0x0101 != lpPointer->csColor) && (0x0401 != lpPointer->csColor) && (0x0801 != lpPointer->csColor) && 
        (0x1001 != lpPointer->csColor) && (0x1801 != lpPointer->csColor) && (0x2001 != lpPointer->csColor)))
      {
      pMoveCursor = DIBMoveCursor;
      pSetCursor = DIBSetCursor;
      pEnableCursor = DIBEnableCursor;
      _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_SW_CURSOR);
      _FF(gdiFlags) |= (SDATA_GDIFLAGS_DIB_CURSOR);
      _FF(lpPDevice)->deFlags &= ~HWCURSOR;
      }
   else
      {
      pMoveCursor = SWMoveCursor;
      pSetCursor = SWSetCursor;
      pEnableCursor = SWEnableCursor;
      _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_DIB_CURSOR);
      _FF(gdiFlags) |= SDATA_GDIFLAGS_SW_CURSOR;
      _FF(lpPDevice)->deFlags &= ~HWCURSOR;
      }

   /*
      If MouseTrails are not on and
      we have space in the FB for a cursor and
      the cursor is a mono-cursor and the mode is not 2048x????
      then we can handle it
                                                         */
    if ((!_FF(fMouseTrailsOn)) &&
        (0x0 != dwCursorLoc) &&
        (0x0101 == lpPointer->csColor) &&
        (!((2048L == ModeList[_FF(ModeNumber)].dwWidth) && IS_VOODOO3)))
    {
         pMoveCursor = BansheeMoveCursor;
         pSetCursor = BansheeSetCursor;
         pEnableCursor = BansheeEnableCursor;
         _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR);
         _FF(lpPDevice)->deFlags |= HWCURSOR;
    }
 
#ifdef SLI_AA
   // All slaves should be ignored???
   if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SLI_AA_MASTER))
      {
      pMoveCursor = SWMoveCursor;
      pSetCursor = HostSetCursor;
      pEnableCursor = HostEnableCursor;
      pSaveCursorExclude = HostSaveCursorExclude;
      pRestoreCursorExclude = HostRestoreCursorExclude;
      pDrawCursor = HostDrawCursor;
      _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_DIB_CURSOR);
      _FF(gdiFlags) |= SDATA_GDIFLAGS_SW_CURSOR;
      _FF(lpPDevice)->deFlags &= ~HWCURSOR;
      }
   else
      {
      pSaveCursorExclude = SaveCursorExclude;
      pRestoreCursorExclude = RestoreCursorExclude;
      pDrawCursor = DrawCursor;
      }
#endif


    transition  |=(WORD)( ( _FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR ) >> 2);

   /* 
        11 swcursor to swcuror
        10 swcursor to dibcursor/hwcursor
        01 dibcursor/hwcursor to swcursor
        00 dibcursor/hwcursor to dibcursor/hwcursor
   */

    switch (transition)
    {
        case 0x01:
        case 0x03:
        // save cursor exclusion
        DPF(DBGLVL_CURSOR, " sw hw cursorposx %d,  cursorposX %d",
                _FF(CursorPosX), _FF(CursorPosY));
        pSaveCursorExclude( _FF(CursorPosX) - _FF(HotspotX), _FF(CursorPosY) - _FF(HotspotY));
        break;

        case 0x02:
        // erase cursor
        DPF(DBGLVL_CURSOR, " sw hw cursorposx %d,  cursorposX %d",
                _FF(CursorPosX), _FF(CursorPosY) );
        pRestoreCursorExclude( _FF(LastCursorPosX), _FF(LastCursorPosY));
        break;

        default:
        break;
   }

   dwFormat = GetCursorFormat((CURSORSHAPE FAR *)lpPointer) & SSTG_SRC_FORMAT;
   if ((_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR) &&
        (dwFormat != (_FF(screenFormat) & SSTG_SRC_FORMAT)) &&
        (SSTG_PIXFMT_1BPP != dwFormat))
      {
      if (0x0401 == lpPointer->csColor)
         From4BPPCursor(lpCursor, &SaveCursor, _FF(screenFormat) & SSTG_SRC_FORMAT);
      else if (SSTG_PIXFMT_8BPP == dwFormat)
         From8BPPCursor(lpCursor, &SaveCursor, _FF(screenFormat) & SSTG_SRC_FORMAT);
      else if (SSTG_PIXFMT_8BPP == (_FF(screenFormat) & SSTG_SRC_FORMAT))
         To8BPPCursor(lpCursor, &SaveCursor, dwFormat);
      }

    return 1;
}

/*----------------------------------------------------------------------
Function name:  From4BPPCursor

Description:    Expands the 4 BPP mask via a colortable and converts it to
                8/16/24/32 bpp from 4 bpp.

Information:

Return:         int      0 = Always returned
----------------------------------------------------------------------*/
DWORD From4BPPto8[] = {
   0x00000000, 0x00000001, 0x00000002, 0x00000003,
   0x00000004, 0x00000005, 0x00000006, 0x00000007,
   0x000000F8, 0x000000F9, 0x000000FA, 0x000000FB,
   0x000000FC, 0x000000FD, 0x000000FE, 0x000000FF,
   };

DWORD From4BPPto16[] = {
   0x00000000, 0x00007800, 0x000003E0, 0x00007BE0,
   0x0000000F, 0x0000780F, 0x000003EF, 0x0000BDF7,
   0x00007BEF, 0x0000F800, 0x000007E0, 0x0000FFE0,
   0x0000001F, 0x0000F81F, 0x000007FF, 0x0000FFFF,
   };

DWORD From4BPPto2432[] = {
   0x00000000, 0x00800000, 0x00008000, 0x00808000,
   0x00000080, 0x00800080, 0x00008080, 0x00C0C0C0,
   0x00808080, 0x00FF0000, 0x0000FF00, 0x00FFFF00,
   0x000000FF, 0x00FF00FF, 0x0000FFFF, 0x00FFFFFF,
   };

int From4BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwScreenFormat)
{
   LPPOINTERINFO4 lpPointer;
   DWORD * pTable;
   BYTE *pByte;
   BYTE bData;
   int i;
   int j;
   int nSize;
   int nIndex;

   // What is screen BPP
   switch (dwScreenFormat)
      {
      case SSTG_PIXFMT_8BPP:
         lpSaveCursor->csColor = 0x0801;
         nSize = 1;
         pTable = From4BPPto8;
         break;

      case SSTG_PIXFMT_16BPP:
         lpSaveCursor->csColor = 0x1001;
         nSize = 2;
         pTable = From4BPPto16;
         break;

      case SSTG_PIXFMT_24BPP:
         lpSaveCursor->csColor = 0x1801;
         nSize = 3;
         pTable = From4BPPto2432;
         break;

      case SSTG_PIXFMT_32BPP:
         lpSaveCursor->csColor = 0x2001;
         nSize = 4;
         pTable = From4BPPto2432;
         break;
   
   default:
         lpSaveCursor->csColor = 0x2001;
         nSize = 4;
         break;
      }

   lpPointer = (LPPOINTERINFO4)lpCursor;
   pByte = (BYTE *)&lpSaveCursor->csXORBits;
   for (i=0; i<512; i++)
      {
      bData = lpPointer->csXORBits[i];
      for (j=0; j<2; j++)
         {
         nIndex = bData;      
         bData <<=4;
         nIndex &= 0xF0;
         nIndex >>= 4;
         *((DWORD *)pByte) = pTable[nIndex];
         pByte+=nSize;
         }
      }         

   return 0;
}

/*----------------------------------------------------------------------
Function name:  From8BPPCursor

Description:    Expands the old mask via the HWColorTable and converts it to
                16/24/32 bpp from 8 bpp.

Information:

Return:         int      0 = Always returned
----------------------------------------------------------------------*/
extern DWORD HWColorTable[256];
int From8BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwScreenFormat)
{
   DWORD dwColor;
   WORD wColor;
   LPPOINTERINFO8 lpPointer;
   BYTE * pByte;
   BYTE * pColor;
   int nSize;
   int i;
   int j;

   // What is screen BPP
   if (SSTG_PIXFMT_16BPP == dwScreenFormat)
      {
      lpSaveCursor->csColor = 0x1001;
      nSize = 2;
      }
   else if (SSTG_PIXFMT_24BPP == dwScreenFormat)
      {
      lpSaveCursor->csColor = 0x1801;
      nSize = 3;
      }
   else if (SSTG_PIXFMT_32BPP == dwScreenFormat)
      {
      lpSaveCursor->csColor = 0x2001;
      nSize = 4;
      }
  
   pByte = (BYTE *)&lpSaveCursor->csXORBits;
   lpPointer = (LPPOINTERINFO8)lpCursor;
   
   // Expand the XOR mask to screen size
   for (i=0; i<1024; i++)
      {
      dwColor = HWColorTable[lpPointer->csXORBits[i]];
      if (2 == nSize)
         {
         wColor = (WORD)(((dwColor & 0x00F80000) >> 8) |
            ((dwColor & 0x0000FC00) >> 5) |
            ((dwColor & 0x000000F8) >> 3));
         pColor = (BYTE *)&wColor;
         }
      else
         pColor = (BYTE *)&dwColor;

      for (j=0; j<nSize; j++)
         *pByte++ = *pColor++;
      }
   return 0;
}


/*----------------------------------------------------------------------
Function name:  To8BPPCursor

Description:    Point samples the old mask and converts it from 
                16/24/32 bpp from 8 bpp.

Information:

Return:         int      0 = Always returned
----------------------------------------------------------------------*/
int To8BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwCursorFormat)
{
   LPPOINTERINFO8 lpPointer;
   BYTE * pByte;
   int nSize;
   int i;

   // What is cursor BPP
   if (SSTG_PIXFMT_16BPP == dwCursorFormat)
      {
      nSize = 2;
      }
   else if (SSTG_PIXFMT_24BPP == dwCursorFormat)
      {
      nSize = 3;
      }
   else if (SSTG_PIXFMT_32BPP == dwCursorFormat)
      {
      nSize = 4;
      }
  
   lpSaveCursor->csColor = 0x0801;
   lpPointer = (LPPOINTERINFO8)lpCursor;
   pByte = (BYTE * )&lpPointer->csXORBits;
   lpPointer = (LPPOINTERINFO8)lpSaveCursor;
   
   // Contract the XOR mask to screen size
   for (i=0; i<1024; i++)
      {
      lpPointer->csXORBits[i] = *pByte;
      pByte += nSize;
      }

   return 0;
}

/*----------------------------------------------------------------------
Function name:  CheckCursor

Description:    CheckCursor is called on each timer interrupt. The function
                should determine whether the cursor needs redrawing and
                whether drawing is enabled.  If so, the function should
                redraw the cursor.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds CheckCursor(void)
{
    // If we are busy then return
    if (_FF(lpPDevice)->deFlags & BUSY)
    {
        return;
    }

   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return;


    // If we are hardware cursor then return
    if( _FF(gdiFlags) & SDATA_GDIFLAGS_DIB_CURSOR)
      {
      DIB_CheckCursorExt(_FF(lpPDevice));
      }

   return;
}

/*----------------------------------------------------------------------
Function name:  DIBMoveCursor

Description:    Calls the DIB engine MoveCursor function.

Information:

Return:         BOOL    FALSE if BUSY or EXCLUSIVE mode or,
                        result of DIB MoveCursor call.
----------------------------------------------------------------------*/
BOOL DIBMoveCursor(SHORT x, SHORT y)
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return (FALSE);

   if (_FF(lpPDevice)->deFlags & BUSY)
      return (FALSE);

   return DIB_MoveCursorExt(x, y, _FF(lpPDevice));
}

/*----------------------------------------------------------------------
Function name:  DIBMoveHelp

Description:    Calls the DIB engine MoveCursorExt function.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void DIBMoveHelp()
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return;

   if (_FF(lpPDevice)->deFlags & BUSY)
      return;

   DIB_MoveCursorExt(_FF(CursorPosX), _FF(CursorPosY), _FF(lpPDevice));
}

/*----------------------------------------------------------------------
Function name:  DIBSetCursor

Description:    Calls the DIB engine SetCursor function.

Information:

Return:         BOOL    FALSE if BUSY or EXCLUSIVE mode or,
                        result of DIB SetCursor call.
----------------------------------------------------------------------*/
BOOL DIBSetCursor(CURSORSHAPE FAR * lpCursor)
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return (FALSE);

   if (_FF(lpPDevice)->deFlags & BUSY)
      return (FALSE);

   DIB_SetCursorExt(lpCursor, _FF(lpPDevice));
   return(TRUE);
}

/*----------------------------------------------------------------------
Function name:  DIBEnableCursor

Description:    Performs the DIB engine EnableCursor function via a
                call to the DIB SetCursorExt.

Information:

Return:         BOOL    FALSE if BUSY or EXCLUSIVE mode or,
                        result of DIB SetCursor call.
----------------------------------------------------------------------*/
void DIBEnableCursor(int nFlag)
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return;

   if (_FF(lpPDevice)->deFlags & BUSY)
      return ;

   if (nFlag)
      DIB_SetCursorExt((LPVOID)&SaveCursor, _FF(lpPDevice));
   else
      DIB_SetCursorExt(NULL, _FF(lpPDevice));
}

/***************************************************************************
 *
 *   Here comes the SW cursor
 *
 ***************************************************************************/
/*----------------------------------------------------------------------
Function name:  SWMoveCursor

Description:    Move the screen location of the cursor via SW.

Information:

Return:         BOOL    FALSE = Failure (multiple conditions)
                        TRUE  = Success 
----------------------------------------------------------------------*/
BOOL SWMoveCursor(SHORT x, SHORT y)
{
    DPF(DBGLVL_CURSOR, " Enter SW MoveCursor");

    if (_FF(lpPDevice)->deFlags & BUSY)
       {
       (_FF(cursorMissed))++;
       return(FALSE);
       }

    if ((_FF(gdiFlags) & SDATA_GDIFLAGS_CURSOR_EXCLUDE) ||
        (!(_FF(gdiFlags) & SDATA_GDIFLAGS_CURSOR_ENABLED)))
      {
      (_FF(cursorMissed))++;
      return(FALSE);
      }

    if (_FF(gdiFlags) & SKIP_FLAGS)
   	return TRUE;

   if(!(SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
   {
       return FALSE;
   }

    pRestoreCursorExclude((int)_FF(LastCursorPosX), (int)_FF(LastCursorPosY));
    x = x - _FF(HotspotX);
    y = y - _FF(HotspotY);
    pSaveCursorExclude(x, y);
    pDrawCursor(x, y); 

   DPF(DBGLVL_CURSOR, " Exit MoveCursor");
   ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 

    return (TRUE);
}
 
/*----------------------------------------------------------------------
Function name:  GetCursorFormat

Description:    Returns dword with stride and pix depth info for XOR
                mask ONLY. The and mask stride and pix depth is same
                across pix depth.  Returned dword is used for
                srcFormat/dstFormat register.  The cursor is 32 by 32
                bytes.
Information:

Return:         DWORD   Used to set srcFormat/DstFormat HW registers
----------------------------------------------------------------------*/
DWORD GetCursorFormat (CURSORSHAPE FAR *lpCursor)
{
DWORD retval;
LPPOINTERINFO lpPointer = (LPPOINTERINFO)lpCursor;
   
   
    switch (lpPointer->csColor)
    {
        case 0x101:
        // stride for mono xor mask is  4 bytes
        retval = CURSOR_BMP_STRIDE | SSTG_PIXFMT_1BPP;
        break;

        case 0x801:
        // stride for 8bpp xor mask is  32 bytes
        retval = CURSOR_BMP_STRIDE | SSTG_PIXFMT_8BPP;
        break;
 
        case 0x1001:
        // stride for 16bpp xor mask is  64 bytes
        retval = CURSOR_BMP_STRIDE | SSTG_PIXFMT_16BPP;
        break;
         
        case 0x1801:
        // stride for 24bpp xor mask is  96 bytes
        retval = CURSOR_BMP_STRIDE | SSTG_PIXFMT_24BPP;
        break;

        case 0x2001:
        // stride for 8bpp xor mask is  128 bytes
        retval = CURSOR_BMP_STRIDE | SSTG_PIXFMT_32BPP;
        break;

        default:
        // this is for 4bpp
        // Put in a Bogus Format which we will fix up latter
        retval = CURSOR_BMP_STRIDE | (0x0FUL << SSTG_SRC_FORMAT_SHIFT);
        break;
    }
 
    return retval;
}
   
/*----------------------------------------------------------------------
Function name:  SWSetCursor

Description:    The software set cursor function.  Also, determines if
                it's a HW or SW cursor.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure
----------------------------------------------------------------------*/
BOOL SWSetCursor(CURSORSHAPE FAR * lpCursor)
{
   DWORD srcFormat, dstFormat;
   DWORD * lpXorMask;
   DWORD * lpAndMask;
   WORD  bytesPerPix;
   DWORD  dwordsNeeded;
   unsigned int i;
   LPPOINTERINFO lpPointer = (LPPOINTERINFO)lpCursor;
   CMDFIFO_PROLOG(cmdFifo);

    if (_FF(lpPDevice)->deFlags & BUSY)
    {
        return(FALSE);
    }
 
    if (_FF(gdiFlags) & SKIP_FLAGS)
      {
      _FF(SWcursorFormat)=GetCursorFormat(lpCursor);
   	return TRUE;
      }

    _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY; 

    CMDFIFO_SETUP(cmdFifo);

    lpAndMask = (DWORD *) &(lpPointer->csANDBits); 
    lpXorMask = (DWORD *) &(lpPointer->csXORBits); 
    
    
    // cursor storage area is 32X32X4bytes.
 
    if (0x0101 == ((LPPOINTERINFO)lpCursor)->csColor)
      dstFormat=(_FF(screenFormat) & SSTG_SRC_FORMAT) | CURSOR_BMP_STRIDE;
    else
      dstFormat=GetCursorFormat(lpCursor);

    _FF(SWcursorFormat) = dstFormat;
 
 
    // host blt mono "and" mask to color cursor 
    // bitmap location
    // using 0xFFFFFF as chroma color 
    CMDFIFO_CHECKROOM(cmdFifo,13 );
    SETPH(cmdFifo, SSTCP_PKT2|
          clip0minBit|
          clip0maxBit|
          dstBaseAddrBit     |
          dstFormatBit       |
          commandExBit|
          srcFormatBit       |
          srcXYBit           |
          colorBackBit       |
          colorForeBit       |
          dstXYBit           |
          dstSizeBit         |
          commandBit );
	 SET(cmdFifo, lph3g->clip0min, 0);
	 SET(cmdFifo, lph3g->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
    SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorAndStart));
    SET(cmdFifo, lph3g->dstFormat, dstFormat);
    SET(cmdFifo, lph3g->commandEx, 0x0);
    SET(cmdFifo, lph3g->srcFormat, AND_MASK_STRIDE | SSTG_SRC_PACK_SRC | SSTG_PIXFMT_1BPP);
    SET(cmdFifo, lph3g->srcXY , 0);
    SET(cmdFifo, lph3g->colorBack, 0x00000000L)
    SET(cmdFifo, lph3g->colorFore, 0x00FFFFFF)
    SET(cmdFifo, lph3g->dstSize , (32UL << 16)| 32UL );
    SET(cmdFifo, lph3g->dstXY , 0UL );
    SETC(cmdFifo, lph3g->command,
        (SSTG_ROP_SRC << SSTG_ROP0_SHIFT)|
        SSTG_HOST_BLT );
    BUMP(13);
 
    // and mask is mono 
    // 32pix X 32pix /8PixelperByte / 4bytesPerDwords = 32
    CMDFIFO_CHECKROOM(cmdFifo, 33);
    SETPH(cmdFifo, SSTCP_PKT1| 
          SSTCP_PKT1_2D|
          LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
          32UL  <<SSTCP_PKT1_NWORDS_SHIFT); 
    for( i=0; i< 32; i++)
    {
        SET(cmdFifo, lph3g->launch[0], lpAndMask[i]);
    }
    BUMP(33);

   // stride and bpp for xormask
   srcFormat=GetCursorFormat(lpCursor);
    
   // If mask is monochrome as is the case with 2048x1536
   if (SSTG_PIXFMT_1BPP == (srcFormat & SSTG_SRC_FORMAT))
      {
      // host blt mono "xor" mask to color cursor 
      // bitmap location
      // using 0xFFFFFF as chroma color 
      CMDFIFO_CHECKROOM(cmdFifo, 7);
      SETPH(cmdFifo, SSTCP_PKT2|
            dstBaseAddrBit     |
            srcFormatBit       |
            srcXYBit           |
            //colorBackBit       |
            //colorForeBit       |
            dstXYBit           |
            dstSizeBit         |
            commandBit );
      SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorXorStart));
      SET(cmdFifo, lph3g->srcFormat, AND_MASK_STRIDE | SSTG_SRC_PACK_SRC | SSTG_PIXFMT_1BPP);
      SET(cmdFifo, lph3g->srcXY , 0);
      //SET(cmdFifo, lph3g->colorBack, 0x00000000L)
      //SET(cmdFifo, lph3g->colorFore, 0x00FFFFFF)
      SET(cmdFifo, lph3g->dstSize , (32UL << 16)| 32UL );
      SET(cmdFifo, lph3g->dstXY , 0UL );
      SETC(cmdFifo, lph3g->command,
         (SSTG_ROP_SRC << SSTG_ROP0_SHIFT)|
         SSTG_HOST_BLT );
      BUMP(7);
 
      // xor mask is mono 
      // 32pix X 32pix /8PixelperByte / 4bytesPerDwords = 32
      CMDFIFO_CHECKROOM(cmdFifo, 33);
      SETPH(cmdFifo, SSTCP_PKT1| 
            SSTCP_PKT1_2D|
            LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
            32UL  <<SSTCP_PKT1_NWORDS_SHIFT); 
      for( i=0; i< 32; i++)
         {
         SET(cmdFifo, lph3g->launch[0], lpXorMask[i]);
         }
      BUMP(33);
      }
   else
      {
      // dst chroma blt color xormask to SWcursorbitmap 
      // ie rop1(src fail, dst pass)
      // fail write pix, pass ignore pix
      CMDFIFO_CHECKROOM(cmdFifo, 7);
      SETPH(cmdFifo, SSTCP_PKT2|
            dstBaseAddrBit     |
            srcFormatBit       |
            srcXYBit           |
            dstXYBit           |
            dstSizeBit         |
            commandBit
            );

      SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorXorStart));
      SET(cmdFifo,  lph3g->srcFormat,srcFormat);
      SET(cmdFifo,  lph3g->srcXY, 0L);
      SET(cmdFifo,  lph3g->dstSize , (32UL << 16)| 32UL );
      SET(cmdFifo,  lph3g->dstXY , 0UL);
      SETC(cmdFifo,  lph3g->command,
          SSTG_HOST_BLT|
          0xCC000000);
      BUMP (7);
        
 
      bytesPerPix = ((lpPointer->csColor) >> 8)  >> 3 ;
      dwordsNeeded = ( 32 * 32 * bytesPerPix) >> 2 ;
      CMDFIFO_CHECKROOM(cmdFifo, dwordsNeeded + 1);
      SETPH(cmdFifo, SSTCP_PKT1| 
            SSTCP_PKT1_2D|
            LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
            dwordsNeeded  <<SSTCP_PKT1_NWORDS_SHIFT); 

      for( i=0; i< dwordsNeeded; i++)
         {
         SET(cmdFifo, lph3g->launch[0], lpXorMask[i]);
         }
      BUMP(dwordsNeeded +1);
      }

    CMDFIFO_EPILOG(cmdFifo);
    return(TRUE);
}

/*----------------------------------------------------------------------
Function name:  SWEnableCursor

Description:    The software EnableCursor function

Information:

Return:         VOID
----------------------------------------------------------------------*/
void SWEnableCursor(int nFlag)
{
    if (_FF(lpPDevice)->deFlags & BUSY)
       {
       return ;
       }

   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return;

    // If cursor is already enabled and sw then erase it...
    if ((SDATA_GDIFLAGS_CURSOR_ENABLED == (SDATA_GDIFLAGS_CURSOR_ENABLED & _FF(gdiFlags))) &&
        (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR))
      RestoreCursorExclude((int)_FF(LastCursorPosX), (int)_FF(LastCursorPosY));
}

/*----------------------------------------------------------------------
Function name:  SaveCursorExclude

Description:    Saves the area of the display under the cursor prior to
                drawing the cursor.

Information:

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int SaveCursorExclude(int x, int y)
{ 
    DWORD dstFormat;
    SHORT Xsize;
    SHORT Ysize;
    SHORT x1, y1;
    CMDFIFO_PROLOG(cmdFifo);
 
    if (_FF(gdiFlags) & (SKIP_FLAGS | SDATA_GDIFLAGS_CURSOR_EXCLUDE))
   	return TRUE;
   
    x1 = x;
    y1 = y;
   
    Xsize = SWCURSOR_WIDTH;
    if (x < 0)
       {
       Xsize += x;
       x = 0;
       }

   if (x + SWCURSOR_WIDTH > _FF(hres))
      {
      Xsize = _FF(hres) - x;
      }

    Ysize = SWCURSOR_HEIGHT;
    if (y < 0)
       {
       Ysize += y;
       y = 0;
       }

   if (y + SWCURSOR_HEIGHT > _FF(vres))
      {
      Ysize = _FF(vres) - y;
      }

    // Have we been clipped ?
    if ((Xsize > 0) && (Ysize > 0))
      {
      _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY; 

      CMDFIFO_SETUP(cmdFifo);
 
      dstFormat = _FF(screenFormat);
      dstFormat &= ~(SSTG_SRC_PACK);
      dstFormat &= 0xFFFFC000;
      dstFormat |= EXCLUSION_BMP_STRIDE;
        
      // save under cursor 32x32
      CMDFIFO_CHECKROOM(cmdFifo, 17);
      SETPH(cmdFifo, SSTCP_PKT2|
             clip0minBit|
             clip0maxBit|
             dstBaseAddrBit|
             dstFormatBit|
             srcBaseAddrBit|
             commandExBit|
             srcFormatBit|
             srcXYBit|
             dstSizeBit|
             dstXYBit|
             commandBit);
        
   	SET(cmdFifo, lph3g->clip0min, 0);
   	SET(cmdFifo, lph3g->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
      SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorExclusionStart));
      SET(cmdFifo, lph3g->dstFormat, dstFormat);
      SET(cmdFifo, lph3g->srcBaseAddr, _FF(gdiDesktopStart));
      SET(cmdFifo, lph3g->commandEx, 0x0);
      SET(cmdFifo, lph3g->srcFormat, _FF(screenFormat));


      SET(cmdFifo, lph3g->srcXY, ((DWORD) y << 16) | (DWORD) x );
      SET(cmdFifo, lph3g->dstSize, ((DWORD) Ysize << 16) | (DWORD)Xsize );
      SET(cmdFifo, lph3g->dstXY,  0UL);
      SETC(cmdFifo, lph3g->command,
           SSTG_GO |
           0xCC000000 |
           SSTG_BLT);
        
      SETPH(cmdFifo,   SSTCP_PKT2 | dstBaseAddrBit | dstFormatBit);
      SET(cmdFifo,lph3g->dstBaseAddr, _FF(gdiDesktopStart)); 
      SET(cmdFifo,lph3g->dstFormat, _FF(screenFormat));

      WAXBUG_3DNOPFIX;
      BUMP(17);
      CMDFIFO_EPILOG(cmdFifo);
      }  

    _FF(LastCursorPosX)= x1;
    _FF(LastCursorPosY)= y1;
    return(TRUE);
 
}

 
/*----------------------------------------------------------------------
Function name:  RestoreCursorExclude

Description:    Restore the area of the display where the cursor was
                drawn.

Information:

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int RestoreCursorExclude(int x, int y)
{
    DWORD srcFormat;
    SHORT Xsize;
    SHORT Ysize;
    CMDFIFO_PROLOG(cmdFifo);
    
    if (_FF(gdiFlags) & (SKIP_FLAGS | SDATA_GDIFLAGS_CURSOR_EXCLUDE))
   	return TRUE;

    Xsize = SWCURSOR_WIDTH;
    if (x < 0)
      {
      Xsize += x;
      x = 0;
      }

    if (x + SWCURSOR_WIDTH > _FF(hres))
      {
      Xsize = _FF(hres) - x;
      }

    Ysize = SWCURSOR_HEIGHT;
    if (y < 0)
      {
      Ysize += y;
      y = 0;
      }

   if (y + SWCURSOR_HEIGHT > _FF(vres))
      {
      Ysize = _FF(vres) - y;
      }

    //Have we been clipped?
    if ((Xsize > 0) && (Ysize > 0))
      { 
      CMDFIFO_SETUP(cmdFifo);

      CMDFIFO_CHECKROOM(cmdFifo, 17);

      SETPH(cmdFifo, SSTCP_PKT2|
             clip0minBit|
             clip0maxBit|
             dstBaseAddrBit|
             dstFormatBit|
             srcBaseAddrBit|
             commandExBit|
             srcFormatBit|
             srcXYBit|
             dstSizeBit|
             dstXYBit|
             commandBit);
 
      SET(cmdFifo, lph3g->clip0min, 0);
      SET(cmdFifo, lph3g->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
      SET(cmdFifo, lph3g->dstBaseAddr,_FF(gdiDesktopStart)); 
      SET(cmdFifo, lph3g->dstFormat, _FF(screenFormat));
      SET(cmdFifo, lph3g->srcBaseAddr, _FF(SWcursorExclusionStart));
      SET(cmdFifo, lph3g->commandEx, 0x0);
      srcFormat = _FF(screenFormat);
      srcFormat &= ~(SSTG_SRC_PACK);
      srcFormat &= 0xFFFFC000;
      srcFormat |= EXCLUSION_BMP_STRIDE;
      SET(cmdFifo, lph3g->srcFormat, srcFormat);
      SET(cmdFifo, lph3g->srcXY, 0UL);

      SET(cmdFifo, lph3g->dstSize, ((DWORD)Ysize << 16) | (DWORD)Xsize);
      SET(cmdFifo, lph3g->dstXY, (((DWORD) y <<16) | (DWORD) x ) );
      SETC(cmdFifo, lph3g->command,
            SSTG_GO |
            0xCC000000 |
            SSTG_BLT);
 
      // restore src Base and src Format
       
      SETPH(cmdFifo,   SSTCP_PKT2 | srcBaseAddrBit | srcFormatBit);
      SET(cmdFifo,lph3g->srcBaseAddr, _FF(gdiDesktopStart)); 
      SET(cmdFifo,lph3g->srcFormat, _FF(screenFormat));

      WAXBUG_3DNOPFIX;
      BUMP(17);
      CMDFIFO_EPILOG(cmdFifo);
      }

    return(TRUE);
}

/*----------------------------------------------------------------------
Function name:  DrawCursor

Description:    Draws the cursor by blitting the bitmap in the
                SwCursorbitmapStart.
Information:

Return:         int     TRUE = early return due to EXCLUSIVE mode
                        0    = Successfully drawn
----------------------------------------------------------------------*/
int DrawCursor(int X, int Y)
{
   DWORD dstFormat;
   SHORT xSize;
   SHORT ySize;
   SHORT xCur;
   SHORT yCur;

   CMDFIFO_PROLOG(cmdFifo);

   if (_FF(gdiFlags) & SKIP_FLAGS)
   	return TRUE;
   
   if ((SDATA_GDIFLAGS_CURSOR_ENABLED == (SDATA_GDIFLAGS_CURSOR_ENABLED & _FF(gdiFlags))) &&
       (SDATA_GDIFLAGS_CURSOR_EXCLUDE != (SDATA_GDIFLAGS_CURSOR_EXCLUDE & _FF(gdiFlags))))
      {

      // if X is less then zero then fix it up
      xSize = SWCURSOR_WIDTH;
      xCur = 0;
      if (X < 0)
         {
         xSize = xSize + X;
         xCur = (SHORT)SWCURSOR_WIDTH - xSize;
         X = 0;  
         }

      if (X + SWCURSOR_WIDTH > _FF(hres))
         {
         xSize = _FF(hres) - X;
         }

      // Ditto with Y
      ySize = SWCURSOR_HEIGHT;
      yCur = 0;
      if (Y < 0)
         {
         ySize = ySize + Y;
         yCur = (SHORT)SWCURSOR_HEIGHT - ySize;
         Y = 0;  
         }

      if (Y + SWCURSOR_HEIGHT > _FF(vres))
         {
         ySize = _FF(vres) - Y;
         }

      // if the cursor is clipped no need to draw it
      if ((xSize < 1) || (ySize < 1))
         return 0;

      CMDFIFO_SETUP(cmdFifo);
 
      dstFormat = _FF(screenFormat);
      dstFormat &= ~(SSTG_SRC_PACK);
      dstFormat &= 0xFFFFC000;
      dstFormat |= EXCLUSION_BMP_STRIDE;
        
      // Get Screen
      CMDFIFO_CHECKROOM(cmdFifo, 34);
      SETPH(cmdFifo, SSTCP_PKT2|
             clip0minBit|
             clip0maxBit|
             dstBaseAddrBit|
             dstFormatBit|
             srcBaseAddrBit|
             commandExBit|
             srcFormatBit|
             srcXYBit|
             dstSizeBit|
             dstXYBit|
             commandBit);
        
   	SET(cmdFifo, lph3g->clip0min, 0);
   	SET(cmdFifo, lph3g->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
      SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorSrcStart));
      SET(cmdFifo, lph3g->dstFormat, dstFormat);
      SET(cmdFifo, lph3g->srcBaseAddr, _FF(gdiDesktopStart));
      SET(cmdFifo, lph3g->commandEx, 0x0);
      SET(cmdFifo, lph3g->srcFormat, _FF(screenFormat));

      SET(cmdFifo, lph3g->srcXY, ((DWORD) Y << 16) | (DWORD) X );
      SET(cmdFifo, lph3g->dstSize, ((DWORD) ySize << 16) | (DWORD)xSize );
      SET(cmdFifo, lph3g->dstXY,((DWORD) yCur << 16) | (DWORD)xCur);
      SETC(cmdFifo, lph3g->command,
           SSTG_GO |
           0xCC000000 |
           SSTG_BLT);

      SETPH(cmdFifo, SSTCP_PKT2|
            srcBaseAddrBit     |
            srcFormatBit       |
            srcXYBit           |
            dstSizeBit         |
            dstXYBit           |
            commandBit
            );  
        
      SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorAndStart));
      SET(cmdFifo,  _FF(lpGRegs)->srcFormat, _FF(SWcursorFormat));
      SET(cmdFifo,  _FF(lpGRegs)->srcXY, ((DWORD) yCur << 16) | (DWORD)xCur);
      SET(cmdFifo,  _FF(lpGRegs)->dstSize , ((DWORD) ySize << 16) | (DWORD)xSize );
      SET(cmdFifo,  _FF(lpGRegs)->dstXY, ((DWORD) yCur << 16) | (DWORD)xCur);
      SETC(cmdFifo,  _FF(lpGRegs)->command,
           SSTG_GO|
           SSTG_BLT|
           SSTG_ROP_AND << SSTG_ROP0_SHIFT);
        
      SETPH(cmdFifo, SSTCP_PKT2|
            srcBaseAddrBit     |
            commandBit
            );  
        
      SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorXorStart));
      SETC(cmdFifo,  _FF(lpGRegs)->command,
           SSTG_GO|
           SSTG_BLT|
           SSTG_ROP_XOR << SSTG_ROP0_SHIFT);
        
      SETPH(cmdFifo, SSTCP_PKT2|
            dstBaseAddrBit     |
            dstFormatBit       |
            srcBaseAddrBit     |
            srcFormatBit       |
            dstXYBit           |
            commandBit
            );  
        
      SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, _FF(gdiDesktopStart));
      SET(cmdFifo, _FF(lpGRegs)->dstFormat, _FF(screenFormat));
      SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorSrcStart));
      SET(cmdFifo, lph3g->srcFormat, dstFormat);
      SET(cmdFifo,  _FF(lpGRegs)->dstXY,( ((DWORD)(Y & 0x1FFF) <<16) ) | (DWORD)(X & 0x1FFF));
      SETC(cmdFifo,  _FF(lpGRegs)->command,
           SSTG_GO|
           SSTG_BLT|
           0xCC000000);
        
      SETPH(cmdFifo,   SSTCP_PKT2|
              srcBaseAddrBit|
              srcFormatBit);
      SET(cmdFifo,_FF(lpGRegs)->srcBaseAddr, _FF(gdiDesktopStart)); 
      SET(cmdFifo,_FF(lpGRegs)->srcFormat, _FF(screenFormat));
      WAXBUG_3DNOPFIX;
      BUMP(34);
      CMDFIFO_EPILOG(cmdFifo);
   }

   return 0;
}

/*----------------------------------------------------------------------
Function name:  RestoreCursor

Description:    Restores the cursor shape if the cursor is Enabled.

Information:    

Return:         int     00 = Always returned
----------------------------------------------------------------------*/
int RestoreCursor(void)
{
    // Make sure that hwCurPatAddr is valid
    SETDW(lph3IORegs->hwCurPatAddr, _FF(cursorStart));
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_CURSOR_ENABLED)
       {
       SetCursor((CURSORSHAPE FAR *)&SaveCursor);
       }

   return 0;
}

#ifdef SLI_AA
/*----------------------------------------------------------------------
Function name:  SwitchToHostCursor

Description:    Switches the cursor shape if the cursor is Enabled.

Information:    

Return:         int     00 = Always returned
----------------------------------------------------------------------*/
int _loadds SwitchToHostCursor(void)
{
   _FF(lpPDevice)->deFlags &= ~BUSY;
   RestoreCursor();
   _FF(CursorSurface) = _FF(CurrentSurface) = _FF(lpPDevice)->deBitsOffset;
   _FF(lpPDevice)->deFlags |= BUSY;

   return 0x0;
}
#endif

/***************************************************************************
 *
 *   Here comes the HW cursor
 *
 ***************************************************************************/
/*----------------------------------------------------------------------
Function name:  BansheeSetCursor

Description:    The V2/V3 HW dependent SetCursor function

Information:    The cursor must be in linear space and is layed
                out as follows

     |----------------- 128 bits --------------------|
     |------64 AND Bits----|----------64 XOR Bits----|
     |----------|----------|-------------|-----------|
     |          |          |             |           |
     |  AND     |    1's   |   XOR       |  0's      |
     |  Mask    |          |   Mask      |           |
     |          |          |             |           |
     |----------|----------|-------------|-----------|
     |          |          |             |           |
     |          |          |             |           |
     |   1's    |    1's   |    0's      |   0's     |
     |          |          |             |           |
     |          |          |             |           |
     |----------|----------|-------------|-----------|

     This makes the quadrants as follow's

     |----------|----------|
     |          |          |
     |  Mask    | Trans-   |
     |          | parent   |
     |          |          |
     |----------|----------|
     |          |          |
     | Trans-   | Trans-   |
     | parent   | parent   |
     |          |          |
     |----------|----------|

Return:         VOID
----------------------------------------------------------------------*/
BOOL BansheeSetCursor(CURSORSHAPE * lpCursor)
{
    LPPOINTERINFO1 lpPointerInfo1 = (LPPOINTERINFO1)lpCursor;
    DWORD dwFB;
    DWORD FAR * lpANDBits = (DWORD FAR *)&lpPointerInfo1->csANDBits;
#ifdef BIG_CURSOR
    int nBigCursor;
#endif

   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return FALSE;

   //
   // This is probably a unnecessary check
   //
    if (0x0101 != lpPointerInfo1->csColor)
        return FALSE;
 
    // Get the pointer into FB where to put the cursor
    dwFB = dwCursorLoc;
 
    // Put Cursor in Upper Left Corner
#ifdef BIG_CURSOR
    // Stretch the cursor to see which cursor we are using if
    // Bigcursor is set in registry
    GetBinary(DisplayInfo.diDevNodeHandle, &nBigCursor, "BigCursor", 0);
    if (nBigCursor)
       DoBigCursor(dwFB, lpANDBits);
    else
#endif

   if (CURSOR_DBL_Y == bScanlineDouble)
      DoCursorDblX(dwFB, lpANDBits);
   else
      DoCursor(dwFB, lpANDBits);
 
 #ifdef CRASHTEST
    if (doCT)
    {
	FXWAITFORIDLE();
    }
 #endif // #ifdef CRASHTEST
    //
    // Load in the cursor colors
    //
    SETDW(lph3IORegs->hwCurC0, 0x00000000);  // Black for Color 0
    SETDW(lph3IORegs->hwCurC1, 0x00FFFFFF);  // White for Color 1
    //
    //Now that the cursor is ready enable it
    //
    BansheeEnableCursor(0x01);
 
    return TRUE;
}

/*----------------------------------------------------------------------
Function name:  BansheeMoveCursor

Description:    The V2/V3 HW dependent MoveCursor function.  Only called
                if a HW cursor is active.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure due to EXCLUSIVE mode
----------------------------------------------------------------------*/
BOOL BansheeMoveCursor(SHORT x, SHORT y)
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return FALSE;

    //
    // Don't allow X and Y to go < 0
    //
    // Adjust the cursor by 63x63 to account for lower right start
    // and subtract off the Hotspot
    x = x + 0x3F - _FF(HotspotX);
    y = y + 0x3F - _FF(HotspotY);

    if (x < 0)
        x = 0;
 
    if (y < 0)
        y = 0;
 
    SETDW(lph3IORegs->hwCurLoc, ((DWORD)y << 16) | x);

    _FF(LastCursorPosX) =  _FF(CursorPosX) ;
    _FF(LastCursorPosY) =  _FF(CursorPosY) ;
 
    return TRUE;
}

/*----------------------------------------------------------------------
Function name:  BansheeEnableCursor

Description:    The V2/V3 HW dependent EnableCursor function

Information:

Return:         VOID
----------------------------------------------------------------------*/
void BansheeEnableCursor(int nFlag)
{
    DWORD dwData;

   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return;

    // Make sure that we are using Microsoft Cursor
    dwData = GET(lph3IORegs->vidProcCfg) & ~(SST_CURSOR_MODE);
 
    // Enable or Disable Cursor dependent on the flag passed in
    if (nFlag)
        dwData |= (SST_CURSOR_EN);
    else
        dwData &= ~(SST_CURSOR_EN);
 
    SETDW(lph3IORegs->vidProcCfg, dwData);
}
/* ************************************************************  */ 
 

/***************************************************************************
 *
 *   Here comes the Host cursor
 *
 ***************************************************************************/
#ifdef SLI_AA
extern DoBitCopy(DWORD, WORD, DWORD, WORD, DWORD, DWORD, WORD, WORD);
extern DoBitAndCopy(DWORD, WORD, DWORD, WORD, DWORD, WORD, WORD);
extern DoBitXorCopy(DWORD, WORD, DWORD, WORD, DWORD, WORD, WORD);
extern UINT GetFlatSel(void);
/*----------------------------------------------------------------------
Function name:  DoAndExpand

Description:    This function expands the And Mask to correct Pixel Depth
Information:

Return:         zero
----------------------------------------------------------------------*/
int DoAndExpand(DWORD * lpAndMask, DWORD dwCursor, DWORD dwBpp)
{
   DWORD AndMask;
   DWORD dw1;
   DWORD dw2;
   DWORD dwFB;
   DWORD dwMask;
   int i;
   int j;
   int k;
   int nSize;
   BYTE AndByte;

   if (8 == dwBpp)
      {
      nSize = 1;
      dwMask = 0xFFL;
      }
   else if (16 == dwBpp)
      {
      nSize = 2;
      dwMask = 0xFFFFL;
      }
   else if (24 == dwBpp)
      {
      nSize = 3;
      dwMask = 0xFFFFFFL;
      }
   else if (32 == dwBpp)
      {
      nSize = 4;
      dwMask = 0xFFFFFFFFL;
      }

   for (i=0; i<32; i++)
      {
      AndMask = lpAndMask[i];
      dwFB = dwCursor;
      dwCursor += 128;
      for (j=0; j<4; j++) 
         {
            AndByte = (BYTE)(AndMask & 0xFF);
            for (k=0; k<8; k++)
               {
               // Read FB and Mask off new portion
               dw2 = h3READ(0x0, (DWORD *)dwFB) & ~dwMask;

               if (AndByte & 0x80)
                  dw1 = 0xFFFFFFFFL & dwMask;
               else
                  dw1 = 0x00000000L & dwMask;

               dw1 |= dw2;      
               h3WRITE(0x0, (DWORD *)dwFB, dw1);

               dwFB += nSize;
               AndByte <<=1;         
               }
         AndMask >>= 8;
         }
      }
   
   return 0;
}

/*----------------------------------------------------------------------
Function name:  DoXorExpand

Description:    This function expands the Xor Mask to correct Pixel Depth
Information:

Return:         zero
----------------------------------------------------------------------*/
int DoXorExpand(BYTE * lpXorMask, DWORD dwCursor, DWORD dwBpp)
{
   DWORD dw1;
   DWORD dw2;
   DWORD dwFB;
   DWORD dwMask;
   int i;
   int j;
   int nSize;
   int k;
   int l;

   if (8 == dwBpp)
      {
      nSize = 1;
      dwMask = 0xFFL;
      }
   else if (16 == dwBpp)
      {
      nSize = 2;
      dwMask = 0xFFFFL;
      }
   else if (24 == dwBpp)
      {
      nSize = 3;
      dwMask = 0xFFFFFFL;
      }
   else if (32 == dwBpp)
      {
      nSize = 4;
      dwMask = 0xFFFFFFL;
      }

   for (i=0; i<32; i++)
      {
      dwFB = dwCursor;
      dwCursor += 128;
      for (j=0; j<4; j++) 
         {
         for (k=0; k<8; k++)
            {
            dw1 = 0;
            for (l=0; l<nSize; l++)
               dw1 |= (((DWORD)lpXorMask[l]) << (8*l));
            
            dw2 = h3READ(0x0, (DWORD *)dwFB) & ~dwMask;
            dw1 |= dw2;
            h3WRITE(0x0, (DWORD *)dwFB, dw1);
            dwFB += nSize;
            lpXorMask += nSize;
            }
         }
      }
   
   return 0;
}

/*----------------------------------------------------------------------
Function name:  DoXor1BitExpand

Description:    This function expands a 1BIT Xor Mask to correct Pixel Depth
Information:

Return:         zero
----------------------------------------------------------------------*/
int DoXor1BitExpand(BYTE * lpXorMask, DWORD dwCursor, DWORD dwBpp)
{
   DWORD dw1;
   DWORD dw2;
   DWORD dwFB;
   DWORD dwMask;
   int i;
   int j;
   int nSize;
   int k;
   BYTE XorByte;

   if (8 == dwBpp)
      {
      nSize = 1;
      dwMask = 0xFFL;
      }
   else if (16 == dwBpp)
      {
      nSize = 2;
      dwMask = 0xFFFFL;
      }
   else if (24 == dwBpp)
      {
      nSize = 3;
      dwMask = 0xFFFFFFL;
      }
   else if (32 == dwBpp)
      {
      nSize = 4;
      dwMask = 0xFFFFFFL;
      }

   for (i=0; i<32; i++)
      {
      dwFB = dwCursor;
      dwCursor += 128;
      for (j=0; j<4; j++) 
         {
         XorByte = *lpXorMask++;
         for (k=0; k<8; k++)
            {
            if (XorByte & 0x80)
               dw1 = dwMask;
            else
               dw1 = 0x0;
            XorByte <<=1;            

            dw2 = h3READ(0x0, (DWORD *)dwFB) & ~dwMask;
            dw1 |= dw2;
            h3WRITE(0x0, (DWORD *)dwFB, dw1);
            dwFB += nSize;
            }
         }
      }
   
   return 0;
}

/*----------------------------------------------------------------------
Function name:  GetBPP

Description:    Get Bytes per Pixels
Information:

Return:         Bytes Per Pixel
----------------------------------------------------------------------*/
SHORT GetBPP(void)
{
   SHORT wReturn = 0x0;
   wReturn = (WORD)(ModeList[_FF(ModeNumber)].dwBPP + 7) >> 3;
   return wReturn;
}

/*----------------------------------------------------------------------
Function name:  GetPitch

Description:    Get Pitch
Information:

Return:         Pitch In Bytes
----------------------------------------------------------------------*/
DWORD GetPitch()
{
   DWORD dwPitch;

   if (_FF(gdiDesktopStart) & SSTG_IS_TILED)
      {
      dwPitch = _FF(ddTilePitch);
      }
   else
      {
      dwPitch = _FF(pitch);
      }
      
   return dwPitch;
}

/*----------------------------------------------------------------------
Function name:  GetFBAddr

Description:    Get FB Address
Information:

Return:         Get FB Address
----------------------------------------------------------------------*/
DWORD GetFBAddr(int X, int Y, WORD wBytesPerPixel)
{
   DWORD dwPitch;
   DWORD dwOffset;
   DWORD dwReturn;
   DWORD tileInX;
   DWORD tileInY;

   if (_FF(gdiDesktopStart) & SSTG_IS_TILED)
      {
      dwPitch = _FF(ddTilePitch);
      dwOffset = _FF(gdiDesktopStart) & ~SSTG_IS_TILED;
      dwOffset -= _FF(ddTileMark);
      tileInY = (dwOffset >> 12)/_FF(ddTileStride);
      tileInX = (dwOffset >> 12)%_FF(ddTileStride);
      dwOffset = (tileInY << 5) * dwPitch + (tileInX << 7) + _FF(ddTileMark);
      }
   else
      {
      dwPitch = _FF(pitch);
      dwOffset = _FF(CurrentSurface);
      }
      
   dwReturn = dwOffset + _FF(lfbBase) + Y * dwPitch + X * (DWORD)wBytesPerPixel;

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  HostSetCursor

Description:    The Host Set cursor function. 
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure
----------------------------------------------------------------------*/
BOOL HostSetCursor(CURSORSHAPE FAR * lpCursor)
{
   DWORD srcFormat;
   DWORD dstFormat;
   DWORD dstBPP;
   BYTE * lpXorMask;
   DWORD * lpAndMask;
   LPPOINTERINFO lpPointer = (LPPOINTERINFO)lpCursor;


    if (_FF(lpPDevice)->deFlags & BUSY)
    {
        return(FALSE);
    }
 
    if (0x0101 == ((LPPOINTERINFO)lpCursor)->csColor)
      dstFormat=(_FF(screenFormat) & SSTG_SRC_FORMAT) | CURSOR_BMP_STRIDE;
    else
      dstFormat=GetCursorFormat(lpCursor);

    _FF(SWcursorFormat) = dstFormat;
    if (_FF(gdiFlags) & SKIP_FLAGS)
      {
   	return TRUE;
      }

    lpAndMask = (DWORD *) &(lpPointer->csANDBits); 
    lpXorMask = (BYTE *) &(lpPointer->csXORBits); 

    // Determine Screen BPP
    switch (_FF(screenFormat) & SSTG_SRC_FORMAT)
      {
      // This ain't Possible
      case SSTG_PIXFMT_1BPP:
            dstBPP = 0x01;
#ifdef DEBUG
            _asm int 3;
#endif
            break;

      case SSTG_PIXFMT_8BPP:
            dstBPP = 0x08;
            break;

      case SSTG_PIXFMT_15BPP:
            dstBPP = 0x10;
            break;

      case SSTG_PIXFMT_16BPP:
            dstBPP = 0x10;
            break;

      case SSTG_PIXFMT_24BPP:
            dstBPP = 0x18;
            break;

      case SSTG_PIXFMT_32BPP:
            dstBPP = 0x20;
            break;

      case SSTG_PIXFMT_422YUV:
      case SSTG_PIXFMT_422UYV:
      default:
#ifdef DEBUG
            _asm int 3;
#endif
            dstBPP = 0x20;
            break;
      }

    FXWAITFORIDLE();
    SET_SLI_READ(H3VDD_ENABLE_SLI_READ);
    DoAndExpand(lpAndMask, _FF(HostcursorAndStart), dstBPP);

    srcFormat=GetCursorFormat(lpCursor);
    if (SSTG_PIXFMT_1BPP == (srcFormat & SSTG_SRC_FORMAT))
       DoXor1BitExpand(lpXorMask, _FF(HostcursorXorStart), dstBPP);
    else
       DoXorExpand(lpXorMask, _FF(HostcursorXorStart), dstBPP);
    FXWAITFORIDLE();
    SET_SLI_READ(_FF(dwSLIMode));

    return(TRUE);
}

/*----------------------------------------------------------------------
Function name:  HostEnableCursor

Description:    The Host EnableCursor function

Information:

Return:         VOID
----------------------------------------------------------------------*/
void HostEnableCursor(int nFlag)
{
    if (_FF(lpPDevice)->deFlags & BUSY)
       {
       return ;
       }

   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SKIP_FLAGS)
      return;

    // If cursor is already enabled and sw then erase it...
    if ((SDATA_GDIFLAGS_CURSOR_ENABLED == (SDATA_GDIFLAGS_CURSOR_ENABLED & _FF(gdiFlags))) &&
        (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR))
      {
      HostRestoreCursorExclude((int)_FF(LastCursorPosX), (int)_FF(LastCursorPosY));
      }
}     

/*----------------------------------------------------------------------
Function name:  HostSaveCursorExclude

Description:    Saves the area of the display under the cursor prior to
                drawing the cursor.

Information:

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int HostSaveCursorExclude(int X, int Y)
{ 
   DWORD dwDestLoc;
   DWORD dwSrcLoc;
   DWORD dwPitch;
   SHORT xSize;
   SHORT ySize;
   SHORT wBytesPerPixel;
   SHORT xLen;
   SHORT wMaskAdj;
   SHORT wScreenAdj;
   SHORT x1;
   SHORT y1;
   UINT FlatSel;
 
    if (_FF(gdiFlags) & (SKIP_FLAGS | SDATA_GDIFLAGS_CURSOR_EXCLUDE))
   	return TRUE;
   
   _FF(CursorFlipCount) = _FF(FlipCount);
   _FF(CursorSurface) = _FF(CurrentSurface);

   FlatSel = GetFlatSel();

    x1 = X;
    y1 = Y;
   
    xSize = SWCURSOR_WIDTH;
    if (X < 0)
       {
       xSize += X;
       X = 0;
       }

   if (X + SWCURSOR_WIDTH > _FF(hres))
      {
      xSize = _FF(hres) - X;
      }

    ySize = SWCURSOR_HEIGHT;
    if (Y < 0)
       {
       ySize += Y;
       Y = 0;
       }

   if (Y + SWCURSOR_HEIGHT > _FF(vres))
      {
      ySize = _FF(vres) - Y;
      }

    // Have we been clipped ?
    if ((xSize > 0) && (ySize > 0))
      {
      FXWAITFORIDLE();
      SET_SLI_READ(H3VDD_ENABLE_SLI_READ);
      wBytesPerPixel = GetBPP();
      dwPitch = GetPitch();
      xLen = xSize * wBytesPerPixel;

      dwSrcLoc = _FF(CurrentSurface) + Y * dwPitch + X * (DWORD)wBytesPerPixel;
      dwDestLoc = _FF(HostcursorExclusionStart);

      wScreenAdj = (WORD)dwPitch - xLen;
      wMaskAdj = 128 - xLen;
      
      // Copy Image to Offscreen
      DoBitCopy(dwSrcLoc, FlatSel, dwDestLoc, FlatSel, wScreenAdj, wMaskAdj, xLen, ySize);
      FXWAITFORIDLE();
      SET_SLI_READ(_FF(dwSLIMode));
      }  

    _FF(LastCursorPosX)= x1;
    _FF(LastCursorPosY)= y1;
 
    return(TRUE);
 
}

 
/*----------------------------------------------------------------------
Function name:  HostRestoreCursorExclude

Description:    Restore the area of the display where the cursor was
                drawn.

Information:

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int HostRestoreCursorExclude(int X, int Y)
{
   DWORD dwDestLoc;
   DWORD dwSrcLoc;
   DWORD dwPitch;
   SHORT xSize;
   SHORT ySize;
   SHORT wBytesPerPixel;
   SHORT xLen;
   SHORT wMaskAdj;
   SHORT wScreenAdj;
   UINT FlatSel;
    
   if ((_FF(CurrentSurface) != _FF(CursorSurface)) ||
       (_FF(FlipCount) != _FF(CursorFlipCount)))
      return TRUE;

   if (_FF(gdiFlags) & (SKIP_FLAGS | SDATA_GDIFLAGS_CURSOR_EXCLUDE))
   	return TRUE;

   FlatSel = GetFlatSel();

    xSize = SWCURSOR_WIDTH;
    if (X < 0)
      {
      xSize += X;
      X = 0;
      }

   if (X + SWCURSOR_WIDTH > _FF(hres))
      {
      xSize = _FF(hres) - X;
      }

    ySize = SWCURSOR_HEIGHT;
    if (Y < 0)
      {
      ySize += Y;
      Y = 0;
      }

   if (Y + SWCURSOR_HEIGHT > _FF(vres))
      {
      ySize = _FF(vres) - Y;
      }

    //Have we been clipped?
    if ((xSize > 0) && (ySize > 0))
      { 
      FXWAITFORIDLE();
      SET_SLI_READ(H3VDD_ENABLE_SLI_READ);
      wBytesPerPixel = GetBPP();
      dwPitch = GetPitch();
      xLen = xSize * wBytesPerPixel;

      dwSrcLoc = _FF(HostcursorExclusionStart);
      dwDestLoc = _FF(CurrentSurface) + Y * dwPitch + X * (DWORD)wBytesPerPixel;

      wScreenAdj = (WORD)dwPitch - xLen;
      wMaskAdj = 128 - xLen;
      
      // Copy Image to Offscreen
      DoBitCopy(dwSrcLoc, FlatSel, dwDestLoc, FlatSel, wMaskAdj, wScreenAdj, xLen, ySize);
      FXWAITFORIDLE();
      SET_SLI_READ(_FF(dwSLIMode));
      }

    return(TRUE);
}

/*----------------------------------------------------------------------
Function name:  HostDrawCursor

Description:    Draws the cursor 
Information:

Return:         int     TRUE = early return due to EXCLUSIVE mode
                        0    = Successfully drawn
----------------------------------------------------------------------*/
int HostDrawCursor(int X, int Y)
{
   DWORD Offset;
   DWORD dwDestLoc;
   DWORD dwSrcLoc;
   DWORD dwMaskLoc;
   DWORD dwPitch;
   SHORT xSize;
   SHORT ySize;
   SHORT xCur;
   SHORT yCur;
   SHORT wBytesPerPixel;
   SHORT xLen;
   SHORT wMaskAdj;
   SHORT wScreenAdj;
   UINT FlatSel;
   

   if (_FF(gdiFlags) & SKIP_FLAGS)
   	return TRUE;
   
   if ((SDATA_GDIFLAGS_CURSOR_ENABLED == (SDATA_GDIFLAGS_CURSOR_ENABLED & _FF(gdiFlags))) &&
       (SDATA_GDIFLAGS_CURSOR_EXCLUDE != (SDATA_GDIFLAGS_CURSOR_EXCLUDE & _FF(gdiFlags))))
      {

      FXWAITFORIDLE();
      SET_SLI_READ(H3VDD_ENABLE_SLI_READ);
      FlatSel = GetFlatSel();
      // if X is less then zero then fix it up
      xSize = SWCURSOR_WIDTH;
      xCur = 0;
      if (X < 0)
         {
         xSize = xSize + X;
         xCur = (SHORT)SWCURSOR_WIDTH - xSize;
         X = 0;  
         }

      if (X + SWCURSOR_WIDTH > _FF(hres))
         {
         xSize = _FF(hres) - X;
         }

      // Ditto with Y
      ySize = SWCURSOR_HEIGHT;
      yCur = 0;
      if (Y < 0)
         {
         ySize = ySize + Y;
         yCur = (SHORT)SWCURSOR_HEIGHT - ySize;
         Y = 0;  
         }

      if (Y + SWCURSOR_HEIGHT > _FF(vres))
         {
         ySize = _FF(vres) - Y;
         }

      // if the cursor is clipped no need to draw it
      if ((xSize < 1) || (ySize < 1))
         return 0;

      wBytesPerPixel = GetBPP();
      dwPitch = GetPitch();
      xLen = xSize * wBytesPerPixel;

      // Offset = yCur * Pitch (128) + xCur * Bytes Per Pixels
      Offset = (yCur<<7) + xCur * wBytesPerPixel;
      dwDestLoc = Offset + _FF(HostcursorSrcStart);
      dwSrcLoc = _FF(CurrentSurface) + Y * dwPitch + X * (DWORD)wBytesPerPixel;

      wScreenAdj = (WORD)dwPitch - xLen;
      wMaskAdj = 128 - xLen;
      
      // Copy Image to Offscreen
      DoBitCopy(dwSrcLoc, FlatSel, dwDestLoc, FlatSel, wScreenAdj, wMaskAdj, xLen, ySize);
      
      // Do And Mask
      dwMaskLoc = _FF(HostcursorAndStart) + Offset;
      DoBitAndCopy(dwMaskLoc, FlatSel, dwDestLoc, FlatSel, wMaskAdj, xLen, ySize);

      // Do Xor Mask
      dwMaskLoc = _FF(HostcursorXorStart) + Offset;
      DoBitXorCopy(dwMaskLoc, FlatSel, dwDestLoc, FlatSel, wMaskAdj, xLen, ySize);

      // Back to the screen
      DoBitCopy(dwDestLoc, FlatSel, dwSrcLoc, FlatSel, wMaskAdj, wScreenAdj, xLen, ySize);
      FXWAITFORIDLE();
      SET_SLI_READ(_FF(dwSLIMode));
   }

   return 0;
}

#endif




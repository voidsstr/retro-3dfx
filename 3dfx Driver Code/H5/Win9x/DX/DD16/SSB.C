/* -*-c++-*- */
/* $Header: ssb.c, 7, 10/11/00 8:51:40 PM, Brent$ */
/*
** Copyright (c) 1998-1999, 3Dfx Interactive, Inc.
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
** File name:   ssb.c
**
** exported function:
**     SaveScreenBitmap
**
** public function:
**     InvalidateSSB, DiscardAllSSB
**
** Description: SaveScreenbitmap functions.  Save a desktop
**              rectangle to off-screen memory.
**              Restore the previously saved rectangle to desktop.
**              Discard the saved rectangle.
**
** $Revision: 7$
** $Date: 10/11/00 8:51:40 PM$
**
** $History: ssb.c $
** 
** *****************  Version 28  *****************
** User: Cwilcox      Date: 8/27/99    Time: 11:33a
** Updated in $/devel/h5/Win9x/dx/dd16
** Fixed PRS #7981, caused by failing to disable SaveScreenBitmap when
** entering DirectX.
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 6/16/99    Time: 4:47p
** Updated in $/devel/h5/Win9x/dx/dd16
** Added SKIP_FLAGS and PUNT_EXP
** 
** *****************  Version 25  *****************
** User: Michael      Date: 12/30/98   Time: 9:31a
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 24  *****************
** User: Andrew       Date: 11/30/98   Time: 1:01a
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to check to see if the Device is Busy before using the
** Device.  The Busy bit is set in low power mode so the device will
** return all f's if read from it when PCI64=D3.  This will cause a
** infinite SW loop.
** 
** *****************  Version 23  *****************
** User: Michael      Date: 8/18/98    Time: 3:04p
** Updated in $/devel/h3/Win95/dx/dd16
** Minor WB98 optimization for the work Andy did on the previous checkin.
** The "save under" bugs should now all be fixed.
** 
** *****************  Version 22  *****************
** User: Andrew       Date: 8/14/98    Time: 11:42p
** Updated in $/devel/h3/Win95/dx/dd16
** Removed the debug code around the rect so that we can use the rect to
** see if we have right bitmap
** 
** *****************  Version 21  *****************
** User: Michael      Date: 7/17/98    Time: 5:01p
** Updated in $/devel/h3/Win95/dx/dd16
** It appears that SSB_SAVE and SSB_RESTORE are not always called
** sequentially as the documentaion says.  The problem is that we get a
** SSB_DISCARD call and then a SSB_RESTORE call for the Bitmap whose
** memory has already been freed back into the memory pool.  This will
** potentially result in trashed memory being restored if anyone else
** allocates a bitmap prior to the restore.  Fixes 1465.
**
*/

#include "h3.h"
#include "header.h"
#include "memmgr16.h"
#include <entrleav.h>
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

#ifdef PERF_NEWMM
#include "robject.h"
#endif


//-- External references ----------------------------------------------------
//
extern WORD PASCAL wFlatDataSel;
#ifdef PERF_NEWMM
extern PVRAMNODE AllocOffscreenMemory(DWORD BitmapSize);
extern void FreeOffscreenMemory(PVRAMNODE pNode);
#endif

//
//-- export function prototypes ---------------------------------------------
//
UINT _loadds WINAPI SaveScreenBitmap(LPRECT, WORD);

//===========================================================================
#ifdef SSB
//===========================================================================

#ifdef CMDFIFO
DWORD dwordCount;
#endif

//
//-- Definitions in fifomgr.h (16 bit version)-------------------------------
//
#if !defined(BREAK_STALL)
#define BREAK_STALL()	         // as nothing
#endif

#if !defined(POLL_STALL)
#define POLL_STALL()           // as nothing
#endif

#if !defined(DEBUG_FIX)
#define DEBUG_FIX
#endif

//
//-- Definitions for SaveScreenBitmap ---------------------------------------
//
#define   MAX_SSB        4     // max. number of bitmap saved at once

#define   SSB_SAVE       0     // SaveScreenBitmap commands
#define   SSB_RESTORE    1
#define   SSB_DISCARD    2

// SaveScreenBitmap caching flags
#define   SSB_TRASHED    1     // set by InvalidateSSB only

typedef struct tag_ssbInfo
{
    DWORD dwFlags;       // set if cached bitmap is invalidated
#ifdef PERF_NEWMM    
    PVRAMNODE   pNode;   // pointer to the node that references the stored bitmap
#else    
    DWORD fpVidMemStart; // off-screen start address of the saved bitmap
#endif    
    RECT  rect;          // save the rectangle
    // we have 16 bytes now, a power of two makes efficient array math
}
SSBINFO;

typedef enum tag_destType { to_desktop, to_offscreen } DESTTYPE;


//
//-- function prototypes ----------------------------------------------------
//
void DiscardAllSSB(void);
#ifdef PERF_NEWMM
void _loadds WINAPI InvalidateSSB(PVRAMNODE);
extern void CacheDepopulate();

#else
void _loadds WINAPI InvalidateSSB(DWORD);
#endif
void XferBits(DWORD, DWORD, int,int, int,int, int, int, DESTTYPE);

//
//
//-- Globals ----------------------------------------------------------------
//

BOOL saveScreenBitmapAllowed  = FALSE; // initialize in h3.c, if mminit() is successful
BOOL saveScreenBitmapDisabled = FALSE; // set by dddrv16.c to disable when DirectX active
int    ssb_top = -1;     // stack is empty
SSBINFO ssb_stack[MAX_SSB];

#ifdef NULLDRIVER
FxU32 nullAll = 0;
FxU32 nullSSB = 0;
#endif // #ifdef NULLDRIVER


extern GLOBALDATA      *lpDriverData;


/*----------------------------------------------------------------------
Function name:  SaveScreenBitmap

Description:    Save and restore a screen rectangle to/from
                off-screen memory.
Information:    
    Input
      lpRect   pointer to a rectangle to save/restore
      command  save, restore or discard the saved bitmap
  
Return:         UINT    1 for success, 0 for failure.
----------------------------------------------------------------------*/
UINT _loadds WINAPI SaveScreenBitmap(LPRECT lpRect, WORD wCommand)
{
    int            width, height;
    DWORD          dwWidthBytes;
#ifdef PERF_NEWMM
    PVRAMNODE      pAlloc;
    DWORD          dwBitmapSize;
#else        
    DWORD          dwRetCode;
    DWORD          fpVidMemStart;
    MM_ALLOCSIZE   AllocSize;
#endif    
    RECT           ssbRect;

#ifdef NULLDRIVER
    if (nullAll || nullSSB)
	return(SUCCESS);
#endif // #ifdef NULLDRIVER

    if (lpDriverData->gdiFlags & SKIP_FLAGS)
         return SUCCESS;

    // Added this case to handle when we are in Low Power Mode
    // When we enter D3 then everything goes away and so
    // we should just Punt
    if (PUNT_EXP)
       return (FAIL);  

    if ((!saveScreenBitmapAllowed) || (saveScreenBitmapDisabled))
    {
       return (FAIL);    // let Windows handle request
    }

    switch( wCommand )
    {  //
       // Save the rectangle to off-screen memory.
       //
       case SSB_SAVE:
       {
          // fail the call if the ssb_stack is full.
          if (ssb_top >= MAX_SSB)
          {
             (">>> (ssb) Stack is full, stack size is %lx <<<", ssb_top);
             return (FAIL);
          }

          width = (lpRect->right - lpRect->left);
          height= (lpRect->bottom - lpRect->top);
          dwWidthBytes = ((DWORD) width) * (_FF(bpp) >> 3);
          dwWidthBytes = (dwWidthBytes + 3) & ~3;   // round up to next dword

#ifdef PERF_NEWMM
          dwBitmapSize = dwWidthBytes * height;
          pAlloc = AllocOffscreenMemory(dwBitmapSize);
          
          if (pAlloc)
          {
             pAlloc->NodeFlags = FLAG_GDI_SSB;
             // push bitmap on stack
             ssb_top++;
             ssb_stack[ssb_top].dwFlags &= ~SSB_TRASHED;
             ssb_stack[ssb_top].pNode = pAlloc;
             ssb_stack[ssb_top].rect = *lpRect;
             if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR))
             {  //
                // software cursor is in use, exclude it
                // BeginAccess will wait for blter to idle
                //
                BeginAccess(_FF(lpPDevice), lpRect->left, lpRect->top,
                   lpRect->right, lpRect->bottom, CURSOREXCLUDE);
             }

             XferBits(dwWidthBytes, pAlloc->dwStart,
                lpRect->left, lpRect->top, 0, 0,
                width, height, to_offscreen);

             if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR))
             {  //
                // software cursor is in use, unexclude it
                //
                EndAccess(_FF(lpPDevice), CURSOREXCLUDE);
             }
#ifdef DEBUG
             DPF(DBGLVL_SSB, "(ssb) save: sp(%x) Rect(%d %d %d %d)", ssb_top, ssb_stack[ssb_top].rect.left, ssb_stack[ssb_top].rect.top, ssb_stack[ssb_top].rect.right, ssb_stack[ssb_top].rect.bottom);
#endif
             return (SUCCESS);
          }
                                                         
#else   // PERF_NEWMM
          AllocSize.dwSize = sizeof(MM_ALLOCSIZE);
          AllocSize.dwRequestSize = (DWORD) dwWidthBytes * (DWORD) height;
          AllocSize.dwFlags = MM_ALLOC_FROM_END_OF_HEAP;
          AllocSize.mmAlignType = dword_align;
          AllocSize.mmHeapType = linear_heap1;
          AllocSize.fpHostifyCallBackFunc = (FARPROC)InvalidateSSB;
          fpVidMemStart = mmAlloc(&AllocSize, &dwRetCode);
          if (MM_OK == dwRetCode)
          {
             // push bitmap on stack
             ssb_top++;
             ssb_stack[ssb_top].dwFlags &= ~SSB_TRASHED;
             ssb_stack[ssb_top].fpVidMemStart = fpVidMemStart;
             ssb_stack[ssb_top].rect = *lpRect;

             if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR))
             {  //
                // software cursor is in use, exclude it
                // BeginAccess will wait for blter to idle
                //
                BeginAccess(_FF(lpPDevice), lpRect->left, lpRect->top,
                   lpRect->right, lpRect->bottom, CURSOREXCLUDE);
             }

             XferBits(dwWidthBytes, fpVidMemStart,
                lpRect->left, lpRect->top, 0, 0,
                width, height, to_offscreen);

             if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR))
             {  //
                // software cursor is in use, unexclude it
                //
                EndAccess(_FF(lpPDevice), CURSOREXCLUDE);
             }
#ifdef DEBUG
             DPF(DBGLVL_SSB, "(ssb) save: sp(%x) Rect(%d %d %d %d)", ssb_top, ssb_stack[ssb_top].rect.left, ssb_stack[ssb_top].rect.top, ssb_stack[ssb_top].rect.right, ssb_stack[ssb_top].rect.bottom);
#endif
             return (SUCCESS);
          }
          
#endif // PERF_NEWMM
          
          else  // alloc fail
          {
             return (FAIL);    // no off-screen memory
          }

       }  // endcase SSB_SAVE

       //
       // Restore the previously saved rectangle to desktop.
       //
       case SSB_RESTORE:
       {
          if (ssb_top < 0)     // stack is empty if ssb_top == -1
          {
             return (FAIL);    // nothing to restore
          }

          // is it already discarded and freed by mmEvict() ?
          if (ssb_stack[ssb_top].dwFlags & SSB_TRASHED)
          {
             ssb_stack[ssb_top].dwFlags &= ~SSB_TRASHED;
             ssb_top--;        // just pop it off the stack
             return (FAIL);    // tell Windows to redraw
          }

#ifdef PERF_NEWMM

          // Contrary to DDK documentation, the restores are sometimes
          // out of order.  This is responsible for alot of desktop
          // corruption that was being reported.  We handle it by
          // discarding all SSBs once this situation occurs as subsequent
          // SSB restorations will most likely also be out of order.
          // We also special case here for WB98.  There is no reason to
          // do the check if we are only one deep.  WB98 never gets
          // SSBs more than one deep.
          //
          // Fix for PRS 13948. Always check for out of order restores,
          // no matter how many are on the stack. Even with only one bitmap
          // on the stack GDI can still request restores on bitmaps that
          // have been discarded.
            
	     ssbRect = ssb_stack[ssb_top].rect;

        // ensure at least the size is the same
#ifndef DEBUG
         if (  (ssbRect.left != lpRect->left) ||
               (ssbRect.right != lpRect->right) ||
               (ssbRect.top != lpRect->top) ||
               (ssbRect.bottom != lpRect->bottom)
#else
          if ( (0L == ssb_stack[ssb_top].pNode->dwStart) ||
               (ssbRect.left != lpRect->left) ||
               (ssbRect.right != lpRect->right) ||
               (ssbRect.top != lpRect->top) ||
               (ssbRect.bottom != lpRect->bottom)
#endif
             )
          {
             DPF(DBGLVL_SSB, ">>> (ssb) restore: saved rectangle differs <<<");
             return (FAIL);
          }

          width = (lpRect->right - lpRect->left);
          height= (lpRect->bottom - lpRect->top);
          
          dwWidthBytes = ((DWORD) width) * (_FF(bpp) >> 3);
          dwWidthBytes = (dwWidthBytes + 3) & ~3;   // round up to next dword

          if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR))
          {  //
             // software cursor is in use, exclude it
             // BeginAccess will wait for blter to idle
             //
             BeginAccess(_FF(lpPDevice), lpRect->left, lpRect->top,
                lpRect->right, lpRect->bottom, CURSOREXCLUDE);
          }                   

          XferBits(dwWidthBytes, ssb_stack[ssb_top].pNode->dwStart, 0, 0,
             lpRect->left, lpRect->top, width, height, to_desktop);

          if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR))
          {  //
             // software cursor is in use, unexclude it
             //
             EndAccess(_FF(lpPDevice), CURSOREXCLUDE);
          }

          FreeOffscreenMemory(ssb_stack[ssb_top].pNode);
          ssb_top--;        // pop SSB off the stack

#ifdef DEBUG
          ssb_stack[ssb_top+1].pNode = 0L;
          DPF(DBGLVL_SSB, "(ssb) restore: sp(%x) Rect(%d %d %d %d)", ssb_top+1, ssb_stack[ssb_top+1].rect.left, ssb_stack[ssb_top+1].rect.top, ssb_stack[ssb_top+1].rect.right, ssb_stack[ssb_top+1].rect.bottom);
#endif
          return (SUCCESS);
      }   

#else   // PERF_NEWMM
         
          fpVidMemStart = ssb_stack[ssb_top].fpVidMemStart;

          // Contrary to DDK documentation, the restores are sometimes
          // out of order.  This is responsible for alot of desktop
          // corruption that was being reported.  We handle it by
          // discarding all SSBs once this situation occurs as subsequent
          // SSB restorations will most likely also be out of order.
          // We also special case here for WB98.  There is no reason to
          // do the check if we are only one deep.  WB98 never gets
          // SSBs more than one deep.
          if (ssb_top > 0)
          {
		     ssbRect = ssb_stack[ssb_top].rect;

             // ensure at least the size is the same
#ifndef DEBUG
             if ((ssbRect.left != lpRect->left) ||
                 (ssbRect.right != lpRect->right) ||
                 (ssbRect.top != lpRect->top) ||
                 (ssbRect.bottom != lpRect->bottom)
#else
             if ( (0L == fpVidMemStart) ||
                  (ssbRect.left != lpRect->left) ||
                  (ssbRect.right != lpRect->right) ||
                  (ssbRect.top != lpRect->top) ||
                  (ssbRect.bottom != lpRect->bottom)
#endif
                )
             {
                DPF(DBGLVL_SSB, ">>> (ssb) restore: saved rectangle differs <<<");
                return (FAIL);
             }
          }

          width = (lpRect->right - lpRect->left);
          height= (lpRect->bottom - lpRect->top);

          dwWidthBytes = ((DWORD) width) * (_FF(bpp) >> 3);
          dwWidthBytes = (dwWidthBytes + 3) & ~3;   // round up to next dword

         if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR))
          {  //
             // software cursor is in use, exclude it
             // BeginAccess will wait for blter to idle
             //
             BeginAccess(_FF(lpPDevice), lpRect->left, lpRect->top,
                lpRect->right, lpRect->bottom, CURSOREXCLUDE);
          }

          XferBits(dwWidthBytes, fpVidMemStart, 0, 0,
             lpRect->left, lpRect->top, width, height, to_desktop);

          if (_FF(gdiFlags) & (SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR))
          {  //
             // software cursor is in use, unexclude it
             //
             EndAccess(_FF(lpPDevice), CURSOREXCLUDE);
          }

          mmFree(fpVidMemStart);
          ssb_top--;        // pop SSB off the stack

#ifdef DEBUG
          ssb_stack[ssb_top+1].fpVidMemStart = 0L;
          DPF(DBGLVL_SSB, "(ssb) restore: sp(%x) Rect(%d %d %d %d)", ssb_top+1, ssb_stack[ssb_top+1].rect.left, ssb_stack[ssb_top+1].rect.top, ssb_stack[ssb_top+1].rect.right, ssb_stack[ssb_top+1].rect.bottom);
#endif
          return (SUCCESS);

       }  //endcase SSB_RESTORE
#endif // PERF_NEWMM          

       //
       // Discard the previously saved rectangle, if there was one.
       // Note that lpRect is ignored and is usually null.
       //
       case SSB_DISCARD:
       {
          DiscardAllSSB();      // discard everything on the stack
          return (SUCCESS);
       }

       default:
          return (FAIL);
    }  // SSB commands
}


/*----------------------------------------------------------------------
Function name:  InvalidateSSB

Description:    Called by mmEvict to invalidate the saved screen
                bitmap in order to make room for DDRAW allocation.
                Note that we cannot change ssb_top because we may
                not always be evicting the the one that is on top
                of the stack.
Information:    
    Input
      fpVidMemStart  tells us which bitmap to invalidate
  
Return:         VOID
----------------------------------------------------------------------*/
#ifdef PERF_NEWMM 

void _loadds WINAPI InvalidateSSB(PVRAMNODE pNode)
{
    int itop = ssb_top;

    while (itop >= 0)
    {
       if (ssb_stack[itop].pNode == pNode)
       {
          ssb_stack[itop].dwFlags |= SSB_TRASHED;  // let SSB_RESTORE know
          FreeOffscreenMemory(pNode);
          break;
       }
       itop--;
    }

#ifdef DEBUG
    if (itop < 0)
    {
       DPF(DBGLVL_SSB, ">>> (InvalidateSSB) SSB not found. <<<");
    }
#endif
}

#else   // PERF_NEWMM

void _loadds WINAPI InvalidateSSB(DWORD fpVidMemStart)
{
    int itop = ssb_top;

    while (itop >= 0)
    {
       if (ssb_stack[itop].fpVidMemStart == fpVidMemStart)
       {
          ssb_stack[itop].dwFlags |= SSB_TRASHED;  // let SSB_RESTORE know
          mmFree(fpVidMemStart);
          break;
       }
       itop--;
    }

#ifdef DEBUG
    if (itop < 0)
    {
       DPF(DBGLVL_SSB, ">>> (InvalidateSSB) SSB not found. <<<");
    }
#endif
}

#endif // PERF_NEWMM

/*----------------------------------------------------------------------
Function name:  DiscardAllSSB

Description:    Discard all off-screen saved screen bitmaps.  This
                is called when the desktop is to be switched out
                to background.  We will pop the items off the stack,
                and free the associated memory.  When Windows call
                us to restore it, the stack will be empty, we return
                FAIL to tell the OS to repaint.
Information:    
  
Return:         VOID
----------------------------------------------------------------------*/
void DiscardAllSSB(void)
{
#ifdef PERF_NEWMM
    PVRAMNODE   pNode;
    
    while (ssb_top >= 0)
    {  //
       // ensure the item is not already invalidated by mmEvict, and
       // the address we pass to mmFree is not null
       //
       if (!(ssb_stack[ssb_top].dwFlags & SSB_TRASHED) &&
            (pNode = ssb_stack[ssb_top].pNode)
          )
       {
            FreeOffscreenMemory(pNode);

#ifdef DEBUG
          ssb_stack[ssb_top].dwFlags |= SSB_TRASHED;
#endif
       }
       ssb_top--;
    }

#ifdef DEBUG
    if (ssb_top >= 0)    // stack shoud be empty
    {
       (">>> (ssb) not all SSB are discarded. <<<");
    }
#endif

#else   // PERF_NEWMM

    DWORD fpVidMemStart;

    while (ssb_top >= 0)
    {  //
       // ensure the item is not already invalidated by mmEvict, and
       // the address we pass to mmFree is not null
       //
       if (!(ssb_stack[ssb_top].dwFlags & SSB_TRASHED) &&
            (fpVidMemStart = ssb_stack[ssb_top].fpVidMemStart)
          )
       {
          mmFree(fpVidMemStart);

#ifdef DEBUG
          ssb_stack[ssb_top].dwFlags |= SSB_TRASHED;
#endif
       }
       ssb_top--;
    }

#ifdef DEBUG
    if (ssb_top >= 0)    // stack shoud be empty
    {
       (">>> (ssb) not all SSB are discarded. <<<");
    }
#endif

#endif // PERF_NEWMM
}


/*----------------------------------------------------------------------
Function name:  XferBits

Description:    Perform a simple screen to screen blt, no overlap,
                always SRCCOPY and direction bit is zero.
Information:    
    Input
      dwWidthBytes   width in bytes rounded to dword, and align on 16 bit
      fpVidMemStart  linear starting address of off-screen memory
      srcX           source X (in pixel)
      srcY           source Y (in pixel)
      dstX           destination X (in pixel)
      dstY           destination Y (in pixel)
      width          width of rectangle to transfer (in pixel)
      height         height of rectangle to transfer
      destination    transfer rectangle to desktop or off-screen VRAM

    NOTE:  src/dstFormat and src/dstAddr registers are restored
           to desktop settings
  
Return:         VOID
----------------------------------------------------------------------*/
#ifdef PERFNOP
FxU32 doPerfNop = 0;
#endif // #ifdef PERFNOP

void 
XferBits(DWORD dwWidthBytes, DWORD fpVidMemStart,
	 int srcX, int srcY, int dstX, int dstY,
	 int width, int height, DESTTYPE destination)
{
    DWORD srcFormat, dstFormat;
    DWORD srcAddr, dstAddr;
    DWORD srcXY, dstXY;

    CMDFIFO_PROLOG(cmdFifo);

    if (_FF(gdiFlags) & SKIP_FLAGS)
	return;

    _FF(lpPDevice)->deFlags |= BUSY;

    CMDFIFO_SETUP(cmdFifo);

    if (_FF(gdiFlags) & (SDATA_GDIFLAGS_2D_DIRTY |
			 SDATA_GDIFLAGS_DST_WAS_DEVBIT |
			 SDATA_GDIFLAGS_SRC_WAS_DEVBIT))
    {
	CMDFIFO_CHECKROOM(cmdFifo, 7);
	SETPH(cmdFifo,   (SSTCP_PKT2 |
			  clip0minBit |
			  clip0maxBit |
			  dstBaseAddrBit |
			  dstFormatBit |
			  srcBaseAddrBit |
			  commandExBit));
	SET(cmdFifo, lph3g->clip0min, 0);
	SET(cmdFifo, lph3g->clip0max,
	    (_FF(bi).biHeight << 16) | _FF(bi).biWidth);
	SET(cmdFifo, lph3g->dstBaseAddr, _FF(gdiDesktopStart));
	SET(cmdFifo, lph3g->dstFormat, _FF(screenFormat));
	SET(cmdFifo, lph3g->srcBaseAddr, _FF(gdiDesktopStart));
	SET(cmdFifo, lph3g->commandEx, 0);
	BUMP(7);

	_FF(gdiFlags) &= ~(SDATA_GDIFLAGS_2D_DIRTY |
			   SDATA_GDIFLAGS_DST_WAS_DEVBIT |
			   SDATA_GDIFLAGS_SRC_WAS_DEVBIT);
    }

    //
    // get default settings
    //
    srcFormat = _FF(screenFormat);
    srcAddr = _FF(gdiDesktopStart);
    dstFormat = _FF(screenFormat);
    dstAddr = _FF(gdiDesktopStart);

    if (to_desktop == destination)   // restore saved bitmap?
    {
	srcFormat &= ~(SSTG_SRC_PACK);
	srcFormat |= SSTG_SRC_PACK_SRC;
	srcFormat &= (0xFFFFC000);
	srcFormat |= (dwWidthBytes & 0x3FFF);
	srcAddr = fpVidMemStart - _FF(lfbBase);
	srcXY = 0L;
	dstXY = (((DWORD)dstY << 16) | (DWORD)dstX);
    }
    else  // save bitmap to off screen
    {
	dstFormat &= ~(SSTG_SRC_PACK);
	dstFormat |= SSTG_SRC_PACK_SRC;
	dstFormat &= (0xFFFFC000);
	dstFormat |= (dwWidthBytes & 0x3FFF);
	dstAddr = fpVidMemStart - _FF(lfbBase);
	srcXY = (((DWORD)srcY << 16) | (DWORD)srcX);
	dstXY = 0L;
    }

    //
    // issue the blt
    //
    CMDFIFO_CHECKROOM(cmdFifo, 9);
    SETPH(cmdFifo, (SSTCP_PKT2 |
		    dstBaseAddrBit |
		    dstFormatBit |
		    srcBaseAddrBit |
		    srcFormatBit |
		    srcXYBit |
		    dstSizeBit |
		    dstXYBit |
		    commandBit));
    SET(cmdFifo, lph3g->dstBaseAddr, dstAddr);
    SET(cmdFifo, lph3g->dstFormat, dstFormat);
    SET(cmdFifo, lph3g->srcBaseAddr, srcAddr);
    SET(cmdFifo, lph3g->srcFormat, srcFormat);
    SET(cmdFifo, lph3g->srcXY, srcXY);
    SET(cmdFifo, lph3g->dstSize, (((DWORD)height << 16) | (DWORD)width));
    SET(cmdFifo, lph3g->dstXY, dstXY);
    SETC(cmdFifo, lph3g->command, 0xCC000000L | SSTG_GO | SSTG_BLT);
    BUMP(9);

    //
    // restore src/dstAddr and src/dstFormat to desktop settings
    //
    if (to_desktop == destination)
	_FF(gdiFlags) |= SDATA_GDIFLAGS_SRC_WAS_DEVBIT;
    else
	_FF(gdiFlags) |= SDATA_GDIFLAGS_DST_WAS_DEVBIT;



    _FF(lpPDevice)->deFlags &= ~BUSY;

    CMDFIFO_EPILOG(cmdFifo);
}

//===========================================================================
#else  // ifndef SSB
//===========================================================================


/*----------------------------------------------------------------------
Function name:  SaveScreenBitmap

Description:    Since display.def is not built automatically,
                we always have to inlcude this dummy entry point
                so that the linker can resolve the exported
                SaveScreenBitmap function.  This will never be
                called when ifndef SSB is true.
                
Information:    only compiled when "#ifndef SSB"

Return:         UINT    FAIL is always returned.
----------------------------------------------------------------------*/

//
// Since display.def is not built automatically, we always have to inlcude
// this dummy entry point so that the linker can resolve the exported
// SaveScreenBitmap function.  This will never be called when ifndef SSB
// is true.
//
UINT _loadds WINAPI SaveScreenBitmap(LPRECT lpRect, WORD wCommand)
{
    return (FAIL);
}

//===========================================================================
#endif // ifdef SSB
//===========================================================================

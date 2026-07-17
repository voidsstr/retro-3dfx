/* -*-c++-*- */
/* $Header: lock.c, 3, 10/11/00 8:54:41 PM, Brent$ */
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
** File name:   lock.c
**
** Description: Lock related functions.
**
** $Revision: 3$
** $Date: 10/11/00 8:54:41 PM$
**
** $History: lock.c $
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 8/19/99    Time: 12:23p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code so that hwcPageMappingFind will work on the Simulator
** 
** *****************  Version 5  *****************
** User: Peter        Date: 2/05/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** removed irritating trace message for opengl guys
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/11/99    Time: 3:53p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 3  *****************
** User: Peter        Date: 12/14/98   Time: 3:14p
** Updated in $/devel/h3/Win95/dx/minivdd
** removed vxdinline from copy_page_table which interferred w/
** optimization
** 
** *****************  Version 2  *****************
** User: Peter        Date: 12/08/98   Time: 9:18a
** Updated in $/devel/h3/Win95/dx/minivdd
** removed warning
** 
** *****************  Version 1  *****************
** User: Peter        Date: 12/08/98   Time: 9:15a
** Created in $/devel/h3/Win95/dx/minivdd
** vm lock stuff
** 
*/

#include <stddef.h>

#define WIN40SERVICES
#include "h3vdd.h"
#include "h3.h"

// Fix for building with Windows 98 DDK (VxDWraps already has defined)
#undef _CopyPageTable

#pragma VxD_LOCKED_DATA_SEG

/* Bookkeeping data.
 *
 *  lockedVM     - if non-zero the handle of the currently locked vm.
 *  systemVM     - system vm handle. This is only valid after the first
 *                 actual use of the locking api.
 *  lockApiInitP - non-zero if the api got its resources at init time
 */
FxU32 lockedVM;
static FxU32 systemVM;
static FxBool  lockApiInitP;

#pragma VxD_LOCKED_CODE_SEG

/* declspec(naked) prolog/epilog stuff ganked from ken w/ a little mod
 * to do the ebp stuff for you w/o polluting the mainline source w/
 * inline assembly cruft.
 */
#define NAKED_PROLOG_NOFLAGS(__vm) \
{ \
  __asm pushad \
  __asm mov ebp, esp \
  __asm sub esp, __LOCAL_SIZE \
  __asm mov __vm, ebx 

#define NAKED_EPILOG_NOFLAGS() \
  __asm mov esp, ebp \
  __asm popad \
  __asm ret \
}

#define NAKED_PROLOG(__vm, __clientRegs) \
{ \
  __asm mov edx, ebp \
  __asm pushfd \
  __asm pushad \
  __asm mov ebp, esp \
  __asm sub esp, __LOCAL_SIZE \
  __asm mov __vm, ebx \
  __asm mov __clientRegs, ebp

#define NAKED_EPILOG() \
  __asm mov esp, ebp \
  __asm popad \
  __asm popfd \
  __asm ret \
}


/*----------------------------------------------------------------------
Function name:  VDD_Get_VM_Info

Description:    Get the pointers for the CRTC and MemC
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void VXDINLINE
VDD_Get_VM_Info(HVM* crtc, HVM* memC)
{
  HVM
    tmpCrtc,
    tmpMemC;

  VxDCall(VDD_Get_VM_Info);
  __asm {
    mov tmpCrtc, edi;
    mov tmpMemC, esi;
  }

  *crtc = tmpCrtc;
  *memC = tmpMemC;
}


/*----------------------------------------------------------------------
Function name:  VMM_Validate_VM_Handle

Description:    
                
Information:

Return:         BOOL    1 for Success, 0 for Failure
----------------------------------------------------------------------*/
BOOL VXDINLINE
VMM_Validate_VM_Handle(HVM vm)
{
  BOOL retVal = 0;

  __asm mov ebx, vm
  VMMCall(Validate_VM_Handle);
  __asm {
    jc __errExit;
    mov retVal, 1;
  }
  
 __errExit:
  return retVal;
}


/*----------------------------------------------------------------------
Function name:  VMM_Map_Flat

Description:    Perform a 16:16 for flat conversion.
                
Information:

Return:         fxU32   Value of the flat selector or 0 for failure.
----------------------------------------------------------------------*/
FxU32 VXDINLINE
VMM_Map_Flat(FxU32 regSeg, FxU32 regOff)
{
  FxU32
    retVal = 0x00UL;
  
  __asm {
    mov ah, byte ptr regSeg;
    mov al, byte ptr regOff;
  }

  VMMCall(Map_Flat);

  __asm {
    cmp eax, -1;
    je __errExit;
    mov retVal, eax;
  }

 __errExit:
  return retVal;
}


/*----------------------------------------------------------------------
Function name:  VMM_System_Control

Description:    
                
Information:

Return:         FxBOOL  1 for success or FXFALSE for failure.
----------------------------------------------------------------------*/
FxBool VXDINLINE
VMM_System_Control(FxU32 controlMsg, 
                   HVM targetVM,
                   FxU32 vId,
                   FxU32 flags,
                   FxU32 assocVM)
{
  FxBool
    retVal = FXFALSE;

  __asm {
    mov eax, controlMsg;
    mov ebx, targetVM;
    mov edx, vId;
    mov esi, flags;
    mov edi, assocVM;
  }

  VMMCall(System_Control);

  __asm {
    jc __errExit;
    mov retVal, 1;
  }

 __errExit:
  return retVal;
}


/*----------------------------------------------------------------------
Function name:  VMM_Close_VM

Description:    
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void VXDINLINE
VMM_Close_VM(HVM vm,
             FxU32 timeout,
             FxU32 flags)
{
  __asm {
    mov eax, timeout;
    mov ebx, vm;
    mov ecx, flags;
  }

  VMMCall(Close_VM);
}


/*----------------------------------------------------------------------
Function name:  VMM_Copy_Page_Table

Description:    
                
Information:
    NB: This routine should not currently be set to be VXDINLINE
    because it thrashes all of the non-volatile registers. This causes
    a conflict between the inlining and optimization because the
    optimizer does not know and tries to save values in registers
    across an inlined function call. Nasty!

Return:         FxU32
----------------------------------------------------------------------*/
FxU32
VMM_Copy_Page_Table(FxU32 linearPage,
                    FxU32 pteBufCount,
                    FxU32* pteBuf)
{
  FxU32 
    retVal = 0x00UL;

  __asm {
    push dword ptr 0x00; /* flags */
    push pteBuf;
    push pteBufCount;
    push linearPage;
  }

  /* Returns non-zero in eax for success */
  VMMCall(_CopyPageTable);

  __asm {
    mov retVal, eax;
  }

  return retVal;
}


/*----------------------------------------------------------------------
Function name:  LockAPI_Dispatch

Description:    Allow dos programs to call into the display
                driver to get ioctl like functionality for getting
                'exclusive' access to the hw, but to try to allow
                for loss through user intervention.
Information:
    ebx:    Current VM Handle
    edi:    Current thread handle
    edx:    Reference data (driver description block)
    ebp:    Client register pointer block
        eax     - Return value buffer
        ebx     - Call type
            0 - is fullscreen vm
            1 - exclusive lock
            2 - shared lock
            3 - unlock vm

Return:         VOID
----------------------------------------------------------------------*/
_declspec(naked) void
LockAPI_Dispatch(void)
{
#define FN_NAME "LockAPI_Dispatch"
  FxU32
    curVM;
  CLIENT_STRUCT
    *clientRegs;

  NAKED_PROLOG(curVM, clientRegs)
  {
    FxU32
      retVal = 0x00UL;

    /* Default to failure, set carry flag */
    clientRegs->CRS.Client_EFlags |= 0x01UL;
    
    /* Sanity check state of the world */
    if (!lockApiInitP) goto __errExit;
    if (clientRegs->CRS.Client_EAX == 0x00UL) goto __errExit;

    if (systemVM == 0x00UL) systemVM = Get_Sys_VM_Handle();

    Debug_Printf("%s: VM(0x%X) FunctionSel(0x%X) ClientBuf(0x%X:0x%X)\n ", 
                 FN_NAME,
                 curVM,
                 clientRegs->CRS.Client_EBX,
                 clientRegs->CRS.Client_DS, clientRegs->CRS.Client_EAX);
    switch(clientRegs->CRS.Client_EBX) {
    case 0: /* fullscreenP */
      {
        HVM crtc, memc;

        /* VDD_Get_VM_INFO
         *   crtc owner
         *     system vm - desktop
         *     other     - fullscreen dos box
         *   memC owner
         *     other vm  - windowed dos box
         */
        VDD_Get_VM_Info(&crtc, &memc);
        retVal = (crtc == curVM);

        Debug_Printf("\tcrtc(0x%X) memc(0x%X)\n", crtc, memc);
      }
      break;

    case 1: /* exclusive lock */
      {
        /* Only one exclusive lock at a time */
        retVal = (lockedVM == 0x00UL);
        if (!retVal) goto __errExclusive;

        /* Is the current vm something that it makes sense to lock? */
        retVal = (VMM_Validate_VM_Handle(curVM) &&
                  (curVM != systemVM));
        if (!retVal) goto __errExclusive;

        /* Make the vm the current locked vm */
        lockedVM = curVM;

        Debug_Printf("\tlockedVM(0x%X) systemVM(0x%X)\n",
                     lockedVM, systemVM);
        
    __errExclusive:
        ;
      }
      break;

    case 2: /* shared lock */
      break;

    case 3: /* release lock */
      {
        /* Do we have a lock to relase? */
        retVal = (lockedVM != 0x00UL);
        if (!retVal) goto __errRelease;

        /* Revert to unlocked state */
        lockedVM = 0x00UL;

    __errRelease:
        ;
      }
      break;

    default:
      Debug_Printf("\tInvalid selector\n");
    }

    /* Write out return value to user buffer */
    {
      FxU32*
        userBuf = (FxU32*)VMM_Map_Flat(offsetof(CLIENT_STRUCT, CRS.Client_DS),
                                       offsetof(CLIENT_STRUCT, CRS.Client_EAX));

      if (userBuf == NULL) goto __errExit;
      *userBuf = retVal;

      /* We may have failed, but the call's results are valid */
      clientRegs->CRS.Client_EFlags &= ~0x01UL;

      Debug_Printf("\tretVal(0x%X)\n", retVal);
    }
    
 __errExit:
    ;
  }
  NAKED_EPILOG()

#undef FN_NAME
}


/*----------------------------------------------------------------------
Function name:  LockAPI_ClearLock

Description:    We cannot return to a locked vm from some other
                state that involved the 2d driver because both
                think that they have exclusive access to the hw.
                The 'solution' is to kill off the locked vm here
                before the 2d driver gets control back after the
                mini-vdd does the mode switch.  
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
LockAPI_ClearLock(void)
{
#define FN_NAME "LockAPI_ClearLock"
  FxU32
    curVM;

  Debug_Printf("%s: lockedVM(0x%X)\n", FN_NAME, lockedVM);

  /* Make sure that it really is still alive */
  if (lockedVM == 0x00UL) goto __errExit;
  if (!VMM_Validate_VM_Handle(lockedVM)) goto __errExit;

  /* Get the current vm w/ focus and see if the lockedVM has focus. If
   * so then we should switch in a different vm so killing this one
   * off does not leave the system in some whacky state.
   */
  curVM = Get_Execution_Focus();
  if (curVM == lockedVM) {
    Debug_Printf("\tFocusVM(0x%X)\n", curVM);

    /* Suspend the locked vm's execution just in case something goes
     * wonky when we change focus.  
     */
    VMM_System_Control(VM_SUSPEND,
                       lockedVM,
                       0,
                       0, 
                       0);

    /* Broadcast focus change to the system vm. */
    VMM_System_Control(SET_DEVICE_FOCUS,
                       systemVM,
                       0,
                       0, 
                       0);
  }

  /* Kill off the vm and reset the locked state */
  VMM_Close_VM(lockedVM, 
               0x5000UL,
               0x00UL);
  lockedVM = 0x00UL;

 __errExit:
  ;
#undef FN_NAME
}


/*----------------------------------------------------------------------
Function name:  LockAPI_Cleanup

Description:    If somehow the application quit w/o releasing the
                lock make sure that we cleanup for future calls.
Information:
    ebx:    VM handle terminating

Return:         VOID
----------------------------------------------------------------------*/
_declspec(naked) void 
LockAPI_Cleanup(void)
{
#define FN_NAME "LockAPI_Cleanup"
  FxU32
    curVM;
  CLIENT_STRUCT
    *clientRegs;

  NAKED_PROLOG(curVM, clientRegs);
  {
    if (curVM == lockedVM) lockedVM = 0x00UL;
  }
  NAKED_EPILOG();

  /* Always succeeds */
  __asm clc
#undef FN_NAME
}


/*----------------------------------------------------------------------
Function name:  LockAPI_ScreenSwitchP

Description:    Check to see if it is ok to switch back to hi-res
                mode from fullscreen vga. This is only used for
                alt-tab switches, and not for things like
                ctrl-alt-del which comes down through the 2d side at
                mode switch time.
Information:
    eax:     0 if not a VESA mode or -1 if it is a VESA mode.
    ebx:     VM Handle.
    ecx:     INT 10H mode number.  

Return:         VOID
----------------------------------------------------------------------*/
static _declspec(naked) void
LockAPI_ScreenSwitchP(void)
{
#define FN_NAME "LockAPI_ScreenSwitchP"
  HVM
    curVM;

  /* Pre-clear the carry flag since we're probably not going to be
   * in this mode a lot 
   */
  __asm clc;
  
  NAKED_PROLOG_NOFLAGS(curVM);
  {    
    /* Check to see if we have a locked vm */
    if (lockedVM == 0x00UL) goto __checkDone;
    if (curVM != lockedVM) goto __checkDone;
    
    /* Be paranoid and make sure that this is a valid vm before forcing
     * no switch to happen. We could not have 're-used' the vm handle
     * because vm termination should force the _lockedVM to be cleared
     * via a call to LockAPI_Cleanup 
     */
    if (!VMM_Validate_VM_Handle(lockedVM)) goto __checkDone;
    
    /* Set teh cary bit for no switch away. */
    __asm stc;
    
 __checkDone:
    ;
  }
  NAKED_EPILOG_NOFLAGS();
#undef FN_NAME
}


/*----------------------------------------------------------------------
Function name:  hwcPageMappingFind

Description:    
                
Information:

Return:         FxU32   The page mapping or 0 for failure.
----------------------------------------------------------------------*/
FxU32
hwcPageMappingFind(const FxU32 mapAddr,
                   const FxU32 remapAddr)
{
#define FN_NAME "hwcPageMappingFind"
  FxU32
    retVal = 0x00UL;
#ifndef WIN_CSIM
  FxU32
    pteMap,
    pteRemap;

  /* Get the associated physical address entries */
  if ((VMM_Copy_Page_Table(mapAddr >> 12, 1, &pteMap) == 0x00UL) ||
      (VMM_Copy_Page_Table(remapAddr >> 12, 1, &pteRemap) == 0x00UL)) goto __errExit;

  /* Make sure the physical pages exist */
  if (((pteMap & 0x01UL) == 0x00UL) ||
      ((pteRemap & 0x01UL) == 0x00UL)) goto __errExit;

  /* Extract only the physical address bits from the pte's,
   * and compute an offset.
   */
  pteMap &= ~0x0FFFUL;
  pteRemap &= ~0x0FFFUL;

  retVal = ((pteRemap - pteMap) + (remapAddr & 0x0FFFUL));
//  Debug_Printf("Remap %x %x Map %x %x ret %x\n", pteRemap, remapAddr, pteMap, mapAddr, retVal);
 __errExit:
#else
  retVal = remapAddr - (mapAddr & ~ 0x0FFFUL);
//  Debug_Printf("Remap %x Map %x ret %x\n", remapAddr, mapAddr, retVal);
#endif
  return retVal;
#undef FN_NAME
}


/*----------------------------------------------------------------------
Function name:  LockAPI_Init

Description:    Perform the setup.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
#pragma VxD_ICODE_SEG
/* h3vdd.c */
extern DWORD
VDD_Get_Mini_Dispatch_Table(DWORD **pDT);

void
LockAPI_Init(void)
{
#define FN_NAME "LockAPI_Init"
  PDWORD
    vddTable;        // VDD dispatch table
  DWORD 
    tableCount;

  /* Get the minivdd dispatch table to see if we can add any procs
   * that we need.
   */
  tableCount = VDD_Get_Mini_Dispatch_Table(&vddTable);
  lockApiInitP = (tableCount >= CHECK_SCREEN_SWITCH_OK);
  if (!lockApiInitP) goto __errExit;

  /* Setup state to check screen switches and no locked vm's */
  vddTable[CHECK_SCREEN_SWITCH_OK] = (DWORD)LockAPI_ScreenSwitchP;
  lockedVM = 0x00UL;
  systemVM = 0x00UL;

 __errExit:
  Debug_Printf("%s: lockApiInitP(0x%X)\n", FN_NAME, lockApiInitP);
#undef FN_NAME
}

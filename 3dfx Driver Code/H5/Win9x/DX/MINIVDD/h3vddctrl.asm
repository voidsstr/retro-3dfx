;/* $Header: /devel/h3/win95/dx/minivdd/h3vddctrl.asm 16    4/09/99 2:24p Stb_bseitsin $ */
;/*
;** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
;** All Rights Reserved.
;**
;** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
;** the contents of this file may not be disclosed to third parties, copied or
;** duplicated in any form, in whole or in part, without the prior written
;** permission of 3Dfx Interactive, Inc.
;**
;** RESTRICTED RIGHTS LEGEND:
;** Use, duplication or disclosure by the Government is subject to restrictions
;** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
;** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
;** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
;** rights reserved under the Copyright Laws of the United States.
;**
;** File name:   h3vddctrl.asm
;**
;** Description: Mini-VDD Support Functions.
;**
;** $Revision: 16 $
;** $Date: 4/09/99 2:24p $
;**
;** $History: h3vddctrl.asm $
;; 
;; *****************  Version 16  *****************
;; User: Stb_bseitsin Date: 4/09/99    Time: 2:24p
;; Updated in $/devel/h3/win95/dx/minivdd
;; Fix elseif H4 on line 141.
;; 
;; *****************  Version 15  *****************
;; User: Stb_bseitsin Date: 4/09/99    Time: 12:39p
;; Updated in $/devel/h3/win95/dx/minivdd
;; Added Napalm registers. Added ifdef H5.
;; 
;; *****************  Version 14  *****************
;; User: Andrew       Date: 3/10/99    Time: 4:30p
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Changed Virtual Device Name
;; 
;; *****************  Version 13  *****************
;; User: Andrew       Date: 2/10/99    Time: 9:09a
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Added WIN40COMPAT and WIN41SERVICES to work with Win '98 vmm.h
;; 
;; *****************  Version 12  *****************
;; User: Stb_srogers  Date: 1/29/99    Time: 8:06a
;; Updated in $/devel/h3/win95/dx/minivdd
;; 
;; *****************  Version 11  *****************
;; User: Andrew       Date: 1/21/99    Time: 5:50p
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Added new Control_Dispatch
;; 
;; *****************  Version 10  *****************
;; User: Michael      Date: 1/08/99    Time: 1:50p
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Implement the 3Dfx/STB unified header.
;; 
;; *****************  Version 9  *****************
;; User: Peter        Date: 12/08/98   Time: 9:16a
;; Updated in $/devel/h3/Win95/dx/minivdd
;; c-ified video memory fifo support stuff
;; 
;; *****************  Version 8  *****************
;; User: Peter        Date: 11/30/98   Time: 6:48p
;; Updated in $/devel/h3/Win95/dx/minivdd
;; query for possibly re-mapped base address
;; 
;; *****************  Version 7  *****************
;; User: Andrew       Date: 10/05/98   Time: 4:02p
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Added a check for the return code so that we can work in multi-monitor
;; 
;; *****************  Version 6  *****************
;; User: Andrew       Date: 4/28/98    Time: 2:36p
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Fixed bug with carry flag set incorrectly for MiniVDD_HookInt10.
;; 
;; *****************  Version 5  *****************
;; User: Andrew       Date: 4/27/98    Time: 7:06a
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Added a function to do a int 10 to force VDD to do int 10 ax=0x3 at a
;; mroe opportune time.
;; 
;; *****************  Version 4  *****************
;; User: Ken          Date: 4/15/98    Time: 6:41p
;; Updated in $/devel/h3/win95/dx/minivdd
;; added unified header to all files, with revision, etc. info in it
;**
;*/
        
;****************************************************************************
; H3CTRL.ASM
;----------------------------------------------------------------------------
;
; Banshee Mini-VDD Support Functions C Wrapper
;
;****************************************************************************

title           H3VDD Mini-VDD Support Functions
.386p

.xlist
MINIVDD  EQU 1
WIN40COMPAT EQU 1
WIN41SERVICES EQU 1
include         VMM.INC
include    	VMMREG.INC
include 	VWIN32.INC
include         MINIVDD.INC
.list

;; lock.c 
EXTRN        _LockAPI_Dispatch:  NEAR
EXTRN        _LockAPI_Cleanup:   NEAR

;; kmvt.asm
;extrn       GetDDHAL:proc
;extrn       _GetKernelInfo:proc

VxD_DATA_SEG
; DEV CONTROL Vars
DevCtlRet             dd 0
VxD_DATA_ENDS

;       Virtual Device Declaration
;
; For Napalm we'll want to change H4VDD to H5VDD,  but I'm
; leaving it along for now, since I don't understand
; what the impact would be to change it to H5VDD. Also, remember
; to change h3vdd.def.
; Bob Seitsinger - 4/7/99
; Today the day Bob -- APS 5/11/00
;
ifdef H5
        Declare_Virtual_Device  H5VDD,               \
                                3,                   \
                                1,                   \
                                MiniVDD_Control,     \
                                Undefined_Device_ID, \
                                VDD_Init_Order,      \
                                , \
                                _LockAPI_Dispatch,   \
                                ,
else
ifdef H4
        Declare_Virtual_Device  H4VDD,               \
                                3,                   \
                                1,                   \
                                MiniVDD_Control,     \
                                Undefined_Device_ID, \
                                VDD_Init_Order,      \
                                , \
                                _LockAPI_Dispatch,   \
                                ,
else
        Declare_Virtual_Device  H3VDD,               \
                                3,                   \
                                1,                   \
                                MiniVDD_Control,     \
                                Undefined_Device_ID, \
                                VDD_Init_Order,      \
                                , \
                                _LockAPI_Dispatch,   \
                                ,
endif
endif

VxD_LOCKED_CODE_SEG
;
;
Begin_Control_Dispatch  MiniVDD
        Control_Dispatch Sys_Dynamic_Device_Init,   MiniVDD_Dynamic_Init,sCall,<ebx>
        Control_Dispatch Device_Init,               MiniVDD_Dynamic_Init,sCall,<ebx>
        Control_Dispatch System_Exit,               MiniVDD_System_Exit,sCall
        
        Control_Dispatch Debug_Query,               OnDebugQuery, sCall

        Control_Dispatch Sys_VM_Terminate,          MiniVDD_Sys_VM_Terminate,sCall
        Control_Dispatch PNP_NEW_DEVNODE,           MiniVDD_PNP_NEW_DEVNODE
        Control_Dispatch W32_DeviceIOControl        MiniVDD_W32_DevIoCtl,sCall, <esi>
        Control_Dispatch VM_Terminate               _LockAPI_Cleanup
End_Control_Dispatch MiniVDD



PNP_NewDevNode  PROTO NEAR C :DWORD,:DWORD,:DWORD


;*----------------------------------------------------------------------
;Function name:  MiniVDD_PNP_NEW_DEVNODE
;
;Description:    A new PNP Dev Node.
;
;Information:
;
;Return:         DWORD   0 is always returned.
;----------------------------------------------------------------------*
public  MiniVDD_PNP_NEW_DEVNODE
BeginProc MiniVDD_PNP_NEW_DEVNODE, RARE

    push    ebx
    push    ebp
    push    edi
    push    esi

    invoke  PNP_NewDevNode, eax, ebx, edx

    ; Right now we don't care about return value so say we are always successful
    xor     eax, eax

    clc
    pop     esi
    pop     edi
    pop     ebp
    pop     ebx

    ret                                     
EndProc MiniVDD_PNP_NEW_DEVNODE


;*----------------------------------------------------------------------
;Function name:  _HookInt10
;
;Description:    This routine is to get into the Int10 chain so
;                that we can catch the DDC calls at the start of
;                Windows '95.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
public  _HookInt10
_HookInt10 PROC
        mov     eax, 10h
        mov     esi, OFFSET32 MiniVDD_Int10
        VMMCall Hook_V86_Int_Chain
        ret
_HookInt10 ENDP


;*----------------------------------------------------------------------
;Function name:  MiniVDD_Int10
;
;Description:    
;
;Information:
; On Entry: eax = int number (should be 10h)
;           ebx = current Virtual Machine handle
;           ebp = pointer to client register structure
;
;Return:         CF cleared if routine handled the int10h request.
;----------------------------------------------------------------------*
VESASupport PROTO NEAR C
public MiniVDD_Int10
BeginProc MiniVDD_Int10, DOSVM

    pushad
    cmp     eax,10h                 ; Handle an Int 10h?
    jne     I10_PassIt              ; No
                                    ; Yes
    mov     eax, [ebp].Client_EAX
    cmp     ax,4F15h                ; Is it a VESA VBE/DDC call?
    je      I10_HandleIt            ; Yes
                                    ; No
I10_PassIt:
    popad
    stc                             ; Signal request not handled
    ret

I10_HandleIt:                       ; Handle the request here.
    invoke  VESASupport
    jnc I10_PassIt                  ; VESA Support is opposite Int10 wrt carry flag    
    popad
    clc
    ret

EndProc MiniVDD_Int10


;*----------------------------------------------------------------------
;Function name:  _HelpVDD
;
;Description:    The VDD has a variable called Vid_Flags.  The
;                first time the VDD_Int_10 hook flag is called
;                it wants to set mode 3.  I don't know why it
;                wants to do this.  To make sure that this happens
;                at a opportune time I call this function once 
;                on the first mode set.  It calls the bios using
;                function ax=1400 which for us is OEMExtensions1
;                which based on the version V210D is nothing.
;
;Information:
;
;Return:         CF cleared if routine handled the int10h request.
;----------------------------------------------------------------------*
        public _HelpVDD
BeginProc _HelpVDD
        push    ebx                             ; Save ebx
        push    ebp                             ; Save ebp
        mov     ebp, [esp+0ch]                  ; Get Client Structure
        mov     ebx, [esp+10h]                  ; Current vm
        pushad
        Push_Client_State
        VMMcall Begin_Nest_V86_Exec             ; 
        mov     [ebp.Client_AX], 1400h          ; Benign Function 
        mov     eax,10h
        VMMcall Exec_Int
        VMMcall End_Nest_Exec                   ; All done with software ints
        Pop_Client_State
        popad
        pop     ebp                             ; Restore ebp
        pop     ebx                             ; Restore ebx
        ret
EndProc _HelpVDD

VxD_LOCKED_CODE_ENDS

 end

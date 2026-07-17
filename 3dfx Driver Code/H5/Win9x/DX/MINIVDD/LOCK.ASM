; Copyright (c) 1995, 3Dfx Interactive, Inc.
; All Rights Reserved.
;
; This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
; the contents of this file may not be disclosed to third parties, copied or
; duplicated in any form, in whole or in part, without the prior written
; permission of 3Dfx Interactive, Inc.
;
; RESTRICTED RIGHTS LEGEND:
; Use, duplication or disclosure by the Government is subject to restrictions
; as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
; and Computer Software clause at DFARS 252.227-7013, and/or in similar or
; successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
; rights reserved under the Copyright Laws of the United States.
;
; $Revision: 1 $
; $Date: 1/29/99 9:13a $
;
; $History: lock.asm $
;; 
;; *****************  Version 1  *****************
;; User: Stb_srogers  Date: 1/29/99    Time: 9:13a
;; Created in $/devel/h3/win95/dx/minivdd
;; 
;; *****************  Version 1  *****************
;; User: Peter        Date: 11/30/98   Time: 6:47p
;; Created in $/devel/h3/Win95/dx/minivdd
;; dos locked vm stuff

title lock.asm
.386p

.xlist
include         VMM.INC
include         debug.inc
include         MINIVDD.INC
.list

VxD_DATA_SEG
    ;; Exclusive locked vm
    public _lockedVM
    _lockedVM               dd 0

    ;; Cached system vm handle so that we don't have
    ;; to get this in each hook proc.
    systemVM                dd 0

    ;; Could we install our hook for checking screen switches?
    ;; This is necessary for the 'exclusive' lock, and we
    ;; will fail opens w/o it.
    lockAPIInitP            dd 0

    pteBuf                  dd 0
VxD_DATA_ENDS        

VxD_CODE_SEG

BeginProc       _MiniVDD_LockAPI_Init
    pushad

    ;; Hook in our mini-vdd proc to the main windows vdd
    ;;  ecx:    # of mini-vdd hookable functions
    ;;  edi:    linear address of dispatch table
    VxDCall VDD_Get_Mini_Dispatch_Table

    ;; Check to make sure the screen switch proc exists in the table.
    ;; This assumes that entries never go away in the middle
    cmp     ecx, CHECK_SCREEN_SWITCH_OK
    jl      initDone

    ;; Install our function for checking to see if it
    ;; is ok to do an alt-tab or alt-enter
    MiniVDDDispatch CHECK_SCREEN_SWITCH_OK, LockAPI_ScreenSwitchP

    ;; Zero systemVM and lockedVM handles for paranoia's sake
    mov         dword ptr [_lockedVM], 0
    mov         dword ptr [systemVM], 0

    ;; We're good to go.
    mov         dword ptr [lockAPIInitP], 1

initDone:
    popad    
    ret
EndProc         _MiniVDD_LockAPI_Init
    
    ;; Allow dos programs to call into the display driver to
    ;; get ioctl like functionality.
    ;;
    ;;  ebx:    Current VM Handle
    ;;  edi:    Current thread handle
    ;;  edx:    Reference data (driver description block)
    ;;  ebp:    Client register pointer block
    ;;      eax     - Return value buffer
    ;;      ebx     - Call type
    ;;          0 - is fullscreen vm
    ;;          1 - exclusive lock
    ;;          2 - shared lock
    ;;          3 - unlock vm
    ;;      ecx:edx - Far return address (cs:eip)
    
BeginProc MiniVDD_LockAPI_Dispatch
    pushad

    Trace_Out "MiniVDD_LockAPI_Dispatch: VM:#ebx Thread:#edi"

    ;; Default to failure
    mov     ecx, [ebp.Client_EAX]
    cmp     ecx, 0
    jz      dispatchDone
    mov     dword ptr [ecx], 0

    ;; Initted api?
    cmp     [lockAPIInitP], 0
    je      dispatchDone

    ;; Check the operation type
    mov     eax, [ebp.Client_EBX]
    cmp     eax, numCommands
    jge     dispatchDone

    ;; Dispatch to the handler
    lea     eax, dword ptr [commandTable + eax * 4]
    jmp     dword ptr [eax]

commandTable label dword
    dd offset32 fullscreenP
    dd offset32 lockExclusive
    dd offset32 lockShared
    dd offset32 releaseVM
numCommands equ (($-commandTable) / 4)    

fullscreenP:
    Trace_Out " fullscreenP"

    ;; returns  edi: crtc owner
    ;;              system vm - desktop
    ;;              other     - fullscreen dos box
    ;;          esi: memC owner
    ;;              other vm  - windowed dos box
    VxDcall VDD_Get_VM_Info

    ;; Is the current vm the owner of crtc?
    cmp     ebx, edi
    je      dispatchSuccess
    jmp     dispatchDone
    
    ;;  Exclusive:  vm changes are disallowed, and changes
    ;;              are checked at runtime. alt-tab changes
    ;;              are not allowed. ctrl-alt-del is allowed,
    ;;              but causes the vm to be nuked since we
    ;;              are forcing it off the hw.
lockExclusive:
    Trace_Out " lockExclusive"
    ;; Only one exclusive lock at a time (duh!)
    cmp     [_lockedVM], 0
    jne     dispatchDone

    ;; Be paranoid and make sure that this is a valid vm
    VMMcall Validate_VM_Handle
    jc      dispatchDone

    ;; Does it make sense to lock the system vm?
    ;; I don't think so, but am probably missing something.
    cmp     ebx, [systemVM]
    je      dispatchDone    

    ;; VM locked state
    mov     dword ptr [_lockedVM], ebx

    ;; Do we have a valid cached systemVM handle?
    cmp     [systemVM], 0
    jne     dispatchSuccess

    VMMcall Get_Sys_VM_Handle
    mov     dword ptr [systemVM], ebx
    
    jmp     dispatchSuccess

    ;;  Shared:     vm changes may cause the hw to be from 
    ;;              teh application and it needs to query 
    ;;              that the context is still valid.
    ;; 
    ;; NB: Not yet supported for pm apps.
lockShared:
    Trace_Out " lockShared"
    jmp     dispatchDone
    
releaseVM:
    Trace_Out " releaseVM"
    
    ;; Is there a lock?
    cmp     [_lockedVM], 0
    je      dispatchDone

    ;; Clear the lock flag
    mov     dword ptr [_lockedVM], 0
    jmp     dispatchSuccess

dispatchSuccess:
    Trace_Out " Success"
        
    mov     eax, [ebp.Client_EAX]
    mov     dword ptr [eax], 1

dispatchDone:
IFDEF DEBUG
    xor     eax, eax
    mov     ax, word ptr [ebp.Client_CS]
    mov     ebx, [ebp.Client_EIP]
    
    Trace_Out " Client call addr(cs:eip): #eax:#ebx"
ENDIF ; DEBUG
    
IF 0                               
    ;; It seems that the far call through esp interacts badly
    ;; with something in the os's mechanism to get here so we
    ;; force teh return to the caller by getting teh far return
    ;; address from the client's stack
    mov     eax, [ebp.Client_ECX]
    mov     [ebp.Client_CS], ax
    
    mov     ebx, [ebp.Client_EDX]
    mov     [ebp.Client_EIP], ebx

    Trace_Out " Client passed return addr(cs:eip): #eax:#ebx"
    
    ;; Pop the far return address
    add     [ebp.Client_ESP], 8
ENDIF

IFDEF DEBUG
    ;; Client far return addr
    mov     eax, [ebp.Client_ESP]
    xor     ebx, ebx
    mov     bx, word ptr [ebp.Client_SS]

    xor     ecx, ecx
    mov     cx, ds
    mov     ds, bx
    mov     ebx, dword ptr [eax]
    mov     edx, dword ptr [eax + 4]
    mov     ds, cx

    Trace_Out " Client Return Addr(cs:eip): #ebx:#edx"
ENDIF ; DEBUG    

    popad
    ret
EndProc MiniVDD_LockAPI_Dispatch

    ;; We cannot return to a locked vm from some other
    ;; state that involved the 2d driver because both 
    ;; think that they have exclusive access to the hw
    ;; The 'solution' is to kill off the locked vm here
    ;; before the 2d driver gets control back after the
    ;; mini-vdd does the mode switch for it.
BeginProc _MiniVDD_LockAPI_ClearLock
    pushad

    Trace_Out "MiniVDD_LockAPI_ClearLock"
    
    ;; Get the current locked vm
    mov     edi, [_lockedVM]    
    cmp     edi, 0
    je      clearDone

    ;; Be paranoid and make sure that this is a valid vm
    mov     ebx, edi
    VMMcall Validate_VM_Handle
    jc      clearDone    

    ;; Get the current vm w/ focus and see if
    ;; the lockedVM has focus. If so then we should
    ;; switch in a different vm so killing this one off
    ;; does not strand device focus.
    VMMcall Get_Execution_Focus
    cmp     edi, ebx
    jne     clearCloseVM

    ;; Suspend the locked vm's execution just in case
    ;; something goes wonky when we change focus.
    mov     eax, VM_Suspend
    mov     ebx, edi
    VMMcall System_Control

    ;; Broadcast focus change to the system vm.
    mov     eax, Set_Device_Focus   ; Control message type
    mov     ebx, [systemVM]         ; VM to get focus
    xor     edx, edx                ; VID - 0 is broadcast
    xor     esi, esi                ; flags
    xor     edi, edi                ; assocVMHandle (not needed if flags == 0)
    VMMcall System_Control

    ;;  eax:    TimeOut (ms)
    ;;  ebx:    vm handle
    ;;  ecx:    Flags (No continue)
clearCloseVM:       
    Trace_Out " Close_VM: #eax"
    mov     ebx, [_lockedVM]
    mov     eax, 5000h
    xor     ecx, ecx
    VMMcall Close_VM

clearDone:    
    ;; Always clear the locked state just in case.
    mov     dword ptr [_lockedVM], 0

    popad
    return
EndProc _MiniVDD_LockAPI_ClearLock

    ;; If somehow the application quit w/o releasing the
    ;; lock make sure that we cleanup for future calls.
    ;;      ebx:    VM handle terminating
BeginProc MiniVDD_LockAPI_Cleanup
    ;; Pre-clear carry flag for success
    clc

    ;; Get the current locked vm
    mov     eax, [_lockedVM]
    cmp     eax, 0
    je      cleanupDone

    ;; Does it match the current locked vm?
    cmp     eax, ebx
    je      cleanupDone

    ;; Clear the locked state
    mov     dword ptr [_lockedVM], 0

cleanupDone:
    return
EndProc MiniVDD_LockAPI_Cleanup

    ;; Check to see if it is ok to switch back to hi-res
    ;; mode from fullscreen vga. This is only used for 
    ;; alt-tab switches, and not for ctrl-alt-del which
    ;; comes down through the 2d side at mode switch time.
    ;; 
    ;;      eax:     0 if not a VESA mode or -1 if it is a VESA mode.
    ;;      ebx:     VM Handle.
    ;;      ecx:     INT 10H mode number.    
BeginProc   MiniVDD_LockAPI_ScreenSwitchP
    push    eax
    push    ebx
    
    ;; Pre-clear the carry flag since we're probably not
    ;; going to be in this mode a lot
    clc
    
    ;; Check to see if we have a locked vm
    mov     eax, [_lockedVM]
    cmp     eax, 0
    je      checkDone

    ;; Check the vm. 
    cmp     ebx, eax
    jne     checkDone

    ;; Be paranoid and make sure that this is a valid vm
    ;; before forcing no switch to happen. We could not
    ;; have 're-used' the vm handle because vm termination
    ;; should force the _lockedVM to be cleared via a call
    ;; to MiniVDD_LockAPI_Cleanup
    mov     ebx, eax
    VMMcall Validate_VM_Handle
    jc      checkDone    

    ;; Set teh cary bit for no switch away.
    stc

checkDone:
    pop     ebx
    pop     eax
    return 
EndProc     MiniVDD_LockAPI_ScreenSwitchP

STACKOFF    =   16
_remapAddr$ =   4 + STACKOFF        
_mapAddr$   =   8 + STACKOFF    
    
BeginProc _hwcPageMappingFind
    push    ebx
    push    ebp
    push    edi
    push    esi

    ;; Get parameters
    ;;  ebx:    current (possibly re-mapped) linear address
    ;;  esi:    target linear address    
    mov     ebx, dword ptr [esp + _mapAddr$]
    mov     esi, dword ptr [esp + _remapAddr$]

    ;; Get the physical address of the known linear address
    shr     esi, 12
    lea     ecx, [pteBuf]

    ;; Get page table entries
    ;;  esi:    Page #
    ;;    1:    # of pte's to copy
    ;;  ecx:    Address of buffer to hold entries
    ;;    0:    Flags (always 0)
    VMMcall _CopyPageTable <esi, 1, ecx, 0>

    ;; Valid pte? Bad things are afoot if not
    cmp     eax, 0
    je      __pageDone

    ;; Build physical address mask
    mov     ebp, dword ptr -1
    shr     ebp, 12
    shl     ebp, 12

    ;; esi: Target physical address
    mov     esi, dword ptr [pteBuf]
    and     esi, ebp

    ;; Are we into the current vm range area?
    ;; The physical address is not going to be mapped here
    cmp     ebx, 10F000h
    je      __pageDone

    ;; Get linear page num from linear page address
    mov     ecx, ebx
    shr     ecx, 12

    ;; Get address of return value buffer for 1 pte
    lea     edi, [pteBuf]

    ;; Get one pte
    VMMcall _CopyPageTable <ecx, 1, edi, 0>

    ;; If the call failed then fail the entire call because
    ;; these pages should be resident and valid.
    cmp     eax, 0
    je      __pageDone

    ;; Check the pte address
    mov     edx, dword ptr [pteBuf]

    ;; Is the physical page present?
    ;; Our hw pages should always be present, but
    ;; best to be paranoid.
    test    edx, 1h
    jz      __pageDone

    and     edx, ebp            ; Target's physical page address
    sub     edx, esi            ; Tage offset from 'base'
    and     ebx, 0FFFh          ; Offset within target page
    add     ebx, edx            ; Offset from 'base'

    jmp     __pageSuccess

    ;; Failure. Return NULL physical address
__pageDone:
    xor     ebx, ebx

    ;; ebx: Linear address of target physical address base
__pageSuccess:
    mov     eax, ebx

    pop     esi
    pop     edi
    pop     ebp
    pop     ebx
    
    ret
EndProc _hwcPageMappingFind

VxD_CODE_ENDS
END    
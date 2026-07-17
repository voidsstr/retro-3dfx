;/*
;** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
;** $Revision: 2 $
;** $Date: 3/30/99 4:10p $
;*/

         name  P6Stuff

        .586p
_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

    hasMSR        dd 0    

    ;; Detect whether we're running on a cpu w/ msr's/. If so then we
    ;; expose an api for letting user level clients set them at will.
    ;; Mostly this is used for setting mtrr's for things like uswc.
    ;;
    ;; First, determine whether CPUID instruction is available
    ;;    [look to see if bit 21 of EFLAGS can be toggled]
    ;; If not, punt.
    ;; If CPUID is available,
    ;;       Check to see if intel processor.
    ;;       Check to see if version == 6. 
    ;;       Do P6 specific stuff.
    PUBLIC _CheckForMSR@0
_CheckForMSR@0 PROC NEAR
    pushad                            ; save all regs.
    
    ; First, determine whether CPUID instruction is available.
    pushfd                            ; push original EFLAGS.
    pop     eax                       ; pop into eax
    mov     ecx, eax                  ; save original EFLAGS in ecx
    xor     eax, 0200000h             ; flip ID bit in EFLAGS
    push    eax                       ; put it back on stack
    popfd                             ; pop into EFLAGS
    pushfd                            ; get EFLAGS back
    pop     eax                       ; into eax
    xor     eax, ecx                  ; check to see if we could toggle ID
    jz      noMSR                     ; Sorry, not P5 or P6.

    ; Now determine whether it's an intel CPU.
    xor     eax, eax                  ; eax = 0.
    cpuid                             ; get cpuid
    xor     ebx, 0756e6547h           ; "Genu"
    jnz     notIntel                  ; 
    xor     edx, 049656e69h           ; "ineI"
    jnz     notIntel                  ; 
    xor     ecx, 06c65746eh           ; "ntel"
    jnz     notIntel
    jmp     intelP6

NotIntel:
    ;; This is a non-Intel processor. Figure out whether it supports
    ;; msr's from the cpuid bit. It is up to the client to know how
    ;; to program these correctly so checking for mtrr support is not
    ;; necessary here.
    xor     esi, esi     ; default feature flags
    xor     edi, edi     ; default extended feature flags

    ;; execute standard feature function
    mov     eax, 1
    cpuid
    mov     esi, edx

    test    esi, 20h                     ; rdmsr/wrmsr support (bit 5)
    jz      noMSR
    jmp     noCheckMTRR

intelP6:        
    ; Intel processor, can we ask for model number?
    cmp     eax, 1
    jl      noMSR                     ; can't ask for version! must not be P6

    ; Yes, you may ask for version number, verify family >= 6.
    mov     eax, 1
    cpuid                             ; get family/model/stepping
    shr     eax, 8                    ; rid of model & stepping number
    and     eax, 0fh                  ; use only family
    cmp     eax, 6                    ; 
    jl      noMSR

    ; Intel P6 processor. 
    ; Make sure it supports Memory Type Range Request registers
    test    edx, 01000h               ; bit 12 in edx = MTRR support
    jz      noMSR                     ; not supported!
    test    edx, 00020h               ; bit 5, supports MSR?
    jz      noMSR                     ; rdmsr, wrmsr not supported!
    
noCheckMTRR:    
    mov     hasMSR, 1h                  ; Tell the code we have a P6   
    jmp     doneCheck

noMSR:
    mov     hasMSR, 0h                  ; Tell the code we have a P6

doneCheck:
    popad                               ; Restore user regs before
    mov     eax, hasMSR                 ; setting the return value.
    ret
_CheckForMSR@0 ENDP    
    
;;;----------------------------------------------------------------------------
;;;  ECX = 6 (GETMSR)
;;; 
;;;  Input:
;;;    DWORD 0:    MSR Number
;;;    DWORD 1:    irrelavent
;;;    DWORD 2:    irrelavent
;;;  Output:
;;;    DWORD 0:    MSR Number if success; 0FFFh if failure
;;;    DWORD 1:    Lo 32 bits (EAX after rdmsr)
;;;    DWORD 2:    Hi 32 bits (EDX after rdmsr)
    PUBLIC  _GetMSR@4
_GetMSR@4 PROC NEAR

    push    ebx

    ;; First check to see if we're on a P6
    mov     eax, hasMSR
    test    eax, eax
    jz      getFail
   
    ;; Get the client buffer
    mov     ebx, DWORD PTR [esp + 8] ; buffer

    ;; Get the MSR
    ;; First, clear eax and edx;
    xor     eax, eax
    xor     edx, edx   

    mov     ecx, DWORD PTR [ebx]            ; rdmsr expects MSR in ecx
    rdmsr                             ; Read the MSR
          
    ;; Save output data - remember we already have the output data
    ;; pointer in ebx.
    mov     [ebx], ecx   
    mov     [ebx + 4], eax            ; Store lo 32 bits in lo DWORD
    mov     [ebx + 8], edx            ; Store hi 32 bits in hi DWORD

    ;; Success is non-zero and the msr # is good.
    mov     eax, ecx

getFail:
    pop     ebx
    ret     4
_GetMSR@4 ENDP    

;;;----------------------------------------------------------------------------
;;;  ECX = 7 (SETMSR)
;;;
;;;  Input:
;;;    DWORD 0:    MSR Number 
;;;    DWORD 1:    Lo 32 bits (EAX after rdmsr)
;;;    DWORD 2:    Hi 32 bits (EDX after rdmsr)
;;;  Output:
;;;    DWORD 0:    MSR Number if success; FFFh if failure
;;;    DWORD 1:    irrelavent
;;;    DWORD 2:    irrelavent
    PUBLIC _SetMSR@4
_SetMSR@4 PROC NEAR

    push    ebx
   
    ;; First check to see if we're on a P6
    mov     eax, hasMSR
    test    eax, eax
    jz      setFail

    mov     ebx, DWORD PTR [esp + 8]
    
    ;; Set the MSR
    ;;
    ;; At this point, ebx contains the buffer specifying the msr
    mov     ecx, DWORD PTR [ebx]            ; rdmsr expects MSR in ecx
    mov     eax, DWORD PTR [ebx + 4]        ; Load the lo DWORD
    mov     edx, DWORD PTR [ebx + 8]        ; load the hi DWORD

    wrmsr

    ;; Success, use msr # again
    mov     eax, ecx
       
setFail:        
    pop     ebx
    ret     4
_SetMSR@4 ENDP
        
;******************************************************************************
_TEXT   ends
        end

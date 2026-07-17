;/* $Header: /devel/h5/WinNT/Src/Video/Miniport/h5/i386/p6test.asm 1     9/01/99 11:26a Doconnell $ */
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
;** File name:   p6test.asm
;**
;** Description: Determines whether the OS & CPU support KNI instructions.
;**
;** $Revision: 1 $
;** $Date: 9/01/99 11:26a $


;*----------------------------------------------------------------------
;Function name:  _TestForP6
;
;Description:    Detect whether we're running on the P6. If so,
;                write privileged registers to map the 16MB
;                address space as USWC, and the first 4K as UC.
;Information:
; We can detect a P6 using the following mechanism:
;   First, determine whether CPUID instruction is available
;       [look to see if bit 21 of EFLAGS can be toggled]
;   If not, punt.
;   If CPUID is available,
;       Check to see if intel processor.
;       Check to see if version == 6. 
;
;   If not an Intel CPU check to see if it is an AMD that supports MTRR's
; 
; no registers are changed!
;
;Return:
;
; _isP6 = 
;
;	0 - MTRR's not supported
;	1 - Intel CPU, P6 or higher
;            for intel p6, 0x80000001 means full CPU, OS support for katmai
;	4 - AMD K6-style MTRRs
;	8 - PentiumII-style MTRRs (K7)
;
;----------------------------------------------------------------------*



TITLE           p6test.asm

                .586P
                .model FLAT

EXTRN           _isP6:DWORD

_TEXT           SEGMENT
                .code

KNI_SUPPORT     equ 080000000h


PUBLIC _TestForP6
;
_TestForP6	proc

   pushad                            ; save all regs.
;	   Trace_Out "FXMEMMAP: Identifying presence of P6"
    
   ;
   ; First, determine whether CPUID instruction is available.
   ;
;   Trace_Out "FXMEMMAP: Is CPUID available?"
   pushfd                            ; push original EFLAGS.
   pop     eax                       ; pop into eax
   mov     ecx, eax                  ; save original EFLAGS in ecx
   xor     eax, 0200000h             ; flip ID bit in EFLAGS
   push    eax                       ; put it back on stack
   popfd                             ; pop into EFLAGS
   pushfd                            ; get EFLAGS back
   pop     eax                       ; into eax
   xor     eax, ecx                  ; check to see if we could toggle ID
   jz      not_P6                    ; Sorry, not P5 or P6.

   ;
   ; Now determine whether it's an intel P6 CPU.
   ;

;   Trace_Out "FXMEMMAP: Is it an Intel CPU?"
   xor     eax, eax                  ; eax = 0.
   cpuid                             ; get cpuid
   xor     ebx, 0756e6547h           ; "Genu"
   jnz     not_Intel                 ; 
   xor     edx, 049656e69h           ; "ineI"
   jnz     not_Intel                 ; 
   xor     ecx, 06c65746eh           ; "ntel"
   jnz     not_Intel

   ;
   ; Intel processor, can we ask for model number?
   ;
;   Trace_Out "FXMEMMAP: May we ask for version number?"
   cmp     eax, 1
   jl      not_P6                    ; can't ask for version! must not be P6

   ;
   ; Yes, you may ask for version number, verify family==6.
   ;
;   Trace_Out "FXMEMMAP: Verifying architecture family"
   mov     eax, 1
   cpuid                             ; get family/model/stepping
   shr     eax, 8                    ; rid of model & stepping number
   and     eax, 0fh                  ; use only family
   cmp     eax, 6                    ; 
   jne     not_P6                    ; sorry, some other model

   ;
   ; Intel P6 processor. 
   ; Make sure it supports Memory Type Range Request registers
   ;
;   Trace_Out "FXMEMMAP: Does this P6 support MTRRs?"; 
   test    edx, 01000h               ; bit 12 in edx = MTRR support
   jz      not_P6                    ; not supported!
   test    edx, 00020h               ; bit 5, supports MSR?
   jz      not_P6                    ; rdmsr, wrmsr not supported!
    
   ; Intel P6 processor identified.
   ;
   ; Work around P6 write combining problem with certain intel chipsets.
   ; Check registry entry to see if we should bypass the fix code.
   ;
;   call    P6_CheckWcRegEntry        ; Check registry to find out
;   jc      DontCheckForBrokenChipset ; whether to bypass check and
;   call    P6_CheckForBrokenIntelChipset; fix for broken Intel chipset
    
DontCheckForBrokenChipset:           ; on a P6.

   mov     _isP6, 1h                  ; Tell the code we have a P6

   ; Now look to see if it's a Katmai
   ;
   test	   edx, 02000000h	     ; bit 25 in edx = full KNI support
   jz      no_Kni
   mov     eax, CR0
   test    eax, 04h                  ; bit 2 of CR0 is CR0.EM, emulation
   jnz     no_Kni
   mov     eax, CR4
   test    eax, 0200h                ; bit 9 of CR4 is CR4.OSFXSR, OS support
   jz      no_Kni                    ;    for task switching KNI state

   ; otherwise, we have KNI support
   ;
   mov     eax, KNI_SUPPORT
   or      _isP6, eax	
				
no_Kni:							
CheckForP6_Exit:
   popad
   ret

not_P6:
;   Trace_Out "FXMEMMAP: Sorry, not P6: #eax #ebx #ecx #edx"
   mov     _isP6, 0h                  ; Tell the code we have a P6
   popad
   ret


not_Intel:
    ;; This is a non-Intel processor. Figure out whether it supports
    ;; AMD K6 style MTRR's or K7 (PII) MTRR's
    ;;
    ;; 0004h   K6-style MTRRs
    ;; 0008h   PentiumII-style MTRRs

    xor esi, esi     ; default feature flags
    xor edi, edi     ; default extended feature flags

    ;; Test whether extended feature function is supported

    mov eax, 80000000h
    cpuid
    cmp eax, 80000000h
    jbe NoExtendedFeatures

    ;; execute extended feature function

    mov eax, 80000001h
    cpuid
    mov edi, edx

NoExtendedFeatures:

    ;; execute standard feature function

    mov eax, 1
    cpuid
    mov esi, edx
    mov ebp, eax           ; save family/model/stepping

    ;; get the vendor string 
 
    mov eax, 0
    cpuid
    
    ; PRS 4722 Cyrix 6x86 MX sets EAX to 1 here, which ends up going into _isP6
    ; which is not a good thing, therefore, clear eax to clear up the confusion
    xor eax, eax

ChkAMD:
    cmp ebx, 68747541h     ; 'htuA'
    jne UnknownVendor
    cmp edx, 69746E65h     ; 'itne'
    jne UnknownVendor
    cmp ecx, 444D4163h     ; 'DMAc'
    jne UnknownVendor 

CPUisAMD:
    xor  eax, eax 
    mov  edx, ebp          ; family/model/stepping information
    and  edx, 00000FFFh    ; extract family/model/stepping
    cmp  edx, 00000588h    ; CXT, Sharptooth, or K7 ?
    jb   AmdMTRRchkDone    ; nope, definitely no MTRRs
    cmp  edx, 00000600h    ; K7 or better ?
    jb   AmdHasK6MTRR      ; nope, but supports K6 MTRRs
    or   eax, 8            ; set P2_MTRR feature flag
    jmp  AmdMTRRchkDone    ;
AmdHasK6MTRR:
    or   eax, 4            ; set K6_MTRR feature flag
AmdMTRRchkDone:
IF 0
    test esi, 00800000h    ; check for MMX bit in features
    jz   DoneCpu
    or   eax, 1            ; set MMX feature flag
    test edi, 80000000h    ; check for 3DNow! bit in extended features
    jz   DoneCpu
    or   eax, 2            ; set 3DNow! feature flag
ENDIF
    jmp  DoneCpu

UnknownVendor:
	;; eax is 0.  Must be non Intel, non AMD.
DoneCpu:
    mov  _isP6, eax        ; Return CPU class
    jmp  CheckForP6_Exit

_TestForP6	endp

_TEXT   ENDS

END

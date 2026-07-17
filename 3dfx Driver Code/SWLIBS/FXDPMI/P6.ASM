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
;** $Date: 6/11/97 11:56p $
;*/

;// This VxD provides a DeviceIoControl interface to a subset of DPMI type
;// services in the Windows95 VMM because Int 31h DPMI calls do not work in 
;// Win32 apps/DLLs. The VWIN32.VXD allegedly provides routines for invoking
;// interrupt 31h but I could not find them documented in the DDK.

     name  P6Stuff
;******************************************************************************
;
; Detect whether we're running on the P6. If so, write privileged registers
; to map the 16MB address space as USWC, and the first 4K as UC.
;
;
; We can detect a P6 using the following mechanism:
;   First, determine whether CPUID instruction is available
;       [look to see if bit 21 of EFLAGS can be toggled]
;   If not, punt.
;   If CPUID is available,
;       Check to see if intel processor.
;       Check to see if version == 6. 
;
;   Do P6 specific stuff.
;
; Usage: P6Stuff    eax = physical address of Voodoo.
; 
; no registers are changed!
;
;******************************************************************************

.586p
_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'
ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING
    
dwp    equ     <dword ptr>  

; Arguments (STKOFF = 16 from 4 pushes)
MSRSTKOFF$ = 16
INS$       = 4 + MSRSTKOFF$
OUTS$      = 8 + MSRSTKOFF$

MSRNO      = 0      
MSRLO      = 4
MSRHI      = 8   
    
_DATA   SEGMENT
   isP6        dd  0                 ; Is it a P6?
   p6Called    dd  0                 ; Have we checked?
_DATA   ENDS

public  _P6Stuff
_P6Stuff proc
   mov eax, 4[esp]
   pushad                            ; save all regs.
   mov p6Called, 1  
   mov ebp, eax                      ; move to ebp, will be untouched there
   and ebp, 0fffff000h               ; page align.
   jz  not_P6                        ; Bad physical address (0!)

   ;; 
   ;; First, determine whether CPUID instruction is available.
   ;;
   pushfd                            ; push original EFLAGS.
   pop eax                           ; pop into eax
   mov ecx, eax                      ; save original EFLAGS in ecx
   xor eax, 0200000h                 ; flip ID bit in EFLAGS
   push    eax                       ; put it back on stack
   popfd                             ; pop into EFLAGS
   pushfd                            ; get EFLAGS back
   pop eax                           ; into eax
   xor eax, ecx                      ; check to see if we could toggle ID
   jz  not_P6                        ; Sorry, not P5 or P6.
    
   ;; 
   ;; Now determine whether it's an intel P6 CPU.
   ;; 
   xor eax, eax                      ; eax = 0.
   cpuid                             ; get cpuid
   xor ebx, 0756e6547h               ; "Genu"
   jnz not_P6          
   xor edx, 049656e69h               ; "ineI"
   jnz not_P6
   xor ecx, 06c65746eh               ; "ntel"
   jnz not_P6
    
is_P6:       
   popad
   mov eax, 01h
   ret
      
not_P6:
   popad
   mov eax, 00h
   ret
    
_P6Stuff endp

public _D32GetMSR    
_D32GetMSR proc
   ;; save ebx, esi, edi, ebp
   push    ebx
   push    esi

   push    edi
   push    ebp
   
   mov     eax, p6Called
   test    eax, eax
   jnz     pastP6Check1

   call    _P6Stuff                  ; Check to see if it's a p6    
    
pastP6Check1:  

   ;; Store pointer to input data structure in esi
   mov     esi, dwp [esp + INS$]
   test    esi, esi                  ; Test for zero
   jz      GetMSRFail                ; zero is bad

   ;; Store pointer to output data structure in edi
   mov     edi, dwp [esp + OUTS$]    ; 
   test    edi, edi                  ; Test for zero
   jz      GetMSRFail                ; zero is bad

   ;;
   ;; Get the MSR
   ;;

   ;; First, clear eax and edx
   xor     eax, eax
   xor     edx, edx

   mov     ecx, dwp [esi + MSRNO]    ; Load the MSR number into ecx
   mov     ebx, 0deadcafeh
   rdmsr                             ; Read the MSR
   mov     ebx, 0h

   ;;
   ;; Save the output data to outS
   ;;
   mov     [edi + MSRNO], ecx        ; Save the MSR number
   mov     [edi + MSRLO], eax        ; Save the lo DWORD
   mov     [edi + MSRHI], edx        ; Save the hi DWORD

   pop     ebp
   pop     edi
   pop     esi
   pop     ebx
       
   mov     eax, 01h                  ; Success
   ret

GetMSRFail:    
   pop     ebp
   pop     edi
   pop     esi
   pop     ebx
       
   mov     eax, 00h                  ; Failure
   ret    
   
_D32GetMSR endp   

public _D32SetMSR    
_D32SetMSR proc
   ;; save ebx, esi, edi, ebp
   push    ebx
   push    esi

   push    edi
   push    ebp
   
   mov     eax, p6Called
   test    eax, eax
   jnz     pastP6Check2

   call    _P6Stuff                  ; Check to see if it's a p6    
    
pastP6Check2:  

   ;; Store pointer to input data structure in esi
   mov     esi, dwp [esp + INS$]
   test    esi, esi                  ; Test for zero
   jz      SetMSRFail                ; zero is bad

   ;; Store pointer to output data structure in edi
   mov     edi, dwp [esp + OUTS$]    ; 
   test    edi, edi                  ; Test for zero
   jz      SetMSRFail                ; zero is bad

   ;;
   ;; Set the MSR
   ;;

   ;; Set ecx, eax, edx
   mov     ecx, dwp [esi + MSRNO]    ; Load the MSR number into ecx
   mov     eax, dwp [esi + MSRLO]    ; Load the MSR hi bit into eax
   mov     edx, dwp [esi + MSRHI]    ; Load the MSR lo bit into edx
   mov     ebx, 0deadcafeh  
   wrmsr                             ; Read the MSR
   mov     ebx, 0h
    
   ;;
   ;; Save the output data to outS
   ;;
   mov     [edi + MSRNO], ecx        ; Save the MSR number

   pop     ebp
   pop     edi
   pop     esi
   pop     ebx
       
   mov     eax, 01h                  ; Success
   ret

SetMSRFail:    
   pop     ebp
   pop     edi
   pop     esi
   pop     ebx
       
   mov     eax, 00h                  ; Failure
   ret  

_D32SetMSR endp   
      

;******************************************************************************
_TEXT   ends
   end

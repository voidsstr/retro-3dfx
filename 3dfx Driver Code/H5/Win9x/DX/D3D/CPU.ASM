;/*
;** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
;** File name:  cpu.asm
;**
;** Description: cpu detection function
;**
;** $Revision: 5 $
;** $Date: 2/16/99 3:32p $
;**
;** $Log: /devel/h3/W2K/Src/Video/Displays/h3/i386/cpu.asm $
;; 
;; 5     2/16/99 3:32p Russ
;; previous rev breaks N5T builds with D3D, so for NT5 if STBKNI isn't
;; already defined set it to 0
;; 
;; 4     2/11/99 8:28a Stb_skephart
;; 
;; 3     2/03/99 3:38p Sreid
;; K6/K7 WC enable from contractor
;;
;; 2     1/26/99 5:25p Peterm
;; Added unified header information
;
; */

ifdef WINNT   ; OLD CPUID CODE
TITLE           cpu.asm
                        .586P
                        .model FLAT

_TEXT       SEGMENT
            .code


ifdef WINNT
ifndef STBKNI
STBKNI = 0
endif
endif


PUBLIC  _cpu_detect_asm

;*----------------------------------------------------------------------
;function name: _cpu_detect_asm
;
;description:   Detects CPU type
;
;return:        DWORD in eax indicating CPU type
;----------------------------------------------------------------------*
_cpu_detect_asm PROC NEAR
P6Stuff:
    .586
    pushad                              ; save all regs.

    ; First, determine whether CPUID instruction is available.
    ; If it's not, then it's a 386 or 486.
    pushfd                              ; push original EFLAGS.
    pop eax                             ; pop into eax
    mov ecx, eax                        ; save original EFLAGS in ecx
    xor eax, 0200000h                   ; flip ID bit in EFLAGS
    push    eax                         ; put it back on stack
    popfd                               ; pop into EFLAGS
    pushfd                              ; get EFLAGS back
    pop eax                             ; into eax
    xor eax, ecx                        ; check to see if we could toggle ID
    jz  NotPentium                      ; Sorry, not P5 or P6.

    ;
    ; Now determine whether it's an intel P6 CPU.
    ;
    ;;  Is it an Intel CPU?
    xor eax, eax                        ; eax = 0.
    cpuid                               ; get cpuid
    xor ebx, 0756e6547h                 ; "Genu"
    jnz NotIntel
    xor edx, 049656e69h                 ; "ineI"
    jnz NotIntel
    xor ecx, 06c65746eh                 ; "ntel"
    jnz NotIntel                        ;
    ;;  Verifying architecture family
    mov eax, 1
    cpuid                               ; get family/model/stepping
    shr eax, 8                          ; rid of model & stepping number
    and eax, 0fh                        ; use only family
    cmp eax, 6
    jl     IsP5                         ; It's a P5
    ;; Else it's a P6
    ;
    ; Intel P6 processor.
    ; Make sure it supports Memory Type Range Request registers
    ;
IsP6:
    popad
    mov eax, 6                          ;
    ret                                 ; return

IsP5:
    popad
    mov eax, 5                          ;
    ret

NotPentium:
    popad
    mov eax, 4
    ret

NotIntel:
    popad
    mov eax, 0ffffffffh
    ret

_cpu_detect_asm ENDP



; STB-SK 02/09/99 add detection of Katmai CPU's
if STBKNI
;
PUBLIC _getKatmai
;
;*----------------------------------------------------------------------
;function name: getKatmai
;
;description:	Detect whether or not Katmai new instructions are supported
;		Warning -- this should *only* be executed on an Intel CPU
;		that supports the CPUID instruction. Don't just insert this 
;		into a random block of code -- it will fail on some CPU's.
;		Check _cpu_detect_asm and _getCPUInfo first!
;
;
;returns:       
;	0	Not Katmai enabled CPU (or no OS Katmai support)
;	1	Katmai enabled CPU
;
;----------------------------------------------------------------------*
_getKatmai	proc
	pushad
	mov	eax, 1
	cpuid
	test	edx, 1 SHL 25
	jz	NotKatmai		; Check Katmai new instruction in CPU features

	mov	eax, cr0
	test	eax, 1 SHL 2		; check for fp emulation, if enabled, fail
	jnz	NotKatmai

	mov	eax, cr4
	test	eax, 1 SHL 9		; check for OS support of Katmai
	jz	NotKatmai		; no support, bail!

Katmai:	
	popad
	mov	eax, 1
	ret

NotKatmai:
	popad
	mov	eax, 0
	ret
			
_getKatmai	endp
endif

;
PUBLIC  _getCPUInfo
;*----------------------------------------------------------------------
;function name: getCPUInfo
;
;description:
;
;returns:       CPU type (low word of EAX) and
;               family/model/stepping (high word of EAX)
;               Also EDX contains feature dword (EDX after CPUID with EAX==1)
;
;  CPU type:
;     0         CPUID not supported
;     1         Intel  ("GenuineIntel")
;     2         AMD    ("AuthenticAMD")
;     3         Cyrix  ("CyrixInstead")
;     4         IDT    ("CentaurHauls")
;    -1         What the hell is it ???
;
;----------------------------------------------------------------------*
_getCPUInfo     proc
                push    ebx

                ; check for support of cpuid
                pushfd
                pop     eax
                mov     ecx, eax
                xor     eax, 0200000h
                push    eax
                popfd
                pushfd
                pop     eax
                xor     eax, ecx
                jz      noCPUID
                ; we have cpuid supported
                ; then execute 0 function
                xor     eax, eax
                cpuid
                ; now we have vendor ID string in EBX:ECX:EDX

                ; first check if it's an Intel cpu
                cmp     ebx, 'uneG'                 ; "Genu"
                jnz     @F
                cmp     edx, 'Ieni'                 ; "ineI"
                jnz     @F
                cmp     ecx, 'letn'                 ; "ntel"
                jnz     @F

                ; ok, regular Intel genuine crap
                mov     ebx, 1
                jmp     getFMSword
        @@:
                ; check if it's an AMD cpu
                cmp     ebx, 'htuA'                 ; "Auth"
                jnz     @F
                cmp     edx, 'itne'                 ; "enti"
                jnz     @F
                cmp     ecx, 'DMAc'                 ; "cAMD"
                jnz     @F

                ; we have this AMD baby
                mov     ebx, 2
                jmp     getFMSword
        @@:
                ; check if it's a Cyrix cpu
                cmp     ebx, 'iryC'                 ; "Cyri"
                jnz     @F
                cmp     edx, 'snIx'                 ; "xIns"
                jnz     @F
                cmp     ecx, 'daet'                 ; "tead"
                jnz     @F

                ; cyrix cpu (raising from dead)
                mov     ebx, 3
                jmp     getFMSword
        @@:
                ; check if it's a IDT WinChip
                cmp     ebx, 'tneC'                 ; "Cent"
                jnz     @F
                cmp     edx, 'Hrua'                 ; "aurH"
                jnz     @F
                cmp     ecx, 'slua'                 ; "auls"
                jnz     @F

                ; oh... stuff from the sky
                mov     ebx, 4
                jmp     getFMSword

        @@:
                ; hey, what the hell is it ???
                mov     eax, 0000FFFFh
                xor     edx, edx
                jmp     return

        getFMSword:
                push    ebx
                ; ok, execute 1st function
                mov     eax, 1
                cpuid
                ; now EAX(13:0) contains FMS word and EDX - features
                pop     ebx
                and     eax, 0FFFFh       ; Cyrix has FMS in bits 15-0
                shl     eax, 16
                or      eax, ebx
                jmp     return
        noCPUID:
                xor     eax, eax
                xor     edx, edx
        return:
                pop     ebx
                ret
_getCPUInfo     endp

_TEXT   ENDS


endif ; WINNT   ; OLD CPUID CODE
END

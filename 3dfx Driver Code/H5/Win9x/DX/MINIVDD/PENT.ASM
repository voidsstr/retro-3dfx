;/* $Header: /devel/h3/Win95/dx/minivdd/pent.asm 4     1/12/99 9:23a Michael $ */
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
;** File name:   pent.asm
;**
;** Description: Pentuim Cycle Timer and support functions.
;**
;** $Revision: 4 $
;** $Date: 1/12/99 9:23a $
;**
;** $History: pent.asm $
;; 
;; *****************  Version 4  *****************
;; User: Michael      Date: 1/12/99    Time: 9:23a
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Implement the 3Dfx/STB unified header.
;; 
;; *****************  Version 3  *****************
;; User: Andrew       Date: 10/30/98   Time: 7:28a
;; Updated in $/devel/h3/Win95/dx/minivdd
;; Added some code to check for TimeStamp Counter
;; 
;; *****************  Version 2  *****************
;; User: Ken          Date: 4/15/98    Time: 6:42p
;; Updated in $/devel/h3/win95/dx/minivdd
;; added unified header to all files, with revision, etc. info in it
;**
;*/

        ;TITLE   Pentium Timing Utility Routines
        ;
        ; Public Symbols
        ;
        PUBLIC  _TimeStamp
        PUBLIC  _GetTime
        PUBLIC  _TotalTime
        PUBLIC  _VKD_Define_Hot_Key
        PUBLIC  _VKD_Remove_Hot_Key
        PUBLIC  _HotKey
        .386
        include vmm.inc
        include vkd.inc

        ;Define Data Segment        

VxD_LOCKED_DATA_SEG

P6wcfix_keyname   db "SOFTWARE\3Dfx Interactive\Shared", 0
P6wcfix_valuename db "DisableP6WCfix", 0

;extern _isP6:dword		; unused in this file!
    
VxD_LOCKED_DATA_ENDS
	
VxD_LOCKED_CODE_SEG


;*----------------------------------------------------------------------
;Function name:  _TimeStamp
;
;Description:    This routine determines if the Time Counter is
;                supported.
;Information:
;
;Return:         EAX    1 for success, 0 for failure
;----------------------------------------------------------------------*
_TimeStamp  PROC    
        push    ebp                             ; Standard Stack Frame
        mov     ebp, esp                        ; Stack Frame                  
        push    ebx                             ; Save register
        push    ecx                             ; Save register
        push    edx                             ; Ditto
        pushfd                                  ; Save flags

        pushfd                                  ; Get Flags
        pop     eax                             ; Gotta 'em
        mov     ebx, eax                        ; Save
        xor     eax, 00200000h                  ; Switch Bit 21
        push    eax                             ; Push eflags
        popfd                                   ; Restore
        pushfd                                  ; Get Flags
        pop     eax                             ; Gotta 'em
        cmp     eax, ebx                        ; Bit 21 Swapped?
        jz      NO_CPUID                        ; Nope then exit
        ;
        ; Ok now we can use the CPUID
        ; what we need to find out is if time stamp
        ; are present
        ;        
        mov     eax, 1                          ; Get Standard Features Flag
        db      0Fh, 0A2h                       ; CPUID
        and     edx, 10h                        ; Bit 4 is TimeStamp
        jz      NO_CPUID                        ; Nope

        ;
        ; Gotte 'em so use 'em
        ;
        mov     eax, 1                          ; return 1
        jmp     JMP_RET

NO_CPUID:
        xor     eax, eax                        ; return 0
JMP_RET:
        popfd                                   ; Restore flags
        pop     edx                             ; Restore edx
	pop	ecx                             ; Ditto w/ ecx
	pop	ebx                             ; Ditto w/ ebx
        pop     ebp                             ; Ditto w/ ebp
        ret
_TimeStamp ENDP


;*----------------------------------------------------------------------
;Function name:  _GetTime
;
;Description:    This routine reads the Pentuim Cycle Timer into
;                a 64-bit word pointed at by ebp+8.
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
_GetTime  PROC    
        push    ebp                             ; Standard Stack Frame
        mov     ebp, esp                        
	push	edx                             ; Save edx
        push    edi                             ; Save edi
        db      0Fh, 31h                        ; This is a RDTSC
        mov     edi, [ebp+8]                    ; Get Address of 64 bit word
        stosd                                   ; Save it
        xchg    eax, edx                        ; Get High 32 bit
        stosd                                   ; Save it
        pop     edi                             ; Restore
	pop	edx
        pop     ebp
        ret
_GetTime ENDP


;*----------------------------------------------------------------------
;Function name:  _TotalTime
;
;Description:    This routine is used to subtract
;                TotalTime = EndTime - StartTime.
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
_TotalTime  PROC  
        push    ebp                             ; Standard Stack Frame
        mov     ebp, esp
        push    ebx                             ; Save some registers
        push    ecx        
	push	edx
        push    edi
        push    esi
        mov     edi, [ebp+8]                    ; Start Time    
        mov     esi, [ebp+0ch]                  ; End Time
        mov     eax, [esi]                      ; EndTime Low
        mov     edx, [esi+4]                    ; EndTime High 
        mov     ecx, [edi]                      ; StartTime Low
        mov     ebx, [edi+4]                    ; StartTime High
        sub     eax, ecx                        ; 64 bit subtract
        sbb     edx, ebx                        ; Ditto        
        mov     edi, [ebp+10h]                  ; Result Addr
        stosd                                   ; Result
        xchg    eax, edx
        stosd                                   ; Result
        pop     esi                             ; Restore
        pop     edi
	pop	edx
        pop     ecx
        pop     ebx
        pop     ebp
        ret
_TotalTime ENDP


;*----------------------------------------------------------------------
;Function name:  _VKD_Define_Hot_Key
;
;Description:    Hook into Keyboard and get hot key
;                
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
PFUNC           equ     [ebp+8]
PDATA           equ     [ebp+12]
KEYCODE         equ     [ebp+16]
SDATA           equ     [ebp+20]
FLAGS           equ     [ebp+24]
_VKD_Define_Hot_Key PROC  
        push    ebp
        mov     ebp, esp
        push    ebx
        push    ecx
        push    edx
        push    edi
        push    esi

        mov     eax, KEYCODE
        mov     ebx, SDATA
        mov     ecx, FLAGS
        xor     edi, edi
        mov     esi, PFUNC
        mov     edx, PDATA
        VxDCall VKD_Define_Hot_Key
        jnc     Good_Ret
        xor     eax, eax

Good_Ret:   
        pop     esi
        pop     edi
        pop     edx
        pop     ecx
        pop     ebx
        pop     ebp
        ret
_VKD_Define_Hot_Key ENDP

;*----------------------------------------------------------------------
;Function name:  _VKD_Remove_Hot_Key
;
;Description:    Hook into Keyboard and get hot key
;                
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
HANDLE          equ     [ebp+8]
_VKD_Remove_Hot_Key PROC  
        push    ebp
        mov     ebp, esp

        mov     eax, HANDLE
        VxDCall VKD_Remove_Hot_Key

        pop     ebp
        ret
_VKD_Remove_Hot_Key ENDP

;*----------------------------------------------------------------------
;Function name:  _HotKey
;
;Description:    HotKey function
;       al = scan code
;       ah = state
;       ebx hot key handle
;       ecx = global shift state
;       edx = points to reference data
;       edi = elapsed time for delay                
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
_HotKey PROC  
        push    eax
        mov     eax, [edx]
        xor     eax , 1
        mov     [edx], eax
        pop     eax
        ret
_HotKey ENDP

VxD_LOCKED_CODE_ENDS
	
END

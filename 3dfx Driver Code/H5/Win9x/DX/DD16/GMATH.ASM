;/* $Header: /devel/h3/Win95/dx/dd16/gmath.asm 2     12/29/98 11:02a Michael $ */
;/*
;** Copyright (c) 1998-1999, 3Dfx Interactive, Inc.
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
;** File name:   gmath.asm
;**
;** Description: 32-bit x 32-bit multiply and divide functions
;**
;** $Revision: 2 $
;** $Date: 12/29/98 11:02a $
;**
;** $History: gmath.asm $
;; 
;; *****************  Version 2  *****************
;; User: Michael      Date: 12/29/98   Time: 11:02a
;; Updated in $/devel/h3/Win95/dx/dd16
;; Implement the 3Dfx/STB unified header.
;** 
;**
;*/

        PAGE    59,132
        TITLE   Gamma Math Routines
        PUBLIC  _imult   
        PUBLIC  _idiv

_TEXT   SEGMENT PUBLIC  WORD 'CODE' USE16
        Assume CS:_TEXT, DS:DGROUP, SS:DGROUP

DGROUP  GROUP  _BSS, _DATA

 
_BSS   SEGMENT PUBLIC  WORD '_BSS' USE16
_BSS   ENDS

_DATA   SEGMENT PUBLIC  WORD 'DATA' USE16
_DATA   ENDS

FRACMULT equ 10000

        .486p

;*----------------------------------------------------------------------
;Function name:  _imult
;
;Description:    Do a 32-bit x 32-bit multiply and then divide by
;                FRACMULT.  Done in assembler to get a 64-bit result
;                to reduce the chance of overflow.
;
;Information:
;
;Return:         DX:AX  Result of the multiply
;----------------------------------------------------------------------*
_imult PROC FAR  
	push	bp    			; Save bp
        mov     bp, sp                  ; get frame pointer
        push    ebx                     ; Save registers
        push    edx
        push    eax
       
        mov     eax, [bp+6]             ; Get mult1
        mov     ebx, [bp+10]            ; Get mult2
        xor     edx, edx                ; edx=0
        mul     ebx                     ; mult
        mov     ebx, [bp+14]            ; divisor
        div     ebx                     ; edx:eax/ebx

        mov     ebx, eax                ; save result
        pop     eax                     ; get reg
        mov     ax, bx                  ; low half
        pop     edx                     ; get reg
        shr     ebx, 16                 ; high half
        mov     dx, bx                  ; get result
        pop     ebx                     ; pop reg
        pop	bp			; Restore stack frame
	ret                             ; The end
_imult ENDP


;*----------------------------------------------------------------------
;Function name:  _idiv
;
;Description:    Do a 32-bit x 32-bit multiply by FRACMULT and then
;                divide by divisor.  Done in assembler to get a
;                64-bit result to reduce the chance of overflow.
;
;Information:
;
;Return:         DX:AX  Result of the divide
;----------------------------------------------------------------------*
_idiv PROC FAR   
	push	bp    			; Save bp
        mov     bp, sp                  ; get frame pointer
        push    ebx                     ; Save some registers
        push    edx
        push    eax
       
        
        mov     eax, [bp+6]             ; dividend
        mov     ebx, [bp+14]            ; Shift to get fraction
        xor     edx, edx                ; edx=0
        mul     ebx                     ;
        mov     ebx, [bp+10]            ; divisor
        div     ebx                     ; divide

        mov     ebx, eax                ; save result
        pop     eax                     ; get reg
        mov     ax, bx                  ; low half
        pop     edx                     ; get reg
        shr     ebx, 16                 ; high half
        mov     dx, bx                  ; save result
        pop     ebx                     ; get reg
        pop	bp			; Restore stack frame
	ret                             ; The end
_idiv ENDP

_TEXT   ENDS
END    




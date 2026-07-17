;/* $Header: /devel/h3/Win95/dx/dd16/reg16.asm 3     12/30/98 9:30a Michael $ */
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
;** File name:   reg16.asm
;**
;** Description: Misc functions to work on the 16-bit side.
;**              Some of these functions may no longer be used!
;**
;** $Revision: 3 $
;** $Date: 12/30/98 9:30a $
;**
;** $History: reg16.asm $
;; 
;; *****************  Version 3  *****************
;; User: Michael      Date: 12/30/98   Time: 9:30a
;; Updated in $/devel/h3/Win95/dx/dd16
;; Implement the 3Dfx/STB unified header.
;** 
;**
;*/

INCLUDE CMACROS.INC
externFP  LoadLibraryEx32W
externFP  GetProcAddress32W
extern OUTPUTDEBUGSTRING:far

	.MODEL  LARGE
	.586p
INCLUDELIB      LLIBCE
INCLUDELIB	OLDNAMES.LIB

reg16	SEGMENT  WORD USE16 PUBLIC 'CODE'
reg16 ENDS


data2 SEGMENT  DWORD PUBLIC 'DATA'
data2 ENDS

CONST	SEGMENT  WORD USE16 PUBLIC 'CONST'
CONST	ENDS

_BSS	SEGMENT  WORD USE16 PUBLIC 'BSS'
_BSS	ENDS

;DGROUP	GROUP	CONST, _BSS, _DATA
;	ASSUME DS: DGROUP
;	ASSUME  SS: NOTHING

EXTRN	_h3WRITE:FAR

data2 SEGMENT
align 4
t1 dd ?
t2 dd  ?
t3 dw  ?
t4 dd ?

align 4

globalD SavedProcAddress, 0
globalW CodeSelector32, 0
globalW CodeFiller, 0
globalW DataSelector32, 0
globalW DataFiller, 0

SpecialLibName db "WorkIt.dll", 0
TestFuncName   db "Test", 0

data2 ENDS

reg16      SEGMENT
	ASSUME	CS: reg16 

_data_	EQU	[bp+12h]
_reg_	EQU	[bp+0eh]
_hwptr_	EQU [bp+0ah]


;*----------------------------------------------------------------------
;Function name:  H3WRITE16
;
;Description:    Perform a register write to the HW.
;
;Information:    Calls _h3WRITE.
;
;Return:         VOID
;----------------------------------------------------------------------*
	PUBLIC H3WRITE16
	H3WRITE16 PROC FAR
	push bp
	mov bp, sp

	mov eax, dword ptr _data_
	push eax
	mov eax, dword ptr _reg_
	push eax
	mov eax, dword ptr _hwptr_
	push eax

	call FAR PTR _h3WRITE
	mov sp, bp
	pop bp
	db 66h
	ret
	H3WRITE16 endp


;*----------------------------------------------------------------------
;Function name:  _DbgOut
;
;Description:    Send a message to the debug terminal.
;
;Information:    Calls OUTPUTDEBUGSTRING.
;
;Return:         VOID
;----------------------------------------------------------------------*
	PUBLIC _DbgOut
	_DbgOut PROC FAR
	push bp
	mov bp, sp

	push [bp+014h]  ;selector
	push [bp+012h] ;offset

	call 	FAR PTR OUTPUTDEBUGSTRING

	mov sp, bp
	pop bp
	db 66h
	ret 
	_DbgOut  endp 

; ********************************************************************** 
; arg last to first
glyphIndex_GAT				EQU 	[bp+0eh]		
nfAWTableOffset_GAT  		EQU		[bp+0ah]	
FontPtr_GAT 				EQU		[bp+08h]
; ********************************************************************** 


;*----------------------------------------------------------------------
;Function name:  _GETAWTABLEDATA
;
;Description:    Return the offset of a glyph.
;
;Information:
;
;Return:         AX     Offset to the glyph.
;----------------------------------------------------------------------*
	PUBLIC _GETAWTABLEDATA
	_GETAWTABLEDATA PROC FAR

	push bp 
	mov bp, sp

	mov ax, FontPtr_GAT
	mov es, ax
	mov edx, dword ptr nfAWTableOffset_GAT
	movzx edi, word ptr glyphIndex_GAT	

	mov ax, word ptr es:[edx][edi*2]  ; get glyph offset.

	mov sp, bp
	pop bp
	ret
	_GETAWTABLEDATA endp


; ********************************************************************** 
HeaderSize_GGD		EQU		[bp+006h]
Offset_GGD  		EQU		[bp+00ch]	
FontPtr_GGD 		EQU		[bp+00ah]	
glyphIndex_GGD		EQU 	[bp+010h] ; last arg
dataOffset_GGD		EQU		[bp+012h]
; ********************************************************************** 
; ********************************************************************** 


;*----------------------------------------------------------------------
;Function name:  _GETGLYPHDATA
;
;Description:    Obtain the data for a specified glyph.
;
;Information:
;
;Return:         DX:AX     Returns dword of glyph data.
;----------------------------------------------------------------------*
	PUBLIC _GETGLYPHDATA
	_GETGLYPHDATA PROC FAR

	push bp 
	mov bp, sp

	mov ax, FontPtr_GGD
	mov es, ax
	mov edx, dword ptr Offset_GGD
	movzx edi, word ptr glyphIndex_GGD	
	

	movzx edi, word ptr es:[edx][edi*2]  ; get glyph offset.

	add  di, word ptr HeaderSize_GGD
	add  di, word ptr dataOffset_GGD

	mov ax, word ptr es:[edi];           ;get first dword of rowglyph data
	add edi, 2
	mov dx,  word ptr es:[edi];

	mov sp, bp
	pop bp
	ret

	_GETGLYPHDATA endp

; ********************************************************************** 
; ********************************************************************** 



Dataptr				EQU		[bp+006h]		
FontPtr_GGS			EQU		[bp+00ah]		
Offset_GGS 			EQU		[bp+00eh]	
glyphIndex_GGS		EQU 	[bp+012h]


;*----------------------------------------------------------------------
;Function name:  _GETGLYPHSTRUCT
;
;Description:    Obtain a glyph's structure.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
PUBLIC _GETGLYPHSTRUCT
	_GETGLYPHSTRUCT PROC FAR

	push bp 
	mov bp, sp
	push ds
	push es
	push esi
	push edi

	lds si, dword ptr Dataptr
	movzx esi, si
	

	; load font pointer
	les di, FontPtr_GGS
	;mov ax, FontPtr_GGS    ;FontPtr_GGS needs to be bp+c
	;mov es, ax

	mov ebx, dword ptr Offset_GGS
	movzx edi, word ptr glyphIndex_GGS	

	; move smallrow/largerow glyph header.  move 2 dwords, size of 
	; largerowglyph. let c code sort dataPtr.

	movzx edi, word ptr es:[ebx][edi*2]  ; get glyph offset.

	mov eax, dword ptr es:[edi];           ;get first dword of rowglyph data
	mov dword ptr ds:[esi], eax
	add si, 4
	add edi, 4
	mov eax,  dword ptr es:[edi];
	mov dword ptr ds:[esi], eax

	pop edi
	pop esi
	pop es
	pop ds

	pop bp
	ret
	_GETGLYPHSTRUCT endp


;*----------------------------------------------------------------------
;Function name:  Start32
;
;Description:    Sets up 32-bit environment for the driver
;                to play in.
;
;Information:    Contains numerous INT 3s!
;                Uses INT 31h.
;
;Return:         AX     0 is returned.
;----------------------------------------------------------------------*
cProc   Start32, <FAR, PUBLIC, PASCAL>, <esi, edi>
cBegin
		ASSUME DS:data2
        int     3

        ; load our 32-bit library

        push    SEG    SpecialLibName
        push    OFFSET SpecialLibName
        xor     eax, eax
        push    eax
        push    eax
        call    LoadLibraryEx32W

        or      dx, dx
        jnz     @f
        jmp     Start32_Bail

        ; retrieve linear address of our 32-bit test function

@@:     push    dx
        push    ax
        push    SEG    TestFuncName
        push    OFFSET TestFuncName
        call    GetProcAddress32W

        or      dx, dx
        jnz     @f
        jmp     Start32_Bail

        ; let's save the address, why don't we

@@:     mov     WORD PTR SavedProcAddress, ax
        mov     WORD PTR SavedProcAddress[2], dx

        ; create two selectors for our 32-bit stuff

        mov     cx, 2

        xor     ax, ax
        int     31h

        jnc     @f
        jmp     Start32_Bail

        ; the first selector will be our code selector

@@:     mov     CodeSelector32, ax

        ; set code segment base to zero

        mov     bx, ax
        xor     cx, cx
        xor     dx, dx

        mov     ax, 7
        int     31h

        jnc     @f
        jmp     Start32_Bail

        ; set code segment limit to four gigs

@@:     mov     bx, CodeSelector32
        mov     cx, 0FFFFh
        mov     dx, 0FFFFh

        mov     ax, 8
        int     31h

        jnc     @f
        jmp     Start32_Bail

        ; wouldn't it be nice if our code selector was 32-bit CODE?

@@:     mov     bx, CodeSelector32
        mov     cx, 0C0FAh

        mov     ax, 9
        int     31h

        jnc     @f
        jmp     Start32_Bail

        ; now get the selector increment

@@:     mov     ax, 3
        int     31h

        jnc     @f
        jmp     Start32_Bail

        ; the second selector wil be our... data selector!

@@:     add     ax, CodeSelector32
        mov     DataSelector32, ax

        ; set data segment base to zero

        mov     bx, ax
        xor     cx, cx
        xor     dx, dx

        mov     ax, 7
        int     31h

        jnc     @f
        jmp     Start32_Bail

        ; set data segment limit to four gigs

@@:     mov     bx, DataSelector32
        mov     cx, 0FFFFh
        mov     dx, 0FFFFh

        mov     ax, 8
        int     31h

        jnc     @f
        jmp     Start32_Bail

        ; wouldn't it be nice if our code selector was 32-bit DATA?

@@:     mov     bx, DataSelector32
        mov     cx, 0C0F2h

        mov     ax, 9
        int     31h

        jnc     @f
        jmp     Start32_Bail

        ; let's try making the call...

@@:     int     3

		push 	0deadh
		push 	0beefh
        push    0
        push    cs
        push    0
        push    OFFSET @f

        jmp     FWORD PTR SavedProcAddress

        ; what, done already?

@@:     jmp     Start32_Done

Start32_Bail:
        xor     ax, ax

Start32_Done:
cEnd


reg16 ENDS
END

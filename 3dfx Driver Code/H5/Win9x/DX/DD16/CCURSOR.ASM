;/* $Header: /devel/h5/Win9x/dx/dd16/ccursor.asm 13    6/16/99 4:44p Andrew $ */
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
;** File name:   ccursor.asm
;**
;** Description: Cursor MASM support functions
;**
;** $Revision: 13 $
;** $Date: 6/16/99 4:44p $
;**
;** $History: ccursor.asm $
;; 
;; *****************  Version 13  *****************
;; User: Andrew       Date: 6/16/99    Time: 4:44p
;; Updated in $/devel/h5/Win9x/dx/dd16
;; Added clear direction as the code always assumes that we are going
;; forward when sometimes we may be going backwards
;; 
;; *****************  Version 12  *****************
;; User: Andrew       Date: 6/04/99    Time: 4:09p
;; Updated in $/devel/h5/Win9x/dx/dd16
;; Added Code for SLI to do AND Mask and XOR Mask
;; 
;; *****************  Version 10  *****************
;; User: Michael      Date: 12/28/98   Time: 11:22a
;; Updated in $/devel/h3/Win95/dx/dd16
;; Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
;; files where this was missing.
;** 
;**
;*/

        option OLDSTRUCTS
        .MODEL LARGE
        .586p

;----------------------------------------------------------------------------
;----------------------------------------------------------------------------
_TEXT   SEGMENT  WORD USE16 PUBLIC 'CODE'
_TEXT   ENDS
_DATA   SEGMENT  WORD USE16 PUBLIC 'DATA'
_DATA   ENDS

        ASSUME DS: _DATA
        ASSUME SS: NOTHING
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; D A T A
;----------------------------------------------------------------------------
_DATA segment WORD PUBLIC 'DATA' use16

IFDEF GBLDATA_IN_PDEV
        extrn   lpDriverPDevice:DWORD
ELSE
        ; our PDevice
        extrn _DriverData:DWORD
        lpDriverPDevice equ _DriverData
ENDIF
        extern  wFlatDataSel:WORD

_DATA ends

;----------------------------------------------------------------------------
; C O D E
;----------------------------------------------------------------------------
_TEXT segment WORD PUBLIC 'CODE' use16

        extrn DIB_CheckCursorExt:FAR      ; in DIBENG

_TEXT ends

;----------------------------------------------------------------------------
; C O D E
;----------------------------------------------------------------------------
_TEXT segment WORD PUBLIC 'CODE' use16
        assume cs:_TEXT
        assume ds:nothing
        assume es:nothing
        assume fs:nothing
        assume gs:nothing



        assume ds:_DATA

;*----------------------------------------------------------------------
;Function name:  SetCursorBusy
;
;Description:    Sets the cursor busy semaphore (busy bit).
;
;Information:
;
;Return:         AX     1 if function set the cursor busy bit
;                       0 if cursor busy bit was already set
;----------------------------------------------------------------------*
SetCursorBusy PROC C USES edi es  lpCursorSemaphore :DWORD,
 
        les di, lpCursorSemaphore
        bts WORD PTR es:[di], 1
        jc  SetCursorAlreadyBusy         ;  already busy
        mov ax , 1
        ret
        
SetCursorAlreadyBusy:
       mov ax, 0
       ret
 
SetCursorBusy endp


;*----------------------------------------------------------------------
;Function name:  ClearCursorBusy
;
;Description:    Clears the cursor busy semaphore (busy bit).
;
;Information:
;
;Return:         AX     1 if function cleared the cursor busy bit
;                       0 if cursor busy bit was not previously set
;----------------------------------------------------------------------*
ClearCursorBusy PROC C USES  edi es  lpCursorSemaphore :DWORD,
 
	les di, lpCursorSemaphore
        btr WORD PTR es:[di], 1
        jnc  ClearCursorNotBusy         ;  Clearing busy thats not set.
        mov ax , 1
        ret
        
ClearCursorNotBusy:
       mov ax, 0 
       ret
 
ClearCursorBusy endp


;*----------------------------------------------------------------------
;Function name:  SetBusy
;
;Description:    Sets the busy semaphore.
;
;Information:
;
;Return:         AX     1 if function set the busy bit
;                       0 if busy bit was already set
;----------------------------------------------------------------------*
SetBusy PROC C USES edi es  lpSemaphore :DWORD,
 
	les di, lpSemaphore
        bts WORD PTR es:[di], 1
        jc  SetAlreadyBusy         ;  already busy
        mov ax , 1
        ret
        
SetAlreadyBusy:
       mov ax, 0
       ret
 
SetBusy endp


;*----------------------------------------------------------------------
;Function name:  ClearBusy
;
;Description:    Clear the busy semaphore.
;
;Information:
;
;Return:         AX     1 if function cleared the busy bit
;                       0 if busy bit was not previously set
;----------------------------------------------------------------------*
ClearBusy PROC C USES  edi es  lpSemaphore :DWORD,
 
	les di, lpSemaphore
        btr WORD PTR es:[di], 0
        jnc  ClearNotBusy         ;  Clearing busy thats not set.
        mov ax , 1
        ret
        
ClearNotBusy:
       mov ax, 0 
       ret
 
ClearBusy endp


;*----------------------------------------------------------------------
;Function name:  DoCursor
;
;Description:    Stores cursor's AND and XOR masks.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
        assume ds:_DATA
DoCursor PROC C uses eax ebx cx edx edi si ds es DataOffset:DWORD, lpMask:DWORD
        mov     ax, wFlatDataSel        ; 32 Bit Data Sel
        mov     es, ax                  ; Load ds
        mov     edi, DataOffset         ; Get Offset
        lds     si, lpMask              ; Get Mask
        xor     edx, edx                ; eax=0
        mov     ebx, edx                ; ebx=0
        not     ebx                     ; ebx=FFFFFFFF
        mov     cx, 32
MaskStore:
        mov     eax, DWORD PTR [si]     ; Get And Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, ebx                ; Store 1's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, DWORD PTR [si+80h] ; Get Xor Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, edx                ; 0's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        add     si, 4                   ; Next Mask
        loop    MaskStore

        mov     cx, 32
UnusedStore:
        mov     eax, ebx                ; Store 1's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, edx                ; 0's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        db      67h                     ; Address Override
        stosd                           ; Store Data
        loop    UnusedStore
        ret
DoCursor        ENDP


;*----------------------------------------------------------------------
;Function name:  DoBigCursor
;
;Description:    Doubles the cursor's AND and XOR masks (32x32 -> 64x64).
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
ifdef BIG_CURSOR
DoBigCursor PROC C uses eax ebx cx edx edi si ds es DataOffset:DWORD, lpMask:DWORD
        mov     ax, wFlatDataSel        ; 32 Bit Data Sel
        mov     es, ax                  ; Load ds
        mov     edi, DataOffset         ; Get Offset
        lds     si, lpMask              ; Get Mask
        mov     cx, 32
BigMaskStore:
        mov     bx, 2
MaskDbl:
        mov     eax, DWORD PTR [si]     ; Get And Mask
        call    BitDbl
        mov     eax, DWORD PTR [si+80h] ; Get Xor Mask
        call    BitDbl
        dec     bx
        jnz     MaskDbl
        add     si, 4                   ; Next Mask
        loop    BigMaskStore
        ret
DoBigCursor        ENDP


;*----------------------------------------------------------------------
;Function name:  BitDbl
;
;Description:    Doubles a cursor 32x32 mask.  Called for both
;                AND and XOR.
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
BitDbl  PROC
        push    ebx
        push    ecx
        push    edx        

        mov     bx, 4
StDbl:
        xor     edx, edx
        mov     cx, 8
Dbl:
        ror     al, 1
        jnc     SkipOr
        or      dl, 3h
SkipOr:
        ror     dx, 2
        loop    Dbl
        xchg    dh, dl
        xchg    edx, eax
        db      67h                     ; Address Override
        stosw                           ; Store Data
        xchg    edx, eax
        shr     eax, 8
        dec     bx
        jnz     StDbl
        pop     edx
        pop     ecx
        pop     ebx
        ret
BitDbl  ENDP   
endif


;*----------------------------------------------------------------------
;Function name:  DoCursorDblX
;
;Description:    Stores cursor's doubled AND and XOR masks.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
        assume ds:_DATA
DoCursorDblX PROC C uses eax ebx cx edx edi si ds es DataOffset:DWORD, lpMask:DWORD
        mov     ax, wFlatDataSel        ; 32 Bit Data Sel
        mov     es, ax                  ; Load ds
        mov     edi, DataOffset         ; Get Offset
        lds     si, lpMask              ; Get Mask
        xor     edx, edx                ; eax=0
        mov     ebx, edx                ; ebx=0
        not     ebx                     ; ebx=FFFFFFFF
        mov     cx, 32
MaskStoreI:
        mov     eax, DWORD PTR [si]     ; Get And Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, ebx                ; Store 1's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, DWORD PTR [si+80h] ; Get Xor Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, edx                ; 0's
        db      67h                     ; Address Override
        stosd                           ; Store Data

        mov     eax, DWORD PTR [si]     ; Get And Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, ebx                ; Store 1's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, DWORD PTR [si+80h] ; Get Xor Mask
        db      67h                     ; Address Override
        stosd                           ; Store Data
        mov     eax, edx                ; 0's
        db      67h                     ; Address Override
        stosd                           ; Store Data
        add     si, 4                   ; Next Mask
        loop    MaskStoreI

        ret
DoCursorDblX        ENDP

;*----------------------------------------------------------------------
;Function name:  FastDWordCopy
;
;Description:    A faster version of DoBitCopy. Can be used when data
;                to copy is a DWORD multiple.
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*

FastDWordCopy PROC C uses ecx edi esi ds es SrcAddr:FWORD, DestAddr:FWORD, dwDeltaScan:DWORD, wHeight:WORD
        lds     esi, FWORD PTR SrcAddr
        les     edi, FWORD PTR DestAddr
        movzx   ecx, wHeight
        imul    ecx, dwDeltaScan
        shr     ecx, 2
        cld
        rep     movsd dword ptr es:[edi], dword ptr ds:[esi] 
        ret
FastDWordCopy endp

;*----------------------------------------------------------------------
;Function name:  DoBitCopy
;
;Description:    Moves cursor from a Source to Destination via
;                SW (REP MOVS).
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
DoBitCopy PROC C uses eax ebx ecx edx edi esi ds es SrcAddr:FWORD, DestAddr:FWORD, dwSrcAdj:DWORD, dwDestAdj:DWORD, wWidth:WORD, wHeight:WORD
        cld                                     ; Clear Flag
        lds     esi, FWORD PTR SrcAddr          ; ds:esi = Source
        les     edi, FWORD PTR DestAddr         ; es:edi = DestStart
        mov     eax, dwSrcAdj                   ; Source Adjust
        mov     edx, dwDestAdj                  ; Dest Adjust
        mov     bx, wHeight                     ; Number of Times
        shl     ebx, 16                         ; Height in Upper word of ebx
        xor     ecx, ecx                        ; ecx=0
DoLoop:
        mov     bx, wWidth                      ; Get Width
        shr     bx, 1                           ; Number of Words
        adc     cx, 0                           ; Carry has # of bytes
        db      67h                             ; Address Override
        rep     movsb                           ; Move a byte
        shr     bx, 1                           ; Number of Dwords
        mov     cx, bx                          ; cx=#Dwords
        db      67h                             ; Address Override
        rep     movsd                           ; Move it
        adc     cx, 0                           ; Carry has #words
        db      67h                             ; Address Override
        rep     movsw                           ; Move a word
        add     esi, eax                        ; Start of Next Source Line
        add     edi, edx                        ; Start of Next Dest Line
        xor     bx, bx                          ; bx=0
        sub     ebx, 10000h                     ; Height=Height - 1
        jnz     DoLoop

        ret
DoBitCopy       ENDP


ifdef SLI_AA
;*----------------------------------------------------------------------
;Function name:  DoBitAndCopy
;
;Description:    Moves And Mask on Top of Image
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
DoBitAndCopy PROC C uses eax ebx ecx edx edi esi ds es SrcAddr:FWORD, DestAddr:FWORD, dwMaskAdj:DWORD, wWidth:WORD, wHeight:WORD
        cld                                     ; Clear Flag
        lds     esi, FWORD PTR SrcAddr          ; ds:esi = Source
        les     edi, FWORD PTR DestAddr         ; es:edi = DestStart
        mov     edx, dwMaskAdj                  ; Mask Adjust
        mov     bx, wHeight                     ; Number of Times
        shl     ebx, 16                         ; Height in Upper word of ebx
        xor     ecx, ecx                        ; ecx=0
DoAndLoop:
        mov     bx, wWidth                      ; Get Width
        shr     bx, 1                           ; Number of Words
        adc     cx, 0                           ; Carry has # of bytes
        or      cx, cx                          ; Is cx == 0
        jz      SkipAndByte                
        db      67h                             ; Address Override
        lodsb                                   ; Move a byte
        and     al, BYTE PTR [edi]              ; And Data
        db      67h                             ; Address Override
        stosb
SkipAndByte:
        xor     cx, cx                          ; cx=0
        shr     bx, 1                           ; Number of Dwords
        adc     cx, 0                           ; Carry has #words
        or      cx, cx                          ; Is cx == 0
        jz      SkipAndWord                
        db      67h                             ; Address Override
        lodsw                                   ; Move a byte
        and     ax, WORD PTR [edi]              ; And Data
        db      67h                             ; Address Override
        stosw

SkipAndWord:
        mov     cx, bx                          ; cx=#Dwords
        or      cx, cx                          ; Is cx == 0
        jz      SkipAndDword                
DwordAnd:
        db      67h                             ; Address Override
        lodsd                                   ; Move a byte
        and     eax, DWORD PTR [edi]            ; And Data
        db      67h                             ; Address Override
        stosd
        loop    DwordAnd

SkipAndDword:
        add     esi, edx                        ; Start of Next Source Line
        add     edi, edx                        ; Start of Next Dest Line
        xor     bx, bx                          ; bx=0
        sub     ebx, 10000h                     ; Height=Height - 1
        jnz     DoAndLoop

        ret
DoBitAndCopy       ENDP


;*----------------------------------------------------------------------
;Function name:  DoBitXorCopy
;
;Description:    Moves Xor Mask on Top of Image
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
DoBitXorCopy PROC C uses eax ebx ecx edx edi esi ds es SrcAddr:FWORD, DestAddr:FWORD, dwMaskAdj:DWORD, wWidth:WORD, wHeight:WORD
        cld                                     ; Clear Flag
        lds     esi, FWORD PTR SrcAddr          ; ds:esi = Source
        les     edi, FWORD PTR DestAddr         ; es:edi = DestStart
        mov     edx, dwMaskAdj                  ; Mask Adjust
        mov     bx, wHeight                     ; Number of Times
        shl     ebx, 16                         ; Height in Upper word of ebx
        xor     ecx, ecx                        ; ecx=0
DoXorLoop:
        mov     bx, wWidth                      ; Get Width
        shr     bx, 1                           ; Number of Words
        adc     cx, 0                           ; Carry has # of bytes
        or      cx, cx                          ; Is cx == 0
        jz      SkipXorByte                
        db      67h                             ; Address Override
        lodsb                                   ; Move a byte
        xor     al, BYTE PTR [edi]              ; And Data
        db      67h                             ; Address Override
        stosb
SkipXorByte:
        xor     cx, cx                          ; cx=0
        shr     bx, 1                           ; Number of Dwords
        adc     cx, 0                           ; Carry has #words
        or      cx, cx                          ; Is cx == 0
        jz      SkipXorWord                
        db      67h                             ; Address Override
        lodsw                                   ; Move a byte
        xor     ax, WORD PTR [edi]              ; And Data
        db      67h                             ; Address Override
        stosw

SkipXorWord:
        mov     cx, bx                          ; cx=#Dwords
        or      cx, cx                          ; Is cx == 0
        jz      SkipXorDword                
DwordXor:
        db      67h                             ; Address Override
        lodsd                                   ; Move a byte
        xor     eax, DWORD PTR [edi]            ; And Data
        db      67h                             ; Address Override
        stosd
        loop    DwordXor

SkipXorDword:
        add     esi, edx                        ; Start of Next Source Line
        add     edi, edx                        ; Start of Next Dest Line
        xor     bx, bx                          ; bx=0
        sub     ebx, 10000h                     ; Height=Height - 1
        jnz     DoXorLoop

        ret
DoBitXorCopy       ENDP
endif

;*----------------------------------------------------------------------
;Function name:  ClearMem
;
;Description:    Clears a specified size of memory.
;
;Information:    Function appears to not be used.
;
;Return:         VOID
;----------------------------------------------------------------------*
ClearMem PROC C uses eax ecx edi es dwStart:DWORD, dwSize:DWORD
        mov     ax, wFlatDataSel
        mov     es, ax
        mov     edi, dwStart
        xor     eax, eax
        mov     ecx, dwSize
        db      67h
        db      66h
        rep     stosd
        ret
ClearMem        ENDP


;*----------------------------------------------------------------------
;Function name:  LoadPalette
;
;Description:    Quick Load of the Palette
;
;Information:    
;
;Return:         VOID
;----------------------------------------------------------------------*
LoadPalette PROC C uses eax ebx ecx edx edi esi ds es DacAddr:DWORD, pColor:DWORD, FlatSel:WORD, Start:WORD, EndIndex:WORD
        lds     si, pColor                      ; Get Palette Data
        mov     ax, FlatSel                     ; Get Flat Selector
        mov     es, ax                          ; Set es to flat Selector
        mov     edi, DacAddr                    ; Get Address of Dac Index
        xor     ebx, ebx                        ; Clear Index Counter
        mov     bx, Start                       ; Set to Start

ColorLoop:
        cmp     bx, EndIndex                    ; Done???
        jge     ColorExit                       ; yes then exit

        xor     ecx, ecx                        ; Clear repeat counter
        mov     es:[edi], ebx                   ; Set Index
        lodsd                                   ; Get Data Data
        mov     edx, es:[edi]                   ; Get Status
        inc     ebx                             ; Get Next Index
        and     eax, 00FFFFFFh                  ; Only use 24 bits
Retry:
        mov     es:[edi+4], eax                 ; Store Data
        inc     ecx                             ; Increment repeat counter
        mov     edx, es:[edi+4]                 ; Get Data
        xor     edx, eax                        ; Same
        jz      ColorLoop                       ; yes
        cmp     cx, 10                          ; enough
        jl      Retry                           ; Nope, try again
        jmp     ColorLoop                       ; Yes, go to next

ColorExit:                                      ;
                                  
        ret                             
LoadPalette ENDP


_TEXT ends
END

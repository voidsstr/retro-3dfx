;/* $Header: /devel/h3/win95/dx/dd16/fifo16.asm 2     2/09/99 10:17a Stb_pzheng $ */
;/*
;** Copyright (c) 1995-1998, 3Dfx Interactive, Inc.
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
;**
;** $Revision: 2 $
;** $Date: 2/09/99 10:17a $
;**
;** $History: fifo16.asm $
;; 
;; *****************  Version 2  *****************
;; User: Stb_pzheng   Date: 2/09/99    Time: 10:17a
;; Updated in $/devel/h3/win95/dx/dd16
;; 
;; *****************  Version 1  *****************
;; User: Stb_srogers  Date: 1/29/99    Time: 9:05a
;; Created in $/devel/h3/win95/dx/dd16
;; 
;; 
;**
;*/


;----------------------------------------------------------------------------
;
; fifo16.asm
;
; ASM equivalents to the 32bit cmdfifo routines
; All of this code was written based on INC\FIFOMGR.H
;
;----------------------------------------------------------------------------

FIFO16FILE equ 1    ; make FIFOMGR.INC not extern the routines in this file

        .xlist
        include shared.inc
        include gdidib.inc
        include fifomgr.inc
        .list

.586p

_TEXT   SEGMENT WORD PUBLIC 'CODE' USE16 

;
; fifo_MakeRoom
;
;   ebx = hwPtr
;   ecx = n
;
; Returns ebx = new hwPtr
;

fifo_MakeRoom Proc Near Public
        mov     eax, ebx        ; eax = incoming hwPtr
        mov     edx, ecx        ; edx = number of fifo entries needed (n)
        shl     edx, 2          ; n * 4
        add     eax, edx        ; hwPtr + (n * 4)
        cmp     eax, GLOB.mainFifo.FXend ; if ((hwPtr + dwords needed)>mainFifo.FXend)
        jb      short FMR_10
        call    fifo_Wrap            ; THEN wrap the fifo around to the beginning
FMR_10:        
        cmp     GLOB.mainFifo.space, ecx
        jge     short FMR_30
        LOAD_CMDFIFO_RDPTR      ; sets edx = rdPtr
        add     edx, GLOB.mainFifo.FXoffset
        cmp     edx, ebx
        jg      short FMR_20
        mov     eax, GLOB.mainFifo.FXend
        sub     eax, ebx
        shr     eax, 2          ; (mainFifo.FXend - hwPtr)/4
        mov     GLOB.mainFifo.space, eax
        cmp     eax, ecx
        jge     short FMR_10
        call    fifo_Wrap
        jmp     short FMR_10
FMR_20:
        mov     eax, edx
        sub     eax, ebx
        shr     eax, 2          ; (rdPtr - hwPtr)/4
        sub     eax, 1
        mov     GLOB.mainFifo.space, eax
        jmp     short FMR_10
FMR_30:        
        ret
fifo_MakeRoom Endp


IFDEF PERF_NEW16BITBLT

;
; fifo_MakeRoom_Fast
;
;   ebx = hwPtr
;   ecx = n
;
; Returns ebx = new hwPtr
;
; This is the version of fifo_MakeRoom to be used with the optimized macro
; FIFO_CHECKROOM2
;

fifo_MakeRoom_Fast Proc Near Public

        add     GLOB.mainFifo.space, ecx
        lea     eax, [ebx+ecx*4]
        cmp     eax, GLOB.mainFifo.FXend ; if ((hwPtr + dwords needed)>mainFifo.FXend)
        jb      short FMR_10
        call    fifo_Wrap            ; THEN wrap the fifo around to the beginning
FMR_10:        
        cmp     GLOB.mainFifo.space, ecx
        jge     short FMR_30
        LOAD_CMDFIFO_RDPTR      ; sets edx = rdPtr
        add     edx, GLOB.mainFifo.FXoffset
        cmp     edx, ebx
        jg      short FMR_20
        mov     eax, GLOB.mainFifo.FXend
        sub     eax, ebx
        shr     eax, 2          ; (mainFifo.FXend - hwPtr)/4
        mov     GLOB.mainFifo.space, eax
        cmp     eax, ecx
        jge     short FMR_10
        call    fifo_Wrap
        jmp     short FMR_10
FMR_20:
        mov     eax, edx
        sub     eax, ebx
        shr     eax, 2          ; (rdPtr - hwPtr)/4
        sub     eax, 1
        mov     GLOB.mainFifo.space, eax
        jmp     short FMR_10
FMR_30: 
        ret
                        
fifo_MakeRoom_Fast Endp
ENDIF ; PERF_NEW16BITBLT

;
; fifo_Wrap
;
;   ebx = hwPtr
;

fifo_Wrap Proc Near Public
FW_10:
        LOAD_CMDFIFO_RDPTR      ; edx = rdPtr
        add     edx, GLOB.mainFifo.FXoffset
        cmp     edx, ebx
        ja      short FW_10
        cmp     edx, GLOB.mainFifo.start
        je      short FW_10
   ifdef DEBUGFIFO
        xor     eax, eax
        push    ebx     ; hwPtr
        push    eax     ; hwIndex = 0 
        mov     eax, GLOB.mainFifo.FXjmp
        push    eax
        call    _mySetPH
        add     sp, 3 * 4
    ifdef AGP_CMDFIFO
        cmp     GLOB.doAgpCF, 0
        je      short FW_20
        push    ebx     ; save hwPtr
        add     ebx, 4          ; next dword
        xor     eax, eax
        push    ebx
        push    eax     ; hwIndex = 0
        mov     eax, GLOB.mainFifo.jmp2
        call    _mySetPH
        add     sp, 3 * 4
        pop     ebx     ; restore hwPtr
FW_20:             
    endif ; ifdef AGP_CMDFIFO
   else ; ifdef DEBUGFIFO
        mov     eax, GLOB.mainFifo.FXjmp
        mov     es:[ebx], eax
    ifdef AGP_CMDFIFO
        cmp     GLOB.doAgpCF, 0
        je      short FW_30
        push    ebx     ; save hwPtr
        add     ebx, 4  ; next dword (hwIndex = 1)
        mov     eax, GLOB.mainFifo.jmp2
        mov     es:[ebx], eax
        pop     ebx     ; restore hwPtr
    endif ; ifdef AGP_CMDFIFO
   endif ; ifdef DEBUGFIFO
FW_30:        
        
        P6FENCE
        
   ifdef AGP_CMDFIFO
        cmp     GLOB.doAgpCF, 0
        je      short FW_40
        push    ebx     ; hwPtr
        call    myWrapAgp
        add     sp, 1 * 4
FW_40:        
   endif
        mov     ebx, GLOB.mainFifo.start
        mov     eax, edx
        sub     eax, ebx
        shr     eax, 2
        sub     eax, 1
        mov     GLOB.mainFifo.space, eax

        ret
fifo_Wrap Endp

;
; reset_Invariants
;
; ECX = reset mask

reset_Invariants Proc Near Public
        CMDFIFO_PROLOG
        xor     eax, eax
        mov     RSTWRDS, eax
        mov     RSTMASK, eax
        
        test    ecx, RESET_PDEV
        jz      short RI_20
        cmp     GLOB.ddPrimaryInTile, 0
        je      short RI_10
        or      GLOB.gdiDesktopStart, SSTG_IS_TILED
        mov     edx, GLOB.screenFormat
        mov     eax, GLOB.ddTileStride
        and     edx, NOT (SSTG_DST_LINEAR_STRIDE + SSTG_DST_TILE_STRIDE)
        shl     eax, SSTG_DST_STRIDE_SHIFT
        or      edx, eax
        mov     GLOB.screenFormat, edx
        push    es
        push    edi
        mov     eax, GLOB.ddTilePitch
        les     di, dword ptr GLOB.lpPDevice
        mov     es:[di].DIBENGINE.deDeltaScan, eax
        pop     edi
        pop     es
        jmp     short RI_20
RI_10:
        and     GLOB.gdiDesktopStart, NOT SSTG_IS_TILED
        mov     edx, GLOB.screenFormat
        mov     eax, GLOB.pitch
        and     edx, NOT (SSTG_DST_LINEAR_STRIDE + SSTG_DST_TILE_STRIDE)
        shl     eax, SSTG_DST_STRIDE_SHIFT
        or      edx, eax
        mov     GLOB.screenFormat, edx
        push    es
        push    edi
        mov     eax, GLOB.pitch
        les     di, dword ptr GLOB.lpPDevice
        mov     es:[di].DIBENGINE.deDeltaScan, eax
        pop     edi
        pop     es
RI_20:
        test    ecx, RESET_DST
        jz      short RI_21
        add     dword ptr RSTWRDS, 5
        or      dword ptr RSTMASK, (clip0minBit + clip0maxBit + dstBaseAddrBit + dstFormatBit + commandExBit)
RI_21:    
        test    ecx, RESET_SRC
        jz      short RI_22
        add     dword ptr RSTWRDS, 1
        or      dword ptr RSTMASK, srcBaseAddrBit
RI_22:    
        cmp     dword ptr RSTWRDS, 0
        je      RI_30
        add     dword ptr RSTWRDS, 1
        CMDFIFO_SETUP
        push    ecx     ; save RESET_MASK
        mov     ecx, RSTWRDS
        CMDFIFO_CHECKROOM ecx
        mov     ecx, RSTMASK
        or      ecx, SSTCP_PKT2
        SETPH   ecx
        pop     ecx     ; restore RESET_MASK        
        test    ecx, RESET_DST
        jz      short RI_23
        SETI    clip0min, 0
        mov     eax, GLOB.bi.biHeight
        shl     eax, 16
        mov     ax, word ptr GLOB.bi.biWidth
        SETI    clip0max, eax
        SETM    dstBaseAddr, GLOB.gdiDesktopStart
        SETM    dstFormat, GLOB.screenFormat
RI_23:
        test    ecx, RESET_SRC
        jz      short RI_24
        SETM    srcBaseAddr, GLOB.gdiDesktopStart
RI_24:     
        test    ecx, RESET_DST
        jz      short RI_25
        SETI    commandEx, 0
RI_25: 
        mov     ecx, RSTWRDS
        BUMP    ecx
        CMDFIFO_EPILOG
RI_30:    
        ret
reset_Invariants Endp


_TEXT   ENDS

END


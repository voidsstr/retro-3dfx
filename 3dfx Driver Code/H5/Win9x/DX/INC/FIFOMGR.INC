;/* $Header:  $ */
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
;** $Revision: $
;** $Date: $
;**
;** $History: $
;; 
;; 
;**
;*/

;----------------------------------------------------------------------------
;
; fifomgr.inc
;
; ASM macro equivalents of fifomgr.h macros
; Macros assume that DS = driver data segment
;
; Notes:
;
;   Between CMDFIFO_CHECKROOM and BUMP, ebx must be preserved
;   FXLEAVE sets ECX = 0 for PUNT, non-zero for normal return
;
;----------------------------------------------------------------------------

        .xlist
        include h3g.inc
        include h3regs.inc
        include h3gdefs.inc
        include dibeng.inc
        .list

; STB Begin change
; STB-SR 12/16/98 Added for STB Performance optimizations
ifdef INCSTBPERF
INCLUDE ..\..\BUILD\STBPERF.INC
endif
; STB END change



RESET_DST       equ     0001h
RESET_SRC       equ     0002h
RESET_PDEV      equ     0004h

PRIMARY_CMDFIFO equ     cmdFifo0

;
; Equates to make things easier
;

RBP     equ     bp

; These locals will be the same in all routines that use the cmdfifo

CF      equ     [RBP-04]
IDX     equ     [RBP-08]
CLIPSEL equ     [RBP-12]
DSTADDR equ     [RBP-16]
DSTFMT  equ     [RBP-20]
SRCADDR equ     [RBP-24]
SRCFMT  equ     [RBP-28]
DSTDBM  equ     [RBP-32]
SRCDBM  equ     [RBP-36]
P6SWAP  equ     [RBP-40]
RSTWRDS equ     [RBP-44]
RSTMASK equ     [RBP-48]
NEXTLOC equ     52      ; should always be last one + 4

GLOB    equ     _DriverData

;****************************
; P6FENCE
;
; Flush write-combine cache
;****************************
P6FENCE Macro
   ifdef WRITECOMBINE
		;db 00fh		; Tried sfence, was not any faster
		;db 0aeh
		;db 0ffh
        xchg    eax, P6SWAP
   endif        
Endm

;****************************
; LOAD_CMDFIFO_RDPTR
;****************************
LOAD_CMDFIFO_RDPTR Macro
        push    edi
        mov     edi, GLOB.lpCRegs
        lea     edi, [edi].SstCRegs.PRIMARY_CMDFIFO.readPtrL ;
        mov     edx, es:[edi]   ; GET( ((SstCRegs *) p)->PRIMARY_CMDFIFO.readPtrL )
        pop     edi
Endm

CMDFIFO_PROLOG Macro
Endm


;*******************************************************
; CMDFIFO_SETUP
;
; Retrieves the current CMDFIFO pointer from DriverData
; Sets up ES with FlatSel
;*******************************************************
CMDFIFO_SETUP Macro
ifdef STB_FIFO16_ENABLE
        mov     eax, GLOB.mainFifo.FXptr
        xor     edx, edx
        mov     dword ptr IDX, edx      ; initialize HWINDEX to 0
        mov     dx, _FlatSel
        mov     dword ptr CF, eax       ; initialize local CMDFIFO pointer
        mov     es, dx
endif
Endm

;*******************************************************
; CMDFIFO_SETUP_LITE
;
; Retrieves the current CMDFIFO pointer from DriverData
; Assumes ES is already the FlatSel
;*******************************************************
CMDFIFO_SETUP_LITE Macro
ifdef STB_FIFO16_ENABLE
        mov     eax, GLOB.mainFifo.FXptr
        xor     edx, edx
        mov     dword ptr IDX, edx      ; initialize HWINDEX to 0
        mov     dword ptr CF, eax       ; initialize local CMDFIFO pointer
endif
Endm

;**************************************************************
; CMDFIFO_EPILOG
;
; Finishes using the CMDFIFO by saving the current CMDFIFOPTR
; in the global data structure and a few other things
;**************************************************************
CMDFIFO_EPILOG Macro
        Local   CF_10
        mov     eax, IDX
        sub     GLOB.mainFifo.space, eax  ; _FF(mainFifo.space) -= hwIndex 
        xor     edx, edx
        shl     eax, 2
        add     CF, eax                 ; hwPtr += (hwIndex << 2)
        mov     IDX, edx                ; hwIndex = 0
   ifdef AGP_CMDFIFO
        cmp     GLOB.doAgpCF, 0         ; if (_FF(doAgpCF))
        je      short CF_10
        push    dword ptr CF
        call    myAgpEpilog             ;    myAgpEpilog((DWORD)hwPtr)
        add     sp, 1 * 4
CF_10:        
   endif        
        mov     eax, CF
        mov     GLOB.mainFifo.FXptr, eax    ; _FF(mainFifo.FXptr) = hwPtr
Endm

;***********
; SETCF
;***********
SETCF Macro SetData
        mov     dword ptr es:[ebx], SetData
        add     ebx, 4
Endm        
        
;**************************************************
; SETI
;
; Puts a literal value or register into the packet
;**************************************************
SETI Macro FxReg, LitVal
        SETCF   LitVal
Endm

;*************************************************************
; SETPH
;
; Sets up the packet header
; Prepares EBX with the current CMDFIFO value for SETx macros
;*************************************************************
SETPH Macro SetBits
        mov     ebx, CF         ; setup hwPtr for packet programming
        SETCF   SetBits
Endm


;*****************************************************************
; SETM
;
; Store the data at the specified memory location into the packet
;*****************************************************************
SETM Macro FxReg, DataAddr
        mov     eax, dword ptr DataAddr
        SETCF   eax
Endm

;********************************************
; SETCMD
;
; Store the command starter into the packet
;********************************************
SETCMD Macro FxReg, SetBits
        Local   SC_10, SC_11
   ifdef PERFNOP
        cmp     doPerfNop, 0
        je      short SC_10
        SETI    FxReg, 0
        jmp     short SC_11
SC_10:
        SETI    FxReg, SetBits
SC_11:
   else
        SETI    FxReg, SetBits
   endif        
Endm        

;*************************************************************
; GET_CF_OFFSET
;
; Prepares EBX with the current CMDFIFO value for SETx macros
;*************************************************************
GET_CF_OFFSET Macro SetBits
        mov     ebx, CF         ; setup hwPtr for packet programming
Endm

;*************************************************************
; GET_CF_OFFSET_FAST
;
; Prepares EBX with the current CMDFIFO value for SETx macros
; Special version of GET_CF_OFFSET for use with the optimised
; CMDFIFO_CHECKROOM_FAST
;*************************************************************
IFDEF PERF_NEW16BITBLT
GET_CF_OFFSET_FAST Macro SetBits
        mov     ebx, GLOB.mainFifo.FXptr
Endm
ENDIF

;****************************************
; BUMP
;
; Increments our hardware index counter
; Decrements our fifo dword count
;****************************************
BUMP Macro HowMany
        add     dword ptr IDX, HowMany
        ;sub     GLOB.fifoDwordCount, HowMany
Endm                                

;****************************************
; BUMP_FAST
;
; This is a special case of BUMP for use 
; with CMDFIFO_CHECKROOM_FAST 
;
; Increments our hardware index counter
; Decrements our fifo dword count
;****************************************
IFDEF PERF_NEW16BITBLT
BUMP_FAST Macro HowMany
      add   ebx, HowMany
      mov   GLOB.mainFifo.FXptr, ebx   
Endm                  
ENDIF              
                               
;*********************
; CMDFIFO_CHECKBUMP - check whether fencing needs to occur, to prevent bump overflow
;*********************
CMDFIFO_CHECKBUMP Macro HowMany
		Local	Update_bumpval
		mov		eax, dword ptr GLOB.mainFifo.bump
		add		eax, HowMany
		cmp		eax, 0ffffh
		jb		Update_bumpval
		P6FENCE
		mov		eax, HowMany
Update_bumpval:
		mov		dword ptr GLOB.mainFifo.bump, eax
Endm

;*********************
; CMDFIFO_CHECKROOM
;*********************
CMDFIFO_CHECKROOM Macro HowMany
        Local   CR_10
		CMDFIFO_CHECKBUMP HowMany
        mov     eax, IDX
        sub     GLOB.mainFifo.space, eax
        xor     edx, edx
        shl     eax, 2
        add     CF, eax
        mov     IDX, edx
        cmp     dword ptr GLOB.mainFifo.space, HowMany
        jae     short CR_10
        mov     ebx, CF
        mov     ecx, HowMany
        call    fifo_MakeRoom
        mov     CF, ebx
CR_10:
Endm
     
;************************
; CMDFIFO_CHECKROOM_FAST
;************************
IFDEF PERF_NEW16BITBLT
CMDFIFO_CHECKROOM_FAST Macro HowMany
        Local   CR_9, CR_10
		CMDFIFO_CHECKBUMP HowMany        
        sub     dword ptr GLOB.mainFifo.space, HowMany
        jl      short CR_9      ; Fall Through in majority of calls
        jmp     short CR_10
CR_9:        
        mov     ecx, HowMany
        call    fifo_MakeRoom_Fast
        sub     GLOB.mainFifo.space, ecx        
CR_10:
Endm
ENDIF
     

;***********************************************************************
; FXENTER
;
; Monster macro for handling common pre-drawing stuff
; (softare cursor exclusion, destination type (screen or device bitmap)
; checking, etc.
;
; Use the FXENTER <label to jmp on SW cursor>, SRC_PRESENT form 
;  when source is present
;
;   rbissell@stb,990128: made "OnSoftwareCursor" param optional
;                        to cater to situations where we can handle it.
;
; Register assumes:
;
;       FS:DI   = pointer to destination device (lpDst)
;       GS:SI   = pointer to source if present
;***********************************************************************

; SRC_PRESENT equ 1

LPDST   equ     fs:[di]
LPSRC   equ     gs:[si]
ifdef SLI_AA
SKIP_TO_C_FLAGS equ SDATA_GDIFLAGS_SW_CURSOR or SDATA_GDIFLAGS_SLI_AA_MASTER or SDATA_GDIFLAGS_SLI_AA_SLAVE
SKIP_TOTAL_FLAGS equ SDATA_GDIFLAGS_HWC_EXCLUSIVE or SDATA_GDIFLAGS_SLI_AA_SLAVE
else
SKIP_TO_C_FLAGS equ SDATA_GDIFLAGS_SW_CURSOR
SKIP_TOTAL_FLAGS equ SDATA_GDIFLAGS_HWC_EXCLUSIVE 
endif

FXENTER Macro   OnSoftwareCursor, SrcPresent 
        Local   FX_50, FX_60, FX_61, FX_62, FX_63, FX_16, FX_65

		;******************************
		; Need to Pre-Load the FlatSel
		;******************************
		xor		edx, edx
        mov     dx, _FlatSel
        mov     es, dx

		mov		ebx, dword ptr GLOB.lpDeFlags 
        test    word ptr es:[ebx], (BUSY + PALETTE_XLAT)  ;     goto Use_DibEng;
        jnz     Use_DibEngine
        bts     word ptr es:[ebx], BUSY_BIT      ; pDeFlags = lpDriverData->lpDeFlags;
        jc      Use_DibEngine                                     ; SET_BUSY(pDeFlags);
        
        xor     eax, eax        ; default to dst not a device bitmap
        test    word ptr LPDST.DIBENGINE.deFlags, OFFSCREEN ; we already know its VRAM :)
        setnz   al
        mov     dword ptr DSTDBM, eax
        xor     eax, eax        ; default to src not a device bitmap
     IFNB <SrcPresent>
        movzx   edx, gs:[si].DIBENGINE.deFlags  ; if ((lpSrc->deFlags & VRAM+OFFSCREEN) ==
        and     edx, (VRAM + OFFSCREEN)         ;       (VRAM+OFFSCREEN))
        cmp     edx, (VRAM + OFFSCREEN)
        jne     short FX_15
        mov     eax, 1                          ; then Yes, we have a src device bitmap
FX_15:
     ENDIF
        mov     dword ptr SRCDBM, eax
        
        cmp     GLOB.gdiFlags, 0
        je      FX_60
        
        ;mov        eax, GLOB.gdiFlags 
        test    word ptr GLOB.gdiFlags, SKIP_TOTAL_FLAGS
        jz      short FX_16
		mov		ebx, dword ptr GLOB.lpDeFlags 
        and     word ptr es:[ebx], NOT BUSY
        jmp     All_Done
FX_16:        
        test    byte ptr GLOB.gdiFlags, SDATA_GDIFLAGS_2D_DIRTY
        jz      FX_61
        
        ; if direct draw changed some 2d registers, or changed the desktop
        ; from being linear to tiled or vice-versa,
        ; we need to reset the invariant 2d regs and the stride of the
        ; screen's pDevice so that dib engine rendering will be correct

        xor     ecx, ecx
        mov     cl, (RESET_DST + RESET_SRC + RESET_PDEV) 
        call    reset_Invariants
        and     GLOB.gdiFlags, NOT (SDATA_GDIFLAGS_2D_DIRTY + SDATA_GDIFLAGS_DST_WAS_DEVBIT + SDATA_GDIFLAGS_SRC_WAS_DEVBIT)
        
FX_61:        
        
        ; if the previous drawing operation was to a device bitmap,
        ; the invariants were clobbered, so we need to reset them,
        ; but there's no need to do this if we're not going to clobber
        ; them again anyway, unless we need to save away the current
        ; dstBaseAddr and dstFormat (e.g., StretchBltBufferToScreen),
        ; so we'd need to put them to known states anyway

        test    byte ptr GLOB.gdiFlags, SDATA_GDIFLAGS_DST_WAS_DEVBIT
        jz      short FX_62
        cmp     dword ptr DSTDBM, 0
        jne     short FX_62
        xor     ecx, ecx
        mov     cl, RESET_DST
        call    reset_Invariants
        and     GLOB.gdiFlags, NOT SDATA_GDIFLAGS_DST_WAS_DEVBIT
FX_62:        

     IFNB  <SrcPresent>
        ; if the previous drawing operation used a device bitmap as a
        ; source, and the current operation doesn't use a device bitmap
        ; as a source, reset the source 2d invariants

        test    GLOB.gdiFlags, SDATA_GDIFLAGS_SRC_WAS_DEVBIT
        jz      short FX_63
        cmp     dword ptr SRCDBM, 0       ; is source a device bitmap?
        jne     short FX_63
        mov     ecx, RESET_SRC
        call    reset_Invariants
        and     GLOB.gdiFlags, NOT SDATA_GDIFLAGS_SRC_WAS_DEVBIT
     ENDIF
FX_63:
ifdef SLI_AA
        ; If SW cursor is on or SLI_AA forget this and go back to
        ; the trusty C code.
        test    word ptr GLOB.gdiFlags, SKIP_TO_C_FLAGS
        jz      short FX_65
		mov		ebx, dword ptr GLOB.lpDeFlags 
        and     word ptr es:[ebx], NOT BUSY
		jmp		OnSoftwareCursor
FX_65:
else
    IFNB <OnSoftwareCursor>
        ; If SW cursor is on, for get this and go back to
        ; the trusty C code.
        test    word ptr GLOB.gdiFlags, SKIP_TO_C_FLAGS
        jz      short FX_65
		mov		ebx, dword ptr GLOB.lpDeFlags 
        and     word ptr es:[ebx], NOT BUSY
		jmp		OnSoftwareCursor
FX_65:
    ENDIF
endif

FX_60:        
        CMDFIFO_SETUP_LITE
        
     IFNB  <SrcPresent>
        cmp     dword ptr SRCDBM, 0       ; is source a device bitmap?
        je      FX_70           ; if not, outta here!
        mov     eax, dword ptr LPSRC.DIBENGINE.deBits
        sub     eax, dword ptr GLOB.lfbBase
        mov     dword ptr SRCADDR, eax
        mov     eax, GLOB.screenFormat                                    ;  srcFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
        and     eax, NOT (SSTG_DST_LINEAR_STRIDE + SSTG_DST_TILE_STRIDE)  ;                    SSTG_DST_TILE_STRIDE);
        mov     edx, gs:[si].DIBENGINE.deDeltaScan                        ;  srcFormat |= mylpSrc->deDeltaScan & SSTG_DST_LINEAR_STRIDE;
        and     edx, SSTG_DST_LINEAR_STRIDE                               ;
        or      eax, edx
        mov     dword ptr SRCFMT, eax
        
        xor     ecx, ecx
        mov     cl, 3
        CMDFIFO_CHECKROOM      ecx 
        SETPH   (SSTCP_PKT2 + srcBaseAddrBit + srcFormatBit)
        SETM    srcBaseAddr, SRCADDR
        SETM    srcFormat, SRCFMT
        BUMP    3
        or      GLOB.gdiFlags, SDATA_GDIFLAGS_SRC_WAS_DEVBIT        
        ; DANGER
        ; missing the C enterleave FXENTER_COMPUTE_DSTBA_FORMAT stuff
        ; DANGER
FX_70:        
     ENDIF
        cmp     dword ptr DSTDBM, 0       ; is destination a device bitmap
        je      FX_50     ; if not, outta here!
        
        mov     eax, dword ptr LPDST.DIBENGINE.deBits
        sub     eax, dword ptr GLOB.lfbBase
        mov     dword ptr DSTADDR, eax
        
        mov     eax, dword ptr GLOB.screenFormat
        and     eax, 70000h                     ; & SSTG_DST_FORMAT
        mov     edx, dword ptr LPDST.DIBENGINE.deDeltaScan
        and     edx, 03fffh                         ; & SSTG_SRC_LINEAR_STRIDE
        or      eax, edx
        mov     dword ptr DSTFMT, eax

        xor     ecx, ecx
        mov     cl, 5
        CMDFIFO_CHECKROOM     ecx
        GET_CF_OFFSET
        xor     eax, eax
        mov     ax, (SSTCP_PKT2 + clip0minBit + clip0maxBit + dstBaseAddrBit + dstFormatBit)
        mov     es:[ebx], eax ;PH
        add     ebx, 4
        xor     eax, eax
        mov     es:[ebx], eax ; clip0min
        add     ebx, 4
        mov     eax, dword ptr LPDST.DIBENGINE.deWidth 
        mov     es:[ebx], eax ; clip0max
        add     ebx, 4
        mov     eax,dword ptr DSTADDR 
        mov     es:[ebx], eax ; dstBaseAddr
        add     ebx, 4
        mov     eax,dword ptr DSTFMT 
        mov     es:[ebx], eax ; dstFormat

        BUMP    5
        or      GLOB.gdiFlags, SDATA_GDIFLAGS_DST_WAS_DEVBIT
        ; DANGER
        ; missing the C enterleave FXENTER_COMPUTE_DSTBA_FORMAT stuff
        ; DANGER
FX_50:
Endm

;***********************************************************************
; FXENTER_FAST
;
; Monster macro for handling common pre-drawing stuff
; (softare cursor exclusion, destination type (screen or device bitmap)
; checking, etc.
;
; Use the FXENTER_FAST <label to jmp on SW cursor>, SRC_PRESENT form 
;  when source is present
;
;   rbissell@stb,990128: made "OnSoftwareCursor" param optional
;                        to cater to situations where we can handle it.
;
;   shouston@3dfx.com,10-7-99:  Specially optimised version of FXENTER 
;                               for bitblt and ExtTextOut
; Register assumes:
;
;       FS:DI   = pointer to destination device (lpDst)
;       GS:SI   = pointer to source if present
;***********************************************************************

IFDEF PERF_NEW16BITBLT
FXENTER_FAST Macro   OnSoftwareCursor, SrcPresent2 
        Local   FX_16, FX_50, FX_60, FX_65, FX_61, FX_62, FX_63, FX_70, FX_71, Check_Invariants

        ;******************************
        ; Need to Pre-Load the FlatSel
        ;******************************

        xor     edx, edx
        mov     dx, _FlatSel
        mov     es, dx

        mov     ebx, dword ptr GLOB.lpDeFlags 

        bts     word ptr es:[ebx], BUSY_BIT      ; pDeFlags = lpDriverData->lpDeFlags;
        jb      Use_DibEngine                    ; SET_BUSY(pDeFlags);

        xor     eax, eax        ; default to dst not a device bitmap
        test    word ptr LPDST.DIBENGINE.deFlags, OFFSCREEN ; we already know its VRAM :)
        setnz   al
        mov     dword ptr DSTDBM, eax

IFNB <SrcPresent2>
        xor     eax, eax
        test    word ptr LPSRC.DIBENGINE.deFlags, OFFSCREEN ; just test offscreen, we already know its VRAM
        setnz   al
        mov     dword ptr SRCDBM, eax

        ; TODO: This could be further optimized.... most of the time (in WB99) all Check_Invariants 
        ; will do is set the src base addr. An faster routine could be written that removes the other
        ; invariant checks, as is done for the dest invariants check.

        jmp     Check_Invariants
ELSE 
        mov     edx, GLOB.gdiFlags
;        and      edx, SDATA_GDIFLAGS_2D_DIRTY + SKIP_TOTAL_FLAGS + SDATA_GDIFLAGS_DST_WAS_DEVBIT + SDATA_GDIFLAGS_SW_CURSOR
        and     edx, SKIP_TO_C_FLAGS + SDATA_GDIFLAGS_2D_DIRTY + SDATA_GDIFLAGS_DST_WAS_DEVBIT + SKIP_TOTAL_FLAGS
ENDIF 
        or      eax, edx
        cmp     eax, 0 
        jne     Check_Invariants
        jmp     FX_50
      
Check_Invariants: 

        test    word ptr GLOB.gdiFlags, SKIP_TOTAL_FLAGS
        jnz     ExclusiveMode
      
FX_16:

        test    byte ptr GLOB.gdiFlags, SDATA_GDIFLAGS_2D_DIRTY
        jz      FX_61

        ; if direct draw changed some 2d registers, or changed the desktop
        ; from being linear to tiled or vice-versa,
        ; we need to reset the invariant 2d regs and the stride of the
        ; screen's pDevice so that dib engine rendering will be correct
      
        xor     ecx, ecx
        mov     cl, (RESET_DST + RESET_SRC + RESET_PDEV) 
        call    reset_Invariants
        and     GLOB.gdiFlags, NOT (SDATA_GDIFLAGS_2D_DIRTY + SDATA_GDIFLAGS_DST_WAS_DEVBIT + SDATA_GDIFLAGS_SRC_WAS_DEVBIT)
        
FX_61:        
        
        ; if the previous drawing operation was to a device bitmap,
        ; the invariants were clobbered, so we need to reset them,
        ; but there's no need to do this if we're not going to clobber
        ; them again anyway, unless we need to save away the current
        ; dstBaseAddr and dstFormat (e.g., StretchBltBufferToScreen),
        ; so we'd need to put them to known states anyway

        test    byte ptr GLOB.gdiFlags, SDATA_GDIFLAGS_DST_WAS_DEVBIT   ; was the last operation to offscreen? 
        jz      short FX_62         ; No, jmp
        cmp     dword ptr DSTDBM, 0 ; Current operation to onscreen?
        jne     short FX_62         ; No, jmp
        
        ; we get here if the last operation was to offscreen and the current is to onscreen
        
        xor     ecx, ecx
        mov     cl, RESET_DST
        call    reset_Invariants
        and     GLOB.gdiFlags, NOT SDATA_GDIFLAGS_DST_WAS_DEVBIT
FX_62: 
     IFNB  <SrcPresent2>
        ; if the previous drawing operation used an offscreen bitmap as a
        ; source, and the current operation doesn't use an offscreen bitmap
        ; as a source, reset the source 2d invariants

        test    GLOB.gdiFlags, SDATA_GDIFLAGS_SRC_WAS_DEVBIT    ; was the last src offscreen?
        jz      short FX_63                 ; No, jmp
        cmp     dword ptr SRCDBM, 0         ; is the src onscreen?
        jne     short FX_63                 ; No, jmp
        mov     ecx, RESET_SRC
        call    reset_Invariants
        and     GLOB.gdiFlags, NOT SDATA_GDIFLAGS_SRC_WAS_DEVBIT
     ENDIF

          

FX_63:
     IFNB <OnSoftwareCursor>
        ; If SW cursor is on, for get this and go back to
        ; the trusty C code.
;        test    byte ptr GLOB.gdiFlags, SDATA_GDIFLAGS_SW_CURSOR
        test    word ptr GLOB.gdiFlags, SKIP_TO_C_FLAGS
        jz      short FX_65
        mov     ebx, dword ptr GLOB.lpDeFlags 
        and     word ptr es:[ebx], NOT BUSY
        jmp     OnSoftwareCursor
FX_65:
    ENDIF

FX_60:        

     IFNB  <SrcPresent2>
        cmp     dword ptr SRCDBM, 0       ; is source in offscreen memory?
        je      FX_70                     ; if not, outta here!
        
        xor     ecx, ecx
        mov     cl, 2
        GET_CF_OFFSET_FAST          
        CMDFIFO_CHECKROOM_FAST       ecx 
                 
        mov     es:[ebx], dword ptr (SSTCP_PKT2 + srcBaseAddrBit)
        mov     eax, dword ptr LPSRC.DIBENGINE.deBits
        sub     eax, dword ptr GLOB.lfbBase        
        mov     es:[ebx+4], eax
        
        mov     eax, GLOB.screenFormat                                    ;  srcFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
        and     eax, NOT SSTG_SRC_LINEAR_STRIDE
        mov     edx, gs:[si].DIBENGINE.deDeltaScan                        ;  srcFormat |= mylpSrc->deDeltaScan & SSTG_DST_LINEAR_STRIDE;
        and     edx, SSTG_SRC_LINEAR_STRIDE                               
        or      eax, edx        
        mov     [SrcFormat], eax

        BUMP_FAST   8      
        or      GLOB.gdiFlags, SDATA_GDIFLAGS_SRC_WAS_DEVBIT        
        jmp     FX_71
        ; DANGER
        ; missing the C enterleave FXENTER_COMPUTE_DSTBA_FORMAT stuff
        ; DANGER
FX_70: 
        ; The src is onscreen memory, set the format correspondingly
        mov     eax, GLOB.screenFormat
        mov     [SrcFormat], eax 
FX_71:       
     ENDIF
        
        cmp     dword ptr DSTDBM, 0       ; is current operation to onscreen?
        je      FX_50     ; yes, outta here!
        
        ; we get here if the current operation is to offscreen
        
        mov     eax, dword ptr LPDST.DIBENGINE.deBits
        sub     eax, dword ptr GLOB.lfbBase
        mov     dword ptr DSTADDR, eax
        
        mov     eax, dword ptr GLOB.screenFormat
        and     eax, 70000h                     ; & SSTG_DST_FORMAT
        mov     edx, dword ptr LPDST.DIBENGINE.deDeltaScan
        and     edx, 03fffh                         ; & SSTG_SRC_LINEAR_STRIDE
        or      eax, edx
        mov     dword ptr DSTFMT, eax

        xor     ecx, ecx
        mov     cl, 5
        GET_CF_OFFSET_FAST
        CMDFIFO_CHECKROOM_FAST     ecx

        xor     eax, eax
        mov     ax, (SSTCP_PKT2 + clip0minBit + clip0maxBit + dstBaseAddrBit + dstFormatBit)
        mov     es:[ebx], eax ;PH
        xor     eax, eax
        mov     es:[ebx+4], eax ; clip0min
        mov     eax, dword ptr LPDST.DIBENGINE.deWidth 
        mov     es:[ebx+8], eax ; clip0max
        mov     eax,dword ptr DSTADDR 
        mov     es:[ebx+12], eax ; dstBaseAddr
        mov     eax,dword ptr DSTFMT 
        mov     es:[ebx+16], eax ; dstFormat
        BUMP_FAST     20

        or      GLOB.gdiFlags, SDATA_GDIFLAGS_DST_WAS_DEVBIT
        ; DANGER
        ; missing the C enterleave FXENTER_COMPUTE_DSTBA_FORMAT stuff
        ; DANGER
FX_50:

Endm FXENTER_FAST 
ENDIF ; PERF_NEW16BITBLT


FXLEAVE Macro
        Local   FXL_10 
        CMDFIFO_EPILOG
		mov		ebx, dword ptr GLOB.lpDeFlags 
        and     word ptr es:[ebx], NOT BUSY
        jmp     short All_Done
Use_DibEngine:
        test    byte ptr GLOB.gdiFlags, SDATA_GDIFLAGS_2D_DIRTY
        jz      short FXL_10
        xor     ecx, ecx
        mov     cl, RESET_PDEV
        call    reset_Invariants
FXL_10:        
All_Done:        
Endm

IFDEF PERF_NEW16BITBLT
FXLEAVE_FAST Macro
        Local   FXL_10 
ExclusiveMode:        
        mov     ebx, dword ptr GLOB.lpDeFlags 
        and     word ptr es:[ebx], NOT BUSY
        jmp     short All_Done
Use_DibEngine:
        test    byte ptr GLOB.gdiFlags, SDATA_GDIFLAGS_2D_DIRTY
        jz      short FXL_10
        xor     ecx, ecx
        mov     cl, RESET_PDEV
        call    reset_Invariants
FXL_10:        
All_Done:        
Endm
ENDIF

;
; External data that these macros need
;

_DATA   SEGMENT WORD PUBLIC 'DATA' USE16 

EXTERN  _DriverData:GLOBALDATA
ifdef STB_FIFO16_ENABLE 
EXTERN  _FlatSel:WORD
endif

_DATA   ENDS

ASSUME DS:_DATA

_TEXT   SEGMENT WORD PUBLIC 'CODE' USE16 

ifndef FIFO16FILE       ; don't include these externs in FIFO16.ASM

EXTERN  fifo_MakeRoom:NEAR
ifdef PERF_NEW16BITBLT
EXTERN  fifo_MakeRoom_Fast:NEAR
endif
EXTERN  fifo_Wrap:NEAR
EXTERN  reset_Invariants:NEAR

endif

ifdef AGP_CMDFIFO

EXTERN  myWrapAgp:FAR
EXTERN  myAgpEpilog:FAR
EXTERN  bumpAgp:FAR

endif

ifdef DEBUGFIFO

EXTERN  _mySetCF:FAR
EXTERN  _mySetPH:FAR

endif

_TEXT ENDS


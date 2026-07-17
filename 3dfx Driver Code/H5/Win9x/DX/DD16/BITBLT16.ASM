;/* $Header: /devel/h5/Win9x/dx/dd16/bitblt16.asm 11    7/20/99 5:04p Andrew $ */
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
;** $Revision: 11 $
;** $Date: 7/20/99 5:04p $
;**
;** $History: bitblt16.asm $
;; 
;; *****************  Version 11  *****************
;; User: Andrew       Date: 7/20/99    Time: 5:04p
;; Updated in $/devel/h5/Win9x/dx/dd16
;; Moved SDATA_GDIFLAGS_HWC_EXCLUSIVE check to after check to see if it is
;; a HOST BASED bitmap
;; 
;; *****************  Version 10  *****************
;; User: Andrew       Date: 6/16/99    Time: 4:43p
;; Updated in $/devel/h5/Win9x/dx/dd16
;; Added SKIP_TOTAL_FLAGS
;; 
;; *****************  Version 8  *****************
;; User: Stb_mimhoff  Date: 5/27/99    Time: 8:04a
;; Updated in $/devel/h3/win95/dx/dd16
;; Added check fo HWC_EXCLUSIVE bit to BitBlt function
;; 
;; *****************  Version 7  *****************
;; User: Stb_sjohnston Date: 4/07/99    Time: 2:51p
;; Updated in $/devel/h3/win95/dx/dd16
;; Cleaner fix for Slashdot/Wax bug (PRS 4457) from Chris Edgington
;; 
;; *****************  Version 6  *****************
;; User: Stb_shanna   Date: 3/13/99    Time: 9:09a
;; Updated in $/devel/h3/win95/dx/dd16
;; Fixed PRS 5109 by reloading trached needed registers with the dest ptr.
;; 
;; *****************  Version 5  *****************
;; User: Stb_rbissell Date: 3/12/99    Time: 2:10p
;; Updated in $/devel/h3/win95/dx/dd16
;; Fix for PRS #s 4943 and 4993
;; 
;; *****************  Version 4  *****************
;; User: Stb_sjohnston Date: 3/03/99    Time: 8:47p
;; Updated in $/devel/h3/win95/dx/dd16
;; Fix for PRS 4676 and 4681 - in 8bpp modes, use 32bit code instead of
;; optimized 16bit code
;; 
;; *****************  Version 3  *****************
;; User: Stb_pzheng   Date: 2/24/99    Time: 5:24p
;; Updated in $/devel/h3/win95/dx/dd16
;; Added fix for build problem when STBPERF_16BIT_BITBLTis not defined
;; 
;; *****************  Version 2  *****************
;; User: Stb_pzheng   Date: 2/09/99    Time: 10:16a
;; Updated in $/devel/h3/win95/dx/dd16
;; 
;; 
;**
;*/

;----------------------------------------------------------------------------
;
; bitblt16.asm
;
; ASM equivalents text routines
;
;----------------------------------------------------------------------------

incDrawMode EQU 1
        .xlist
        include shared.inc
        include gdidib.inc
        include gdidefs.inc
        include fifomgr.inc
        .list

; STB Begin change
; STB-SR 12/16/98 Added for STB Performance optimizations
ifdef INCSTBPERF
INCLUDE ..\..\BUILD\STBPERF.INC
endif
; STB END change

ifdef STB_16BIT_BITBLT

PATTERN_REG_1 EQU (100h SHR 2)
MAXCOLORDWORDS EQU 1024
_THK16TEXT   SEGMENT WORD USE16 PUBLIC 'CODE'
_THK16TEXT   ENDS

_DATA   SEGMENT WORD USE16 PUBLIC 'DATA'

; **********************************************************************
;  Table of rops
;  Bit 0 : blt uses source
;  Bit 1 : blt uses pattern
;  Bit 2 : blt uses destination
;  Bits 8-15 : Rop code to use when doing pattern blt
; **********************************************************************
                      
ifdef PERF_NEW16BITBLT
    
USES_SRC    EQU 1
USES_PAT    EQU 2
USES_DEST   EQU 4
                      
align 2

RopFlags dw 0CC00h, 7, 7, 3, 7, 1106h, 7, 7     ; 00h
		dw 7, 7, 2206h, 7, 3, 7, 7, 3302h
		dw 7, 5, 7, 7, 7, 7, 7, 7               ; 10h
		dw 7, 7, 7, 7, 7, 7, 7, 7
		dw 7, 7, 5, 7, 7, 7, 7, 7               ; 20h
		dw 7, 7, 7, 7, 7, 7, 7, 7
		dw 3, 7, 7, 1, 7, 7, 7, 7               ; 30h
		dw 7, 7, 7, 7, 3, 7, 7, 3
		dw 7, 7, 7, 7, 5, 7, 7, 7               ; 40h
		dw 7, 7, 7, 7, 7, 7, 7, 7
		dw 4406h, 7, 7, 7, 7, 5504h, 7, 7       ; 50h
		dw 7, 7, 6606h, 7, 7, 7, 7, 7706h
		dw 7, 7, 7, 7, 7, 7, 5, 7               ; 60h
		dw 7, 7, 7, 7, 7, 7, 7, 7
		dw 7, 7, 7, 7, 7, 7, 7, 5               ; 70h
		dw 7, 7, 7, 7, 7, 7, 7, 7
		dw 7, 7, 7, 7, 7, 7, 7, 7               ; 80h
		dw 5, 7, 7, 7, 7, 7, 7, 7
		dw 7, 7, 7, 7, 7, 7, 7, 7               ; 90h
		dw 7, 5, 7, 7, 7, 7, 7, 7
		dw 8806h, 7, 7, 7, 7, 9906h, 7, 7       ; A0h 
		dw 7, 7, 0AA04h, 7, 7, 7, 7, 0BB06h
		dw 7, 7, 7, 7, 7, 7, 7, 7               ; B0h
		dw 7, 7, 7, 5, 7, 7, 7, 7
		dw 3, 7, 7, 3, 7, 7, 7, 7               ; C0h
		dw 7, 7, 7, 7, 1, 7, 7, 3
		dw 7, 7, 7, 7, 7, 7, 7, 7               ; D0h
		dw 7, 7, 7, 7, 7, 5, 7, 7
		dw 7, 7, 7, 7, 7, 7, 7, 7               ; E0h
		dw 7, 7, 7, 7, 7, 7, 5, 7
		dw 0CC02h, 7, 7, 3, 7, 0DD06h, 7, 7     ; F0h
		dw 7, 7, 0EE06h, 7, 3, 7, 7, 0CC00h
		
SrcFormat   dd  0h

endif ; PERF_NEW16BITBLT

_DATA   ENDS

        EXTRN _lpDriverData:FAR PTR
        EXTRN _lph3IORegs:DWORD
        EXTRN _h3READ:FAR
        EXTRN wFlatDataSel:WORD


.586p
.MMX


_TEXT   SEGMENT WORD PUBLIC 'CODE' USE16 

;********************
; EXTERN PROTOTYPES
;********************

DIB_BitBlt          PROTO   FAR PASCAL
ifdef PERF_NEW16BITBLT
_InsertBitmapIntoCache  PROTO   FAR PASCAL
endif

IFDEF STB_16BIT_BITBLT
BitBltC             PROTO   FAR PASCAL
ENDIF

IFDEF STBPERF_CMBITBLT
        extern  _EvictBmp:FAR
ENDIF

IFNDEF SKIP_STAGE2
        extern  _DoHostStage2:FAR
ENDIF

IFDEF PERF_NEW16BITBLT
   BANSHEE_SYNC MACRO
           local   DoRead, SkipRead
           mov     ax, _DATA
           mov     ds, ax
           push    ds
           lds     si, _lpDriverData ; Global Data
           lds     si, DWORD PTR [si]         ; Dib Engine of Main Device
           test    BYTE PTR [si+1Ch], BUSY  ; Is Device Busy
           pop     ds
           jnz     short SkipRead
           push    ebx
           push    esi
           mov     esi, _lph3IORegs
           mov     bx, wFlatDataSel
           mov     ds, bx
   DoRead:
           mov     ebx, [esi]
           test    bh, 2
           jnz     short DoRead
           pop     esi
           pop     ebx
   SkipRead:
   ENDM
ENDIF ; PERF_NEW16BITBLT

;**************************
; BitBlt
;
; Handles BitBlt calls
;
;**************************


; Prepare equates for locals
COLOR       equ     (dword ptr [RBP-(NEXTLOC+00)])
DSTSIZE     equ     (dword ptr [RBP-(NEXTLOC+04)])
DSTXY       equ     (dword ptr [RBP-(NEXTLOC+08)])
ifdef PERF_NEW16BITBLT

COMMANDREG  equ     (dword ptr [RBP-(NEXTLOC+12)])
DIBDELTASCAN equ    (dword ptr [RBP-(NEXTLOC+16)])
DIBSYSADDR  equ     (fword ptr [RBP-(NEXTLOC+22)])
CURRENTROPFLAGS equ (word ptr [RBP-(NEXTLOC+24)])
XCOUNTER    equ     (word ptr [RBP-(NEXTLOC+26)])
YCOUNTER    equ     (word ptr [RBP-(NEXTLOC+28)])
LOCALS      equ     NEXTLOC+30
else

LOCALS      equ     NEXTLOC+12
endif

; Prepare equates for parameters
PBASE   equ     6       ; (far return CS:IP + BP)

lpDestDev   EQU (DWORD  PTR [RBP+(PBASE+28)])       ;--> to dest bitmap descriptor
wDestX      EQU (WORD   PTR [RBP+(PBASE+26)])       ;Destination origin - x coordinate
wDestY      EQU (WORD   PTR [RBP+(PBASE+24)])       ;Destination origin - y coordinate
lpSrcDev    EQU (DWORD  PTR [RBP+(PBASE+20)])       ;--> to source bitmap descriptor
wSrcX       EQU (WORD   PTR [RBP+(PBASE+18)])       ;Source origin - x coordinate
wSrcY       EQU (WORD   PTR [RBP+(PBASE+16)])       ;Source origin - y coordinate
wXext       EQU (WORD   PTR [RBP+(PBASE+14)])       ;x extent of the BLT
wYext       EQU (WORD   PTR [RBP+(PBASE+12)])       ;y extent of the BLT
Rop3        EQU (DWORD  PTR [RBP+(PBASE+8)])        ;Raster operation descriptor
lpPBrush    EQU (DWORD  PTR [RBP+(PBASE+4)])        ;--> to a physical brush (pattern)
lpDrawMode  EQU (DWORD  PTR [RBP+(PBASE+0)])        ;--> to a drawmode
bRop        EQU (BYTE   PTR Rop3[2])                ;Rop code is in byte 2 of Rop dword


ifdef STB_16BIT_BITBLT
BitBlt          PROC    FAR PASCAL PUBLIC
else
BitBlt16        PROC    FAR PASCAL PUBLIC
endif




        ;******************************
        ; Setup passed stack reference 
        ;******************************
        push    bp
        mov     bp, sp                      ;setup EBP for accessing stack params
        xor     esp, esp
        mov     sp,bp
        sub     sp, LOCALS                 ; Make room for locals
        push    ds
        push    esi
        push    di

ifdef PERF_NEW16BITBLT
        lfs     di, lpDestDev

; *************************
; * The DoHostStage2 Code *
; *************************
IFNDEF PERF_NEWMM
        mov     eax, lpSrcDev
        cmp     eax, 0
        je      BB16DevInfoNull_1

        lfs     di, lpSrcDev
        cmp     fs:[di].DIBENGINE.deType, 05250h     ; is this a DIBENG device type
        jnz     short BB16DevInfoNull_1                ; if not, do nothing
        mov     ax, fs:[di].DIBENGINE.deFlags        ; if so, check the flags
        and     ax, VRAM OR OFFSCREEN           ; for device bitmap settings
        cmp     ax, VRAM OR OFFSCREEN           ; is it a device bitmap?
        jnz     short BB16DevInfoNull_1           ; if not, do nothing

        cmp     word ptr fs:[di].DIBENGINE.deDriverReserved+2,0  ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        jz      short BB16DevInfoNull_1               ; if (NULL != pDqueue) 

        lds     si,fs:[di].DIBENGINE.deDriverReserved         ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        mov     ax, [si+16+8]                       ; pDevInfo = (PDEVINFO)pDqueue->pData;  
        cmp     ax, 1
        jnz     short BB16DevInfoNull_1               ; if (DV_ONHOST == pDevInfo->wStatus)
        
        ; ok, this wasn't trivially rejected, which means we must let 
        ; _DoHostStage2 do his work
        push    fs
        lds     si, lpSrcDev
        push    di                              ; Save register
        push    es                              ; Save register
        push    ds                              ; Selector of lpPDevice
        push    si                              ; Offset of lpPDevice
        call    _DoHostStage2                   ; Move it so we can be rid of it
        pop     di                              ; clean stack
        pop     di                              ; clean stack
        pop     es                              ; restore register
        pop     di                              ; restore register
        pop     fs
BB16DevInfoNull_1:

        ;Fix for PRS 5109: "A general protection fault occurs on module 3dfx16v3.drv
        ;                   when you change resolution at 1280x1024."
        ;5109 fix start
        ;Reload the destination since it was trached in the source check above
        mov     eax, lpDestDev
        cmp     eax, 0
        je      BB16DevInfoNull
        lfs     di, lpDestDev
        ;5109 fix end

        cmp     fs:[di].DIBENGINE.deType, 05250h     ; is this a DIBENG device type
        jnz     short BB16DevInfoNull                ; if not, do nothing
        mov     ax, fs:[di].DIBENGINE.deFlags        ; if so, check the flags
        and     ax, VRAM OR OFFSCREEN           ; for device bitmap settings
        cmp     ax, VRAM OR OFFSCREEN           ; is it a device bitmap?
        jnz     short BB16DevInfoNull           ; if not, do nothing

        cmp     word ptr fs:[di].DIBENGINE.deDriverReserved+2,0  ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        jz      short BB16DevInfoNull               ; if (NULL != pDqueue) 

        lds     si,fs:[di].DIBENGINE.deDriverReserved         ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        mov     ax, [si+16+8]                       ; pDevInfo = (PDEVINFO)pDqueue->pData;  
        cmp     ax, 1
        jnz     short BB16DevInfoNull               ; if (DV_ONHOST == pDevInfo->wStatus)
        
        ; ok, this wasn't trivially rejected, which means we must let 
        ; _DoHostStage2 do his work
        push    fs
        lds     si, lpDestDev
        push    di                              ; Save register
        push    es                              ; Save register
        push    ds                              ; Selector of lpPDevice
        push    si                              ; Offset of lpPDevice
        call    _DoHostStage2                   ; Move it so we can be rid of it
        pop     di                              ; clean stack
        pop     di                              ; clean stack
        pop     es                              ; restore register
        pop     di                              ; restore register
        pop     fs

BB16DevInfoNull:
ENDIF        
; ********************************
; * End of the DoHostStage2 code *
; ********************************


        cmp     fs:[di].DIBENGINE.deType, 0     ; Is the dest mono?
        je      MonoDest                  
        mov     ax, fs:[di].DIBENGINE.deFlags 
        test    ax, BUSY+PALETTE_XLAT       
        jnz     BB16PuntToDibEngine
        test    ax, VRAM
        jz      DestInSysMem

DestInVidMem:
        
        mov     ax, _DATA       
        mov     ds, ax          

;       cmp     fs:[di].DIBENGINE.deBitsPixel, 8
;       jz      BB16JumpBacktoC
    
        ; Look up the rop table to find out if the blt uses src/pat/dest
        movzx   ebx, byte ptr [Rop3+2]
        mov     ax, word ptr RopFlags[ebx*2]
        mov     [CURRENTROPFLAGS], ax
        test    ax, USES_SRC 
        jnz     BltUsesSrc

        movsx   esi, byte ptr [Rop3+2]
        test    al, USES_PAT    ; Is pattern used?
        jz      BB16PatCopyCont    ; we are doing a black/whiteness blt
        
        ;************************
        ; PATCOPY Case
        ;************************
BB16DoPatCopy:
        ; Check to see if we have a solid
        ; rect fill condtion.  If not
        ; Let 32-bit side handle it

        xor     esi, esi
        lgs     si, lpPBrush                ;Let's check the brush

        mov     ax, gs:[si].DIB_Brush8.dp8BrushStyle
        mov     cl, gs:[si].DIB_Brush8.dp8BrushFlags                 
                
        cmp     ax, BS_SOLID
        jne     NotSolidBrush        
        
        test    cl, COLORSOLID      
        jz      ColourPatBlt
        
        mov     esi, DWORD PTR gs:[si].DIB_Brush8.dp8BrushBits

BB16PatCopyCont:
        ; PRS 4457 - web browser hang - hardware WAX bug
        movzx   eax, wDestY                     ; need to save the lastY because 16bit FXENTER cannot  
        mov     GLOB.lastY, eax                 ; (it does not have the destination bounds)            
        
        ;****************************************
        ; Prepare FIFO access
        ;****************************************
        FXENTER_FAST BB16JumpBacktoC 
        
        xor     ecx, ecx
        mov     cl, 5        
        GET_CF_OFFSET_FAST        
        CMDFIFO_CHECKROOM_FAST       ecx

        mov     dword ptr es:[ebx], (SSTCP_PKT2 + colorForeBit + dstSizeBit + dstXYBit + commandBit)
        mov     es:[ebx+4], esi                
        mov     eax, DWORD PTR [wYext]
        mov     edx, DWORD PTR [wDestY]
        ror     eax, 10h
        ror     edx, 10h
        movzx   ecx, byte ptr [CURRENTROPFLAGS+1]
        shl     ecx, 18h
        or      ecx, SSTG_GO + SSTG_RECTFILL  
        mov     es:[ebx+8], eax           ; dstSize
        mov     es:[ebx+12], edx          ; dstXY
        mov     es:[ebx+16], ecx
        
        BUMP_FAST   20

BB16DoneSuccessLeave:        
        FXLEAVE_FAST

BB16DoneSuccess:
        mov     ax,1

;*************************
; Hey, We're DONE!
; Return to GDI ASAP
;*************************
BB16Exit:
        ;all done - pop saved registers

        pop     di
        pop     esi
        pop     ds
        xor     esp, esp
        lea     sp,[bp]
        pop     bp

        retf    32 
        
        
; **********************************************
; * Code to handle blts which use a source DIB *
; **********************************************                     

BltUsesSrc:           
;        jmp     BB16JumpBacktoC
        
        lgs     si, [lpSrcDev]
        cmp     gs:[si].DIBENGINE.deType, 0
        je      BB16JumpBacktoC             ; Mono src        
        
        ; PRS #1696 - check that colordepths match
        mov     al, gs:[si].DIBENGINE.deBitsPixel
        cmp     al, fs:[di].DIBENGINE.deBitsPixel
        jne     BB16PuntToDibEngine
        
        test    gs:[si].DIBENGINE.deFlags, VRAM
        jz      BB16JumpBacktoC     ; For now we are passing back to C if the src is in sys mem        
        
        mov     ax, [wSrcX]         ; ax = left        
        mov     bx, [wSrcY]         ; bx = top
        mov     cx, ax
        mov     dx, bx
        add     cx, [wXext]         ; cx = right        
        jle     BB16DoneSrcBlt
        add     dx, [wYext]         ; dx = bottom
        jle     BB16DoneSrcBlt        
        
        cmp     ax, 0
        jl      ClipLeft
LeftClipped:
        cmp     bx, 0
        jl      ClipTop
TopClipped:

        sub     cx, gs:[si].DIBENGINE.deWidth   ; Is right > deWidth
        jg      ClipRight   ; If so, clip
RightClipped:
        sub     dx, gs:[si].DIBENGINE.deHeight  ; Is bottom > deHeight
        jg      ClipBottom  ; If so, clip
BottomClipped:

        xor     edx, edx            ; Clear the CommandReg local variable
        mov     [COMMANDREG], edx

        FXENTER_FAST    BB16JumpBacktoC, 1        

        ; PRS 4457 - web browser hang - hardware WAX bug
        
        movzx   eax, [wSrcY]
        mov     ebx, [GLOB.lastY]
        
        movzx   ecx, ax
        add     cx, [wYext]        
        mov     [GLOB.lastY], ecx   ; Save the current LastY
        
        xor     edx, edx
        cmp     eax, ebx    
        setle   dl          ; flag if srcY <= lastY
        
        sub     ebx, 8      ; lastY - 8 
        cmp     eax, ebx
        setge   dh          ; flag if srcY >= (lastY - 8)
        
        cmp     dx, 0101h       ; if both conditions were met
        je      Insert3DNOP     ; do the WAX bug workaround. 

NOPInserted:    
                   
        test    [CURRENTROPFLAGS], USES_PAT ; Blt uses Src and Pat?
        jz      short @f                    
        
        ; If so, handle the pattern first
                
        xor     esi, esi
        cmp     [lpPBrush], esi
        je      ResetBusyAndPunt    ; Mark we are no longer busy before punting
        lgs     si, [lpPBrush]        
        call    HandleBrush
        jz      BB16JumpBacktoC     ; HandleBrush just can't handle it
        cmp     ax, 2               ; If the return value is 2
        je      @f                  ; Don't set MonoPattern in command reg
        or      [COMMANDREG], SSTG_MONO_PATTERN ; Use pattern in blt below
@@:
        xor     ecx, ecx
        mov     cl, 6
        GET_CF_OFFSET_FAST
        CMDFIFO_CHECKROOM_FAST  ecx

        ; Check to see if the src and dest rects are in the same bitmap. If so we need
        ; to set the blt direction correspondingly.
        ; NOTE: is it enough to compare the src and dest addresses? Are there cases 
        ; where these are different but still point to the same bmp?
        mov     edx, DWORD PTR [wSrcY]
        mov     ecx, DWORD PTR [wDestY]
                
        mov     eax, [lpDestDev]
        cmp     eax, [lpSrcDev]
                
        je      SetBltDirection

DirSet:   
                
        mov     dword ptr es:[ebx], (SSTCP_PKT2 + srcFormatBit + srcXYBit + dstSizeBit + dstXYBit + commandBit)
        mov     eax, SrcFormat                                     
        mov     es:[ebx+4], eax
        mov     eax, dword ptr [wYext]
        ror     edx, 16
        ror     eax, 16
        ror     ecx, 16
        mov     es:[ebx+8], edx     ; SrcXY
        mov     es:[ebx+12], eax    ; dstSize
        mov     es:[ebx+16], ecx    ; dstXY
        movzx   eax, byte ptr [Rop3+2]
        shl     eax, 24        
        or      eax, (SSTG_GO + SSTG_BLT)
        or      eax, [COMMANDREG]   ; or in DirBit and MonoPattern bits if necessary             
        mov     es:[ebx+20], eax
        BUMP_FAST   24
        jmp     BB16DoneSuccessLeave
                                              
; **************************************************************************
; * Clipping code. For proper coding standards these should be subroutines *
; * but for speed we are jumping around.                                   *
; **************************************************************************

ClipLeft:
        sub     [wDestX], ax
        mov     [wXext], cx
        mov     [wSrcX], 0
        jmp     LeftClipped
         
ClipTop:
        sub     [wDestY], bx
        mov     [wYext], dx
        mov     [wSrcY], 0
        jmp     TopClipped         
        
ClipRight:
        sub     [wXext], cx     ; sub the amount right is > deWidth
        jle     BB16DoneSrcBlt  
        jmp     RightClipped
        
ClipBottom:
        sub     [wYext], dx
        jle     BB16DoneSrcBlt
        jmp     BottomClipped
        

; ***************************************************************
; * This code decides in which direction the blt should be done *
; * based on the positions of the src and dest                  *
; ***************************************************************

SetBltDirection:
        ; edx - wSrc
        ; ecx - wDest
        
        ; Is the destination above the src?
        cmp     cx, dx
        jl      DirSet  ; yes, use positive Y direction
        
        je      @f
        
        ; Src is above the dest
        ; Do the blt from the bottom up (neg Y direction)
        or      [COMMANDREG], SSTG_YDIR 
        add     dx, [wYext]  ; SrcY + Yext - 1
        dec     dx
        
        add     cx, [wYext]  ; DestY + Yext -1
        dec     cx
        jmp     DirSet
        
@@:
        ror     edx, 16
        ror     ecx, 16
                
        cmp     cx, dx
        jle     @f
        
        ; The dest is to the right of the src and at the same Y pos
        ; Use neg X direction
        or      [COMMANDREG], SSTG_XDIR
        add     dx, [wXext]
        dec     dx
        add     cx, [wXext]
        dec     cx

@@:  
        ; Restore X values to upper 16-bits
        ror     edx, 16
        ror     ecx, 16
        jmp     DirSet
                              

BB16DoneSrcBlt: 
        ; This is here so that conditional jumps that should almost never 
        ; be taken are forward references and hence predicted not taken.
        jmp     BB16DoneSuccess      

                                      
; *************************************************************************************
; * The destination bitmap is mono. Evict the source bitmap from the offscreen cache. *
; * The reasoning behind this is there is a good chance the source will be used       *
; * for another VtoM blt. If the src is in system memory this operation will          *
; * execute much faster due to the long time it takes to read from video memory.      *
; *************************************************************************************
 
MonoDest:        
;        jmp     BB16JumpBacktoC
        
        cmp     word ptr lpSrcDev+2, 0          ; Is there a src dev?
        jz      BB16PuntToDibEngine             ; if not Punt

        lgs     si, lpSrcDev
        cmp     gs:[si].DIBENGINE.deType, 0     ; Is the src mono?
        jz      BB16PuntToDibEngine             ; if so, Punt
        
        test    byte ptr gs:[si].DIBENGINE.deFlags, 8 ; OFFSCREEN?
        jz      BB16PuntToDibEngine             ; if not, Punt

IFDEF PERF_NEWMM
        BANSHEE_SYNC    ; Using the old memory manager we wait for idle in EvictBmp
ENDIF        
        
        push    gs                              ; Selector of lpPDevice
        push    si                              ; Offset of lpPDevice
        
        ; Note: If we go back to using the old memory manager we should be checking if the
        ; bitmap is already on the host 
        call    _EvictBmp                       ; Evict this bitmap   
        pop     edi                             ; Clean Stack

        mov     gs:[si].DIBENGINE.deReserved1, 1    ; Flag this bitmap as recacheable        
        jmp     BB16PuntToDibEngine
        
        
; ************************************************************************
; * If the destination is in system memory we recache it before carrying *
; * out the blt. The reason for this is to get bitmaps that were used    *
; * for Video to Mono blts back into offscreen memory.                   *
; ************************************************************************

DestInSysMem:
;        jmp     BB16JumpBacktoC
        
        ; We reach here if the dst is colour and in sys mem
        xor     eax, eax
        cmp     lpSrcDev, eax       ; Do we have a src
        jz      BB16PuntToDibEngine ; Punt if not
        
        lgs     si, lpSrcDev
        cmp     gs:[si].DIBENGINE.deType, ax ; Is the src mono?
        jz      BB16PuntToDibEngine ; Punt if so
        
        ; Is the src in VRAM?
        test    byte ptr gs:[si].DIBENGINE.deFlags+1, 80h
        jz      BB16PuntToDibEngine ; No, Punt C->C blts
        
        cmp     byte ptr fs:[di].DIBENGINE.deBitsPixel, 8
        jb      BB16PuntToDibEngine
                        
        ; The Src is in vid mem
        ; The dest is colour and is in sys mem
        ; Re-cache the destination
        ; This increases the time spent in S->C and V->C but
        ; allows us to re-cache bitmaps which were evicted for
        ; Video to Mono blts. 
                
        cmp     fs:[di].DIBENGINE.deDeltaScan, 0    ; Negative deltaScan
        jl      BB16PuntToDibEngine                 ; If so, punt

        cmp     fs:[di].DIBENGINE.deReserved1, 1    ; Did we evict this bitmap in a VtoM blt or a cache depopulate?
        jne     BB16JumpBacktoC                     ; If not, we won't recache it
                        
        mov     eax, dword ptr fs:[di].DIBENGINE.deBits ; Store the system memory address of the bits
        mov     cx, word ptr fs:[di].DIBENGINE.deBits+4 ; before recaching
        mov     dword ptr [DIBSYSADDR], eax
        mov     word ptr [DIBSYSADDR+4], cx
        mov     eax, fs:[di].DIBENGINE.deDeltaScan
        mov     [DIBDELTASCAN], eax
        
        mov     ax, _DATA
        mov     ds, ax                
               
        push    di      ; Save reg
        push    es      ; Save reg
        push    lpDestDev
        call    _InsertBitmapIntoCache
        pop     ecx     ; clean stack
        pop     es      ; restore reg
        pop     di      ; restore reg
        
        jz      BB16PuntToDibEngine     ; Weren't able to re-cache bitmap
                
        ; Is the whole of the destination bitmap used in the upcoming blt operation?
        ; If not we need to copy the bits into the cache
        
;        lfs     di, [lpDestDev]        
        mov     eax, dword ptr fs:[di].DIBENGINE.deWidth    ; Width:Height in eax
        ror     eax, 16
        cmp     dword ptr [wYext], eax  ; compare with wYext:wXext
        jne     CopyDIBBits
        
        ; Is the destination actually used in the blt? If not, there is no need to copy
        ; the bits into the cache.
        movzx   eax, byte ptr [Rop3+2]
        test    word ptr RopFlags[eax*2], USES_DEST 
        jz      DestInVidMem                        

; ***********************************************************************
;  The destination is used in the impending blt operation. Copy the bits
;  of the bitmap 'skeleton' we have just recached into the cache.                            
; ***********************************************************************
CopyDIBBits:           

        ; The following blt code will only ever copy bits from system 
        ; memory into the offscreen cache. Therefore it is not necessary
        ; to check for SW cursor exclustion in FXENTER_FAST. Doing so
        ; will actually result in jumping to the C code before the bits
        ; have been transferred. Fix for PRS 13178.
        
        FXENTER_FAST 
              
        xor     ecx, ecx
        mov     cl, 6
        GET_CF_OFFSET_FAST
        CMDFIFO_CHECKROOM_FAST  ecx
        
        mov     es:[ebx], dword ptr (SSTCP_PKT2 + srcFormatBit + srcXYBit + dstSizeBit + dstXYBit + commandBit)
                                                                  
        mov     eax, GLOB.screenFormat
        and     eax, NOT SSTG_SRC_LINEAR_STRIDE
        mov     edx, fs:[di].DIBENGINE.deDeltaScan
        and     edx, SSTG_SRC_LINEAR_STRIDE
        or      edx, eax

        mov     es:[ebx+4], edx
        xor     eax, eax
        mov     es:[ebx+8], eax
        mov     edx, dword ptr fs:[di].DIBENGINE.deWidth
        mov     es:[ebx+12], edx
        mov     es:[ebx+16], eax
        mov     es:[ebx+20], dword ptr (0CC000000h + SSTG_HOST_BLT)
        BUMP_FAST   24
        
        or      GLOB.gdiFlags, SDATA_GDIFLAGS_SRC_WAS_DEVBIT    ; We've changed the SrcFormat register, flag that it
                                                                ; will need to be reset next time the src is onscreen                                                                                                                  
        cld
        movzx   eax, fs:[di].DIBENGINE.deWidthBytes
        movzx   ecx, fs:[di].DIBENGINE.deHeight
        mul     ecx
        shr     eax, 2      ; Convert bytes to DWORDS
        
        ; TODO: Check if tweaking MAXCOLORDWORDS effects performance 
        cmp     eax, MAXCOLORDWORDS
        jge     LineByLineCopy
               
        mov     ecx, eax
        inc     ecx
        GET_CF_OFFSET_FAST
        CMDFIFO_CHECKROOM_FAST  ecx
        mov     edx, ecx        ; Save the number of DWORDS to bump
        dec     ecx
        mov     eax, ecx
        shl     eax, SSTCP_PKT1_NWORDS_SHIFT
        or      eax, (SSTCP_PKT1 + SSTCP_PKT1_2D + (LAUNCH_REG_1 SHL SSTCP_REGBASE_SHIFT) )
        mov     es:[ebx], eax
        lgs     esi, [DIBSYSADDR]
        push    edi
        lea     edi, [ebx+4]
        rep     movsd es:[edi], gs:[esi]
        pop     edi        
        shl     edx, 2
        BUMP_FAST   edx   
        
        mov     ebx, dword ptr GLOB.lpDeFlags
        and     word ptr es:[ebx], NOT BUSY     
        jmp     DestInVidMem

LineByLineCopy:         
        mov     ax, fs:[di].DIBENGINE.deHeight
        mov     [YCOUNTER], ax
        lgs     esi, [DIBSYSADDR]

        mov     ecx, fs:[di].DIBENGINE.deDeltaScan
        mov     ecx, [DIBDELTASCAN]
        shr     ecx, 2         
        mov     [XCOUNTER], cx
                        
        push    edi
        GET_CF_OFFSET_FAST
                
LineCopyLoop:      
        movzx   ecx, [XCOUNTER]  
        inc     ecx       
        CMDFIFO_CHECKROOM_FAST  ecx
        mov     edx, ecx        ; Save number of DWORDS to bump
        dec     ecx
        mov     eax, ecx
        shl     eax, SSTCP_PKT1_NWORDS_SHIFT
        or      eax, (SSTCP_PKT1 + SSTCP_PKT1_2D + (LAUNCH_REG_1 SHL SSTCP_REGBASE_SHIFT))
        mov     es:[ebx], eax
        
        lea     edi, [ebx+4]
        rep     movsd es:[edi], gs:[esi]        
        shl     edx, 2
        BUMP_FAST   edx
        dec     [YCOUNTER]
        jnz     LineCopyLoop   
                
        pop     edi 
        mov     ebx, dword ptr GLOB.lpDeFlags
        and     word ptr es:[ebx], NOT BUSY     
        jmp     DestInVidMem
        
        
; ***************************************************************
; * Code to handle pattern blts that do not have a solid colour *
; ***************************************************************
         
NotSolidBrush:         
        cmp     ax, 5           ; This is Imhoff's fix for PRS 7293
        ja      BB16Exit        
               
        cmp     ax, BS_PATTERN
        jne     BB16PuntToDibEngine
        
        ; PRS 4457 - web browser hang - hardware WAX bug
        movzx   eax, wDestY                     ; need to save the lastY because 16bit FXENTER cannot  
        add     ax, wYext         
        mov     GLOB.lastY, eax                 ; (it does not have the destination bounds)                    
        
        test    cl, PATTERNMONO
        jz      ColourPatBlt
        
MonoPatBlt:
        ; 
        ; 1st load the pattern
        ;
        FXENTER_FAST BB16JumpBacktoC                
        xor     ecx, ecx
        mov     cl, 8 
        GET_CF_OFFSET_FAST        
        CMDFIFO_CHECKROOM_FAST ecx
        mov     dword ptr es:[ebx], (SSTCP_PKT2 + pattern0aliasBit + pattern1aliasBit + colorBackBit + colorForeBit + dstSizeBit + dstXYBit + commandBit)
        movzx   eax, word ptr gs:[esi].DIB_Brush8.dp8BrushMono+11
        shl     eax, 16
        movzx   edx, word ptr gs:[esi].DIB_Brush8.dp8BrushMono+3
        or      eax, edx
        mov     es:[ebx+4], eax

        movzx   eax, word ptr gs:[esi].DIB_Brush8.dp8BrushMono+27
        shl     eax, 16
        movzx   edx, word ptr gs:[esi].DIB_Brush8.dp8BrushMono+19
        or      eax, edx
        mov     es:[ebx+8], eax
        
        lgs     si, lpDrawMode
        mov     eax, gs:[si].DRAWMODE.bkColor
        mov     edx, gs:[si].DRAWMODE.TextColor
        
        mov     es:[ebx+12], edx
        mov     es:[ebx+16], eax        
        ;
        ; Then do the blt
        ;  
        mov     eax, DWORD PTR [wYext]
        mov     edx, DWORD PTR [wDestY]
        ror     eax, 16
        ror     edx, 16
        movzx   ecx, byte ptr [Rop3+2]
        mov     es:[ebx+20], eax ; dstSize        
        shl     ecx, 24
        mov     es:[ebx+24], edx ; dstXY
        or      ecx, SSTG_GO + SSTG_RECTFILL + SSTG_MONO_PATTERN
        mov     es:[ebx+28], ecx    ; command
        BUMP_FAST   32
        jmp     BB16DoneSuccessLeave        
         
                
        ;
        ; **** Colour Pat blt ****
        ;  
        
ColourPatBlt:           
        ;
        ; 1st download the pattern
        ;
        FXENTER_FAST BB16JumpBacktoC        
        ; Fix for PRS 1835
        movzx   ecx, gs:[si].DIB_Brush8.dp8BrushBpp
        cmp     cl, 20
        jne     @f
        mov     cl, 16
@@:
        cld
        lea     si, gs:[si].DIB_Brush8.dp8BrushBits        
        shl     ecx, 1  ; ((bpp * 64) >> 3) >> 2. This is number of DWORDs in pattern
        inc     ecx
        GET_CF_OFFSET_FAST
        CMDFIFO_CHECKROOM_FAST   ecx
        
        dec     ecx
        mov     eax, ecx
        shl     eax, SSTCP_PKT1_NWORDS_SHIFT
        or      eax, (SSTCP_PKT1 + SSTCP_PKT1_2D + SSTCP_INC + (PATTERN_REG_1 SHL SSTCP_REGBASE_SHIFT)) 
        mov     es:[ebx], eax
        add     ebx, 4
        
        mov     edi, ebx
        rep movsd dword ptr es:[edi], dword ptr gs:[esi]
        mov     ebx, edi
        
        ; 
        ; Then do the blt
        ; 
        
        xor     ecx, ecx
        mov     cl, 4
        CMDFIFO_CHECKROOM_FAST ecx
        
        mov     dword ptr es:[ebx], (SSTCP_PKT2 + dstSizeBit + dstXYBit + commandBit)
        mov     eax, DWORD PTR [wYext]
        mov     edx, DWORD PTR [wDestY]
        ror     eax, 16
        ror     edx, 16
        movzx   ecx, byte ptr [Rop3+2]
        mov     es:[ebx+4], eax ; dstSize        
        shl     ecx, 24
        mov     es:[ebx+8], edx ; dstXY
        or      ecx, SSTG_GO + SSTG_RECTFILL
        mov     es:[ebx+12], ecx    ; command
        BUMP_FAST   16
        jmp     BB16DoneSuccessLeave
        
; **********************************************************************
                      
Insert3DNOP:
        xor     ecx, ecx
        mov     cl, 2
        GET_CF_OFFSET_FAST
        CMDFIFO_CHECKROOM_FAST  ecx
        ; The following packet is (mask (1) << SSTCP_PKT4_MASK_SHIFT) | SSTCP_PKT4_NOPCMD | SSTCP_PKT4 | chip (0) << 11
        mov     es:[ebx], DWORD PTR (8000h + (48h SHL SSTCP_REGBASE_SHIFT) + SSTCP_PKT4)
        xor     eax, eax
        mov     es:[ebx+4], eax
        BUMP_FAST   8
        jmp     NOPInserted

;        
; If FXENTER has been called and we now need to punt, the busy flag must be cleared
; first or we will stall waiting for it to clear later.
;
ResetBusyAndPunt:
        mov     ebx, dword ptr GLOB.lpDeFlags
        and     word ptr es:[ebx], NOT BUSY     
        jmp     BB16PuntToDibEngine                
        
; ******************************
; * END OF THE NEW BITBLT CODE *
; ******************************
       
else    ; PERF_NEW16BITBLT
       
        ;******************************
        ; If not VRAM, BUSY 
        ;    or PALETTE_XLAT, Punt!
        ;******************************
        lfs     di, lpDestDev

IFDEF STBPERF_CMBITBLT
        cmp     fs:[di].DIBENGINE.deType, 0  
        jne     short BB16DestDev_Check 

        cmp     fs:[di].DIBENGINE.deBitsPixel, 1
        jnz     short BB16DestDev_Check  
 
        cmp     word ptr lpSrcDev+2, 0
        jz      short BB16DestDev_Check 

        lds     si, lpSrcDev
        cmp     [si].DIBENGINE.deType, 05250h           ; is this a DIBENG device type
        jnz     short BB16DestDev_Check                 ; if not, do nothing

        mov     ax, [si].DIBENGINE.deFlags              ; if so, check the flags
        and     ax, VRAM OR OFFSCREEN                   ; for device bitmap settings
        cmp     ax, VRAM OR OFFSCREEN                   ; is it a device bitmap?
        jnz     short BB16DestDev_Check                 ; if yes, do nothing

        cmp     [si].DIBENGINE.deBitsPixel, 16
        jl      short BB16DestDev_Check 

        cmp     word ptr [si].DIBENGINE.deDriverReserved+2,0    ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        jz      short BB16DestDev_Check                 ; if (NULL != pDqueue) 

        lds     si, [si].DIBENGINE.deDriverReserved     ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        
        mov     ax, [si+16+8]
        cmp     ax, 2                                   ; if (DV_OFFSCREEN == pDevInfo->wStatus)
IFDEF SKIP_STAGE2
        jnz     short BB16DestDev_Check                           
ELSE
        jz      @f
        cmp     ax, 1                                   ; if (DV_ONHOST == pDevInfo->wStatus)
        jz      @f
        jmp     short BB16DestDev_Check                           
@@:        
ENDIF
        ; ok, this wasn't trivially rejected, which means we must let 
        ; _DoHostStage2 do his work
        lds     si, lpSrcDev
        push    di                              ; Save register
        push    es                              ; Save register
        push    ds                              ; Selector of lpPDevice
        push    si                              ; Offset of lpPDevice
IFDEF SKIP_STAGE2
        call    _EvictBmp                       ; Evict this bitmap
ELSE  ; SKIP_STAGE2
        cmp     ax, 1                           ; if this bitmap is already supposed be on host
        jnz     DoEvict                         ; then call DoHostStage2 and skip EvictBmp
        call    _DoHostStage2                   ; Move it so we can be rid of it
        jmp     CMDone
DoEvict:
        call    _EvictBmp                       ; Evict this bitmap
CMDone:
ENDIF ; SKIP_STAGE2
        pop     di                              ; clean stack
        pop     di                              ; clean stack
        pop     es                              ; restore register
        pop     di                              ; restore register
        jmp     BB16PuntToDibEngine

BB16DestDev_Check:
endif  ; idfef STBPERF_CMBITBLT 


IFNDEF SKIP_STAGE2

		mov		eax, lpSrcDev
		cmp		eax, 0
		je		BB16DevInfoNull_1

		lfs     di, lpSrcDev
		cmp     fs:[di].DIBENGINE.deType, 05250h     ; is this a DIBENG device type
        jnz     short BB16DevInfoNull_1                ; if not, do nothing
        mov     ax, fs:[di].DIBENGINE.deFlags        ; if so, check the flags
        and     ax, VRAM OR OFFSCREEN           ; for device bitmap settings
        cmp     ax, VRAM OR OFFSCREEN           ; is it a device bitmap?
        jnz     short BB16DevInfoNull_1           ; if not, do nothing

        cmp     word ptr fs:[di].DIBENGINE.deDriverReserved+2,0  ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        jz      short BB16DevInfoNull_1               ; if (NULL != pDqueue) 

        lds     si,fs:[di].DIBENGINE.deDriverReserved         ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        mov     ax, [si+16+8]                       ; pDevInfo = (PDEVINFO)pDqueue->pData;  
        cmp     ax, 1
        jnz     short BB16DevInfoNull_1               ; if (DV_ONHOST == pDevInfo->wStatus)
        
        ; ok, this wasn't trivially rejected, which means we must let 
        ; _DoHostStage2 do his work
		push	fs
        lds     si, lpSrcDev
        push    di                              ; Save register
        push    es                              ; Save register
        push    ds                              ; Selector of lpPDevice
        push    si                              ; Offset of lpPDevice
        call    _DoHostStage2                   ; Move it so we can be rid of it
        pop     di                              ; clean stack
        pop     di                              ; clean stack
        pop     es                              ; restore register
        pop     di                              ; restore register
		pop		fs
BB16DevInfoNull_1:

		;Fix for PRS 5109: "A general protection fault occurs on module 3dfx16v3.drv
		;                   when you change resolution at 1280x1024."
		;5109 fix start
		;Reload the destination since it was trached in the source check above
		mov		eax, lpDestDev
		cmp		eax, 0
		je		BB16DevInfoNull
		lfs     di, lpDestDev
		;5109 fix end

        cmp     fs:[di].DIBENGINE.deType, 05250h     ; is this a DIBENG device type
        jnz     short BB16DevInfoNull                ; if not, do nothing
        mov     ax, fs:[di].DIBENGINE.deFlags        ; if so, check the flags
        and     ax, VRAM OR OFFSCREEN           ; for device bitmap settings
        cmp     ax, VRAM OR OFFSCREEN           ; is it a device bitmap?
        jnz     short BB16DevInfoNull           ; if not, do nothing

        cmp     word ptr fs:[di].DIBENGINE.deDriverReserved+2,0  ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        jz      short BB16DevInfoNull               ; if (NULL != pDqueue) 

        lds     si,fs:[di].DIBENGINE.deDriverReserved         ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        mov     ax, [si+16+8]                       ; pDevInfo = (PDEVINFO)pDqueue->pData;  
        cmp     ax, 1
        jnz     short BB16DevInfoNull               ; if (DV_ONHOST == pDevInfo->wStatus)
        
        ; ok, this wasn't trivially rejected, which means we must let 
        ; _DoHostStage2 do his work
		push	fs
        lds     si, lpDestDev
        push    di                              ; Save register
        push    es                              ; Save register
        push    ds                              ; Selector of lpPDevice
        push    si                              ; Offset of lpPDevice
        call    _DoHostStage2                   ; Move it so we can be rid of it
        pop     di                              ; clean stack
        pop     di                              ; clean stack
        pop     es                              ; restore register
        pop     di                              ; restore register
		pop		fs

BB16DevInfoNull:
ENDIF

        mov     dx, fs:[di].DIBENGINE.deFlags
        xor     dx, VRAM                        
        test    dx,VRAM+BUSY+PALETTE_XLAT
        jnz     BB16PuntToDibEngine
		
        mov     ax, _DATA					; Always check for Glide active
        mov     ds, ax						; before accessing hardware
	test    word ptr GLOB.gdiFlags, SKIP_TOTAL_FLAGS
	jnz		BB16Exit
 
	cmp		fs:[di].DIBENGINE.deBitsPixel, 8
	jz		BB16JumpBacktoC


        mov     cl,bRop                 ;CL=Rop
        
        ;*************************
        ; Check for Special case
        ; PATCOPY Blits
        ;*************************
        cmp     cl,0F0h                        
        je      BB16DoPatCopy
        
        cmp     cl,0FFh                        
        je      BB16DoWhiteness

        cmp     cl,00h                        
        je      BB16DoBlackness

        jmp     BB16JumpBacktoC 

        ;************************
        ; Blackness case
        ;************************
BB16DoBlackness:
        xor     edx, edx
        mov     COLOR, edx                      ; EDX = FFFFFFFFh
        jmp     short BB16PatCopyCont

        ;************************
        ; Whiteness case
        ;************************
BB16DoWhiteness:
        xor     edx, edx
        dec     edx
        mov     COLOR, edx                      ; EDX = FFFFFFFFh
        jmp     short BB16PatCopyCont
        
        ;************************
        ; PATCOPY Case
        ;************************
BB16DoPatCopy:
        ; Check to see if we have a solid
        ; rect fill condtion.  If not
        ; Let 32-bit side handle it

        lds     si, lpPBrush                ;Let's check the brush
        mov     al, byte ptr [si].DIB_Brush8.dp8BrushStyle
        cmp     al, BS_HOLLOW

		cmp     al, 5	 ; Imhoff - Fix for PRS 7293
		ja      BB16Exit

        je      BB16DoneSuccess
        cmp     al, BS_SOLID
        jne     BB16JumpBacktoC 

        test    BYTE PTR [si], COLORSOLID
        jz      BB16JumpBacktoC
        mov     edx, DWORD PTR [si].DIB_Brush8.dp8BrushBits; dp8BrushBits
        mov     COLOR, edx                      ; EDX = FFFFFFFFh
    
BB16PatCopyCont:
       

        ;********************************
        ; Get the Dest size and position
        ;********************************
        mov     ax,  wDestX 
        mov     word ptr DSTXY, ax
        mov     ax,  wDestY 
        mov     word ptr DSTXY+2, ax

        mov     ax,  wXext
        mov     word ptr DSTSIZE, ax
        mov     ax,  wYext
        mov     word ptr DSTSIZE+2, ax

        ;******************************
        ; Load our data segment
        ;******************************
        xor     eax, eax
        mov     ax, _DATA
        mov     ds, eax

        ; PRS 4457 - web browser hang - hardware WAX bug 
        movzx   eax, wDestY                     ; need to save the lastY because 16bit FXENTER cannot  
        mov     GLOB.lastY, eax                 ; (it does not have the destination bounds)            
        
        ;****************************************
        ; Prepare FIFO access
        ;****************************************
        CMDFIFO_PROLOG
        FXENTER BB16JumpBacktoC 
        
        xor     ecx, ecx
        mov     cl, 5
        CMDFIFO_CHECKROOM       ecx
        GET_CF_OFFSET
        mov     dword ptr es:[ebx], (SSTCP_PKT2 + colorForeBit + dstSizeBit + dstXYBit + commandBit)
        add     ebx, 4
        mov     eax, COLOR 
        mov     es:[ebx], eax           ; colorFore
        add     ebx, 4
        mov     eax, DSTSIZE
        mov     es:[ebx], eax           ; dstSize
        add     ebx, 4
        mov     eax, DSTXY
        mov     es:[ebx], eax           ; dstXY
        add     ebx, 4
        mov     eax, (SSTG_ROP_SRCCOPY + SSTG_GO + SSTG_RECTFILL)
        mov     es:[ebx], eax           ;  command
        BUMP    5

BB16DoneSuccessLeave:        
        FXLEAVE

BB16DoneSuccess:
        mov     ax,1

;*************************
; Hey, We're DONE!
; Return to GDI ASAP
;*************************
BB16Exit:

        ;all done - pop saved registers

        pop     di
        pop     esi
        pop     ds
        xor     esp, esp
        lea     sp,[bp]
        pop     bp

        retf    32

endif   ; PERF_NEW16BITBLT        

;*************************
; Can't deal with life
; Punt this to the DIBEng
;*************************
BB16PuntToDibEngine:
        ;pop saved regs and pass ExtTextOut call on to DIB Engine
        pop     di
        pop     esi
        pop     ds
        xor     esp, esp
        lea     sp,[bp]
        pop     bp
 
        jmp     DIB_BitBlt


;*************************
; Oops, we have a software
; cursor and I'm too 
; lazy to code 16-bit
; exclusion code
;*************************
BB16JumpBacktoC:
        ;pop saved regs and pass ExtTextOut call on to DIB Engine
        pop     di
        pop     esi
        pop     ds
        xor     esp, esp
        lea     sp,[bp]
        pop     bp
 
ifdef STB_16BIT_BITBLT
        jmp     BitBltC
else
        jmp     DIB_BitBlt
endif


ifdef STB_16BIT_BITBLT
BitBlt Endp
else
BitBlt16 Endp
endif


ifdef PERF_NEW16BITBLT

; ************************************************************
; * This subroutine handles the pattern part if the blt uses *
; * both a pattern and a src DIB.                            *
; * Since WB99 only uses solid brushes along with src DIBS   *
; * we send anything else back to the C code                 *
; ************************************************************

HandleBrush PROC NEAR
assume  si:NEAR PTR DIB_Brush8
            
    mov     dl, gs:[si].dp8BrushFlags
    mov     ax, gs:[si].dp8BrushStyle
    cmp     ax, BS_SOLID
    jne     ExitFail
    
    ; SolidBrush
        
    test    dl, COLORSOLID
    jz      ExitFail
        
    xor     ecx, ecx
    mov     cl, 3
    GET_CF_OFFSET_FAST
    CMDFIFO_CHECKROOM_FAST  ecx

    mov     es:[ebx], dword ptr ( SSTCP_PKT2 + colorForeBit + colorBackBit )
    mov     eax, dword ptr gs:[si].dp8BrushBits
    mov     es:[ebx+4], eax
    mov     es:[ebx+8], eax
    BUMP_FAST   12

    mov     ax, 1
    ret
                                
ExitFail:
    mov     ebx, dword ptr GLOB.lpDeFlags
    and     word ptr es:[ebx], NOT BUSY     
    xor     ax, ax
    ret        
        
HandleBrush endp                      

endif ; PERF_NEW16BITBLT

ifdef PERF_NEWMM
BigCopy PROC FAR C PUBLIC SrcSel:WORD, SrcOffset:DWORD, DstSel:WORD, DstOffset:DWORD, Count:WORD
;        parmW   SrcSel
;        parmD   SrcOffset
;        parmW   DstSel
;        parmD   DstOffset
;        parmW   Count
;cBegin
        push    ds
        push    esi
        push    edi

        cld
        mov     ds, SrcSel
        mov     esi, SrcOffset          ; ds:esi point to the source

        mov     es, DstSel
        mov     edi, DstOffset          ; es:edi point to the destination

        xor     ecx, ecx                ; get byte count
        mov     cx, Count
        shr     ecx, 2                  ; convert to dword count

        jz	Short @f		
        rep	movs dword ptr es:[edi], dword ptr ds:[esi]
@@:
        mov	cl, byte ptr Count
        and	cl, 3			; remaining bytes
        jz	Short @f		
        rep	movs byte ptr es:[edi], byte ptr ds:[esi]
@@:
        pop     edi
        pop     esi
        pop     ds
        leave
        retf
BigCopy endp
endif ; PERF_NEWMM

_TEXT   ENDS

endif
END


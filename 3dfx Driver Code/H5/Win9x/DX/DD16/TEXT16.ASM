;/* $Header $ */
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
;** $Revision $
;** $Date $
;**
;** $Log $
;; 
;; *****************  Version 19  *****************
;; User: Andrew       Date: 7/20/99    Time: 4:46p
;; Updated in $/devel/h5/Win9x/dx/dd16
;; Fixed Glide Problem where SDATA_GDIFLAGS_HWC_EXCLUSIVE was skipping
;; host bitmaps
;; 
;; *****************  Version 18  *****************
;; User: Andrew       Date: 6/16/99    Time: 4:46p
;; Updated in $/devel/h5/Win9x/dx/dd16
;; Added SKIP_TOTAL_FLAGS
;; 
;; *****************  Version 16  *****************
;; User: Stb_mimhoff  Date: 5/27/99    Time: 8:05a
;; Updated in $/devel/h3/win95/dx/dd16
;; Added check fo HWC_EXCLUSIVE bit to ExtTextOut function
;; 
;; *****************  Version 15  *****************
;; User: Stb_mimhoff  Date: 5/05/99    Time: 1:40p
;; Updated in $/devel/h3/win95/dx/dd16
;; Fix for PRS 5845... Added check for opaque rect right and bottom less
;; than left or top
;; 
;; *****************  Version 14  *****************
;; User: Stb_shanna   Date: 3/15/99    Time: 5:26p
;; Updated in $/devel/h3/win95/dx/dd16
;; Fixed PRS 4945 by circumventing a HW bug that does not allow drawing
;; spans of more than 16k bytes.
;; 
;; *****************  Version 13  *****************
;; User: Stb_shanna   Date: 3/11/99    Time: 10:42a
;; Updated in $/devel/h3/win95/dx/dd16
;; Fixed PRS 5008: used a DWORD instead of a WORD for the largerow glyph
;; offset
;; 
;; *****************  Version 12  *****************
;; User: Stb_sjohnston Date: 3/08/99    Time: 5:22p
;; Updated in $/devel/h3/win95/dx/dd16
;; Fix for PRSes - 4749 Winstone 99 Font corruption-hang 4937 Winstone 99
;; Lock Up 4885 Letters on Word pad are distorted
;; 
;; *****************  Version 11  *****************
;; User: Stb_mmcclure Date: 3/05/99    Time: 1:23p
;; Updated in $/devel/h3/win95/dx/dd16
;; For Bob Johnston:  Fix for PRS 4749 Winstone 99 Font corruption while
;; running Poweroint.  Incorrect index into a LARGE ROW glyph structure.
;; 
;; *****************  Version 10  *****************
;; User: Stb_mmcclure Date: 3/04/99    Time: 11:39a
;; Updated in $/devel/h3/win95/dx/dd16
;; From Bob Johnston: Fix for PRS 4757, clip optimization seems to cause
;; problems with the g, p, and y letters on a web page containing tables.
;; If you scroll up and down slowly, the bottom of the letters gets cut
;; off.
;; 
;; *****************  Version 9  *****************
;; User: Stb_sjohnston Date: 3/03/99    Time: 8:52p
;; Updated in $/devel/h3/win95/dx/dd16
;; Fix for PRS 4676 and 4681 - in 8bpp modes, use 32bit code instead of
;; optimized 16bit code
;; 
;; *****************  Version 8  *****************
;; User: Stb_pzheng   Date: 2/24/99    Time: 5:23p
;; Updated in $/devel/h3/win95/dx/dd16
;; Restored version 6 of this file and added fix for build problem when
;; STBPERF_16BIT_TEXTOUT is not defined
;; 
;; *****************  Version 6  *****************
;; User: Stb_pzheng   Date: 2/24/99    Time: 9:44a
;; Updated in $/devel/h3/win95/dx/dd16
;; Optimized glyph loop
;; 
;; *****************  Version 5  *****************
;; User: Stb_pzheng   Date: 2/09/99    Time: 10:16a
;; Updated in $/devel/h3/win95/dx/dd16
;; 
;; 
;**
;*/

;----------------------------------------------------------------------------
;
; text16.asm
;
; ASM equivalents text routines
;
;----------------------------------------------------------------------------

        .xlist
        include shared.inc
        include gdidib.inc
        include fifomgr.inc
        .list

; STB Begin change
; STB-SR 12/16/98 Added for STB Performance optimizations
ifdef INCSTBPERF
INCLUDE ..\..\BUILD\STBPERF.INC
endif
; STB END change
ifdef STB_16BIT_TEXTOUT

_THK16TEXT   SEGMENT WORD USE16 PUBLIC 'CODE'
_THK16TEXT   ENDS

_DATA   SEGMENT WORD USE16 PUBLIC 'DATA'
_DATA   ENDS

        EXTRN _lpDriverData:FAR PTR
        EXTRN _lph3IORegs:DWORD
        EXTRN _h3READ:FAR



.586p
.MMX

_TEXT   SEGMENT WORD PUBLIC 'CODE' USE16 



DIB_ExtTextOut      PROTO   FAR PASCAL

ifdef STB_16BIT_TEXTOUT
ExtTextOutC         PROTO   FAR PASCAL
endif

IFNDEF SKIP_STAGE2
        extern  _DoHostStage2:FAR
ENDIF


T16DRAWSTRING Macro   GLYPHSIZE,  INDEXSIZE, CHARWIDTHS
        ;********************************
        ; Setup Main glyph xfer loop 
        ;********************************
        Local   T16DSLoopTop, T16DSSkipGlyph
    
IFIDN <CHARWIDTHS>, <Dx>
        xor     ecx, ecx
        mov     NCHARS, ecx
ENDIF

        lfs     di, lpString
        mov     eax, (SSTG_ROP_SRCCOPY + SSTG_TRANSPARENT + SSTG_HOST_BLT)
        or      eax, CLIPSEL
		mov		GLYPHCMD, eax
		mov		eax, (SSTCP_PKT2 + dstSizeBit + dstXYBit + commandBit)  
		mov		GLYPHPH, eax
        xor     eax,eax
T16DSLoopTop:
IFIDN <INDEXSIZE>, <ByteIndex>
        mov     al, fs:[di]                             ; EAX = glyph_index
ELSE
        mov     ax, fs:[di]                             ; EAX = glyph_index
ENDIF
        mov     GLYPHIDX, eax
        mov     bx,[NewFontSeg.nfGlyphOffset]
        mov     edx, gs:[bx]
IFIDN <GLYPHSIZE>, <SmallRow>
        lea     ebx, [edx+eax*2]
        movzx   ebx, word ptr gs:[ebx]

if 0
        ;***************************
        ; RAJ-01/29/99 Experiment
        ;  with KNI prefetchtna
        ;  on the glyph data
        ;  at gs:[ebx]
        ;***************************
        db      65h
        db      0fh
        db      18h
        db      03h
endif


        mov     eax, gs:[bx]                            ; org_x, org_y, width, height
        mov     SRSTRUC, eax                            ; Save off SR values.
        add     ebx, 4
        mov     GLYPHPTR, ebx                           ; Save off next Glyph address+ SR Struc size

        mov     ecx, eax                                ; Copy eax data to ecx
        and     ax, 0ffh
        movsx   ax, al                                  ; ax = X org
        shr     cx, 8
        movsx   cx, cl                                  ; cx = Y org
ELSE
        lea     ebx, [edx+eax*4]
		
		; Fix for PRS 5008: Winstone 99: system hang while running w/ a 1600 x 1200 desktop at any bit depth.
		; The fix is to use a DWORD for the glyph offset since the glyph offset table is an array of DWORDs
		; and not WORD offsets.

        mov		ebx, gs:[ebx]
        mov     eax, gs:[ebx]                            ; org_x, org_y, 
        mov     LRORGXY, eax                            

		; STB-MDM For Bob J.  Fix for PRS 4749 Winstone 99 Font corruption while running Poweroint.  
		; Incorrect index into a LARGE ROW glyph structure.
        mov     eax, gs:[ebx+4]                          ;  width, height
        mov     LRORGWH, eax                            
        add     ebx, 8
        mov     GLYPHPTR, ebx                           ; Save off next Glyph address+ SR Struc size

        mov     cx, word ptr LRORGXY+2                  ; cx = Y org
        mov     ax, word ptr LRORGXY                    ; ax = X org
ENDIF
        ;*******************************
        ; Position X, Y for the glyph
        ;*******************************
        add     ax, wDestXOrg                           ; ax = X
        neg     cx
        add     cx, wDestYOrg                           ; cx = Y

IFIDN <GLYPHSIZE>, <SmallRow>
        ;***************************
        ; Glyph Trivial rejection
        ;***************************
        mov     dx, word ptr SRSTRUC+2
        cmp     dl, 0
        jz      T16DSSkipGlyph
        cmp     dh, 0
        jz      T16DSSkipGlyph

        cmp     ax, word ptr CLPBR                      ; X > Clip Right
        jg      T16DSSkipGlyph
        and     dx, 0ffh
        movsx   dx, dl
        add     dx, ax                                  ; X+Width
        cmp     dx, word ptr CLPTL                      ; X+Width < Clip Left
        jl      T16DSSkipGlyph

        ;******************
        ; Save XY position
        ;******************
        mov     word ptr GLYPHPOS, ax
        mov     word ptr GLYPHPOS+2, cx

        ;**************************
        ; Compute # bytes per row
        ;**************************
        xor     ax, ax
        mov     al, byte ptr SRSTRUC+2                  ; AX = Width
        add     ax, 7                                   ; AX = Width+7
        shr     ax, 3                                   ; AX = num bytes/row

        ;*****************************
        ; Compute # dwords per Glyph
        ;*****************************
        xor     bx, bx
        mov     bl, byte ptr SRSTRUC+3                  ; BX = Height
        imul    bx
        add     ax, 3
        shr     ax, 2                                   ; AX = num dwords/glyph
        
ELSE
        ;***************************
        ; Glyph Trivial rejection
        ;***************************
        mov     dx, word ptr LRORGWH 
        cmp     dx, 0
        jz      T16DSSkipGlyph

        cmp     word ptr LRORGWH+2, 0
        jz      T16DSSkipGlyph

        cmp     ax, word ptr CLPBR                      ; X > Clip Right
        jg      T16DSSkipGlyph
        add     dx, ax                                  ; X+Width
        cmp     dx, word ptr CLPTL                      ; X+Width < Clip Left
        jl      T16DSSkipGlyph

        ;******************
        ; Save XY position
        ;******************
        mov     word ptr GLYPHPOS, ax
        mov     word ptr GLYPHPOS+2, cx

        ;**************************
        ; Compute # bytes per row
        ;**************************
        mov     ax, word ptr LRORGWH                    ; AX = Width
        add     ax, 7                                   ; AX = Width+7
        shr     ax, 3                                   ; AX = num bytes/row

        ;*****************************
        ; Compute # dwords per Glyph
        ;*****************************
        mov     bx, word ptr LRORGWH+2                  ; BX = Height
        imul    bx
        add     ax, 3
        shr     ax, 2                                   ; AX = num dwords/glyph
ENDIF

        ;*********************************
        ; Add 5 to the num dwords/glyph
        ; and setup the FIFO to blast the
        ; glyph out!
        ;*********************************
        xor     ecx, ecx
        mov     cx, ax
        mov     GLYPHDWORDS, ecx
        add     ax, 5
        xor     ecx, ecx
        mov     cx, ax
        mov     BUMPVAL, ecx                            ; Save the # Dword writes
        
        ;*********************************
        ; Setup Glyph blt pos and Size
        ;*********************************
        CMDFIFO_CHECKROOM       ecx 
        GET_CF_OFFSET
		mov		eax, GLYPHPH
		mov		es:[ebx], eax
        add     ebx, 4
IFIDN <GLYPHSIZE>, <SmallRow>
        xor     eax, eax
        mov     al, byte ptr SRSTRUC+2                              ; AX is width
        xor     ecx, ecx
        mov     cl, byte ptr SRSTRUC+3                              ; BX is height
        shl     ecx, 16
        or      eax, ecx
ELSE
        mov     eax, LRORGWH
ENDIF
        mov     es:[ebx],   eax                                     ; EAX = Height-Width
        add     ebx, 4
        mov     eax, GLYPHPOS
        mov     es:[ebx],   eax                                     ; EAX = Height-Width
        add     ebx, 4
        mov     eax, GLYPHCMD
        mov     es:[ebx], eax
        add     ebx, 4

        ;*********************************
        ; Blast Glyhp Data
        ;*********************************
        mov     ecx, GLYPHDWORDS
        mov     eax, ecx
        shl     eax, SSTCP_PKT1_NWORDS_SHIFT 
        or      eax, (SSTCP_PKT1 + SSTCP_PKT1_2D + 0100h) 
        mov     es:[ebx], eax
        add     ebx, 4

        push    edi
        mov     edi, ebx
        mov     esi, GLYPHPTR
        rep movsd es:[edi], gs:[esi]
        pop     edi

        mov     ecx, BUMPVAL
        BUMP    ecx

T16DSSkipGlyph: 

        ;**********************************
        ; Compute escapement to next glyph 
        ;**********************************
IFIDN <CHARWIDTHS>, <Dx>
        push    ds
        xor     ebx, ebx
        lds     bx, lpCharWidths
        mov     eax, NCHARS
        lea     ebx,[ebx+eax*2]
        mov     ax, ds:[bx]
        pop     ds
        inc     word ptr NCHARS
ELSE
        mov     eax, GLYPHIDX
        mov     bx,[NewFontSeg.nfAWTable] 
        mov     edx, gs:[bx]
        lea     ebx, [edx+eax*2]
        xor     eax, eax
        mov     ax, gs:[ebx]
ENDIF
        add     ax, wDestXOrg                           ; ax = wDestXOrg + nfAWTable[glyph_index]
        mov     wDestXOrg, ax                           ; Save back into the stack
        
IFIDN <INDEXSIZE>, <ByteIndex>
        inc     di
ELSE
        add     di, 2
ENDIF
        xor     eax, eax
        dec     wCount
        jnz     T16DSLoopTop

        jmp     Text16DoneSuccessLeave
EndM

;
; ExtTextOut
;
; Handles ExtTextOut call
;

; Prepare equates for locals
OPQTL       equ     (dword ptr [RBP-(NEXTLOC+00)])
OPQBR       equ     (dword ptr [RBP-(NEXTLOC+04)])
OPQWH       equ     (dword ptr [RBP-(NEXTLOC+08)])
BKCOLOR     equ     (dword ptr [RBP-(NEXTLOC+12)])
FGCOLOR     equ     (dword ptr [RBP-(NEXTLOC+16)])
CLPTL       equ     (dword ptr [RBP-(NEXTLOC+20)]) 
CLPBR       equ     (dword ptr [RBP-(NEXTLOC+24)]) 
NCHARS      equ     (dword ptr [RBP-(NEXTLOC+28)]) 
SRSTRUC     equ     (dword ptr [RBP-(NEXTLOC+32)]) 
LRORGXY     equ     (dword ptr [RBP-(NEXTLOC+36)])
LRORGWH     equ     (dword ptr [RBP-(NEXTLOC+40)])
GLYPHPTR    equ     (dword ptr [RBP-(NEXTLOC+44)])
GLYPHPOS    equ     (dword ptr [RBP-(NEXTLOC+48)])
GLYPHDWORDS equ     (dword ptr [RBP-(NEXTLOC+52)])
GLYPHIDX    equ     (dword ptr [RBP-(NEXTLOC+56)])
BUMPVAL     equ     (dword ptr [RBP-(NEXTLOC+60)])
GLYPHCMD    equ     (dword ptr [RBP-(NEXTLOC+64)])
GLYPHPH     equ     (dword ptr [RBP-(NEXTLOC+68)])

LOCALS      equ     NEXTLOC+72

; Prepare equates for parameters
PBASE   equ     6       ; (far return CS:IP + BP)

lpDestDev       equ (dword ptr [RBP+(PBASE+24h)])
wDestXOrg       equ (word  ptr [RBP+(PBASE+22h)])
wDestYOrg       equ (word  ptr [RBP+(PBASE+20h)])
lpClipRect      equ (dword ptr [RBP+(PBASE+1Ch)])
lpString        equ (dword ptr [RBP+(PBASE+18h)])
wCount          equ (word  ptr [RBP+(PBASE+16h)])
lpFontInfo      equ (dword ptr [RBP+(PBASE+12h)])
lpDrawMode      equ (dword ptr [RBP+(PBASE+0Eh)])
lpTextForm      equ (dword ptr [RBP+(PBASE+0Ah)])
lpCharWidths    equ (dword ptr [RBP+(PBASE+06h)])
lpOpaqueRect    equ (dword ptr [RBP+(PBASE+02h)])
wOptions        equ (word  ptr [RBP+(PBASE+00h)])

ifdef STB_16BIT_TEXTOUT
ExtTextOut      PROC    FAR PASCAL PUBLIC
else
ExtTextOut16    PROC    FAR PASCAL PUBLIC
endif




        ;******************************
        ; Setup passed stack reference 
        ;******************************
        push    bp
        mov     bp, sp                      ;setup EBP for accessing stack params
	xor		esp, esp
	mov		sp, bp
        sub     sp, LOCALS                 ; Make room for locals
        push    ds
        push    si
        push    di

IFNDEF SKIP_STAGE2
	lfs	di, lpDestDev
        cmp     fs:[di].DIBENGINE.deType, 05250h     ; is this a DIBENG device type
        jnz     short Text16DevInfoNull                ; if not, do nothing
        mov     ax, fs:[di].DIBENGINE.deFlags        ; if so, check the flags
        and     ax, VRAM OR OFFSCREEN           ; for device bitmap settings
        cmp     ax, VRAM OR OFFSCREEN           ; is it a device bitmap?
        jnz     short Text16DevInfoNull           ; if not, do nothing

        cmp     word ptr fs:[di].DIBENGINE.deDriverReserved+2,0  ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        jz      short Text16DevInfoNull               ; if (NULL != pDqueue) 

        lds     si,fs:[di].DIBENGINE.deDriverReserved         ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        mov     ax, [si+16+8]                       ; pDevInfo = (PDEVINFO)pDqueue->pData;  
        cmp     ax, 1
        jnz     short Text16DevInfoNull               ; if (DV_ONHOST == pDevInfo->wStatus)
        
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

Text16DevInfoNull:
ENDIF

        ;******************************
        ; If not VRAM, BUSY 
        ;    or PALETTE_XLAT, Punt!
        ;******************************
        lfs     di, lpDestDev
        mov     dx, fs:[di].DIBENGINE.deFlags
        xor     dx, VRAM                        
        ;
        ; If it is the host let it happen even if GLide is running
        ;
        test    dx, VRAM
        jnz     Text16PuntToDibEngine

        ;******************************
        ; If size call, Punt!
        ;******************************
        cmp     wCount, 0
        jl      Text16PuntToDibEngine

        ;
        ; If the Destination Device is the Display Card
        ; make sure the hardware is not being used by
        ; Glide or SLI/AA
        ;
        mov     ax, _DATA					; Always check for Glide active
        mov     ds, ax						; before accessing hardware
	test    word ptr GLOB.gdiFlags, SKIP_TOTAL_FLAGS
        jnz     Text16Exit

        ;
        ;
        ;
        test    dx, BUSY+PALETTE_XLAT
        jnz     Text16PuntToDibEngine

        ;********************************************
        ; Check to see if text is a supported format
        ;********************************************
        test    wOptions, ETO_LEVEL_MODE                ; is text anti-alias?
        jnz     Text16PuntToDibEngine                   ;punt this for now 
        test    wOptions, ETO_BYTE_PACKED               ; is this a supported type?
        jz      Text16PuntToDibEngine                   ; nope
        
	cmp		fs:[di].DIBENGINE.deBitsPixel, 8
	jz		Text16JumpBacktoC

        ;******************************
        ; Load lpClipRect to stack
        ;   Get out now if TR 
        ;******************************
        lds     bx, lpClipRect
        mov     cx, [bx]            ;left
        mov     dx, [bx+2]          ;top
        mov     ax, [bx+4]          ;right
        mov     bx, [bx+6]          ;bottom
        mov     word ptr CLPTL, cx
        mov     word ptr CLPTL+2, dx
        mov     word ptr CLPBR, ax
        mov     word ptr CLPBR+2, bx
        cmp     bx, dx                       ; test for trivial rejection case
        jle     Text16DoneSuccess
        
        cmp     ax, cx                       ; test for trivial rejection case
        jle     Text16DoneSuccess

        


        ;****************************************
        ; Grab color out of the DrawMode struct
        ;****************************************
        lgs     bx, lpDrawMode
        mov     eax, gs:[bx+4]  ; lpDrawMode->bkColor
        mov     ebx, gs:[bx+8]  ; lpDrawMode->TextColor
        mov     BKCOLOR, eax
        mov     FGCOLOR, ebx
        
        ;******************************
        ; Load our data segment
        ;******************************
        xor     eax, eax
        mov     ax, _DATA
        mov     ds, eax

        ;****************************************
        ; Prepare FIFO access
        ;****************************************
        CMDFIFO_PROLOG
        FXENTER Text16JumpBacktoC 
        
        ;**************************************************
        ; Prepare the engine for clipping and set 
        ;  our clipselect bit to clip1
        ;**************************************************
		xor		ecx, ecx
		mov		cl, 3
        CMDFIFO_CHECKROOM     ecx 
        GET_CF_OFFSET
        mov     dword ptr es:[ebx],(SSTCP_PKT2 + clip1minBit + clip1maxBit)
        add     ebx, 4
        mov     eax, CLPTL
        mov     es:[ebx], eax                   ; clip1min lpClip->topleft
        add     ebx, 4
        mov     edx, CLPBR
        mov     es:[ebx], edx               ; clip1max lpClip->bottomright
        BUMP    3
        mov     dword ptr CLIPSEL, SSTG_CLIPSELECT

    
        ;**************************************************
        ; Is valid Opaque Rect?  If So, Grab values needed 
        ; to draw the rect out of their
        ; far pointers and store them locally
        ;**************************************************
        cmp     word ptr lpOpaqueRect+2, 0
        jz      Text16NullOpaqueRect    

        lfs     di, lpOpaqueRect

Text16OpaqueLoopTop:
        mov     cx, fs:[di]         ; lpOpaque->left
        mov     dx, fs:[di+2]       ; lpOpaque->top
        mov     ax, fs:[di+4]       ; lpOpaque->right
        mov     bx, fs:[di+6]       ; lpOpaque->bottom
; Fix for TriGem bug, PRS 12184 text corruption in SQL Query Analyzer
		cmp 	ax, 1000h			; Check for out of range 'right'
		jle		short @f  			; In range
		mov 	ax, word ptr CLPBR	; Set 'right' same as Clip Rect
@@:
		cmp 	cx, 0h				; Check for out of range 'left'
		jge		short @f			; In range
		mov 	cx, word ptr CLPTL	; Set 'left' same as Clip Rect

@@:	
        mov     word ptr OPQTL, cx
        mov     word ptr OPQTL+2, dx
        mov     word ptr OPQBR, ax
        mov     word ptr OPQBR+2, bx


if 1	; PRS 8400: Severe text corruption in Notepad
	; Multiple error conditions to handle below.
	; First, reject case of "inverted" width in
	; opaque rectangle (i.e. left > right).
	; Second, if left edge is negative use the
	; right edge for the width.
	; NOTE: this should probably be done for height
	; as well.  Since this is the optimized code
	; path and such an instance has not yet been
	; encountered don't process this for now.  MGM
	cmp	ax, cx			; Inverted width?
	jle	Text16DoMoreOpaqueRects	; Y: forget it
        sub     ax, cx              	; ax = Opaque Rect Width
	jns	@f			; left rect edge negative?
   	mov	ax, word ptr OPQBR	; Y: use right for width
@@:
	sub     bx, dx              	; bx = Opaque Rect Height   
else
        ; Check to see if W or H overflow - PRS 5845 
        ; See note in textout.c PRS 2486
        sub     ax, cx              ; ax = Opaque Rect Width
		js		Text16DoMoreOpaqueRects
        sub     bx, dx              ; bx = Opaque Rect Height   
		js		Text16DoMoreOpaqueRects

	;******************************************************
        ; Circumvent HW bug that doesn't allow drawing spans
        ; of more than 16k bytes at 32-bpp (#1623 and #1929).
        ;******************************************************
		
		; The hardware bug seems to affect 8-bpp and 16-bpp as well
		; so the color depth check is eliminated.
if 0	; Fix for PRS 4945: Notepad text gets corrupted when the text is highlighted.		
        cmp     GLOB.bpp, 32
        jne     short @f
endif	; end of fix
        cmp     ax, 01000h
        jbe     short @f
        mov     ax, 01000h
@@:        
endif
        mov     word ptr OPQWH, ax          ; Store Rect Width to Stack
        mov     word ptr OPQWH+2, bx            ; Store Rect Height to Stack

        mov     ecx, OPQTL
        mov     edx, OPQBR

        cmp     edx, ecx
        jle     Text16NullOpaqueRect

        ;***************************
        ; Rip out Exclude rect(s)!
        ;***************************
		xor		ecx, ecx
		mov		cl, 5
        CMDFIFO_CHECKROOM       ecx 
        GET_CF_OFFSET
        mov     dword ptr es:[ebx], (SSTCP_PKT2 + colorForeBit + dstSizeBit + dstXYBit + commandBit)
        add     ebx, 4
        mov     eax, BKCOLOR 
        mov     es:[ebx], eax                   ; ColorFore
        add     ebx, 4
        mov     edx, OPQWH
        mov     es:[ebx], edx                       ; dstSize
        add     ebx, 4
        mov     ecx, OPQTL
        mov     es:[ebx], ecx                   ; dstXY, OPQTL
        add     ebx, 4
        mov     eax, (SSTG_ROP_SRCCOPY + SSTG_GO + SSTG_RECTFILL)
        or      eax, CLIPSEL
        mov     es:[ebx], eax                   ; command
        BUMP    5

Text16DoMoreOpaqueRects:

        add     di, 8               ; Advance to next possible Opaque Rect

        mov     ecx, fs:[di]
        mov     edx, fs:[di+4]
        cmp     edx, ecx            ; Check if there is another valid Opaque Rect
        jg      Text16OpaqueLoopTop ; Yep, better go handle it.


Text16NullOpaqueRect:
        cmp     wCount, 0
        je      Text16DoneSuccessLeave

        ;***************************************
        ; Check for special case Trivial Reject
        ;  on entire string
        ;****************************************

if 0	;PRS 4757, this optimization seems to cause problems with
		; Web pages with low hanging "g" chars on scolling
        mov     ax, wDestYOrg                ; help out our hardware clipping a bit
        cmp     ax, word ptr CLPTL+2         ; If string is up past top clip, reject it!
        jl      Text16DoneSuccessLeave 
endif

        mov     ax, wDestXOrg                
        cmp     ax, word ptr CLPBR           ; If string is to the right of the clip, reject it!
        jg      Text16DoneSuccessLeave 

        ;*****************************************
        ; Setup 2D Engine for the monochrome text
        ;  Program in the foreground color.
        ;  Because we are always transparent,
        ;  we don't need background color.
        ;*****************************************
		xor		ecx, ecx
		mov		cl, 4
        CMDFIFO_CHECKROOM       ecx
        GET_CF_OFFSET
        xor     eax, eax
        mov     dword ptr es:[ebx], (SSTCP_PKT2 + srcFormatBit + srcXYBit + colorForeBit)   ; PH
        add     ebx, 4
        mov     dword ptr es:[ebx], (SSTG_PIXFMT_1BPP + SSTG_SRC_PACK_8)                ; SrcFormat
        add     ebx, 4
        mov     es:[ebx], eax                                                   ; SrcXY
        add     ebx, 4
        mov     eax, FGCOLOR                                                    
        mov     es:[ebx], eax                                               ; TextColor
        BUMP    4

        ;*****************************************
        ; Figure out which Glyph Xfer Loop we're
        ;  going to.
        ;*****************************************
        xor     esi, esi
        xor     eax, eax
        mov     ax, word ptr lpFontInfo+2
        mov     gs, ax
        cmp     word ptr lpCharWidths+2, 0
        jne     Text16DrawGlyphLpDx                 ; Use Dx Array for porpotional spcaing

        test    wOptions, ETO_GLYPH_INDEX
        jnz     Text16DrawWordIndexGlyph            ; WORD based index glyphs

        mov     bx,[NewFontSeg.nfFormat] 
        test    word ptr gs:[bx], NF_LARGE          ; Must be Byte (char based) glyphs   
        jnz     Text16DrawByteIndexLargeGlyph       ; LARGE ROW


;***************************
; Glyph Xfer Routines!
;***************************


Text16DrawByteIndexGlyph:
    T16DRAWSTRING  SmallRow, ByteIndex, NonDx


Text16DrawByteIndexLargeGlyph:
    T16DRAWSTRING  LargeRow, ByteIndex, NonDx

Text16DrawWordIndexGlyph:
    ; PRS 4885 - fixes large font draw problem
    mov     bx,[NewFontSeg.nfFormat] 
    test    word ptr gs:[bx], NF_LARGE
    jnz     Text16DrawWordIndexLargeGlyph
    T16DRAWSTRING  SmallRow, WordIndex, NonDx

Text16DrawWordIndexLargeGlyph:
    T16DRAWSTRING  LargeRow, WordIndex, NonDx


Text16DrawGlyphLpDx:
    test    wOptions, ETO_GLYPH_INDEX
    jnz     Text16DrawWordIndexGlyphLpDx
    mov     bx,[NewFontSeg.nfFormat] 
    test    word ptr gs:[bx], NF_LARGE
    jnz     Text16DrawByteIndexLargeGlyphLpDx
    T16DRAWSTRING  SmallRow, ByteIndex, Dx

Text16DrawByteIndexLargeGlyphLpDx:
    T16DRAWSTRING  LargeRow, ByteIndex, Dx


Text16DrawWordIndexGlyphLpDx:
    mov     bx,[NewFontSeg.nfFormat] 
    test    word ptr gs:[bx], NF_LARGE
    jnz     Text16DrawWordIndexLargeGlyphLpDx
    T16DRAWSTRING  SmallRow, WordIndex, Dx

Text16DrawWordIndexLargeGlyphLpDx:
    T16DRAWSTRING  LargeRow, WordIndex, Dx



Text16DoneSuccessLeave:        
        FXLEAVE


Text16DoneSuccess:
        mov     ax,1

;*************************
; Hey, We're DONE!
; Return to GDI ASAP
;*************************
Text16Exit:

        ;all done - pop saved registers

        pop     di
        pop     si
        pop     ds
		xor		esp, esp
        lea     sp,[bp]
        pop     bp

        retf    40

;*************************
; Can't deal with life
; Punt this to the DIBEng
;*************************
Text16PuntToDibEngine:
        ;pop saved regs and pass ExtTextOut call on to DIB Engine
        pop     di
        pop     si
        pop     ds
		xor		esp, esp
        lea     sp,[bp]
        pop     bp
 
        jmp     DIB_ExtTextOut


;*************************
; Oops, we have a software
; cursor and I'm too 
; lazy to code 16-bit
; exclusion code
;*************************
Text16JumpBacktoC:
        ;pop saved regs and pass ExtTextOut call on to DIB Engine
        pop     di
        pop     si
        pop     ds
		xor		esp, esp
        lea     sp,[bp]
        pop     bp
 
ifdef STB_16BIT_TEXTOUT
        jmp     ExtTextOutC
else
        jmp     DIB_ExtTextOut
endif


ifdef STB_16BIT_TEXTOUT
ExtTextOut Endp
else
ExtTextOut16 Endp
endif


_TEXT   ENDS

endif

END



;----------------------------------------------------------------------------
;
; polyline.asm
;
; ASM equivalent OS_POLYLINE routine
;
;----------------------------------------------------------------------------

incDrawMode EQU 1

        .xlist
        include shared.inc
        include dibeng.inc
        include gdidefs.inc
        include fifomgr.inc
        .list

ifdef INCSTBPERF
INCLUDE ..\..\BUILD\STBPERF.INC
endif ;INCSTBPERF

ifdef STBPERF_POLYLINE16

_THK16TEXT   SEGMENT WORD USE16 PUBLIC 'CODE'
_THK16TEXT   ENDS

PS_NULL         equ 5
LS_SOLID        equ 0
LS_INSIDEFRAME  equ 6

;; on bit is foreground.
DOT         equ 00000000fh  ;on bit is foreground
DASH        equ 000ffffffh  ;dash=24, bkgrnd=8
DOTDASH     equ 000f00fffh  ;dash=12, bkgrnd=8, dot=4
DASHDOTDOT  equ 00f0f0fffh  ;dash=12, dot1=4, dot2=4, bkgrnd=4

DOT_STYLE_SIZE          equ 7
DASH_STYLE_SIZE         equ 31
DOTDASH_STYLE_SIZE      equ 31
DASHDOTDOT_STYLE_SIZE   equ 31

MATCH_DIB           equ (8 SHL SSTG_LSPOS_INT_SHIFT)

DASH_STYLE          equ (DASH_STYLE_SIZE SHL SSTG_LSSIZE_SHIFT) + MATCH_DIB
DOT_STYLE           equ (DOT_STYLE_SIZE SHL SSTG_LSSIZE_SHIFT)
DOTDASH_STYLE       equ (DOTDASH_STYLE_SIZE SHL SSTG_LSSIZE_SHIFT) + MATCH_DIB
DASHDOTDOT_STYLE    equ (DASHDOTDOT_STYLE_SIZE SHL SSTG_LSSIZE_SHIFT) + MATCH_DIB

CMD_STIPPLE         equ (SSTG_EN_LINESTIPPLE + SSTG_REVERSIBLE)


OutputC32       PROTO   FAR PASCAL

;*----------------------------------------------------------------------
;Function name: PREPARE_STACK
;
;Description:   Align the stack and make room for pParams
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
extrn _dwDevNode:DWORD
STB_PREPARESTACK Macro
        and     sp, NOT 3       ; align stack
        push    _dwDevNode      ; leave room on stack for pParams
Endm


;*----------------------------------------------------------------------
;Function name: FARCALL32
;
;Description:   The call that "thunks" to the 32-bit side.
;
;Information:   This is a MACRO.
;
;Return:        VOID
;----------------------------------------------------------------------*
extrn _apfnTable32:DWORD
extrn wFlatDataSel:WORD
;;;extrn _pfnTemp32:FWORD
STB_FARCALL32 Macro FnNumber
        mov     ax, wFlatDataSel
        mov     es, ax
        mov     ecx, _apfnTable32
        mov     edx, es:[ecx][FnNumber*4]
        call    DWORD PTR [edx]
;;;        mov     DWORD PTR _pfnTemp32, edx
;;;        call    [_pfnTemp32]
Endm


_DATA   SEGMENT WORD USE16 PUBLIC 'DATA'

;;   **********************************************************************   
;;/*  
;;/*   
;;   << THIS TABLE, rop2ConvertTable, is FOR LINES ONLY >>
;;   The rop2ConvertTable converts the Rop2 in Drawmode to rop3.
;;   that use src argument instead of pattern arguement.
;;   Rop2 operate on dest and pattern.  However the hardware feeds the
;;   stipple patterns as source input into the rops.
;;*/
;;   **********************************************************************   
rop2ConvertTable    dd  000000000h       ;; filler
                    dd  000000000h       ;; R2_BLACK
                    dd  011000000h       ;; notmergepen   DSon
                    dd  022000000h       ;; masknotpen    DSna   
                    dd  033000000h       ;; notcopypen    Sn
                    dd  044000000h       ;; maskpennot    SDna
                    dd  055000000h       ;; not           Dn
                    dd  066000000h       ;; xorpen        DSx
                    dd  077000000h       ;; notmaskpen    DSan
                    dd  088000000h       ;; maskpen       DSa
                    dd  099000000h       ;; notxorpen     DSxn
                    dd  0AA000000h       ;; nop           D
                    dd  0BB000000h       ;; mergenotpen   DSno
                    dd  0CC000000h       ;; copy pen      S
                    dd  0DD000000h       ;; mergepennot   SDno
                    dd  0EE000000h       ;; mergepen      DSo
                    dd  0FF000000h       ;; white         1


;// To reduce the amount of code needed to process pen styles, this
;// array contains the lineStyle, lineStipple, and cmdData values
;// needed for each style. We can then look up the values based on
;// the given pen style instead of calculating the values each time.
;// Note, we use the GDIDEFS.H MaxLineStyle, but we add one because
;// the styles start with 0!
PenConvert          dd  000000000h, 000000000h,         000000000h      ;LS_SOLID
                    dd  DASH,       DASH_STYLE,         CMD_STIPPLE     ;LS_DASHED
                    dd  DOT,        DOT_STYLE,          CMD_STIPPLE     ;LS_DOTTED
                    dd  DOTDASH,    DOTDASH_STYLE,      CMD_STIPPLE     ;LS_DOTDASHED
                    dd  DASHDOTDOT, DASHDOTDOT_STYLE,   CMD_STIPPLE     ;LS_DASHDOTDOT
                    dd  000000000h, 000000000h,         000000000h      ;LS_NOLINE


NumSingleLine       dd 0
NumSinglePixelLine  dd 0
                        
_DATA   ENDS



        EXTRN _lpDriverData:FAR PTR
        EXTRN _lph3IORegs:DWORD
        EXTRN _h3READ:FAR


ifndef SKIP_STAGE2
extern  _DoHostStage2:FAR
endif ;SKIP_STAGE2

DIB_Output          PROTO   FAR PASCAL


.586p

_TEXT   SEGMENT WORD PUBLIC 'CODE' USE16 




;
; PolyLine16
;
; Handles Output call for the OS_POLYLINE style
;

; Prepare equates for locals
COMMAND     equ     (dword ptr [RBP-(NEXTLOC+00)])
BGCOLOR     equ     (dword ptr [RBP-(NEXTLOC+04)])
FGCOLOR     equ     (dword ptr [RBP-(NEXTLOC+08)])

LOCALS      equ     NEXTLOC+12

; Prepare equates for parameters
PBASE       equ     6     ; (far return CS:IP + BP)


STB_SAVEREGS Macro
        pushad          ; 32 bytes: EAX, ECX, EDX, EBX, original ESP, EBP, ESI, EDI; 5 clock-ticks on PII
        push    ds      ;  2 bytes
        push    es      ;  2 bytes
        push    fs      ;  2 bytes
ENDM

STB_RESTOREREGS Macro
        pop     fs      ;  2 bytes
        pop     es      ;  2 bytes
        pop     ds      ;  2 bytes
        popad           ; 32 bytes: EDI, ESI, EBP, EDX, ECX, <add 4 to current ESP>, EBX, EDX, ECX, EAX; 5 clock-ticks on PII
ENDM

;THESE ARE FOR USE AFTER THE STACK FRAME IS INITIALIZED ("push bp; sub sp LOCALS").
lpDestDev       equ (dword ptr [RBP+(PBASE+18h)])
wStyle          equ (word  ptr [RBP+(PBASE+16h)])
wCount          equ (word  ptr [RBP+(PBASE+14h)])
lpPoints        equ (dword ptr [RBP+(PBASE+10h)])
lpPPen          equ (dword ptr [RBP+(PBASE+0Ch)])
lpPBrush        equ (dword ptr [RBP+(PBASE+08h)])
lpDrawMode      equ (dword ptr [RBP+(PBASE+04h)])
lpClipRect      equ (dword ptr [RBP+(PBASE+00h)])


OS_POLYLINE     equ 18


;*----------------------------------------------------------------------
;Function name: PolyLine16
;
;Description:   The 16-bit "Output(OS_POLYLINE)" workhorse
;
;Information:   
;
;Return:        VOID
;----------------------------------------------------------------------*
PolyLine16      PROC    FAR PASCAL PUBLIC

        ;******************************
        ; Setup passed stack reference 
        ;******************************
        push    bp
        mov     bp, sp                      ;setup EBP for accessing stack params
        xor     esp, esp
        mov     sp,bp
        sub     sp, LOCALS                 ; Make room for locals
        push    ds
        push    si
        push    di

        lds     si, lpDestDev


;;; RBISSELL,990204: ADDED THESE "SKIP_STAGE2" SECTIONS TO FIX BUG (NEED TO CALL "_HostStage2")
ifndef SKIP_STAGE2

        cmp     ds:[si].DIBENGINE.deType, TYPE_DIBENG           ; is this a DIBENG device type
        jne     pl_trivialreject

;;; RBISSELL,990310: PRS 4969: some off-screen polylines were being rendered on-screen,
;;;                  because the following check was not being done at all.
        mov     dx,					ds:[si].DIBENGINE.deFlags
        and     dx,					VRAM OR OFFSCREEN           ; for device bitmap settings
        cmp     dx,					VRAM OR OFFSCREEN           ; is it a device bitmap?
        jne     short pl_nohoststage2							; if not, do nothing

        lds     si, [si].DIBENGINE.deDriverReserved             ; pDqueue = (PDQUEUE)lpPDevice->deDriverReserved; 
        assume  si:nothing
        mov     ax, ds
        or      ax, si
        jz      short pre_pl_nohoststage2                       ; if (NULL != pDqueue) 
        
        mov     ax, [si+16+8]
        cmp     ax, 1
        jnz     short pre_pl_nohoststage2                       ; if (DV_ONHOST == pDevInfo->wStatus)

        lds     si, lpDestDev
        push    di                                              ; Save register
        push    es                                              ; Save register
        push    ds                                              ; Selector of lpPDevice
        push    si                                              ; Offset of lpPDevice
        call    _DoHostStage2                                   ; Move it so we can be rid of it
        pop     di                                              ; clean stack
        pop     di                                              ; clean stack
        pop     es                                              ; restore register
        pop     di                                              ; restore register

	jmp     short pl_nohoststage2

pre_pl_nohoststage2:

        lds     si, lpDestDev

pl_nohoststage2:
endif ;SKIP_STAGE2

;;; APS 990701: Check these flags after we have done DoHostStage2
;;; RBISSELL,990310: PRS 4969: some off-screen polylines were being rendered on-screen,
;;;                  because the following trivial rejectection was being skipped over.
;;;                  solution: moved this to the top.


        mov     dx,					ds:[si].DIBENGINE.deFlags
        xor     dx,                 VRAM                        ; flip polarity of the VRAM bit
        test    dx,                 VRAM+BUSY+PALETTE_XLAT		; check this here to avoid loading ds again later!
        jnz     pl_trivialreject								; if not in the FB, or hardware is BUSY, or XLAT req'd, then punt

	cmp		ds:[si].DIBENGINE.deBitsPixel, 8
	jz		short pl_notpolyline	

        cmp     wStyle,             OS_POLYLINE
        jne     short pl_notpolyline

        cmp     wCount,             2
        jb      short pl_trivialaccept                          ; nothing to do, so say we did it.

        lfs     di,                 lpPPen
        mov     ax,                 fs:[di].DIB_Pen.dpPenStyle
        cmp     ax,                 PS_NULL
        je      short pl_trivialaccept

		; Fix for PRS 4957 - begins
        cmp     ax,                 LS_SOLID
        jne     short pl_notpolyline
		; Fix for PRS 4957 - ends

        cmp     lpClipRect,         0
        je      pl_ospolyline
        lfs     di,                 lpClipRect
        mov     ax,                 fs:[di+4]                   ;right
        cmp     ax,                 fs:[di+0]                   ;left
        jle     short pl_trivialaccept
        mov     ax,                 fs:[di+6]                   ;bottom
        cmp     ax,                 fs:[di+2]                   ;top
        jle     short pl_trivialaccept

        jmp     short pl_ospolyline
        

pl_swcursor:
pl_notpolyline:

        pop     di
        pop     si
        pop     ds
        xor     esp, esp
        lea     sp,[bp]
        pop     bp

        jmp     OutputC32                                       ; let the old code handle it

pl_trivialaccept:
        mov     ax,                 1
        pop     di
        pop     si
        pop     ds
        xor     esp, esp
        lea     sp,[bp]
        pop     bp

        retf    28

pl_trivialreject:
        xor     ax,                 ax
        pop     di
        pop     si
        pop     ds
        xor     esp, esp
        lea     sp,[bp]
        pop     bp

        jmp     DIB_Output

pl_ospolyline:
        xor     eax,                eax
        mov     COMMAND,            eax


        ; All trivial rejections have been handled above.
        lfs     di,                 lpDestDev
        mov     ax,                 _DATA                   ; needed to access rop conversion table
        mov     ds,                 ax
        CMDFIFO_PROLOG
        FXENTER pl_swcursor


pl_checkcliprect:
        cmp     lpClipRect,         0                       ; do we have a clip rect?
        je      pl_nocliprect
        
        CMDFIFO_CHECKROOM           3
        GET_CF_OFFSET
        SETPH   (  SSTCP_PKT2       \
                 + clip1minBit      \
                 + clip1maxBit      \
                )
        lfs     di,                 lpClipRect
        SETM    foo,                fs:[di+0]               ; clip1min lpClip->topleft
        SETM    foo,                fs:[di+4]               ; clip1max lpClip->bottomright
        BUMP    3
        or      COMMAND,            SSTG_CLIPSELECT
pl_nocliprect:


pl_drawmode:
        test    [lpDrawMode].DRAWMODE.bkMode, BKMODE_TRANSPARENT
        jz      pl_isopaque
        or      COMMAND,            SSTG_TRANSPARENT        ; transparent background
pl_isopaque:

        lfs     di,                 lpDrawMode
        assume  di:NEAR PTR DRAWMODE
        mov     ax,                 _DATA                   ; needed to access rop conversion table
        mov     ds,                 ax
        mov     eax,                fs:[di].bkColor         ; gonna need this later,
        mov     BGCOLOR,            eax                     ; so might as well save it now.
        mov     bx,                 fs:[di].Rop2
		;
        ; CSJ - Fix PRS 5246
		;
		; GDI sometimes gives us a Rop2 == FFFF
		;
		; This causes the offset used in the rop conversion table lookup
		; to be wrong. In 32bit code, the value is innocuous but in 16bit 
		; code, the value, when passed to the FIFO, causes the entire screen 
		; to corrupt and the machine to be locked.
		;
		; Detecting when this case happened and punting to the DIB Engine
		; caused a performace loss in Microstation of about 2.5 points on
		; a P3.
		;
        and     bx,                 001Fh
		; end CSJ - Fix PRS 5246
		;
        shl     bx,                 2
        mov     eax,                ds:[bx+rop2ConvertTable]
        and     eax,                NOT SSTG_CLIPSELECT
        or      COMMAND,            eax


pl_checkpen:
        lfs     di,                 lpPPen
        assume  di:NEAR PTR DIB_Pen

        mov     eax,                fs:[di].dpPenColor      ; gonna need this later,
        mov     FGCOLOR,            eax                     ; so might as well save it now.

        movzx   eax,                fs:[di].dpPenStyle
        test    ax,                 LS_SOLID                ; only stipple if not solid
        je      pl_nopendata
        test    ax,                 LS_INSIDEFRAME          ; only stipple if not inside_frame
        je      pl_nopendata


pl_setpendata:
        push    eax
        CMDFIFO_CHECKROOM           3
        GET_CF_OFFSET
        SETPH   (  SSTCP_PKT2       \
                 + lineStippleBit   \
                 + lineStyleBit     \
                )
        pop     eax
        mov     esi,                eax                     ; esi contains the pen style #
        shl     eax,                8   
        shl     esi,                4
        add     esi,                eax                     ; esi*8 + esi*4 = esi*12 (12 bytes/record)
        SETM    foo,                ds:PenConvert[si+0]     ; lineStippleData
        SETM    foo,                ds:PenConvert[si+4]     ; lineStyleData
        BUMP    3

        mov     eax,                ds:PenConvert[si+8]     ; cmdData
        or      COMMAND,            eax
pl_nopendata:


        xor     esi, esi
        lfs     si,                 lpPoints
        assume  si:NOTHING
        movzx   ecx,                wCount
        cmp     ecx,                3
        jnb     pl_uselaunch

pl_singleline:
        or      COMMAND,            SSTG_POLYLINE + SSTG_GO
        CMDFIFO_CHECKROOM           6
        GET_CF_OFFSET
        SETPH   (  SSTCP_PKT2       \
                 + srcXYBit         \
                 + colorBackBit     \
                 + colorForeBit     \
                 + dstXYBit         \
                 + commandBit       \
                )
        SETM    foo,                fs:[si+0]               ; fortunately, the coords are given in the requisite order!
        SETM    foo,                BGCOLOR                 ; background color
        SETM    foo,                FGCOLOR                 ; foreground color
        SETM    foo,                fs:[si+4]               ; fortunately, the coords are given in the requisite order!
        SETM    foo,                COMMAND                 ; 2D command
        BUMP    6
        jmp     pl_allshewrote


pl_uselaunch:
        push    ecx
        or      COMMAND,            SSTG_POLYLINE
        CMDFIFO_CHECKROOM           5
        GET_CF_OFFSET
        SETPH   (  SSTCP_PKT2   \
                 + srcXYBit     \
                 + colorBackBit \
                 + colorForeBit \
                 + commandBit   \
                )
        SETM    foo,                fs:[si+0]               ; fortunately, the coords are given in the requisite order!
        SETM    foo,                BGCOLOR                 ; background color
        SETM    foo,                FGCOLOR                 ; foreground color
        SETM    foo,                COMMAND                 ; 2D command
        BUMP    5

        pop     ecx
        CMDFIFO_CHECKROOM           ecx
        GET_CF_OFFSET
        mov     edx,                ecx                     ; all this for a packet header!
        dec     edx
        shl     edx,                SSTCP_PKT1_NWORDS_SHIFT
        mov     eax,                LAUNCH_REG_1
        shl     eax,                SSTCP_REGBASE_SHIFT
        or      edx,                eax
        or      edx,                SSTCP_PKT1 + SSTCP_PKT1_2D
        SETPH   edx                                         ; all that for a packet header!

        push    ecx                                         ; need this later
        dec     ecx                                         ; already sent one endpoint
        add     esi,                4                       ; begin with the second point

pl_loop:
        SETM    foo,                fs:[esi]
        add     esi,                4                       ; the next coord pair
        loopd   pl_loop

        pop     ecx
        BUMP    ecx


pl_allshewrote:
        assume  di:NOTHING
        FXLEAVE

        mov     ax,                 1

        pop     di
        pop     si
        pop     ds
        xor     esp, esp
        lea     sp,[bp]
        pop     bp

        retf    28                                          ; 28 bytes of params on the stack

PolyLine16 Endp

_TEXT   ENDS

endif

END


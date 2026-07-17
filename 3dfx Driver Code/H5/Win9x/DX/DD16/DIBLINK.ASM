;/* $Header: /devel/h3/Win95/dx/dd16/diblink.asm 15    12/28/98 11:24a Michael $ */
;/*
;** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
;** All Rights Reserved.
;**
;** Portions Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
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
;** File name:   diblink.asm
;**
;** Description: diblink and support funtions
;**
;** $Revision: 15 $
;** $Date: 12/28/98 11:24a $
;**
;** $History: diblink.asm $
;; 
;; *****************  Version 15  *****************
;; User: Michael      Date: 12/28/98   Time: 11:24a
;; Updated in $/devel/h3/Win95/dx/dd16
;; Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
;; files where this was missing.
;** 
;** 
;*/

        option OLDSTRUCTS
        .xlist
        include dibeng.inc
        .list
        .386

;----------------------------------------------------------------------------
;----------------------------------------------------------------------------
_TEXT   SEGMENT  WORD USE16 PUBLIC 'CODE'
_TEXT   ENDS
_DATA   SEGMENT  WORD USE16 PUBLIC 'DATA'
_DATA   ENDS
CONST   SEGMENT  WORD USE16 PUBLIC 'CONST'
CONST   ENDS
_BSS    SEGMENT  WORD USE16 PUBLIC 'BSS'
_BSS    ENDS
DGROUP  GROUP   CONST, _BSS, _DATA
        ASSUME DS: DGROUP
        ASSUME SS: NOTHING
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; MACROS
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
DIBLINK macro name,target,extra
;----------------------------------------------------------------------------
extrn target:FAR
public name
name:
ifnb <extra>
        mov     ax,_DATA
        mov     es,ax
        pop     ecx
        push    es:[extra]
        push    ecx
endif
        jmp     target
        endm

;----------------------------------------------------------------------------
; D A T A
;----------------------------------------------------------------------------
_DATA segment WORD PUBLIC 'DATA' use16

        ; every windows DLL must has 16 bytes reserved for kernel
resptr  db      16 dup(0)

IFDEF GBLDATA_IN_PDEV
        extrn lpDriverPDevice:DWORD
ELSE
        ; our PDevice
        extrn _DriverData:DWORD
        lpDriverPDevice equ _DriverData
ENDIF

        ; our hModule
        extrn _hModule:WORD


        extrn _Palettized:FAR             ; Palettized Flag (H3.C)

next_int2f      dd      0


_DATA ends

;----------------------------------------------------------------------------
; C O D E
;----------------------------------------------------------------------------
_TEXT segment WORD PUBLIC 'CODE' use16
        extrn LocalInit:FAR               ; in KERNEL
        extrn DIB_BitBlt:FAR              ; in DIBENG
        extrn GetExePtr:FAR               ; in KERNEL

        ;; for bitblt

        extrn ToBackground:FAR            ; in DDMINI.C
        extrn ToForeground:FAR            ; in DDMINI.C
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

;;DIBLINK BitBlt,           DIB_BitBlt
;;DIBLINK ColorInfo,          DIB_ColorInfo
;;DIBLINK Control,          DIB_Control
;;DIBLINK Disable,          DIB_Disable
;;DIBLINK Enable,           DIB_Enable
DIBLINK EnumDFonts,         DIB_EnumDFonts
DIBLINK EnumObj,            DIB_EnumObjExt,             lpDriverPDevice
;;DIBLINK Output,             DIB_Output
DIBLINK Pixel,              DIB_Pixel
;;DIBLINK RealizeObject,      DIB_RealizeObjectExt,       lpDriverPDevice
DIBLINK Strblt,             DIB_Strblt
DIBLINK ScanLR,             DIB_ScanLR
DIBLINK DeviceMode,         DIB_DeviceMode
;;DIBLINK ExtTextOut,         DIB_ExtTextOut
DIBLINK GetCharWidth,       DIB_GetCharWidth
DIBLINK DeviceBitmap,       DIB_DeviceBitmap
DIBLINK FastBorder,         DIB_FastBorder
DIBLINK SetAttribute,       DIB_SetAttribute
;DIBLINK DibBlt,             DIB_DibBltExt               _Palettized
DIBLINK CreateDIBitmap,     DIB_CreateDIBitmap
;DIBLINK DibToDevice,        DIB_DibToDevice
;;DIBLINK SetPalette        DIB_SetPaletteExt,          lpDriverPDevice
DIBLINK GetPalette,         DIB_GetPaletteExt,          lpDriverPDevice
DIBLINK SetPaletteTranslate,DIB_SetPaletteTranslateExt, lpDriverPDevice
DIBLINK GetPaletteTranslate,DIB_GetPaletteTranslateExt, lpDriverPDevice
DIBLINK UpdateColors,       DIB_UpdateColorsExt,        lpDriverPDevice
;DIBLINK StretchBlt,         DIB_StretchBlt
;DIBLINK StretchDIBits,      DIB_StretchDIBits
;;DIBLINK SelectBitmap,       DIB_SelectBitmap
;;DIBLINK BitmapBits,         DIB_BitmapBits
;;DIBLINK  ReEnable,        DIB_ReEnable

DIBLINK Inquire,            DIB_Inquire
;;DIBLINK SetCursor,          DIB_SetCursorExt,           lpDriverPDevice
;;DIBLINK MoveCursor,         DIB_MoveCursorExt,          lpDriverPDevice
;;DIBLINK CheckCursor,        DIB_CheckCursorExt,         lpDriverPDevice

;;DIBLINK BeginAccess,        DIB_BeginAccess
;;DIBLINK EndAccess,          DIB_EndAccess


;--------------------------Private-Routine-----------------------------;
;keep the C runtime out of our driver
;-----------------------------------------------------------------------;

public __aNulmul
public __aNulrem
public __acrtused

__acrtused:
__aNulrem:
__aNulmul:
        int 3

;*----------------------------------------------------------------------
;Function name:  hook_int2f
;
;Description:    hook INT 2f so we know when the SysVM is background.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
        assume ds:nothing
        assume es:nothing
public hook_int2f
hook_int2f proc far

        mov     ax,4000h  ;  STOP_IO_TRAP
        int     2Fh

        push    ds
        mov     ax,_DATA
        mov     ds,ax
        assume  ds:_DATA

        cmp     [next_int2f],0
        jnz     hook_int2f_exit

        mov     ax, 352Fh                       ; get interrupt vector
        int     21h                             ; get the current vector in ES:BX
        mov     word ptr next_int2f[2], es
        mov     word ptr next_int2f[0], bx

        mov     dx, offset int_2f               ; ax:dx -> int 2f handler
        mov     ax, seg int_2f
        push    ds                              ; save ds
        mov     ds, ax                          ; ds:dx -> int 2f handler
        mov     ax, 252Fh                       ; set interrupt vector
        int     21h                             ; set the new vector
        pop     ds                              ; restore ds

hook_int2f_exit:
        pop     ds
        retf

hook_int2f endp


;*----------------------------------------------------------------------
;Function name:  unhook_int2f
;
;Description:    unhook previously hooked INT 2f.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
        assume ds:nothing
        assume es:nothing

public unhook_int2f
unhook_int2f proc far

        mov     ax,4007h ; START_IO_TRAP
        int     2Fh

        push    ds
        mov     ax,_DATA
        mov     ds,ax
        assume  ds:_DATA

        cmp     [next_int2f],0
        jz      unhook_int2f_exit

        xor     ax,ax
        xor     dx,dx
        xchg    ax,word ptr next_int2f[2]      ; ax:dx -> int 2f handler
        xchg    dx,word ptr next_int2f[0]

        push    ds                              ; save ds
        mov     ds, ax                          ; ds:dx -> int 2f handler
        mov     ax, 252Fh                       ; set interrupt vector
        int     21h                             ; set the new vector
        pop     ds                              ; restore ds

unhook_int2f_exit:
        pop     ds
        retf

unhook_int2f endp


;*----------------------------------------------------------------------
;Function name:  int_2f
;
;Description:    handle the INT 2f.
;
;Information:
;
;Return:         VOID
;----------------------------------------------------------------------*
        assume ds:nothing
        assume es:nothing

int_2f proc far
        push    eax                         ; +34 make retf frame
        push    ds                          ; +32 save this
        pushad                              ; +0  and these

        mov     bx,_DATA
        mov     ds,bx
        assume  ds:_DATA

        mov     bx,sp                       ; ss:bx -> pushad, ds, retf, iret
        mov     ecx,[next_int2f]            ; get next handler
        mov     ss:[bx+34],ecx              ; setup so we retf to it.

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -;
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -;
        cmp     ax,4001h
        je      background

        cmp     ax,4002h
        je      foreground
        
        jmp     int_2f_exit

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -;
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -;
foreground:
        push    es
        call    ToForeground
        pop     es
        jmp     int_2f_exit

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -;
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -;
background:
        push    es
        call    ToBackground
        pop     es

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -;
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -;
int_2f_exit:
        popad
        pop     ds
        retf

int_2f endp


;*----------------------------------------------------------------------
;Function name:  LibEntry
;
;Description:    Called when DLL is loaded.
;
;Information:
; Entry:
;       CX    = size of heap
;       DI    = module handle
;       DS    = automatic data segment
;       ES:SI = address of command line (not used)
; Registers Preserved:
;       SI,DI,DS,BP
; Registers Destroyed:
;       AX,BX,CX,DX,ES,FLAGS
; Calls:
;       None
; History:
;
;       06-27-89 -by-  Todd Laney [ToddLa]
;       Created.
;
;Return:         AX     TRUE  if success or,
;                       FALSE if error
;----------------------------------------------------------------------*
        assume ds:_DATA
        assume es:nothing
        extern  GlobalSmartPageLock:far

LibEntry:
        jcxz    @f
        pushd   0
        push    cx
        call    LocalInit
@@:
        push    di
        call    GetExePtr
        mov     [_hModule],ax

        push    _TEXT
        call    GlobalSmartPageLock
        push    _DATA
        call    GlobalSmartPageLock

        mov     ax,1
        retf


;*----------------------------------------------------------------------
;Function name:  WEP
;
;Description:    Called when the DLL is unloaded.  It is passed 1 WORD
;                parameter that is TRUE if the system is going down,
;                or zero if the app is.
;
;Information:
;
;Return:         AX = 1 is always returned 
;----------------------------------------------------------------------*
        assume ds:nothing
        assume es:nothing

public  WEP
WEP     proc far
        push    bp      
        mov     bp, sp
        mov     ax, [bp+6]
        or      ax, ax
        jz      short SkipUnhook
        call    unhook_int2f
SkipUnhook:
        mov     ax,1
        pop     bp
        retf    2

WEP     endp

_TEXT ends

        end     LibEntry

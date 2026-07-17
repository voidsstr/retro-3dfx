;/* $Header: /devel/h3/Win95/dx/dd16/xhwcext.asm 2     12/30/98 2:37p Michael $ */
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
;** File name:   xhwcext.asm
;**
;** Description: Set/clear GDI's busy bit.
;**
;** $Revision: 2 $
;** $Date: 12/30/98 2:37p $
;**
;** $History: xhwcext.asm $
;; 
;; *****************  Version 2  *****************
;; User: Michael      Date: 12/30/98   Time: 2:37p
;; Updated in $/devel/h3/Win95/dx/dd16
;; Implement the 3Dfx/STB unified header.
;**
;*/

;----------------------------------------------------------------------------
;----------------------------------------------------------------------------
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
;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

BUSY            equ     0000000000010000b      ; from ddk95/inc16/dibeng.inc      
BUSYBIT         equ     4h


;*----------------------------------------------------------------------
;Function name: hwcSetGDIBusy
;
;Description:   Set the GDI busy bit.
;
;Information:   
;
;Return:        AX      1 if success, 0 if already busy.
;----------------------------------------------------------------------*
hwcSetGDIBusy PROC C USES edi es  lpGDISemaphore :DWORD,
 
        les di, lpGDISemaphore
        bts WORD PTR es:[di], BUSYBIT
        jc  alreadyBusy
        mov ax , 1
        ret
        
alreadyBusy:

        mov ax, 0
        ret
 
hwcSetGDIBusy endp


;*----------------------------------------------------------------------
;Function name: hwcClearGDIBusy
;
;Description:   Clear the GDI busy bit.
;
;Information:   
;
;Return:        VOID
;----------------------------------------------------------------------*
hwcClearGDIBusy PROC C USES edi es  lpGDISemaphore :DWORD,
 
        les di, lpGDISemaphore
        btr WORD PTR es:[di], BUSYBIT ; just being lazy about computing a mask
        ret
 
hwcClearGDIBusy endp

_TEXT ends
END

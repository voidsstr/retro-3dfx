;
;	FFF0DATA.ASM - Holds the strapping options data for Napalm2.
;	Copyright (c) 1990-2000 3dfx Interactive, Inc.
;	All rights reserved.
;
;	Written by:	Kaymann L. Woo
;	Date:			7/21/00
;
;	$Revision$
;	$Date$
;	$Logfile$
;	$Log$

include oem.inc

_TEXT	SEGMENT word common USE16 'CODE'

	Public	NewData

	Org	0FFF0h

NewData:

IF	((NumberOfChips eq 4) or (NumberOfChips eq 2))
	dw	00000h	;0xFFF6/0xFFF7
	dw	00000h	;0xFFF4/0xFFF5
	dw	0C400h	;0xFFF2/0xFFF3
	dw	00000h	;0xFFF0/0xFFF1
ELSE
	dw	00000h	;0xFFF6/0xFFF7
	dw	00000h	;0xFFF4/0xFFF5
	dw	0C400h	;0xFFF2/0xFFF3
	dw	00000h	;0xFFF0/0xFFF1
ENDIF

_TEXT	EndS

	End


	page	57, 132
	title	Initialization and Dispatch Routines
	subttl	MAIN.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	MAIN.ASM - Initialization and dispatch routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	Main		Entry point to RAM version loader
;	HandlerINT10	INT 10h function handler
;	HandlerINT6D	INT 6Dh function handler
;	NOPFunction	Do nothing BIOS call
;
;	$Revision: 3 $
;	$Date: 6/16/99 1:33p $
;	$Logfile: /devel/h5/BIOS/SRC/MAIN.ASM $
;	$Log: /devel/h5/BIOS/SRC/MAIN.ASM $
;; 
;; 3     6/16/99 1:33p Kaymann
;; In init_code, made a call to Patch32KROM when compiling a 32K ROM BIOS
;; only.
;; 
;; 2     5/11/99 2:48p Kaymann
;; Set the 32K ROM size to report a 40K ROM.  This is later patched to
;; report
;; 32K by the routine InitBIOS.
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
IFDEF ROM
	%out	!	Assembling: MAIN.ASM (ROM Version - ROMMAIN.OBJ)
ELSE
	%out	!	Assembling: MAIN.ASM (RAM Version - MAIN.OBJ)
ENDIF
;
.MODEL TINY
.CODE
.386
;
	extrn	SetMode:near
	extrn	SetCursorType:near
	extrn	SetCursorPosition:near
	extrn	GetCursorPosition:near
	extrn	ReadLightpen:near
	extrn	SetActivePage:near
	extrn	ScrollUp:near
	extrn	ScrollDown:near
	extrn	ReadCharacter:near
	extrn	WriteCharAttribute:near
	extrn	WriteChar:near
	extrn	SetCGAPalette:near
	extrn	WriteDot:near
	extrn	ReadDot:near
	extrn	WriteTTY:near
	extrn	GetMode:near
	extrn	PaletteHandler:near
	extrn	FontHandler:near
	extrn	AlternateSelect:near
	extrn	WriteString:near
	extrn	GetDccInfo:near
	extrn	GetFunctionalityInfo:near
	extrn	SaveRestoreState:near
IF VESA
	extrn	HandlerVESA:near
ENDIF
	extrn	InitBIOS:near
	extrn	GetDSSeg0:near
	extrn	OEMInitBIOS:near
;
IF EXTENDED_BIOS
	extrn	OEMExtensions1:near
	extrn	OEMExtensions2:near
ENDIF
;
	extrn	BIOSSIZE:byte
	extrn	tblROMConfig:word
	extrn	selBIOSData:word
IFNDEF ROM
	extrn	segInitParm:word
	extrn	segInit2ndSP:word
	extrn	segInitDCC:word
	extrn	segInitOEM:word
	extrn	segfi8x14:word
	extrn	segfi8x8:word
	extrn	segfi8x8Top:word
	extrn	segfi9x14:word
	extrn	segfi8x16:word
	extrn	segfi9x16:word
	extrn	segROM8x8:word
	extrn	segROM8x8Top:word
	extrn	segROM8x14:word
	extrn	segROM8x16:word
	extrn	segROM9x14:word
	extrn	segROM9x16:word
	extrn	segOEMExtRegs:word
	extrn	segOEMModeDesc:word
	extrn	selCodeSeg:word
ENDIF
IF PCIBIOS
	extrn	tblPCIInfo:byte
ENDIF

IFDEF ROM
IFE	ROM_64K
	extrn	Patch32KROM:near
ENDIF
ENDIF

;
;
IFDEF ROM
	public	Start
Start:
	db	055h, 0AAh 
IF   ROM_64K
;We must be a 40K ROM
  db 050h

;We must be a 48K ROM
;  db 060h

;We must be a 64K ROM
;  db 080h

ELSE
;We must be a 40K ROM
  db 050h

;We must be a 32K ROM
;  db 040h
ENDIF
	jmp	short FirstJump			; This MUST be a short jump!
;
; For compatibility with IBM AT diagnostics disk
;
	org	005h
	db	'7400'
	db	0E9h, 04Ch, 019h
;
; For compatibility with PS/2 diagnostics
;
	org	00Ch
	db	077h, 0CCh, "VIDEO ", 00Dh, 000h
;
; Pointer to PCI table must go at offset 18h
;
	org	018h
IF PCIBIOS
	dw	offset tblPCIInfo
ELSE
	dw	0
ENDIF
;
; The letters "IBM" must be at offset 1Eh
;
	org	01Eh
	db	"IBM VGA Compatible", 000h
;
; On some microchannel machines, the POS (Programmable Option Select)
; registers (ports 102h thru 105h) are initialized with the data found
; at offset 030h of the adapter's expansion ROM. Though most PS/2's don't
; do this for video ROMs, that may change in the future.
;
	org	030h
	db	001h, 000h, 000h, 000h
;
; Auto date stamping and vendor ID placed here by MAKEIMG
;
	org	038h
	db	'MM/DD/YY'
	db	'051562LGC       '
;
	org	050h
	dw	offset tblROMConfig
;
	org	052h			; This may be moved, but never lower than 52h
	public	FirstJump
FirstJump:
	jmp	init_code		; This may be a long or short jump
;
	db	00h
	db STR_OEMPRODUCT
	db	"BIOS - Version "
	db	STR_OEMVERSION
	db	" Copyright (C) 1990-2000 Elpin Systems, Inc.", 0Dh, 0Ah, 00h

	org 0A0h
	public VitalProductData
VitalProductData	label	byte
; This is will be the info normally built in
	db	082h			; ID String Tag
	dw	008h			; LR Length: word (8 bytes)
	db	'3Dfx VPD'		; ID String
	;33,44,66,78,20,56,50,44 = 25Fh
	;Subtotal = 2E9h

	db	090h			; VPD Tag
	dw  03Dh			; LR Length: word (59 bytes)
	;Subtotal = CDh


	db	'PN' ;50,4E=9Eh	; Part No. Tag
	db  007h			; SR Length: byte (10 bytes)
	db  '0000000'	; Blank Data
	;(7*30) = 150h
	;Subtotal = 1F5h

	db  'EC' ;45,43=88h	; ECO Level Tag
	db	003h			; SR Length: byte (3 bytes) 
	db  '000'			; Blank Data
	;(3*30) = 90h
	;Subtotal = 11Bh

	db	'SN' ;53,4E=A1h	; Serial # Tag
	db	01Eh			; SR Length: byte (30 bytes)
	db	'                              '	; All Blanks
	;(1E*20) = 3C0h
	;Subtotal = 47Fh

	db	'RL' ;52,45=97h	; ROM Level Tag 
	db	007h			; SR Length: byte (6 bytes)
	db	'1.11   '
	;31,2E,31,31,20,20,20 = 121h
	;Subtotal = 1BFh

	db	079h			; End Tag
	db	07Ch			; Blank VPD Block Checksum byte

;
init_code:
	push	bp
	call	GetDSSeg0		; DS = Seg0
	push	cs
	pop	es			; ES = ROM segment
	call	OEMInitBIOS		; Load BIOS?
	jc	short init_exit		;  N: Skip initialization
	call	InitBIOS

IFDEF ROM
IFE	ROM_64K
	call	Patch32KROM
ENDIF
ENDIF

init_exit:
	pop	bp
	retf
;
ELSE
	org	0100h
Start:
	jmp	short Main

;Created this dummy label so that the RAM BIOS would compile.
;This isn't actually pointing to the VPD data.

	public VitalProductData
VitalProductData	label	byte

ENDIF
;
	public	tblBIOSFunctions, MAX_FUNCTIONS
tblBIOSFunctions	label	word
	dw	SetMode				; Function 00h
	dw	SetCursorType			; Function 01h
	dw	SetCursorPosition		; Function 02h
	dw	GetCursorPosition		; Function 03h
	dw	ReadLightpen			; Function 04h
	dw	SetActivePage			; Function 05h
	dw	ScrollUp			; Function 06h
	dw	ScrollDown			; Function 07h
	dw	ReadCharacter			; Function 08h
	dw	WriteCharAttribute		; Function 09h
	dw	WriteChar			; Function 0Ah
	dw	SetCGAPalette			; Function 0Bh
	dw	WriteDot			; Function 0Ch
	dw	ReadDot				; Function 0Dh
	dw	WriteTTY			; Function 0Eh
	dw	GetMode				; Function 0Fh
	dw	PaletteHandler			; Function 10h
	dw	FontHandler			; Function 11h
	dw	AlternateSelect			; Function 12h
	dw	WriteString			; Function 13h
IF EXTENDED_BIOS
	dw	OEMExtensions1			; Function 14h
	dw	OEMExtensions2			; Function 15h
ELSE
	dw	NOPFunction			; Function 14h
	dw	NOPFunction			; Function 15h
ENDIF
	dw	NOPFunction			; Function 16h
	dw	NOPFunction			; Function 17h
	dw	NOPFunction			; Function 18h
	dw	NOPFunction			; Function 19h
	dw	GetDccInfo			; Function 1Ah
	dw	GetFunctionalityInfo		; Function 1Bh
	dw	SaveRestoreState		; Function 1Ch
MAX_FUNCTIONS	equ	($ - tblBIOSFunctions) / 2
;
;	HandlerINT10 - INT 10h function handler
;
;	Entry:	Dependent on function
;	Exit:	Dependent on function
;
	public	HandlerINT10
HandlerINT10	proc
	assume	ds:nothing, es:nothing
IF (VARIATION_002 eq 0)
	int	6Dh			; Dispatch to INT 6Dh handler
	iret
HandlerINT10	endp
ENDIF
;
;	HandlerINT6D - INT 6Dh function handler
;
;	Entry:	Dependent on function
;	Exit:	Dependent on function
;
	public	HandlerINT6D
HandlerINT6D	proc
	assume	ds:Seg0, es:nothing
	sti
	cld
	push	si
	push	ds
	mov	si,ax
	mov	ds,cs:[selBIOSData]
	shr	si,8
	shl	si,1
IF VESA
	cmp	ah,04Fh
	je	HandlerVESA
ENDIF
	cmp	ah,MAX_FUNCTIONS
	jae	short NOPFunction
	jmp	cs:[tblBIOSFunctions + si]
HandlerINT6D	endp
;
IF (VARIATION_002 eq 1)
HandlerINT10	endp
ENDIF
;
;	NOPFunction - This function fills the holes in the jump table with a NULL function
;
;	Entry:	SI, DS are pushed on the stack
;	Exit:	None
;
	public	NOPFunction
NOPFunction	proc
	pop	ds
	pop	si
	iret
NOPFunction	endp
;
IFNDEF ROM
;
;	Main - Entry point to RAM version loader
;
;	Entry:	None
;	Exit:	None
;
	public	Main
Main	proc
	assume	ds:nothing, es:nothing
;
; Point to local data structures
;
	push	cs
	pop	ds
	assume	ds:_TEXT
	call	OEMInitBIOS			; Load BIOS?
	jnc	short Main_05			;  Y: Continue
	mov	ax,4C01h			;  N: Exit to DOS
	int	21h
;
Main_05:
	cli
	mov	[segInitParm],cs
	mov	[segInit2ndSP],cs
	mov	[segInitDCC],cs
	mov	[segInitOEM],cs
	cmp	[segOEMExtRegs],0		; Is this a NULL ptr?
	jz	short Main_10			;  Y: Leave it a NULL ptr
	mov	[segOEMExtRegs],cs		;  N: Update to the new segment
Main_10:
	mov	[segOEMModeDesc],cs
	mov	[segfi8x14],cs
	mov	[segfi8x8],cs
	mov	[segfi8x8Top],cs
	mov	[segfi9x14],cs
	mov	[segfi8x16],cs
	mov	[segfi9x16],cs
	mov	[segROM8x8],cs
	mov	[segROM8x8Top],cs
	mov	[segROM8x14],cs
	mov	[segROM8x16],cs
	mov	[segROM9x14],cs
	mov	[segROM9x16],cs
	mov	[selCodeSeg],cs
	sti
;
	call	GetDSSeg0		; DS = Seg0
	push	cs
	pop	es			; ES = ROM segment
	assume	ds:Seg0, es:_TEXT
;
	call	InitBIOS
	mov	dx,offset BIOSSIZE
	int	27h			; Terminate and remain resident
Main	endp
;
ENDIF
;
	end	Start
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


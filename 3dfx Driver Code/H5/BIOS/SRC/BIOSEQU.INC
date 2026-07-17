;
;	BIOSEQU.INC - BIOS equates
;	Copyright (c) 1990-1997 Elpin Systems, Inc.
;	All rights reserved.
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	$Revision$
;	$Date$
;	$Logfile$
;	$Log$
;
;
;
MAXIMUM_STD_MODE	equ	013h		; VGA is mode 0 thru 13h
NULL_MODE		equ	07Fh		; Mode to "do nothing"
MAX_OEMCOPYRIGHT	equ	120		; Maximum of 120 bytes
;
; "fEGAInfo" equates
;
NO_CURSOR_EMULATE	equ	00000001b
VGA_MONOCHROME		equ	00000010b
WAIT_RETRACE		equ	00000100b
VGA_INACTIVE		equ	00001000b
NO_REGEN_CLEAR		equ	10000000b
;
; "fVGAInfo" equates
;
ALL_MODES		equ	00000001b
PALETTE_SUMMING		equ	00000010b
MONO_ANALOG		equ	00000100b
NO_DEFAULT_PALETTE	equ	00001000b
SCAN_400_ACTIVE		equ	00010000b
DISPLAY_SWITCH_ACTIVE	equ	01000000b
SCAN_200_ACTIVE		equ	10000000b
;
; I/O addresses
;
CRTC_MINDEX		equ	03B4h
CRTC_MDATA		equ	03B5h
CRTC_CINDEX		equ	03D4h
CRTC_CDATA		equ	03D5h
INPUT_MSTATUS		equ	CRTC_MINDEX + 6
INPUT_CSTATUS		equ	CRTC_CINDEX + 6
ATC_INDEX		equ	03C0h
ATC_RDATA		equ	03C1h
MISC_OUTPUT		equ	03C2h
SUBSYS_ENABLE		equ	03C3h
SEQ_INDEX		equ	03C4h
SEQ_DATA		equ	03C5h
DAC_MASK		equ	03C6h
DAC_RINDEX		equ	03C7h
DAC_WINDEX		equ	03C8h
DAC_DATA		equ	03C9h
MISC_INPUT		equ	03CCh
GDC_INDEX		equ	03CEh
GDC_DATA		equ	03CFh
;
; 8514/A-Class I/O addresses
;
ROM_PAGE		equ	046E8h
SUBSYS_STAT		equ	042E8h
SUBSYS_CNTL		equ	042E8h
ADVFUNC_CNTL		equ	04AE8h
MULT_MISC		equ	0BEE8h
;
;  Useful timing related I/O addresses
;
PORTB			equ	061h
FLAG_REFRESHSTATUS	equ	010h		; Port B refresh signal status
IODELAYPORT		equ	0EDh		; Unused I/O port
;
BUS_ISA			equ	001h
BUS_PCI			equ	002h
BUS_VESA		equ	003h
;
; Structures
;
SEQ	struc			; Sequencer parameters
  ClockMode			db	?
  MapMask			db	?
  FontAddr			db	?
  MemMode			db	?
SEQ	ends
;
CRTC	struc			; CRTC parameters
  HTotal			db	?
  HDispEnd			db	?
  HBlankStart			db	?
  HBlankEnd			db	?
  HSyncStart			db	?
  HSyncEnd			db	?
  VTotal			db	?
  Overflow			db	?
  PresetRowScan			db	?
  CellHeight			db	?
  CursorScanStart		db	?
  CursorScanEnd			db	?
  RegenStartHigh		db	?
  RegenStartLow			db	?
  CursorPositHigh		db	?
  CursorPositLow		db	?
  VSyncStart			db	?
  VSyncEnd			db	?
  VDispEnd			db	?
  RowOffset			db	?
  ULineRow			db	?
  VBlankStart			db	?
  VBlankEnd			db	?
  Mode				db	?
  LineCompare			db	?
CRTC	ends
;
ATC	struc			; ATC parameters
  Palette			db	16 dup (?)
  Mode				db	?
  Overscan			db	?
  PlaneEnable			db	?
  Panning			db	?
ATC	ends
;
GDC	struc			; GDC parameters
  SetReset			db	?
  EnabSetReset			db	?
  ColorCmp			db	?
  ROP				db	?
  ReadMap			db	?
  Mode				db	?
  Misc				db	?
  CDC				db	?
  BitMask			db	?
GDC	ends
;
PARMTABLE	struc		; Parameter table
  Columns			db	?
  Rows				db	?
  CharHeight			db	?
  RegenLength			dw	?
  SEQ				db	size SEQ dup (?)
  Misc				db	?
  CRTC				db	size CRTC dup (?)
  ATC				db	size ATC dup (?)
  GDC				db	size GDC dup (?)
PARMTABLE	ends
;
SAVEPTR		struc		; Save pointer
  ParmTablePtr			dd	?
  SaveAreaPtr			dd	?
  AuxTextFontPtr		dd	?
  AuxGraphicsFontPtr		dd	?
  SecondarySavePtr		dd	?
  Reserved0			dd	?
  Reserved1			dd	?
SAVEPTR		ends
;
SAVEPTR2	struc		; Secondary save pointer
  TableLength			dw	?
  DCCPtr			dd	?
  Aux2TextFontPtr		dd	?
  PaletteProfilePtr		dd	?
  Reserved0			dd	?
  Reserved1			dd	?
  OEMData			dd	?
SAVEPTR2	ends
;
AUXTEXTFONT	struc
  CharHeight			db	?
  Block				db	?
  Count				dw	?
  Start				dw	?
  Font				dd	?
  Rows				db	?
  ModeList			db	?	; String of bytes terminated with 0FFh
AUXTEXTFONT	ends
;
AUXGRAPHICSFONT	struc
  Rows				db	?
  CharHeight			dw	?
  Font				dd	?
  ModeList			db	?	; String of bytes terminated with 0FFh
AUXGRAPHICSFONT	ends
;
AUXTEXTFONT2	struc
  CharHeight			db	?
  Block				db	?
  Reserved			db	?
  Font				dd	?
  ModeList			db	?	; String of bytes terminated with 0FFh
AUXTEXTFONT2	ends
;
PALETTEPROFILE	struc		; User palette profile
  UnderlineFlag			db	?
  ReservedByte			db	?
  ReservedWord			dw	?
  ATCCount			dw	?
  ATCIndex			dw	?
  ATCDataPtr			dd	?
  DACCount			dw	?
  DACIndex			dw	?
  DACDataPtr			dd	?
  ModeList			db	?	; String of bytes terminated with 0FFh
PALETTEPROFILE	ends
;
OEMDATA		struc
  oemExtRegs			dd	?
  oemModeDesc			dd	?
OEMDATA		ends
;
LENGTH_EXTSTATE			equ	256	; Maximum size of OEM save structure
;
MDTYPE_CTEXT			equ	0	; Color text
MDTYPE_MTEXT			equ	1	; Monochrome text
MDTYPE_CGA			equ	2	; CGA 4- or 2-color mode
MDTYPE_MGRAF			equ	3	; EGA Monochrome graphics
MDTYPE_1BPP			equ	4	; Normal 2-color mode
MDTYPE_4BPP			equ	5	; Planar graphics
MDTYPE_8BPP			equ	6	; 8 BPP graphics
MDTYPE_16BPP			equ	7	; 16 BPP graphics
MDTYPE_24BPP			equ	8	; 24 BPP graphics
MDTYPE_NULL			equ	9	; Null Mode
; 	
MODEDESC	struc
  mdMode			db	?	; VGA Mode number
  mdIMode			db	?	; Internal Mode number
  mdVMode			dw	?	; VESA Mode number
  mdType			db	?	; Mode type
  mdPages			db	?	; Number of video pages
  mdBPP				db	?	; Number of bits per pixel
  mdXRes			dw	?	; X Resolution
  mdYRes			dw	?	; Y Resolution
  mdRowOff			dw	?	; Row Offset
  mdCharHeight			db	?	; Character Height
MODEDESC	ends
;
;	MONO_OUT - Output a string to a co-resident monochrome monitor
;
MONO_OUT	MACRO	S, nocrlf
	LOCAL	__szMessage, __Label_10
IFDEF DEBUG
	jmp	short __Label_10
__szMessage	db	S
  IFB	<nocrlf>
		db	00Dh, 00Ah
  ENDIF
		db	000h
;
__Label_10:
	pushfd
	pushad
	push	cs
	push	ds
	push	es
	push	ss
	mov	esi,offset __szMessage
  IFNDEF MonoOut
	extrn	MonoOut:near
  ENDIF
	call	MonoOut
	pop	ax		; SS
	pop	es
	pop	ds
	pop	ax		; CS
	popad
	popfd
ENDIF
ENDM
;
;	COM_OUT - Output a string to a COM port
;
COM_OUT	MACRO	S, nocrlf
	LOCAL	__szMessage, __Label_10
IFDEF DEBUG
	jmp	short __Label_10
__szMessage	db	S
  IFB	<nocrlf>
		db	00Dh, 00Ah
  ENDIF
		db	000h
;
__Label_10:
	pushfd
	pushad
	push	cs
	push	ds
	push	es
	push	ss
	mov	esi,offset __szMessage
  IFNDEF COMOut
	extrn	COMOut:near
  ENDIF
	call	COMOut
	pop	ax		; SS
	pop	es
	pop	ds
	pop	ax		; CS
	popad
	popfd
ENDIF
ENDM
;
;	ALIGN_DWORD - Align a data item to a DWORD boundary by filling with NOP's
;
ALIGN_DWORD	MACRO	seg
	LOCAL	segname
IFNB	<seg>
	segname	EQU	seg
ELSE
	segname	EQU	_TEXT
ENDIF
IF (($ - offset segname:0) and 3)
	db	(4 - (($ - offset segname:0) and 3)) dup (090h)
ENDIF
ENDM
;
;	Copyright (c) 1990-1997 Elpin Systems, Inc.
;	All rights reserved.
;


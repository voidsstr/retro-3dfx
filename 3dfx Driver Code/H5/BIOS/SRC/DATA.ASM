	page	57, 132
	title	Data Tables
	subttl	DATA.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	DATA.ASM - Data tables
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/DATA.ASM $
;	$Log: /devel/h5/BIOS/SRC/DATA.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: DATA.ASM
;
.MODEL TINY
.CODE
.386
;
	extrn	tblOEMData:byte
	extrn	szOEMCopyright:byte
IF EXTENDED_BIOS
	extrn	tblOEMConfig:near
ENDIF
IF VESA_300
	extrn	VBE_PMEntryPoint:near
	extrn	VBE_PMInitialization:near
ENDIF
;
	include	BIOSPARM.INC
; Extended parameter tables MUST follow "BIOSPARM.INC"
IF EXTENDED_BIOS
	include	OEMPARM.INC
ENDIF
;
	include	FONT8X8.INC
	include	FONT8X14.INC
	include	FONT8X16.INC
;
; The following variations are used to include both, one, or none
; of the fixup tables:
;			VARIATION_003 = 0	VARIATION_003 = 1
; VARIATION_004 = 0	Normal, both included	9x16 included, 9x14 not
; VARIATION_004 = 1	Neither included	Neither included
;
IF (VARIATION_003 eq 0)
  IF (VARIATION_004 eq 0)
	include	FONT9X14.INC
	include	FONT9X16.INC
  ELSE
	public	tblFont9x14
tblFont9x14	label	byte
	db	000h
	include	FONT9X16.INC
  ENDIF
ELSE
	public	tblFont9x14, tblFont9x16
tblFont9x14	label	byte
tblFont9x16	label	byte
	db	000h
ENDIF
	include	DACDATA.INC
;
	public	tblROMConfig
tblROMConfig	label	word
IF EXTENDED_BIOS
	dw	offset tblOEMConfig		; OEM specific configuration
ELSE
	dw	0
ENDIF
	dw	offset tblSavePtr		; Pointer to save ptr table
	dw	offset tblFont8x8		; 8x8 font
	dw	offset tblFont8x14		; 8x14 font
	dw	offset tblFont8x16		; 8x16 font
	dw	offset tblFont9x14		; 9x14 fixup
	dw	offset tblFont9x16		; 9x16 fixup
	dw	offset szOEMCopyright		; OEM copyright string
;
	public	szSignonMsg, LENGTH_SIGNON_MSG
		db	0			; For NT diagnostics
szSignonMsg	db STR_OEMPRODUCT
IF      (DVI_BIOS OR TVOUT_BIOS)
		db "Supported "
ENDIF
		db	"BIOS - Version "
		db	STR_OEMVERSION
		db	0Dh, 0Ah
		db	"Copyright (C) 1990-2000 Elpin Systems, Inc.", 0Dh, 0Ah
		db	"All rights reserved.", 0Dh, 0Ah
LENGTH_SIGNON_MSG	equ	($ - szSignonMsg)
		db	00h
;
	public	tblSavePtr, segInitParm, segInit2ndSP
tblSavePtr	label	dword
		dw	tblStdParameters
segInitParm	dw	ROM_SEGMENT
		dw	0
		dw	0
		dw	0
		dw	0
		dw	0
		dw	0
		dw	tbl2ndSavePtr
segInit2ndSP	dw	ROM_SEGMENT
		dw	0
		dw	0
		dw	0
		dw	0
;
	public	tbl2ndSavePtr, segInitDCC, segInitOEM
tbl2ndSavePtr	label	dword
		dw	26
		dw	tblInitDCC
segInitDCC	dw	ROM_SEGMENT
		dw	0
		dw	0
		dw	0
		dw	0
		dw	0
		dw	0
		dw	0
		dw	0
		dw	tblOEMData
segInitOEM	dw	ROM_SEGMENT
;
	public	tblInitDCC
tblInitDCC	label	dword
	db	010h
	db	001h
	db	008h
	db	000h
	db	000h, 000h
	db	000h, 001h
	db	000h, 002h
	db	002h, 001h
	db	000h, 004h
	db	004h, 001h
	db	000h, 005h
	db	002h, 005h
	db	000h, 006h
	db	001h, 006h
	db	005h, 006h
	db	000h, 008h
	db	001h, 008h
	db	000h, 007h
	db	002h, 007h
	db	002h, 006h
;
	public	tblFunctionality
tblFunctionality	label	byte
	db	0FFh, 0E0h, 00Fh		; Modes 0-7, D-13
	db	000h, 000h, 000h, 000h
	db	007h				; 200, 350, 400 scan lines
	db	002h				; Max 2 blocks available @ time
	db	008h				; 8 blocks active
	db	0FFh
	db	00Eh				; No lightpen
	db	000h, 000h
	db	03Fh				; Saveptr functions
	db	000h
;
; To implement the protected mode interface for VBE 3.0, all code must
; be able to be run in real mode and 16:16 protected mode. The only
; major difference between the two environments are the segments /
; selectors. Therefore, instead of hardwiring segments into the code,
; the segment/selectors are grabbed from a data structure that a VBE
; 3.0-aware application would know about and fixup appropriately.
;
if VESA_300
	public	tblPMInfoBlock, byInProtMode, byPMICheckSum
	public	selBIOSData, selA0000, selB0000, selB8000, selCodeSeg
	ALIGN_DWORD
tblPMInfoBlock	label	byte
		db	'PMID'
		dw	offset VBE_PMEntryPoint
		dw	offset VBE_PMInitialization
selBIOSData	dw	00000h
selA0000	dw	0A000h
selB0000	dw	0B000h
selB8000	dw	0B800h
selCodeSeg	dw	ROM_SEGMENT
byInProtMode	db	000h
byPMICheckSum	db	000h
ELSE
	public	selBIOSData, selA0000, selB0000, selB8000, selCodeSeg
selBIOSData	dw	00000h
selA0000	dw	0A000h
selB0000	dw	0B000h
selB8000	dw	0B800h
selCodeSeg	dw	ROM_SEGMENT
ENDIF
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;



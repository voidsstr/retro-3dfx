;
;	BIOSVARS.INC - BIOS variables
;	Copyright (c) 1990-1997 Elpin Systems, Inc.
;	All rights reserved.
;
;	Written by:	Larry Coffey
;	Date:		5/21/90
;
;	$Revision$
;	$Date$
;	$Logfile$
;	$Log$
;
;
Seg0	segment at 0
;
; Print screen vector
;
	org	005h*4
vecINT05	label	dword
offINT05	dw	?
segINT05	dw	?
;
; BIOS entry point
;
	org	010h*4
vecINT10	label	dword
offINT10	dw	?
segINT10	dw	?
;
; Pointer to 6845 parameter table (CGA emulation)
;
	org	01Dh*4
vecINT1D	label	dword
offINT1D	dw	?
segINT1D	dw	?
;
; Upper 128 bytes of CGA character set
;
	org	01Fh*4
vecINT1F	label	dword
offINT1F	dw	?
segINT1F	dw	?
;
; Motherboard Redirection vector
;
	org	042h*4
vecINT42	label	dword
offINT42	dw	?
segINT42	dw	?
;
; Font vector
;
	org	043h*4
vecINT43	label	dword
offINT43	dw	?
segINT43	dw	?
;
; Extra vector VGA uses to redirect INT 10h calls
;
	org	06Dh*4
vecINT6D	label	dword
offINT6D	dw	?
segINT6D	dw	?
;
; Equipment flag:
;
;	x x 1 1 x x x x = monochrome equipment
;	x x 1 0 x x x x = color equipment
;
	org	0410h
fEquip		db	?
;
; Standard BIOS variables
;
	org	0449h
byMode		db	?		; Enumerated video state
wColumns	dw	?		; Number of columns on screen
wRegenLength	dw	?		; Length of video buffer
wRegenStart	dw	?		; Start of video buffer
wCursorPosition	dw	8 dup (?)	; Cursor position for each page
wCursorType	label	word		; Cursor start and stop scan line
byCursorStop	db	?
byCursorStart	db	?
byActivePage	db	?		; Current page
wCRTCAddr	dw	?		; I/O address of CRTC (3D4h or 3B4h)
byCGA3D8	db	?		; CGA compatible variable
byCGA3D9	db	?		; CGA compatible variable
;
	org	046Ch
dwSysTimer	label	dword
wSysTimerLow	dw	?		; Low word of timer tick variable
wSysTimerHigh	dw	?		; High word of timer tick variable
fReset		dw	?		; 1234h if warm boot
;
; BIOS variables added with introduction of the EGA
;
	org	0484h
byRows		db	?		; Number of rows on screen minus one
wCharHeight	dw	?		; Number of scan lines in character
fEGAInfo	db	?		; Bit 0: 0 = Emulate cursor type
					; Bit 0: 1 = Set cursor type as is
					; Bit 1: 0 = VGA is color
					; Bit 1: 1 = VGA is monochrome
					; Bit 2: 0 = Write to memory anytime
					; Bit 2: 1 = Wait for retrace to write
					; Bit 3: 0 = VGA active
					; Bit 3: 1 = VGA inactive
					; Bit 4: reserved must be 0
					; Bit 5: reserved must be 1
					; Bit 6: reserved must be 1
					; Bit 7: 0 = Clear regen on set mode
					; Bit 7: 1 = Don't clear regen buffer
fEGAInfo3	db	?		; EGA feature bits and dip switches
fVGAInfo	db	?		; Bit 0: 0 = All modes inactive
					; Bit 0: 1 = All modes active
					; Bit 1: 0 = Palette summing inactive
					; Bit 1: 1 = Palette summing active
					; Bit 2: 0 = Mono monitor inactive
					; Bit 2: 1 = Mono monitor active
					; Bit 3: 0 = Default palette active
					; Bit 3: 1 = Default palette inactive
					; Bit 4: 0 = 400 scan line inactive
					; Bit 4: 1 = 400 scan line active
					; Bit 5: reserved must be 0
					; Bit 6: 0 = Display switch inactive
					; Bit 6: 1 = Display switch active
					; Bit 7: 0 = 200 scan line inactive
					; Bit 7: 1 = 200 scan line active
byDCC		db	?		; Display combination code
;
	org	04A8h
lpSavePtr	dd	?		; Pointer to the save pointer table
;
Seg0	ends
;
;	Copyright (c) 1990-1997 Elpin Systems, Inc.
;	All rights reserved.
;


	page	57, 132
	title	Set Mode Routines
	subttl	MODE.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	MODE.ASM - Mode set code
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	SetMode				Set video mode
;	AdjustInfoVars			Adjust mode control variables and call CGA/MDA BIOS if needed.
;	SetBIOSVariables		Setup BIOS variables
;	SetRegs				Set VGA registers
;	LoadPalette			Load the system palette and the user defined palette if there is one
;	LoadDefaultRAMDAC		Load the default palette
;	LoadColorTextDACTable		Load the DAC for the color text modes
;	LoadColorDACTable		Load the DAC for the 16-color modes
;	LoadCompressedDACTable		Load a compressed DAC table
;	LoadCGADACTable			Load the DAC for the CGA-type modes
;	LoadMonochromeDAC		Load the DAC for the monochrome mode
;	LoadColor256DAC			Load the DAC for the 256-color modes
;	LoadSysFont			Load the system font
;	LoadDefaultATCAndSavePtr	Load a the default ATC registers and store them in the saveptr area if necessary
;	IsModeInTable			Check for the current mode number in the mode number stream.
;	GetMode				Get video information
;	IsNullMode			Is there a request to set a mode of type NULL
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/MODE.ASM $
;	$Log: /devel/h5/BIOS/SRC/MODE.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: MODE.ASM
;
.MODEL TINY
.CODE
.386
;
	extrn	IsNullMode:near
	extrn	FullCpuOn:near
	extrn	FullCpuOff:near
	extrn	IsValidMode:near
	extrn	IsModeMono:near
	extrn	IsTextMode:near
	extrn	Is9DotMode:near
	extrn	GetFontPtr:near
	extrn	PreFontLoad:near
	extrn	PostFontLoad:near
	extrn	GetFixupPtr:near
	extrn	LoadFont:near
	extrn	LoadFixup:near
	extrn	ATCOff:near
	extrn	ATCOn:near
	extrn	SetPaletteAndOverscanFromBuffer:near
	extrn	SetRAMDACRangeWithSumming:near
	extrn	LoadIndexRange:near
	extrn	OEMSetRegs:near
	extrn	OEMGetInternalMode:near
	extrn	OEMClearScreen:near
	extrn	OEMFixupFont:near
	extrn	OEMPreSetRegs:near
	extrn	OEMGetParmTable:near
	extrn	WaitVerticalRetrace:near
	extrn	GetIndexRegister:near
	extrn	GetDSSeg0:near
	extrn	GetESSeg0:near
	extrn	SetDACRegisterAutoIndex:near
IF (VARIATION_005 eq 1)
	extrn	OEMWriteRAMDACIdx:near
	extrn	OEMWriteRAMDACData:near
ENDIF
IF EXTENDED_BIOS
	extrn	OEMLoad16BPPDAC:near
	extrn	OEMLoad24BPPDAC:near
ENDIF
;
	extrn	tbl16ColorDAC:byte
	extrn	LENGTH_16COLOR_DAC_TABLE:abs
	extrn	tblCGADAC:byte
	extrn	LENGTH_CGA_DAC_TABLE:abs
	extrn	tblMonochromeDAC:byte
	extrn	tbl256ColorDAC:byte
	extrn	LENGTH_256COLOR_DAC_TABLE:abs
	extrn	GetModeDescPtr:near
;
;	Function 00h:
;	SetMode - Set video mode
;
;	Entry:	SI, DS are pushed on stack
;		AL = Mode number
;		DS = Seg0
;	Exit:	None
;
	public	SetMode
SetMode		proc
	assume	ds:Seg0, es:nothing
	pusha
	push	es
;
	call	AdjustInfoVars
	jc	short SetMode_exit
	mov	al,[byMode]
	call	OEMGetParmTable
	call	SetBIOSVariables
	call	ATCOff
	call	OEMPreSetRegs
	call	SetRegs
	call	FullCpuOn
	call	LoadPalette
	call	OEMSetRegs
	call	LoadSysFont
	test	[fEGAInfo],NO_REGEN_CLEAR	; Clear regen buffer?
	jnz	short SetMode_10		;  N: Skip it
	call	OEMClearScreen
SetMode_10:
	call	FullCpuOff
	call	ATCOn
;
SetMode_exit:
	pop	es
	popa
	pop	ds
	pop	si
	iret
SetMode		endp
;
;	AdjustInfoVars - Adjust mode control variables and call CGA/MDA BIOS if needed.
;
;	Entry:	AL = Mode to set
;		DS = Seg0
;	Exit:	Carry flag set as follows:
;		  CF = Exit set_mode
;		  NC = Continue to execute set_mode
;
;	Logic:
;
;	if (all_mode_all_monitors) {
;		if (mono_mode) {
;			crtc_index = 03B4h
;			equip_flag = mono
;			info_3 = 0Bh
;			info = VGA_MONOCHROME
;		} else {
;			crtc_index = 03D4h
;			equip_flag = color
;			info = not VGA_MONOCHROME
;			if (200 scan lines) {
;				info_3 = 08h
;			} else {
;				info_3 = 09h
;			}
;		}
;	} else {
;		if (mono_mode) {
;			if (equip_flag == mono) {
;				if (info == VGA_MONOCHROME) {
;					crtc_index = 03B4h
;					set VGA mode
;				} else {
;					set MDA mode (INT 42h)
;				}
;			} else {
;				if (info == VGA_MONOCHROME) {
;					set CGA mode (INT 42h)
;				} else {
;					mode_number = 0
;					crtc_index = 03D4h
;				}
;			}
;		} else {	/* color mode */
;			if (equip_flag == mono) {
;				if (info == VGA_MONOCHROME) {
;					mode_number = 7
;					crtc_index = 03B4h
;				} else {
;					set MDA mode (INT 42h)
;				}
;			} else {
;				if (info == VGA_MONOCHROME) {
;					set CGA mode (INT 42h)
;				} else {
;					crtc_index = 03D4h
;					set VGA mode
;				}
;			}
;		}
;
;
;	AX, BX, CX, DX are trashed
;
;	??? What about invalid modes with high order bit set?
;
	public	AdjustInfoVars
AdjustInfoVars	proc
	assume	ds:Seg0, es:nothing
	mov	ah,al
	and	ax,0807Fh
	call	IsValidMode
	jnz	AdjustInfoVars_error_exit
	mov	[byMode],al
;
	call	IsNullMode			; Is it the NULL mode?
	jz	AdjustInfoVars_error_exit	;  Y: Go exit, do nothing else
;
	mov	cl,[fEGAInfo]
	and	cl,not (NO_REGEN_CLEAR or VGA_INACTIVE)
	or	cl,ah
	mov	[fEGAInfo],cl
	mov	bl,[fEquip]
	mov	bh,bl
	and	bx,0CF30h
	mov	dx,CRTC_MINDEX
;
; If all modes, all monitors is set, then set vars and go set mode. Otherwise,
;  test to see if mode set needs to go through motherboard BIOS.
;
	test	[fVGAInfo],ALL_MODES
	jz	short AdjustInfoVars_20
	or	bh,030h
	mov	bl,00Bh
	or	[fEGAInfo],VGA_MONOCHROME
	call	IsModeMono
	jz	short AdjustInfoVars_10
	and	[fEGAInfo],not VGA_MONOCHROME
	mov	dl,low CRTC_CINDEX
	and	bh,0EFh
	mov	bl,008h
	test	[fVGAInfo],SCAN_200_ACTIVE
	jnz	short AdjustInfoVars_10
	mov	bl,009h
AdjustInfoVars_10:
	mov	[wCRTCAddr],dx
	mov	[fEquip],bh
	and	[fEGAInfo3],0F0h
	or	[fEGAInfo3],bl
	jmp	short AdjustInfoVars_exit
;
; At this point:
;	AL = mode_number
;	BL = equip_flag && 30h
;	BH = equip_flag && CFh
;	CL = info
;	DX = CRTC_INDEX_MONO
;
; If mode is color, equip is color, and we are color, then set color mode
;
AdjustInfoVars_20:
	call	IsModeMono			; Mode = mono?
	jz	short AdjustInfoVars_40		;  Y: Go test mono stuff
	cmp	bl,030h				; Equip = mono?
	je	short AdjustInfoVars_30		;  Y: Go test color vs. mono
	mov	dl,low CRTC_CINDEX		; Mode is color
	test	cl,VGA_MONOCHROME		; VGA is mono?
	jnz	short AdjustInfoVars_passthru	;  Y: Pass to CGA
	mov	[wCRTCAddr],dx			;  N: Set normal color mode
	jmp	short AdjustInfoVars_exit
;
; Mode is color, but equip is mono...
;
AdjustInfoVars_30:
	test	cl,VGA_MONOCHROME		; VGA is mono?
	jz	short AdjustInfoVars_passthru	;  N: Pass to MDA
	mov	al,07h				;  Y: Set mode 7
	mov	[byMode],al
	mov	[wCRTCAddr],dx
	jmp	short AdjustInfoVars_exit
;
; Mode is mono...
;
AdjustInfoVars_40:
	cmp	bl,30h				; Equip = mono?
	jne	short AdjustInfoVars_50		;  N: Go test mono vs. color
	test	cl,VGA_MONOCHROME		; VGA is mono?
	jz	short AdjustInfoVars_passthru	;  N: Pass to MDA
	mov	[wCRTCAddr],dx			;  Y: Set normal mono mode
	jmp	short AdjustInfoVars_exit
;
; Mode is mono, but equip is color...
;
AdjustInfoVars_50:
	test	cl,VGA_MONOCHROME		; VGA is mono?
	jnz	short AdjustInfoVars_passthru	;  Y: Pass to CGA
	sub	al,al				; Set mode 0
	mov	[byMode],al
	mov	[wCRTCAddr],CRTC_CINDEX
;
AdjustInfoVars_exit:
	clc
	ret
;
; Pass to motherboard for MDA or CGA handling. It is important to set the
;  character height properly before setting the mode or the cursor will
;  disappear on the secondary monitor.
;
AdjustInfoVars_passthru:
	cbw
	mov	bl,0Eh
	cmp	al,7
	je	short AdjustInfoVars_passthru_10
	mov	bl,08h
AdjustInfoVars_passthru_10:
	mov	[byRows],18h
	mov	byte ptr [wCharHeight],bl
	or	[fEGAInfo],VGA_INACTIVE		; Tell apps that VGA is inactive
	int	42h
;
AdjustInfoVars_error_exit:
	stc
	ret
AdjustInfoVars	endp
;
;	SetBIOSVariables - Setup BIOS variables
;
;	Entry:	DS = Seg0
;		ES:DI = Pointer to parameter entry
;	Exit:	None
;
;	AX, BX, CX are trashed
;	DI, ES are perserved
;
	public	tblCGA3D8
tblCGA3D8	label	byte
	db	02Ch, 028h, 02Dh, 029h, 02Ah, 02Eh, 01Eh, 029h
;
	public	SetBIOSVariables
SetBIOSVariables	proc
	assume	ds:Seg0, es:nothing
;
; First clear out "wRegenStart" and the eight cursor position words
;
	push	di
	push	es
	mov	cx,9
	mov	di,offset Seg0:wRegenStart
	call	GetESSeg0			; ES = Seg0
	assume	es:Seg0
	sub	ax,ax
	rep stosw
	pop	es
	pop	di
	assume	es:nothing
;
	mov	[byActivePage],al
	mov	bl,[byMode]
	mov	bh,ah
	cmp	bl,7
	ja	SetBIOSVariables_10
	mov	al,cs:[tblCGA3D8 + bx]
	mov	[byCGA3D8],al
	mov	[byCGA3D9],3Fh
	cmp	bl,6
	je	SetBIOSVariables_10
	mov	[byCGA3D9],30h
SetBIOSVariables_10:
	mov	al,es:[di].PARMTABLE.Columns
	mov	[wColumns],ax
	mov	al,es:[di].PARMTABLE.CharHeight
	mov	[wCharHeight],ax
	mov	al,es:[di].PARMTABLE.Rows
	mov	[byRows],al
	mov	ax,es:[di].PARMTABLE.RegenLength
	mov	[wRegenLength],ax
	mov	ax,word ptr es:[di].PARMTABLE.CRTC.CRTC.CursorScanStart
	xchg	al,ah
	mov	[wCursorType],ax
	ret
SetBIOSVariables	endp
;
;	SetRegs - Set VGA registers
;
;	Entry:	ES:DI = Pointer to standard parameter table entry
;		DS = Seg0
;	Exit:	None
;
;	Trashes AX, BX, CX, DX, SI
;
	public	SetRegs
SetRegs	proc
	assume	ds:Seg0, es:nothing
	push	di
	push	ds
	push	es
;
	mov	si,di
	push	es
	pop	ds				; DS:SI = pointer to parms
;
; Load sequencer and misc. output
;
	mov	dx,SEQ_INDEX
	mov	ax,0100h
	out	dx,ax				; Sync reset
	mov	dl,low MISC_OUTPUT
	mov	al,[si].PARMTABLE.Misc
	out	dx,al
	mov	dl,low SEQ_INDEX
	push	si
	mov	cx,4
	mov	bl,1
	lea	si,[si].PARMTABLE.SEQ
	call	LoadIndexRange
	pop	si
	mov	ax,0300h
	out	dx,ax				; End sync reset
;
; Load CRTC
;
	push	ds
	call	GetDSSeg0
	mov	dx,[wCRTCAddr]
	pop	ds
	mov	ax,02011h
	out	dx,ax				; Unlock CRTC
	sub	bl,bl
	mov	cx,size CRTC
	push	si
	lea	si,[si].PARMTABLE.CRTC
	call	LoadIndexRange
	pop	si
;
; Load GDC
;
	mov	dl,low GDC_INDEX
	sub	bl,bl
	mov	cx,9
	push	si
	lea	si,[si].PARMTABLE.GDC
	call	LoadIndexRange
	pop	si
;
; Clear VSYNC select
;
	mov	dl,0DAh
	sub	al,al
	out	dx,al			; Clear VSYNC select
;
	pop	es
	pop	ds
	pop	di
	ret
SetRegs	endp
;
;	LoadPalette - Load the system palette and the user defined palette if there is one
;
;	Entry:	DS = Seg0
;		ES:DI = Pointer to parameter entry
;	Exit:	None
;
	public	LoadPalette
LoadPalette	proc
	assume	ds:Seg0, es:nothing
	push	di
	push	es
;
	mov	dx,[wCRTCAddr]
	add	dl,6				; DX = Input Status 1
	push	dx				; Save Input Status 1 address
;
; Load default internal palette
;
	lea	si,es:[di].PARMTABLE.ATC
	call	LoadDefaultATCAndSavePtr
	cli
	in	al,dx
	mov	dl,low ATC_INDEX
	mov	al,014h
	out	dx,al
	sub	al,al
	out	dx,al
	sti
;
; Load default external palette
;
	test	[fVGAInfo],NO_DEFAULT_PALETTE	; Load default palette?
	jnz	short LoadPalette_10		;  N: Skip loading of RAMDAC
	call	LoadDefaultRAMDAC
LoadPalette_10:
	pop	dx				; Restore Input Status 1 address
	les	di,[lpSavePtr]
	les	di,es:[di].SAVEPTR.SecondarySavePtr
	mov	ax,es
	or	ax,di				; NULL pointer?
	jz	short LoadPalette_exit		;  Y: Skip user palette load
	les	di,es:[di].SAVEPTR2.PaletteProfilePtr
	mov	ax,es
	or	ax,di				; NULL pointer?
	jz	short LoadPalette_exit		;  Y: Skip user palette load
;
; At this point, a palette profile table exists, so load the palette if the
;  mode matches a mode number in the table.
;
;	ES:DI = Pointer to Palette Profile Table
;	DX = Input Status 1 Register
;
	lea	si,es:[di].PALETTEPROFILE.ModeList
	call	IsModeInTable
	jnz	short LoadPalette_exit
	push	ds
	mov	cx,es:[di].PALETTEPROFILE.ATCCount		; Get number of regs to set
	mov	ah,byte ptr es:[di].PALETTEPROFILE.ATCIndex	; Get starting index
	lds	si,es:[di].PALETTEPROFILE.ATCDataPtr		; Get address of buffer
	assume	ds:nothing
	call	SetPaletteAndOverscanFromBuffer
	mov	ax,1012h					; Set RAMDAC block function
	mov	cx,es:[di].PALETTEPROFILE.DACCount		; Get number of regs to set
	mov	bx,es:[di].PALETTEPROFILE.DACIndex		; Get starting index
	les	dx,es:[di].PALETTEPROFILE.DACDataPtr		; Get address of buffer
	int	10h
	pop	ds
LoadPalette_exit:
;
	pop	es
	pop	di
	ret
LoadPalette	endp
;
;	LoadDefaultRAMDAC - Load the default palette
;
;	Entry:	DS = Seg0
;	Exit:	None
;
;	AX, BX, CX, DX, DI, SI are trashed
;
	public	tblDefaultPalette
tblDefaultPalette	label	word
	dw	LoadColorTextDACTable
	dw	LoadMonochromeDAC
	dw	LoadCGADACTable
	dw	LoadMonochromeDAC
	dw	LoadColorDACTable
	dw	LoadColorDACTable
	dw	LoadColor256DAC
IF EXTENDED_BIOS
	dw	OEMLoad16BPPDAC
	dw	OEMLoad24BPPDAC
ELSE
	dw	LoadDACDoNothing
	dw	LoadDACDoNothing
ENDIF
	dw	LoadDACDoNothing
LENGTH_DEFAULT_PALETTE_TABLE	equ	($ - tblDefaultPalette) / 2
;
	public	LoadDefaultRAMDAC
LoadDefaultRAMDAC	proc
	assume	ds:Seg0, es:nothing
	mov	dx,DAC_MASK
	mov	al,0FFh
	out	dx,al				; Set DAC mask to 0FFh
	mov	al,[byMode]
;
; BEGIN HACK
;
; 200 scan line planar modes use the CGA RAMDAC settings instead of
; the planar mode settings. Since this only applies to modes 0Dh and 0Eh
; and never will apply to extended modes, we do an exact compare here.
;
	mov	bx,2*2				; Point to CGA entry in table
	cmp	al,0Dh
	je	short LoadDefaultRAMDAC_20
	cmp	al,0Eh
	je	short LoadDefaultRAMDAC_20
;
; END HACK
;
	call	GetModeDescPtr			; Get Mode Description pointer
	mov	bl,es:[bx].MODEDESC.mdType
	sub	bh,bh
	cmp	bl,LENGTH_DEFAULT_PALETTE_TABLE
	jb	short LoadDefaultRAMDAC_10
	mov	bl,LENGTH_DEFAULT_PALETTE_TABLE - 1
LoadDefaultRAMDAC_10:
	shl	bx,1
LoadDefaultRAMDAC_20:
	push	es
	assume	es:_TEXT
	push	cs
	pop	es
	call	cs:[tblDefaultPalette + bx]
	assume	es:nothing
	pop	es
LoadDefaultRAMDAC_exit:
	ret
LoadDefaultRAMDAC	endp
;
;	LoadColorTextDACTable - Load the DAC for the color text modes
;
;	Entry:	DS = Seg0
;		ES = Code segment
;	Exit:	None
;
;	WARNING! This routine has no "RET" statement and must be followed
;	by "LoadColorDACTable"!
;
;	AX, BX, CX, DX, SI are trashed.
;
	public	tblDACValues
tblDACValues	db	000h, 015h, 02Ah, 03Fh
;
	public	LoadColorTextDACTable
LoadColorTextDACTable	proc
	assume	ds:Seg0, es:_TEXT
	test	[fVGAInfo],SCAN_200_ACTIVE	; 200 scan lines?
	jnz	short LoadCGADACTable		;  Yes: Go set CGA-type palette
LoadColorTextDACTable	endp
;
;	LoadColorDACTable - Load the DAC for the 16-color modes
;
;	Entry:	DS = Seg0
;		ES = code
;	Exit:	None
;
;	WARNING! This routine must follow "LoadColorTextDACTable"!
;	WARNING! This routine has no "RET" statement and must be followed
;	by "LoadCompressedDACTable"!
;
;	AX, BX, CX, DX, DI, SI are trashed.
;
	public	LoadColorDACTable
LoadColorDACTable	proc
	assume	ds:Seg0, es:_TEXT
	mov	si,offset tbl16ColorDAC
	mov	di,LENGTH_16COLOR_DAC_TABLE
	sub	al,al
LoadColorDACTable	endp
;
;	LoadCompressedDACTable - Load a compressed DAC table
;
;	Entry:	DS = Seg0
;		ES:SI = Pointer to compressed DAC table
;		DI = Length of the table
;		AL = Starting DAC index
;	Exit:	None
;
;	Note:	The compression scheme used is very simple:
;			7  6  5  4  3  2  1  0
;			x  x  B1 B0 G1 G0 R1 R0
;		where the two bits for each color are used to
;		look up a table of values: 00h, 15h, 2Ah, 3Fh
;
;	WARNING! This routine must follow "LoadColorDACTable"!
;
;	AX, BX, CX, DX, DI, SI are trashed.
;
	public	LoadCompressedDACTable
LoadCompressedDACTable	proc
IF (VARIATION_005 eq 1)
	mov	dx,DAC_WINDEX
	mov	bl,al
	call	OEMWriteRAMDACIdx
	mov	bx,offset tblDACValues
ELSE
	mov	bx,offset tblDACValues
	mov	dx,DAC_WINDEX
	out	dx,al
ENDIF
;
; Loop through each entry in the table. First expand the bits into
; their appropriate bytes, then convert to gray scale if necessary
; and then write the values out to the RAMDAC. Note that speed isn't
; the important thing here, size is.
;
; The register used by the summing routine are:
;	DH = Red
;	CH = Green
;	CL = Blue
; Therefore, expand the values into those registers.
;
LoadCompressedDACTable_10:
	mov	ah,es:[si]			; Get compressed data byte
	inc	si				; Next location in table
;
	mov	al,ah
	and	al,003h
	xlat	cs:[bx]
	mov	dh,al				; DH = Red
;
	mov	al,ah
	and	al,00Ch
	shr	al,2
	xlat	cs:[bx]
	mov	ch,al				; CH = Green
;
	shr	ax,12
	xlat	cs:[bx]
	mov	cl,al				; CL = Blue
;
	call	SetDACRegisterAutoIndex		; Write the value
;
	dec	di
	jnz	short LoadCompressedDACTable_10
	ret
LoadCompressedDACTable	endp
;
;	LoadCGADACTable - Load the DAC for the CGA-type modes
;
;	Entry:	DS = Seg0
;		ES = code
;	Exit:	None
;
;	AX, BX, CX, DX, DI, SI are trashed.
;
	public	LoadCGADACTable
LoadCGADACTable	proc
	assume	ds:Seg0, es:_TEXT
	mov	si,offset tblCGADAC
	mov	di,LENGTH_CGA_DAC_TABLE
	sub	al,al
	push	si
	push	di
	call	LoadCompressedDACTable		; Load first half
	pop	di
	pop	si
	mov	al,020h
	jmp	LoadCompressedDACTable		; Load second half
LoadCGADACTable	endp
;
;	LoadMonochromeDAC - Load the DAC for the monochrome mode
;
;	Entry:	DS = Seg0
;		ES = code
;	Exit:	None
;
;	AX, BX, CX, DX, SI are trashed.
;
	public	LoadMonochromeDAC
LoadMonochromeDAC	proc
	assume	ds:Seg0, es:_TEXT
	mov	si,offset tblMonochromeDAC
	mov	dx,DAC_WINDEX
	sub	al,al
IF (VARIATION_005 eq 1)
	mov	bl,al
	call	OEMWriteRAMDACIdx
ELSE
	out	dx,al
ENDIF
	inc	dx
	mov	cx,2
LoadMonochromeDAC_10:
	push	cx
	push	si
	mov	cx,4
LoadMonochromeDAC_20:
	lods	byte ptr es:[si]
IF (VARIATION_005 eq 1)
	push	cx
	push	dx
	mov	dh,al
	mov	ch,al
	mov	cl,al
	mov	bx,8
LoadMonochromeDAC_30:
	call	OEMWriteRAMDACData
	dec	bx
	jnz	short LoadMonochromeDAC_30
	pop	dx
	pop	cx
ELSE
	mov	bx,8*3
LoadMonochromeDAC_30:
	out	dx,al
	dec	bx
	jnz	short LoadMonochromeDAC_30
ENDIF
	loop	LoadMonochromeDAC_20
	pop	si
	pop	cx
	loop	LoadMonochromeDAC_10
	ret
LoadMonochromeDAC	endp
;
;	LoadColor256DAC - Load the DAC for the 256-color modes
;
;	Entry:	DS = Seg0
;		ES = code
;	Exit:	None
;
;	AX, BX, CX, DX, SI are trashed.
;
	public	LoadColor256DAC
LoadColor256DAC	proc
	assume	ds:Seg0, es:_TEXT
	mov	si,offset tbl256ColorDAC
	mov	cx,LENGTH_256COLOR_DAC_TABLE
	sub	bl,bl
	jmp	SetRAMDACRangeWithSumming
LoadColor256DAC	endp
;
;	LoadSysFont - Load the system font
;
;	Entry:	DS = Seg0
;	Exit:	None
;
	public	LoadSysFont
LoadSysFont	proc
	assume	ds:Seg0, es:nothing
	push	bp
	mov	al,[byMode]
	call	IsTextMode			; Text mode?
	jnz	LoadSysFont_100			;  N: Go load graphics font
;
; Load text font here
;
	call	PreFontLoad			; Prepare for loading
;
; First, determine if a different primary font is to be loaded. This is
; pointed at by the SAVEPTR data structure.
;
	les	di,[lpSavePtr]
	les	di,es:[di].SAVEPTR.AuxTextFontPtr
	mov	dx,es
	or	dx,di				; NULL Pointer?
	jz	short LoadSysFont_10		;  Y: Go load standard font
;
; Determine if this mode is in the list of modes that the font
; is "active" for.
;
	lea	si,es:[di].AUXTEXTFONT.ModeList
	call	IsModeInTable			; Is the mode in the list?
	jnz	short LoadSysFont_10		;  N: Go load standard font
;
; Load the alternate primary font
;
	mov	ax,01110h
	mov	bh,es:[di].AUXTEXTFONT.CharHeight
	mov	bl,es:[di].AUXTEXTFONT.Block
	mov	cx,es:[di].AUXTEXTFONT.Count
	mov	dx,es:[di].AUXTEXTFONT.Start
	les	bp,es:[di].AUXTEXTFONT.Font
	int	10h
	jmp	short LoadSysFont_30
;
; Load the standard system font
;
LoadSysFont_10:
	call	GetFontPtr			; Returns ES:DI, BH
	sub	dx,dx				; Start at character 0
	mov	bl,dl				; Start in block 0
	mov	cx,256				; Load everything
	call	LoadFont
IF EXTENDED_BIOS
	call	OEMFixupFont			; Possibly clear more scans
ENDIF
IF (VARIATION_003 eq 0)
	call	Is9DotMode			; Is mode a 9-dot mode?
	jnz	short LoadSysFont_30		;  N: Exit load font
	call	GetFixupPtr
	sub	bl,bl
	call	LoadFixup
ENDIF
;
; Common text mode font load thread resumes here.
; Determine if a secondary font is to be loaded. This is pointed at
; by the SAVEPTR2 data structure and is further dependent on whether
; the character height in the data structure matches the current
; character height.
;
LoadSysFont_30:
	call	PostFontLoad
	les	di,[lpSavePtr]
	les	di,es:[di].SAVEPTR.SecondarySavePtr
	les	di,es:[di].SAVEPTR2.Aux2TextFontPtr
	mov	dx,es
	or	dx,di				; NULL Pointer?
	jz	short LoadSysFont_exit		;  Y: No more font loading
	mov	al,es:[di].AUXTEXTFONT2.CharHeight
	cmp	byte ptr [wCharHeight],al	; Height matches current?
	jne	short LoadSysFont_exit		;  N: Go exit
;
; Determine if this mode is in the list of modes that the font
; is "active" for.
;
	lea	si,es:[di].AUXTEXTFONT2.ModeList
	call	IsModeInTable			; Is the mode in the list?
	jnz	short LoadSysFont_exit		;  N: No more font loading
;
; Load the secondary font.
;
	mov	ax,01100h
	mov	bl,es:[di].AUXTEXTFONT2.Block
	mov	bh,es:[di].AUXTEXTFONT2.CharHeight
	mov	cx,256
	sub	dx,dx
	les	bp,es:[di].AUXTEXTFONT2.Font
	int	10h
	jmp	short LoadSysFont_exit
;
; Load a graphics font here (actually, just set up the proper pointer)
; First, determine if a different primary font is to be loaded. This is
; pointed at by the SAVEPTR data structure.
;
LoadSysFont_100:
	les	di,[lpSavePtr]
	les	di,es:[di].SAVEPTR.AuxGraphicsFontPtr
	mov	dx,es
	or	dx,di				; NULL Pointer?
	jz	short LoadSysFont_110		;  Y: Go point to standard font
;
; Determine if this mode is in the list of modes that the font
; is "active" for.
;
	lea	si,es:[di].AUXGRAPHICSFONT.ModeList
	call	IsModeInTable			; Is the mode in the list?
	jnz	short LoadSysFont_110		;  N: Go load standard font
;
; Load the alternate primary font
;
	mov	ax,01121h
	sub	bl,bl
	mov	cx,es:[di].AUXGRAPHICSFONT.CharHeight
	mov	dl,es:[di].AUXGRAPHICSFONT.Rows
	les	bp,es:[di].AUXGRAPHICSFONT.Font
	int	10h
	jmp	short LoadSysFont_exit
;
; Load the standard system font
;
LoadSysFont_110:
	call	GetFontPtr
	mov	[offINT43],di
	mov	[segINT43],es
;
; Common exit
;
LoadSysFont_exit:
	pop	bp
	ret
LoadSysFont	endp
;
;	LoadDefaultATCAndSavePtr - Load a the default ATC registers and store them in the saveptr area if necessary
;
;	Entry:	ES:SI = Pointer to palette data
;		DX = I/O Address of Input Status Register
;		DS = Seg0
;	Exit:	None
;
;	AX, CX, SI are trashed
;	DX is preserved
;
	public	LoadDefaultATCAndSavePtr
LoadDefaultATCAndSavePtr	proc
	assume	ds:Seg0, es:nothing
	push	dx
	push	di
	push	ds
	push	es
;
	push	es
	les	di,[lpSavePtr]
	les	di,es:[di].SAVEPTR.SaveAreaPtr
	assume	ds:nothing, es:nothing
	pop	ds
	mov	ax,es
	or	ax,di				; NULL Pointer? Y: Skip save area
	jz	short LoadDefaultATCAndSavePtr_10
	mov	cx,16
	push	si
	rep movsb				; Save the 16 palette registers
	inc	si				; Skip the ATC Mode register
	movsb					; Save the overscan register
	pop	si
LoadDefaultATCAndSavePtr_10:
;
	sub	bl,bl				; Initial index
	mov	cx,20				; Number of ATC registers
	cli
	in	al,dx
	mov	dl,ATC_INDEX and 0ffh
LoadDefaultATCAndSavePtr_20:
	mov	al,bl
	out	dx,al
	lodsb
	out	dx,al
	inc	bx
	loop	LoadDefaultATCAndSavePtr_20
	sti
;
	pop	es
	pop	ds
	pop	di
	pop	dx
	ret
LoadDefaultATCAndSavePtr	endp
;
;	IsModeInTable - Check for the current mode number in the mode number stream.
;	The table ends with an 0FFh.
;
;	Entry:	DS = Seg0
;		ES:SI = Pointer to mode table
;	Exit:	Zero flag set as follows:
;		  ZF = Mode found
;		  NZ = Mode not found
;
;	AX, SI are trashed
;
	public	IsModeInTable, LoadDACDoNothing
IsModeInTable	proc
	assume	ds:Seg0, es:nothing
	mov	ah,[byMode]
IsModeInTable_10:
	lods	byte ptr es:[si]
	cmp	ah,al
	je	IsModeInTable_20
	cmp	al,0FFh				; End of table?
	jne	IsModeInTable_10		;  N: Go back and check
	dec	ax				; Clear Zero Flag to NZ
IsModeInTable_20:
LoadDACDoNothing	label	near		; Just a convenient ret
	ret
IsModeInTable	endp
;
;	Function 0Fh:
;	GetMode - Get video information
;
;	Entry:	SI, DS pushed on stack
;	Exit:	AL = current mode
;		AH = number of columns
;		BH = active page
;
	public	GetMode
GetMode		proc
	assume	ds:Seg0, es:nothing
	mov	al,[fEGAInfo]
	and	al,080h
	or	al,[byMode]
	mov	ah,byte ptr [wColumns]
	mov	bh,[byActivePage]
	pop	ds
	pop	si
	iret
GetMode		endp
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


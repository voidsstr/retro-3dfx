	page	57, 132
	title	Alternate Select Functions
	subttl	ALT.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	ALT.ASM - Alternate Select Functions
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	AlternateSelect		Alternate select routines
;	ReturnVGAInformation	Return extended information about EGA/VGA
;	SelectAltPrtsc		Select the alternate print screen routine
;	SelectScanLines		Select 200, 350, or 400 scan lines in text modes
;	DisablePaletteLoading	Disable palette (and DAC) writes (no BIOS call can affect the palette)
;	DisableVideoSubsystem	Enable/Disable video subsystem via write to disable bit (3C3h or 46E8h)
;	DisableSumming		Enable/Disable summing to gray scale (30% red, 59% green, & ll% blue)
;	DisableCursorEmulation	Enable/Disable cursor emulation
;	DisplaySwitch		Display switch subfunctions
;	DisableVideoDisplay	Enable/Disable display via sequencer index 1, bit 5
;	PrintScreenHandler	Print screen routine substituted at INT 5
;	PrintCRLF		Send a carriage return/linefeed pair to the printer
;	PrintCharacter		Send a character to the printer
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/ALT.ASM $
;	$Log: /devel/h5/BIOS/SRC/ALT.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;

	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: ALT.ASM
;
.MODEL TINY
.CODE
.386
;
	extrn	FullCpuOn:near
	extrn	FullCpuOff:near
	extrn	OEMSleepVGA:near
	extrn	OEMWakeUpVGA:near
	extrn	LocalSetCursorPosition:near
	extrn	GetDSSeg0:near
IF EXTENDED_BIOS
	extrn	OEMExtensionsAlt:near
ENDIF
;
;	Function 12h:
;	AlternateSelect - Alternate select routines
;
;	Entry:	DS, SI are pushed on the stack
;		DS = Seg0
;		BL = Subfunction
;	Exit:	Dependent on subfunction
;
ALTSELFUNCTION	struc
  asfNumber		db	?
  asfFunction		dw	?
ALTSELFUNCTION	ends
;
	public	tblAltSelFunctions, tblAltSelFunctionsEnd
tblAltSelFunctions	label	byte
	ALTSELFUNCTION	<010h, ReturnVGAInformation>
	ALTSELFUNCTION	<020h, SelectAltPrtsc>
	ALTSELFUNCTION	<030h, SelectScanLines>
	ALTSELFUNCTION	<031h, DisablePaletteLoading>
	ALTSELFUNCTION	<032h, DisableVideoSubsystem>
	ALTSELFUNCTION	<033h, DisableSumming>
	ALTSELFUNCTION	<034h, DisableCursorEmulation>
	ALTSELFUNCTION	<035h, DisplaySwitch>
	ALTSELFUNCTION	<036h, DisableVideoDisplay>
tblAltSelFunctionsEnd	label	byte
;
	public	AlternateSelect
AlternateSelect	proc	near
	mov	si,offset tblAltSelFunctions	; Point to function table
AltSelHandler_10:
	cmp	cs:[si].ALTSELFUNCTION.asfNumber,bl	; Sub-function code matches?
	je	AltSelHandler_20			;  Y: Go handle it
	add	si,3					;  N: Point to next entry
	cmp	si,offset tblAltSelFunctionsEnd		; End of table?
	jb	AltSelHandler_10			;  N: Go check next sub-function
IF EXTENDED_BIOS
	call	OEMExtensionsAlt			; If not in list, let OEM handle it
ENDIF
	jmp	short AltSelHandler_exit
;
AltSelHandler_20:
	call	cs:[si].ALTSELFUNCTION.asfFunction	; Dispatch to sub-function
;
AltSelHandler_exit:
	pop	ds
	pop	si
	iret
AlternateSelect	endp
;
;	ReturnVGAInformation - Return extended information about EGA/VGA
;
;	Entry:	DS = Seg0
;	Exit:	BL = 3 (formerly memory size)
;		BH = Color/mono status
;			0 = Color mode is current
;			1 = Mono mode is current
;		CL = Switch settings
;		CH = Feature bits
;
	public	ReturnVGAInformation
ReturnVGAInformation	proc	near
	assume	ds:Seg0, es:nothing
	mov	bx,00103h				; Default to mono mode
	cmp	byte ptr [wCRTCAddr],low CRTC_CINDEX	; Color mode?
	jne	ReturnVGAInformation_10			;  N: Got data
	sub	bh,bh					;  Y: Flag it as color
ReturnVGAInformation_10:
	mov	cl,[fEGAInfo3]
	mov	ch,cl
	and	cx,0F00Fh
	shr	ch,4
	mov	al,bl			; For compatibility with Model 70
	ret
ReturnVGAInformation	endp
;
;	SelectAltPrtsc - Select the alternate print screen routine
;
;	Entry:	DS = Seg0
;	Exit:	None
;
	public	SelectAltPrtsc
SelectAltPrtsc	proc	near
	assume	ds:Seg0, es:nothing
	cli
	mov	[offINT05],offset PrintScreenHandler
	mov	[segINT05],cs
	sti
	ret
SelectAltPrtsc	endp
;
;	SelectScanLines - Select 200, 350, or 400 scan lines in text modes
;
;	Entry:	DS = Seg0
;		AL = Number of scan lines
;			0 = 200 lines
;			1 = 350 lines
;			2 = 400 lines
;	Exit:	AL = 12h
;
;	SI is trashed.
;
; Scan line table:
;	High order byte = Value for "info_3"
;	Low order byte = Value for "vga_info"
;
	public	tblSelectScanLines
tblSelectScanLines	label	word
	dw	SCAN_200_ACTIVE + 0800h		; 200 scans
	dw	0000h + 0900h			; 350 scans
	dw	SCAN_400_ACTIVE + 0900h		; 400 scans
;
	public	SelectScanLines
SelectScanLines	proc	near
	assume	ds:Seg0, es:nothing
	cmp	al,2				; Out of range?
	ja	SelectScanLines_exit		;  Y: Go exit
	cbw
	mov	si,ax
	shl	si,1
	mov	ax,cs:[tblSelectScanLines + si]
	and	[fVGAInfo],not (SCAN_200_ACTIVE + SCAN_400_ACTIVE)
	or	[fVGAInfo],al
	and	[fEGAInfo3],0F0h
	or	[fEGAInfo3],ah
	mov	ax,01212h			; For compatibility with Model 70
SelectScanLines_exit:
	ret
SelectScanLines	endp
;
;	DisablePaletteLoading - Disable palette (and DAC) writes (no BIOS call can affect the palette)
;
;	Entry:	DS = Seg0
;		AL = Palette loading state
;			0 = Enable palette loading
;			1 = Disable palette loading
;	Exit:	AL = 12h
;
	public	DisablePaletteLoading
DisablePaletteLoading	proc	near
	assume	ds:Seg0, es:nothing
	cmp	al,1				; Out of range?
	ja	DisablePaletteLoading_exit	;  Y: Go exit
.errnz	NO_DEFAULT_PALETTE - 00001000b
	shl	al,3
	and	[fVGAInfo],not NO_DEFAULT_PALETTE
	or	[fVGAInfo],al
	mov	al,12h
DisablePaletteLoading_exit:
	ret
DisablePaletteLoading	endp
;
;	DisableVideoSubsystem - Enable/Disable video subsystem via write to disable bit (3C3h or 46E8h)
;
;	Entry:	DS = Seg0
;		AL = Video subsystem state
;			0 = Enable video subsystem
;			1 = Disable video subsystem
;	Exit:	AL = 12h
;
	public	DisableVideoSubsystem
DisableVideoSubsystem	proc	near
	assume	ds:Seg0, es:nothing
	cmp	al,1				; Out of range?
	ja	DisableVideoSubsystem_exit	;  Y: Go exit
	je	DisableVideoSubsystem_10	; Disable? Y: Go handle
	call	OEMWakeUpVGA			;  N: Wake up VGA
	jmp	short DisableVideoSubsystem_20
;
DisableVideoSubsystem_10:
	call	OEMSleepVGA			; Sleep the VGA
;
DisableVideoSubsystem_20:
	mov	al,012h
DisableVideoSubsystem_exit:
	ret
DisableVideoSubsystem	endp
;
;	DisableSumming - Enable/Disable summing to gray scale (30% red, 59% green, & ll% blue)
;
;	Entry:	DS = Seg0
;		AL = Palette summing state
;			0 = Enable summing
;			1 = Disable summing
;	Exit:	AL = 12h
;
	public	DisableSumming
DisableSumming	proc	near
	assume	ds:Seg0, es:nothing
	cmp	al,1				; Out of range?
	ja	DisableSumming_exit		;  Y: Go exit
.errnz	PALETTE_SUMMING - 00000010b
	shl	al,1
	xor	al,2
	and	[fVGAInfo],not PALETTE_SUMMING
	or	[fVGAInfo],al
	mov	al,012h
DisableSumming_exit:
	ret
DisableSumming	endp
;
;	DisableCursorEmulation - Enable/Disable cursor emulation
;
;	Entry:	DS = Seg0
;		AL = Cursor emulation state
;			0 = Enable cursor emulation
;			1 = Disable cursor emulation
;	Exit:	AL = 12h
;
	public	DisableCursorEmulation
DisableCursorEmulation	proc	near
	assume	ds:Seg0, es:nothing
	cmp	al,1				; Out of range?
	ja	DisableCursorEmulation_exit	;  Y: Go exit
	and	[fEGAInfo],not NO_CURSOR_EMULATE
.errnz	NO_CURSOR_EMULATE - 00000001b
	or	[fEGAInfo],al
	mov	al,012h
DisableCursorEmulation_exit:
	ret
DisableCursorEmulation	endp
;
;	DisplaySwitch - Display switch subfunctions
;
;	Entry:	DS = Seg0
;	Exit:	None
;
	public	DisplaySwitch
DisplaySwitch	proc	near
	assume	ds:Seg0, es:nothing
; Does anybody really need compatibility with the Model 50 only?
	ret
DisplaySwitch	endp
;
;	DisableVideoDisplay - Enable/Disable display via sequencer index 1, bit 5
;
;	Entry:	DS = Seg0
;		AL = Video enable state
;			0 = Enable video
;			1 = Disable video
;	Exit:	AL = 12h
;
	public	DisableVideoDisplay
DisableVideoDisplay	proc	near
	assume	ds:Seg0, es:nothing
	push	dx
	cmp	al,1				; Out of range?
	ja	short DisableVideoDisplay_exit	;  Y: Go exit
	je	short DisableVideoDisplay_10
	call	FullCpuOff			; Turn display on
	jmp	short DisableVideoDisplay_20
DisableVideoDisplay_10:
	call	FullCpuOn			; Turn display off
DisableVideoDisplay_20:
	mov	al,012h
DisableVideoDisplay_exit:
	pop	dx
	ret
DisableVideoDisplay	endp
;
;	PrintScreenHandler - Print screen routine substituted at INT 5
;
;	Entry:	Unknown
;	Exit:	All registers untouched
;
	public	PrintScreenHandler
PrintScreenHandler	proc	near
	assume	ds:nothing, es:nothing
	sti
	pusha
	push	ds
;
	call	GetDSSeg0			; Get BIOS data segment
	assume	ds:Seg0
;
; Save current cursor position for later restoration
;
	mov	bl,[byActivePage]
	sub	bh,bh
	shl	bx,1
	push	[wCursorPosition + bx]		; Save cursor position
;
	mov	bh,[byActivePage]
	sub	dx,dx				; Start at 0,0
;
; Loop through each row on the screen, a character at a time
;
PrintScreenHandler_10:
	call	LocalSetCursorPosition		; Move the cursor one column
	mov	ah,008h				; Read character
	int	10h
	call	PrintCharacter			; Print the character
	inc	dx				; Next column
	cmp	dl,byte ptr [wColumns]		; Last column on screen?
	jb	PrintScreenHandler_10		;  N: Go do next
	call	PrintCRLF			; Send CR/LF to printer
	sub	dl,dl				; Start at column 0 on
	inc	dh				;  the next row
	cmp	dh,[byRows]			; Last row?
	jbe	PrintScreenHandler_10		;  N: Go do next row
;
; Restore to the original cursor position
;
	pop	dx				; Get original cursor position...
	call	LocalSetCursorPosition		;  ...and restore it
;
	pop	ds
	popa
	iret
PrintScreenHandler	endp
;
;	PrintCRLF - Send a carriage return/linefeed pair to the printer
;
;	Entry:	None
;	Exit:	None
;
	public	PrintCRLF
PrintCRLF	proc	near
	assume	ds:nothing, es:nothing
	mov	al,0Dh
	call	PrintCharacter
	mov	al,0Ah
	call	PrintCharacter
	ret
PrintCRLF	endp
;
;	PrintCharacter - Send a character to the printer
;
;	Entry:	AL = Character to print
;	Exit:	None
;
	public	PrintCharacter
PrintCharacter	proc	near
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
;
	sub	dx,dx			; DX = 0
	mov	ah,dl			; AH = 0
	int	17h			; AL = Character to print
;
	pop	dx
	pop	ax
	ret
PrintCharacter	endp
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


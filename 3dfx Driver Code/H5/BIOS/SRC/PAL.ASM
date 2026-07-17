	page	57, 132
	title	Palette Functions
	subttl	PAL.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	PAL.ASM - Palette functions
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	SetCGAPalette			CGA palette function
;	PaletteHandler			EGA/VGA palette function handler
;	SetOnePaletteRegister		Set one palette register
;	SetOverscanRegister		Set the overscan register
;	SetAllPaletteRegisters		Set all palette registers
;	ToggleIntensityBlinkBit		Set the intensity or blinking bit in the ATC mode register
;	GetOnePaletteRegister		Read one ATC register
;	GetOverscanRegister		Read overscan register
;	GetAllPaletteRegisters		Read all the ATC registers
;	SetOneDACRegister		Set one DAC register
;	SetDACRegisterAutoIndex		Physically write one DAC register
;	SetBlockOfDACRegisters		Set block of DAC registers
;	SetRAMDACRangeWithSumming	Set a block of RAMDAC registers with gray scale summing if necessary
;	SetColorPage			Set color page (ATC 14h)
;	GetDACRegister			Read one DAC register
;	GetBlockOfDACRegisters		Read a block of DAC registers
;	SetDACMask			Set the DAC mask
;	GetDACMask			Get the DAC mask
;	GetColorPage			Return the value of the color page
;	SumDACToGrayScale		Sum a range of DAC registers to gray scale
;	SumSingleValue			Sum one value to gray
;	SetPaletteAndOverscanFromBuffer	Set the palette registers and overscan from a buffer
;	FillDAC				Fill the RAMDAC with a given value
;	LoadATCRange			Load a range of ATC registers
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/PAL.ASM $
;	$Log: /devel/h5/BIOS/SRC/PAL.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: PAL.ASM
;
.MODEL TINY
.CODE
.386
;
	extrn	GetIndexRegister:near
	extrn	WaitVerticalRetrace:near
	extrn	WaitBlank:near
	extrn	IsTextMode:near
	extrn	IsModeMono:near
	extrn	ResetATCFlipFlop:near
IF (VARIATION_005 eq 1)
	extrn	OEMWriteRAMDACIdx:near
	extrn	OEMWriteRAMDACData:near
	extrn	OEMReadRAMDAC:near
ENDIF
;
;	Function 0Bh:
;	SetCGAPalette - CGA palette function
;
;	Entry:	DS, SI are pushed on the stack
;		DS = Seg0
;		BH = Color ID to set (0 = Set background, 1 = Set palette)
;		BL = Color value
;	Exit:	None
;
	public	tblICGAPalette0, tblICGAPalette1, tblNCGAPalette0, tblNCGAPalette1
tblICGAPalette0	db	012h, 014h, 016h
tblICGAPalette1	db	013h, 015h, 017h
tblNCGAPalette0	db	002h, 004h, 006h
tblNCGAPalette1	db	003h, 005h, 007h
;
	public	tblPaletteAddr
tblPaletteAddr	label	word
	dw	tblNCGAPalette0
	dw	tblICGAPalette0
	dw	tblNCGAPalette1
	dw	tblICGAPalette1
;
	public	SetCGAPalette
SetCGAPalette	proc
	assume	ds:Seg0, es:nothing
	pusha
;
	mov	al,[byMode]
	call	IsModeMono		; Is this a monochrome mode?
	jz	SetCGAPalette_exit	;  Y: Don't set any color palette
;
	cmp	bh,1			; Set color palette?
	je	SetCGAPalette_10	;  Y: Go set palette
	ja	SetCGAPalette_exit	; Out of range? Y: Go exit
;
; Set background intensity flag and background color flag in
; CGA port 3D9 emulation byte.
;
	mov	al,bl			; Color to set
	mov	ah,[byCGA3D9]		; Original setting of 3D9h
	and	ax,0E010h		; Clear intensity bit / Isolate intensity bit
	or	al,ah			; 3D9 has background intensity set
	and	bl,00Fh			; Isolate background color
	or	al,bl			; 3D9 has background color
	jmp	short SetCGAPalette_20
;
SetCGAPalette_10:
	mov	al,bl			; Palette to set
	shl	al,5			; Assume the position
	mov	ah,[byCGA3D9]		; Original setting of 3D9h
	and	ax,0DF20h		; Clear palette flag / Isolate palette flag
	or	al,ah			; 3D9 has palette ID
;
; Translate CGA 3D9h port to the VGA internal palette. At this point:
;	AL = CGA 3D9h emulated port value
;
SetCGAPalette_20:
	mov	[byCGA3D9],al		; Set it
	call	WaitVerticalRetrace	; DX = 3xAh; Leaves ATC in index state
;
	push	dx			; Save Input Status 1 address
	mov	bh,al
	and	bx,0F00h		; Isolate background/overscan color
	cmp	bh,08h			; Intense colors?
	jb	SetCGAPalette_30	;  N: Use as is
	add	bh,08h			;  Y: Adjust for palette position
SetCGAPalette_30:
	mov	dl,low ATC_INDEX	; DX = 3C0h
	mov	al,[byMode]
	call	IsTextMode		; Is this a text mode?
					;   NOTE: ZF is tested much later
					;	  and must be preserved.
					;	  Instructions marked with
					;	  an '*' don't affect the flags.
	cli				; *
	jz	SetCGAPalette_40	; *  Y: Don't set background
	mov	al,0			; *  N: Set background color
	out	dx,al			; * Set index 0, ATC now in data state
	mov	al,bh			; *
	out	dx,al			; * Set color data, ATC back in index state
SetCGAPalette_40:
	mov	al,11h			; * Set overscan index, ATC now in data state
	out	dx,al			; *
	mov	al,bh			; * Set overscan color
	out	dx,al			; *
	sti				; *
	pop	dx			; * Restore Input Status 1 address
	jz	SetCGAPalette_50	; If text mode, exit!
;
	mov	bl,[byCGA3D9]
	push	cs
	pop	ds			; mov ds,cs
	assume	ds:nothing
	and	bx,0030h		; Isolate intensity and color set value
	shr	bx,3			; Shift into position and multiply by 2
	mov	si,cs:[bx + offset tblPaletteAddr]
	mov	bl,1			; Start at ATC index 1
	mov	cx,3			; Load 4 ATC registers
	call	LoadATCRange		; ATC is left in index state
;
SetCGAPalette_50:
	mov	dl,low ATC_INDEX
	mov	al,20h
	out	dx,al			; Re-enable attribute controller
;
SetCGAPalette_exit:
	popa
	pop	ds
	pop	si
	iret
SetCGAPalette	endp
;
;	Function 10h:
;	PaletteHandler - EGA/VGA palette function handler
;
;	Entry:	DS, SI are pushed on the stack
;		AL = Subfunction
;	Exit:	Dependent on subfunction
;
	public	tblPaletteHandler
tblPaletteHandler	label	word
	dw	SetOnePaletteRegister		; Subfunction 0
	dw	SetOverscanRegister		; Subfunction 1
	dw	SetAllPaletteRegisters		; Subfunction 2
	dw	ToggleIntensityBlinkBit		; Subfunction 3
	dw	NullRoutine
	dw	NullRoutine
	dw	NullRoutine
	dw	GetOnePaletteRegister		; Subfunction 7
	dw	GetOverscanRegister		; Subfunction 8
	dw	GetAllPaletteRegisters		; Subfunction 9
	dw	NullRoutine
	dw	NullRoutine
	dw	NullRoutine
	dw	NullRoutine
	dw	NullRoutine
	dw	NullRoutine
	dw	SetOneDACRegister		; Subfunction 10h
	dw	NullRoutine
	dw	SetBlockOfDACRegisters		; Subfunction 12h
	dw	SetColorPage			; Subfunction 13h
	dw	NullRoutine
	dw	GetDACRegister			; Subfunction 15h
	dw	NullRoutine
	dw	GetBlockOfDACRegisters		; Subfunction 17h
	dw	SetDACMask			; Subfunction 18h
	dw	GetDACMask			; Subfunction 19h
	dw	GetColorPage			; Subfunction 1Ah
	dw	SumDACToGrayScale		; Subfunction 1Bh
LENGTH_PALETTE_HANDLER_TABLE	equ	($ - tblPaletteHandler) / 2
;
	public	PaletteHandler
PaletteHandler	proc
	assume	ds:Seg0, es:nothing
	cmp	al,LENGTH_PALETTE_HANDLER_TABLE	; Out of range?
	jae	PaletteHandler_exit		;  Y: Go exit
	test	[fEGAInfo],VGA_INACTIVE		; VGA active?
	jnz	PaletteHandler_exit		;  N: Go exit
	push	ax
	cbw
	mov	si,ax
	shl	si,1
	call	cs:[tblPaletteHandler + si]
	pop	ax
;
PaletteHandler_exit:
	pop	ds
	pop	si
	iret
PaletteHandler	endp
;
;	Subfunction 000h:
;	SetOnePaletteRegister - Set one palette register
;
;	Entry:	DS = Seg0
;		BL = ATC index
;		BH = Value to set
;	Exit:	None
;
;	AX, SI are trashed.
;	DX is preserved.
;
	public	SetOnePaletteRegister, NullRoutine
SetOnePaletteRegister	proc
	assume	ds:Seg0, es:nothing
	push	dx
;
	push	ds
	lds	si,[lpSavePtr]
	assume	ds:nothing
	lds	si,[si].SAVEPTR.SaveAreaPtr
	mov	ax,ds
	or	ax,si				; SavePtr ptr = NULL?
	jz	SetOnePaletteRegister_20	;  Y: Skip saving
	mov	al,bl
	sub	ah,ah
	cmp	al,10h				; ATC Mode register?
	je	SetOnePaletteRegister_20	;  Y: Skip saving
	cmp	al,11h				; Overscan register?
	ja	SetOnePaletteRegister_20	;  >: Skip saving
SetOnePaletteRegister_10:
	add	si,ax
	mov	[si],bh
;
SetOnePaletteRegister_20:
	assume	ds:Seg0
	pop	ds
;
; Use a wait for vertical retrace instead of a wait
; for horizontal blank here to be compatible with the
; Borland's "BGIDEMO" program. A wait for horizontal
; blank causes tearing on some systems.
;
	call	WaitVerticalRetrace
;
	cli
	in	al,dx
	mov	dl,low ATC_INDEX
	mov	al,bl
	out	dx,al
	mov	al,bh
	out	dx,al
	mov	al,020h
	out	dx,al
	sti
;
	pop	dx
NullRoutine	proc
	ret
NullRoutine	endp
SetOnePaletteRegister	endp
;
;	Subfunction 001h:
;	SetOverscanRegister - Set the overscan register
;
;	Entry:	DS = Seg0
;		BH = Value to set
;	Exit:	None
;
;	SI, DS are trashed.
;
	public	SetOverscanRegister
SetOverscanRegister	proc
	assume	ds:Seg0, es:nothing
	push	dx
;
	push	ds
	lds	si,[lpSavePtr]
	assume	ds:nothing
	lds	si,[si].SAVEPTR.SaveAreaPtr
	mov	ax,ds
	or	ax,si
	jz	SetOverscanRegister_10
	add	si,17
	mov	[si],bh
SetOverscanRegister_10:
	assume	ds:Seg0
	pop	ds
;
	call	WaitVerticalRetrace
	cli
	in	al,dx
	mov	dl,low ATC_INDEX
	mov	al,31h
	out	dx,al
	mov	al,bh
	out	dx,al
	mov	al,020h
	out	dx,al
	sti
;
	pop	dx
	ret
SetOverscanRegister	endp
;
;	Subfunction 002h:
;	SetAllPaletteRegisters - Set all palette registers
;
;	Entry:	DS = Seg0
;		ES:DX = Pointer to 17-byte table
;	Exit:	None
;
;	AX, SI, DS are trashed.
;	CX, DX, DI, ES are preserved.
;
	public	SetAllPaletteRegisters
SetAllPaletteRegisters	proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	dx
	push	di
	push	es
;
	mov	si,dx
	call	WaitVerticalRetrace
	push	es
	les	di,[lpSavePtr]
	pop	ds
	assume	ds:nothing
	les	di,es:[di].SAVEPTR.SaveAreaPtr
	mov	ax,es
	or	ax,di
	jz	SetAllPaletteRegisters_10
	mov	cx,17
	push	si
	rep movsb
	pop	si
SetAllPaletteRegisters_10:
;
	mov	cx,17			; Number of ATC regs + overscan
	sub	ah,ah			; Start index
	call	SetPaletteAndOverscanFromBuffer
;
	pop	es
	pop	di
	pop	dx
	pop	cx
	ret
SetAllPaletteRegisters	endp
;
;	Subfunction 003h:
;	ToggleIntensityBlinkBit - Set the intensity or blinking bit in the ATC mode register
;
;	Entry:	DS = Seg0
;		BL = Toggle state:
;		  0: Enable intensity
;		  1: Enable blinking
;	Exit:	None
;
	public	ToggleIntensityBlinkBit
ToggleIntensityBlinkBit	proc
	assume	ds:Seg0, es:nothing
	push	dx
	mov	dx,[wCRTCAddr]
	add	dl,6
	cli
	in	al,dx
	mov	dl,low ATC_INDEX
	mov	al,030h
	call	GetIndexRegister	; This leaves ATC in data state
	mov	al,bl
	and	al,1
	shl	al,3
	and	ah,0F7h
	or	al,ah
	out	dx,al
	mov	al,20h
	out	dx,al
	sti
	pop	dx
	ret
ToggleIntensityBlinkBit	endp
;
;	Subfunction 007h:
;	GetOnePaletteRegister - Read one ATC register
;
;	Entry:	DS = Seg0
;		BL = Index to be read
;	Exit:	BH = Value read
;
;	Note:	This function may be called by the get information
;		(Function 1Bh) BIOS call, even when the BIOS is
;		currently talking to a monochrome adapter. Therefore,
;		the BIOS variable for the VGA's CRTC may not be
;		accurate.
;
	public	GetOnePaletteRegister
GetOnePaletteRegister	proc
	assume	ds:Seg0, es:nothing
	push	dx
	cli
	call	ResetATCFlipFlop	; Set ATC to index state
	mov	dx,ATC_INDEX
	mov	al,bl
	call	GetIndexRegister
	mov	bh,ah			; Value to return
	mov	al,ah
	out	dx,al			; Reset to index state
	mov	al,20h
	out	dx,al			; Turn display back on
	call	ResetATCFlipFlop	; For compatibility with DOS EDIT
	sti
	pop	dx
	ret
GetOnePaletteRegister	endp
;
;	Subfunction 008h:
;	GetOverscanRegister - Read overscan register
;
;	Entry:	DS = Seg0
;	Exit:	BH = Value read
;
	public	GetOverscanRegister
GetOverscanRegister	proc
	assume	ds:Seg0, es:nothing
	push	dx
	mov	dx,[wCRTCAddr]
	add	dl,6
	cli
	in	al,dx
	mov	dl,low ATC_INDEX
	mov	al,11h
	call	GetIndexRegister
	mov	bh,ah			; Value to return
	mov	al,ah
	out	dx,al			; Reset to index state
	mov	al,20h
	out	dx,al			; Turn display back on
	sti
	pop	dx
	ret
GetOverscanRegister	endp
;
;	Subfunction 009h:
;	GetAllPaletteRegisters - Read all the ATC registers
;
;	Entry:	DS = Seg0
;		ES:DX = Pointer to 17-byte table
;	Exit:	None
;
	public	GetAllPaletteRegisters
GetAllPaletteRegisters	proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	dx
	push	di
;
	mov	di,dx
	mov	dx,[wCRTCAddr]
	add	dl,6
	cli
	in	al,dx
	mov	dl,low ATC_INDEX
	sub	ah,ah
	mov	cx,16
GetAllPaletteRegisters_10:
	mov	al,ah
	out	dx,al
	inc	dx
	in	al,dx
	stosb
	dec	dx
	out	dx,al
	inc	ah
	loop	GetAllPaletteRegisters_10
	mov	al,11h
	out	dx,al
	inc	dx
	in	al,dx
	stosb
	dec	dx
	out	dx,al
	mov	al,20h
	out	dx,al
	sti
;
	pop	di
	pop	dx
	pop	cx
	ret
GetAllPaletteRegisters	endp
;
;	Subfunction 010h:
;	SetOneDACRegister - Set one DAC register
;
;	Entry:	DS = Seg0
;		BX = DAC index
;		CL = Blue
;		CH = Green
;		DH = Red
;	Exit:	None
;
	public	SetOneDACRegister
SetOneDACRegister	proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	dx
;
	push	dx
	call	WaitVerticalRetrace	; IBM does this - system BIOS's depend on it
IF (VARIATION_005 eq 1)
	call	OEMWriteRAMDACIdx
ELSE
	mov	dx,DAC_WINDEX
	mov	al,bl
	out	dx,al
ENDIF
	pop	dx
	call	SetDACRegisterAutoIndex
;
	pop	dx
	pop	cx
	ret
SetOneDACRegister	endp
;
;	SetDACRegisterAutoIndex - Physically write one DAC register
;
;	Entry:	DS = Seg0
;		DH = Red
;		CH = Green
;		CL = Blue
;	Exit:	None
;
;	Note:	1) Assume index has been set, or in a loop, auto-index
;		   is functioning.
;		2) If summing to gray is required, the values will be
;		   adjusted here.
;
	public	SetDACRegisterAutoIndex
SetDACRegisterAutoIndex	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	dx
;
	test	[fVGAInfo],PALETTE_SUMMING + MONO_ANALOG	; Summing active?
	jz	short SetDACRegisterAutoIndex_10		;  N: Go load register
	call	SumSingleValue
;
SetDACRegisterAutoIndex_10:
IF (VARIATION_005 eq 1)
	call	OEMWriteRAMDACData
ELSE
	mov	al,dh
	mov	dx,DAC_DATA
	cli
	out	dx,al				; Red
	mov	al,ch
	out	dx,al				; Green
	mov	al,cl
	out	dx,al				; Blue
	sti
ENDIF
;
	pop	dx
	pop	ax
	ret
SetDACRegisterAutoIndex	endp
;
;	Subfunction 012h:
;	SetBlockOfDACRegisters - Set block of DAC registers
;
;	Entry:	DS = Seg0
;		BX = First DAC index
;		CX = Number of registers to set
;		ES:DX = Pointer to a table of RGB values
;	Exit:	None
;
	public	SetBlockOfDACRegisters
SetBlockOfDACRegisters	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	dx
;
	mov	si,dx
	call	WaitVerticalRetrace
	call	SetRAMDACRangeWithSumming
;
	pop	dx
	pop	cx
	pop	bx
	ret
SetBlockOfDACRegisters	endp
;
;	SetRAMDACRangeWithSumming - Set a block of RAMDAC registers with gray scale summing if necessary
;
;	Entry:	ES:SI = Pointer to RAMDAC table
;		BL = Starting index
;		CX = Number of registers to load
;		DS = Seg0
;	Exit:	None
;
;	AX, BX, CX, DX are trashed
;
	public	SetRAMDACRangeWithSumming
SetRAMDACRangeWithSumming	proc
	assume	ds:Seg0, es:nothing
IF (VARIATION_005 eq 1)
	call	OEMWriteRAMDACIdx
ELSE
	mov	al,bl
	mov	dx,DAC_WINDEX
	out	dx,al				; Set RAMDAC index
ENDIF
	mov	bx,cx
SetRAMDACRangeWithSumming_10:
	mov	dh,es:[si]			; DH = Red
	inc	si
	mov	ch,es:[si]			; CH = Green
	inc	si
	mov	cl,es:[si]			; CL = Blue
	inc	si
	call	SetDACRegisterAutoIndex
	dec	bx
	jnz	short SetRAMDACRangeWithSumming_10
	ret
SetRAMDACRangeWithSumming	endp
;
;	Subfunction 013h:
;	SetColorPage - Set color page (ATC 14h)
;
;	Entry:	DS = Seg0
;		BL = Page type:
;		  0: Select paging mode
;			BH = Page mode selection:
;			  0: Select 4 pages of 64 DAC registers
;			  1: Select 16 pages of 16 DAC registers
;		  1: Select color page
;			BH = Select page:
;			  0-3: 4-page mode
;			  0-F: 16-page mode
;	Exit:	None
;
	public	SetColorPage
SetColorPage	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	dx
;
	cmp	bl,1			; Function out of range?
	ja	SetColorPage_exit	;  Y: Abort subfunction
	mov	dx,[wCRTCAddr]
	add	dl,6			; DX = Input status 1
	cli
	in	al,dx			; Reset flip-flop to index state
	mov	dl,low ATC_INDEX
	mov	al,10h
	call	GetIndexRegister	; AH = ATC mode register
	test	bl,bl			; Select paging mode?
	jnz	SetColorPage_20		;  N: Go set page
;
; Select paging mode
;
	mov	al,ah
	and	al,07fh
	and	bh,1
	ror	bh,1
	or	al,bh
	out	dx,al
	mov	al,20h
	out	dx,al
	jmp	short SetColorPage_exit
;
; Select color page
;
SetColorPage_20:
	mov	al,ah			; Reset back to index state by
	out	dx,al			;  writing data value back into reg
	mov	al,14h
	out	dx,al			; Set index to color page register
	test	ah,080h			; 16 pages of 16 DAC regs?
	jnz	SetColorPage_30		;  Y: Go set requested page
	shl	bh,2			;  N: Adjust for 4 pages of 64 regs
SetColorPage_30:
	mov	al,bh
	out	dx,al			; Set page
	mov	al,20h
	out	dx,al			; Turn display back on
;
SetColorPage_exit:
	sti
	pop	dx
	pop	bx
	ret
SetColorPage	endp
;
;	Subfunction 015h:
;	GetDACRegister - Read one DAC register
;
;	Entry:	DS = Seg0
;		BX = Index of DAC register
;	Exit:	CL = Blue value
;		CH = Green value
;		DH = Red value
;
	public	GetDACRegister
GetDACRegister	proc
	assume	ds:Seg0, es:nothing
IF (VARIATION_005 eq 1)
	call	OEMReadRAMDAC
	ret
ELSE
	push	dx
	mov	dx,DAC_RINDEX
	mov	al,bl
	out	dx,al
	inc	dx
	inc	dx
	cli
	in	al,dx
	mov	ah,al
	in	al,dx
	mov	ch,al
	in	al,dx
	mov	cl,al
	sti
	pop	dx
	mov	dh,ah
	ret
ENDIF
GetDACRegister	endp
;
;	Subfunction 017h:
;	GetBlockOfDACRegisters - Read a block of DAC registers
;
;	Entry:	DS = Seg0
;		ES:DX = Pointer to table that receives RGB values
;		BX = Index to start reading DAC
;		CX = Number of DAC registers to read
;	Exit:	None
;
	public	GetBlockOfDACRegisters
GetBlockOfDACRegisters	proc
	assume	ds:Seg0, es:nothing
IF (VARIATION_005 eq 1)
	push	bx
	push	cx
	push	dx
	push	di
;
	mov	di,dx
	mov	dx,cx
GetBlockOfDACRegisters_10:
	call	OEMReadRAMDAC
	mov	es:[di],dh
	inc	di
	mov	es:[di],ch
	inc	di
	mov	es:[di],cl
	inc	di
	inc	bx
	dec	dx
	jnz	short GetBlockOfDACRegisters_10
;
	pop	di
	pop	dx
	pop	cx
	pop	bx
	ret
ELSE
	push	cx
	push	dx
	push	di
;
	mov	di,dx
	mov	dx,DAC_RINDEX
	mov	al,bl
	out	dx,al
	inc	dx
	inc	dx
	mov	ax,cx
GetBlockOfDACRegisters_10:
	cli
	in	al,dx
	stosb
	in	al,dx
	stosb
	in	al,dx
	stosb
	sti
	loop	GetBlockOfDACRegisters_10
;
	pop	di
	pop	dx
	pop	cx
	ret
ENDIF
GetBlockOfDACRegisters	endp
;
;	Subfunction 018h:
;	SetDACMask - Set the DAC mask
;
;	Entry:	DS = Seg0
;		BL = DAC mask
;	Exit:	None
;
	public	SetDACMask
SetDACMask	proc
	assume	ds:Seg0, es:nothing
	push	dx
	mov	dx,DAC_MASK
	mov	al,bl
	out	dx,al
	pop	dx
	ret
SetDACMask	endp
;
;	Subfunction 019h:
;	GetDACMask - Get the DAC mask
;
;	Entry:	DS = Seg0
;	Exit:	BL = DAC mask
;
	public	GetDACMask
GetDACMask	proc
	assume	ds:Seg0, es:nothing
	push	dx
	mov	dx,DAC_MASK
	in	al,dx
	mov	bl,al
	pop	dx
	ret
GetDACMask	endp
;
;	Subfunction 01Ah:
;	GetColorPage - Return the value of the color page
;
;	Entry:	DS = Seg0
;	Exit:	BL = Paging mode
;		BH = Current page
;
	public	GetColorPage
GetColorPage	proc
	assume	ds:Seg0, es:nothing
	push	dx
	mov	dx,[wCRTCAddr]
	add	dl,6			; DX = Input status 1
	cli
	in	al,dx			; Reset flip-flop to index mode
	mov	dl,low ATC_INDEX
	mov	al,010h
	call	GetIndexRegister	; AH = ATC mode register
	mov	al,ah			; Reset back to index state by
	out	dx,al			;  writing data value back into reg
	mov	bl,ah
	mov	al,014h
	call	GetIndexRegister	; Get current page
	mov	al,ah
	out	dx,al
	mov	al,20h
	out	dx,al
	sti
	mov	bh,ah
	and	bl,080h
	rol	bl,1
	jc	GetColorPage_10		; 16 pages of 16 DAC regs?
	shr	bh,2			;  N: adjust for 4 pages of 64 regs
GetColorPage_10:
	pop	dx
	ret
GetColorPage	endp
;
;	Subfunction 01Bh:
;	SumDACToGrayScale - Sum a range of DAC registers to gray scale
;
;	Entry:	DS = Seg0
;		BX = Index to start summing
;		CX = Number of registers to sum
;	Exit:	None
;
	public	SumDACToGrayScale
SumDACToGrayScale	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	dx
;
	jcxz	SumDACToGrayScale_20
SumDACToGrayScale_10:
	push	cx
	call	GetDACRegister
	call	SumSingleValue
	call	SetOneDACRegister
	inc	bx
	pop	cx
	loop	SumDACToGrayScale_10
SumDACToGrayScale_20:
;
	pop	dx
	pop	cx
	pop	bx
	ret
SumDACToGrayScale	endp
;
;	SumSingleValue - Sum one value to gray
;
;	Entry:	CL = Blue
;		CH = Green
;		DH = Red
;	Exit:	CL = CH = DH = Gray value
;
;	AX, DL are trashed
;
;	Formula: Value = ((red * 30) + (green * 59) + (blue * 11) + 50) / 100
;
	public	SumSingleValue
SumSingleValue	proc
	assume	ds:nothing, es:nothing
	push	bx
	mov	al,30			; 30% Red
	mul	dh
	mov	bx,ax			; Accumulate value in BX
	mov	al,59			; 59% Green
	mul	ch
	add	bx,ax
	mov	al,11			; 11% Blue
	mul	cl
	add	ax,bx
	sub	dx,dx
	mov	bx,100			; Move decimal point back two positions
	add	ax,50			; Round off
	div	bx
	mov	dh,al
	mov	cl,al
	mov	ch,al
	pop	bx
	ret
SumSingleValue	endp
;
;	SetPaletteAndOverscanFromBuffer - Set the palette registers and
;	overscan from a (maximum) 17-byte buffer. Special handling is
;	necessary, because there is an extra register between the overscan
;	and the end of the palette registers.
;
;	Entry:	DS:SI = Pointer to buffer
;		CX = Number of registers to set
;		AH = First register to set
;		DX = Port address of the Input Status 1 register
;	Exit:	None
;
;	AX, CX, DX, SI are trashed
;
	public	SetPaletteAndOverscanFromBuffer
SetPaletteAndOverscanFromBuffer	proc
	cli
	in	al,dx			; Set index/data toggle to index state
	mov	dl,low ATC_INDEX
SetPalAndOscanFromBuf_10:
	cmp	ah,10h			; Already passed palette regs?
	jae	SetPalAndOscanFromBuf_30; Y: Go handle overscan
	mov	al,ah
	out	dx,al			; Set index
	lodsb				; Get palette data
	out	dx,al			; Set data
	inc	ah			; Next index...
	loop	SetPalAndOscanFromBuf_10
SetPalAndOscanFromBuf_20:
	mov	al,20h			; Turn VGA access to palette back on
	out	dx,al
	sti
	ret
;
; Overscan handler
;
SetPalAndOscanFromBuf_30:
	mov	al,11h
	out	dx,al
	lodsb
	out	dx,al
	jmp	short SetPalAndOscanFromBuf_20
SetPaletteAndOverscanFromBuffer	endp
;
IF (VARIATION_001 eq 0)
;
;	FillDAC - Fill the RAMDAC with a given value
;
;	Entry:	AH = Red value
;		BL = Green value
;		BH = Blue value
;	Exit:	None
;
	public	FillDAC
FillDAC	proc
  IF (VARIATION_005 eq 1)
	pusha
	mov	dh,ah
	mov	ch,bl
	mov	cl,bh
	sub	bl,bl
	call	OEMWriteRAMDACIdx
	mov	dx,256
FillDAC_10:
	call	OEMWriteRAMDACData
	dec	dx
	jnz	short FillDAC_10
	popa
	ret
  ELSE
	pusha
	mov	dx,DAC_WINDEX
	sub	al,al
	out	dx,al			; Start at index 0
	inc	dx			; DX = 3C9h
	mov	cx,256			; All 256 DAC registers
FillDAC_10:
	mov	al,ah			; Red
	out	dx,al
	mov	al,bl			; Green
	out	dx,al
	mov	al,bh			; Blue
	out	dx,al
	loop	FillDAC_10
	popa
	ret
  ENDIF			; VARIATION_005
FillDAC	endp
ENDIF			; VARIATION_001
;
;	LoadATCRange - Load a range of ATC registers
;
;	Entry:	DS:SI = Pointer to parameter
;		BL = Index to start
;		CX = Number of registers to load
;		DX = I/O Address of Input Status Register
;	Exit:	DS:SI = Updated pointer
;		CX = 0
;
;	All other registers preserved.
;
	public	LoadATCRange
LoadATCRange	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
	cli
	in	al,dx
	mov	dl,low ATC_INDEX
LoadATCRange_10:
	mov	al,bl
	out	dx,al
	lodsb
	out	dx,al
	inc	bx
	loop	LoadATCRange_10
	sti
	pop	dx
	pop	ax
	ret
LoadATCRange	endp
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


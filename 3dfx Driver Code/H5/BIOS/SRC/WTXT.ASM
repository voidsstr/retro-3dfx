	page	57, 132
	title	Character Write Functions
	subttl	WTXT.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	WTXT.ASM - Character write functions
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	WriteCharAttribute		Write character/attribute to video memory
;	WriteCharAttributeMonoText	Write to monochrome text mode
;	WriteCharAttributeColorText	Write to color text mode
;	WriteCharAttributeCGA		Write a character in CGA graphics mode
;	WriteCharAttributePlanar	Write a character in planar graphics mode
;	WriteCharAttribute256		Write a character in 256-color graphics mode
;	WriteCharAttribute16BPP		Write a character in 16 Bit-color graphics mode
;	WriteCharAttribute24BPP		Write a character in 24 Bit-color graphics mode
;	WriteChar			Write character to character memory
;	WriteCharMonoText		Write character only in monochrome text mode
;	WriteCharColorText		Write character only in color text mode
;	WriteTTY			Write character to video memory making screen behave as TTY device
;	WriteTTYMonoText		Write character as TTY in mono text mode
;	WriteTTYColorText		Write character as TTY in color text mode
;	WriteTTYCGA			Write character as TTY in CGA graphics mode
;	WriteTTYPlanar			Write character as TTY in planar graphics mode
;	WriteTTY256			Write character as TTY in 256-color graphics mode
;	WriteTTY16BPP			Write character as TTY in 16 Bit-color graphics mode
;	WriteTTY24BPP			Write character as TTY in 24 Bit-color graphics mode
;	WriteCGAChar			Write one character to CGA memory
;	Write256Char			Write one character to VGA memory
;	Write16BPPChar			Write one character in 16 Bit graphics mode
;	Write24BPPChar			Write one character in 24 bit graphics mode
;	ExpandCGAByteToWord		Expand a font byte to CGA color word for CGA lo-res mode
;	WritePlanarChar			Write one planar character
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/WTXT.ASM $
;	$Log: /devel/h5/BIOS/SRC/WTXT.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
IF (EXTENDED_BIOS and EXTENDED_TTY)
	%out	!	Assembling: WTXT.ASM - Extensions with TTY functions
ELSE
  IF (EXTENDED_BIOS and not EXTENDED_TTY)
	%out	!	Assembling: WTXT.ASM - Extensions without TTY functions
  ELSE
	%out	!	Assembling: WTXT.ASM
  ENDIF
ENDIF
;
.MODEL TINY
.CODE
.386
;
	extrn	LocalSetCursorPosition:near
	extrn	GetIndexRegister:near
	extrn	ShortBeep:near
	extrn	GetModeDescPtr:near
	extrn	SetActivePage:near
	extrn	GetRowOffset:near
	extrn	GetDSSeg0:near
	extrn	GetESSegA000:near
	extrn	GetESSegB000:near
	extrn	GetESSegB800:near
IF (EXTENDED_BIOS and EXTENDED_TTY)
	extrn	OEMSetBank:near
	extrn	OEMIncBank:near
	extrn	GetBPP:near
ENDIF
;
;	Function 09h:
;	WriteCharAttribute - Write character/attribute to video memory
;
;	Entry:	DS, SI are pushed on the stack
;		DS = Seg0
;		AL = Character to write
;		BL = Attribute of character (if bit 7 = 1: XOR char)
;		BH = Display page to write to
;		CX = Number of times to write character
;	Exit:	None
;
; The following table must have one entry per mode type
;
	public	tblWriteCharAttribute
tblWriteCharAttribute	label	word
	dw	WriteCharAttributeColorText
	dw	WriteCharAttributeMonoText
	dw	WriteCharAttributeCGA
	dw	WriteCharAttributePlanar
	dw	WriteCharAttributePlanar
	dw	WriteCharAttributePlanar
	dw	WriteCharAttribute256
IF EXTENDED_BIOS
  IF EXTENDED_TTY
	dw	WriteCharAttribute16BPP
	dw	WriteCharAttribute24BPP
	dw	WriteCharDoNothing
  ELSE
	dw	WriteCharDoNothing
	dw	WriteCharDoNothing
	dw	WriteCharDoNothing
  ENDIF
ENDIF
;
	public	WriteCharAttribute
WriteCharAttribute	proc
	assume	ds:Seg0, es:nothing
	pusha
	push	es
;
	push	ax					; Save character code
	push	bx					; Save page and attribute
	mov	al,[byMode]
	call	GetModeDescPtr				; Returns ptr in ES:BX
	mov	si,word ptr es:[bx].MODEDESC.mdType	; Get mode type from table
	pop	bx					; Restore page and attribute
	pop	ax					; Restore character code
	jc	short WriteCharAttribute_exit		; Error? Y: Mode doesn't exist
;
	and	si,00FFh
	shl	si,1					; Each is a WORD entry
	call	cs:[tblWriteCharAttribute + si]
;
WriteCharAttribute_exit:
	pop	es
	mov	bp,sp
	mov	[bp+0Eh],ax			; For compatibility with model 70
	popa
	pop	ds
	pop	si
	iret
WriteCharAttribute	endp
;
;	WriteCharAttributeMonoText - Write to monochrome text mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BL = Attribute of character
;		BH = Display page to write to
;		CX = Number of times to write character
;	Exit:	None
;
	public	WriteCharAttributeMonoText
WriteCharAttributeMonoText	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB000			; Get video segment
	jmp	short WriteCharAttText
WriteCharAttributeMonoText	endp
;
;	WriteCharAttributeColorText - Write to color text mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BL = Attribute of character
;		BH = Display page to write to
;		CX = Number of times to write character
;	Exit:	None
;
;	AX, BX, CX, DX, DI, SI, ES are trashed.
;
WriteCharAttText_10:
	assume	ds:Seg0, es:nothing
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	shr	bx,1
WriteCharAttText_20:
	add	si,[wRegenLength]
	dec	bx
	jnz	WriteCharAttText_20
	jmp	short WriteCharAttText_30
;
	public	WriteCharAttributeColorText, WriteCharAttText
WriteCharAttributeColorText	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB800			; Get video segment
;
; At this point, ES is either set to B000h (mono text) or B800h (color text)
;
WriteCharAttText	label	near
	push	ax			; For compatibility with Model 70
	mov	ah,bl
	sub	si,si
	test	bh,bh			; Page 0?
	jnz	WriteCharAttText_10	;  N: Go handle "non-standard" condition
	mov	dx,[wCursorPosition]	;  Y: Use default values
WriteCharAttText_30	label	near
	mov	di,ax
	mov	ax,[wColumns]
	mul	dh
	add	al,dl
	adc	ah,0
	shl	ax,1
	add	ax,si
	xchg	ax,di
	rep stosw
	pop	ax			; For compatibility with Model 70
	public	WriteCharDoNothing
WriteCharDoNothing	label	near
	ret
WriteCharAttributeColorText	endp
;
;	WriteCharAttributeCGA - Write a character in CGA graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BL = Attribute of character (if bit 7 = 1: XOR char)
;		CX = Number of times to write character
;	Exit:	None
;
;	AX, BX, CX, DX, BP, DI, SI, DS, ES are trashed.
;
	public	WriteCharAttributeCGA
WriteCharAttributeCGA	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB800			; Get video segment
	mov	dx,[wCursorPosition]
WriteCharAttributeCGA_10:
	push	ax
	call	WriteCGAChar
	pop	ax
	inc	dx
	loop	WriteCharAttributeCGA_10
	sub	ax,ax				; For compatibility with Model 70
	ret
WriteCharAttributeCGA	endp
;
;	WriteCharAttributePlanar - Write a character in planar graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BL = Attribute of character (if bit 7 = 1: XOR char)
;		BH = Display page to write to
;		CX = Number of times to write character
;	Exit:	None
;
;	AX, BX, CX, DX, BP, DI, SI, DS, ES are trashed.
;
	public	WriteCharAttributePlanar
WriteCharAttributePlanar	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short WriteCharAttributePlanar_exit
ENDIF
	call	GetESSegA000			; Point to video segment
	push	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]	; DX = Cursor position
	pop	bx
	cmp	[byMode],11h			; Mode 11h?
	jne	WriteCharAttributePlanar_10	;  N: Go handle normal
	or	bl,0Fh				;  Y: All attributes map to 0Fh (including 00h)
WriteCharAttributePlanar_10:
	push	ax
	call	WritePlanarChar
	pop	ax
	inc	dx
	loop	WriteCharAttributePlanar_10
WriteCharAttributePlanar_exit:
	mov	ax,0F02h			; For compatibility with Model 70
	ret
WriteCharAttributePlanar	endp
;
;	WriteCharAttribute256 - Write a character in 256-color graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BL = Attribute of character (if bit 7 = 1: XOR char)
;		CX = Number of times to write character
;	Exit:	None
;
;	AX, BX, CX, DX, BP, DI, SI, DS, ES are trashed.
;
	public	WriteCharAttribute256
WriteCharAttribute256	proc
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short WriteCharAttribute256_exit
ENDIF
	call	GetESSegA000			; Get video segment
	mov	dx,[wCursorPosition]
WriteCharAttribute256_10:
	push	ax
	call	Write256Char
	pop	ax
	inc	dx
	loop	WriteCharAttribute256_10
WriteCharAttribute256_exit:
	sub	ax,ax				; For compatibility with Model 70
	ret
WriteCharAttribute256	endp
;
IF (EXTENDED_BIOS and EXTENDED_TTY)
;
;	WriteCharAttribute16BPP - Write a character in 16 Bit-color graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BL = Attribute of character (if bit 7 = 1: XOR char)
;		CX = Number of times to write character
;	Exit:	None
;
;	AX, BX, CX, DX, BP, DI, SI, DS, ES are trashed.
;
	public	WriteCharAttribute16BPP
WriteCharAttribute16BPP	proc
	call	GetESSegA000			; Get video segment
	mov	dx,[wCursorPosition]
WriteCharAttribute16BPP_10:
	push	ax
	call	Write16BPPChar
	pop	ax
	inc	dx
	loop	WriteCharAttribute16BPP_10
	ret
WriteCharAttribute16BPP	endp
;
;	WriteCharAttribute24BPP - Write a character in 24 Bit-color graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BL = Attribute of character (if bit 7 = 1: XOR char)
;		CX = Number of times to write character
;	Exit:	None
;
;	AX, BX, CX, DX, BP, DI, SI, DS, ES are trashed.
;
	public	WriteCharAttribute24BPP
WriteCharAttribute24BPP	proc
	call	GetESSegA000			; Get video segment
	mov	dx,[wCursorPosition]
WriteCharAttribute24BPP_10:
	push	ax
	call	Write24BPPChar
	pop	ax
	inc	dx
	loop	WriteCharAttribute24BPP_10
	ret
WriteCharAttribute24BPP	endp
;
ENDIF			; (EXTENDED_BIOS and EXTENDED_TTY)
;
;	Function 0Ah:
;	WriteChar - Write character to character memory
;
;	Entry:	DS, SI are pushed on the stack
;		DS = Seg0
;		AL = Character to write
;		BL = Graphics mode: character color (if bit 7 = 1: XOR char)
;		BH = Display page to write to
;		CX = Number of times to write character
;	Exit:	None
;
; The following table must have one entry per mode type
;
	public	tblWriteChar
tblWriteChar	label	word
	dw	WriteCharColorText
	dw	WriteCharMonoText
	dw	WriteCharAttributeCGA
	dw	WriteCharAttributePlanar
	dw	WriteCharAttributePlanar
	dw	WriteCharAttributePlanar
	dw	WriteCharAttribute256
IF EXTENDED_BIOS
  IF EXTENDED_TTY
	dw	WriteCharAttribute16BPP
	dw	WriteCharAttribute24BPP
	dw	WriteCharDoNothing
  ELSE
	dw	WriteCharDoNothing
	dw	WriteCharDoNothing
	dw	WriteCharDoNothing
  ENDIF
ENDIF
;
	public	WriteChar
WriteChar	proc
	pusha
	push	es				
;						
	push	ax					; Save character code
	push	bx					; Save page & attribute
	mov	al,[byMode]
	call	GetModeDescPtr				; Returns ptr in ES:BX
	mov	si,word ptr es:[bx].MODEDESC.mdType	; Get mode type from table
	pop	bx					; Restore page & attribute
	pop	ax					; Restore character code
	jc	short WriteChar_exit			; Error? Y: Mode doesn't exist
;
	and	si,00FFh
	shl	si,1				; Each is a WORD entry
	call	cs:[tblWriteChar + si]
;
WriteChar_exit:
	pop	es
	mov	bp,sp
	mov	[bp+0Eh],ax			; For compatibility with model 70
	popa
	pop	ds
	pop	si
	iret
WriteChar	endp
;
;	WriteCharMonoText - Write character only in monochrome text mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BH = Display page to write to
;		CX = Number of times to write character
;	Exit:	None
;
	public	WriteCharMonoText
WriteCharMonoText	proc
	call	GetESSegB000			; Get video segment
	jmp	short WriteCharText
WriteCharMonoText	endp
;
;	WriteCharColorText - Write character only in color text mode
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BH = Display page to write to
;		CX = Number of times to write character
;	Exit:	none
;
WriteCharText_10:
	assume	ds:Seg0, es:nothing
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	shr	bx,1				; Restore org page
WriteCharText_20:
	add	si,[wRegenLength]
	dec	bx
	jnz	WriteCharText_20
	jmp	short WriteCharText_30
;
	public	WriteCharColorText, WriteCharText
WriteCharColorText	proc
	call	GetESSegB800			; Get video segment
;
; At this point, ES is either set to B000h (mono text) or B800h (color text)
;
WriteCharText	label	near
	push	ax				; For compatability with Model 70
	sub	si,si
	test	bh,bh				; Page 0?
	jnz	WriteCharText_10		;  N: Go do "non-standard" calculations
	mov	dx,[wCursorPosition]		;  Y: Use default values
WriteCharText_30	label	near
	mov	di,ax
	mov	ax,[wColumns]
	mul	dh
	add	al,dl
	adc	ah,0
	shl	ax,1
	add	ax,si
	xchg	ax,di
WriteCharText_40:
	stosb
	inc	di
	loop	WriteCharText_40
	pop	ax				; For compatability with Model 70
	ret
WriteCharColorText	endp
;
;	Function 0Eh:
;	WriteTTY - Write character to video memory making screen behave as TTY device
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		AL = Character to write to TTY device
;		BL = Foreground color in graphics modes
;	Exit:	None
;
; The following table must have one entry per mode type
;
	public	tblWriteTTY
tblWriteTTY	label	word
	dw	WriteTTYColorText
	dw	WriteTTYMonoText
	dw	WriteTTYCGA
	dw	WriteTTYPlanar
	dw	WriteTTYPlanar
	dw	WriteTTYPlanar
	dw	WriteTTY256
IF EXTENDED_BIOS
  IF EXTENDED_TTY
	dw	WriteTTY16BPP
	dw	WriteTTY24BPP
	dw	WriteCharDoNothing
  ELSE
	dw	WriteCharDoNothing
	dw	WriteCharDoNothing
	dw	WriteCharDoNothing
  ENDIF
ENDIF
;
	public	WriteTTY
WriteTTY	proc
	pusha
	push	es
;
	push	ax					; Save character code
	push	bx					; Save attribute
	mov	al,[byMode]
	call	GetModeDescPtr				; Returns ptr in ES:BX
	mov	si,word ptr es:[bx].MODEDESC.mdType	; Get mode type from table
	pop	bx					; Restore attribute
	pop	ax					; Restore character code
	jc	short WriteTTY_exit			; Error? Y: Mode doesn't exist
;	
	and	si,00FFh
	shl	si,1					; Each is a WORD entry
	call	cs:[tblWriteTTY + si]
;
WriteTTY_exit:
	pop	es
	popa
	pop	ds
	pop	si
	iret
WriteTTY	endp
;
;	WriteTTYMonoText - Write character as TTY in mono text mode
;
;	Entry:	DS = Seg0
;		AL = Character to write to TTY device
;	Exit:	None
;
	public	WriteTTYMonoText
WriteTTYMonoText	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB000			; Get video segment
	jmp	short WriteTTYText
WriteTTYMonoText	endp
;
;	WriteTTYColorText - Write character as TTY in color text mode
;
;	Entry:	DS = Seg0
;		AL = Character to write to TTY device
;	Exit:	None
;
;	AX, BX, CX, DX, BP, DI, SI, DS, ES are trashed.
;
	public	WriteTTYColorText, WriteTTYText
WriteTTYColorText	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB800			; Get video segment
;
; At this point, ES is either set to B000h (mono text) or B800h (color text)
;
WriteTTYText	label	near
	mov	bl,[byActivePage]
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	mov	bh,[byActivePage]
	cmp	al,13			; Carriage return?
	jbe	WriteTTYText_30		;  <=: Go handle possible ctrl character
WriteTTYText_10:			;  >: Treat as character
	push	ax
	mov	ax,[wColumns]
	mul	dh
	mov	di,[wRegenStart]
	add	al,dl
	adc	ah,0
	shl	ax,1
	add	di,ax
	pop	ax
	stosb
	inc	dl
	cmp	dl,byte ptr [wColumns]	; End of row?
	jae	WriteTTYText_20		;  Y: Go emulate carriage return / linefeed
	call	LocalSetCursorPosition
	ret
;
WriteTTYText_20:
	sub	dl,dl
	jmp	short WriteTTYText_50
;
WriteTTYText_30:
	jb	WriteTTYText_40		; Below? - test for control char
;
; Carriage return
;
	sub	dl,dl
	call	LocalSetCursorPosition
	ret
;
WriteTTYText_40:
	cmp	al,10			; Linefeed?
	ja	WriteTTYText_10		;  Above: Go write regular character
	jb	WriteTTYText_70		;  Below: Check for control char
;
; Linefeed
;
WriteTTYText_50:
	cmp	dh,[byRows]
	jae	WriteTTYText_60
	inc	dh
	call	LocalSetCursorPosition
	ret
;
; Full screen scroll
;
WriteTTYText_60:
	push	cx
	mov	dh,[byRows]
	call	LocalSetCursorPosition
	mov	di,[wRegenStart]
	mov	si,[wColumns]
	mov	ax,si
	shl	si,1
	mul	[byRows]
	mov	cx,ax
	push	ds
	push	es
	pop	ds
	rep movsw
	pop	ds
	mov	ah,es:[di+1]		; Get current attribute
	mov	al,020h			; Fill with spaces
	mov	cx,[wColumns]
	rep stosw
	pop	cx
	ret
;
WriteTTYText_70:
	cmp	al,7			; Bell?
	je	WriteTTYText_90		;  Y: Go handle bell
	cmp	al,8			; Backspace?
	jne	WriteTTYText_10		;  N: Go do regular character
;
; Backspace
;
	test	dl,dl			; Beginning of row?
	jz	WriteTTYText_80		;  Y: Skip movement
	dec	dx			;  N: Go back one column
	call	LocalSetCursorPosition
WriteTTYText_80:
	ret
;
; Bell
;
WriteTTYText_90:
	call	ShortBeep
	ret
WriteTTYColorText	endp
;
;	WriteTTYCGA - Write character as TTY in CGA graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write to TTY device
;		BL = Color to write
;	Exit:	None
;
	public	WriteTTYCGA
WriteTTYCGA	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB800			; Get video segment
	mov	dx,[wCursorPosition]
	sub	bh,bh
	cmp	al,13
	jbe	WriteTTYCGA_30
WriteTTYCGA_10:
	push	dx
	call	WriteCGAChar
	pop	dx
	inc	dl
	cmp	dl,byte ptr [wColumns]
	jae	WriteTTYCGA_20
	call	LocalSetCursorPosition
	ret
;
WriteTTYCGA_20:
	sub	dl,dl
	jmp	short WriteTTYCGA_50
;
WriteTTYCGA_30:
	jb	WriteTTYCGA_40
;
; Carriage return
;
	sub	dl,dl
	call	LocalSetCursorPosition
	ret
WriteTTYCGA_40:
	cmp	al,10			; Linefeed?
	ja	WriteTTYCGA_10		;  Above: go write regular character
	jb	WriteTTYCGA_70		;  Below: check for other control char
;
; Linefeed
;
WriteTTYCGA_50:
	cmp	dh,[byRows]
	jae	WriteTTYCGA_60
	inc	dh
	call	LocalSetCursorPosition
	ret
;
; Full screen scroll
;
WriteTTYCGA_60:
	push	cx
	mov	dh,[byRows]
	call	LocalSetCursorPosition
	sub	di,di
	mov	si,80*4
	mov	cx,80*24*4
	push	ds
	push	es
	pop	ds
	rep movsb
	mov	di,02000h
	mov	si,02000h+(80*4)
	mov	cx,80*24*4
	rep movsb
	sub	ax,ax
	mov	di,80*24*4
	mov	cx,80*2
	rep stosw
	mov	di,2000h+(80*24*4)
	mov	cx,80*2
	rep stosw
	pop	ds
	pop	cx
	ret
;
WriteTTYCGA_70:
	cmp	al,7			; Bell?
	je	WriteTTYCGA_90		;  Y: Go handle bell
	cmp	al,8			; Backspace?
	jne	WriteTTYCGA_10		;  N: Go do regular character
;
; Backspace
;
	test	dl,dl			; Beginning of row?
	jz	WriteTTYCGA_80		;  Y: Skip movement
	dec	dx			;  N: Move back one column
	call	LocalSetCursorPosition
WriteTTYCGA_80:
	ret
;
; Bell
;
WriteTTYCGA_90:
	call	ShortBeep
	ret
WriteTTYCGA	endp
;
;	WriteTTYPlanar - Write character as TTY in planar graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write to TTY device
;		BL = Color to write
;	Exit:	None
;
	public	WriteTTYPlanar
WriteTTYPlanar	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short WriteTTYPlanar_exit
ENDIF
	call	GetESSegA000			; Get video segment
	movzx	si,[byActivePage]
	shl	si,1
	mov	dx,[wCursorPosition + si]
	mov	bh,[byActivePage]
	cmp	al,13
	jbe	WriteTTYPlanar_30
WriteTTYPlanar_10:
	call	WritePlanarChar
	inc	dl
	cmp	dl,byte ptr [wColumns]
	jae	WriteTTYPlanar_20
	call	LocalSetCursorPosition
WriteTTYPlanar_exit:
	ret
;
WriteTTYPlanar_20:
	sub	dl,dl
	jmp	short WriteTTYPlanar_50
;
WriteTTYPlanar_30:
	jb	WriteTTYPlanar_40
;
; Carriage return
;
	sub	dl,dl
	call	LocalSetCursorPosition
	ret
WriteTTYPlanar_40:
	cmp	al,10			; Linefeed?
	ja	WriteTTYPlanar_10	;  Above: go write regular character
	jb	WriteTTYPlanar_70	;  Below: check for other control char
;
; Linefeed
;
WriteTTYPlanar_50:
	cmp	dh,[byRows]
	jae	WriteTTYPlanar_60
	inc	dh
	call	LocalSetCursorPosition
	ret
;
; Full screen scroll
;
WriteTTYPlanar_60:
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jmp	short WriteTTY256_60	; Go to generic full screen scroll code
ELSE
	push	cx
	mov	dh,[byRows]
	call	LocalSetCursorPosition
	mov	di,[wRegenStart]
	mov	cx,[wColumns]
	mov	al,byte ptr [wCharHeight]
	mul	cl
	mov	si,di
	add	si,ax
	push	ax 			; Number of bytes/character row
	mov	dl,[byRows]
	sub	dh,dh
	mul	dx
	mov	cx,ax
	push	ds
	push	es
	pop	ds
	mov	dx,GDC_INDEX
	mov	al,5
	call	GetIndexRegister
	push	ax
	or	ah,1			; Write mode 1 for latch move
	out	dx,ax
	rep movsb
	pop	ax
	out	dx,ax
	pop	ds
	pop	cx			; Get number of bytes/character row
	sub	ax,ax
	rep stosb
	pop	cx
	ret
ENDIF
;
WriteTTYPlanar_70:
	cmp	al,7			; Bell?
	je	WriteTTYPlanar_90	;  Y: Go handle bell
	cmp	al,8			; Backspace?
	jne	WriteTTYPlanar_10	;  N: Go do regular character
;
; Backspace
;
	test	dl,dl			; Beginning of row?
	jz	WriteTTYPlanar_80	;  Y: Skip movement
	dec	dx			;  N: Go back one column
	call	LocalSetCursorPosition
WriteTTYPlanar_80:
	ret
;
; Bell
;
WriteTTYPlanar_90:
	call	ShortBeep
	ret
WriteTTYPlanar	endp
;
;	WriteTTY256 - Write character as TTY in 256-color graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write to TTY device
;		BL = Color to write
;	Exit:	None
;
	public	WriteTTY256
WriteTTY256	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short WriteTTY256_exit
ENDIF
	call	GetESSegA000			; Get video segment
	mov	dx,[wCursorPosition]
	sub	bh,bh
	cmp	al,13
	jbe	WriteTTY256_30
;
; Entry for "normal" characters (non-control character below a carriage return)
;
WriteTTY256_10:
	push	dx
	call	Write256Char
	pop	dx
	inc	dl
	cmp	dl,byte ptr [wColumns]
	jae	WriteTTY256_20
	call	LocalSetCursorPosition
WriteTTY256_exit:
	ret
;
WriteTTY256_20:
	sub	dl,dl
	jmp	short WriteTTY256_50
;
WriteTTY256_30:
	jb	WriteTTY256_40
;
; Carriage return
;
	sub	dl,dl
	call	LocalSetCursorPosition
	ret
WriteTTY256_40:
	cmp	al,10			; Linefeed?
	ja	WriteTTY256_10		;  Above: go write regular character
	jb	WriteTTY256_70		;  Below: check for other control char
;
; Linefeed
;
WriteTTY256_50:
	cmp	dh,[byRows]
	jae	WriteTTY256_60
	inc	dh
	call	LocalSetCursorPosition
	ret
;
; Full screen scroll
;
WriteTTY256_60	label	near
IF (EXTENDED_BIOS and EXTENDED_TTY)
; Note that planar mode code jumps into this routine (see "WriteTTYPlanar_60")
	push	cx
	mov	dh,[byRows]
	sub	cx,cx
	call	LocalSetCursorPosition
	mov	ax,0601h
	mov	dl,byte ptr [wColumns]
	dec	dx
	int	10h
	pop	cx
	ret
ELSE
	push	cx
	mov	dh,[byRows]
	call 	LocalSetCursorPosition
	sub	di,di
	mov	ax,[wColumns]
	shl	ax,2
	mul	[wCharHeight]
	push	ax			; Number of words/character row
	shl	ax,1
	mov	si,ax
	mov	cl,[byRows]
	sub	ch,ch
	mul	cx
	mov	cx,ax
	shr	cx,1
	push	ds
	push	es
	pop	ds
	rep movsw
	pop	ds
	pop	cx			; Retreive word/character row
	sub	ax,ax
	rep stosw
	pop	cx
	ret
ENDIF
;
WriteTTY256_70:
	cmp	al,7			; Bell?
	je	WriteTTY256_90		;  Yes: go handle bell
	cmp	al,8			; Backspace?
	jne	WriteTTY256_10		;  No: go do regular character
;
; Backspace
;
	test	dl,dl			; Beginning of row?
	jz	WriteTTY256_80		;  Y: Skip movement
	dec	dx			;  N: Go back one column
	call	LocalSetCursorPosition
WriteTTY256_80:
	ret
;
; Bell
;
WriteTTY256_90:
	call	ShortBeep
	ret
WriteTTY256	endp
;
IF (EXTENDED_BIOS and EXTENDED_TTY)
;
;	WriteTTY16BPP - Write character as TTY in 16 bit-color graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write to TTY device
;		BL = Color to write
;	Exit:	None
;
	public	WriteTTY16BPP
WriteTTY16BPP	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegA000			; Get video segment
	mov	dx,[wCursorPosition]
	sub	bh,bh
	cmp	al,13
	jbe	WriteTTY16BPP_30
;
; Entry for "normal" characters (non-control character below a carriage return)
;
WriteTTY16BPP_10:
	push	dx
	call	Write16BPPChar
	pop	dx
	inc	dl
	cmp	dl,byte ptr [wColumns]
	jae     WriteTTY16BPP_20
	call	LocalSetCursorPosition
	ret
;
WriteTTY16BPP_20:
	sub	dl,dl
	jmp	short WriteTTY16BPP_50
;
WriteTTY16BPP_30:
	jb	WriteTTY16BPP_40
;
; Carriage return
;
	sub	dl,dl
	call	LocalSetCursorPosition
	ret
WriteTTY16BPP_40:
	cmp	al,10			; Linefeed?
	ja	WriteTTY16BPP_10	;  Above: go write regular character
	jb	WriteTTY16BPP_70	;  Below: check for other control char
;
; Linefeed
;
WriteTTY16BPP_50:
	cmp	dh,[byRows]
	jae	WriteTTY16BPP_60
	inc	dh
	call	LocalSetCursorPosition
	ret
;
; Full screen scroll
;
WriteTTY16BPP_60:
	push	cx
	mov	dh,[byRows]
	sub	cx,cx
	call	LocalSetCursorPosition
	mov	dl,byte ptr [wColumns]
	dec	dx
	mov	ax,0601h
	int	10h
	pop	cx
	ret
;
WriteTTY16BPP_70:
	cmp	al,7			; Bell?
	je	WriteTTY16BPP_90	;  Yes: go handle bell
	cmp	al,8			; Backspace?
	jne	WriteTTY16BPP_10	;  No: go do regular character
;
; Backspace
;
	test	dl,dl			; Beginning of row?
	jz	WriteTTY16BPP_80	;  Y: Skip movement
	dec	dx			;  N: Go back one column
	call	LocalSetCursorPosition
WriteTTY16BPP_80:
	ret
;
; Bell
;
WriteTTY16BPP_90:
	call	ShortBeep
	ret
WriteTTY16BPP	endp
;
;	WriteTTY24BPP - Write character as TTY in 24 bit-color graphics mode
;
;	Entry:	DS = Seg0
;		AL = Character to write to TTY device
;		BL = Color to write
;	Exit:	None
;
	public	WriteTTY24BPP
WriteTTY24BPP	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegA000			; Get video segment
	mov	dx,[wCursorPosition]
	sub	bh,bh
	cmp	al,13
	jbe	WriteTTY24BPP_30
;
; Entry for "normal" characters (non-control character below a carriage return)
;
WriteTTY24BPP_10:
	push	dx
	call	Write24BPPChar
	pop	dx
	inc	dl
	cmp	dl,byte ptr [wColumns]
	jae     WriteTTY24BPP_20
	call	LocalSetCursorPosition
	ret
;
WriteTTY24BPP_20:
	sub	dl,dl
	jmp	short WriteTTY24BPP_50
;
WriteTTY24BPP_30:
	jb	WriteTTY24BPP_40
;
; Carriage return
;
	sub	dl,dl
	call	LocalSetCursorPosition
	ret
WriteTTY24BPP_40:
	cmp	al,10			; Linefeed?
	ja	WriteTTY24BPP_10	;  Above: go write regular character
	jb	WriteTTY24BPP_70	;  Below: check for other control char
;
; Linefeed
;
WriteTTY24BPP_50:
	cmp	dh,[byRows]
	jae	WriteTTY24BPP_60
	inc	dh
	call	LocalSetCursorPosition
	ret
;
; Full screen scroll
;
WriteTTY24BPP_60:
	push	cx
	mov	dh,[byRows]
	sub	cx,cx
	call	LocalSetCursorPosition
	mov	dl,byte ptr [wColumns]
	dec	dx
	mov	ax,0601h
	int	10h
	pop	cx
	ret
;
WriteTTY24BPP_70:
	cmp	al,7			; Bell?
	je	WriteTTY24BPP_90	;  Yes: go handle bell
	cmp	al,8			; Backspace?
	jne	WriteTTY24BPP_10	;  No: go do regular character
;
; Backspace
;
	test	dl,dl			; Beginning of row?
	jz	WriteTTY24BPP_80	;  Y: Skip movement
	dec	dx			;  N: Go back one column
	call	LocalSetCursorPosition
WriteTTY24BPP_80:
	ret
;
; Bell
;
WriteTTY24BPP_90:
	call	ShortBeep
	ret
WriteTTY24BPP	endp
;
ENDIF			; (EXTENDED_BIOS and EXTENDED_TTY)
;
;	WriteCGAChar - Write one character to CGA memory
;
;	Entry:	DS = Seg0
;		AL = Character to write
;		BL = Color of character (if bit 7 = 1: XOR char)
;		DL = Column
;		DH = Row
;		ES = 0B800h
;	Exit:	None
;
	public	WriteCGAChar
WriteCGAChar	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	dx
	push	ds
	push	ax
	mov	cx,[wCharHeight]
	mov	al,cl			; Start address = ( (wCharHeight/2)
	mul	byte ptr [wColumns]	;		* (wColumns*2)
	mov	bh,dl			;		* (current row) )
	shl	bh,1			;		+ (current column) * 2
	mov	dl,dh
	sub	dh,dh
	mul	dx
	add	al,bh
	adc	ah,0
	mov	di,ax			; ES:DI = Pointer to video memory
	mov	bh,[byMode]
	lds	si,[vecINT43]
	pop	ax			; Get character code back
	test	al,080h			; Character code in the upper range?
	jz	WriteCGAChar_05		;  N: Go handle normal
	and	al,07Fh			;  Y: Mask off high order bit
	call	GetDSSeg0		; DS = Seg0
	lds	si,[vecINT1F]		; Use upper half of CGA set
WriteCGAChar_05:
	mul	cl
	add	si,ax
	cmp	bh,6			; Hi-res CGA?
	je	WriteCGAChar_80		;  Yes: go handle this
	test	bl,80h
	jnz	WriteCGAChar_40
;
; MOVE Mode 4 & 5 character
;
	and	bl,3
WriteCGAChar_10:
	lodsb
	call	ExpandCGAByteToWord
	stosw
	dec	di
	dec	di
	test	di,2000h
	jnz	WriteCGAChar_20
	add	di,2000h
	jmp	short WriteCGAChar_30
WriteCGAChar_20:
	sub	di,2000h-80
WriteCGAChar_30:
	loop	WriteCGAChar_10
	jmp	short WriteCGAChar_exit
;
; XOR Mode 4 & 5 character
;
WriteCGAChar_40:
	and	bl,3
WriteCGAChar_50:
	lodsb
	call	ExpandCGAByteToWord
	xor	es:[di],ax
	test	di,2000h
	jnz	WriteCGAChar_60
	add	di,2000h
	jmp	short WriteCGAChar_70
WriteCGAChar_60:
	sub	di,2000h-80
WriteCGAChar_70:
	loop	WriteCGAChar_50
	jmp	short WriteCGAChar_exit
;
; MOVE Mode 6 character
;
WriteCGAChar_80:
	shr	di,1			; No longer 2-bytes/character
	test	bl,80h
	jnz	WriteCGAChar_120
WriteCGAChar_90:
	movsb
	dec	di
	test	di,2000h
	jnz	WriteCGAChar_100
	add	di,2000h
	jmp	short WriteCGAChar_110
WriteCGAChar_100:
	sub	di,2000h-80
WriteCGAChar_110:
	loop	WriteCGAChar_90
	jmp	short WriteCGAChar_exit
;
; XOR Mode 6 character
;
WriteCGAChar_120:
	lodsb
	xor	es:[di],al
	test	di,2000h
	jnz	WriteCGAChar_130
	add	di,2000h
	jmp	short WriteCGAChar_140
WriteCGAChar_130:
	sub	di,2000h-80
WriteCGAChar_140:
	loop	WriteCGAChar_120
;
WriteCGAChar_exit:
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	ret
WriteCGAChar	endp
;
;	ExpandCGAByteToWord - Expand a font byte to CGA color word for CGA lo-res mode
;
;	Entry:	AL = Font byte
;		BL = Color
;	Exit:	AX = Word to write to memory
;
	public	ExpandCGAByteToWord
ExpandCGAByteToWord	proc
	assume	ds:nothing, es:nothing
	push	cx
	push	dx
	sub	dx,dx
	mov	cx,8
ExpandCGAByteToWord_10:
	rol	dx,1
	rol	dx,1
	rcl	al,1
	jnc	ExpandCGAByteToWord_20
	or	dl,bl
ExpandCGAByteToWord_20:
	loop	ExpandCGAByteToWord_10
	mov	ax,dx
	xchg	al,ah
	pop	dx
	pop	cx
	ret
ExpandCGAByteToWord	endp
;
;	WritePlanarChar - Write one planar character
;
;	Entry:	DS = Seg0
;		ES = A000h
;		AL = Character to write
;		BL = Color of character (if bit 7 = 1: XOR char)
;		BH = Page
;		DL = Column
;		DH = Row
;	Exit:	None
;
;	AX, SI, DI, ES are trashed
;
WritePlanarChar_10:
	assume	ds:Seg0, es:nothing
	add	di,[wRegenLength]
	dec	bh
	jnz	WritePlanarChar_10
	jmp	short WritePlanarChar_20
;
	public	WritePlanarChar
WritePlanarChar	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	dx
	push	bp
	push	ds
;
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	call	GetRowOffset		; BP = Row offset
ELSE
	mov	bp,[wColumns]		; Offset to next row
ENDIF
	push	ax
	sub	di,di
	test	bh,bh			; Page 0?
	jnz	WritePlanarChar_10	;  N: Go handle "non-standard" calculations
WritePlanarChar_20	label	near
;
; Starting position = (current row) * wColumns * wCharHeight + (current column)
;
	mov	cx,[wCharHeight]
	mov	al,dh
	sub	dh,dh
	add	di,dx			; DI = current column + page offset
	mul	cl			; AX = wCharHeight * (current row)
	mul	bp			; DX:AX = wColumns * AX
	add	di,ax			; ES:DI = Start position
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	adc	dx,0			; DX:DI = DX:AX + (current column + page offset)
	call	OEMSetBank
ENDIF
	pop	ax
	mul	cl
	lds	si,[vecINT43]
	add	si,ax			; DS:SI = Font pointer
;
; Save registers that will be affected and set to working values
;
	mov	dx,SEQ_INDEX
	mov	al,2
	call	GetIndexRegister
	mov	ah,0Fh			; Map mask = all planes enabled
	out	dx,ax
	mov	dl,GDC_INDEX and 0FFh
	sub	al,al
	call	GetIndexRegister
	sub	ah,ah			; Set/reset = 0000 binary
	out	dx,ax
	inc	ax
	call	GetIndexRegister
	mov	ah,bl			; Enable set/reset = !color
	not	ah
	out	dx,ax
	mov	al,3
	call	GetIndexRegister
	mov	ah,18h
	test	bl,80h
	jnz	WritePlanarChar_30
	sub	ah,ah			; Data rotate = 0
WritePlanarChar_30:
	out	dx,ax
	mov	al,8
	call	GetIndexRegister
	mov	ah,0ffh			; Bit mask = 11111111 binary
	out	dx,ax
;
WritePlanarChar_40:
	lodsb
	xchg	es:[di],al
	add	di,bp			; Next scan line
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jnc	WritePlanarChar_50
	call	OEMIncBank
WritePlanarChar_50:
ENDIF
	loop	WritePlanarChar_40
;
	mov	dx,GDC_INDEX
;
	sub	ax,ax
	out	dx,ax
	inc	ax
	out	dx,ax
	mov	al,03h
	out	dx,ax
	mov	ax,0FF08h
	out	dx,ax
;
	pop	ds
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	ret
WritePlanarChar	endp
;
;	Write one character in 256 color mode
;
;	Entry:	DS = Seg0
;		ES = A000h
;		AL = Character to write
;		BL = Color to write
;		DL = Column
;		DH = Row
;	Exit:	None
;
	public	Write256Char
Write256Char	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	dx
	push	bp
	push	ds
;
;	NOTE:	There are no 256 color modes that use a different row offset
;		than the horizontal screen resolution. The test program,
;		DMU, sets CRTC[13] and then changes the BIOS variable
;		"wColumns" to match it.
;
;IF (EXTENDED_BIOS and EXTENDED_TTY)
;	call	GetRowOffset		; BP = Row offset
;ELSE
	mov	bp,[wColumns]
	shl	bp,3			; Offset to next row = wColumns * 3
;ENDIF
	push	ax			; Save character code
;
; Starting position =
;	((current row) * wColumns * wCharHeight + (current column)) * 8
;
	mov	cx,[wCharHeight]
	mov	al,dh			; AL = Row
	sub	dh,dh
	mov	si,dx
	shl	si,3			; SI = (current column) * 8
	mul	cl			; AX = wCharHeight * (current row)
	mul	bp			; DX:AX = wColumns * 8 * AX
	add	ax,si
IF (EXTENDED_BIOS and EXTENDED_TTY)
	adc	dx,0			; DX:AX = DX:AX + (current column)
	call	OEMSetBank
ENDIF
	mov	di,ax			; ES:DI = Starting position
	pop	ax			; Restore character code
	mul	cl
	lds	si,[vecINT43]
	add	si,ax			; DS:SI = Pointer to font
;
	sub	bp,8			; BP = Row offset after 1 char write
	mov	dx,cx			; DX = Number of scan lines
Write256Char_10:
	lodsb				; Get single scan of font glyph
	mov	ah,al
	mov	cx,8
Write256Char_20:
	sub	al,al
	rcl	ah,1
	jnc	Write256Char_30
	mov	al,bl
Write256Char_30:
	stosb
IF (EXTENDED_BIOS and EXTENDED_TTY)
	test	di,di			; Bank crossing?
	jnz	Write256Char_40		;  N: Skip setting new bank
	call	OEMIncBank		;  Y: Set next bank
Write256Char_40:
ENDIF
	loop	Write256Char_20
	add	di,bp
IF (EXTENDED_BIOS and EXTENDED_TTY)
	jnc	Write256Char_50		; Bank crossing? N: Skip setting bank
	call	OEMIncBank		;  Y: Set next bank
Write256Char_50:
ENDIF
	dec	dx
	jnz	Write256Char_10
;
	pop	ds
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	ret
Write256Char	endp
;
IF (EXTENDED_BIOS and EXTENDED_TTY)
;
;	Write one character in 16-BPP color mode
;
;	Entry:	DS = Seg0
;		ES = A000h
;		AL = Character to write
;		BL = Color to write
;		DL = Column
;		DH = Row
;	Exit:	None
;
	public	Write16BPPChar
Write16BPPChar	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	dx
	push	bp
	push	ds
;
	call	GetRowOffset		; BP = Row offset
	push	ax			; Save character code
;
; Starting position =
;	((current row) * wColumns * wCharHeight + (current column)) * BPP
;
	mov	cx,[wCharHeight]
	mov	al,dh			; DH holds row
	sub	dh,dh
	mov	si,dx
	shl	si,4			; SI = (current column) * 16

	mul	cl			; AX = wCharHeight * (current row)
	mul	bp			; DX:AX = wColumns * 16 * AX
	add	ax,si
	adc	dx,0			; DX:AX = DX:AX + (current column)
	mov	di,ax			; ES:DI = Starting position
	call	OEMSetBank
;
	pop	ax			; Restore character code
	mul	cl
	lds	si,[vecINT43]
	add	si,ax			; DS:SI = Pointer to font
;		   
	sub	bp,16			; BP = Row offset after 1 char write
	mov	bx,cx			; BX = Number of scan lines
Write16BPPChar_10:
	lodsb				; Get single scan of font glyph
	mov	ah,al			; AH = Font byte
	mov	cx,8			; Number of bits in the font byte
Write16BPPChar_20:
	cwd				; DX = 0000h or FFFFh based on AH.7
	mov	es:[di],dx
	inc	di
	inc	di
	rol	ah,1			; Next font bit in high order bit
	loop	Write16BPPChar_20
	add	di,bp
	jc	Write16BPPChar_100
	cmp	di,bp
	je	Write16BPPChar_100
Write16BPPChar_50:
	dec	bx
	jnz	Write16BPPChar_10
;
	pop	ds
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	ret
;
; For speed reasons, handle incrementing the bank here
;
Write16BPPChar_100:
	call	OEMIncBank
	jmp	short Write16BPPChar_50
Write16BPPChar	endp
;
;	Write one character in 24-BPP color mode
;
;	Entry:	DS = Seg0
;		ES = A000h
;		AL = Character to write
;		BL = Color to write
;		DL = Column
;		DH = Row
;	Exit:	None
;
	public	Write24BPPChar
Write24BPPChar	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	dx
	push	bp
	push	ds
; 
	push	ax			; Save character code
	call	GetRowOffset		; BP = Row offset
	mov	cx,[wCharHeight]
	mov	al,[byMode]
	call	GetBPP			; AL = 24 or 32
	movzx	bx,al			; BX = BPP
	mov	al,dh			; AL = Row
	sub	dh,dh
	mov	si,dx
	cmp	bl,24			; 24 BPP mode?
	je	short Write24BPPChar_05	;  Y: Go calculate X position
	shl	si,5			; SI = (current column) * 32
	jmp	short Write24BPPChar_07
Write24BPPChar_05:
	shl	si,1			; SI = (current column) * 2
	add	si,dx			; SI = (current column) * 3
	shl	si,3			; SI = (current column) * 24
Write24BPPChar_07:
;
	mul	cl   			; AX = wCharHeight * (current row)
	mul	bp			; DX:AX = Row offset * AX
	add	ax,si
	adc	dx,0			; DX:AX = DX:AX + (current column)
	mov	di,ax			; ES:DI = Starting position
	call	OEMSetBank
;
	pop	ax			; Restore character code
	mul	cl
	lds	si,[vecINT43]
	add	si,ax			; DS:SI = Pointer to font
;		   
	sub	bp,bx			; BP = Row offset after 1 char write
	mov	bh,cl			; BH = Number of scan lines
Write24BPPChar_10:
	lodsb				; Get single scan of font glyph
	mov	ah,al			; AH = Font byte
	mov	cx,8			; Number of bits in the font byte
Write24BPPChar_20:
	cwd				; DX = 0000h or FFFFh based on AH.7
	mov	es:[di],dx
	inc	di
	inc	di
	mov	es:[di],dl
	inc	di
	cmp	bl,32			; NC if 32 BPP, CF if 24 BPP
	cmc				; Now it's NC if 24 BPP, CF if 32 BPP
	adc	di,0			; Add 1 if 32 BPP to skip alpha byte
	rol	ah,1			; Next font bit in high order bit
	loop	Write24BPPChar_20
	add	di,bp
	jc	Write24BPPChar_100
	cmp	di,bp
	je	Write24BPPChar_100
Write24BPPChar_50:
	dec	bh
	jnz	Write24BPPChar_10
;
	pop	ds
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	ret
;
; For speed reasons, handle incrementing the bank here
;
Write24BPPChar_100:
	call	OEMIncBank
	jmp	short Write24BPPChar_50
Write24BPPChar	endp
;
ENDIF		; (EXTENDED_BIOS and EXTENDED_TTY)
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


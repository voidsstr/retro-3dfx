	page	57, 132
	title	Character Read Functions
	subttl	RTXT.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	RTXT.ASM - Character read functions
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	ReadCharacter		Read a character from video memory
;	ReadMonoTextChar	Read a character in monochrome text mode
;	ReadColorTextChar	Read a character in color text mode
;	ReadCGAChar		Read a character in CGA-style graphics mode
;	ShrinkCGAWordToByte	Shrink 2-bit/pixel mode to 1-bit/pixel
;	ReadPlanarChar		Read a character in planar graphics mode
;	Read256Char		Read a character in 256 color mode
;	Read64KChar		Read a character in 16 BPP mode
;	Read24Char		Read a character in 24 BPP mode
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/RTXT.ASM $
;	$Log: /devel/h5/BIOS/SRC/RTXT.ASM $
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
	%out	!	Assembling: RTXT.ASM - Extensions with TTY functions
ELSE
  IF (EXTENDED_BIOS and not EXTENDED_TTY)
	%out	!	Assembling: RTXT.ASM - Extensions without TTY functions
  ELSE
	%out	!	Assembling: RTXT.ASM
  ENDIF
ENDIF
;
.MODEL TINY
.CODE
.386
;
	extrn	GetIndexRegister:near
	extrn	GetModeDescPtr:near
	extrn	GetESSegB000:near
	extrn	GetESSegB800:near
	extrn	GetDSSegA000:near
	extrn	GetDSSegB800:near
IF (EXTENDED_BIOS and EXTENDED_TTY)
	extrn	OEMSetBank:near
	extrn	OEMIncBank:near
ENDIF
;
;	Function 08h:
;	ReadCharacter - Read a character from video memory
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		BH = Page
;	Exit:	AL = Character read
;		AH = Attribute read
;
	public	tblReadCharacter
tblReadCharacter	label	word
	dw	ReadColorTextChar
	dw	ReadMonoTextChar
	dw	ReadCGAChar
	dw	ReadPlanarChar
	dw	ReadPlanarChar
	dw	ReadPlanarChar
	dw	Read256Char
IF EXTENDED_BIOS
  IF EXTENDED_TTY and (not VESA_300)
	dw	Read64KChar
	dw	Read24Char
	dw	ReadCharDoNothing
  ELSE
	dw	ReadCharDoNothing
	dw	ReadCharDoNothing
	dw	ReadCharDoNothing
  ENDIF
ENDIF
LENGTH_READ_CHARACTER_TABLE	equ	($ - tblReadCharacter)
;
	public	ReadCharacter
ReadCharacter	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	dx
	push	es
;
	push	bx				; Save page
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	si,word ptr es:[bx].MODEDESC.mdType
	pop	bx				; Restore page
	jc	short ReadCharacter_exit	; Error: Mode not found
;
	and	si,00FFh
	shl	si,1
	cmp	si,LENGTH_READ_CHARACTER_TABLE
	jae	ReadCharacter_exit
	call	cs:[tblReadCharacter + si]
;
ReadCharacter_exit:
	pop	es
	pop	dx
	pop	bx
	pop	ds
	pop	si
	iret
ReadCharacter	endp
;
;	ReadMonoTextChar - Read a character in monochrome text mode
;
;	Entry:	DS = Seg0
;		BH = Page
;	Exit:	AL = Character read
;		AH = Attribute read
;
	public	ReadMonoTextChar
ReadMonoTextChar	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB000			; Get video segment
	jmp	short ReadTextChar
ReadMonoTextChar	endp
;
;	ReadColorTextChar - Read a character in color text mode
;
;	Entry:	DS = Seg0
;		BH = Page
;	Exit:	AL = Character read
;		AH = Attribute read
;
	public	ReadColorTextChar, ReadTextChar
ReadColorTextChar	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB800			; Get video segment
ReadTextChar	label	near
	mov	bl,bh
	sub	bh,bh
	mov	ax,[wRegenLength]
	imul	ax,bx
	mov	si,ax
	shl	bx,1
	mov	bx,[wCursorPosition + bx]
	mov	al,byte ptr [wColumns]
	mul	bh
	add	al,bl
	adc	ah,0
	shl	ax,1
	add	si,ax
	lods	word ptr es:[si]
;	mov	ax,es:[si]
	public	ReadCharDoNothing
ReadCharDoNothing	label	near	; Just a convenient "ret"
	ret
ReadColorTextChar	endp
;
;	ReadCGAChar - Read a character in CGA-style graphics mode
;
;	Entry:	DS = Seg0
;	Exit:	AL = Character read
;
	public	ReadCGAChar
ReadCGAChar	proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	bp
	push	di
;
; Allocate enough space to build character
;
	mov	cx,[wCharHeight]
	sub	sp,cx
	mov	bp,sp
	push	cx
;
	les	di,[vecINT43]		; ES:DI = Pointer to font table
	mov	dx,[wCursorPosition]
	mov	al,cl			; Start address = ( (wCharHeight/2)
	mul	byte ptr [wColumns]	;		* (wColumns*2)
	mov	bh,dl			;		* (current row) )
	shl	bh,1			;		+ (current column) * 2
	mov	dl,dh
	sub	dh,dh
	mul	dx
	add	al,bh
	adc	ah,0
	mov	si,ax
	mov	al,[byMode]
	assume	ds:nothing
	call	GetDSSegB800		; DS:SI = Pointer to video memory
;
; Build the character and place it in allocated memory
;
	push	cx			; Save wCharHeight
	push	di			; Save font address
	push	es
	mov	bx,ss
	mov	es,bx
	mov	di,bp
;
	cmp	al,6
	jne	ReadCGAChar_40
;
; Other than the memory interleave, mode 6 is easy - just copy...
;
	shr	si,1			; No longer 2-bytes/character
ReadCGAChar_10:
	movsb
	dec	si
	test	si,2000h
	jnz	ReadCGAChar_20
	add	si,2000h
	jmp	short ReadCGAChar_30
ReadCGAChar_20:
	sub	si,2000h-80
ReadCGAChar_30:
	loop	ReadCGAChar_10
	jmp	short ReadCGAChar_80
;
; Mode 4 & 5 stuff...
;
ReadCGAChar_40:
	lodsw
	xchg	al,ah				; Low byte = low address
	call	ShrinkCGAWordToByte
	stosb
	dec	si
	dec	si
	test	si,2000h
	jnz	ReadCGAChar_50
	add	si,2000h
	jmp	short ReadCGAChar_60
ReadCGAChar_50:
	sub	si,2000h-80
ReadCGAChar_60:
	loop	ReadCGAChar_40
;
ReadCGAChar_80:
	pop	ds			; Restore font address
	pop	si
	pop	cx			; Restore wCharHeight
;
; Now search font table for match
;
	sub	ax,ax
	mov	bx,cx
ReadCGAChar_90:
	mov	di,bp
	mov	cx,bx
	repz cmpsb
	jz	ReadCGAChar_found
	add	si,cx
	inc	al			; INC AL because must ignore overflow.
	jnz	ReadCGAChar_90
ReadCGAChar_found:
	cbw
;
; Unallocate build character space
;
	pop	bx
	add	sp,bx
;
	pop	di
	pop	bp
	pop	cx
	ret
ReadCGAChar	endp
;
;	ShrinkCGAWordToByte - Shrink 2-bit/pixel mode to 1-bit/pixel
;
;	Entry:	AX = Video data
;	Exit:	AL = Shrunk data
;
;	BX and DX are trashed.
;
tblShrink	label	byte
	db	0, 0, 0, 0,  0,  0,  0,   0
	db	1, 2, 4, 8, 16, 32, 64, 128
	db	1, 2, 4, 8, 16, 32, 64, 128
	db	1, 2, 4, 8, 16, 32, 64, 128
;
	public	ShrinkCGAWordToByte
ShrinkCGAWordToByte	proc
	push	di
;
	sub	bx,bx
	mov	dl,bl
	mov	di,offset cs:tblShrink
;
	rept 7
	mov	bl,al
	and	bl,3
	shl	bx,3
	add	dl,cs:[bx + di]
	shr	ax,2
;	shr	ax,1
;	shr	ax,1
	inc	di
	endm
	mov	bl,al
	and	bl,3
	shl	bx,3

	add	dl,cs:[bx + di]
	mov	al,dl
;
	pop	di
	ret
ShrinkCGAWordToByte	endp
;
;	ReadPlanarChar - Read a character in planar graphics mode
;
;	Entry:	DS = Seg0
;		BH = Page
;	Exit:	AL = Character read
;
ReadPlanarChar_10:
	assume	ds:Seg0, es:nothing
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	shr	bx,1				; Restore org page
ReadPlanarChar_20:
	add	si,[wRegenLength]
	dec	bx
	jnz	ReadPlanarChar_20
	jmp	short ReadPlanarChar_30
;
	public	ReadPlanarChar
ReadPlanarChar	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	ReadPlanarChar_exit
ENDIF
	push	cx
	push	bp
	push	di
;
; First, calculate page offset
;
	sub	si,si
	or	bh,bh
	jnz	short ReadPlanarChar_10
	mov	dx,[wCursorPosition]
;
; Allocate enough space to build character
;
ReadPlanarChar_30	label	near
	mov	ax,[wCharHeight]
	mov	cx,ax
	inc	ax
	shr	ax,1
	shl	ax,1
	sub	sp,ax
	mov	bp,sp
	push	ax
;
	les	di,[vecINT43]		; ES:DI = Pointer to font table
	mov	al,dh
	mul	cl
	mov	bh,dl
	mul	[wColumns]
	add	al,bh
	adc	ah,0
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	adc	dx,0
ENDIF
	add	si,ax
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	adc	dx,0
	call	OEMSetBank
ENDIF
	mov	bx,[wColumns]
	assume	ds:nothing
	call	GetDSSegA000		; DS:SI = Pointer to video memory
;
; Setup color compare. Compare to "00", and the inverse of the font byte
;  (hopefully) will be read back.  This requires a background color of "0",
;  naturally.
;
	mov	dx,GDC_INDEX
	mov	al,02h
	call	GetIndexRegister
	push	ax
	sub	ah,ah
	out	dx,ax			; Color Compare Value = "0"
	mov	al,07h
	call	GetIndexRegister
	push	ax
	mov	ah,0Fh			; Color Don't Care = care about all
	out	dx,ax
	mov	al,05h
	call	GetIndexRegister
	push	ax
	or	ah,08h			; Read Mode 0 (color compare)
	out	dx,ax
;
; Copy character to allocated system memory.
;
	push	cx			; Save wCharHeight
	push	di			; Save font pointer
	push	es
	push	ss
	pop	es			; MOV ES,SS
	mov	di,bp
	dec	bx
ReadPlanarChar_40:
	lodsb
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	test	si,si
	jnz	ReadPlanarChar_45
	call	OEMIncBank
ReadPlanarChar_45:
ENDIF
	not	al
	stosb
	add	si,bx			; Next row down
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jnc	ReadPlanarChar_47
	call	OEMIncBank
ReadPlanarChar_47:
ENDIF
	loop	ReadPlanarChar_40
;
	pop	ds			; Restore font pointer
	pop	si
	pop	cx			; Restore wCharHeight
	mov	di,bp
;
; Restore GDC to before color compare.
;
	pop	ax			; Restore Read Mode
	out	dx,ax
	pop	ax			; Restore Color Don't Care
	out	dx,ax
	pop	ax			; Restore Color Compare Value
	out	dx,ax
;
; Now search font table for match
;
	sub	ax,ax
	mov	bx,cx
ReadPlanarChar_50:
	mov	di,bp
	mov	cx,bx
	repz cmpsb
	jz	ReadPlanarChar_found
	add	si,cx
	inc	al			; INC AL because must ignore overflow.
	jnz	ReadPlanarChar_50
ReadPlanarChar_found:
	cbw
;
; Unallocate build character space
;
	pop	bx
	add	sp,bx
;
	pop	di
	pop	bp
	pop	cx
ReadPlanarChar_exit:
	ret
ReadPlanarChar	endp
;
;	Read256Char - Read a character in 256 color mode
;
;	Entry:	DS = Seg0
;		BX = Color
;	Exit:	AL = Character read
;
	public	Read256Char
Read256Char	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short Read256Char_exit
ENDIF
	push	cx
	push	bp
	push	di
;
; Allocate enough space to build character
;
	mov	ax,[wCharHeight]
	mov	cx,ax			; CX = Character height
	inc	ax
	shr	ax,1			; Allocate an even number of bytes
	shl	ax,1			;  on the stack to build a copy
	sub	sp,ax			;  of the character's font image.
	mov	bp,sp
	push	ax
;
; Starting position = 
;	((current row) * wColumns * wCharHeight + (current column)) * 8
;
	mov	dx,[wCursorPosition]	; Current cursor position
	mov	al,dh			; AL = Row
	mov	di,[wColumns]
	shl	di,3			; Offset to next row = wColumns * 3
	sub	dh,dh
	mov	si,dx
	shl	si,3			; SI = (current column) * 8
	mul	cl			; AX = wCharHeight * (current row)
	mul	di			; DX:AX = wColumns * 8 * AX
	add	si,ax
IF (EXTENDED_BIOS and EXTENDED_TTY)
	adc	dx,0			; DX:AX = DX:AX + (current column)
	call	OEMSetBank
ENDIF
	mov	dx,di			; DX = Row offset
;
	les	di,[vecINT43]		; ES:DI = Pointer to font table
	call	GetDSSegA000		; DS:SI = Pointer to video memory
	assume	ds:nothing
;
; Copy the character to allocated system memory.
;
	push	cx			; Save wCharHeight
	push	di			; Save font pointer
	push	es
	mov	ax,ss
	mov	es,ax
	mov	di,bp			; Pointer to stack

	sub	dx,8			; Row offset minus char width
	mov	bx,cx			; Number of scan lines
Read256Char_10:
	mov	cx,8 
	sub	ax,ax			; Accumulate font bits in AL
Read256Char_20:
	cmp	byte ptr [si],0
	jz	@F
	or	al,80h
@@:
	rol	al,1
	inc	si
	loop	Read256Char_20
	stosb
	add	si,dx			; Next row down
IF (EXTENDED_BIOS and EXTENDED_TTY)
	jc	Read256Char_100
	cmp	si,dx			; Check if we are crossing a bank
	je	Read256Char_100
Read256Char_30:
ENDIF
	dec	bx
	jnz	Read256Char_10

	pop	ds			; Restore font pointer
	pop	si
	pop	cx			; Restore wCharHeight
	mov	di,bp
;
; Now search font table for match
;
	sub	ax,ax
	mov	bx,cx
Read256Char_40:
	mov	di,bp
	mov	cx,bx
	repz cmpsb
	jz	Read256Char_found
	add	si,cx
	inc	al			; INC AL because must ignore overflow.
	jnz	Read256Char_40
Read256Char_found:
	cbw
;
; Unallocate build character space
;
	pop	bx
	add	sp,bx
;
	pop	di
	pop	bp
	pop	cx
Read256Char_exit:
	ret
;
; Handle incrementing the bank here
;
IF (EXTENDED_BIOS and EXTENDED_TTY)
Read256Char_100:
	call	OEMIncBank
	jmp	short Read256Char_30
ENDIF
Read256Char	endp
;
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
;
;	Read64KChar - Read a character in 64K color mode
;
;	Entry:	DS = Seg0
;		DH = Row
;		DL = Column
;		BX = Color
;	Exit:	AL = Character read
;
	public	Read64KChar
Read64KChar	proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	bp
	push	di
;
; Allocate enough space to build character
;
	mov	ax,[wCharHeight]		;
	mov	cx,ax				; Save off wCharHeight for later use
	inc	ax
	shr	ax,1
	shl	ax,1
	sub	sp,ax
	mov	bp,sp
	push	ax
;
	les	di,[vecINT43]		; ES:DI = Pointer to font table
;
; Starting position = 
;	(current row) * wColumns * wCharHeight + (current column) * 16
Read64KChar_1:
	mov	al,dh			; Row
	mul	cl			; Row * Char Height (CX holds Char Height)
	mov	bl,dl			; Save Column

	mov	dx,[wColumns]		; wColumns * char height (8)
	shl	dx,4			; Offset to next row

	push	dx			; Save offset to next row
	mul	dx			; (CharHeight * Cur Row)AX * (wColumns)DX
	sub	bh,bh			; 
	shl	bx,4			; Current Column * 16
	add	ax,bx			; 
	adc	dx,0
	call	OEMSetBank
	pop	dx			; Restore offset to next row
;
	mov	si,ax
	call	GetDSSegA000		; DS:SI = Pointer to video memory
;
; Copy the character to allocated system memory.
;
	push	cx			; Save wCharHeight
	push	di			; Save font pointer
	push	es
	mov	ax,ss
	mov	es,ax
	mov	di,bp
	sub	dx,16			; Row offset minus char width
	mov	bx,cx			; Number of scan lines
Read64KChar_10:
	mov	cx,8 
	sub	al,al			; Accumulate font bits in AL
Read64KChar_20:
	cmp	word ptr [si],0
	jz	@F
	or	al,80h
@@:
	rol	al,1
	inc	si
	inc	si
	loop	Read64KChar_20
	stosb
	add	si,dx			; Next row down
	jc	Read64KChar_100
	cmp	si,dx			; Check if we are crossing a bank
	je	Read64KChar_100
Read64KChar_30:
	dec	bx
	jnz	Read64KChar_10
;
	pop	ds			; Restore font pointer
	pop	si
	pop	cx			; Restore wCharHeight
	mov	di,bp
;
; Now search font table for match
;
	sub	ax,ax
	mov	bx,cx
Read64KChar_40:
	mov	di,bp
	mov	cx,bx
	repz cmpsb
	jz	Read64KChar_found
	add	si,cx
	inc	al			; INC AL because must ignore overflow.
	jnz	Read64KChar_40
Read64KChar_found:
	cbw
;
; Unallocate build character space
;
	pop	bx			; Now contains AX
	add	sp,bx			; Put the stack back to normal
;
	pop	di
	pop	bp
	pop	cx
	ret
;
; Handle incrementing the bank here
;
Read64KChar_100:
	call	OEMIncBank
	jmp	short Read64KChar_30
Read64KChar	endp
;
;	Read24Char - Read a character in 24 color mode
;
;	Entry:	DS = Seg0
;		DH = Row
;		DL = Column
;		BX = Color
;	Exit:	AL = Character read
;
	public	Read24Char
Read24Char	proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	bp
	push	di
;
; Allocate enough space to build character
;
	mov	ax,[wCharHeight]		;
	mov	cx,ax				; Save off wCharHeight for later use
	inc	ax
	shr	ax,1
	shl	ax,1
	sub	sp,ax
	mov	bp,sp
	push	ax
;
	les	di,[vecINT43]		; ES:DI = Pointer to font table
;
Read24Char_1:
	mov	al,dh			; Row
	mul	cl			; Row * Char Height (CX holds Char Height)
	mov	bl,dl			; Save Column
;
	push	ax
	push	bx
	push	es
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	dx,es:[bx].MODEDESC.mdRowOff
	pop	es
	pop	bx
	pop	ax
;
	push	cx			; Save char height to use later
	push	dx			; Save offset to next row
	mul	dx			; (CharHeight * Cur Row)AX * (wColumns)DX
	sub	bh,bh			; 
	mov	cx,bx			; Save org value
	shl	bx,1			; Current Column * 24
	add	bx,cx
	shl	bx,3
	add	ax,bx			; 
	adc	dx,0
	call	OEMSetBank
	pop	dx			; Restore offset to next row
	pop	cx
;
	mov	si,ax
	call	GetDSSegA000		; DS:SI = Pointer to video memory
;
; Copy the character to allocated system memory.
;
	push	cx			; Save wCharHeight
	push	di			; Save font pointer
	push	es
	mov	ax,ss
	mov	es,ax
	mov	di,bp
	sub	dx,24			; Row offset minus char width
	mov	bx,cx			; Number of scan lines
Read24Char_10:
	mov	cx,8 
	sub	al,al			; Accumulate font bits in AL
Read24Char_20:
	cmp	byte ptr [si],0
	jz	@F
	or	al,80h
@@:
	rol	al,1
	inc	si
	inc	si
	inc	si
	loop	Read24Char_20
	stosb
	add	si,dx			; Next row down
	jc	Read24Char_100
	cmp	si,dx			; Check if we are crossing a bank
	je	Read24Char_100
Read24Char_30:
	dec	bx
	jnz	Read24Char_10
;
	pop	ds			; Restore font pointer
	pop	si
	pop	cx			; Restore wCharHeight
	mov	di,bp
;
; Now search font table for match
;
	sub	ax,ax
	mov	bx,cx
Read24Char_40:
	mov	di,bp
	mov	cx,bx
	repz cmpsb
	jz	Read24Char_found
	add	si,cx
	inc	al			; INC AL because must ignore overflow.
	jnz	Read24Char_40
Read24Char_found:
	cbw
;
; Unallocate build character space
;
	pop	bx			; Now contains AX
	add	sp,bx			; Put the stack back to normal
;
	pop	di
	pop	bp
	pop	cx
	ret
;
; Handle incrementing the bank here
;
Read24Char_100:
	call	OEMIncBank
	jmp	short Read24Char_30
Read24Char	endp
;
ENDIF
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


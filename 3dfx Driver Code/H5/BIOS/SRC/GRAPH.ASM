	page	57, 132
	title	Read and Write Pixel Routines
	subttl	GRAPH.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	GRAPH.ASM - Graphics related functions
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	ReadLightpen		Read lightpen position
;	WriteDot		Write a pixel
;	WriteDotCGA		Write a pixel in CGA mode
;	WriteDotPlanar		Write a pixel in EGA/VGA planar mode
;	WriteDot256		Write a pixel in VGA 256-color mode
;	WriteDotHiColor		Write a pixel in 16 BPP mode
;	WriteDotTrueColor	Write a pixel in 24 BPP mode
;	ReadDot			Read a pixel
;	ReadDotCGA		Read a pixel in CGA graphics mode
;	ReadDotPlanar		Read a pixel in EGA/VGA planar mode
;	ReadDot256		Read a pixel in VGA linear graphics mode
;	ReadDotHiColor		Read a pixel in 16 BPP mode
;	ReadDotTrueColor	Read a pixel in 24 BPP mode
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/GRAPH.ASM $
;	$Log: /devel/h5/BIOS/SRC/GRAPH.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
	%out	!	Assembling: GRAPH.ASM - Extensions with TTY functions
ELSE
  IF (EXTENDED_BIOS and not (EXTENDED_TTY and (not VESA_300)))
	%out	!	Assembling: GRAPH.ASM - Extensions without TTY functions
  ELSE
	%out	!	Assembling: GRAPH.ASM
  ENDIF
ENDIF
;
.MODEL TINY
.CODE
.386
;
	extrn	GetModeDescPtr:near
	extrn	GetESSegA000:near
	extrn	GetESSegB800:near
	extrn	GetDSSegA000:near
	extrn	GetDSSegB800:near
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
	extrn	OEMSetBank:near
ENDIF
;
;	Function 04h:
;	ReadLightpen - Read lightpen position
;
;	Entry:	SI, DS are pushed on stack
;		DS = Seg0
;	Exit:	AX = 0 (function not supported)
;
	public	ReadLightpen
ReadLightpen	proc
	assume	ds:nothing, es:nothing
	sub	ax,ax
	pop	ds
	pop	si
	iret
ReadLightpen	endp
;
;	Function 0Ch:
;	WriteDot - Write a pixel
;
;	Entry:	SI, DS are pushed on stack
;		DS = Seg0
;		AL = Pixel color
;		BH = Page
;		CX = Column
;		DX = Row
;	Exit:	None
;
;
	public	tblWriteDot
tblWriteDot	label	word
	dw	WriteDotDoNothing
	dw	WriteDotDoNothing
	dw	WriteDotCGA
	dw	WriteDotPlanar
	dw	WriteDotPlanar
	dw	WriteDotPlanar
	dw	WriteDot256
IF EXTENDED_BIOS
  IF EXTENDED_TTY and (not VESA_300)
	dw	WriteDotHiColor
	dw	WriteDotTrueColor
	dw	WriteDotDoNothing
  ELSE
	dw	WriteDotDoNothing
	dw	WriteDotDoNothing
	dw	WriteDotDoNothing
  ENDIF
ENDIF
LENGTH_TBLWRITEDOT		equ	($ - tblWriteDot) / 2

	public	WriteDot
WriteDot	proc
	assume	ds:Seg0, es:nothing
	pusha
	push	es
;
	push	ax					; Save pixel color
	push	bx					; Save page
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	si,word ptr es:[bx].MODEDESC.mdType	; DOESN'T AFFECT FLAGS
	pop	bx					; DOESN'T AFFECT FLAGS
	pop	ax					; DOESN'T AFFECT FLAGS
	jc	short WriteDot_exit			; Bad mode? Y: Exit
	and	si,00FFh
	shl	si,1
	call	cs:[tblWriteDot + si]
;
WriteDot_exit:
	pop	es
	popa
	pop	ds
	pop	si
	iret
WriteDot	endp
;
;	WriteDotCGA - Write a pixel in CGA mode
;
;	Entry:	DS = Seg0
;		AL = Pixel color (bit 7 = 1, XOR pixel)
;		BH = Page
;		CX = Column
;		DX = Row
;	Exit:	none
;
	public	WriteDotCGA
WriteDotCGA	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB800		; Get video segment
	sub	di,di
	mov	bl,al			; Save the color for later
	shr	dx,1
	jnc	WriteDotCGA_10
	mov	di,2000h		; Every other line is this offset
WriteDotCGA_10:
	mov	ax,80
	mul	dx
	add	di,ax			; DI = DX*80/2
	mov	ax,cx
	shr	ax,1
	shr	ax,1			; 4-pixels per byte in mode 4 & 5
	cmp	[byMode],006h
	jne	WriteDotCGA_40
;
; Handle mode 6 here
;
	shr	ax,1			; 8-pixels per byte in this mode
	add	di,ax
	and	cl,7
	test	bl,80h
	lahf
	and	bl,1
	ror	bl,1
	ror	bl,cl			; Shift pixel into position
	sahf
	jnz	WriteDotCGA_20
	mov	al,07Fh
	ror	al,cl			; Shift mask into position
	and	al,es:[di]		; Get video data
	or	al,bl			; Add color to proper position
	stosb				; ...and stick it back
	ret
;
; Handle XOR mode 6 here
;
WriteDotCGA_20:
	xor	es:[di],bl
	ret
;
; Handle mode 4 & 5 here
;
WriteDotCGA_40:
	add	di,ax
	and	cl,3
	shl	cl,1			; 2-bits per pixel
	test	bl,080h
	lahf
	and	bl,3
	ror	bl,2			; Prepare for rotate
	ror	bl,cl			; Shift pixel into position
	sahf
	jnz	WriteDotCGA_50
	mov	al,03Fh
	ror	al,cl			; Shift mask into position
	and	al,es:[di]		; Get video data
	or	al,bl			; Add color to proper position
	stosb				; ...and stick it back
	ret
;
; Handle XOR mode 4 & 5 here
;
WriteDotCGA_50:
	xor	es:[di],bl
;
	public	WriteDotDoNothing
WriteDotDoNothing	proc		; Just a convenient "ret"
	ret
WriteDotDoNothing	endp
WriteDotCGA	endp
;
;	WriteDotPlanar - Write a pixel in EGA/VGA planar mode
;
;	Entry:	DS = Seg0
;		AL = Pixel color (bit 7 = 1, XOR pixel)
;		BH = Page
;		CX = Column
;		DX = Row
;	Exit:	none
;
WriteDotPlanar_10:
	assume	ds:Seg0, es:nothing
	mov	al,bh			; Do multiply based on non-page 0
	cbw				;  graphics mode. This is done here
	mul	[wRegenLength]		;  to optimize for the "normal" case.
	add	di,ax
	jmp	short WriteDotPlanar_20
;
	public	WriteDotPlanar
WriteDotPlanar	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not (EXTENDED_TTY and (not VESA_300)))
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short WriteDotPlanar_exit
ENDIF
	call	GetESSegA000		; Get video segment
	mov	si,ax
	mov	ax,[wColumns]
	mul	dx
	mov	di,ax			; DI = row * columns
	test	bh,bh			; Page 0?
	jnz	WriteDotPlanar_10	;  N: Oh well, go do multiply
WriteDotPlanar_20	label	near
	mov	ax,cx
	shr	ax,3			; AX = X/8 for byte position
	add	di,ax			; ES:DI now contains proper address
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
	adc	dx,0			; Assume banked modes use only page 0
	call	OEMSetBank
ENDIF
;
; Setup pixel mask
;
	and	cl,7
	mov	ah,080h
	ror	ah,cl
	mov	al,8
	mov	dx,GDC_INDEX
	out	dx,ax			; Pixel mask set
;
	mov	ax,00F01h
	out	dx,ax			; Enable set/reset for all planes
	mov	ax,si			; Get back pixel color
	mov	ah,al			;  ...and place it in set/reset reg
	sub	al,al			;  (Note that the XOR flag is written
	out	dx,ax			;   into an unused bit)
	test	ah,080h			; XOR the pixel?
	mov	ax,01803h		;  Assume yes...
	jnz	WriteDotPlanar_30
	sub	ah,ah			;  Nope, just plane MOVE
WriteDotPlanar_30:
	out	dx,ax			; Write raster op
	mov	dl,low SEQ_INDEX
	mov	ax,00F02h		; Enable all planes for writing
	out	dx,ax
	mov	al,0FFh			; Finally, write the pixel. XCHG does
	xchg	al,es:[di]		;  read, then write for latch operation
;
	mov	dl,low GDC_INDEX
	sub	ax,ax
	out	dx,ax			; Clear set/reset to default value
	inc	ax
	out	dx,ax			; Clear set/reset enable to default value
	inc	ax
	inc	ax			; Reset to MOVE condition
	out	dx,ax
	mov	ax,0FF08h
	out	dx,ax			; Set bitmask to default value
WriteDotPlanar_exit:
	ret
WriteDotPlanar	endp
;
;	WriteDot256 - Write a pixel in VGA 256-color mode
;
;	Entry:	DS = Seg0
;		AL = Pixel color
;		CX = Column
;		DX = Row
;	Exit:	none
;
	public	WriteDot256
WriteDot256	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not (EXTENDED_TTY and (not VESA_300)))
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short WriteDot256_exit
ENDIF
	call	GetESSegA000		; Get video segment
	mov	bh,al			; Save pixel color for later
	mov	ax,[wColumns]
	shl	ax,3
	mul	dx
	add	ax,cx
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
	adc	dx,0
	call	OEMSetBank
ENDIF
	mov	di,ax
	mov	al,bh
	stosb
WriteDot256_exit:
	ret
WriteDot256	endp
;
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
;
;	WriteDotHiColor - Write a pixel in 16 BPP mode
;
;	Entry:	DS = Seg0
;		BX = Color
;		CX = Column
;		DX = Row
;	Exit:	None
;
	public	WriteDotHiColor
WriteDotHiColor	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegA000		; Get video segment
	mov	ax,[wColumns]
	shl	ax,4
	mul	dx
	shl	cx,1
	add	ax,cx
	adc	dx,0
	call	OEMSetBank
	mov	di,ax
	mov	es:[di],bx
	ret
WriteDotHiColor	endp
;
;	WriteDotTrueColor - Write a pixel in 24 BPP mode
;
;	Entry:	DS = Seg0
;		AL = Color (B)
;		BX = Color (R, G)
;		CX = Column
;		DX = Row
;	Exit:	None
;
	public	WriteDotTrueColor
WriteDotTrueColor	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	bx
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	ax,es:[bx].MODEDESC.mdRowOff	; Get row offset
	mul	dx
	mov	bx,cx
	shl	bx,1
	add	bx,cx				; BX = CX*3
	add	ax,bx
	adc	dx,0
	call	OEMSetBank
	call	GetESSegA000			; Get video segment
	mov	di,ax
	pop	bx
	pop	ax
	stosb
	mov	es:[di],bx
	ret
WriteDotTrueColor	endp
ENDIF
;
;	Function 0Dh:
;	ReadDot - Read a pixel
;
;	Entry:	SI, DS are pushed on stack
;		DS = Seg0
;		BH = Page
;		CX = Column
;		DX = Row
;	Exit:	AL = Color of dot read
;
	public	tblReadDot
tblReadDot	label	word
	dw	ReadDotDoNothing
	dw	ReadDotDoNothing
	dw	ReadDotCGA
	dw	ReadDotPlanar
	dw	ReadDotPlanar
	dw	ReadDotPlanar
	dw	ReadDot256
IF EXTENDED_BIOS
  IF EXTENDED_TTY and (not VESA_300)
	dw	ReadDotHiColor
	dw	ReadDotTrueColor
	dw	ReadDotDoNothing
  ELSE
	dw	ReadDotDoNothing
	dw	ReadDotDoNothing
	dw	ReadDotDoNothing
  ENDIF
ENDIF
LENGTH_TBLREADDOT	equ	($ - tblReadDot) / 2
;
	public	ReadDot
ReadDot		proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	dx
	push	es
;
	push	bx					; Save page
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	si,word ptr es:[bx].MODEDESC.mdType	; DOESN'T AFFECT FLAGS
	pop	bx					; DOESN'T AFFECT FLAGS
	jc	short ReadDot_exit			; Bad mode? Y: Go exit
	and	si,00FFh
	shl	si,1
	call	cs:[tblReadDot + si]
	mov	ah,0Dh					; For compatibility reasons
;
ReadDot_exit:
	pop	es
	pop	dx
	pop	cx
	pop	ds
	pop	si
	iret
ReadDot		endp
;
;	ReadDotCGA - Read a pixel in CGA graphics mode
;
;	Entry:	CX = Column
;		DX = Row
;	Exit:	AL = Color of dot read
;
	public	ReadDotCGA
ReadDotCGA	proc
	assume	ds:Seg0, es:nothing
	push	bx
	sub	si,si
	shr	dx,1
	jnc	ReadDotCGA_10
	mov	si,2000h		; Every other line is this offset
ReadDotCGA_10:
	mov	ax,80
	mul	dx
	add	si,ax			; SI = DX*80/2
	mov	ax,cx
	shr	ax,1
	shr	ax,1			; 4-pixels per byte in mode 4 & 5
	cmp	[byMode],006h		; Flags must remain untouched until...
	call	GetDSSegB800		;  (flags unaffected in here)
	assume	ds:nothing		;  ...almost there
	jne	ReadDotCGA_20		; HERE!
;
; Handle mode 6 here
;
	shr	ax,1			; 8-pixels per byte in this mode
	add	si,ax
	and	cl,7
	lodsb
	inc	cx
	rol	al,cl
	and	al,1
	pop	bx
	ret
;
; Handle mode 4 & 5 here
;
ReadDotCGA_20:
	add	si,ax
	and	cl,3
	shl	cl,1			; 2-bits per pixel
	lodsb
	inc	cx
	inc	cx
	rol	al,cl
	and	al,3
	pop	bx
;
	public	ReadDotDoNothing
ReadDotDoNothing	proc		; Just a convenient "ret"
	ret
ReadDotDoNothing	endp
ReadDotCGA	endp
;
;	ReadDotPlanar - Read a pixel in EGA/VGA planar mode
;
;	Entry:	BH = Page
;		CX = Column
;		DX = Row
;		DS = Seg0
;	Exit:	AL = Color of dot read
;
ReadDotPlanar_10:
	assume	ds:Seg0, es:nothing
	mov	al,bh			; Do multiply based on non-page 0
	cbw				;  graphics mode. This is done here
	mul	[wRegenLength]		;  to optimize for the "normal" case.
	add	si,ax
	jmp	short ReadDotPlanar_20
;
	public	ReadDotPlanar
ReadDotPlanar	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not (EXTENDED_TTY and (not VESA_300)))
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short ReadDotPlanar_exit
ENDIF
	push	bx
	mov	ax,[wColumns]
	mul	dx
	mov	si,ax			; SI = row * columns
	test	bh,bh			; Page 0?
	jnz	ReadDotPlanar_10	;  N: Oh well, go do multiply
ReadDotPlanar_20	label	near
	mov	ax,cx
	shr	ax,3			; AX = X/8 for byte position
	add	si,ax			; SI now contains proper offset
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
	adc	dx,0			; Assume banked modes use only page 0
	call	OEMSetBank
ENDIF
	and	cl,7			; CL = bit position (left justified)
	mov	bh,080h
	shr	bh,cl			; BH = bit mask
	call	GetDSSegA000		; DS:SI now contains proper address
	assume	ds:nothing
	mov	dx,GDC_INDEX
	mov	ax,0004h		; Read plane select (start at 0)
	sub	bl,bl			; BL = bit accumulator
;
	out	dx,ax			; Read plane 0
	mov	bl,[si]
	inc	ah
	and	bl,bh
	ror	bl,1
	rept	2			; Read plane 1 & 2
	out	dx,ax
	mov	ch,[si]
	inc	ah
	and	ch,bh
	or	bl,ch
	ror	bl,1
	endm
	out	dx,ax			; Read plane 3
	mov	ch,[si]
	inc	ah
	and	ch,bh
	or	bl,ch
;
	sub	cl,4
	rol	bl,cl
	mov	al,bl
	pop	bx
ReadDotPlanar_exit:
	ret
ReadDotPlanar	endp
;
;	ReadDot256 - Read a pixel in VGA linear graphics mode
;
;	Entry:	CX = Column
;		DX = Row
;		DS = Seg0
;	Exit:	AL = Color of dot read
;
	public	ReadDot256
ReadDot256	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not (EXTENDED_TTY and (not VESA_300)))
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	short ReadDot256_exit
ENDIF
	mov	ax,[wColumns]
	shl	ax,3
	mul	dx
	add	ax,cx
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
	adc	dx,0
	call	OEMSetBank
ENDIF
	call	GetDSSegA000			; Get video segment
	mov	si,ax
	lodsb
ReadDot256_exit:
	ret
ReadDot256	endp
;
IF (EXTENDED_BIOS and (EXTENDED_TTY and (not VESA_300)))
;
;	ReadDotHiColor - Read a pixel in 16 BPP mode
;
;	Entry:	CX = Column
;		DX = Row
;		DS = Seg0
;	Exit:	BX = Color of dot read
;
	public	ReadDotHiColor
ReadDotHiColor	proc
	assume	ds:Seg0, es:nothing
	push	ax
	mov	ax,[wColumns]
	shl	ax,4
	mul	dx
	shl	cx,1
	add	ax,cx
	adc	dx,0
	call	OEMSetBank
	call	GetDSSegA000			; Get video segment
	mov	si,ax
	mov	bx,[si]
	pop	ax
	ret
ReadDotHiColor	endp
;
;	ReadDotTrueColor - Read a pixel in 24 BPP mode
;
;	Entry:	CX = Column
;		DX = Row
;		DS = Seg0
;	Exit:	AL:BX = Color of dot read
;
	public	ReadDotTrueColor
ReadDotTrueColor	proc
	assume	ds:Seg0, es:nothing
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	ax,es:[bx].MODEDESC.mdRowOff	; Get row offset
	mul	dx
	mov	bx,cx
	shl	bx,1
	add	bx,cx				; BX = CX*3
	add	ax,bx
	adc	dx,0
	call	OEMSetBank
	call	GetDSSegA000			; Get video segment
	mov	si,ax
	lodsb
	mov	bx,[si]
	ret
ReadDotTrueColor	endp
;
ENDIF
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


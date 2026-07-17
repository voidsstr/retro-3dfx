	page	57, 132
	title	Scroll Up and Scroll Down Routines
	subttl	SCROLL.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	SCROLL.ASM - Scroll Up and Scroll Down Routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	ScrollUp		Scroll up a rectangular region
;	BlankEntireTextWindow	Blank the entire text window
;	ScrollUpMonoText	Scroll up a monochrome text window
;	ScrollUpColorText	Scroll up a color text window
;	BlankEntireCGAWindow	Blank a CGA window
;	ScrollUpCGA		Scroll up a CGA window
;	BlankEntirePlanarWindow	Blank the entire planar window
;	ScrollUpPlanar		Scroll up a window in VGA planar modes
;	BlankEntirePackedWindow	Blank the entire packed pixel window
;	ScrollUpPacked		Scroll up a window in VGA packed pixel modes
;	ScrollDown		Scroll down a rectangular region
;	ScrollDownMonoText	Scroll down a monochrome text window
;	ScrollDownColorText	Scroll down a color text window
;	ScrollDownCGA		Scroll down a CGA window
;	ScrollDownPlanar	Scroll down a window in planar mode
;	ScrollDownPacked	Scroll down a window in packed pixel mode
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/SCROLL.ASM $
;	$Log: /devel/h5/BIOS/SRC/SCROLL.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
SCROLLFRAME	struc
  sfReserved	dw	?
  sfRowOffset	dw	?
  sfMoveCount	dw	?
  sfBPP		db	?
  sfBankWrite	dw	?
  sfBankRead	dw	?
  sfFiller	db	3 dup (?)	; Make it DWORD aligned
SCROLLFRAME	ends
;
;
IF (EXTENDED_BIOS and EXTENDED_TTY)
	%out	!	Assembling: SCROLL.ASM - Extensions with TTY functions
ELSE
  IF (EXTENDED_BIOS and not EXTENDED_TTY)
	%out	!	Assembling: SCROLL.ASM - Extensions without TTY functions
  ELSE
	%out	!	Assembling: SCROLL.ASM
  ENDIF
ENDIF
;
.MODEL TINY
.CODE
.386
;
	extrn	GetIndexRegister:near
	extrn	GetModeDescPtr:near
	extrn	GetESSegA000:near
	extrn	GetESSegB000:near
	extrn	GetESSegB800:near
IF (EXTENDED_BIOS and EXTENDED_TTY)
	extrn	OEMSetWBank:near
	extrn	OEMSetRBank:near
	extrn	OEMIncWBank:near
	extrn	OEMIncRBank:near
	extrn	OEMSetBank:near
	extrn	OEMIncBank:near
	extrn	OEMDecWBank:near
	extrn	OEMDecRBank:near
	extrn	OEMGetWBank:near
ENDIF
;
;	Function 06h:
;	ScrollUp - Scroll up a rectangular region
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
;
	public	tblScrollUp
tblScrollUp	label	word
	dw	ScrollUpColorText
	dw	ScrollUpMonoText
	dw	ScrollUpCGA
	dw	ScrollUpPlanar
	dw	ScrollUpPlanar
	dw	ScrollUpPlanar
	dw	ScrollUpPacked
IF EXTENDED_BIOS
  IF EXTENDED_TTY
	dw	ScrollUpPacked
	dw	ScrollUpPacked
	dw	ScrollDoNothing
  ELSE
	dw	ScrollDoNothing
	dw	ScrollDoNothing
	dw	ScrollDoNothing
  ENDIF
ENDIF
LENGTH_SCROLL_UP_TABLE	equ	($ - tblScrollUp)

	public	ScrollUp
ScrollUp	proc
	assume	ds:Seg0, es:nothing
	pusha
	push	es
;
	push	ax				; Save number of lines
	push	bx				; Save blank attribute
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	si,word ptr es:[bx].MODEDESC.mdType
ScrollUp_20:
	pop	bx				; DOES NOT AFFECT FLAGS
	pop	ax				; DOES NOT AFFECT FLAGS
	jc	short ScrollUp_exit		; Bad mode, go to exit
;
	and	si,00FFh
	shl	si,1
	call	cs:[tblScrollUp + si]
;
ScrollUp_exit:
	pop	es
	popa
	pop	ds
	pop	si
	iret
ScrollUp	endp
;
;	BlankEntireTextWindow - Blank the entire text window
;
;	Entry:	DS = Seg0
;		ES = Video segment
;		BH = Attribute used to blank window
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	BlankEntireTextWindow
BlankEntireTextWindow	proc
	assume	ds:Seg0, es:nothing
;
; Make:	ES:DI window start
;	CX = number of columns
;	DX = number of rows
;	BP = offset to next row
;	AX = fill variable
;
	mov	al,byte ptr [wColumns]
	mul	ch
	add	al,cl
	adc	ah,0
	shl	ax,1
	add	ax,[wRegenStart]
	mov	di,ax
	sub	dl,cl
	sub	dh,ch
	mov	cl,dl
	sub	ch,ch
	inc	cx
	mov	dl,dh
	sub	dh,dh
	inc	dx
	mov	bp,[wColumns]
	shl	bp,1
	mov	al,' '
	mov	ah,bh
;	mov	ax,bx				; Move Fill attribute into AX
;
; Fill window
;
	mov	bx,cx
BlankEntireTextWindow_10:
	mov	cx,bx
	push	di
	rep stosw
	pop	di
	add	di,bp
	dec	dx
	jnz	BlankEntireTextWindow_10
;
	public	ScrollDoNothing
ScrollDoNothing	proc			; Just a convenient ret
	ret
ScrollDoNothing	endp
BlankEntireTextWindow	endp
;
;	ScrollUpMonoText - Scroll up a monochrome text window
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	ScrollUpMonoText
ScrollUpMonoText	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB000			; Get video segment
	jmp	short ScrollUpText
ScrollUpMonoText	endp
;
;	ScrollUpColorText - Scroll up a color text window
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	ScrollUpColorText, ScrollUpText, ScrollText
ScrollUpColorText	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB800			; Get video segment
ScrollUpText	label	near
	test	al,al
	jz	BlankEntireTextWindow
;
; Make:	ES:DI window start
;	DS:SI window 1st row source
;	CX = number of columns
;	DX = number of rows to move
;	BP = offset to next row
;	AX = fill variable
;	BX = number of rows to blank at end
;
	mov	bl,al
	mov	al,dh
	sub	al,ch
	cmp	bl,al				; Clear more lines than exist?
	ja	BlankEntireTextWindow		;  Yes: blank entire window
	mov	bp,[wColumns]
	mov	al,' '
	mov	ah,bh
	push	ax				; Save fill variable
	mov	ax,bp
	mul	ch
	add	al,cl
	adc	ah,0
	shl	ax,1
	add	ax,[wRegenStart]
	mov	di,ax				; ES:DI = destination
	shl	bp,1				; BP = offset to next row
	sub	bh,bh				; BX = number of rows to blank
	push	bx
	mov	si,di
ScrollUpText_10:
	add	si,bp
	dec	bx
	jnz	ScrollUpText_10
	pop	bx
	push	es
	pop	ds				; DS:SI = source
	sub	dl,cl
	sub	dh,ch
	mov	cl,dl
	sub	ch,ch
	inc	cx				; CX = number of columns
	mov	dl,dh
	sub	dh,dh
	inc	dx
	sub	dx,bx				; DX = number of rows
	pop	ax				; Retrieve fill variable
;
; Scroll text window
;
ScrollText	label	near
ScrollUpText_20:
	push	cx
	push	di
	push	si
	rep movsw
	pop	si
	pop	di
	pop	cx
	add	si,bp
	add	di,bp
	dec	dx
	jnz	ScrollUpText_20
;
; Blank final lines...
;
	mov	dx,cx
ScrollUpText_30:
	mov	cx,dx
	push	di
	rep stosw
	pop	di
	add	di,bp
	dec	bx
	jnz	ScrollUpText_30
;
	ret
ScrollUpColorText	endp
;
;	BlankEntireCGAWindow - Blank a CGA window
;
;	Entry:	DS = Seg0
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	BlankEntireCGAWindow
BlankEntireCGAWindow	proc
	assume	ds:Seg0, es:nothing
;
; Make:	ES:DI window start
;	CX = number of columns
;	DX = number of scan lines
;	BP = offset to next row
;	AX = fill variable
;
	mov	bp,[wColumns]
	mov	al,byte ptr [wColumns]
	mul	ch
	push	dx
	mul	[wCharHeight]
	shr	ax,1
	pop	dx
	add	al,cl
	adc	ah,0
	cmp	[byMode],006h
	je	BlankEntireCGAWindow_10
	shl	ax,1
	shl	bp,1
BlankEntireCGAWindow_10:
	mov	di,ax
	sub	dl,cl
	sub	dh,ch
	mov	cl,dl
	sub	ch,ch
	inc	cx
	cmp	[byMode],006h
	je	BlankEntireCGAWindow_20
	shl	cx,1
BlankEntireCGAWindow_20:
	mov	dl,dh
	sub	dh,dh
	inc	dx
	mov	al,byte ptr [wCharHeight]
	mul	dl
	mov	dx,ax
	shr	dx,1
	mov	al,bh
;
; Fill window
;
	push	dx
	push	di
	mov	bx,cx
BlankEntireCGAWindow_30:
	mov	cx,bx
	push	di
	rep stosb
	pop	di
	add	di,bp
	dec	dx
	jnz	BlankEntireCGAWindow_30
;
	pop	di
	pop	dx
	add	di,2000h
BlankEntireCGAWindow_40:
	mov	cx,bx
	push	di
	rep stosb
	pop	di
	add	di,bp
	dec	dx
	jnz	BlankEntireCGAWindow_40
;
	ret
BlankEntireCGAWindow	endp
;
;	ScrollUpCGA - Scroll up a CGA window
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	none
;
	public	ScrollUpCGA, ScrollCGA
ScrollUpCGA	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB800			; Get video segment
	test	al,al
	jz	BlankEntireCGAWindow
;
; Make:	ES:DI window start
;	DS:SI window 1st row source
;	CX = number of columns
;	DX = number of scan lines to move
;	BP = offset to next row
;	AX = fill variable
;	BX = number of rows to blank at end
;
	mov	bl,al
	mov	al,dh
	sub	al,ch
	cmp	bl,al				; Clear more lines than exist?
	ja	BlankEntireCGAWindow		;  Yes: blank entire window
	push	bx				; Save row count, attribute
	sub	dh,ch				; DH = number of rows - 1
	sub	dl,cl				; DL = number of columns - 1
	mov	al,ch				; AL = start row
	mov	bh,cl				; BH = start column
	mov	cx,dx				; CX = number of rows, columns
	mov	bp,[wColumns]			; BP = columns
	mov	si,[wCharHeight]
	shr	si,1				; SI = wCharHeight / 2
	sub	ah,ah
	mul	bp
	mul	si				; DI (start position) =
	add	al,bh				;  ((start row) * (wColumns)
	adc	ah,0				;  * (wCharHeight) / 2)
	mov	di,ax				;  + (start column)
	mov	al,ch				; # of scan lines to move =
	sub	ah,ah				;  (((end row)-(start row)) -
	sub	al,bl				;  (# of lines to scroll)) + 1
	inc	ax
	mul	si
	mov	dx,ax				; DX = number of scan lines to move
	sub	ch,ch
	inc	cx				; CX = number of bytes/row
	cmp	[byMode],006h			; Mode 6?
	je	ScrollUpCGA_10			;  Yes: skip fixups
	shl	di,1				;  No: fixup start offset,
	shl	cx,1				;      fixup bytes/row,
	shl	bp,1				;      fixup columns
ScrollUpCGA_10:
	pop	ax				; Retrieve row count
	push	ax				; Save fill attribute
	push	dx				; Save scan line count
	sub	ah,ah
	mul	si
	mov	bx,ax				; BX = scan lines to blank
	mul	bp
	mov	si,di
	add	si,ax				; SI = start source
	pop	dx				; Restore scan line count
	pop	ax				; Restore fill attribute
	mov	al,ah				;  ...and put it in AL
	push	es
	pop	ds				; Make DS = video segment
;
; Scroll CGA window
;
ScrollCGA	label	near
	push	dx
	push	di
	push	si
ScrollUpCGA_20:
	push	cx
	push	di
	push	si
	rep movsb
	pop	si
	pop	di
	pop	cx
	add	di,bp
	add	si,bp
	dec	dx
	jnz	ScrollUpCGA_20
	pop	si
	pop	di
	pop	dx
	add	di,2000h
	add	si,2000h
ScrollUpCGA_30:
	push	cx
	push	di
	push	si
	rep movsb
	pop	si
	pop	di
	pop	cx
	add	di,bp
	add	si,bp
	dec	dx
	jnz	ScrollUpCGA_30
;
; Blank CGA window
;
	push	bx
	push	di
ScrollUpCGA_40:
	push	cx
	push	di
	rep stosb
	pop	di
	pop	cx
	add	di,bp
	dec	bx
	jnz	ScrollUpCGA_40
	pop	di
	pop	bx
	sub	di,02000h
ScrollUpCGA_50:
	push	cx
	push	di
	rep stosb
	pop	di
	pop	cx
	add	di,bp
	dec	bx
	jnz	ScrollUpCGA_50
;
	ret
ScrollUpCGA	endp
;
;	BlankEntirePlanarWindow - Blank the entire planar window
;
;	Entry:	DS = Seg0
;		ES = Video segment
;		BH = Attribute used to blank window
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	BlankEntirePlanarWindow
BlankEntirePlanarWindow	proc
;
; Make:	ES:DI window start
;	CX = number of columns
;	DX = number of rows
;	BP = offset to next row
;	AX = fill variable
;
	mov	bp,[wColumns]
	sub	dh,ch
	sub	dl,cl
	mov	al,ch
	push	dx
	mul	byte ptr [wCharHeight]
	mul	bp
	add	ax,[wRegenStart]		; Add in current page start
	add	al,cl
	adc	ah,0
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	adc	dx,0
	call	OEMSetBank
ENDIF
	mov	di,ax				; ES:DI = start of window
	pop	cx
	mov	al,ch
	sub	ch,ch
	inc	cx				; CX = number of columns
	inc	al
	mul	byte ptr [wCharHeight]
;
; Blank window
;
	push	ax
	mov	dx,GDC_INDEX
	mov	al,5
	call	GetIndexRegister
	and	ah,0FCh
	or	ah,002h
	out	dx,ax				; Write mode 2
	pop	ax
;
	mov	dx,ax
	mov	al,bh
	mov	bx,cx
	sub	bp,cx
BlankEntirePlanarWindow_10:
	mov	cx,bx
;
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
BlankEntirePlanarWindow_20:
	stosb
	test	di,di
	jnz	BlankEntirePlanarWindow_30
	call	OEMIncWBank
BlankEntirePlanarWindow_30:
	loop	BlankEntirePlanarWindow_20
ELSE
	rep stosb
ENDIF
;
	add	di,bp
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jnc	BlankEntirePlanarWindow_40
	call	OEMIncWBank
BlankEntirePlanarWindow_40:
ENDIF
	dec	dx
	jnz	BlankEntirePlanarWindow_10
;
	mov	dx,GDC_INDEX
	mov	al,5
	call	GetIndexRegister
	and	ah,0FCh
	out	dx,ax				; Write mode 0
	ret
BlankEntirePlanarWindow	endp
;
;	ScrollUpPlanar - Scroll up a window in VGA planar modes
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	none
;
	public	ScrollUpPlanar, ScrollPlanar
ScrollUpPlanar	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	ScrollUpPlanar_exit
ENDIF
	call	GetESSegA000			; Get video segment
	test	al,al
	jz	BlankEntirePlanarWindow
;
; Make:	ES:DI window start
;	DS:SI window 1st row source
;	CX = number of columns
;	DX = number of scan lines to move
;	BP = offset to next row
;	AX = fill variable
;	BX = number of rows to blank at end
;
	mov	bl,al
	mov	al,dh
	sub	al,ch
	cmp	bl,al				; Clear more lines than exist?
	ja	BlankEntirePlanarWindow		;  Yes: blank entire window
;
	push	bx				; Save row count, attribute
	sub	dh,ch				; DH = number of rows - 1
	sub	dl,cl				; DL = number of columns - 1
	mov	al,ch				; AL = start row
	mov	bh,cl				; BH = start column
	mov	cx,dx				; CX = number of rows, columns
	mov	bp,[wColumns]			; BP = columns
	mov	si,[wCharHeight]		; SI = scan lines per character
	sub	ah,ah
	mul	bp
	mul	si				; DI (start position) =
	add	al,bh				;  ((start row) * (wColumns)
						;  * (wCharHeight))
	adc	ah,0				;  + (start column)
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	adc	dx,0
	call	OEMSetBank
ENDIF
	mov	di,ax
	mov	al,ch				; # of scan lines to move =
	sub	ah,ah				;  (((end row)-(start row)) -
	sub	al,bl				;  (# of lines to scroll)) + 1
	inc	ax
	mul	si
	mov	dx,ax				; DX = number of scan lines
	sub	ch,ch
	inc	cx				; CX = number of bytes/row
	pop	ax				; Retrieve row count
	push	ax				; Save fill attribute
	push	dx				; Save scan line count
	sub	ah,ah
	mul	si
	mov	bx,ax				; BX = scan lines to blank
	mul	bp
	mov	si,di
	add	si,ax				; SI = start source
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	pushf
	call	OEMGetWBank			; Read bank starts as being the same as write bank
	popf
	adc	dx,0
	call	OEMSetRBank
ENDIF
	pop	dx				; Restore scan line count
	pop	ax				; Restore fill attribute
	mov	al,ah				;  ...and put it in AL	
	push	es
	pop	ds				; Make DS = video segment
;
; Scroll planar window
;
ScrollPlanar	label	near
	push	ax
	push	dx
	mov	dx,GDC_INDEX
	mov	al,5
	call	GetIndexRegister
	and	ah,0FCh
	or	ah,1
	out	dx,ax				; Write mode 1
	pop	dx
;
	sub	bp,cx
ScrollUpPlanar_40:
	push	cx
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
ScrollUpPlanar_50:
	movsb
	test	di,di
	jnz	ScrollUpPlanar_60
	call	OEMIncWBank
ScrollUpPlanar_60:
	test	si,si
	jnz	ScrollUpPlanar_70
	call	OEMIncRBank
ScrollUpPlanar_70:
	loop	ScrollUpPlanar_50
ELSE
	rep movsb
ENDIF
	pop	cx
	add	di,bp
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jnc	ScrollUpPlanar_75
	call	OEMIncWBank
ScrollUpPlanar_75:
ENDIF
	add	si,bp
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jnc	ScrollUpPlanar_77
	call	OEMIncRBank
ScrollUpPlanar_77:
ENDIF
	dec	dx
	jnz	ScrollUpPlanar_40
;
	mov	dx,GDC_INDEX
	mov	al,5
	call	GetIndexRegister
	and	ah,0FCh
	or	ah,002h
	out	dx,ax				; Write mode 2
	pop	ax
;
; Blank planar window
;
ScrollUpPlanar_80:
	push	cx
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
ScrollUpPlanar_90:
	stosb
	test	di,di
	jnz	ScrollUpPlanar_100
	call	OEMIncWBank
ScrollUpPlanar_100:
	loop	ScrollUpPlanar_90
ELSE
	rep stosb
ENDIF
	pop	cx
	add	di,bp
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jnc	ScrollUpPlanar_110
	call	OEMIncWBank
ScrollUpPlanar_110:
ENDIF
	dec	bx
	jnz	ScrollUpPlanar_80
;
	mov	dx,GDC_INDEX
	mov	al,05h
	call	GetIndexRegister
	and	ah,0FCh
	out	dx,ax				; Write mode 0
ScrollUpPlanar_exit:
	ret
ScrollUpPlanar	endp
;
;	BlankEntirePackedWindow - Blank the entire packed pixel window
;
;	Entry:	DS = Seg0
;		ES = Video segment
;		BH = Attribute used to blank window
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;		BP = Pointer to stack frame
;	Exit:	None
;
;
; OPTIMIZATION HACK:
;
;	These are "jumps away" from the inner loop of the blank code to
;	handle the once every 64K occurance of a bank crossing. This
;	specifically jumps back into the inner loop after handling
;	the bank crossing.
;
BlankEntirePackedWindow_1000:
IF (EXTENDED_BIOS and EXTENDED_TTY)
	inc	[bp].SCROLLFRAME.sfBankWrite
	mov	dx,[bp].SCROLLFRAME.sfBankWrite
	call	OEMSetWBank
ENDIF
	jmp	short BlankEntirePackedWindow_50
BlankEntirePackedWindow_1010:
IF (EXTENDED_BIOS and EXTENDED_TTY)
	inc	[bp].SCROLLFRAME.sfBankWrite
	mov	dx,[bp].SCROLLFRAME.sfBankWrite
	call	OEMSetWBank
ENDIF
	jmp	short BlankEntirePackedWindow_60
;
	public	BlankEntirePackedWindow
BlankEntirePackedWindow	proc
;
; ENTRY POINT TO BLANK PACK PIXEL ROUTINE
;
; Make:	ES:DI window start
;	CX = number of columns
;	DX = number of rows
;	AX = fill variable
;
	sub	dh,ch
	sub	dl,cl
	mov	al,ch
	push	bx				; Save color
	push	dx				; Save deltas
	mov	bl,dl
	sub	bh,bh
	inc	bx
	shl	bx,3				; Assume 8 BPP
	mov	[bp].SCROLLFRAME.sfMoveCount,bx
	mul	byte ptr [wCharHeight]
	mul	[bp].SCROLLFRAME.sfRowOffset
	sub	ch,ch
	shl	cx,3				; Assume 8 BPP
	mov	di,cx
	cmp	[bp].SCROLLFRAME.sfBPP,15	; 15 or 16 BPP?
	jb	BlankEntirePackedWindow_10	;  Y: Assumption correct
	add	cx,di				;  N: Assume 24 BPP
	add	[bp].SCROLLFRAME.sfMoveCount,bx
	cmp	[bp].SCROLLFRAME.sfBPP,24	; 24 BPP?
	jne	BlankEntirePackedWindow_10	;  N: Assumption correct
	add	cx,di				;  Y: Set columns correctly
	add	[bp].SCROLLFRAME.sfMoveCount,bx
BlankEntirePackedWindow_10:
	add	ax,cx
	mov	di,ax				; ES:DI = start of window
	adc	dx,0
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMSetBank
	mov	[bp].SCROLLFRAME.sfBankWrite,dx
ENDIF
;
	pop	cx				; Retrieve deltas
	mov	al,ch
	inc	al
	mul	byte ptr [wCharHeight]
	mov	dx,ax
	pop	ax				; Retrieve color
	mov	al,ah
;
; Blank packed pixel window
;
	mov	cx,[bp].SCROLLFRAME.sfMoveCount
	sub	[bp].SCROLLFRAME.sfRowOffset,cx
	mov	bx,dx				; BX = Number of lines to blank
	shr	cx,1
BlankEntirePackedWindow_30:
	push	cx
BlankEntirePackedWindow_40:
	stosw
	test	di,di				; Are we crossing a bank?
	jz	BlankEntirePackedWindow_1000	;  Y: Go handle
BlankEntirePackedWindow_50	label	near
	loop	BlankEntirePackedWindow_40
	add	di,[bp].SCROLLFRAME.sfRowOffset	; Are we crossing a bank?
	jc	BlankEntirePackedWindow_1010	;  Y: Go handle
BlankEntirePackedWindow_60	label	near
	pop	cx
	dec	bx
	jnz	BlankEntirePackedWindow_30
;
	add	sp,size SCROLLFRAME
	pop	bp
	ret
BlankEntirePackedWindow	endp
;
;	ScrollUpPacked - Scroll up a window in VGA packed pixel modes
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	ScrollUpPacked
ScrollUpPacked	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	ScrollUpPacked_exit
ENDIF
	push	bp				; Create a stack frame for
	sub	sp,size SCROLLFRAME		;  local variables
	mov	bp,sp
;
	push	bx
	push	ax
 	mov 	al,[byMode]
	call	GetModeDescPtr
	mov	al,byte ptr es:[bx].MODEDESC.mdBPP	; Get bits per pixel (BPP)
	mov	[bp].SCROLLFRAME.sfBPP,al		;  ...and save it locally
	mov	ax,es:[bx].MODEDESC.mdRowOff		; Get row offset
	mov	[bp].SCROLLFRAME.sfRowOffset,ax		;  ...and save it locally
	pop	ax
	pop	bx
;
	call	GetESSegA000			; Get video segment
	test	al,al				; Blank the entire window?
	jnz	ScrollUpPacked_10		;  N: Skip forward
	jmp	BlankEntirePackedWindow		;  Y: Go handle that
ScrollUpPacked_10:
;
; Make:	ES:DI window start
;	DS:SI window 1st row source
;	CX = number of columns
;	DX = number of scan lines to move
;	AX = fill variable
;	BX = number of rows to blank at end
;
	mov	bl,al
	mov	al,dh
	sub	al,ch
	cmp	bl,al				; Clear more lines than exist?
	jnz	ScrollUpPacked_20		;  No:  Lets do some scrolling
	jmp	BlankEntirePackedWindow		;  Yes: blank entire window
ScrollUpPacked_20:
	push	bx				; Save row count, attribute
	sub	dh,ch				; DH = number of rows - 1
	sub	dl,cl				; DL = number of columns - 1
	mov	al,ch				; AL = start row
	mov	bl,cl				; BL = start column
	mov	cx,dx				; CX = number of rows, columns
;
; Calculate the starting position:
;
;	(Upper left row * row offset * character height) +
;	(Upper left column * BPP)
;
; Number of bytes to move:
;
;	(Lower right column - upper left column) * BPP
;
	push	cx
	sub	ch,ch
	inc	cx
	shl	cx,3				; Number of bytes to move (assume 8 BPP)
	mov	[bp].SCROLLFRAME.sfMoveCount,cx
	mov	si,[wCharHeight]		; SI = scan lines per character
	sub	ah,ah
	mul	si				; Start position = row * row offset
	mul	[bp].SCROLLFRAME.sfRowOffset	;   char height + column * BPP
	sub	bh,bh
	shl	bx,3				; Assume 8 bpp
	mov	di,bx
	cmp	[bp].SCROLLFRAME.sfBPP,15	; 15 or 16 BPP?
	jb	ScrollUpPacked_30		;  Y: Assumption correct
	add	bx,di				;  N: Assume 24 BPP
	add	[bp].SCROLLFRAME.sfMoveCount,cx
	cmp	[bp].SCROLLFRAME.sfBPP,24	; 24 BPP?
	jne	ScrollUpPacked_30		;  N: Assumption correct
	add	bx,di				;  Y: Set columns correctly
	add	[bp].SCROLLFRAME.sfMoveCount,cx	;     Set move count correctly
ScrollUpPacked_30:
	add	ax,bx
	mov	di,ax
	adc	dx,0
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMSetBank			; Set up the banks
	mov	[bp].SCROLLFRAME.sfBankWrite,dx	; Save off what bank we are in
	mov	[bp].SCROLLFRAME.sfBankRead,dx
ENDIF
	pop	cx
;
	pop	bx				; Get row count...
	push	bx				;  ...and save it again
	mov	al,ch				; # of scan lines to move =
	sub	ah,ah				;  (((end row)-(start row)) -
	sub	al,bl				;  (# of lines to scroll)) + 1
	inc	ax
	mul	si
	mov	dx,ax				; DX = number of scan lines
;
	pop	ax				; Retrieve row count
	push	ax				; Save fill attribute
	push	dx				; Save scan line count
	sub	ah,ah
	mul	si
	mov	bx,ax				; BX = scan lines to blank
	mul	[bp].SCROLLFRAME.sfRowOffset
	mov	si,di
	add	si,ax				; SI = start source
	adc	dx,0
IF (EXTENDED_BIOS and EXTENDED_TTY)
	add	dx,[bp].SCROLLFRAME.sfBankWrite	; Add in the write bank
	call	OEMSetRBank			; Set up the Read bank
	mov	[bp].SCROLLFRAME.sfBankRead,dx
ENDIF
;
	pop	dx				; Restore scan line count
	pop	ax				; Restore fill attribute
	mov	al,ah				;  ...and put it in AL
	push	es
	pop	ds				; Make DS = video segment
;
	mov	cx,[bp].SCROLLFRAME.sfMoveCount
	sub	[bp].SCROLLFRAME.sfRowOffset,cx	; "sfRowOffset" contains # of bytes to next scan
	push	bx
	mov	bx,dx				; BX = Number of lines to move
	shr	cx,1
ScrollUpPacked_50:
	push	cx
ScrollUpPacked_60:
	movsw
	test	di,di				; Are we crossing a bank?
	jz	ScrollUpPacked_1000		;  Y: Set next write bank
ScrollUpPacked_70:
	test	si,si				; Are we crossing a bank?
	jz	ScrollUpPacked_1010		;  Y: Set next read bank
ScrollUpPacked_80:
	loop	ScrollUpPacked_60
;
	add	di,[bp].SCROLLFRAME.sfRowOffset	; DI = Next line down
	jc	ScrollUpPacked_1020		; If bank crossed, go handle
ScrollUpPacked_90:
	add	si,[bp].SCROLLFRAME.sfRowOffset	; SI = Next line down
	jc	ScrollUpPacked_1030		; If bank crossed, go handle
ScrollUpPacked_100:
;
	pop	cx
	dec	bx
	jnz	ScrollUpPacked_50
	pop	bx
;
; Blank remaining portion of packed pixel window
;
ScrollUpPacked_110:
	push	cx
ScrollUpPacked_120:
	stosw
	test	di,di				; Are we crossing a bank?
	jz	ScrollUpPacked_1040		;  Y: Set next write bank
ScrollUpPacked_130:
	loop	ScrollUpPacked_120
	pop	cx
	add	di,[bp].SCROLLFRAME.sfRowOffset	; Next line down
	jc	ScrollUpPacked_1050		; If bank crossed, go handle
ScrollUpPacked_140:
	dec	bx
	jnz	ScrollUpPacked_110
;
	add	sp,size SCROLLFRAME
	pop	bp
ScrollUpPacked_exit:
	ret
;
; OPTIMIZATION HACK:
;
;	These are "jumps away" from the inner loop of the scroll code to
;	handle the once every 64K occurance of a bank crossing. This
;	specifically jumps back into the scroll loop after handling
;	the bank crossing.
;
ScrollUpPacked_1000:
IF (EXTENDED_BIOS and EXTENDED_TTY)
	inc	[bp].SCROLLFRAME.sfBankWrite
	mov	dx,[bp].SCROLLFRAME.sfBankWrite
	call	OEMSetWBank
ENDIF
	jmp	short ScrollUpPacked_70
ScrollUpPacked_1010:
IF (EXTENDED_BIOS and EXTENDED_TTY)
	inc	[bp].SCROLLFRAME.sfBankRead
	mov	dx,[bp].SCROLLFRAME.sfBankRead
	call	OEMSetRBank
ENDIF
	jmp	short ScrollUpPacked_80
ScrollUpPacked_1020:
IF (EXTENDED_BIOS and EXTENDED_TTY)
	inc	[bp].SCROLLFRAME.sfBankWrite
	mov	dx,[bp].SCROLLFRAME.sfBankWrite
	call	OEMSetWBank
ENDIF
	jmp	short ScrollUpPacked_90
ScrollUpPacked_1030:
IF (EXTENDED_BIOS and EXTENDED_TTY)
	inc	[bp].SCROLLFRAME.sfBankRead
	mov	dx,[bp].SCROLLFRAME.sfBankRead
	call	OEMSetRBank
ENDIF
	jmp	short ScrollUpPacked_100
ScrollUpPacked_1040:
IF (EXTENDED_BIOS and EXTENDED_TTY)
	inc	[bp].SCROLLFRAME.sfBankWrite
	mov	dx,[bp].SCROLLFRAME.sfBankWrite
	call	OEMSetWBank
ENDIF
	jmp	short ScrollUpPacked_130
ScrollUpPacked_1050:
IF (EXTENDED_BIOS and EXTENDED_TTY)
	inc	[bp].SCROLLFRAME.sfBankWrite
	mov	dx,[bp].SCROLLFRAME.sfBankWrite
	call	OEMSetWBank
ENDIF
	jmp	short ScrollUpPacked_140
ScrollUpPacked	endp
;
;	Function 07h:
;	ScrollDown - Scroll down a rectangular region
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	tblScrollDown
tblScrollDown	label	word
	dw	ScrollDownColorText
	dw	ScrollDownMonoText
	dw	ScrollDownCGA
	dw	ScrollDownPlanar
	dw	ScrollDownPlanar
	dw	ScrollDownPlanar
	dw	ScrollDownPacked
IF EXTENDED_BIOS
  IF EXTENDED_TTY
	dw	ScrollDownPacked
	dw	ScrollDownPacked
	dw	ScrollDoNothing
  ELSE
	dw	ScrollDoNothing
	dw	ScrollDoNothing
	dw	ScrollDoNothing
  ENDIF
ENDIF
LENGTH_SCROLL_DOWN_TABLE	equ	($ - tblScrollDown)
;
	public	ScrollDown
ScrollDown	proc
	assume	ds:Seg0, es:nothing
	pusha
	push	es
;
	push	ax				; Save number of lines
	push	bx				; Save fill attribute
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	si,word ptr es:[bx].MODEDESC.mdType
	pop	bx				; DOES NOT AFFECT THE FLAGS
	pop	ax				; DOES NOT AFFECT THE FLAGS
	jc	short ScrollDown_exit		; Bad Mode Y: go to exit
;
	and	si,00FFh
	shl	si,1
	call	cs:[tblScrollDown + si]
;
ScrollDown_exit:
	pop	es
	popa
	pop	ds
	pop	si
	iret
ScrollDown	endp
;
;	ScrollDownMonoText - Scroll down a monochrome text window
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column				     	
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	ScrollDownMonoText
ScrollDownMonoText	proc
	assume	ds:Seg0, es:nothing
	call	GetESSegB000			; Get video segment
	jmp	short ScrollDownText
ScrollDownMonoText	endp
;
;	ScrollDownColorText - Scroll down a color text window
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
ScrollDownBlankEntireTextWindow:
	jmp	BlankEntireTextWindow
;
	public	ScrollDownColorText, ScrollDownText
ScrollDownColorText	proc
	call	GetESSegB800			; Get video segment
ScrollDownText	label	near
	test	al,al
	jz	ScrollDownBlankEntireTextWindow
;
; Make:	ES:DI window start
;	DS:SI window 1st row source
;	CX = number of columns
;	DX = number of rows to move
;	BP = offset to next row
;	AX = fill variable
;	BX = number of rows to blank at end
;
	mov	bl,al
	mov	al,dh
	sub	al,ch
	cmp	bl,al				; Clear more lines than exist?
	jae	ScrollDownBlankEntireTextWindow	;  Y: Blank entire window
	mov	bp,[wColumns]
	mov	al,' '
	mov	ah,bh
	push	ax				; Save fill variable
	mov	ax,bp
	mul	dh
	add	al,cl
	adc	ah,0
	shl	ax,1
	add	ax,[wRegenStart]
	mov	di,ax				; ES:DI = destination
	shl	bp,1				; BP = offset to next row
	sub	bh,bh				; BX = number of rows to blank
	push	bx
	mov	si,di
ScrollDownText_10:
	sub	si,bp
	dec	bx
	jnz	ScrollDownText_10
	pop	bx
	push	es
	pop	ds				; DS:SI = source
	sub	dl,cl
	sub	dh,ch
	mov	cl,dl
	sub	ch,ch
	inc	cx				; CX = number of columns
	mov	dl,dh
	sub	dh,dh
	inc	dx
	sub	dx,bx				; DX = number of rows
	pop	ax				; Retrieve fill variable
	neg	bp
	jmp	ScrollText
ScrollDownColorText	endp
;
;	ScrollDownCGA - Scroll down a CGA window
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
ScrollDownBlankEntireCGAWindow:
	jmp	BlankEntireCGAWindow
;
	public	ScrollDownCGA
ScrollDownCGA	proc
	call	GetESSegB800			; Get video segment
	test	al,al
	jz	ScrollDownBlankEntireCGAWindow
;
; Make:	ES:DI window start
;	DS:SI window 1st row source
;	CX = number of columns
;	DX = number of scan lines to move
;	BP = offset to next row
;	AX = fill variable
;	BX = number of rows to blank at end
;
	mov	bl,al
	mov	al,dh
	sub	al,ch				; Clear more lines than exist?
	cmp	bl,al				;  Yes: blank entire window
	ja	ScrollDownBlankEntireCGAWindow
	push	bx				; Save row count, attribute
	mov	al,dh				; AL = start row
	sub	dh,ch				; DH = number of rows - 1
	sub	dl,cl				; DL = number of columns - 1
	mov	bh,cl				; BH = start column
	mov	cx,dx				; CX = number of rows, columns
	mov	bp,[wColumns]			; BP = columns
	mov	si,[wCharHeight]
	shr	si,1				; SI = wCharHeight / 2
	sub	ah,ah
	inc	ax
	mul	bp
	mul	si				; DI (start position) =
	add	al,bh				;  ((start row) * (wColumns)
	adc	ah,0				;  * (wCharHeight) / 2)
	mov	di,ax				;  + (start column)
	sub	di,bp
	mov	al,ch				; # of scan lines to move =
	sub	ah,ah				;  (((end row)-(start row)) -
	sub	al,bl				;  (# of lines to scroll)) + 1
	inc	ax
	mul	si
	mov	dx,ax				; DX = number of scan lines
	sub	ch,ch
	inc	cx				; CX = number of bytes/row
	cmp	[byMode],006h			; Mode 6?
	je	ScrollDownCGA_05		;  Yes: skip fixups
	shl	di,1				;  No: fixup start offset,
	shl	cx,1				;      fixup bytes/row,
	shl	bp,1				;      fixup columns
ScrollDownCGA_05:
	pop	ax				; Retrieve row count
	push	ax				; Save fill attribute
	push	dx				; Save scan line count
	sub	ah,ah
	mul	si
	mov	bx,ax				; BX = scan lines to blank
	mul	bp
	mov	si,di
	sub	si,ax				; SI = start source
	pop	dx				; Restore scan line count
	pop	ax				; Restore fill attribute
	mov	al,ah				;  ...and put it in AL
	push	es
	pop	ds				; Make DS = video segment
	neg	bp				; Make BP neg for reverse scroll
	jmp	ScrollCGA
	ret
ScrollDownCGA	endp
;
;	ScrollDownPlanar - Scroll down a window in planar mode
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
ScrollDownBlankEntirePlanarWindow:
	jmp	BlankEntirePlanarWindow
;
	public	ScrollDownPlanar
ScrollDownPlanar	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	ScrollDownPlanar_exit
ENDIF
	call	GetESSegA000			; Get video segment
	test	al,al
	jz	ScrollDownBlankEntirePlanarWindow
;
; Make:	ES:DI window start
;	DS:SI window 1st row source
;	CX = number of columns
;	DX = number of scan lines to move
;	BP = offset to next row
;	AX = fill variable
;	BX = number of rows to blank at end
;
	mov	bl,al
	mov	al,dh
	sub	al,ch				; Clear more lines than exist?
	cmp	bl,al				;  Yes: blank entire window
	ja	ScrollDownBlankEntirePlanarWindow
	push	bx				; Save row count, attribute
	mov	al,dh				; AL = start row
	sub	dh,ch				; DH = number of rows - 1
	sub	dl,cl				; DL = number of columns - 1
	mov	bh,cl				; BH = start column
	mov	cx,dx				; CX = number of rows, columns
	mov	bp,[wColumns]			; BP = columns
	mov	si,[wCharHeight]		; SI = scan lines per character
	sub	ah,ah
	inc	ax
	mul	bp
	mul	si				; DI (start position) =
	add	al,bh				;  ((start row + 1) * (wColumns)
						;  * (wCharHeight))
	adc	ah,0				;  + (start column) - wColumns
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	adc	dx,0
	call	OEMSetBank
ENDIF	
	mov	di,ax
	sub	di,bp
	mov	al,ch				; # of scan lines to move =
	sub	ah,ah				;  (((end row)-(start row)) -
	sub	al,bl				;  (# of lines to scroll)) + 1
	inc	ax
	mul	si
	mov	dx,ax				; DX = number of scan lines
	sub	ch,ch
	inc	cx				; CX = number of bytes/row
	pop	ax				; Retrieve row count
	push	ax				; Save fill attribute
	push	dx				; Save scan line count
	sub	ah,ah
	mul	si
	mov	bx,ax				; BX = scan lines to blank
	mul	bp
	mov	si,di
	sub	si,ax				; SI = start source
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	pushf
	call	OEMGetWBank			; Read bank starts as being the same as write bank
	popf
	sbb	dx,0
	call	OEMSetRBank
ENDIF	
	pop	dx				; Restore scan line count
	pop	ax				; Restore fill attribute
	mov	al,ah				;  ...and put it in AL
	push	es
	pop	ds				; Make DS = video segment
	neg	bp				; Make BP neg for reverse scroll
;
; Scroll planar window
;
	push	ax
	push	dx
	mov	dx,GDC_INDEX
	mov	al,5
	call	GetIndexRegister
	and	ah,0FCh
	or	ah,1
	out	dx,ax				; Write mode 1
	pop	dx
;
	sub	bp,cx
ScrollDownPlanar_40:
	push	cx
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
ScrollDownPlanar_50:
	movsb
	test	di,di
	jnz	ScrollDownPlanar_60
	call	OEMIncWBank
ScrollDownPlanar_60:
	test	si,si
	jnz	ScrollDownPlanar_70
	call	OEMIncRBank
ScrollDownPlanar_70:
	loop	ScrollDownPlanar_50
ELSE
	rep movsb
ENDIF
	pop	cx
;
	add	di,bp
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jc	ScrollDownPlanar_75
	call	OEMDecWBank
ScrollDownPlanar_75:
ENDIF
	add	si,bp
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jc	ScrollDownPlanar_77
	call	OEMDecRBank
ScrollDownPlanar_77:
ENDIF
	dec	dx
	jnz	ScrollDownPlanar_40
;
	mov	dx,GDC_INDEX
	mov	al,5
	call	GetIndexRegister
	and	ah,0FCh
	or	ah,002h
	out	dx,ax				; Write mode 2
	pop	ax
;
; Blank planar window
;
ScrollDownPlanar_80:
	push	cx
;
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
ScrollDownPlanar_90:
	stosb
	test	di,di
	jnz	ScrollDownPlanar_100
	call	OEMIncWBank
ScrollDownPlanar_100:
	loop	ScrollDownPlanar_90
ELSE
	rep stosb
ENDIF
	pop	cx
;
	add	di,bp
IF ((EXTENDED_BIOS and EXTENDED_TTY) and EXTENDED_PLANARPAGES)
	jc	ScrollDownPlanar_110
	call	OEMIncWBank
ScrollDownPlanar_110:
ENDIF
	dec	bx
	jnz	ScrollDownPlanar_80
;
	mov	dx,GDC_INDEX
	mov	al,05h
	call	GetIndexRegister
	and	ah,0FCh
	out	dx,ax				; Write mode 0
ScrollDownPlanar_exit:
	ret
ScrollDownPlanar	endp
;	   
;	ScrollDownPacked - Scroll down a window in 256-color mode
;
;	Entry:	DS = Seg0
;		AL = Number of lines (AL = 0: Blank entire window)
;		BH = Attribute used to blank line
;		CL = Upper left column
;		CH = Upper left row
;		DL = Lower right column
;		DH = Lower right row
;	Exit:	None
;
	public	ScrollDownBlankEntirePackedWindow
ScrollDownBlankEntirePackedWindow:
	jmp	BlankEntirePackedWindow
;
	public	ScrollDownPacked
ScrollDownPacked	proc
	assume	ds:Seg0, es:nothing
IF (EXTENDED_BIOS and not EXTENDED_TTY)
;
; If extensions were enabled, but extended TTY was not included, do a check
; for standard modes here so that an extended mode does not "fall into"
; this function (which is written both for extended and standard modes).
;
	cmp	[byMode],013h
	ja	ScrollDownPacked_exit
ENDIF
	push	bp				; Create a stack frame for
	sub	sp,size SCROLLFRAME		;  local variables
	mov	bp,sp
;
	push	bx
	push	ax
 	mov 	al,[byMode]			; Get Current mode
	call	GetModeDescPtr			; Pointer to Mode Description
	mov	al,byte ptr es:[bx].MODEDESC.mdBPP	; Get bits per pixel (BPP)
	mov	[bp].SCROLLFRAME.sfBPP,al		;  ...and save it locally
	mov	ax,es:[bx].MODEDESC.mdRowOff		; Get row offset
	mov	[bp].SCROLLFRAME.sfRowOffset,ax		;  ...and save it locally
	pop	ax
	pop	bx
;
	call	GetESSegA000			; Get video segment
	test	al,al				; Do we scroll or blank?
	jz	ScrollDownBlankEntirePackedWindow ; N: Blank entire Window
						;   Y: Continue on
;
; Make:	ES:DI window start
;	DS:SI window 1st row source
;	CX = number of columns
;	DX = number of scan lines to move
;	BP = offset to next row
;	AX = fill variable
;	BX = number of rows to blank at end
;
	mov	bl,al
	mov	al,dh
	sub	al,ch				; Clear more lines than exist?
	cmp	bl,al				;  Yes: blank entire window
	ja	ScrollDownBlankEntirePackedWindow
	push	bx				; Save row count, attribute
	mov	al,dh				; AL = start row
	sub	dh,ch				; DH = number of rows - 1
	sub	dl,cl				; DL = number of columns - 1
	mov	bl,cl				; BL = start column
	mov	cx,dx				; CX = number of rows, columns
;
; Calculate the starting position:
;
;	(Upper left row * row offset * character height) +
;	(Upper left column * BPP)
;
; Number of bytes to move:
;
;	(Lower right column - upper left column) * BPP
;
	push	cx
	sub	ch,ch
	inc	cx
	shl	cx,3
	mov	[bp].SCROLLFRAME.sfMoveCount,cx	; Number of byte to move (assume 8 bpp)
	mov	si,[wCharHeight]		; SI = scan lines per character
	sub	ah,ah
	inc	ax				; Start at next row down - row offset
	mul	si				; Start position = row * row offset
	mul	[bp].SCROLLFRAME.sfRowOffset	;   char height + column * BPP
	sub	bh,bh
	shl	bx,3				; Assume 8 bpp
	mov	di,bx
	cmp	[bp].SCROLLFRAME.sfBPP,15	; 15 or 16 BPP?
	jb	ScrollDownPacked_20		;  Y: Assumption correct
	add	bx,di				;  N: Assume 24 BPP
	add	[bp].SCROLLFRAME.sfMoveCount,cx
	cmp	[bp].SCROLLFRAME.sfBPP,24	; 24 BPP?
	jne	ScrollDownPacked_20		;  N: Assumption correct
	add	bx,di				;  Y: Set columns correctly
	add	[bp].SCROLLFRAME.sfMoveCount,cx	;     Set move count correctly
ScrollDownPacked_20:
	add	ax,bx
	mov	di,ax
	adc	dx,0
	sub	di,[bp].SCROLLFRAME.sfRowOffset	; Get to end of previous row
	sbb	dx,0
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMSetBank			; Set up the banks
	mov	[bp].SCROLLFRAME.sfBankWrite,dx	; Save off what bank we are in
	mov	[bp].SCROLLFRAME.sfBankRead,dx
ENDIF
	pop	cx
;
	pop	bx				; Get row count...
	push	bx				;  ...and save it again
	mov	al,ch				; # of scan lines to move =
	sub	ah,ah				;  (((end row)-(start row)) -
	sub	al,bl				;  (# of lines to scroll)) + 1
	inc	ax

	mul	si
	mov	dx,ax				; DX = number of scan lines
;
	sub	ch,ch
	shl	cx,3				; CX = number of bytes/row
	pop	ax				; Retrieve row count
	push	ax				; Save fill attribute
	push	dx				; Save scan line count
	sub	ah,ah
	mul	si
	mov	bx,ax				; BX = scan lines to blank
	mul	[bp].SCROLLFRAME.sfRowOffset
	mov	si,di
	sub	si,ax				; SI = start source
;
	adc	dx,0
	jc	ScrollDownPacked_30
IF (EXTENDED_BIOS and EXTENDED_TTY)
	mov	ax,[bp].SCROLLFRAME.sfBankWrite	; Add in the write bank
	sub	ax,dx				; Take out what is needed 
	xchg	ax,dx				; Make DL the value to set
	call	OEMSetRBank			; Set up the Read bank
	mov	[bp].SCROLLFRAME.sfBankRead,dx
ENDIF
ScrollDownPacked_30:
	pop	dx				; Restore scan line count
	pop	ax				; Restore fill attribute
	mov	al,ah				;  ...and put it in AL
	mov	cx,[bp].SCROLLFRAME.sfMoveCount	; Move in our variable
	add	[bp].SCROLLFRAME.sfRowOffset,cx
	neg	[bp].SCROLLFRAME.sfRowOffset	; Make BP neg for reverse scroll
	push	es
	pop	ds				; Make DS = video segment
;
	shr	cx,1
ScrollDownPacked_40:
	push	cx
ScrollDownPacked_50:
	movsw
	test	di,di				; Bank Crossing?
	jz	ScrollDownPacked_1010		;  Y: Go handle
ScrollDownPacked_60:
	test	si,si				; Bank Crossing?
	jz	ScrollDownPacked_1020		;  Y: Go handle
ScrollDownPacked_70:
	loop	ScrollDownPacked_50
	pop	cx
;
; Add the row offset to the current position to begin moving next
; line. Note the skip around the bank checking for the READ position
; if the last scan line was moved. The READ bank shouldn't be changed
; if no more scrolling occurs. The WRITE bank may still have to be since
; the clearing begins right after the scroll.
;
        add	di,[bp].SCROLLFRAME.sfRowOffset	; Did we cross a bank?
	jnc	short ScrollDownPacked_1030	;  Y: Go Handle
ScrollDownPacked_80:
	dec	dx
	jz	short ScrollDownPacked_110
	add	si,[bp].SCROLLFRAME.sfRowOffset	; Did we cross a bank?
	jnc	short ScrollDownPacked_1040	;  Y: Go handle
ScrollDownPacked_90:
	jmp	short ScrollDownPacked_40
;
; Blank 256-color window
;
ScrollDownPacked_110:
	push	cx
ScrollDownPacked_120:
	stosw
	test	di,di				; Bank Crossing?
	jz	ScrollDownPacked_1050		;  Y: Go handle
ScrollDownPacked_130:
	loop	ScrollDownPacked_120
	pop	cx
	dec	bx
	jz	short ScrollDownPacked_150
;
; Add the row offset to the current position to begin clearing the next
; line. Note the skip around the bank checking if the last scan line was
; cleared. The bank shouldn't be changed if no more clearing occurs.
;
	add	di,[bp].SCROLLFRAME.sfRowOffset	; Bank Crossing?
	jnc	short ScrollDownPacked_1060	;  Y: Go handle
ScrollDownPacked_140:
	jmp	short ScrollDownPacked_110
;
ScrollDownPacked_150:
	add	sp,size SCROLLFRAME
	pop	bp
ScrollDownPacked_exit:
	ret
;
; OPTIMIZATION HACK:
;
;	These are "jumps away" from the inner loop of the scroll code to
;	handle the once every 64K occurance of a bank crossing. This
;	specifically jumps back into the scroll loop after handling
;	the bank crossing.
;
ScrollDownPacked_1010:					; Inc
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMIncWBank
ENDIF
	jmp	short ScrollDownPacked_60
ScrollDownPacked_1020:					; Inc
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMIncRBank
ENDIF
	jmp	short ScrollDownPacked_70
ScrollDownPacked_1030:					; Dec
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMDecWBank
ENDIF
	jmp	short ScrollDownPacked_80
ScrollDownPacked_1040:					; Dec
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMDecRBank
ENDIF
	jmp	short ScrollDownPacked_90
ScrollDownPacked_1050:					; Inc
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMIncWBank
ENDIF
	jmp	short ScrollDownPacked_130
ScrollDownPacked_1060:					; Dec
IF (EXTENDED_BIOS and EXTENDED_TTY)
	call	OEMDecWBank
ENDIF
	jmp	short ScrollDownPacked_140
ScrollDownPacked	endp
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


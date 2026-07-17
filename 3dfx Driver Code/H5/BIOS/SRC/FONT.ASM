	page	57, 132
	title	Font Load Routines
	subttl	FONT.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	FONT.ASM - Font load routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	GetFontPtr			Get the font based on character height
;	PreFontLoad			Setup for font load
;	PostFontLoad			Fixup from font load
;	GetFixupPtr			Get pointer to 9x?? fixup font
;	LoadFont			Load the font
;	LoadFixup			Load 9x?? fixup
;	FontHandler			Font load handler
;	GetFontInfo			Return font information
;	LoadUserFont			Load user defined font
;	Load8x14Font			Load 8x14 font
;	Load8x8Font			Load 8x8 font
;	SetFontBlock			Set the address the font is grabbed from
;	Load8x16Font			Load 8x16 font
;	LoadUserFontWithRecalc		Load user font with rows recalc'd
;	Load8x14FontWithRecalc		Load 8x14 font with rows recalc'd
;	Load8x8FontWithRecalc		Load 8x8 font with rows recalc'd
;	Load8x16FontWithRecalc		Load 8x16 font with rows recalc'd
;	GraphicsLoad8x8FontIntoINT1F	Load upper half of 8x8 character set for CGA graphics modes
;	GraphicsLoadUserFont		Load user table as graphics character set
;	GraphicsLoad8x14Font		Load 8x14 font as graphics character set
;	GraphicsLoad8x8Font		Load 8x8 font as graphics character set
;	GraphicsLoad8x16Font		Load 8x16 font as graphics character set
;	GetVerticalResolution		Calculate the vertical resolution from the original CRTC values
;	SetVerticalResolution		Set the vertical display end
;	GetFontBlockPtr			Get the pointer to the beginning of the font load region
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/FONT.ASM $
;	$Log: /devel/h5/BIOS/SRC/FONT.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: FONT.ASM
;
.MODEL TINY
.CODE
.386
;
	extrn	GetIndexRegister:near
	extrn	GetESSegA000:near
	extrn	OEMGetParmTable:near
	extrn	GetESSegA000:near
;
	extrn	tblFont8x8:byte
	extrn	tblFont8x14:byte
	extrn	tblFont8x16:byte
	extrn	tblFont9x14:byte
	extrn	tblFont9x16:byte
;
;	GetFontPtr - Get the font based on character height
;
;	Entry:	DS = Seg0
;	Exit:	ES:DI = Pointer to font
;		BH = Character height
;
	public	GetFontPtr
GetFontPtr	proc
	assume	ds:Seg0, es:nothing
	push	cs
	pop	es
	mov	di,offset tblFont8x8
	mov	bh,8
	mov	al,byte ptr [wCharHeight]
	cmp	al,9
	jbe	short GetFontPtr_exit
	mov	bh,14
	mov	di,offset tblFont8x14
	cmp	al,14
	jbe	short GetFontPtr_exit
	mov	bh,16
	mov	di,offset tblFont8x16
GetFontPtr_exit:
	ret
GetFontPtr	endp
;
;	PreFontLoad - Setup for font load
;
;	Entry:	None
;	Exit:	None
;
;	AX is trashed here
;
	public	PreFontLoad
PreFontLoad	proc
	push	dx
	mov	dx,GDC_INDEX
	mov	ax,00005h
	out	dx,ax
	mov	al,06h
	call	GetIndexRegister
	and	ah,001h				; Preserve graphics mode bit
	or	ah,004h
	out	dx,ax
	mov	dl,low SEQ_INDEX
	mov	ax,00402h
	out	dx,ax
	mov	ax,00604h
	out	dx,ax
	pop	dx
	ret
PreFontLoad	endp
;
;	PostFontLoad - Fixup from font load
;
;	Entry:	None
;	Exit:	None
;
;	AX, BX are trashed here
;
	public	PostFontLoad
PostFontLoad	proc
	push	dx
	mov	dx,MISC_INPUT
	mov	bx,00A06h
	in	al,dx
	test	al,1				; Is mode mono?
	jz	short PostFontLoad_10		;  Y: Reset GDC misc to B0000
	mov	bx,00E06h			;  N: Reset GDC misc to B8000
PostFontLoad_10:
	mov	dl,low GDC_INDEX
	mov	ax,bx
	out	dx,ax
	mov	ax,01005h
	out	dx,ax
	mov	dl,low SEQ_INDEX
	mov	ax,00302h
	out	dx,ax
	mov	ax,00204h
	out	dx,ax
	pop	dx
	ret
PostFontLoad	endp
;
;	GetFixupPtr - Get pointer to 9x?? fixup font
;
;	Entry:	DS = Seg0
;	Exit:	ES:DI = Pointer to 9x?? fixup
;		BH = Size of fixup
;
	public	GetFixupPtr
GetFixupPtr	proc
	assume	ds:Seg0
	push	cs
	pop	es
	mov	di,offset tblFont9x14
	mov	bh,14
	cmp	[wCharHeight],14
	jbe	short GetFixupPtr_exit
	mov	di,offset tblFont9x16
	mov	bh,16
GetFixupPtr_exit:
	ret
GetFixupPtr	endp
;
;	LoadFont - Load the font
;
;	Entry:	ES:DI = Pointer to the font
;		BL = Block to load
;		BH = Character height
;		CX = Number of characters to load
;		DX = First character to load
;	Exit:	None
;
;	Assume "PreFontLoad" has been called
;
;	AX, CX, DX, DI, SI are trashed
;	BX, DS, ES are maintained
;
	public	LoadFont
LoadFont	proc
	assume	ds:nothing, es:nothing
	push	bx
	push	ds
	push	es
;
	mov	si,di
	push	es
	pop	ds				; DS:SI = pointer to font
;
	call	GetFontBlockPtr			; ES:DI = Beginning of font memory
	jcxz	LoadFont_exit
	mov	dx,cx
	mov	bl,bh
	sub	bh,bh
	push	bp
	mov	bp,32
	sub	bp,bx
LoadFont_10:
	mov	cx,bx
	rep movsb
	add	di,bp
	dec	dx
	jnz	short LoadFont_10
	pop	bp
LoadFont_exit:
	pop	es
	pop	ds
	pop	bx
	ret
LoadFont	endp
;
;	LoadFixup - Load 9x?? fixup
;
;	Entry:	ES:DI = Pointer to fixup table
;		BL = Block to load
;		BH = Character height
;	Exit:	none
;
;	Assume "PreFontLoad" has been called
;
;	AX, CX, DX, DI, SI are trashed
;	BX, DS, ES are maintained
;
	public	LoadFixup
LoadFixup	proc
	push	bx
	push	ds
	push	es
;
	mov	si,di
	push	es
	pop	ds				; DS:SI = pointer to font
;
	push	bx
	sub	bh,bh
	shl	bx,1
	call	GetESSegA000			; Get font segment
	mov	di,cs:[tblFontBlock+bx]		; ES:DI = start of font block
	pop	bx
;
	mov	dl,32
	mov	bl,bh
	sub	bh,bh
LoadFixup_10:
	lodsb					; Get next ASCII code
	test	al,al				; End of table?
	jz	short LoadFixup_20		;  Y: Go exit
	push	di
	mul	dl
	add	di,ax
	mov	cx,bx
	rep movsb
	pop	di
	jmp	short LoadFixup_10
LoadFixup_20:
;
	pop	es
	pop	ds
	pop	bx
	ret
LoadFixup	endp
;
;	Function 11h:
;	FontHandler - Font load handler
;
;	Entry:	DS, SI are pushed on the stack
;		DS = Seg0
;		AL = Subfunction
;	Exit:	Dependent on subfunction
;
FontFunction_struc	struc
  ffNumber	db	?
  ffFunction	dw	?
FontFunction_struc	ends
;
	public	tblFontFunction
tblFontFunction	label	byte
	FontFunction_struc	<000h, LoadUserFont>
	FontFunction_struc	<001h, Load8x14Font>
	FontFunction_struc	<002h, Load8x8Font>
	FontFunction_struc	<003h, SetFontBlock>
	FontFunction_struc	<004h, Load8x16Font>
	FontFunction_struc	<010h, LoadUserFontWithRecalc>
	FontFunction_struc	<011h, Load8x14FontWithRecalc>
	FontFunction_struc	<012h, Load8x8FontWithRecalc>
	FontFunction_struc	<014h, Load8x16FontWithRecalc>
	FontFunction_struc	<020h, GraphicsLoad8x8FontIntoINT1F>
	FontFunction_struc	<021h, GraphicsLoadUserFont>
	FontFunction_struc	<022h, GraphicsLoad8x14Font>
	FontFunction_struc	<023h, GraphicsLoad8x8Font>
	FontFunction_struc	<024h, GraphicsLoad8x16Font>
NUMBER_OF_FONT_FUNCTIONS	equ	($ - tblFontFunction) / (size FontFunction_struc)
;
	public	FontHandler
FontHandler	proc
	assume	ds:Seg0, es:nothing
	cmp	al,030h			; Special case this function
	je	short GetFontInfo
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	es
	mov	di,NUMBER_OF_FONT_FUNCTIONS
	mov	si,offset tblFontFunction
	mov	ah,al
FontHandler_10:
	lods	byte ptr cs:[si]
	cmp	al,ah
	je	short FontHandler_20
	inc	si
	inc	si
	dec	di
	jnz	short FontHandler_10
	jmp	short FontHandler_exit
;
FontHandler_20:
	lods	word ptr cs:[si]
	mov	si,ax
	call	si
;
FontHandler_exit:
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	si
	iret
FontHandler	endp
;
;	GetFontInfo - Return font information
;
;	Entry:	DS, SI are pushed on the stack
;		DS = Seg0
;	Exit:	Dependent on subfunction
;
	public	tblFontInfoPtr, segfiINT1F, segfiINT43
	public	segfi8x14, segfi8x8, segfi8x8Top
	public	segfi9x14, segfi8x16, segfi9x16
tblFontInfoPtr	label	dword
		dw	01Fh*4
segfiINT1F	dw	0
		dw	043h*4
segfiINT43	dw	0
		dw	offROM8x14
segfi8x14	dw	ROM_SEGMENT
		dw	offROM8x8
segfi8x8	dw	ROM_SEGMENT
		dw	offROM8x8Top
segfi8x8Top	dw	ROM_SEGMENT
		dw	offROM9x14
segfi9x14	dw	ROM_SEGMENT
		dw	offROM8x16
segfi8x16	dw	ROM_SEGMENT
		dw	offROM9x16
segfi9x16	dw	ROM_SEGMENT
;
	public	lpROM8x8, offROM8x8, segROM8x8
lpROM8x8	label	dword
offROM8x8	dw	tblFont8x8
segROM8x8	dw	ROM_SEGMENT
;
	public	lpROM8x8Top, offROM8x8Top, segROM8x8Top
lpROM8x8Top	label	dword
offROM8x8Top	dw	tblFont8x8 + 128*8
segROM8x8Top	dw	ROM_SEGMENT
;
	public	lpROM8x14, offROM8x14, segROM8x14
lpROM8x14	label	dword
offROM8x14	dw	tblFont8x14
segROM8x14	dw	ROM_SEGMENT
;
	public	lpROM8x16, offROM8x16, segROM8x16
lpROM8x16	label	dword
offROM8x16	dw	tblFont8x16
segROM8x16	dw	ROM_SEGMENT
;
	public	lpROM9x14, offROM9x14, segROM9x14
lpROM9x14	label	dword
offROM9x14	dw	tblFont9x14
segROM9x14	dw	ROM_SEGMENT
;
	public	lpROM9x16, offROM9x16, segROM9x16
lpROM9x16	label	dword
offROM9x16	dw	tblFont9x16
segROM9x16	dw	ROM_SEGMENT
;
	public	GetFontInfo
GetFontInfo	proc
	assume	ds:Seg0, es:nothing
	push	bx
	mov	cx,[wCharHeight]
	mov	dl,[byRows]
	cmp	bh,7
	ja	short GetFontInfo_exit
	mov	bl,bh
	sub	bh,bh
	shl	bx,2
	les	bp,cs:[tblFontInfoPtr + bx]
	les	bp,es:[bp]
GetFontInfo_exit:
	pop	bx
	pop	ds
	pop	si
	iret
GetFontInfo	endp
;
;	LoadUserFont - Load user defined font
;
;	Entry:	BL = Block to load
;		BH = Character height (number of bytes per character)
;		CX = Number of characters to load
;		DX = Character position to begin load
;		ES:BP = Pointer to font
;	Exit:	None
;
	public	LoadUserFont
LoadUserFont	proc
	assume	ds:Seg0, es:nothing
	jcxz	LoadUserFont_exit
	mov	di,bp				; ES:DI = start of load
	call	PreFontLoad			; AX is trashed here
	call	LoadFont
	call	PostFontLoad			; AX, BX are trashed here
LoadUserFont_exit:
	ret
LoadUserFont	endp
;
;	Load8x14Font - Load 8x14 font
;
;	Entry:	BL = Block to load
;	Exit:	None
;
	public	Load8x14Font
Load8x14Font	proc
	assume	ds:Seg0, es:nothing
	call	PreFontLoad			; AX is trashed here
	mov	bh,14
	mov	cx,256
	sub	dx,dx
	les	di,cs:[lpROM8x14]
	call	LoadFont			; BX is maintained
	call	PostFontLoad			; AX, BX are trashed here
	ret
Load8x14Font	endp
;
;	Load8x8Font - Load 8x8 font
;
;	Entry:	BL = Block to load
;	Exit:	None
;
	public	Load8x8Font
Load8x8Font	proc
	assume	ds:Seg0, es:nothing
	call	PreFontLoad			; AX is trashed here
	mov	bh,8
	mov	cx,256
	sub	dx,dx
	les	di,cs:[lpROM8x8]
	call	LoadFont
	call	PostFontLoad			; AX, BX are trashed here
	ret
Load8x8Font	endp
;
;	SetFontBlock - Set the address the font is grabbed from
;
;	Entry:	BL = Block specifier
;	Exit:	None
;
	public	SetFontBlock
SetFontBlock	proc
	assume	ds:Seg0, es:nothing
	mov	dx,SEQ_INDEX
	mov	al,3
	mov	ah,bl
	out	dx,ax
	ret
SetFontBlock	endp
;
;	Load8x16Font - Load 8x16 font
;
;	Entry:	BL = Block to load
;	Exit:	None
;
	public	Load8x16Font
Load8x16Font	proc
	assume	ds:Seg0, es:nothing
	call	PreFontLoad			; AX is trashed here
	mov	bh,16
	mov	cx,256
	sub	dx,dx
	les	di,cs:[lpROM8x16]
	call	LoadFont
	call	PostFontLoad			; AX, BX are trashed here
	ret
Load8x16Font	endp
;
;	LoadUserFontWithRecalc - Load user font with rows recalc'd
;
;	Entry:	BL = Block to load
;		BH = Character height (number of bytes per character)
;		CX = Number of characters to load
;		DX = Character position to begin load
;		ES:BP = Pointer to font
;	Exit:	None
;
	public	LoadUserFontWithRecalc
LoadUserFontWithRecalc	proc
	assume	ds:Seg0, es:nothing
	jcxz	LoadUserFontWithRecalc_exit
	mov	di,bp				; ES:DI = start of load
	call	PreFontLoad			; AX is trashed here
	call	LoadFont			; BX, DS is maintained here
	call	RecalcRows
	call	PostFontLoad			; AX, BX are trashed here
LoadUserFontWithRecalc_exit:
	ret
LoadUserFontWithRecalc	endp
;
;	Load8x14FontWithRecalc - Load 8x14 font with rows recalc'd
;
;	Entry:	BL = Block to load
;	Exit:	None
;
	public	Load8x14FontWithRecalc
Load8x14FontWithRecalc	proc
	assume	ds:Seg0, es:nothing
	call	PreFontLoad			; AX is trashed here
	mov	bh,14
	mov	cx,256
	sub	dx,dx
	les	di,cs:[lpROM8x14]
	call	LoadFont			; BX, DS are maintained here
	call	RecalcRows
	call	PostFontLoad			; AX, BX are trashed here
	ret
Load8x14FontWithRecalc	endp
;
;	Load8x8FontWithRecalc - Load 8x8 font with rows recalc'd
;
;	Entry:	BL = Block to load
;	Exit:	None
;
	public	Load8x8FontWithRecalc
Load8x8FontWithRecalc	proc
	assume	ds:Seg0, es:nothing
	call	PreFontLoad			; AX is trashed here
	mov	bh,8
	mov	cx,256
	sub	dx,dx
	les	di,cs:[lpROM8x8]
	call	LoadFont			; BX, DS are maintained here
	call	RecalcRows
	call	PostFontLoad			; AX, BX are trashed here
	ret
Load8x8FontWithRecalc	endp
;
;	Load8x16FontWithRecalc - Load 8x16 font with rows recalc'd
;
;	Entry:	BL = Block to load
;	Exit:	None
;
	public	Load8x16FontWithRecalc
Load8x16FontWithRecalc	proc
	assume	ds:Seg0, es:nothing
	call	PreFontLoad			; AX is trashed here
	mov	bh,16
	mov	cx,256
	sub	dx,dx
	les	di,cs:[lpROM8x16]
	call	LoadFont			; BX, DS are maintained here
	call	RecalcRows
	call	PostFontLoad			; AX, BX are trashed here
	ret
Load8x16FontWithRecalc	endp
;
;	GraphicsLoad8x8FontIntoINT1F - Load upper half of 8x8 character set for CGA graphics modes
;
;	Entry:	DS = Seg0
;		ES:BP = Pointer to font (characters 128-255)
;	Exit:	None
;
	public	GraphicsLoad8x8FontIntoINT1F
GraphicsLoad8x8FontIntoINT1F	proc
	assume	ds:Seg0, es:nothing
	cli
	mov	[offINT1F],bp
	mov	[segINT1F],es
	sti
	ret
GraphicsLoad8x8FontIntoINT1F	endp
;
;	GraphicsLoadUserFont - Load user table as graphics character set
;
;	Entry:	DS = Seg0
;		BL = Row specifier:
;			0 - User defined:
;		DL = Number of rows
;			1 - 14 rows
;			2 - 25 rows
;			3 - 43 rows
;		CX = Character height
;		ES:BP = Pointer to font
;	Exit:	None
;
	public	tblRows
tblRows	db	0, 14, 25, 43
;
	public	GraphicsLoadUserFont
GraphicsLoadUserFont	proc
	assume	ds:Seg0, es:nothing
	mov	[offINT43],bp
	mov	[segINT43],es
	mov	[wCharHeight],cx
	test	bl,bl
	jz	short GraphicsLoadUserFont_10
	sub	bh,bh
	mov	dl,cs:[tblRows + bx]
GraphicsLoadUserFont_10:
	dec	dx
	mov	[byRows],dl
	ret
GraphicsLoadUserFont	endp
;
;	GraphicsLoad8x14Font - Load 8x14 font as graphics character set
;
;	Entry:	DS = Seg0
;		BL = Row specifier:
;			0 - User defined:
;		DL = Number of rows
;			1 - 14 rows
;			2 - 25 rows
;			3 - 43 rows
;	Exit:	none
;
	public	GraphicsLoad8x14Font
GraphicsLoad8x14Font	proc
	assume	ds:Seg0, es:nothing
	les	di,cs:[lpROM8x14]
	mov	[offINT43],di
	mov	[segINT43],es
	mov	[wCharHeight],14
	test	bl,bl
	jz	short GraphicsLoad8x14Font_10
	sub	bh,bh
	mov	dl,cs:[tblRows + bx]
GraphicsLoad8x14Font_10:
	dec	dx
	mov	[byRows],dl
	ret
GraphicsLoad8x14Font	endp
;
;	GraphicsLoad8x8Font - Load 8x8 font as graphics character set
;
;	Entry:	DS = Seg0
;		BL = Row specifier:
;			0 - User defined:
;		DL = Number of rows
;			1 - 14 rows
;			2 - 25 rows
;			3 - 43 rows
;	Exit:	none
;
	public	GraphicsLoad8x8Font
GraphicsLoad8x8Font	proc
	assume	ds:Seg0, es:nothing
	les	di,cs:[lpROM8x8]
	mov	[offINT43],di
	mov	[segINT43],es
	mov	[wCharHeight],8
	test	bl,bl
	jz	short GraphicsLoad8x8Font_10
	sub	bh,bh
	mov	dl,cs:[tblRows + bx]
GraphicsLoad8x8Font_10:
	dec	dx
	mov	[byRows],dl
	ret
GraphicsLoad8x8Font	endp
;
;	GraphicsLoad8x16Font - Load 8x16 font as graphics character set
;
;	Entry:	DS = Seg0
;		BL = Row specifier:
;			0 - User defined:
;		DL = Number of rows
;			1 - 14 rows
;			2 - 25 rows
;			3 - 43 rows
;	Exit:	None
;
	public	GraphicsLoad8x16Font
GraphicsLoad8x16Font	proc
	assume	ds:Seg0, es:nothing
	les	di,cs:[lpROM8x16]
	mov	[offINT43],di
	mov	[segINT43],es
	mov	[wCharHeight],16
	test	bl,bl
	jz	short GraphicsLoad8x16Font_10
	sub	bh,bh
	mov	dl,cs:[tblRows + bx]
GraphicsLoad8x16Font_10:
	dec	dx
	mov	[byRows],dl
	ret
GraphicsLoad8x16Font	endp
;
;	RecalcRows - Recalculate rows
;
;	Entry:	DS = Seg0
;		BH = Character height
;	Exit:	None
;
;	AX, BX, CX, DX are trashed
;
	public	RecalcRows
RecalcRows	proc
	assume	ds:Seg0, es:nothing
;
; Set the vertical display end
;
	call	GetVerticalResolution		; DX = CRTC address returned
	div	bh
	dec	ax
	mov	[byRows],al
	mov	byte ptr [wCharHeight],bh
	inc	ax
	mul	bh
	dec	ax
	call	SetVerticalResolution		; Expects DX = 3x4h
;
; Set the underline start register, if monochrome
;
	cmp	[byMode],07h			; Mono text mode?
	jne	short RecalcRows_10		;  N: Skip setting CRTC[14]
	mov	ah,bh
	mov	al,14h
	out	dx,ax
RecalcRows_10:
;
; Calculate character height and start/stop cursor scans
;
	mov	al,9				; CRTC[09]
	call	GetIndexRegister
	and	ah,not 01Fh
	dec	bh
	or	ah,bh
	out	dx,ax
	inc	ax				; CRTC[0A]
	cmp	bh,007h				; Character height > 8?
	jbe	short RecalcRows_20		;  N: Cursor stop is at bottom of cell
	dec	bh				;  Y: Leave gap between cursor stop and cell bottom
RecalcRows_20:
	mov	ah,bh
	dec	ah
	out	dx,ax
	mov	[byCursorStart],ah		; Save the new cursor start scan
	inc	ax
	mov	ah,bh
	out	dx,ax
	mov	[byCursorStop],ah		; Save the new cursor end scan
;
; Calculate the new regen length: ((byRows + 1) * wColumns) * 2
;
	mov	al,[byRows]
	inc	ax
	mul	byte ptr [wColumns]
	shl	ax,1
	mov	[wRegenLength],ax
	ret
RecalcRows	endp
;
;	GetVerticalResolution - Calculate the vertical resolution from the original CRTC values
;
;	Entry:	DS = Seg0
;	Exit:	AX = Vertical resolution
;		DX = CRTC
;
;	Note:	This routine calculates the value from the ORIGINAL
;		parameter table that the current mode used. This
;		prevents a bug in DMU where multiple recalculations
;		change the vertical resolution.
;
;	CX is trashed
;
	public	GetVerticalResolution
GetVerticalResolution	proc
	assume	ds:Seg0, es:nothing
	push	di
	push	es
;
	call	OEMGetParmTable			; ES:DI = Ptr to parm table
	mov	al,es:[di].PARMTABLE.CRTC.CRTC.VDispEnd
	mov	ah,es:[di].PARMTABLE.CRTC.CRTC.Overflow
	mov	ch,ah
	and	ah,002h				; Isolate bit 8
	and	ch,040h				; Isolate bit 9
	shr	ah,1				; Put bit 8 into position
	rol	ch,3				; Put bit 9 into position
	or	ah,ch
	inc	ax
						; Multiply vertical by 2?
	test	es:[di].PARMTABLE.CRTC.CRTC.Mode,4
	jz	short GetVerticalResolution_10	;  N: Skip it
	shl	ax,1				;  Y: Do it
GetVerticalResolution_10:			; Scan double?
	test	es:[di].PARMTABLE.CRTC.CRTC.CellHeight,080h
	jz	short GetVerticalResolution_20	;  N: Skip it
	inc	ax
	shr	ax,1				;  Y: Halve vert resolution
GetVerticalResolution_20:
	mov	dx,[wCRTCAddr]
;
	pop	es
	pop	di
	ret
GetVerticalResolution	endp
;
;	SetVerticalResolution - Set the vertical display end
;
;	Entry:	AX = 10 bit value to program into CRTC
;		DX = CRTC address
;	Exit:	None
;
;	AX, CX are trashed
;
	public	SetVerticalResolution
SetVerticalResolution	proc
	assume	ds:nothing, es:nothing
	mov	cx,ax
	mov	al,09h
	call	GetIndexRegister
	test	ah,080h				; Double scanned?
	jz	short SetVerticalResolution_10	;  N: Use number as is
	inc	cx				;  Y: vde = (vde + 1)*2 - 1
	shl	cx,1				;  Y: Multiply by 2
	dec	cx
SetVerticalResolution_10:
	mov	ah,cl
	mov	al,12h
	out	dx,ax
	mov	al,11h
	call	GetIndexRegister		; Get old write protect value
	push	ax
	and	ah,07fh				; Un-write protect registers
	out	dx,ax
	mov	al,7
	call	GetIndexRegister
	and	ah,not 042h			; Clear old bits
	mov	cl,ch
	and	cx,00201h
	shl	cl,1
	ror	ch,3
	or	ah,cl
	or	ah,ch
	out	dx,ax
	pop	ax
	out	dx,ax				; Restore write protect state
	ret
SetVerticalResolution	endp
;
;	GetFontBlockPtr - Get the pointer to the beginning of the font load region
;
;	Entry:	BL = Block to load
;		DX = First character to load
;	Exit:	ES:DI = Pointer to font memory
;
;	All other registers are preserved.
;
	public	tblFontBlock
tblFontBlock	label	word
	dw	00000h
	dw	04000h
	dw	08000h
	dw	0C000h
	dw	02000h
	dw	06000h
	dw	0A000h
	dw	0E000h
;
	public	GetFontBlockPtr
GetFontBlockPtr	proc
	push	bx
	push	dx
;
	and	bx,0007h			; Clear BH, limit BL range
	shl	bx,1
	call	GetESSegA000
	mov	di,cs:[tblFontBlock + bx]	; ES:DI = start of font block
;
; Make ES:DI point to memory where the 1st character loaded will be put:
;    DI = DI + (DX*32)
;
	shl	dx,5
	add	di,dx				; ES:DI = start font load here
;
	pop	dx
	pop	bx
	ret
GetFontBlockPtr	endp
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


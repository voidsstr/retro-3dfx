	page	57, 132
	title	Write String Functions
	subttl	STRING.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	STRING.ASM - Write String functions
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	WriteString			Write string of TTY characters
;	WriteStrConstAttNoMove		Write string of TTY characters, constant attribute, cursor not moved
;	WriteStrConstAttMove		Write string of TTY characters, constant attribute, cursor moved
;	WriteStrAttNoMove		Write string of TTY characters, string includes attribute, cursor not moved
;	WriteStrAttMove			Write string of TTY characters, string includes attribute, cursor moved
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/STRING.ASM $
;	$Log: /devel/h5/BIOS/SRC/STRING.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: STRING.ASM
;
.MODEL TINY
.CODE
.386
;
	extrn	LocalSetCursorPosition:near
	extrn	GetModeDescPtr:near
;
	extrn	tblWriteTTY:word
	extrn	tblWriteCharAttribute:word
;
;	Function 13h:
;	WriteString - Write string of TTY characters
;
;	Entry:	SI, DS are pushed on the stack
;		AL = String type:
;		  0: Constant attribute, cursor not moved
;		  1: Constant attribute, cursor moved
;		  2: String includes attributes, cursor not moved
;		  3: String includes attributes, cursor moved
;		BL = Attribute (string type 0, 1)
;		BH = Page number
;		CX = Character count
;		DL = Cursor column to start
;		DH = Cursor row to start
;		DS = Seg0
;		ES:BP = Pointer to character string
;	Exit:	None
;
	public	tblWriteString
tblWriteString	label	word
	dw	WriteStrConstAttNoMove
	dw	WriteStrConstAttMove
	dw	WriteStrAttNoMove
	dw	WriteStrAttMove
;
	public	WriteString
WriteString	proc
	assume	ds:Seg0, es:nothing
	pusha
;
	jcxz	WriteString_exit
	cmp	al,3
	ja	short WriteString_exit
	cbw
	mov	si,ax
	shl	si,1				; SI = Entry to string type
;
	mov	al,[byMode]
	push	bx				; Save page & attribute
	push	es				; Save string segment
	call	GetModeDescPtr			; Returns ptr in ES:BX
	mov	al,es:[bx].MODEDESC.mdType	; Get mode type from table
	pop	es				; Restore string segment
	pop	bx				; Restore page & attribute
	jc	WriteString_exit		; Error? Y: Mode doesn't exist
;
	cbw
	shl	ax,1				; AX = Mode type * 2
	call	cs:[tblWriteString + si]
;
WriteString_exit:
	popa
	pop	ds
	pop	si
	iret
WriteString	endp
;
;	WriteStrConstAttNoMove - Write string of TTY characters, constant attribute, cursor not moved
;	(string type 0)
;
;	Entry:	SI, DS are pushed on the stack
;		AX = Mode type*2
;		BL = Attribute
;		BH = Page number
;		CX = Character count
;		DL = Cursor column to start
;		DH = Cursor row to start
;		DS = Seg0
;		ES:BP = Pointer to character string
;	Exit:	None
;
	public	WriteStrConstAttNoMove
WriteStrConstAttNoMove	proc
	assume	ds:Seg0, es:nothing
	mov	si,ax
	push	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	ax,[wCursorPosition + bx]
	pop	bx
	push	ax			; Save original cursor position
;
WriteStrCANM_10:
	call	LocalSetCursorPosition
	mov	al,es:[bp]
	inc	bp
	push	bx
	push	cx
	push	si
	push	es
	cmp	al,13
	ja	WriteStrCANM_30
	je	WriteStrCANM_20
	cmp	al,10
	ja	WriteStrCANM_30
	je	WriteStrCANM_20
	cmp	al,8
	ja	WriteStrCANM_30
	cmp	al,7
	jb	WriteStrCANM_30
WriteStrCANM_20:
	push	bx
	call	cs:[tblWriteTTY + si]
	pop	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	jmp	short WriteStrCANM_40
WriteStrCANM_30:
	mov	cx,1
	push	bx
	push	dx
	push	si
	call	cs:[tblWriteCharAttribute + si]
	pop	si
	pop	dx
	pop	bx
	inc	dx
	cmp	dl,byte ptr [wColumns]
	jb	WriteStrCANM_40
	push	bx
	mov	al,13
	push	bx
	push	si
	push	es
	call	cs:[tblWriteTTY + si]
	pop	es
	pop	si
	pop	bx
	mov	al,10
	call	cs:[tblWriteTTY + si]
	pop	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]

WriteStrCANM_40:
	pop	es
	pop	si
	pop	cx
	pop	bx
	loop	WriteStrCANM_10
;
	pop	dx
	call	LocalSetCursorPosition
	ret
WriteStrConstAttNoMove	endp
;
;	WriteStrConstAttMove - Write string of TTY characters, constant attribute, cursor moved
;	(string type 1)
;
;	Entry:	SI, DS are pushed on the stack
;		AX = Mode number*2
;		BL = Attribute
;		BH = Page number
;		CX = Character count
;		DL = Cursor column to start
;		DH = Cursor row to start
;		DS = Seg0
;		ES:BP = Pointer to character string
;	Exit:	None
;
	public	WriteStrConstAttMove
WriteStrConstAttMove	proc
	assume	ds:Seg0, es:nothing
	mov	si,ax
WriteStrCAM_10:
	call	LocalSetCursorPosition
	mov	al,es:[bp]
	inc	bp
	push	bx
	push	cx
	push	si
	push	es
	cmp	al,13
	ja	WriteStrCAM_30
	je	WriteStrCAM_20
	cmp	al,10
	ja	WriteStrCAM_30
	je	WriteStrCAM_20
	cmp	al,8
	ja	WriteStrCAM_30
	cmp	al,7
	jb	WriteStrCAM_30
WriteStrCAM_20:
	push	bx
	call	cs:[tblWriteTTY + si]
	pop	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	jmp	short WriteStrCAM_40
WriteStrCAM_30:
	mov	cx,1
	push	bx
	push	dx
	push	si
	call	cs:[tblWriteCharAttribute + si]
	pop	si
	pop	dx
	pop	bx
	inc	dx
	cmp	dl,byte ptr [wColumns]
	jb	WriteStrCAM_40
	push	bx
	mov	al,13
	push	bx
	push	si
	push	es
	call	cs:[tblWriteTTY + si]
	pop	es
	pop	si
	pop	bx
	mov	al,10
	call	cs:[tblWriteTTY + si]
	pop	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
WriteStrCAM_40:
	pop	es
	pop	si
	pop	cx
	pop	bx
	loop	WriteStrCAM_10
	call	LocalSetCursorPosition
	ret
WriteStrConstAttMove	endp
;
;	WriteStrAttNoMove - Write string of TTY characters, string includes attribute, cursor not
;	moved (string type 2)
;
;	Entry:	SI, DS are pushed on the stack
;		AX = Mode number*2
;		BH = Page number
;		CX = Character count
;		DL = Cursor column to start
;		DH = Cursor row to start
;		DS = Seg0
;		ES:BP = Pointer to character string
;	Exit:	None
;
	public	WriteStrAttNoMove
WriteStrAttNoMove	proc
	assume	ds:Seg0, es:nothing
	mov	si,ax
	push	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	ax,[wCursorPosition + bx]
	pop	bx
	push	ax			; Save original cursor position
;
WriteStrANM_10:
	call	LocalSetCursorPosition
	mov	al,es:[bp]
	inc	bp
	push	bx
	push	cx
	push	si
	push	es
	cmp	al,13
	ja	WriteStrANM_30
	je	WriteStrANM_20
	cmp	al,10
	ja	WriteStrANM_30
	je	WriteStrANM_20
	cmp	al,8
	ja	WriteStrANM_30
	cmp	al,7
	jb	WriteStrANM_30
WriteStrANM_20:
	push	bx
	call	cs:[tblWriteTTY + si]
	pop	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	jmp	short WriteStrANM_40
WriteStrANM_30:
	mov	bl,es:[bp]
	inc	bp
	mov	cx,1
	push	bx
	push	dx
	push	si
	call	cs:[tblWriteCharAttribute + si]
	pop	si
	pop	dx
	pop	bx
	inc	dx
	cmp	dl,byte ptr [wColumns]
	jb	WriteStrANM_40
	push	bx
	mov	al,13
	push	bx
	push	si
	push	es
	call	cs:[tblWriteTTY + si]
	pop	es
	pop	si
	pop	bx
	mov	al,10
	call	cs:[tblWriteTTY + si]
	pop	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
WriteStrANM_40:
	pop	es
	pop	si
	pop	cx
	pop	bx
	loop	WriteStrANM_10
;
	pop	dx
	call	LocalSetCursorPosition
	ret
WriteStrAttNoMove	endp
;
;	WriteStrAttMove - Write string of TTY characters, string includes attribute, cursor moved
;	(string type 3)
;
;	Entry:	SI, DS are pushed on the stack
;		AX = Mode number*2
;		BH = Page number
;		CX = Character count
;		DL = Cursor column to start
;		DH = Cursor row to start
;		DS = Seg0
;		ES:BP = Pointer to character string
;	Exit:	None
;
	public	WriteStrAttMove
WriteStrAttMove	proc
	assume	ds:Seg0, es:nothing
	mov	si,ax
WriteStrAttMove_10:
	call	LocalSetCursorPosition
	mov	al,es:[bp]
	inc	bp
	push	bx
	push	cx
	push	si
	push	es
	cmp	al,13
	ja	WriteStrAttMove_30
	je	WriteStrAttMove_20
	cmp	al,10
	ja	WriteStrAttMove_30
	je	WriteStrAttMove_20
	cmp	al,8
	ja	WriteStrAttMove_30
	cmp	al,7
	jb	WriteStrAttMove_30
WriteStrAttMove_20:
	push	bx
	call	cs:[tblWriteTTY + si]
	pop	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	jmp	short WriteStrAttMove_40
WriteStrAttMove_30:
	mov	bl,es:[bp]
	inc	bp
	mov	cx,1
	push	bx
	push	dx
	push	si
	call	cs:[tblWriteCharAttribute + si]
	pop	si
	pop	dx
	pop	bx
	inc	dx
	cmp	dl,byte ptr [wColumns]
	jb	WriteStrAttMove_40
	push	bx
	mov	al,13
	push	bx
	push	si
	push	es
	call	cs:[tblWriteTTY + si]
	pop	es
	pop	si
	pop	bx
	mov	al,10
	call	cs:[tblWriteTTY + si]
	pop	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
WriteStrAttMove_40:
	pop	es
	pop	si
	pop	cx
	pop	bx
	loop	WriteStrAttMove_10
	call	LocalSetCursorPosition
	ret
WriteStrAttMove	endp
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


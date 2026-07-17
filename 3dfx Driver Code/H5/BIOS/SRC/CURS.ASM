	page	57, 132
	title	Cursor Type and Position Routines
	subttl	CURS.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	CURS.ASM - Cursor Type and Position Routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	SetCursorType		Set cursor start and stop scan line
;	FixupCursor		Fixup the cursor to emulate a 6845
;	SetCursorPosition	Set cursor position
;	LocalSetCursorPosition	Local routine for set cursor position
;	GetCursorPosition	Get cursor position
;	SetActivePage		Set regen start
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/CURS.ASM $
;	$Log: /devel/h5/BIOS/SRC/CURS.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: CURS.ASM
;
	extrn	IsTextMode:near
;
.MODEL TINY
.CODE
.386
;
;	Function 01h:
;	SetCursorType - Set cursor start and stop scan line
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		CL = Cursor end scan line
;		CH = Cursor start scan line
;	Exit:	AX = last value programmed into CRTC
;
	public	SetCursorType
SetCursorType	proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	dx
;
	mov	al,[byMode]
	call	IsTextMode			; Is this a graphics mode?
	jnz	SetCursorType_exit		;  Y: Skip this function
;
	mov	[wCursorType],cx		; VGA inactive or emulate off?
	test	[fEGAInfo],NO_CURSOR_EMULATE + VGA_INACTIVE
	jnz	SetCursorType_20		;  Y: Set cursor as is
	test	ch,20h				; Turn cursor off?
	jnz	SetCursorType_10		;  Y: Set cursor to compatible value
	call	FixupCursor
	jmp	short SetCursorType_20
;
; HACK: Force the start scan line to (almost) the last possible value of
; of the scan line register. This is the value that IBM uses instead of
; the 20h as documented.
;
SetCursorType_10:
	mov	cx,01E00h			; IBM VGA uses this
;
; END HACK
;
SetCursorType_20:
	mov	dx,[wCRTCAddr]
	mov	al,CRTC.CursorScanStart
	mov	ah,ch
	out	dx,ax
	inc	ax
	mov	ah,cl
	out	dx,ax
;
SetCursorType_exit:
	pop	dx
	pop	cx
	pop	ds
	pop	si
	iret
SetCursorType	endp
;
;	FixupCursor - Fixup the cursor to emulate a 6845
;
;	Entry:	DS = Seg0
;		CL = Cursor end scan line
;		CH = Cursor start scan line
;	Exit:	CX = Cursor type fixed up
;
	public	FixupCursor
FixupCursor	proc
	assume	ds:Seg0, es:nothing
	mov	dl,byte ptr [wCharHeight]
	dec	dx
	cmp	cl,ch				; End >= start?
	jge	short FixupCursor_10		;  Y: Okay to fixup
	test	cl,cl
	mov	cl,dl				;  N: end = char_height
	jnz	FixupCursor_exit
	mov	cx,02000h			; Hide cursor
	jmp	short FixupCursor_exit
FixupCursor_10:
	cmp	cl,3				; Overbar cursor?
	jbe	FixupCursor_exit		;  Yes: exit as is
	mov	ah,ch
	inc	ah
	inc	ah
	cmp	ah,cl				; Underline cursor?
	jb	FixupCursor_20			;  No: try other type
	dec	dx
	sub	cl,ch
	mov	ch,dl
	sub	dl,cl
	mov	cl,dl
	cmp	cl,ch
	jge	FixupCursor_exit
	mov	cl,byte ptr [wCharHeight]
	dec	cx
	jmp	short FixupCursor_exit
FixupCursor_20:
	cmp	ch,2				; Full block cursor?
	jg	FixupCursor_30			;  No: try other type
	mov	cl,dl
	jmp	short FixupCursor_exit
FixupCursor_30:
	mov	cl,dl
	mov	ch,dl
	shr	ch,1
FixupCursor_exit:
	ret
FixupCursor	endp
;
;	Function 02h:
;	SetCursorPosition - Set cursor position
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		BH = Page number
;		DL = Column
;		DH = Row
;	Exit:	None
;
	public	SetCursorPosition
SetCursorPosition	proc
	assume	ds:Seg0, es:nothing
	pusha
	call	LocalSetCursorPosition
	popa
	pop	ds
	pop	si
	iret
SetCursorPosition	endp
;
;	LocalSetCursorPosition - Local routine for set cursor position
;
;	Entry:	DS = Seg0
;		BH = Page number
;		DL = Column
;		DH = Row
;	Exit:	None
;
	public	LocalSetCursorPosition
LocalSetCursorPosition	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	dx
;
	sub	ax,ax
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	[wCursorPosition + bx],dx
	shr	bx,1
	cmp	bl,[byActivePage]
	jne	LocalSetCursorPosition_exit
	mov	bx,[wRegenStart]
	shr	bx,1
	mov	al,dh
	mul	byte ptr [wColumns]
	add	al,dl
	adc	ah,0
	add	bx,ax
	mov	dx,[wCRTCAddr]
	mov	al,CRTC.CursorPositHigh
	mov	ah,bh
	out	dx,ax
	inc	ax
	mov	ah,bl
	out	dx,ax
LocalSetCursorPosition_exit:
;
	pop	dx
	pop	bx
	ret
LocalSetCursorPosition	endp
;
;	Function 03h:
;	GetCursorPosition - Get cursor position
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		BH = Page number
;	Exit:	CX = Cursor type
;		DL = Column
;		DH = Row
;
	public	GetCursorPosition
GetCursorPosition	proc
	push	bx
	mov	bl,bh
	sub	bh,bh
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	mov	cx,[wCursorType]
	pop	bx
	pop	ds
	pop	si
	iret
GetCursorPosition	endp
;
;	Function 05h:
;	SetActivePage - Set regen start
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		AL = Page to set to
;	Exit:	None
;
	public	SetActivePage
SetActivePage	proc
	assume	ds:Seg0, es:nothing
	pusha
;
	mov	[byActivePage],al
	sub	bx,bx
	cbw					; Page should be < 8
	push	ax				; Save page #
	test	al,al				; Page 0?
	jz	SetActivePage_20		;  Y: Regen start is 0
SetActivePage_10:
	add	bx,[wRegenLength]
	dec	ax
	jnz	SetActivePage_10
SetActivePage_20:
	mov	[wRegenStart],bx
	push	ax
	mov	al,[byMode]
	call	IsTextMode
	pop	ax
	jnz	SetActivePage_30
	shr	bx,1
SetActivePage_30:
	mov	dx,[wCRTCAddr]
	mov	al,CRTC.RegenStartHigh
	mov	ah,bh
	out	dx,ax
	inc	ax
	mov	ah,bl
	out	dx,ax
	pop	bx				; Get page #
	shl	bx,1
	mov	dx,[wCursorPosition + bx]
	shr	bx,1
	mov	bh,bl
	call	LocalSetCursorPosition
;
	popa
	pop	ds
	pop	si
	iret
SetActivePage	endp
;
	end
;
;	Copyright (C) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


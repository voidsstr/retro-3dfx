	page	57, 132
	title	Debug Routines
	subttl	DEBUG.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	DEBUG.ASM - Debug routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/DEBUG.ASM $
;	$Log: /devel/h5/BIOS/SRC/DEBUG.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
;	Routines in this file:
;	MonoOut			Dump a string to the monochrome monitor
;	MonoOutInternalChar	Write a character to monochrome memory
;	MonoOutInternalScroll	Scroll the entire screen up one line
;	MonoOutClear		Clear all of monochrome memory
;	MonoOutInternalDumpReg	Display the contents of a named register
;	MonoOutMemICmp		Compare two case-independent memory locations
;	_MonoString		Display a string to the mono monitor
;	HandlerINT42		INT 42h redirection handler
;
;	Note:	The stack parameters are bound to TINY and SMALL models.
;		If this should change to MEDIUM, LARGE, or HUGE, then
;		each stack reference would have to be incremented by 2
;		to compensate for the segment pushed on the stack
;		by a far call.
;
;	Note:	These routines can only be executed in RAM; they will
;		not function correctly in ROM.
;
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
IFDEF DEBUG
  IFDEF ROM
	%out	!	Assembling: DEBUG.ASM (Debug version - For ROM Boot Testing only!)
  ELSE
	%out	!	Assembling: DEBUG.ASM (Debug version - For RAM BIOS only)
  ENDIF
ELSE
	%out	!	Assembling: DEBUG.ASM (Non-debug version)
ENDIF
;
;
.MODEL TINY
.CODE
.386
;
IFDEF DEBUG
;
; To debug a boot ROM, a hardwired segment is used for the necessary
; RAM addresses. Otherwise, for normal debugging (as a TSR), writing
; back to the BIOS's code segment is acceptable for this.
;
	public	DWMONOROW, DWMONOCOL, DWUSEMONOFLAG, SZBUFFER
IFDEF ROM
  SEG_HARDWIRED_DATA	equ	6000h
  DWMONOROW		equ	0
  DWMONOCOL		equ	4
  DWUSEMONOFLAG		equ	8
  SZBUFFER		equ	12
ELSE
  DWMONOROW		dd	0
  DWMONOCOL		dd	0
  DWUSEMONOFLAG		dd	0
  SZBUFFER		db	128 dup (0)
ENDIF
;
	public	dwMonoAddr, wMonoOffset, wMonoSegment
dwMonoAddr	label	dword
wMonoOffset	dw	00000h
wMonoSegment	dw	0B000h
;
	public	wCOMPort
wCOMPort	dw	0		; Use COM1 as default
;
; Register structure is:
;	Length of register name string
;	Register name
;	Size of register in bytes
;	Stack location
;
	public	monoRegs
monoRegs	label	byte
		db	2, "AH",  1, 027h
		db	2, "AL",  1, 026h
		db	2, "AX",  2, 026h
		db	3, "EAX", 4, 026h
		db	2, "BH",  1, 01Bh
		db	2, "BL",  1, 01Ah
		db	2, "BX",  2, 01Ah
		db	3, "EBX", 4, 01Ah
		db	2, "CH",  1, 023h
		db	2, "CL",  1, 022h
		db	2, "CX",  2, 022h
		db	3, "ECX", 4, 022h
		db	2, "DH",  1, 01Fh
		db	2, "DL",  1, 01Eh
		db	2, "DX",  2, 01Eh
		db	3, "EDX", 4, 01Eh
		db	2, "SP",  2, 016h
		db	3, "ESP", 4, 016h
		db	2, "BP",  2, 012h
		db	3, "EBP", 4, 012h
		db	2, "DI",  2, 00Ah
		db	3, "EDI", 4, 00Ah
		db	2, "SI",  2, 00Eh
		db	3, "ESI", 4, 00Eh
		db	2, "SS",  2, 002h
		db	2, "ES",  2, 004h
		db	2, "DS",  2, 006h
		db	2, "CS",  2, 008h
		db	0
;
	public	tblMonoHex
tblMonoHex	label	byte
	db	'0123456789ABCDEF'
;
;	MonoOut - Dump a string to the monochrome monitor
;
;	Entry:	CS:ESI = Pointer to string
;		Stack frame has:
;			PUSHFD
;			PUSHAD
;			PUSH	CS
;			PUSH	DS
;			PUSH	ES
;			PUSH	SS
;			Return Address
;	Exit:	None
;
;	String can contain register values denoted by a "#".
;	Valid registers are:
;		AH, AL		AX		EAX
;		BH, BL		BX		EBX
;		CH, CL		CX		ECX
;		DH, DL		DX		EDX
;				SP		ESP
;				BP		EBP
;				DI		EDI
;				SI		ESI
;				CS
;				DS
;				ES
;				SS
;
;	Stack frame:
;		0	Return Address
;		2	SS
;		4	ES
;		6	DS
;		8	CS
;		A	EDI
;		E	ESI
;		12	EBP
;		16	ESP
;		1A	EBX
;		1E	EDX
;		22	ECX
;		26	EAX
;		2A	EFlags
;
	public	MonoOut
MonoOut	proc
	mov	ebp,esp
	cld
;
IFDEF ROM
	push	SEG_HARDWIRED_DATA
ELSE
	push	cs
ENDIF
	pop	ds				; Set up data segment
;
	mov	dword ptr ds:[DWUSEMONOFLAG],1
	mov	ebx,[DWMONOROW]
	mov	ecx,[DWMONOCOL]
	sub	edi,edi
	les	di,cs:[dwMonoAddr]
MonoOut_10:
	lods byte ptr cs:[esi]
	test	al,al				; End of string?
	jz	short MonoOut_1000		;  Y: Go exit
	cmp	al,0Ah
	je	short MonoOut_20
	cmp	al,0Dh
	je	short MonoOut_30
	cmp	al,'#'
	je	short MonoOut_40
	call	MonoOutInternalChar
	jmp	short MonoOut_10
;
; Handle linefeed
MonoOut_20:
	sub	ecx,ecx
	mov	ds:[DWMONOCOL],ecx
	jmp	short MonoOut_10
;
; Handle carriage return
MonoOut_30:
	inc	ebx
	mov	ds:[DWMONOROW],ebx
	cmp	bl,25
	jb	short MonoOut_10
	mov	ebx,24
	mov	ds:[DWMONOROW],ebx
	call	MonoOutInternalScroll
	jmp	short MonoOut_10
;
; Handle '#' character
MonoOut_40:
	call	MonoOutInternalDumpReg
	jmp	short MonoOut_10
;
MonoOut_1000:
	mov	dword ptr ds:[DWUSEMONOFLAG],0
	ret
MonoOut	endp
;
;	MonoOutInternalChar - Write a character to monochrome memory
;
;	Entry:	AL = Character
;		EBX = Row
;		ECX = Column
;		ES:EDI = Mono memory map
;	Exit:	EBX = Updated to next row
;		ECX = Updated to next column
;
	public	MonoOutInternalChar
MonoOutInternalChar	proc
	push	eax
	push	edx
	push	edi
;
	imul	edx,ebx,160		; Assume 80 columns
	add	edx,ecx
	add	edx,ecx
	mov	ah,07h			; Normal attribute
	mov	es:[edi+edx],ax
	inc	ecx			; Next column
	cmp	cl,80			; At max?
	jb	short MOIC_exit		;  N: Go exit
	sub	ecx,ecx			;  Y: Reset to column zero and do a line fee
	inc	ebx			; Next row
	cmp	bl,25			; At max?
	jb	short MOIC_exit		;  N: Go exit
	mov	ebx,24			;  Y: Force to last line
	call	MonoOutInternalScroll
;
MOIC_exit:
	mov	ds:[DWMONOROW],ebx
	mov	ds:[DWMONOCOL],ecx
	pop	edi
	pop	edx
	pop	eax
	ret
MonoOutInternalChar	endp
;
;	MonoOutInternalScroll - Scroll the entire screen up one line
;
;	Entry:	ES:EDI = Monochrome memory map
;	Exit:	None
;
	public	MonoOutInternalScroll
MonoOutInternalScroll	proc
	pushad
	push	ds
	push	es
	pop	ds
	mov	esi,edi
	add	esi,160
	mov	ecx,(80*24*2)/4
	rep movsd
	mov	ecx,160/4
	mov	eax,007200720h
	rep stosd
	pop	ds
	popad
	ret
MonoOutInternalScroll	endp
;
;	MonoOutClear - Clear all of monochrome memory
;
;	Entry:	None
;	Exit:	None
;
	public	MonoOutClear
MonoOutClear	proc
	pushad
	push	es
	cld
	sub	edi,edi
	les	di,cs:[dwMonoAddr]
	mov	ecx,(80*25*2)/4
	mov	eax,07200720h
	rep stosd
	pop	es
	popad
	ret
MonoOutClear	endp
;
;	MonoOutInternalDumpReg - Display the contents of a named register
;
;	Entry:	CS:ESI = Pointer to embedded register name
;		EBX = Row
;		ECX = Column
;		ES:EDI = Mono memory map
;		EBP = Pointer to stack frame
;	Exit:	EBX = Updated to next row
;		ECX = Updated to next column
;		ESI = Updated to next character in string
;
	public	MonoOutInternalDumpReg
MonoOutInternalDumpReg	proc
	push	eax
	push	edx
;
; Find the register name in the register name table
;
	push	ebx
	push	ecx
	mov	ebx,offset monoRegs
MOIDR_10:
	movzx	ecx,byte ptr cs:[ebx]		; Get string length
	jecxz	short MOIDR_20			; End of table? Y: Error
	inc	ebx				; Point to string
	push	cx				; String length
	push	si				; Source string
	push	bx				; Dest string
	call	MonoOutMemICmp
	je	short MOIDR_30
	add	ebx,ecx
	inc	ebx
	inc	ebx
	jmp	short MOIDR_10
MOIDR_20:
	pop	ecx
	pop	ebx
	jmp	short MOIDR_exit
;
; Match found
;
MOIDR_30:
	add	esi,ecx				; Update string past name
	add	ebx,ecx
	movzx	ecx,byte ptr cs:[ebx]		; ECX = Size of data
	inc	ebx
	movzx	ebx,byte ptr cs:[ebx]
;
; Get the data from the stack frame. Note that the following instruction
; may have problems with some of the DOS extenders due to the high-order
; of EBP (from ESP) not set to 0000h:
;	mov	eax,ss:[ebp + ebx]
; Therefore, the following hack gets around this problem.
	push	di
	mov	di,bp
	mov	eax,ss:[di + bx]		; EAX = Data
	pop	di
;
; Convert the data (EAX = Data, ECX = # of Bytes) and display it
;
	mov	edx,ecx				; EDX = # of bytes
	shl	ecx,3
	sub	cl,4
	ror	eax,cl				; Shift high order nibble into position
	pop	ecx
	pop	ebx
	push	esi
	mov	esi,edx				; ESI = # of bytes
	shl	esi,1				; ESI = # of nibbles
MOIDR_40:
	push	eax
	and	eax,0Fh
	mov	al,byte ptr cs:[offset tblMonoHex + eax]
;
; Determine which device to write the character to and then
; send it on its way.
;
	cmp	dword ptr ds:[DWUSEMONOFLAG],1	; Send to mono device?
	jne	short MOIDR_50			;  N: Go send to COM port
	call	MonoOutInternalChar		;  Y: Write it to mono device
	jmp	short MOIDR_60
MOIDR_50:
	call	COMOutInternalChar
MOIDR_60:
;
	pop	eax
	rol	eax,4				; Shift next nibble into position
	dec	esi
	jnz	short MOIDR_40			; Next byte
	pop	esi
;
MOIDR_exit:
	pop	edx
	pop	eax
	ret
MonoOutInternalDumpReg	endp
;
;	MonoOutMemICmp - Compare two case-independent memory locations
;
;	Entry:	Stack frame:
;			08h	String length
;			06h	Source String
;			04h	Destination String
;			02h	Return Address
;			00h	BP
;	Exit:	Flag set as if a CMP operation occurred. The following
;		comparisons are valid:
;			JE or JZ	Strings equal
;			JB		Strings not equal, source arithmetically less than dest
;			JA		Strings not equal, source arithmetically greater than dest
;
;	Note:	This routine cleans up the stack after itself
;
	public	MonoOutMemICmp
MonoOutMemICmp	proc
	push	bp
	mov	bp,sp
	push	ax
	push	cx
	push	si
	push	di
	push	es
;
	mov	cx,[bp+08h]		; ECX = String length
	mov	si,[bp+06h]		; CS:SI = String1 source
	mov	di,[bp+04h]		; CS:DI = String2 source
	push	cs
	pop	es
MOMIC_10:
	lods byte ptr cs:[esi]		; Get character
	cmp	al,'a'			; Convert to upper case
	jb	short MOMIC_20
	cmp	al,'z'
	ja	short MOMIC_20
	sub	al,020h
MOMIC_20:
	scasb				; Character matches?
	jne	short MOMIC_30		;  N: Go exit
	loop	short MOMIC_10		; Doesn't affect flags
MOMIC_30:
	pop	es			; Doesn't affect flags
	pop	di
	pop	si
	pop	cx
	pop	ax
	pop	bp			; Doesn't affect flags
	ret	6			; Doesn't affect flags
MonoOutMemICmp	endp
;
;	_MonoString - Display a string to the mono monitor
;
;	Entry:	[BP+4] = Far pointer to string
;	Exit:	None
;
	public	_MonoString
_MonoString	proc	near
	push	bp
	mov	bp,sp
	pushfd
	pushad
	push	cs
	push	ds
	push	es
	push	ss
;
; Copy the string locally
;
IFDEF ROM
	push	SEG_HARDWIRED_DATA
	pop	es
	mov	di,SZBUFFER
ELSE
	push	cs
	pop	es
	mov	di,offset SZBUFFER
ENDIF
	lds	si,[bp+4]
	sub	cx,cx
_MonoString_10:
	inc	cx				; Count characters
	lodsb					; Get next character
	test	al,al				; Is it the end of the string?
	jz	short _MonoString_20		;  Y: Exit loop
	stosb					; Put next character
	cmp	cx,125				; Is it the end of the buffer?
	jb	short _MonoString_10		;  N: Go do next character
;
_MonoString_20:
	mov	ax,00A0Dh			; Add carriage return/linefeed
	stosw
	sub	al,al				; Add NULL
	stosb
;
IFDEF ROM
	mov	esi,SZBUFFER
ELSE
	mov	esi,offset SZBUFFER
ENDIF
	call	MonoOut
;
	pop	ax				; Discard SS
	pop	es
	pop	ds
	pop	ax				; Discard CS
	popad
	popfd
	pop	bp
	ret
_MonoString	endp
;
;	HandlerINT42 - INT 42h redirection handler
;
;	Entry:	Dependent on function
;	Exit:	Dependent on function
;
;	Note:	This is placed here so that a break point can
;		be set prior to diving into the system BIOS.
;
dwINT42		label	dword
	dw	0F065h
	dw	0F000h
;
	public	HandlerINT42
HandlerINT42	proc
	assume	ds:Seg0, es:nothing
	nop
	nop
	jmp	dword ptr cs:[dwINT42]
HandlerINT42	endp
;
;	COMOut - Dump a string to the COM port
;
;	Entry:	CS:ESI = Pointer to string
;		Stack frame has:
;			PUSHFD
;			PUSHAD
;			PUSH	CS
;			PUSH	DS
;			PUSH	ES
;			PUSH	SS
;			Return Address
;	Exit:	None
;
	public	COMOut
COMOut	proc
	mov	ebp,esp
	cld
;
IFDEF ROM
	push	SEG_HARDWIRED_DATA
ELSE
	push	cs
ENDIF
	pop	ds				; Set up data segment
;
COMOut_10:
	lods	byte ptr cs:[esi]
	test	al,al				; End of string?
	jz	short COMOut_1000		;  Y: Go exit
	cmp	al,'#'				; Embedded register?
	je	short COMOut_20			;  Y: Go handle
	call	COMOutInternalChar		;  N: Write char, as-is
	jmp	short COMOut_10
;
; Handle '#' character
COMOut_20:
	call	MonoOutInternalDumpReg
	jmp	short COMOut_10
;
COMOut_1000:
	ret
COMOut	endp
;
;	COMOutInternalChar - Write a character to the COM port
;
;	Entry:	AL = Character
;	Exit:	None
;
	public	COMOutInternalChar
COMOutInternalChar	proc
	pushad
;
	mov	ah,001h
	mov	dx,cs:[wCOMPort]
	int	14h
;
COIC_exit:
	popad
	ret
COMOutInternalChar	endp
;
ENDIF
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


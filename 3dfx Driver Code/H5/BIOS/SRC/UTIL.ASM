	page	57, 132
	title	Miscellaneous Utility Routines
	subttl	UTIL.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	UTIL.ASM - Miscellaneous utility routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	GetDSSeg0		Point DS at the BIOS data segment
;	GetESSeg0		Point ES at the BIOS data segment
;	GetESSegA000		Point ES at video memory at A0000h
;	GetESSegB000		Point ES at video memory at B0000h
;	GetESSegB800		Point ES at video memory at B8000h
;	GetDSSegA000		Point DS at video memory at A0000h
;	GetDSSegB800		Point DS at video memory at B8000h
;	FullCpuOn		Give the CPU full bandwidth to video memory
;	FullCpuOff		Share the CPU bandwidth with the CRTC
;	GetIndexRegister	Read the value of an indexed register
;	IsValidMode		Verify mode is legal
;	IsModeMono		See if mode is mono
;	IsTextMode		See if mode is a text mode
;	Is9DotMode		See if current mode is 9-dot mode
;	ShortBeep		Beep the speaker once, quickly
;	LongBeep		Beep the speaker once, slowly
;	WaitTicks		Wait a given number of timer ticks
;	Delay15us		Delay for a given number of 15us chunks
;	WaitBlank		Wait until display is in blanking
;	WaitNotBlank		Wait until display is not in blanking
;	WaitVerticalRetrace	Wait until display is in vertical retrace
;	WaitNotVerticalRetrace	Wait until display is not in vertical retrace
;	ATCOff			Turn off Attribute Controller
;	ATCOn			Turn on Attribute Controller
;	LoadIndexRange		Load a range of index registers
;	GetCRTCAddr		Get the actual address of the CRTC (independent of BIOS vars)
;	ErrorBeep		Beep the speaker a given number of times
;	GetModeDescPtr		Get pointer to the mode description table
;	GetOEMTablePtr		Get a pointer to the OEM data table
;	SafeInt1A		Check INT 1Ah and call it if exists
;	ResetATCFlipFlop	Reset the ATC into index state
;	GetRowOffset		Return the actual row offset of the current mode
;	GetOEMExtRegsPtr	Return the pointer to the extended registers
;	GetBPP			Get bits per pixel
;	IsPlanarMode		See if mode is a planar mode
;	VBEGetModeDescPtr	Get pointer to the mode description table based on VBE mode number
;	strcpy			Copy a NULL terminated string to another location
;	VBEMode2VMode		Convert a VESA mode number into an OEM mode number
;	VMode2VBEMode		Convert an OEM mode number into a VESA mode number
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/UTIL.ASM $
;	$Log: /devel/h5/BIOS/SRC/UTIL.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: UTIL.ASM
;
.MODEL TINY
.CODE
.386
;
	extrn	selBIOSData:word
	extrn	selA0000:word
	extrn	selB0000:word
	extrn	selB8000:word
	extrn	tblOEMData:byte
IF PCIBIOS
	extrn	OEMGetBusType:near
ENDIF
IF EXTENDED_BIOS
	extrn	bySizeEXTREGS:byte
ENDIF
;
;	GetDSSeg0 - Point DS at the BIOS data segment
;
;	Entry:	None
;	Exit:	DS = Seg0
;
	public	GetDSSeg0
GetDSSeg0	proc
	assume	ds:nothing, es:nothing
	mov	ds,cs:[selBIOSData]
	ret
GetDSSeg0	endp
;
;	GetESSeg0 - Point ES at the BIOS data segment
;
;	Entry:	None
;	Exit:	ES = Seg0
;
	public	GetESSeg0
GetESSeg0	proc
	assume	ds:nothing, es:nothing
	mov	es,cs:[selBIOSData]
	ret
GetESSeg0	endp
;
;	GetESSegA000 - Point ES at video memory at A0000h
;
;	Entry:	None
;	Exit:	ES = Segment A0000h
;
	public	GetESSegA000
GetESSegA000	proc
	assume	ds:nothing, es:nothing
	mov	es,cs:[selA0000]
	ret
GetESSegA000	endp
;
;	GetESSegB000 - Point ES at video memory at B0000h
;
;	Entry:	None
;	Exit:	ES = Segment B0000h
;
	public	GetESSegB000
GetESSegB000	proc
	assume	ds:nothing, es:nothing
	mov	es,cs:[selB0000]
	ret
GetESSegB000	endp
;
;	GetESSegB800 - Point ES at video memory at B8000h
;
;	Entry:	None
;	Exit:	ES = Segment B8000h
;
	public	GetESSegB800
GetESSegB800	proc
	assume	ds:nothing, es:nothing
	mov	es,cs:[selB8000]
	ret
GetESSegB800	endp
;
;	GetDSSegA000 - Point DS at video memory at A0000h
;
;	Entry:	None
;	Exit:	DS = Segment A0000h
;
	public	GetDSSegA000
GetDSSegA000	proc
	assume	ds:nothing, es:nothing
	mov	ds,cs:[selA0000]
	ret
GetDSSegA000	endp
;
;	GetDSSegB800 - Point DS at video memory at B8000h
;
;	Entry:	None
;	Exit:	DS = Segment B8000h
;
	public	GetDSSegB800
GetDSSegB800	proc
	assume	ds:nothing, es:nothing
	mov	ds,cs:[selB8000]
	ret
GetDSSegB800	endp
;
;	FullCpuOn - Give the CPU full bandwidth to video memory
;
;	Entry:	None
;	Exit:	None
;
;	AX, DX are trashed.
;
	public	FullCpuOn
FullCpuOn	proc
	assume	ds:nothing, es:nothing
	mov	dx,SEQ_INDEX
	mov	al,1
	call	GetIndexRegister
	or	ah,020h
	out	dx,ax
	ret
FullCpuOn	endp
;
;	FullCpuOff - Share the CPU bandwidth with the CRTC
;
;	Entry:	None
;	Exit:	None
;
;	AX, DX are trashed.
;
	public	FullCpuOff
FullCpuOff	proc
	assume	ds:nothing, es:nothing
	mov	dx,SEQ_INDEX
	mov	al,1
	call	GetIndexRegister
	and	ah,not 020h
	out	dx,ax
	ret
FullCpuOff	endp
;
;	GetIndexRegister - Read the value of an indexed register
;
;	Entry:	DX = I/O port
;		AL = Index to read
;	Exit:	AH = Data read
;
;	DX and AL are preserved
;
	public	GetIndexRegister
GetIndexRegister	proc
	assume	ds:nothing, es:nothing
	out	dx,al
	mov	ah,al
	inc	dx
	in	al,dx
	dec	dx
	xchg	al,ah
	ret
GetIndexRegister	endp
;
;	IsValidMode - Verify mode is legal
;
;	Entry:	AL = mode number
;	Exit:	Zero flag set as follows:
;		  ZF = Mode is valid
;		  NZ = Mode is not valid
;
	public	IsValidMode
IsValidMode	proc
	assume	ds:nothing, es:nothing
	pusha
	push	es
;
	call	GetModeDescPtr		; ES:BX = Pointer, CF = Error
	lahf				; AH = Flags (CF = bit 0)
	test	ah,01h			; Set zero flag opposite of carry
;
	pop	es
	popa
	ret
IsValidMode	endp
;
;	IsModeMono - See if mode is mono
;
;	Entry:	AL = mode number
;	Exit:	Zero flag set as follows:
;		  ZF = Mode is valid
;		  NZ = Mode is not valid
;
	public	IsModeMono
IsModeMono	proc
	assume	ds:nothing, es:nothing
	push	bx
	push	es
;
	call	GetModeDescPtr
	jc	short IsModeMono_exit
	cmp	es:[bx].MODEDESC.mdType,MDTYPE_MTEXT
	je	short IsModeMono_exit
	cmp	es:[bx].MODEDESC.mdType,MDTYPE_MGRAF
;
IsModeMono_exit:
	pop	es
	pop	bx
	ret
IsModeMono	endp
;
;	IsTextMode - See if mode is a text mode
;
;	Entry:	AL = mode number
;	Exit:	Zero flag set as follows:
;		  ZF = mode is text
;		  NZ = mode is graphics
;
	public	IsTextMode
IsTextMode	proc
	assume	ds:nothing, es:nothing
	push	bx
	push	es
;
	call	GetModeDescPtr
	jc	short IsTextMode_exit
	cmp	es:[bx].MODEDESC.mdType,MDTYPE_CTEXT
	je	short IsTextMode_exit
	cmp	es:[bx].MODEDESC.mdType,MDTYPE_MTEXT
;
IsTextMode_exit:
	pop	es
	pop	bx
	ret
IsTextMode	endp
;
;	Is9DotMode - See if current mode is 9-dot mode
;
;	Entry:	None
;	Exit:	Zero flag set as follows:
;		  ZF = 9-dot mode exists
;		  NZ = 8-dot mode exists
;
	public	Is9DotMode
Is9DotMode	proc
	assume	ds:nothing, es:nothing
	pusha
	mov	dx,SEQ_INDEX
	mov	al,1
	call	GetIndexRegister
	test	ah,1
	popa
	ret
Is9DotMode	endp
;
;	ShortBeep - Beep the speaker once, quickly
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;
	public	ShortBeep
ShortBeep	proc
	assume	ds:nothing, es:nothing
	pusha
	mov	al,0B6h
	out	43h,al
	mov	al,033h
	out	42h,al
	mov	al,005h
	out	42h,al
	in	al,061h
	push	ax
	or	al,003h
	out	61h,al
	mov	cx,4
	call	WaitTicks
	pop	ax
	out	61h,al
	popa
	ret
ShortBeep	endp
;
;	LongBeep - Beep the speaker once, slowly
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;
	public	LongBeep
LongBeep	proc
	assume	ds:nothing, es:nothing
	pusha
	mov	al,0B6h
	out	43h,al
	mov	al,033h
	out	42h,al
	mov	al,005h
	out	42h,al
	in	al,061h
	push	ax
	or	al,003h
	out	61h,al
	mov	cx,12
	call	WaitTicks
	pop	ax
	out	61h,al
	popa
	ret
LongBeep	endp
;
;	WaitTicks - Wait a given number of timer ticks
;
;	Entry:	CX = Number of ticks to wait
;	Exit:	None
;
;	All registers are preserved
;
;	Note:	Each tick is 18.2 times per second or once every
;		55 milliseconds (or 55000 microseconds). Therefore
;		the "Delay15us" routine needs to be called about
;		3700 times for each timer tick.
;
	public	WaitTicks
WaitTicks	proc
	assume	ds:nothing, es:nothing
	push	cx
;
WaitTicks_10:
	push	cx
	mov	cx,3700
	call	Delay15us
	pop	cx
	loop	WaitTicks_10
;
	pop	cx
	ret
WaitTicks	endp
;
;	Delay15us - Delay for a given number of 15us chunks
;
;	Entry:	CX = Number of 15 microsecond increments to delay
;	Exit:	None
;
;	All registers are preserved.
;
;	Note:	This delay function uses the memory refresh status
;		to provide a real-time timing reference.
;
	public	Delay15us
Delay15us	proc
	push	ax
;
	jcxz	Delay15us_exit
	sub	ah,ah
Delay15us_10:
	in	al,IODELAYPORT		; Read from unused I/O port for delay
	in	al,PORTB		; Get Port B status
	and	al,FLAG_REFRESHSTATUS	; Mask out refresh status
	cmp	al,ah			; Is it the same as previous
	je	short Delay15us_10
	mov	ah,al			; State has changed, save it
	loop	Delay15us_10
;
Delay15us_exit:
	pop	ax
	ret
Delay15us	endp
;
;	WaitBlank - Wait until display is in blanking
;
;	Entry:	DS = Seg0
;	Exit:	DX = Input Status 1 address
;
	public	WaitBlank
WaitBlank	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	cx
	call	GetCRTCAddr
	add	dl,06h
	sub	cx,cx
WaitBlank_10:
	in	al,dx
	test	al,1
	loopz	WaitBlank_10
	pop	cx
	pop	ax
	ret
WaitBlank	endp
;
;	WaitNotBlank - Wait until display is not in blanking
;
;	Entry:	DS = Seg0
;	Exit:	DX = Input Status 1 address
;
	public	WaitNotBlank
WaitNotBlank	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	cx
	call	GetCRTCAddr
	add	dl,06h
	sub	cx,cx
WaitNotBlank_10:
	in	al,dx
	test	al,1
	loopnz	WaitNotBlank_10
	pop	cx
	pop	ax
	ret
WaitNotBlank	endp
;
;	WaitVerticalRetrace - Wait until display is in vertical retrace
;
;	Entry:	DS = Seg0
;	Exit:	DX = Input Status 1 address
;
	public	WaitVerticalRetrace
WaitVerticalRetrace	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	cx
	mov	dx,[wCRTCAddr]
	add	dl,06h
	sub	cx,cx
WaitVerticalRetrace_10:
	in	al,dx
	test	al,8
	loopz	WaitVerticalRetrace_10
	pop	cx
	pop	ax
	ret
WaitVerticalRetrace	endp
;
;	WaitNotVerticalRetrace - Wait until display is not in vertical retrace
;
;	Entry:	DS = Seg0
;	Exit:	DX = Input Status 1 address
;
	public	WaitNotVerticalRetrace
WaitNotVerticalRetrace	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	cx
	mov	dx,[wCRTCAddr]
	add	dl,06h
	sub	cx,cx
WaitNotVerticalRetrace_10:
	in	al,dx
	test	al,8
	loopnz	WaitNotVerticalRetrace_10
	pop	cx
	pop	ax
	ret
WaitNotVerticalRetrace	endp
;
;	ATCOff - Turn off Attribute Controller
;
;	Entry:	None
;	Exit:	None
;
	public	ATCOff
ATCOff	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
;
	cli
	call	ResetATCFlipFlop	; Reset ATC to index state
	mov	dx,ATC_INDEX
	sub	al,al
	out	dx,al
	sti
;
	pop	dx
	pop	ax
	ret
ATCOff	endp
;
;	ATCOn - Turn on Attribute Controller
;
;	Entry:	None
;	Exit:	None
;
	public	ATCOn
ATCOn	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
;
	cli
	call	ResetATCFlipFlop	; Reset ATC to index state
	mov	dx,ATC_INDEX
	mov	al,20h
	out	dx,al
	call	ResetATCFlipFlop	; For compatibility with DOS EDIT
	sti
;
	pop	dx
	pop	ax
	ret
ATCOn	endp
;
;	LoadIndexRange - Load a range of index registers
;
;	Entry:	DS:SI = Pointer to parameters
;		BL = Index to start
;		CX = Number of registers to load
;		DX = I/O Address
;	Exit:	DS:SI = Updated pointer
;
	public	LoadIndexRange
LoadIndexRange	proc
	assume	ds:nothing, es:nothing
LoadIndexRange_10:
	mov	ah,bl
	lodsb
	xchg	al,ah
	out	dx,ax
	inc	bx
	loop	LoadIndexRange_10
	ret
LoadIndexRange	endp
;
;	GetCRTCAddr - Get the actual address of the CRTC (independent of BIOS vars)
;
;	Entry:	None
;	Exit:	DX = Address of CRTC (3D4h = color, 3B4h = mono)
;
;	Assume the VGA is awake.
;
	public	GetCRTCAddr
GetCRTCAddr	proc
	assume	ds:nothing, es:nothing
	push	ax
	mov	dx,MISC_INPUT
	in	al,dx
	mov	dx,CRTC_MINDEX			; Assume mono CRTC
	test	al,1				; Is CRTC mono?
	jz	short GetCRTCAddr_exit		;  Y: Go exit
	mov	dl,low CRTC_CINDEX		;  N: Return color CRTC
GetCRTCAddr_exit:
	pop	ax
	ret
GetCRTCAddr	endp
;
;	ErrorBeep - Beep the speaker a given number of times
;
;	Entry:	BX = Number of long beeps
;		CX = Number of short beeps
;	Exit:	None
;
	public	ErrorBeep
ErrorBeep	proc
	pusha
;
	push	cx
ErrorBeep_10:
	call	LongBeep
	mov	cx,2
	call	WaitTicks
	dec	bx
	jnz	ErrorBeep_10
	pop	cx
;
	mov	bx,cx
ErrorBeep_20:
	call	ShortBeep
	mov	cx,2
	call	WaitTicks
	dec	bx
	jnz	short ErrorBeep_20
;
	popa
	ret
ErrorBeep	endp
;
;	GetModeDescPtr - Get pointer to the mode description table
;
;	Entry:	AL = VGA Mode number
;	Exit:	ES:BX = Pointer to table (If error, BX = 0)
;		Carry flag set as follows:
;			NC = Pointer is valid
;			CF = Error (pointer invalid)
;
;	All other registers preserved.
;
	public	GetModeDescPtr
GetModeDescPtr	proc
	assume	ds:nothing, es:nothing
	call	GetOEMTablePtr
	les	bx,es:[bx].OEMDATA.oemModeDesc
GetModeDescPtr_10:
	cmp	es:[bx].MODEDESC.mdMode,0FFh	; End of table?
	je	short GetModeDescPtr_error	;  Y: It's an error
	cmp	es:[bx].MODEDESC.mdMode,al	; Mode matches?
	je	short GetModeDescPtr_20		;  Y: Go exit
	add	bx,size MODEDESC
	jmp	short GetModeDescPtr_10
GetModeDescPtr_20:
	clc
	ret
;
GetModeDescPtr_error:
	sub	bx,bx
	stc
	ret
GetModeDescPtr	endp
;
;	GetOEMTablePtr - Get a pointer to the OEM data table
;
;	Entry:	None
;	Exit:	ES:BX = Pointer to OEM data table
;
;	All other registers are preserved.
;
	public	GetOEMTablePtr
GetOEMTablePtr	proc
	assume	ds:nothing, es:nothing
	push	ax
	call	GetESSeg0			; ES = Seg0
	assume	es:Seg0
	les	bx,[lpSavePtr]
	assume	es:nothing
	les	bx,es:[bx].SAVEPTR.SecondarySavePtr
	les	bx,es:[bx].SAVEPTR2.OEMData
	mov	ax,es
	or	ax,bx				; NULL ptr?
	jz	short GetOEMTablePtr_100	;  Y: Go handle special
GetOEMTablePtr_exit:
	pop	ax
	ret
;
; ERROR: A NULL ptr is in our reserved field. Default to the existing
; table in the ROM.
;
GetOEMTablePtr_100:
	push	cs
	pop	es
	mov	bx,offset tblOEMData
	jmp	short GetOEMTablePtr_exit
GetOEMTablePtr	endp
;
IF PCIBIOS
;
;	SafeInt1A - Check INT 1Ah and call it if exists
;
;	Entry:	PCI BIOS function call parameters
;	Exit:	If INT 1Ah is non-zero
;			PCI BIOS function return values
;		Else
;			Carry flag set (CF = 1)
;
	public	SafeInt1A
SafeInt1A	proc
	assume	ds:nothing, es:nothing
	pusha
	push	ds
;
	call	OEMGetBusType			; AL = Bus type
	cmp	al,BUS_PCI			; PCI Bus?
	jne	short SafeInt1A_fail		;  N: Fail the INT 1Ah
;
	call	GetDSSeg0			; DS = Seg0
	lds	ax,ds:[1Ah*4]
	mov	bx,ds
	or	ax,bx				; Vector exists?
	jz	short SafeInt1A_fail		;  N: Fail the INT 1Ah
;
	pop	ds
	popa
	int	1Ah
	ret
;
SafeInt1A_fail:
	stc
	pop	ds
	popa
	ret
SafeInt1A	endp
;
ENDIF
;
;	ResetATCFlipFlop - Reset the ATC into index state
;
;	Entry:	None
;	Exit:	None
;
;	Assume interrupts are disabled.
;	All registers are preserved.
;
	public	ResetATCFlipFlop
ResetATCFlipFlop	proc
	push	ax
	push	dx
;
	mov	dx,INPUT_CSTATUS
	in	al,dx
	mov	dl,low INPUT_MSTATUS
	in	al,dx
;
	pop	dx
	pop	ax
	ret
ResetATCFlipFlop	endp
;
;	GetRowOffset - Return the actual row offset of the current mode
;
;	Entry:	DS = Seg0
;	Exit:	BP = Row offset in bytes
;
	public	GetRowOffset
GetRowOffset	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	bx
	push	es
;
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	bp,es:[bx].MODEDESC.mdRowOff	; Get row offset
;
	pop	es
	pop	bx
	pop	ax
	ret
GetRowOffset	endp
;
;	IsNullMode - Is there a request to set a mode of type NULL
;
;	Entry:	AX = Mode number
;	Exit:	Zero flag set as follows:
;			ZF = Null mode is requested
;			NZ = Not a null mode
;
	public IsNullMode
IsNullMode	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	bx
	push	es
;
	and	al,07Fh				; Mask off memory clear flag
	call	GetModeDescPtr			; Get pointer to MODEDESC entry
	cmp	es:[bx].MODEDESC.mdType,MDTYPE_NULL
;
	pop	es
	pop	bx
	pop	ax
	ret
IsNullMode	endp
;
IF EXTENDED_BIOS
;
;	GetOEMExtRegsPtr - Return the pointer to the extended registers
;
;	Entry:	AL = Mode number
;	Exit:	ES:BX = Ptr to EXTREGS table
;
	public	GetOEMExtRegsPtr
GetOEMExtRegsPtr	proc
	assume	ds:nothing, es:nothing
	push	ax
	call	GetModeDescPtr			; ES:BX = Ptr to MODEDESC
	mov	al,es:[bx].MODEDESC.mdIMode	; AL = Internal mode number
	call	GetOEMTablePtr			; ES:BX = OEM table
	les	bx,es:[bx].OEMDATA.oemExtRegs	; ES:BX = Ptr to extended regs
	mov	ah,cs:[bySizeEXTREGS]
	mul	ah				; Calculate offset of ext regs entry
	add	bx,ax				; ES:BX = Pointer to ext regs entry
	pop	ax
	ret
GetOEMExtRegsPtr	endp
;
ENDIF
;
IF VESA
;
;	GetBPP - Get bits per pixel
;
;	Entry:	AL = Mode number
;	Exit:	AL = Bits per pixel
;		Carry flag set as follows:
;			NC = BPP returned
;			CF = Error (AL = 0)
;
;	All other registers preserved.
;
	public	GetBPP
GetBPP	proc
	assume	ds:nothing, es:nothing
	push	bx
	push	es
;
	call	GetModeDescPtr
	mov	al,0				; ** DOESN'T AFFECT FLAGS **
	jc	short GetBPP_exit
	mov	al,es:[bx].MODEDESC.mdBPP	; ** DOESN'T AFFECT FLAGS **
;
GetBPP_exit:
	pop	es
	pop	bx
	ret
GetBPP	endp
;
;	IsPlanarMode - See if mode is a planar mode
;
;	Entry:	AL = mode number
;	Exit:	Zero flag set as follows:
;		  ZF = mode is planar
;		  NZ = mode is non-planar
;
	public	IsPlanarMode
IsPlanarMode	proc
	assume	ds:nothing, es:nothing
	push	bx
	push	es
;
	call	GetModeDescPtr
	jc	short IsPlanarMode_exit
	cmp	es:[bx].MODEDESC.mdType,MDTYPE_4BPP
	je	short IsPlanarMode_exit
	cmp	es:[bx].MODEDESC.mdType,MDTYPE_MGRAF
	je	short IsPlanarMode_exit
	cmp	es:[bx].MODEDESC.mdType,MDTYPE_1BPP
;
IsPlanarMode_exit:
	pop	es
	pop	bx
	ret
IsPlanarMode	endp
;
;	VBEGetModeDescPtr - Get pointer to the mode description table based on VBE mode number
;
;	Entry:	AX = VBE Mode number
;	Exit:	ES:BX = Pointer to table (If error, BX = 0)
;		Carry flag set as follows:
;			NC = Pointer is valid
;			CF = Error (pointer invalid)
;
;	All other registers preserved.
;
	public	VBEGetModeDescPtr
VBEGetModeDescPtr	proc
	assume	ds:nothing, es:nothing
	call	GetOEMTablePtr
	les	bx,es:[bx].OEMDATA.oemModeDesc
VBEGetModeDescPtr_10:
	cmp	es:[bx].MODEDESC.mdMode,0FFh	; End of table?
	je	short VBEGetModeDescPtr_error	;  Y: It's an error
	cmp	es:[bx].MODEDESC.mdVMode,ax	; VBE mode matches?
	je	short VBEGetModeDescPtr_20	;  Y: Go exit
	add	bx,size MODEDESC
	jmp	short VBEGetModeDescPtr_10
;
VBEGetModeDescPtr_20:
	clc
	ret
;
VBEGetModeDescPtr_error:
	sub	bx,bx
	stc
	ret
VBEGetModeDescPtr	endp
;
;	strcpy - Copy a NULL terminated string to another location
;
;	Entry:	DS:SI = Source
;		ES:DI = Destination
;	Exit:	None
;
;	AL, DI, SI are trashed.
;
	public	strcpy
strcpy	proc
	assume	ds:nothing, es:nothing
strcpy_10:
	lodsb
	stosb
	test	al,al			; Test for NULL terminator
	jnz	short strcpy_10		;  Y: Bail out
	ret
strcpy	endp
;
;	VBEMode2VMode - Convert a VESA mode number into an OEM mode number
;
;	Entry:	AX = 16-bit mode number
;	Exit:	AL = 8-bit mode number - 0FFh if doesn't exist
;		AH = Internal mode number - 0FFh if doesn't exist
;		Carry flag set as follows:
;		   NC = Function successful
;		   CF = Mode unavailable
;
;	All registers are preserved.
;	Access to extended registers get enabled here.
;
	public	VBEMode2VMode
VBEMode2VMode	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	es
	call	VBEGetModeDescPtr
	mov	ax,0FFFFh			; Assume error.
	jc	short VBEMode2VMode_exit	; Error? Y: Exit
	mov	al,es:[bx].MODEDESC.mdMode
	mov	ah,es:[bx].MODEDESC.mdIMode
VBEMode2VMode_exit:
	pop	es
	pop	bx
	ret
VBEMode2VMode	endp
;
;	VMode2VBEMode - Convert an OEM mode number into a VESA mode number
;
;	Entry:	AL = 8-bit mode number
;	Exit:	BX = VESA 16-bit mode number
;		Carry flag set as follows:
;			NC = VESA mode number found
;			CF = No matching VESA mode (BX = AL)
;
	public	VMode2VBEMode
VMode2VBEMode	proc
	assume	ds:Seg0, es:nothing
	push	es
	call	GetModeDescPtr
	jc	short VMode2VBEMode_exit	; Error? Y: Exit
	mov	bx,es:[bx].MODEDESC.mdVMode
	cmp	bx,0FFFFh
	jne	short VMode2VBEMode_exit
	mov	bl,al
	sub	bh,bh				; BX = OEM Mode number
	clc
VMode2VBEMode_exit:
	cmc
	pop	es
	ret
VMode2VBEMode	endp
;
ENDIF
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


	page	57, 132
	title	BIOS State and Functionality Profile Routines
	subttl	STATE.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	STATE.ASM - BIOS state and functionality profile routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	GetDccInfo		Return DCC information
;	GetFunctionalityInfo	Return information about the current state and capabilities of adapter
;	GetColorCount		Get number of colors in a given mode
;	GetPageCount		Get number of pages in a given mode
;	GetScanLineNumber	Return a representative number for the number of scan lines
;	GetPrimaryBlock		Return the primary font block
;	GetInfoState		Return the state of the info bytes
;	GetSavePtrState		Return the state of the saveptr variable
;	SaveRestoreState	Save/Restore the state of the VGA adapter
;	SaveHardwareState	Save the VGA registers to a system buffer
;	RestoreHardwareState	Restore the VGA registers from a system buffer
;	SaveBIOSState		Save the BIOS variables to a system buffer
;	RestoreBIOSState	Restore the BIOS variables from a system buffer
;	SaveDACState		Save the DAC registers to a system buffer
;	RestoreDACState		Restore the DAC registers from a system buffer
;	StoreIndexRange		Read and store a range of indexed registers into a memory buffer
;	StoreATCIndexRange	Read and store a range of ATC registers into a memory buffer
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/STATE.ASM $
;	$Log: /devel/h5/BIOS/SRC/STATE.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: STATE.ASM
;
.MODEL TINY
.CODE
.386
;
	extrn	GetIndexRegister:near
	extrn	GetCRTCAddr:near
	extrn	GetVerticalResolution:near
	extrn	GetOnePaletteRegister:near
	extrn	SetOnePaletteRegister:near
	extrn	LoadIndexRange:near
	extrn	GetModeDescPtr:near
	extrn	LoadATCRange:near
	extrn	GetDSSeg0:near
IF VESA
	extrn	OEMSaveExtState:near
	extrn	OEMRestoreExtState:near
	extrn	OEMPreSaveRestore:near
	extrn	OEMPostSaveRestore:near
ENDIF
;
	extrn	tblFunctionality:byte
;
;	Function 1Ah:
;	GetDccInfo - Return DCC information
;
;	Entry:	DS, SI are pushed on stack
;		DS = Seg0
;		AL = DCC Subfunction
;			0 = Get DCC
;			1 = Set DCC
;				BL = Active display code
;				BH = Secondary display code
;	Exit:	AL = 1Ah
;		Subfunction 0:
;			BL = Active display code
;			BH = Secondary display code
;
	public	GetDccInfo
GetDccInfo	proc
	assume	ds:Seg0, es:nothing
	cmp	al,1
	ja	GetDccInfo_exit
	mov	ah,al				; Doesn't touch flags...
	je	GetDccInfo_10
	mov	bl,[byDCC]
	mov	bh,al
GetDccInfo_10:
	mov	al,byte ptr [wCRTCAddr]
	lds	si,[lpSavePtr]
	assume	ds:nothing
	lds	si,[si].SAVEPTR.SecondarySavePtr
	lds	si,[si].SAVEPTR2.DCCPtr		; ...Doesn't touch flags
	je	SetDccInfo
	shl	bx,1
	mov	bx,[si + bx + 4]		; Get DCC entry
	xchg	bl,bh
	test	bh,bh				; Secondary exists?
	jz	GetDccInfo_40			;  N: Go exit
;
; Figure out primary vs. secondary based on what's active now
;
	cmp	al,low CRTC_CINDEX		; Color mode active?
	jne	GetDccInfo_20			;  N: Go handle mono logic
	test	bl,1				; Primary is color?
	jz	GetDccInfo_40			;  Y: Go exit
	jmp	short GetDccInfo_30		;  N: Go XCHG monitor codes
;
; Monochrome mode active
;
GetDccInfo_20:
	test	bl,1				; Primary is mono?
	jnz	GetDccInfo_40			;  Y: Go exit
GetDccInfo_30:
	xchg	bl,bh				;  N: XCHG monitor codes
;
GetDccInfo_40:
	mov	ax,01A1Ah
GetDccInfo_exit:
	pop	ds
	pop	si
	iret
;
; At this point:
;
;	AH = 1
;	AL = crtc_addr (low byte)
;	BL = Active display code
;	BH = Secondary display code
;	DS:SI = Pointer to DCC table
;
SetDccInfo:
	push	cx
	sub	ch,ch
	mov	cl,[si]
	add	si,4
	add	si,cx
	add	si,cx
	dec	si
	dec	si
	std
SetDccInfo_10:
	lodsw
	cmp	ax,bx
	je	SetDccInfo_20
	xchg	al,ah
	cmp	ax,bx
	je	SetDccInfo_20
	loop	SetDccInfo_10
	jmp	short SetDccInfo_30
SetDccInfo_20:
	dec	cx
	call	GetDSSeg0			; Get BIOS data segment
	assume	ds:Seg0
	mov	[byDCC],cl
SetDccInfo_30:
	pop	cx
	jmp	short GetDccInfo_40
GetDccInfo	endp
;
;	Function 1Bh:
;	GetFunctionalityInfo - Return information about the current state and capabilities of adapter
;
;	Entry:	DS, SI are pushed on stack
;		DS = Seg0
;		BX = 0
;		ES:DI = Pointer to target buffer (size = 40h bytes)
;	Exit:	AL = 1Bh
;
	public	GetFunctionalityInfo
GetFunctionalityInfo	proc
	assume	ds:Seg0, es:nothing
	test	bx,bx
	jnz	GetFunctionalityInfo_exit
	push	bx
	push	cx
	push	dx
	push	di
;
	mov	ax,offset tblFunctionality
	stosw
	mov	ax,cs
	stosw
	mov	si,offset byMode
	mov	cx,(offset byCGA3D9 - offset byMode) + 1
	rep movsb
	mov	al,[byRows]
	inc	ax			; "INC AX" to save a byte
	stosb
	mov	ax,[wCharHeight]
	stosw
	mov	ax,01A00h
	int	10h
	mov	ax,bx
	stosw
	mov	al,[byMode]
	call	GetColorCount
	stosw
	mov	al,[byMode]
	call	GetPageCount
	stosb
	call	GetScanLineNumber
	stosb
	call	GetPrimaryBlock
	stosw
	call	GetInfoState
	stosb
	sub	ax,ax
	stosw
	stosb
	mov	al,3
	stosb
	call	GetSavePtrState
	stosb
	mov	cx,13
	sub	ax,ax
	rep stosb
;
	pop	di
	pop	dx
	pop	cx
	pop	bx
	mov	ax,01Bh
GetFunctionalityInfo_exit:
	pop	ds
	pop	si
	iret
GetFunctionalityInfo	endp
;
;	GetColorCount - Get number of colors in a given mode
;
;	Entry:	AL = Mode number
;		DS = Seg0
;	Exit:	AX = Number of colors in current mode
;
;	BX and CX are trashed
;
	public	GetColorCount
GetColorCount	proc
	assume	ds:Seg0, es:nothing
	push	es
	call	GetModeDescPtr				; Requires AL=mode on entry
	mov	ax,0					; If error, assume mono (no flags affected by "MOV AX,0")
	jc	GetColorCount_exit			; Error? Y: Go exit
	cmp	byte ptr [wCRTCAddr],low CRTC_MINDEX	; Is it a mono mode?
	je	GetColorCount_exit			;  Y: Report zero colors
	mov	cl,es:[bx].MODEDESC.mdBPP		;  N: Retrieve BPP
	mov	ax,0001h
	shl	ax,cl				; AX = Number of colors
	test	ax,ax				; Did HiColor & TrueColor modes overflow?
	jnz	GetColorCount_exit		;  N: Got valid value, go exit
	dec	ax				;  Y: Flag AX as 0FFFFh
GetColorCount_exit:
	pop	es
	ret
GetColorCount	endp
;
;	GetPageCount - Get number of pages in a given mode
;
;	Entry:	AL = Mode number
;	Exit:	AL = Number of pages in current mode
;
;	BX is trashed
;
	public	GetPageCount
GetPageCount	proc
	assume	ds:nothing, es:nothing
	push	es
	call	GetModeDescPtr
	mov	al,01h				; **DOESN'T AFFECT FLAGS** If error, assume 1 page
	jc	GetPageCount_exit		; Error? Y: Go exit
	mov	al,es:[bx].MODEDESC.mdPages	;  N: Retrieve pages
GetPageCount_exit:
	pop	es
	ret
GetPageCount	endp
;
;	GetScanLineNumber - Return a representative number for the number of scan lines
;
;	Entry:	DS = Seg0
;	Exit:	AL = Scan line ID
;			0 = 200 scan lines
;			1 = 350 scan lines
;			2 = 400 scan lines
;			3 = 480 scan lines
;			4 - 255 = reserved
;
;	CX, DX are trashed
;
	public	GetScanLineNumber
GetScanLineNumber	proc
	assume	ds:Seg0, es:nothing
;
; BEGIN KLUDGE - Mode 13h is a double scanned mode but does it without
; actually setting the double scan bit. Instead, it uses the character
; cell height (CRTC[9].0..4) to scan out each row twice.
;
	sub	dx,dx			; Assume 200 scan line mode
	cmp	[byMode],13h		; Mode 13h?
	je	GetScanLineNumber_10	;  Y: Got it, go exit
;
; END KLUDGE
;
	call	GetVerticalResolution	; DX = 3x4h
	sub	dx,dx			; Assume 200 scan line mode
	cmp	ax,200			; 200 scans?
	je	GetScanLineNumber_10	;  Y: Got it, go exit
	inc	dx
	cmp	ax,350			; 350 scans?
	je	GetScanLineNumber_10	;  Y: Got it, go exit
	inc	dx
	cmp	ax,400			; 400 scans?
	je	GetScanLineNumber_10	;  Y: Got it, go exit
	inc	dx			;  N: Assume 480 scans
GetScanLineNumber_10:
	mov	ax,dx
	ret
GetScanLineNumber	endp
;
;	GetPrimaryBlock - Return the primary font block
;
;	Entry:	None
;	Exit:	AX = Font block #
;
;	DX is trashed
;
	public	GetPrimaryBlock
GetPrimaryBlock	proc
	assume	ds:nothing, es:nothing
	mov	dx,SEQ_INDEX
	mov	al,3
	call	GetIndexRegister
	mov	al,ah
	and	ax,01003h
	shr	ah,2
	or	al,ah
	cbw
	ret
GetPrimaryBlock	endp
;
;	GetInfoState - Return the state of the info bytes
;
;	Entry:	DS = Seg0
;	Exit:	AL = State information
;			Bit 0 = 1: All modes all display active
;			Bit 1 = 1: Summing active
;			Bit 2 = 1: Monochrome display attached
;			Bit 3 = 1: Default palette load disabled
;			Bit 4 = 1: Cursor emulation active
;			Bit 5 = 0: Intensity
;				1: Blinking
;			Bit 6 - 7 = Reserved (set to 0)
;
;	BX is trashed
;
	public	GetInfoState
GetInfoState	proc
	assume	ds:Seg0, es:nothing
	mov	bl,010h
	call	GetOnePaletteRegister
	and	bh,008h
	shl	bh,2
	mov	al,[fEGAInfo]
	mov	ah,[fVGAInfo]
	and	ax,00F01h
	shl	al,4
	or	al,ah
	or	al,bh
	xor	al,10h			; Invert bit 4
	ret
GetInfoState	endp
;
;	GetSavePtrState - Return the state of the saveptr variable
;
;	Entry:	DS = Seg0
;	Exit:	AL = Saveptr state
;			Bit 0 = 1: 512 character set active
;			Bit 1 = 1: Dynamic save area active
;			Bit 2 = 1: Auxiliary font override active
;			Bit 3 = 1: Graphics font override active
;			Bit 4 = 1: Palette override active
;			Bit 5 = 1: DCC extension active
;
;	BX, DX are trashed
;
	public	GetSavePtrState
GetSavePtrState	proc
	assume	ds:Seg0, es:nothing
	push	es
	mov	dx,SEQ_INDEX
	mov	al,3
	call	GetIndexRegister
	sub	al,al			; Assume 512 char set not active
	test	ah,ah			; 512 char set active?
	jz	GetSavePtrState_10	;  N: Continue
	inc	ax			;  Y: Set bit 0 = 1
GetSavePtrState_10:
	les	bx,[lpSavePtr]
	les	bx,es:[bx].SAVEPTR.SaveAreaPtr
	mov	dx,es
	or	dx,bx			; Dynamic save area active?
	jz	GetSavePtrState_20	;  N: Continue
	or	al,2			;  Y: Set bit 1 = 1
GetSavePtrState_20:
	les	bx,[lpSavePtr]
	les	bx,es:[bx].SAVEPTR.AuxTextFontPtr
	mov	dx,es
	or	dx,bx			; Auxiliary font override active?
	jz	GetSavePtrState_30	;  N: Continue
	or	al,4			;  Y: Set bit 2 = 1
GetSavePtrState_30:
	les	bx,[lpSavePtr]
	les	bx,es:[bx].SAVEPTR.AuxGraphicsFontPtr
	mov	dx,es
	or	dx,bx			; Graphics font override active?
	jz	GetSavePtrState_40	;  N: Continue
	or	al,8			;  Y: Set bit 3 = 1
GetSavePtrState_40:
	les	bx,[lpSavePtr]
	les	bx,es:[bx].SAVEPTR.SecondarySavePtr
	les	bx,es:[bx].SAVEPTR2.PaletteProfilePtr
	mov	dx,es
	or	dx,bx			; Palette override active?
	jz	GetSavePtrState_50	;  N: Continue
	or	al,010h			;  Y: Set bit 4 = 1
GetSavePtrState_50:
	les	bx,[lpSavePtr]
	les	bx,es:[bx].SAVEPTR.SecondarySavePtr
	les	bx,es:[bx].SAVEPTR2.DCCPtr
	mov	dx,es
	mov	bx,cs
	cmp	dx,bx			; DCC table is ours?
	je	GetSavePtrState_60	;  Y: Continue
	or	al,020h			;  N: Set bit 5 = 1
GetSavePtrState_60:
	pop	es
	ret
GetSavePtrState	endp
;
;	Function 1Ch:
;	SaveRestoreState - Save/Restore the state of the VGA adapter
;
;	Entry:	AL = Subfunction code
;			0 - Return size of save/restore buffer
;				CX = Requested states
;			1 - Save state
;				CX = Requested states
;				ES:BX = Pointer to buffer
;			2 - Restore state
;				CX = Requested states
;				ES:BX = Pointer to buffer
;	Exit:	AL = 1Ch (function supported)
;
HWSTATE		struc
  SEQ_Index		db	?
  CRTC_Index		db	?
  GDC_Index		db	?
  ATC_Index		db	?
  AR14			db	?
  SEQ			db	size SEQ dup (?)
  Misc			db	?
  CRTC			db	size CRTC dup (?)
  ATC			db	size ATC dup (?)
  GDC			db	size GDC dup (?)
  CRTC_Address		dw	?
  Latch0		db	?
  Latch1		db	?
  Latch2		db	?
  Latch3		db	?
HWSTATE		ends
;
BIOSSTATE	struc
  Video_Equip		db	?
  DataBlock1		db	30 dup (?)
  DataBlock2		db	7 dup (?)
  SavePtr		dd	?
  INT05			dd	?
  INT1D			dd	?
  INT1F			dd	?
  INT43			dd	?
BIOSSTATE	ends
;
DACSTATE	struc
  RWFlag		db	?
  Index			db	?
  DACMask		db	?
  Data			db	3*256 dup (?)
DACSTATE	ends
;
	public	tblSaveRestoreBufSize
tblSaveRestoreBufSize	label	byte
	db	000h
	db	(((size HWSTATE) + 32 + 63) / 64)
	db	(((size BIOSSTATE) + 32 + 63) / 64)
	db	((((size HWSTATE) + (size BIOSSTATE)) + 32 + 63) / 64)
	db	(((size DACSTATE) + 32 + 63) / 64)
	db	((((size HWSTATE) + (size DACSTATE)) + 32 + 63) / 64)
	db	((((size BIOSSTATE) + (sizE DACSTATE)) + 32 + 63) / 64)
	db	((((size HWSTATE) + (size BIOSSTATE) + (size DACSTATE)) + 32 + 63) / 64)
IF VESA
	db	LENGTH_EXTSTATE / 64
	db	((LENGTH_EXTSTATE + (size HWSTATE) + 32 + 63) / 64)
	db	((LENGTH_EXTSTATE + (size BIOSSTATE) + 32 + 63) / 64)
	db	(((LENGTH_EXTSTATE + (size HWSTATE) + (size BIOSSTATE)) + 32 + 63) / 64)
	db	((LENGTH_EXTSTATE + (size DACSTATE) + 32 + 63) / 64)
	db	(((LENGTH_EXTSTATE + (size HWSTATE) + (size DACSTATE)) + 32 + 63) / 64)
	db	(((LENGTH_EXTSTATE + (size BIOSSTATE) + (sizE DACSTATE)) + 32 + 63) / 64)
	db	(((LENGTH_EXTSTATE + (size HWSTATE) + (size BIOSSTATE) + (size DACSTATE)) + 32 + 63) / 64)
ENDIF
;
	public	SaveRestoreState
SaveRestoreState	proc
	assume	ds:Seg0, es:nothing
	test	al,al
	jnz	SaveRestoreState_10
	mov	bx,cx
IF VESA
	and	bx,0000Fh
ELSE
	and	bx,00007h
ENDIF
	mov	bl,cs:[tblSaveRestoreBufSize + bx]
	mov	ax,0001Ch		; Flag function as supported
SaveRestoreState_exit_unsupported:
	pop	ds
	pop	si
	iret
;
SaveRestoreState_10:
IF VESA
	call	OEMPreSaveRestore	; Set any Registers needed for access
ENDIF
	cmp	al,2					; Restore state?
	ja	SaveRestoreState_exit_unsupported	;  >: Invalid value
	je	SaveRestoreState_100			;  =: Go handle
;
; Save state
;
	push	bx
	push	dx
	push	di
;
	mov	di,bx
	add	di,32
	test	cl,1			; Save hardware state?
	jz	SaveRestoreState_20	;  N: Go test next state
	mov	es:[bx],di		;  Y: Save address of buffer...
	call	SaveHardwareState	;     ...and save the state
SaveRestoreState_20:
	test	cl,2			; Save BIOS state?
	jz	SaveRestoreState_30	;  N: Go test next state
	mov	es:[bx+2],di		;  Y: Save address of buffer...
	call	SaveBIOSState		;     ...and save the state
SaveRestoreState_30:
	test	cl,4			; Save DAC state?
	jz	SaveRestoreState_40	;  N: Go test next state
	mov	es:[bx+4],di		;  Y: Save address of buffer...
	call	SaveDACState		;     ...and save the state
SaveRestoreState_40:
IF VESA
	test	cl,8			; Save extended register state?
	jz	SaveRestoreState_50	;  N: Go exit
	mov	es:[bx+6],di		;  Y: Save address of buffer...
	call	OEMSaveExtState		;     ...and save the state
SaveRestoreState_50:
	call	OEMPostSaveRestore
ENDIF
;
	mov	ax,0001Ch		; Function supported
	pop	di
	pop	dx
	pop	bx
	pop	ds
	pop	si
	iret
;
; Restore state
;
SaveRestoreState_100:
IF VESA
	call	OEMPreSaveRestore	; Set any Registers needed for access
ENDIF
	push	bx
	push	dx
	test	cl,1			; Restore hardware state?
	jz	SaveRestoreState_110	;  N: Go test next state
	mov	si,es:[bx]		;  Y: Get address of the buffer...
	call	RestoreHardwareState	;     ...and restore the state
SaveRestoreState_110:
	test	cl,2			; Restore BIOS state?
	jz	SaveRestoreState_120	;  N: Go test next state
	mov	si,es:[bx+2]		;  Y: Get address of the buffer...
	call	RestoreBIOSState	;     ...and restore the state
SaveRestoreState_120:
	test	cl,4			; Restore DAC state?
	jz	SaveRestoreState_130	;  N: Go test next state
	mov	si,es:[bx+4]		;  Y: Get address of the buffer...
	call	RestoreDACState		;     ...and restore the state
SaveRestoreState_130:
IF VESA
	test	cl,8			; Restore extended register state?
	jz	SaveRestoreState_140	;  N: Go exit
	mov	si,es:[bx+6]		;  Y: Get address of the buffer...
	call	OEMRestoreExtState	;     ...and restore the state
SaveRestoreState_140:
	call	OEMPostSaveRestore
ENDIF
	mov	ax,0001Ch
	pop	dx
	pop	bx
	pop	ds
	pop	si
	iret
SaveRestoreState	endp
;
;	SaveHardwareState - Save the VGA registers to a system buffer
;
;	Entry:	DS = Seg0
;		ES:DI = Pointer to save buffer
;	Exit:	ES:DI = Updated pointer
;
;	AX, DX are trashed
;	BX, CX, BP, ES are preserved
;
	public	SaveHardwareState
SaveHardwareState	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	bp
;
; Save Indexes
;
	mov	dx,SEQ_INDEX
	in	al,dx
	stosb
	call	GetCRTCAddr
	push	dx				; Save CRTC Index for later
	in	al,dx
	stosb
	mov	dl,low GDC_INDEX
	in	al,dx
	stosb
	mov	dl,low ATC_INDEX
	in	al,dx
	stosb
;
; Save ATC 14h
;
	mov	bl,14h
	call	GetOnePaletteRegister
	mov	al,bh
	stosb
;
; Save Sequencer and Miscellaneous registers
;
	mov	dl,low SEQ_INDEX
	mov	ah,1
	mov	cx,4
	call	StoreIndexRange
	mov	dl,low MISC_INPUT
	in	al,dx
	stosb
;
; Save CRTC registers
;
	pop	dx				; Get CRTC Index...
	push	dx				;  ...and save it again
	mov	cx,25
	mov	ah,ch
	call	StoreIndexRange
;
; Save Attribute Controller registers
;
	mov	cx,20
	mov	ah,ch
	call	StoreATCIndexRange
;
; Save Graphics Data Controller registers
;
	mov	dl,low GDC_INDEX
	mov	cx,9
	mov	ah,ch
	call	StoreIndexRange
;
; CRTC Address and latches
;
	pop	bp				; Get CRTC address
	mov	ax,bp
	stosw
	mov	dl,low GDC_INDEX
	mov	al,4				; Read map select
	call	GetIndexRegister
	push	ax				; Save map select
	mov	bx,0004h
	mov	cx,4
SaveHardwareState_10:
	mov	ax,bx
	out	dx,ax				; Set map select
	xchg	dx,bp				; DX = CRTC Index
	mov	al,22h
	call	GetIndexRegister
	mov	al,ah
	stosb					; Store latch value
	xchg	dx,bp				; DX = GDC Index
	inc	bh
	loop	SaveHardwareState_10
	pop	ax
	out	dx,ax				; Restore map select
;
	pop	bp
	pop	cx
	pop	bx
	ret
SaveHardwareState	endp
;
;	RestoreHardwareState - Restore the VGA registers from a system buffer
;
;	Entry:	DS = Seg0
;		ES:SI = Pointer to save buffer
;	Exit:	None
;
;	AX, DX are trashed
;	BX, CX, DS, ES are preserved
;
	public	RestoreHardwareState
RestoreHardwareState	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	ds
;
	assume	ds:nothing
	push	es
	pop	ds
	push	si			; Save pointer to start of save area
;
; Set ATC Index 14
;
	add	si,4			; Skip indexes for now
	lodsb
	mov	bh,al
	mov	bl,14h
	push	si
	call	SetOnePaletteRegister	; Trashes SI
	pop	si
;
; Set Sequencer and Miscellaneous registers
;
	mov	dx,SEQ_INDEX
	mov	ax,0100h
	out	dx,ax			; Sync reset
	mov	bl,1
	mov	cx,4
	call	LoadIndexRange
	mov	dl,low MISC_OUTPUT
	lodsb
	out	dx,al
	mov	dl,low SEQ_INDEX
	mov	ax,0300h
	out	dx,ax			; End sync reset
;
; Set CRTC registers
;
	call	GetCRTCAddr
	mov	ax,02011h
	out	dx,ax			; Unlock CRTC registers
	mov	cx,25
	mov	bl,ch
	call	LoadIndexRange
;
; Set ATC registers
;
	push	dx			; Save CRTC address
	add	dl,6
	mov	cx,20
	mov	bl,ch
	call	LoadATCRange
;
; Set GDC Registers
;
	mov	dl,low GDC_INDEX
	mov	cx,9
	mov	bl,ch
	call	LoadIndexRange
;
; Load the latches
;
;
; Set the indexes
;
	pop	dx			; Restore CRTC address
	pop	bx			; Restore pointer to start of save area
	mov	al,[bx].HWSTATE.CRTC_Index
	out	dx,al
	add	dl,6
	cli
	in	al,dx
	mov	dl,low ATC_INDEX
	mov	al,[bx].HWSTATE.ATC_Index
	out	dx,al
	sti
	mov	dl,low SEQ_INDEX
	mov	al,[bx].HWSTATE.SEQ_Index
	out	dx,al
	mov	dl,low GDC_INDEX
	mov	al,[bx].HWSTATE.GDC_Index
	out	dx,al
;
	pop	ds
	pop	cx
	pop	bx
	ret
RestoreHardwareState	endp
;
;	SaveBIOSState - Save the BIOS variables to a system buffer
;
;	Entry:	DS = Seg0
;		ES:DI = Pointer to save buffer
;	Exit:	ES:DI = Updated pointer
;
;	AX, SI are trashed
;	BX, CX is preserved
;
	public	SaveBIOSState
SaveBIOSState	proc
	assume	ds:Seg0, es:nothing
	push	cx
;
	mov	al,[fEquip]
	and	al,30h
	stosb
	mov	cx,size BIOSSTATE.DataBlock1
	mov	si,offset Seg0:byMode
	rep movsb
	mov	cx,size BIOSSTATE.DataBlock2
	mov	si,offset Seg0:byRows
	rep movsb
	mov	si,offset Seg0:lpSavePtr
	movsw
	movsw
	mov	si,offset Seg0:vecINT05
	movsw
	movsw
	mov	si,offset Seg0:vecINT1D
	movsw
	movsw
	mov	si,offset Seg0:vecINT1F
	movsw
	movsw
	mov	si,offset Seg0:vecINT43
	movsw
	movsw
;
	pop	cx
	ret
SaveBIOSState	endp
;
;	RestoreBIOSState - Restore the BIOS variables from a system buffer
;
;	Entry:	DS = Seg0
;		ES:SI = Pointer to save buffer
;	Exit:	none
;
;	AX, SI are trashed
;	BX, CX, DI, DS, ES are preserved
;
	public	RestoreBIOSState
RestoreBIOSState	proc
	assume	ds:Seg0, es:nothing
	push	cx
	push	di
	push	ds
	push	es
;
	mov	al,[fEquip]
	and	al,030h
	or	es:[si],al
	mov	[fEquip],al
	inc	si
;
; XCHG	DS, ES
;
	push	ds
	push	es
	pop	ds
	pop	es
;
	mov	cx,size BIOSSTATE.DataBlock1
	mov	di,offset Seg0:byMode
	rep movsb
;
	mov	cx,size BIOSSTATE.DataBlock2
	mov	di,offset Seg0:byRows
	rep movsb
;
	mov	di,offset Seg0:lpSavePtr
	movsw
	movsw
	mov	di,offset Seg0:vecINT05
	movsw
	movsw
	mov	di,offset Seg0:vecINT1D
	movsw
	movsw
	mov	di,offset Seg0:vecINT1F
	movsw
	movsw
	mov	di,offset Seg0:vecINT43
	movsw
	movsw
;
	pop	es
	pop	ds
	pop	di
	pop	cx
	ret
RestoreBIOSState	endp
;
;	SaveDACState - Save the DAC registers to a system buffer
;
;	Entry:	DS = Seg0
;		ES:DI = Pointer to save buffer
;	Exit:	ES:DI = Updated pointer
;
;	AX, DX are trashed
;	BX, CX, ES are preserved
;
	public	SaveDACState
SaveDACState	proc
	assume	ds:Seg0, es:nothing
	push	cx
;
	mov	dx,DAC_RINDEX
	in	al,dx
	stosb
	inc	dx
	test	al,3
	in	al,dx			; Doesn't touch flags
	jz	SaveDACState_10
	dec	ax			; Word DEC saves a byte
SaveDACState_10:
	stosb
	mov	dl,low DAC_MASK
	in	al,dx
	stosb
	inc	dx
	sub	al,al
	out	dx,al
	inc	dx
	inc	dx
	mov	cx,256*3
SaveDACState_20:
	in	al,dx
	stosb
	loop	SaveDACState_20
;
	pop	cx
	ret
SaveDACState	endp
;
;	RestoreDACState - Restore the DAC registers from a system buffer
;
;	Entry:	DS = Seg0
;		ES:SI = Pointer to save buffer
;	Exit:	ES:SI = Updated pointer
;
;	AX, DX are trashed
;	BX, CX, DS are preserved
;
	public	RestoreDACState
RestoreDACState	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	ds
;
	push	es
	pop	ds
	push	si			; Save pointer to start of save area
;
	mov	dx,DAC_MASK
	inc	si
	inc	si
	lodsb
	out	dx,al
	inc	dx
	inc	dx
	sub	al,al
	out	dx,al
	mov	cx,256*3
	inc	dx
RestoreDACState_10:
	lodsb
	out	dx,al
	loop	RestoreDACState_10
;
; Set index
;
	pop	bx			; Restore pointer to start of save area
	test	[bx].DACSTATE.RWFlag,003h
	dec	dx
	jz	RestoreDACState_20
	dec	dx
RestoreDACState_20:
	mov	al,[bx].DACSTATE.Index
	out	dx,al
;
	pop	ds
	pop	cx
	pop	bx
	ret
RestoreDACState	endp
;
;	StoreIndexRange - Read and store a range of indexed registers into a memory buffer
;
;	Entry:	AH = Index to start
;		CX = Number of registers to read
;		DX = Port address of indexed register
;		ES:DI = Pointer to buffer
;	Exit:	ES:DI = Updated pointer to buffer
;
;	AX and CX are trashed.
;
	public	StoreIndexRange
StoreIndexRange	proc
StoreIndexRange_10:
	mov	al,ah
	out	dx,al
	inc	dx
	in	al,dx
	stosb
	dec	dx
	inc	ah
	loop	StoreIndexRange_10
	ret
StoreIndexRange	endp
;
;	StoreATCIndexRange - Read and store a range of ATC registers into a memory buffer
;
;	Entry:	AH = Index to start
;		CX = Number of registers to read
;		ES:DI = Pointer to buffer
;	Exit:	ES:DI = Updated pointer to buffer
;
;	AX, CX, and DX are trashed.
;
	public	StoreATCIndexRange
StoreATCIndexRange	proc
	mov	dx,INPUT_MSTATUS
	cli
	in	al,dx
	mov	dl,low INPUT_CSTATUS
	in	al,dx				; Puts toggle in INDEX state
	mov	dl,low ATC_INDEX
StoreATCIndexRange_10:
	mov	al,ah
	out	dx,al				; Puts toggle in DATA state
	inc	dx
	in	al,dx				; Toggle untouched
	stosb
	dec	dx
	out	dx,al				; Puts toggle in INDEX state
	inc	ah
	loop	StoreATCIndexRange_10
	mov	al,20h				; Turn ATC back on
	out	dx,al
	sti
	ret
StoreATCIndexRange	endp
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


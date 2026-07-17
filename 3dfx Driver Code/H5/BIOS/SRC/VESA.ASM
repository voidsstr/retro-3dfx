	page	57, 132
	title	Initialization and Dispatch Routines
	subttl	VESA.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	VESA.ASM - Initialization and dispatch routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	HandlerVESA			Call requested VESA subfunction
;	VBE_ReturnControllerInfo	Return VBEINFOBLOCK data structure
;	VBE_ReturnModeInfo		Return MODEINFOBLOCK data structure for a specific mode number
;	VBERMI_Init			Determine if mode exists and initialize CPU registers
;	VBERMI_ModeAttribute		Create mode attribute word
;	VBERMI_WindowAttributes		Fill in window attributes
;	VBERMI_SizeAttributes		Fill info buffer with resolution-type statistics
;	VBERMI_ColorAttributes		Fill direct color fields
;	VBERMI_LinearAddress		Fill linear address fields
;	VBERMI_ClockAttributes		Fill clock fields
;	VBEGetMemSize			Return the current size of available memory in bytes
;	VBE_SetMode			Set a specific video mode
;	VBE_GetMode			Get current video mode
;	VBE_SRState			Save/Restore state handler
;	VBE_BankHandler			Get/Set video memory bank
;	VBE_WinFunc			Bank handler (called via "far" call)
;	VBE_LogicalScanLine		Set/Get Logical Scan Line Length
;	VBEGetXResolution		Return the current horizontal resolution
;	VBEPixels2Bytes			Convert number of pixels to number of bytes
;	GetFudgedBPP			Return the bits per pixel value needed for video memory offset calculations
;	GetOffsetShiftFactor		Return the row offset shift factor
;	VBEGetMaxRowOffset		Return the maximum row offset for the current mode
;	GetModeTypeX2			Return the mode type times two
;	GetVRes				Get vertical resolution
;	VBESetOffset			Set a requested scan line length in BYTEs
;	VBECurrentOffsetAdjusted	Return current mode's number of bytes per scan line (adjusted)
;	VBECurrentPixelsPerScan		Calculate current pixels per scan line (May be greater than resolution)
;	VBEGetTextCharHeight		Return physical character height in text mode
;	VBE_DisplayStart		Set/Get Display Start
;	VBETextSetDisplayStart		Set display start for text modes
;	VBECGASetDisplayStart		Set display start for CGA modes 4, 5, & 6
;	VBEPlanarSetDisplayStart	Set display start for planar modes
;	VBE8BitSetDisplayStart		Set display start for 8-bit modes
;	VBE16BitSetDisplayStart		Set display start for 16-bit modes
;	VBE24BitSetDisplayStart		Set display start for 24-bit modes
;	VBETextGetDisplayStart		Get display start for text modes
;	VBECGAGetDisplayStart		Get display start in modes 4, 5, & 6
;	VBEPlanarGetDisplayStart	Get display start for planar modes
;	VBE8BitGetDisplayStart		Get display start for 8 BPP modes
;	VBE16BitGetDisplayStart		Get display start for 16 BPP modes
;	VBE24BitGetDisplayStart		Get display start in 24 BPP mode
;	SetPixelPan			Set the pixel panning value
;	GetPixelPan			Get the current pixel panning value
;	VBE_DACFormat			Set/Get DAC Palette format
;	VBE_DACData			Set/Get DAC Palette Data
;	VBE_PMI				VBE Protected Mode Interface
;	VBE_DotClock			Get/Set pixel clock
;	VBE_DPMS			Display Power Management Signaling functions (DPMS).
;	VBE_DDC				Handles entry to VESA DDC function
;	DDCStartupSequence		Perform the startup sequence to read at a given address
;	DDCEnable			Enable the DDC interface
;	DDCDelay			Allow monitor time to react (just barely)
;	DDCStart			Send a "start" condition over the access.bus
;	DDCDisable			Disable DDC Interface
;	DDCChecksum			Do a checksum of the buffer
;	DDCStop				Send a "stop" condition over the access.bus
;	DDCWaitClockHigh		Wait for DDC clock to go high
;	DDCWrite			Write a byte over the display data channel
;	DDCRead				Read a byte from the display data channel
;
;	$Revision: 3 $
;	$Date: 8/30/99 5:20p $
;	$Logfile: /devel/h5/BIOS/SRC/VESA.ASM $
;	$Log: /devel/h5/BIOS/SRC/VESA.ASM $
;; 
;; 3     8/30/99 5:20p Kaymann
;; Modified the panel code in DDCReadEDID to use the routine IsCRTAttached
;; and IsPanelAttached to see if the device is attached before determining
;; to read EDID from the panel or CRT.
;; 
;; 2     5/24/99 2:11p Kaymann
;; Modified the DDCReadEDID routine to read EDID from the panel when
;; panel is active.
;; 
;; Created the routine ReadFP_EDID to read EDID data from the panel.
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
	include	VESA.INC
;
	%out	!	Assembling: VESA.ASM
;
.MODEL TINY
.CODE
.386
;
IF VESA
;
	extrn	strcpy:near
	extrn	OEMGetMemSize:near
	extrn	OEMFixupVBEModeInfo:near
	extrn	OEMGetLinearMemBase:near
	extrn	OEMSetLinearMemBase:near
	extrn	OEMIsInLinearMode:near
	extrn	OEMEnableExtensions:near
	extrn	GetModeDescPtr:near
	extrn	VBEGetModeDescPtr:near
	extrn	OEMIsModeAvailable:near
	extrn	IsModeMono:near
	extrn	IsTextMode:near
	extrn	IsPlanarMode:near
	extrn	VBEMode2VMode:near
	extrn	VMode2VBEMode:near
	extrn	OEMSetBank:near
	extrn	OEMGetWBank:near
	extrn	OEMGetCurrentXChar:near
	extrn	OEMSetRowOffset:near
	extrn	OEMGetRowOffset:near
	extrn	OEMSetDisplayStart:near
	extrn	OEMGetDisplayStart:near
	extrn	GetBPP:near
	extrn	GetCRTCAddr:near
	extrn	GetIndexRegister:near
	extrn	WaitNotVerticalRetrace:near
	extrn	WaitVerticalRetrace:near
	extrn	ResetATCFlipFlop:near
	extrn	OEMSetDACFormat:near
	extrn	OEMGetDACFormat:near
	extrn	GetDSSeg0:near
	extrn	OEMAdjustMaxRowOffset:near
  IF VESA_DPMS
	extrn	OEMSetDPMS:near
	extrn	OEMGetDPMS:near
  ENDIF
  IF VESA_DDC
	extrn	OEMDDCClockLowDataLow:near
	extrn	OEMDDCClockLowDataHigh:near
	extrn	OEMDDCClockHighDataLow:near
	extrn	OEMDDCClockHighDataHigh:near
	extrn	OEMDDCEnable:near
	extrn	OEMDDCEnableDFP:near
	extrn	OEMDDCDisable:near
	extrn	OEMDDCIsClockHigh:near
	extrn	OEMDDCIsDataHigh:near
  ENDIF
  IF VESA_300
	extrn	OEMGetMaxDotClock:near
	extrn	OEMGetDotClock:near
	extrn	OEMFixupCRTC:near
	extrn	OEMProtectedModeInit:near
	extrn	GetESSeg0:near
	extrn	SetMode:near
  ENDIF
;
	extrn	tblVBEInfoBlock:byte
	extrn	tblVBEModes:word
	extrn	LENGTH_VBEMODES:abs
	extrn	szVBEVendorName:byte
	extrn	szVBEProductName:byte
	extrn	szVBEProductRev:byte
	extrn	szOEMName:byte
	extrn	tblDirectColor16BPP:byte
	extrn	tblDirectColor24BPP:byte
	extrn	LENGTH_DCTABLE:abs
	extrn	tblProtectedModeInterface:byte
	extrn	LENGTH_PMITABLE:abs
  IF VESA_300
	extrn	byInProtMode:byte
	extrn	segInitParm:word
	extrn	segInit2ndSP:word
	extrn	segInitDCC:word
	extrn	segInitOEM:word
	extrn	segOEMExtRegs:word
	extrn	segOEMModeDesc:word
	extrn	segfiINT1F:word
	extrn	segfiINT43:word
	extrn	segfi8x14:word
	extrn	segfi8x8:word
	extrn	segfi8x8Top:word
	extrn	segfi9x14:word
	extrn	segfi8x16:word
	extrn	segfi9x16:word
	extrn	segROM8x8:word
	extrn	segROM8x8Top:word
	extrn	segROM8x14:word
	extrn	segROM8x16:word
	extrn	segROM9x14:word
	extrn	segROM9x16:word
	extrn	selCodeSeg:word
	extrn	tblFont8x8:byte
	extrn	tblSavePtr:byte
  ENDIF

	extrn	Read_And_Shadow_IO_Base:near

IF DVI_BIOS
	extrn	GetIOBase:near
	extrn	I2CStart:near
	extrn	I2CSendByte:near
	extrn	I2CDelay:near
	extrn	I2CReadBusXBytes:near
	extrn	I2CStop:near	
	extrn	IsCRTAttached:near
	extrn	IsPanelAttached:near
ENDIF

;
;	HandlerVESA - Call requested VESA subfunction
;
;	Entry:	AH = 4Fh
;		AL = Subfunction
;		DS = Seg0 segment
;		BX, CX, DX, BP, DI, SI, ES = Function specific
;		SI, DS are pushed on the stack
;	Exit:	AH = Success flag
;			0 = Function successfully completed
;			1 = Function unsuccessful
;		AL = Function support flag
;			4Fh = Supported function
;			XXh = Unsupported function
;		BX, CX, DX, BP, DI = Function specific
;
;	Note:	DS, SI have been pushed on the stack already
;
tblHandlerVESA	label	word
	dw	VBE_ReturnControllerInfo	; Function 00h
	dw	VBE_ReturnModeInfo		; Function 01h
	dw	VBE_SetMode			; Function 02h
	dw	VBE_GetMode			; Function 03h
	dw	VBE_SRState			; Function 04h
	dw	VBE_BankHandler			; Function 05h
	dw	VBE_LogicalScanLine		; Function 06h
	dw	VBE_DisplayStart		; Function 07h
	dw	VBE_DACFormat			; Function 08h
	dw	VBE_DACData			; Function 09h
	dw	VBE_PMI				; Function 0Ah
IF (VESA_300 eq 0)
	dw	VBE_Dummy			; Function 0Bh
ELSE
	dw	VBE_DotClock			; Function 0Bh
ENDIF
	dw	VBE_Dummy			; Function 0Ch
	dw	VBE_Dummy			; Function 0Dh
	dw	VBE_Dummy			; Function 0Eh
	dw	VBE_Dummy			; Function 0Fh
  IF VESA_DPMS
	dw	VBE_DPMS			; Function 10h
  ELSE
	dw	VBE_Dummy			; Function 10h
  ENDIF
  IF VESA_DDC
	dw	VBE_Dummy			; Function 11h
	dw	VBE_Dummy			; Function 12h
	dw	VBE_Dummy			; Function 13h
	dw	VBE_Dummy			; Function 14h
	dw	VBE_DDC				; Function 15h
  ENDIF
LENGTH_VESADISPATCHTABLE	equ	($ - tblHandlerVESA) / 2
;
	public	HandlerVESA
HandlerVESA	proc
	assume	ds:Seg0, es:nothing
	MONO_OUT	"VBE: AX=#AX BX=#BX CX=#CX DX=#DX ES:DI=#ES:#DI"
	cmp	al,LENGTH_VESADISPATCHTABLE
	jae	short HandlerVESA_unsupported
	cbw
	shl	ax,1
	mov	si,ax
	call	cs:[si + offset tblHandlerVESA]
HandlerVESA_exit:
	pop	ds
	pop	si
	iret
;
HandlerVESA_unsupported:
	mov	ax,0100h		; Unsuccessful and unsupported function
	jmp	short HandlerVESA_exit
HandlerVESA	endp
;
;	VBE Function 00h
;	VBE_ReturnControllerInfo - Return VBEINFOBLOCK data structure
;
;	Entry:	ES:DI = Pointer to buffer
;		DS = Seg0 segment
;	Exit:	AX = Success flag
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_ReturnControllerInfo
VBE_ReturnControllerInfo	proc
	assume	ds:Seg0, es:nothing
	pusha
	push	ds
;
	push	cs
	pop	ds
	assume	ds:nothing	; Make DS point to ROM data segment
;
; If, on entry, the application has placed the string "VBE2" in the
; signature area, then extended VBE 2.0 information is returned to
; the application. Otherwise, assume that the application may NOT
; have allocated enough space and therefore don't return the information.
;
	sub	cx,cx		; CX = 0 (assume VBE version 1.0)
	cmp	dword ptr es:[di].VBEINFOBLOCK.VbeSignature,'2EBV'
	jne	short VBE_RCI_10
	inc	cx		; CX = 1 (flag VBE version 2.0 or 3.0)
VBE_RCI_10:
	push	cx		; Save VBE info structure version for later
;
; Retrieve "standard" 1.0 style information
;
	mov	cx,size VBEINFOBLOCK_102
	mov	si,offset tblVBEInfoBlock
	push	di		; Save pointer to buffer
	rep movsb
	mov	cx,256 - (size VBEINFOBLOCK_102)
	sub	al,al
	rep stosb		; Fill rest of buffer with 0's
	pop	di		; Restore pointer to buffer
;
; Return memory size in kilobytes
;
	call	OEMGetMemSize

;Because we have problems with viewing memory above 16Meg I created this
;work around where if I anything above 16 Meg, I trick VBE and return
;we're a 16 Meg board.
  cmp   ax,16*4		  									;Are we >16Meg
  jle   VBEFunc00_SkipVBEMemPatch					;Jump if <16 Meg

  mov   ax,16*4											;Slam dunk 16 Meg

VBEFunc00_SkipVBEMemPatch:

	shl	ax,2
	mov	es:[di].VBEINFOBLOCK_102.TotalMemory,ax
IF VESA_300
	mov	es:[di].VBEINFOBLOCK.VbeVersion,300h		; Version 3.00
ENDIF
;
; Fixup the pointers
;
	mov	word ptr es:[di + 2].VBEINFOBLOCK_102.OEMStringPtr,cs
	mov	word ptr es:[di + 2].VBEINFOBLOCK_102.VideoModePtr,cs
;
	pop	cx			; Retrieve VBE info structure version
	jcxz	short VBE_RCI_exit	; Version 1.X VBE data? Y: Go exit
;
; Version 2.0 VBE structure is a little weird here. Everything must be
; moved into an additional RAM buffer.
;
	lea	bx,es:[di].VBEINFOBLOCK.VBE_OEM_Data
	push	bx						; Save start of string table
	xchg	bx,di						; ES:BX = data structure, ES:DI = string table
IF (VESA_300 eq 0)
	mov	es:[bx].VBEINFOBLOCK.VbeVersion,200h		; Version 2.00
ENDIF
	mov	es:[bx].VBEINFOBLOCK.OemSoftwareRev,100h	; Software revision
	mov	word ptr es:[bx].VBEINFOBLOCK.OEMStringPtr,di
	mov	word ptr es:[bx+2].VBEINFOBLOCK.OEMStringPtr,es
	mov	si,offset szOEMName
	call	strcpy						; Put OEM string in buffer
	mov	word ptr es:[bx].VBEINFOBLOCK.VideoModePtr,di
	mov	word ptr es:[bx+2].VBEINFOBLOCK.VideoModePtr,es
	mov	cx,LENGTH_VBEMODES
	mov	si,offset tblVBEModes
	rep movsw
	mov	word ptr es:[bx].VBEINFOBLOCK.OemVendorNamePtr,di
	mov	word ptr es:[bx+2].VBEINFOBLOCK.OemVendorNamePtr,es
	mov	si,offset szVBEVendorName
	call	strcpy
	mov	word ptr es:[bx].VBEINFOBLOCK.OemProductNamePtr,di
	mov	word ptr es:[bx+2].VBEINFOBLOCK.OemProductNamePtr,es
	mov	si,offset szVBEProductName
	call	strcpy
	mov	word ptr es:[bx].VBEINFOBLOCK.OemProductRevPtr,di
	mov	word ptr es:[bx+2].VBEINFOBLOCK.OemProductRevPtr,es
	mov	si,offset szVBEProductRev
	call	strcpy
	pop	ax			; Retrieve start of string table
	mov	cx,256
	sub	cx,di
	add	cx,ax			; CX = 256 - (DI - AX)
	sub	al,al
	rep stosb			; Clear out unused space
;
VBE_RCI_exit:
	pop	ds
	popa
	mov	ax,004Fh		; Always show success
VBE_Dummy	label	near		; Any near "ret" will do
	ret
VBE_ReturnControllerInfo	endp
;
;	VBE Function 01h
;	VBE_ReturnModeInfo - Return MODEINFOBLOCK data structure for a specific mode number
;
;	Entry:	CX = Mode number
;		ES:DI = Pointer to MODEINFOBLOCK buffer
;		DS = Seg0 segment
;	Exit:	AX = Success flag
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_ReturnModeInfo
VBE_ReturnModeInfo	proc
	assume	ds:Seg0, es:nothing
	pusha
	call	VBERMI_Init		; Determine if mode exists
	jc	short VBERMI_nosupport	;  N: Exit (existing modes - even if unavailable still returns info)
;
; Start filling in the buffer. At this point:
;   BL = Mode number
;   BH = IMode number
;   DS:SI = Pointer to MODEDESC data structure
;   ES:DI = Pointer to user's buffer
;
	call	VBERMI_ModeAttribute	; Calculate and fill mode attribute
	call	VBERMI_WindowAttributes	; Fill window attribute
	call	VBERMI_SizeAttributes	; Fill resolution-type information
	call	VBERMI_ColorAttributes	; Fill direct color fields
	call	VBERMI_LinearAddress	; Fill linear address fields
IF VESA_300
	call	VBERMI_ClockAttributes	; Get maximum pixel clock
ENDIF
	call	OEMFixupVBEModeInfo	; Allow last chance fixups
;
; Normal exit
;
	popa
	mov	ax,004Fh		; Flag success
	ret
;
; Exit for an unknown mode
;
VBERMI_nosupport:
	popa
	mov	ax,014Fh		; Flag function unsuccessful
	ret
VBE_ReturnModeInfo	endp
;
;	VBERMI_Init - Determine if mode exists and initialize CPU registers
;
;	Entry:	CX = Mode number
;		DS = Seg0 segment
;	Exit:	BL = 8-bit mode number
;		BH = Internal mode number
;		DS:SI = Pointer to MODEDESC data
;		ES:DI = Pointer to VBEMODEINFOBLOCK buffer
;		Carry flag:
;		   NC: Mode exists, but may be unavailable
;		   CF: Mode doesn't exist, above registers contain invalid data
;
;	All other registers are preserved.
;
	public	VBERMI_Init
VBERMI_Init	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	cx
	push	bp
	push	es
;
	mov	bp,es				; Save ES for later without having to POP
	call	OEMEnableExtensions		; Enable extensions
	mov	ax,cx
	cmp	ax,100h				; VBE mode number?
	jae	short VBERMI_Init_10		;  Y: Go handle
	call	GetModeDescPtr			;  N: Get ptr based on 8-bit mode
	jmp	short VBERMI_Init_20
VBERMI_Init_10:
	call	VBEGetModeDescPtr
VBERMI_Init_20:
	jc	short VBERMI_Init_exit		; Mode exists? N: Exit with error
;
	push	es
	pop	ds				; MOV DS,ES
	assume	ds:nothing
	mov	si,bx				; DS:SI = Ptr to MODEDESC data
	mov	al,[si].MODEDESC.mdIMode	; AL = Internal mode number
	mov	bl,[si].MODEDESC.mdMode		; BL = OEM mode number
	mov	bh,al				; BH = Internal mode number
;
; Got all registers setup, now clear out the buffer
;
	mov	es,bp				; Get MODEINFOBLOCK pointer back
	push	di				; Save buffer address
	mov	cx,256/2
	sub	ax,ax
	rep stosw
	pop	di				; Save buffer address
	clc
;
VBERMI_Init_exit:
	pop	es
	pop	bp
	pop	cx
	pop	ax
	ret
VBERMI_Init	endp
;
;	VBERMI_ModeAttribute - Create mode attribute word
;
;	Entry:	BL = Mode number
;		BH = Imode number
;		DS:SI = Pointer to MODEDESC data
;		ES:DI = Pointer to MODEINFOBLOCK buffer
;	Exit:	None
;
;	Note:	On exit, "VBEMODEINFOBLOCK.ModeAttributes" is filled
;		with the following attribute word:
;			Bit 0:	0 = Not supported in current H/W
;				1 = Supported
;			Bit 1:	1 (reserved) (VBE 1.0 flag for data structure now required)
;			Bit 2:	0 = No BIOS support
;				1 = BIOS support
;			Bit 3:	0 = Monochrome CRTC
;				1 = Color CRTC
;			Bit 4:	0 = Text mode
;				1 = Graphics mode
;			Bit 5:	0 = VGA compatible
;				1 = Not VGA compatible
;			Bit 6:	0 = VGA compatible windowed memory mode available
;				1 = No compatible windowed memory mode available
;			Bit 7:	0 = No linear frame buffer available
;				1 = Linear frame buffer available
;		VBE 3.0 also defines these:
;			Bit 8:	0 = Double scan mode is not available
;				1 = Double scan mode is available
;			Bit 9:	0 = Interlace mode is not available
;				1 = Interlace mode is available
;			Bit 10:	0 = H/W triple buffering is not available
;				1 = H/W triple buffering is available
;			Bit 11: 0 = H/W stereoscopic display is not available
;				1 = H/W stereoscopic display is available
;			Bit 12:	0 = Dual display start address is not available
;				1 = Dual display start address is available
;			Bits 13-15:  Reserved
;
;	All registers are preserved.
;
	public	VBERMI_ModeAttribute
VBERMI_ModeAttribute	proc
	assume	ds:nothing, es:nothing
	push	cx
	push	dx
;
; Build the mode attribute into the DX register. Start with the
; initial condition of the following bits filled in:
;  Bit 1: Extended data structure support (always a "1" for VBE 1.2 and up)
;  Bit 2: Extended TTY support
;  Bit 5: VGA compatibility
;
  IF VESA_VGACOMPAT
    IF EXTENDED_TTY
	mov	dx,00006h		; Extended TTY + VGA compat
    ELSE
	mov	dx,00002h		; VGA compatibility
    ENDIF
  ELSE
	mov	dx,00022h		; No VGA compatibility / no TTY
  ENDIF
;
; Determine if the mode is available
;
	mov	al,bl
	call	OEMIsModeAvailable	; Mode available?
	jc	short VBERMI_MA_10	;  Y: Mode doesn't exist
	or	dl,001h			;  N: Mode does exist
;
; If extended TTY support is not available, but this is a standard
; mode or a text mode, then add that flag back.
;
VBERMI_MA_10:
  IF (not EXTENDED_TTY)
	call	IsTextMode		; Is this a text mode?
	jz	short VBERMI_MA_15	;  Y: Flag TTY support
	cmp	al,013h			; Standard VGA mode?
	ja	short VBERMI_MA_20	;  N: Skip ahead / No TTY support
VBERMI_MA_15:
	or	dl,004h			;  Y: Flag TTY support
VBERMI_MA_20:
  ENDIF
;
; Set the mono/color flag and the text/graphics flag
;
	call	IsModeMono		; Does this mode use a mono CRTC?
	jz	short VBERMI_MA_30	;  Y: Got value already
	or	dl,008h			;  N: Flag color CRTC
VBERMI_MA_30:
	call	IsTextMode		; Is this a text mode?
	jz	short VBERMI_MA_40	;  Y: Got value already
	or	dl,010h			;  N: Flag graphics mode
;
; Adjust hardware support flag based on whether a co-resident adapter
; exists. For example, mode 7 (mono text) can't be set if a monochrome
; adapter is in the system.
;
VBERMI_MA_40:
	push	ds
	call	GetDSSeg0		; DS = Seg0
	assume	ds:Seg0
	test	[fVGAInfo],01h		; All modes on all monitors?
	jnz	short VBERMI_MA_50	;  Y: No problem, go exit
	mov	cl,[fEGAInfo]
	not	cl			; Invert mono/color to match VESA bit polarity
	and	cl,002h			; Isolate mono/color VGA-type bit
	shl	cl,2
	mov	ch,dl
	and	ch,08h			; Isolate mono/color mode bit
	cmp	cl,ch			; Is this a color mode on a color VGA (or mono/mono)?
	je	short VBERMI_MA_50	;  Y: Good, this is legal
	and	dl,0FEh			;  N: Not legal, mode not available
VBERMI_MA_50:
	pop	ds
	assume	ds:nothing
;
	mov	es:[di].VBEMODEINFOBLOCK.ModeAttributes,dx
;
	pop	dx
	pop	cx
	ret
VBERMI_ModeAttribute	endp
;
;	VBERMI_WindowAttributes - Fill in window attributes
;
;	Entry:	BL = Mode number
;		BH = Imode number
;		DS:SI = Pointer to MODEDESC data
;		ES:DI = Pointer to MODEINFOBLOCK buffer
;	Exit:	None
;
	public	VBERMI_WindowAttributes
VBERMI_WindowAttributes	proc
	assume	ds:nothing, es:nothing
	pusha
	mov	al,006h				; Window A is R/W
	cmp	bl,013h				; Standard VGA mode?
	jbe	short VBERMI_WA_10		;  Y: Fixed window
	inc	ax				;  N: Relocatable window
VBERMI_WA_10:
	mov	es:[di].VBEMODEINFOBLOCK.WinAAttributes,al	; Window A is R/W
	mov	word ptr es:[di].VBEMODEINFOBLOCK.WinFuncPtr,offset VBE_WinFunc
	mov	word ptr es:[di+2].VBEMODEINFOBLOCK.WinFuncPtr,cs
;
; Calculate video segment address and size
;
	mov	al,[si].MODEDESC.mdType		; Get mode type
	mov	dx,0B800h			; Assume B800h for 32K
	mov	cx,32
	cmp	al,MDTYPE_CTEXT			; Color text?
	je	short VBERMI_WA_20		;  Y: Got it
	cmp	al,MDTYPE_CGA			; CGA mode?
	je	short VBERMI_WA_20		;  Y: Got it
	mov	dh,0B0h				; Assume B000h for 32K
	cmp	al,MDTYPE_MTEXT			; Mono text?
	je	short VBERMI_WA_20		;  Y: Got it
	mov	dh,0A0h				; Everything else is A000h
	shl	cx,1				;  for 64K
;
VBERMI_WA_20:
	mov	es:[di].VBEMODEINFOBLOCK.WinASegment,dx		; Window A location
	mov	es:[di].VBEMODEINFOBLOCK.WinGranularity,cx	; Window Granularity
	mov	es:[di].VBEMODEINFOBLOCK.WinSize,cx		; Window Size
;
	popa
	ret
VBERMI_WindowAttributes	endp
;
;	VBERMI_SizeAttributes - Fill info buffer with resolution-type statistics
;
;	Entry:	BL = Mode number
;		BH = Imode number
;		DS:SI = Pointer to MODEDESC data
;		ES:DI = Pointer to MODEINFOBLOCK buffer
;	Exit:	None
;
	public	VBERMI_SizeAttributes
VBERMI_SizeAttributes	proc
	assume	ds:Seg0, es:nothing
	pushad
;
	mov	ax,[si].MODEDESC.mdRowOff
	mov	es:[di].VBEMODEINFOBLOCK.BytesPerScanLine,ax
IF VESA_300
	mov	es:[di].VBEMODEINFOBLOCK.LinBytesPerScanLine,ax
ENDIF
	mov	ax,[si].MODEDESC.mdXRes
	mov	es:[di].VBEMODEINFOBLOCK.XResolution,ax
	mov	ax,[si].MODEDESC.mdYRes
	mov	es:[di].VBEMODEINFOBLOCK.YResolution,ax
	mov	al,[si].MODEDESC.mdCharHeight
	mov	es:[di].VBEMODEINFOBLOCK.YCharSize,al
	mov	al,[si].MODEDESC.mdBPP
	mov	es:[di].VBEMODEINFOBLOCK.BitsPerPixel,al
;
	mov	al,bl
	sub	cx,cx			; Assume text mode
	call	IsTextMode		; Is it text mode?
	jz	short VBERMI_SA_05	;  Y: Got value
	inc	cx			;  N: It's a graphics mode
VBERMI_SA_05:
;
; Char width
;
	mov	al,9			; Assume 9-dot mode
	jcxz	VBERMI_SA_30		; Text mode? Y: Use 9-dot mode
	dec	ax			;  N: Use 8-dot mode
VBERMI_SA_30:
	mov	es:[di].VBEMODEINFOBLOCK.XCharSize,al
;
; Number of planes
;
	mov	dl,1			; Assume one plane
	mov	al,bl
	call	IsPlanarMode		; Is it a planar mode?
	jnz	short VBERMI_SA_40	;  N: Only one plane
	mov	dl,4			;  Y: Four planes
VBERMI_SA_40:
	mov	es:[di].VBEMODEINFOBLOCK.NumberOfPlanes,dl
;
; Memory model
;
	mov	al,es:[di].VBEMODEINFOBLOCK.BitsPerPixel
	sub	dl,dl			; AL = BPP, DL = 0
	jcxz	VBERMI_SA_50		; Text? Y: Already set
	inc	dx			; Assume CGA
	cmp	bl,6			; Cheat: Mode 4,5,6 are only CGA-type modes
	jbe	short VBERMI_SA_50	;  Y: Flag CGA
	inc	dx
	inc	dx			; Skip Hercules, assume planar
	cmp	al,4			; All 4-bit per pixel modes are planar
	jbe	short VBERMI_SA_50	;  Y: Flag planar (even 1-bit planars)
	inc	dx			; Assume packed pixel (VESA test program needs this)
	cmp	al,8			; 8-bit per pixel?
	je	short VBERMI_SA_50	;  Y: Flag 8-bit
	inc	dx			; Skip non-chain 4, 256 color
	inc	dx			; Assume "direct" color
VBERMI_SA_50:
	mov	es:[di].VBEMODEINFOBLOCK.MemoryModel,dl
;
; Banks (CGA, Herc. style)
;
	mov	dx,0001h		; Assume 1 bank
	jcxz	VBERMI_SA_60		; Text modes? Y: Skip "banks"
	cmp	bl,6			; CGA modes?
	ja	short VBERMI_SA_60	;  N: Skip "banks"
	add	dx,0801h		;  Y: CGA has 2 "banks" of 8K each
VBERMI_SA_60:
	mov	es:[di].VBEMODEINFOBLOCK.NumberOfBanks,dl
	mov	es:[di].VBEMODEINFOBLOCK.BankSize,dh
;
; Calculate the number of pages available:
;
;	Memory needed by video mode = Row offset * Y Resolution * Number of planes
;	Pages = Total display memory / Memory needed by Video Mode
;
	mov	al,bl
	call	VBEGetMemSize		; DX:AX = Memory size
	shl	eax,16
	mov	ax,dx
	rol	eax,16			; EAX = Total memory
	movzx	ecx,es:[di].VBEMODEINFOBLOCK.BytesPerScanLine
	movzx	edx,es:[di].VBEMODEINFOBLOCK.NumberOfPlanes
	imul	ecx,edx
	movzx	edx,es:[di].VBEMODEINFOBLOCK.YResolution
	imul	ecx,edx			; ECX = Memory required
	cdq				; EDX:EAX = Total memory
IF VESA_300
;
; A clarification was made in VBE 3.0 that memory per page should be
; rounded up to the nearest 64K (a "ceiling" function). It's no coincidence
; that this clarification should be made by the SciTech members of the VBE
; committee since it is their test program that has notoriously crashed
; many systems based on this [suspect] assumption.
;
; Note that this change will make the original VBE test supplied by
; a member the VESA committee (for 1.0) fail the page count test.
;
	add	eax,0FFFFh		; Add 64K-1
	adc	edx,0
	sub	ax,ax			; Lop off the lower 64K
ENDIF
	div	ecx
	test	al,al
	jz	short VBERMI_SA_70
	dec	ax
VBERMI_SA_70:
	mov	es:[di].VBEMODEINFOBLOCK.NumberOfImagePages,al
IF VESA_300
	mov	es:[di].VBEMODEINFOBLOCK.BnkNumberOfImagePages,al
	mov	es:[di].VBEMODEINFOBLOCK.LinNumberOfImagePages,al
ENDIF
; Reserved byte
	mov	es:[di].VBEMODEINFOBLOCK.Reserved1,1
;
	popad
	ret
VBERMI_SizeAttributes	endp
;
;	VBERMI_ColorAttributes - Fill direct color fields
;
;	Entry:	BL = Mode number
;		BH = Imode number
;		DS:SI = Pointer to MODEDESC data
;		ES:DI = Pointer to MODEINFOBLOCK buffer
;	Exit:	None
;
	public	VBERMI_ColorAttributes
VBERMI_ColorAttributes	proc
	assume	ds:nothing, es:nothing
	pusha
;
	mov	si,offset tblDirectColor16BPP			; Assume 16-bit per pixel
	cmp	es:[di].VBEMODEINFOBLOCK.BitsPerPixel,16	; 16 bits per pixel?
	jb	short VBERMI_CA_exit				;  <: Leave lower BPP modes alone
	je	short VBERMI_10					;  =: Use 16 BPP table
	mov	si,offset tblDirectColor24BPP			;  >: Use 24 BPP table
VBERMI_10:
	mov	cx,LENGTH_DCTABLE
IF (VESA_300 eq 0)
	lea	di,[di].VBEMODEINFOBLOCK.RedMaskSize
	rep movs byte ptr es:[di],cs:[si]
ELSE
	push	cx
	push	di
	push	si
	lea	di,[di].VBEMODEINFOBLOCK.RedMaskSize
	rep movs byte ptr es:[di],cs:[si]
	pop	si
	pop	di
	pop	cx
	dec	cx			; Don't copy "DirectColorModeInfo" the 2nd time
	lea	di,[di].VBEMODEINFOBLOCK.LinRedMaskSize
	rep movs byte ptr es:[di],cs:[si]
ENDIF
;
VBERMI_CA_exit:
	popa
	ret
VBERMI_ColorAttributes	endp
;
;	VBERMI_LinearAddress - Fill linear address fields
;
;	Entry:	BL = Mode number
;		BH = Imode number
;		DS:SI = Pointer to MODEDESC data
;		ES:DI = Pointer to MODEINFOBLOCK buffer
;	Exit:	None
;
	public	VBERMI_LinearAddress
VBERMI_LinearAddress	proc
	push	eax
	cmp	bl,013h					; Standard VGA mode?
	jbe	short VBERMI_LinearAddress_exit		;  Y: No linear address available
	call	OEMGetLinearMemBase
IF (VESA_300 eq 0)
	mov	es:[di].VBEMODEINFOBLOCK.OffScreenMemPtr,0
ENDIF
	mov	es:[di].VBEMODEINFOBLOCK.PhysBasePtr,eax
	test	eax,eax						; Linear address is 0?
	jz	short VBERMI_LinearAddress_exit			;  Y: Linear address not supported
	or	es:[di].VBEMODEINFOBLOCK.ModeAttributes,080h	;  N: Linear address available
VBERMI_LinearAddress_exit:
	pop	eax
	ret
VBERMI_LinearAddress	endp
;
IF VESA_300
;
;	VBERMI_ClockAttributes - Fill clock fields
;
;	Entry:	BL = Mode number
;		BH = Imode number
;		DS:SI = Pointer to MODEDESC data
;		ES:DI = Pointer to MODEINFOBLOCK buffer
;	Exit:	None
;
	public	VBERMI_ClockAttributes
VBERMI_ClockAttributes	proc
	push	eax
	call	OEMGetMaxDotClock
	mov	es:[di].VBEMODEINFOBLOCK.MaxPixelClock,eax
	pop	eax
	ret
VBERMI_ClockAttributes	endp
;
ENDIF
;
;	VBEGetMemSize - Return the current size of available memory in bytes
;
;	Entry:	AL = Mode number
;	Exit:	DX:AX = Memory size in bytes
;
;	Note:	This function takes into consideration standard VGA modes
;
	public	VBEGetMemSize
VBEGetMemSize	proc
	assume	ds:nothing, es:nothing
	push	bx
	mov	bl,al				; BL = Mode number
	sub	dx,dx
	mov	ax,08000h			; Assume 32K (DX:AX = 00008000h)
	cmp	bl,7				; CGA/MDA compatible mode?
	jbe	short VBEGetMemSize_exit	;  Y: Memory only 32K
	push	ax
	mov	al,bl
	call	IsTextMode			; Is extended mode a text mode?
	pop	ax				;  *** Doesn't affect flags ***
	jz	short VBEGetMemSize_exit	;  Y: It's also a 32K mode
	mov	ax,dx
	inc	dx				; DX:AX = 00010000h
	cmp	bl,013h				; Mode 13h (can only use 64K)?
	je	short VBEGetMemSize_exit	;  Y: It's a 64K mode
	mov	dx,4				;  *** Doesn't affect flags ***
	jb	short VBEGetMemSize_exit	; 256K (planar) mode (DX:AX = 00040000h)
;
; Assume all extended graphics modes have all video memory available to it.
;
VBEGetMemSize_10:
IF (not EXTENDED_PLANARPAGES)
	mov	al,bl
	call	IsPlanarMode
	mov	ax,0
	mov	dx,4
	jz	short VBEGetMemSize_exit
ENDIF
	call	OEMGetMemSize

;Because we have problems with viewing memory above 16Meg I created this
;work around where if I anything above 16 Meg, I trick VBE and return
;we're a 16 Meg board.  This is where we return how many video pages
; we support.

  cmp   ax,16*4		  									;Are we >16Meg
  jle   VBEGetMemSize_SkipVBEMemPatch				;Jump if <16 Meg

  mov   ax,16*4											;Slam dunk 16 Meg

VBEGetMemSize_SkipVBEMemPatch:

	shl	ax,2
	mov	bx,ax				; BX = Mem size in K (Fix later - LGC)
	mov	ax,(1024/2)*64			; Limits of x86 architecture
	shl	bx,1				; Limits of x86 architecture
	mul	bx				; DX:AX = Video memory in bytes
;
VBEGetMemSize_exit:
	pop	bx
	ret
VBEGetMemSize	endp
;
;	VBE Function 02h
;	VBE_SetMode - Set a specific video mode
;
;	Entry:	BX = Mode number field:
;			Bits 0-8:	Mode number
;			Bits 9-10:	Reserved (Must be 0) (VBE 3.0)
;			Bit 11 = 0:	Use default refresh rate (VBE 3.0)
;			Bit 11 = 1:	Use user specified CRTC values (VBE 3.0)
;			Bits 12-13:	Reserved for VBE/AF (Must be 0) (VBE 3.0)
;			Bit 14 = 0:	Use VGA buffer based at A000h (VBE 2.0)
;			Bit 14 = 1:	Use linear/flat frame buffer model (VBE 2.0)
;			Bit 15 = 0:	Clear display memory
;			Bit 15 = 1:	Don't clear display memory
;		ES:DI = Pointer to CRTCINFOBLOCK (VBE 3.0)
;		DS = Seg0 segment
;	Exit:	AX = Success flag
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_SetMode
VBE_SetMode	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	cx
	push	dx
;
	mov	ax,bx
IF (VESA_300 eq 0)
	and	ah,01Fh				; AX = Mode number (w/o flags)
ELSE
	and	ah,007h				; AX = Mode number (w/o flags)
ENDIF
	cmp	ax,0100h			; Standard VGA?
	jb	short VBE_SetMode_10		;  Y: Go do normal set mode
	call	VBEMode2VMode			;  N: Get std mode number
	jc	short VBE_SetMode_unsuccessful
;	    
VBE_SetMode_10:
	call	OEMIsModeAvailable		; Mode available?
	jc	short VBE_SetMode_unsuccessful	;  N: Don't set the mode
	push	ax				; Save 8-bit mode number
	push	bx				; Save frame buffer flag
	and 	bh,080h
	or	al,bh				; Add memory clear flag
	sub	ah,ah				; Mode set function
	int	10h
	pop	bx				; Get frame buffer flag
	pop	ax				; Get 8-bit mode number
	cmp	[byMode],al			; Mode set properly?
	jne	short VBE_SetMode_unsuccessful	;  N: Flag error
;
; Handle linear frame buffer flag
;
	test	bh,040h				; Use linear frame buffer?
	jz	short VBE_SetMode_50		;  N: Skip setting it
	call	OEMSetLinearMemBase		;  Y: Go set it
;
; Handle alternate CRTC set
;
VBE_SetMode_50:
IF VESA_300
	test	bh,008h				; Use alternate CRTC table?
	jz	short VBE_SetMode_success	;  N: Skip it, all done
	call	OEMFixupCRTC
ENDIF
;
VBE_SetMode_success:
	mov	ax,004Fh
VBE_SetMode_exit:
	pop	dx
	pop	cx
	pop	bx
	ret
;
VBE_SetMode_unsuccessful:
	mov	ax,014Fh
	jmp	short VBE_SetMode_exit
VBE_SetMode	endp
;
;	VBE Function 03h
;	VBE_GetMode - Get current video mode
;
;	Entry:	DS = Seg0 segment
;	Exit:	AX = Success flag
;		BX = Mode number
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_GetMode
VBE_GetMode	proc
	assume	ds:Seg0, es:nothing
	push	dx
	mov	al,[byMode]			; Get current mode number...
	call	VMode2VBEMode			;  ...and convert it to a VBE #
	cmp	bx,0FFFFh			; Mode exists?
	jne	VBE_GetMode_10			;  Y: Got it
	cbw					;  N: Use what was given
	mov	bx,ax
;
; Determine if mode is in linear addressing mode or not
;
VBE_GetMode_10:
	call	OEMIsInLinearMode		; Mode uses linear framebuffer?
	jnc	short VBE_GetMode_20		;  N: Don't flag it
	or	bh,040h				;  Y: Flag linear mode (bit 14)
;
; Determine if regen clear flag is set
;
VBE_GetMode_20:
	mov	al,[fEGAInfo]			; Get memory clear flag...
	and	al,NO_REGEN_CLEAR		;  ...isolate it
	or	bh,al				;  ...and add it to bit 15
;
	mov	ax,004Fh			; Flag success
	pop	dx
	ret
VBE_GetMode	endp
;
;	VBE Function 04h
;	VBE_SRState - Save/Restore state handler
;
;	Entry:	DS = Seg0 segment
;		DL = Subfunction code
;		   DL = 0:	Return save/restore state buffer size
;			CX = Requested states
;				Bit 0:		H/W State
;				Bit 1:		BIOS State
;				Bit 2:		DAC State
;				Bit 3:		VBE State
;				Bits 4-15:	Reserved
;		   DL = 1:	Save VBE state
;			CX = Requested states
;			ES:BX = Pointer to buffer
;		   DL = 2:	Restore VBE state
;			CX = Requested states
;			ES:BX = Pointer to buffer (used during save state)
;	Exit:	AX = VBE return status
;		If subfunction 0:
;			BX = Number of 64-byte blocks needed
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_SRState
VBE_SRState	proc
	push	cx
	mov	ah,01Ch			; Standard BIOS Save/Restore State function
	mov	al,dl			; Requested states
	and	cx,000Fh		; Clear unused bits
	int	10h
	mov	ax,004Fh		; Indicate success
	pop	cx
	ret
VBE_SRState	endp
;
;	VBE Function 05h
;	VBE_BankHandler - Get/Set video memory bank
;
;	Entry:	BL = Window number (0 = A, 1 = B)
;		BH = Subfunction:
;			00h: Set bank
;			01h: Get bank
;		DX = Bank number
;	Exit:	AX = VBE return status
;		If subfunction 1:
;			DX = Bank number
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_BankHandler
VBE_BankHandler	proc
	assume	ds:Seg0, es:nothing
	mov	ax,014Fh			; Assume failure
	test	bl,bl				; Window A?
	jnz	short VBE_BankHandler_exit	;  N: No support for Window B
	cmp	bh,1				; Get bank?
	je	short VBE_BankHandler_GetBank	;  Y: Go do it
	test	bh,bh				; Set bank?
	jnz	short VBE_BankHandler_exit	;  N: No support
	call	OEMSetBank
VBE_BankHandler_support:
	mov	ax,004Fh
VBE_BankHandler_exit:
	ret
;
VBE_BankHandler_GetBank:
	call	OEMGetWBank
	jmp	short VBE_BankHandler_support
VBE_BankHandler	endp
;
;	VBE_WinFunc - Bank handler (called via "far" call)
;
;	Entry:	BL = Window number (0 = A, 1 = B)
;		BH = Subfunction:
;			00h: Set bank
;			01h: Get bank
;		DX = Bank number
;	Exit:	If subfunction 1:
;			DX = Bank number
;
;	AX, DX are trashed.
;
	public	VBE_WinFunc
VBE_WinFunc	proc
	assume	ds:nothing, es:nothing
	test	bl,bl				; Window A?
	jnz	short VBE_WinFunc_exit		;  N: No support for Window B
	cmp	bh,1				; Get bank?
	je	short VBE_WinFunc_GetBank	;  Y: Go do it
	test	bh,bh				; Set bank?
	jnz	short VBE_WinFunc_exit		;  N: No support
;
; Set the bank
;
VBE_WinFunc_SetBank:
	call	OEMSetBank
VBE_WinFunc_exit:
	retf
;
VBE_WinFunc_GetBank:
	call	OEMGetWBank
	retf
VBE_WinFunc	endp
;
;	VBE Function 06h
;	VBE_LogicalScanLine - Set/Get Logical Scan Line Length
;
;	Entry:	DS = Seg0 segment
;		BL = Subfunction:
;			00h:	Set scan line length in pixels
;			01h:	Get scan line length
;			02h:	Set scan line length in bytes
;			03h:	Get maximum scan line length
;			80h:	Set scan line length after vertical retrace
;		CX = Desired scan line length (ignored for "Get" subfunction)
;	Exit:	AX = VBE return status
;		BX = Bytes per scan line
;		CX = Actual pixels per scan line
;		DX = Maximum number of scan lines
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_LogicalScanLine
VBE_LogicalScanLine	proc
	assume	ds:Seg0, es:nothing
	push	bp
	push	di
;
	call	OEMEnableExtensions
	mov	al,bl
	test	al,al
	jz	VBELSL_SetPixels
	cmp	al,001h
	je	VBELSL_GetCurrent
	cmp	al,002h
	je	VBELSL_SetBytes
	cmp	al,003h
	je	VBELSL_GetMaxWidth
VBELSL_unsupported:
	mov	ax,0014Fh			; Unknown subfunctions are unsupported
	jmp	short VBELSL_exit
;
VBELSL_baddata:
	mov	ax,0024Fh			; Data out of range
	jmp	short VBELSL_exit
;
; Set logical scan line information, then fall through to the get scan line
; information subfunction to return actual settings.
;
VBELSL_SetPixels:
	call	VBEGetXResolution		; AX = X Resolution
	cmp	cx,ax				; Is requested offset < resolution
	jb	VBELSL_baddata			;  Y: Must be an error
	call	VBEPixels2Bytes
VBELSL_SetBytes:
	call	VBEGetMaxRowOffset		; AX = Row offset
	cmp	cx,ax				; Is requested offset > maximum
	ja	VBELSL_baddata			;  Y: Must be an error
	mov	ax,cx
	call	VBESetOffset			; Go set new offset
;
; Get logical scan line information
;
VBELSL_GetCurrent:
	call	VBECurrentOffsetAdjusted	; Get current bytes per scan line
VBELSL_GetCurrent_10:
	mov	bx,ax
	call	VBECurrentPixelsPerScan		; Get current pixels per scan line
	mov	cx,ax
;
; Calculate maximum number of scan lines
;
	mov	al,[byMode]
	call	VBEGetMemSize			; DX:AX = Memory size
	div	bx				; Max scans = total memory / (bytes per scan * planes)
	mov	dx,ax
	mov	al,[byMode]
  IF VESA_STDMODES
	call	IsTextMode			; Text mode?
	jnz	VBELSL_GetCurrent_20		;  N: Skip ahead
	call	VBEGetTextCharHeight		;  Y: Multiply back for Y resolution
	mul	dx
	mov	dx,ax
	jmp	short VBELSL_GetCurrent_30
VBELSL_GetCurrent_20:
  ENDIF
	call	IsPlanarMode			; Planar mode?
	jnz	VBELSL_GetCurrent_30		;  N: Skip ahead
	shr	dx,2				;  Y: There is 4X less memory
VBELSL_GetCurrent_30:
;
	mov	ax,004Fh
VBELSL_exit:
	pop	di
	pop	bp
	ret
;
VBELSL_GetMaxWidth:
	call	VBEGetMaxRowOffset
	jmp	short VBELSL_GetCurrent_10
VBE_LogicalScanLine	endp
;
;	VBEGetXResolution - Return the current horizontal resolution
;
;	Entry:	DS = Seg0
;	Exit:	AX = X Resolution
;
	public	VBEGetXResolution
VBEGetXResolution	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	es
;
	mov	al,[byMode]
	call	GetModeDescPtr			; ES:BX = Ptr to MODEDESC
  IF VESA_STDMODES
	call	IsTextMode			; Is this a text mode?
	mov	ax,es:[bx].MODEDESC.mdXRes	;  ** DOESN'T AFFECT FLAGS **
	jnz	VBEGetXResolution_exit		;  N: Got X Resolution
	mov	bx,ax
	call	OEMGetCurrentXChar
	mul	bl				; AX = columns * (8 or 9)
  ELSE
	mov	ax,es:[bx].MODEDESC.mdXRes
  ENDIF
;
VBEGetXResolution_exit:
	pop	es
	pop	bx
	ret
VBEGetXResolution	endp
;
;	VBEPixels2Bytes - Convert number of pixels to number of bytes
;
;	Entry:	CX = Number of pixels
;		DS = Seg0 segment
;	Exit:	CX = Number of bytes
;
;	All registers preserved.
;
	public	VBEPixels2Bytes
VBEPixels2Bytes	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	bx
	push	dx
;
	mov	dx,cx			; DX = Pixels
	call	OEMGetCurrentXChar	; AL = 8 or 9
	mov	bl,al			; BL is bits per character
	call	GetOffsetFactors	; CH = Bits per pixel, CL = Shift factor
;
; Offset = (Pixels per scan line * bits per pixel) / bits per character)
;
	movzx	ax,ch
	push	ax			; Save BPP for later
	mul	dx			; DX:AX is numerator
	sub	bh,bh
	div	bx			; DX is remainder, AX is "answer"
	pop	bx			; Retrieve BPP into BX
;
; Compensate for the granularity of the row offset register in most VGAs
; by stepping up to the next possible row offset value if the current
; value would get truncated. 24-BPP modes need to be handled special due
; to the odd number of bytes per pixel.
;
	cmp	bl,24				; 24-BPP?
	je	short VBEPixels2Bytes_100	;  Y: Go handle special
;
	test	dx,dx				; Left over pixels?
	jz	short VBEPixels2Bytes_10	;  N: Skip ahead
	mov	bx,1				;  Y: Snap to next memory location
	shl	bx,cl				; Compensate for weirdities in row offset register
	add	ax,bx
	jmp	short VBEPixels2Bytes_20
;
VBEPixels2Bytes_10:
	mov	bx,ax
	shr	bx,cl
	shl	bx,cl
	cmp	bx,ax
	jae	short VBEPixels2Bytes_20
	mov	ax,1
	shl	ax,cl
	add	ax,bx
;
VBEPixels2Bytes_20:
;
VBEPixels2Bytes_exit:
	mov	cx,ax			; Return row offset in CX
	pop	dx
	pop	bx
	pop	ax
	ret
;
; 24-BPP modes need an address that is divisible by both 3 and 8. Snap
; to the next closest location that gives us that. Worst case is that
; we lose 23 pixels.
;
VBEPixels2Bytes_100:
	test	al,007h
	jz	short VBEPixels2Bytes_exit
	add	ax,3
	jmp	short VBEPixels2Bytes_100
VBEPixels2Bytes	endp
;
;	GetOffsetFactors - Return the offset shift factor and the fudged BPP
;
;	Entry:	DS = Seg0 segment
;	Exit:	CL = Shift factor
;		CH = Fudged BPP
;
;	Assume extensions are enabled.
;	All registers are preserved.
;
	public	tblOffsetFactors
tblOffsetFactors	label	word
;	       Shift  Fudged
;	       Factor  BPP
	db	002h, 002h		; MDTYPE_CTEXT
	db	002h, 002h		; MDTYPE_MTEXT
	db	002h, 001h		; MDTYPE_CGA
	db	001h, 001h		; MDTYPE_MGRAF
	db	001h, 001h		; MDTYPE_1BPP
	db	001h, 001h		; MDTYPE_4BPP
	db	003h, 008h		; MDTYPE_8BPP
	db	003h, 010h		; MDTYPE_16BPP
	db	003h, 018h		; MDTYPE_24BPP
	db	000h, 000h		; MDTYPE_NULL
;
	public	GetOffsetFactors
GetOffsetFactors	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	bx
	push	si
	push	es
;
	mov	al,[byMode]
	call	GetModeDescPtr			; ES:BX = Ptr to MODEDESC
	movzx	si,byte ptr es:[bx].MODEDESC.mdType
	shl	si,1
	mov	cx,cs:[offset tblOffsetFactors + si]
	cmp	ch,24				; 24 BPP?
	jb	short GetOffsetFactors_10	;  N: Skip ahead
	mov	ch,es:[bx].MODEDESC.mdBPP	;  Y: 24 BPP may also be 32 BPP
GetOffsetFactors_10:
;
	pop	es
	pop	si
	pop	bx
	pop	ax
	ret
GetOffsetFactors	endp
;
;	VBEGetMaxRowOffset - Return the maximum row offset for the current mode
;
;	Entry:	DS = Seg0
;	Exit:	AX = Maximum row offset in BYTEs
;
.errnz	MDTYPE_CTEXT
.errnz	MDTYPE_MTEXT - 1
.errnz	MDTYPE_CGA - 2
.errnz	MDTYPE_MGRAF - 3
.errnz	MDTYPE_1BPP - 4
.errnz	MDTYPE_4BPP - 5
.errnz	MDTYPE_8BPP - 6
.errnz	MDTYPE_16BPP - 7
.errnz	MDTYPE_24BPP - 8
;
	public	VBEGetMaxRowOffset
VBEGetMaxRowOffset	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	dx
	push	si
;
	call	GetModeTypeX2			; SI = mdType * 2
	mov	al,[byMode]
	call	GetVRes				; AX = Vertical resolution
	mov	bx,ax
	mov	al,[byMode]
	call	VBEGetMemSize			; DX:AX = Memory size
;
; At this point:
;
;	DX:AX = Memory size
;	BX = Vertical resolution in scan lines
;	SI = Mode type * 2
;	DS = Seg0
;
	jmp	cs:[tblMaxRowOffset + si]
;
; For text mode, the maximum row offset is:
;
;    Memory Size (always 32K) / (Vertical Resolution / Character Height)
;    which [algebraically] is the same as...
;    (32K / Vertical Resolution) * Character Height
;
VBEGetMaxRowOffset_text:
	div	bx
	imul	[wCharHeight]
	and	al,0F0h				; Play safe; make it "mod 16"
	jmp	short VBEGetMaxRowOffset_exit
;
; For planar mode, the maximum row offset is:
;
;    (Memory Size / 4) / Vertical Resolution
;    which [algebraically] is the same as...
;    Memory Size / (Vertical Resolution * 4)
;
; For packed pixel modes, the maximum row offset is:
;
;    Memory Size / Vertical Resolution
;
VBEGetMaxRowOffset_planar:
	shl	bx,2
VBEGetMaxRowOffset_packed:
	div	bx
	and	al,0F0h				; Play safe; make it "mod 16"
	jmp	short VBEGetMaxRowOffset_exit
;
; For 24-bit modes, the maximum row offset is:
;
;    ((Memory Size / Vertical Resolution) / 3) * 3
;
; Note: It's true the 32-bit mode calculations fall into here as well,
;	but the end result is almost identical. In the interest of saving
;	code space, no special handling for 32-bit modes have been added.
;
VBEGetMaxRowOffset_24bit:
	div	bx
	mov	bx,3
	cwd
	div	bx
	imul	ax,ax,3
VBEGetMaxRowOffset_24bit_10:
	test	al,007h				; Make sure this "mod 8"
	jz	short VBEGetMaxRowOffset_exit
	sub	ax,3				; Try previous pixel...
	jmp	short VBEGetMaxRowOffset_24bit_10
;
VBEGetMaxRowOffset_exit:
	call	OEMAdjustMaxRowOffset
	pop	si
	pop	dx
	pop	bx
	ret
;
	public	tblMaxRowOffset
tblMaxRowOffset	label	word
	dw	VBEGetMaxRowOffset_text
	dw	VBEGetMaxRowOffset_text
	dw	VBEGetMaxRowOffset_packed
	dw	VBEGetMaxRowOffset_planar
	dw	VBEGetMaxRowOffset_planar
	dw	VBEGetMaxRowOffset_planar
	dw	VBEGetMaxRowOffset_packed
	dw	VBEGetMaxRowOffset_packed
	dw	VBEGetMaxRowOffset_24bit
VBEGetMaxRowOffset	endp
;
;	GetModeTypeX2 - Return the mode type times two
;
;	Entry:	DS = Seg0
;	Exit:	SI = Mode type shifted left once
;
;	All other registers preserved.
;
	public	GetModeTypeX2
GetModeTypeX2	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	bx
	push	es
	mov	al,[byMode]
	call	GetModeDescPtr
	mov	al,es:[bx].MODEDESC.mdType
	cbw
	shl	ax,1
	mov	si,ax
	pop	es
	pop	bx
	pop	ax
	ret
GetModeTypeX2	endp
;
;	GetVRes - Get vertical resolution
;
;	Entry:	AL = Mode number
;	Exit:	AX = Vertical resolution
;		Carry flag set as follows:
;			NC = Vertical resolution returned
;			CF = Error (AL = 0)
;
;	All other registers preserved.
;
	public	GetVRes
GetVRes	proc
	assume	ds:nothing, es:nothing
	push	bx
	push	es
;
	call	GetModeDescPtr
	mov	al,0				; ** DOESN'T AFFECT FLAGS **
	jc	GetVRes_exit
	mov	ax,es:[bx].MODEDESC.mdYRes	; ** DOESN'T AFFECT FLAGS **
;
GetVRes_exit:
	pop	es
	pop	bx
	ret
GetVRes	endp
;
;	VBESetOffset - Set a requested scan line length in BYTEs
;
;	Entry:	AX = Requested offset
;		DS = Seg0 segment
;	Exit:	None
;
;	Assume extensions are enabled.
;
	public	VBESetOffset
VBESetOffset	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	cx
	push	dx
;
	call	GetOffsetFactors
	push	ax
	shr	ax,cl
	call	OEMSetRowOffset
	pop	ax
;
; Compute a new value for "wColumns" so that DMU will draw characters
; properly after changing the row offset length.
;
	cwd
	shr	cx,8			; CL <= CH, CH = 0
	div	cx			; DX:AX / CX ==> AX = "Answer"
	mov	[wColumns],ax
;
	pop	dx
	pop	cx
	pop	ax
	ret
VBESetOffset	endp
;
;	VBECurrentOffsetAdjusted - Return current mode's number of bytes per scan line (adjusted)
;
;	Entry:	DS = Seg0 segment
;	Exit:	AX = Bytes per scan line
;
;	Assume extensions are enabled.
;
	public	VBECurrentOffsetAdjusted
VBECurrentOffsetAdjusted	proc
	assume	ds:Seg0, es:nothing
	push	cx
	call	OEMGetRowOffset
	call	GetOffsetFactors
	shl	ax,cl
	pop	cx
	ret
VBECurrentOffsetAdjusted	endp
;
;	VBECurrentPixelsPerScan - Calculate current pixels per scan line
;					(May be greater than resolution)
;
;	Entry:	BX = Offset to next scan line
;		DS = Seg0 segment
;	Exit:	AX = Actual pixels per scan line
;
	public	VBECurrentPixelsPerScan
VBECurrentPixelsPerScan	proc
	assume	ds:Seg0, es:nothing
	push	dx
	push	cx
	call	GetOffsetFactors	; CH is bits per pixel
	call	OEMGetCurrentXChar	; AL = 8 or 9
;
; Pixels per scan line = (offset * bits per character) / bits per pixel
;
	cbw
	mul	bx			; DX:AX = Numerator
	shr	cx,8			; CL <= CH, CH = 0
	div	cx			; DX is remainder, AX is "answer"
;
	pop	cx
	pop	dx
	ret
VBECurrentPixelsPerScan	endp
;
  IF VESA_STDMODES
;
;	VBEGetTextCharHeight - Return physical character height in text mode
;
;	Entry:	None
;	Exit:	AX = Character height
;
;	Assume text mode
;
	public	VBEGetTextCharHeight
VBEGetTextCharHeight	proc
	push	dx
	call	GetCRTCAddr			; DX = 3x4h
	mov	al,009h				; Get max scan line (char height)
	out	dx,al
	inc	dx
	in	al,dx
	and	ax,0001Fh			; Isolate max scan
	inc	ax				; Height = (CRTC[9].0..4) + 1
	pop	dx
	ret
VBEGetTextCharHeight	endp
;
  ENDIF		; VESA_STDMODES
;
;	VBE Function 07h
;	VBE_DisplayStart - Set/Get Display Start
;
;	Entry:	DS = Seg0 segment
;		BH = 0 (Reserved, must be set to 0)
;		BL = Subfunction:
;			00h:	Set display start
;			01h:	Get display start
;			80h:	Set display start after vertical retrace
;		For subfunction 00h and 80h:
;			CX = First displayed pixel in scan line
;			DX = First displayed scan line
;	Exit:	AX = VBE return status
;		For subfunction 01h:
;			BH = 0 (Reserved, must be set to 0)
;			CX = First displayed pixel in scan line
;			DX = First displayed scan line
;
;	Note:	DS, SI have been pushed on the stack already
;
.errnz	MDTYPE_CTEXT
.errnz	MDTYPE_MTEXT - 1
.errnz	MDTYPE_CGA - 2
.errnz	MDTYPE_MGRAF - 3
.errnz	MDTYPE_1BPP - 4
.errnz	MDTYPE_4BPP - 5
.errnz	MDTYPE_8BPP - 6
.errnz	MDTYPE_16BPP - 7
.errnz	MDTYPE_24BPP - 8
;
	public	tblSetDisplayStartRtn
tblSetDisplayStartRtn	label	word
  IF VESA_STDMODES
	dw	VBETextSetDisplayStart		; MDTYPE_CTEXT
	dw	VBETextSetDisplayStart		; MDTYPE_MTEXT
	dw	VBECGASetDisplayStart		; MDTYPE_CGA
	dw	VBEPlanarSetDisplayStart	; MDTYPE_MGRAF
	dw	VBEPlanarSetDisplayStart	; MDTYPE_1BPP
  ELSE
	dw	VBE_Dummy			; MDTYPE_CTEXT
	dw	VBE_Dummy			; MDTYPE_MTEXT
	dw	VBE_Dummy			; MDTYPE_CGA
	dw	VBE_Dummy			; MDTYPE_MGRAF
	dw	VBE_Dummy			; MDTYPE_1BPP
  ENDIF
	dw	VBEPlanarSetDisplayStart	; MDTYPE_4BPP
	dw	VBE8BitSetDisplayStart		; MDTYPE_8BPP
	dw	VBE16BitSetDisplayStart		; MDTYPE_16BPP
	dw	VBE24BitSetDisplayStart		; MDTYPE_24BPP
	dw	VBE_Dummy			; MDTYPE_NULL
;
	public	tblGetDisplayStartRtn
tblGetDisplayStartRtn	label	word
  IF VESA_STDMODES
	dw	VBETextGetDisplayStart		; MDTYPE_CTEXT
	dw	VBETextGetDisplayStart		; MDTYPE_MTEXT
	dw	VBECGAGetDisplayStart		; MDTYPE_CGA
	dw	VBEPlanarGetDisplayStart	; MDTYPE_MGRAF
	dw	VBEPlanarGetDisplayStart	; MDTYPE_1BPP
  ELSE
	dw	VBE_Dummy			; MDTYPE_CTEXT
	dw	VBE_Dummy			; MDTYPE_MTEXT
	dw	VBE_Dummy			; MDTYPE_CGA
	dw	VBE_Dummy			; MDTYPE_MGRAF
	dw	VBE_Dummy			; MDTYPE_1BPP
  ENDIF
	dw	VBEPlanarGetDisplayStart	; MDTYPE_4BPP
	dw	VBE8BitGetDisplayStart		; MDTYPE_8BPP
	dw	VBE16BitGetDisplayStart		; MDTYPE_16BPP
	dw	VBE24BitGetDisplayStart		; MDTYPE_24BPP
	dw	VBE_Dummy			; MDTYPE_NULL
;
	public	VBE_DisplayStart
VBE_DisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMEnableExtensions
	cmp	bl,1
	je	short VBE_DisplayStart_GetStart
	test	bl,bl
	jz	short VBE_DisplayStart_SetStart
	cmp	bl,80h				; Wait vertical retrace?
	jne	short VBE_DisplayStart_nosupport
	push	dx
	call	WaitNotVerticalRetrace
	call	WaitVerticalRetrace
	pop	dx
;
VBE_DisplayStart_SetStart:
	pushad
	call	GetModeTypeX2				; SI = Mode type * 2
	call	cs:[tblSetDisplayStartRtn + si]
	popad						; Doesn't affect flags
	jc	short VBE_DisplayStart_nosupport	; If error, flag it
;
VBE_DisplayStart_support:
	mov	ax,004Fh
VBE_DisplayStart_exit:
	ret
;
VBE_DisplayStart_nosupport:
	mov	ax,014Fh
	jmp	VBE_DisplayStart_exit
;
VBE_DisplayStart_GetStart:
	push	eax
	push	bx
	push	si
	push	di
	call	GetModeTypeX2			; SI = Mode type
	call	cs:[tblGetDisplayStartRtn + si]
	pop	di
	pop	si
	pop	bx
	pop	eax
	sub	bh,bh				; Per VBE spec
	jmp	short VBE_DisplayStart_support
VBE_DisplayStart	endp
;
  IF VESA_STDMODES
;
;	VBETextSetDisplayStart - Set display start for text modes
;
;	Entry:	CX = X
;		DX = Y
;		DS = Seg0 segment
;	Exit:	Carry flag set as follows:
;			CF = Error, couldn't set address
;			NC = No error
;	Algorithm:
;		BPC = 8 or 9
;		Preset row scan = Y % char height
;		Display start = X / BPC + (Y / char height) * row offset * 2
;		Pixel pan = X % BPC (adjusted for 9-dot)
;
;	Note: 	No need to preserve AX, BX, CX, DX, BP, DI, SI
;
	public	VBETextSetDisplayStart
VBETextSetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	sub	ax,ax
	xchg	ax,dx				; DX:AX = Y
	div	[wCharHeight]			; DX = Preset row scan, AX = Y/char height
	mov	si,ax				; SI = Y / char height
	mov	bl,dl				; BL = Preset row scan
	call	GetCRTCAddr			; DX = 3x4h
	mov	al,8
	call	GetIndexRegister
	and	ah,080h				;
	or	ah,bl
	out	dx,ax				; Set preset row scan
;
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1				; Adjust for text mode
	mul	si
	mov	bx,ax				; BX = (Y / height) * offset
;
	call	OEMGetCurrentXChar		; AL = 8 or 9
	cbw					; AX = 8 or 9
	cwd					; DX = 0
	xchg	ax,cx				; CX = 8 or 9, AX = X
	div	cx				; AX = X / BPC, DX = pixel pan
	add	ax,bx
	mov	bx,dx				; BX = Pixel pan value (to be adjusted)
	cwd					; DX = 0; AX.15 always = 0
	call	OEMSetDisplayStart
;
	cmp	cl,9				; 9-dot mode?
	jne	VBETextSetDisplayStart_10	;  N: Use calc'd value
	dec	bx				;  Y: Adjust pixel pan value
	jns	VBETextSetDisplayStart_10
	mov	bl,8
VBETextSetDisplayStart_10:
	mov	al,bl
	call	SetPixelPan
	clc
	ret
VBETextSetDisplayStart	endp
;
;	VBECGASetDisplayStart - Set display start for CGA modes 4, 5, & 6
;
;	Entry:	CX = X
;		DX = Y
;		DS = Seg0 segment
;	Exit:	Carry flag set as follows:
;			CF = Error, couldn't set address
;			NC = No error
;	Algorithm:
;		Display start = X / 8 + (Y * row offset)
;		Pixel panning = X % 8
;
;	Note: 	No need to preserve AX, BX, CX, DX, BP, DI, SI
;
	public	VBECGASetDisplayStart
VBECGASetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMGetRowOffset			; AX = Row offset
	mul	dx
	push	cx				; Preserve pixel pan value
	shr	cx,3
	add	ax,cx
	adc	dx,0
	call	OEMSetDisplayStart
	pop	ax				; Retrieve pixel pan value
	and	al,07h
	call	SetPixelPan
	clc
	ret
VBECGASetDisplayStart	endp
;
  ENDIF		; VESA_STDMODES
;
;	VBEPlanarSetDisplayStart - Set display start for planar modes
;
;	Entry:	CX = X
;		DX = Y
;		DS = Seg0 segment
;	Exit:	Carry flag set as follows:
;			CF = Error, couldn't set address
;			NC = No error
;
;	Algorithm:
;		Display start = X / 8 + (Y * row offset * 2)
;		Pixel panning = X % 8
;
;	Note: 	No need to preserve AX, BX, CX, DX, BP, DI, SI
;
	public	VBEPlanarSetDisplayStart
VBEPlanarSetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1
	mul	dx
	push	cx				; Preserve pixel pan value
	shr	cx,3
	add	ax,cx
	adc	dx,0
	call	OEMSetDisplayStart
	pop	ax
	and	al,07h
	call	SetPixelPan
	clc
	ret
VBEPlanarSetDisplayStart	endp
;
;	VBE8BitSetDisplayStart - Set display start for 8-bit modes
;
;	Entry:	CX = X
;		DX = Y
;		DS = Seg0 segment
;	Exit:	Carry flag set as follows:
;			CF = Error, couldn't set address
;			NC = No error
;
;	Algorithm:
;		Display start = X / 4 + (Y * row offset * 2)
;
;	Note: 	No need to preserve AX, BX, CX, DX, BP, DI, SI
;
	public	VBE8BitSetDisplayStart
VBE8BitSetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	cmp	dx,0178h
	test	cl,003h					; Pixel on 4-pixel boundary?
	stc						; ...assume error...
	jnz	short VBE8BitSetDisplayStart_exit	;  N: Skip this
	call	OEMGetRowOffset				; AX = Row offset
	shl	ax,1
	mul	dx
	shr	cx,2
	add	ax,cx
	adc	dx,0
	call	OEMSetDisplayStart
	clc
VBE8BitSetDisplayStart_exit:
	ret
VBE8BitSetDisplayStart	endp
;
;	VBE16BitSetDisplayStart - Set display start for 16-bit modes
;
;	Entry:	CX = X
;		DX = Y
;		DS = Seg0 segment
;	Exit:	Carry flag set as follows:
;			CF = Error, couldn't set address
;			NC = No error
;
;	Algorithm:
;		Display start = X / 2 + (Y * row offset * 2)
;
;	Note: 	No need to preserve AX, BX, CX, DX, BP, DI, SI
;
	public	VBE16BitSetDisplayStart
VBE16BitSetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	test	cl,003h				; 4-pixel boundary?
	stc					; ...assume error...
	jnz	VBE16BitSetDisplayStart_exit	;  N: Skip this
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1				; result * 2
	mul	dx				; Y * row offset
	shr	cx,1				; X / 2
	add	ax,cx				; X / 2 + (y * offset * 2)
	adc	dx,0
	call	OEMSetDisplayStart
VBE16BitSetDisplayStart_exit:
	ret
VBE16BitSetDisplayStart	endp
;
;	VBE24BitSetDisplayStart - Set display start for 24-bit modes
;
;	Entry:	CX = X
;		DX = Y
;		DS = Seg0 segment
;	Exit:	Carry flag set as follows:
;			CF = Error, couldn't set address
;			NC = No error
;
;	Algorithm:
;		Display start = ((X / 4) * 3) + (Y * row offset * 2)
;
;	Note: 	No need to preserve AX, BX, CX, DX, BP, DI, SI
;
	public	VBE24BitSetDisplayStart
VBE24BitSetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	imul	cx,cx,3
	test	cl,003h				; Offset can be set?
	stc					;  (Assume error)
	jnz	VBE24BitSetDisplayStart_exit	;  N: Flag error
	shr	cx,2				; CX = (CX * 3) / 4
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1
	mul	dx
	add	ax,cx
	adc	dx,0
	call	OEMSetDisplayStart
	clc
VBE24BitSetDisplayStart_exit:
	ret
VBE24BitSetDisplayStart	endp
;
  IF VESA_STDMODES
;
;	VBETextGetDisplayStart - Get display start for text modes
;
;	Entry:	DS = Seg0 segment
;	Exit:	CX = X
;		DX = Y
;
;	Algorithm:
;		BPC = 8 | 9
;		X = ((display start % (row offset * 2)) * BPC) + pixel pan
;		Y = ((display start / (row offset * 2)) * char height) + preset row scan
;
;	No need to preserve BX, DI, SI
;
	public	VBETextGetDisplayStart
VBETextGetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1
	mov	bx,ax
	call	OEMGetDisplayStart		; DX:AX = Display start
	div	bx
	mov	si,ax				; SI = start / offset
	mov	di,dx				; DI = start % offset
	call	OEMGetCurrentXChar		; AL = 8 or 9
	mov	cl,al				; Save this for later
	cbw
	imul	di,ax				; DI = (start % offset) * BPC
	imul	si,[wCharHeight]		; SI = (start / offset) * char height
;
	call	GetCRTCAddr
	mov	al,08h
	call	GetIndexRegister
	mov	al,ah
	and	ax,01Fh
	add	si,ax				; SI = Y
;
	call	GetPixelPan			; AX = Pixel panning (unadjusted)
	cmp	cl,09h				; 9-dot per character?
	jne	VTGDS_10			;  N: Use panning as is
	inc	ax				;  Y: Adjust for 9-dot modes
	cmp	al,9				;  Blew past end of char?
	jb	VTGDS_10			;   N: Use this value
	sub	al,al				;   Y: Reset to 0
VTGDS_10:
	add	ax,di
	mov	cx,ax				; CX = X
	mov	dx,si				; DX = Y
	ret
VBETextGetDisplayStart	endp
;
;	VBECGAGetDisplayStart - Get display start in modes 4, 5, & 6
;
;	Entry:	DS = Seg0 segment
;	Exit:	CX = X
;		DX = Y
;
;	Algorithm:
;		X = ((display start % row offset) * 8) + pixel pan
;		Y = display start / row offset
;
;	No need to preserve BX, DI, SI
;
	public	VBECGAGetDisplayStart
VBECGAGetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMGetRowOffset			; AX = Row offset
	mov	bx,ax
	call	OEMGetDisplayStart		; DX:AX = Display start
	div	bx
	mov	cx,dx
	mov	dx,ax				; DX = Y
	shl	cx,3				; CX = (start % offset) * 8
	call	GetPixelPan			; AX = Pixel pan
	add	cx,ax				; CX = X
	ret
VBECGAGetDisplayStart	endp
;
  ENDIF		; VESA_STDMODES

;
;	VBEPlanarGetDisplayStart - Get display start for planar modes
;
;	Entry:	DS = Seg0 segment
;	Exit:	CX = X
;		DX = Y
;
;	Algorithm:
;		X = ((display start % (row offset * 2)) * 8) + pixel pan
;		Y = display start / (row offset * 2)
;
;	No need to preserve BX, DI, SI
;
	public	VBEPlanarGetDisplayStart
VBEPlanarGetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1
	mov	bx,ax
	call	OEMGetDisplayStart		; DX:AX = Display start
	div	bx
	mov	cx,dx
	mov	dx,ax				; DX = Y
	shl	cx,3				; CX = (start % offset) * 8
	call	GetPixelPan			; AX = Pixel pan
	add	cx,ax				; CX = X
	ret
VBEPlanarGetDisplayStart	endp
;
;	VBE8BitGetDisplayStart - Get display start for 8 BPP modes
;
;	Entry:	DS = Seg0 segment
;	Exit:	CX = X
;		DX = Y
;
;	Algorithm:
;		X = (display start % (row offset * 2)) * 8
;		Y = display start / (row offset * 2)
;
;	No need to preserve BX, DI, SI
;
	public	VBE8BitGetDisplayStart
VBE8BitGetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1
	mov	bx,ax
	call	OEMGetDisplayStart		; DX:AX = Display start
	div	bx
	mov	cx,dx
	mov	dx,ax				; DX = Y
	shl	cx,2				; CX = (start % offset) * 4
	ret
VBE8BitGetDisplayStart	endp
;
;	VBE16BitGetDisplayStart - Get display start for 16 BPP modes
;
;	Entry:	DS = Seg0 segment
;	Exit:	CX = X
;		DX = Y
;
;	Algorithm:
;		X = (display start % (row offset * 2)) * 2
;		Y = display start / (row offset * 2)
;
;	No need to preserve BX, DI, SI
;
	public	VBE16BitGetDisplayStart
VBE16BitGetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1
	mov	bx,ax
	call	OEMGetDisplayStart		; DX:AX = Display start
	div	bx
	mov	cx,dx
	mov	dx,ax				; DX = Y
	shl	cx,1
	ret
VBE16BitGetDisplayStart	endp
;		    
;	VBE24BitGetDisplayStart - Get display start in 24 BPP mode
;
;	Entry:	DS = Seg0 segment
;	Exit:	CX = X
;		DX = Y
;
;	Algorithm:
;		X = ((display start % (row offset * 2)) * 4) / 3
;		Y = display start / (row offset * 2)
;
;	No need to preserve BX, DI, SI
;
	public	VBE24BitGetDisplayStart
VBE24BitGetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	call	OEMGetRowOffset			; AX = Row offset
	shl	ax,1
	mov	bx,ax
	call	OEMGetDisplayStart		; DX:AX = Display start
	div	bx
	push	ax				; Save Y value
	mov	ax,dx
	cwd					; DX = 0
	shl	ax,2				; AX = (start % offset) * 4
	mov	cx,3
	div	cx				; AX = AX / 3
	mov	cx,ax				; CX = X
	pop	dx				; Restore Y value and return it
	ret
VBE24BitGetDisplayStart	endp
;
;	SetPixelPan - Set the pixel panning value
;
;	Entry:	AL = Pixel panning value
;	Exit:	None
;
	public	SetPixelPan
SetPixelPan	proc
	assume	ds:nothing, es:nothing
	push	dx
	cli
	call	ResetATCFlipFlop
	mov	dx,03C0h
	push	ax
	mov	al,033h
	out	dx,al
	pop	ax
	out	dx,al
	sti
	pop	dx
	ret
SetPixelPan	endp
;
;	GetPixelPan - Get the current pixel panning value
;
;	Entry:	None
;	Exit:	AX = Pixel panning value
;
	public	GetPixelPan
GetPixelPan	proc
	assume	ds:nothing, es:nothing
	push	dx
	cli
	call	ResetATCFlipFlop
	mov	dx,03C0h
	mov	al,033h
	out	dx,al
	inc	dx
	in	al,dx
	sti
	cbw
	pop	dx
	ret
GetPixelPan	endp
;
;	VBE Function 08h
;	VBE_DACFormat - Set/Get DAC Palette format
;
;	Entry:	BL = Subfunction
;			00h = Set DAC Palette Format
;			01h = Get DAC Palette Format
;		BH = Current number of bits of color per primary (Set)
;		DS = Seg0 segment
;	Exit:	AX = Success flag
;		BH = Current number of bits of color per primary
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_DACFormat
VBE_DACFormat	proc	near
	assume	ds:Seg0, es:nothing
	push	es
	cmp	bl,001h				; Value out of range?
	ja	short VBE_DACFormat_badexit	;  Y: Exit with error
	jb	short VBE_DACFormat_Set		;  N: =0?, Set DAC format
;
VBE_DACFormat_Get:
	call	OEMGetDACFormat
	mov	bh,al				; Return 6 or 8-bits per gun
	mov	ax,0004Fh			; Flag success
VBE_DACFormat_exit:
	pop	es
	ret
;
VBE_DACFormat_badexit:
	mov	ax,0014Fh			; Subfunction out of range
	jmp	short VBE_DACFormat_exit
;
VBE_DACFormat_Set:
	push	bx				; Preserve BL
	mov	al,[byMode]
	call	GetModeDescPtr			; ES:BX = Ptr to mode description
	cmp	es:[bx].MODEDESC.mdBPP,8	; Is this mode 8 BPP or less?
	pop	bx				;  ...flags not affected...
	ja	short VBE_DACFormat_badmodeexit	;  N: Return error (according to VBE spec)
;
	mov	al,bh
	call	OEMSetDACFormat			; Set 6-bit/8-bit per gun
	jc	short VBE_DACFormat_badexit	; If bad value, return error
	jmp	short VBE_DACFormat_Get		; Return current setting
;
VBE_DACFormat_badmodeexit:
	mov	ax,0034Fh			; Not supported in current mode
	jmp	short VBE_DACFormat_exit
VBE_DACFormat	endp
;
;	VBE Function 09h
;	VBE_DACData - Set/Get DAC Palette Data
;
;	Entry:	BL = Subfunction:
;			00h = Set Palette data
;			01h = Get Palette data
;			02h = Set Secondary Palette data
;			03h = Get Secondary Palette data
;			80h = Set Palette data during vertical retrace with blanking
;		CX = Number of palette registers
;		DX = First palette register
;		ES:DI = Pointer to DAC data
;		DS = Seg0 segment
;	Exit:	AX = Success flag
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_DACData
VBE_DACData	proc	near
	assume	ds:Seg0, es:nothing
	test	bl,bl				; Subfunction 0?
	jz	short VBE_DACData_Set		;  Y: Go handle
	cmp	bl,1				; Subfunction 1?
	je	short VBE_DACData_Get		;  Y: Go handle
	cmp	bl,3				; Subfunction 2 or 3?
	jbe	short VBE_DACData_2nd		;  Y: Go flag error
	cmp	bl,80h				; Subfunction 80h?
	jne	short VBE_DACData_badexit	;  N: Go flag error
	push	dx
	call	WaitVerticalRetrace
	pop	dx
;
; Set DAC data
;
VBE_DACData_Set:
	pushad
	push	ds
	push	es
	pop	ds
	mov	si,di
	mov	al,dl
	mov	dx,DAC_WINDEX
	out	dx,al
	inc	dx				; DX = DAC Data (3C9h)
VBE_DACData_Set_10:
	lodsd
	ror	eax,16
	out	dx,al				; Red
	rol	eax,8
	out	dx,al				; Green
	rol	eax,8
	out	dx,al				; Blue
	loop	VBE_DACData_Set_10
	pop	ds
	popad
VBE_DACData_succexit:
	mov	ax,0004Fh
	ret
;
; Get DAC data
;
VBE_DACData_Get:
	pushad
	mov	al,dl
	mov	dx,DAC_RINDEX
	out	dx,al
	inc	dx
	inc	dx				; DX = DAC Data (3C9h)
VBE_DACData_Get_10:
	sub	eax,eax
	in	al,dx				; Red
	shl	eax,8
	in	al,dx				; Green
	shl	eax,8
	in	al,dx				; Blue
	stosd
	loop	VBE_DACData_Get_10
	popad
	jmp	short VBE_DACData_succexit
;
VBE_DACData_badexit:
	mov	ax,0014Fh			; Subfunction out of range
	ret
VBE_DACData_2nd:
	mov	ax,0024Fh			; No hardware support
	ret
VBE_DACData	endp
;
;	VBE Function 0Ah
;	VBE_PMI - VBE Protected Mode Interface
;
;	Entry:	BL = 0: Return protected mode table
;		DS = Seg0 segment
;	Exit:	AX = Success flag
;		ES:DI = Pointer to table (real mode segment:offset)
;		CX = Length of table
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_PMI
VBE_PMI	proc
	assume	ds:Seg0, es:nothing
	mov	di,offset tblProtectedModeInterface
	push	cs
	pop	es				; ES:DI = Pointer to PMI table
	mov	cx,LENGTH_PMITABLE
	mov	ax,004Fh
	ret
VBE_PMI	endp
;
  IF VESA_300
;
;	VBE Function 0Bh
;	VBE_DotClock - Get/Set pixel clock
;
;	Entry:	DS = Seg0 segment
;		BL = Subfunction
;		DX = Mode number
;
;		Other registers dependent on subfunction
;		Subfunction 0:		Get closest pixel clock
;			ECX = Requested pixel clock in units of Hz
;
;	Exit:	AX = Success flag
;		ECX = Closest pixel clock
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_DotClock
VBE_DotClock	proc
	assume	ds:Seg0, es:nothing
	test	bl,bl
	jnz	short VBE_DotClock_error
	call	OEMGetDotClock
	mov	ax,004Fh
	ret
;
VBE_DotClock_error:
	mov	ax,014Fh
	ret
VBE_DotClock	endp
;
  ENDIF
;
  IF VESA_DPMS
;
;	VBE Function 10h
;	VBE_DPMS - Display Power Management Signaling functions (DPMS).
;
;	Entry:	AX = 4F10h
;		BL = Subfunction
;		ES:DI = Null pointer
;		DS = Seg0
;
;		Other registers dependent on subfunction
;		Subfunction 0:
;			None
;		Subfunction 1:
;			BH = Requested power state
;			     0 = On
;			     1 = Stand by
;			     2 = Suspend
;			     4 = Off
;			     8 = Reduced on
;		Subfunction 2:
;			None
;
;	Exit:	AX = VBE return status
;		Other registers dependent on subfunction:
;		Subfunction 0:
;			BH = States supported:
;			     bit 0	Stand by
;			     bit 1	Suspend
;			     bit 2	Off
;			     bit 3	Reduced on
;			     bits 4-7	Reserved
;			BL = VBE/PM Version number:
;			     bits 0-3	Minor version number (0)
;			     bits 4-7	Major version number (1)
;		Subfunction 1:
;			None
;		Subfunction 2:
;			BH = Power state currently requested by controller
;			     0 = On
;			     1 = Stand by
;			     2 = Suspend
;			     4 = Off
;			     8 = Reduced on
;
;	The VESA committee defined a method of signalling a monitor
;	to shutdown or to go into standby mode. The sync signals are
;	used in the following manner:
;
;	H Sync	V Sync	Flag  Result
;	======	======	====  ======
;	Pulses	Pulses	0     Monitor is active
;	None	Pulses	1     Monitor is in "stand-by" mode
;	Pulses	None	2     Monitor is in "suspend" mode
;	None	None	4     Monitor is in "shut down" mode
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_DPMS
VBE_DPMS	proc
	assume	ds:Seg0, es:nothing
	mov	ax,0014Fh		; Assume error
	cmp	bl,2			; Subfunction 2?
	ja	short VBE_DPMS_exit	;  >: Error, no such subfunction
	je	short VBE_DPMS_20	;  =: Go do subfunction 2
	test	bl,bl			; Subfunction 0?
	jnz	short VBE_DPMS_10	;  N: Must be subfunction 1
;
; Function 0 - Report VBE/PM Capabilities
;
	mov	bx,0710h		; "Standby", "Suspend" and "Off", version 1.0
	jmp	short VBE_DPMS_succexit
;
; Function 1 - Set Display Power State
;
VBE_DPMS_10:
	call	OEMSetDPMS
	jc	VBE_DPMS_exit		; If error, return error (AX = 014Fh)
	jmp	short VBE_DPMS_succexit
;
; Function 2 - Get Display Power State
;
VBE_DPMS_20:
	call	OEMGetDPMS
VBE_DPMS_succexit:
	mov	ax,004Fh		; Successful function
VBE_DPMS_exit:
	ret
VBE_DPMS	endp
;
  ENDIF
  IF VESA_DDC
;
;	VBE_DDC - Handles entry to VESA DDC function
;
;	Entry:	AX = 4F15h
;		BL = DDC sub-function
;		CX = Controller ID (00 = primary controller)
;		Other registers are sub-function dependent
;	Exit:	AX = Status
;		Other registers are sub-function dependent
;
;	Note:	DS, SI have been pushed on the stack already
;
	public	VBE_DDC
VBE_DDC	proc
	assume	ds:nothing, es:nothing
	call	OEMEnableExtensions
	call	Read_And_Shadow_IO_Base

;Need to reset the DDC device before talking to it.
	call	DDCStartupSequence
	call	DDCStart
	call	DDCDelay
	call	DDCStop
	call	DDCDelay

	mov	ah,001h				; Assume failure
	cmp	bl,1				; Read EDID function?
	jb	DDCStatus			;  N: If status, go handle
	je	DDCReadEDID			;  Y: Go handle read
	cmp	bl,2				; Read VDIF function?
	je	DDCReadVDIF			;  Y: Go handle
VBE_DDC_exit:
	mov	al,04Fh
	ret
;
;	DDCStatus - Return DDC status
;
;	Entry:	ES:DI = NULL pointer (must be 0:0 in version 1.0)
;	Exit:	BH = Approx time in seconds, rounded up, to transfer
;			one EDID block (128 bytes)
;		BL = DDC Level supported:
;			bit 0 = 0	DDC1 not supported
;			bit 0 = 1	DDC1 supported
;			bit 1 = 0	DDC2 not supported
;			bit 1 = 1	DDC2 supported
;			bit 2 = 0	Screen not blanked during data transfer
;			bit 2 = 1	Screen blanked during data transfer
;
	public	DDCStatus
DDCStatus	label	near
	cli					; Don't let system interfere
	sub	bx,bx				; Assume error
	sub	al,al				; Read byte at address 0
	call	DDCStartupSequence
	jz	DDCStatus_err			; Error? Go flag no support
	push	cx
	sub	cx,cx				; Indicating ACK is needed
	call	DDCRead				; Read the byte?
	pop	cx				; No effect on ZF
	jz	DDCStatus_err			;  N: Error. Flag no support
	mov	bx,00802h			; DDC2 supported, 8 secs download
DDCStatus_err:
	sub	ah,ah				; Flag function passed (even if monitor doesn't)
	call	DDCStop
	call	DDCDisable
	sti					; Allow interrupts again
	jmp	short VBE_DDC_exit
;
;	DDCReadEDID - Return EDID
;
;	Entry:	DX = Block number (must be 0 in version 1.0)
;		ES:DI = EDID data buffer
;	Exit:	ES:DI = Data stored in EDID buffer
;		AH = Status flag
;
	public	DDCReadEDID
DDCReadEDID	label	near
	test	dx,dx				; Block 0?
	jnz	DDCReadEDID_err			;  N: Violates spec

	mov	si,8				; Max number of attempts
DDCReadEDID_10:
	cli
	mov	cx,128				; Size of EDID
	sub	al,al				; Start at address 0
	push	di
	call	DDCStartupSequence
	jz	DDCReadEDID_err1
DDCReadEDID_20:
	call	DDCRead				; Read byte at specified address
	jz	DDCReadEDID_err1
	stosb					; Save byte from monitor
	loop	DDCReadEDID_20
	pop	di
	call	DDCStop
	call	DDCDisable
	sti
	call	DDCChecksum			; Did monitor send valid data?
	jz	DDCReadEDID_Success		;  Y: Go exit
	call	WaitVerticalRetrace		; Wait a while (let monitor relax)
	dec	si
	jnz	DDCReadEDID_10			; Go try again
;
DDCReadEDID_err:
	call	DDCDisable
	sti
	mov	ah,01h				; Flag error
	jmp	short VBE_DDC_exit
;
DDCReadEDID_err1:
	pop	di				; Balance the stack
	jmp	short DDCReadEDID_err
;
DDCReadEDID_Success:
	sub	ah,ah				; Flag passage
	jmp	short VBE_DDC_exit
;
;	Return VDIF block
;
;	Entry:	DX = VDIF block number
;		ES:DI = VDIF data buffer
;	Exit:	ES:DI = Data stored in VDIF buffer
;
	public	DDCReadVDIF
DDCReadVDIF	label	near
;;	push	bx
;;	call	ReadVDIF
;;	pop	bx
;;	sub	ah,ah				; Flag passage
	mov	ah,01h
	jmp	VBE_DDC_exit
VBE_DDC	endp
;
;	DDCStartupSequence - Perform the startup sequence to read at a given address
;
;	Entry:	AL = Start address
;	Exit:	Zero flag is set as follows:
;			ZF = Error occurred
;			NZ = No error occurred
;
;	AL is trashed, all other registers are preserved.
;	Assume extensions are enabled.
;	Assume interrupts are disabled.
;
	public	DDCStartupSequence
DDCStartupSequence	proc
	assume	ds:nothing, es:nothing
IF DVI_BIOS
	call	IsCRTAttached				;See if the CRT is attached
	jnz 	ReadCRT_EDID				;Jump if CRT attached
  call  IsPanelAttached			;See if panel attached
  jz    ReadCRT_EDID				;Jump if no panel attached

	call	DDCEnableDFP
	jz	   DDCStartupSequence_exit

	jmp	short StartReadEDID
ReadCRT_EDID:
ENDIF

	call	DDCEnable		; Enable DDC2 interface
	jz	DDCStartupSequence_exit

StartReadEDID:
	call	DDCStart		; Send start sequence
	jz	DDCStartupSequence_exit
	push	ax
	mov	al,0A0h			; Send "write" command
	call	DDCWrite
	pop	ax			; ** DOESN'T AFFECT FLAGS **
	jz	DDCStartupSequence_exit
	call	DDCWrite		; AL = Start address
	jz	DDCStartupSequence_exit
	call	DDCStart
	mov	al,0A1h			; Send "read" command
	call	DDCWrite
DDCStartupSequence_exit:
	ret
DDCStartupSequence	endp
;
;	DDCEnable - Enable the DDC interface
;
;	Entry:	None
;	Exit:	Zero flag is set as follows:
;			ZF = Error occurred
;			NZ = No error occurred
;
;	All registers are preserved.
;	Assume that extensions are enabled.
;	Assume interrupts are disabled.
;
	public	DDCEnable
DDCEnable	proc
	assume	ds:nothing, es:nothing
	pusha
;
	call	OEMDDCEnable
	call	DDCDelay
;
; Toggle the clock to force a switch to DDC2
;
	call	OEMDDCClockHighDataHigh
	call	DDCDelay
;
; Wait until they release the data line or the clock line
;
	mov	bx,40			; Allow "a long" time for transition
DDCEnable_05:
	sub	cx,cx
DDCEnable_10:
	call	OEMDDCIsClockHigh
	jnz	DDCEnable_20
	call	OEMDDCIsDataHigh
	jnz	DDCEnable_20
	dec	bx
	jnz	DDCEnable_05
DDCEnable_20:
;
	or	cx,bx			; Return ZF as error
;
	popa
	ret
DDCEnable	endp

;**********************************************************************

DDCEnableDFP		proc	near
	assume	ds:nothing, es:nothing
	pusha
;
	call	OEMDDCEnableDFP
	call	DDCDelay
;
; Toggle the clock to force a switch to DDC2
;
	call	OEMDDCClockHighDataHigh
	call	DDCDelay
;
; Wait until they release the data line or the clock line
;
	mov	bx,40			; Allow "a long" time for transition
DDCEnableDFP_05:
	sub	cx,cx
DDCEnableDFP_10:
	call	OEMDDCIsClockHigh
	jnz	DDCEnableDFP_20
	call	OEMDDCIsDataHigh
	jnz	DDCEnableDFP_20
	dec	bx
	jnz	DDCEnableDFP_05
DDCEnableDFP_20:
;
	or	cx,bx			; Return ZF as error
;
	popa
	ret
DDCEnableDFP		endp


;**********************************************************************

;
;	DDCDelay - Allow monitor time to react (just barely)
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;
	public	DDCDelay
DDCDelay	proc
	assume	ds:nothing, es:nothing
	push	cx

;Test to slow DDC transactions for slower monitors
	mov	cx,300
;;	mov	cx,50
DDCDelay_10:
	jmp	$+2
	loop	DDCDelay_10
	pop	cx
	ret
DDCDelay	endp
;
;	DDCStart - Send a "start" condition over the access.bus
;
;	Entry:	None
;	Exit:	Zero flag is set as follows:
;			ZF = Error occurred
;			NZ = No error occurred
;
;	All registers are preserved.
;	Assume that extensions are enabled.
;	Assume interrupts are disabled.
;
;	Assume IDOUT was set to 0. (JCL)
;
	public	DDCStart
DDCStart	proc
	assume	ds:nothing, es:nothing
	pusha
;
; Raise clock and data
;
	call	OEMDDCClockHighDataHigh
	call	DDCDelay
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
;
; Wait for clock to go high
;
	call	DDCWaitClockHigh
;
; Switch data from high to low
;
	call	OEMDDCClockHighDataLow
	call	DDCDelay
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
;
; Switch clock from high to low
;
	call	OEMDDCClockLowDataLow
	call	DDCDelay
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
;
	sub	cx,cx
	inc	cx
	popa
	ret
DDCStart	endp
;
;	DDCDisable - Disable DDC Interface
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;	Assume extensions are enabled.
;	Assume interrupts are disabled.
;	All registers are preserved.
;
	public	DDCDisable
DDCDisable	proc
	assume	ds:nothing, es:nothing
	call	OEMDDCDisable
	ret
DDCDisable	endp
;
;	DDCChecksum - Do a checksum of the buffer
;
;	Entry:	ES:DI = Pointer to buffer
;	Exit:	Zero flag set as follows:
;			ZF = Checksum passed
;			NZ = Checksum failed
;
;	All registers are preserved.
;
	public	DDCChecksum
DDCChecksum	proc
	pusha
	mov	cx,128
	sub	al,al
DDCChecksum_10:
	add	al,es:[di]
	inc	di
	loop	DDCChecksum_10
;
; KLUDGE - If checksum is 0, but the entire buffer is filled with zeros,
; then a valid EDID has NOT been received. Verify that the EDID is valid
; by checking that the first two bytes of the buffer is 000h and 0FFh.
;
	test	al,al
	jnz	short DDCChecksum_20
	cmp	word ptr es:[di-80h],0FF00h
DDCChecksum_20:
;
; KLUDGE - End
;
	popa
	ret
DDCChecksum	endp
;
;	DDCStop - Send a "stop" condition over the access.bus
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;	Assume extensions are enabled.
;	Assume clock is low.
;	Assume interrupts are disabled.
;
	public	DDCStop
DDCStop	proc
	assume	ds:nothing, es:nothing
	pusha
;
; Make data low
;
	call	OEMDDCClockLowDataLow
	call	DDCDelay
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
;
; Raise the clock, keep data low
;
	call	OEMDDCClockHighDataLow
	call	DDCDelay
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	DDCWaitClockHigh
;
; Raise the data, keep clock high
;
	call	OEMDDCClockHighDataHigh
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
;
	popa
	ret
DDCStop	endp
;
;	DDCWaitClockHigh - Wait for DDC clock to go high
;
;	Entry:	None
;	Exit: 	Zero flag set as follows:
;			ZF = Error occurred
;			NZ = No error occurred
;
	public	DDCWaitClockHigh
DDCWaitClockHigh	proc
	push	bx
;
	mov	bx,2
DDCWaitClockHigh_10:

;	sub	cx,cx
	mov	cx,100h

DDCWaitClockHigh_20:
	call	OEMDDCIsClockHigh
	jnz	DDCWaitClockHigh_30
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	loop	DDCWaitClockHigh_20
	dec	bx
	jnz	DDCWaitClockHigh_10
DDCWaitClockHigh_30:
	or	cx,bx			; Return ZF as error
;
	pop	bx
	ret
DDCWaitClockHigh	endp
;
;	DDCWrite - Write a byte over the display data channel
;
;	Entry:	AL = Byte to write
;	Exit:	Zero flag is set as follows:
;			ZF = Error occurred
;			NZ = No error occurred
;
;	All registers are preserved.
;	Assume extensions are enabled.
;	Assume clock is low.
;	Assume interrupts are disabled.
;
	public	DDCWrite
DDCWrite	proc
	assume	ds:nothing, es:nothing
	pusha
;
; Write the eight bits of data
;
	mov	si,8
DDCWrite_10:
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	test	al,80h			; Write a one bit?
	jz	DDCWrite_20		;  N: Go write a zero
;
; Write a one bit
;
	call	OEMDDCClockLowDataHigh
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	OEMDDCClockHighDataHigh
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	DDCWaitClockHigh
	jz	DDCWrite_error
	call	OEMDDCClockLowDataHigh
	jmp	short DDCWrite_30
;
; Write a zero bit
;
DDCWrite_20:
	call	OEMDDCClockLowDataLow
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	OEMDDCClockHighDataLow
	call	DDCWaitClockHigh
	jz	DDCWrite_error
	call	OEMDDCClockLowDataLow
;
DDCWrite_30:
	shl	al,1
	dec	si
	jnz	DDCWrite_10
;
; Generate a pulse for the slave to release the data bus (ACK)
;
	call	OEMDDCClockLowDataHigh
	call	OEMDDCClockHighDataHigh
	call	DDCDelay
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	DDCDelay	  
	call	DDCWaitClockHigh		; CX is non-zero on non-error
	jz	DDCWrite_error
	call	OEMDDCClockLowDataHigh
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
;
DDCWrite_exit:
	test	cx,cx				; Clear ZF for no error
	popa
	ret
;
DDCWrite_error:
	sub	cx,cx				; Set ZF for error
	jmp	short DDCWrite_exit
DDCWrite	endp
;
;	DDCRead - Read a byte from the display data channel
;
;	Entry:	None
;	Exit:	AL = Byte read
;		Zero flag is set as follows:
;			ZF = Error occurred
;			NZ = No error occurred
;
;	Assume extensions are enabled.
;	Assume clock is low.
;	Assume interrupts are disabled.
;
	public	DDCRead
DDCRead	proc
	assume	ds:nothing, es:nothing
	push	bx
	push	cx
	push	dx
	push	si
;
	sub	bl,bl				; BL = Accumulated value
	mov	si,8
DDCRead_10:
	shl	bl,1
	call	OEMDDCClockHighDataHigh
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	DDCDelay	  
	call	DDCDelay	  
	call	DDCDelay	  
	push	cx
	call	DDCWaitClockHigh
	pop	cx
	jz	DDCRead_exit
	call	OEMDDCIsDataHigh
	jz	DDCRead_20
	or	bl,1
DDCRead_20:
	call	OEMDDCClockLowDataHigh		; Lower the clock
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	DDCDelay	  
	call	DDCDelay	  
	call	DDCDelay	  
	dec	si
	jnz	DDCRead_10
;
	test	cx,cx				; Send NACK?
;	test	dx,dx				; Send ACK?
	jz	DDCRead_30			;  N: Go send NACK
;
; Send ACK
;
	call	OEMDDCClockLowDataLow		; Lower the clock, lower the data
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	OEMDDCClockHighDataLow		; Raise the clock, lower the data
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	DDCWaitClockHigh
	jz	DDCRead_exit
	call	OEMDDCClockLowDataLow		; Lower the clock, lower the data
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	OEMDDCClockLowDataHigh		; Lower the clock, float the data
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	jmp	short DDCRead_40
;
; Send NACK
;
DDCRead_30:
	call	OEMDDCClockLowDataHigh		; Lower the clock, float the data
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	OEMDDCClockHighDataHigh		; Raise the clock, keep data high
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	call	DDCWaitClockHigh		; CX is non-zero on non-error
	jz	DDCRead_exit
	call	OEMDDCClockLowDataHigh		; Lower the clock, float the data
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
;
DDCRead_40:
	call	DDCDelay
;Try double delay for older monitors
	call	DDCDelay	  
	mov	al,bl
;
DDCRead_exit:
	test	cx,cx				; Set/clear ZF on error/non-error
	pop	si
	pop	dx
	pop	cx
	pop	bx
	ret
DDCRead	endp
;
  ENDIF
;
  IF VESA_300
;
;	VBE_PMEntryPoint - INT 10h entry point replacement for protected mode
;
;	Entry:	Dependent on function
;	Exit:	Dependent on function
;
	public	VBE_PMEntryPoint
VBE_PMEntryPoint	proc
	assume	ds:nothing, es:nothing
	cmp	ah,04Fh				; VBE function?
	jne	short VBE_PMEntryPoint_retf	;  N: Not supported
	push	si
	push	ds
	call	GetDSSeg0
	cmp	al,LENGTH_VESADISPATCHTABLE
	jae	short VBE_PMEntryPoint_unsupported
	cbw
	shl	ax,1
	mov	si,ax
	call	cs:[si + offset tblHandlerVESA]
VBE_PMEntryPoint_exit:
	pop	ds
	pop	si
VBE_PMEntryPoint_retf:
	retf
;
VBE_PMEntryPoint_unsupported:
	mov	ax,0100h		; Unsuccessful and unsupported function
	jmp	short VBE_PMEntryPoint_exit
VBE_PMEntryPoint	endp
;
;	VBE_PMInitialization - Protected mode initialization routine
;
;	Entry:	None
;	Exit:	None
;
	public	VBE_PMInitialization
VBE_PMInitialization	proc
	push	ds
	push	es
;
	mov	ds,cs:[selCodeSeg]		; DS = Writeable code selector
	call	GetESSeg0
	assume	es:Seg0, ds:_TEXT
	mov	es:[offINT1F],offset tblFont8x8 + 128*8
	mov	es:[segINT1F],ds
	mov	es:[offINT43],offset tblFont8x8
	mov	es:[segINT43],ds
	mov	word ptr es:[lpSavePtr],offset tblSavePtr
	mov	word ptr es:[lpSavePtr+2],ds
;
; Do a similar initialization to a TSR implementation of the BIOS
;
	mov	[segInitParm],ds
	mov	[segInit2ndSP],ds
	mov	[segInitDCC],ds
	mov	[segInitOEM],ds
	cmp	[segOEMExtRegs],0		; Is this a NULL ptr?
	jz	short VBE_PMInit_10		;  Y: Leave it a NULL ptr
	mov	[segOEMExtRegs],ds		;  N: Update to the new segment
VBE_PMInit_10:
	mov	[segOEMModeDesc],ds
	mov	[segfi8x14],ds
	mov	[segfi8x8],ds
	mov	[segfi8x8Top],ds
	mov	[segfi9x14],ds
	mov	[segfi8x16],ds
	mov	[segfi9x16],ds
	mov	[segROM8x8],ds
	mov	[segROM8x8Top],ds
	mov	[segROM8x14],ds
	mov	[segROM8x16],ds
	mov	[segROM9x14],ds
	mov	[segROM9x16],ds
	mov	[selCodeSeg],ds
;
	mov	[segfiINT1F],es			; Initialize to Seg0
	mov	[segfiINT43],es
;
	call	OEMProtectedModeInit
	pop	es
	pop	ds
	retf
VBE_PMInitialization	endp
;
  ENDIF
;
ENDIF

;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


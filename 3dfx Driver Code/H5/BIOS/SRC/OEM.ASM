	page	57, 132
	title	OEM Specific Routines
	subttl	OEM.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	OEM.ASM - OEM specific routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	OEMInit			Called after waking up the VGA
;	OEMWakeUpVGA		Wake up the VGA after being put to sleep by "SleepVGA".
;	OEMSleepVGA		Put the VGA to sleep
;	OEMGetParmTable		Return the pointer to parameter table
;	OEMGetInternalMode	Get the internal mode number for a given VGA mode
;	OEMPreSetRegs		Make special preparations to load registers
;	OEMSetRegs		Set extended registers (standard VGA has been set)
;	OEMClearScreen		Clear out the regen buffer
;	OEMFixupFont		Fixup the current font
;3	OEMWriteRAMDACIdx	Write a RAMDAC index register
;3	OEMWriteRAMDACData	Write a RAMDAC data register
;3	OEMReadRAMDAC		Read a RAMDAC register
;V	OEMFixupVBEModeInfo	Fixup the MODEINFOBLOCK information
;V	OEMGetMemSize		Return the pre-calculated memory size
;V	OEMGetLinearMemBase	Return the address of the linear framebuffer
;V	OEMSetLinearMemBase	Set the current mode to use the linear frame buffer
;V	OEMIsInLinearMode	Determine if current mode is using linear addressing
;V	OEMEnableExtensions	Enable access to extended registers
;V	OEMIsModeAvailable	Determine if mode is available with current hardware
;V	OEMSaveExtState		Save extended registers
;V	OEMRestoreExtState	Restore extended registers
;V	OEMPreSaveRestore	Set up any registers needed for access
;V	OEMPostSaveRestore	Reset any register set by pre call
;V	OEMGetCurrentXChar	Return the width (in pixels) of a character
;V	OEMSetRowOffset		Set the row offset in the CRTC
;V	OEMGetRowOffset		Get the row offset from the CRTC
;V	OEMAdjustMaxRowOffset	Adjust the maximum row offset value
;V	OEMSetDisplayStart	Set the CRTC display start address
;V	OEMGetDisplayStart	Get the CRTC display start address
;V	OEMSetDACFormat		Set 6-bits or 8-bits per gun
;V	OEMGetDACFormat		Return 6-bits or 8-bits per gun
;V3	OEMGetMaxDotClock	Return the maximum dot clock
;V3	OEMGetDotClock		Get closest dot clock
;V3	OEMFixupCRTC		Fixup the current mode with a user CRTC table
;V3	OEMProtectedModeInit	Protected mode initialization
;VDPMS	OEMSetDPMS		Set display power state
;VDPMS	OEMGetDPMS		Get display power state
;VDDC	OEMDDCClockHighDataHigh	Set clock high, data high
;VDDC	OEMDDCClockLowDataHigh	Set clock low, data high
;VDDC	OEMDDCClockHighDataLow	Set clock high, data low
;VDDC	OEMDDCClockLowDataLow	Set clock low, data low
;VDDC	OEMDDCIsClockHigh	Return status of clock line
;VDDC	OEMDDCIsDataHigh	Return status of data line
;VDDC	OEMDDCEnable		Enable the DDC protocol
;VDDC	OEMDDCDisable		Disable the DDC protocol
;V/E	OEMSetBank		Set the aperature to the given bank number
;V/E	OEMGetWBank		Return the current bank for writing
;E	OEMIncBank		Set the aperature to the next bank number
;E	OEMSetRBank		Set the aperature to the given bank number
;E	OEMIncRBank		Set the aperature to the next bank number (Read bank only)
;E	OEMDecRBank		Set the aperature to the next bank number (Read bank only)
;E	OEMSetWBank		Set the aperature to the given bank number (Write bank only)
;E	OEMIncWBank		Set the aperature to the next bank number (Write bank only)
;E	OEMDecWBank		Set the aperature to the next bank number (Write bank only)
;E	OEMLoad16BPPDAC		Load 16 BPP RAMDAC settings
;E	OEMLoad24BPPDAC		Load 24 BPP RAMDAC settings
;E	OEMExtensions1		Extended OEM functions #1
;E	OEMExtensions2		Extended OEM functions #2
;E	OEMExtensionsAlt	Extended OEM functions on failed ALT-select lookup
;P	OEMGetBusType		Return the expansion bus type
;	GetIOBase		Return the I/O base for the 3Dfx extended I/O
;	GetMemBase		Return the memory base for the 3Dfx memory space
;	IsVGAMode		Determine if the current mode is in VGA mode
;	SetAbsoluteDotClock	Set the pixel clock to the given frequency
;	CalcClockWord		Calculate M,N,P values for a given frequency
;	CalcFreq		Calculate the frequency in HZ from a given M,N,P set
;	WhatIsClockWord		Determine the frequence of a given clock word
;	GetExtRegsEntry		Get a pointer to a given mode's extended regsiters
;	I2CInit			Initialize the I2C interface
;	I2CDelay		Delay long enough to give the I2C hardware time to catch up
;	I2CStart		Send the start strobe up the I2C line
;	I2CStop			Send the stop strobe up the I2C line
;	I2CData0		Write a 0 to the I2C data line
;	I2CData1		Write a 1 to the I2C data line
;	I2CClock0		Write a 0 to the I2C clock line
;	I2CClock1		Write a 1 to the I2C clock line
;	I2CDataReadBit		Read the SDA line
;	I2CClockReadBit		Read the SCL line
;	I2CWriteRegister	Write an I2C register
;	I2CSendByte		Send a BYTE of data down the I2C pipeline
;	IsNTSCMode		Determine if the current configuration is NTSC output
;	IsNTSCMode		Determine if the current configuration is NTSC output
;	FixupTVOut		Fixup the CRTC for TV output
; IsPanelAttached         Determine if the panel was detected during POST
; CheckForPanel           Check if panel currently connected
; I2CWriteRegister_Panel  Writes I2C data to the panel chip.
; Program_Panel           Programs panel with table driven parameters
; Panel                   Programs panel for the particular mode.
;	PatchVertDisplayTime		Patch Vertical Display time and VerticalX2 register.
;
;	$Revision: 17 $
;	$Date: 9/09/99 3:38p $
;	$Logfile: /devel/h5/BIOS/SRC/OEM.ASM $
;	$Log: /devel/h5/BIOS/SRC/OEM.ASM $
;; 
;; 17    9/09/99 3:38p Kaymann
;; Created externs for the 640x480, 800x600, and 1024x768 VGA tables.
;; 
;; In FixupPanel,
;; 1) Added support for doing 320x200, 320x240, 512x384, and 400x300
;; modes.
;; 2) Modified the code which patched CR02 to decreament the CR02 value
;;    by 1 instead of copying the CR01 value.  This helped fixing the
;;    problem with halved modes not coming up.
;; 
;; In ParseAndProgramDetailedTimings,
;; 1) CR01, Horizontal Display End,  is NOT updated when in doing a halved
;;    mode.
;; 2) CR05, Horisontal sync End, is foced to 1Fh for halved modes.
;; 
;; Created the routine ProgramCRTCPanelTimings read and program the CRTC
;; timings in BIOSPARM.INC.
;; 
;; Created the routine IsHalvedExtMode to return if we're a halved
;; extende mode mode.
;; 
;; 16    9/04/99 5:51p Kaymann
;; In the FixUpPanel routine, moved the code which set REG_VIDINFORMAT to
;; 8000h to the top of the routine.  This ensures that REG_VIDINFORMAT is
;; always set when programming the panel.
;; 
;; 15    9/02/99 5:52p Kaymann
;; Created the routine IsCRTAttached to return if the CRT is active.
;; 
;; Added code into the FixupPanel return to support programming the
;; CRTC and PLL registers for scaled panels.
;; 
;; Created the routine GetDetailedTimingXYRes to parse a panel EDID
;; detailed timing block and return the XY resolution for that detailed
;; timing block.
;; 
;; Created the routine ParseAndProgramDetailedTiming to parse a detailed
;; timing block, calculate the CRTC and PLL parameters, and program the
;; CRTC and PLL parameters.
;; 
;; Created the routine DoWePatchVGAmodes to return if we need to patch the
;; CRTC parameters for the current VGA mode.
;; 
;; Modified the routine I2CReadByte to preserve DX.
;; 
;; 14    8/04/99 6:31p Kaymann
;; In FixupPanel, added code to handle progamming the VGA registers for
;; non-scalable panels.
;; 
;; 13    7/28/99 2:53p Kaymann
;; Moved the CheckForPanel and CheckFPSideScaling routines to OEMPOST.ASM
;; since this is actually once during POST.
;; 
;; Created the routine IsPanelSideScaling to return if the attached
;; panel support panel side scaling.
;; 
;; Modified the GetPanelSize routine to read the panel size from
;; SCRATCH_REG3.
;; 
;; Modified the routine I2CReadByte to PUSH ans POP AX.  AX needs to be
;; preserved for the Panel sizing routine.
;; 
;; Modified IsPanelAttached routine to read Scratch Register 2, CR1F, bit
;; 1.
;; 
;; 12    7/22/99 4:43p Kaymann
;; In FixupTVOut, made sure Misc. Output[7:6] are 11b to force Hsync
;; and Vsync to positive.
;; 
;; 11    7/16/99 3:14p Kaymann
;; Modified IsNTSCMode, IsPALMode, and GetPALMode to read the TV
;; configuration from the EEPROM byte 0.
;; 
;; Modified routines which checked if composite TV is active to use
;; the equate FLD_COMPOSITE instead of a hard coded value.
;; 
;; 10    7/02/99 4:49p Kaymann
;; Added BT869 register writes neccessary to adjust the Sub-carrier
;; frequency
;; so that we will be within 50ppm in NTSC and 25ppm in PAL output.
;; Voodoo3 PRS#6772
;; 
;; 9     6/24/99 11:06a Kaymann
;; In FixUpTV, Fixed bug in MISC register programming which caused
;; Mode 7 not to display on the TV.
;; 
;; 8     6/09/99 10:59a Kaymann
;; Modfied the following bank switch routines to reflect the new
;; Read/Write
;; Appeture bit definations in VgaInit0 and VgaInit1.
;; OEMSetBank, OEMSetWBank, OEMGetWBank, OEMSetRBank, OEMGetRBank,
;; OEMIncBank, OEMIncWBank, OEMDecWBank, OEMIncRBank, and OEMDecRBank.
;; 
;; 7     5/28/99 12:20p Kaymann
;; Added the routines CheckForPanel and I2CWriteRegister for checking if a
;; panel is attached.
;; 
;; Added the routines I2CReadBusXBytes and I2CReadByte to read DDC
;; information from the panel.
;; 
;; Added the routine ChecKFPSideScaling to read the DDC information from
;; the panel and determine if scaling is supported in the panel.
;; 
;; 6     5/21/99 2:48p Kaymann
;; Modified GetExtRegsEntry routine to properly load parameters from the
;; EXTREGS table for 200, 350, and 400 scanline text mode.
;; 
;; 5     5/11/99 3:07p Kaymann
;; Moved routines which are only needed during POST to OEMPOST.ASM.
;; 
;; Merge the routines GetSubdeviceID and GetSubvendorID into GetSSID.
;; 
;; 4     5/05/99 7:12p Kaymann
;; In OEMSignOn, modified the sign on message to also print if SDRAM or
;; SGRAM was detected.
;; 
;; Replaced the SDRAM_BIOS compile option with a call to the routine
;; CheckIfSGRAM in the following routines:
;; InitRegisters
;; OEMGetMemSize
;; 
;; Modified OEMSignOn to handle 32Meg and 64Meg sign on message.
;; 
;; Modified OEMGetMemSize to handle return 32Meg and 64Meg of
;; memory installed.
;; 
;; 3     5/05/99 11:34a Kaymann
;; In OEMSignOn, modified the sign on message to also print if SDRAM or
;; SGRAM was detected.
;; 
;; Replaced the SDRAM_BIOS compile option with a call to the routine
;; CheckIfSGRAM in the following routines:
;; InitRegisters
;; OEMGetMemSize
;; 
;; 2     5/04/99 6:01p Kaymann
;; Modified the routines GetSubdeviceID and GetSubvendorID to get the
;; offset to the tblPCIInfo table from read C000:0018 instead of using the
;; tblPCIInfo label.
;; 
;; Modified the Panel routine to check if a 1280x1024 panel is attached to
;; determine which set of panel timings to load.
;; 
;; Created the routine GetPanelSize, to returnsize of panel currently
;; attached.
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
	%out	!	Assembling: OEM.ASM - 3Dfx Interactive
;
.MODEL TINY
.CODE
.386
;

	extrn	fBoardConfig:DWORD

	extrn	GetModeDescPtr:near
IF VESA
	extrn	GetCRTCAddr:near
	extrn	GetIndexRegister:near
ENDIF
IF EXTENDED_BIOS
	extrn	LoadColor256DAC:near
ENDIF
	extrn	SafeInt1A:near
	extrn	GetFontBlockPtr:near
	extrn	GetOEMTablePtr:near
	extrn	IsTextMode:near
	extrn	GetBPP:near
	extrn	WaitVerticalRetrace:near
	extrn	FullCpuOn:near
	extrn	FullCpuOff:near
	extrn	ResetATCFlipFlop:near
	extrn	Delay15us:near
;
	extrn	selA0000:word
	extrn	selB0000:word
	extrn	selB8000:word
	extrn	byOEMSignOnColor:byte
	extrn	szOEMCopyright:byte
	extrn	LENGTH_MEMORYSIZE:abs
	extrn	LENGTH_SDRAM_MEMORY:abs
	extrn	szSDRAM_Memory:byte
	extrn	LENGTH_SGRAM_MEMORY:abs
	extrn	szSGRAM_Memory:byte
	extrn	szMemorySize:byte
	extrn	tblMemSizeStrs:byte
	extrn	regPCIInit0:dword
	extrn	regMiscInit0:dword
	extrn	regMiscInit1:dword
	extrn	regDRAMInit0:dword
	extrn	regDRAMInit1:dword
	extrn	regAGPInit0:dword
	extrn	regPLLCtrl1:dword
;;	extrn	regPLLCtrl2:dword
	extrn	regSGRAMMode:dword
IF      TVOUT_BIOS
	extrn	tblTVDataNTSC:byte
	extrn	tblTVDataPAL:byte
IF ROM_64K
  extrn tblTVPatchPALN:byte
  extrn tblTVPatchPALM:byte
  extrn tblTVPatchPALNc:byte
ENDIF
  extrn regTVMode:byte
ENDIF
	extrn	regTMUGBEINIT:DWORD

IF DVI_BIOS
	extrn		DVI_Patch_Table_00_1024:WORD
	extrn		DVI_Patch_Table_00_1280:WORD

	extrn		ExtMode640x480Table:byte
	extrn		ExtMode800x600Table:byte
	extrn		ExtMode1024x768Table:byte

	extrn		ExtMode320x240Table:byte
	extrn		ExtMode400x300Table:byte
	extrn		ExtMode512x384Table:byte

	extrn		DDCEnableDFP:near
	extrn		DDCWrite:near
	extrn		DDCRead:near
	extrn		DDCStart:near
	extrn		DDCStop:near
	extrn		DDCDisable:near
ENDIF

	extrn		tblPCIInfo:BYTE

;**********************************************************************
;
;	OEMInit - Called after waking up the VGA
;
;	Entry:	AX = Initial mode number
;		DS = Seg0
;	Exit:	AX = Initial mode number
;
	public	OEMInit
OEMInit	proc
	assume	ds:Seg0, es:nothing
	ret
OEMInit	endp

;**********************************************************************
;
;	OEMWakeUpVGA - Wake up the VGA after being put to sleep by "SleepVGA".
;
;	Entry:	None
;	Exit:	None
;
	public	OEMWakeUpVGA
OEMWakeUpVGA	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
	mov	dx,SUBSYS_ENABLE
	mov	al,001h
	out	dx,al
	pop	dx
	pop	ax
	ret
OEMWakeUpVGA	endp

;**********************************************************************
;
;	OEMSleepVGA - Put the VGA to sleep.
;
;	Entry:	None
;	Exit:	None
;
	public	OEMSleepVGA
OEMSleepVGA	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
	mov	dx,SUBSYS_ENABLE
	sub	al,al
	out	dx,al
	pop	dx
	pop	ax
	ret
OEMSleepVGA	endp

;**********************************************************************
;
;	OEMGetParmTable - Return the pointer to parameter table
;
;	Entry:	DS = Seg0
;	Exit:	ES:DI = Pointer to parameter table
;
	public	OEMGetParmTable
OEMGetParmTable	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	si
;
	mov	al,[byMode]
	cmp	al,013h				; Extended mode?
	jbe	short OEMGetParmTable_20	;  N: Go handle normal
;
; Extended modes
;
	call	GetModeDescPtr			; ES:BX = Ptr to MODEDESC entry
	call	GetExtRegsEntry			; ES:SI = Ptr to EXTREGS entry
	mov	al,es:[si].EXTREGS.xrParmIdx
	test	al,al				; Shared parameter table?
	jnz	short OEMGetParmTable_30	;  Y: Use this number
	mov	al,[byMode]			;  N: Use standard method
OEMGetParmTable_20:
	call	OEMGetInternalMode
OEMGetParmTable_30:
	les	di,[lpSavePtr]
	les	di,es:[di]
	cbw
	shl	ax,6
	add	di,ax
;
	pop	si
	pop	bx
	ret
OEMGetParmTable	endp

;**********************************************************************
;
;	OEMGetInternalMode - Get the internal mode number for a given VGA mode
;
;	Entry:	AL = VGA mode number
;		DS = Seg0
;	Exit:	AL = Internal mode number
;
	public	OEMGetInternalMode
OEMGetInternalMode	proc
	assume	ds:Seg0, es:nothing
	push	bx
	push	es
;
; Check for multiple mode tables per mode number
;
	cmp	al,07h				; Is mode greater than 7?
	ja	OGIM_20				;  Y: Single mode table
	je	OGIM_10				;  =?: Could be 7 or 7+
	cmp	al,03h				; Is it mode 4, 5, or 6?
	ja	OGIM_exit			;  Y: Got internal mode number
;
; Multiple mode tables exist for these modes. At this point:
;	AL = 0-3, 7
;
; The following modes map to the given internal mode:
;
;	VGA Mode	Internal Mode	Number of
;	Number		Number		Scan Lines
;	0		00h		200
;	0*		13h		350
;	0+		17h		400
;	1		01h		200
;	1*		14h		350
;	1+		17h		400
;	2		02h		200
;	2*		15h		350
;	2+		18h		400
;	3		03h		200
;	3*		16h		350
;	3+		18h		400
;	7		07h		350
;	7+		19h		400
;
	test	[fVGAInfo],SCAN_200_ACTIVE	; 200 scan lines?
	jnz	short OGIM_exit			;  Y: Got internal mode already
	mov	bl,al
	add	al,13h				; Assume 350 lines
	test	[fVGAInfo],SCAN_400_ACTIVE	; 400 scan lines?
	jz	short OGIM_exit			;  N: Got internal mode number
	mov	al,bl				;  Y: Calculate starting position
	shr	al,1
	add	al,17h
	jmp	short OGIM_exit
OGIM_10:
	test	[fVGAInfo],SCAN_400_ACTIVE	; 400 scan lines?
	jz	short OGIM_exit			;  N: Got internal mode number
	mov	al,19h				;  Y: Use this mode table
	jmp	short OGIM_exit
;
; Scan the mode description table for the internal mode number
;
OGIM_20:
	call	GetModeDescPtr
	jc	short OGIM_error		; Mode found? N: Return error
	mov	al,es:[bx].MODEDESC.mdIMode	; Get internal mode
OGIM_exit:
	pop	es
	pop	bx
	ret
;
; Mode wasn't found, so default to 200 scan line mode 0
;
OGIM_error:
	sub	al,al
	jmp	short OGIM_exit
OEMGetInternalMode	endp

;**********************************************************************
;
;	OEMPreSetRegs - Make special preparations to load registers
;
;	Entry:	ES:DI = Pointer to standard parameter table entry
;		DS = Seg0
;	Exit:	None
;
	public	OEMPreSetRegs
OEMPreSetRegs	proc
	assume	ds:Seg0, es:nothing
	push	eax
	push	dx
;
	call	OEMEnableExtensions		; Enable access to extended regs
	call	GetIOBase
	add	dx,REG_VGAINIT0
	in	eax,dx
	and	eax,00000100h			; Set 6 bit RAMDAC, preserve wake up select
	or	al,040h				; Enable extensions
	out	dx,eax
;
	sub	dx,dx				; Set Bank 0
	call	OEMSetBank
	mov	dx,03C2h
	mov	al,067h
	out	dx,al
;
	pop	dx
	pop	eax
	ret
OEMPreSetRegs	endp

;**********************************************************************
;
;	OEMSetRegs - Set extended registers (standard VGA has been set)
;
;	Entry:	ES:DI = Standard VGA parameter pointer
;		DS = Seg0
;	Exit:	None
;
	public	OEMSetRegs
OEMSetRegs	proc
	assume	ds:Seg0, es:nothing
	pushad
	push	es
;
; Palette snooping is broken. Disable it even if the system BIOS
; sets it. It must be disabled here, since the system BIOS may
; enable it after the POST routine is run.
;
	mov	ax,0B102h		; Find PCI device
	sub	si,si
	mov	cx,PCI_ID_DEVICE
	mov	dx,PCI_ID
	int	1Ah			; No error checking is needed by now
	mov	ax,0B109h		; Read config WORD
	mov	di,004h			; Command register
	int	1Ah
	and	cl,0DFh			; Disable palette snooping
	mov	ax,0B10Ch		; Write config WORD
	int	1Ah
;
	mov	al,[byMode]
	call	GetModeDescPtr			; Get pointer to mode description
	jc	OEMSetRegs_exit			; Error? Y: Exit without setting anything
;
	mov	ax,es:[bx].MODEDESC.mdRowOff
	push	ax				; Save Stride
;
	movzx	eax,es:[bx].MODEDESC.mdYRes
	shl	eax,12
	push	eax				; Maybe needed to double Y size
	mov	cx,es:[bx].MODEDESC.mdXRes
	and	cx,0FFFh
	or	ax,cx
	push	eax				; Save resolution
;
	call	GetExtRegsEntry			; ES:SI = Ptr to EXTREGS entry
;
; Set CRTC overflow
;
	mov	dx,[wCRTCAddr]
	mov	ax,CRTC_HEXT_OVER		; AX = 001Ah
	out	dx,ax				; Clear H extended overflow
	inc	ax				; AL = 1Bh
	mov	ah,es:[si].EXTREGS.xrOverflow
	out	dx,ax
;
; Clear linear mode flag
;
	mov	al,SCRATCH_REG1
	call	GetIndexRegister
	and	ah,not FL_LINEARMODE
	out	dx,ax
;
	call	GetIOBase
	mov	di,dx				; DI = I/O Base
;
; Set REG_VIDCLOCK
;
	add	dx,REG_VIDCLOCK
	sub	eax,eax
	mov	ax,es:[si].EXTREGS.xrClock
	test	ax,ax				; Valid value?
	jz	short OEMSetRegs_10		;  N: Don't load
	out	dx,eax
OEMSetRegs_10:
;
; ECX is used as the "Flags" value for the next page & a half of code
;
	movzx	ecx,es:[si].EXTREGS.xrFlags	; ECX is xrFlags
;
; Set REG_DACMODE
;
	lea	dx,[di + REG_DACMODE]
	sub	eax,eax
	out	dx,eax
;
; Set REG_VIDDESKTOPSTART
;
	lea	dx,[di + REG_VIDDESKTOPSTART]
	sub	eax,eax
	out	dx,eax
;
; Set REG_VIDSCREENSIZE
;
	pop	eax				; Get saved resolution
	pop	edx				; Get Y size for [maybe] doubling
	test	cl,04h				; Use vertical x2?
	jz	short OEMSetRegs_20		;  N: Use resolution as is
	add	eax,edx				;  Y: Double the Y
OEMSetRegs_20:
	lea	dx,[di + REG_VIDSCREENSIZE]
	out	dx,eax
;
	sub	eax,eax				; Zero out high EAX
	pop	ax				; Get saved stride
	lea	dx,[di + REG_VIDDESKTOPSTRIDE]
	out	dx,eax
;
; Set REG_VGAINIT0, VGAINIT1, and REG_VIDPROCCFG
; Note that bit 0 (the VGA vs. Native mode bit) of the "xrFlags" entry
; in the EXTREGS table controls four other bits.
;
; First, VGAINIT0
;
	mov	bl,cl
	lea	dx,[di + REG_VGAINIT0]
	in	eax,dx
	and	eax,000000100h			; Maintain adapter/MB style
	and	bl,001h				; Mask off non-VGAInit0 bits
	shl	bx,12				; Use extended shift out
	or	bx,00040h			; Enable extensions
	or	ax,bx				; Combine all the bits...
	out	dx,eax				;  ...and write 'em.
;
; Secondly, VGAINIT1
;
	lea	dx,[di + REG_VGAINIT1]
	mov	eax,ecx
	and	eax,001h
	shl	eax,20
	out	dx,eax
;
; Finally, VIDPROCCFG
;
	lea	dx,[di + REG_VIDPROCCFG]
	in		eax,dx				;Read current VidProcCfg value
	and	eax,030000000h		;Preserve bit 28:28, memory optimization bits
	mov	ebx,eax				;Save in EBX

	mov	eax,ecx
	and	al,038h				; Isolate BPP flags...
	shl	eax,15				;  ...position them
	test	cl,001h				; VGA mode?
	jz	short OEMSetRegs_100		;  Y: Got value already ("0")
	or	al,081h				;  N: Enable video processor/fetch desktop
	test	cl,038h				; Palettized mode?
	jz	short OEMSetRegs_90		;  Y: Don't disable palette
	or	ah,00Ch				;  N: Disable CLUT
OEMSetRegs_90:
	and	cl,006h				; Isolate interlace and /2 flags...
	shl	cl,2				;  ...and  position them
	or	al,cl				;  ...and set them
OEMSetRegs_100:

	or		eax,ebx				;OR in memory optimization bits

	out	dx,eax
;
; Set REG_DRAMINIT1 - Turn on extended refresh.  This may cause a slight
; performance hit in VGA modes (Sonic the Hedgehog), but cures a problem
; with losing memory contents when in a PM state.
;
	lea	dx,[di + REG_DRAMINIT1]
	in	eax,dx
	or	al,1
	out	dx,eax


;***************************************************************************
;Disable the TV interface when TV or DVI is disabled.  Will be reenabled
;  when enabling TV or DVI.
	lea	dx,[di + REG_VIDINFORMAT]
	in		eax,dx
	and	eax,not 000008000h
	out	dx,eax
;***************************************************************************


;
IF 	DVI_BIOS
        call    IsPanelAttached
        jz      Panel_Not_Attached

        call    FixupPanel              ;Program panel
        jmp     OEMSetRegs_exit         ;All Done.  No need to talk to the TV.

Panel_Not_Attached:
ENDIF

IF TVOUT_BIOS
;Turn on CRT

	lea	dx,[di + REG_MISCINIT1]		;Read current state of MiscInit1
	in		eax,dx							;We need to either set or clear bit 8
	and	ah,0FEh							;Make sure the CRT DAC is on	(Power up DAC)
	out	dx,eax

  call  IsTVActive
	jz	   short OEMSetRegs_exit
OEMSetRegs_110:
	call	FixupTVOut

;********************************************************************************
;Drive only TV and disable CRT

;First, turn off the CRT active bit
	call	GetCRTCAddr
	mov	al,SCRATCH_REG2
	call	GetIndexRegister
	and 	ah,not 80h  					;Disable CRT
	out	dx,ax

	lea	dx,[di + REG_MISCINIT1]		;Read current state of MiscInit1
	in		eax,dx							;We need to either set or clear bit 8
	or		ah,001h							;Make sure the CRT DAC is off (Power down DAC)
	out	dx,eax
;********************************************************************************

ENDIF
;

OEMSetRegs_exit:
	pop	es
	popad
	ret
OEMSetRegs	endp

;**********************************************************************
;
;	OEMClearScreen - Clear out the regen buffer
;
;	Entry:	DS = Seg0
;	Exit:	None
;
;	All registers are preserved.
;
	public	OEMClearScreen
OEMClearScreen	proc
	assume	ds:Seg0, es:nothing
	pusha
	push	eax
	push	es
;
	mov	al,[byMode]
	call	GetModeDescPtr
	jc	short OEMClearScreen_exit	; Mode found? N: Exit without clearing
	sub	di,di				; All modes start at offset 0
	mov	ax,00720h			; Assume text mode
	mov	cx,04000h			; 16K words
	mov	dx,cs:[selB8000]		; Color text is at B8000h
	mov	bl,es:[bx].MODEDESC.mdType	; Get mode type
	cmp	bl,MDTYPE_CTEXT			; Is it a color text mode?
	je	short OEMClearScreen_10		;  Y: Got data, so go clear
	mov	dx,cs:[selB0000]		; Assume mono text at B0000h
	cmp	bl,MDTYPE_MTEXT			; Is it a mono text mode?
	je	OEMClearScreen_10		;  Y: Got data, so go clear
	cmp	bl,MDTYPE_CGA			; Is it a CGA graphics mode?
	jne	OEMClearScreen_20		;  N: Go clear all of memory
	mov	dx,cs:[selB8000]		;  Y: CGA is at B8000h
	sub	ax,ax				;     CGA is cleared with 0
;
; Do limited clear (text, CGA modes)
;
OEMClearScreen_10:
	mov	es,dx
	rep stosw
	jmp	short OEMClearScreen_exit
;
; At this point:
;	BL = Mode type
;	DI = 0
;	DS = Seg0
;
OEMClearScreen_20:
	mov	ax,1				; Assume one 64K block
	cmp	[byMode],13h			; Is this a standard VGA mode?
	jbe	short OEMClearScreen_30		;  Y: Only clear the first 256K
	call	OEMGetMemSize			; AX = # of 256K blocks
	cmp	bl,MDTYPE_4BPP			; Is this a planar mode
	je	short OEMClearScreen_30		;  Y: Planar modes talk to 4 bytes at once
	shl	ax,2				;  N: Hit each byte individually
OEMClearScreen_30:
	mov	si,ax				; SI = # of 64K blocks
	mov	ax,cs:[selA0000]
	mov	es,ax				; All other modes are at A0000h
	mov	ax,di
	mov	dx,di
OEMClearScreen_40:
	call	OEMSetBank
	mov	cx,8000h
	rep stosw
	inc	dx				; Next bank
	dec	si				; More banks?
	jnz	OEMClearScreen_40		;  Y: Go do next bank
;
	sub	dx,dx
	call	OEMSetBank
;
OEMClearScreen_exit:
	pop	es
	pop	eax
	popa
	ret
OEMClearScreen	endp

;**********************************************************************
;
;	OEMFixupFont - Fixup the current font
;
;	Entry:	BL = Block to load
;		BH = Character height
;	Exit:	None
;
;	Assume "PreFontLoad" has been called.
;
;	All registers are preserved.
;
	public	OEMFixupFont
OEMFixupFont	proc
	assume	ds:nothing, es:nothing
	pusha
	push	ds
	push	es
	sub	dx,dx
	mov	ds,dx
	assume	ds:Seg0
	mov	cl,byte ptr [wCharHeight]
	sub	cl,bh				; CL = Number of scans to clear
	jbe	OEMFixupFont_exit		;  None? Go exit
	call	GetFontBlockPtr			; ES:DI = Beginning of font memory
	sub	ch,ch				; CX = Number of bytes to clear
	mov	dx,GDC_INDEX
	mov	ax,00204h			; Set read map select to font plane
	out	dx,ax
	mov	dx,32
	sub	dx,cx				; DX = Offset to next character
	mov	al,bh
	cbw
	add	di,ax				; Point to beginning of first character to clear
	mov	si,256				; SI = Number of characters to clear
	mov	bl,cl				; BL = Reload value: number of bytes to clear
OEMFixupFont_10:
	mov	al,es:[di-1]			; Fill rest of font with last byte of font glyph
	mov	cl,bl				; CX = Number of bytes to clear
	rep stosb
	add	di,dx				; Next character to clear
	dec	si
	jnz	OEMFixupFont_10
	mov	dx,GDC_INDEX
	mov	ax,00004h			; Set read map select back to plane 0
	out	dx,ax
OEMFixupFont_exit:
	pop	es
	pop	ds
	popa
	ret
OEMFixupFont	endp

;**********************************************************************
;
IF (VARIATION_005 eq 1)
;
;	OEMWriteRAMDACIdx - Write a RAMDAC index register
;
;	Entry:	BL = Index of DAC Register
;	Exit:	None
;
	public	OEMWriteRAMDACIdx
OEMWriteRAMDACIdx	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
;
	mov	dx,DAC_WINDEX
	mov	al,bl
	out	dx,al
;
	pop	dx
	pop	ax
	ret
OEMWriteRAMDACIdx	endp

;**********************************************************************
;
;	OEMWriteRAMDACData - Write a RAMDAC data register
;
;	Entry:	DH = Red value
;		CH = Green value
;		CL = Blue value
;	Exit:	None
;
	public	OEMWriteRAMDACData
OEMWriteRAMDACData	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
;
	mov	al,dh
	mov	dx,DAC_DATA
	cli
	out	dx,al				; Red
	mov	al,ch
	out	dx,al				; Green
	mov	al,cl
	out	dx,al				; Blue
	sti
;
	pop	dx
	pop	ax
	ret
OEMWriteRAMDACData	endp

;**********************************************************************
;
;	OEMReadRAMDAC - Read a RAMDAC register
;
;	Entry:	BL = Index of DAC register
;	Exit:	CL = Blue value
;		CH = Green value
;		DH = Red value
;
	public	OEMReadRAMDAC
OEMReadRAMDAC	proc
	assume	ds:nothing, es:nothing
	push	dx
	mov	dx,DAC_RINDEX
	mov	al,bl
	out	dx,al
	inc	dx
	inc	dx
	cli
	in	al,dx
	mov	ah,al
	in	al,dx
	mov	ch,al
	in	al,dx
	mov	cl,al
	sti
	pop	dx
	mov	dh,ah
	ret
OEMReadRAMDAC	endp
;
ENDIF
;
IF VESA
;
;**********************************************************************
;	OEMFixupVBEModeInfo - Fixup the MODEINFOBLOCK information
;
;	Entry:	BL = Mode number
;		BH = Imode number
;		CX = VBE mode number
;		DS:SI = Pointer to MODEDESC data
;		ES:DI = Pointer to MODEINFOBLOCK buffer
;	Exit:	None
;
	public	OEMFixupVBEModeInfo
OEMFixupVBEModeInfo	proc
	assume	ds:nothing, es:nothing
	ret
OEMFixupVBEModeInfo	endp

;**********************************************************************
;
;	OEMGetMemSize - Return the pre-calculated memory size
;
;	Entry:	None
;	Exit:	AX = Number of 256K blocks
;
	public	tblMemSize

IF	Napalm2_BIOS
;					0	  1	 2    3		4		5		6		7		8		9		 A		  B
tblMemSize	dw	4*4, 8*4, 8*4, 16*4, 16*4, 32*4, 32*4, 64*4, 64*4, 128*4, 128*4, 256*4
;
ELSE
;					0	  1	 2    3		4		5		6		7		8	  
tblMemSize	dw	4*4, 8*4, 8*4, 16*4, 16*4, 32*4, 32*4, 64*4, 64*4,
;
ENDIF

	public	OEMGetMemSize
OEMGetMemSize	proc
	assume	ds:nothing, es:nothing

	push	bx
	push	dx
	call	GetIOBase
	add	dx,REG_DRAMINIT0
	in	eax,dx
	shr	eax,26
	and	ax,00Fh			; Make AH = 0
	mov	bx,ax
	shl	bx,1
	mov	ax,cs:[tblMemSize + bx]

IF	Napalm2_BIOS
;New scheme to get memory size for Napalm2
	shr	ax,1				;Divide memroy size by 2

	mov	bx,ax				; Save memory size in BX

;Read TMUGbeInit[27:25] and check if 32MBit DDR.
;TMUGbeInit[27:25] = 3,4, or 5 == 32MBit DDR.

	call	GetIOBase
	add	dx,REG_TMUGBEINIT	
	in		eax,dx
	shr	eax,25		
	and	al,07h

;Check for 32Mbit DDR memory.  If found, div 2.
	cmp	al,03h
	jl		EndNapalm2MemorySizing
	cmp	al,05h
	jg		EndNapalm2MemorySizing


	shr	bx,1			;Divide by 2.  

EndNapalm2MemorySizing:

	mov	ax,bx				; Save memory size in AX

endif


	pop	dx
	pop	bx

	ret
OEMGetMemSize	endp

;**********************************************************************
;
;	OEMGetLinearMemBase - Return the address of the linear framebuffer
;
;	Entry:	DS:SI = Pointer to MODEDESC data
;	Exit:	EAX = Address of linear framebuffer
;
	public	OEMGetLinearMemBase
OEMGetLinearMemBase	proc
	assume	ds:nothing, es:nothing
	sub	eax,eax				; Assume no linear address
	cmp	[si].MODEDESC.mdBPP,8		; Mode less than 8 BPP?
	jb	short OEMGetLinearMemBase_exit	;  Y: No linear addressing
	call	GetMemBase			;  N: Get the linear address
OEMGetLinearMemBase_exit:
	ret
OEMGetLinearMemBase	endp

;**********************************************************************
;
;	OEMSetLinearMemBase - Set the current mode to use the linear frame buffer
;
;	Entry:	DS = Seg0
;	Exit:	None
;
	public	OEMSetLinearMemBase
OEMSetLinearMemBase	proc
	assume	ds:Seg0, es:nothing
	push	ax
	push	dx
;
	call	GetCRTCAddr
	mov	al,SCRATCH_REG1
	call	GetIndexRegister
	or	ah,FL_LINEARMODE
	out	dx,ax
;
	pop	dx
	pop	ax
	ret
OEMSetLinearMemBase	endp

;**********************************************************************
;
;	OEMIsInLinearMode - Determine if current mode is using linear addressing
;
;	Entry:	DS = Seg0
;		BX = Mode number (maybe 7-bit mode number or 15-bit mode number)
;	Exit:	Carry flag set as follows:
;			NC = Not in linear mode
;			CF = Current mode uses linear framebuffer
;
;	All registers preserved.
;
;	Note:	This call is made from "VBE_GetMode" and could be
;		used to "fixup" the mode number if necessary.
;
	public	OEMIsInLinearMode
OEMIsInLinearMode	proc
	push	ax
	push	dx
	cmp	[byMode],013h
	jbe	short OEMIsInLinearMode_not
;
; Determine if mode uses linear framebuffer
;
	call	GetCRTCAddr
	mov	al,SCRATCH_REG1
	call	GetIndexRegister
	test	ah,FL_LINEARMODE		; Last mode set as linear?
	jz	short OEMIsInLinearMode_not	;  N: Flag non-linear
	stc					;  Y: Flag it as non-linear
	jmp	short OEMIsinLinearMode_exit
;
OEMIsInLinearMode_not:
	clc
OEMIsInLinearMode_exit:
	pop	dx
	pop	ax
	ret
OEMIsInLinearMode	endp

;**********************************************************************
;
;	OEMEnableExtensions - Enable access to extended registers
;
;	Entry:	None
;	Exit:	None
;
;	All registers preserved.
;
	public	OEMEnableExtensions
OEMEnableExtensions	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
;
	call	GetIOBase
	add	dx,REG_VGAINIT0
	in	eax,dx
	or	al,040h
	out	dx,eax
;
	pop	dx
	pop	ax
	ret
OEMEnableExtensions	endp

;**********************************************************************
;
;	OEMIsModeAvailable - Determine if mode is available with current hardware
;
;	Entry:	AL = OEM mode number
;	Exit:	Carry flag set as follows:
;			NC = Mode is available
;			CF = Mode is not available
;
;	All registers are preserved.
;
	public	OEMIsModeAvailable
OEMIsModeAvailable	proc
	assume	ds:nothing, es:nothing
	pusha
	push	es
;
	call	GetModeDescPtr
	jc	short OEMIsModeAvailable_exit
;
; Memory size required = Row Offset * YRes
;
	mov	ax,es:[bx].MODEDESC.mdRowOff
	mul	es:[bx].MODEDESC.mdYRes		; DX:AX = Memory needed in bytes
	mov	cx,(256/8)*1024
	div	cx				; AX = Memory needed in 256K chunks
	add	dx,0FFFFh			; If DX != 0, CF = 1
	adc	ax,0
	xchg	ax,dx				; One byte "mov dx,ax"
	call	OEMGetMemSize			; AX = Memory available in 256K chunks
	shl	ax,3				; AX*3 to get in same range as DX
	cmp	ax,dx				; If not enough memory, CF = 1
;
OEMIsModeAvailable_exit:
	pop	es
	popa
	ret
OEMIsModeAvailable	endp

;**********************************************************************
;
;	OEMSaveExtState - Save extended registers
;
;	Entry:	ES:DI = Pointer to save buffer
;		DS = Seg0
;	Exit:	None
;
	public	OEMSaveExtState
OEMSaveExtState	proc
	assume	ds:Seg0, es:nothing
	ret
OEMSaveExtState	endp

;**********************************************************************
;
;	OEMRestoreExtState - Restore extended registers
;
;	Entry:	ES:SI = Pointer to restore buffer
;		DS = Seg0
;	Exit:	None
;
	public	OEMRestoreExtState
OEMRestoreExtState	proc
	assume	ds:Seg0, es:nothing
	ret
OEMRestoreExtState	endp

;**********************************************************************
;
;	OEMPreSaveRestore - Set up any registers needed for access
;
;	Entry:	DS = Seg0
;		CX = Requested states
;		ES:BX = Pointer to buffer
;	Exit:	None
;
	public	OEMPreSaveRestore
OEMPreSaveRestore	proc
	assume	ds:Seg0, es:nothing
	ret
OEMPreSaveRestore	endp

;**********************************************************************
;
;	OEMPostSaveRestore - Reset any register set by pre call
;
;	Entry:	DS = Seg0
;		CX = Requested states
;		ES:BX = Pointer to buffer
;	Exit:	None
;
	public	OEMPostSaveRestore
OEMPostSaveRestore	proc
	assume	ds:Seg0, es:nothing
	ret
OEMPostSaveRestore endp

;**********************************************************************
;
;	OEMGetCurrentXChar - Return the width (in pixels) of a character
;
;	Entry:	DS = Seg0
;	Exit:	AL = Character width (in pixels)
;
	public	OEMGetCurrentXChar
OEMGetCurrentXChar	proc
	assume	ds:Seg0, es:nothing
	mov	al,8		; All extended modes have 8-pixel wide chars
	ret
OEMGetCurrentXChar	endp

;**********************************************************************
;
;	OEMSetRowOffset - Set the row offset in the CRTC
;
;	Entry:	AX = Row offset
;		DS = Seg0
;	Exit:	None
;
	public	OEMSetRowOffset
OEMSetRowOffset	proc
	assume	ds:Seg0, es:nothing
	push	eax
	push	bx
	push	dx
;
	call	IsVGAMode			; Is this in native mode?
	jnz	short OEMSetRowOffset_10	;  Y: Go handle extended mode
;
; VGA mode
;
	call	GetCRTCAddr
	mov	bx,ax
	mov	ah,al
	mov	al,013h
	out	dx,ax				; Write low order offset
	jmp	short OEMSetRowOffset_exit
;
; Native mode
;
OEMSetRowOffset_10:
	shl	ax,3				; Shift to WORD size
	mov	bx,ax
	call	GetIOBase			; DX = IO base
	add	dx,REG_VIDDESKTOPSTRIDE
	in	eax,dx				; Get current setting...
	and	eax,0FFFF8000h			;  ...save high-order bits...
	or	ax,bx				;  ...add in row offset...
	out	dx,eax				;  ...and write it back out
;
OEMSetRowOffset_exit:
	pop	dx
	pop	bx
	pop	eax
	ret
OEMSetRowOffset	endp

;**********************************************************************
;
;	OEMGetRowOffset - Get the row offset from the CRTC
;
;	Entry:	DS = Seg0
;	Exit:	AX = Row offset
;
	public	OEMGetRowOffset
OEMGetRowOffset	proc
	assume	ds:Seg0, es:nothing
	push	dx
;
	call	IsVGAMode			; Is this in native mode?
	jnz	short OEMGetRowOffset_10	;  Y: Go handle extended mode
;
; VGA mode
;
	call	GetCRTCAddr
	mov	al,013h
	call	GetIndexRegister
	mov	al,ah
	sub	ah,ah
	jmp	short OEMGetRowOffset_exit
;
; Native mode
;
OEMGetRowOffset_10:
	push	eax
	call	GetIOBase
	add	dx,REG_VIDDESKTOPSTRIDE
	in	eax,dx
	and	eax,07FFFh
	shr	ax,3
	xchg	ax,dx				; Save AX into DX
	pop	eax
	xchg	ax,dx				; Recover DX back to AX
;
OEMGetRowOffset_exit:
	pop	dx
	ret
OEMGetRowOffset	endp

;**********************************************************************
;
;	OEMAdjustMaxRowOffset - Adjust the maximum row offset value
;
;	Entry:	AX = Calculated size
;		DS = Seg0
;	Exit:	AX = Adjust size (if necessary)
;
	public	OEMAdjustMaxRowOffset
OEMAdjustMaxRowOffset	proc
	assume	ds:Seg0, es:nothing
	ret
OEMAdjustMaxRowOffset	endp

;**********************************************************************
;
;	OEMSetDisplayStart - Set the CRTC display start address
;
;	Entry:	DX:AX = Display start
;		DS = Seg0
;	Exit:	None
;
;	Assume extensions are enabled
;	All registers are preserved
;
	public	OEMSetDisplayStart
OEMSetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	push	bx
;
	call	IsVGAMode			; Is this in native mode?
	jnz	short OEMSetDisplayStart_10	;  Y: Go handle extended mode
;
; VGA mode
;
	mov	bh,dl
	mov	bl,al
	call	GetCRTCAddr
	mov	al,0Ch
	out	dx,ax				; Display start high
	inc	ax				; AL = 0Dh
	mov	ah,bl
	out	dx,ax				; Display start low
	jmp	short OEMSetDisplayStart_exit
;
; Native mode
;
OEMSetDisplayStart_10:
	push	eax
	push	dx
	shl	eax,16
	mov	ax,dx
	ror	eax,16				; EAX = Display start
	shl	eax,2
	call	GetIOBase
	add	dx,REG_VIDDESKTOPSTART
	out	dx,eax
	pop	dx
	pop	eax
;
OEMSetDisplayStart_exit:
	pop	bx
	ret
OEMSetDisplayStart	endp

;**********************************************************************
;
;	OEMGetDisplayStart - Get the CRTC display start address
;
;	Entry:	DS = Seg0 segment
;	Exit:	DX:AX = Linear start address
;
;	Assume extensions are enabled.
;	All registers are preserved.
;
	public	OEMGetDisplayStart
OEMGetDisplayStart	proc
	assume	ds:Seg0, es:nothing
	push	bx
;
	call	IsVGAMode			; Is this in native mode?
	jnz	short OEMGetDisplayStart_10	;  Y: Go handle extended mode
;
; VGA mode
;
	call	GetCRTCAddr
	mov	al,00Ch
	call	GetIndexRegister
	mov	bh,ah				; BH = Start address high
	inc	ax				; AX = 0Dh
	call	GetIndexRegister
	mov	bl,ah				; BL = Start address low
	sub	dx,dx
	mov	ax,bx				; DX:AX = Start address
	jmp	short OEMGetDisplayStart_exit
;
; Native mode
;
OEMGetDisplayStart_10:
	push	eax
	call	GetIOBase
	add	dx,REG_VIDDESKTOPSTART
	in	eax,dx				; EAX = Start address
  shr   eax,2
  mov   bx,ax
  shr   eax,16
  mov   dx,ax
	pop	eax
	mov	ax,bx
;
OEMGetDisplayStart_exit:
	pop	bx
	ret
OEMGetDisplayStart	endp

;**********************************************************************
;
;	OEMSetDACFormat - Set 6-bits or 8-bits per gun
;
;	Entry:	AL = DAC format
;	Exit:	Carry flag set as follows:
;			NC = Valid value (even if H/W can't set it)
;			CF = Invalid value
;
	public	OEMSetDACFormat
OEMSetDACFormat	proc
	cmp	al,6				; Less than 6-bit per gun?
	jb	short OEMSetDACFormat_error	;  Y: Bad value
;
; If hardware supports it, a value of 6 & 7 sets 6-bits per gun, and a
; value of 8 and up sets 8-bits per gun.
;
	push	eax
	push	dx
	call	GetIOBase
	add	dx,REG_VGAINIT0
	cmp	al,8				; Set 6 bits per gun?
	in	eax,dx				; ** DOESN'T AFFECT FLAGS **
	jb	short OEMSetDACFormat_10	;  Y: Go set 6 bits
	or	al,004h				; Set 8 bits per gun
	jmp	short OEMSetDACFormat_20
OEMSetDACFormat_10:
	and	al,0FBh				; Set 6 bits per gun
OEMSetDACFormat_20:
	out	dx,eax
;
	pop	dx
	pop	eax
	clc					; Flag success
	ret
;
OEMSetDACFormat_error:
	stc
	ret
OEMSetDACFormat	endp

;**********************************************************************
;
;	OEMGetDACFormat - Return 6-bits or 8-bits per gun
;
;	Entry:	None
;	Exit:	AL = DAC format (6-bits or 8-bits per gun)
;
	public	OEMGetDACFormat
OEMGetDACFormat	proc
	push	bx
	push	dx
;
	call	GetIOBase
	add	dx,REG_VGAINIT0
	push	eax
	in	eax,dx
	mov	bl,006h				; Assume 6 bits per gun
	test	al,004h				; Is it 6 bits?
	jz	OEMGetDACFormat_10		;  Y: Got it
	mov	bl,008h				;  N: Flag it as 8
OEMGetDACFormat_10:
	pop	eax
	mov	al,bl
;
	pop	dx
	pop	bx
	ret
OEMGetDACFormat	endp

;**********************************************************************
;
  IF VESA_300
;
;	OEMGetMaxDotClock - Return the maximum dot clock
;
;	Entry:	BL = Mode number
;		BH = Imode number
;		DS:SI = Pointer to MODEDESC data
;	Exit:	EAX = Maximum dot clock
;
	public	OEMGetMaxDotClock
OEMGetMaxDotClock	proc
	assume	ds:nothing, es:nothing
	mov	al,bl
	call	IsTextMode
	mov	eax,40000000			; 40 MHz
	jz	OEMGetMaxDotClock_exit
	mov	eax,160000000			; 100 MHz
OEMGetMaxDotClock_exit:
	ret
OEMGetMaxDotClock	endp

;**********************************************************************
;
;	OEMGetDotClock - Get closest dot clock
;
;	Entry:	ECX = Requested dot clock in HZ
;		DX = Mode number
;	Exit:	ECX = Closest dot clock
;
	public	OEMGetDotClock
OEMGetDotClock	proc
	assume	ds:nothing, es:nothing
	push	eax
	push	ebx
	push	esi
	push	edi
	push	ebp
;
	mov	eax,ecx
	call	CalcClockWord
	call	WhatIsClockWord
	mov	ecx,eax
;
	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	pop	eax
	ret
OEMGetDotClock	endp

;**********************************************************************
;
;	OEMFixupCRTC - Fixup the current mode with a user CRTC table
;
;	Entry:	DS = Seg0
;		ES:DI = Pointer to CRTCINFOBLOCK data structure
;	Exit:	None
;
	public	OEMFixupCRTC
OEMFixupCRTC	proc
	assume	ds:Seg0, es:nothing
	pushad
;
	mov	dx,[wCRTCAddr]
	mov	ax,02011h
	out	dx,ax			; Unlock CRTC
;
	mov	al,[byMode]
	call	GetBPP
	cmp	al,8			; Current color depth < 8 BPP?
	jb	OEMFixupCRTC_exit	;  N: Not supported at lower BPP
;
; Set dot clock
;
	mov	eax,es:[di].CRTCINFOBLOCK.PixelClock
	call	SetAbsoluteDotClock
;
; Set horizontal total
;
	mov	ax,es:[di].CRTCINFOBLOCK.HorizontalTotal
	shr	ax,3			; Convert pixels to characters
	sub	ax,5			; VGA HTotal = HTotal - 5
	mov	ch,ah
	and	ch,001h			; Accumulate extended overflow in CH
	shl	ax,8			; AH = HTotal, AL = 00h
	out	dx,ax
;
; Set horizontal blank end
;
	mov	bh,ah
	add	bh,3			; BH = HBlank end = HTotal - 2
	mov	al,003h
	call	GetIndexRegister
	push	bx			; Need to save HBE.5 for later
	and	bh,01Fh			; Use only bits 0..4
	and	ah,0E0h			; Preserve display enable skew
	or	ah,bh
	out	dx,ax
;
; Set horizontal sync start
;
	mov	ax,es:[di].CRTCINFOBLOCK.HorizontalSyncStart

	call	CheckIf1024Mode
	jz		SkipPatchOfHorizontalSyncStart

	dec	ax

SkipPatchOfHorizontalSyncStart:

	shr	ax,3			; Convert pixels to characters
	bt	ax,8
	setc	cl
	shl	cl,6
	or	ch,cl			; CH = Bit 6,0 has HSS[8],HTot[8]
	mov	ah,al
	mov	al,004h
	out	dx,ax
;
; Set horizontal sync end
;
	mov	bx,es:[di].CRTCINFOBLOCK.HorizontalSyncEnd

	call	CheckIf1024Mode
	jz		SkipPatchOfHorizontalSyncEnd

	dec	bx

SkipPatchOfHorizontalSyncEnd:

	shr	bx,3			; Convert to characters
	mov	al,005h
	call	GetIndexRegister
	and	ah,060h			; Preserve original sync skew
	and	bl,01Fh			; Use only HSE bits 0..4
	or	ah,bl
	pop	bx			; Retrieve HBE.5...
	and	bh,020h			;  ...and isolate it...
	shl	bh,2			;  ...and move it into position.
	or	ah,bh			; AH = HSE, HBE.5 combined into VGA format
	out	dx,ax
;
; Set horizontal extended overflow (for most modes this will be 00h)
;
	mov	al,CRTC_HEXT_OVER
	call	GetIndexRegister
	and	ah,03Ch			; Lop off HSS and HTotal bits
	or	ah,ch			; Add extended overflow bits
	out	dx,ax
;
; Set vertical total
;
	mov	ax,es:[di].CRTCINFOBLOCK.VerticalTotal
	dec	ax
	dec	ax			; VTotal = VTotal - 2
	mov	ch,ah
	and	ch,001h			; Accumulate overflow in CH
	bt	ax,9
	setc	cl
	shl	cl,5
	or	ch,cl			; CH = Bit 5,0 has VTot 9,8
	mov	bh,ah
	and	bh,004h			; Accumulate extended overflow in BH
	shr	bh,2			; BH = Bit 0 has VTot[10]
	mov	ah,al
	mov	al,006h
	out	dx,ax
	mov	al,016h
	sub	ah,2
	out	dx,ax			; Set vertical blank end to VTOT - 4
;
; Set vertical sync start
;
	mov	ax,es:[di].CRTCINFOBLOCK.VerticalSyncStart

	dec	ax

	bt	ax,8
	setc	cl
	shl	cl,2
	or	ch,cl
	bt	ax,9
	setc	cl
	shl	cl,7
	or	ch,cl			; CH = Added Bit 7,2 for VSS 9,8
	bt	ax,10
	setc	cl
	shl	cl,6
	or	bh,cl			; BH = Bit 6, 0 has VSS[10],VTot[10]
	mov	ah,al
	mov	al,010h
	out	dx,ax
;
; Set vertical overflow register
;
	mov	al,007h
	call	GetIndexRegister
	and	ah,05Ah
	or	ah,ch
	out	dx,ax
;
; Set vertical extended overflow (for most modes this will be 0)
;
	mov	al,CRTC_VEXT_OVER
	call	GetIndexRegister
	and	ah,03Ch			; Lop off VSS and VTotal bits
	or	ah,bh			; Add extended overflow bits
	out	dx,ax
;
; Set vertical sync end
;
	mov	ah,byte ptr es:[di].CRTCINFOBLOCK.VerticalSyncEnd

	dec	ah

	and	ah,00Fh
	or	ah,020h
	mov	al,011h
	out	dx,ax

	call	PatchVertDisplayTime

;
; Set the polarities
;
	mov	ah,es:[di].CRTCINFOBLOCK.Flags
	and	ah,00Ch				; Isolate polarities...
	shl	ah,4				;  ...and move into position
	mov	dl,low MISC_INPUT
	in	al,dx
	and	al,03Fh				; Clear existing polarities...
	or	al,ah				;  ...and set the new ones
	mov	dl,low MISC_OUTPUT
	out	dx,al
;
; Set the interlace flag
;
	call	GetIOBase
	add	dx,REG_VIDPROCCFG
	in	eax,dx
	mov	bl,es:[di].CRTCINFOBLOCK.Flags
	and	bl,002h				; Isolate interlace flag...
	shl	bl,2				;  ...align it with VIDPROCCFG
	out	dx,eax
;
OEMFixupCRTC_exit:
	popad
	ret
OEMFixupCRTC	endp

;**********************************************************************
;CheckIf1024Mode - Checks CRTC register 01h, Horizontal Display End, to
;		see if we're a 1024xXXX mode.  If so, return zero flag set.
; Entry : DX = CRTC base address 3D4h or 3B4h
; Exit  : Zero flag = 1, if 1024xXXX mode
;			  Otherwize, Zero flag = 0.
;			  All registers are preserved.

public	CheckIf1024Mode

CheckIf1024Mode	proc	near
	push	ax

	mov	al,01h					;Read current value of CRTC 01h.
	call	GetIndexRegister		;Horizontal Display End
	cmp	ah, 7Fh					;Are we a 1024 horizontal mode?

	pop	ax
	ret
CheckIf1024Mode	endp

;******************************************************************************************
;
;PatchVertDisplayTime - This routine will disable VerticalX2, 5C[4]=0, and
;recalculate the Vertical Display Time, CR07, when doing mode 320x200, 
;320x240, 400x300, and 512x384.
;This routine is called from OEMFixupCRTC


PatchVertDisplayTime	proc	near
	pushad

	call	GetIOBase						
	add	dx,REG_VIDPROCCFG					;Let first figure out if we're a
	in		eax,dx								;320x200, 320x240, 400x300, or 512x384 mode.
	test	al,10h								;Check 5C[4] if VerticalX2 enabled
	jz		DoNotPatchVertDisplayTime		;Jump if not a VerticalX2 mode.

;We must be either a 320x200, 320x240, 512x384, or 400x300 mode.
	and	al, not 10h							;Clear 5C[4]
	out	dx,eax								;Write new 5C[4]

	xor	bx,bx

	call	GetCRTCAddr							;Get current Vertical Display Time 10 bit value.
	mov 	al,07h
	call	GetIndexRegister					;Get Vertical Display Time bit 8
	test	ah,02h
	jz		CheckCR07Bit6
	or		bh,01h

CheckCR07Bit6:
	test	ah,40h								;Get Vertical Display Time bit 9

	jz		ReadCR12
	or		bh,02h

ReadCR12:
	mov 	al,12h		
	call	GetIndexRegister					;Get Vertical Display Time bit 7-0
	mov	bl,ah

;BX =  Vertical Display Time			

;Because the BIOS default timing uses VerticalX2, the default vertical 
;display time is twice the expect display time.  I've disabled VerticalX2
;above, so now I need to divide the current vertical display time by 2.

	shr	bx,1	

;Now we need to write the new vertical display time.

	mov	ah,bl									;Write Vertical Display time bit 7-0
	out	dx,ax									;  to CR12.


	mov 	al,07h			 					;Write Vertical display time overflows
	call	GetIndexRegister
	and	ah,not 42h							;First clear overflow bits

	test	bh,01h	  							;Write Vertical display time bit 8, 
	jz		SetCR07Bit6							;  if needed.
	or		ah,02h
SetCR07Bit6:									;Write Vertical display time bit 9,
	test	bh,02h								;  if needed.
	jz		SetCR07
	or		ah,04h

SetCR07:
	out	dx,ax									;Write new CR07 value

DoNotPatchVertDisplayTime:
	popad
	ret
PatchVertDisplayTime	endp

;**********************************************************************

;******************************************************************************************

;
;	OEMProtectedModeInit - Protected mode initialization
;
;	Entry:	DS = Writeable selector to the code segment
;		ES = Selector to Seg0
;	Exit:	None
;
;	Note:	This routine is supposedly called when a VBE 3.0-aware
;		application switches to protected mode. This code is
;		executed in a 16:16 segment.
;
	public	OEMProtectedModeInit
OEMProtectedModeInit	proc
	assume	ds:_TEXT, es:Seg0
	ret
OEMProtectedModeInit	endp

;**********************************************************************
;
  ENDIF			; VESA_300
;
  IF VESA_DPMS
;
;	OEMSetDPMS - Set display power state
;
;	Entry:	BH = Requested power state:
;			0 = On		4 = Off
;			1 = Stand by	8 = Reduced on
;			2 = Suspend
;		DS = Seg0
;	Exit:	Carry flag set as follows:
;			NC = State set properly
;			CF = Invalid state requested
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
	public	tblSetDPMS
tblSetDPMS	db	000h, 008h, 002h, 000h, 00Ah
;
	public	OEMSetDPMS
OEMSetDPMS	proc
	assume	ds:Seg0, es:nothing
	cmp	bh,2				; Validate values
	jbe	short OEMSetDPMS_10
	cmp	bh,4
	jne	short OEMSetDPMS_error
OEMSetDPMS_10:
	push	eax
	push	ebx
	push	dx
;
	call	FullCpuOn			; Turn off VGA scanout (full CPU access)
	movzx	bx,bh
	test	bx,bx				; Turn on VGA?
	jnz	short OEMSetDPMS_20		;  N: Scanout in proper state
	call	FullCpuOff			;  Y: Turn on scanout (VGA shares access)
OEMSetDPMS_20:
	call	GetIOBase			; DX = IO base
	add	dx,REG_DACMODE
	in	eax,dx				; Get current settings
	and	al,0E1h				; Clear DPMS bits
	or	al,cs:[tblSetDPMS + bx]		; Set new settings...
	out	dx,eax				;  ...and write 'em out


	pop	dx
	pop	ebx
	pop	eax
	clc
	ret
;
OEMSetDPMS_error:
	stc
	ret
OEMSetDPMS	endp

;**********************************************************************
;
;	OEMGetDPMS - Get display power state
;
;	Entry:	DS = Seg0
;	Exit:	BH = Power state currently requested by controller
;			0 = On		4 = Off
;			1 = Stand by	8 = Reduced on
;			2 = Suspend
;
	public	tblGetDPMS
tblGetDPMS	label	byte
	db	0, 2, 0, 2, 1, 4, 1, 4
;
	public	OEMGetDPMS
OEMGetDPMS	proc
	assume	ds:Seg0, es:nothing
	push	eax
	push	dx
	push	si
;
	call	GetIOBase
	add	dx,REG_DACMODE
	in	eax,dx
	and	ax,0000Eh
	shr	ax,1
	mov	si,ax
	mov	bh,cs:[tblGetDPMS + si]
;
	pop	si
	pop	dx
	pop	eax
	ret
OEMGetDPMS	endp

;**********************************************************************
;
  ENDIF			; VESA_DPMS
;
  IF VESA_DDC
;
;	OEMDDCClockHighDataHigh - Set clock high, data high
;
;	Entry:	None
;	Exit:	None
;
	public	OEMDDCClockHighDataHigh
OEMDDCClockHighDataHigh	proc	near
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	or	eax,000180000h
	out	dx,eax
	pop	dx
	pop	ax
	ret
OEMDDCClockHighDataHigh	endp

;**********************************************************************
;
;	OEMDDCClockLowDataHigh - Set clock low, data high
;
;	Entry:	None
;	Exit:	None
;
	public	OEMDDCClockLowDataHigh
OEMDDCClockLowDataHigh	proc	near
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	and	eax,0FFE7FFFFh
	or	eax,000100000h
	out	dx,eax
	pop	dx
	pop	ax
	ret
OEMDDCClockLowDataHigh	endp

;**********************************************************************
;
;	OEMDDCClockHighDataLow - Set clock high, data low
;
;	Entry:	None
;	Exit:	None
;
	public	OEMDDCClockHighDataLow
OEMDDCClockHighDataLow	proc	near
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	and	eax,0FFE7FFFFh
	or	eax,000080000h
	out	dx,eax
	pop	dx
	pop	ax
	ret
OEMDDCClockHighDataLow	endp

;**********************************************************************
;
;	OEMDDCClockLowDataLow - Set clock low, data low
;
;	Entry:	None
;	Exit:	None
;
	public	OEMDDCClockLowDataLow
OEMDDCClockLowDataLow	proc	near
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	and	eax,0FFE7FFFFh
	out	dx,eax
	pop	dx
	pop	ax
	ret
OEMDDCClockLowDataLow	endp

;**********************************************************************
;
;	OEMDDCIsClockHigh - Return status of clock line
;
;	Entry:	None
;	Exit:	Zero flag set as follows:
;			ZF = Clock is low
;			NZ = Clock is high
;
	public	OEMDDCIsClockHigh
OEMDDCIsClockHigh	proc	near
	assume	ds:nothing, es:nothing
	push	eax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	test	eax,000200000h
	pop	dx
	pop	eax
	ret
OEMDDCIsClockHigh	endp

;**********************************************************************
;
;	OEMDDCIsDataHigh - Return status of data line
;
;	Entry:	None
;	Exit:	Zero flag set as follows:
;			ZF = Data is low
;			NZ = Data is high
;
	public	OEMDDCIsDataHigh
OEMDDCIsDataHigh	proc	near
	assume	ds:nothing, es:nothing
	push	eax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	test	eax,000400000h
	pop	dx
	pop	eax
	ret
OEMDDCIsDataHigh	endp

;**********************************************************************
;
;	OEMDDCEnable - Enable the DDC protocol
;
;	Entry:	None
;	Exit:	None
;
	public	OEMDDCEnable
OEMDDCEnable	proc	near
	assume	ds:nothing, es:nothing
	push	eax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	or	eax,000040000h
	out	dx,eax
	pop	dx
	pop	eax
	ret
OEMDDCEnable	endp

;**********************************************************************
;
;	OEMDDCDisable - Disable the DDC protocol
;
;	Entry:	None
;	Exit:	None
;
	public	OEMDDCDisable
OEMDDCDisable	proc	near
	assume	ds:nothing, es:nothing
	push	eax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	and	eax,09FFBFFFFh
	out	dx,eax
	pop	dx
	pop	eax
	ret
OEMDDCDisable	endp

;**********************************************************************
;
;	OEMDDCEnableDFP - Enable the DDC protocol
;
;	Entry:	None
;	Exit:	None
;
	public	OEMDDCEnableDFP
OEMDDCEnableDFP	proc	near
	assume	ds:nothing, es:nothing
	push	eax
	push	dx
	call	GetIOBase
	add	dx,REG_VIDSERIALPARALLEL
	in		eax,dx
	and	eax,00FFFFFFFh
	or		eax,020040000h
	out	dx,eax
	pop	dx
	pop	eax
	ret
OEMDDCEnableDFP	endp

;**********************************************************************
;
  ENDIF		; VESA_DDC
;
ENDIF		; VESA
;
IF (VESA + EXTENDED_BIOS)
;
;	OEMSetBank - Set the aperature to the given bank number
;
;	Entry:	DX = Bank number
;	Exit:	None
;
;	All registers are preserved.
;
	public	OEMSetBank
OEMSetBank	proc
	assume	ds:nothing, es:nothing
	call	OEMSetWBank
	call	OEMSetRBank
	ret
OEMSetBank	endp

;**********************************************************************
;
;	OEMSetWBank - Set the aperature to the given bank number (Write bank only)
;
;	Entry:	DL = Bank number
;	Exit:	None
;
;	All registers are preserved.
;
	public	OEMSetWBank
OEMSetWBank	proc
	assume	ds:nothing, es:nothing
	push	eax
	push	ebx
	push	dx
;
	sub	ebx,ebx 			; Clear hi-order word
	mov	bx,dx
	shl	bx,1				; Get number of 32k banks
	push	ebx				; Save write bank

	and	bx,03FFh			; Preserve only bits [09:00]

	call	GetIOBase
	add	dx,REG_VGAINIT1
	in		eax,dx
	and	eax,not 600003FFh	; Clear write appeture bits
	or		ax,bx				; Fill in write bank

	pop	ebx  				; Get back write bank.

;Work on getting bit 11:10 of the write appertue.
	and	ebx,not 03FFh	; Preserve only bits [11:10]
	shl	ebx,19	 		; Move write appeture bits [11:10] to bit 
								;   positions [30:29]
	or		eax,ebx			; OR in new write appeture

	out	dx,eax			;Write it
;
	pop	dx
	pop	ebx
	pop	eax
	ret
OEMSetWBank	endp

;**********************************************************************
;
;	OEMGetWBank - Return the current bank for writing
;
;	Entry:	None
;	Exit:	DX = Bank #
;
	public	OEMGetWBank
OEMGetWBank	proc
	assume	ds:nothing, es:nothing
	push	eax
	push	ebx
;
	call	GetIOBase
	add	dx,REG_VGAINIT1
	in	eax,dx

	mov	ebx,eax			; Save bank in EBX

	and	ax,03FFh

	and	ebx,060000000h	;Preserve bits [11:10] of the write appeture.
	shr	ebx,19			;Move bits from [30:29] to [11:10]
	or		ax,bx				;Merge the two halves of the bank together.

	shr	ax,1				; Get number of 64k banks
	mov	dx,ax
;
	pop	ebx
	pop	eax
	ret
OEMGetWBank	endp

;**********************************************************************
;
;	OEMSetRBank - Set the aperature to the given bank number
;
;	Entry:	DX = Bank number
;	Exit:	None
;
;	All registers are preserved.
;
	public	OEMSetRBank
OEMSetRBank	proc
	assume	ds:nothing, es:nothing
	push	eax
	push	ebx
	push	dx
;
	sub	ebx,ebx
	mov	bx,dx

	shl	ebx,1 					; Get number of 32k banks

	push	ebx						; Save Read bank

	and	bx,03FFh

	shl	ebx,10					; Get number of 32k banks

	call	GetIOBase
	add	dx,REG_VGAINIT1
	in		eax,dx
	and	eax,0FFF003FFh
	or		eax,ebx					; Fill in read bank
	out	dx,eax

	pop	ebx						;Get Read appeture
	and	bx,00C00h				;Preserve read appetures bits [11:10]
	shl	ebx,15					;Move read appetures bits [11:10] to bit [26:25]

	call	GetIOBase
	add	dx,REG_VGAINIT0
	in		eax,dx
	and	eax,not 006000000h	; Clear bits [11:10] of the read appeture
										; 	REG_VGAINIT0[26:25]
	or		eax,ebx
	out	dx,eax

;
	pop	dx
	pop	ebx
	pop	eax
	ret
OEMSetRBank	endp

;**********************************************************************
;
;	OEMGetRBank - Get the aperature to the read bank number
;
;	Entry:	None
;	Exit:		DX = Bank number
;
;	All registers are preserved.
;
	public	OEMSetRBank
OEMGetRBank	proc
	assume	ds:nothing, es:nothing
	push	eax
	push	ebx
;
	call	GetIOBase
	add	dx,REG_VGAINIT1
	in	eax,dx						;Get bits [9:0] of the read bank
	and	eax,not 0FFF003FFh

	shr	eax,10					;Move bits from [19:10] to [9:0]
	mov	bx,ax						;Save bank in BX

	call	GetIOBase
	add	dx,REG_VGAINIT0
	in		eax,dx
	and	eax,006000000h			;Save bits [11:10] of the read appeture
	shr	eax,15 					;Move bits [26:25] to [11:10]

	or		bx,ax

	shr	bx,1

	mov	dx,bx

;
	pop	ebx
	pop	eax
	ret
OEMGetRBank	endp
;**********************************************************************
;
ENDIF
;
IF EXTENDED_BIOS
;
;	OEMIncBank - Set the aperature to the next bank number
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;	Assume extensions are enabled.
;
	public	OEMIncBank
OEMIncBank	proc
	assume	ds:nothing, es:nothing
	push	dx
	call	OEMGetWBank
	inc	dx
	call	OEMSetBank
	pop	dx
	ret
OEMIncBank	endp

;**********************************************************************
;
;	OEMIncRBank - Set the aperature to the next bank number (Read bank only)
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;	Assume extensions are enabled.
;
	public	OEMIncRBank
OEMIncRBank	proc
	assume	ds:nothing, es:nothing
	push	dx
	call	OEMGetRBank
	inc	dx
	call	OEMSetRBank
	pop	dx
	ret
OEMIncRBank	endp

;**********************************************************************
;
;	OEMDecRBank - Set the aperature to the next bank number (Read bank only)
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;	Assume extensions are enabled.
;
	public	OEMDecRBank
OEMDecRBank	proc
	assume	ds:nothing, es:nothing
	push	dx
	call	OEMGetRBank
	dec	dx
	call	OEMSetRBank
	pop	dx
	ret
OEMDecRBank	endp

;**********************************************************************
;
;	OEMIncWBank - Set the aperature to the next bank number (Write bank only)
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;	Assume extensions are enabled.
;
	public	OEMIncWBank
OEMIncWBank	proc
	assume	ds:nothing, es:nothing
	push	dx
	call	OEMGetWBank
	inc	dx
	call	OEMSetWBank
	pop	dx
	ret
OEMIncWBank	endp

;**********************************************************************
;
;	OEMDecWBank - Set the aperature to the next bank number (Write bank only)
;
;	Entry:	None
;	Exit:	None
;
;	All registers are preserved.
;	Assume extensions are enabled.
;
	public	OEMDecWBank
OEMDecWBank	proc
	assume	ds:nothing, es:nothing
	push	dx
	call	OEMGetWBank
	dec	dx
	call	OEMSetWBank
	pop	dx
	ret
OEMDecWBank	endp

;**********************************************************************
;
;	OEMLoad16BPPDAC - Load 16 BPP RAMDAC settings
;
;	Entry:	DS = Seg0
;	Exit:	None
;
;	Note:	Load a modified color ramp into the RAMDAC
;
	public	OEMLoad16BPPDAC
OEMLoad16BPPDAC	proc
	assume ds:Seg0, es:nothing
	jmp	LoadColor256DAC
OEMLoad16BPPDAC	endp

;**********************************************************************
;
;	OEMLoad24BPPDAC - Load 24 BPP RAMDAC settings
;
;	Entry:	DS = Seg0
;	Exit:	None
;
;	Note:	Load a color ramp into the RAMDAC
;
	public	OEMLoad24BPPDAC
OEMLoad24BPPDAC	proc
	assume ds:Seg0, es:nothing
	jmp	LoadColor256DAC
OEMLoad24BPPDAC	endp

;**********************************************************************
;
;	OEMExtensions1 - Extended OEM functions #1
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		AL = Subfunction
;	Exit:	AL = 14h
;		AH = Error code (00h = Function passed, 01h = Function failed)
;
	public	OEMExtensions1
OEMExtensions1	proc
	assume	ds:Seg0, es:nothing
	pop	ds
	pop	si
	iret
OEMExtensions1	endp

;**********************************************************************
;
;	OEMExtensions2 - Extended OEM functions #2
;
;	Entry:	SI, DS are pushed on the stack
;		DS = Seg0
;		AL = Subfunction
;	Exit:	AL = 15h
;		AH = Error code (00h = Function passed, 01h = Function failed)
;
	public	OEMExtensions2
OEMExtensions2	proc
	assume	ds:Seg0, es:nothing
	pop	ds
	pop	si
	iret
OEMExtensions2	endp

;**********************************************************************
;
;	OEMExtensionsAlt - Extended OEM functions on failed ALT-select lookup
;
;	Entry:	AH = 12h
;		BL = Subfunction
;		DS = Seg0
;		Other registers dependent on subfunctions
;	Exit:	AL = 12h (if subfunction is handled)
;		AH = Error code (00h = Function passed, 01h = Function failed)
;
;	Note:	SI and DS have been saved and will be restored on exit.
;
	public	OEMExtensionsAlt
OEMExtensionsAlt	proc
	assume	ds:Seg0, es:nothing
	ret
OEMExtensionsAlt	endp

;**********************************************************************
;
ENDIF
;
IF PCIBIOS
;
;	OEMGetBusType - Return the expansion bus type
;
;	Entry:	None
;	Exit:	AL = Bus type:
;			01h = ISA bus
;			02h = PCI bus
;			03h = VL bus
;
	public	OEMGetBusType
OEMGetBusType	proc	near
	assume	ds:nothing, es:nothing
	mov	al,BUS_PCI
	ret
OEMGetBusType	endp

;**********************************************************************
;
ENDIF

;
;	GetIOBase - Return the I/O base for the 3Dfx extended I/O
;
;	Entry:	None
;	Exit:	DX = I/O base (0 = Error)
;
;	All other registers are preserved.
;
	public	GetIOBase
GetIOBase	proc
	push	ax
	call	GetCRTCAddr
	mov	al,SCRATCH_REG0
	call	GetIndexRegister
	mov	dh,ah
	sub	dl,dl
	pop	ax
	ret
GetIOBase	endp

;**********************************************************************

;
;	GetMemBase - Return the memory base for the 3Dfx memory space
;
;	Entry:	None
;	Exit:	EAX = Memory base (0 = Error)
;
;	All other registers are preserved.
;
	public	GetMemBase
GetMemBase	proc
	assume ds:nothing, es:nothing
	push	bx
	push	ecx
	push	dx
	push	di
	push	si
;
	mov	ax,0B102h		; Find PCI device
	sub	si,si
	mov	cx,PCI_ID_DEVICE
	mov	dx,PCI_ID
	call	SafeInt1A		; Check INT 1Ah for existance first
	jc	short GetMemBase_error	;  Error? Y: Exit with error
	test	ah,ah			; Completion successful?
	jnz	short GetMemBase_error	;  N: Exit with error
;
	mov	ax,0B10Ah		; Read config DWORD
	mov	di,014h			; Memory Base Address
	int	1Ah
	test	ah,ah			; Function passed?
	jnz	short GetMemBase_error	;  N: Exit with error
	mov	eax,ecx
	and	al,0F0h			; Clear information bits
;
GetMemBase_exit:
	pop	si
	pop	di
	pop	dx
	pop	ecx
	pop	bx
	ret
;
GetMemBase_error:
	sub	eax,eax
	jmp	short GetMemBase_exit
GetMemBase	endp

;**********************************************************************
;
;	IsVGAMode - Determine if the current mode is in VGA mode
;
;	Entry:	None
;	Exit:	Zero flag set as follows:
;			ZF = VGA mode
;			NZ = Native mode
;
	public	IsVGAMode
IsVGAMode	proc
	assume ds:nothing, es:nothing
	push	eax
	push	dx
;
	call	GetIOBase
	add	dx,REG_VIDPROCCFG
	in	eax,dx
	test	al,001h
;
	pop	dx
	pop	eax
	ret
IsVGAMode	endp

;**********************************************************************
;
;	SetAbsoluteDotClock - Set the pixel clock to the given frequency
;
;	Entry:	EAX = Clock frequency in HZ
;	Exit:	None
;
	public	SetAbsoluteDotClock
SetAbsoluteDotClock	proc
	assume	ds:nothing, es:nothing
	pushad
	call	CalcClockWord
	call	GetIOBase
	add	dx,REG_VIDCLOCK
	and	eax,0FFFFh		; Clear high order word
	out	dx,eax
	mov	dx,MISC_INPUT
	in	al,dx
	or	al,00Ch							; Select extended clock
	mov	dl,low MISC_OUTPUT
	out	dx,al
	popad
	ret
SetAbsoluteDotClock	endp

;**********************************************************************
;
;	CalcClockWord - Calculate M,N,P values for a given frequency
;
;	Entry:	EAX = Clock frequency in HZ
;	Exit:	AX = M,N,P value
;
;	The clock equation is:
;		fout = (14.31818 + (N + 2) / (M + 2)) / (2 ** P)
;
;	The Clock WORD is:
;		Bits 0-1:	P
;		Bits 2-7:	M
;		Bits 8-15:	N
;
;	EAX, EBX, ECX, ESI, EDI, EBP are trashed.
;
	public	CalcClockWord
CalcClockWord	proc
	assume	ds:nothing, es:nothing
	mov	ebp,eax				; EBP = Target clock
	mov	esi,0FFFFFFFFh			; ESI = Best delta
	sub	ebx,ebx				; Upper EBX = Best N,P
	mov	ecx,ebx				; Upper ECX = Best M
;
; Try each "P" value from 0 to 3
;
	mov	bl,1

	cmp	ebp,149000000	;Is Fout above 149MHz
	jge	CalcClockWord_10

	inc	bl					;Set BL to 02h for clock above or equal 65MHz

	cmp	ebp,65000000	;Is Fout above 65MHz
	jge	CalcClockWord_10

	inc	bl					;Set BL to 03h for clock below 65MHz
CalcClockWord_10:
	cmp	bl,4				; Any more P's to try?
	jae	short CalcClockWord_1000	;  N: Go exit
;
	mov	bh,1				; BH = "M" = 1
CalcClockWord_20:
	cmp	bh,64				; Any more M's to try?
	jae	short CalcClockWord_80		;  N: Go to the next P
;
	sub	ch,ch				; CH = "N"
CalcClockWord_30:
	cmp	ch,253				; Any more N's to try?
	jae	short CalcClockWord_70		;  N: Go to next M
;
	call	CheckVCO
	jc		short CalcClockWord_50

	call	CalcFreq			; EAX = New frequency
	sub	eax,ebp				; EAX = Delta between target and this
	jnc	short CalcClockWord_40
	neg	eax
CalcClockWord_40:
	cmp	esi,eax				; Does this beat the best?
	jbe	short CalcClockWord_50		;  N: Skip ahead
	mov	esi,eax				;  Y: New delta
	push	bx
	shl	ebx,16				; Save best M,P
	pop	bx
	push	cx
	shl	ecx,16				; Save best N
	pop	cx
CalcClockWord_50:
;
	inc	ch
	jmp	short CalcClockWord_30		; End "N" loop
;
CalcClockWord_70:
	inc	bh
	jmp	short CalcClockWord_20		; End "M" loop
;
CalcClockWord_80:
	inc	bl
	jmp	short CalcClockWord_10		; End "P" loop
;
; All done, so retrieve the best M,N,& P, and build the clock WORD.
;
CalcClockWord_1000:
	shr	ebx,16				; BH = "M", BL = "P"
	shr	ecx,16				; CH = "N"
	mov	al,bl
	shl	bh,2
	or	al,bh
	mov	ah,ch				; AX = Clock word
	ret
CalcClockWord	endp

;**********************************************************************
;CheckVCO - Check of the VCO is within the operating limits
;
;	Entry:	BL = P Value
;				BH = M Value
;				CH = N Value
;				EBP = Desired pixel clock
;	Exit:	CLC if within operating limits
;			STS if out of operating limits
	public	CheckVCO
CheckVCO	proc
	assume	ds:nothing, es:nothing
;if ((PixelClock > 36MHz) && (m > 10)) break;
	cmp	ebp,36000000
	jle	CheckHigherVCO
	cmp	bh, 10
	jg		FailedCheckVCO

;if ((PixelClock > 200MHz) && (m > 5)) break;
CheckHigherVCO:
	cmp	ebp,200000000
	jle	PassedVCOCheck
	cmp	bh, 5
	jg		FailedCheckVCO

PassedVCOCheck:
	clc
	jmp	short EndCheckVCO

FailedCheckVCO:
	stc

EndCheckVCO:
	ret
CheckVCO	endp

;**********************************************************************
;
;	CalcFreq - Calculate the frequency in HZ from a given M,N,P set
;
;	Entry:	BL = P Value
;		BH = M Value
;		CH = N Value
;	Exit:	EAX = Frequency
;
;	All registers are preserved.
;
	public	CalcFreq
CalcFreq	proc
	assume	ds:nothing, es:nothing
	push	bx
	push	ecx
	push	edx
;
	add	bh,2
	add	ch,2
	mov	cl,bl
	mov	bl,1
	shl	bl,cl
	mov	eax,14318180			; Reference frequency (in Hz)
	movzx	edx,ch
	imul	edx				; Freq = 14318180 * N
	movzx	ecx,bh
	idiv	ecx				; Freq = Freq / M
	sub	edx,edx
	movzx	ecx,bl
	idiv	ecx				; Freq = Freq / P
;
	pop	edx
	pop	ecx
	pop	bx
	ret
CalcFreq	endp

;**********************************************************************
;
;	WhatIsClockWord - Determine the frequence of a given clock word
;
;	Entry:	AX = Clock word
;	Exit:	EAX = Clock frequency
;
;	BX and CX are trashed.
;
;	The Clock WORD is:
;		Bits 0-1:	P
;		Bits 2-7:	M
;		Bits 8-15:	N
;
	public	WhatIsClockWord
WhatIsClockWord	proc
	assume	ds:nothing, es:nothing
	mov	bl,al
	and	bl,003h		; BL = "P"
	mov	bh,al
	shr	bh,2		; BH = "M"
	mov	ch,ah		; CH = "N"
	call	CalcFreq
	ret
WhatIsClockWord	endp

;**********************************************************************
;
;	GetExtRegsEntry - Get a pointer to a given mode's extended regsiters
;
;	Entry:	ES:BX = Pointer to MODEDESC entry
;	Exit:	ES:SI = Pointer to EXTREGS entry
;
	public	GetExtRegsEntry
GetExtRegsEntry	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	cx
;

	movzx	ax,es:[bx].MODEDESC.mdIMode	; Get internal mode number

	cmp	es:[bx].MODEDESC.mdMode,07h
	je		LoadTextModeEntry

	cmp	es:[bx].MODEDESC.mdMode,03h
	ja		NoExtRegsEntryPatch

LoadTextModeEntry:
	mov	al,es:[bx].MODEDESC.mdMode
	call	OEMGetInternalMode
	xor	ah,ah

NoExtRegsEntryPatch:
	mov	cl,size EXTREGS			; Calculate offset into EXTREGS table
	mul	cl
	call	GetOEMTablePtr			; ES:BX = Pointer to OEMDATA table
	les	si,es:[bx].OEMDATA.oemExtRegs
	add	si,ax				; ES:SI = Pointer to EXTREGS for this mode
;
	pop	cx
	pop	ax
	ret
GetExtRegsEntry	endp

;**********************************************************************
;This routine checks Scratch register 2[7] to see if CRT is active.
;Will return zero flag = 0 if CRT active
;Entry: NONE
;Exit : Zero flag cleared if CRT active
	public	IsCRTAttached
IsCRTAttached         proc    near
	push	ax
	push	dx

	call	GetCRTCAddr
	mov	al,SCRATCH_REG2
	call	GetIndexRegister

	test	ah,FLD_CRT_ACTIVE

	pop	dx
	pop	ax

	ret
IsCRTAttached         endp

;**********************************************************************
;


;**********************************************************************
IF      (DVI_BIOS OR TVOUT_BIOS)

;
;	I2CDelay - Delay long enough to give the I2C hardware time to catch up
;
;	Entry:	None
;	Exit:	None
;
	public	I2CDelay
I2CDelay	proc
	assume	ds:nothing, es:nothing
	push	cx
	mov	cx,3
	call	Delay15us
	pop	cx
	ret
I2CDelay	endp

;**********************************************************************
;
;	I2CStart - Send the start strobe up the I2C line
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	None
;
	public	I2CStart
I2CStart	proc
	assume	ds:nothing, es:nothing
	call	I2CData1
	call	I2CClock1
	call	I2CData0
	call	I2CClock0
	ret
I2CStart	endp

;**********************************************************************
;
;	I2CStop - Send the stop strobe up the I2C line
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	None
;
	public	I2CStop
I2CStop	proc
	assume	ds:nothing, es:nothing
	call	I2CClock0
	call	I2CData0
	call	I2CClock1
	call	I2CData1
	ret
I2CStop	endp

;**********************************************************************
;
;	I2CData0 - Write a 0 to the I2C data line
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	None
;
	public	I2CData0
I2CData0	proc
	assume	ds:nothing, es:nothing
	push	eax
	in	eax,dx			; Get current value
	and	eax,0FDFFFFFFh		; Set SDA to 0
	out	dx,eax			; And send it along it's way
	call	I2CDelay		; Wait for hardware to catch up
	pop	eax
	ret
I2CData0	endp

;**********************************************************************
;
;	I2CData1 - Write a 1 to the I2C data line
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	None
;
	public	I2CData1
I2CData1	proc
	assume	ds:nothing, es:nothing
	push	eax
	in	eax,dx			; Get current value
	or	eax,002000000h		; Set SDA to 1
	out	dx,eax			; And send it along it's way
	call	I2CDelay		; Wait for hardware to catch up
	pop	eax
	ret
I2CData1	endp

;**********************************************************************
;
;	I2CClock0 - Write a 0 to the I2C clock line
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	None
;
	public	I2CClock0
I2CClock0	proc
	assume	ds:nothing, es:nothing
	push	eax
	in	eax,dx			; Get current value
	and	eax,0FEFFFFFFh		; Set SCL to 0
	out	dx,eax			; And send it along it's way
	call	I2CDelay		; Wait for hardware to catch up
	pop	eax
	ret
I2CClock0	endp

;**********************************************************************
;
;	I2CClock1 - Write a 1 to the I2C clock line
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	None
;
	public	I2CClock1
I2CClock1	proc
	assume	ds:nothing, es:nothing
	push	eax
	in	eax,dx			; Get current value
	or	eax,001000000h		; Set SCL to 1
	out	dx,eax			; And send it along it's way
	call	I2CDelay		; Wait for hardware to catch up
	pop	eax
	ret
I2CClock1	endp

;**********************************************************************
;
;	I2CDataReadBit - Read the SDA line
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	AL = 1 or 0
;
	public	I2CDataReadBit
I2CDataReadBit	proc
	assume	ds:nothing, es:nothing
	push	eax			; Save high order WORD
	in	eax,dx
	bt	eax,27
	pop	eax			; Restore high order WORD (*** DOESN'T AFFECT FLAGS ***)
	setc	al
	ret
I2CDataReadBit	endp

;**********************************************************************
;
;	I2CClockReadBit - Read the SCL line
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	AL = 1 or 0
;
	public	I2CClockReadBit
I2CClockReadBit	proc
	assume	ds:nothing, es:nothing
	push	eax			; Save high order WORD
	in	eax,dx
	bt	eax,26
	pop	eax			; Restore high order WORD (*** DOESN'T AFFECT FLAGS ***)
	setc	al
	ret
I2CClockReadBit	endp

;**********************************************************************
;
;	I2CWriteRegister - Write an I2C register
;
;	Entry:	AL = Register to write
;		AH = Data to write
;		DX = I/O address of I2C register
;	Exit:	CX = Status (0 = Failure, non-0 = Success)
;
	public	I2CWriteRegister
I2CWriteRegister	proc
	assume	ds:nothing, es:nothing
	push	eax
;
	mov	cx,5				; Allow 5 retries
I2CWriteRegister_10:
	call	I2CStart
	push	ax
	mov	al,088h
	call	I2CSendByte			; Send BT868 address
	pop	ax
	jc	short I2CWriteRegister_20	; Error? Y: Skip sending data
	call	I2CSendByte			; Send register
	jc	short I2CWriteRegister_20	; Error? Y: Skip sending data
	xchg	al,ah
	call	I2CSendByte			; Send the data
	xchg	al,ah				; *** DOESN'T AFFECT FLAGS ***
I2CWriteRegister_20:
	pushf					; Save status of writes...
	call	I2CStop				;  * This reset sequence *
;;;	call	I2CData0			;  * needs to occur on   *
;;;	call	I2CClock0			;  * success OR failure. *
	popf					; ...Restore status of writes
	jnc	short I2CWriteRegister_exit	; Success? Y: Exit, N: Try again
	loop	I2CWriteRegister_10
;
; If execution gets here, then a failure has occured. After retrying
; several times, the dang device just wouldn't respond. Time to quit
; and go home. Since the CX register is set to 0 after the LOOP
; instruction, we can just fall through and use the CX as the success
; or failure condition.
;
	MONO_OUT	"I2CWriteRegister: Failure to write data."
;
I2CWriteRegister_exit:
	mov   	cx,0Ah
I2CWriteRegister_28:
	call	I2CDelay		; Wait for hardware to catch up
	loop	I2CWriteRegister_28
	pop	eax
	ret
I2CWriteRegister	endp

;**********************************************************************
;
;	I2CSendByte - Send a BYTE of data down the I2C pipeline
;
;	Entry:	AL = Data to write
;		DX = I/O address of I2C register
;	Exit:	Carry flag is set as follows:
;			NC = BYTE written successfully
;			CF = Error occurred
;
	public	I2CSendByte
I2CSendByte	proc
	assume	ds:nothing, es:nothing
	push	eax
	push	cx
;
	mov	cx,8			; Eight bits per byte
I2CSendByte_10:
	test	al,080h			; Write a 1?
	jnz	short I2CSendByte_20	;  Y: Go do it
	call	I2CData0		;  N: Write a 0
	jmp	short I2CSendByte_30
I2CSendByte_20:
	call	I2CData1
I2CSendByte_30:
	call	I2CClock1
	call	I2CClock0
	rol	al,1
	loop	I2CSendByte_10
;
; Check for the acknowledge
;
	call	I2CData1
	call	I2CClock1
	call	I2CDataReadBit
	bt	ax,0			; Reading a "1" is a failure
	jnc	short I2CSendByte_exit	; Failure? N: Skip ahead
	pushf				;  Y: Save condition as exit code
	call	I2CDelay		; Give hardware a chance to catch up
	popf				; Restore exit code
;
I2CSendByte_exit:
	pushf
	call	I2CClock0
	popf
	pop	cx
	pop	eax
	ret
I2CSendByte	endp

;**********************************************************************

ENDIF
;**********************************************************************


IF TVOUT_BIOS
;
;	I2CInit - Initialize the I2C interface
;
;	Entry:	None
;	Exit:	DX = I/O address of I2C register
;
;	Initialize DX to be the I/O base plus the offset of the register
;	used to talk to the I2C lines.
;
;	Notes:
;	1) I2C register is I/O base + 78h
;	2) The bits of this register are:
;		3322 2222 2222 1111 1111 11
;		1098 7654 3210 9876 5432 1098 7654 3210
;		xxxx EDCB Axxx xxxx xxxx xxxx xxxx xxxx
;		 A - I2C enable (1 = enable)
;		 B - SCL Out
;		 C - SDA Out
;		 D - SCL In
;		 E - SDA In
;	3) All other bits MUST be masked, since they control other 
;
	public	I2CInit
I2CInit	proc
	assume	ds:nothing, es:nothing
	push	eax
;
	call	GetIOBase			; DX = IO base
	add	dx,REG_VIDSERIALPARALLEL
	in	eax,dx
	and	eax,0F07FFFFFh
	or	eax,083800000h			; Enable I2C interface
	out	dx,eax
	call	I2CDelay			; Wait for hardware to catch up
;
	pop	eax
	ret
I2CInit	endp

;**********************************************************************

IsTVActive proc
	push	ax
	push	dx
;
	call	GetCRTCAddr
	mov	al,SCRATCH_REG2
	call	GetIndexRegister
  test  ah,FLD_TV_ACTIVE
;
	pop	dx
	pop	ax
  ret
IsTVActive endp

;**********************************************************************
;
;	IsNTSCMode - Determine if the current configuration is NTSC output
;
;	Entry:	None
;	Exit:	Zero flag set as follows:
;			ZF = Currently in NTSC mode
;			NZ = Not in NTSC mode
;
	public	IsNTSCMode
IsNTSCMode	proc
	push	ax
	push	dx

	call	I2CInit						;Sets DX to IOBase + 78
	mov	ah,00						   ;Adjust the read address
	call	I2CReadAuxEEPROM 			;Read the Aux EEPROM
	and	al,FLD_TVMASK
	cmp	al,FLD_NTSCMODE
;
	pop	dx
	pop	ax
	ret
IsNTSCMode	endp

;**********************************************************************
;
;	IsPALMode - Determine if the current configuration is PAL output
;
;	Entry:	None
;	Exit:	Zero flag set as follows:
;			ZF = Currently in NTSC mode
;			NZ = Not in NTSC mode
;
	public	IsPALMode
IsPALMode	proc
	push	ax
	push	dx

	call	I2CInit						;Sets DX to IOBase + 78
	mov	ah,00						   ;Adjust the read address
	call	I2CReadAuxEEPROM 			;Read the Aux EEPROM
	and	al,FLD_TVMASK
	cmp	al,FLD_PALMODE
;
	pop	dx
	pop	ax
	ret
IsPALMode	endp

;**********************************************************************

IF ROM_64K
;
;GetPALMode - Return which flavor of PAL TV we are.
;
; Entry:   None
; Exit :   CL = PAL TV mode.
;               00 = PAL (BGDHI)
;               01 = PAL N
;               02 = PAL M
;               03 = PAL Nc
;
GetPALMode    proc  near
  push  ax
  push  dx

	call	I2CInit						;Sets DX to IOBase + 78
	mov	ah,00						   ;Adjust the read address
	call	I2CReadAuxEEPROM 			;Read the Aux EEPROM
  and   al,FLD_PALMODEMASK

  mov   cl,al
  shr   cl,03h

  pop   dx
  pop   ax
  ret
GetPALMode    endp

;**********************************************************************
ENDIF

;
;	FixupTVOut - Fixup the CRTC for TV output
;
;	Entry:	ES:SI = Ptr to EXTREGS entry for current mode
;	Exit:	None
;
	public	FixupTVOut
FixupTVOut	proc
	assume	ds:Seg0, es:nothing

	pushad
	push	ds
;
	mov	cl,[byMode]			; CL = Mode number
	mov	ch,es:[si].EXTREGS.xrFlags	; CH = xrFlags
	push	cs
	pop	ds				; MOV DS,CS
	assume	ds:nothing
;
	movzx	bx,es:[si].EXTREGS.xrTVOutIdx
	cmp	bl,0FFh				; Can this mode be set?
	je	FixupTVOut_exit			;  N: Go exit
	imul	si,bx,size TVDATA
	mov	di,offset tblTVDataNTSC		; Assume NTSC
	call	IsNTSCMode			; Is it NTSC?
	jz	short FixupTVOut_10		;  Y: Got it...

IF ROM_64K
  call  GetPALMode        ;Determine if we're flavor or PAL we are.
  cmp   cl,FLD_PAL_M               ;Are we PAL M
  je    FixupTVOut_10
ENDIF

	mov	di,offset tblTVDataPAL		;  N: Use PAL
FixupTVOut_10:
	add	si,di				; DS:SI = Ptr to TVDATA entry
;
	call	GetIOBase
	mov	di,dx				; DI = IOBase
	add	dx,REG_VIDINFORMAT
	mov	eax,000008160h
	out	dx,eax
;
;No need to talk to MiscInit since this register was redefined in V3.
;;	lea	dx,[di + REG_MISCINIT1]
;;	mov	eax,040000001h
;;	out	dx,eax
;
	lea	dx,[di + REG_VIDCLOCK]
	movzx	eax,[si].TVDATA.tvClock
	out	dx,eax				; Dot clock
;
; Set the clock select to select the extended clock and set
; a polarity that is compatible with the VGA-TV conversion.
;
	mov	dx,MISC_INPUT
	in		al,dx

	mov	dx,MISC_OUTPUT
	or		al,0CCh				; Select ext. clock and Hsync and Vsync are +.
	out	dx,al
;
; Make sure the mode is an 8xN (not an 9xN) font by forcing the
; timing to 8 dot. Note that the font load (which happens after the
; registers are set -- namely, this routine) determines whether to
; load the 9-dot fixup based on the setting of the bit in SEQ[1].0.
;
	mov	dx,SEQ_INDEX
	mov	al,001h
	call	GetIndexRegister
	or	ah,001h
	out	dx,ax
;
; Set the pixel panning register to 0 (9-bit modes have a non-zero
; value by default.
;
	call	ResetATCFlipFlop
	mov	dl,low ATC_INDEX
	mov	al,033h
	out	dx,al
	sub	al,al
	out	dx,al
;
; Load the CRTC registers
;
	call	GetCRTCAddr
	mov	ax,02C11h
	out	dx,ax
	sub	al,al
	mov	ah,[si].TVDATA.tvCRTC0
	out	dx,ax
	mov	al,003h
	mov	ah,[si].TVDATA.tvCRTC3
	out	dx,ax
	inc	ax				; AL = 04h
	mov	ah,[si].TVDATA.tvCRTC4
	test	ch,ch				; Extended 8,16,24 BPP mode?
	jz	short FixupTVOUT_20		;  N: Use sync as is
	sub	ah,3				;  Y: Compensate for sync skew
FixupTVOUT_20:
	out	dx,ax
	mov	ah,[si].TVDATA.tvCRTC5
	mov	al,cl
	call	IsTextMode			; Is the current mode text?
	jnz	short FixupTVOUT_30		;  N: Skip ahead
	or	ah,020h				;  Y: Add sync skew of 1
FixupTVOUT_30:
	mov	al,005h
	out	dx,ax
	inc	ax				; AL = 06h
	mov	ah,[si].TVDATA.tvCRTC6
	out	dx,ax
	inc	ax				; AL = 07h
	mov	ah,[si].TVDATA.tvCRTC7
	out	dx,ax
	mov	al,010h
	mov	ah,[si].TVDATA.tvCRTC10
	out	dx,ax
	mov	ax,08C11h
	out	dx,ax

;;	call	I2CInit			; DX = I/O addr of I2C register
	call	GetCRTCAddr			
	mov	al,SCRATCH_REG2
	call	GetIndexRegister
	and	ah,FLD_COMPOSITE
	jz		DoSVideoOut			; Scratch_Reg2 bit6 (Set=Composite)
	call	I2CInit				; DX = I/O addr of I2C register
	mov	ax,000CEh				; Composite
	call	I2CWriteRegister
	mov	ax,023BAh				; Disable DAC-B
	call	I2CWriteRegister	; SLAVER
	jmp		ContinueWithOthers
DoSVideoOut:
	call	I2CInit				; DX = I/O addr of I2C register
	mov	ax,024CEh				; For swapped DAC lines
;	mov	ax,018CEh				; For unswapped DAC lines
	call	I2CWriteRegister	; Enable s-video DACs
	mov	ax,021BAh				; DAC-B and DAC-C enabled 
	call	I2CWriteRegister	; SLAVER

ContinueWithOthers:

	mov	al,0B8h
	mov	ah,[si].TVDATA.tvBT868Cfg
	call	I2CWriteRegister	; Config (PAL or NTSC)

;---------------------------------------------------------------------------
	call	IsPALMode         ; Check if we're PAL
	jnz	PatchNTSC

;;	mov	ax,00076h
;;	call	I2CWriteRegister
;;
;;	mov	ax,0847Ah
;;	call	I2CWriteRegister
;;
;;	mov	ax,0A47Ch
;;	call	I2CWriteRegister
;;
;;	mov	ax,06A7Eh
;;	call	I2CWriteRegister
;;
;;	mov	ax,07D80h
;;	call	I2CWriteRegister
;;
;;	mov	ax,0188Ch
;;	call	I2CWriteRegister
;;
;;	mov	ax,02786h
;;	call	I2CWriteRegister
;;
;;
;;	mov	ax,02082h
;;	call	I2CWriteRegister
;;
;;	mov	ax,0d884h
;;	call	I2CWriteRegister
;;
;;
;;	mov	ax,00b8eh
;;	call	I2CWriteRegister
;;
;;	mov	ax,0c08ah
;;	call	I2CWriteRegister
;;
;;
;;	mov	ax,0EA90h
;;	call	I2CWriteRegister
;;
;;	mov	ax,03992h
;;	call	I2CWriteRegister
;;
;;	mov	ax,09094h
;;	call	I2CWriteRegister
;;
;;	mov	ax,03596h
;;	call	I2CWriteRegister
;;
;;	mov	ax,0DE98h
;;	call	I2CWriteRegister
;;
;;	mov	ax,04D9Ah
;;	call	I2CWriteRegister
;;
;;
;;	mov	ax,00E9Ch
;;	call	I2CWriteRegister
;;
;;	mov	ax,0889Eh
;;	call	I2CWriteRegister
;;
;;	mov	ax,00CA0h
;;	call	I2CWriteRegister
;;
;;	mov	ax,02AA2h
;;	call	I2CWriteRegister
;;
;;
;;	mov	ax,057A6h
;;	call	I2CWriteRegister
;;
;;	mov	ax,080A8h
;;	call	I2CWriteRegister
;;
;;	mov	ax,048AAh
;;	call	I2CWriteRegister
;;
;;	mov	ax,06EAEh
;;	call	I2CWriteRegister
;;
;;	mov	ax,0DBB0h
;;	call	I2CWriteRegister
;;
;;	mov	ax,076B2h
;;	call	I2CWriteRegister
;;
;;	mov	ax,020B4h
;;	call	I2CWriteRegister

	jmp	ProgramBTConfig

PatchNTSC:
	cmp	[si].TVDATA.tvCRTC10,0B0h		;Are we a 640x400 or 320x200 mode??
	jne	CheckNTSC_640x350					;Jump if not a 640x400 or 320x200 mode.

;Must be a NTSC 640x400 and 320x200 timings
;Vertical + 2
	mov	ax,02082h
	call	I2CWriteRegister

	mov	ax,0d884h
	call	I2CWriteRegister

	mov	ax,0EA90h
	call	I2CWriteRegister

	mov	ax,03992h
	call	I2CWriteRegister

	mov	ax,09094h
	call	I2CWriteRegister

	mov	ax,03596h
	call	I2CWriteRegister

	mov	ax,0DE98h
	call	I2CWriteRegister

	mov	ax,04D9Ah
	call	I2CWriteRegister

	jmp	ProgramBTConfig

;---------------------------------------------------------------------------
CheckNTSC_640x350:
	cmp	[si].TVDATA.tvCRTC10,075h		;Are we a 640x350 mode??
	jne	ProgramBTConfig					;Jump if not a 640x350 mode.

;Must be NTSC 640x350 mode
	mov	ax,01E82h
	call	I2CWriteRegister

	mov	ax,0DD84h
	call	I2CWriteRegister

	mov	ax,0608Ah
	call	I2CWriteRegister

	mov	ax,0408Ch
	call	I2CWriteRegister

	mov	ax,00C8Eh
	call	I2CWriteRegister

	mov	ax,0A490h
	call	I2CWriteRegister

	mov	ax,02E92h
	call	I2CWriteRegister

	mov	ax,05E94h
	call	I2CWriteRegister

	mov	ax,03596h
	call	I2CWriteRegister

	mov	ax,09A98h
	call	I2CWriteRegister

	mov	ax,0499Ah
	call	I2CWriteRegister

	push	dx
	call	GetCRTCAddr			
	mov	ax,0A016h
	out	dx,ax
	pop	dx

ProgramBTConfig:
;---------------------------------------------------------------------------

; BT869 MSC adjustment for Sub-Carrier registers
; Fclk should reflect the actual Pixel clock input

	mov	al,0AEh
	mov	ah,[si].TVDATA.tvBT868MSC0
	call	I2CWriteRegister

	mov	al,0B0h
	mov	ah,[si].TVDATA.tvBT868MSC1
	call	I2CWriteRegister

	mov	al,0B2h
	mov	ah,[si].TVDATA.tvBT868MSC2
	call	I2CWriteRegister

	mov	al,0B4h
	mov	ah,[si].TVDATA.tvBT868MSC3
	call	I2CWriteRegister

;---------------------------------------------------------------------------

	mov	ax,000C4h
	call	I2CWriteRegister	; Do not enable outputs
	mov	ax,08DA0h
	call	I2CWriteRegister	; Enable external clock
	mov	ax,000C6h
	call	I2CWriteRegister	; RGB in, 12 bit mux'd, disable DAC A output.

;	mov	ax,021BAh
;	call	I2CWriteRegister	; SLAVER
;	mov	ax,024CEh			; For swapped DAC lines
;	mov	ax,018CEh			; For unswapped DAC lines
;	call	I2CWriteRegister	; Enable s-video DACs
	mov	ax,009C8h
	call	I2CWriteRegister	; Least degree of flicker filter

	mov	al,08Ch
	mov	ah,[si].TVDATA.tvBT868Blank
	call	I2CWriteRegister	; H Blank

IF ROM_64K
;This is where we patch the TV registers for PAL M, N, and Nc modes.
	call	IsPALMode         ; Check if we're PAL
  jnz   Donot_Patch_TV    ; Jump if not PAL

  call  GetPALMode        ;Determine if we're flavor or PAL we are.
  cmp   cl,FLD_PAL_BGDHI  ;Are we PAL BGDHI
  je    Donot_Patch_TV    ;No patch required for this PAL mode

  mov   si,offset tblTVPatchPALN   ;Load PAL N Patch
  cmp   cl,FLD_PAL_N               ;Are we PAL N
  je    Check_Patch_PAL_TV         ;Jump if we are

  mov   si,offset tblTVPatchPALM   ;Load PAL M Patch
  cmp   cl,FLD_PAL_M               ;Are we PAL M
  je    Check_Patch_PAL_TV         ;Jump if we are

  mov   si,offset tblTVPatchPALNc  ;Load PAL Nc Patch
  cmp   cl,FLD_PAL_Nc              ;Are we PAL Nc
  jne   Donot_Patch_TV             ;Jump if we are

Check_Patch_PAL_TV:
  cmp   bx,03                      ;Are we 800x600
  jne   StartPatch_PAL_TV
  add   si, size TVPALPATCH

StartPatch_PAL_TV:
	mov	al,0A2h
	mov	ah,[si].TVPALPATCH.PALRegA2
	call	I2CWriteRegister 
 
	mov	al,0AEh
	mov	ah,[si].TVPALPATCH.PALRegAE
	call	I2CWriteRegister 

	mov	al,0B0h
	mov	ah,[si].TVPALPATCH.PALRegB0
	call	I2CWriteRegister 

	mov	al,0B2h
	mov	ah,[si].TVPALPATCH.PALRegB2
	call	I2CWriteRegister 

	mov	al,0B4h
	mov	ah,[si].TVPALPATCH.PALRegB4
	call	I2CWriteRegister 

;This were we program common PAL TV parameters
;  mov   ax,02AA2h
;	call	I2CWriteRegister 

  mov   ax,0F0A4h
	call	I2CWriteRegister 

  mov   ax,057A6h
	call	I2CWriteRegister 

  mov   ax,080A8h
	call	I2CWriteRegister 

  mov   ax,048AAh
	call	I2CWriteRegister 

  mov   ax,08CACh
	call	I2CWriteRegister 

Donot_Patch_TV:
ENDIF
;
FixupTVOut_exit:
	pop	ds
	popad
	ret
FixupTVOut	endp

;**********************************************************************
;
ENDIF
;
IF      DVI_BIOS
        %out    !                               DVI BIOS

;**********************************************************************
;	Entry:	ES:SI = Ptr to EXTREGS entry for current mode
	public	FixupPanel
FixupPanel   proc    near
  push  ds
  push  es
  push  si
  push  di
  push  dx
	push	cx
  push  bx
  push  ax


  xor   ax,ax
  mov   ds,ax

  mov   bl,ds:[byMode]                  ;Get and save current mode number

;A cheap way of setting up DS and ES to CS :)
  push  cs
  pop   ds
  push  cs
  pop   es
	
;--------------------------------------------------------------------------------
	push	dx
	call	GetIOBase								; DX = IO base
	add	dx,REG_VIDINFORMAT					; Enable the GPIO port
	in 	eax,dx									;   to the panel.
	or		eax,00408000h							;Also set bit 22 to descramble colors.
	out	dx,eax									;   This fixes the problem with wrong colors on panel.

	call	GetIOBase								; DX = IO base
	add	dx,REG_TMUGBEINIT
	in 	eax,dx			 						 
IF	Napalm2_BIOS
	or		eax,000A0000h							; Delay panel data by 10 clocks
ELSE
	or		eax,00040000h							; Delay panel data by 4 clocks
ENDIF
	out	dx,eax ;

	pop	dx
	
	call	IsPanelSideScaling
	jz		NonScalingPanel

	push	dx
;--------------------------------------------------------------------------------
	call	GetIOBase								; DX = IO base
	add	dx,REG_VIDPROCCFG						; Need to disable the scanline
	in		eax,dx									;   doubling register.
	and	al,not 010h
	out	dx,eax

;We must be a scaled panel.
;--------------------------------------------------------------------------------
;Delay blank so that the image is one the screen.
	mov	cl,03h									;Default blank delay for all modes except mode 03(400 line)

	cmp	bl,03h									;Are we mode 03h?
	jne	DFP_ProgramMiscInit0					;Jump if not Mode 03h
	call	CheckIf400Scanline					; Are we a 400 scanline mode??
	jz		DFP_ProgramMiscInit0					; Jump if not a 400 scanlie mode.

	mov	cl,04h									;Default blank delay for mode 03(400 line)


DFP_ProgramMiscInit0:
	mov	dl,REG_MISCINIT0
	in		eax,dx									;Read current MiscInit0 setting
	and	ah,not 07h								;Clear bit 8-10
	or		ah,cl										;OR in new value
	out	dx,eax
;--------------------------------------------------------------------------------
	pop	dx

;Get panel EDID established timing block.

	push	bx					;Save internal mode number
	xor	eax,eax			;Clear EDID storage buffer

	mov	cx,1				;Read the first byte of the established timing block
	mov	si,25h
	call	DFPReadByte
	mov	al,bl
	shl	eax,8

	mov	cx,1				;Read the second byte of the established timing block
	mov	si,24h
	call	DFPReadByte
	mov	al,bl
	shl	eax,8

	mov	cx,1				;Read the third byte of the established timing block
	mov	si,23h
	call	DFPReadByte
	mov	al,bl
	pop	bx					;Get back internal mode number

;eax = panel established timing

;--------------------------------------------------------------------------------
;Check if BIOS mode is on established timing block.

;Are we 1024x768
	cmp	bl,05Eh									;Are we 1024x768x8
	je		Check1024x768EstTimingBit			;Jump if we're 1024x768x8
	cmp	bl,072h									;Are we 1024x768x16
	je		Check1024x768EstTimingBit			;Jump if we're 1024x768x16
	cmp	bl,073h									;Are we 1024x768x24
	je		Check1024x768EstTimingBit			;Jump if we're 1024x768x24

;Are we 512x384
	cmp	bl,026h									;Are we 512x384x8
	je		Check512x384EstTimingBit			;Jump if we're 512x384x8
	cmp	bl,027h									;Are we 512x384x16
	je		Check512x384EstTimingBit			;Jump if we're 512x384x16
	cmp	bl,028h									;Are we 512x384x24
	je		Check512x384EstTimingBit			;Jump if we're 512x384x24

;Are we 800x600
	cmp	bl,06Ah									;Are we 800x600x4
	je		Check800x600EstTimingBit			;Jump if we're 800x600x4
	cmp	bl,05Ch									;Are we 800x600x8
	je		Check800x600EstTimingBit			;Jump if we're 800x600x8
	cmp	bl,070h									;Are we 800x600x16
	je		Check800x600EstTimingBit			;Jump if we're 800x600x16
	cmp	bl,071h									;Are we 800x600x24
	je		Check800x600EstTimingBit			;Jump if we're 800x600x24

;Are we 400x300
	cmp	bl,023h									;Are we 400x300x8
	je		Check400x300EstTimingBit			;Jump if we're 400x300x8
	cmp	bl,024h									;Are we 400x300x16
	je		Check400x300EstTimingBit			;Jump if we're 400x300x16
	cmp	bl,025h									;Are we 400x300x24
	je		Check400x300EstTimingBit			;Jump if we're 400x300x24

;Are we 640x480
	cmp	bl,011h									;Are we 640x480x4
	je		Check640x480EstTimingBit			;Jump if we're 640x480x8
	cmp	bl,012h									;Are we 640x480x4
	je		Check640x480EstTimingBit			;Jump if we're 640x480x8
	cmp	bl,05Fh									;Are we 640x480x8
	je		Check640x480EstTimingBit			;Jump if we're 640x480x8
	cmp	bl,06Eh									;Are we 640x480x16
	je		Check640x480EstTimingBit			;Jump if we're 640x480x16
	cmp	bl,069h									;Are we 640x480x24
	je		Check640x480EstTimingBit			;Jump if we're 640x480x24

;Are we 320x240
	cmp	bl,020h									;Are we 320x240x8
	je		Check320x240EstTimingBit			;Jump if we're 320x240x8
	cmp	bl,021h									;Are we 320x240x16
	je		Check320x240EstTimingBit			;Jump if we're 320x240x16
	cmp	bl,022h									;Are we 320x240x24
	jne	CheckDetailedTimingField			;Jump if we're not 320x240x24

Check320x240EstTimingBit:
	test	al,20h								;Test if the panel support 1024x768@60Hz
	jz		CheckDetailedTimingField		;Parse EDID detailed timing field.
	
	mov	si,offset ExtMode320x240Table
;	mov	si,offset ExtMode640x480Table

	call	ProgramCRTCPanelTimings

	mov	eax,25175000
	call	SetAbsoluteDotClock

	jmp	ExitPanelRoutine					;Jump if supported.
;	jmp	RemoveCRTCBorders

Check640x480EstTimingBit:
	test	al,20h								;Test if the panel support 640x480@60Hz
	jnz	RemoveCRTCBorders					;Jump if supported.
	jmp	CheckDetailedTimingField		;Parse EDID detailed timing field.

Check1024x768EstTimingBit:
	test	ah,08h								;Test if the panel support 1024x768@60Hz
	jnz	ExitPanelRoutine					;Jump if supported.
	jmp	CheckDetailedTimingField		;Parse EDID detailed timing field.

Check512x384EstTimingBit:
	test	ah,08h								;Test if the panel support 1024x768@60Hz
	jz		CheckDetailedTimingField		;Parse EDID detailed timing field.

	mov	si,offset ExtMode512x384Table
;	mov	si,offset ExtMode1024x768Table

	call	ProgramCRTCPanelTimings

;;	mov	eax,65000000
	mov	eax,25180000
	call	SetAbsoluteDotClock

	jmp	ExitPanelRoutine					;Jump if supported.

Check800x600EstTimingBit:
	test	al,01h								;Test if the panel support 800x600@60Hz
	jnz	ExitPanelRoutine					;Jump if supported.
	jmp	CheckDetailedTimingField		;Parse EDID detailed timing field.

Check400x300EstTimingBit:
	test	al,01h								;Test if the panel support 1024x768@60Hz
	jz		CheckDetailedTimingField		;Parse EDID detailed timing field.

	mov	si,offset ExtMode400x300Table
;	mov	si,offset ExtMode800x600Table

	call	ProgramCRTCPanelTimings

	mov	eax,25180000
;	mov	eax,40000000
	call	SetAbsoluteDotClock

	jmp	ExitPanelRoutine					;Jump if supported.

;--------------------------------------------------------------------------------
CheckDetailedTimingField:

	push	ds
	xor	ax,ax
	mov	ds,ax
	mov	al,ds:[byMode]
	call	GetModeDescPtr			; Get pointer to mode description
	pop	ds

	mov	cx,cs:[bx].MODEDESC.mdXRes
	cmp	cx,40						; Are we 40 column text mode??
	jne	CheckIf80Column		; Jump if we not a 40 column text mode.
	mov	cx,320					; Say that we are a 320xXXX mode.
	jmp	LoadBIOSYRes			; Now let's see what our YRes is.

CheckIf80Column:
	cmp	cx,80	  					; Are we a 80 column text mode??
	jne	CheckIf320XRes			; Jump if we're not a 80 column text mode
	mov	cx,640					; Assume we're a 640xXXX mode
	call	CheckIf400Scanline	; Are we a 400 scanline mode??
	jz		LoadBIOSYRes			; Jump if not a 400 scanlie mode.
	mov	cx,720					; We must be 400 scan line, so we use a 9 dot font.
										; 9 times 80 equals 720 pixels.
	jmp	LoadBIOSYRes			; Now let's see what our YRes is.

CheckIf320XRes:
;This is where you need to check for all those low res modes.
;	cmp	cx,640					; Are we 320xXXX mode.
;	jge	LoadBIOSYRes			; Jump if not a 320xXXX mode
;	shl	cx,1						; Return that we're actually a 640 mode.

	cmp	cx,640					; Are we a halved mode??
	jge	LoadBIOSYRes			; Jump if not a halved mode
	
	mov	cx,640					; Force a 640xXXX desktop mode.

LoadBIOSYRes:
	shl	ecx,16					; ECX[31:16] = XRes
	mov	cx,cs:[bx].MODEDESC.mdYRes
	cmp	cx,25						; Are we a 25 line text mode??
	jne	Check200YRes			; Jump if we're not.

;We a 25 line text mode.

	mov	cx,400					; Assume we're a 400 scanline text mode
	call	CheckIf400Scanline	; Are we a 400 scanline text mode??
	jnz	DoneCalcBIOSXYRes		; Jump if we're a 400 scanline text mode

;;	mov	cx,400
		
;Check if we're a 200 scan line 
	push	ds
	push	ax
	sub	ax,ax
	mov	ds,ax
	test	ds:[fVGAInfo],SCAN_200_ACTIVE	; 200 scan lines?
	pop	ax							; If 200 scan line text mode, use 400 scan line 
	pop	ds							; text mode timings.
	jnz	DoneCalcBIOSXYRes		; Jump if we're a 200 scanline text mode

	mov	cx,350					; We can only be a 350 scanline text mode.
	jmp	DoneCalcBIOSXYRes		; All Done.

Check200YRes:
;This is where you need to check for all those low res modes.
	cmp	cx,200					; Check if we're a 200 scanline grfx mode
	jne	DoneCalcBIOSXYRes		; Jump if we're not a 200 scanline grfx mode
	shl	cx,1						; If 200 scan line grfx, use 400 scanline timings

;	cmp	cx,400					; Check if we're a 200 scanline grfx mode
;	jge	DoneCalcBIOSXYRes		; Jump if we're not a 200 scanline grfx mode
;	mov	cx,400					; Assume we're a 400 scanline text mode

DoneCalcBIOSXYRes:
	mov	ebx,ecx

;--------------------------------------------------------------------------------

	call	FindDetailTimings
  jnc   RemoveCRTCBorders

LoadDetailedTimingBlock:
	call	ParseAndProgramDetailedTiming
	jmp	ExitPanelRoutine				 
;--------------------------------------------------------------------------------
RemoveCRTCBorders:

	call	GetCRTCAddr				 ;Unlock CRTC registers
	mov	al,11h
	call	GetIndexRegister
	and	ah,not 80h
	out	dx,ax

	mov	al,02h					 ;Kill right border
	call	GetIndexRegister
	dec	ah
	out	dx,ax

	mov	al,03h					 ;Kill left border
	call	GetIndexRegister
	inc	ah
	out	dx,ax

	mov	al,12h					 ;Kill bottom border
	call	GetIndexRegister
	mov	al,15h
	out	dx,ax

	mov	al,06h					 ;Kill top border
	call	GetIndexRegister
	mov	al,16h
	inc	ah
	out	dx,ax

EndPanelScalingRoutine:
	jmp	ExitPanelRoutine

;--------------------------------------------------------------------------------

NonScalingPanel:


	mov	cl,3									  	;Size of DVI patch table
	mov	ax,00h									;Pointer to common set of DVI parameters
					
	cmp	bl,07h									;Check if we're mode 07h
	je		PatchMode07	       					;Jump if we're mode 07h
		
	cmp	bl,03h									;Check if we're a color text mode.  Mode 03 or lower
	jg		LoadPtrToTable							;Jump if we're not a color text mode.

PatchTextMode:
;We must be a mode 07,03,02,01,or 00.

	cmp	bl,02h									;Are we mode 02,03, or mode 07
	jl		PatchMode00								;Jump if mode 00,01
	call	CheckIf400Scanline					;check if we're a 400 scanline text
	jz		CalcOffsetIntoTable					;Jump if not 400 scanline mode
	mov	ax,01h									;AX = Offset to 400 scanline patch table mode 02/03 text
	jmp	CalcOffsetIntoTable

PatchMode00:
	call	CheckIf400Scanline				   ;Check if we're a 400 scanline Mode 00/01 text
	jz 	CalcOffsetIntoTable					;Jump if not 400 scanline mode 00/01 text
	mov	ax,02h									;AX = Offset to 400 scanline patch table mode 00/01 text
	jmp	CalcOffsetIntoTable


PatchMode07:
	mov	ax,02h									;AX = Offset to 400 scanline patch table mode 07 text
	jmp	CalcOffsetIntoTable

LoadPtrToTable:
	movzx	ax,byte ptr es:[si].EXTREGS.xrDVIIdx	;Get index into DVI Patch table

	cmp	al,0FFh									;Is the mode supported
	je		ExitPanelRoutine						;Jump if mode not supported

CalcOffsetIntoTable:

	mul	cl											;Calculate offset into patch table

	push	cx
	push	bx

	call	GetPanelSize
	cmp	bx,1024

	pop	bx
	pop	cx

	je		Load1024PatchTable

  mov	di,offset DVI_Patch_Table_00_1280		;Get pointer to first patch table
	jmp	short AddLcdPatchOffset

Load1024PatchTable:
  mov	di,offset DVI_Patch_Table_00_1024		;Get pointer to first patch table

AddLcdPatchOffset:
	add	di,ax												;Added in offset into patch table

;DI = pointer to patch table to use for current mode

	;Force hardware to use external clk
	mov	dx,MISC_INPUT
	in		al,dx
	mov	dx,MISC_OUTPUT
	or		al,0Ch
	out	dx,al

	;Disable VGA /2 clock
	mov	dx,SEQ_INDEX
	mov	al,01h
	call	GetIndexRegister
	and	ah,not 08h
	out	dx,ax

	;Unlock CRTC registers
	call	GetCRTCAddr
	mov	al,11h
	call	GetIndexRegister
	and	ah,not 80h
	out	dx,ax

	;Program Video clock for a 1024x768 60Hz mode.
	call	GetIOBase			; DX = IO base
	push	dx
	add	dx,REG_VIDCLOCK
	mov	eax,06B07h			;1024x768 clock

	push	cx
	push	bx

	call	GetPanelSize
	cmp	bx,1024

	pop	bx
	pop	cx

	je		ProgramLCDDclk

	mov	eax,0B312h			;1280x1024 clock

ProgramLCDDclk:
	out	dx,eax
	pop	dx

;-------------------------------------------------------

	push	bx
	call	GetPanelSize
	cmp	bx,1280
	pop	bx
	je		CheckForSkipPatch1280x1024Modes

;-------------------------------------------------------
;If we're a 1024x768 mode, we don't need to patch the mode
	cmp	bl,05Eh			;Are we a 1024x768 mode
	je		ExitPanelRoutine
	cmp	bl,072h			;Are we a 1024x768 mode
	je		ExitPanelRoutine
	cmp	bl,073h			;Are we a 1024x768 mode
	je		ExitPanelRoutine
	jmp	ContinuePatchForPanelMode
;-------------------------------------------------------

CheckForSkipPatch1280x1024Modes:

;If we're a 1280x1024 mode, we don't need to patch the mode
	cmp	bl,058h			;Are we a 1280x1024 mode
	je		ExitPanelRoutine
	cmp	bl,074h			;Are we a 1280x1024 mode
	je		ExitPanelRoutine
	cmp	bl,075h			;Are we a 1280x1024 mode
	je		ExitPanelRoutine
	
;-------------------------------------------------------
ContinuePatchForPanelMode:

	;See if Vertical timings X2 is enabled
	add	dx,REG_VIDPROCCFG
	in		eax,dx
	test	al,010h
	jz		Program_CR00	;Jump if not a Vertical X2 mode

;We must be a vertical X2 mode.

	and	al,not 010h		;Clear the Vertical X2 bit
	out	dx,eax

;Since we disabled VerticalX2 mode, we MUST divide the current vertical display time by 2.

;Read the current vertical display time and patch it with the correct value
	call	GetCRTCAddr
	mov 	al,012h
	call	GetIndexRegister

	cmp	ah,057h			;Are we a 400x300 mode
	je		Patch400x300	;Jump if we are

	cmp	ah,08Fh			;Are we a 320x200 mode
	je		Patch320x200	;Jump if we are

	cmp	ah,0FFh			;Are we a 512x384 mode
	je		Patch512x384	;Jump if we are

;We must be a 320x240 mode.

Patch320x240:
	mov	ah,0EFh			;CR12 patch for 320x240
	jmp	Program_CR12

Patch512x384:
	mov	ah,07Fh			;CR12 patch for 512x384
	jmp	Program_CR12

Patch320x200:
	mov	ah,0C7h			;CR12 patch for 320x200
	jmp	Program_CR12

Patch400x300:
	mov	ah,02bh			;CR12 patch for 400x300

Program_CR12:
	out	dx,ax				;Write new CR12 value

Program_CR00:
	call	GetCRTCAddr
	mov	al,00h
	mov	ah,byte ptr es:[di].LCD_CR.xrDVICR00
	out	dx,ax				;Write CR00 value

	mov	al,06h
	mov	ah,byte ptr es:[di].LCD_CR.xrDVICR06
	out	dx,ax				;Write CR06 value

	mov	al,07h
	mov	ah,byte ptr es:[di].LCD_CR.xrDVICR07
	out	dx,ax				;Write CR07 value

GetPointerToPanelParms:
	call	GetPanelSize
	cmp	bx,1024
	je		Load1024PanelValue

Load1280PanelValue:
	add	si, EXTREGS.xrDVICR04_1280

	mov	ax,011bh
	out	dx,ax

	jmp	short LCDProgramPanel

Load1024PanelValue:
	;Load pointer to 1024x768 panel timings
	add	si, EXTREGS.xrDVICR04_1024

LCDProgramPanel:
	mov	ah, es:[si].LCDREGS.xrDVICR04
	mov	al,04h
	out	dx,ax				;Write CR04 value

	mov	ah, es:[si].LCDREGS.xrDVICR05
	mov	al,05h
	out	dx,ax				;Write CR05 value

	mov	ah, es:[si].LCDREGS.xrDVICR10
	mov	al,10h
	out	dx,ax				;Write CR10 value

	mov	ah, es:[si].LCDREGS.xrDVICR11
	mov	al,11h
	out	dx,ax

	mov	ah, es:[si].LCDREGS.xrDVICR02
	mov	al,02h
	out	dx,ax

	mov	ah, es:[si].LCDREGS.xrDVICR03
	mov	al,03h
	out	dx,ax

	mov	ah, es:[si].LCDREGS.xrDVICR15
	mov	al,15h
	out	dx,ax

	mov	ah, es:[si].LCDREGS.xrDVICR16
	mov	al,16h
	out	dx,ax

	mov	al,09h
	call	GetIndexRegister
	or		ah,20h
	out	dx,ax				;Write CR09 value


	mov	al,07h
	call	GetIndexRegister
	and	ah,not 08h
	or		ah,01h
	out	dx,ax				;Write CR07 value

	mov	al,11h
	call	GetIndexRegister
	or		ah,80h
	out	dx,ax				;Write CR00 value

	mov	dx,SEQ_INDEX
	mov	al,011h
	call	GetIndexRegister
	or		ah,01h
	out	dx,ax				;Write CR00 value

ExitPanelRoutine:
  pop	ax
  pop	bx
	pop	cx
  pop	dx
  pop	di
  pop	si
  pop	es
  pop	ds

        ret
FixupPanel		endp

;**********************************************************************
;Parses the panels detailed timing field to find a particular mode
;Entry:	EBX[31:16] = XRes 
;			EBX[15:00] = YRes
;Exit:	DI = Pointer to detail timing in panel
;			Carry flag set if mode is found

FindDetailTimings		proc	near
;This is the check of code where I parse the panel EDID block
	mov	cx,04						;There are only 4 detailed timing blocks in 
										;EDID

	mov	di,36h					;Point to the first timing block.  This the 
										;	native mode of the panel.

GetNextDetailTiming:
	call	GetDetailTimingXYRes
;	AX - XRes	DX - YRes
	shl	eax,16
	mov	ax,dx

;	EAX[31:16] XRes EAX[15:00] YRes

	cmp	eax,ebx					;Did we find the mode??
	je		FoundDetailedTiming	;Jump if we did

LoadNextDetailedTiming:
	add	di,18						;Move pointer to next detailed timing block
	dec	cx
	jnz	GetNextDetailTiming	;If we haven't hit the end, look at next block.

;We must of not been able to find the detailed timing
	clc
	ret

FoundDetailedTiming:
	stc								;Return success by setting the carry flag.
	ret
FindDetailTimings		endp

;**********************************************************************
;GetDetailTimingXYRes gets the panels horizontal and vertical active time.
;Entry:	DI - pointer to EDID detailed timing block to parse
;Exit:	AX - XRes
;			DX - YRes
GetDetailTimingXYRes	proc	near
	push	bx
	push	cx

;Grab the EDID Horizontal active time
	xor	eax,eax

	mov	cx,1						;Read the high nibble of the horizontal 
	mov	si,di						;  active time.
	add	si,04h
	call	DFPReadByte
	and	bl,0F0h
	shr	bl,4

	mov	cx,1						;Read the low byte of the horizontal 
	mov	si,di						;  active time 
	add	si,02h
	call	DFPReadByte

	mov	ax,bx						; AX = horizontal active time (XRes)

;----------------------------------------------------------------------

	mov	cx,1						;Read the upper nibble of vertical active
	mov	si,di
	add	si,07h
	call	DFPReadByte
	and	bl,0F0h
	shr	bl,4

	mov	cx,1						;Read the lower byte of vertical active
	mov	si,di
	add	si,05h
	call	DFPReadByte

	mov	dx,bx
	; DX = vertical active time (YRes)


	pop	cx
	pop	bx
	ret
GetDetailTimingXYRes	endp
;**********************************************************************
;The routine calculates and programs the CRTC parameters using the 
;DFP EDID data.
;Entry	: DX = Pointer to EDID detailed timings.
;Exit	: nothing.

ParseAndProgramDetailedTiming	proc	near


;Unlock CRTC register

	mov	dx,SEQ_INDEX			;Force card into 8 bit font mode
	mov	al,001h
	call	GetIndexRegister
	or		ah,001h
	out	dx,ax

	call	GetCRTCAddr				;Unlock the CRTC registers
	mov	al,11h
	call	GetIndexRegister
	and	ah,not 80h
	out	dx,ax

;*******************************************************

;Grab the EDID pixel clock
	xor	eax,eax

	mov	cx,1						;Read the high byte of the pixel
	mov	si,di						;  clock frequency
	inc	si
	call	DFPReadByte
	mov	al,bl
	shl	eax,8

	mov	cx,1
	mov	si,di

	push	dx							;Read the low byte of the pixel
	call	DFPReadByte				;  clock frequency
	mov	al,bl
	mov	ecx,10000
	mul	ecx
	pop	dx

;EAX has the desired pixel clock.
;Now call the BIOS routine which programs the pixel clock
	call	SetAbsoluteDotClock

;*******************************************************
;Grab the EDID Horizontal active time

	call	GetDetailTimingXYRes

;Patch 720 modes to be actually 640 modes
	cmp	ax,720
	jne	ContProgramingCR01andCR02

	mov	ax,640
ContProgramingCR01andCR02:

	call	DoWePatchVGAmodes
	jnc	@F
	shr	ax,1
@@:

	shl	ax,05h					;Shift horizontal active time into the upper
										;  byte of AX.
	sub	ax,100h					;Subtract one from the result.

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++
;CR01 and CR02 - Horizontal Display Time and Blank Start

	call	GetCRTCAddr

	call	IsHalvedExtMode		;If we're a halved mode.  Skip programming CR01.
	jc		SkipProgrammingCR01

	mov	al,01						
	out	dx,ax						;Program horizontal display end

SkipProgrammingCR01:
	push	ax
	call	PatchHalvedModesHCRTC

	mov	al,02
	out	dx,ax						;Program horisontal blank start
	pop	ax

;*******************************************************
;Grab the EDID Horizontal blank time


	mov	cx,1						;Read the high nibble of horizontal blank time
	mov	si,di
	add	si,04h
	call	DFPReadByte
	and	bl,00Fh

	mov	cx,1
	mov	si,di
	add	si,03h
	call	DFPReadByte				;Read the low byte of the horizontal blank time

	call	DoWePatchVGAmodes
	jnc	@F
	shr	bx,1
@@:

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++
;CR00 and CR03 - Horizontal Total Time and Blank End

	push	ax
	call	GetCRTCAddr

	shl	bx,05h					;Divide total time by 8 and shift the total 
										;  time into the upper byte of BX.

	add	ax,bx						;Added horizontal display time and blank time
										;  to get horizontal total time



	push	ax
	call	PatchHalvedModesHCRTC

	mov	al,03h

	push	ax
	and	ah,1Fh
	or		ah,80h
	out	dx,ax						;Program CR03, Horizontal Blank End.
	pop	ax

	push	ax
	and	ah,20h
	mov	cl,ah						;Bit 5 of horizontal blank end overflows into
	shl	cl,02h					;  CR05[7]

	mov	al,05h
	call	GetIndexRegister
	and	ah,not 80h
	or		ah,cl
	out	dx,ax						;Set CR05[7], if needed.

	pop	ax
	pop	ax


  sub	ah,4h						;Horizontal total is 5 less then the actual 
										;  value.  Blank end is already subtracted by 1.
										;  Now subtract by 4.
	mov	al,00h
	out	dx,ax						;Program horizontal total.

	pop	ax
;*******************************************************
;Grab the EDID Horizontal sync offset

	mov	cx,1						;Read the upper 2 bits of the horizontal sync
	mov	si,di						;  offset.
	add	si,0Bh
	call	DFPReadByte
	and	bl,0C0h
	shr	bl,06h

	mov	cx,1						;Read the low byte of the horizontal sync
	mov	si,di						;  offset.
	add	si,08h
	call	DFPReadByte

	call	DoWePatchVGAmodes
	jnc	@F
	shr	bx,1
@@:

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++
;CR04 - Horizontal Sync Start

	call	GetCRTCAddr

	shl	bx,05h					;Divide sync start by 8 and shift the sync
										;  start into the upper byte of BX.
	add	ah,bh						;Added sync start with display time to get
										;  the beginning of horizontal sync

	add	ah,01

	call	PatchHalvedModesHCRTC

	mov	al,04h

	out	dx,ax						;Program CR04, horizontal sync start.

;*******************************************************
;For some reason, the value that we calculate for CR05, sync end, causes
;the panel to go out of sync, even thou we calculate the correct value.
;For now, I'll leave it alone.

;Grab the EDID Horizontal sync time
IF 0

	mov	cx,1						;Read the upper 2 bits of the horizontal sync
	mov	si,di						;  time.
	add	si,0Bh
	call	DFPReadByte
	and	bl,030h
	mov	al,bl
	shr	eax,4
										
	mov	cx,1						;Read the low byte of the horizontal sync
	mov	si,di						;  time.
	add	si,09h
	call	DFPReadByte

	call	DoWePatchVGAmodes
	jnc	@F
	shr	bx,1
@@:

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++
;CR05 - Horizontal Sync End

	call	GetCRTCAddr
	
	shl	bx,05h					;Divide sync time by 8 and shift the sync
										;  time into the upper byte of BX.
	add	ah,bh						;Add the sync offset with sync time to get
										;  sync end.

	mov	al,05h					;Program sync end into CR05
	out	dx,ax

ELSE

	call	IsHalvedExtMode		;Check if we're a havled mode
	jnc	SkipPatchCR05			;Skip patch CR05

	call	GetCRTCAddr				;DX = CRTC base address
	mov	ax,01005h
	out	dx,ax						;Write new value.

;	mov	al,05h					;Get current value of CR05
;	call	GetIndexRegister
;	or		ah,0Bh					;Slam dunk bits [04:00]=1Fh
;	or		ah,1Fh					;Slam dunk bits [04:00]=1Fh
;	out	dx,ax						;Write new value.

SkipPatchCR05:

ENDIF

;*******************************************************
;Grab the EDID Vertical active time

	call	GetDetailTimingXYRes
	mov	bx,dx						;Save YRes in BX

	mov	ax,bx						;Save display time in AX

	dec	bx							;Vertical display time is one less than 
										;  expected

	call	PatchHalvedModesVCRTC

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++
;CR12 and CR15 - Vertical Dispaly Time and Blank Start

	push	ax
	push	bx
	call	GetCRTCAddr

	call	IsHalvedExtMode
	jnc	ProgramVerticalDisplayTime

;	shr	bx,1

	push	ds
	xor	ax,ax
	mov	ds,ax
	mov	al,ds:[byMode]
	call	GetModeDescPtr			; Get pointer to mode description
	pop	ds

	mov	bx,cs:[bx].MODEDESC.mdYRes
	dec	bx

ProgramVerticalDisplayTime:
;-------------------------------------------------------
;	BX = Vertical display time
; SI = Vertical blank start
;-------------------------------------------------------
;Check vertical display time bit 10
	mov	al,1Bh
	call	GetIndexRegister
;;	and	ah,not 10h				;Clear bits 4 and 3
	and	ah,not 18h				;Clear bits 4 and 3
	test	bx,400h

	jz		CheckVeticalBlankStart
	or		ah,08h					;Set vertical display overflow bit 10
CheckVeticalBlankStart:
	test	si,400h
	jz		ProgramBit10
	or		ah,10h					;Set blank start overflow bit 10
ProgramBit10:
	out	dx,ax						;Program overflow bits

;;	jz		ProgramBit10
;;	or		ah,18h				;Set vertical display and blank start
;;									;  overflow bit 10
;;ProgramBit10:
;;	out	dx,ax					;Program overflow bits

;-------------------------------------------------------
;Check bit 09	and 08
	mov	al,09h
	call	GetIndexRegister
	and	ah,not 20h
	test	si,200h					;Test bit 9 of blank start
	jz		ProgramBit09_CR09
	or		ah,20h					;Set vertical blank start overflow bit 9
ProgramBit09_CR09:
	out	dx,ax						;Program overflow bits

	mov	al,07h
	call	GetIndexRegister
;;	and	ah,not 08h				;Clear bits 6,3, and 1
	and	ah,not 4Ah				;Clear bits 6,3, and 1
	test	bx,200h					;Test vertical display end bit 9 
	jz		CheckBit8
	or		ah,40h					;Set vertical display overflow bit 9

CheckBit8:
	test	bx,100h					;Test bit 8 vertical display end
	jz		CheckBit8VertBlankEnd
	or		ah,02h					;Set vertical display overflow bit 8
CheckBit8VertBlankEnd:
	test	si,100h					;Test bit 8 vertical blank start
	jz		ProgramBit9and8_CR07
	or		ah,08h					;Set vertical blank start overflow bit 8

;;	jz		ProgramBit9and8_CR07
;;
;;	or		ah,0Ah					;Set vertical display and blank start
;;										;  overflow bit 8

ProgramBit9and8_CR07:	
	out	dx,ax						;Program overflow bits
;-------------------------------------------------------
;Now write bits 7:0
	mov	ah,bl
	mov	al,12h
	out	dx,ax						;Program vertical display time, CR12

	mov	ax,si
	shl	ax,08

	mov	al,15h						
	out	dx,ax						;Program vertical blank start, CR15

;-------------------------------------------------------
	pop	bx
	pop	ax
;*******************************************************
;Grab the EDID Vertical blank time

	mov	cx,1						;Read upper nibble of vertical blank time
	mov	si,di
	add	si,07h
	call	DFPReadByte
	and	bl,00Fh

	mov	cx,1		  				;Read lower byte of the vertical blank time
	mov	si,di
	add	si,06h
	call	DFPReadByte

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++

;CR00 and CR16 - Vertical Total Time and Blank end
	push	ax

	add	bx,ax						;Added vertical display and blank time
										; to get vertical total.

	call	PatchHalvedModesVCRTC

;-------------------------------------------------------
;	BX = Vertical total time
; SI = Vertical blank end
;-------------------------------------------------------

	call	GetCRTCAddr
;-------------------------------------------------------
;Check bit 10
	mov	al,1Bh
	call	GetIndexRegister
	and	ah,not 01h				;Clear the original vertical total bit 10 

	test	bx,400h					;Test bit 10
	jz		Vtotal_Bit10

	or		ah,01h					;Set vertical total bit 10.
Vtotal_Bit10:
	out	dx,ax						;Program overflow bit 10.

;-------------------------------------------------------
;Check bit 09 and 08
	mov	al,07h
	call	GetIndexRegister
	and	ah,not 021h				;Clear vertical total overflow bits 09 and 08

	test	bx,200h					;Test bit 09
	jz		Vtotal_Bit09

	or		ah,020h					;Set bit 09

Vtotal_Bit09:
	test	bx,100h					;Test bit 08
	jz		Vtotal_Bit08

	or		ah,001h					;Set bit 08

Vtotal_Bit08:
	out	dx,ax						;Write vertical total overflow bits 09 and 08

	mov	ah,bl

	sub	ah,02						;The value programmed for vertical overflow 
										; is two less then actual.
	mov	al,06
	out	dx,ax						;Program vertical total

	mov	ax,si
	shl	ax,08h

	dec	ah							;The value programmed for vertical blank end
										; is one less then the actual vertical total.

;;	inc	ah							;The value programmed for vertical blank end
;;										; is one less then the actual vertical total.

	mov	al,16h
	out	dx,ax						;Program vertical blank end.
;-------------------------------------------------------
	pop	ax

;*******************************************************
;Grab the EDID Vertical sync offset time
	xor	ecx,ecx

	mov	cx,1						;Read the upper two bits of
	mov	si,di						;the vertical sync offset
	add	si,0Bh
	call	DFPReadByte
	and	bl,00Ch

	mov	cl,bl
	shl	cl,2

	push	cx

	mov	cx,1						;Read the lower 4 bytes of the 
	mov	si,di						;vertical sync offset
	add	si,0Ah
	call	DFPReadByte
	and	bl,0F0h
	shr	bl,04h

	pop	cx

	xor 	bh,bh
	or		bl,cl						;BX = sync offset

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++
;CR10 - Vertical Sync Start

;-------------------------------------------------------
	call	GetCRTCAddr

	add	bx,ax						;Add vertical display time with sync offset to
										;  get sync start

	call	PatchHalvedModesVCRTC
	mov	bx,si

;Check bit 09 and 08
	mov	al,07h
	call	GetIndexRegister
	and	ah,not 084h				;Clear overflow bits 9 and 8

	test	bx,200h					;Test bit 09
	jz		Vsyncstart_Bit09

	or		ah,080h					;Set bit 09

Vsyncstart_Bit09:
	test	bx,100h					;Test bit 08h
	jz		Vsyncstart_Bit08

	or		ah,004h					;Set bit 08

Vsyncstart_Bit08:
	out	dx,ax						;Write bit 09 and 08

	mov	ah,bl
	mov	al,10h
	out	dx,ax						;Program sync start

;----------------------------------------------------------------------

;Grab the EDID Vertical sync time
	xor	ecx,ecx

	mov	cx,1						;Read the upper 2 bits of vertical sync time
	mov	si,di
	add	si,0Bh
	call	DFPReadByte
	and	bl,003h
	mov	cl,bl
	shl	ecx,4

	push	cx

	mov	cx,1						;Read the lower 4 bits of vertical sync time
	mov	si,di
	add	si,0Ah
	call	DFPReadByte
	and	bl,00Fh

	pop	cx

	or		bl,cl						;BX =  vertical sync time

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++
;CR11 - Vertical Sync End

	call	GetCRTCAddr

	add	bl,ah						;Added vertical sync start and time to get
										;  sync end.

	and	bl,0Fh 					;We only need to program the lower nibble
	mov	al,11h
	call	GetIndexRegister
	and	ah,0F0h					;Clear old vertical sync end value
	or		ah,bl						;Create new CR11 by ORing in new vertical sync 
										;  end value.

	or		ah,80h					;Let's be good little programmers and lock the
										;  the CRTC value.

	out	dx,ax						;Program new CR11 value.

;----------------------------------------------------------------------

	ret

ParseAndProgramDetailedTiming	endp
;**********************************************************************
;Patches the horizontal CRTC timings for halved modes only
;Formula for this is:
;		((Desktop Res - Display Res) / 8 ) / 2
;e.g. ((640 - 320) / 8) / 2 = 20 = 14h

;Entry:	AH[7:4] = Horizontal timing parameter
;Exit:	AH[7:4] = Adjusted Horizontal timing parameter

PatchHalvedModesHCRTC		proc	near

;	call	IsHalvedExtMode		;If we're a halved mode.  Skip programming CR01.
;	jnc 	EndPatchHalvedModesHCRTC

	call	Is320x200ExtMode					;Are we 320x200
	jc		Patch320HCRTC						;Jump is we are 320x200
	call	Is320x240ExtMode					;Are we 320x240
	jc		Patch320HCRTC						;Jump is we are 320x240

	call	Is512x384ExtMode					;Are we 512x384
	jc		Patch512HCRTC						;Jump is we are 512x384

	call	Is400x300ExtMode					;Are we 400x300
	jnc	EndPatchHalvedModesHCRTC		;Jump is we are NOT 400x300

Patch400HCRTC:
	sub	ah,0Fh
	jmp	EndPatchHalvedModesHCRTC

Patch512HCRTC:
	sub	ah,08h
	jmp	EndPatchHalvedModesHCRTC

Patch320HCRTC:
	sub	ah,14h

EndPatchHalvedModesHCRTC:
	ret
PatchHalvedModesHCRTC		endp
;**********************************************************************
;Patches the vertical CRTC timings for halved modes only
;Entry:	BX = Horizontal timing parameter
;Exit:	SI = Adjusted Horizontal timing parameter

PatchHalvedModesVCRTC		proc	near

	mov	si,bx

;	call	IsHalvedExtMode		;If we're a halved mode.  Skip programming CR01.
;	jnc 	EndPatchHalvedModesVCRTC
;
;	sub	si,64h


	call	Is320x200ExtMode					;Are we 320x200
	jc		Patch200VCRTC						;Jump is we are 320x200

	call	Is320x240ExtMode					;Are we 320x240
	jc		Patch240VCRTC						;Jump is we are 320x240

	call	Is512x384ExtMode					;Are we 512x384
	jc		Patch384VCRTC						;Jump is we are 512x384

	call	Is400x300ExtMode					;Are we 400x300
	jnc	EndPatchHalvedModesVCRTC		;Jump is we are NOT 400x300

Patch300VCRTC:
	sub	si,32h
	jmp	EndPatchHalvedModesVCRTC

Patch384VCRTC:
	sub	si,08h
	jmp	EndPatchHalvedModesVCRTC

Patch240VCRTC:
	sub	si,50h
	jmp	EndPatchHalvedModesVCRTC

Patch200VCRTC:
	sub	si,64h

EndPatchHalvedModesVCRTC:
	ret
PatchHalvedModesVCRTC		endp
;**********************************************************************

ProgramCRTCPanelTimings	proc	near

	push	dx
	push	ax

	call	GetCRTCAddr				 ;Unlock CRTC registers

	mov	al,11h
	call	GetIndexRegister
	and	ah,not 80h
	out	dx,ax

	mov	al,00h
	mov	ah,cs:[si].CRTC.HTotal
	out	dx,ax

	mov	al,02h
	mov	ah,cs:[si].CRTC.HBlankStart
	out	dx,ax

	mov	al,03h
	mov	ah,cs:[si].CRTC.HBlankEnd
	out	dx,ax

	mov	al,04h
	mov	ah,cs:[si].CRTC.HSyncStart
	out	dx,ax

	mov	al,05h
	mov	ah,cs:[si].CRTC.HSyncEnd
	out	dx,ax

	mov	al,06h
	mov	ah,cs:[si].CRTC.VTotal
	out	dx,ax

	mov	al,12h
	mov	ah,cs:[si].CRTC.VDispEnd
	out	dx,ax

	mov	al,07h
	mov	ah,cs:[si].CRTC.Overflow
	out	dx,ax

	mov	al,09h
	mov	ah,cs:[si].CRTC.CellHeight
	out	dx,ax

	mov	al,10h
	mov	ah,cs:[si].CRTC.VSyncStart
	out	dx,ax

	mov	al,11h
	mov	ah,cs:[si].CRTC.VSyncEnd
	out	dx,ax

	mov	al,15h
	mov	ah,cs:[si].CRTC.VBlankStart
	out	dx,ax

	mov	al,16h
	mov	ah,cs:[si].CRTC.VBlankEnd
	out	dx,ax

	mov	al,11h
	call	GetIndexRegister
	or		ah,80h
	out	dx,ax

	pop	ax
	pop	dx
	ret
ProgramCRTCPanelTimings	endp

;**********************************************************************
;This check if the mode being set on the panel is a special mode
;where the CRTC parameters have to be halved for it to work on the panel.
;Currently VGA modes 00, 01, 04, 05, and 0Dh have to be patched.

DoWePatchVGAmodes	proc		near
	push	ax
	push	ds
	xor	ax,ax
	mov	ds,ax
	mov	al,ds:[byMode]
	cmp	al,00h
	je		PatchThisMode
	cmp	al,01h
	je		PatchThisMode
	cmp	al,04h
	je		PatchThisMode
	cmp	al,05h
	je		PatchThisMode
	cmp	al,0Dh
	je		PatchThisMode

	clc
	jmp	DoneDoWePatchVGAmodes

PatchThisMode:
	stc

DoneDoWePatchVGAmodes:
	pop	ds
	pop	ax

	ret
DoWePatchVGAmodes	endp

;**********************************************************************
;This check if the mode being set on the panel is a extended halved mode
;where the CRTC01, Horizontal display time, remains constant. 
;Halved modes are 320x200, 320x240, 400x300x and 512x384.

IsHalvedExtMode	proc		near
	push	ax
	push	ds
	xor	ax,ax
	mov	ds,ax
	mov	al,ds:[byMode]

	call	Is320x200ExtMode
	jc		PatchThisHalvedExtendedMode
	call	Is320x240ExtMode
	jc		PatchThisHalvedExtendedMode
	call	Is400x300ExtMode
	jc		PatchThisHalvedExtendedMode
	call	Is512x384ExtMode
	jc		PatchThisHalvedExtendedMode

	clc
	jmp	DoneDoWeHalvedExtModes

PatchThisHalvedExtendedMode:
	stc

DoneDoWeHalvedExtModes:
	pop	ds
	pop	ax

	ret
IsHalvedExtMode	endp
;----------------------------------------------------------------------
;Check if the current mode is 320x200.  If so, sets carry flag.
;Entry:	None.
;Exit:	Carry flag set if 320x200.

Is320x200ExtMode	proc	near
	push	ax
	push	ds
	xor	ax,ax
	mov	ds,ax
	mov	al,ds:[byMode]

;Are we 320x200
	cmp	al,078h									;Are we 320x200x8
	je		Found320x200ExtMode
	cmp	al,079h									;Are we 320x200x16
	je		Found320x200ExtMode
	cmp	al,07Ah									;Are we 320x200x24
	je		Found320x200ExtMode

	clc
	jmp	EndIs320x200ExtMode

Found320x200ExtMode:
	stc

EndIs320x200ExtMode:
	pop	ds
	pop	ax
	ret
Is320x200ExtMode	endp

;----------------------------------------------------------------------
;Check if the current mode is 320x240.  If so, sets carry flag.
;Entry:	None.
;Exit:	Carry flag set if 320x240.

Is320x240ExtMode	proc	near
	push	ax
	push	ds
	xor	ax,ax
	mov	ds,ax
	mov	al,ds:[byMode]

;Are we 320x240
	cmp	al,020h									;Are we 320x240x8
	je		Found320x240ExtMode
	cmp	al,021h									;Are we 320x240x16
	je		Found320x240ExtMode
	cmp	al,022h									;Are we 320x240x24
	je		Found320x240ExtMode

	clc
	jmp	EndIs320x240ExtMode

Found320x240ExtMode:
	stc

EndIs320x240ExtMode:
	pop	ds
	pop	ax
	ret
Is320x240ExtMode	endp

;----------------------------------------------------------------------
;Check if the current mode is 400x300.  If so, sets carry flag.
;Entry:	None.
;Exit:	Carry flag set if 400x300.

Is400x300ExtMode	proc	near
	push	ax
	push	ds
	xor	ax,ax
	mov	ds,ax
	mov	al,ds:[byMode]

;Are we 400x300
	cmp	al,023h									;Are we 400x300x8
	je		Found400x300ExtMode
	cmp	al,024h									;Are we 400x300x16
	je		Found400x300ExtMode
	cmp	al,025h									;Are we 400x300x24
	je		Found400x300ExtMode

	clc
	jmp	EndIs400x300ExtMode

Found400x300ExtMode:
	stc

EndIs400x300ExtMode:
	pop	ds
	pop	ax
	ret
Is400x300ExtMode	endp

;----------------------------------------------------------------------
;Check if the current mode is 512x384.  If so, sets carry flag.
;Entry:	None.
;Exit:	Carry flag set if 512x384.

Is512x384ExtMode	proc	near
	push	ax
	push	ds
	xor	ax,ax
	mov	ds,ax
	mov	al,ds:[byMode]

;Are we 512x384
	cmp	al,026h									;Are we 512x384x8
	je		Found512x384ExtMode
	cmp	al,027h									;Are we 512x384x16
	je		Found512x384ExtMode
	cmp	al,028h									;Are we 512x384x24
	je		Found512x384ExtMode

	clc
	jmp	EndIs512x384ExtMode

Found512x384ExtMode:
	stc

EndIs512x384ExtMode:
	pop	ds
	pop	ax
	ret
Is512x384ExtMode	endp

;**********************************************************************
;This routine checks if we're a 400 scanline text mode.  
;If 400 scanline text, zero flag cleared.
	public	CheckIf400Scanline
CheckIf400Scanline   proc    near
        push    ds
        xor     ax,ax
        mov     ds,ax
        test    byte ptr ds:[fVGAInfo],SCAN_400_ACTIVE      ;Check if we're 400 scanline text mode
        pop     ds
        ret
CheckIf400Scanline   endp

;**********************************************************************

;This routines returns the size of the panel.  Looks at Scratch_Reg2[2:1] 
;to determine panel size.
;00 = 1024x768
;01 = 1280x104
;10 = 1600x1200
;
;Entry:		None
;Exit:		BX = Panel Width
;				CX = Panel Height
	public	GetPanelSize
GetPanelSize			proc		near
	push	dx
	push	ax

	call	GetCRTCAddr
	mov 	al,SCRATCH_REG3
	call	GetIndexRegister
	and	ah,not CFG_DFP_SIZE_MASK

;1600x1200 panel???
	mov	bx,1600
	mov	cx,1200
	cmp	ah,CFG_DFP_SIZE_1600
	je		EndGetPanelSize
	
;1280x1024 panel???
	mov	bx,1280
	mov	cx,1024
	cmp	ah,CFG_DFP_SIZE_1280
	je		EndGetPanelSize

;Can only be a 1024x768 panel
	mov	bx,1024
	mov	cx,768

EndGetPanelSize:

	pop	ax
	pop	dx
	ret
GetPanelSize			endp
;**********************************************************************
;This routine checks if panel side scaling is supported.  If so, zero
;flag cleared.
IsPanelSideScaling	proc	near
	push	ax
	push	dx

	call	GetCRTCAddr
	mov	al,SCRATCH_REG3
	call	GetIndexRegister

	test	ah,FLD_PANELSCALING	;Test ScratchReg2[3] to see if 
										;panel side scaling

	pop	dx
	pop	ax

	ret
IsPanelSideScaling	endp

;**********************************************************************
;This routine checks Scratch register 3[0] to see if a panel was detected
;during POST.  Will return zero flag = 0 if panel detected
;Entry: NONE
;Exit : Zero flag cleared if panel attached
	public	IsPanelAttached
IsPanelAttached         proc    near
	push	ax
	push	dx

	call	GetCRTCAddr
	mov	al,SCRATCH_REG3
	call	GetIndexRegister

	test    ah,FLD_DVI  

	pop     dx
	pop     ax

	ret
IsPanelAttached         endp
;**********************************************************************

ENDIF

IF TVOUT_BIOS	  
;
;
;	I2CReadAuxEEPROM - Read the Auxiliary EEPROM TV out data
;	Designed for ATMEL 24C01A not the 24C01
;
;	Entry:	DX = I/O address of I2C register
;
;	Exit:	AL = Byte from EEPROM 
;
	public	I2CReadAuxEEPROM
I2CReadAuxEEPROM	proc
	assume	ds:nothing, es:nothing
	push	ecx

	mov	cx,5h
RetryThis:
	dec	cx
	jz		UltimateFail

	call	I2CStart            ; Send a start on the I2C
; The 24C01A requires a dummy write to setup the read address
	mov	al, 0A0h			; Required write cycle to set address
	call	I2CSendByte			; Send 24C01A address
	jnc	Reed1
	call	I2CStop				; Failed, try again at top
	jmp	RetryThis	
Reed1:
	mov	al, ah				; Set Address to 0 (dummy write)
	call	I2CSendByte
	jnc	Reed2
	call	I2CStop				; Failed, try again at top
	jmp	RetryThis	
Reed2:

	call	I2CStart            ; Send another start (required for read)

	mov	al, 0A1h			; Read at 0
	call	I2CSendByte
	jnc	Reed3
	call	I2CStop				; Failed, try again at top
	jmp	RetryThis	
Reed3:							; If here we should get the data now
  call  I2CReadBus1Byte	; Get the BYTE
UltimateFail:
	call	I2CStop				;  * This reset sequence *

	pop	ecx
	ret
I2CReadAuxEEPROM	endp

;
;	I2CReadBus1Byte - Read a BYTE of data from SDA1
;
;	Entry:	DX = I/O address of I2C register
;	Exit:	AL = BYTE Read from I2C Bus1
;
	public	I2CReadBus1Byte
I2CReadBus1Byte	proc
	assume	ds:nothing, es:nothing
	push	ecx

    xor		ah,ah			; Clear AH for storage 
	mov		cx,7			; Eight bits per byte
	call	I2CClock1		; Clock it
I2CBus1_10:
	call	I2CDataReadBit	; Read state of the data bit
    or      ah,al
    shl     ah,1			; Shift for the next bit
	call	I2CClock0		; Data set on negative edge of CLK
	call	I2CClock1		; Clock it
	loop	I2CBus1_10		; Loop till 7 bits

	call	I2CDataReadBit	; Read state of the data bit
    or      ah,al			; Last bit, don't shift it
	call	I2CClock0		
	call	I2CClock1		; Ignore the Ack
	xchg	al,ah

	pop		ecx
	ret
I2CReadBus1Byte	endp

ENDIF

IF DVI_BIOS
;
;	I2CReadBusXBytes - Read multiple BYTEs of data from SDA1
;
;	Entry:	DX = I/O address of I2C register
;			CX = ACK or NACK Flag (0 = NACK)
;	Exit:	AL = BYTE Read from I2C Bus1
;
	public	I2CReadBusXBytes
I2CReadBusXBytes	proc
	assume	ds:nothing, es:nothing
	push	ecx

  xor		ah,ah			; Clear AH for storage 
	mov		cx,7			; Eight bits per byte
	call	I2CClock1		; Clock it
I2CBus1_100:
	call	I2CDataReadBit	; Read state of the data bit
  or      ah,al
  shl     ah,1			; Shift for the next bit
	call	I2CClock0		; Data set on negative edge of CLK
	call	I2CClock1		; Clock it
	loop	I2CBus1_100		; Loop till 7 bits

	call	I2CDataReadBit	; Read state of the data bit
    or      ah,al			; Last bit, don't shift it
	call	I2CClock0		
	pop	ecx
	push	ecx
	cmp	cx,1
	je	SendNACK
	call	I2CData0		; If CX != 0 then more bytes to read
	call	I2CDelay		; and send an ACK
	call	I2CClock1
	call	I2CDelay
	call	I2CClock0
	call	I2CDelay
	call	I2CData1
	call	I2CDelay
	jmp	SentACK
SendNACK:
	call	I2CData1		; If CX = 0 then last byte to read
	call	I2CDelay		; and send a NACK
	call	I2CClock1
	call	I2CDelay
SentACK:
	xchg	al,ah

	pop	ecx
	ret
I2CReadBusXBytes	endp
ENDIF

;**********************************************************************
;GetSSID - This routine goes out to the end of ROM and return
; the BIOS PCI subdevice and subvendor ID.
;Entry : NONE
;Exit : EAX[15:00] = PCI Subvendor ID
;			EAX[31:16] = PCI Subdevice ID
;
GetSSID				proc	near
	push	ebx
	push	ds
	push	di

	mov	ax,0C000h
	mov	ds,ax

;Get the ROM size from the PCI block in the ROM
 	mov	di,word ptr ds:[18h]
	add	di,10h
;	mov	di, offset tblPCIInfo+10h

	movzx	eax,word ptr ds:[di]
;	movzx	eax,byte ptr ds:[0002h]

	mov	ebx,512
	mul	ebx
	xchg	eax,ebx
	dec	ebx
	mov	eax,dword ptr ds:[ebx-07h]

	pop	di
	pop	ds
	pop	ebx
	ret
GetSSID				endp
	
;**********************************************************************
;Read_And_Shadow_IO_Base - This routine goes out and reads the current
; I/O Base address from PCI configuration offset 18h.  This address is then
; copied into scratch register 0, CRTC 1Ch. This routine is primarily used
; by VBE_DDC in VESA.ASM.
;
;Entry : None
;Exit : None
;**********************************************************************
	public	Read_And_Shadow_IO_Base

Read_And_Shadow_IO_Base	proc	near
	pusha

	mov	ax,0B102h									; Find PCI device
	sub	si,si
	mov	cx,PCI_ID_DEVICE
	mov	dx,PCI_ID
	call	SafeInt1A									; Check INT 1Ah for existance first
	jc	short Read_And_Shadow_IO_Base_Error		;  Error? Y: Exit with error
	test	ah,ah											; Completion successful?
	jnz	short Read_And_Shadow_IO_Base_Error	;  N: Exit with error
;
	mov	ax,0B10Ah									; Read config DWORD
	mov	di,018h										; Memory Base Address
	int	1Ah
	test	ah,ah											; Function passed?
	jnz	short Read_And_Shadow_IO_Base_Error	;  N: Exit with error
	mov	ah,ch	
	mov	al,SCRATCH_REG0  							;Shadow the I/O base into CR1C.
	call	GetCRTCAddr
	out	dx,ax

Read_And_Shadow_IO_Base_Error:

	popa
	ret
Read_And_Shadow_IO_Base	endp
;**********************************************************************
;Find3DFXCard - Goes out and find the active 3Dfx card.
;Entry : none
;Exit :	SI - Index number of device
;       Carry set if function failed.
;			Carry cleared if function passed

	public	Find3DFXCard
Find3DFXCard		proc	near
	sub	si,si				;Clear index number

ScanFor3DFXcard:
	mov	ax,0B102h		; Find PCI device
	mov	cx,PCI_ID_DEVICE
	mov	dx,PCI_ID
	call	SafeInt1A		; Check INT 1Ah for existance first
	jc	short Find3DFXCard_error	;  Error? Y: Exit with error
	test	ah,ah			; Completion successful?
	jnz	short Find3DFXCard_error	;  N: Exit with error
;

  mov   ax,cs
  cmp   ax,0C000h
  jne   FindGetIOBase

	mov	ax,0B10Ah			; Read config DWORD
	mov	di,02Ch				; PCI Subvendor and Subdevice
	int	1Ah
	test	ah,ah					; Function passed?
	jnz	Find3DFXCard_error

;CX = PCI Subvendor ID

	call	GetSSID

;;	call	GetSubvendorID		;Get Subvendor ID from ROM
	cmp	ax,cx					;Compare ID in ROM against one in PCI space
	je		CheckSubdeviceID	;Jump if identical
	inc	si						;Search for next device
	jmp	short ScanFor3DFXcard

CheckSubdeviceID:
	shr	ecx,16
	shr	eax,16
	
;CX = PCI Subdevice ID
;;	call	GetSubdeviceID		;Get Subdevice ID from ROM
	cmp	ax,cx					;Compare ID in ROM against one in PCI space
	je		CheckCardIsEnable	;Jump if identical
	inc	si						;Search for next device
	jmp	short ScanFor3DFXcard

CheckCardIsEnable:
;OK, now we check if the card is actually enabled

	mov	ax,0B108h			; Read config DWORD
	mov	di,004h				; PCI Subvedor and Subdevice
	int	1Ah
	test	ah,ah					; Function passed?
	jnz	short Find3DFXCard_error	;  N: Exit with error

	and	cl,03h				;Check bits 0 and 1
	cmp	cl,03h				;Is memory and I/O active??
	je		FindGetIOBase
	inc	si						;Search for next device
	jmp	short ScanFor3DFXcard

FindGetIOBase:
	clc
	ret

Find3DFXCard_error:	
	stc
	ret
Find3DFXCard		endp
;**********************************************************************
;CheckIfSGRAM - Returns if we're a SDRAM or SGRAM board
;This is done by reading DRAMInit1[30].
;SGRAM = DRAMInit1[30]=0
;SDRAM = DRAMInit1[30]=1
;
;Entry: NONE
;Exit:  Z = SGRAM
;			NZ = SDRAM

	public	CheckIfSGRAM
CheckIfSGRAM		proc	near
	test	dword ptr cs:[fBoardConfig],01h
	ret
CheckIfSGRAM		endp
;**********************************************************************

IF (DDR_BIOS OR SDR_BIOS)
;**********************************************************************
;CheckIfDDR - Returns if we're a DDR board
;Check fBoardConfig[1:0]
;Entry: NONE
;Exit:  Z = DDR
;			NZ = Not DDR

	public	CheckIfDDR
CheckIfDDR		proc	near
	push	ax
	mov	al,byte ptr cs:[fBoardConfig]
	and	al,03h
	cmp	al,02h
	pop	ax
	ret
CheckIfDDR		endp
;**********************************************************************
;CheckIfSDR - Returns if we're a SDR board
;Check fBoardConfig[1:0]
;Entry: NONE
;Exit:  Z = SDR
;			NZ = Not SDR

	public	CheckIfSDR
CheckIfSDR		proc	near
	push	ax
	mov	al,byte ptr cs:[fBoardConfig]
	and	al,03h
	cmp	al,03h
	pop	ax
	ret
CheckIfSDR		endp
ENDIF

IF DVI_BIOS

;**********************************************************************
;Read a specified number of bytes (up to 4 bytes) from the EDID block 
;  in the panel.
;Entry:	CX = Number of bytes to read from panel
;		   SI = Address of where to read the bytes from
;Exit:	EBX = Data read from specified location

	public	DFPReadByte
DFPReadByte		proc		near
	push	eax
	push	dx

;----------------------------------------------------------------------

	cmp	cx,04h					;We can only read 4 bytes at a time.
	jg		ErrorI2CReadByte		;Greater than 4 bytes.  EJECT! EJECT!! EJECT!!!

	mov	ax,si						; Start at address 0

;----------------------------------------------------------------------
	call	DDCEnableDFP			;Enable port to DFP
	jz	DDCStartupSequence_exit1

StartReadEDID1:
	call	DDCStart					; Send start sequence
	jz	DDCStartupSequence_exit1
	push	ax
	mov	al,0A0h					; Send "write" command
	call	DDCWrite
	pop	ax			; ** DOESN'T AFFECT FLAGS **
	jz	DDCStartupSequence_exit1
	call	DDCWrite					; AL = Start address
	jz	DDCStartupSequence_exit1
	call	DDCStart
	mov	al,0A1h					; Send "read" command
	call	DDCWrite
DDCStartupSequence_exit1:
	jz		ErrorI2CReadByte

;----------------------------------------------------------------------

DFP_DDCReadEDID_20:
	dec	cx							;Decreament count
	call	DDCRead					; Read byte at specified address
	jz		ErrorI2CReadByte

	shl	ebx,8						;Shift over previously read byte
	mov	bl,al						;Save new read byte

	or		cx,cx						;Have we read the last byte??
	jz		StopReadingBytes		;Jump if we have.

	jmp	DFP_DDCReadEDID_20	;Read next byte

StopReadingBytes:
;If we get here, we're all done.

	call	DDCStop
	call	DDCDisable

	pop	dx
	pop	eax
	clc								;Return Sucess
	ret

ErrorI2CReadByte:
	pop	dx
	pop	eax
	stc
	ret								;Return failure
DFPReadByte		endp
ENDIF
;**********************************************************************

;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


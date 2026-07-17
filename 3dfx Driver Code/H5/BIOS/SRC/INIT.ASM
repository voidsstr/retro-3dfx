	page	57, 132
	title	Initialization and Power On Self Test (POST) Routines
	subttl	INIT.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	INIT.ASM - Initialization routines
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	InitBIOS		Entry point to the BIOS POST routines
;	InitInfoVars		Initialize the info variables
;	IsAdapterPresent	Test to see if a co-resident adapter is here
;	ForceCRTCIO		Force I/O addressing to either a mono or color CRTC
;	MonitorDetect		Detect a color versus monochrome monitor
;	TriggerDetect		Return the state of the analog comparator
;
;	$Revision: 4 $
;	$Date: 6/16/99 1:28p $
;	$Logfile: /devel/h5/BIOS/SRC/init.asm $
;	$Log: /devel/h5/BIOS/SRC/init.asm $
;; 
;; 4     6/16/99 1:28p Kaymann
;; Moved the patch for the 32K ROM BIOS size into the routine Patch32KROM
;; in OEMPOST.ASM.
;; 
;; 3     6/09/99 10:53a Kaymann
;; In the InitBIOS routine, add a conditional compile option around the
;; 32K ROM size patch to compile it into a ROM binary and not a RAM
;; binary.
;; Fixes bug with 32K TSR hanging while loading.
;; 
;; 2     5/11/99 2:44p Kaymann
;; For the 32K ROM only, added code to patch the ROM size to 32K at
;; C000:0002 and the PCI Info block.  This patch isn't executed if
;; the BIOS detects that the ROM size at C000;0002 is set to 64K.
;; This is to work around an issue on the Compaq 5240 machine.
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: INIT.ASM
;
.MODEL TINY
.CODE
.386
;
DEFAULT_TRIGGER		equ		013h	; Monitor sense trigger
;
	extrn	HandlerINT10:near
IF (VARIATION_002 eq 0)
	extrn	HandlerINT6D:near
ENDIF
IFDEF DEBUG
	extrn	HandlerINT42:near
ENDIF
	extrn	GetIndexRegister:near
	extrn	OEMInitWakeUp:near
	extrn	OEMInit:near
IF (VARIATION_001 eq 0)
	extrn	FillDAC:near
ENDIF
	extrn	ErrorBeep:near
	extrn	OEMWakeUpVGA:near
	extrn	OEMSleepVGA:near
	extrn	OEMSignOn:near
	extrn	WaitVerticalRetrace:near
	extrn	WaitNotVerticalRetrace:near
	extrn	GetCRTCAddr:near
	extrn	WaitBlank:near
	extrn	WaitNotBlank:near
	extrn	ResetATCFlipFlop:near
IF (VARIATION_001 eq 0)
	extrn	OEMNoMonitor:near
ENDIF
;
	extrn	tblSavePtr:byte
	extrn	tblFont8x8:byte
	extrn	szSignonMsg:byte
	extrn	LENGTH_SIGNON_MSG:abs
	extrn	wOEMFrameCount:word
	extrn	byOEMSignOnColor:byte
;
;	InitBIOS - Entry point to the BIOS POST routines
;
;	Entry:	DS = Seg0
;		ES = ROM segment
;	Exit:	None
;
;	AX, BX, CX, DX, BP, and ES are trashed
;
	public	InitBIOS
InitBIOS	proc
	assume	ds:Seg0, es:nothing
;
; Initialize vectors and global BIOS variables
;
	cli
	mov	[offINT10],offset HandlerINT10
	mov	[segINT10],cs
	mov	[offINT1F],offset tblFont8x8 + 128*8
	mov	[segINT1F],cs
IFDEF DEBUG
	mov	[offINT42],offset HandlerINT42
	mov	[segINT42],cs
ELSE
	mov	[offINT42],0F065h
	mov	[segINT42],0F000h
ENDIF
	mov	[offINT43],offset tblFont8x8
	mov	[segINT43],cs
IF (VARIATION_002 eq 0)
	mov	[offINT6D],offset HandlerINT6D
	mov	[segINT6D],cs
ENDIF
	mov	word ptr [lpSavePtr],offset tblSavePtr
	mov	word ptr [lpSavePtr+2],cs
	sti
;
	call	OEMInitWakeUp			; Initialize chip / OEM config
	call	InitInfoVars			; Returns AX = Mode to set
	call	OEMInit
IF (VARIATION_001 eq 0)
	call	MonitorDetect
ENDIF
	int	10h

IFE	DisableCopyrightMsg
	mov	ax,01301h
	sub	bh,bh
	mov	bl,cs:[byOEMSignOnColor]
	mov	cx,LENGTH_SIGNON_MSG
	sub	dx,dx
	mov	bp,cs
	mov	es,bp
	mov	bp,offset szSignonMsg
	int	10h
	call	OEMSignOn
;
	mov	cx,cs:[wOEMFrameCount]		; Wait a number of frames
InitBIOS_10:
	call	WaitNotVerticalRetrace
	call	WaitVerticalRetrace
	loop	InitBIOS_10
ENDIF

;
	ret
InitBIOS	endp
;
;	InitInfoVars - Initialize the info variables
;
;	Entry:	DS = Seg0
;	Exit:	AX = Mode # to set
;
	public	InitInfoVars
InitInfoVars	proc
	assume	ds:Seg0, es:nothing
	push	dx
	mov	[fEGAInfo],060h			; Assume VGA only
	mov	[fEGAInfo3],09h
	mov	[fVGAInfo],011h
	mov	[byDCC],0Bh
	mov	al,[fEquip]
	and	al,not 030h
	or	al,020h
	mov	[fEquip],al
;
; Test for monochrome adapter
;
	mov	al,001h				; Set CRTC to color
	call	ForceCRTCIO
	mov	dx,CRTC_MINDEX
	call	IsAdapterPresent
	jne	InitInfoVars_10
	and	[fVGAInfo],not 01h
	mov	[byDCC],0Ch
;
; Init MDA adapter
;
	push	word ptr [fEquip]
	or	[fEquip],030h

;Do a set mode 7 without clearing the screen.
	mov	ax,0087h
	int	10h
	pop	word ptr [fEquip]
;
	mov	ax,3
	jmp	short InitInfoVars_exit
;
; Test for color adapter
;
InitInfoVars_10:
	sub	al,al				; Set CRTC to mono
	call	ForceCRTCIO
	mov	dx,CRTC_CINDEX
	call	IsAdapterPresent
	jne	InitInfoVars_20
	mov	al,[fVGAInfo]
	and	al,not 01h
	or	al,6
	mov	[fVGAInfo],al
	mov	[fEGAInfo],062h
;
; Init CGA adapter
;

;Do a set mode 3 without clearing the screen.
	mov	ax,0083h
	int	10h
;
	or	[fEquip],030h
	mov	ax,0007h
	jmp	short InitInfoVars_exit
;
InitInfoVars_20:
	mov	ax,0003h
;
InitInfoVars_exit:
	pop	dx
	ret
InitInfoVars	endp
;
;	IsAdapterPresent - Test to see if a co-resident adapter is here
;
;	Entry:	DX = CRTC port to test
;	Exit:	Zero flag set as follows:
;		  ZF = Adapter is present
;		  NZ = Adapter is not present
;
;	AX is trashed.
;
	public	IsAdapterPresent
IsAdapterPresent	proc
	assume	ds:Seg0, es:nothing
	mov	ax,0AA0Fh
	out	dx,ax
	call	ResetATCFlipFlop	; Clear floating values from bus
	call	GetIndexRegister
	cmp	ah,0AAh
	jne	IsAdapterPresent_exit
	mov	ax,02A0Eh
	out	dx,ax
	call	ResetATCFlipFlop	; Clear floating values from bus
	call	GetIndexRegister
	cmp	ah,02Ah
	jne	IsAdapterPresent_exit
	mov	ax,0550Fh
	out	dx,ax
	call	ResetATCFlipFlop	; Clear floating values from bus
	call	GetIndexRegister
	cmp	ah,055h
	jne	IsAdapterPresent_exit
	sub	ah,ah
	out	dx,ax			; Restore CRTC[0F] to 00h
	mov	ax,0150Eh
	out	dx,ax
	call	ResetATCFlipFlop	; Clear floating values from bus
	call	GetIndexRegister
	cmp	ah,015h
	mov	ah,00h			; ** DOESN'T AFFECT FLAGS **
	out	dx,ax			; ** DOESN'T AFFECT FLAGS ** Restore CRTC[0E] to 00h
IsAdapterPresent_exit:
	ret
IsAdapterPresent	endp
;
;	ForceCRTCIO - Force I/O addressing to either a mono or color CRTC
;
;	Entry:	AL = Color / mono setting (1 = Color, 0 = Mono)
;	Exit:	None
;
	public	ForceCRTCIO
ForceCRTCIO	proc
	push	ax
	push	dx
;
	mov	ah,al
	mov	dx,MISC_INPUT
	in	al,dx
	and	ax,001FEh
	or	al,ah
	mov	dl,low MISC_OUTPUT
	out	dx,al
;
	pop	dx
	pop	ax
	ret
ForceCRTCIO	endp
;
IF (VARIATION_001 eq 0)
;
;	MonitorDetect - Detect a color versus monochrome monitor
;
;	Entry:	AX = Mode to set on exit (mode 3 or mode 7)
;		DS = Seg0
;	Exit:	None
;
;	All registers are preserved.
;
;	The monitor detection is based on an analog comparator that is
;	triggered above a certain voltage level depending on whether
;	the red, green, or blue lines are connected to the monitor.
;	(Presumably, there is a pull-up resistor at the other end?)
;
;	Regardless, by determining the level at which each of the
;	red, green, and blue lines trigger the comparator (read back
;	at 3C2h, bit 4), we can make the following determinations:
;
;		Color Monitor	Mono Monitor	No Monitor
;	Red	Connect		No Connect	No Connect
;	Green	Connect		Connect		No Connect
;	Blue	Connect		No Connect	No Connect
;
;	If a connection is made, then the comparator will trigger when
;	the RAMDAC data is above 10h (typically in the 1Ch - 1Eh range).
;	If no connection is made, then the comparator will trigger when
;	the RAMDAC data is below 10h (typically in the 09h - 0Bh range).
;
;	Note that the polarity of 3C2.4 is inverted (1 = no trigger).
;
;	Upon entry to this routine, the BIOS variables are set to a
;	color monitor. Therefore, only the determination of a monochrome
;	monitor is necessary. Testing of the blue line will be skipped,
;	with the assumption that a test of the red line oughta be enough
;	to determine whether a color monitor is attached.
;	
	public	MonitorDetect
MonitorDetect	proc
	assume	ds:Seg0, es:nothing
	pusha

;Do a setmode without clearing the screen.
	or		al,80h
	int	10h				; Set text mode
;
; Find the red trigger point
;
	mov	bx,00808h
	mov	ah,008h
MonitorDetect_10:
	call	FillDAC
	call	TriggerDetect
	jz	MonitorDetect_20
	inc	ah
	cmp	ah,03Fh
	jb	MonitorDetect_10
;
; Error, no trigger was detected.
;
	jmp	short MonitorDetect_error
;
; Find the green trigger point
;
MonitorDetect_20:
	push	ax				; Save the red trigger
	mov	ah,008h
MonitorDetect_30:
	call	FillDAC
	call	TriggerDetect
	jz	MonitorDetect_40
	inc	bl
	cmp	bl,03Fh
	jb	MonitorDetect_30
;
; Error, no trigger was detected.
;
	pop	ax
	jmp	short MonitorDetect_error
;
; Determine which monitor is attached
;
MonitorDetect_40:
	pop	ax				; AH = Red trigger
	mov	al,bl				; AL = Green trigger
;
	cmp	ah,DEFAULT_TRIGGER		; Red trigger < 10h?
	jb	MonitorDetect_50		;  Y: Not color
	cmp	al,DEFAULT_TRIGGER		; Green trigger < 10h?
	jae	MonitorDetect_exit		;  N: Color found
;
MonitorDetect_50:
	cmp	ah,DEFAULT_TRIGGER		; Red trigger < 10h
	jae	MonitorDetect_60		;  N: Not mono
	cmp	al,DEFAULT_TRIGGER		; Green trigger < 10h
	jb	MonitorDetect_60		;  Y: Not mono
;
; Mono monitor found
;
	or	[fVGAInfo],PALETTE_SUMMING + MONO_ANALOG
	jmp	short MonitorDetect_exit
;
; No monitor found
;
MonitorDetect_60:
  IFE EXTENDED_BIOS
;
; Note: the original IBM BIOS flagged "no monitor" as a monochrome
; monitor. These days, it is highly unlikely that when a monitor is
; eventually attached to the VGA after boot that that monitor will
; be monochrome. It is much, much more likely that the user forgot
; to attach a color monitor before booting the machine and will do
; so soon after booting.
;
; Hence, several OEM requests to remove this "compatibility" feature.
;
	or	[fVGAInfo],PALETTE_SUMMING + MONO_ANALOG
  ENDIF
	call	OEMNoMonitor
;
MonitorDetect_exit:
;We don't need to clear the DAC since there is a  setmode call at the end
;	of this routine.
;   
;	sub	bx,bx				; Clear RAMDAC of test values
;	sub	ah,ah
;	call	FillDAC
	popa
	ret
;
MonitorDetect_error:
	mov	bx,1				; One long
	mov	cx,3				; Three short
	call	ErrorBeep
	jmp	short MonitorDetect_exit
MonitorDetect	endp
;
;	TriggerDetect - Return the state of the analog comparator
;
;	Entry:	None
;	Exit:	Zero flag returned as follows:
;			ZF = Comparator did not trigger
;			NZ = Comparator triggered
;
	public	TriggerDetect
TriggerDetect	proc
	assume	ds:nothing, es:nothing
	push	ax
	push	dx
;
	cli
	call	WaitBlank
	call	WaitNotBlank
	mov	dl,0C2h
	in	al,dx
	sti
	test	al,010h
;
	pop	dx
	pop	ax
	ret
TriggerDetect	endp
;
ENDIF
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


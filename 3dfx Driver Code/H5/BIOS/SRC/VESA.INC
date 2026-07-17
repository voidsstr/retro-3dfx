;
;	VESA.INC - VESA constants, structure and data definitions
;	Copyright (c) 1990-1997 Elpin Systems, Inc.
;	All rights reserved.
;
;	Written by:	Larry Coffey
;	Date:		1/10/94
;
;	$Revision$
;	$Date$
;	$Logfile$
;	$Log$
;
;
; Various VESA BIOS Extension flags
;
; For VBEINFOBLOCK.Capabilities and VBEINFOBLOCK_102.Capabilities:
;
VBE_6BITDAC			equ	000h	; DAC is 6-bits per gun
VBE_8BITDAC			equ	001h	; DAC is switchable to 8-bits per gun
VBE_VGAIO			equ	000h	; Controller has VGA compatible I/O
VBE_NOVGAIO			equ	002h	; Controller doesn't have VGA compatible I/O
VBE_NOBLKDAC			equ	000h	; Normal RAMDAC
VBE_BLKDAC			equ	004h	; RAMDAC must use blanking bit in Function 09h
VBE_NOSTEREO			equ	000h	; No hardware stereoscopic support
VBE_STEREO			equ	008h	; Hardware stereoscopic support
VBE_EXTSTEREO			equ	000h	; Stereo signalling is external
VBE_VESASTEREO			equ	010h	; Stereo signalling uses VESA EVC connector
;
; For VBEMODEINFOBLOCK.ModeAttributes:
;
VBEMA_MODENOSUPPORT		equ	0000h
VBEMA_MODESUPPORT		equ	0001h
VBEMA_TTYNOSUPPORT		equ	0000h
VBEMA_TTYSUPPORT		equ	0004h
VBEMA_MONO			equ	0000h
VBEMA_COLOR			equ	0008h
VBEMA_TEXT			equ	0000h
VBEMA_GRAPHICS			equ	0010h
VBEMA_VGACOMPAT			equ	0000h
VBEMA_VGANOCOMPAT		equ	0020h
VBEMA_VGABANKSUPPORT		equ	0000h
VBEMA_VGANOBANKSUPPORT		equ	0040h
VBEMA_NOLINEAR			equ	0000h
VBEMA_LINEAR			equ	0080h
VBEMA_NODOUBLESCAN		equ	0000h
VBEMA_DOUBLESCAN		equ	0100h
VBEMA_NOINTERLACE		equ	0000h
VBEMA_INTERLACE			equ	0200h
VBEMA_NOHWTRIPLEBUFFER		equ	0000h
VBEMA_HWTRIPLEBUFFER		equ	0400h
VBEMA_NOHWSTEREO		equ	0000h
VBEMA_HWSTEREO			equ	0800h
VBEMA_NODUALDISPLAY		equ	0000h
VBEMA_DUALDISPLAY		equ	1000h
;
; For CRTCINFOBLOCK.Flags:
;
VBECIB_NODOUBLESCAN		equ	0000h
VBECIB_DOUBLESCAN		equ	0001h
VBECIB_NONINTERLACED		equ	0000h
VBECIB_INTERLACED		equ	0002h
VBECIB_HSYNCPOS			equ	0000h
VBECIB_HSYNCNEG			equ	0004h
VBECIB_VSYNCPOS			equ	0000h
VBECIB_VSYNCNEG			equ	0008h
;
; VBEINFOBLOCK data structure (as defined by VBE 3.0 revision 9)
;
VBEINFOBLOCK	struc
	VbeSignature		dd	'ASEV'		; VBE Signature
  IF VESA_300
	VbeVersion		dw	300h		; VBE Version
  ELSE
	VbeVersion		dw	200h		; VBE Version
  ENDIF
	OEMStringPtr		dd	?		; Pointer to OEM string
	Capabilities		db	4 dup (?)	; Capabilities of graphics
	VideoModePtr		dd	?		; Pointer to VBE mode list
	TotalMemory		dw	?		; Number of 64K memory blocks
; VBE 2.0 & 3.0 stuff
	OemSoftwareRev		dw	?		; VBE BIOS Software revision
	OemVendorNamePtr	dd	?		; Pointer to Vendor Name string
	OemProductNamePtr	dd	?		; Pointer to Product Name string
	OemProductRevPtr	dd	?		; Pointer to Product Revision string
	Reserved		db	222 dup (?)	; Reserved
	VBE_OEM_Data		db	256 dup (?)	; Data Scratch for OEM strings
VBEINFOBLOCK	ends
;
VBEINFOBLOCK_102	struc	; Old version of info block structure
	VbeSignature		dd	'ASEV'		; VBE Signature
	VbeVersion		dw	102h		; VBE Version
	OEMStringPtr		dd	?		; Pointer to OEM string (offset)
	Capabilities		dd	?		; Capabilities of graphics
	VideoModePtr		dd	?		; Pointer to VBE mode list (offset)
	TotalMemory		dw	?		; Number of 64K memory blocks
VBEINFOBLOCK_102	ends
;
; VBEMODEINFOBLOCK data structure (as defined by VBE 3.0 revision 10)
;
VBEMODEINFOBLOCK	struc
; Mandatory (all revisions)
	ModeAttributes		dw	?	; Mode attributes
	WinAAttributes		db	?	; Window A attributes
	WinBAttributes		db	?	; Window B attributes
	WinGranularity		dw	?	; Window granularity
	WinSize			dw	?	; Window size
	WinASegment		dw	?	; Window A start segment
	WinBSegment		dw	?	; Window B start segment
	WinFuncPtr		dd	?	; Pointer to window function
	BytesPerScanLine	dw	?	; Bytes per scan line
; Mandatory (VBE 1.20 and above)
	XResolution		dw	?	; Horizontal resolution
	YResolution		dw	?	; Vertical resolution
	XCharSize		db	?	; Character cell width
	YCharSize		db	?	; Character cell height
	NumberOfPlanes		db	?	; Number of memory planes
	BitsPerPixel		db	?	; Bits per pixel
	NumberOfBanks		db	?	; Number of banks
	MemoryModel		db	?	; Memory model type
	BankSize		db	?	; Bank size in KB
	NumberOfImagePages	db	?	; Number of images
	Reserved1		db	1	; Reserved for page function
; Direct color fields (required for direct/6 and YUV/7 models)
	RedMaskSize		db	?	; Size of direct color red mask in bits
	RedFieldPosition	db	?	; Bit position of LSB of red mask
	GreenMaskSize		db	?	; Size of direct color green mask in bits
	GreenFieldPosition	db	?	; Bit position of LSB of green mask
	BlueMaskSize		db	?	; Size of direct color blue mask in bits
	BlueFieldPosition	db	?	; Bit position of LSB of blue mask
	RsvdMaskSize		db	?	; Size of direct color reserved mask in bits
	RsvdFieldPosition	db	?	; Bit position of LSB of reserved mask
	DirectColorModeInfo	db	?	; Direct color mode attributes
; Mandatory information (VBE 2.00 and above)
	PhysBasePtr		dd	?	; Physical address for flat frame buffer
	OffScreenMemPtr		dd	?	; *RSVD in VBE 3.0* Pointer to start of off-screen memory
	OffScreenMemPtrSize	dw	?	; *RSVD in VBE 3.0* Amount of off-screen memory in 1K units
; Mandatory information (VBE 3.0 and above)
	LinBytesPerScanLine	dw	?	; Byte per scan for linear modes
	BnkNumberOfImagePages	db	?	; Number of images for banked modes
	LinNumberOfImagePages	db	?	; Number of images for linear modes
	LinRedMaskSize		db	?	; Size of direct color red mask in bits (linear modes)
	LinRedFieldPosition	db	?	; Bit position of LSB of red mask (linear modes)
	LinGreenMaskSize	db	?	; Size of direct color green mask in bits (linear modes)
	LinGreenFieldPosition	db	?	; Bit position of LSB of green mask (linear modes)
	LinBlueMaskSize		db	?	; Size of direct color blue mask in bits (linear modes)
	LinBlueFieldPosition	db	?	; Bit position of LSB of blue mask (linear modes)
	LinRsvdMaskSize		db	?	; Size of direct color reserved mask in bits (linear modes)
	LinRsvdFieldPosition	db	?	; Bit position of LSB of reserved mask (linear modes)
	MaxPixelClock		dd	?	; Maximum pixel clock (in Hz) for graphics mode
	Reserved2		db	189 dup (?)
VBEMODEINFOBLOCK	ends
;
; CRTCINFOBLOCK data structure (as defined by VBE 3.0 revision 9)
;
CRTCINFOBLOCK	struc
	HorizontalTotal		dw	?	; Horizontal total in pixels
	HorizontalSyncStart	dw	?	; Horizontal sync start in pixels
	HorizontalSyncEnd	dw	?	; Horizontal sync end in pixels
	VerticalTotal		dw	?	; Vertical total in scan lines
	VerticalSyncStart	dw	?	; Vertical sync start in scan lines
	VerticalSyncEnd		dw	?	; Vertical sync end in scan lines
	Flags			db	?	; Flags
	PixelClock		dd	?	; Pixel clock in units of Hz
	RefreshRate		dw	?	; Refresh rate in units of .01 Hz
	Reserved		db	40 dup (?)
CRTCINFOBLOCK	ends
;
;	Copyright (c) 1990-1997 Elpin Systems, Inc.
;	All rights reserved.
;


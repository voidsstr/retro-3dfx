	page	57, 132
	title	Build Statistics
	subttl	BIOSNEWS.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	BIOSNEWS.ASM - Statistics about current build
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	Routines in this file:
;	None
;
;	This module produces absolutely no code and is compiled on every MAKE.
;
;	$Revision: 1 $
;	$Date: 4/30/99 1:52p $
;	$Logfile: /devel/h5/BIOS/SRC/BIOSNEWS.ASM $
;	$Log: /devel/h5/BIOS/SRC/BIOSNEWS.ASM $
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
;
.MODEL TINY
.CODE
.386
;
	%out	!
	%out	!			BIOS News
	%out	!
	%out	!	Version:
;
; By defining "IDENTIFY_BIOSTYPE" and then including OEM.INC, a string
; will be displayed with the OEM name in it.
;
IDENTIFY_BIOSTYPE	equ	1
	include	OEM.INC
;
; Display debug version status
;
IFDEF DEBUG
  	%out	!		Debug Version (only executes as a TSR in RAM)
ELSE
  	%out	!		Non-Debug Version
ENDIF
;
; Display extension status
;
IF EXTENDED_BIOS
  IF EXTENDED_TTY
  	%out	!		Extensions Enabled (with TTY extensions)
    IF EXTENDED_PLANARPAGES
	%out	!			- Planar modes use extended memory
    ELSE
	%out	!			- Planar modes use only VGA memory (256K)
    ENDIF
  ELSE
  	%out	!		Extensions Enabled (with no TTY extensions)
  ENDIF
ELSE
  	%out	!		Extensions Disabled
ENDIF
;
; Display VESA status
;
IF VESA
  	%out	!		VESA BIOS Extensions Enabled
  IF VESA_DPMS
	%out	!			- With Display Power Management Signalling (DPMS)
  ENDIF
  IF VESA_DDC
	%out	!			- With Display Data Channel (DDC)
  ENDIF
ELSE
  	%out	!		VESA BIOS Extensions Disabled
ENDIF
;
; Display ROM segment address
;
IF ROM_SEGMENT eq 0C000h
	%out	!		ROM Segment at 0C000h
ELSE
  IF ROM_SEGMENT eq 0E000h
	%out	!		ROM Segment at 0E000h
  ENDIF
ENDIF
;
; Display monitor detect status
;
IF VARIATION_001
  	%out	!		VARIATION 001: Monitor detection DISABLED (default to color)
ENDIF
;
; Display INT 10h redirection status
;
IF VARIATION_002
  	%out	!		VARIATION 002: INT 10h vector NOT redirected to INT 6Dh
ENDIF
;
; Font fixup status
;
IF VARIATION_003
  	%out	!		VARIATION 003: 9x14 & 9x16 Font fixup code NOT implemented
ENDIF
;
; 9x14 font fixup
;
IF VARIATION_004
  	%out	!		VARIATION 004: 9x14 (only) Font fixup code NOT implemented
ENDIF
;
; RAMDAC Reads & Writes
;
IF VARIATION_005
	%out	!		VARIATION 005: Non-standard RAMDAC handling
ENDIF
;
  	%out	!
;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


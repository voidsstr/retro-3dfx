	page	57, 132
	title	Resident Size Flag
	subttl	BIOSEND.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	BIOSEND.ASM - Resident size flag
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	This file is the very last file linked into the ROM or the TSR image.
;
;	$Revision: 2 $
;	$Date: 5/11/99 2:40p $
;	$Logfile: /devel/h5/BIOS/SRC/BIOSEND.ASM $
;	$Log: /devel/h5/BIOS/SRC/BIOSEND.ASM $
;; 
;; 2     5/11/99 2:40p Kaymann
;; For the 32K ROM only, added 1500 bytes of padding, db 1500 dup (FFh),
;; to push the BIOS POST past the 32K barrier.
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
;
	%out	!	Assembling: BIOSEND.ASM
;
.MODEL TINY
.CODE
.386
;

;
	public	BIOSSIZE
BIOSSIZE	label	byte
IFE ROM_64K
	DB		1500		dup	(0FFh)
ENDIF

;
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


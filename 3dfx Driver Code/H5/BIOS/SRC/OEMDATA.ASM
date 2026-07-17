	page	57, 132
	title	OEM Specific Data Definitions
	subttl	OEMDATA.ASM - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	OEMDATA.ASM - OEM specific data definitions
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	$Revision$
;	$Date$
;	$Log$
;	$Log$
;; 
;; 11    8/04/99 6:42p Kaymann
;; Expanded the tblExtRegisters table for CR02, CR03, CR15, and CR16
;; settings for 1024x768 and 1280x1024 panels.
;; 
;; 10    7/23/99 4:28p Kaymann
;; Renamed all the 18Xh mode numbers to 12Xh.
;; Refer to the document MODELIST.DOC for mode numbers.
;; 
;; 9     7/02/99 4:49p Kaymann
;; Modified TVData Tables to add BT869 register corrections for
;; Sub-carrier
;; frequency adjustment of NTSC and PAL outputs.
;; Voodoo3 PRS#6772
;; 
;; 8     6/24/99 11:10a Kaymann
;; Modifed all the VGA 320x200 TV modes to use the 320x200 TV parameters.
;; Fixes bug with 320x200 VGA modes not being displayed on the TV.
;; 
;; 7     5/21/99 2:53p Kaymann
;; In tblModeDescriptions, changed the internal mode number for modes
;; 00,01,02, and 03, to 00, 01, 02, and 03h.
;; 
;; In tblExtRegisters, adjusted all the 1024x768 panel CR04, CR05, CR10,
;; and CR11 settings to center the image on the panel.
;; 
;; 6     5/17/99 2:09p Kaymann
;; Modified the VBE protected mode function, Set Display Start Address, to
;; also look for active display when checking for VSYNC.  This will
;; guarantee
;; you will find the start of VSYNC.
;; 
;; 5     5/11/99 2:56p Kaymann
;; Modified the compile option for tblExtRegisters table. Removed the TV
;; parameters for the tblExtRegisters table for a non-TV BIOS.
;; 
;; For the 32K BIOS only, modified the PCI Info block to report a 40K
;; BIOS.
;; This is later patched to report 32K by the routine InitBIOS.
;; 
;; 4     5/05/99 7:04p Kaymann
;; Created the strings sz32768K and sz65536K
;; 
;; 3     5/05/99 11:34a Kaymann
;; Created the strings szSDRAM_Memory and szSGRAM_Memory.
;; 
;; 2     5/04/99 6:02p Kaymann
;; Added 1280x1024 panel timings into tblExtRegisters and the
;; XLCD_Patch_Table table.
;; 
;; 1     4/30/99 1:52p Kaymann
;; Initial Version of Napalm BIOS.
;; Based on the Avenger BIOS version 2.11.
;  
	include	BIOSEQU.INC
	include	BIOSVARS.INC
	include	OEM.INC
  include  CLKPLL.INC
;
IF VESA
	include	VESA.INC
ENDIF

	extrn	VitalProductData:byte

;
	%out	!	Assembling: OEMDATA.ASM - 3Dfx Interactive
;
.MODEL TINY
.CODE
.386
;
	public	tblOEMData, segOEMExtRegs, segOEMModeDesc
tblOEMData	label	byte
		dw	tblExtRegisters
segOEMExtRegs	dw	ROM_SEGMENT
		dw	tblModeDescriptions
segOEMModeDesc	dw	ROM_SEGMENT
;
	public	wOEMFrameCount, byOEMSignOnColor
wOEMFrameCount		dw	70*4			; Four seconds
byOEMSignOnColor	db	007h			; White
;
	public	szOEMCopyright, LENGTH_OEMCOPYRIGHT
szOEMCopyright	label	byte
	dw	(LENGTH_OEMCOPYRIGHT - 2)
	db	STR_OEMNAME, 13, 10
LENGTH_OEMCOPYRIGHT	equ	($ - szOEMCopyright)
	db	((MAX_OEMCOPYRIGHT - LENGTH_OEMCOPYRIGHT) + 2) dup (0)
;
IF EXTENDED_BIOS
	public	bySizeEXTREGS
  IFDEF EXTREGS
bySizeEXTREGS		db	size EXTREGS
  ELSE
bySizeEXTREGS		db	0
  ENDIF
ENDIF
;
	public	tblModeDescriptions
tblModeDescriptions	label	byte
;			    mode  imode  vmode  mode type     pages BPP   X     Y  offset height
;	MODEDESC	<000h, 017h, 0000h, MDTYPE_CTEXT, 008h,  4,   40,   25, 80,   16>
;	MODEDESC	<001h, 017h, 0001h, MDTYPE_CTEXT, 008h,  4,   40,   25, 80,   16>
;	MODEDESC	<002h, 018h, 0002h, MDTYPE_CTEXT, 008h,  4,   80,   25, 160,  16>
;	MODEDESC	<003h, 018h, 0003h, MDTYPE_CTEXT, 008h,  4,   80,   25, 160,  16>

	MODEDESC	<000h, 000h, 0000h, MDTYPE_CTEXT, 008h,  4,   40,   25, 80,   16>
	MODEDESC	<001h, 001h, 0001h, MDTYPE_CTEXT, 008h,  4,   40,   25, 80,   16>
	MODEDESC	<002h, 002h, 0002h, MDTYPE_CTEXT, 008h,  4,   80,   25, 160,  16>
	MODEDESC	<003h, 003h, 0003h, MDTYPE_CTEXT, 008h,  4,   80,   25, 160,  16>

	MODEDESC	<004h, 004h, 0004h,   MDTYPE_CGA, 001h,  2,  320,  200, 80,    8>
	MODEDESC	<005h, 005h, 0005h,   MDTYPE_CGA, 001h,  2,  320,  200, 80,    8>
	MODEDESC	<006h, 006h, 0006h,   MDTYPE_CGA, 001h,  1,  640,  200, 80,    8>
	MODEDESC	<007h, 019h, 0007h, MDTYPE_MTEXT, 008h,  0,   80,   25, 160,  16>
	MODEDESC	<00Dh, 00Dh, 000Dh,  MDTYPE_4BPP, 008h,  4,  320,  200, 40,    8>
	MODEDESC	<00Eh, 00Eh, 000Eh,  MDTYPE_4BPP, 004h,  4,  640,  200, 80,    8>
	MODEDESC	<00Fh, 011h, 000Fh, MDTYPE_MGRAF, 002h,  1,  640,  350, 80,   14>
	MODEDESC	<010h, 012h, 0010h,  MDTYPE_4BPP, 002h,  4,  640,  350, 80,   14>
	MODEDESC	<011h, 01Ah, 0011h,  MDTYPE_1BPP, 001h,  1,  640,  480, 80,   16>
	MODEDESC	<012h, 01Bh, 0012h,  MDTYPE_4BPP, 001h,  4,  640,  480, 80,   16>
	MODEDESC	<013h, 01Ch, 0013h,  MDTYPE_8BPP, 001h,  8,  320,  200, 320,   8>
	MODEDESC	<05Bh, 008h, 0100h,  MDTYPE_8BPP, 001h,  8,  640,  400, 640,  16>
	MODEDESC	<05Fh, 009h, 0101h,  MDTYPE_8BPP, 001h,  8,  640,  480, 640,  16>
	MODEDESC	<06Ah, 00Ah, 0102h,  MDTYPE_4BPP, 001h,  4,  800,  600, 100,  16>
	MODEDESC	<05Ch, 00Bh, 0103h,  MDTYPE_8BPP, 001h,  8,  800,  600, 800,  16>
	MODEDESC	<05Eh, 00Fh, 0105h,  MDTYPE_8BPP, 001h,  8, 1024,  768, 1024, 16>
	MODEDESC	<06Bh, 00Ch, 0107h,  MDTYPE_8BPP, 001h,  8, 1280, 1024, 1280, 16>
	MODEDESC	<068h, 010h, 0108h, MDTYPE_CTEXT, 006h,  4,   80,   60, 160,   8>
IFE OPTIMIZE_ROM
	MODEDESC	<055h, 01Dh, 0109h, MDTYPE_CTEXT, 002h,  4,  132,   25, 264,  16>
	MODEDESC	<054h, 01Eh, 010Ah, MDTYPE_CTEXT, 002h,  4,  132,   43, 264,   8>
	MODEDESC	<065h, 01Fh, 010Bh, MDTYPE_CTEXT, 002h,  4,  132,   50, 264,   8>
	MODEDESC	<064h, 020h, 010Ch, MDTYPE_CTEXT, 002h,  4,  132,   60, 264,   8>
ENDIF
IF BANSHEE_DOUBLESCAN
	MODEDESC	<078h, 021h, 0120h,  MDTYPE_8BPP, 001h,  8,  320,  200, 320,   8>
	MODEDESC	<020h, 022h, 0123h,  MDTYPE_8BPP, 001h,  8,  320,  240, 320,   8>
	MODEDESC	<023h, 023h, 0126h,  MDTYPE_8BPP, 001h,  8,  400,  300, 400,   8>
	MODEDESC	<026h, 024h, 0129h,  MDTYPE_8BPP, 001h,  8,  512,  384, 512,   8>
ENDIF
;
; Modes without their own parameter tables must have an internal mode number
; greater than the last mode with a one-to-one link with the parameter tables.
;
IF BANSHEE_DOUBLESCAN
	MODEDESC	<079h, 025h, 010Eh, MDTYPE_16BPP, 001h, 16,  320,  200, 640,   8>
	MODEDESC	<07Ah, 026h, 010Fh, MDTYPE_24BPP, 001h, 24,  320,  200, 1024,  8>
	MODEDESC	<021h, 027h, 0124h, MDTYPE_16BPP, 001h, 16,  320,  240, 640,   8>
	MODEDESC	<022h, 028h, 0125h, MDTYPE_24BPP, 001h, 24,  320,  240, 1024,  8>
	MODEDESC	<024h, 029h, 0127h, MDTYPE_16BPP, 001h, 16,  400,  300, 800,   8>
	MODEDESC	<025h, 02Ah, 0128h, MDTYPE_24BPP, 001h, 24,  400,  300, 2048,  8>
	MODEDESC	<027h, 02Bh, 012Ah, MDTYPE_16BPP, 001h, 16,  512,  384, 1024, 14>
	MODEDESC	<028h, 02Ch, 012Bh, MDTYPE_24BPP, 001h, 24,  512,  384, 2048, 14>
ENDIF
	MODEDESC	<029h, 02Dh, 012Ch, MDTYPE_16BPP, 001h, 16,  640,  400, 1280, 16>
	MODEDESC	<02Ah, 02Eh, 012Dh, MDTYPE_24BPP, 001h, 24,  640,  400, 2048, 16>
	MODEDESC	<06Eh, 02Fh, 0111h, MDTYPE_16BPP, 001h, 16,  640,  480, 1280, 16>
	MODEDESC	<069h, 030h, 0112h, MDTYPE_24BPP, 001h, 24,  640,  480, 2048, 16>
	MODEDESC	<070h, 031h, 0114h, MDTYPE_16BPP, 001h, 16,  800,  600, 1600, 16>
	MODEDESC	<071h, 032h, 0115h, MDTYPE_24BPP, 001h, 24,  800,  600, 4096, 16>
	MODEDESC	<072h, 033h, 0117h, MDTYPE_16BPP, 001h, 16,  1024, 768, 2048, 16>
	MODEDESC	<073h, 034h, 0118h, MDTYPE_24BPP, 001h, 24,  1024, 768, 4096, 16>
	MODEDESC	<074h, 035h, 011Ah, MDTYPE_16BPP, 001h, 16,  1280,1024, 2560, 16>
	MODEDESC	<075h, 036h, 011Bh, MDTYPE_24BPP, 001h, 24,  1280,1024, 4096, 16>
	db	0FFh		; Terminate the table
;
IF EXTENDED_BIOS
;
; The following table is the extended registers set per mode. This table
; is accessed in the same manner as the standard parameter table, i.e.,
; via the "internal mode number". Therefore, an entry MUST exist for every
; mode position in the standard parameter table.
;
; Note that modes without their own parameter tables must have an internal
; mode number greater than the last mode with a one-to-one link to the
; parameter tables.
;
	public	tblExtRegisters
tblExtRegisters	label	byte         
IF	(DVI_BIOS OR TVOUT_BIOS)
;																	 		  1024	1024	1024	1024  1024  1024  1024  1024  1280  1280  1280  1280  1280	1280	1280	1280
;                                     			    			DVI	 DVI	 DVI	 DVI   DVI   DVI   DVI   DVI   DVI   DVI   DVI   DVI   DVI	 DVI	 DVI	 DVI	 
;		      Flags  CR1B  TVOut  Parm  Clock   			DVI  CR04   CR05  CR10  CR11  CR02  CR03  CR15  CR16  CR04  CR05  CR10  CR11  CR02  CR03  CR15  CR16
	EXTREGS	<000h, 000h, 004h , 000h, PLL_VGACLK, 		000h, 054h, 009h, 020h, 005h, 050h, 080h, 060h, 082h, 070h, 001h, 0A0h, 00Eh, 050h, 080h, 060h, 082h>	; 0:  Mode 0
	EXTREGS	<000h, 000h, 004h , 000h, PLL_VGACLK, 		000h, 054h, 001h, 020h, 005h, 050h, 080h, 060h, 082h, 070h, 001h, 0A0h, 00Eh, 050h, 080h, 060h, 082h>	; 1:  Mode 1
	EXTREGS	<000h, 000h, 001h , 000h, PLL_VGACLK, 		000h, 070h, 001h, 030h, 020h, 060h, 08Ch, 046h, 068h, 080h, 010h, 0C0h, 010h, 060h, 08Ch, 046h, 068h>	; 2:  Mode 2
	EXTREGS	<000h, 000h, 001h , 000h, PLL_VGACLK, 		000h, 070h, 001h, 030h, 020h, 060h, 08Ch, 046h, 068h, 080h, 010h, 0C0h, 010h, 060h, 08Ch, 046h, 068h>	; 3:  Mode 3
	EXTREGS	<000h, 000h, 004h , 000h, PLL_VGACLK, 		000h, 058h, 08Ch, 020h, 00Ch, 050h, 01Bh, 03Eh, 060h, 070h, 001h, 0A0h, 001h, 050h, 0BBh, 03Eh, 060h>	; 4:  Mode 4
	EXTREGS	<000h, 000h, 004h , 000h, PLL_VGACLK, 		000h, 058h, 08Ch, 020h, 00Ch, 050h, 01Bh, 03Eh, 060h, 070h, 001h, 0A0h, 001h, 050h, 0BBh, 03Eh, 060h>	; 5:  Mode 5
	EXTREGS	<000h, 000h, 001h , 000h, PLL_VGACLK, 		000h, 071h, 001h, 010h, 00Eh, 060h, 08Ch, 046h, 068h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 046h, 068h>	; 6:  Mode 6
	EXTREGS	<000h, 000h, 000h , 000h, PLL_VGACLK, 		000h, 061h, 014h, 010h, 00Eh, 05Ch, 090h, 040h, 05Ah, 080h, 010h, 0C0h, 001h, 050h, 082h, 020h, 060h>	; 7:  Mode 7
	EXTREGS	<001h, 000h, 001h , 000h, PLL_VGACLK, 		000h, 068h, 01Ch, 030h, 001h, 060h, 08Ch, 04Eh, 070h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 04Eh, 070h>	; 8:  Mode 5B/100
	EXTREGS	<001h, 000h, 002h , 000h, PLL_VGACLK, 		000h, 068h, 01Ch, 070h, 001h, 060h, 08Ch, 067h, 088h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 067h, 088h>	; 9:  Mode 5F/101
	EXTREGS	<000h, 000h, 003h , 000h, PLL_40_000MHZ, 	003h, 074h, 009h, 0A0h, 00Dh, 070h, 098h, 0A0h, 0C1h, 088h, 010h, 0F0h, 000h, 070h, 098h, 0A0h, 0C1h>	; A:  Mode 6A/102
	EXTREGS	<001h, 000h, 003h , 000h, PLL_40_000MHZ, 	003h, 074h, 006h, 0A0h, 00Dh, 070h, 098h, 0A0h, 0C1h, 088h, 010h, 0F0h, 000h, 070h, 098h, 0A0h, 0C1h>	; B:  Mode 5C/103
	EXTREGS	<001h, 051h, 0FFh , 000h, PLL_108_000MHZ, 0FFh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h>	; C:  Mode 6B/107
	EXTREGS	<000h, 000h, 004h , 000h, PLL_VGACLK, 		000h, 058h, 08Ch, 020h, 00Eh, 050h, 01Bh, 03Eh, 060h, 080h, 010h, 0A0h, 00Eh, 050h, 0BBh, 03Eh, 060h>	; D:  Mode D
	EXTREGS	<000h, 000h, 001h , 000h, PLL_VGACLK, 		000h, 071h, 001h, 010h, 00Eh, 060h, 08Ch, 046h, 068h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 046h, 068h>	; E:  Mode E
	EXTREGS	<001h, 000h, 0FFh , 000h, PLL_65_000MHZ, 	004h, 083h, 094h, 003h, 009h, 000h, 000h, 000h, 000h, 098h, 010h, 070h, 000h, 000h, 000h, 000h, 000h>	; F:  Mode 5E/105
	EXTREGS	<000h, 000h, 001h , 000h, PLL_VGACLK, 		0FFh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 080h, 010h, 0A0h, 00Eh, 000h, 000h, 000h, 000h>	; 10: Mode 68/108
	EXTREGS	<000h, 000h, 000h , 000h, PLL_VGACLK, 		000h, 070h, 004h, 020h, 00Fh, 060h, 014h, 046h, 068h, 080h, 010h, 0A0h, 00Eh, 068h, 010h, 046h, 068h>	; 11: Mode F
	EXTREGS	<000h, 000h, 000h , 000h, PLL_VGACLK, 		000h, 071h, 001h, 020h, 00Fh, 068h, 010h, 046h, 068h, 080h, 010h, 0A0h, 00Eh, 068h, 010h, 046h, 068h>	; 12: Mode 10
	EXTREGS	<000h, 000h, 004h , 000h, PLL_VGACLK, 		000h, 054h, 009h, 020h, 005h, 048h, 080h, 008h, 029h, 068h, 010h, 080h, 001h, 048h, 080h, 010h, 029h>	; 13: Mode 0*
	EXTREGS	<000h, 000h, 004h , 000h, PLL_VGACLK, 		000h, 054h, 001h, 020h, 005h, 048h, 080h, 008h, 029h, 068h, 010h, 080h, 001h, 048h, 080h, 010h, 029h>	; 14: Mode 1*
	EXTREGS	<000h, 000h, 000h , 000h, PLL_VGACLK, 		000h, 070h, 001h, 010h, 00Fh, 068h, 010h, 046h, 068h, 080h, 010h, 0C0h, 001h, 068h, 010h, 046h, 068h>	; 15: Mode 2*
	EXTREGS	<000h, 000h, 000h , 000h, PLL_VGACLK, 		000h, 070h, 001h, 010h, 00Fh, 068h, 010h, 046h, 068h, 080h, 010h, 0C0h, 001h, 068h, 010h, 046h, 068h>	; 16: Mode 3*
	EXTREGS	<000h, 000h, 004h , 000h, PLL_VGACLK, 		000h, 054h, 001h, 020h, 005h, 050h, 080h, 056h, 070h, 068h, 010h, 0C0h, 001h, 050h, 080h, 056h, 070h>	; 17: Mode 0/1+
	EXTREGS	<000h, 000h, 001h , 000h, PLL_VGACLK, 		001h, 070h, 009h, 010h, 00Eh, 060h, 08Ch, 015h, 026h, 080h, 010h, 0C0h, 001h, 060h, 08Ch, 015h, 048h>	; 18: Mode 2/3+
	EXTREGS	<000h, 000h, 001h , 000h, PLL_VGACLK, 		000h, 068h, 018h, 010h, 00Eh, 05Ch, 090h, 040h, 05Ah, 080h, 010h, 0C0h, 001h, 050h, 082h, 020h, 060h>	; 19: Mode 7+
	EXTREGS	<000h, 000h, 002h , 000h, PLL_VGACLK, 		000h, 071h, 001h, 050h, 00Fh, 060h, 08Ch, 067h, 088h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 067h, 088h>	; 1A: Mode 11
	EXTREGS	<000h, 000h, 002h , 000h, PLL_VGACLK, 		000h, 071h, 001h, 050h, 00Fh, 060h, 08Ch, 067h, 088h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 067h, 088h>	; 1B: Mode 12
	EXTREGS	<000h, 000h, 001h , 000h, PLL_VGACLK, 		000h, 071h, 001h, 020h, 00Fh, 060h, 08Ch, 04Eh, 070h, 080h, 010h, 070h, 000h, 060h, 08Ch, 04Eh, 070h>	; 1C: Mode 13
	EXTREGS	<000h, 000h, 0FFh , 000h, PLL_40_000MHZ, 	0FFh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h>	; 1D: Mode 55/109
	EXTREGS	<000h, 000h, 0FFh , 000h, PLL_40_000MHZ, 	0FFh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h>	; 1E: Mode 54/10A
	EXTREGS	<000h, 000h, 0FFh , 000h, PLL_40_000MHZ, 	0FFh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h>	; 1F: Mode 65/10B
	EXTREGS	<000h, 000h, 0FFh , 000h, PLL_40_000MHZ, 	0FFh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h>	; 20: Mode 64/10C
	EXTREGS	<005h, 000h, 001h , 000h, PLL_12_500MHZ, 	007h, 054h, 008h, 000h, 001h, 050h, 080h, 000h, 022h, 070h, 010h, 070h, 000h, 050h, 080h, 000h, 022h>	; 21: Mode 78/180
	EXTREGS	<005h, 000h, 002h , 000h, PLL_12_500MHZ, 	007h, 058h, 008h, 000h, 00Ch, 050h, 080h, 000h, 022h, 070h, 010h, 070h, 000h, 050h, 080h, 000h, 022h>	; 22: Mode 20/181
	EXTREGS	<005h, 000h, 0FFh , 000h, PLL_20_000MHZ, 	008h, 060h, 010h, 020h, 00Ch, 050h, 080h, 000h, 022h, 070h, 010h, 080h, 000h, 050h, 080h, 000h, 022h>	; 23: Mode 23/184
	EXTREGS	<005h, 000h, 0FFh , 000h, PLL_32_500MHZ, 	008h, 060h, 014h, 020h, 001h, 050h, 084h, 04Fh, 070h, 070h, 010h, 0A0h, 000h, 050h, 084h, 014h, 070h>	; 24: Mode 26/187
	EXTREGS	<00Dh, 000h, 001h , 021h, PLL_12_500MHZ, 	007h, 054h, 008h, 000h, 001h, 050h, 080h, 000h, 022h, 070h, 010h, 070h, 000h, 050h, 080h, 000h, 022h>	; 25: Mode 79/10E
	EXTREGS	<015h, 000h, 001h , 021h, PLL_12_500MHZ, 	007h, 054h, 008h, 000h, 001h, 050h, 080h, 000h, 022h, 070h, 010h, 070h, 000h, 050h, 080h, 000h, 022h>	; 26: Mode 7A/10F
	EXTREGS	<00Dh, 000h, 002h , 022h, PLL_12_500MHZ, 	007h, 058h, 008h, 000h, 00Ch, 050h, 080h, 000h, 022h, 070h, 010h, 070h, 000h, 050h, 080h, 000h, 022h>	; 27: Mode 21/182
	EXTREGS	<015h, 000h, 002h , 022h, PLL_12_500MHZ, 	007h, 058h, 008h, 000h, 00Ch, 050h, 080h, 000h, 022h, 070h, 010h, 070h, 000h, 050h, 080h, 000h, 022h>	; 28: Mode 22/183
	EXTREGS	<00Dh, 000h, 0FFh , 023h, PLL_20_000MHZ, 	008h, 060h, 010h, 020h, 00Ch, 050h, 080h, 000h, 022h, 070h, 010h, 080h, 000h, 050h, 080h, 000h, 022h>	; 29: Mode 24/185
	EXTREGS	<015h, 000h, 0FFh , 023h, PLL_20_000MHZ, 	008h, 060h, 010h, 020h, 00Ch, 050h, 080h, 000h, 022h, 070h, 010h, 080h, 000h, 050h, 080h, 000h, 022h>	; 2A: Mode 25/186
	EXTREGS	<00Dh, 000h, 0FFh , 024h, PLL_32_500MHZ, 	008h, 060h, 014h, 020h, 001h, 050h, 084h, 04Fh, 070h, 070h, 010h, 0A0h, 000h, 050h, 084h, 014h, 070h>	; 2B: Mode 27/188
	EXTREGS	<015h, 000h, 0FFh , 024h, PLL_32_500MHZ, 	008h, 060h, 014h, 020h, 001h, 050h, 084h, 04Fh, 070h, 070h, 010h, 0A0h, 000h, 050h, 084h, 014h, 070h>	; 2C: Mode 28/189
	EXTREGS	<009h, 000h, 001h , 008h, PLL_VGACLK, 		000h, 068h, 01Ch, 030h, 001h, 060h, 08Ch, 04Eh, 070h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 04Eh, 070h>	; 2D: Mode 29/18A
	EXTREGS	<011h, 000h, 001h , 008h, PLL_VGACLK, 		000h, 068h, 01Ch, 030h, 001h, 060h, 08Ch, 04Eh, 070h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 04Eh, 070h>	; 2E: Mode 2A/18B
	EXTREGS	<009h, 000h, 002h , 009h, PLL_VGACLK, 		000h, 068h, 01Ch, 070h, 001h, 060h, 08Ch, 067h, 088h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 067h, 088h>	; 2F: Mode 6E/111
	EXTREGS	<011h, 000h, 002h , 009h, PLL_VGACLK, 		000h, 068h, 01Ch, 070h, 001h, 060h, 08Ch, 067h, 088h, 080h, 010h, 0A0h, 00Eh, 060h, 08Ch, 067h, 088h>	; 30: Mode 69/112
	EXTREGS	<009h, 000h, 003h , 00Bh, PLL_40_000MHZ, 	003h, 074h, 006h, 0A0h, 00Dh, 070h, 098h, 0A0h, 0C1h, 088h, 010h, 0F0h, 000h, 070h, 098h, 0A0h, 0C1h>	; 31: Mode 70/114
	EXTREGS	<011h, 000h, 003h , 00Bh, PLL_40_000MHZ, 	003h, 074h, 006h, 0A0h, 00Dh, 070h, 098h, 0A0h, 0C1h, 088h, 010h, 0F0h, 000h, 070h, 098h, 0A0h, 0C1h>	; 32: Mode 71/115
	EXTREGS	<009h, 000h, 0FFh , 00Fh, PLL_65_000MHZ, 	004h, 083h, 094h, 003h, 009h, 000h, 000h, 000h, 000h, 098h, 010h, 070h, 000h, 000h, 000h, 000h, 000h>	; 33: Mode 72/117
	EXTREGS	<011h, 000h, 0FFh , 00Fh, PLL_65_000MHZ, 	004h, 083h, 094h, 003h, 009h, 000h, 000h, 000h, 000h, 098h, 010h, 070h, 000h, 000h, 000h, 000h, 000h>	; 34: Mode 73/118
	EXTREGS	<009h, 051h, 0FFh , 00Ch, PLL_108_000MHZ, 0FFh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h>	; 35: Mode 74/11A
	EXTREGS	<011h, 051h, 0FFh , 00Ch, PLL_108_000MHZ, 0FFh, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h>	; 36: Mode 75/11B
;                                   
ELSE
;                             
;		      Flags  CR1B  Parm  Clock
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 0:  Mode 0
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 1:  Mode 1
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 2:  Mode 2
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 3:  Mode 3
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 4:  Mode 4
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 5:  Mode 5
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 6:  Mode 6
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 7:  Mode 7
	EXTREGS	<001h, 000h, 000h, PLL_VGACLK>		; 8:  Mode 5B/100
	EXTREGS	<001h, 000h, 000h, PLL_VGACLK>		; 9:  Mode 5F/101
	EXTREGS	<000h, 000h, 000h, PLL_40_000MHZ>	; A:  Mode 6A/102
	EXTREGS	<001h, 000h, 000h, PLL_40_000MHZ>	; B:  Mode 5C/103
	EXTREGS	<001h, 051h, 000h, PLL_108_000MHZ>	; C:  Mode 6B/107
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; D:  Mode D
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; E:  Mode E
	EXTREGS	<001h, 000h, 000h, PLL_65_000MHZ>	; F:  Mode 5E/105
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 10: Mode 68/108
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 11: Mode F
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 12: Mode 10
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 13: Mode 0*
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 14: Mode 1*
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 15: Mode 2*
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 16: Mode 3*
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 17: Mode 0/1+
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 18: Mode 2/3+
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 19: Mode 7+
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 1A: Mode 11
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 1B: Mode 12
	EXTREGS	<000h, 000h, 000h, PLL_VGACLK>		; 1C: Mode 13
	EXTREGS	<000h, 000h, 000h, PLL_40_000MHZ>	; 1D: Mode 55/109
	EXTREGS	<000h, 000h, 000h, PLL_40_000MHZ>	; 1E: Mode 54/10A
	EXTREGS	<000h, 000h, 000h, PLL_40_000MHZ>	; 1F: Mode 65/10B
	EXTREGS	<000h, 000h, 000h, PLL_40_000MHZ>	; 20: Mode 64/10C
	EXTREGS	<005h, 000h, 000h, PLL_12_500MHZ>	; 21: Mode 78/180
	EXTREGS	<005h, 000h, 000h, PLL_12_500MHZ>	; 22: Mode 20/181
	EXTREGS	<005h, 000h, 000h, PLL_20_000MHZ>	; 23: Mode 23/184
	EXTREGS	<005h, 000h, 000h, PLL_32_500MHZ>	; 24: Mode 26/187
; Modes without parameter tables must go at the end of this table
	EXTREGS	<00Dh, 000h, 021h, PLL_12_500MHZ>	; 25: Mode 79/10E
	EXTREGS	<015h, 000h, 021h, PLL_12_500MHZ>	; 26: Mode 7A/10F
	EXTREGS	<00Dh, 000h, 022h, PLL_12_500MHZ>	; 27: Mode 21/182
	EXTREGS	<015h, 000h, 022h, PLL_12_500MHZ>	; 28: Mode 22/183
	EXTREGS	<00Dh, 000h, 023h, PLL_20_000MHZ>	; 29: Mode 24/185
	EXTREGS	<015h, 000h, 023h, PLL_20_000MHZ>	; 2A: Mode 25/186
	EXTREGS	<00Dh, 000h, 024h, PLL_32_500MHZ>	; 2B: Mode 27/188
	EXTREGS	<015h, 000h, 024h, PLL_32_500MHZ>	; 2C: Mode 28/189
	EXTREGS	<009h, 000h, 008h, PLL_VGACLK>		; 2D: Mode 29/18A
	EXTREGS	<011h, 000h, 008h, PLL_VGACLK>		; 2E: Mode 2A/18B
	EXTREGS	<009h, 000h, 009h, PLL_VGACLK>		; 2F: Mode 6E/111
	EXTREGS	<011h, 000h, 009h, PLL_VGACLK>		; 30: Mode 69/112
	EXTREGS	<009h, 000h, 00Bh, PLL_40_000MHZ>	; 31: Mode 70/114
	EXTREGS	<011h, 000h, 00Bh, PLL_40_000MHZ>	; 32: Mode 71/115
	EXTREGS	<009h, 000h, 00Fh, PLL_65_000MHZ>	; 33: Mode 72/117
	EXTREGS	<011h, 000h, 00Fh, PLL_65_000MHZ>	; 34: Mode 73/118
	EXTREGS	<009h, 051h, 00Ch, PLL_108_000MHZ>	; 35: Mode 74/11A
	EXTREGS	<011h, 051h, 00Ch, PLL_108_000MHZ>	; 36: Mode 75/11B
ENDIF

                                    
IF      TVOUT_BIOS
	public	tblTVDataNTSC
tblTVDataNTSC	label	byte

;             clk    		 cfg   hblk  cr0   cr3   cr4   cr5   cr6   cr7   cr10  BtAE	 BtB0  BtB2	 BtB4
;Use the 640x400 timing for right now, until we get the 640x350 overscan working.
	TVDATA	<PLL_29_189MHZ, 000h, 08Bh, 087h, 080h, 05Dh, 080h, 0A2h, 01Fh, 075h, 03Ch, 0BDh, 081h, 020h>	; 640x350 
	TVDATA	<PLL_29_189MHZ, 000h, 08Ch, 073h, 080h, 069h, 080h, 0E8h, 01Fh, 0B0h, 03Ch, 0BDh, 081h, 020h>	; 640x400
	TVDATA	<PLL_29_189MHZ, 000h, 074h, 05Dh, 080h, 056h, 080h, 056h, 0BAh, 00Ch, 03Ch, 0BDh, 081h, 020h>	; 640x480
	TVDATA	<PLL_38_769MHZ, 002h, 020h, 069h, 080h, 06Dh, 01Bh, 0DDh, 0F8h, 088h, 00Ch, 0A8h, 0A2h, 017h>	; 800x600
	TVDATA	<PLL_29_189MHZ, 000h, 080h, 037h, 090h, 036h, 080h, 0E8h, 01Fh, 0B0h, 03Ch, 0BDh, 081h, 020h>	; 320x200

;
	public	tblTVDataPAL
tblTVDataPAL	label	byte

;             clk    		 cfg   hblk  cr0   cr3   cr4   cr5   cr6   cr7   cr10  BtAE  BtB0  BtB2	 BtB4
	TVDATA	<PLL_29_500MHZ, 001h, 004h, 071h, 080h, 058h, 080h, 06Fh, 0BAh, 016h, 010h, 09Eh, 07Ah, 026h>	; 640x350
	TVDATA	<PLL_29_500MHZ, 001h, 004h, 071h, 080h, 058h, 080h, 06Fh, 03Eh, 0F0h, 010h, 09Eh, 07Ah, 026h>	; 640x400
	TVDATA	<PLL_29_500MHZ, 001h, 004h, 071h, 080h, 058h, 080h, 06Fh, 0BAh, 016h, 010h, 09Eh, 07Ah, 026h>	; 640x480	
  TVDATA	<PLL_36_000MHZ, 003h, 050h, 073h, 096h, 070h, 095h, 0ECh, 0F0h, 08Ch, 088h, 0C2h, 086h, 01Fh>	; 800x600
	TVDATA	<PLL_29_500MHZ, 001h, 004h, 036h, 090h, 02Dh, 080h, 06Fh, 03Eh, 0F0h, 010h, 09Eh, 07Ah, 026h>	; 320x200

IF ROM_64K
  public   tblTVPatchPALN
tblTVPatchPALN   label byte
;              RegA2 RegAE RegB0 RegB2 RegB4
  TVPALPATCH  < 02Eh, 00Ch, 08Ch, 079h, 026h> ;640xXXX PAL N
  TVPALPATCH  < 02Eh, 018h, 028h, 087h, 01Fh> ;800x600 PAL N

  public   tblTVPatchPALM
tblTVPatchPALM   label byte
;              RegA2 RegAE RegB0 RegB2 RegB4
  TVPALPATCH  < 02Ah, 06Eh, 0DBh, 076h, 020h> ;640xXXX PAL M
  TVPALPATCH  < 02Ah, 07Eh, 042h, 09Ch, 017h> ;800x600 PAL M

  public   tblTVPatchPALNc
tblTVPatchPALNc   label byte
;              RegA2 RegAE RegB0 RegB2 RegB4
  TVPALPATCH  < 024h, 01Eh, 0C0h, 015h, 01Fh> ;640xXXX PAL Nc
  TVPALPATCH  < 024h, 035h, 0EFh, 078h, 019h> ;800x600 PAL Nc
ENDIF
ENDIF

;

	public	szSDRAM_Memory, LENGTH_SDRAM_MEMORY
szSDRAM_Memory	db	'Total SDRAM '
LENGTH_SDRAM_MEMORY	equ	($ - szSDRAM_Memory)

	public	szSGRAM_Memory, LENGTH_SGRAM_MEMORY
szSGRAM_Memory	db	'Total SGRAM '
LENGTH_SGRAM_MEMORY	equ	($ - szSGRAM_Memory)

	public	szDDR_Memory, LENGTH_DDR_MEMORY
szDDR_Memory	db	'Total DDR '
LENGTH_DDR_MEMORY	equ	($ - szDDR_Memory)

;
	public	szSDR_Memory, LENGTH_SDR_MEMORY
szSDR_Memory	db	'Total SDR '
LENGTH_SDR_MEMORY	equ	($ - szSDR_Memory)
;
	public	szMemorySize, LENGTH_MEMORYSIZE
szMemorySize	db	'Video Memory Size: '
LENGTH_MEMORYSIZE	equ	($ - szMemorySize)
;
	public	sz4096K, sz8192K, sz16384K, sz32768K,sz65536K,sz131072K
IF	Napalm2_BIOS
	public	sz262144K
ENDIF

sz4096K		db	5,'4096K'
sz8192K		db	5,'8192K'
sz16384K	db	6,'16384K'
sz32768K	db	6,'32768K'
sz65536K	db	6,'65536K'
sz131072K	db	7,'131072K'
IF	Napalm2_BIOS
sz262144K	db 7,'256144K'
ENDIF
;
	public	tblMemSizeStrs
tblMemSizeStrs	label	byte
	dw	4*4
	dw	offset sz4096K
	dw	8*4
	dw	offset sz8192K
	dw	16*4
	dw	offset sz16384K
	dw	32*4
	dw	offset sz32768K
	dw	64*4
	dw	offset sz65536K
	dw	128*4
	dw	offset sz131072K
IF	Napalm2_BIOS
	dw	256*4
	dw	offset sz262144K
ENDIF
	dw	0
;
; The following is the OEM-specific configurable BIOS values.
; Note that the order and makeup of this table is cooperatively
; structured with an OEM configuration utility.
;
	public	tblOEMConfig, regPCIInit0, regMiscInit0, regMiscInit1
	public	regDRAMInit0, regDRAMInit1, regAGPInit0, regPLLCtrl1
	public	regPLLCtrl2, regSGRAMMode
	public	regTVMode
	public	regTMUGBEINIT
	public	fBoardConfig

	OEMConfigVer	dw		03h
tblOEMConfig	label	byte
regPCIInit0		dd	DEFAULT_PCIINIT0		; IOBase[04h]
regMiscInit0		dd	DEFAULT_MISCINIT0		; IOBase[10h]
regMiscInit1		dd	DEFAULT_MISCINIT1		; IOBase[14h]
regDRAMInit0		dd	DEFAULT_DRAMINIT0		; IOBase[18h]
regDRAMInit1		dd	DEFAULT_DRAMINIT1		; IOBase[1Ch]
regAGPInit0		dd	DEFAULT_AGPINIT0		; IOBase[20h]
regPLLCtrl1		dd	DEFAULT_PLLCTRL1		; IOBase[44h]
regPLLCtrl2		dd	DEFAULT_PLLCTRL2		; IOBase[48h]
regSGRAMMode		dd	DEFAULT_SGRAMMODE		; IOBase[30h][10Dh]
tblPanelTable_Common_Reg        dw      0
tblPanelTable_Common            dw      0
tblPanelTable_Mode_Reg          dw      0
tblPanelTable_Mode              dw      0

IF      TVOUT_BIOS
  IF PAL_BIOS
     regTVMode   db FLD_PALMODE
  ELSE
     regTVMode   db FLD_NTSCMODE
  ENDIF
ELSE
     regTVMode   db 0
ENDIF
	Reserved			db	0

	fBoardConfig	dd	BoardCRTSupport + BoardMemType + BoardI2CLineShorted + BoardBusType + BoardDFPSupport + BoardTVOutSupport

	regTMUGBEINIT	dd	DEFAULT_TMUGBEINIT

	ptrVBEProductName		dw	offset szVBEProductName
	ptrOEMChipName			dw offset szOEMChipName
	ptrOEMOEMBIOSVersion	dw offset szOEMOEMBIOSVersion

;
ENDIF
;
IF PCIBIOS
;
; PCI adapter information table
;
	ALIGN_DWORD
	dw	0			; Needed by MAKEIMG to realign this table
	public	tblPCIInfo
tblPCIInfo	label	byte
	db	'PCIR'			; Signature
	dw	PCI_ID			; Vendor ID
	dw	PCI_ID_DEVICE		; Device ID
	dw	offset VitalProductData	
	dw	0018h			; Size of this structure
	db	0000h			; PCI data structure revision
	db	000h, 000h, 003h	; Class code (P.I., Sub-class, Base-class)

IF   ROM_64K
	dw	050h			; ROM Image size (40K)
;	dw	060h			; ROM Image size (48K)
;	dw	080h			; ROM Image size (64K)
ELSE
	dw	050h			; ROM Image size (40K)
;	dw	040h			; ROM Image size (32K)
ENDIF

	dw	0000h			; Vendor Revision Level
	db	000h			; Code type (x86)
	db	080h			; Indicator (Last PCI BIOS in ROM)
	dw	0000h			; Reserved
ENDIF
;
IF VESA
;
; VESA BIOS Extensions (VBE) specific data definitions
;
	public	tblVBEInfoBlock
tblVBEInfoBlock	label	byte
	VBEINFOBLOCK_102	<,,offset szOEMName, VBE_8BITDAC or VBE_VGAIO or VBE_NOBLKDAC or VBE_NOSTEREO or VBE_EXTSTEREO, offset tblVBEModes, 4>
;
	public	tblVBEModes, LENGTH_VBEMODES
tblVBEModes	label	word
	dw      0100h, 0101h, 0102h, 0103h, 0105h, 0107h
	dw	0108h
IFE OPTIMIZE_ROM
        dw      0109h, 010Ah, 010Bh, 010Ch
ENDIF
IF BANSHEE_DOUBLESCAN
	dw	0120h, 010Eh, 010Fh, 0123h, 0124h, 0125h, 0126h, 0127h
	dw	0128h, 0129h, 012Ah, 012Bh, 012Ch, 012Dh
ENDIF
	dw	0111h, 0112h, 0114h
	dw	0115h, 0117h, 0118h, 011Ah, 011Bh
	dw	0FFFFh		; End of list
LENGTH_VBEMODES		equ	($ - tblVBEModes) / 2
;
	public	szVBEVendorName
szVBEVendorName	label	byte
	db	STR_VBE_OEMVENDOR, 0
;
	public	szVBEProductName
szVBEProductName	label	byte
	db	STR_OEMPRODUCT, 0
;
	public	szVBEProductRev
szVBEProductRev	label	byte
	db	STR_VBE_OEMPRODUCTREV, 0
;
	public	szOEMName
szOEMName	db	STR_OEMNAME, 0
;
	public	szOEMChipName
szOEMChipName				db	STR_OEMCHIPNAME, 0
;
	public	szOEMOEMBIOSVersion
szOEMOEMBIOSVersion		db	STR_OEMVERSION, 0

;
	public	tblDirectColor16BPP, tblDirectColor24BPP, LENGTH_DCTABLE
tblDirectColor16BPP	db	5, 11, 6, 5, 5, 0, 0, 0, 0
tblDirectColor24BPP	db	8, 16, 8, 8, 8, 0, 0, 0, 0
LENGTH_DCTABLE	equ	($ - tblDirectColor24BPP)
;
;
; Note: The following information is part of the protected mode
;	interface. It is assumed that this code is called from
;	a 32-bit segment, different from the 16-bit segment
;	that this is being compiled into. Therefore any WORD
;	operand access MUST be proceeded with the 066h prefix,
;	and any DWORD access MUST be encoded as for a WORD operand.
;
;	Also, assume that any I/O privilege needed is provided.
;
	public	tblProtectedModeInterface, LENGTH_PMITABLE
	public	VBEPMI05, VBEPMI07, VBEPMI09, IOPRIV
tblProtectedModeInterface	label	byte
	dw	(offset VBEPMI05 - offset tblProtectedModeInterface)
	dw	(offset VBEPMI07 - offset tblProtectedModeInterface)
	dw	(offset VBEPMI09 - offset tblProtectedModeInterface)
	dw	(offset IOPRIV - offset tblProtectedModeInterface)
;
;	VBEPMI05 - Set Bank
;
;	Entry:	DX = Window to set
;	Exit:	None
;
VBEPMI05:
	pusha									; PUSHAD
	sub	bx,bx							; SUB EBX,EBX - Clear hi-order word

	mov	bl,dl							; MOV BX,DX
	shl	bx,1							; SHL EBX,1

  push  bx

  db 066h
  db 081h
  db 0E3h
  dw 03FFh          				;and   bx,03FFh
               
	call	PMI_GetIOBase
	dw	0000h								; Needed for forward reference 32-bit call
	db	066h
	add	dx,REG_VGAINIT1			; ADD DX,REG_VGAINIT1
	in	ax,dx								; IN EAX,DX
	db	25h								; Opcode for AND EAX,<immediate>
	dw	00000h
	dw	09FF0h							; AND EAX,09FF00000h
	db	066h
	mov	ax,bx							; MOV AX,BX
	shl	bx,10							; SHL EBX,10
	or	   ax,bx							; OR EAX,EBX

  pop   bx

  push  bx

  db 066h
  db 081h
  db 0E3h
  dw 0400h          				;and   bx,0400h

  shl   bx,19

  or    ax,bx

  pop   bx

	out	dx,ax							; OUT DX,EAX

;********************************************************************

  db 066h
  db 081h
  db 0E3h
  dw 0400h          				;and   bx,0C00h

  shl   bx,15

	call	PMI_GetIOBase
	dw	0000h								; Needed for forward reference 32-bit call
	db	066h
	add	dx,REG_VGAINIT0			; ADD DX,REG_VGAINIT1
	in		ax,dx							; IN EAX,DX

	db	25h								; Opcode for AND EAX,<immediate>
	dw	0FFFFh
	dw	0F9FFh							; AND EAX,09FF00000h

  or 	ax,bx

  out   dx,ax

;
	popa									; POPAD
	retn
;
;	VBEPMI07 - Set display start
;
;	Entry:	BL = 00h - Set display start
;		     80h - Set display start in vertical retrace
;		CX = Display start address (Bits 0 - 15)
;		DX = Display start address (Bits 16 - 31)
;	Exit:	None
;
VBEPMI07:
	pusha					; PUSHAD
	test	bl,080h				; Wait for vertical retrace?
	jz	short VBEPMI07_20		;  N: Skip ahead
;
	push	dx				; PUSH EDX
	db	66h
	mov	dx,03DAh			; MOV DX,3DAh

VBEPMI07_05:
	in	al,dx
	test	al,008h				; In Display period?
	jz	short VBEPMI07_05		;  N: Keep waiting

VBEPMI07_10:
	in	al,dx
	test	al,008h				; In vertical retrace period?
	jnz	short VBEPMI07_10		;  N: Keep waiting
	pop	dx				; POP EDX
;
VBEPMI07_20:
	db	066h
	mov	ax,dx				; MOV AX,DX
	shl	ax,16				; SHL EAX,16
	db	066h
	mov	ax,cx				; MOV AX,CX
	call	PMI_GetIOBase
	dw	0000h				; Needed for forward reference 32-bit call
	shl	ax,2				; SHL EAX,2
	db	066h
	add	dx,REG_VIDDESKTOPSTART		; ADD DX,REG_VIDDESKTOPSTART
	out	dx,ax				; OUT DX,EAX
	popa					; POPAD
	retn
;
;	VBEPMI09 - Set Primary DAC data
;
;	Entry:	BL = 00h: Set DAC data
;		   = 80h: Set DAC data after wait for vertical blank
;		CX = Number of palette registers
;		DX = First palette register
;		ES:DI = Pointer to data
;	Exit:	None
;
VBEPMI09:
	pusha					; PUSHAD
	push	ds
	test	bl,80h				; Wait for vertical retrace period?
	jz	short VBEPMI09_20		;  N: Skip ahead
	push	dx				; PUSH EDX
	db	66h				; MOV DX,3DAh
	mov	dx,03DAh			; Assume color mode
VBEPMI09_10:
	in	al,dx
	test	al,008h				; In vertical retrace?
	jnz	short VBEPMI09_10		;  N: Continue waiting
	pop	dx				; POP EDX
VBEPMI09_20:
	cld
	push	es
	pop	ds
	mov	si,di				; DS:SI = Pointer to DAC data
;
; The following is caused by a quirk with the VBE spec. Despite the
; "claimed" 32-bit interface, the VBE spec only specifies 16-bit registers.
; Since the assembler would generate the 66h prefix in a 16-bit segment
; (such as necessary to compile a single-segment file for the ROM), the
; following instruction must be hand-coded.
;
	db	00Fh, 0B7h, 0C9h		; MOVZX ECX,CX
;
	mov	al,dl
	db	66h				; MOV DX,3C8h
	mov	dx,03C8h
	out	dx,al				; Set first index
	inc	dx
VBEPMI09_30:
	lodsw					; LODSD
	ror	ax,16				; ROR EAX,16
	out	dx,al				; Red
	rol	ax,8				; ROL EAX,8
	out	dx,al				; Green
	rol	ax,8				; ROL EAX,8
	out	dx,al				; Blue
	loop	VBEPMI09_30
;
	pop	ds
	popa					; POPAD
	retn
;
;	PMI_GetIOBase - Get I/O address
;
;	Entry:	None
;	Exit:	EDX = Base I/O address
;
	public	PMI_GetIOBase
PMI_GetIOBase:
	nop					; Two bytes are wasted due to
	nop					; the hand-compiled CALL
; Real entry point is here:
	push	ax				; PUSH EAX
	db	066h
	mov	dx,03D4h			; MOV DX,03D4h
	mov	al,SCRATCH_REG0
	out	dx,al
	inc	dx				; INC EDX
	in	al,dx
	movzx	dx,al				; MOVZX EDX,AL
	shl	dx,8				; SHL EDX,8
	pop	ax				; POP EAX
	retn
;
IOPRIV:
	dw	03C8h, 03C9h
	dw	03D4h, 03D5h, 03DAh
	dw	0FFFFh
	dw	0FFFFh
LENGTH_PMITABLE		equ	($ - tblProtectedModeInterface)
;
ENDIF
;
IF      DVI_BIOS
public		DVI_Patch_Table_00_1024
DVI_Patch_Table_00_1024		label		word
	db			0A3h										;CR00
	db			020h										;CR06
	db			0BBh										;CR07

DVI_Patch_Table_01_1024		label		word
	db			0A3h										;CR00
	db			010h										;CR06
	db			0BAh										;CR07

DVI_Patch_Table_02_1024		label		word
	db			0A4h										;CR00
	db			018h										;CR06
	db			0BBh										;CR07

DVI_Patch_Table_03_1024		label		word
	db			0A3h										;CR00
	db			020h										;CR06
	db			0F1h										;CR07

DVI_Patch_Table_04_1024		label		word
	db			0A3h										;CR00
	db			024h										;CR06
	db			0F5h										;CR07

DVI_Patch_Table_05_1024		label		word
	db			0A3h										;CR00
	db			020h										;CR06
	db			0E9h										;CR07

DVI_Patch_Table_06_1024		label		word
	db			0A3h										;CR00
	db			020h										;CR06
	db			0E9h										;CR07

;320xXXX LCD table	
DVI_Patch_Table_07_1024		label		word
	db			0A3h										;CR00
	db			020h										;CR06
	db			0B9h										;CR07

;512x384 and 400x300 LCD table	
DVI_Patch_Table_08_1024		label		word
	db			0A3h										;CR00
	db			020h										;CR06
	db			0ABh										;CR07

public		DVI_Patch_Table_00_1280
;640xXXX modes
DVI_Patch_Table_00_1280		label		word
	db			0D0h										;CR00
	db			018h										;CR06
	db			09Ah										;CR07

;Mode 03+
DVI_Patch_Table_01_1280		label		word
	db			0B8h										;CR00
	db			018h										;CR06
	db			09Ah										;CR07

;Mode 00+
DVI_Patch_Table_02_1280		label		word
	db			0B8h										;CR00
	db			018h										;CR06
	db			09Ah										;CR07
	
;800X600
DVI_Patch_Table_03_1280		label		word
	db			0D0h										;CR00
	db			01Ch										;CR06
	db			0D0h										;CR07

;1024X768
DVI_Patch_Table_04_1280		label		word
	db			0D0h										;CR00
	db			01Ch										;CR06
	db			0D4h										;CR07

;
DVI_Patch_Table_05_1280		label		word
	db			000h										;CR00
	db			000h										;CR06
	db			000h										;CR07

;
DVI_Patch_Table_06_1280		label		word
	db			000h										;CR00
	db			000h										;CR06
	db			000h										;CR07

;320xXXX
DVI_Patch_Table_07_1280		label		word
	db			0D0h										;CR00
	db			020h										;CR06
	db			098h										;CR07

;400x300 & 512x384
DVI_Patch_Table_08_1280		label		word
	db			0D0h										;CR00
	db			020h										;CR06
	db			092h										;CR07

;**********************************************************************
;CRTC parameters for center a halved mode on the panel

public	ExtMode320x240Table
ExtMode320x240Table	label	byte
	db	05Fh, 04Fh, 03Bh, 08Fh, 040h, 010h
	db	00Bh, 03Ch, 000h, 040h, 000h, 000h
	db	000h, 000h, 000h, 000h, 070h, 005h
	db	0EFh, 050h, 000h, 067h, 094h, 0E3h
	db	0FFh

public	ExtMode400x300Table
ExtMode400x300Table	label	byte
;400x300 on a 640x400 desktop
	 db  05Fh, 03Fh, 040h, 094h, 046h, 010h
	 db  00Bh, 03Eh, 000h, 000h, 000h, 000h 
	 db  000h, 000h, 007h, 03Fh, 091h, 005h 
	 db  02Bh, 080h, 000h, 05Dh, 0DAh, 0E3h 
	 db  0FFh 

;400x300 on a 640x480 desktop
;	db	05Fh, 03Fh, 040h, 094h, 049h, 010h
;	db	00Bh, 03Eh, 000h, 040h, 000h, 000h
;	db	000h, 000h, 000h, 000h, 090h, 005h
;	db	02Bh, 080h, 000h, 085h, 0B2h, 0E3h
;	db	0FFh

;400x300 on a 800x600 desktop
;   db  07Fh, 063h, 04Ah, 08Ah, 054h, 081h
;   db  072h, 03Eh, 000h, 040h, 000h, 000h
;   db  000h, 000h, 000h, 000h, 0C0h, 001h
;   db  02Bh, 064h, 000h, 0B0h, 0CBh, 0E3h
;   db  0FFh


public	ExtMode512x384Table
ExtMode512x384Table	label	byte
;512x384 on a 640x400 desktop
	 db  05Fh, 03Fh, 048h, 09Ch, 04Ch, 010h
	 db  00Bh, 03Eh, 000h, 000h, 000h, 000h 
	 db  000h, 000h, 007h, 03Fh, 0C3h, 005h 
	 db  07Fh, 080h, 000h, 087h, 003h, 0E3h 
	 db  0FFh 

;512x384 on a 640x480 desktop
;	db	05Fh, 03Fh, 048h, 09ch, 049h, 010h
;	db	00Bh, 03Eh, 000h, 040h, 000h, 000h
;	db	000h, 000h, 000h, 000h, 0B0h, 005h
;	db	07Fh, 080h, 000h, 082h, 0FFh, 0E3h
;	db	0FFh

;512x384 on a 1024x768 desktop
;	db	0A3h, 07Fh, 060h, 0A8h, 070h, 014h
;	db	024h, 0B3h, 000h, 060h, 000h, 000h
;	db	000h, 000h, 000h, 000h, 040h, 009h
;	db	07Fh, 080h, 000h, 040h, 066h, 0E3h
;	db	0FFh

ENDIF
	end
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;



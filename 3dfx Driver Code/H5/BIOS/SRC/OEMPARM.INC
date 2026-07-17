	page	57, 132
	subttl	OEMPARM.INC - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	OEMPARM.INC - OEM extended parameter table data
;
;	Written by:	Larry Coffey
;	Date:		7/17/96
;
;	The data in this file will (must) immediately follow the standard
;	VGA parameter table.
;
;	$Revision$
;	$Date$
;	$Logfile$
;	$Log$
;
;
;
IF1
	%out	!		Using 3Dfx parameters for positions 1D and up
ENDIF
;
	public	tblExtModeParms
tblExtModeParms	label	byte
IFE OPTIMIZE_ROM
;
; Mode 55h / VESA Mode 109h / Internal Mode 1Dh
;	132x25 Color Text (8x16 font) - 40.0 MHz, 31.5 KHz, 70 Hz
;
	db	084h, 018h, 010h
	dw	02000h
	db	001h, 003h, 000h, 002h
	db	06Fh
	db	09Ah, 083h, 083h, 09Eh, 088h, 01Ah
	db	0BFh, 01Fh, 000h, 04Fh, 00Dh, 00Eh
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 042h, 01Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 00Ch, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 54h / VESA Mode 10Ah / Internal Mode 1Eh
;	132x43 Color Text (8x9 font) - 40.0 MHz, 31.5 KHz, 70 Hz
;
	db	084h, 02Ah, 009h
	dw	04000h
	db	001h, 003h, 000h, 002h
	db	06Fh
	db	09Ah, 083h, 083h, 09Eh, 088h, 01Ah
	db	0BFh, 01Fh, 000h, 048h, 007h, 008h
	db	000h, 000h, 000h, 000h, 092h, 084h
	db	082h, 042h, 01Fh, 089h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 00Ch, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 65h / VESA Mode 10Bh / Internal Mode 1Fh
;	132x50 Color Text (8x8 font) - 40.0 MHz, 31.5 KHz, 70 Hz
;
	db	084h, 031h, 008h
	dw	04000h
	db	001h, 003h, 000h, 002h
	db	06Fh
	db	09Ah, 083h, 083h, 09Eh, 088h, 01Ah
	db	0BFh, 01Fh, 000h, 047h, 006h, 007h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 042h, 01Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 00Ch, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 64h / VESA Mode 10Ch / Internal Mode 20h
;	132x60 Color Text (8x8 font) - 40.0 MHz, 31.5 KHz, 60 Hz
;
	db	084h, 03Bh, 008h
	dw	04000h
	db	001h, 003h, 000h, 002h
	db	0EFh
	db	09Ah, 083h, 083h, 09Eh, 088h, 01Ah
	db	00Bh, 03Eh, 000h, 047h, 006h, 007h
	db	000h, 000h, 000h, 000h, 0EAh, 08Ch
	db	0DFh, 042h, 01Fh, 0E7h, 004h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 00Ch, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
ENDIF
;
IF BANSHEE_DOUBLESCAN
;
; Mode 78 / VESA Mode 180 / *Internal Mode 21h*
; Mode 79 / VESA Mode 10E / Internal Mode 25h
; Mode 7A / VESA Mode 10F / Internal Mode 26h
;	320x200 - 256-color, 32K-color, 16M-color (8x8 font, 40x25 "Text")
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 008h
	dw	0FFFFh
	db	001h, 00Fh, 000h, 00Eh
	db	06Fh
	db	02Dh, 027h, 028h, 090h, 028h, 08Eh
	db	0BFh, 01Fh, 000h, 0C0h, 000h, 000h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 028h, 01Fh, 096h, 0B9h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 008h, 009h, 00Ah, 00Bh
	db	00Ch, 00Dh, 00Eh, 00Fh, 001h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 040h
	db	005h, 00Fh, 0FFh
;
; Mode 20 / VESA Mode 181 / *Internal Mode 22h*
; Mode 21 / VESA Mode 182 / Internal Mode 27h
; Mode 22 / VESA Mode 183 / Internal Mode 28h
;	320x240 - 256-color, 32K-color, 16M-color (8x8 font, 40x30 "Text")
;	25.175 MHz, 31.5 KHz, 60 Hz
;
	db	028h, 01Dh, 008h
	dw	0FFFFh
	db	001h, 00Fh, 000h, 00Eh
	db	0EFh
	db	02Dh, 027h, 028h, 090h, 028h, 08Eh
	db	00Bh, 03Eh, 000h, 0C0h, 000h, 000h
	db	000h, 000h, 000h, 000h, 0E9h, 00Bh
	db	0DFh, 028h, 01Fh, 0E7h, 004h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 008h, 009h, 00Ah, 00Bh
	db	00Ch, 00Dh, 00Eh, 00Fh, 001h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 040h
	db	005h, 00Fh, 0FFh
;
; Mode 23 / VESA Mode 184 / *Internal Mode 23h*
; Mode 24 / VESA Mode 185 / Internal Mode 29h
; Mode 25 / VESA Mode 186 / Internal Mode 2Ah
;	400x300 - 256-color, 32K-color, 16M-color (8x8 font, 50x37 "Text")
;	40.000/2 MHz, 35.5 KHz, 60 Hz
;
        db      032h, 024h, 008h
        dw      0FFFFh
        db      001h, 00Fh, 000h, 00Eh
        db      02Fh
        db      03Dh, 031h, 032h, 080h, 034h, 01Ch
        db      072h, 0F0h, 000h, 060h, 000h, 000h
        db      000h, 000h, 000h, 000h, 058h, 00Ch
        db      057h, 064h, 000h, 058h, 073h, 0E3h
        db      0FFh
        db      000h, 001h, 002h, 003h, 004h, 005h
        db      006h, 007h, 008h, 009h, 00Ah, 00Bh
        db      00Ch, 00Dh, 00Eh, 00Fh, 001h, 000h
        db      00Fh, 000h
        db      000h, 000h, 000h, 000h, 000h, 040h
        db      005h, 00Fh, 0FFh
;
; Mode 26 / VESA Mode 187 / *Internal Mode 24h*
; Mode 27 / VESA Mode 188 / Internal Mode 2Bh
; Mode 28 / VESA Mode 189 / Internal Mode 2Ch
;	512x384 - 256-color, 32K-color, 16M-color (8x14 font, 64x27 "Text")
;	65.000/2 MHz, 48.0 KHz, 60 Hz
;
	db	040h, 01Ch, 00Eh
	dw	0FFFFh
	db	001h, 00Fh, 000h, 00Eh
	db	02Fh
	db	04Fh, 03Fh, 040h, 083h, 041h, 009h
	db	024h, 0F5h, 000h, 060h, 000h, 000h
	db	000h, 000h, 000h, 000h, 002h, 008h
	db	0FFh, 080h, 000h, 0FFh, 025h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 008h, 009h, 00Ah, 00Bh
	db	00Ch, 00Dh, 00Eh, 00Fh, 041h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 040h
	db	005h, 00Fh, 0FFh
ENDIF
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


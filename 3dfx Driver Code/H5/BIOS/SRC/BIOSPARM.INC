;
;	BIOSPARM.INC - Parameter table data
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	$Revision: 2 $
;	$Date: 9/09/99 3:15p $
;	$Logfile: /devel/h5/BIOS/SRC/BIOSPARM.INC $
;	$Log: /devel/h5/BIOS/SRC/BIOSPARM.INC $
;; 
;; 2     9/09/99 3:15p Kaymann
;; Created labels for the 640x480, 800x600, and 1024x768 default timings.
;; Fixed up all the 800x600 and 1024x768 timings.
;
;
	%out	!		Using 3Dfx parameters for positions 0 thru 1Ch
;
	public	tblStdParameters
tblStdParameters	label	byte
;
; Mode 0 / Internal Mode 00h
;	40x25 - Color Text (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 008h
	dw	00800h
	db	009h, 003h, 000h, 002h
	db	063h
	db	02Dh, 027h, 028h, 090h, 02Bh, 0A0h
	db	0BFh, 01Fh, 000h, 0C7h, 006h, 007h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 014h, 01Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 010h, 011h, 012h, 013h
	db	014h, 015h, 016h, 017h, 008h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 1 / Internal Mode 01h
;	40x25 - Color Text (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 008h
	dw	00800h
	db	009h, 003h, 000h, 002h
	db	063h
	db	02Dh, 027h, 028h, 090h, 02Bh, 0A0h
	db	0BFh, 01Fh, 000h, 0C7h, 006h, 007h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 014h, 01Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 010h, 011h, 012h, 013h
	db	014h, 015h, 016h, 017h, 008h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 2 / Internal Mode 02h
;	80x25 - Color Text (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 008h
	dw	01000h
	db	001h, 003h, 000h, 002h
	db	063h
	db	05Fh, 04Fh, 050h, 082h, 055h, 081h
	db	0BFh, 01Fh, 000h, 0C7h, 006h, 007h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 028h, 01Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 010h, 011h, 012h, 013h
	db	014h, 015h, 016h, 017h, 008h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 3 / Internal Mode 03h
;	80x25 - Color Text (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 008h
	dw	01000h
	db	001h, 003h, 000h, 002h
	db	063h
	db	05Fh, 04Fh, 050h, 082h, 055h, 081h
	db	0BFh, 01Fh, 000h, 0C7h, 006h, 007h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 028h, 01Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 010h, 011h, 012h, 013h
	db	014h, 015h, 016h, 017h, 008h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 4 / Internal Mode 04h
;	320x200 - 4-color CGA (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 008h
	dw	04000h
	db	009h, 003h, 000h, 002h
	db	063h
	db	02Dh, 027h, 028h, 090h, 02Bh, 080h
	db	0BFh, 01Fh, 000h, 0C1h, 000h, 000h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 014h, 000h, 096h, 0B9h, 0A2h
	db	0FFh
	db	000h, 013h, 015h, 017h, 002h, 004h
	db	006h, 007h, 010h, 011h, 012h, 013h
	db	014h, 015h, 016h, 017h, 001h, 000h
	db	003h, 000h
	db	000h, 000h, 000h, 000h, 000h, 030h
	db	00Fh, 000h, 0FFh
;
; Mode 5 / Internal Mode 05h
;	320x200 - 4-color CGA (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 008h
	dw	04000h
	db	009h, 003h, 000h, 002h
	db	063h
	db	02Dh, 027h, 028h, 090h, 02Bh, 080h
	db	0BFh, 01Fh, 000h, 0C1h, 000h, 000h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 014h, 000h, 096h, 0B9h, 0A2h
	db	0FFh
	db	000h, 013h, 015h, 017h, 002h, 004h
	db	006h, 007h, 010h, 011h, 012h, 013h
	db	014h, 015h, 016h, 017h, 001h, 000h
	db	003h, 000h
	db	000h, 000h, 000h, 000h, 000h, 030h
	db	00Fh, 000h, 0FFh
;
; Mode 6 / Internal Mode 06h
;	640x200 - 2-color CGA (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 008h
	dw	04000h
	db	001h, 001h, 000h, 006h
	db	063h
	db	05Fh, 04Fh, 050h, 082h, 054h, 080h
	db	0BFh, 01Fh, 000h, 0C1h, 000h, 000h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 028h, 000h, 096h, 0B9h, 0C2h
	db	0FFh
	db	000h, 017h, 017h, 017h, 017h, 017h
	db	017h, 017h, 017h, 017h, 017h, 017h
	db	017h, 017h, 017h, 017h, 001h, 000h
	db	001h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h
	db	00Dh, 000h, 0FFh
;
; Mode 7 / Internal Mode 07h
;	80x25 - Mono Text (9x14 Font)
;	28.321 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 00Eh
	dw	01000h
	db	000h, 003h, 000h, 003h
	db	0A6h
	db	05Fh, 04Fh, 050h, 082h, 055h, 081h
	db	0BFh, 01Fh, 000h, 04Dh, 00Bh, 00Ch
	db	000h, 000h, 000h, 000h, 083h, 085h
	db	05Dh, 028h, 00Dh, 063h, 0BAh, 0A3h
	db	0FFh
	db	000h, 008h, 008h, 008h, 008h, 008h
	db	008h, 008h, 010h, 018h, 018h, 018h
	db	018h, 018h, 018h, 018h, 00Eh, 000h
	db	00Fh, 008h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Ah, 000h, 0FFh
;
; Mode 5Bh / VBE Mode 100h / *Internal Mode 08h*
; Mode 29h / VBE Mode 18Ah / Internal Mode 2Dh
; Mode 2Ah / VBE Mode 18Bh / Internal Mode 2Fh
;	640X400 - 256-color, 32K-color, 16M-color (8x16 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 010h
	dw	0FFFFh
	db	001h, 00Fh, 000h, 00Eh
	db	063h
	db	05Fh, 04Fh, 050h, 082h, 051h, 09dh
	db	0BFh, 01Fh, 000h, 040h, 000h, 000h
	db	000h, 000h, 000h, 000h, 09Ch, 00Eh
	db	08Fh, 050h, 01Fh, 096h, 0B9h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 008h, 009h, 00Ah, 00Bh
	db	00Ch, 00Dh, 00Eh, 00Fh, 041h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 040h
	db	005h, 00Fh, 0FFh
;
; Mode 5Fh / VBE Mode 101h / *Internal Mode 09h*
; Mode 6E / VESA Mode 111h / Internal Mode 2Fh
; Mode 69 / VESA Mode 112h / Internal Mode 30h
;	640X480 - 256-color, 32K-color, 16M-color (8x16 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 01Dh, 010h
	dw	0FFFFh
	db	001h, 00Fh, 000h, 00Eh
	db	0E3h

public	ExtMode640x480Table
ExtMode640x480Table	label	byte

	db	05Fh, 04Fh, 050h, 082h, 051h, 09Dh
	db	00Bh, 03Eh, 000h, 040h, 000h, 000h
	db	000h, 000h, 000h, 000h, 0E9h, 00Bh
	db	0DFh, 050h, 000h, 0E7h, 004h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 008h, 009h, 00Ah, 00Bh
	db	00Ch, 00Dh, 00Eh, 00Fh, 041h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 040h
	db	005h, 00Fh, 0FFh
;
; Mode 6Ah / VBE Mode 102h / Internal Mode 0Ah
;	800X600 - 16-color (8x16 Font)
;	40.000 MHz, 38.000 KHz, 60 Hz
;
	db	064h, 024h, 010h
	dw	0FA00h
	db	001h, 00Fh, 000h, 006h
	db	02Fh
	db	07Fh, 063h, 063h, 083h, 06Bh, 01Bh
	db	072h, 0F0h, 000h, 060h, 000h, 000h
	db	000h, 000h, 000h, 000h, 058h, 00Ch
	db	057h, 032h, 000h, 057h, 073h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 001h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h
	db	005h, 00Fh, 0FFh
;
; Mode 5Ch / VBE Mode 103h / *Internal Mode 0Bh*
; Mode 70h / VBE Mode 114h / Internal Mode 31h
; Mode 71h / VBE Mode 115h / Internal Mode 32h
;	800X600 - 256-color, 32K-color, 16M-color (8x16 Font)
;	40.000 MHz, 38.000 KHz, 60 Hz
;
        db      064h, 024h, 010h
        dw      0FFFFh
        db      001h, 00Fh, 000h, 00Eh
        db      02Fh

public	ExtMode800x600Table
ExtMode800x600Table	label	byte
        db      07Fh, 063h, 063h, 083h, 068h, 018h
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
; Mode 6B / VESA Mode 107 / *Internal Mode 0Ch*
; Mode 74 / VESA Mode 11A / Internal Mode 35h
; Mode 75 / VESA Mode 11B / Internal Mode 36h
;	1280x1024 - 256-color, 32K-color, 16M-color (8x16 Font)
;	108.0 MHz, 64 KHz, 60 Hz
;
	db	0A0h, 03Fh, 010h
	dw	0FFFFh
	db	001h, 00Fh, 000h, 00Eh
	db	02Fh
	db	0CEh, 09Fh, 0A0h, 091h, 0A5h, 013h
	db	028h, 052h, 000h, 040h, 000h, 000h
	db	000h, 000h, 000h, 000h, 000h, 003h
	db	0FFh, 0A0h, 000h, 001h, 028h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 008h, 009h, 00Ah, 00Bh
	db	00Ch, 00Dh, 00Eh, 00Fh, 041h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 040h
	db	005h, 00Fh, 0FFh
;
; Mode D / Internal Mode 0Dh
;	320x200 - 16-color planar (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 008h
	dw	02000h
	db	009h, 00Fh, 000h, 006h
	db	063h
	db	02Dh, 027h, 028h, 090h, 02Bh, 080h
	db	0BFh, 01Fh, 000h, 0C0h, 000h, 000h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 014h, 000h, 096h, 0B9h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 010h, 011h, 012h, 013h
	db	014h, 015h, 016h, 017h, 001h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h
	db	005h, 00Fh, 0FFh
;
; Mode E / Internal Mode 0Eh
;	640x200 - 16-color planar (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 008h
	dw	04000h
	db	001h, 00Fh, 000h, 006h
	db	063h
	db	05Fh, 04Fh, 050h, 082h, 054h, 080h
	db	0BFh, 01Fh, 000h, 0C0h, 000h, 000h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 028h, 000h, 096h, 0B9h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 010h, 011h, 012h, 013h
	db	014h, 015h, 016h, 017h, 001h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h
	db	005h, 00Fh, 0FFh
;
; Mode 5Eh / VBE Mode 105h / *Internal Mode 0Fh*
; Mode 72h / VBE Mode 117h / Internal Mode 33h
; Mode 73h / VBE Mode 118h / Internal Mode 34h
;	1024X768 - 256-color, 32K-color, 16M-color (8x16 Font)
;	65.000 MHz, 48.500 KHz, 60 Hz
;
	db	080h, 02Fh, 010h
	dw	0FFFFh
	db	001h, 00Fh, 000h, 00Eh
	db	0EFh

public	ExtMode1024x768Table
ExtMode1024x768Table	label	byte

	db	0A3h, 07Fh, 080h, 0A0h, 083h, 014h
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
;
; Mode 68h / VESA Mode 108h / Internal Mode 10h
;	80x60 Color Text (8x8 font) - 25.175 MHz, 31.5 KHz, 60 Hz
;
	db	050h, 03Bh, 008h
	dw	2600h
	db	001h, 003h, 000h, 002h
	db	0E3h
	db	05Fh, 04Fh, 050h, 082h, 055h, 083h
	db	00Bh, 03Eh, 000h, 047h, 006h, 007h
	db	000h, 000h, 000h, 000h, 0E9h, 08Bh
	db	0DFh, 028h, 01Fh, 0E7h, 004h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 00Ch, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode F / Internal Mode 11h
;	640x350 - 2-bit mono pseudo-planar (8x14 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 00Eh
	dw	08000h
	db	001h, 00Fh, 000h, 006h
	db	0A2h
	db	05Fh, 04Fh, 050h, 082h, 054h, 080h
	db	0BFh, 01Fh, 000h, 040h, 000h, 000h
	db	000h, 000h, 000h, 000h, 083h, 085h
	db	05Dh, 028h, 00Fh, 063h, 0BAh, 0E3h
	db	0FFh
	db	000h, 008h, 000h, 000h, 018h, 018h
	db	000h, 000h, 000h, 008h, 000h, 000h
	db	000h, 018h, 000h, 000h, 00Bh, 000h
	db	005h, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h
	db	005h, 005h, 0FFh
;
; Mode 10h / Internal Mode 12h
;	640x350 - 16-bit planar (8x14 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 00Eh
	dw	08000h
	db	001h, 00Fh, 000h, 006h
	db	0A3h
	db	05Fh, 04Fh, 050h, 082h, 054h, 080h
	db	0BFh, 01Fh, 000h, 040h, 000h, 000h
	db	000h, 000h, 000h, 000h, 083h, 085h
	db	05Dh, 028h, 00Fh, 063h, 0BAh, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 001h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h
	db	005h, 00Fh, 0FFh
;
; Mode 0* / Internal Mode 13h
;	40x25 - Color Text (8x14 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 00Eh
	dw	00800h
	db	009h, 003h, 000h, 002h
	db	0A3h
	db	02Dh, 027h, 028h, 090h, 02Bh, 0A0h
	db	0BFh, 01Fh, 000h, 04Dh, 00Bh, 00Ch
	db	000h, 000h, 000h, 000h, 083h, 085h
	db	05Dh, 014h, 01Fh, 063h, 0BAh, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 008h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 1* / Internal Mode 14h
;	40x25 - Color Text (8x14 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 00Eh
	dw	00800h
	db	009h, 003h, 000h, 002h
	db	0A3h
	db	02Dh, 027h, 028h, 090h, 02Bh, 0A0h
	db	0BFh, 01Fh, 000h, 04Dh, 00Bh, 00Ch
	db	000h, 000h, 000h, 000h, 083h, 085h
	db	05Dh, 014h, 01Fh, 063h, 0BAh, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 008h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 2* / Internal Mode 15h
;	80x25 - Color Text (8x14 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 00Eh
	dw	01000h
	db	001h, 003h, 000h, 002h
	db	0A3h
	db	05Fh, 04Fh, 050h, 082h, 055h, 081h
	db	0BFh, 01Fh, 000h, 04Dh, 00Bh, 00Ch
	db	000h, 000h, 000h, 000h, 083h, 085h
	db	05Dh, 028h, 01Fh, 063h, 0BAh, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 008h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 3* / Internal Mode 16h
;	80x25 - Color Text (8x14 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 00Eh
	dw	01000h
	db	001h, 003h, 000h, 002h
	db	0A3h
	db	05Fh, 04Fh, 050h, 082h, 055h, 081h
	db	0BFh, 01Fh, 000h, 04Dh, 00Bh, 00Ch
	db	000h, 000h, 000h, 000h, 083h, 085h
	db	05Dh, 028h, 01Fh, 063h, 0BAh, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 008h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 0+/1+ / Internal Mode 17h
;	40x25 - Color Text (9x16 Font)
;	28.321 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 010h
	dw	00800h
	db	008h, 003h, 000h, 002h
	db	067h
	db	02Dh, 027h, 028h, 090h, 02Bh, 0A0h
	db	0BFh, 01Fh, 000h, 04Fh, 00Dh, 00Eh
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 014h, 01Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 00Ch, 000h
	db	00Fh, 008h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 2+/3+ / Internal Mode 18h
;	80x25 - Color Text (9x16 Font)
;	28.321 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 010h
	dw	01000h
	db	000h, 003h, 000h, 002h
	db	067h
	db	05Fh, 04Fh, 050h, 082h, 055h, 081h
	db	0BFh, 01Fh, 000h, 04Fh, 00Dh, 00Eh
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 028h, 01Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 00Ch, 000h
	db	00Fh, 008h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Eh, 000h, 0FFh
;
; Mode 7+ / Internal Mode 19h
;	80x25 - Mono Text (9x16 Font)
;	28.321 MHz, 31.5 KHz, 70 Hz
;
	db	050h, 018h, 010h
	dw	01000h
	db	000h, 003h, 000h, 002h
	db	066h
	db	05Fh, 04Fh, 050h, 082h, 055h, 081h
	db	0BFh, 01Fh, 000h, 04Fh, 00Dh, 00Eh
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 028h, 00Fh, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 008h, 008h, 008h, 008h, 008h
	db	008h, 008h, 010h, 018h, 018h, 018h
	db	018h, 018h, 018h, 018h, 00Eh, 000h
	db	00Fh, 008h
	db	000h, 000h, 000h, 000h, 000h, 010h
	db	00Ah, 000h, 0FFh
;
; Mode 11h / Internal Mode 1Ah
;	640x480 - 2-color planar (8x16 Font)
;	25.175 MHz, 31.5 KHz, 60 Hz
;
	db	050h, 01Dh, 010h
	dw	0A000h
	db	001h, 00Fh, 000h, 006h
	db	0E3h
	db	05Fh, 04Fh, 050h, 082h, 054h, 080h
	db	00Bh, 03Eh, 000h, 040h, 000h, 000h
	db	000h, 000h, 000h, 000h, 0EAh, 08Ch
	db	0DFh, 028h, 000h, 0E7h, 004h, 0C3h
	db	0FFh
	db	000h, 03Fh, 03Fh, 03Fh, 03Fh, 03Fh
	db	03Fh, 03Fh, 03Fh, 03Fh, 03Fh, 03Fh
	db	03Fh, 03Fh, 03Fh, 03Fh, 001h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h
	db	005h, 001h, 0FFh
;
; Mode 12h / Internal Mode 1Bh
;	640x480 - 16-color planar (8x16 Font)
;	25.175 MHz, 31.5 KHz, 60 Hz
;
	db	050h, 01Dh, 010h
	dw	0A000h
	db	001h, 00Fh, 000h, 006h
	db	0E3h
	db	05Fh, 04Fh, 050h, 082h, 054h, 080h
	db	00Bh, 03Eh, 000h, 040h, 000h, 000h
	db	000h, 000h, 000h, 000h, 0EAh, 08Ch
	db	0DFh, 028h, 000h, 0E7h, 004h, 0E3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	014h, 007h, 038h, 039h, 03Ah, 03Bh
	db	03Ch, 03Dh, 03Eh, 03Fh, 001h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 000h
	db	005h, 00Fh, 0FFh
;
; Mode 13h / Internal Mode 1Ch
;	320x200 - 256-color (8x8 Font)
;	25.175 MHz, 31.5 KHz, 70 Hz
;
	db	028h, 018h, 008h
	dw	02000h
	db	001h, 00Fh, 000h, 00Eh
	db	063h
	db	05Fh, 04Fh, 050h, 082h, 054h, 080h
	db	0BFh, 01Fh, 000h, 041h, 000h, 000h
	db	000h, 000h, 000h, 000h, 09Ch, 08Eh
	db	08Fh, 028h, 040h, 096h, 0B9h, 0A3h
	db	0FFh
	db	000h, 001h, 002h, 003h, 004h, 005h
	db	006h, 007h, 008h, 009h, 00Ah, 00Bh
	db	00Ch, 00Dh, 00Eh, 00Fh, 041h, 000h
	db	00Fh, 000h
	db	000h, 000h, 000h, 000h, 000h, 040h
	db	005h, 00Fh, 0FFh
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


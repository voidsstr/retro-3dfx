;
;	DACDATA.INC - Default RAMDAC tables
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;
;	Written by:	Larry Coffey
;	Date:		5/22/90
;
;	$Revision$
;	$Date$
;	$Logfile$
;	$Log$
;
;
; 16-color mode DAC table
;
	public	tbl16ColorDAC, LENGTH_16COLOR_DAC_TABLE
tbl16ColorDAC	label	byte
	db	000h		; 000h, 000h, 000h
	db	020h		; 000h, 000h, 02Ah
	db	008h		; 000h, 02Ah, 000h
	db	028h		; 000h, 02Ah, 02Ah
	db	002h		; 02Ah, 000h, 000h
	db	022h		; 02Ah, 000h, 02Ah
	db	00Ah		; 02Ah, 02Ah, 000h
	db	02Ah		; 02Ah, 02Ah, 02Ah
	db	010h		; 000h, 000h, 015h
	db	030h		; 000h, 000h, 03Fh
	db	018h		; 000h, 02Ah, 015h
	db	038h		; 000h, 02Ah, 03Fh
	db	012h		; 02Ah, 000h, 015h
	db	032h		; 02Ah, 000h, 03Fh
	db	01Ah		; 02Ah, 02Ah, 015h
	db	03Ah		; 02Ah, 02Ah, 03Fh
	db	004h		; 000h, 015h, 000h
	db	024h		; 000h, 015h, 02Ah
	db	00Ch		; 000h, 03Fh, 000h
	db	02Ch		; 000h, 03Fh, 02Ah
	db	006h		; 02Ah, 015h, 000h
	db	026h		; 02Ah, 015h, 02Ah
	db	00Eh		; 02Ah, 03Fh, 000h
	db	02Eh		; 02Ah, 03Fh, 02Ah
	db	014h		; 000h, 015h, 015h
	db	034h		; 000h, 015h, 03Fh
	db	01Ch		; 000h, 03Fh, 015h
	db	03Ch		; 000h, 03Fh, 03Fh
	db	016h		; 02Ah, 015h, 015h
	db	036h		; 02Ah, 015h, 03Fh
	db	01Eh		; 02Ah, 03Fh, 015h
	db	03Eh		; 02Ah, 03Fh, 03Fh
	db	001h		; 015h, 000h, 000h
	db	021h		; 015h, 000h, 02Ah
	db	009h		; 015h, 02Ah, 000h
	db	029h		; 015h, 02Ah, 02Ah
	db	003h		; 03Fh, 000h, 000h
	db	023h		; 03Fh, 000h, 02Ah
	db	00Bh		; 03Fh, 02Ah, 000h
	db	02Bh		; 03Fh, 02Ah, 02Ah
	db	011h		; 015h, 000h, 015h
	db	031h		; 015h, 000h, 03Fh
	db	019h		; 015h, 02Ah, 015h
	db	039h		; 015h, 02Ah, 03Fh
	db	013h		; 03Fh, 000h, 015h
	db	033h		; 03Fh, 000h, 03Fh
	db	01Bh		; 03Fh, 02Ah, 015h
	db	03Bh		; 03Fh, 02Ah, 03Fh
	db	005h		; 015h, 015h, 000h
	db	025h		; 015h, 015h, 02Ah
	db	00Dh		; 015h, 03Fh, 000h
	db	02Dh		; 015h, 03Fh, 02Ah
	db	007h		; 03Fh, 015h, 000h
	db	027h		; 03Fh, 015h, 02Ah
	db	00Fh		; 03Fh, 03Fh, 000h
	db	02Fh		; 03Fh, 03Fh, 02Ah
	db	015h		; 015h, 015h, 015h
	db	035h		; 015h, 015h, 03Fh
	db	01Dh		; 015h, 03Fh, 015h
	db	03Dh		; 015h, 03Fh, 03Fh
	db	017h		; 03Fh, 015h, 015h
	db	037h		; 03Fh, 015h, 03Fh
	db	01Fh		; 03Fh, 03Fh, 015h
	db	03Fh		; 03Fh, 03Fh, 03Fh
LENGTH_16COLOR_DAC_TABLE	equ	($ - tbl16ColorDAC)
;
; CGA-type modes DAC table
; 	Note:	This table repeats twice. Once, starting at index 00h, and
;		again starting at index 20h
;
	public	tblCGADAC, LENGTH_CGA_DAC_TABLE
tblCGADAC	label	byte
	db	000h		; 000h, 000h, 000h
	db	020h		; 000h, 000h, 02Ah
	db	008h		; 000h, 02Ah, 000h
	db	028h		; 000h, 02Ah, 02Ah
	db	002h		; 02Ah, 000h, 000h
	db	022h		; 02Ah, 000h, 02Ah
	db	006h		; 02Ah, 015h, 000h
	db	02Ah		; 02Ah, 02Ah, 02Ah
	db	000h		; 000h, 000h, 000h
	db	020h		; 000h, 000h, 02Ah
	db	008h		; 000h, 02Ah, 000h
	db	028h		; 000h, 02Ah, 02Ah
	db	002h		; 02Ah, 000h, 000h
	db	022h		; 02Ah, 000h, 02Ah
	db	006h		; 02Ah, 015h, 000h
	db	02Ah		; 02Ah, 02Ah, 02Ah
	db	015h		; 015h, 015h, 015h
	db	035h		; 015h, 015h, 03Fh
	db	01Dh		; 015h, 03Fh, 015h
	db	03Dh		; 015h, 03Fh, 03Fh
	db	017h		; 03Fh, 015h, 015h
	db	037h		; 03Fh, 015h, 03Fh
	db	01Fh		; 03Fh, 03Fh, 015h
	db	03Fh		; 03Fh, 03Fh, 03Fh
	db	015h		; 015h, 015h, 015h
	db	035h		; 015h, 015h, 03Fh
	db	01Dh		; 015h, 03Fh, 015h
	db	03Dh		; 015h, 03Fh, 03Fh
	db	017h		; 03Fh, 015h, 015h
	db	037h		; 03Fh, 015h, 03Fh
	db	01Fh		; 03Fh, 03Fh, 015h
	db	03Fh		; 03Fh, 03Fh, 03Fh
LENGTH_CGA_DAC_TABLE	equ	($ - tblCGADAC)
;
; Monochrome DAC
;
	public	tblMonochromeDAC
tblMonochromeDAC	label	byte
	db	000h, 02Ah, 000h, 03Fh
;
; 256-color mode DAC
;
	public	tbl256ColorDAC, LENGTH_256COLOR_DAC_TABLE
tbl256ColorDAC	label	byte
	db	000h, 000h, 000h
	db	000h, 000h, 02Ah
	db	000h, 02Ah, 000h
	db	000h, 02Ah, 02Ah
	db	02Ah, 000h, 000h
	db	02Ah, 000h, 02Ah
	db	02Ah, 015h, 000h
	db	02Ah, 02Ah, 02Ah
	db	015h, 015h, 015h
	db	015h, 015h, 03Fh
	db	015h, 03Fh, 015h
	db	015h, 03Fh, 03Fh
	db	03Fh, 015h, 015h
	db	03Fh, 015h, 03Fh
	db	03Fh, 03Fh, 015h
	db	03Fh, 03Fh, 03Fh
	db	000h, 000h, 000h
	db	005h, 005h, 005h
	db	008h, 008h, 008h
	db	00Bh, 00Bh, 00Bh
	db	00Eh, 00Eh, 00Eh
	db	011h, 011h, 011h
	db	014h, 014h, 014h
	db	018h, 018h, 018h
	db	01Ch, 01Ch, 01Ch
	db	020h, 020h, 020h
	db	024h, 024h, 024h
	db	028h, 028h, 028h
	db	02Dh, 02Dh, 02Dh
	db	032h, 032h, 032h
	db	038h, 038h, 038h
	db	03Fh, 03Fh, 03Fh
	db	000h, 000h, 03Fh
	db	010h, 000h, 03Fh
	db	01Fh, 000h, 03Fh
	db	02Fh, 000h, 03Fh
	db	03Fh, 000h, 03Fh
	db	03Fh, 000h, 02Fh
	db	03Fh, 000h, 01Fh
	db	03Fh, 000h, 010h
	db	03Fh, 000h, 000h
	db	03Fh, 010h, 000h
	db	03Fh, 01Fh, 000h
	db	03Fh, 02Fh, 000h
	db	03Fh, 03Fh, 000h
	db	02Fh, 03Fh, 000h
	db	01Fh, 03Fh, 000h
	db	010h, 03Fh, 000h
	db	000h, 03Fh, 000h
	db	000h, 03Fh, 010h
	db	000h, 03Fh, 01Fh
	db	000h, 03Fh, 02Fh
	db	000h, 03Fh, 03Fh
	db	000h, 02Fh, 03Fh
	db	000h, 01Fh, 03Fh
	db	000h, 010h, 03Fh
	db	01Fh, 01Fh, 03Fh
	db	027h, 01Fh, 03Fh
	db	02Fh, 01Fh, 03Fh
	db	037h, 01Fh, 03Fh
	db	03Fh, 01Fh, 03Fh
	db	03Fh, 01Fh, 037h
	db	03Fh, 01Fh, 02Fh
	db	03Fh, 01Fh, 027h
	db	03Fh, 01Fh, 01Fh
	db	03Fh, 027h, 01Fh
	db	03Fh, 02Fh, 01Fh
	db	03Fh, 037h, 01Fh
	db	03Fh, 03Fh, 01Fh
	db	037h, 03Fh, 01Fh
	db	02Fh, 03Fh, 01Fh
	db	027h, 03Fh, 01Fh
	db	01Fh, 03Fh, 01Fh
	db	01Fh, 03Fh, 027h
	db	01Fh, 03Fh, 02Fh
	db	01Fh, 03Fh, 037h
	db	01Fh, 03Fh, 03Fh
	db	01Fh, 037h, 03Fh
	db	01Fh, 02Fh, 03Fh
	db	01Fh, 027h, 03Fh
	db	02Dh, 02Dh, 03Fh
	db	031h, 02Dh, 03Fh
	db	036h, 02Dh, 03Fh
	db	03Ah, 02Dh, 03Fh
	db	03Fh, 02Dh, 03Fh
	db	03Fh, 02Dh, 03Ah
	db	03Fh, 02Dh, 036h
	db	03Fh, 02Dh, 031h
	db	03Fh, 02Dh, 02Dh
	db	03Fh, 031h, 02Dh
	db	03Fh, 036h, 02Dh
	db	03Fh, 03Ah, 02Dh
	db	03Fh, 03Fh, 02Dh
	db	03Ah, 03Fh, 02Dh
	db	036h, 03Fh, 02Dh
	db	031h, 03Fh, 02Dh
	db	02Dh, 03Fh, 02Dh
	db	02Dh, 03Fh, 031h
	db	02Dh, 03Fh, 036h
	db	02Dh, 03Fh, 03Ah
	db	02Dh, 03Fh, 03Fh
	db	02Dh, 03Ah, 03Fh
	db	02Dh, 036h, 03Fh
	db	02Dh, 031h, 03Fh
	db	000h, 000h, 01Ch
	db	007h, 000h, 01Ch
	db	00Eh, 000h, 01Ch
	db	015h, 000h, 01Ch
	db	01Ch, 000h, 01Ch
	db	01Ch, 000h, 015h
	db	01Ch, 000h, 00Eh
	db	01Ch, 000h, 007h
	db	01Ch, 000h, 000h
	db	01Ch, 007h, 000h
	db	01Ch, 00Eh, 000h
	db	01Ch, 015h, 000h
	db	01Ch, 01Ch, 000h
	db	015h, 01Ch, 000h
	db	00Eh, 01Ch, 000h
	db	007h, 01Ch, 000h
	db	000h, 01Ch, 000h
	db	000h, 01Ch, 007h
	db	000h, 01Ch, 00Eh
	db	000h, 01Ch, 015h
	db	000h, 01Ch, 01Ch
	db	000h, 015h, 01Ch
	db	000h, 00Eh, 01Ch
	db	000h, 007h, 01Ch
	db	00Eh, 00Eh, 01Ch
	db	011h, 00Eh, 01Ch
	db	015h, 00Eh, 01Ch
	db	018h, 00Eh, 01Ch
	db	01Ch, 00Eh, 01Ch
	db	01Ch, 00Eh, 018h
	db	01Ch, 00Eh, 015h
	db	01Ch, 00Eh, 011h
	db	01Ch, 00Eh, 00Eh
	db	01Ch, 011h, 00Eh
	db	01Ch, 015h, 00Eh
	db	01Ch, 018h, 00Eh
	db	01Ch, 01Ch, 00Eh
	db	018h, 01Ch, 00Eh
	db	015h, 01Ch, 00Eh
	db	011h, 01Ch, 00Eh
	db	00Eh, 01Ch, 00Eh
	db	00Eh, 01Ch, 011h
	db	00Eh, 01Ch, 015h
	db	00Eh, 01Ch, 018h
	db	00Eh, 01Ch, 01Ch
	db	00Eh, 018h, 01Ch
	db	00Eh, 015h, 01Ch
	db	00Eh, 011h, 01Ch
	db	014h, 014h, 01Ch
	db	016h, 014h, 01Ch
	db	018h, 014h, 01Ch
	db	01Ah, 014h, 01Ch
	db	01Ch, 014h, 01Ch
	db	01Ch, 014h, 01Ah
	db	01Ch, 014h, 018h
	db	01Ch, 014h, 016h
	db	01Ch, 014h, 014h
	db	01Ch, 016h, 014h
	db	01Ch, 018h, 014h
	db	01Ch, 01Ah, 014h
	db	01Ch, 01Ch, 014h
	db	01Ah, 01Ch, 014h
	db	018h, 01Ch, 014h
	db	016h, 01Ch, 014h
	db	014h, 01Ch, 014h
	db	014h, 01Ch, 016h
	db	014h, 01Ch, 018h
	db	014h, 01Ch, 01Ah
	db	014h, 01Ch, 01Ch
	db	014h, 01Ah, 01Ch
	db	014h, 018h, 01Ch
	db	014h, 016h, 01Ch
	db	000h, 000h, 010h
	db	004h, 000h, 010h
	db	008h, 000h, 010h
	db	00Ch, 000h, 010h
	db	010h, 000h, 010h
	db	010h, 000h, 00Ch
	db	010h, 000h, 008h
	db	010h, 000h, 004h
	db	010h, 000h, 000h
	db	010h, 004h, 000h
	db	010h, 008h, 000h
	db	010h, 00Ch, 000h
	db	010h, 010h, 000h
	db	00Ch, 010h, 000h
	db	008h, 010h, 000h
	db	004h, 010h, 000h
	db	000h, 010h, 000h
	db	000h, 010h, 004h
	db	000h, 010h, 008h
	db	000h, 010h, 00Ch
	db	000h, 010h, 010h
	db	000h, 00Ch, 010h
	db	000h, 008h, 010h
	db	000h, 004h, 010h
	db	008h, 008h, 010h
	db	00Ah, 008h, 010h
	db	00Ch, 008h, 010h
	db	00Eh, 008h, 010h
	db	010h, 008h, 010h
	db	010h, 008h, 00Eh
	db	010h, 008h, 00Ch
	db	010h, 008h, 00Ah
	db	010h, 008h, 008h
	db	010h, 00Ah, 008h
	db	010h, 00Ch, 008h
	db	010h, 00Eh, 008h
	db	010h, 010h, 008h
	db	00Eh, 010h, 008h
	db	00Ch, 010h, 008h
	db	00Ah, 010h, 008h
	db	008h, 010h, 008h
	db	008h, 010h, 00Ah
	db	008h, 010h, 00Ch
	db	008h, 010h, 00Eh
	db	008h, 010h, 010h
	db	008h, 00Eh, 010h
	db	008h, 00Ch, 010h
	db	008h, 00Ah, 010h
	db	00Bh, 00Bh, 010h
	db	00Ch, 00Bh, 010h
	db	00Dh, 00Bh, 010h
	db	00Fh, 00Bh, 010h
	db	010h, 00Bh, 010h
	db	010h, 00Bh, 00Fh
	db	010h, 00Bh, 00Dh
	db	010h, 00Bh, 00Ch
	db	010h, 00Bh, 00Bh
	db	010h, 00Ch, 00Bh
	db	010h, 00Dh, 00Bh
	db	010h, 00Fh, 00Bh
	db	010h, 010h, 00Bh
	db	00Fh, 010h, 00Bh
	db	00Dh, 010h, 00Bh
	db	00Ch, 010h, 00Bh
	db	00Bh, 010h, 00Bh
	db	00Bh, 010h, 00Ch
	db	00Bh, 010h, 00Dh
	db	00Bh, 010h, 00Fh
	db	00Bh, 010h, 010h
	db	00Bh, 00Fh, 010h
	db	00Bh, 00Dh, 010h
	db	00Bh, 00Ch, 010h
LENGTH_256COLOR_DAC_TABLE	equ	($ - tbl256ColorDAC)/3
;
;	Copyright (c) 1990-1998 Elpin Systems, Inc.
;	All rights reserved.
;


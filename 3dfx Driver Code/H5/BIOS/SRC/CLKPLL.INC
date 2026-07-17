	page	57, 132
	title	Clock PLL settings
	subttl	CLLPLL.INC - Copyright (C) 1990-1998 Elpin Systems, Inc.
;
;	CLKPLL.INC - Clock PLL settings
;
;	Written by:	Kaymann L. Woo
;	Date:		2/21/00
;
;  $Author$
;	$Revision$
;	$Date$
;	$Log$

PLL_VGACLK         equ     00000h          ; VGA Clocks
PLL_12_500MHZ      equ     01312h          ; 12.5 MHz clock
;PLL_12_500MHZ      equ     0F98Bh          ; 12.5 MHz clock
PLL_20_000MHZ      equ     0BC3Fh          ; 20 MHz
PLL_29_189MHZ		equ		03D0Bh			; 28.189 MHz
PLL_29_500MHZ		equ		0CC5Eh			; 29.500 MHz
PLL_32_500MHZ      equ     0EA2Fh          ; 32.5 MHz
PLL_36_000MHZ		equ		0AE85h			; 36.000 MHz (PAL 800x600)
PLL_38_769MHZ      equ     0DC9Dh          ; 38.769 MHz (NTSC 800x600)
PLL_40_000MHZ      equ     0BC3Eh          ; 40 MHz clock
PLL_65_000MHZ      equ     06B07h          ; 65 MHz clock
PLL_108_000MHZ     equ     0B312h          ; 108 MHz clock

//
// 	TEXT.CPP	-	Routines to load the output shift register with map data based on.
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
// 	Author:			Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
//		Routines in this file:
// 	GetTextChar		Used to load the output shift register with map data in text format.
//
#include "vgaint.h"

//
// 	GetTextChar - Used to load the output shift register with map 
//							data in text format.
//
//		Entry:	shiftreg		Pointer to 9 entry output shift register
//					address		Current scanout address in map
//					row_count	Current row counter value form CRTC row counter
//
// 	Exit:		<WORD>		Address for next character
//
WORD GetTextChar (BYTE *shift_reg, WORD address, BYTE row_count)
{
	BYTE char_bits; // contains map 0 data - character value
	BYTE attr_bits; // contains map 1 data - attribute
	BYTE font_bits; // contains map 2 data - glyph bits
	WORD font_addr; // glyph address derived from row counter, character value
						 // and character map select
	WORD sa;        // map address of font a
	WORD sb;        // map address of font b
	BYTE fg;        // foreground color for character
	BYTE bg;        // background color for character
	int i;
	BOOL bDisplayCursor;

	// Assume cursor is not on this scan line / character cell
	bDisplayCursor = FALSE;

	// Get character index
	char_bits = ReadMap (0, ScanAddressMUX (address, row_count));

	// Get character attributes
	attr_bits = ReadMap (1, ScanAddressMUX (address, row_count));

	// Generate the composit address of the character glyph
	// composit row count and character intex to get position in font
	font_addr = (((WORD)(char_bits)) << 5) | ((WORD)row_count);

	// Assemble character pointers
	sa = ((WORD)(bySEQReg[SEQ_MAP_SELECT] & FLD_MAL)) << SHFT_MAL;
	sa |= ((WORD)(bySEQReg[SEQ_MAP_SELECT] & BIT_MA2)) << SHFT_MA2;;

	sb = ((WORD)(bySEQReg[SEQ_MAP_SELECT] & FLD_MBL)) << SHFT_MBL;
	sb |= ((WORD)(bySEQReg[SEQ_MAP_SELECT] & BIT_MB2)) << SHFT_MB2;

	// Attr bit 3 is font select
	// Use font a
	if (attr_bits & 0x08)
		font_addr |= sa; 
	// Use font b
	else
  		font_addr |= sb; 

	// Get 8 bit glyph row from map 2
	font_bits = ReadMap (2, font_addr);

	// Check underline
  	// If underline, set all bits to foreground
	if (((byCRTCReg[CRTC_UNDERLINE] & FLD_SUL)== row_count) && 
	  	((attr_bits & 0x77)== 0x01))
		font_bits = 0xff;

	// Should we display cursor? is cursor blink on?
	if (byGlobalCursorBlink && (!(byCRTCReg[CRTC_CURSOR_START] & BIT_CO)) &&
	  	// Is cursor pointing to current scanout address?
   	(address == ((((WORD)byCRTCReg[CRTC_CURSOR_HIGH]) << 8) |
		((WORD)byCRTCReg[CRTC_CURSOR_LOW]))) && 
		// Is current row within cursor display range?
		(row_count >= (byCRTCReg[CRTC_CURSOR_START] & FLD_RSCB)) &&
   	(row_count <= (byCRTCReg[CRTC_CURSOR_END] & FLD_RSCE)))
			chGlobalCursorSkew = (byCRTCReg[CRTC_CURSOR_END] & FLD_CSK) >> SHFT_CSK;

	// Handle cursor skew
	if (chGlobalCursorSkew >= 0)
		// Display cursor, set all bits to foreground
		if (chGlobalCursorSkew-- == 0)
		{
			bDisplayCursor = TRUE;
			font_bits = 0xff;
		}

	// Get foreground
	fg = attr_bits & 0x0f;

	// Blink enabled ?
	if (byATCReg[ATC_MODE] & BIT_EB)
	{
		// 3 bit background
		bg = (attr_bits >> 4) & 0x07;

		if ((!byGlobalAttrBlink) && (attr_bits & 0x80) && (!bDisplayCursor))
			fg = bg;
	}
	else
  		// 4 bit background
		bg = (attr_bits >> 4) & 0x0F;

		// Map font bits to colors
	for (i = 0; i < 8; i++)
		if (font_bits & (1 << (7 - i)))		// Bit set - color it foreground
			shift_reg[i] = fg;
		else											// Bit clear - color it background
			shift_reg[i] = bg;

	// If 9 bits per character
	if (!(bySEQReg[SEQ_CLOCKING] & BIT_D89))
	{
		// Extended line graphics ?
		if (byATCReg[ATC_MODE] & BIT_ELG)
		{
			// Extended line characters ?
			// Yes, extend them
			if((char_bits>= 0xc0) && (char_bits < 0xe0))
				shift_reg[8] = shift_reg[7];
			// Fill extra bit with background
			else
				shift_reg[8] = bg;
		}
		// Fill extra bit with background
		else
			shift_reg[8] = bg;
	}
	return (NextScanAddr (address));
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

//
//		GRAPH.CPP - Routines to load the output shift register with map data based on the graphic display mode.
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Author:			Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
// 	Routines in this file:
// 	GetCGAChar		Used to load the output shift register with map data in CGA format
//		GetEGAChar		Used to load the output shift register with map data in EGA format
//		GetVGAChar		Used to load the output shift register with map data in VGA format
//
#include "vgaint.h"

//
// 	GetCGAChar - Used to load the output shift register with map data in cga format
//
//		Entry:	bpShiftReg	Pointer to 9 entry output shift register
//					wAddress		Current scanout address in map
//					byRowCount	Current row counter value form CRTC row counter
//		Exit:		<WORD>		Address for next character
//
//  	CGA Notes:	
//									1) Always returns 8 pixels per character
//									2) 2 bits per pixel arranged linearly in maps 0 & 1
//
WORD GetCGAChar (BYTE *bpShiftReg, WORD wAddress, BYTE byRowCount)
{
	BYTE byData;

	// Get 2 maps/8 pixels worth of data
	byData = ReadMap (0, ScanAddressMUX (wAddress, byRowCount));
	
	bpShiftReg[0] = (byData & 0xc0) >> 6;
	bpShiftReg[1] = (byData & 0x30) >> 4;
	bpShiftReg[2] = (byData & 0x0c) >> 2;
	bpShiftReg[3] = byData & 0x03;

	byData = ReadMap (1, ScanAddressMUX (wAddress, byRowCount));

	bpShiftReg[4] = (byData & 0xc0) >> 6;
	bpShiftReg[5] = (byData & 0x30) >> 4;
	bpShiftReg[6] = (byData & 0x0c) >> 2;
	bpShiftReg[7] = byData & 0x03;

	byData = ReadMap (2, ScanAddressMUX (wAddress, byRowCount));

	bpShiftReg[0] |= (byData & 0xc0) >> 4;
	bpShiftReg[1] |= (byData & 0x30) >> 2;
	bpShiftReg[2] |= byData & 0x0c;
	bpShiftReg[3] |= (byData & 0x03) << 2;

	byData = ReadMap (3, ScanAddressMUX (wAddress, byRowCount));

	bpShiftReg[4] |= (byData & 0xc0) >> 4;
	bpShiftReg[5] |= (byData & 0x30) >> 2;
	bpShiftReg[6] |= byData & 0x0c;
	bpShiftReg[7] |= (byData & 0x03) << 2;

	return (NextScanAddr (wAddress));
}

//
//		GetEGAChar - Used to load the output shift register with map data in ega format
//
// 	Entry:	bpShiftReg		Pointer to 9 entry output shift register
//					wAddress			Current scanout address in map
//					byRowCount		Current row counter value form CRTC row counter
// 	Exit:		<WORD>			Address for next character
//
//		EGA Notes:
//						1) Always returns 8 pixels per character
//						2) 4 bits per pixel arranged planar in maps 0,1,2 & 3
//
WORD GetEGAChar (BYTE *bpShiftReg, WORD wAddress, BYTE byRowCount)
{
	WORD byData0, byData1, byData2, byData3;
	int i;
	BYTE byData;

	// Get 4 maps/8 pixels worth of data
	byData0 = (WORD)ReadMap (0, ScanAddressMUX (wAddress, byRowCount));
	byData1 = ((WORD)ReadMap (1, ScanAddressMUX (wAddress, byRowCount))) << 1;
	byData2 = ((WORD)ReadMap (2, ScanAddressMUX (wAddress, byRowCount))) << 2;
	byData3 = ((WORD)ReadMap (3, ScanAddressMUX (wAddress, byRowCount))) << 3;

	// Convert planar data to byte data in shift register for 8 pixels
	for (i = 7; i >= 0; i--)
	{
		byData = ((BYTE)(((byData0 & 0x080)| (byData1 & 0x100)| (byData2 & 0x200)|
				 (byData3&0x400)) >> 7));
		bpShiftReg[7-i] = byData;
		byData0 = byData0 << 1;
		byData1 = byData1 << 1;
		byData2 = byData2 << 1;
		byData3 = byData3 << 1;
	}
	return (NextScanAddr (wAddress));
}

//
//		GetVGAChar - Used to load the output shift register with map data in vga format
//	
//		Entry:	bpShiftReg	Pointer to 9 entry output shift register
//					wAddress		Current scanout address in map
//					byRowCount	Current row counter value form CRTC row counter
// 	Exit:		<WORD>		Address for next character
//
//
//		EGA Notes:
//			1) Primarily used in Mode 13
//			2) Always returns 8 pixels per character
//			3) 8 bits per pixel, 1 complete pixel per map
//			4) Since the shift register is only 4 bits deep the 8 bit values
//				are stored in the shift register as 4MSB followed by 4LSB
//			5) The scanout logic reassembles the 8 bit data after the shift 
//				register producing 1 8 bit pixel per 2 pixel clock cycles.  The
//		   	output logic compensates by duplication the pixels.  This produces
//				8 pixels (1 per pixel clock) but only 4 UNIQUE pixels per character
//				time.  The wierd packing also creates strange artifacts if non-
//				default values are loaded in the palettes.
//
WORD GetVGAChar(BYTE *bpShiftReg, WORD wAddress, BYTE byRowCount)
{
	BYTE byData;

	// Get 4 maps/8 pixels worth of data
	byData = ReadMap (0, ScanAddressMUX (wAddress, byRowCount));
	// Double clock for vga mode
	bpShiftReg[0] = byData >> 4;
	bpShiftReg[1] = byData & 0xf;

	byData = ReadMap (1, ScanAddressMUX (wAddress, byRowCount));
	// Double clock for vga mode
	bpShiftReg[2] = byData >> 4;
	bpShiftReg[3] = byData & 0xf;

	byData = ReadMap (2,ScanAddressMUX (wAddress, byRowCount));
	// Double clock for vga mode
	bpShiftReg[4] = byData >> 4;
	bpShiftReg[5] = byData & 0xf;

	byData = ReadMap (3,ScanAddressMUX (wAddress, byRowCount));
	// Double clock for vga mode
	bpShiftReg[6] = byData >> 4;
	bpShiftReg[7] = byData & 0xf;

	return (NextScanAddr (wAddress));
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

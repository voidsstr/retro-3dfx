//
// 	READMODE.CPP - Global Read registers
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
// 	Author:			Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
//		Routines in this file:
//		ReadMode0	Processes map data at the specified address and returns a byte to the CPU
//		ReadMode1	Processes map data at the specified address and returns a byte to the CPU
//
#include "vgaint.h"

// 32 bit read register for read and write modes
BYTE read_reg[4];

//
//		ReadMode0 - Processes map data at the specified address and returns 
//						a byte to the CPU.
//
// 	Entry:	dwAddress	0-64K map address
//			  		bySkipRead	Read the address or not
// 	Exit:		<BYTE>		Byte data at address
//
BYTE ReadMode0 (DWORD dwAddress, BYTE bySkipRead)
{
	// Map address after passing thru address MUX 
	WORD wMuxAddr;

	if (bySkipRead)
    wMuxAddr = (WORD)dwAddress;
	else
	{
		// Apply the address mux to the map address
		wMuxAddr = MemAddressMux (dwAddress);

		// Read the maps
		read_reg[0] = ReadMap (0, wMuxAddr);
		read_reg[1] = ReadMap (1, wMuxAddr);
		read_reg[2] = ReadMap (2, wMuxAddr);
		read_reg[3] = ReadMap (3, wMuxAddr);
	}
  
	// Chain 4 - 2 lsb determine which map to read
	if(bySEQReg[SEQ_MEM_MODE]&BIT_CH4)
		return (read_reg[(WORD)(dwAddress & 0x3L)]);
	else if (!(byGDCReg[GDC_MODE] & BIT_GDC_OE))
	{
		// Chain 1 - map select determines which map to read
		switch (byGDCReg[GDC_MAP_SEL] & FLD_MS)
		{
			case FLD_MS_0:
			  	return (read_reg[0]);
			case FLD_MS_1:
			  	return (read_reg[1]);
			case FLD_MS_2:
			  	return (read_reg[2]);
			case FLD_MS_3:
			  	return (read_reg[3]);
		}
	}
	else
	{
		// Chain 2 - both maps select and lsb determines map to read
		switch (byGDCReg[GDC_MAP_SEL] & FLD_MS)
		{
			case FLD_MS_0:
			case FLD_MS_1:
				return (read_reg[(WORD)(dwAddress & 0x1L)]);

			case FLD_MS_2:
			case FLD_MS_3:
				return (read_reg[(WORD)(dwAddress & 0x1L) + 0x2]);
		}
	}
	return (OPEN_BUS);
}

//
//		ReadMode1 - Processes map data at the specified address and returns
//						a byte to the CPU.
//
//		Entry:	dwAddress	0-64K map address
//					bySkipRead	Skip the read or not
//		Exit:		<BYTE>		Byte data at address
//
BYTE ReadMode1 (DWORD dwAddress, BYTE bySkipRead)
{
	WORD wMuxAddr;		// map address after passing thru address mux
	BYTE byColor;		// 4 bit color used for color compare
	BYTE byDontCare;	// 4 bit don't care field for color compare
	BYTE byRslt;		// 8 bit result of compare
	BYTE byComposit;	// 4 bit planar data derived from maps
	int i;

	if (bySkipRead)
		wMuxAddr = (WORD)dwAddress;
	else
	{
		// Apply the address mux to the map address
		wMuxAddr = MemAddressMux(dwAddress);
		
		// Read the maps
  		read_reg[0] = ReadMap (0, wMuxAddr);
  		read_reg[1] = ReadMap (1, wMuxAddr);
  		read_reg[2] = ReadMap (2, wMuxAddr);
  		read_reg[3] = ReadMap (3, wMuxAddr);
	}

	// Get 4 bit color
	byColor = byGDCReg[GDC_COLOR_CMP] & FLD_CC;

	// Get 4 bit don't care
	byDontCare = byGDCReg[GDC_COLOR_DC] & FLD_MX;

	// Force bits to be ignored to 0 in compared color
	byColor &= byDontCare;

	byRslt = 0;

	for (i = 0; i < 8; i++)
	{
		// Assemble planar color for each of 8 pixels
		byComposit = (read_reg[0] >> i) & 0x1;
		byComposit |= (BYTE)(((((WORD)read_reg[1]) << 1) >> i) & 0x2);
		byComposit |= (BYTE)(((((WORD)read_reg[2]) << 2) >> i) & 0x4);
		byComposit |= (BYTE)(((((WORD)read_reg[3]) << 3) >> i) & 0x8);

		// Force bits to be ignored to 0 in map data
		byComposit &= byDontCare;

		// If 4 bit value equal to color - set bit in result
		if (byComposit == byColor)
			byRslt |= (1 << i);
	}
	
	return (byRslt);
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

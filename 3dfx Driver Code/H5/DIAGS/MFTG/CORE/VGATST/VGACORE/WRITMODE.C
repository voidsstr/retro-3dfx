//
// 	WRITEMODE.CPP - Routines to handle write modes
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Author:			Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
//		Routines in this file:
// 	WriteMode0		Processes the byte written by the CPU and updates. 
// 	WriteMode1		Processes the byte written by the CPU and updates. 
//		WriteMode2		Processes the byte written by the CPU and updates. 
//		WriteMode3		Processes the byte written by the CPU and updates.
//		GDCRotate		Rotates byte data by 0-7 bits.
// 	GDCSetReset		Performs GDC set/reset functionality and loads result into 
//							32 bit write register.
// 	GDC_ALU			Perform GDC ALU operation between value in the 32 bit write
//							register and the 32 bit read register.
// 	GDCWrite			Write the contents of the write register to the 
//							appropriate maps.
//		MemAddressMux	Modify the VGA address for chaining.
//
#include "vgaint.h"

// 32 bit read register for read and write modes
//	declared in 'READMODE.CPP'
extern BYTE read_reg[4];

//
//		WriteMode0 - Processes the byte written by the CPU and updates 
//						 the VGA maps.
//
//		Entry:	dwAddress		0-64K map address
//					byData			Data to write 
//		Exit:		None
//
void WriteMode0 (DWORD dwAddress, BYTE byData)
{

	BYTE byWDat[4];	// 32 bit write data
	BYTE byMsk;			// 8 bit write mask

	// Barrel shift data down by up to 7 bits
	byData = GDCRotate (byData);

	// Apply set/reset function
	GDCSetReset (byWDat, byData, byGDCReg[GDC_SET_RESET], byGDCReg[GDC_ENA_SR]);

	// Apply ALU operation
	GDC_ALU (byWDat);

	// Get bit mask
	byMsk = byGDCReg[GDC_BIT_MASK];

	// Write data to map and combine with read data using bit mask
	GDCWrite (byWDat, dwAddress, byMsk);
}

//
//		WriteMode1 - Processes the byte written by the CPU and updates 
//						 the VGA maps.
//
//		Entry:	dwAddress	0-64K map address
//					byData		Data to write 
//		Exit:		None
//
void WriteMode1 (DWORD dwAddress, BYTE byData)
{
	BYTE byDummy[4];

	// Prevent compiler warnings
	byData = byData;

	// Since mask is implicitly 0 - don't need to calculate:
   //	  1.	Rotate
   //	  2.	Set/Reset
   //	  3.	ALU
	GDCWrite (byDummy, dwAddress, 0x00);
}

//
//		WriteMode2 - Processes the byte written by the CPU and updates 
//						 the VGA maps.
//
//		Entry:	dwAddress	0-64K map address
//					byData		Data to write
//		Exit:		None
//
void WriteMode2 (DWORD dwAddress, BYTE byData)
{

	BYTE byWDat[4];
	BYTE byMsk;

	// Apply Set/Reset function - acts like SR enable is true
	GDCSetReset (byWDat, 0x00, byData, 0x0F);

	// Apply ALU operation
	GDC_ALU (byWDat);

	byMsk = byGDCReg[GDC_BIT_MASK];

	// Apply the bit mask
	GDCWrite (byWDat, dwAddress, byMsk);
}

//
//		WriteMode3 - Processes the byte written by the CPU and updates
//						 the VGA maps.
//
//		Entry:	dwAddress	0-64K map address
//					byData		Data to write 
//		Exit:		None
//
void WriteMode3 (DWORD dwAddress, BYTE byData)
{
	BYTE byWDat[4];
	BYTE byMsk;

	// Rotate the data
	byData = GDCRotate (byData);

	// Apply Set/Reset function - acts like data is SR and enable is true
	GDCSetReset (byWDat, 0x00, byGDCReg[GDC_SET_RESET], 0x0F);

	// Apply ALU operation
	GDC_ALU (byWDat);

	// Combine mask with shifed data
	// this uses rotated data
	byMsk = byGDCReg[GDC_BIT_MASK] & byData;

	// Apply the bit mask
	GDCWrite (byWDat, dwAddress, byMsk);
}

//
//		GDCRotate - Rotates byte data by 0-7 bits
//
//		Entry:	byData	Data to rotate 
//		Exit:		<BYTE>	Rotated data
//
BYTE GDCRotate (BYTE byData)
{
	int	i;
	BYTE	byRot;				// Rotate count

	// Get the rotate count
	byRot = byGDCReg[GDC_DATA_ROT] & FLD_RC; 

	// A quick and dirty rotate
	for (i = 0; i < byRot; i++)
		byData = (BYTE) (((byData & 0x01) << 7) | ((BYTE) (byData >> 1)));
#if 0
	_asm 
	{
		mov cl,[byRot]
  		ror [byData],cl
	}
#endif

	return (byData);
}

//
//		GDCSetReset - Performs GDC set/reset functionality and loads result into 
//						  32 bit write register.
//
//		Entry:	byWReg	Pointer to 32 bit write register
//					byData	Data to write 
//					bySetRes	4 bit Set/Reset value
//					byEnable	4 bit Set/Reset enable
//		Exit:		None
//
void GDCSetReset (BYTE *byWReg, BYTE byData, BYTE bySetRes, BYTE byEnable)
{
	// 4 bit test mask
	BYTE byMask; 
  	int i;

	// Repeat for each byte
	for (i = 0; i < 4; i++)
	{
  	 	byMask = 1 << i;
		// Do Set/Reset
		if (byEnable & byMask)
		{
			// If enabled and set - set byte to all 1s
			if (bySetRes & byMask)
				byWReg[i] = 0xFF;
			
			// If enabled and reset - set byte to all 0s 
			else
				byWReg[i] = 0x00;
  		}
    	// If disabled - copy 8 bit data to byte
		else
			byWReg[i] = byData;
	}
}

//
//		GDC_ALU - Perform GDC ALU operation between value in the 32 bit write
//					 register and the 32 bit read register.
//
//		Entry:	byWReg	Pointer to 32 bit write register
//		Exit:		None
//
void GDC_ALU (BYTE *byWReg)
{
	int i;

	// Repeat for each byte
	for (i = 0; i < 4; i++)
	{
		// Logic operation
		switch (byGDCReg[GDC_DATA_ROT] & FLD_FS)
		{
			// NOOP
			case FLD_FS_NOOP:
				break;
			
			// Logical AND
			case FLD_FS_AND:
				byWReg[i] &= read_reg[i];
				break;

			// Logical OR
			case FLD_FS_OR:
				byWReg[i] |= read_reg[i];
				break;

			// Logical XOR
			case FLD_FS_XOR:
				byWReg[i] ^= read_reg[i];
				break;
		}
	}
}

//
//		GDCWrite - Write the contents of the write register to the 
//					  appropriate maps.
//
//		Entry:	byWDat		Pointer to 32 bit write data
//					dwAddress	0-64K map address
//					byMask		8 bit write mask (selects between wdat and read reg)
//		Exit:		None
//
void GDCWrite (BYTE *byWDat, DWORD dwAddress, BYTE byMask)
{

	WORD wMuxAddr;	// Map address after passing thru address mux

	// Map the address
	wMuxAddr = MemAddressMux (dwAddress);

	// Apply the bit mask and chaining
	if (bySEQReg[SEQ_MEM_MODE] & BIT_CH4)
	{
		// Chain 4
		WriteMap ((BYTE) (dwAddress & 0x3L),
					wMuxAddr,
					(read_reg[(WORD)(dwAddress & 0x3L)] &
					(~byMask)) | (byWDat[(WORD)(dwAddress & 0x3L)] & byMask));
	}
	else if (bySEQReg[SEQ_MEM_MODE] & BIT_SEQ_OE)
	{
		// Chain 1
		WriteMap (0, wMuxAddr, ((read_reg[0] & (~byMask)) | (byWDat[0] & byMask)));
		WriteMap (1, wMuxAddr, ((read_reg[1] & (~byMask)) | (byWDat[1] & byMask)));
		WriteMap (2, wMuxAddr, ((read_reg[2] & (~byMask)) | (byWDat[2] & byMask)));
		WriteMap (3, wMuxAddr, ((read_reg[3] & (~byMask)) | (byWDat[3] & byMask)));
	}
	else
	{
		// Chain 2
		switch (dwAddress & 0x1)
		{
			case 0:
				WriteMap (0, wMuxAddr, (read_reg[0] & (~byMask)) | (byWDat[0] & byMask));
				WriteMap (2, wMuxAddr, (read_reg[2] & (~byMask)) | (byWDat[2] & byMask));
				break;

			case 1:
				WriteMap (1, wMuxAddr, (read_reg[1] & (~byMask)) | (byWDat[1] & byMask));
				WriteMap (3, wMuxAddr, (read_reg[3] & (~byMask)) | (byWDat[3] & byMask));
				break;
		}
	}
}

//
//		MemAddressMux - Modify the VGA address for chaining
//
// 	Entry:	dwAddress	0-128K linear memory address
// 	Exit:		<WORD>		0-64K address modified for chaining
//
WORD MemAddressMux (DWORD dwAddress)
{
	WORD wMuxAddr;

	
	// Chain 4
	if (bySEQReg[SEQ_MEM_MODE] & BIT_CH4)
		wMuxAddr = (WORD)((dwAddress & 0xFFFCL) | ((dwAddress >> 14) & 0x3));
	
	// Normal
	else if (!(byGDCReg[GDC_MISC] & BIT_OE))
		wMuxAddr = (WORD)(dwAddress & 0xFFFFL);
	else
	{
		if ((byGDCReg[GDC_MISC] & FLD_MM) == FLD_MM_A0BF)
		{
			// Chain 2 - 128K
			// Extended memory
			if (bySEQReg[SEQ_MEM_MODE] & BIT_EM)
				wMuxAddr = (WORD)((dwAddress & 0xFFFEL) | ((dwAddress >> 16) & 0x1L));
			// No extended memory
			else
				wMuxAddr = (WORD)(dwAddress & 0xFFFFL);
		}
		else
		{
			// Chain 2 - non 128K
			if (bySEQReg[SEQ_MEM_MODE] & BIT_EM)
			{
				// Extended memory
				// wMuxAddr0 = NOT Page Select
				if (byMiscReg & BIT_PSEL)
					wMuxAddr = (WORD)(dwAddress & 0xFFFEL);
				else
					wMuxAddr = (WORD)((dwAddress & 0xFFFEL) | 0x1L);
			}
			else
				// No extended memory
				wMuxAddr = (WORD)((dwAddress & 0xFFFEL) | ((dwAddress >> 14) & 0x1L));
		}
	}
	return (wMuxAddr);
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

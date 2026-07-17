//
// 	MEM.CPP - Routines to read and write VGA memory
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Author:			Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
// 	Routines in this file:
// 	MemLinRead	Reads VGA memory at a given 20 bit linear address
// 	MemLinWrite	Writes VGA memory at a given 20 bit linear address
// 	PtrToLin		Converts segment:offset address to 20 bit linear address
//
#include "vgaint.h"

//
// 	MemLinRead - Reads VGA memory at a given 20 bit linear address
//
// 	Entry:	dwAddress	20 bit linear address to read from
// 	Exit:		<BYTE>			byte data at address
//
BYTE MemLinRead(DWORD dwAddress)
{
	// First 0x500 locations in system memory map to fake BIOS
	if (dwAddress < 0x500L)
		return (ReadBIOS ((WORD) dwAddress));

  	// Is ram enabled?
	if ((byMiscReg & BIT_ERAM)&& (bySEQReg[SEQ_RESET] & BIT_SEQ_SR) &&
		(bySEQReg[SEQ_RESET] & BIT_ASR))
	{
		// Convert system bus address to VGA address based on memory mapping
		switch(byGDCReg[GDC_MISC]&FLD_MM)
		{
      	
			// VGA memory mapped to 0xA0000 - 0xBFFFF
			case FLD_MM_A0BF:

				if ((dwAddress >= 0xA0000L) && (dwAddress < 0xC0000L))
					dwAddress -= 0xA0000;
				// Return open bus for out of range
				else
					return (OPEN_BUS);
				break;


			// VGA memory mapped to 0xA0000 - 0xAFFFF
			case FLD_MM_A0AF:

				if ((dwAddress >= 0xA0000L) && (dwAddress < 0xB0000L))
					dwAddress -= 0xA0000;
				// Return open bus for out of range
				else
					return (OPEN_BUS);
				break;

			// VGA memory mapped to 0xB0000 - 0xB7FFF
			case FLD_MM_B0B7:

				if ((dwAddress >= 0xB0000L) && (dwAddress < 0xB8000L))
					dwAddress -= 0xB0000;
				// Return open bus for out of range
				else
					return (OPEN_BUS);
				break;

			// VGA memory mapped to 0xB8000 - 0xBFFFF
			case FLD_MM_B8BF:

				if((dwAddress >= 0xB8000L) && (dwAddress < 0xC0000L))
					dwAddress -= 0xB8000;
				// Return open bus for out of range
				else
					return (OPEN_BUS);
				break;

			default:
				
				return (OPEN_BUS);
		}
		
		// Call selected read mode
		if (byGDCReg[GDC_MODE] & BIT_RM)
			return (ReadMode1 (dwAddress, 0));
		else
			return (ReadMode0 (dwAddress, 0));
  	}
	// Ram disabled - ignore
	else
		return (OPEN_BUS);
}

//
// 	MemLinWrite - Writes VGA memory at a given 20 bit linear address
//
//		Entry:	dwAddress	20 bit linear address to read from
//					byData		Byte data to write to address
//		Exit:		None
//
void MemLinWrite (DWORD dwAddress, BYTE byData)
{

	// First 0x500 locations in system memory map to fake BIOS
	if (dwAddress < 0x500L)
	{
		WriteBIOS ((WORD)dwAddress, byData);
		return;
	}

	// Is ram enabled ?
 if ((byMiscReg & BIT_ERAM) && (bySEQReg[SEQ_RESET] & BIT_SEQ_SR) && 
		(bySEQReg[SEQ_RESET] & BIT_ASR))
	{
		  printf("yes ram enabled");
		// Convert system bus address to VGA address based on memory mapping
		switch(byGDCReg[GDC_MISC] & FLD_MM)
		{
			// VGA memory mapped to 0xA0000 - 0xBFFFF
			case FLD_MM_A0BF:
			 printf("dwaddr = %x A0BF", dwAddress);
				if ((dwAddress >= 0xA0000L) && (dwAddress < 0xC0000L))
					dwAddress -= 0xA0000;
				else
					return;
				break;


			// VGA memory mapped to 0xA0000 - 0xAFFFF
			case FLD_MM_A0AF:
				 printf("dwaddr = %x A0AF", dwAddress);

				if ((dwAddress >= 0xA0000L) && (dwAddress < 0xB0000L))
					dwAddress -= 0xA0000;
				else
					return;
				break;


      	// VGA memory mapped to 0xB0000 - 0xB7FFF
			case FLD_MM_B0B7:
				printf("dwaddr = %x B0B7", dwAddress);

				if ((dwAddress >= 0xB0000L) && (dwAddress < 0xB8000L))
					dwAddress -= 0xB0000;
				else
					return;
      		break;

			// VGA memory mapped to 0xB8000 - 0xBFFFF
			case FLD_MM_B8BF:
				printf("dwaddr = %x B8BF", dwAddress);

				if ((dwAddress >= 0xB8000L) && (dwAddress < 0xC0000L))
					dwAddress -= 0xB8000;
				else
					return;
				break;

			default:
				return;
		}

		// Call selected write mode
		switch (byGDCReg[GDC_MODE] & FLD_WM)
		{
			case FLD_WM_0:

				WriteMode0 (dwAddress,byData);
				break;

			case FLD_WM_1:

				WriteMode1 (dwAddress, byData);
				break;

			case FLD_WM_2:

				WriteMode2 (dwAddress, byData);
				break;

			case FLD_WM_3:

				WriteMode3 (dwAddress, byData);
		   	break;
		}
		
		
	}	//else{
   //	printf ("miscreg=%x seqreg=%x seqregasr=%x",(byMiscReg & BIT_ERAM), (bySEQReg[SEQ_RESET] & BIT_SEQ_SR), 
	 //	(bySEQReg[SEQ_RESET] & BIT_ASR));

		 
	
   //	}
}

//
// 	PtrToLin - Converts segment:offset address to 20 bit linear address
//
//		Entry:	lpbyAddr		Segment:Offset address
// 	Exit:		<DWORD>		20 bit linear address
//
DWORD PtrToLin (SEGOFF lpbyAddr)
{
	DWORD dwRetAddr;

	dwRetAddr = (((DWORD)lpbyAddr) & 0xFFFF);
	dwRetAddr += (((DWORD)lpbyAddr) & 0xFFFF0000) >> 12;

	return (dwRetAddr);
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

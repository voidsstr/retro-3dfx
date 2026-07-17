//
// 	MAPS.CPP	-	Global definitions for map and simulated bios storage.
//						Routines to allocate, free, read and write simulated memory.
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Author:		  	Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
// 	Routines in this file:
//		InitMaps		Used to initially allocate storage for maps and bios memory
//		FreeMaps		Used to free storage for maps and bios memory
// 	ReadBIOS		Read a byte from the simulated bios memory mapped from 0x000 to 0x500
//		WriteBIOS	Read a byte to the simulated bios memory mapped from 0x000 to 0x500
//		ReadMap		Read a byte from the specified memory map 
//		WriteMap		Write a byte to the specified memory map 
//
#include "vgaint.h"

//
//	Note:	The 64K byte maps are allocated as two seperate 32K bytes maps
//			due to the inability of large models to deal with 64K continous
//			chunks.
//
//
LPBYTE	bfpMap00, bfpMap01;	// Pointers for map 0
LPBYTE	bfpMap10, bfpMap11;	// Pointers for map 1
LPBYTE	bfpMap20, bfpMap21;	// Pointers for map 2
LPBYTE	bfpMap30, bfpMap31;	// Pointers for map 3
LPBYTE	bfpBIOSMem;				// Pointer for 0x0000 - 0x0500 BIOS memory area

//
//
//		InitMaps -	Used to initially allocate storage for maps and bios memory
//
//		Entry:	None
//		Exit:		<BOOL>	0 if success, -1 if failure
//
int InitMaps ()
{
	if((bfpMap00 = AllocateMemory (0x8000)) == NULL)
  		return (-1);
	if((bfpMap01 = AllocateMemory (0x8000)) == NULL)
  		return (-1);
	if((bfpMap10 = AllocateMemory (0x8000)) == NULL)
  		return (-1);
	if((bfpMap11 = AllocateMemory (0x8000)) == NULL)
  		return (-1);
	if((bfpMap20 = AllocateMemory (0x8000)) == NULL)
  		return (-1);
	if((bfpMap21 = AllocateMemory (0x8000)) == NULL)
  		return (-1);
	if((bfpMap30 = AllocateMemory (0x8000)) == NULL)
  		return (-1);
	if((bfpMap31 = AllocateMemory (0x8000)) == NULL)
  		return (-1);
	if((bfpBIOSMem = AllocateMemory (0x500)) == NULL)
  		return (-1);

	return (0);
}

//
//		FreeMaps - Used to free storage for maps and bios memory
//
//		Entry: None
//		Exit:	 None
//
void FreeMaps ()
{
	FreeMemory (bfpMap00);
	FreeMemory (bfpMap01);
	FreeMemory (bfpMap10);
	FreeMemory (bfpMap11);
	FreeMemory (bfpMap20);
	FreeMemory (bfpMap21);
	FreeMemory (bfpMap30);
	FreeMemory (bfpMap31);
	FreeMemory (bfpBIOSMem);
}

//
// 	ReadBIOS - Read a byte from the simulated bios memory mapped from 0x000 to 0x500
//
// 	Entry:	wAddress		Memory address in the range 0x000 - 0x500
//		Exit:		BYTE			Byte stored at specified address
//
BYTE ReadBIOS (WORD wAddress)
{
	return (bfpBIOSMem[wAddress]);
}

//
//		WriteBIOS - Read a byte to the simulated bios memory mapped from 0x000 to 0x500
//
// 	Entry:	wAddress		Memory address in the range 0x000 - 0x500
//			  		byData		Data to write to memory
// 	Exit:		None
//
void WriteBIOS (WORD wAddress, BYTE byData)
{
	bfpBIOSMem[wAddress] = byData;
}

//
//		ReadMap - Read a byte from the specified memory map 
//
//		Entry:	byMapno	Map number in the range of 0-3
//					wAddress	Memory address in the range 0-64K
// 	Exit:		BYTE		Data at the specicied location in the specified map
//
BYTE ReadMap (BYTE byMapno, WORD wAddress)
{
	// Select the map
	switch (byMapno)
	{
		case 0:

  			// Map is split due to memory limitations
  			if (wAddress & 0x8000)
    			return (bfpMap01[wAddress & 0x7FFF]);
			else
				return (bfpMap00[wAddress]);

		case 1:
		
			if (wAddress & 0x8000)
				return (bfpMap11[wAddress & 0x7FFF]);
			else
				return (bfpMap10[wAddress]);

		case 2:

  			if (wAddress & 0x8000)
				return (bfpMap21[wAddress & 0x7FFF]);
			else
				return (bfpMap20[wAddress]);

		case 3:
		
			if(wAddress & 0x8000)
				return(bfpMap31[wAddress&0x7FFF]);
			else
				return(bfpMap30[wAddress]);

		default:
		
			// Invalid map - return open bus
			return (OPEN_BUS);
	}
}

//
//		WriteMap -	Write a byte to the specified memory map 
//
//		Entry:	byMapno		Map number in the range of 0-3
//					wAddress		Memory address in the range 0-64K
//					byData		Data to write to memory
// 	Exit:		None
//
void WriteMap (BYTE byMapno, WORD wAddress, BYTE byData)
{

	// Select map
	switch (byMapno)
	{

		case 0:

			// Check to see if map is enabled
			if (bySEQReg[SEQ_MAP_MASK] & BIT_M0E)
			{
				// Map is split due to memory limitations
				if (wAddress & 0x8000)
					bfpMap01[wAddress & 0x7FFF] = byData;
				else
					bfpMap00[wAddress] = byData;
    		}
    		break;

		case 1:

			if (bySEQReg[SEQ_MAP_MASK] & BIT_M1E)
			{
				if(wAddress & 0x8000)
					bfpMap11[wAddress & 0x7FFF] = byData;
				else
					bfpMap10[wAddress] = byData;
			}
    		break;

		case 2:
    	
			if (bySEQReg[SEQ_MAP_MASK] & BIT_M2E)
			{
				if (wAddress & 0x8000)
					bfpMap21[wAddress&0x7FFF] = byData;
				else
					bfpMap20[wAddress] = byData;
			}
    		break;

		case 3:

			if (bySEQReg[SEQ_MAP_MASK] & BIT_M3E)
			{
				if (wAddress & 0x8000)
					bfpMap31[wAddress & 0x7FFF] = byData;
				else
					bfpMap30[wAddress] = byData;
			}
			break;

		default:
			// Invalid map - ignore
			;
	}
}

//
//		AllocateMemory - Allocate a requested block of memory
//
//		Entry:	dwSize	Size of memory block requested
//		Exit:		<LPBYTE>	Pointer to allocated block (NULL = Error)
//
LPBYTE AllocateMemory (DWORD dwSize)
{
#ifdef __MSVC16__
	return ((LPBYTE) _fmalloc ((WORD) dwSize));
#else
	return ((LPBYTE) malloc (dwSize));
#endif
}

//
//		FreeMemory - Free an allocated block of memory
//
//		Entry:	lpby		Pointer to previously allocated memory block
//		Exit:		None
//
void FreeMemory (LPBYTE lpby)
{
#ifdef __MSVC16__
	_ffree (lpby);
#else
	free (lpby);
#endif
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

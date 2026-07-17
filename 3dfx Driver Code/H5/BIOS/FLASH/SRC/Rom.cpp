//
//		ROM.CPP - ROM interface routines for FLASH.EXE
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/7/98
//		Last modified:	2/8/99
//
//		Routines in this file:
//		BeginROM				Start the ROM programming sequence
//		EndROM				Terminate the ROM programming sequence
//		ROMByteWrite		Write a BYTE to the ROM address space
//		ROMByteRead			Read a BYTE from the ROM
//		FlushBus				Flush the bus by doing two I/O reads
//		GetROMID				Return the ROM manufacturer and device ID
//		SendROMCommand		Send a command to the ROM
//		SaveROM				Save the current ROM image to a binary image file
//		WriteROMRestore	Restore the image from the data resource
//		WriteROMFromFile	Write the ROM image from the open file
//		ROMErase				Send the ROM erase command to the EEPROM
//		ROMVerifyWrite		Verify that a BYTE was burnt into the EEPROM
//
#include	<stdio.h>
#include	<string.h>
#include	<io.h>
#include	<i86.h>
#include	<fcntl.h>
#include	"flash.h"

//
//		BeginROM - Start the ROM programming sequence
//
//		Entry:	None
//		Exit:		None
//
//		The following method of mapping in the ROM is borrowed from
//		Andy A. at 3Dfx:
//
//		This is accomplished by stealing register space from
//		the card that has already been mapped in.  When the ROM
//		is done programming, the registers are restored.
//
//		We will use Membase0 + 0xA00000 in this case.  There 
//		is no associated logic with it, so the device should
//		respond and the ROM circuit will control data flow and
//		handshaking.
//
void BeginROM (void)
{
	REALMODECALL	rmc;

	// Change ROM address
	rmc.eax = 0xB10D;						// Write a DWORD to config space
	rmc.ebx = wBusDevID;
	rmc.ecx = dwPhysROMBase | 0x01;
	rmc.edi = 0x0030;
	RealModeINT (&rmc, 0x1A);

	// Enable memory space control response (to allow the PCI bridge
	// to pass through the above address)
	rmc.eax = 0xB10C;						// Write a WORD to config space
	rmc.ecx = wPCICommand | 0x2;
	rmc.edi = 0x0004;
	RealModeINT (&rmc, 0x1A);

	// Enable ROM writes
	dwOrgMiscInit0 = *lpLinMiscInit0;

//If Napalm, I need to enable access to MiscInit1
	if((wDeviceID == ChipIdNapalm) || (wDeviceID == ChipIdNapalm2))
		*lpLinMiscInit0 = (dwOrgMiscInit0 & 0xBFFFFFFF);

	dwOrgMiscInit1 = *lpLinMiscInit1;

//Enable ROM writes and disable PCI ROM size

	if(wDeviceID == ChipIdRampage)
//For Rampage, disable ROM writes is on Bit 2
		*lpLinMiscInit1 = (dwOrgMiscInit1 & 0xFDFFFFFF) | 0x04;
	else
//For chips eariler that Rampage, disable ROM writes is on Bit 4
		*lpLinMiscInit1 = (dwOrgMiscInit1 & 0xFDFFFFFF) | 0x10;

	Delay15us (10);						// Wait long enough for the hardware to catch up
}

//
//		EndROM - Terminate the ROM programming sequence
//
//		Entry:	None
//		Exit:		None
//
//		Restore the config space to its original state.
//
void EndROM (void)
{
	REALMODECALL	rmc;

	// Disable ROM writes
	*lpLinMiscInit0 = dwOrgMiscInit0;
	*lpLinMiscInit1 = dwOrgMiscInit1;

	// Change ROM address
	rmc.eax = 0xB10D;						// Write a DWORD to config space
	rmc.ebx = wBusDevID;
	rmc.ecx = dwPhysOrgROMBase;
	rmc.edi = 0x0030;
	RealModeINT (&rmc, 0x1A);

	// Restore memory space control response
	rmc.eax = 0xB10C;						// Write a WORD to config space
	rmc.ecx = wPCICommand;
	rmc.edi = 0x0004;
	RealModeINT (&rmc, 0x1A);
}

//
//		ROMByteWrite - Write a BYTE to the ROM address space
//
//		Entry:	wOffset		Offset within ROM area
//					byData		Data to write
//		Exit:		None
//
void ROMByteWrite (WORD wOffset, BYTE byData)
{
	*(lpLinROMBase + wOffset) = byData;
	FlushBus ();
}

//
//		ROMByteRead - Read a BYTE from the ROM
//
//		Entry:	wOffset		Offset within ROM space to read
//		Exit:		<BYTE>		Data read
//
BYTE ROMByteRead (WORD wOffset)
{
	BYTE	byTemp;

	byTemp = *(lpLinROMBase + wOffset);
	FlushBus ();

	return (byTemp);
}

//
//		FlushBus - Flush the bus by doing two I/O reads
//
//		Entry:	None
//		Exit:		None
//
void FlushBus (void)
{
	_inpd (wIOBase);
	_inpd (wIOBase);
}

//
//		GetROMID - Return the ROM manufacturer and device ID
//
//		Entry:	lpMan				Pointer to the manufacturer ID (returned)
//					lpDevice			Pointer to the device ID (returned)
//					lpwSectorSize	Size (in BYTEs) of an EEPROM sector
//		Exit:		None
//
void GetROMID (LPBYTE lpMan, LPBYTE lpDevice, LPWORD lpwSectorSize)
{
	SendROMCommand (0xF0);				// Exit ID mode (just in case)
	SendROMCommand (0x90);				// Enter ID mode
	*lpMan = ROMByteRead (0);			// Read manufacturer ID
	*lpDevice = ROMByteRead (1);		// Read device ID
	SendROMCommand (0xF0);				// Exit ID mode

	switch (*lpMan)
	{
		case MANID_AMD_TI:

			*lpwSectorSize = 1;
			break;

		case MANID_ATMEL:

			if(( *lpDevice == ATMEL_49BV512 ) || (*lpDevice == ATMEL_49F010 ))
				*lpwSectorSize = 1;
			else
				*lpwSectorSize = 128;
			break;

		case MANID_SST:

			if(( *lpDevice == SST_29EE010 ) || (*lpDevice == SST_29LE512 ))
				*lpwSectorSize = 128;
			else
				*lpwSectorSize = 1;
			break;

		default:

			*lpwSectorSize = 1;
			break;
	}
}

//
//		SendROMCommand - Send a command to the ROM
//
//		Entry:	byCommand		Command to send
//		Exit:		None
//
void SendROMCommand (BYTE byCommand)
{
	ROMByteWrite (0x5555, 0xAA);
	ROMByteWrite (0x2AAA, 0x55);
	ROMByteWrite (0x5555, byCommand);
}

//
//		SaveROM - Save the current ROM image to a binary image file
//
//		Entry:	lpszFile		Filename to save image to
//					lpwCheckSum	Pointer to checksum value (returned)
//		Exit:		BOOL			Success flag (TRUE = Successfully written, FALSE = Not)
//
BOOL SaveROM (LPSTR lpszFile, LPWORD lpwCheckSum)
{
	int	hSaveFile, i, j, n;
#ifdef		USERFLASH
	int	z;
#endif		//UserFlash
	WORD	wChksum, wAddr;

	hSaveFile = open (lpszFile, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE | S_IREAD);
	if (hSaveFile <= 0) return (FALSE);

	// Write the file out, and compute the checksum as we go
	*lpwCheckSum = wChksum = 0;
	wAddr = 0;
#ifdef		USERFLASH
	z = 0;
#endif		//USERFLASH

	for (i = 0; i < (dwOldROMSaveSize / FILEIO_BUFFER); i++)  // Use 32K/64K now
	{
		// Get a chunk o' bytes from the ROM and compute the checksum
		for (j = 0; j < FILEIO_BUFFER; j++)
		{
			byBuffer[j] = ROMByteRead (wAddr);

#ifdef		USERFLASH
			if(i == 0)
			// VPD is in the first block
			{
				if((j >= 0xA0) && (j <= 0xEA)) 
				// VPD Info that must be saved is at 0xA0 to 0xEA
				{
					byVPDBuffer[z] = byBuffer[j]; // Save the VPD from the Original
					z++;
				}
			}
#endif		//USERFLASH

			wChksum += byBuffer[j];
			wAddr++;
		}

		// Write the chunk to the file
		n = write (hSaveFile, byBuffer, FILEIO_BUFFER);
		if (n != FILEIO_BUFFER)
		{
			close (hSaveFile);
			return (FALSE);
		}
	}

	*lpwCheckSum = wChksum;
	close (hSaveFile);
	return (TRUE);
}

//
//		WriteROMRestore - Restore the image from the data resource
//
//		Entry:	lpwCheckSum	Pointer to the computed checksum (returned)
//					lpbCritical	Pointer to the "critical section" flag. If this
//									is TRUE when an error occurs, then a fatal error
//									has occurred and the device is unbootable.
//		Exit:		<BOOL>		Success flag (TRUE = Successful, FALSE = Not)
//
BOOL WriteROMRestore (LPWORD lpwCheckSum, LPBOOL lpbCritical)
{
	WORD	wChksum, wCounter;
	long int	i;

	// Flag that an error here IS in the "critical" section of the code.
	// Basically, an error here will create an unbootable device!
	*lpbCritical = TRUE;

	// Compute the checksum and copy the image into ROM
	*lpwCheckSum = wChksum = 0;
	wCounter = wSectorSize;
	for (i = 0; i < 0x8000; i++)    // The image file never changes!
	{
		wChksum += tblROMImage[i];
		if (wCounter >= wSectorSize)
		{
			SendROMCommand (0xA0);
			wCounter = 0;
		};
		ROMByteWrite ((WORD) i, tblROMImage[i]);
		if (++wCounter >= wSectorSize)
		{
			if (!ROMVerifyWrite ((WORD) i, tblROMImage[i]))
				return (FALSE);										// Fatal error
		}
	}
	*lpwCheckSum = wChksum;

	// Made it through -- now it is again (theoretically) a bootable device.
	*lpbCritical = FALSE;

	return (TRUE);
}

//
//		WriteROMFromFile - Write the ROM image from the open file
//
//		Entry:	hFile			File handle of input file
//					lpwCheckSum	Pointer to the computed checksum (returned)
//					lpbCritical	Pointer to the "critical section" flag. If this
//									is TRUE when an error occurs, then a fatal error
//									has occurred and the device is unbootable.
//		Exit:		<BOOL>		Success flag (TRUE = Successful, FALSE = Not)
//
//		Note:		The file is read twice -- once to compute the checksum
//					and once to actually write the data. If an invalid
//					ROM image is found, then NO IMAGE IS WRITTEN.
//
BOOL WriteROMFromFile (int hFile, LPWORD lpwCheckSum, LPBOOL lpbCritical)
{
	WORD				wChksum, wAddr, wCounter;
	BYTE				ROMSize;
	unsigned int	n, i, j;
	BOOL				bFirst;

#ifdef	USERFLASH
	unsigned int	z;
	BOOL			bBothVPD;
#endif		//USERFLASH

	// Flag that an error here is not in the "critical" section of the
	// code; ie, when *some* of the data is written to the EEPROM.
	*lpbCritical = FALSE;

	// Read the file and compute the checksum
	*lpwCheckSum = wChksum = 0;
	bFirst = TRUE;

//Read the TRUE ROM size and recalc dwNewBinFileSize, if needed.
	read (hFile, byBuffer, FILEIO_BUFFER);
	ROMSize = byBuffer[2];
	if(ROMSize > 0x40)
		dwNewBinFileSize = ROMSize * 512;

//Need to trick FLASH when flashing a 32K BIOS which is in a 64K binary.		
	if((ROMSize == 0x40) && (dwNewBinFileSize == 64*1024))
		dwNewBinFileSize = 0x40 * 512;
		
//Rewind file to the beginning
	lseek (hFile, 0, SEEK_SET);

	for (i = 0; i < (dwNewBinFileSize / FILEIO_BUFFER); i++)	// Use 32K/64K now
	{
		n = read (hFile, byBuffer, FILEIO_BUFFER);

		if (n != FILEIO_BUFFER) return (FALSE);

		// On the first read, determine that the ROM has the correct
		// signature and ROM size.
		if ((i == 0) && (bFirst))
		{
			bFirst = FALSE;

			if (!((byBuffer[0] == 0x55) && (byBuffer[1] == 0xAA)))
				return (FALSE);
		}

		// Compute the checksum
		for (j = 0; j < FILEIO_BUFFER; j++)
			wChksum += byBuffer[j];
	}

	// Return the computed checksum. If the low order BYTE of the checksum
	// is NOT zero, then an invalid checksum exists. Since this would create
	// an un-bootable machine, do NOT burn the EEPROM.

	*lpwCheckSum = wChksum;
	if (LOBYTE (wChksum) != 0) return (FALSE);

	// Flag that an error here IS in the "critical" section of the code.
	// Basically, an error here will create an unbootable device!
	DisplayString (szBeginROM);
	*lpbCritical = TRUE;

	// If processing reaches here, then a valid ROM image is available for
	// downloading to the EEPROM. Rewind the file, and start reading again.

#ifdef	USERFLASH
	bBothVPD = ((IsVPDAvailable()) && (NewFileVPDAvailable(hFile)));
#endif	//USERFLASH

	lseek (hFile, 0, SEEK_SET);
	wCounter = wSectorSize;
	for (i = 0; i < (dwNewBinFileSize / FILEIO_BUFFER); i++)	//Use 32K/64k now
	{
		n = read (hFile, byBuffer, FILEIO_BUFFER);
		if (n != FILEIO_BUFFER) return (FALSE);			// Fatal error

#ifdef	USERFLASH
		if (bKeepOldRegs)
		{
			if ((i * FILEIO_BUFFER <= wNewRegPtr) && 
			   ((i * FILEIO_BUFFER + FILEIO_BUFFER) >= wNewRegPtr))
			{
				CopyInOldRegs (); 
			}
		}
		if ((i == 0) && (wDeviceID > 3) && (bBothVPD))
		{
		// Is this the first block write and was VPD info 
		// available from the Old "Saved" ROM?
		// Yes = Special case handling because the first block has the VPD
		// info in it.
			z = 0;
			wChksum = 0;
			for (j = 0; j < FILEIO_BUFFER; j++)
			{
				if (wCounter >= wSectorSize)
				{
					SendROMCommand (0xA0);
					wCounter = 0;
				}
				wAddr = (WORD) (i*FILEIO_BUFFER + j);

				// The VPDBuffer info will be copied into the EEPROM instead of 
				// from the file for the range of offsets 0xA0 to 0xDE. 
				// However, the ROM version number and End Tag must be copied from 
				// the new file from offsets 0xDF to 0xE9 
				// It must also be checksummed on the fly with the checksum byte
				// being copied in at 0xEA
				if(j < 0xA0) // Copy new file upto VPD Start at 0xA0				
				{
					ROMByteWrite (wAddr, byBuffer[j]);
					if (++wCounter >= wSectorSize)
					{
						if (!ROMVerifyWrite (wAddr, byBuffer[j]))
							return (FALSE);							// Fatal error
					}
				}
				if((j >= 0xA0) && (j <= 0xDE))	// VPD Info 0xA0 to 0xDE
				// Just the Part No., ECO Lvl, and Ser. No. need to be carried over
				{
					ROMByteWrite (wAddr, byVPDBuffer[z]);
					wChksum += byVPDBuffer[z];
					if (++wCounter >= wSectorSize)
					{
						if (!ROMVerifyWrite (wAddr, byVPDBuffer[z]))
							return (FALSE);						 	// Fatal error
					}
					z++;
				}
				if((j >= 0xDF) && (j <= 0xE9)) // VPD End info 0xDF to 0xE9				
				//ROM Lvl and End Tag come from the new file				
				{
					ROMByteWrite (wAddr, byBuffer[j]);
					wChksum += byBuffer[j];
					if (++wCounter >= wSectorSize)
					{
						if (!ROMVerifyWrite (wAddr, byBuffer[j]))
							return (FALSE);							// Fatal error
					}
				}
				if(j == 0xEA) // VPD Checksum goes here
				{
					wChksum = ((WORD) (0x0100 - (wChksum & 0x00FF)));
					ROMByteWrite (wAddr, (LOBYTE(wChksum)));
					//After this wChksum is irrelevant
					//but the rest of the block needs to be
					//copied  
					if (++wCounter >= wSectorSize)
					{
						if (!ROMVerifyWrite (wAddr, (LOBYTE(wChksum))))
							return (FALSE);							// Fatal error
					}
				}
				if(j > 0xEA) // Copy the rest of the block
				{
					ROMByteWrite (wAddr, byBuffer[j]);
					if (++wCounter >= wSectorSize)
					{
						if (!ROMVerifyWrite (wAddr, byBuffer[j]))
							return (FALSE);							// Fatal error
					}
				}
			}
		}
		else
		{
		//Handle it just like before on subsequent blocks or if no VPD info was 
		//available from the Saved ROM.
			for (j = 0; j < FILEIO_BUFFER; j++)
			{
				if (wCounter >= wSectorSize)
				{
					SendROMCommand (0xA0);
					wCounter = 0;
				}
				wAddr = (WORD) (i*FILEIO_BUFFER + j);

				if (bKeepOldRegs)
				{
					if (wAddr == (dwNewBinFileSize - 1))
					{
						// To protect the Checksum byte from being prematurely 
						// written check the Atmel 49BV512 Spec for explanation
						ROMByteWrite (wAddr, 0xFF);
						if (++wCounter >= wSectorSize)
						{
							if (!ROMVerifyWrite (wAddr, 0xFF))
							return (FALSE);							// Fatal error
						}
					} 
					else
					{
						ROMByteWrite (wAddr, byBuffer[j]);
						if (++wCounter >= wSectorSize)
						{
							if (!ROMVerifyWrite (wAddr, byBuffer[j]))
								return (FALSE);						// Fatal error
						}
					}

				}
				else
				{
					ROMByteWrite (wAddr, byBuffer[j]);
					if (++wCounter >= wSectorSize)
					{
						if (!ROMVerifyWrite (wAddr, byBuffer[j]))
							return (FALSE);							// Fatal error
					}
				}
			}
		}
#else
		for (j = 0; j < FILEIO_BUFFER; j++)
		{
			if (wCounter >= wSectorSize)
			{
				SendROMCommand (0xA0);
				wCounter = 0;
			}
			wAddr = (WORD) (i*FILEIO_BUFFER + j);
			ROMByteWrite (wAddr, byBuffer[j]);
			if (++wCounter >= wSectorSize)
			{
				if (!ROMVerifyWrite (wAddr, byBuffer[j]))
					return (FALSE);									// Fatal error
			}
		}
#endif	//USERFLASH
	}

//#ifdef BANSHEE	// Can't use software to select 64k for programming in Voodoo 3.

	// 3Dfx always uses a 64K ROM device. Even though the signature is for
	// a 32K ROM and the VGA ROM must ALWAYS be 32K, for some reason, the
	// sub-vendor ID needs to show up at the end of the device. Note that
	// this signature is not visible by any system software.
	// The sub-vendor ID are four BYTEs starting at the 8 BYTEs from the
	// end of the buffer. Assume that "byBuffer" is at least 8 BYTEs.
	// Write FF's consectively until it's time to write the ID's.

//This code will be execute if we're less then 64K but the board is strapped
//for 64K.  This means, we need to shadow the ID to the upper 64k of ROM.
    if (( dwOldROMSaveSize < 0x10000 ) && (_inpd (wIOBase+0x14)&0x0200))

//    if ( wSectorSize > 1 )
    {
		for (i = 0x8000; i < 0xFFF8; i++)
		{
			if (wCounter >= wSectorSize)
			{
				SendROMCommand (0xA0);
				wCounter = 0;
			}
			ROMByteWrite ((WORD) i, 0xFF);
			if (++wCounter >= wSectorSize)
			{
				if (!ROMVerifyWrite ((WORD) i, 0xFF))
					return (FALSE);									// Fatal error
			}
		}
	}

   for (i = 0; i < 12; i++)
	{
		if (wCounter >= wSectorSize)
		{
			SendROMCommand (0xA0);
			wCounter = 0;
		}
		ROMByteWrite ((WORD) (0xFFF0 + i), byBuffer[(FILEIO_BUFFER - 16) + i]);
		if (++wCounter >= wSectorSize)
		{
			if (!ROMVerifyWrite ((WORD) (0xFFF0 + i), byBuffer[(FILEIO_BUFFER - 16) + i]))
				return (FALSE);									// Fatal error
		}
	}
//#endif

#ifdef	USERFLASH
	// If the old Registers were copied into the new BIOS then the entire BIOS
	// will need to be checksummed
	if (bKeepOldRegs)
	{
		wAddr = 0;
		wChksum = 0;

		while (wAddr < (dwNewBinFileSize - 1))
		{
			wChksum += ((WORD) ROMByteRead (wAddr));
			wAddr++;
		} 

		wChksum = wChksum & 0x00FF;
		wChksum = 0x0100 - wChksum;

		SendROMCommand (0xA0);

		ROMByteWrite (wAddr, LOBYTE(wChksum));
	}
#endif	//USERFLASH

	// Made it through -- now it is again (theoretically) a bootable device.
	*lpbCritical = FALSE;
	DisplayString (szCompleted);

	return (TRUE);
}

//
//		ROMErase - Send the ROM erase command to the EEPROM
//
//		Entry:	None
//		Exit:		None
//
void ROMErase (void)
{
	SendROMCommand (0x80);
	SendROMCommand (0x10);

	// Wait for the ROM to erase itself
	DisplayString (szROMErase);
	while ((ROMByteRead (0) & 0x80) == 0x0);
	DisplayString (szCompleted);

	// Play it safe and wait for 10ms
	Delay15us (20000l/15);
}

//
//		ROMVerifyWrite - Verify that a BYTE was burnt into the EEPROM
//
//		Entry:	wOffset		Offset within ROM image
//					byExpected	Expected data
//		Exit:		<BOOL>		Success flag (TRUE = ROM BYTE written, FALSE = Not)
//
BOOL ROMVerifyWrite (WORD wOffset, BYTE byExpected)
{
	BYTE	byTemp;

	switch (byMan)
	{
		case MANID_AMD_TI:

			while (TRUE)
			{
				byTemp = ROMByteRead (wOffset);

				// Check that DQ7 is the same as the data
				if (((byTemp ^ (~byExpected)) & 0x80) == 0x80)
					break;		// Verified!

				// See if DQ5 is a 1 and look for error
				if ((byTemp & 0x20) == 0x20)
				{
					byTemp = ROMByteRead (wOffset);
					// If the data is not there, then there was an error
					if (((byTemp ^ byExpected) & 0x80) == 0x80)
						return (FALSE);
				}
			}
			break;

		case MANID_ATMEL:

			switch (byDevice)
			{
				case ATMEL_49F010:

					while (TRUE)
					{
						byTemp = ROMByteRead (wOffset);

						// Check that DQ7 is the same as the data
						if (((byTemp ^ (~byExpected)) & 0x80) == 0x80)
							break;		// Verified!
					}
					break;

				case ATMEL_49BV512:
				  Delay15us (10);
				break;

				default:

					Delay15us (10000/15);
					break;
			}
			break;

		case MANID_SST:

			switch (byDevice)
			{
				case SST_29LE512:
				case SST_29EE010:
				case SST_39SF010:
				case SST_39VF512:

					while (TRUE)
					{
						byTemp = ROMByteRead (wOffset);

						// Check that DQ7 is the same as the data
						if (((byTemp ^ (~byExpected)) & 0x80) == 0x80)
							break;		// Verified!
					}
					break;


				default:

					Delay15us (10000/15);
					break;
			}
			break;

	}

	return (TRUE);
}

#ifdef	USERFLASH
//
//		IsVPDAvailable - Check if the old ROM had a VPD to update
//
//		Entry:	(void)
//
//		Exit:	<BOOL>	Success flag (TRUE = VPD exists in byVPDBuffer, FALSE = Not)
//
BOOL IsVPDAvailable (void)
{
	// Check if VPD string is there "3dfx VPD"
	if ((byVPDBuffer[3]==0x33) && /*(byVPDBuffer[4]==0x64) &&*/
		(byVPDBuffer[5]==0x66) && (byVPDBuffer[6]==0x78) &&
		(byVPDBuffer[7]==0x20) && (byVPDBuffer[8]==0x56) &&
		(byVPDBuffer[9]==0x50) && (byVPDBuffer[10]==0x44)) 
	{
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}

//
//		SaveOffOldRegs - Save the configuration table for the 
//			register settings of the Old BIOS. To be maintained
//			when the new BIOS is flashed
//
//		Entry:	(void)
//
//		Exit:	none
//
void SaveOffOldRegs (void)
{
	BYTE	byLoByte, byHiByte;
	WORD	w1Ptr, w2Ptr;
	int		i;

	// Start at the ROM Config entry point org'ed at 0x50
	// Get the tblROMConfig pointer value there
   byLoByte = ROMByteRead(FIRST_POINTER_ADDR);
   byHiByte = ROMByteRead(FIRST_POINTER_ADDR + 1);
	w1Ptr = (((WORD) byHiByte << 8) + ((WORD) byLoByte));

	// Check for a valid pointer if all F's then the EEPROM is "blank" 
	if ( w1Ptr == 0xFFFF)
	{
		bKeepOldRegs = FALSE;
	}
	// Pull the OEM Config table pointer from the ROM Config table
	// Use that as the pointer into the structure
   byLoByte = ROMByteRead(w1Ptr);
   byHiByte = ROMByteRead(w1Ptr + 1);
	w2Ptr = (((WORD) byHiByte << 8) + ((WORD) byLoByte));

	// Check for a valid pointer if all F's then the EEPROM is "blank" 
	if ( w2Ptr == 0xFFFF)
	{
		bKeepOldRegs = FALSE;
	}


	if(wDeviceID == ChipIdRampage)
		{
//If Rampage, we need to update the offsets to the OEM Config data entries
		NUM_REGS = RAMPAGE_NUM_REGS;

		sOEMCfgRegInfo[0].wRegOffset = OFFSET_RAMPAGE_PCIINIT0; 
		sOEMCfgRegInfo[1].wRegOffset = OFFSET_RAMPAGE_MISCINIT0;
		sOEMCfgRegInfo[2].wRegOffset = OFFSET_RAMPAGE_MISCINIT1;
		sOEMCfgRegInfo[3].wRegOffset = OFFSET_RAMPAGE_AGPINIT0;	
		sOEMCfgRegInfo[4].wRegOffset = OFFSET_RAMPAGE_PLLCTRL0;	
		sOEMCfgRegInfo[5].wRegOffset = OFFSET_RAMPAGE_DRAMINIT0;
		sOEMCfgRegInfo[6].wRegOffset = OFFSET_RAMPAGE_DRAMINIT1;
		sOEMCfgRegInfo[7].wRegOffset = OFFSET_RAMPAGE_DRAMINIT2;
		sOEMCfgRegInfo[8].wRegOffset = OFFSET_RAMPAGE_DRAMINIT3;
		sOEMCfgRegInfo[9].wRegOffset = OFFSET_RAMPAGE_2DCOMMAND;
		}
//If Napalm, we need to update the offsets to the OEM Config data entries
	else if(wDeviceID == ChipIdNapalm)
		{
		NUM_REGS = NAPALM_NUM_REGS;

		sOEMCfgRegInfo[0].wRegOffset = OFFSET_NAPALM_PCIINIT0;
		sOEMCfgRegInfo[1].wRegOffset = OFFSET_NAPALM_MISCINIT0;
		sOEMCfgRegInfo[2].wRegOffset = OFFSET_NAPALM_MISCINIT1;
		sOEMCfgRegInfo[3].wRegOffset = OFFSET_NAPALM_DRAMINIT0;
		sOEMCfgRegInfo[4].wRegOffset = OFFSET_NAPALM_DRAMINIT1;
		sOEMCfgRegInfo[5].wRegOffset = OFFSET_NAPALM_AGPINIT0;
		sOEMCfgRegInfo[6].wRegOffset = OFFSET_NAPALM_PLLCTRL1;
		sOEMCfgRegInfo[7].wRegOffset = OFFSET_NAPALM_PLLCTRL2;
		sOEMCfgRegInfo[8].wRegOffset = OFFSET_NAPALM_SGRAMMODE;
		sOEMCfgRegInfo[9].wRegOffset = OFFSET_NAPALM_TMUGBEINIT;
		}
//If Napalm2, we need to update the offsets to the OEM Config data entries
	else if(wDeviceID == ChipIdNapalm2)
		{
		NUM_REGS = NAPALM2_NUM_REGS;

		sOEMCfgRegInfo[0].wRegOffset = OFFSET_NAPALM_PCIINIT0;
		sOEMCfgRegInfo[1].wRegOffset = OFFSET_NAPALM_MISCINIT0;
		sOEMCfgRegInfo[2].wRegOffset = OFFSET_NAPALM_MISCINIT1;
		sOEMCfgRegInfo[3].wRegOffset = OFFSET_NAPALM_DRAMINIT0;
		sOEMCfgRegInfo[4].wRegOffset = OFFSET_NAPALM_DRAMINIT1;
		sOEMCfgRegInfo[5].wRegOffset = OFFSET_NAPALM_AGPINIT0;
		sOEMCfgRegInfo[6].wRegOffset = OFFSET_NAPALM_PLLCTRL1;
		sOEMCfgRegInfo[7].wRegOffset = OFFSET_NAPALM_PLLCTRL2;
		sOEMCfgRegInfo[8].wRegOffset = OFFSET_NAPALM_SGRAMMODE;
		sOEMCfgRegInfo[9].wRegOffset = OFFSET_NAPALM_TMUGBEINIT;
		sOEMCfgRegInfo[10].wRegOffset = OFFSET_NAPALM2_STRAPPING1;
		sOEMCfgRegInfo[11].wRegOffset = OFFSET_NAPALM2_STRAPPING2;
		}
	else
		{
//Must be V3 or lower
		NUM_REGS = V3_NUM_REGS;
		}

	// Save the values off into the structure

	i = 0;
	while (i < NUM_REGS)
	{
		sOEMCfgRegInfo[i].byLLByte = ROMByteRead(w2Ptr+sOEMCfgRegInfo[i].wRegOffset);
		sOEMCfgRegInfo[i].byLHByte = ROMByteRead(w2Ptr+sOEMCfgRegInfo[i].wRegOffset+1);
		sOEMCfgRegInfo[i].byHLByte = ROMByteRead(w2Ptr+sOEMCfgRegInfo[i].wRegOffset+2);
		sOEMCfgRegInfo[i].byHHByte = ROMByteRead(w2Ptr+sOEMCfgRegInfo[i].wRegOffset+3);
		i++;
	}
}	

//
//		GetNewRegsSettingPtr - Return the WORD pointer for the Register 
//			settings table within the new BIOS file. 
//
//		Entry:	<FILE> The New BIOS File 
//
//		Exit:	<WORD> The offset of the Register settings table within
//				the file.
//
WORD GetNewRegsSettingPtr (void)
{
	WORD	wRegTableOffset;

   lseek(hInFile, 0x50, SEEK_SET);		// RomConfigTable Org'd at 0x0050
	read (hInFile, byRegisterOffset, 2);	// Read pointer entry
	// Remember to reverse the byte order
	wRegTableOffset = (((WORD) byRegisterOffset[1] << 8) + ((WORD) byRegisterOffset[0]));

   lseek(hInFile, wRegTableOffset, SEEK_SET);	// OEMConfigTable entry
	read (hInFile, byRegisterOffset, 2);	// Read pointer entry
	// Remember to reverse the byte order
	wRegTableOffset = (((WORD) byRegisterOffset[1] << 8) + ((WORD) byRegisterOffset[0])); 
   lseek(hInFile, 0, SEEK_SET);

	// Return the location of the beginning of the table 
	return (wRegTableOffset);
}

//
//		CopyInOldRegs - Copy the Original Register settings from the 
//			old BIOS into the new BIOS 
//
//		Entry:	<int> The current block of the file in byBuffer 
//
//		Exit:	<void> The Register settings will be copied into the proper
//				location as though it were being copied into the file.
//				Writing to the ROM can then proceed normally directly out of
//				the buffer. 
//
void CopyInOldRegs (void)
{
	int i, p, q, r;


	// Find where the pointer would be in relation to the present block
	p = wNewRegPtr;

	while (p > FILEIO_BUFFER)
	{
		p = p - FILEIO_BUFFER; 
	} 

	i = 0;
	while (i < NUM_REGS)
	{
		q = sOEMCfgRegInfo[i].wRegOffset;
		r = p + q;
		byBuffer[r] = sOEMCfgRegInfo[i].byLLByte;
		byBuffer[r + 1] = sOEMCfgRegInfo[i].byLHByte;
		byBuffer[r + 2] = sOEMCfgRegInfo[i].byHLByte;
		byBuffer[r + 3] = sOEMCfgRegInfo[i].byHHByte;

		i++;
	}
}

//
//		NewFileVPDAvailable - Check the file intended to be flashed to see
//			if the VPD structure is there
//
//		Entry:	<int> The file handle of the new file.
//
//		Exit:	<BOOL> Whether the New File has the VPD or not.
//
BOOL NewFileVPDAvailable (int hNewFile)
{
	char	bSearchBuffer[FILEIO_BUFFER];
	int		i;
	BOOL	bReadRes;


   lseek (hNewFile, 0, SEEK_SET);
	read (hNewFile, bSearchBuffer, FILEIO_BUFFER);
   lseek (hNewFile, 0, SEEK_SET);

	// Check if VPD string is in the New file adjusted for
	// whether "3[D/d]fx VPD"
	i = 0;
	bReadRes = FALSE;

	while ((i < (FILEIO_BUFFER - 8)) && (bReadRes == FALSE))
	{
		if ((bSearchBuffer[i]==0x33) && /*(bSearchBuffer[i+1]==0x64) &&*/
			(bSearchBuffer[i+2]==0x66) && (bSearchBuffer[i+3]==0x78) &&
			(bSearchBuffer[i+4]==0x20) && (bSearchBuffer[i+5]==0x56) &&
			(bSearchBuffer[i+6]==0x50) && (bSearchBuffer[i+7]==0x44)) 
		{
			bReadRes = TRUE;
		}
		i++;
	}
	return (bReadRes);
}

//
//		CheckLaterVersion - Will compare the versions of the ROM and the new file
//		to see which is later
//
//		Entry:	<int> The file handle of the new file.
//
//		Exit:	<BOOL> Whether the New File version is later than the existing ROM.
//
BOOL CheckLaterVersion (int hRefFile)
{
	BOOL	bVersionResults;
	BOOL	bROMVersFound;
	BOOL	bNewFileVersFound;
	WORD	wAddr;
	char	RC1,RP1,RC2,RC3,RP2,RC4,RC5;
	char	FC1,FP1,FC2,FC3,FP2,FC4,FC5;
	char	bRSearchBuffer[FILEIO_BUFFER];
	char	bSearchBuffer[FILEIO_BUFFER];
	int		i;


	bVersionResults = TRUE; // Assume a positive result
	bROMVersFound = FALSE;
	wAddr = 0;

	for(i = 0; i < FILEIO_BUFFER; i++)
	{
		bRSearchBuffer[i] = ROMByteRead (i);
	}

	i = 0;
	while ((i < FILEIO_BUFFER) && (bROMVersFound == FALSE))
	{
     RC1 = bRSearchBuffer[i];
     if((RC1 >= 0x30 /*"0"*/) && (RC1 <= 0x39 /*"9"*/))
     {
       i++;
       RP1 = bRSearchBuffer[i];
       if(RP1 == 0x2E /*"."*/)
       {	
         i++;
         RC2 = bRSearchBuffer[i];
         if((RC2 >= 0x30 /*"0"*/) && (RC2 <= 0x39 /*"9"*/))
         {
           i++;
           RC3 = bRSearchBuffer[i];
           if((RC3 >= 0x30 /*"0"*/) && (RC3 <= 0x39 /*"9"*/))
           {
             bROMVersFound = TRUE; // Assumed found at X.XX although the rest
             i++;              // has to be handled - in case X.XX.XX
             RP2 = bRSearchBuffer[i];
             if(RP2 == 0x2E /*"."*/)
             {	
               i++;
               RC4 = bRSearchBuffer[i];
               if((RC4 >= 0x30 /*"0"*/) && (RC4 <= 0x39 /*"9"*/))
               {
                 i++;
                 RC5 = bRSearchBuffer[i];
                 if((RC5 >= 0x30 /*"0"*/) && (RC5 <= 0x39 /*"9"*/))
                 {
                   i++;
                 }
                 else
                 {
                   i++;
                   RC5 = 0x30; // zero it out
                 }
               }
               else
               {
                 i++;
                 RC4 = 0x30;
                 RC5 = 0x30;
               }
             }
             else
             {
               i++;
               RP2 = 0x2E;
               RC4 = 0x30;
               RC5 = 0x30;
             }
           }
           else
           {
             i++;
           }
         }
         else
         {
           i++;
         }
       }
       else
       {
         i++;
       }
     }
     else
     {
       i++;
     }
	}


// At this point if we can't find a version number for the
// old ROM then anything the new file has is an improvement.
	if(bROMVersFound == FALSE)
	{
		return (bVersionResults);
	}


// Now check the New File
   lseek (hRefFile, 0, SEEK_SET);
	read (hRefFile, bSearchBuffer, FILEIO_BUFFER);
   lseek (hRefFile, 0, SEEK_SET);

	i = 0;
	bNewFileVersFound = FALSE;

	while ((i < FILEIO_BUFFER) && (bNewFileVersFound == FALSE))
	{
     FC1 = bSearchBuffer[i];
     if((FC1 >= 0x30 /*"0"*/) && (FC1 <= 0x39 /*"9"*/))
     {
       i++;
       FP1 = bSearchBuffer[i];
       if(FP1 == 0x2E /*"."*/)
       {	
         i++;
         FC2 = bSearchBuffer[i];
         if((FC2 >= 0x30 /*"0"*/) && (FC2 <= 0x39 /*"9"*/))
         {
           i++;
           FC3 = bSearchBuffer[i];
           if((FC3 >= 0x30 /*"0"*/) && (FC3 <= 0x39 /*"9"*/))
           {
             bNewFileVersFound = TRUE; // Assumed found at X.XX although the rest
             i++;                      // has to be handled - in case X.XX.XX
             FP2 = bSearchBuffer[i];
             if(FP2 == 0x2E /*"."*/)
             {	
               i++;
               FC4 = bSearchBuffer[i];
               if((FC4 >= 0x30 /*"0"*/) && (FC4 <= 0x39 /*"9"*/))
               {
                 i++;
                 FC5 = bSearchBuffer[i];
                 if((FC5 >= 0x30 /*"0"*/) && (FC5 <= 0x39 /*"9"*/))
                 {
                   i++;
                 }
                 else
                 {
                   i++;
                   FC5 = 0x30; // zero it out
                 }
               }
               else
               {
                 i++;
                 FC4 = 0x30;
                 FC5 = 0x30;
               }
             }
             else
             {
               i++;
               FP2 = 0x2E;
               FC4 = 0x30;
               FC5 = 0x30;
             }
           }
           else
           {
             i++;
           }
         }
         else
         {
           i++;
         }
       }
       else
       {
         i++;
       }
     }
     else
     {
       i++;
     }
	}

// If the Version number wasn't found in the New File then we have problems.
   if(bNewFileVersFound)
   {
     if(FC1 < RC1)
     {
       bVersionResults = FALSE;
     }
     else if(FC1 > RC1)
     {
       bVersionResults = TRUE;
     }
     else // (FC1 == RC1)
     {
       if(FC2 < RC2)
       {
         bVersionResults = FALSE;
       }
       else if(FC2 > RC2)
       {
         bVersionResults = TRUE;
       }
       else // (FC2 == RC2)
       {
         if(FC3 < RC3)
         {
           bVersionResults = FALSE;
         }
         else if(FC3 > RC3)
         {
           bVersionResults = TRUE;
         }
         else // (FC3 == RC3)
         {
           if(FC4 < RC4)
           {
             bVersionResults = FALSE;
           }
           else if(FC4 > RC4)
           {
             bVersionResults = TRUE;
           }
           else // (FC4 == RC4)
           {
             if(FC5 < RC5)
             {
               bVersionResults = FALSE;
             }
             else if(FC5 > RC5)
             {
               bVersionResults = TRUE;
             }
             else // (FC5 == RC5)
             {
               bVersionResults = TRUE; // Might as well go with it
             }
           }
         } 
       }
     }
	}
	else
	{
		bVersionResults = FALSE;
	}
	return (bVersionResults);
}
#endif	//USERFLASH

//
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//


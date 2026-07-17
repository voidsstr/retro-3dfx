#define __MACERRORS__
#include <DriverServices.h>
#include <3dfx.h>
#include <gdebug.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <NameRegistry.h>
#include <PCI.h>
#include <SIOUX.h>

#include <h3cinit.h>
#include <h3regs.h>
#include <h3cinitdd_mac.h>

#include <minihwc.h>
#include <hwcio.h>

#include <fxpci.h>
#include <stdio.h>

#include "setmode.h"

#define BLIND_FLASH 1

#define RED_SHIFT       16
#define GREEN_SHIFT     8
#define BLUE_SHIFT      0

#define P6FENCE	__sync()

typedef unsigned long BOOL;
typedef BOOL *LPBOOL;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef DWORD *LPDWORD;
typedef WORD *LPWORD;
typedef BYTE *LPBYTE;
typedef char *LPSTR;

#define __far

// Defines
//#define	FALSE					0
//#define	TRUE 					1
#define	MAX_LINE				256			// Max characters on prompt line
#define	PCI_ID					0x121A
#define	PCI_ID_DEVICE_2000	0x0004
#define	PCI_ID_DEVICE_3000	0x0005
#define	MAX_FILENAME				256
//#define	FILEIO_BUFFER				(4*1024)		// MUST be divisible by 32K

// EEPROM Manufacturers and EEPROM device types
#define	MANID_AMD_TI				0x01			// AMD & TI 29010 have same ID's
#define	AMD_TI_29010				0x20
#define	MANID_ATMEL				0x1F
#define	ATMEL_29010				0xD5
#define	ATMEL_49F010				0x17
#define	MANID_SST				0xBF
#define SST_29EE010				0x07
#define SST_39SF010				0xB5
#define SST_39VF512				0xD4
#if defined(H5)
#define ROM_SIZE 65536
#define ROM_RAW_NAME "3dfx-V4.rom"
#define ROM_IMAGE_NAME "3dfx-v4.img"
#define DEVICE_ID_MIN 0x06
#define DEVICE_ID_MAX 0x0f
#elif defined(H4)
#define ROM_SIZE 65536
#define ROM_RAW_NAME "3dfx-V3.rom"
#define ROM_IMAGE_NAME "3dfx-v3.img"
#define DEVICE_ID_MIN 0x04
#define DEVICE_ID_MAX 0x05
#elif defined(H3)
#define ROM_SIZE 32768
#define ROM_RAW_NAME "3dfx-Banshee.rom"
#define ROM_IMAGE_NAME "3dfx-Banshee.img"
#define DEVICE_ID_MIN 0x03
#define DEVICE_ID_MAX 0x03
#else
#error Unknown chipset!
#endif

#define INPUT_BUFFER_SIZE ROM_SIZE
#define FILEIO_BUFFER				(ROM_SIZE)

// Include the ROM image
#include	"image.h"

FILE			*hInFile = NULL;				// Assume reset to older image
FILE			*hOutDevice = stdout;		// Assume standard output device
WORD			selFlat = 0;					// Flat model selector (alloc'd at runtime)
WORD			selData = 0;					// Valid data selector (filled in runtime)
WORD			wBusDevID = 0;					// Bus & Device ID from PCI BIOS
WORD			wIOBase = 0;					// I/O Base address
WORD			wPCICommand = 0;				// PCI command register (config space)
BYTE			byMan = 0;						// Manufacturer ID from ROM
BYTE			byDevice = 0;					// Device ID from ROM
WORD			wSectorSize = 1;				// Sector size in BYTES of EEPROM
DWORD			dwOrgMiscInit1 = 0;			// Original MiscInit1 register

// Useful addresses
DWORD			dwPhysMemBase0 = NULL;		// Pointer to memory mapped registers (physical addr)
BYTE	__far *lpLinMemBase0 = NULL;		// Pointer to memory mapped registers (linear addr)
DWORD			dwPhysOrgROMBase = NULL;	// Original ROM Base value (physical)
DWORD			dwLinOrgROMBase = NULL;		// Original ROM Base value (linear)
DWORD			dwPhysROMBase = NULL;		// Pointer to "new" ROM location (physical)
BYTE	__far *lpLinROMBase = NULL;		// Pointer to "new" ROM location (linear)
DWORD	__far *lpLinMiscInit1= NULL;		// Pointer to miscInit1 register (linear).

char	szROMSaveFileMac[MAX_FILENAME] = "MAC-SAVE.ROM";	// Old ROM image is saved here
char	szROMSaveFilePC[MAX_FILENAME] = "PC-SAVE.ROM";	// Old ROM image is saved here
BYTE	byBuffer[FILEIO_BUFFER];						// File I/O goes through here

char	szCopyright[] = "\n3Dfx Voodoo 3 EEPROM Flash Utility   Version 1.05"
							"\nCopyright (c) 1998 by 3Dfx Interactive, Inc."
							"\nAll rights reserved.\n";
char	szInvalidArgs[] = "\nInvalid number of command line arguments. Use FLASH /? to list commands.\n";
char	szUsage[] = "\nRuns 3Dfx EEPROM Flash Utility"
						"\n\nFLASH [filename] | [/RESET] | [[/?] | [/HELP]]"
						"\n\n  filename\tName of file containing VGA BIOS ROM image"
						"\n  RESET\t\tReset to older ROM image"
						"\n  ? or HELP\tDisplay this message"
						"\n\nFLASH will copy a ROM image into the EEPROM on the 3Dfx adapter."
						"\nThe previous ROM image will be saved to disk as \"SAVE.ROM\"."
						"\n";
char	szNoFile[] = "\nFile not found\n";
char	szReadError[] = "\nError while reading file\n";
char	szSyntaxError[] = "Syntax error\n";
char	szResetMsg[] = "\nResetting EEPROM to older VGA BIOS image...\n";
char	szNoBoard[] = "\nERROR: A 3Dfx Voodoo 3 is not in this system.\n";
char	szUnknownDevice[] = "\nERROR: Unknown flash EEPROM manufacturer. (Man. ID: %02Xh, Dev. ID: %02Xh)\n";
char	szSuccess[] = "\nEEPROM successfully programmed.\n"
                      "\n*** Shut down, power off and reboot for changes to take effect.\n";
char	szBurnError[] = "\nERROR: EEPROM not written properly! Attempting load from RESET image.\n";
char	szInvalidROM[] = "\nFATAL ERROR: EEPROM not written! Do NOT restart this machine without contacting"
									"\n             the manufacturer of this video card!";
char	szNoSuccess[] = "\nERROR: EEPROM could not be written. A valid image should still exist in the ROM.";
char	szSaveSuccess[] = "\nPrevious ROM image saved as SAVE.ROM.";
char	szNoSave[] = "\nERROR: Previous ROM image was not saved!";
char	szBeginROM[] = "\nBeginning ROM transfer...";
char	szCompleted[] = "completed.";
char	szROMErase[] = "\nErasing EEPROM...";

char	szHelp0[] = "/?";
char	szHelp1[] = "/HELP";
char	szReset[] = "/RESET";

DWORD dwOldROMSaveSize, dwNewBinFileSize;

//
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//

#define MAX_BOARDS 10


		FxU32
			_hwBaseAddress[MAX_BOARDS],
			_lfbBaseAddress[MAX_BOARDS],
			_ioPortAddress[MAX_BOARDS],
			_romBaseAddress[MAX_BOARDS];

RegEntryID _gBoardRegEntryID[MAX_BOARDS];
char _slotName[MAX_BOARDS][256];
char _modelName[MAX_BOARDS][256];

		FxU32
			hwBaseAddress,
			lfbBaseAddress,
			ioPortAddress,
			romBaseAddress,
			newROMBase,

			bn, deviceNum, i, regBase, vgaInit0, origMiscInit1;
		const FxU32
			sgramMode 		= 0x37,
			sgramMask 		= 0xFFFFFFFF,
			sgramColor 		= 0x00000000,
			grxSpeedInMHz = 100,
			memSpeedInMHz = 100;

RegEntryID gBoardRegEntryID;

	hwcBoardInfo bInfo;

void Delay15us(int count)
{
	AbsoluteTime delayTime;						// The duration of delay in # ticks

	Duration foo;
	
	while(count > 0)
	{
		delayTime = DurationToAbsolute(-15);
		DelayForHardware(delayTime);
		count--;
	}
}

void DisplayString(char *foo)
{
	printf(foo);
	fflush(stdout);
}

void BeginROM(void)
{
	FxU32 miscInit1;

	// Remap trick
	newROMBase = hwBaseAddress + 0xA00000; // Enable ROM accesses
	newROMBase |= 1;
    ExpMgrConfigWriteLong(&gBoardRegEntryID, (LogicalAddress)0x30, newROMBase);
	newROMBase &= ~1;

    HWC_IO_LOAD(bInfo.regInfo, miscInit1, origMiscInit1);
    miscInit1 = (origMiscInit1 & 0xFDFFFFFF) | 0x10;
    HWC_IO_STORE(bInfo.regInfo, miscInit1, miscInit1);
	Delay15us (50);						// Wait long enough for the hardware to catch up
}

void EndROM(void)
{
    ExpMgrConfigWriteLong(&gBoardRegEntryID, (LogicalAddress)0x30, romBaseAddress);
    HWC_IO_STORE(bInfo.regInfo, miscInit1, origMiscInit1);
}

void FlushBus(void)
{
	// Do two I/O reads
	volatile long foo;
	foo = *((volatile long *)ioPortAddress);
	foo = *((volatile long *)ioPortAddress);
}

void ROMByteWrite (WORD wOffset, BYTE byData)
{
	__eieio();
	__sync();
	*(volatile BYTE *)(newROMBase + wOffset) = byData;
	__eieio();
	__sync();
	
	FlushBus();
	//printf("write(%04lx) = %02lx\n",wOffset,byData);
}

BYTE ROMByteRead (WORD wOffset)
{
	BYTE	byTemp;

	__eieio();
	__sync();
	byTemp = *(volatile BYTE *)(newROMBase + wOffset);
	__eieio();
	__sync();

	//printf("read(%04lx) = %02lx\n",wOffset,byTemp);
	FlushBus();
	return (byTemp);
}

void SendROMCommand (BYTE byCommand)
{
	ROMByteWrite (0x5555, 0xAA);
	ROMByteWrite (0x2AAA, 0x55);
	ROMByteWrite (0x5555, byCommand);
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
		
			if( *lpDevice == 3 )
				*lpDevice = ATMEL_49F010;
				
			if( *lpDevice == ATMEL_49F010 )
				*lpwSectorSize = 1;
			else
				*lpwSectorSize = 128;
			break;

		case MANID_SST:

			if( *lpDevice == SST_29EE010 )
				*lpwSectorSize = 128;
			else
				*lpwSectorSize = 1;
			break;

		default:

			*lpwSectorSize = 1;
			break;
	}
}

FxU32 GetROMType(FxBool readROM)
{
  FxU16 headerOffset = 0;
  
  /* Suck down entire ROM image if requested. */
  if(readROM) {
    for(i = 0; i < ROM_SIZE; i++) {
      byBuffer[i] = ROMByteRead(i);
    }
  }
  
  /* First make sure we have a valid PCI header. */
  if(byBuffer[headerOffset + 0] == 0x55 && byBuffer[headerOffset + 1] == 0xAA) {
    FxU16 imageOffset;
    
    /* Get offset to first image's PCI Data Structure */
    imageOffset = byBuffer[headerOffset + 0x18] | (byBuffer[headerOffset + 0x19] << 8);
    
#if DEBUG    
    printf("image offset: %d\n",imageOffset);
    printf("vendor: %04lx device: %08lx\n",
      byBuffer[imageOffset + 0x04] | (byBuffer[imageOffset + 0x05] << 8),
      byBuffer[imageOffset + 0x06] | (byBuffer[imageOffset + 0x07] << 8));
    printf("code type: %d\n",byBuffer[imageOffset + 0x14]);  
#endif
    
    /* Return code type byte */
    return byBuffer[imageOffset + 0x14]; 
    
  }
  return 0xffffffff;  /* Unknown */
}
    
BOOL SaveROM (LPSTR lpszFile, LPWORD lpwCheckSum)
{
	FILE *hSaveFile;
	int i, j, n;
	WORD	wChksum, wAddr;

	hSaveFile = fopen (lpszFile, "wb");
	if (hSaveFile == 0) return (FALSE);

	// Write the file out, and compute the checksum as we go
	*lpwCheckSum = wChksum = 0;
	wAddr = 0;

	// Get a chunk o' bytes from the ROM and compute the checksum
	for (j = 0; j < dwOldROMSaveSize; j++)
	{
		byBuffer[j] = ROMByteRead (wAddr);
		wChksum += byBuffer[j];
		wAddr++;
	}

	// Write the chunk to the file
	n = fwrite (byBuffer, 1, dwOldROMSaveSize, hSaveFile);
	if (n != FILEIO_BUFFER)
	{
		fclose (hSaveFile);
		return (FALSE);
	}

	*lpwCheckSum = wChksum;
	fclose (hSaveFile);
	return (TRUE);
}

#define LOBYTE(w)					((BYTE)(w))
#define HIBYTE(w)					((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define LOWORD(l)					((WORD)((DWORD)(l) & 0xFFFF))
#define HIWORD(l)					((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))

// Structures
typedef struct tagPMINFOBLOCK {
	BYTE	Signature[4];
	WORD	EntryPoint;
	WORD	PMInitialize;
	WORD	BIOSDataSel;
	WORD	A0000Sel;
	WORD	B0000Sel;
	WORD	B8000Sel;
	WORD	CodeSegSel;
	BYTE	InProtectMode;
	BYTE	Checksum;
} PMINFOBLOCK;

void ROMErase (void)
{
	SendROMCommand (0x80);
	SendROMCommand (0x10);

	// Wait for the ROM to erase itself
	DisplayString (szROMErase);
	while ((ROMByteRead (0) & 0x80) == 0x0)
		;
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


				default:

					Delay15us (10000/15);
					break;
			}
			break;

		case MANID_SST:

			switch (byDevice)
			{
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
	for (i = 0; i < ROM_SIZE; i++)
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

#if 0
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
	FxU32 miscInit1;

	// Flag that an error here is not in the "critical" section of the
	// code; ie, when *some* of the data is written to the EEPROM.
	*lpbCritical = FALSE;

	// Read the file and compute the checksum
	*lpwCheckSum = wChksum = 0;
	bFirst = TRUE;

//Read the TRUE ROM size and recalc dwNewBinFileSize, if needed.
	read (hFile, byBuffer, FILEIO_BUFFER);
#if !BLIND_FLASH
	ROMSize = byBuffer[2];
	if(ROMSize > 0x40)
		dwNewBinFileSize = ROMSize * 512;

//Need to trick FLASH when flashing a 32K BIOS which is in a 64K binary.		
	if((ROMSize == 0x40) && (dwNewBinFileSize == 64*1024))
		dwNewBinFileSize = 0x40 * 512;
		
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
#else
	ROMSize = ROM_SIZE;
	dwNewBinFileSize = ROMSize;
#endif

	// Flag that an error here IS in the "critical" section of the code.
	// Basically, an error here will create an unbootable device!
	DisplayString (szBeginROM);
	*lpbCritical = TRUE;

	// If processing reaches here, then a valid ROM image is available for
	// downloading to the EEPROM. Rewind the file, and start reading again.
	lseek (hFile, 0, SEEK_SET);
	wCounter = wSectorSize;
	for (i = 0; i < (dwNewBinFileSize / FILEIO_BUFFER); i++)	//Use 32K/64k now
	{
		n = read (hFile, byBuffer, FILEIO_BUFFER);
		if (n != FILEIO_BUFFER) return (FALSE);			// Fatal error
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
	}

#if !BLIND_FLASH
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
    HWC_IO_LOAD(regBase, miscInit1, miscInit1);
    
    if (( dwOldROMSaveSize < 0x10000 ) && (miscInit1 & 0x0200))

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

	for (i = 0; i < 4; i++)
	{
		if (wCounter >= wSectorSize)
		{
			SendROMCommand (0xA0);
			wCounter = 0;
		}
		ROMByteWrite ((WORD) (0xFFF8 + i), byBuffer[(dwNewBinFileSize - 8) + i]);
		if (++wCounter >= wSectorSize)
		{
			if (!ROMVerifyWrite ((WORD) (0xFFF8 + i), byBuffer[(dwNewBinFileSize - 8) + i]))
				return (FALSE);									// Fatal error
		}
	}
#endif

	// Made it through -- now it is again (theoretically) a bootable device.
	*lpbCritical = FALSE;
	DisplayString (szCompleted);

	return (TRUE);
}
#endif


BOOL WriteROMFromFile (FILE *hFile, LPWORD lpwCheckSum, LPBOOL lpbCritical)
{
	WORD				wChksum, wAddr, wCounter;
	unsigned int	n, i, j;
	BOOL				bFirst;
	DWORD ROMSize;
	FxU32 miscInit1, imageType;
	
	// Flag that an error here is not in the "critical" section of the
	// code; ie, when *some* of the data is written to the EEPROM.
	*lpbCritical = FALSE;

	n = fread (byBuffer, 1, FILEIO_BUFFER, hFile);
	if (n != FILEIO_BUFFER) {
      printf("ERROR: file size is not what was expected\n");
	  return (FALSE);
	}
	
	ROMSize = byBuffer[2];
	
	// Don't do size hack for OpenFirmware ROMs.
	imageType = GetROMType(FXFALSE);
	
	if(imageType == 0) {
      if(ROMSize > 0x40)
        dwNewBinFileSize = ROMSize * 512;

      //Need to trick FLASH when flashing a 32K BIOS which is in a 64K binary.		
	  if((ROMSize == 0x40) && (dwNewBinFileSize == 64*1024))
	    dwNewBinFileSize = 0x40 * 512;
    }
		
	// Read the file and compute the checksum
	*lpwCheckSum = wChksum = 0;
	bFirst = TRUE;

    if(imageType == 0) {
	  if (!((byBuffer[0] == 0x55) && (byBuffer[1] == 0xAA))) // PC image
	  {
        printf("ERROR: the header is not valid (0x55 0xAA 0x40)\n");
	    return (FALSE);
	  }
    } else {                                                 // OpenFirmware image
	  if (!((byBuffer[0] == 0x55) && (byBuffer[1] == 0xAA) && (byBuffer[2] == 0x40)))
	  {
        printf("ERROR: the header is not valid (0x55)\n");
	    return (FALSE);
	  }
    }
        
	// Compute the checksum
	for (j = 0; j < dwNewBinFileSize; j++)
		wChksum += byBuffer[j];

	// Return the computed checksum. If the low order BYTE of the checksum
	// is NOT zero, then an invalid checksum exists. Since this would create
	// an un-bootable machine, do NOT burn the EEPROM.
	*lpwCheckSum = wChksum;
	if (LOBYTE (wChksum) != 0)
	{
      printf("ERROR: invalid checksum\n");
	  return (FALSE);
	}

	// Flag that an error here IS in the "critical" section of the code.
	// Basically, an error here will create an unbootable device!
	DisplayString (szBeginROM);
	
	*lpbCritical = TRUE;

	// If processing reaches here, then a valid ROM image is available for
	// downloading to the EEPROM. Rewind the file, and start reading again.

	wCounter = wSectorSize;
	for (j = 0; j < dwNewBinFileSize; j++)
	{
		if (wCounter >= wSectorSize)
		{
			SendROMCommand (0xA0);
			wCounter = 0;
		}
		wAddr = (WORD) j;
		ROMByteWrite (wAddr, byBuffer[j]);
		if (++wCounter >= wSectorSize)
		{
			if (!ROMVerifyWrite (wAddr, byBuffer[j]))
			{
              printf("ERROR: failed to write to the Flash ROM\n");
			  return (FALSE);				
			}					// Fatal error
		}
	}

    printf("transfer complete\n");
    
//#ifdef BANSHEE	// Can't use software to select 64k for programming in Voodoo 3.

	// 3Dfx always uses a 64K ROM device. Even though the signature is for
	// a 32K ROM and the VGA ROM must ALWAYS be 32K, for some reason, the
	// sub-vendor ID needs to show up at the end of the device. Note that
	// this signature is not visible by any system software.
	// The sub-vendor ID are four BYTEs starting at the 8 BYTEs from the
	// end of the buffer. Assume that "byBuffer" is at least 8 BYTEs.
	// Write FF's consectively until it's time to write the ID's.

    HWC_IO_LOAD(bInfo.regInfo, miscInit1, miscInit1);
    
#if DEBUG    
    printf("dwNewBinFileSize: %08lx  miscInit1: %08lx\n",dwNewBinFileSize,miscInit1);
#endif
    
    if (( dwNewBinFileSize < 0x10000 ) /* && (miscInit1 & 0x02000000) */) {
    	
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
			    {
                  printf("ERROR: failed to write to the Flash ROM\n");
			      return (FALSE);				
			    }					// Fatal error
			}
		}
	
		for (i = 0; i < 4; i++)
		{
			if (wCounter >= wSectorSize)
			{
				SendROMCommand (0xA0);
				wCounter = 0;
			}
			ROMByteWrite ((WORD) (0xFFF8 + i), byBuffer[(dwNewBinFileSize - 8) + i]);
			if (++wCounter >= wSectorSize)
			{
				if (!ROMVerifyWrite ((WORD) (0xFFF8 + i), byBuffer[(dwNewBinFileSize - 8) + i]))
			    {
                  printf("ERROR: failed to write to the Flash ROM\n");
			      return (FALSE);				
			    }					// Fatal error
			}
		}
	}
//#endif

	// Made it through -- now it is again (theoretically) a bootable device.
	*lpbCritical = FALSE;
	DisplayString (szCompleted);

	return (TRUE);
}

//
//		CheckSum - Compute the checksum of the buffer
//
//		Entry:	None
//		Exit:		<BYTE>	Checksum value
//
BYTE CheckSum (void)
{
	BYTE		bySum;
	long int	i;

	for (bySum = 0, i = 0; i < INPUT_BUFFER_SIZE - 1; i++)
		bySum += byBuffer[i];

	return (bySum);
}

//
//		FixUpBuffer - Compute a checksum, add the date stamp to the BIOS
//
//		Entry:	None
//		Exit:		None
//
void FixUpBuffer (void)
{
	static char	sDateFlag[] = "MM/DD/YY";
	char			sNow[16];
	//time_t		ltime;
	struct tm	*ptm;
	long int		i, j, nPCIHeader;
	BYTE			sum;
	WORD			wPCIVendor, wPCIDevice;
	BOOL			bPCIFound;

#ifdef COMPAQ
	static char	sCompaqString[] = "V1A 05COMPAQ";

	memcpy (byBuffer + COMPAQ_STRING_OFFSET, sCompaqString,
				sizeof (sCompaqString) - 1);
#endif
#if 0
	if (strnicmp (byBuffer + OFF_DATE, sDateFlag, 8) == 0)
	{
		time (&ltime);
		ptm = localtime (&ltime);
		sprintf (sNow, "%02d/%02d/%02d", ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_year % 100);
		memcpy (byBuffer + OFF_DATE, sNow, 8);
	}
	memcpy (byBuffer + OFF_VENDOR, sVendor, 16);
#endif

	// Search for the PMID string in the VBE 3.0 BIOS. If one exists,
	// then add the checksum at the end of the data structure.
	for (i = 0; i < INPUT_BUFFER_SIZE; i++)
	{
		if (byBuffer[i] == 'P')
		{
			printf ("-- found a PMID string: adjust checksum\n");
			if (memcmp (&byBuffer[i], "PMID", 4) == 0)
			{
				sum = 0;
				for (j = 0; j < sizeof (PMINFOBLOCK) - 1; j++)
					sum += byBuffer[i + j];
				byBuffer[i + (sizeof (PMINFOBLOCK) - 1)] = 0 - sum;
				break;
			}
		}
	}

	// Search for the PCIR string in a PCI BIOS. If one exists,
	// then grab the Vendor and Device ID and place them at the
	// end of the ROM.
	bPCIFound = FALSE;
	for (i = 0; i < INPUT_BUFFER_SIZE; i++)
	{
		if (byBuffer[i] == 'P')
		{
			if (memcmp (&byBuffer[i], "PCIR", 4) == 0)
			printf ("-- found a PCIR string: adjust checksum\n");
			{
				wPCIVendor = (WORD) (byBuffer[i + 4] | (byBuffer[i + 5] << 8));
				wPCIDevice = (WORD) (byBuffer[i + 6] | (byBuffer[i + 7] << 8));
				byBuffer[INPUT_BUFFER_SIZE - 8] = LOBYTE (wPCIVendor);
				byBuffer[INPUT_BUFFER_SIZE - 7] = HIBYTE (wPCIVendor);
				byBuffer[INPUT_BUFFER_SIZE - 6] = LOBYTE (wPCIDevice);
				byBuffer[INPUT_BUFFER_SIZE - 5] = HIBYTE (wPCIDevice);
				nPCIHeader = i;
				bPCIFound = TRUE;
				break;
			}
		}
	}

	// If a PCI header was found in the ROM, make sure that it is
	// aligned on a DWORD boundary. If it is not, then first verify
	// that there is a WORD of zeros preceding the table and then
	// move the table back by a WORD. Due to a MASM bug/quirk the
	// DWORD alignment can NOT be guaranteed, but a WORD alignment
	// can be guaranteed.
	if (bPCIFound)												// PCI header exists?
	{
		if ((nPCIHeader & 0x03) != 0x00)					// DWORD aligned?
		{
			if ((byBuffer[nPCIHeader - 2] == 0) &&		// Room to move?
				(byBuffer[nPCIHeader - 1] == 0))
			{
				printf ("\nAdjusting PCI header.");
				for (i = 0; i < 24; i++)
				{
					byBuffer[(nPCIHeader - 2) + i] = byBuffer[nPCIHeader + i];
				}
				// Update the pointer to the header
				if (byBuffer[0x18] == 0x00) byBuffer[0x19]--;
				byBuffer[0x18] -= 2;
			}
		}
	}

	// Compute and store the checksum
	*(byBuffer + (INPUT_BUFFER_SIZE - 1)) = 0;
	*(byBuffer + (INPUT_BUFFER_SIZE - 1)) = 0 - CheckSum ();
}

FxBool findBoardAddresses(FxU32 vendorID, FxU32 deviceIDmin, FxU32 deviceIDmax, FxU32 boardNum)
{
	RegEntryID boardRegEntryID, regEntryID;
	RegEntryIter regEntryIter;
	RegPropertyIter propIter;
	RegIterationOp regIterOp;
	
	RegPropertyNameBuf propName;
	RegPropertyValueSize propSize;
	char propValBuf[256];

	static const char nameProperty[] = "name";

	FxU32 entryVendorID, entryDeviceID, i, bNum = boardNum;
	FxBool haveVendorID, haveDeviceID, foundBoard, success;	
	
	OSErr err = noErr;
	Boolean done = true, propDone;
	foundBoard = success = FXFALSE;
	
	err = RegistryEntryIDInit(&regEntryID);
	if(err == noErr) {
		
		err = RegistryEntryIterateCreate(&regEntryIter);
		if(err == noErr) {
			
			regIterOp = kRegIterDescendants;
			
			do {
				
				/* Iterate through the device tree */
				haveVendorID = FXFALSE;
				haveDeviceID = FXFALSE;
				
				err = RegistryEntrySearch(&regEntryIter, regIterOp, &regEntryID, &done,
														 nameProperty, NULL, 0);
														 
				if((err != noErr) || done) break;

				/* Now iterate through the properties. */
				err = RegistryPropertyIterateCreate(&regEntryID, &propIter);
				if(err == noErr) {
					
					do {
						err = RegistryPropertyIterate(&propIter, propName, &propDone);
						if((err == noErr) && !propDone) {
						
							err = RegistryPropertyGetSize(&regEntryID, propName, &propSize);
							if((err == noErr) && (propSize < sizeof(propValBuf))) {
							
								err = RegistryPropertyGet(&regEntryID, propName, &propValBuf, &propSize);
								if(err == noErr) {
									
									FxU32 curPropVal = *(FxU32 *)propValBuf;
									
									/* Check for vendor-id and device-id */
									if(!strcmp(propName,"vendor-id")) {
										entryVendorID = curPropVal;
										haveVendorID = FXTRUE;
									} else if(!strcmp(propName,"device-id")) {
										entryDeviceID = curPropVal;
										haveDeviceID = FXTRUE;
									}
								}
							}
						}
					} while(!(haveVendorID && haveDeviceID) && (err == noErr) && !propDone);
					
					err = noErr;
					
					RegistryPropertyIterateDispose(&propIter);
					
					/* See if we found a candidate entry */
					if(haveVendorID && haveDeviceID) {
					
						/* Check for a matching board */
						if((entryVendorID == vendorID) && 
						   ((entryDeviceID >= deviceIDmin) &&
						    (entryDeviceID <= deviceIDmax))) {
							if(boardNum == 0) {
								foundBoard = FXTRUE;
								RegistryEntryIDCopy(&regEntryID, &boardRegEntryID);
								break;
							}
							/* We found a board, but we're looking for a differnt one... keep going */
							boardNum--;
						}
					}
				}
				
				/* Device did not match, so continue along the device tree. */
				regIterOp = kRegIterContinue;
				
			} while(!done && (err == noErr));
			
			/* If we found the board, then grab the info we need from it. */			
			if(foundBoard) { 
#define kNumAssignedAddresses 4
				PCIAssignedAddress assignedAddresses[kNumAssignedAddresses];		// There should be three "phys-addr size" pairs 
				UInt32 applAddress[kNumAssignedAddresses];							// There should be three logical addresses
				UInt32 baseRegister0Index;						// Entry in APPL,address for Base Register 0
				UInt32 baseRegister1Index = 0;					// Entry in APPL,address for Base Register 1
				UInt32 baseRegister2Index;						// Entry in APPL,address for Base Register 2
				UInt32 baseRegister3Index;						// Entry in APPL,address for Base Register 2
			
			  /* Make sure it's the size we expect */
				err = RegistryPropertyGetSize(&boardRegEntryID, kPCIAssignedAddressProperty, &propSize);
				if((err == noErr) && (propSize <= sizeof(assignedAddresses))) {
					
					/* Grab the property */
					err = RegistryPropertyGet(&boardRegEntryID, kPCIAssignedAddressProperty, assignedAddresses, &propSize);
					if(err == noErr) {
					
						/* This is done because the order of the entries is the assigned addresses property do not necessarily
						   come in the same order as the hardware registers. */	
						for(i = 0; i < kNumAssignedAddresses; i++) {
							if(assignedAddresses[i].registerNumber == 0x10)
							{
								baseRegister0Index = i;
							}
							else if(assignedAddresses[i].registerNumber == 0x14)
							{
								baseRegister1Index = i;
							}
							else if(assignedAddresses[i].registerNumber == 0x18)
							{
								baseRegister2Index = i;
							}
							else if(assignedAddresses[i].registerNumber == 0x30)
							{
								baseRegister3Index = i;
							}
						}
					
						err = RegistryPropertyGetSize(&boardRegEntryID, "AAPL,address", &propSize);
						if((err == noErr) && (propSize <= sizeof(applAddress))) {
						
							err = RegistryPropertyGet(&boardRegEntryID, "AAPL,address", applAddress, &propSize);
							
							if(err == noErr) {
								_hwBaseAddress[bNum] = applAddress[baseRegister0Index];
								_lfbBaseAddress[bNum] = applAddress[baseRegister1Index];
								_ioPortAddress[bNum] = applAddress[baseRegister2Index];
								_romBaseAddress[bNum] = applAddress[baseRegister3Index];
								_gBoardRegEntryID[bNum] = boardRegEntryID;
								
								/* Get slot name too. */
								propSize = sizeof(_slotName[bNum]);
								err = RegistryPropertyGet(&boardRegEntryID, "AAPL,slot-name", &_slotName[bNum][0], &propSize);
								
								/* Get model name too. */
								propSize = sizeof(_modelName[bNum]);
								err = RegistryPropertyGet(&boardRegEntryID, "model", &_modelName[bNum][0], &propSize);
								
								
								
								success = FXTRUE;
							}
						}
					}
				}
			}
			
		}
	}
	
	return success;
}

void toupper(char *src, char *dst)
{
  char c;
  
  do {
    c = *src++;
    
    if(c >= 'a' && c <= 'z') {
      c += 'A' - 'a';
    }
    
    *dst++ = c;
  } while(c);
}

int main(int ac, char **av)
{
  WORD wRdChecksum, wWrChecksum;
  BOOL bSave, bBurn, bCritical;
  FxU32 i, numBoards = 0, bNum, imageType;		
  FILE *romFile;
  char romImageName[256], response;
  char romFileName[256], *romSaveName;
  char tempRomImageName[256];
  BYTE byOldROMSize;
  FSSpec theRsrcFile;
  short gRefNum;
  OSErr err = noErr;
  	
  SIOUXSettings.autocloseonquit = 0;
  
#if MAKE_ROM_FILE

  printf("ROM File Name:");
  i = scanf("%255s",romFileName);
  if(i < 1) {
    SIOUXSettings.autocloseonquit = 0;
    return 0;
  }
  
  // Make ROM .img from raw .rom file
  if(romFile = fopen(romFileName,"rb")) {
    memset(byBuffer,0xff,ROM_SIZE);
    fread(byBuffer,1,ROM_SIZE,romFile); 
    fclose(romFile);
	 
    FixUpBuffer();

    printf("Image File Name: ");
    i = scanf("%255s",romImageName);
	if(i < 1) {
	  return 0;
	}
	
    if(romFile = fopen(romImageName,"wb")) {
      fwrite(byBuffer,1,ROM_SIZE,romFile); 
      fclose(romFile);
    }
//_____
	// Make a spec to it so we can open the thing.
	{
		tempRomImageName[0] = sprintf (&tempRomImageName[1], "%s.rsrc", romImageName); 
		err = FSMakeFSSpec(0, 0, (unsigned char *)tempRomImageName, &theRsrcFile);
		if (err == fnfErr)
		{
			FSpCreateResFile(&theRsrcFile, 'RSED', 'rsrc', smSystemScript);	
		}
		else if (err != noErr)
		{
			return err;
		}
	
		gRefNum = FSpOpenResFile(&theRsrcFile, fsRdWrPerm);
		if(gRefNum == -1)
			return ResError();
	}

//_____    
    {	/* add the resource */
		Handle		resHndl;
		short prefsID = 0;
		
		resHndl = NewHandleSys(ROM_SIZE);
		if (resHndl != 0)
		{
			BlockMoveData(byBuffer, *resHndl, ROM_SIZE);
			AddResource(resHndl, 'RomU', prefsID, 0);
			if (ResError() != noErr)
			{
				printf( "### ERROR ### rom resource was not created \n");
				// Do something.
			}
		}
	}

	if(gRefNum != -1)
	{
		UpdateResFile(gRefNum);
		CloseResFile(gRefNum);
		gRefNum = -1;
	}
	
	
  }
  
  
#else
  	
  for(i = 0; i < MAX_BOARDS; i++) {
    if(findBoardAddresses(0x121a,DEVICE_ID_MIN,DEVICE_ID_MAX,i)) {
      numBoards++;
    }
  }
	
  if(numBoards == 0) {
    return 0;
  }

getBoardNumber:

  /* Ask the user which board they want to flash. */
  printf("\nThe following candidate boards were found in your system:\n\n");
  
  for(i = 0; i < numBoards; i++) {
    printf("Board #%d in slot %s\n",i+1,_slotName[i]);
  }
  printf("\n");
  
  do {
    do {
      printf("Please enter the number of the board you wish to upgrade, or 0 to exit: ");
      i = scanf("%20d",&bNum);
    } while(i < 1);
  
    if(bNum == 0) {
      return 0;
    }
    if(bNum > numBoards) {
      printf("Board number out of range.\n");
    }
  } while(bNum > numBoards);
  
  bNum--;
    
  /* Copy over the relavent information from that board to our globals.  Man this
     code is ugly! */

  hwBaseAddress = _hwBaseAddress[bNum];
  lfbBaseAddress = _lfbBaseAddress[bNum];
  ioPortAddress = _ioPortAddress[bNum];
  gBoardRegEntryID = _gBoardRegEntryID[bNum];
  
  bInfo.regInfo.initialized = FXTRUE;
  bInfo.regInfo.ioMemBase = hwBaseAddress + SST_IO_OFFSET;
  bInfo.regInfo.cmdAGPBase = hwBaseAddress + SST_CMDAGP_OFFSET;
  bInfo.regInfo.waxBase = hwBaseAddress + SST_2D_OFFSET;
  bInfo.regInfo.sstBase = hwBaseAddress + SST_3D_OFFSET;
  bInfo.regInfo.lfbBase = hwBaseAddress + SST_LFB_OFFSET;
  bInfo.regInfo.rawLfbBase = (lfbBaseAddress & ~0xf);
  bInfo.regInfo.ioPortBase = (ioPortAddress & ~0xf);
  regBase = bInfo.regInfo.ioPortBase;
	
  /* Enable PCI memory and I/O decode just in case it isn't enabled already. */
  {
    FxU32 
      pciInit0,
      pciCommandReg = 3;
  
    ExpMgrConfigWriteLong(&gBoardRegEntryID, (LogicalAddress)0x4, pciCommandReg);
    
    HWC_IO_LOAD(bInfo.regInfo, pciInit0, pciInit0);
    pciInit0 |= SST_PCI_READ_WS | SST_PCI_WRITE_WS;
    HWC_IO_STORE(bInfo.regInfo, pciInit0, pciInit0);  
  }

  BeginROM();

  GetROMID (&byMan, &byDevice, &wSectorSize);
	
  if ( !(((byMan == MANID_AMD_TI) && (byDevice == AMD_TI_29010)) ||
         ((byMan == MANID_ATMEL) && (byDevice == ATMEL_29010)) ||
		 ((byMan == MANID_ATMEL) && (byDevice == ATMEL_49F010)) ||
		 ((byMan == MANID_SST) && (byDevice == SST_29EE010)) ||
		 ((byMan == MANID_SST) && (byDevice == SST_39SF010)) ||
		 ((byMan == MANID_SST) && (byDevice == SST_39VF512)))
		)
	{
		EndROM ();
		printf(szUnknownDevice,byMan,byDevice);
        SIOUXSettings.autocloseonquit = 1;
		exit (1);
	}

	/* Figure out if it's a PC or Mac ROM image. */	
	imageType = GetROMType(FXTRUE);
    
    if(imageType == 0) {
      printf("\nThis board currently contains an Intel x86 BIOS image.\n"); 
    } else if (imageType == 1) {
      printf("\nThis board currently contains an OpenFirmware (Macintosh) ROM image.\n");
    } else {
      printf("\nThis board currently contains an unknown ROM image.\n");
    }
    
    printf("\nAre you SURE you want to replace the ROM image on this board? (y/n) ");
    i = scanf("%255s",romImageName);
    if(i < 1 || romImageName[0] != 'y') {
      EndROM();
      goto getBoardNumber;
	}

    for(;;) {
      printf("\nEnter ROM Image File Name: ");
      i = scanf("%255s",romImageName);
	  if(i < 1) {
        EndROM();
	    SIOUXSettings.autocloseonquit = 1;
	    return 0;
      }
      
      toupper(romImageName,tempRomImageName);
      
      if(strcmp(tempRomImageName,szROMSaveFileMac) && strcmp(tempRomImageName,szROMSaveFilePC))
        break;

      printf("\nFor safety, please choose a name other than PC-SAVE.ROM or MAC-SAVE.ROM\n");
    }

  	// Save the original ROM image off and erase it.
	//if(imageType == 1) {
	// This is a kludge.  The Mac tools always write in a value of 0x40 for some
	// reason, which confuses the rest of this code.   We assume that Banshee is 
	// always 32K, and Voodoo3 is always 64K.
	byOldROMSize = ROM_SIZE / 512;
	//} else {
	//  byOldROMSize = byBuffer[2];
    //}
    
    if(imageType == 1) {
      romSaveName = szROMSaveFileMac;
    } else {
      romSaveName = szROMSaveFilePC;
    }
    
    //If we're above 32K in size, we must be a 64K ROM binary.
    if (byOldROMSize > 0x40) {
      dwOldROMSaveSize = (64 * 1024);
      bSave = SaveROM (romSaveName, &wRdChecksum);
    } else {
      if (byOldROMSize == 0x40) {
         dwOldROMSaveSize = (32 * 1024);
         bSave = SaveROM (romSaveName, &wRdChecksum);
      }
      else
      {
        printf ("Old ROM Size error!\n");
    	bSave = FALSE;		
      }
    }
    
	/* Verify that the ROM is at least the correct size (and available). */
	if((romFile = fopen(romImageName,"rb"))) {
	  FxU32 size;
	  fseek(romFile,0,SEEK_END);
	  dwNewBinFileSize = ftell(romFile);
	  if(dwNewBinFileSize != ROM_SIZE) {
	    printf("ROM Image is not the expected size for this hardware.\n");
	    SIOUXSettings.autocloseonquit = 1;
	    EndROM ();
	    return 0;
      }	    	  
      fclose(romFile);
    } else {
      printf("File %s not found.\n",romImageName);
      SIOUXSettings.autocloseonquit = 1;
      EndROM ();
      return 0;
    }
 	      
	hInFile = fopen(romImageName,"rb");
	
	i = fread (byBuffer, 1, FILEIO_BUFFER, hInFile);
	if (i != FILEIO_BUFFER) {
      printf("Error reading ROM image\n");
      SIOUXSettings.autocloseonquit = 1;
      EndROM ();
      fclose(hInFile);
      return 0; 
    }
    
    fseek(hInFile,0,SEEK_SET);
    
    // Determine the type of the new ROM.
    imageType = GetROMType(FXFALSE);
    if(imageType == 0) {
      printf("\nThis file contains an Intel x86 BIOS image.  Using this image will make this board usable in a PC and unusable in a Macintosh.\n");
    } else if (imageType == 1) {
      printf("\nThis file contains an OpenFirmware (Macintosh) ROM image.  Using this image will make this board usable in a Macintosh, but unusable in a PC.\n");
    } else {
      printf("\nThis file contains an unkown ROM image type.  Using this image will probably make the board unusable in both PC and Macintosh systems.\n");
    }

    printf("\nAre you SURE you want to use this image file? (y/n) ");
    i = scanf("%255s",romImageName);
    if(i < 1 || romImageName[0] != 'y') {
      EndROM();
      printf("Aborting update.\n");
      SIOUXSettings.autocloseonquit = 1;
      fclose(hInFile);
      return 0;
	}

  	if(bSave) {
  	   printf("\nOld ROM saved as %s\n",romSaveName);
  	}	

	ROMErase ();
    
	// Write the ROM image
	if (hInFile == NULL)
	{
	    /* This should never happen */
	    SIOUXSettings.autocloseonquit = 1;
	    EndROM();
        return 0;
	    
		//bBurn = WriteROMRestore (&wWrChecksum, &bCritical);
		// If an error occurs here, we're pretty much out of luck
	}
	else
	{
		bBurn = WriteROMFromFile (hInFile, &wWrChecksum, &bCritical);
		// If an error occurs here, then on fatal errors attempt to
		// load in the EEPROM image from the resource.
		
		if (!bBurn)
		{
			if (bCritical)
			{
			    printf("Fatal error!  The EEPROM could not be written with a valid image.  Please try flashing the image again.\n");
			    SIOUXSettings.autocloseonquit = 1;
	            EndROM();
		        return 0;
			}
		}
	}

	// Restore the PCI state to its original state
	EndROM ();

#ifdef DEBUG
	printf ("\nbyMan = %02Xh, byDevice = %02Xh, wRdChecksum = %04Xh, wWrChecksum = %04Xh",
			byMan, byDevice, wRdChecksum, wWrChecksum);
#endif

	// If a file is still in use, close it before exiting
	if (hInFile != NULL) fclose (hInFile);
	if (hOutDevice != stdout) fclose (hOutDevice);

	if (bBurn)
		DisplayString (szSuccess);
	else
	{
		if (bCritical)
			DisplayString (szInvalidROM);
		else
			DisplayString (szNoSuccess);
	}

  RegistryEntryIDDispose(&gBoardRegEntryID);
#endif

	return 0;
}

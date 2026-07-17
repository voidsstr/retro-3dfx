//
//		MAIN.CPP - Main file for FLASH.EXE, the EEPROM image utility for 3Dfx's board
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/4/98
//		Last modified:	4/1/99
//
//		Command line:
//		FLASH [filename] | [/RESET] | [[/?] | [/HELP]]
//
//		Routines in this file:
//		main					Entry point
//		InitBoard			Initialize the board and memory space for EEPROM setup
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<i86.h>
#include	<fcntl.h>
#include	<io.h>
#include	"flash.h"

//
//		main - Entry point
//
//		Entry:	argc		Number of command line arguments
//					argv		Pointer to array of command line arguments
//		Exit:		<int>		DOS ERRORLEVEL
//
DWORD    dwOldROMSaveSize, dwNewBinFileSize;

int main (int argc, char **argv)
{
	WORD	wRdChecksum, wWrChecksum;
	BOOL	bSave, bBurn, bCritical;
	BYTE	byOldROMSize;
    BOOL    bValidImage;
    WORD    wBoard2Program;

#ifdef		USERFLASH
	int		i, ArgMatchCount;
#endif		//USERFLASH

	selData = FP_SEG (&hOutDevice);			// Use current DS

	DisplayCopyright ();

#ifdef		USERFLASH
	// Setup the for command line switches 

	// Keep old BIOS register values
	bKeepOldRegs = TRUE;

	// Run Hidden
	bRunWindowed = FALSE;

	// Continue on after Version Check
	bOverrideChVers = FALSE;

	// Continue on after SSID Match Check
	bOverrideNoSSID = FALSE;

	// Continue on though No Hardware Info available
	bOverrideNoInfo = FALSE;

#endif		//USERFLASH

	// Determine command line parameters and display help message if
	// requested. If not, and a valid filename is given, then open
	// the command file for processing.
	if (argc == 2)
	{
		// Help requested?
		if ((stricmp (argv[1], szHelp0) == 0) || (stricmp (argv[1], szHelp1) == 0))
		{
			DisplayUsage ();
			exit (1);
		}
		// Reset to older ROM image?
		if (stricmp (argv[1], szReset) == 0)
		{
			hInFile = NULL;
			DisplayString (szResetMsg);
		}
		// Open ROM image file
		else if ((hInFile = open (argv[1], O_RDONLY | O_BINARY)) <= 0)
		{
			DisplayString (szNoFile);

#ifdef		USERFLASH
			exit (2);
#else	
			exit (1);
#endif		//USERFLASH
		}
	}
	else
	{
#ifdef		USERFLASH
		// It should be from 3 to 7 arguments
		if ((argc > 2) && (argc < 8))
		{
			ArgMatchCount = 2;
			for (i = 2; i < argc; i++)
			{
				// Update to new BIOS register values?
				if (stricmp (argv[i], szUpdateRegs) == 0)
				{
					bKeepOldRegs = FALSE;
					ArgMatchCount++;
				}

				// Check if Run Hidden is requested.
				if (stricmp (argv[i], szRunWindowed) == 0)
				{
					bRunWindowed = TRUE;
					ArgMatchCount++;
				}

				// Check if Overide Version Check.
				if (stricmp (argv[i], szOverrideChVers) == 0)
				{
					bOverrideChVers = TRUE;
					ArgMatchCount++;
				}

				// Check if Overide SSID Check.
				if (stricmp (argv[i], szOverrideNoSSID) == 0)
				{
					bOverrideNoSSID = TRUE;
					ArgMatchCount++;
				}

				// Check if Overide No info Check.
				if (stricmp (argv[i], szOverrideNoInfo) == 0)
				{
					bOverrideNoInfo = TRUE;
					ArgMatchCount++;
				}
			}

			// If argc and the Argument Matched Count aren't equal now
			// then an invalid argument has been entered!
			if (argc != ArgMatchCount)
			{ 
				DisplayString (szInvalidArgs);
				exit (3);
			}

			if ((hInFile = open (argv[1], O_RDONLY | O_BINARY)) <= 0)
			{
				DisplayString (szNoFile);
				exit (2);
			}
		}
		else
		{
			DisplayString (szInvalidArgs);
			exit (3);
		}
	}


	// Check if Save File is being flashed back
	if (stricmp (argv[1], szROMSaveFile) == 0)
	{
		bSaveInUse = TRUE;
	}
	else
	{
		bSaveInUse = FALSE;
	}
#else		
		DisplayString (szInvalidArgs);
		exit (1);
	}
#endif		//USERFLASH
    PCIEnumerate();

    if(wNumBoards == 0)
    {
    	DisplayString (szNoBoard);

#ifdef		USERFLASH
		exit (4);
#else
		exit (1);
#endif		//USERFLASH

    }
    else if(wNumBoards > 1)
    {
       wBoard2Program = SelectBoard();
    }
    else
    {
       wBoard2Program = 0;
    }

    bValidImage = ValidateNewROM(wBoard2Program);


    if( bValidImage == TRUE)
    {
    	// Allocate a flat-model descriptor for use in all memory operations.
    	// Note that, even with a "flat" descriptor, physical addresses will
    	// still have to be converted to linear addresses prior to use.
    	selFlat = AllocateFlatDescriptor ();

    	// Determine if 3Dfx board is in the system here (without BIOS)...
    	if (!InitBoard (wBoard2Program) )
    	{
    		DisplayString (szSetupFailed);

#ifdef		USERFLASH
    		exit (5);
#else
    		exit (1);
#endif		//USERFLASH

    	}

    	// Begin the ROM "burn" sequence by first accessing the ROM and
    	// determining if it is a known device.
    	BeginROM ();
    	GetROMID (&byMan, &byDevice, &wSectorSize);
    	if ( !(((byMan == MANID_AMD_TI) && (byDevice == AMD_TI_29010)) ||
    			((byMan == MANID_ATMEL) && (byDevice == ATMEL_29010)) ||
    			((byMan == MANID_ATMEL) && (byDevice == ATMEL_49F010)) ||
    			((byMan == MANID_ATMEL) && (byDevice == ATMEL_49BV512)) ||
    			((byMan == MANID_ATMEL) && (byDevice == ATMEL_29LV512)) ||
    			((byMan == MANID_SST) && (byDevice == SST_29LE512)) ||
    			((byMan == MANID_SST) && (byDevice == SST_29EE010)) ||
    			((byMan == MANID_SST) && (byDevice == SST_39SF010)) ||
    			((byMan == MANID_SST) && (byDevice == SST_39VF512)))
    		)
    	{
    		EndROM ();
    		DisplayFormattedString (szUnknownDevice, byMan, byDevice);

#ifdef		USERFLASH
    		exit (6);
#else
    		exit (1);
#endif		//USERFLASH

		}

#ifdef		USERFLASH
	
// Check if the version number of the new file is later than the existing BIOS.
		bLaterVersion = CheckLaterVersion(hInFile); 

// If not see if they want to continue.
		if (bLaterVersion == FALSE)
		{
			// If not running hidden, show the message.
			if(bRunWindowed == FALSE)
			{
				bLaterVersion = ContinueYN("\nThe file version is older than the existing BIOS.");
				if (bLaterVersion == FALSE)
				{
					close (hInFile);
					exit (7);
				}
			}
			else
			{
				// If running hidden the message can't be seen and the program
				// will be stopped, unless the command switch overides this.
				if(bOverrideChVers == FALSE)
				{
					exit (7);
				}
			}
		}

		if (bKeepOldRegs)
		{
			// Save off the Register settings of the old BIOS to be copied
			// into the new BIOS, so that the existing register settings
			// won't actually change.
			SaveOffOldRegs ();
		}
#endif		//USERFLASH

    	// Save the original ROM image off and erase it.

    	byOldROMSize = ROMByteRead(0x0002);

    //If we're above 32K in size, we must be a 64K ROM binary.
    	if (byOldROMSize > 0x40)
    //    if (byOldROMSize == 0x80)
        {
            dwOldROMSaveSize = (64 * 1024);
#ifdef		USERFLASH
			if (bSaveInUse)
			{
				bSave = TRUE;
			}
			else
			{
				bSave = SaveROM (szROMSaveFile, &wRdChecksum);
			}
#else	
            bSave = SaveROM (szROMSaveFile, &wRdChecksum);
#endif		//USERFLASH
        }
        else
        {
            if (byOldROMSize == 0x40)
            {
                dwOldROMSaveSize = (32 * 1024);
#ifdef		USERFLASH
				if (bSaveInUse)
				{
					bSave = TRUE;
				}
				else
				{
					bSave = SaveROM (szROMSaveFile, &wRdChecksum);
				}
#else	
                bSave = SaveROM (szROMSaveFile, &wRdChecksum);
#endif		//USERFLASH
            }
    		else
    		{
                printf ("Old ROM Size error!\n");
    			bSave = FALSE;		
    		}
        }
    //	bSave = SaveROM (szROMSaveFile, &wRdChecksum);
    	if (bSave)
    		DisplayString (szSaveSuccess);
    	else
    		DisplayString (szNoSave);
    	ROMErase ();

    	// Write the ROM image
    	if (hInFile == NULL)
    	{
    		bBurn = WriteROMRestore (&wWrChecksum, &bCritical);
    		// If an error occurs here, we're pretty much out of luck
    	}
    	else
    	{
#ifdef		USERFLASH
			 // Do this if the Regs aren't to be updated with new settings
			if (bKeepOldRegs)
			{
				// Get the pointer to the Register settings table
				// in the new file.
				wNewRegPtr = GetNewRegsSettingPtr ();
			}
#endif		//USERFLASH

         	dwNewBinFileSize = filelength (hInFile);
    		bBurn = WriteROMFromFile (hInFile, &wWrChecksum, &bCritical);
    		// If an error occurs here, then on fatal errors attempt to
    		// load in the EEPROM image from the resource.
    		if (!bBurn)
    		{
    			if (bCritical)
    			{
    				DisplayString (szBurnError);
    				bBurn = WriteROMRestore (&wWrChecksum, &bCritical);
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
    	if (hInFile != NULL) close (hInFile);
    	if (hOutDevice != stdout) fclose (hOutDevice);

    	// Free the flat model descriptor
    	FreeDescriptor (selFlat);

    	if (bBurn)
    		DisplayString (szSuccess);
    	else
    	{
    		if (bCritical)
    			DisplayString (szInvalidROM);
    		else
    			DisplayString (szNoSuccess);
    	}
    }
	  else
	  {

#ifdef		USERFLASH
		exit (8);
#else	
    	exit (1);
#endif		//USERFLASH
	  }

	return (0);
}

//
//		InitBoard - Initialize the board and memory space for EEPROM setup
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Board ready, FALSE = Board not found)
//
BOOL InitBoard (WORD wBoard2Program)
{
	REALMODECALL	rmc;

    // Need to compute the bus/dev ID for the real mode calls to work.
    wBusDevID = (WORD) (sBoardsDesc[wBoard2Program].wBusID << 8) |  
       (sBoardsDesc[wBoard2Program].wSlotNum << 3);

    wBusDevID = wBusDevID & 0xffff;

	// Retreive the base address of the memory mapped registers
	rmc.eax = 0xB10A;
    rmc.ebx = wBusDevID;
	rmc.edx = PCI_ID;
    rmc.esi = 0x0000;
	rmc.edi = 0x0010;
	RealModeINT (&rmc, 0x1A);
	if ((rmc.eax & 0x0000FF00) != 0) return (FALSE);
	dwPhysMemBase0 = rmc.ecx & 0xFFFFFFF0;
	lpLinMemBase0 = (BYTE __far *) MK_FP (selFlat, Phys2Linear (dwPhysMemBase0, 0x2000000));

	// Save the address of MiscInit0 register since we use it a lot.
	lpLinMiscInit0 = (DWORD __far *) (lpLinMemBase0 + 0x10);

	// Save the address of MiscInit1 register since we use it a lot.
	lpLinMiscInit1 = (DWORD __far *) (lpLinMemBase0 + 0x14);

	// Retreive the I/O base address
	rmc.eax = 0xB10A;
	rmc.edi = 0x0018;
	RealModeINT (&rmc, 0x1A);
	if ((rmc.eax & 0x0000FF00) != 0) return (FALSE);
	wIOBase = (WORD) (rmc.ecx & 0xFFF8);

	// Retreive the original ROM address
	rmc.eax = 0xB10A;
	rmc.edi = 0x0030;
	RealModeINT (&rmc, 0x1A);
	if ((rmc.eax & 0x0000FF00) != 0) return (FALSE);
	dwPhysOrgROMBase = (DWORD) (rmc.ecx & 0xFFFFFFF0);
	dwLinOrgROMBase = Phys2Linear (dwPhysOrgROMBase, 0x10000);

	// Retreive the command register
	rmc.eax = 0xB10A;
	rmc.edi = 0x0004;
	RealModeINT (&rmc, 0x1A);
	if ((rmc.eax & 0x0000FF00) != 0) return (FALSE);
	wPCICommand = (WORD) (rmc.ecx & 0xFFFF);

	dwPhysROMBase = dwPhysMemBase0 + 0xA00000;
	lpLinROMBase = (BYTE __far *) MK_FP (selFlat, Phys2Linear (dwPhysROMBase, 0x10000));

	return (TRUE);
}

WORD SelectBoard(void)
{
   int  device, bus, slot;
   int  i, DeviceCounter;
   int  choice;
	 char szDevice[20];
   
   DisplayFormattedString("\n Please Select a Device to Program:\n\n");
   for(i = 0; i < wNumBoards; i++)
   {
      bus    = sBoardsDesc[i].wBusID;
      slot   = sBoardsDesc[i].wSlotNum;
      device = sBoardsDesc[i].wDeviceID;
			
		 for (DeviceCounter = 0;DeviceCounter <NUM_TDFX_CHIPS; DeviceCounter++)
		 	if(TdfxDeviceName[DeviceCounter].ChipID == device)
			{
		 		strcpy(szDevice,TdfxDeviceName[DeviceCounter].ChipName);
				break;
			}

      DisplayFormattedString("   %d. %s %s in Slot %d\n", i+1, 
                                                          szBusType[bus], 
                                                          szDevice, 
                                                          slot);
   }
   DisplayFormattedString("\nEnter Choice: ");
   scanf("%d", &choice);
   fflush(stdin);

   if( (choice < 1) || (choice > (int)wNumBoards) )
   {
      DisplayString("Invalid Choice Please try again !!!\n");
      exit(-1);
   }
   return((WORD)(choice-1));
}


BOOL ValidateNewROM(WORD wBoard2Program)
{
//   WORD         	wDeviceID;
   char         	bFileData[FILEIO_BUFFER]; 
   BOOL         	bSuccess = TRUE;
   char         	*pData, *pSpeed, *pSRAM;
//   BOOL         	bFoundV3 = FALSE, bFoundNapalm = FALSE, bFoundRampage = FALSE;
   int          	VersionStart = 0;
   ChipSpeedType	eSpeed;
   ChipRAMType   eRAM;

	 BYTE			  bCounter;

// Now Make sure that this ROM can update the Selected board
   WORD       	wSSDevID;
	WORD		ROMConfigOffset, OEMCfgOffset, BoardConfig;
   int        	i;
   int        	NumBuilds, NumSpeed;
   BOOL       	bFoundBuild = FALSE;
   ChipBusType  	eBusType;

   wDeviceID = sBoardsDesc[wBoard2Program].wDeviceID;

   // Read in the first 256 bytes and then reset the file pointer.  Then
   // find the start of the version string and save it in pData.
   read (hInFile, bFileData, FILEIO_BUFFER);
   lseek(hInFile, 0, SEEK_SET);

   pData = NULL;

//***************************************************************************
//Look for Voodoo3 string.  If string found, we assume a Voodoo3 board.
   while( (VersionStart < (FILEIO_BUFFER - strlen(sVoodoo3))) && (pData == NULL) )
   {
      pData = strstr(&(bFileData[VersionStart]), sVoodoo3);
      VersionStart++;
   }

//Check if we're the BIOS we're flashing is a Voodoo3 BIOS
   if( pData != NULL)
   {
//ROM file is a Voodoo3 BIOS
      bFoundV3 = TRUE;
   }

	if ((bFoundV3 == FALSE) && ((wDeviceID == 4) || (wDeviceID == 5)))
	{
		VersionStart = 0;

		while((VersionStart < (FILEIO_BUFFER - strlen(sVd3Velocity))) && (pData == NULL))
	   {
	      pData = strstr(&(bFileData[VersionStart]), sVd3Velocity);
	      VersionStart++;
	   }

//Check if the BIOS we're flashing is a Velocity (Voodoo3) BIOS (Yea, Marketing!)
	   if( pData != NULL)
	   {
//ROM file is a Voodoo3 BIOS
	      bFoundV3 = TRUE;
	   }
	}

//If pData is not NULL all the rest of the checks will report 
//as FOUND 
//***************************************************************************
	VersionStart = 0;

//Look for Voodoo4 string.  If string found, we assume a Voodoo4 board.
   while( (VersionStart < (FILEIO_BUFFER - strlen(sVoodoo4))) && (pData == NULL) )
   {
      pData = strstr(&(bFileData[VersionStart]), sVoodoo4);
      VersionStart++;
   }

//Check if we're the BIOS we're flashing is a Napalm BIOS
   if( pData != NULL)
   {
//ROM file is a Napalm BIOS
      bFoundNapalm = TRUE;
   }
//***************************************************************************
	VersionStart = 0;

//Look for Voodoo5 string.  If string found, we assume a Voodoo5 board.
   while( (VersionStart < (FILEIO_BUFFER - strlen(sVoodoo5))) && (pData == NULL) )
   {
      pData = strstr(&(bFileData[VersionStart]), sVoodoo5);
      VersionStart++;
   }

//Check if we're the BIOS we're flashing is a Napalm BIOS
   if( pData != NULL)
   {
//ROM file is a Napalm BIOS
      bFoundNapalm = TRUE;
   }

//***************************************************************************
	VersionStart = 0;

//Look for Rampage string.  If string found, we assume a Rampage board.
   while( (VersionStart < (FILEIO_BUFFER - strlen(sRampage))) && (pData == NULL) )
   {
      pData = strstr(&(bFileData[VersionStart]), sRampage);
      VersionStart++;
   }

//Check if we're the BIOS we're flashing is a Rampage BIOS
   if( pData != NULL)
   {
//ROM file is a Rampage BIOS
      bFoundRampage = TRUE;
   }
//***************************************************************************
//Is the card we're flashing a Banshee
   if(wDeviceID == 3) // Banshee
   {
//We must be a Banshee card

//Are we trying to flash a Voodoo3 BIOS onto a Banshee card
      if(bFoundV3 == TRUE)
      {
//Yeap
         DisplayString("Cannot Program a Banshee with Voodoo3 BIOS! Please try again.\n");
         bSuccess = FALSE;
      }
   }
   else if ((wDeviceID == 4) || (wDeviceID == 5))
   {
		if (bFoundV3 == TRUE)
		{
//***** Voodoo3 FLASH Init code *****
//Copy the V3 structures into the generic structures
//I had to do it this way because the sizeof command didn't accruately 
//calcualte the size of structure.  It was always one entry short.
			memcpy(sChipSpeedVer,sV3SpeedVer,sizeof(ChipSpeedVer) * NUM_V3_SPEED);
			memcpy(sChipBuilds,sV3Builds,sizeof(ChipBuildInfo) * NUM_V3_BUILDS);

//			memcpy(sChipSpeedVer,sV3SpeedVer,sizeof(sV3SpeedVer));
//			memcpy(sChipBuilds,sV3Builds,sizeof(sV3Builds));

//The number of Voodoo3 board combinations we support.
     	NumBuilds = NUM_V3_BUILDS;
			NumSpeed  = NUM_V3_SPEED;
		}
		else
			bSuccess = FALSE;
	 }
   else if ( (wDeviceID == ChipIdNapalm) || (wDeviceID == ChipIdNapalm2))
   {
//***** Napalm FLASH Init code *****
		if (bFoundNapalm == TRUE)
		{
//Copy the Napalm structures into the generic structures

//I had to do it this way because the sizeof command didn't accruately 
//calcualte the size of structure.  It was always one entry short.
			memcpy(sChipSpeedVer,sNapalmSpeedVer,sizeof(ChipSpeedVer) * NUM_NAPALM_SPEED);
			memcpy(sChipBuilds,sNapalmBuilds,sizeof(ChipBuildInfo) * NUM_NAPALM_BUILDS);

//			memcpy(sChipSpeedVer,sNapalmSpeedVer,sizeof(sNapalmSpeedVer));
//			memcpy(sChipBuilds,sNapalmBuilds,sizeof(sNapalmBuilds));

//The number of Voodoo3 board combinations we support.
     	NumBuilds = NUM_NAPALM_BUILDS;
			NumSpeed  = NUM_NAPALM_SPEED;
		}
		else
			bSuccess = FALSE;
	 }
   else if (wDeviceID == ChipIdRampage)
   {
//***** Rampage FLASH Init code *****
		if (bFoundV3 == TRUE)
		{
//Copy the V3 structures into the generic structures
//I had to do it this way because the sizeof command didn't accruately 
//calcualte the size of structure.  It was always one entry short.
			memcpy(sChipSpeedVer,sRampageSpeedVer,sizeof(ChipSpeedVer) * NUM_RAMPAGE_SPEED);
			memcpy(sChipBuilds,sRampageBuilds,sizeof(ChipBuildInfo) * NUM_RAMPAGE_BUILDS);

//			memcpy(sChipSpeedVer,sV3SpeedVer,sizeof(sV3SpeedVer));
//			memcpy(sChipBuilds,sV3Builds,sizeof(sV3Builds));

//The number of Voodoo3 board combinations we support.
     	NumBuilds = NUM_V3_BUILDS;
			NumSpeed  = NUM_V3_SPEED;
		}
		else
			bSuccess = FALSE;
	 }
	else
	{
//Unknown Board
	  	printf("\nUnable to identify board\n");
		bSuccess = FALSE;
	}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Are we trying o flash a Banshee BIOS onto a Voodoo3 card
      if((bFoundV3 == FALSE) && ((wDeviceID == 4) || (wDeviceID == 5)))
      {
//Yeap.
         DisplayString("Cannot Program a Voodoo3 with Banshee BIOS! Please try again.\n");
         bSuccess = FALSE;
      }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Figure out the speed of the board
		if((bSuccess == TRUE) && (wDeviceID != 3))
		{
		for (bCounter = 0; bCounter < NumSpeed; bCounter++)
			{
			pSpeed = strstr(pData, sChipSpeedVer[bCounter].String);

			if(pSpeed != NULL)
				{
           	eSpeed = sChipSpeedVer[bCounter].SpeedType;
           	bSuccess = TRUE;
				break;
				}
			else
           bSuccess = FALSE;
			}
		}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Now find the SRAM type on the Voodoo3 board

		if(((bSuccess == TRUE) && (wDeviceID != 3)) && (!bFoundNapalm))
      {
// Are we a Voodoo3 SDRAM board??
         pSRAM = strstr(pSpeed, sChipSRAMType[0]);

         if(pSRAM == NULL)
         {
// We're not a Voodoo3 SDRAM board.  Are we a Voodoo3 SGRAM board??
            pSRAM = strstr(pSpeed, sChipSRAMType[1]);

            if(pSRAM == NULL)
            {
// We're not a Voodoo3 SGRAM board. We don't know what we are.  
// EJECT! EJECT!! EJECT!!!
               DisplayString("Unknown RAM Type in BIOS File\n");
               bSuccess = FALSE;
            }
            else
            {
// We're a Voodoo3 SGRAM board.
              eRAM = CHIP_SG;
            }
         }
         else
         {
// We're a Voodoo3 SDRAM board.
            eRAM = CHIP_SD;
         }
      }
//**********************************************************************
		else 
		if (bFoundNapalm || bFoundRampage)
		{
//Get ROM binary offset to ROMConfig table
		   	ROMConfigOffset = *(bFileData+0x50) + ((*(bFileData+0x51))<<8 );

//Change the file offset to where the ROMConfig table is and read in data
//from data
   		lseek(hInFile, ROMConfigOffset, SEEK_SET);
   		read (hInFile, bFileData, FILEIO_BUFFER);

//The first entry in the ROmConfig table is the pointer to the
//OEMConfig table.  
		   	OEMCfgOffset = *(bFileData+0x0) + ((*(bFileData+0x1))<<8 );

//Move the file pointer to the OEMConfig table
   		lseek(hInFile, OEMCfgOffset, SEEK_SET);
   		read (hInFile, bFileData, FILEIO_BUFFER);

//Now lets read the OEMConfig BoardConfig word
//Need to fix this for Rampage.
//			if (bFoundRampage == TRUE)
////We must be a Rampage.  Read the board config from 0x32
//		   		BoardConfig = *(bFileData+0x32) + ((*(bFileData+0x33))<<8 );
//			else
//We must be a Rampage.  Read the board config from 0x2E
		   		BoardConfig = *(bFileData+0x2E) + ((*(bFileData+0x2F))<<8 );

//Check if SDRAM or SGRAM
			if ((BoardConfig & 0x03) == 1)
//Must be SDRAM
           	eRAM = CHIP_SD;
			else if ((BoardConfig & 0x03) == 0)
//Must be SGRAM
           	eRAM = CHIP_SG;
			else
//We must be DDR or SDR.
           	eRAM = CHIP_SD;
				
//Reset file pointer to the beginning of the file			
   		lseek(hInFile, 0, SEEK_SET);

		}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		if((bSuccess == TRUE) && (wDeviceID != 3))
      {
//Get the SSID of the board the user wants to program
            wSSDevID = sBoardsDesc[wBoard2Program].wSubDeviceID;

//Figure out if we're PCI or AGP
            if(sBoardsDesc[wBoard2Program].wBusID == 0)
            {
               eBusType = CHIP_PCI;
            }
            else
            {
               eBusType = CHIP_AGP;
            }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Scan through the list of supporte boards to determine if we support
// the board.
            for(i = 0; i < NumBuilds; i++)
            {
               if ((wSSDevID == sChipBuilds[i].wSSDevID) &&
					(eBusType == sChipBuilds[i].eBusType) &&
                   (eRAM == sChipBuilds[i].eRAMType) &&
                   (eSpeed == sChipBuilds[i].eSpeedType) )

               {
//We're found the SSID
                  bFoundBuild = TRUE;
                  break;
               }
            }
            
            if(bFoundBuild == TRUE)
            {
//No see if the SSID matchs the expected board configuration
               if( (eBusType == sChipBuilds[i].eBusType) &&
                   (eRAM == sChipBuilds[i].eRAMType) &&
                   (eSpeed == sChipBuilds[i].eSpeedType) )
               {
//We've found a perfect match
                  bSuccess = TRUE;
               }
               else
               {
#ifdef	USERFLASH
//The SSID doesn't match the expected board configuration.  Do we want to go on??
					if (bRunWindowed)
					{
						if (bOverrideNoSSID)
						{
							bSuccess = TRUE;
						}
						else
						{
							close (hInFile);
							exit (10);
						}
					}
					else
					{
						bSuccess = ContinueYN("The information in the BIOS file does not match the selected hardware.");
					}
#else
//The SSID doesn't match the expected board configuration.  Do we want to go on??
					bSuccess = ContinueYN("The information in the BIOS file does not match the selected hardware.");
#endif
               }
            }
            else
            {
#ifdef	USERFLASH
//We couldn't find the SSID in the builds list.  Do we want to still continue??
				if (bRunWindowed)
				{
					if (bOverrideNoInfo)
					{
						bSuccess = TRUE;
					}
					else
					{
						close (hInFile);
						exit (9);
					}
				}
				else
				{
					bSuccess = ContinueYN("There is no build information for the selected hardware.");
				}
#else
//We couldn't find the SSID in the builds list.  Do we want to still continue??
               bSuccess = ContinueYN("There is no build information for the selected hardware.");
#endif
            }
			}

   return(bSuccess);
//---------------------------------------------------------------------------
}
//
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		All rights reserved.
//



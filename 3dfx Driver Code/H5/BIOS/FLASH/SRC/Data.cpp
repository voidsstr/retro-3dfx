//
//		DATA.CPP - Data definitions for FLASH.EXE
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/4/98
//		Last modified:	4/1/99
//
#include	<stdio.h>
#define	_DATAFILE_
#include	"flash.h"

// Include the ROM image
#include	"image.h"

BOOL         	bFoundV3 = FALSE, bFoundNapalm = FALSE, bFoundRampage = FALSE;

BOARDDESC   sBoardsDesc[MAX_BOARDS];        // Slot and Bus number for board ID.
WORD        wNumBoards = 0;                 // Number of 3dfx Boards found.
WORD      	 wDeviceID;

ChipSpeedVer	sChipSpeedVer[10];
char         	sChipSRAMType[][4]={"-SD","-SG"};

ChipBuildInfo sChipBuilds[MAX_BUILDS];

//**********************************************************************
//V3 Data tables
char         	sVd3Velocity[]="Velocity";	// Alternate Market Name

char         	sVoodoo3[]="Voodoo3";

ChipSpeedVer	sV3SpeedVer[]={
		{"100 ", V3_V100}, 
		{"200 ", V3_V200}, 
		{"1000", V3_1000}, 
		{"2000", V3_2000}, 
		{"3000", V3_3000}, 
		{"3500", V3_3500}
		};

ChipBuildInfo sV3Builds[NUM_V3_BUILDS] = {  
     //SSID     BUS    RAM    Speed    TV     LCD  BIOS Size
     {0x0003, CHIP_AGP, CHIP_SD, V3_3500, FALSE, FALSE, CHIP_32K},
     {0x0004, CHIP_AGP, CHIP_SG, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x0005, CHIP_AGP, CHIP_SG, V3_3000, FALSE, FALSE, CHIP_32K},
     {0x0006, CHIP_AGP, CHIP_SG, V3_3500, FALSE, FALSE, CHIP_32K},
     {0x0008, CHIP_PCI, CHIP_SD, V3_3000, FALSE, FALSE, CHIP_32K},
     {0x0009, CHIP_PCI, CHIP_SD, V3_3500, FALSE, FALSE, CHIP_32K},
     {0x000A, CHIP_PCI, CHIP_SG, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x000B, CHIP_PCI, CHIP_SG, V3_3000, FALSE, FALSE, CHIP_32K},
     {0x000C, CHIP_PCI, CHIP_SG, V3_3500, FALSE, FALSE, CHIP_32K},
     {0x000D, CHIP_AGP, CHIP_SD, V3_2000, TRUE,  TRUE,  CHIP_64K},
     {0x000E, CHIP_AGP, CHIP_SD, V3_3000, TRUE,  TRUE,  CHIP_64K},
     {0x000F, CHIP_AGP, CHIP_SD, V3_3500, TRUE,  TRUE,  CHIP_64K},
     {0x0010, CHIP_AGP, CHIP_SG, V3_2000, TRUE,  TRUE,  CHIP_64K},
     {0x0011, CHIP_AGP, CHIP_SG, V3_3000, TRUE,  TRUE,  CHIP_64K},
     {0x0012, CHIP_AGP, CHIP_SG, V3_3500, TRUE,  TRUE,  CHIP_64K},
     {0x0013, CHIP_PCI, CHIP_SD, V3_2000, TRUE,  TRUE,  CHIP_64K},
     {0x0014, CHIP_PCI, CHIP_SD, V3_3000, TRUE,  TRUE,  CHIP_64K},
     {0x0015, CHIP_PCI, CHIP_SD, V3_3500, TRUE,  TRUE,  CHIP_64K},
     {0x0016, CHIP_PCI, CHIP_SG, V3_2000, TRUE,  TRUE,  CHIP_64K},
     {0x0017, CHIP_PCI, CHIP_SG, V3_3000, TRUE,  TRUE,  CHIP_64K},
     {0x0018, CHIP_PCI, CHIP_SG, V3_3500, TRUE,  TRUE,  CHIP_64K},
     {0x0030, CHIP_AGP, CHIP_SD, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x0031, CHIP_AGP, CHIP_SD, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x0032, CHIP_AGP, CHIP_SD, V3_2000, FALSE, TRUE,  CHIP_64K},
     {0x0033, CHIP_AGP, CHIP_SD, V3_2000, FALSE, TRUE,  CHIP_64K},
     {0x0034, CHIP_AGP, CHIP_SD, V3_2000, TRUE,  FALSE, CHIP_64K},
     {0x0035, CHIP_AGP, CHIP_SD, V3_2000, TRUE,  TRUE,  CHIP_64K},
     {0x0036, CHIP_PCI, CHIP_SD, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x0037, CHIP_AGP, CHIP_SD, V3_3000, FALSE, FALSE, CHIP_32K},
     {0x0038, CHIP_AGP, CHIP_SD, V3_3000, FALSE, FALSE, CHIP_32K},
     {0x0039, CHIP_AGP, CHIP_SD, V3_3000, FALSE, TRUE,  CHIP_64K},
     {0x003A, CHIP_AGP, CHIP_SD, V3_3000, TRUE,  FALSE, CHIP_64K},
     {0x003B, CHIP_AGP, CHIP_SD, V3_3000, TRUE,  TRUE,  CHIP_64K},
     {0x003C, CHIP_AGP, CHIP_SG, V3_3500, TRUE,  TRUE,  CHIP_64K},
     {0x003D, CHIP_PCI, CHIP_SD, V3_2000, TRUE,  TRUE,  CHIP_64K},
     {0x003E, CHIP_AGP, CHIP_SD, V3_2000, FALSE, TRUE,  CHIP_64K},
     {0x003F, CHIP_AGP, CHIP_SG, V3_3000, TRUE,  TRUE,  CHIP_64K},
     {0x0040, CHIP_AGP, CHIP_SD, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x0041, CHIP_AGP, CHIP_SD, V3_3000, FALSE, FALSE, CHIP_32K},
     {0x0042, CHIP_AGP, CHIP_SG, V3_3000, FALSE, TRUE,  CHIP_64K},
     {0x0043, CHIP_AGP, CHIP_SG, V3_3000, FALSE, FALSE, CHIP_32K},
     {0x0044, CHIP_PCI, CHIP_SD, V3_2000, TRUE,  FALSE, CHIP_64K},
     {0x0045, CHIP_PCI, CHIP_SD, V3_2000, FALSE, TRUE,  CHIP_64K},
     {0x0046, CHIP_AGP, CHIP_SG, V3_3500, FALSE, TRUE,  CHIP_64K},
     {0x0047, CHIP_AGP, CHIP_SG, V3_3500, TRUE,  FALSE, CHIP_64K},
     {0x0048, CHIP_AGP, CHIP_SG, V3_3500, FALSE, FALSE, CHIP_32K},
     {0x0049, CHIP_AGP, CHIP_SD, V3_3500, TRUE,  TRUE,  CHIP_64K},
     {0x004A, CHIP_AGP, CHIP_SD, V3_3500, FALSE, TRUE,  CHIP_64K},
     {0x004B, CHIP_AGP, CHIP_SG, V3_V100, FALSE, FALSE, CHIP_32K},
     {0x004C, CHIP_AGP, CHIP_SG, V3_V200, FALSE, FALSE, CHIP_32K},
     {0x004D, CHIP_AGP, CHIP_SG, V3_1000, FALSE, FALSE, CHIP_32K},
     {0x004E, CHIP_AGP, CHIP_SG, V3_1000, FALSE, FALSE, CHIP_32K},
     {0x004F, CHIP_AGP, CHIP_SD, V3_3500, TRUE,  FALSE, CHIP_64K},
     {0x0050, CHIP_AGP, CHIP_SG, V3_V200, FALSE, FALSE, CHIP_32K},
     {0x0051, CHIP_AGP, CHIP_SG, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x0052, CHIP_AGP, CHIP_SG, V3_1000, FALSE, FALSE, CHIP_32K},
     {0x0053, CHIP_PCI, CHIP_SG, V3_V100, FALSE, FALSE, CHIP_32K},
     {0x0054, CHIP_PCI, CHIP_SG, V3_V200, FALSE, FALSE, CHIP_32K},
     {0x0055, CHIP_PCI, CHIP_SG, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x0056, CHIP_PCI, CHIP_SD, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x0057, CHIP_PCI, CHIP_SD, V3_3000, FALSE, FALSE, CHIP_32K},
//
     {0x0060, CHIP_AGP, CHIP_SD, V3_3500, TRUE,  FALSE, CHIP_64K},
     {0x0061, CHIP_AGP, CHIP_SD, V3_3500, TRUE,  FALSE, CHIP_64K},
     {0x0062, CHIP_AGP, CHIP_SD, V3_3500, TRUE,  FALSE, CHIP_64K},
//
     {0x124F, CHIP_AGP, CHIP_SD, V3_3500, TRUE,  FALSE, CHIP_64K},
     {0x124A, CHIP_AGP, CHIP_SD, V3_3500, FALSE, TRUE,  CHIP_64K},
     {0x1251, CHIP_AGP, CHIP_SG, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x1252, CHIP_AGP, CHIP_SG, V3_1000, FALSE, FALSE, CHIP_32K},
//
     {0x1037, CHIP_AGP, CHIP_SD, V3_3000, FALSE, FALSE, CHIP_32K},
     {0x103A, CHIP_AGP, CHIP_SD, V3_3000, TRUE,  FALSE, CHIP_64K},
     {0x104B, CHIP_AGP, CHIP_SG, V3_V100, FALSE, FALSE, CHIP_32K},
     {0x104D, CHIP_AGP, CHIP_SG, V3_1000, FALSE, FALSE, CHIP_32K},
//
     {0x1130, CHIP_AGP, CHIP_SD, V3_2000, FALSE, FALSE, CHIP_32K},
     {0x1037, CHIP_AGP, CHIP_SD, V3_3000, FALSE, FALSE, CHIP_32K}
          };

//**********************************************************************
//Napalm and Napalm2 Data tables
char         	sVoodoo4[]="Voodoo4";
char           sVoodoo5[]="Voodoo5";

ChipSpeedVer	sNapalmSpeedVer[]={
		{"6000", NAPALM_6000},
		{"5800", NAPALM_5800},
		{"5500", NAPALM_5500},
		{"5000", NAPALM_5000},
		{"4800", NAPALM_4800},
		{"4500", NAPALM_4500},
		{"4400", NAPALM_4400},
		{"4200", NAPALM_4200},
		{"4000", NAPALM_4000},
		};

ChipBuildInfo sNapalmBuilds[NUM_NAPALM_BUILDS] = {  
     //SSID     BUS    	RAM    	  Speed    	 TV     LCD  BIOS Size
     {0x0001, CHIP_AGP, CHIP_SD, NAPALM_6000, FALSE, FALSE, CHIP_32K},		
     {0x0002, CHIP_AGP, CHIP_SD, NAPALM_5500, FALSE, FALSE, CHIP_32K},
     {0x0002, CHIP_AGP, CHIP_SG, NAPALM_5000, FALSE, FALSE, CHIP_32K},
     {0x0003, CHIP_PCI, CHIP_SG, NAPALM_5000, FALSE, FALSE, CHIP_32K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4800, FALSE, FALSE, CHIP_32K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4800, FALSE, TRUE , CHIP_64K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4800, TRUE,  FALSE, CHIP_64K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4800, TRUE,  TRUE , CHIP_64K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4500, FALSE, FALSE, CHIP_32K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4500, FALSE, TRUE , CHIP_64K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4500, TRUE,  FALSE, CHIP_64K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4500, TRUE,  TRUE , CHIP_64K},
     {0x0005, CHIP_PCI, CHIP_SG, NAPALM_4500, FALSE, FALSE, CHIP_32K},
     {0x0005, CHIP_AGP, CHIP_SD, NAPALM_4500, FALSE, TRUE , CHIP_64K},
     {0x0005, CHIP_AGP, CHIP_SD, NAPALM_4500, TRUE,  FALSE, CHIP_64K},
     {0x0005, CHIP_PCI, CHIP_SD, NAPALM_4500, FALSE, TRUE , CHIP_64K},
     {0x0006, CHIP_AGP, CHIP_SD, NAPALM_4400, FALSE, FALSE, CHIP_32K},

     {0x0005, CHIP_PCI, CHIP_SD, NAPALM_4200, FALSE, TRUE , CHIP_64K},
     {0x0004, CHIP_AGP, CHIP_SD, NAPALM_4000, FALSE, FALSE, CHIP_32K},
          };

//**********************************************************************
//Rampage Data tables
char         	sRampage[]="Rampage";

ChipSpeedVer	sRampageSpeedVer[]={
		{"1000", RAMPAGE_1000},
		{"2000", RAMPAGE_2000}
		};

ChipBuildInfo sRampageBuilds[NUM_RAMPAGE_BUILDS] = {  
     //SSID     BUS    	RAM    	  Speed    	 TV     LCD  BIOS Size
     {0x0032, CHIP_AGP, CHIP_SG, RAMPAGE_1000, FALSE, FALSE, CHIP_32K},
     {0x0040, CHIP_AGP, CHIP_SG, RAMPAGE_2000, TRUE,  FALSE, CHIP_64K}
          };
//**********************************************************************

int			hInFile = NULL;				// Assume reset to older image
FILE			*hOutDevice = stdout;		// Assume standard output device
WORD			selFlat = 0;					// Flat model selector (alloc'd at runtime)
WORD			selData = 0;					// Valid data selector (filled in runtime)
WORD			wBusDevID = 0;					// Bus & Device ID from PCI BIOS
WORD			wIOBase = 0;					// I/O Base address
WORD			wPCICommand = 0;				// PCI command register (config space)
BYTE			byMan = 0;						// Manufacturer ID from ROM
BYTE			byDevice = 0;					// Device ID from ROM
WORD			wSectorSize = 1;				// Sector size in BYTES of EEPROM
DWORD			dwOrgMiscInit0 = 0;			// Original MiscInit0 register
DWORD			dwOrgMiscInit1 = 0;			// Original MiscInit1 register

#ifdef USERFLASH
BYTE			byVPDBuffer[VPD_INFO_SIZE];
BYTE			byRegisterOffset[TABLE_PTRS];
#endif			// USERFLASH

// Useful addresses
DWORD			dwPhysMemBase0 = NULL;		// Pointer to memory mapped registers (physical addr)
BYTE	__far *lpLinMemBase0 = NULL;		// Pointer to memory mapped registers (linear addr)
DWORD			dwPhysOrgROMBase = NULL;	// Original ROM Base value (physical)
DWORD			dwLinOrgROMBase = NULL;		// Original ROM Base value (linear)
DWORD			dwPhysROMBase = NULL;		// Pointer to "new" ROM location (physical)
BYTE	__far *lpLinROMBase = NULL;		// Pointer to "new" ROM location (linear)
DWORD	__far *lpLinMiscInit0= NULL;		// Pointer to miscInit0 register (linear).
DWORD	__far *lpLinMiscInit1= NULL;		// Pointer to miscInit1 register (linear).

char	szROMSaveFile[MAX_FILENAME] = "SAVE.ROM";	// Old ROM image is saved here
BYTE	byBuffer[FILEIO_BUFFER];						// File I/O goes through here

char    szBusType[][4]={"PCI", "AGP"};

DeviceName	TdfxDeviceName[NUM_TDFX_CHIPS]={
			{0x03,"Banshee"},
			{0x04,"Avenger"},
			{0x05,"Avenger"},
			{0x09,"VSA 100"},
           {0x0B,"Napalm2"},
			{0x10,"Rampage"}
			};

#ifdef USERFLASH
char	szCopyright[] = "\n3dfx EEPROM USERFLASH Utility   Version 2.17"
			"\nCopyright (c) 1998 by 3dfx Interactive, Inc."
			"\nAll rights reserved.\n";
char	szInvalidArgs[] = "\nInvalid number of command line arguments. Use USERFLSH /? to list commands.\n";
char	szUsage[] = "\nRuns 3dfx EEPROM USERFLASH Utility"
						"\n\nUSERFLSH [filename] | [/RESET] | [[/?] | [/HELP]]"
						"\n\n  filename\tName of file containing VGA BIOS ROM image"
						"\n  RESET\t\tReset to older ROM image"
						"\n  U\t\tUpdate registers to new settings"
//						"\n  W\t\tRun Hidden in Windows mode"
//						"\n  V\t\tOverride Version# Check (Windowed)"
//						"\n  S\t\tOverride SSID Match Check (Windowed)"
//						"\n  I\t\tOverride Hardware Info Check (Windowed)"
						"\n  ? or HELP\tDisplay this message"
						"\n\nUSERFLASH will copy a ROM image into the EEPROM on the 3dfx adapter."
						"\nThe previous ROM image will be saved to disk as \"SAVE.ROM\"."
						"\n";
#else
char	szCopyright[] = "\n3dfx EEPROM Flash Utility   Version 2.17"
			"\nCopyright (c) 1998 by 3dfx Interactive, Inc."
			"\nAll rights reserved.\n";
char	szInvalidArgs[] = "\nInvalid number of command line arguments. Use FLASH /? to list commands.\n";
char	szUsage[] = "\nRuns 3dfx EEPROM Flash Utility"
						"\n\nFLASH [filename] | [/RESET] | [[/?] | [/HELP]]"
						"\n\n  filename\tName of file containing VGA BIOS ROM image"
						"\n  RESET\t\tReset to older ROM image"
						"\n  ? or HELP\tDisplay this message"
						"\n\nFLASH will copy a ROM image into the EEPROM on the 3dfx adapter."
						"\nThe previous ROM image will be saved to disk as \"SAVE.ROM\"."
						"\n";
#endif

char	szNoFile[] = "\nFile not found\n";
char	szReadError[] = "\nError while reading file\n";
char	szSyntaxError[] = "Syntax error\n";
char	szResetMsg[] = "\nResetting EEPROM to older VGA BIOS image...\n";
char	szNoBoard[] = "\nERROR: A 3dfx Banshee or Voodoo3 is not in this system.\n";
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
char    szSetupFailed[] = "\nFailed to properly configure card.";

char	szHelp0[] = "/?";
char	szHelp1[] = "/HELP";
char	szReset[] = "/RESET";

#ifdef USERFLASH
char	szUpdateRegs[] = "/U";
char	szRunWindowed[] = "/W";
char	szOverrideChVers[] = "/V";
char	szOverrideNoSSID[] = "/S";
char	szOverrideNoInfo[] = "/I";

//**********************************************************************
BYTE	NUM_REGS;

//OEMConfigTable Register Data Tables
REGDESC	sOEMCfgRegInfo[15] = {
			{OFFSET_V3_PCIINIT0,	0,0,0,0},
			{OFFSET_V3_MISCINIT0,	0,0,0,0},
			{OFFSET_V3_MISCINIT1,	0,0,0,0},
			{OFFSET_V3_DRAMINIT0,	0,0,0,0},
			{OFFSET_V3_DRAMINIT1,	0,0,0,0},
			{OFFSET_V3_AGPINIT0,	0,0,0,0},
			{OFFSET_V3_PLLCTRL1,	0,0,0,0},
			{OFFSET_V3_PLLCTRL2,	0,0,0,0},
			{OFFSET_V3_SGRAMMODE,	0,0,0,0},
			{OFFSET_V3_TMUGBEINIT,	0,0,0,0}
			};

BOOL	bLaterVersion;
BOOL	bSaveInUse;
BOOL	bKeepOldRegs;
BOOL	bRunWindowed;
BOOL	bOverrideChVers;
BOOL	bOverrideNoSSID;
BOOL	bOverrideNoInfo;
WORD	wNewRegPtr;
#endif			// USERFLASH

//
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		All rights reserved.
//


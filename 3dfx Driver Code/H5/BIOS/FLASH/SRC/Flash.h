//
//		FLASH.H - Defines, structures, and data types for FLASH.EXE
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/4/98
//		Last modified:	2/8/99
//
#ifndef _FLASH_H_
#define _FLASH_H_

// Defines
#define	FALSE					0
#define	TRUE 					1
#define	MAX_LINE				256			// Max characters on prompt line
#define	PCI_ID					0x121A
#define	PCI_ID_DEVICE_2000	0x0004
#define	PCI_ID_DEVICE_3000	0x0005
#define	MAX_FILENAME			256
#define	FILEIO_BUFFER			(4*1024)		// MUST be divisible by 32K

#define	VPD_INFO_SIZE			(0x100)
#define	FIRST_POINTER_ADDR	 0x0050
#define	TABLE_PTRS				 0x0002


// OEM Configuration Register Table Offsets
//RAMPAGE OEMConfig Structure
#define	RAMPAGE_NUM_REGS					10
#define	OFFSET_RAMPAGE_PCIINIT0 		0x0000
#define	OFFSET_RAMPAGE_MISCINIT0		0x0004
#define	OFFSET_RAMPAGE_MISCINIT1		0x0008
#define	OFFSET_RAMPAGE_AGPINIT0			0x000C
#define	OFFSET_RAMPAGE_PLLCTRL0			0x0010
#define	OFFSET_RAMPAGE_DRAMINIT0 		0x0014
#define	OFFSET_RAMPAGE_DRAMINIT1 		0x0018
#define	OFFSET_RAMPAGE_DRAMINIT2 		0x001C
#define	OFFSET_RAMPAGE_DRAMINIT3		0x0020
#define	OFFSET_RAMPAGE_2DCOMMAND 		0x0024


//Napalm and Napalm2 OEMConfig Structure
#define	NAPALM_NUM_REGS					10
#define	NAPALM2_NUM_REGS					12
#define	OFFSET_NAPALM_PCIINIT0 		0x00000
#define	OFFSET_NAPALM_MISCINIT0		0x00004
#define	OFFSET_NAPALM_MISCINIT1		0x00008
#define	OFFSET_NAPALM_DRAMINIT0		0x0000C
#define	OFFSET_NAPALM_DRAMINIT1		0x00010
#define	OFFSET_NAPALM_AGPINIT0 		0x00014
#define	OFFSET_NAPALM_PLLCTRL1 		0x00018
#define	OFFSET_NAPALM_PLLCTRL2 		0x0001C
#define	OFFSET_NAPALM_SGRAMMODE		0x00020
#define	OFFSET_NAPALM_TMUGBEINIT 	0x00032
#define	OFFSET_NAPALM2_STRAPPING1 	0x0FFF0
#define	OFFSET_NAPALM2_STRAPPING2 	0x0FFF4


//V3 or older OEMConfig structure.
#define	V3_NUM_REGS					10
#define	OFFSET_V3_PCIINIT0 		0x0000
#define	OFFSET_V3_MISCINIT0		0x0004
#define	OFFSET_V3_MISCINIT1		0x0008
#define	OFFSET_V3_DRAMINIT0		0x000C
#define	OFFSET_V3_DRAMINIT1		0x0010
#define	OFFSET_V3_AGPINIT0 		0x0014
#define	OFFSET_V3_PLLCTRL1 		0x0018
#define	OFFSET_V3_PLLCTRL2 		0x001C
#define	OFFSET_V3_SGRAMMODE		0x0020
#define	OFFSET_V3_TMUGBEINIT		0x002D

//Chip ID Defines
//Bring up chip had a PCI device ID of 0x09
#define	ChipIdNapalm		0x09
//#define	ChipIdNapalm		0x07
#define	ChipIdRampage		0x10

#define	ChipIdNapalm2		0x0B

// EEPROM Manufacturers and EEPROM device types
#define	MANID_AMD_TI			0x01			// AMD & TI 29010 have same ID's
#define	AMD_TI_29010			0x20
#define	MANID_ATMEL				0x1F
#define	ATMEL_29LV512			0x3D
#define	ATMEL_29010				0xD5
#define	ATMEL_49BV512			0x03
#define	ATMEL_49F010			0x17
#define	MANID_SST				0xBF
#define SST_29LE512				0x3D
#define SST_29EE010				0x07
#define SST_39SF010				0xB5
#define SST_39VF512				0xD4


// Useful I/O values
#define	IODELAYPORT				0xED			// Unused I/O port
#define	PORTB						0x61
#define	FLAG_REFRESHSTATUS	0x10			// Port B refresh signal status

// Industry standard data types and others
#define	FAR
#ifndef BYTE
typedef	unsigned char			BYTE;
#endif
#ifndef WORD
typedef	unsigned short			WORD;
#endif
#ifndef DWORD
typedef	unsigned long			DWORD;
#endif
#ifndef BOOL
typedef	WORD						BOOL;
#endif
#ifndef LPBYTE
typedef	BYTE						FAR	*LPBYTE;
#endif
#ifndef LPWORD
typedef	WORD						FAR	*LPWORD;
#endif
#ifndef LPBOOL
typedef	BOOL						FAR	*LPBOOL;
#endif
#ifndef LPSTR
typedef	char						FAR	*LPSTR;
#endif
#ifndef SEGOFF
typedef	DWORD						SEGOFF;
#endif

// Industry standard macros
#ifndef LOBYTE
#define LOBYTE(w)					((BYTE)(w))
#endif
#ifndef HIBYTE
#define HIBYTE(w)					((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif
#ifndef LOWORD
#define LOWORD(l)					((WORD)((DWORD)(l) & 0xFFFF))
#endif
#ifndef HIWORD
#define HIWORD(l)					((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#endif

#define MAX_BOARDS    	5
#define MAX_BUILDS		100

#define NUM_TDFX_CHIPS	6

//V3 Defines
#define NUM_V3_BUILDS 	74
#define NUM_V3_SPEED 	6

//Napalm and Napalm2 Defines
#define NUM_NAPALM_BUILDS 	19
#define NUM_NAPALM_SPEED 	9

//Rampage Defines
#define NUM_RAMPAGE_BUILDS 	2
#define NUM_RAMPAGE_SPEED 		2

typedef enum 
{
	V3_V100,
	V3_V200,
	V3_1000,
	V3_2000,
	V3_3000,
	V3_3500,
   NAPALM_6000,
   NAPALM_5800,
   NAPALM_5500,
   NAPALM_5000,
   NAPALM_4800,
   NAPALM_4500,
   NAPALM_4400,
   NAPALM_4200,
   NAPALM_4000,
	RAMPAGE_1000,
	RAMPAGE_2000
} ChipSpeedType;
 
typedef enum 
{
	CHIP_SD,
	CHIP_SG,
} ChipRAMType;

typedef enum 
{
  CHIP_PCI,
  CHIP_AGP,
} ChipBusType;

typedef enum 
{
  CHIP_32K,
  CHIP_64K,
} ChipBiosSize;

typedef	struct 
{
	char				String[5];
	ChipSpeedType	SpeedType;
} ChipSpeedVer;

typedef	struct 
{
	WORD			ChipID;
	char			ChipName[20];
} DeviceName;
 
typedef struct {
   WORD wBusID;
   WORD wSlotNum;
   WORD wVendorID;
   WORD wDeviceID;
   WORD wSubDeviceID;
   WORD wSubVendorID;
}BOARDDESC;

typedef struct {
   WORD        	wSSDevID;
   ChipBusType   	eBusType;
   ChipRAMType   	eRAMType;
   ChipSpeedType 	eSpeedType;
   BOOL        	bTV;
   BOOL        	bLCD;
   ChipBiosSize  	eBiosSize;
} ChipBuildInfo;

#ifdef USERFLASH
typedef struct {
   WORD			wRegOffset;
   BYTE			byLLByte;
	BYTE			byLHByte;
	BYTE			byHLByte;
	BYTE			byHHByte;
} REGDESC;
#endif 			// USERFLASH

// Data structures and data types
typedef struct tagREALMODECALL {
	DWORD	edi;
	DWORD	esi;
	DWORD	ebp;
	DWORD	reserved;
	DWORD	ebx;
	DWORD	edx;
	DWORD	ecx;
	DWORD	eax;
	WORD	flags;
	WORD	es;
	WORD	ds;
	WORD	fs;
	WORD	gs;
	WORD	ip;
	WORD	cs;
	WORD	sp;
	WORD	ss;
} REALMODECALL;
typedef	REALMODECALL					*LPREALMODECALL;


// MAIN.CPP
BOOL InitBoard (WORD);
WORD SelectBoard (void);
BOOL ValidateNewROM (WORD);

// PCIINFO.CPP
BOOL PCIEnumerate(void);

// USER.CPP
BOOL ContinueYN(LPSTR);
void DisplayChar (char);
void DisplayString (LPSTR);
void DisplayCopyright (void);
void DisplayUsage (void);
void DisplayFormattedString (LPSTR, ...);

// ROM.CPP
void BeginROM (void);
void EndROM (void);
void ROMByteWrite (WORD, BYTE);
BYTE ROMByteRead (WORD);
void FlushBus (void);
void GetROMID (LPBYTE, LPBYTE, LPWORD);
void SendROMCommand (BYTE);
BOOL SaveROM (LPSTR, LPWORD);
BOOL WriteROMRestore (LPWORD, LPBOOL);
BOOL WriteROMFromFile (int, LPWORD, LPBOOL);
void ROMErase (void);
BOOL ROMVerifyWrite (WORD, BYTE);

#ifdef USERFLASH
BOOL IsVPDAvailable (void);
void SaveOffOldRegs (void);
WORD GetNewRegsSettingPtr (void);
void CopyInOldRegs (void);
BOOL NewFileVPDAvailable (int);
BOOL CheckLaterVersion (int);
#endif // USERFLASH

// UTIL.CPP
void RealModeINT (REALMODECALL *, BYTE);
WORD AllocateFlatDescriptor (void);
void FreeDescriptor (WORD);
void _outp (WORD, BYTE);
void _outpw (WORD, WORD);
void _outpd (WORD, DWORD);
BYTE _inp (WORD);
WORD _inpw (WORD);
DWORD _inpd (WORD);
DWORD Phys2Linear (DWORD, DWORD);
void Delay15us (long int);

// Data definitions
  #ifndef _DATAFILE_
extern	BYTE			NUM_REGS;

extern	BYTE			tblROMImage[0x8000];

extern	BYTE			tblHexDigits[];
extern	int			hInFile;
extern	FILE			*hOutDevice;
extern	WORD			selFlat;
extern	WORD			selData;
extern	WORD			wBusDevID;
extern	WORD			wIOBase;
extern	WORD			wPCICommand;
extern	BYTE			byMan;
extern	BYTE			byDevice;
extern	WORD			wSectorSize;
extern	DWORD			dwOrgMiscInit0;
extern	DWORD			dwOrgMiscInit1;
extern  	BOARDDESC   sBoardsDesc[MAX_BOARDS];    
extern  	WORD        wNumBoards;
extern	WORD    	   wDeviceID;

extern  char         	sVd3Velocity[];
extern  char         	sVoodoo3[];
extern  BOOL         	bFoundV3;
extern  ChipSpeedVer		sV3SpeedVer[5];
extern  ChipBuildInfo   sV3Builds[NUM_V3_BUILDS];

extern   char           sVoodoo4[];
extern   char           sVoodoo5[];
extern  BOOL				bFoundNapalm;
extern  ChipSpeedVer		sNapalmSpeedVer[5];
extern  ChipBuildInfo   sNapalmBuilds[NUM_NAPALM_BUILDS];

extern  char         	sRampage[];
extern  BOOL				bFoundRampage;
extern  ChipSpeedVer		sRampageSpeedVer[5];
extern  ChipBuildInfo   sRampageBuilds[NUM_RAMPAGE_BUILDS];

extern  ChipSpeedVer		sChipSpeedVer[10];
extern  char            sChipSRAMType[][4];
extern  ChipBuildInfo	sChipBuilds[MAX_BUILDS];
extern  DeviceName		TdfxDeviceName[];
		  	
extern	DWORD			dwPhysMemBase0;
extern	BYTE	__far *lpLinMemBase0;
extern	DWORD			dwPhysOrgROMBase;
extern	DWORD			dwLinOrgROMBase;
extern	DWORD			dwPhysROMBase;
extern	BYTE	__far *lpLinROMBase;
extern	DWORD	__far *lpLinMiscInit0;
extern	DWORD	__far *lpLinMiscInit1;

extern	char			szROMSaveFile[MAX_FILENAME];
extern	BYTE			byBuffer[FILEIO_BUFFER];

extern  char            szBusType[][4];
extern  char            szDevice[][20];

extern	char			szCopyright[];
extern	char			szInvalidArgs[];
extern	char			szUsage[];
extern	char			szNoFile[];
extern	char			szReadError[];
extern	char			szSyntaxError[];
extern	char			szResetMsg[];
extern	char			szNoBoard[];
extern	char			szUnknownDevice[];
extern	char			szSuccess[];
extern	char			szBurnError[];
extern	char			szInvalidROM[];
extern	char			szNoSuccess[];
extern	char			szSaveSuccess[];
extern	char			szNoSave[];
extern	char			szBeginROM[];
extern	char			szCompleted[];
extern	char			szROMErase[];
extern  char            szSetupFailed[];

extern	char			szHelp0[];
extern	char			szHelp1[];
extern	char			szReset[];

extern 	DWORD             dwOldROMSaveSize;
extern  DWORD             dwNewBinFileSize;

#ifdef USERFLASH
extern	char			szUpdateRegs[];
extern	char			szRunWindowed[];
extern	char			szOverrideChVers[];
extern	char			szOverrideNoSSID[];
extern	char			szOverrideNoInfo[];

extern	BYTE			byVPDBuffer[VPD_INFO_SIZE];
extern	BYTE			byRegisterOffset[TABLE_PTRS];

extern	BOOL			bLaterVersion;
extern	BOOL			bSaveInUse;
extern	BOOL			bKeepOldRegs;
extern	BOOL			bRunWindowed;
extern	BOOL			bOverrideChVers;
extern	BOOL			bOverrideNoSSID;
extern	BOOL			bOverrideNoInfo;
extern	WORD			wNewRegPtr;
extern	REGDESC			sOEMCfgRegInfo[];
#endif			// USERFLASH

  #endif			// _DATAFILE_
#endif			// _FLASH_H_

//
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//



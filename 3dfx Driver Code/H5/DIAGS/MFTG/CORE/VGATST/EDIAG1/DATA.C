//
//		DATA.CPP - Data definitions for EDIAG.EXE
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/9/98
//		Last modified:	5/12/98
//
#include	<stdio.h>
#define	_DATAFILE_
#include	"ediag.h"

// Global variables
FILE		*hInDevice = stdin;					// Assume standard input device
FILE		*hOutDevice = stdout;				// Assume standard output device
FILE		*hLogDevice = NULL;					// Assume no error log
FILE		*hCRCDevice = NULL;					// Assume no CRC file
WORD		selFlat = 0;							// Flat model selector (alloc'd at runtime)
WORD		selData = 0;							// Valid data selector (filled in runtime)
LPBYTE	lpVideoA000 = (LPBYTE) 0xA0000;	// Offset from "selFlat" to A000:0000
LPBYTE	lpVideoB000 = (LPBYTE) 0xB0000;	// Offset from "selFlat" to B000:0000
LPBYTE	lpVideoB800 = (LPBYTE) 0xB8000;	// Offset from "selFlat" to B800:0000
BOOL		bLinear = FALSE;						// Linear framebuffer available flag
BYTE		__far *lpLinFrameBuffer = NULL;	// Ptr to linear framebuffer
CARDINFO	CardInfo;								// Board specific data (filled at runtime)
int		nCOMPort = 0;							// Serial port to use for reporting

#ifdef DEBUG
BOOL			bUseLogFile = FALSE;			// Re-open log file for writes
char			szLogFile[MAX_FILENAME];	// Log filename
#endif
char			szCRCFile[MAX_FILENAME];	// CRC filename

BOOL			bRepMono = FALSE;				// Monochrome reporting device available
BOOL			bRepSerial = FALSE;			// Serial reporting device available
BOOL			bRepOEM = FALSE;				// OEM reporting device available

// Adapter & module controls
BOOL		_bTestPending = FALSE;
long int	_nTestIndex = 0;
BOOL		_bGenCRC = FALSE;					// To create a file of CRCs this is set to "TRUE"
BOOL		_bUseCRC = FALSE;					// To use a file of CRCs this is set to "TRUE"
long int	_nCardID = 0;
BOOL		_bExtendedTest = TRUE;			// For QUICK tests, this is "FALSE"
long int	_nMemUnitSize = 4;
DWORD		_dwMemPattern = 0x55AA6699;
DWORD		_dwMemStart = 0;
DWORD		_dwMemEnd = 0;
DWORD		_dwMemMask = 0xFFFFFFFF;		// Memory mask (for dealing with those quirky alpha channels)
BOOL		_bMemFullTest = TRUE;			// If user overrides a parameter, only do what is requested
BOOL		_bClearScreen = TRUE;			// If user requests "help", don't clear screen at program exit
BOOL		_bAllVGAModes = FALSE;			// Do selective VGA mode visual tests
BOOL		_bSelectVGAModes = TRUE;		// Do selective VGA mode visual tests
long int	_nVGAMode = 0x12;			// VGA mode to test
BOOL		_bAllNatModes = FALSE;			// Do selective Native mode visual tests
BOOL		_bSelNatModes = TRUE;			// Do selective Native mode visual tests
long int	_nXRes = 640;				// Native mode X Resolution from cmd line
long int	_nYRes = 480;				// Native mode Y Resolution from cmd line
long int	_nBPP = 8;				// Native mode BPP from cmd line
long int	_nRefresh = 60;				// Native mode Refresh rate from cmd line
BOOL		_bRAMDACVGA = FALSE;			// Test only the VGA RAMDAC
BOOL		_bRAMDACNative = FALSE;			// Test only the Native RAMDAC
BOOL		_bRAMDACPLL = FALSE;			// Test only the PLL registers
BOOL		_bRAMDACCursor = FALSE;			// Test only the hardware cursor
BOOL		_bRAMDACFull = TRUE;			// Test the RAMDAC as both VGA & Native

// Error reporting
DWORD		_errLinAddr = 0;			// Linear address of error
DWORD		_errActual = 0;				// Actual data read
DWORD		_errExpected = 0;			// Expected data read
int		_errSize = 4;				// Unit size of data (1=BYTE, 2=WORD, 4=DWORD)
int		_errCount = 0;				// Number of errors that have occurred

VBEMODEINFOBLOCK		vbeModeInfo;
WORD				wFFFF = 0xFFFF;		// Mode list terminator (for errors)
LPBYTE				lpFont = NULL;		// Pointer to the current font
LPCOLORREF			lpclrTextFore = NULL;	// Pointer to the current foreground color
LPCOLORREF			lpclrTextBack = NULL;	// Pointer to the current background color
BOOL				bOpaque = TRUE;		// Opaque vs. Transparent flag

WORD	yTextRow = 0;					// Cursor row position
WORD	xTextCol = 0;					// Cursor column position
BYTE	byBankShifter = 0;				// Shifts to get to 64K per bank

DWORD dwTestPatterns[] = {
	0xAAAAAAAA, 0x55555555, 0x66666666, 0x99999999,
	0xCCCCCCCC, 0x33333333, 0x00000000, 0xFFFFFFFF,
	0x01234567, 0x89ABCDEF, 0xC3F0A569, 0x817E7E81
};
int	nTestPatterns = sizeof (dwTestPatterns) / sizeof (DWORD);

// Parsing needs
char			chLeader1 = '-';
char			chLeader2 = '/';
char			chSpecialLeader = '@';
char			cCmdDelimiter = '=';
char			cParmDelimiter = ';';
BYTE			tblHexDigits[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};
char			szHelp0[] = "?";
char			szHelp1[] = "HELP";
char			szHelpAll[] = "HELPALL";
char			szInfo[] = "INFO";
char			szCard[] = "CARD";
char			szSpeed[] = "SPEED";
char			szLog[] = "LOG";
char			szTest[] = "T";
char			szTestParm[] = "P";
char			szQuick[] = "QUICK";
char			szExtended[] = "EXTENDED";
char			szWaitKey[] = "WAITKEY";
char			szWait[] = "WAIT";
char			szSerial[] = "SERIAL";
char			szGenCRC[] = "GENCRC";
char			szUseCRC[] = "USECRC";

CMDLINELIST	tblCmdLineList[] = {
	{CMD_TEXTPARM,		szTest,		sizeof (szTest) - 1,		fnSetTest},
	{CMD_NOPARM,		szHelp0,	sizeof (szHelp0) - 1,		fnShowHelp},
	{CMD_NOPARM,		szHelp1,	sizeof (szHelp1) - 1,		fnShowHelp},
	{CMD_NOPARM,		szHelpAll,	sizeof (szHelpAll) - 1,		fnShowAll},
	{CMD_NOPARM,		szInfo,		sizeof (szInfo) - 1,		fnShowInfo},
	{CMD_NUMBERPARM,	szCard,		sizeof (szCard) - 1,		fnSetCard},
	{CMD_TEXTPARM,		szSpeed,	sizeof (szSpeed) - 1,		fnSetSpeed},
	{CMD_FILEPARM,		szLog,		sizeof (szLog) - 1,		fnOpenLog},
	{CMD_TEXTPARM,		szTestParm,	sizeof (szTestParm) - 1,	fnSetParm},
	{CMD_NOPARM,		szWaitKey,	sizeof (szWaitKey) - 1,		fnWaitKey},
	{CMD_NUMBERPARM,	szWait,		sizeof (szWait) - 1,		fnWait},
	{CMD_NUMBERPARM,	szSerial,	sizeof (szSerial) - 1,		fnSetSerial},
	{CMD_FILEPARM,		szGenCRC,	sizeof (szGenCRC) - 1,		fnGenCRC},
	{CMD_FILEPARM,		szUseCRC,	sizeof (szUseCRC) - 1,		fnUseCRC}
};
int	nCmdLineList = sizeof (tblCmdLineList) / sizeof (CMDLINELIST);

// Module names and descriptions
char	szVGAMEM[] = "VGAMEM";
char	szDescVGAMEM[] = "Test VGA framebuffer memory";
char	szNATMEMA[] = "NATMEMA";
char	szDescNATMEMA[] = "Test native mode framebuffer A memory";
char	szNATMEMB[] = "NATMEMB";
char	szDescNATMEMB[] = "Test native mode framebuffer B memory";
char	szZBUFFER[] = "ZBUFFER";
char	szDescZBUFFER[] = "Test Z-buffer memory";
char	szTEXTURE[] = "TEXTURE";
char	szDescTEXTURE[] = "Test texture memory";
char	szVGAIO[] = "VGAIO";
char	szDescVGAIO[] = "Test VGA I/O registers";
char	szNATREG[] = "NATREG";
char	szDescNATREG[] = "Test native mode memory mapped registers";
char	szVGAMODE[] = "VGAMODE";
char	szDescVGAMODE[] = "Test specific VGA mode(s)";
char	szNATMODE[] = "NATMODE";
char	szDescNATMODE[] = "Test a specific native mode resolution";
char	szRAMDAC[] = "RAMDAC";
char	szDescRAMDAC[] = "Test the RAMDAC";
char	szVGAROM[] = "VGAROM";
char	szDescVGAROM[] = "Test the VGA ROM";
char	szACCEL2D[] = "ACCEL2D";
char	szDescACCEL2D[] = "Test 2D acceleration functionality";
char	szBITBLT[] = "BITBLT";
char	szDescBITBLT[] = "Test the BITBLT functionality";
char	szACCEL3D[] = "ACCEL3D";
char	szDescACCEL3D[] = "Test 3D acceleration functionality";
char	szDMA[] = "DMA";
char	szDescDMA[] = "Test DMA channel";

MODULELIST	tblModuleList[] = {
	{PARM_MEMORY,	szNATMEMA,	szDescNATMEMA,	sizeof (szNATMEMA) - 1,	NULL,	fnTestNatMemA},
	{PARM_MEMORY,	szVGAMEM,	szDescVGAMEM,	sizeof (szVGAMEM) - 1,	NULL,	fnTestVGAMem},
	{PARM_MEMORY,	szNATMEMB,	szDescNATMEMB,	sizeof (szNATMEMB) - 1,	NULL,	fnTestNatMemB},
	{PARM_MEMORY,	szZBUFFER,	szDescZBUFFER,	sizeof (szZBUFFER) - 1,	NULL,	fnTestZBuffer},
	{PARM_MEMORY,	szTEXTURE,	szDescTEXTURE,	sizeof (szTEXTURE) - 1,	NULL,	fnTestTexture},
	{PARM_NONE,	szVGAIO,	szDescVGAIO,	sizeof (szVGAIO) - 1,	NULL,	fnTestVGAIO},
	{PARM_NONE,	szNATREG,	szDescNATREG,	sizeof (szNATREG) - 1,	NULL,	fnTestNatReg},
	{PARM_VGAMODE,	szVGAMODE,	szDescVGAMODE,	sizeof (szVGAMODE) - 1,	NULL,	fnTestVGAMode},
	{PARM_NATMODE,	szNATMODE,	szDescNATMODE,	sizeof (szNATMODE) - 1,	NULL,	fnTestNatMode},
	{PARM_RAMDAC,	szRAMDAC,	szDescRAMDAC,	sizeof (szRAMDAC) - 1,	NULL,	fnTestRAMDAC},
	{PARM_MEMORY,	szVGAROM,	szDescVGAROM,	sizeof (szVGAROM) - 1,	NULL,	fnTestVGAROM},
	{PARM_NONE,	szACCEL2D,	szDescACCEL2D,	sizeof (szACCEL2D) - 1,	NULL,	fnTestAccel2D},
	{PARM_NONE,	szBITBLT,	szDescBITBLT,	sizeof (szBITBLT) - 1,	NULL,	fnTestBitBlt},
	{PARM_NONE,	szACCEL3D,	szDescACCEL3D,	sizeof (szACCEL3D) - 1,	NULL,	fnTestAccel3D},
	{PARM_NONE,	szDMA,		szDescDMA,	sizeof (szDMA) - 1,	NULL,	fnTestDMA},
};
int	nModuleList = sizeof (tblModuleList) / sizeof (MODULELIST);

// Parameter parsing tables for the memory tests
char	szPrmFull[] = "FULL";
char	szPrmSize[] = "SIZE";
char	szPrmPat[] = "PAT";
char	szPrmStart[] = "START";
char	szPrmEnd[] = "END";
char	szPrmMask[] = "MASK";
PARMLIST tblParmMemoryList[] = {
	{CMD_NOPARM,		szPrmFull,	sizeof (szPrmFull) - 1,		(LPVOID) &_bMemFullTest},
	{CMD_NUMBERPARM,	szPrmSize,	sizeof (szPrmSize) - 1,		(LPVOID) &_nMemUnitSize},
	{CMD_NUMBERPARM,	szPrmPat,	sizeof (szPrmPat) - 1,		(LPVOID) &_dwMemPattern},
	{CMD_NUMBERPARM,	szPrmStart,	sizeof (szPrmStart) - 1,	(LPVOID) &_dwMemStart},
	{CMD_NUMBERPARM,	szPrmEnd,	sizeof (szPrmEnd) - 1,		(LPVOID) &_dwMemEnd},
	{CMD_NUMBERPARM,	szPrmMask,	sizeof (szPrmMask) - 1,		(LPVOID) &_dwMemMask}
};
int	nParmMemoryList = sizeof (tblParmMemoryList) / sizeof (PARMLIST);

// Parameter parsing tables for VGAMODE
char	szPrmAll[] = "ALL";
char	szPrmMode[] = "MODE";
char	szPrmSelect[] = "SELECT";
PARMLIST tblParmVGAModeList[] = {
	{CMD_NOPARM,		szPrmSelect,	sizeof (szPrmSelect) - 1,	(LPVOID) &_bSelectVGAModes},
	{CMD_NOPARM,		szPrmAll,	sizeof (szPrmAll) - 1,		(LPVOID) &_bAllVGAModes},
	{CMD_NUMBERPARM,	szPrmMode,	sizeof (szPrmMode) - 1,		(LPVOID) &_nVGAMode}
};
int	nParmVGAModeList = sizeof (tblParmVGAModeList) / sizeof (PARMLIST);

// Parameter parsing tables for NATMODE
char	szPrmXRes[] = "XRES";
char	szPrmYRes[] = "YRES";
char	szPrmBPP[] = "BPP";
char	szPrmRefresh[] = "REFRESH";

PARMLIST tblParmNatModeList[] = {
	{CMD_NOPARM,		szPrmSelect,	sizeof (szPrmSelect) - 1,   (LPVOID) &_bSelNatModes},
	{CMD_NOPARM,		szPrmAll,	sizeof (szPrmAll) - 1,	    (LPVOID) &_bAllNatModes},
	{CMD_NUMBERPARM,	szPrmXRes,	sizeof (szPrmXRes) - 1,	    (LPVOID) &_nXRes},
	{CMD_NUMBERPARM,	szPrmYRes,	sizeof (szPrmYRes) - 1,	    (LPVOID) &_nYRes},
	{CMD_NUMBERPARM,	szPrmBPP,	sizeof (szPrmBPP) - 1,	    (LPVOID) &_nBPP},
	{CMD_NUMBERPARM,	szPrmRefresh,	sizeof (szPrmRefresh) - 1,  (LPVOID) &_nRefresh}
};
int	nParmNatModeList = sizeof (tblParmNatModeList) / sizeof (PARMLIST);

// Parameter parsing tables for RAMDAC
char	szPrmVGA[] = "VGA";
char	szPrmNative[] = "NATIVE";
char	szPrmPLL[] = "PLL";
char	szPrmCursor[] = "CURSOR";
PARMLIST tblParmRAMDACList[] = {
	{CMD_NOPARM,		szPrmVGA,		sizeof (szPrmVGA) - 1,		(LPVOID) &_bRAMDACVGA},
	{CMD_NOPARM,		szPrmNative,	sizeof (szPrmNative) - 1,	(LPVOID) &_bRAMDACNative},
	{CMD_NOPARM,		szPrmPLL,		sizeof (szPrmPLL) - 1,		(LPVOID) &_bRAMDACPLL},
	{CMD_NOPARM,		szPrmCursor,	sizeof (szPrmCursor) - 1,	(LPVOID) &_bRAMDACCursor},
	{CMD_NOPARM,		szPrmFull,		sizeof (szPrmFull) - 1,		(LPVOID) &_bRAMDACFull},
};
int	nParmRAMDACList = sizeof (tblParmRAMDACList) / sizeof (PARMLIST);

char	szCmdLine[MAX_LINE];
BYTE	byFileBuffer[FILEIO_BUFFER];
int	nFileBufferSize = FILEIO_BUFFER;
int	nFileBufferIdx = FILEIO_BUFFER + 1;
int	nCmdFileLine = 0;

// Color information
COLORREF			clrWhite = {
	0x01, 0x03, 0x0F, 0x0F, 0x7FFF, 0xFFFF, 0x00FFFFFF
};
COLORREF			clrBlack = {
	0x00, 0x00, 0x00, 0x00, 0x0000, 0x0000, 0x00000000
};
COLORREF			clrRed = {
	0x00, 0x01, 0x0C, 0x0C, 0x7C00, 0xF800, 0x00FF0000
};
COLORREF			clrGreen = {
	0x00, 0x02, 0x0A, 0x0A, 0x03E0, 0x07E0, 0x0000FF00
};
COLORREF			clrBlue = {
	0x00, 0x01, 0x09, 0x09, 0x001F, 0x001F, 0x000000FF
};

// Smooth color filled gradient
BYTE	dactable[] =
{
	0x00, 0x00, 0x00,		0x00, 0x00, 0x00,		0x02, 0x00, 0x02,		0x04, 0x00, 0x04,
	0x06, 0x00, 0x06,		0x08, 0x00, 0x08,		0x0A, 0x00, 0x0A,		0x0C, 0x00, 0x0C,
	0x0E, 0x00, 0x0E,		0x10, 0x00, 0x10,		0x12, 0x00, 0x12,		0x14, 0x00, 0x14,
	0x16, 0x00, 0x16,		0x18, 0x00, 0x18,		0x1A, 0x00, 0x1A,		0x1C, 0x00, 0x1C,
	0x1E, 0x00, 0x1E,		0x20, 0x00, 0x20,		0x22, 0x00, 0x22,		0x24, 0x00, 0x24,
	0x26, 0x00, 0x26,		0x28, 0x00, 0x28,		0x2A, 0x00, 0x2A,		0x2C, 0x00, 0x2C,
	0x2E, 0x00, 0x2E,		0x30, 0x00, 0x30,		0x32, 0x00, 0x32,		0x34, 0x00, 0x34,
	0x36, 0x00, 0x36,		0x38, 0x00, 0x38,		0x3A, 0x00, 0x3A,		0x3C, 0x00, 0x3C,
	0x3E, 0x00, 0x3E,		0x3F, 0x00, 0x3F,		0x3F, 0x00, 0x3E,		0x3F, 0x00, 0x3C,
	0x3F, 0x00, 0x3A,		0x3F, 0x00, 0x38,		0x3F, 0x00, 0x36,		0x3F, 0x00, 0x34,
	0x3F, 0x00, 0x32,		0x3F, 0x00, 0x30,		0x3F, 0x00, 0x2E,		0x3F, 0x00, 0x2C,
	0x3F, 0x00, 0x2A,		0x3F, 0x00, 0x28,		0x3F, 0x00, 0x26,		0x3F, 0x00, 0x24,
	0x3F, 0x00, 0x22,		0x3F, 0x00, 0x20,		0x3F, 0x00, 0x1E,		0x3F, 0x00, 0x1C,
	0x3F, 0x00, 0x1A,		0x3F, 0x00, 0x18,		0x3F, 0x00, 0x16,		0x3F, 0x00, 0x14,
	0x3F, 0x00, 0x12,		0x3F, 0x00, 0x10,		0x3F, 0x00, 0x0E,		0x3F, 0x00, 0x0C,
	0x3F, 0x00, 0x0A,		0x3F, 0x00, 0x08,		0x3F, 0x00, 0x06,		0x3F, 0x00, 0x04,
	0x3F, 0x00, 0x02,		0x3F, 0x00, 0x00,		0x3F, 0x02, 0x00,		0x3F, 0x04, 0x00,
	0x3F, 0x06, 0x00,		0x3F, 0x08, 0x00,		0x3F, 0x0A, 0x00,		0x3F, 0x0C, 0x00,
	0x3F, 0x0E, 0x00,		0x3F, 0x10, 0x00,		0x3F, 0x12, 0x00,		0x3F, 0x14, 0x00,
	0x3F, 0x16, 0x00,		0x3F, 0x18, 0x00,		0x3F, 0x1A, 0x00,		0x3F, 0x1C, 0x00,
	0x3F, 0x1E, 0x00,		0x3F, 0x20, 0x00,		0x3F, 0x22, 0x00,		0x3F, 0x24, 0x00,
	0x3F, 0x26, 0x00,		0x3F, 0x28, 0x00,		0x3F, 0x2A, 0x00,		0x3F, 0x2C, 0x00,
	0x3F, 0x2E, 0x00,		0x3F, 0x30, 0x00,		0x3F, 0x32, 0x00,		0x3F, 0x34, 0x00,
	0x3F, 0x36, 0x00,		0x3F, 0x38, 0x00,		0x3F, 0x3A, 0x00,		0x3F, 0x3C, 0x00,
	0x3F, 0x3E, 0x00,		0x3F, 0x3F, 0x00,		0x3E, 0x3F, 0x00,		0x3C, 0x3F, 0x00,
	0x3A, 0x3F, 0x00,		0x38, 0x3F, 0x00,		0x36, 0x3F, 0x00,		0x34, 0x3F, 0x00,
	0x32, 0x3F, 0x00,		0x30, 0x3F, 0x00,		0x2E, 0x3F, 0x00,		0x2C, 0x3F, 0x00,
	0x2A, 0x3F, 0x00,		0x28, 0x3F, 0x00,		0x26, 0x3F, 0x00,		0x24, 0x3F, 0x00,
	0x22, 0x3F, 0x00,		0x20, 0x3F, 0x00,		0x1E, 0x3F, 0x00,		0x1C, 0x3F, 0x00,
	0x1A, 0x3F, 0x00,		0x18, 0x3F, 0x00,		0x16, 0x3F, 0x00,		0x14, 0x3F, 0x00,
	0x12, 0x3F, 0x00,		0x10, 0x3F, 0x00,		0x0E, 0x3F, 0x00,		0x0C, 0x3F, 0x00,
	0x0A, 0x3F, 0x00,		0x08, 0x3F, 0x00,		0x06, 0x3F, 0x00,		0x04, 0x3F, 0x00,
	0x02, 0x3F, 0x00,		0x00, 0x3F, 0x00,		0x00, 0x3F, 0x02,		0x00, 0x3F, 0x04,
	0x00, 0x3F, 0x06,		0x00, 0x3F, 0x08,		0x00, 0x3F, 0x0A,		0x00, 0x3F, 0x0C,
	0x00, 0x3F, 0x0E,		0x00, 0x3F, 0x10,		0x00, 0x3F, 0x12,		0x00, 0x3F, 0x14,
	0x00, 0x3F, 0x16,		0x00, 0x3F, 0x18,		0x00, 0x3F, 0x1A,		0x00, 0x3F, 0x1C,
	0x00, 0x3F, 0x1E,		0x00, 0x3F, 0x20,		0x00, 0x3F, 0x22,		0x00, 0x3F, 0x24,
	0x00, 0x3F, 0x26,		0x00, 0x3F, 0x28,		0x00, 0x3F, 0x2A,		0x00, 0x3F, 0x2C,
	0x00, 0x3F, 0x2E,		0x00, 0x3F, 0x30,		0x00, 0x3F, 0x32,		0x00, 0x3F, 0x34,
	0x00, 0x3F, 0x36,		0x00, 0x3F, 0x38,		0x00, 0x3F, 0x3A,		0x00, 0x3F, 0x3C,
	0x00, 0x3F, 0x3E,		0x00, 0x3F, 0x3F,		0x00, 0x3E, 0x3F,		0x00, 0x3C, 0x3F,
	0x00, 0x3A, 0x3F,		0x00, 0x38, 0x3F,		0x00, 0x36, 0x3F,		0x00, 0x34, 0x3F,
	0x00, 0x32, 0x3F,		0x00, 0x30, 0x3F,		0x00, 0x2E, 0x3F,		0x00, 0x2C, 0x3F,
	0x00, 0x2A, 0x3F,		0x00, 0x28, 0x3F,		0x00, 0x26, 0x3F,		0x00, 0x24, 0x3F,
	0x00, 0x22, 0x3F,		0x00, 0x20, 0x3F,		0x00, 0x1E, 0x3F,		0x00, 0x1C, 0x3F,
	0x00, 0x1A, 0x3F,		0x00, 0x18, 0x3F,		0x00, 0x16, 0x3F,		0x00, 0x14, 0x3F,
	0x00, 0x12, 0x3F,		0x00, 0x10, 0x3F,		0x00, 0x0E, 0x3F,		0x00, 0x0C, 0x3F,
	0x00, 0x0A, 0x3F,		0x00, 0x08, 0x3F,		0x00, 0x06, 0x3F,		0x00, 0x04, 0x3F,
	0x00, 0x02, 0x3F,		0x00, 0x00, 0x3F,		0x02, 0x02, 0x3F,		0x04, 0x04, 0x3F,
	0x06, 0x06, 0x3F,		0x08, 0x08, 0x3F,		0x0A, 0x0A, 0x3F,		0x0C, 0x0C, 0x3F,
	0x0E, 0x0E, 0x3F,		0x10, 0x10, 0x3F,		0x12, 0x12, 0x3F,		0x14, 0x14, 0x3F,
	0x16, 0x16, 0x3F,		0x18, 0x18, 0x3F,		0x1A, 0x1A, 0x3F,		0x1C, 0x1C, 0x3F,
	0x1E, 0x1E, 0x3F,		0x20, 0x20, 0x3F,		0x22, 0x22, 0x3F,		0x24, 0x24, 0x3F,
	0x26, 0x26, 0x3F,		0x28, 0x28, 0x3F,		0x2A, 0x2A, 0x3F,		0x2C, 0x2C, 0x3F,
	0x2E, 0x2E, 0x3F,		0x30, 0x30, 0x3F,		0x32, 0x32, 0x3F,		0x34, 0x34, 0x3F,
	0x36, 0x36, 0x3F,		0x38, 0x38, 0x3F,		0x3A, 0x3A, 0x3F,		0x3C, 0x3C, 0x3F,
	0x3E, 0x3E, 0x3F,		0x3F, 0x3F, 0x3F,		0x3E, 0x3E, 0x3E,		0x3C, 0x3C, 0x3C,
	0x3A, 0x3A, 0x3A,		0x38, 0x38, 0x38,		0x36, 0x36, 0x36,		0x34, 0x34, 0x34,
	0x32, 0x32, 0x32,		0x30, 0x30, 0x30,		0x2E, 0x2E, 0x2E,		0x2C, 0x2C, 0x2C,
	0x2A, 0x2A, 0x2A,		0x28, 0x28, 0x28,		0x26, 0x26, 0x26,		0x24, 0x24, 0x24,
	0x22, 0x22, 0x22,		0x20, 0x20, 0x20,		0x1E, 0x1E, 0x1E,		0x1C, 0x1C, 0x1C,
	0x1A, 0x1A, 0x1A,		0x18, 0x18, 0x18,		0x16, 0x16, 0x16,		0x14, 0x14, 0x14,
	0x12, 0x12, 0x12,		0x10, 0x10, 0x10,		0x0E, 0x0E, 0x0E,		0x0C, 0x0C, 0x0C,
	0x0A, 0x0A, 0x0A,		0x08, 0x08, 0x08,		0x06, 0x06, 0x06,		0x04, 0x04, 0x04,
	0x02, 0x02, 0x02,		0x00, 0x00, 0x00
};

// Strings
char	szAuthor[] = "Written by: Larry Coffey. (3/9/98)";
char	szCopyright[] = "\nElpin Systems Graphics Diagnostic Utility   Version " STR_VERSION
							"\nCopyright (c) 1998 by Elpin Systems, Inc."
							"\nAll rights reserved.\n";
char	szUsage[] = "\nRuns Elpin Systems Graphics Diagnostic Utility"
						"\n\nDIAG [-? | -HELP | -HELPALL] | [-INFO]"
						"\nDIAG @{filename} [-CARD={card ID}] | [-SPEED={QUICK | EXTENDED}] |"
						"\n     [-SERIAL={port}] | [-LOG={log file}] | [-GENCRC={file} | -USECRC={file}]"
						"\nDIAG -T={module name} [-P={parameter list}] | [-CARD={card ID}] |"
						"\n     [-SPEED={QUICK | EXTENDED}] | [-SERIAL={port}] | [-LOG={log file}]"
						"\n\n  @{filename}\tSpecifies an optional command file"
						"\n  ? or HELP\tLists these options"
						"\n  HELPALL\tLists all module tests and their parameters"
						"\n  INFO\t\tDisplay information about the video card"
						"\n  T\t\tTest a specific part of the video card"
						"\n  P\t\tProvide parameters to the test"
						"\n  CARD\t\tSelect a specific card in a multiple card system"
						"\n  SPEED\t\tSpecify the type of test, either a QUICK or EXTENDED test"
						"\n  SERIAL\tEnable reporting over the serial port (0=None, 1=COM1, 2=COM2)"
						"\n  LOG\t\tSpecify the output log file for errors and other information"
						"\n  GENCRC\tCreate a golden file of CRC values"
						"\n  USECRC\tUse values from a CRC file"
						"\n\nSome tests may run for several seconds and/or cause the screen to blank."
						"\n";
char	szInvalidArgc[] = "\nInvalid number of command line arguments. Use EDIAG /? to list commands.\n";
char	sznInvalidArgv[] = "\nParameter (#%d) on the command line is invalid. Use EDIAG /? to list commands.\n";
char	szNoCommandFile[] = "\nERROR: Invalid command line parameter. Specified command file does not exist.\n";
char	sznInvalidCmdLine[] = "\nERROR: Line #%d in command file is invalid. Could not process command:\n%s\n";
char	szLogOpenError[] = "\nERROR: Requested log file could not be opened for writing.\n";
char	szLogHeader[] = "EDIAG Log File -- Version " STR_VERSION "\n";
char	szCRCOpenError[] = "\nERROR: Requested CRC file could not be opened for writing.\n";
char	szCRCUseAndGenError[] = "\nERROR: GENCRC and USECRC can not be used at the same time.\n";
char	szUseMore[] = "\n\nIf this message is displayed too quickly, then pipe the output through"
							"\nthe DOS \"MORE\" command. For example, at the DOS prompt, enter:"
							"\n\n  EDIAG -HELPALL | MORE\n";
char	szParmNone[] = "\n\tParameters: None";
char	szParmMemory[] = "\n\tParameter:"
								"\n\t\tSIZE={1 | 2 | 4}"
								"\n\t\tPAT={pattern}"
								"\n\t\tSTART={start offset address}"
								"\n\t\tEND={end offset address}"
								"\n\t\tMASK={mask}";
char	szParmVGAMode[] = "\n\tParameters:"
								"\n\t\tALL"
								"\n\t\tMODE={mode number}";
char	szParmNatMode[] = "\n\tParameters:"
								"\n\t\tALL"
								"\n\t\tXRES={horizontal resolution}"
								"\n\t\tYRES={vertical resolution}"
								"\n\t\tBPP={color depth in bits per pixel}"
								"\n\t\tREFRESH={refresh}";
char	szParmRAMDAC[] = "\n\tParameters:"
								"\n\t\tVGA"
								"\n\t\tNATIVE"
								"\n\t\tPLL"
								"\n\t\tCURSOR"
								"\n\t\tFULL";
char	szDeviceNotFound[] = "\nERROR: Graphics card was not found.";
char	szVBE20Error[] = "Invalid Pointer to VBE 2.0 data string.";

//
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//

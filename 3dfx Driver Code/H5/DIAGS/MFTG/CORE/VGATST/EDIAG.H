//
//		EDIAG.H - Defines, structures, and data types for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/9/98
//		Last modified:	5/12/98
//
#ifndef _EDIAG_H_
#define _EDIAG_H_

#include	"vgadata.h"

extern void (*callback_fn)(int);

// Defines
#define	STR_VERSION					"0.78"
#define	FALSE							0
#define	TRUE 							1
#define	MAX_LINE						256			// Max characters on prompt line
#define	MAX_FILENAME				256
#define	MAX_DISPLAY					512
#define	MAX_ERRORS					5
#define	FILEIO_BUFFER				(4*1024)		// MUST be divisible by 32K

//	Character codes
#define	CHR_SLINE_UR				191
#define	CHR_SLINE_LL				192
#define	CHR_SLINE_LR				217
#define	CHR_SLINE_UL				218
#define	CHR_SLINE_VERT				179
#define	CHR_SLINE_HORZ				196
#define	CHR_SLINE_DOWNT			194
#define	CHR_SLINE_UPT				193
#define	CHR_SLINE_LEFTT			195
#define	CHR_SLINE_RIGHTT			180
#define	CHR_SLINE_CROSS			197
#define	CHR_DLINE_UR				187
#define	CHR_DLINE_LL				200
#define	CHR_DLINE_LR				188
#define	CHR_DLINE_UL				201
#define	CHR_DLINE_VERT				186
#define	CHR_DLINE_HORZ				205
#define	CHR_DLINE_DOWNT			203
#define	CHR_DLINE_UPT				202
#define	CHR_DLINE_LEFTT			204
#define	CHR_DLINE_RIGHTT			185
#define	CHR_DLINE_CROSS			206

#define	CHR_DLINE_SLEFTT			182
#define	CHR_DLINE_SRIGHTT			199
#define	CHR_DLINE_SUPT				207
#define	CHR_DLINE_SDOWNT			209
#define	CHR_SPACE					32

// Key codes
#define	KEY_BACKSPACE				0x0008
#define	KEY_ESCAPE					0x001B
#define	KEY_F1						0x3B00
#define	KEY_F2						0x3C00
#define	KEY_F3						0x3D00
#define	KEY_HOME						0x4700
#define	KEY_ARROW_UP				0x4800
#define	KEY_ARROW_LF				0x4B00
#define	KEY_ARROW_RT				0x4D00
#define	KEY_END						0x4F00
#define	KEY_ARROW_DN				0x5000
#define	KEY_INS						0x5200
#define	KEY_DEL						0x5300

// Useful I/O values
#define	IODELAYPORT					0xED				// Unused I/O port
#define	PORTB							0x61
#define	FLAG_REFRESHSTATUS		0x10				// Port B refresh signal status

//	Register definitions
#define	PS2_SETUP					0x94
#define	VGA_SETUP					0x102
#define	ADAPTER_ENABLE				0x46E8
#define	CRTC_MINDEX					0x3B4
#define	CRTC_MDATA					0x3B5
#define	INPUT_MSTATUS_1			0x3BA
#define	FEAT_MWCONTROL				0x3BA
#define	ATC_INDEX					0x3C0
#define	ATC_RDATA					0x3C1
#define	INPUT_STATUS_0				0x3C2
#define	MISC_OUTPUT					0x3C2
#define	MB_ENABLE					0x3C3
#define	SEQ_INDEX					0x3C4
#define	SEQ_DATA						0x3C5
#define	DAC_MASK						0x3C6
#define	DAC_RINDEX					0x3C7
#define	DAC_WINDEX					0x3C8
#define	DAC_DATA						0x3C9
#define	FEAT_RCONTROL				0x3CA
#define	MISC_INPUT					0x3CC
#define	GDC_INDEX					0x3CE
#define	GDC_DATA						0x3CF
#define	CRTC_CINDEX					0x3D4
#define	CRTC_CDATA					0x3D5
#define	INPUT_CSTATUS_1			0x3DA
#define	FEAT_CWCONTROL				0x3DA

//Default Diag App Color Defs.
#define BLACK							0x00
#define BLUE							0x01
#define GREEN							0x02
#define RED								0x03
#define YELLOW							0x04
#define CYAN							0x05
#define Magenta						0x06
#define White							0x07

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
#ifndef LPDWORD
typedef	DWORD						FAR	*LPDWORD;
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
#ifndef LPVOID
typedef	void						FAR	*LPVOID;
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

#ifndef __max
#define	__max(a,b)					(((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
#define	__min(a,b)					(((a) < (b)) ? (a) : (b))
#endif

// Data structures and data types
typedef struct tagREGION {
	DWORD	PhysAddr;	/* physical address base */
	DWORD PhysSize;	/* size in bytes */
	DWORD	MappedAddr;	/* zero if unmapped */
	DWORD	MappedSize;	/* zero if unmapped */
} REGION;
typedef	REGION					FAR *LPREGION;

typedef struct tagCARDINFO {
	WORD		pciVendorID;	// Vendor-specific PCI Vendor ID
	WORD		pciDeviceID;	// Vendor-specific PCI Device ID
	DWORD		physAddr0;		// Physical memory address of framebuffer
	long int	nSizeAddr0;		// Size of framebuffer
	DWORD		physAddr1;		// Physical memory address of additional buffer
	long int	nSizeAddr1;		// Size of additional buffer
	DWORD		physAddr2;		// Physical memory address of additional buffer
	long int	nSizeAddr2;		// Size of additional buffer
	DWORD		physAddr3;		// Physical memory address of additional buffer
	long int	nSizeAddr3;		// Size of additional buffer
	DWORD		physAddr4;		// Physical memory address of additional buffer
	long int	nSizeAddr4;		// Size of additional buffer
	DWORD		physAddr5;		// Physical memory address of additional buffer
	long int	nSizeAddr5;		// Size of additional buffer
	DWORD		physAddr6;		// Physical memory address of additional buffer
	long int	nSizeAddr6;		// Size of additional buffer
	DWORD		physAddr7;		// Physical memory address of additional buffer
	long int	nSizeAddr7;		// Size of additional buffer
	DWORD		physROM;			// Physical address of the ROM
	WORD		ioBase;			// I/O Base
	LPWORD	lpModeList;		// Pointer to available VBE-style modes (0FFFFh terminates)
	BOOL		bInVGAMode;		// In VGA mode flag (TRUE = In VGA Mode, FALSE = In Native Mode)
} CARDINFO;
typedef	CARDINFO					FAR *LPCARDINFO;

typedef struct tagVBEINFOBLOCK		//	VESA BIOS information
{
	BYTE		VESASignature[4];
	WORD		VESAVersion;
	LPBYTE	OEMStringPtr;
	BYTE		Capabilities[4];
	LPWORD	VideoModePtr;
	WORD		TotalMemory;
	WORD		VBE_SoftwareRev;
	LPBYTE	VBE_VendorNamePtr;
	LPBYTE	VBE_ProductNamePtr;
	LPBYTE	VBE_ProductRevPtr;
	BYTE		reserved[222];				//	Fill out to 256 bytes
	BYTE		VBE_OEM_DATA[256];
} VBEINFOBLOCK;
typedef	VBEINFOBLOCK					FAR *LPVBEINFOBLOCK;

typedef struct tagVBEMODEINFOBLOCK		// VESA Mode information block
{
//	Mandatory info for all revs
	WORD	ModeAttributes;
	BYTE	WinAAttributes;
	BYTE	WinBAttributes;
	WORD	WinGranularity;
	WORD	WinSize;
	WORD	WinASegment;
	WORD	WinBSegment;
	DWORD	WinFuncPtr;
	WORD	BytesPerScanLine;
//	Mandatory info for 1.2 and above (bit D1 of "ModeAttributes" = 1)
	WORD	XResolution;
	WORD	YResolution;
	BYTE	XCharSize;
	BYTE	YCharSize;
	BYTE	NumberOfPlanes;
	BYTE	BitsPerPixel;
	BYTE	NumberOfBanks;
	BYTE	MemoryModel;
	BYTE	BankSize;
	BYTE	NumberOfImagePages;
	BYTE	reserved1;
// Direct color fields (required for direct color modes)
	BYTE	RedMaskSize;
	BYTE	RedFieldPosition;
	BYTE	GreenMaskSize;
	BYTE	GreenFieldPosition;
	BYTE	BlueMaskSize;
	BYTE	BlueFieldPosition;
	BYTE	RsvdMaskSize;
	BYTE	RsvdFieldPosition;
	BYTE	DirectColorModeInfo;
// Mandatory info for VBE 2.0 and above
	DWORD	PhysBasePtr;
	DWORD	OffScreenMemPtr;
	WORD	OffScreenMemSize;
// Mandatory info for VBE 3.0 and above
	WORD	LinBytesPerScanLine;
	BYTE	BnkNumberOfImagePages;
	BYTE	LinNumberOfImagePages;
	BYTE	LinRedMaskSize;
	BYTE	LinRedFieldPosition;
	BYTE	LinGreenMaskSize;
	BYTE	LinGreenFieldPosition;
	BYTE	LinBlueMaskSize;
	BYTE	LinBlueFieldPosition;
	BYTE	LinRsvdMaskSize;
	BYTE	LinRsvdFieldPosition;
	DWORD	MaxPixelClock;
	BYTE	reserved2[189];		// Fill out to 256 bytes
} VBEMODEINFOBLOCK;
typedef	VBEMODEINFOBLOCK				FAR *LPVBEMODEINFOBLOCK;

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
typedef	REALMODECALL					FAR *LPREALMODECALL;

#define	CMD_NOPARM						0
#define	CMD_TEXTPARM					1
#define	CMD_NUMBERPARM					2
#define	CMD_FILEPARM					3
typedef	BOOL	(*PFB_SNP)(LPSTR, long int, LPBOOL);	// K&R (2nd ed.) pg. 147
typedef	BOOL	(*PFB)(void);									// K&R (2nd ed.) pg. 147
typedef struct tagCMDLINELIST {
	int		type;					// Following parameter type (0 = None, 1 = Text, 2 = Numeric, 3 = File)
	LPSTR		cmd;					// Pointer to the command text
	int		len;					// Size of command text string
	PFB_SNP	func;					// Pointer to a handler function
} CMDLINELIST;
typedef	CMDLINELIST						FAR *LPCMDLINELIST;

#define	PARM_NONE						0
#define	PARM_MEMORY						1
#define	PARM_VGAMODE					2
#define	PARM_NATMODE					3
#define	PARM_RAMDAC						4
typedef struct tagMODULELIST {
	int		type;					// Parameter list type
	LPSTR		name;					// Name of the module
	LPSTR		desc;					// Description of the module
	int		len;					// Size of the module name string
	PFB_SNP	funcparse;			// Pointer to the function to parse parameters
	PFB		functest;			// Pointer to the function to test
} MODULELIST;
typedef	MODULELIST						FAR *LPMODULELIST;

typedef struct tagPARMLIST {
	int		type;					// Type of parameter (0 = None, 1 = Text, 2 = Numeric, 3 = File)
	LPSTR		name;					// Name of parameter
	int		len;					// Size of the parameter name string
	LPVOID	pvar;					// Pointer to the global variable used by the parameter
} PARMLIST;
typedef	PARMLIST							FAR *LPPARMLIST;

// Color reference
typedef struct tagCOLORREF {
	BYTE			bpp1;				// 2 colors, monochrome
	BYTE			bpp2;				// 4 colors
	BYTE			bpp4;				// 16 colors
	BYTE			bpp8;				// 256 colors
	WORD			bpp15;			// 32K colors
	WORD			bpp16;			// 64K colors
	DWORD			bpp24;			// 16.7M colors
} COLORREF;
typedef	COLORREF							*LPCOLORREF;

// Used by the VGA I/O test
typedef struct tagIOTABLE {
	WORD	wport;			// Address to write
	WORD	rport;			// Address to read
	BOOL	fIndexed;		// Index flag (TRUE = Use index, FALSE = Don't)
	BYTE	idx;				// Index if an indexed port
	BYTE	mask;				// Read/write mask (1 = writeable bit)
	BYTE	rsvdmask;		// Reserved bits mask (1 = reserved)
	BYTE	rsvddata;		// Expected value of reserved bits
	BYTE	caremask;		// Don't care bits (1 = don't care)
	BOOL	bError;			// Error reporting (TRUE = error occured)
	BYTE	byexp;			// Error reporting (expected data)
	BYTE	byact;			// Error reporting (actual data)
} IOTABLE;

// MAIN.CPP
void InitVars (void);
void PrepareExit (void);

// CMDLINE.CPP
BOOL ProcessCmdLineParm (LPSTR, int, LPBOOL);
BOOL ProcessRawCommand (LPSTR, LPBOOL);
BOOL fnShowHelp (LPSTR, long int, LPBOOL);
BOOL fnShowAll (LPSTR, long int, LPBOOL);
BOOL fnShowInfo (LPSTR, long int, LPBOOL);
BOOL fnSetCard (LPSTR, long int, LPBOOL);
BOOL fnSetSpeed (LPSTR, long int, LPBOOL);
BOOL fnOpenLog (LPSTR, long int, LPBOOL);
BOOL fnSetTest (LPSTR, long int, LPBOOL);
BOOL fnSetParm (LPSTR, long int, LPBOOL);
LPSTR GetEntry (LPSTR, int *, char, LPBOOL);
void EvaluateEntry (LPPARMLIST, int, LPSTR, int, LPBOOL);
BOOL fnWaitKey (LPSTR, long int, LPBOOL);
BOOL fnWait (LPSTR, long int, LPBOOL);
BOOL fnSetSerial (LPSTR, long int, LPBOOL);
BOOL fnGenCRC (LPSTR, long int, LPBOOL);
BOOL fnUseCRC (LPSTR, long int, LPBOOL);

// DISPLAY.CPP
void DrawCommonScreen (WORD, LPVBEMODEINFOBLOCK,int);
void WriteBIOSChar (BYTE, BYTE, BYTE, BYTE);
BOOL VerifyScreen (WORD);
DWORD VideoMemoryChecksum (BYTE __far *, int, long int);

// USER.CPP
void DisplayChar (char);
void DisplayString (LPSTR);
void DisplayFormattedString (LPSTR, ...);
void DisplayCopyright (void);
void DisplayUsage (void);
WORD GetKey (void);
void ReportOpen (void);
void ReportClose (void);
void ReportString (LPSTR);

// FILE.CPP
void LogWriteString (LPSTR);
void LogComment (char *szFormat, ...);
BOOL ReadCmdFile (FILE *, LPSTR, int);
void ProcessCmdLine (LPSTR, LPBOOL);
void SaveCRC (DWORD);
DWORD ReadCRC (void);

// TESTS.CPP
BOOL fnTestVGAMem (void);
BOOL fnTestNatMemA (void);
BOOL fnTestNatMemB (void);
BOOL fnTestZBuffer (void);
BOOL fnTestTexture (void);
BOOL fnTestVGAIO (void);
BOOL fnTestNatReg (void);
BOOL fnTestVGAMode (void);
BOOL fnTestNatMode (void);
BOOL fnTestRAMDAC (void);
BOOL fnTestVGAROM (void);
BOOL fnTestAccel2D (void);
BOOL fnTestBitBlt (void);
BOOL fnTestAccel3D (void);
BOOL fnTestDMA (void);

// VGA.CPP
BOOL VGAIOTest (void);
BOOL VGATestRAMDAC (void);
BOOL VGAMemSetReset (void);
BOOL VGAColorCompare (void);
BOOL VGAROPs (void);
BOOL VGAWriteModeOne (void);
BOOL VGAWriteModeTwo (void);
BOOL VGAWriteModeThree (void);
BOOL VGABitMask (void);
void SetLatchedBytes (BYTE __far *, BYTE, int);

// MEM.CPP
BOOL FlatMemoryTests (DWORD, int, long);
BOOL SingleMemoryTest (BYTE __far *, int, long, DWORD);
BOOL FullMemoryTest (BYTE __far *, int, long);
BOOL MemAddressTest (BYTE __far *, int, long);
void FillMemoryRange (BYTE __far *, int, long, DWORD);
BOOL TestMemoryRange (BYTE __far *, int, long, DWORD);
BOOL TestMemWalkingOne (BYTE __far *, int, long);
BOOL TestMemWalkingZero (BYTE __far *, int, long);
void MemError(void);
void MemReportError (void);

//	VBEAPI.CPP
BOOL GetVBEInfo (LPVBEINFOBLOCK);
BOOL GetVBEModeInfo (LPVBEMODEINFOBLOCK, WORD);
void VBEHLine (WORD, WORD, WORD, LPCOLORREF);
void VBEVLine (WORD, WORD, WORD, LPCOLORREF);
void VBETextOut (WORD, WORD, LPSTR, BYTE);
void VBEWriteCharAt (WORD, WORD, BYTE, BYTE);
void VBEWriteChar (BYTE);
void VBECircle (int, int, int, LPCOLORREF);
void VBEPixel (WORD, WORD, LPCOLORREF);
void VBESetBank (BYTE);
WORD VBEGetBank (void);
void VBEIncBank (void);
void SetCursorPositionW (WORD, WORD);
void EnableCursor(int enable);
void SetTextColorTrans (LPCOLORREF, LPCOLORREF, BOOL);
void DrawGradient (WORD, WORD, WORD, WORD);
LPCOLORREF GetNextColor (WORD, WORD);
void SetDAC (BYTE, BYTE, BYTE, BYTE);
BYTE GetColorIntensity (BYTE, BYTE, BYTE);

// VBEDRVR.CPP
void HLineCGA1 (WORD, WORD, WORD, BYTE);
void HLineCGA (WORD, WORD, WORD, BYTE);
void HLinePacked4 (WORD, WORD, WORD, BYTE);
void HLine4 (WORD, WORD, WORD, BYTE);
void HLine8 (WORD, WORD, WORD, BYTE);
void BankHLine8 (WORD, WORD, WORD, BYTE);
void HLine16 (WORD, WORD, WORD, WORD);
void BankHLine16 (WORD, WORD, WORD, WORD);
void HLine24 (WORD, WORD, WORD, DWORD);
void BankHLine24 (WORD, WORD, WORD, DWORD);
void VLine4 (WORD, WORD, WORD, BYTE);
void VLine8 (WORD, WORD, WORD, BYTE);
void VLine16 (WORD, WORD, WORD, WORD);
void VLine24 (WORD, WORD, WORD, DWORD);
void WriteCharPacked4 (BYTE);
void WriteChar4 (BYTE);
void WriteChar8 (BYTE);
void BankWriteChar8 (BYTE);
void WriteChar16 (BYTE);
void BankWriteChar16 (BYTE);
void WriteChar24 (BYTE);
void BankWriteChar24 (BYTE);
void DrawPixel4 (WORD, WORD, BYTE);
void DrawPixel8 (WORD, WORD, BYTE);
void DrawPixel16 (WORD, WORD, WORD);
void DrawPixel24 (WORD, WORD, DWORD);

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
long int ConvertStr2Value (LPSTR, LPBOOL);
DWORD ConvertDecimal (LPBYTE, int, LPBOOL);
DWORD ConvertHexadecimal (LPBYTE, int, LPBOOL);
int TranslateTable (BYTE, LPBYTE, int);
BYTE IsIObitFunctional (WORD, WORD, BYTE);
BOOL SetMode (WORD);
WORD GetMode (void);
void ClearIObitDataBus (void);
BOOL IOByteTest (WORD, BYTE, LPBYTE);
BYTE FlagFailure (BYTE, BYTE, BYTE);
void SetCursorPositionB (BYTE, BYTE);
void WriteBIOSString (BYTE, BYTE, BYTE, LPBYTE, BYTE);
void SetTextColor (LPCOLORREF, LPCOLORREF);
void SetStandardModeInfo (WORD, LPVBEMODEINFOBLOCK);
void BIOSWriteChar (BYTE, BYTE);
BOOL FindPCIDev (WORD, WORD, LPBYTE, LPBYTE);
BOOL ReadPCIDWord (BYTE, BYTE, BYTE, LPDWORD);
BOOL ReadPCIWord (BYTE, BYTE, BYTE, LPWORD);
BOOL ReadPCIByte (BYTE, BYTE, BYTE, LPBYTE);
BOOL WritePCIDWord (BYTE, BYTE, BYTE, DWORD);
BOOL WritePCIWord (BYTE, BYTE, BYTE, WORD);
BOOL WritePCIByte (BYTE, BYTE, BYTE, BYTE);
unsigned long power (unsigned long, unsigned long);
void GetDac (BYTE, LPBYTE, LPBYTE, LPBYTE);
void SetDac (BYTE, BYTE, BYTE, BYTE);
void FillDAC (BYTE, BYTE, BYTE);
void FillDACLength (BYTE, WORD, BYTE, BYTE, BYTE);
void SetDacBlock (LPBYTE, BYTE, WORD);
BYTE RotateByteRight (BYTE, BYTE);
void VGASetPixelAt (BYTE __far *, BYTE, BYTE);
void MoveBytes (BYTE __far *, BYTE __far *, int);

// REPDEV Library functions
#define	REPDEV_MONO					0
#define	REPDEV_SERIAL				1
#define	REPDEV_OEM					16
BOOL OpenReportDevice (int, WORD, int);
BOOL CloseReportDevice (int);
BOOL WriteStringReportDevice (int, LPSTR);

// OEM Library functions
BOOL OEMInit (LPCARDINFO, WORD);
BOOL OEMTerminate (LPCARDINFO);
BOOL OEMSetNativeMode (LPCARDINFO);
BOOL OEMSetVGAMode (LPCARDINFO);
BOOL OEMVerifyScreen (LPCARDINFO, WORD, BOOL, DWORD, BOOL, LPDWORD);
LPVBEMODEINFOBLOCK OEMSetMode (LPCARDINFO, WORD);
LPVBEMODEINFOBLOCK OEMSetResolution (LPCARDINFO, WORD, WORD, WORD, WORD, LPWORD);
void OEMPreMemoryTest (LPCARDINFO, int);
BOOL OEMTestAccel2D (LPCARDINFO);
BOOL OEMTestBitBlt (LPCARDINFO);
BOOL OEMTestAccel3D (LPCARDINFO);
BOOL OEMTestDMA (LPCARDINFO);
BOOL OEMTestRAMDAC (LPCARDINFO);
BOOL OEMTestPLL (LPCARDINFO);
BOOL OEMTestHWCursor (LPCARDINFO);

// Data definitions
#ifndef _DATAFILE_
extern	DWORD 					dwTestPatterns[];
extern	int						nTestPatterns;
extern 	WORD						yTextRow;
extern 	WORD						xTextCol;
extern	BYTE						byBankShifter;
extern 	LPCOLORREF				lpclrTextFore;
extern	LPCOLORREF				lpclrTextBack;
extern	BOOL						bOpaque;
extern	LPBYTE					lpFont;
extern 	VBEMODEINFOBLOCK		vbeModeInfo;
extern	WORD						wFFFF;

extern	FILE						*hInDevice;
extern	FILE						*hOutDevice;
extern	FILE						*hLogDevice;
extern	FILE						*hCRCDevice;
extern	WORD						selFlat;
extern	WORD						selData;
extern	LPBYTE					lpVideoA000;
extern	LPBYTE					lpVideoB000;
extern	LPBYTE					lpVideoB800;
extern	BOOL						bLinear;
extern	BYTE						__far *lpLinFrameBuffer;
extern	CARDINFO					CardInfo;
extern	int						nCOMPort;

#ifdef DEBUG
extern	BOOL						bUseLogFile;
extern	char						szLogFile[MAX_FILENAME];
#endif
extern	char						szCRCFile[MAX_FILENAME];

extern	BOOL						bRepMono;
extern	BOOL						bRepSerial;
extern	BOOL						bRepOEM;

extern	BOOL						_bTestPending;
extern	long int					_nTestIndex;
extern	BOOL						_bGenCRC;
extern	BOOL						_bUseCRC;
extern	long int					_nCardID;
extern	BOOL						_bExtendedTest;
extern	long int					_nMemUnitSize;
extern	DWORD						_dwMemPattern;
extern	DWORD						_dwMemStart;
extern	DWORD						_dwMemEnd;
extern	DWORD						_dwMemMask;
extern	BOOL						_bMemFullTest;
extern	BOOL						_bClearScreen;
extern	BOOL						_bAllVGAModes;
extern	BOOL						_bSelectVGAModes;
extern	long int					_nVGAMode;
extern	BOOL						_bAllNatModes;
extern	BOOL						_bSelNatModes;
extern	long int					_nXRes;
extern	long int					_nYRes;
extern	long int					_nBPP;
extern	long int					_nRefresh;
extern	BOOL						_bRAMDACVGA;
extern	BOOL						_bRAMDACNative;
extern	BOOL						_bRAMDACPLL;
extern	BOOL						_bRAMDACCursor;
extern	BOOL						_bRAMDACFull;

extern	DWORD						_errLinAddr;
extern	DWORD						_errActual;
extern	DWORD						_errExpected;
extern	int						_errSize;
extern	int						_errCount;

extern	char						chLeader1;
extern	char						chLeader2;
extern	char						chSpecialLeader;
extern	char						cCmdDelimiter;
extern	char						cParmDelimiter;
extern	BYTE						tblHexDigits[];
extern	char						szQuick[];
extern	char						szExtended[];
extern	CMDLINELIST				tblCmdLineList[];
extern	int						nCmdLineList;
extern	MODULELIST				tblModuleList[];
extern	int						nModuleList;
extern	PARMLIST 				tblParmMemoryList[];
extern	int						nParmMemoryList;
extern	PARMLIST					tblParmVGAModeList[];
extern	int						nParmVGAModeList;
extern	PARMLIST					tblParmNatModeList[];
extern	int						nParmNatModeList;
extern	PARMLIST 				tblParmRAMDACList[];
extern	int						nParmRAMDACList;

extern	char						szCmdLine[MAX_LINE];
extern	BYTE						byFileBuffer[FILEIO_BUFFER];
extern	int						nFileBufferSize;
extern	int						nFileBufferIdx;
extern	int						nCmdFileLine;

extern	COLORREF					clrWhite;
extern	COLORREF					clrBlack;
extern	COLORREF					clrRed;
extern	COLORREF					clrGreen;
extern	COLORREF					clrBlue;
extern	BYTE						dactable[];

extern	char						szCopyright[];
extern	char						szUsage[];
extern	char						szInvalidArgc[];
extern	char						sznInvalidArgv[];
extern	char						szNoCommandFile[];
extern	char						sznInvalidCmdLine[];
extern	char						szLogOpenError[];
extern	char						szLogHeader[];
extern	char						szCRCOpenError[];
extern	char						szCRCUseAndGenError[];
extern	char						szUseMore[];
extern	char						szParmNone[];
extern	char						szParmMemory[];
extern	char						szParmVGAMode[];
extern	char						szParmNatMode[];
extern	char						szParmRAMDAC[];
extern	char						szDeviceNotFound[];
extern	char						szVBE20Error[];
  #endif			// _DATAFILE_
#endif			// _EDIAG_H_

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


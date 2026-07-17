//
//		VGACORE.H - Defines, structures, and prototypes for VGACORE.LIB
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				2/17/94
//		Last modified:	2/19/98
//
#ifndef _VGACORE_H
#define _VGACORE_H

#if (!defined(SOLARIS) && !defined(hpux))
  #ifdef	__cplusplus
extern "C" {			/* Assume C declarations for C++ */
  #endif					/* __cplusplus */
#endif					/* SOLARIS */

#define	FALSE						0
#define	TRUE						1
#define	VERSION_MAJOR			2
#define	VERSION_MINOR			0
#define	VERSION_SUBMINOR		0
#define	VERSION_STRING			"2.00"

// Implementation specific equates
#ifndef sparc
  #ifndef hpux
	 #define	__MSC__					1
    #ifdef  __MSVC32__
		#define	__X86_32__			1
    #else
		#define	__X86_16__			1
    #endif
  #endif
#endif

#ifndef __MSC__
#define _enable()
#define _disable()
#define __max(a,b)				((a>b)?a:b)
#define __min(a,b)				((a<b)?a:b)
#define _kbhit()					0
#endif

#ifdef __MSC__
  #include	<conio.h>
  #include	<dos.h>
#endif

#ifdef __X86_16__
//#define	FAR						_far
//#else
#define	FAR						
#endif

//	Register definitions
#define	PS2_SETUP				0x94
#define	VGA_SETUP				0x102
#define	ADAPTER_ENABLE			0x46E8
#define	CRTC_MINDEX				0x3B4
#define	CRTC_MDATA				0x3B5
#define	INPUT_MSTATUS_1		0x3BA
#define	FEAT_MWCONTROL			0x3BA
#define	ATC_INDEX				0x3C0
#define	ATC_RDATA				0x3C1
#define	INPUT_STATUS_0			0x3C2
#define	MISC_OUTPUT				0x3C2
#define	MB_ENABLE				0x3C3
#define	SEQ_INDEX				0x3C4
#define	SEQ_DATA					0x3C5
#define	DAC_MASK					0x3C6
#define	DAC_RINDEX				0x3C7
#define	DAC_WINDEX				0x3C8
#define	DAC_DATA					0x3C9
#define	FEAT_RCONTROL			0x3CA
#define	MISC_INPUT				0x3CC
#define	GDC_INDEX				0x3CE
#define	GDC_DATA					0x3CF
#define	CRTC_CINDEX				0x3D4
#define	CRTC_CDATA				0x3D5
#define	INPUT_CSTATUS_1		0x3DA
#define	FEAT_CWCONTROL			0x3DA

//	Useful attribute definitions
#define	ATTR_NORMAL				0x07
#define	ATTR_BLINK				0x87

// Useful timer tick values (18.2065 timer ticks per second)
#define	FIVE_SECONDS			91
#define	TWO_SECONDS				36
#define	ONE_SECOND				18

//	Bus types
#define	BUS_ISA					1
#define	BUS_VESA					2
#define	BUS_PCI					3

//	Error definitions
#define	ERROR_NONE				0			// No error occurred
#define	ERROR_USERABORT		1			// User aborted test
#define	ERROR_UNEXPECTED		2			// Unexpected behavior
#define	ERROR_IOFAILURE		3			// I/O Failure
#define	ERROR_CAPTURE			4			// Capture buffer didn't match
#define	ERROR_INVALIDFRAMES	5			// Wrong or no frame rate
#define	ERROR_INTERNAL			6			// Test couldn't complete (malloc failure, etc.)
#define	ERROR_MEMORY			7			// Video memory failure
#define	ERROR_VIDEO				8			// Video data failure
#define	ERROR_INVALIDCLOCK	9			// Wrong dot clock frequency
#define	ERROR_SIM_INIT			10			// Initialization error in simulation
#define	ERROR_CHECKSUM			11			// Checksum error
#define	ERROR_FILE				12			// File open/read/write/close error

//	Key codes
#define	KEY_BACKSPACE				0x0008
#define	KEY_TAB						0x0009
#define	KEY_ENTER					0x000D
#define	KEY_CTRL_N					0x000E
#define	KEY_CTRL_O					0x000F
#define	KEY_CTRL_R					0x0012
#define	KEY_CTRL_S					0x0013
#define	KEY_ESCAPE					0x001B
#define	KEY_SPACE					0x0020
#define	KEY_N							0x004E
#define	KEY_Y							0x0059
#define	KEY_n							0x006E
#define	KEY_y							0x0079
#define	KEY_SHIFT_TAB				0x0F00
#define	KEY_HOME						0x4700
#define	KEY_ARROW_UP				0x4800
#define	KEY_PGUP						0x4900
#define	KEY_ARROW_LEFT				0x4B00
#define	KEY_ARROW_RIGHT			0x4D00
#define	KEY_END						0x4F00
#define	KEY_ARROW_DOWN				0x5000
#define	KEY_PGDN						0x5100
#define	KEY_DELETE					0x5300
#define	KEY_CTRL_ARROW_LEFT		0x7300
#define	KEY_CTRL_ARROW_RIGHT		0x7400
#define	KEY_CTRL_PGDN				0x7600
#define	KEY_CTRL_PGUP				0x8400
#define	KEY_F1						0x3B00
#define	KEY_F2						0x3C00
#define	KEY_F3						0x3D00
#define	KEY_F4						0x3E00
#define	KEY_F5						0x3F00
#define	KEY_F6						0x4000
#define	KEY_F7						0x4100
#define	KEY_F8						0x4200
#define	KEY_F9						0x4300
#define	KEY_F10						0x4400

// Industry standard macros
#define LOBYTE(w)					((BYTE)(w))
#define HIBYTE(w)					((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define LOWORD(l)					((WORD)((DWORD)(l) & 0xFFFF))
#define HIWORD(l)					((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#define MAKELONG(l,h)			((LONG)(((WORD)(l))|(((DWORD)((WORD)(h)))<<16)))

//	Industry standard type definitions
typedef	unsigned char			BYTE;
typedef	unsigned short			WORD;
#ifdef sparc
typedef	unsigned int			DWORD;
#else
  #ifdef hpux
typedef	unsigned int			DWORD;
  #else
typedef	unsigned long			DWORD;
  #endif
#endif
typedef	WORD						BOOL;
typedef	BYTE						FAR *LPBYTE;
typedef	WORD						FAR *LPWORD;
typedef	DWORD						FAR *LPDWORD;
typedef	char						FAR *LPSTR;
typedef	BOOL						FAR *LPBOOL;
typedef	DWORD						SEGOFF;

// Structures
#ifndef PARMENTRY_DEFINED
  #ifdef __MSC__
#pragma pack( 1 )
  #endif
#define	PARMENTRY_DEFINED		1
typedef struct tagPARMENTRY {
	BYTE	columns;
	BYTE	rows;
	BYTE	char_height;
	BYTE	regen_length_low;
	BYTE	regen_length_high;
	BYTE	seq_data[4];
	BYTE	misc;
	BYTE	crtc_data[25];
	BYTE	atc_data[20];
	BYTE	gdc_data[9];
} PARMENTRY;
typedef	PARMENTRY				FAR *LPPARMENTRY;
  #ifdef __MSC__
#pragma pack()
  #endif
#endif

typedef struct tagVERTDATA {
	WORD	vtot;
	WORD	vde;
	WORD	vss;
	WORD	vbs;
	BYTE	vse;
	BYTE	vbe;
} VERTDATA;
typedef	VERTDATA					FAR *LPVERTDATA;

//
//	Function prototypes
//
//	COREC0.C
void SetPixelAt (SEGOFF, BYTE, BYTE);
BOOL RWMemoryTest (WORD, WORD);
BYTE IsIObitFunctional (WORD, WORD, BYTE);
void ClearIObitDataBus (void);
BYTE FlagFailure (BYTE, BYTE, BYTE);
BOOL IsVGAEnabled (void);
void SetCursorPosition (BYTE, BYTE, BYTE);
WORD GetKey (void);
WORD PeekKey (void);
void DisableCursor (void);
//void EnableCursor (int enable);
DWORD GetFrameRate (void);
DWORD GetSystemTicks (void);
BOOL WaitVerticalRetrace (void);
BOOL WaitNotVerticalRetrace (void);
BYTE GetVideoData (void);
BYTE GetNonVideoData (void);
BYTE RotateByteLeft (BYTE, BYTE);
BYTE RotateByteRight (BYTE, BYTE);
WORD RotateWordRight (WORD, BYTE);
//void MoveBytes (SEGOFF, SEGOFF, int);
SEGOFF CompareBytes (SEGOFF, SEGOFF, int);
SEGOFF VerifyBytes (SEGOFF, BYTE, int);
void SetScans (WORD wScan);
BOOL DisplayInspectionMessage (void);
BOOL FrameCapture (int, int);
void StartCapture(BYTE);
void EndCapture(void);
void SwapWords (LPWORD, LPWORD);
void MemoryFill (SEGOFF, BYTE, WORD);
void GetResolution (LPWORD, LPWORD);
WORD StringLength (LPSTR);
void SystemCleanUp (void);

//	COREC1.C
void PreFontLoad (void);
void PostFontLoad (void);
void LoadFontGlyph (BYTE, BYTE, BYTE, LPBYTE);
void Load8x8 (BYTE);
void Load8x14 (BYTE);
void Load8x16 (BYTE);
void LoadFont (LPBYTE, BYTE, BYTE, WORD, BYTE);
void DrawTransparentChar (WORD, WORD, BYTE, BYTE, BYTE, LPBYTE);
void DrawMonoChar (WORD, WORD, BYTE, LPBYTE, WORD, WORD);
void LoadFixup (LPBYTE, BYTE, BYTE);

//	COREC2.C
int FlagError (int, int, int, DWORD, WORD, DWORD, DWORD);
void DisplayError (void);

//	COREC3.C
//void SetMode (WORD);
void SetDac (BYTE, BYTE, BYTE, BYTE);
void GetDac (BYTE, LPBYTE, LPBYTE, LPBYTE);
void FillDAC (BYTE, BYTE, BYTE);
void FillDACLength (BYTE, WORD, BYTE, BYTE, BYTE);
void SetDacBlock (LPBYTE, BYTE, WORD);
void SetRegs (LPPARMENTRY);
LPPARMENTRY GetParmTable (WORD);
void SetBIOSVars (WORD, LPPARMENTRY);
void ModeLoadFont (WORD, LPPARMENTRY);
void ModeLoadDAC (WORD, LPPARMENTRY);
void ModeClearMemory (WORD);
void SetAll2CPU (BOOL);

// COREC4.C
void TextCharOut (BYTE, BYTE, BYTE, BYTE, BYTE);
void TextStringOut (LPSTR, BYTE, BYTE, BYTE, BYTE);
void PlanarCharOut (BYTE, BYTE, BYTE, BYTE, BYTE);
void CGA4CharOut (BYTE, BYTE, BYTE, BYTE, BYTE);
void CGA2CharOut (BYTE, BYTE, BYTE, BYTE, BYTE);
void MonoGrCharOut (BYTE, BYTE, BYTE, BYTE, BYTE);
void VGACharOut (BYTE, BYTE, BYTE, BYTE, BYTE);
void Line4 (WORD, WORD, WORD, WORD, BYTE);
void HLine4Internal (WORD, WORD, WORD);
void VLine4Internal (WORD, WORD, WORD);
DWORD CalcStart4 (WORD, WORD, LPBYTE);
void SetLine4Columns (WORD);

//
//	Data definitions
//
#ifndef	_DATAFILE_
extern	int	_vcerr_code;
extern	int	_vcerr_part;
extern	int	_vcerr_test;
extern	DWORD	_vcerr_address;
extern	WORD	_vcerr_index;
extern	DWORD	_vcerr_expected;
extern	DWORD	_vcerr_actual;

extern	BYTE	_line_color;
extern	BYTE	_line_rop;
extern	WORD	_line_columns;

extern	BOOL	_bLoadDAC;

extern	WORD	tblFontBlock[];

extern	BYTE	tblFont8x8[];
extern	BYTE	tblFont8x14[];
extern	BYTE	tblFont8x16[];
extern	BYTE	tblFont9x14[];
extern	BYTE	tblFont9x16[];
extern	BYTE	mapFont8x8[];
extern	BYTE	mapFont8x14[];
extern	BYTE	mapFont9x16[];
extern	BYTE	tblParameterTable[];
extern	BYTE	tblSmallParameterTable[];
extern	BYTE	tbl16ColorDAC[];
extern	BYTE	tblCGADAC[];
extern	BYTE	tblMonoDAC[];
extern	BYTE	tbl256ColorDAC[];
extern	int	nDAC16color;
extern	int	nDACcga;
extern	int	nDACmono;
extern	int	nDAC256color;
extern	BOOL	_bCaptureStream;
#endif

#if (!defined(SOLARIS) && !defined(hpux))
  #ifdef __cplusplus
}							/* End of extern "C" { */
  #endif					/* __cplusplus */
#endif					/* SOLARIS && hpux */

#endif					/* End of #ifndef _VGACORE_H */
//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


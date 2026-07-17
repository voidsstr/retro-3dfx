//
//		TEST.H - Equates, data definitions, and prototypes for VGASIM test utility
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				12/18/94
//		Last modified:	5/2/97
//

#define	VER_TEST_MAJOR			"1"
#define	VER_TEST_MINOR			"1"
#define	VER_TEST_SUBMINOR		"0"
#define	VER_TEST_BETALETTER	"A"
#define	VER_TEST_STRING		VER_TEST_MAJOR "." VER_TEST_MINOR VER_TEST_SUBMINOR VER_TEST_BETALETTER
#define	MemoryDwordRead(addr)				(MemDwordRead(addr))
#define	MemoryWordRead(addr)					(MemWordRead(addr))
#define	MemoryByteWrite(addr,data)			(MemByteWrite(addr,data))
#define	MemoryByteRead(addr)					(MemByteRead(addr))

// Type definitions
typedef	BOOL				(FAR *LPFV)(char *, int, BOOL *, int *);
typedef struct tagCMDLINE {
	BYTE	cmd;
	LPFV	proc;
} CMDLINE;

//	Function prototypes
void DisplayCopyright (void);
void DisplayChar (char);
void DisplayString (LPSTR);
void DisplayPrompt (void);
void DumpBytes (SEGOFF, SEGOFF);
void DumpWords (SEGOFF, SEGOFF);
void DumpDwords (SEGOFF, SEGOFF);
void Load8x8 (BYTE);
void Load8x14 (BYTE);
void Load8x16 (BYTE);
void Line4 (WORD, WORD, WORD, WORD, BYTE);
void SetMode (WORD);
char GetDataType (char);
DWORD GetHexData (char **, int *, BOOL *, int);
DWORD GetAddress (char **, int *, DWORD, BOOL, BOOL *);
BOOL CmdMode (char *, int, BOOL *, int *);
BOOL CmdWrite (char *, int, BOOL *, int *);
BOOL CmdTest (char *, int, BOOL *, int *);
BOOL CmdBreak (char *, int, BOOL *, int *);
BOOL ParseCommand (char *, BOOL *, int *);
BOOL CmdDump (char *, int, BOOL *, int *);
BOOL CmdEnter (char *, int, BOOL *, int *);
BOOL CmdIn (char *, int, BOOL *, int *);
BOOL CmdOut (char *, int, BOOL *, int *);
BOOL CmdQuit (char *, int, BOOL *, int *);
BOOL CmdHelp (char *, int, BOOL *, int *);

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

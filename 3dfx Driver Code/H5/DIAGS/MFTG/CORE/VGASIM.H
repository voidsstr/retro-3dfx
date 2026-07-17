//
//		VGASIM.H	- The interface definitions for the simulator Lib.
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Start Date:		12/4/94
//		Last modified:	2/19/98
//
#ifndef _VGASIM_H
#define _VGASIM_H

#if (!defined(SOLARIS) && !defined(hpux))
  #ifdef	__cplusplus
extern "C" {			/* Assume C declarations for C++ */
  #endif					/* __cplusplus */
#endif					/* SOLARIS */

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
#ifdef __X86_16__
#define	FAR						_far
#else
#define	FAR
#endif

// Defines
#define	FALSE					0
#define	TRUE 					1
#define	CAP_SCREEN			0
#define	CAP_OVERSCAN		1
#define	CAP_COMPOSITE		2
#define	BLINK_OFF			0
#define	BLINK_ON				1

// Simulation types
#define	SIM_MOTHERBOARD		0x0000
#define	SIM_ADAPTER				0x0001
#define	SIM_PHYSICAL			0x0000
#define	SIM_SIMULATION			0x0002
#define	SIM_NOVECTORS			0x0000
#define	SIM_VECTORS				0x0004
#define	SIM_STDFRAME			0x0000
#define	SIM_SMALLFRAME			0x0008
#define	SIM_FULLACCESS			0x0000
#define	SIM_TURBOACCESS		0x0010

// Industry standard data types and others
#ifndef BYTE
typedef	unsigned char			BYTE;
#endif
#ifndef WORD
typedef	unsigned short			WORD;
#endif
#ifndef DWORD
  #ifdef sparc
typedef	unsigned int			DWORD;
  #else
    #ifdef hpux
typedef	unsigned int			DWORD;
	 #else
typedef	unsigned long			DWORD;
	 #endif
  #endif
#endif
#ifndef BOOL
typedef	WORD						BOOL;
#endif
#ifndef LPBYTE
  typedef	BYTE					FAR *LPBYTE;
#endif
#ifndef LPWORD
  typedef	WORD					FAR *LPWORD;
#endif
#ifndef LPBOOL
  typedef	BOOL					FAR *LPBOOL;
#endif
#ifndef LPSTR
  typedef	char					FAR *LPSTR;
#endif
#ifndef SEGOFF
  typedef	DWORD					SEGOFF;
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

// Function prototypes
int		SimStart (void);
void 		SimEnd (void);
void 		IOByteWrite (WORD, BYTE);
BYTE 		IOByteRead (WORD);
BOOL 		IOByteTest (WORD, BYTE, LPBYTE);
void 		IOWordWrite (WORD, WORD);
WORD 		IOWordRead (WORD);
void 		MemByteWrite (SEGOFF, BYTE);
BYTE 		MemByteRead (SEGOFF);
BOOL 		MemByteTest (SEGOFF, BYTE, LPBYTE);
void 		MemWordWrite (SEGOFF, WORD);
WORD 		MemWordRead (SEGOFF);
void 		MemDwordWrite (SEGOFF, DWORD);
DWORD		MemDwordRead (SEGOFF);
void 		SimSetCaptureMode (BYTE);
BYTE 		SimGetCaptureMode (void);
void 		WaitCursorBlink (WORD);
void 		WaitAttrBlink (WORD);
int 		CaptureFrame (LPSTR);
WORD 		SimSetType (WORD);
WORD 		SimGetType (void);
BOOL 		SimSetFrameSize (BOOL);
WORD 		SimGetKey (void);
void 		SimSetState (BOOL, BOOL, BOOL);
void 		SimGetState (LPBOOL, LPBOOL, LPBOOL);
void 		SimDumpMemory (LPSTR);
BOOL		SimLoadFont (LPBYTE);
BOOL		SimLoadDac (LPBYTE, WORD);
BOOL		SimClearMemory (DWORD);
void		SimStartCapture (BYTE);
void		SimEndCapture (void);
void 		SimComment (LPSTR);
LPBYTE	SimGetParms (WORD);
int 		VectorStart (void);
void 		VectorEnd (void);
void 		VectorIOByteWrite (WORD, BYTE);
void 		VectorIOByteRead (WORD);
void 		VectorIOByteTest (WORD, BYTE);
void 		VectorIOWordWrite (WORD, WORD);
void 		VectorIOWordRead (WORD);
void 		VectorMemByteWrite (SEGOFF, BYTE);
void 		VectorMemByteRead (SEGOFF);
void 		VectorMemByteTest (SEGOFF, BYTE);
void 		VectorMemWordWrite (SEGOFF, WORD);
void 		VectorMemWordRead (SEGOFF);
void 		VectorMemDwordWrite (SEGOFF, DWORD);
void 		VectorMemDwordRead (SEGOFF);
void 		VectorCaptureFrame (LPSTR);
void 		VectorWaitCursorBlink (WORD);
void 		VectorWaitAttrBlink (WORD);
void 		VectorDumpMemory (LPSTR);
void 		VectorComment (LPSTR);
void		Finish(void);

#if (!defined(SOLARIS) && !defined(hpux))
  #ifdef __cplusplus
}							/* End of extern "C" { */
  #endif					/* __cplusplus */
#endif					/* SOLARIS && hpux */

#endif					// End of #ifndef _VGASIM_H
//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


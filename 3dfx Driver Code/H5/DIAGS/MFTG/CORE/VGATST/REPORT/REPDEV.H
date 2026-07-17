//
//		REPDEV.H - Internal definitions and structures for REPDEV.LIB
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/17/98
//		Last modified:	5/4/98
//

// MONO.CPP
BOOL OpenMonoDevice (void);
BOOL CloseMonoDevice (void);
BOOL WriteStringMonoDevice (LPSTR);
void MonoFullScreenScroll (void);
void MonoSetCursorPosition (int, int);

// SERIAL.CPP
BOOL OpenSerialDevice (int);
BOOL CloseSerialDevice (void);
BOOL WriteStringSerialDevice (LPSTR);

// From OEM library
BOOL OpenOEMReportDevice (void);
BOOL CloseOEMReportDevice (void);
BOOL WriteStringOEMReportDevice (LPSTR);

// Datafile
#ifndef _DATAFILE_
extern	WORD			_selFlatDescriptor;
extern	BOOL			_bMonoOpen;
extern	BOOL			_bSerialOpen;
extern	DWORD			_linMono;
extern	BYTE far		*_lpMono;
extern	int			_nMonoCol;
extern	int			_nMonoRow;
extern	int			_nSerialPort;
#endif

//
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//

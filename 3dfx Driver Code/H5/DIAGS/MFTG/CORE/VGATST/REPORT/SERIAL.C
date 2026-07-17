//
//		SERIAL.CPP - Serial port specific init routines for REPDEV.LIB
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/17/98
//		Last Modified:	5/4/98
//
//		Routines in this file:
//		OpenSerialDevice			Initialize serial port reporting device
//		CloseSerialDevice			Terminate serial reporting device
//		WriteStringSerialDevice	Write a string to the serial port
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include "ediag.h"
#include	"repdev.h"

//
//		OpenSerialDevice - Initialize serial port reporting device
//
//		Entry:	nPort		Serial port (1 = COM1, 2 = COM2, etc.)
//		Exit:		<BOOL>	Success flag (TRUE = Opened, FALSE = Device failed)
//
BOOL OpenSerialDevice (int nPort)
{
	_nSerialPort = nPort - 1;
	_bSerialOpen = TRUE;
	if (nPort == 0) _bSerialOpen = FALSE;

	return (_bSerialOpen);
};

//
//		CloseSerialDevice - Terminate serial reporting device
//
//		Entry:	None
//		Exit:		<BOOL>		Success flag (TRUE = Closed, FALSE = Device failed)
//
BOOL CloseSerialDevice (void)
{
	if (!_bSerialOpen) return (FALSE);

	_bSerialOpen = FALSE;
	return (TRUE);
}

//
//		WriteStringSerialDevice - Write a string to the serial port
//
//		Entry:	lpstr		Pointer to string to write
//		Exit:		<BOOL>	Success flag (TRUE = Written, FALSE = Device failed)
//
BOOL WriteStringSerialDevice (LPSTR lpstr)
{
	union REGS		regs;

	if (!_bSerialOpen) return (FALSE);

	while (*lpstr != '\0')
	{
		// If there is a "newline" character, make sure to send
		// a "return" with it.
		if (*lpstr == '\n')
		{
			regs.h.ah = 0x01;
			regs.h.al = '\r';
			regs.w.dx = (WORD) _nSerialPort;
			int386 (0x14, &regs, &regs);
		}
		regs.h.ah = 0x01;
		regs.h.al = *lpstr;
		regs.w.dx = (WORD) _nSerialPort;
		int386 (0x14, &regs, &regs);
		lpstr++;
	}

	return (TRUE);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//

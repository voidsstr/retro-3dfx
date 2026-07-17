//
//		UTIL.CPP - Utility routines for FLASH.EXE
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/4/98
//		Last modified:	3/9/98
//
//		Routines in this file:
//		RealModeINT					Perform a Real Mode software interrupt (INT nn)
//		AllocateFlatDescriptor	Allocate a flat model data selector
//		FreeDecriptor				Free an allocated selector
//		_outp							Perform an I/O BYTE write
//		_outpw						Perform an I/O WORD write
//		_outpd						Perform an I/O DWORD write
//		_inp							Perform an I/O BYTE read
//		_inpw							Perform an I/O WORD read
//		_inpd							Perform an I/O DWORD read
//		Phys2Linear					Convert a physical address into a linear address
//		Delay15us					Delay for a number of 15 microsecond intervals
//
#include	<stdio.h>
#include	<dos.h>
#include	"flash.h"

//
//		RealModeINT - Perform a Real Mode software interrupt (INT nn)
//
//		Entry:	lprmc		Pointer to a REALMODECALL data structure
//					byINT		INT vector to call
//		Exit:		None
//
void RealModeINT (REALMODECALL *lprmc, BYTE byINT)
{
	union REGS		regs;
	struct SREGS	sregs;

	// Use DPMI handler's stack
	lprmc->ss = 0;
	lprmc->sp = 0;

	// Call down to real mode
	regs.x.eax = 0x300;
	regs.h.bl = byINT;
	regs.h.bh = 0;
	regs.x.ecx = 0;
	sregs.ds = selData;
	sregs.es = FP_SEG (lprmc);
	regs.x.edi = FP_OFF (lprmc);
	int386x (0x31, &regs, &regs, &sregs);
}

//
//		AllocateFlatDescriptor - Allocate a flat model data selector
//
//		Entry:	None
//		Exit:		<WORD>	Selector allocated (0 = Error)
//
WORD AllocateFlatDescriptor (void)
{
	WORD				wSel;
	union REGS		regs;
	struct SREGS	sregs;
	static BYTE		tblDescriptor[8] = {
		0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF2, 0xCF, 0x00
	};

	// Allocate a new LDT descriptor
	regs.w.ax = 0x0000;
	regs.w.cx = 1;
	int386 (0x31, &regs, &regs);
	wSel = regs.w.ax;

	// Set it to flat model (base = 0, limit = FFFFFF, gran = 4K)
	regs.x.eax = 0x000C;
	regs.w.bx = wSel;
	sregs.ds = selData;								// Must do this or return fails
	sregs.es = FP_SEG (tblDescriptor);
	regs.x.edi = FP_OFF (tblDescriptor);
	int386x (0x31, &regs, &regs, &sregs);

	return (wSel);
}

//
//		FreeDecriptor - Free an allocated selector
//
//		Entry:	wSel		Selector to free
//		Exit:		None
//
void FreeDescriptor (WORD wSel)
{
	union REGS	regs;

	regs.w.ax = 0x0001;				// Free descriptor
	regs.w.bx = wSel;
	int386 (0x31, &regs, &regs);
}

//
//		_outp - Perform an I/O BYTE write
//
//		Entry:	wPort		I/O address
//					byValue	Value to set
//		Exit:		None
//
extern void OUTP (WORD, BYTE);
#pragma aux OUTP = \
	"out	dx,al" \
	parm [dx] [al];

void _outp (WORD wPort, BYTE byValue)
{
	OUTP (wPort, byValue);
}

//
//		_outpw - Perform an I/O WORD write
//
//		Entry:	wPort		I/O address
//					wValue	Value to set
//		Exit:		None
//
extern void OUTPW (WORD, WORD);
#pragma aux OUTPW = \
	"out	dx,ax" \
	parm [dx] [ax];

void _outpw (WORD wPort, WORD wValue)
{
	OUTPW (wPort, wValue);
}

//
//		_outpd - Perform an I/O DWORD write
//
//		Entry:	wPort		I/O address
//					dwValue	Value to set
//		Exit:		None
//
extern void OUTPD (WORD, DWORD);
#pragma aux OUTPD = \
	"out	dx,eax" \
	parm [dx] [ax];

void _outpd (WORD wPort, DWORD dwValue)
{
	OUTPD (wPort, dwValue);
}

//
//		_inp - Perform an I/O BYTE read
//
//		Entry:	wPort		I/O address
//		Exit:		<BYTE>	Value read from port
//
extern BYTE INP (WORD);
#pragma aux INP = \
	"sub	eax,eax" \
	"in	al,dx" \
	parm [dx];

BYTE _inp (WORD wPort)
{
	return (INP (wPort));
}

//
//		_inpw - Perform an I/O WORD read
//
//		Entry:	wPort		I/O address
//		Exit:		<WORD>	Value read from port
//
extern WORD INPW (WORD);
#pragma aux INPW = \
	"sub	eax,eax" \
	"in	ax,dx" \
	parm [dx];

WORD _inpw (WORD wPort)
{
	return (INPW (wPort));
}

//
//		_inpd - Perform an I/O DWORD read
//
//		Entry:	wPort		I/O address
//		Exit:		<DWORD>	Value read from port
//
extern DWORD INPD (WORD);
#pragma aux INPD = \
	"in	eax,dx" \
	parm [dx];

DWORD _inpd (WORD wPort)
{
	return (INPD (wPort));
}

//
//		Phys2Linear - Convert a physical address into a linear address
//
//		Entry:	dwPhys		Physical memory address
//					dwLength		Length of region
//		Exit:		<DWORD>		Linear memory address
//
DWORD Phys2Linear (DWORD dwPhys, DWORD dwLength)
{
	union REGS	regs;
	DWORD			dwLinear;

	// If in the first megabyte, then physical = linear
	if (dwPhys < 0x100000) return (dwPhys);

	regs.w.ax = 0x0800;
	regs.w.bx = HIWORD (dwPhys);
	regs.w.cx = LOWORD (dwPhys);
	regs.w.si = HIWORD (dwLength);
	regs.w.di = LOWORD (dwLength);
	int386 (0x31, &regs, &regs);

	dwLinear = (DWORD) ((((DWORD) regs.w.bx) << 16) | regs.w.cx);

	return (dwLinear);
}

//
//		Delay15us - Delay for a number of 15 microsecond intervals
//
//		Entry:	cLoop		Number of 15 microsecond intervals to delay
//		Exit:		None
//
void Delay15us (long int cLoop)
{
	BYTE	byPBStat, byMaskState;

	if (cLoop <= 0) cLoop = 1;

	byPBStat = byMaskState = 0;
	while (cLoop--)
	{
		while (byPBStat == byMaskState)
		{
			_inp (IODELAYPORT);				// Read from an unused I/O port for delay
			byPBStat = _inp (PORTB);		// Get Port B status
			byPBStat = (BYTE) (byPBStat & FLAG_REFRESHSTATUS);
		}
		byMaskState = byPBStat;
	}
}

//
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//


//
//		UTIL.CPP - Utility routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/4/98
//		Last Modified:	5/7/98
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
//		ConvertStr2Value			Convert a text string to an integer value
//		ConvertDecimal				Convert a buffer to a decimal value
//		ConvertHexadecimal		Convert a buffer to a hexadecimal value
//		TranslateTable				Given an item, look it up in a table and return it's position
//		SetMode 						Set a given video mode
//		IsIObitFunctional			Test bits in a register and return a mask of failed bits
//		GetMode						Get the current video mode
//		ClearIObitDataBus			Read input status 1 registers to change the I/O data bus.
//		FlagFailure					Return a bit mask reflecting a failure of two values
//		IOByteTest					Test a byte at a specific I/O port
//		SetCursorPositionB			Set the current cursor position
//		WriteBIOSString			Write a string to the display
//		SetTextColor 				Set text colors on future character writes
//		SetStandardModeInfo		Set necessary mode info in VBEModeInfoBlock
//		BIOSWriteChar				Write character with attribute
//		FindPCIDev					Find a specific PCI device
//		ReadPCIDWord				Read a DWORD from specified PCI device
//		ReadPCIWord					Read a WORD from specified PCI device
//		ReadPCIByte					Read a BYTE from specified PCI device
//		WritePCIDWord				Write a DWORD to a specified PCI device
//		WritePCIWord				Write a WORD to a specified PCI device
//		WritePCIByte				Write a BYTE to a specified PCI device
//		power							Raise a long integer to a given power (base**exp)
//		GetDac						Get values from a specific DAC location
//		SetDac						Set a specific DAC location to a given value
//		FillDAC						Load the entire RAMDAC with a single value
//		FillDACLength				Load a range of RAMDAC registers with a single value
//		SetDacBlock					Set a range of DAC registers
//		RotateByteRight			Rotate a data byte a given number of times
//		VGASetPixelAt				Set a pixel at a given video offset
//		MoveBytes					Move a block of memory of a given size
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<dos.h>
#include	"ediag.h"
#include	"vgacore.h"
#include	"vgasim.h"
static	WORD	_wSimType = SIM_ADAPTER | SIM_PHYSICAL | SIM_NOVECTORS | SIM_STDFRAME | SIM_FULLACCESS;
static	BOOL	_bSimDumpIO = TRUE;
static	BOOL	_bSimDumpMem = TRUE;
static	BOOL	_bSimDumpDAC = TRUE;


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

//
//		IOByteWrite Perform an I/O BYTE write
//
//		Entry:	wPort		I/O address
//					byValue	Value to set
//		Exit:		None
//

void IOByteWrite(WORD wPort, BYTE byValue)
{
	_outp(wPort, byValue);
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
//		IOByteread- Perform an I/O BYTE read
//
//		Entry:	wPort		I/O address
//		Exit:		<BYTE>	Value read from port
//

BYTE IOByteRead(WORD wPort)
{
	return(_inp(wPort));
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
#if 0
	REALMODECALL	rmc;
	DWORD				dwLinear;

	// If in the first megabyte, then physical = linear
	if (dwPhys < 0x100000) return (dwPhys);

	rmc.eax = 0x0800;
	rmc.ebx = HIWORD (dwPhys);
	rmc.ecx = LOWORD (dwPhys);
	rmc.esi = HIWORD (dwLength);
	rmc.edi = LOWORD (dwLength);
	RealModeINT (&rmc, 0x31);

	dwLinear = ((rmc.ebx & 0xFFFF) << 16) | (rmc.ecx & 0xFFFF);

	return (dwLinear);
#else
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
#endif
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
//		ConvertStr2Value - Convert a text string to an integer value
//
//		Entry:	lpstr			Pointer to string
//					lpbError		Pointer to error variable (returned TRUE if success)
//		Exit:		<long int>	Converted value
//
//		The string may be in one of two forms:
//		[sign] digits
//		0X hexdigits
//
long int ConvertStr2Value (LPSTR lpstr, LPBOOL lpbError)
{
	static char	szHexFlag[] = "0X";
	long int		n;

	// Assume no error
	*lpbError = FALSE;

	n = 0;
	if (strnicmp (lpstr, szHexFlag, sizeof (szHexFlag) - 1) == 0)
	{
		// Convert to hexadecimal
		lpstr += 2;
		if (*lpstr == '\0')			// Verify end of string was not hit
			*lpbError = TRUE;
		else
			n = ConvertHexadecimal ((LPBYTE) lpstr, strlen (lpstr), lpbError);
	}
	else
	{
		// Convert decimal
		if (*lpstr == '\0')			// Verify end of string was not hit
			*lpbError = TRUE;
		else
			n = ConvertDecimal ((LPBYTE) lpstr, strlen (lpstr), lpbError);
	}

	return (n);
}

//
//		ConvertDecimal - Convert a buffer to a decimal value
//
//		Entry:	pBuffer		Pointer to buffer to convert
//					nCount		Number of characters to convert
//					pbError		Error flag (TRUE = Error, FALSE = Not)
//		Exit:		<DWORD>		Converted value
//
DWORD ConvertDecimal (LPBYTE pBuffer, int nCount, LPBOOL pbError)
{
	DWORD	dw;
	int	i;

	for (dw = 0, i = 0; i < nCount; i++)
	{
		if (!isdigit (*(pBuffer + i)))
		{
			*pbError = TRUE;
			return (0);
		}
		dw = (dw * 10) + TranslateTable (*(pBuffer + i), tblHexDigits, 10);
	}

	return (dw);
}

//
//		ConvertHexadecimal - Convert a buffer to a hexadecimal value
//
//		Entry:	pBuffer		Pointer to buffer to convert
//					nCount		Number of characters to convert
//					pbError		Error flag (TRUE = Error, FALSE = Not)
//		Exit:		<DWORD>		Converted value
//
DWORD ConvertHexadecimal (LPBYTE pBuffer, int nCount, LPBOOL pbError)
{
	DWORD	dw;
	int	i;
	BYTE	byDigit;

	for (dw = 0, i = 0; i < nCount; i++)
	{
		byDigit = (BYTE) toupper (*(pBuffer + i));
		if (!isxdigit (byDigit))
		{
			*pbError = TRUE;
			return (0);
		}
		dw = (dw * 16) + TranslateTable (byDigit, tblHexDigits, 16);
	}

	return (dw);
}

//
//		TranslateTable - Given an item, look it up in a table and return
//								it's position
//
//		Entry:	byItem		Item to search for
//					tblLookup	Table to search in
//					nSize			Size of table
//		Exit:		<int>			Position of entry (-1 = Not found)
//
int TranslateTable (BYTE byItem, LPBYTE tblLookup, int nSizeTable)
{
	int	i;

	for (i = 0; i < nSizeTable; i++)
		if (byItem == *tblLookup++) return (i);

	return (-1);
}

//		SetMode - Set a given video mode
//
//		Entry:	wMode		Video mode number
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL SetMode (WORD wMode)
{
	REALMODECALL	rmc;

	// If the mode number is greater than 100h, do a VBE mode set, otherwise
	// use the standard VGA BIOS interface.
	if (wMode >= 100)
	{
		// AX = 4F02h, BX = Mode #, INT 10h
		rmc.eax = 0x4F02;
		rmc.ebx = (DWORD) wMode;
	}
	else
	{
		// AH = 00h, AL = mode number
		rmc.eax = wMode;
	}
	RealModeINT (&rmc, 0x10);

	// If mode doesn't match current mode, then there's an error
	if ((GetMode () & 0x7FF) != (wMode & 0x7FF))
		return (FALSE);

	// Fill in the mode information structure. If the video BIOS reports
	// back that the information is not availble AND if it is a standard
	// VGA mode, then fill in the data structure and continue. Otherwise,
	// report that the mode is unavailable and return.
	if (GetVBEModeInfo (&vbeModeInfo, (WORD) (wMode & 0x7FF)) == FALSE)
	{
		if (wMode <= 0x13)
			SetStandardModeInfo (wMode, &vbeModeInfo);
		else
		{
			vbeModeInfo.ModeAttributes = 0;
			vbeModeInfo.XResolution = 0;
			return (FALSE);
		}
	}

	// Initialize font pointer
	if (vbeModeInfo.YCharSize <= 8)
		lpFont = tblFont8x8;
	else if (vbeModeInfo.YCharSize <= 14)
		lpFont = tblFont8x14;
	else
		lpFont = tblFont8x16;

	return (TRUE);
}

//
//		IsIObitFunctional - Test bits in a register and return a mask of failed bits
//
//		Entry:	wIOR		I/O Read address
//					wIOW		I/O Write address
//					byMask	Mask of bits to ignore
//		Exit:		<BYTE>	Bit mask of failed bits
//
BYTE IsIObitFunctional (WORD wIOR, WORD wIOW, BYTE byMask)
{
	BYTE			byBadBits, byUntouched, byNotMask, byOrgReg;
	BYTE			byExpected, byActual;
	static BYTE	bytblTestValues[] = {0x00, 0xFF, 0xAA, 0x55};
	int			i;

	byBadBits = 0;
	byOrgReg = (BYTE) _inp (wIOR);
	byUntouched = (BYTE) (byOrgReg & byMask);
	byNotMask = (BYTE) (~byMask);

	// Test with 0's, 1's, AAh, and 55h
	for (i = 0; i < sizeof (bytblTestValues); i++)
	{
		// Compensate for attribute controller toggle (assume "INDEX" state on entry)
		if ((wIOW == ATC_INDEX) && (wIOR == ATC_RDATA))
			_outp (ATC_INDEX, _inp (ATC_INDEX));
		byExpected = (BYTE) (byUntouched | (byNotMask & bytblTestValues[i]));
		_outp (wIOW, byExpected);
		ClearIObitDataBus ();
		if (!IOByteTest (wIOR, byExpected, &byActual))
			byBadBits |= FlagFailure (byExpected, byActual, byMask);
	}

	// Restore original value (compensate for attribute controller toggle
	//	(assume "INDEX" state on entry)
	if ((wIOW == ATC_INDEX) && (wIOR == ATC_RDATA))
		_outp (ATC_INDEX, _inp (ATC_INDEX));
	_outp (wIOW, byOrgReg);
	return (byBadBits);
}

//
//		GetMode - Get the current video mode
//
//		Entry:	None
//		Exit:		<WORD>	Current mode number
//
WORD GetMode (void)
{
	REALMODECALL	rmc;
	WORD				wMode;

	// Do a VBE get mode
	// AX = 4F03h, INT 10h
	rmc.eax = 0x4F03;
	RealModeINT (&rmc, 0x10);
	wMode = (WORD) rmc.ebx;

	// If the get mode fails, then use the standard VGA BIOS interface
	if ((rmc.eax & 0x0000FF00) != 0x00)
	{
		rmc.eax = 0x0F00;
		RealModeINT (&rmc, 0x10);
		wMode = (WORD) (rmc.eax & 0xFF);
	}

	return (wMode);
}

//
//		ClearIObitDataBus - Read input status 1 registers to change the I/O data bus.
//
//		Entry:	None
//		Exit:		None
//
void ClearIObitDataBus (void)
{
	_inp (INPUT_MSTATUS_1);
	_inp (INPUT_CSTATUS_1);
}

//
//		IOByteTest - Test a byte at a specific I/O port
//
//		Entry:	wPort			I/O address
//					byExp			Expected data
//					lpbyAct		Pointer to return data read
//		Exit:		<BOOL>		Success flag
//
BOOL IOByteTest (WORD wPort, BYTE byExp, LPBYTE lpbyAct)
{
	BYTE	byData;

	byData = _inp (wPort);
	*lpbyAct = byData;

	return (byExp == byData);
}

//
//		FlagFailure - Return a bit mask reflecting a failure of two values
//							which are supposed to compare equal
//
//		Entry:	byExpected		Expected value
//					byActual			Actual value
//					byMask			Mask of don't care bits
//		Exit:		<BYTE>			Bit mask of failed bits
//
BYTE FlagFailure (BYTE byExpected, BYTE byActual, BYTE byMask)
{
	return ((BYTE) ((byExpected & ~byMask) ^ (byActual & ~byMask)));
}

//
//		SetCursorPositionB - Set the current cursor position
//
//		Entry:	col	Text column
//					row	Text row
//		Exit:		None
//
void SetCursorPositionB (BYTE col, BYTE row)
{
	union REGS		regs;

	yTextRow = row;
	xTextCol = col;

	regs.h.ah = 0x02;
	regs.h.dh = row;
	regs.h.dl = col;
	regs.w.bx = 0;
	int386 (0x10, &regs, &regs);
}

//
//		WriteBIOSString - Write a string to the display
//
//		Entry:	col		Column
//					row		Row
//					count		Number of characters in string
//					ptr		Pointer to the string
//					attr		Color of string
//		Exit:		None
//
void WriteBIOSString (BYTE col, BYTE row, BYTE count, BYTE *ptr, BYTE attr)
{
	union REGS		regs;
	REALMODECALL	rmc;
	WORD				wSel, wSeg;

	// Allocate DOS Memory Block
	regs.w.ax = 0x0100;
	regs.w.bx = (WORD) ((count / 16) + 1);
	int386 (0x31, &regs, &regs);
	wSeg = regs.w.ax;
	wSel = regs.w.dx;

	// Copy string to allocated DOS memory
	_fmemcpy (MK_FP (wSel, 0), ptr, count);

	// Write a string
	// AX = 1301, BL = attr, BH = 0, CL = count, CH = 0, DL = col, DH = row
	// ES:BP = string
	rmc.eax = 0x1301;
	rmc.ebx = (DWORD) attr;
	rmc.ecx = (DWORD) count;
	rmc.edx = (DWORD) ((((DWORD) row) << 8) | col);
	rmc.es = wSeg;
	rmc.ebp = 0;
	RealModeINT (&rmc, 0x10);

	// Free allocated DOS block
	regs.w.ax = 0x0101;
	regs.w.dx = wSel;
	int386 (0x31, &regs, &regs);
}

//
//		SetStandardModeInfo - Set necessary mode info in VBEModeInfoBlock
//
//		Entry:	wVMode	Current Video mode
//					lpvbe		Pointer to VBEInfoBlock 
//		Exit:		None
//
void SetStandardModeInfo (WORD wVMode, LPVBEMODEINFOBLOCK pvbemi)
{
	static WORD	tblXRes[] = {40, 40, 80, 80, 320, 320, 640, 80, 0, 0, 0, 0, 0, 320, 640, 640, 640, 640, 640, 320};
	static WORD	tblYRes[] = {25, 25, 25, 25, 200, 200, 200, 25, 0, 0, 0, 0, 0, 200, 200, 350, 350, 480, 480, 200};
	static BYTE	tblBPP[] = {4, 4, 4, 4, 2, 2, 1, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 1, 4, 8};
	static WORD tblMAttr[] = {0x0F, 0x0F, 0x0F, 0x0F, 0x1F, 0x1F, 0x1F, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1F, 0x1F, 0x17, 0x1F, 0x1F, 0x1F, 0x1F};
	static BYTE tblXChar[] = {0x09, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x09, 0x0, 0x0, 0x0, 0x0, 0x0, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08};
	static BYTE	tblYChar[] = {0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x08, 0x08, 0x0E, 0x0E, 0x10, 0x10, 0x08};
	static BYTE	tblMemMdl[] = {0x0, 0x0, 0x0, 0x0, 0x01, 0x01, 0x01, 0x00, 0x0, 0x0, 0x0, 0x0, 0x0, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04};
	static WORD	tblBPScan[] = {80, 80, 160, 160, 80, 80, 80, 160, 0, 0, 0, 0, 0, 40, 80, 80, 80, 80, 80, 320};

	pvbemi->XResolution = tblXRes[wVMode];
	pvbemi->YResolution = tblYRes[wVMode];
	pvbemi->BitsPerPixel = tblBPP[wVMode];
	pvbemi->ModeAttributes = tblMAttr[wVMode];
	pvbemi->XCharSize = tblXChar[wVMode];
	pvbemi->YCharSize = tblYChar[wVMode];
	pvbemi->MemoryModel = tblMemMdl[wVMode];
	pvbemi->BytesPerScanLine = tblBPScan[wVMode];
	pvbemi->WinGranularity = 0x40;						// Assume 64K units
}

//
//		SetTextColor - Set text colors on future character writes
//
//		Entry:	lpfore	Pointer to foreground text color table
//					lpback	Pointer to background text color table
//		Exit:		None
//
void SetTextColor (LPCOLORREF lpfore, LPCOLORREF lpback)
{
	lpclrTextFore = lpfore;
	lpclrTextBack = lpback;
}

//
//		BIOSWriteChar - Write character with attribute
//
//		Entry:	chr		Character
//					attr		Attribute
//		Exit:		None
//
void BIOSWriteChar (BYTE chr, BYTE attr)
{
	union REGS	regs;

	regs.h.ah = 0x09;
	regs.h.al = chr;
	regs.h.bh = 0;
	regs.h.bl = attr;
	regs.w.cx = 1;
	int386 (0x10, &regs, &regs);
}

//
//		FindPCIDev - Find a specific PCI device
//
//		Entry:	wVendorID	Vendor ID to search for
//					wDeviceID	Device ID to search for
//					pbyBusID		Pointer to PCI bus ID for adapter
//					pbyDevID		Pointer to PCI bus device ID for adapter
//		Exit:		<BOOL>		Device found flag (TRUE = Found, FALSE = Not)
//
BOOL FindPCIDev (WORD wVendorID, WORD wDeviceID, LPBYTE pbyBusID, LPBYTE pbyDevID)
{
	REALMODECALL	rmc;

	// Find PCI device
	rmc.eax = 0xB102;
	rmc.ecx = wDeviceID;
	rmc.esi = 0;
	rmc.edx = wVendorID;
	RealModeINT (&rmc, 0x1A);

	// Assign the bus number where device was found
	*pbyBusID = HIBYTE (LOWORD (rmc.ebx));
	*pbyDevID = LOBYTE (LOWORD (rmc.ebx));
	
	if (HIBYTE (LOWORD (rmc.eax)) != 0) 
		return (FALSE);			// Device was not found
	else
		return (TRUE);				// Device was found
}

//
//		ReadPCIDWord - Read a DWORD from specified PCI device
//
//		Entry:	byBusID		PCI Bus ID (from FindPCIDev)
//					byDevID		PCI Device ID (from FindPCIDev)
//					nIndex		Index to read
//					lpdwRetVal	Pointer to store value read
//		Exit:		<BOOL>		Valid read flag (TRUE = Valid, FALSE = Not)
//	
BOOL ReadPCIDWord (BYTE byBusID, BYTE byDevID, BYTE nIndex, DWORD *lpdwRetVal)
{
	REALMODECALL	rmc;

	*lpdwRetVal = 0;
	rmc.eax = 0xB10A;
	rmc.ebx = (byBusID << 8) | byDevID;
	rmc.edi = nIndex;
	RealModeINT (&rmc, 0x1A);

	// Make sure we have a valid response
	if (HIBYTE (LOWORD (rmc.eax)) != 0) 
	 	return (FALSE);

	*lpdwRetVal = rmc.ecx;
	return (TRUE);
}

//
//		ReadPCIWord - Read a WORD from specified PCI device
//
//		Entry:	byBusID		PCI Bus ID (from FindPCIDev)
//					byDevID		PCI Device ID (from FindPCIDev)
//					nIndex		Index to read
//					lpwRetVal	Pointer to store value read
//		Exit:		<BOOL>		Valid read flag (TRUE = Valid, FALSE = Not)
//	
BOOL ReadPCIWord (BYTE byBusID, BYTE byDevID, BYTE nIndex, WORD *lpwRetVal)
{
	REALMODECALL	rmc;

	*lpwRetVal = 0;
	rmc.eax = 0xB109;
	rmc.ebx = (byBusID << 8) | byDevID;
	rmc.edi = nIndex;
	RealModeINT (&rmc, 0x1A);

	// Make sure we have a valid response
	if (HIBYTE (LOWORD (rmc.eax)) != 0) 
	 	return (FALSE);

	*lpwRetVal = LOWORD (rmc.ecx);
	return (TRUE);
}

//
//		ReadPCIByte - Read a BYTE from specified PCI device
//
//		Entry:	byBusID		PCI Bus ID (from FindPCIDev)
//					byDevID		PCI Device ID (from FindPCIDev)
//					nIndex		Index to read
//					lpbyRetVal	Pointer to store value read
//		Exit:		<BOOL>		Valid read flag (TRUE = Valid, FALSE = Not)
//	
BOOL ReadPCIByte (BYTE byBusID, BYTE byDevID, BYTE nIndex, BYTE *lpbyRetVal)
{
	REALMODECALL	rmc;

	*lpbyRetVal = 0;
	rmc.eax = 0xB108;
	rmc.ebx = (byBusID << 8) | byDevID;
	rmc.edi = nIndex;
	RealModeINT (&rmc, 0x1A);

	// Make sure we have a valid response
	if (HIBYTE (LOWORD (rmc.eax)) != 0) 
	 	return (FALSE);

	*lpbyRetVal = LOBYTE (LOWORD (rmc.ecx));
	return (TRUE);
}

//
//		WritePCIDWord - Write a DWORD to a specified PCI device
//
//		Entry:	byBusID		PCI Bus ID (from FindPCIDev)
//					byDevID		PCI Device ID (from FindPCIDev)
//					nIndex		Index to write
//					dwData		Data to write
//		Exit:		<BOOL>		Valid write flag (TRUE = Valid, FALSE = Not)
//	
BOOL WritePCIDWord (BYTE byBusID, BYTE byDevID, BYTE nIndex, DWORD dwData)
{
	REALMODECALL	rmc;

	rmc.eax = 0xB10D;
	rmc.ebx = (byBusID << 8) | byDevID;
	rmc.ecx = dwData;
	rmc.edi = nIndex;
	RealModeINT (&rmc, 0x1A);

	// Make sure we have a valid response
	if (HIBYTE (LOWORD (rmc.eax)) != 0) 
	 	return (FALSE);

	return (TRUE);
}

//
//		WritePCIWord - Write a WORD to a specified PCI device
//
//		Entry:	byBusID		PCI Bus ID (from FindPCIDev)
//					byDevID		PCI Device ID (from FindPCIDev)
//					nIndex		Index to write
//					wData			Data to write
//		Exit:		<BOOL>		Valid write flag (TRUE = Valid, FALSE = Not)
//	
BOOL WritePCIWord (BYTE byBusID, BYTE byDevID, BYTE nIndex, WORD wData)
{
	REALMODECALL	rmc;

	rmc.eax = 0xB10C;
	rmc.ebx = (byBusID << 8) | byDevID;
	rmc.ecx = wData;
	rmc.edi = nIndex;
	RealModeINT (&rmc, 0x1A);

	// Make sure we have a valid response
	if (HIBYTE (LOWORD (rmc.eax)) != 0) 
	 	return (FALSE);

	return (TRUE);
}

//
//		WritePCIByte - Write a BYTE to a specified PCI device
//
//		Entry:	byBusID		PCI Bus ID (from FindPCIDev)
//					byDevID		PCI Device ID (from FindPCIDev)
//					nIndex		Index to write
//					byData		Data to write
//		Exit:		<BOOL>		Valid write flag (TRUE = Valid, FALSE = Not)
//	
BOOL WritePCIByte (BYTE byBusID, BYTE byDevID, BYTE nIndex, BYTE byData)
{
	REALMODECALL	rmc;

	rmc.eax = 0xB10B;
	rmc.ebx = (byBusID << 8) | byDevID;
	rmc.ecx = byData;
	rmc.edi = nIndex;
	RealModeINT (&rmc, 0x1A);

	// Make sure we have a valid response
	if (HIBYTE (LOWORD (rmc.eax)) != 0) 
	 	return (FALSE);

	return (TRUE);
}

//
//		power - Raise a long integer to a given power (base**exp)
//
//		Entry:	base	Base value
//					exp	Exponent
//		Exit:		None
//
unsigned long power (unsigned long base, unsigned long exp)
{
	if (exp == 0)
		return (1);
	else if (exp == 1)
		return (base);
	else
		return (base * power (base, exp - 1));
}

//
//		GetDac - Get values from a specific DAC location
//
//		Entry:	index		Index of external palette
//					lpRed		Pointer to returned red value
//					lpGreen	Pointer to returned green value
//					lpBlue	Pointer to returned blue value
//		Exit:		None
//
void GetDac (BYTE index, LPBYTE lpRed, LPBYTE lpGreen, LPBYTE lpBlue)
{
	_disable ();
	_outp (DAC_RINDEX, index);
	*lpRed = _inp (DAC_DATA);
	*lpGreen = _inp (DAC_DATA);
	*lpBlue = _inp (DAC_DATA);
	_enable ();
}

//
//		SetDac - Set a specific DAC location to a given value
//
//		Entry:	index		Index of external palette
//					red		Red value
//					green		Green value
//					blue		Blue value
//		Exit:		None
//
void SetDac (BYTE index, BYTE red, BYTE green, BYTE blue)
{
	_disable ();
	_outp (DAC_WINDEX, index);
	_outp (DAC_DATA, red);
	_outp (DAC_DATA, green);
	_outp (DAC_DATA, blue);
	_enable ();
}

//
//		FillDAC - Load the entire RAMDAC with a single value
//
//		Entry:	red		Red value
//					green		Green value
//					blue		Blue value
//		Exit:		None
//
void FillDAC (BYTE red, BYTE green, BYTE blue)
{
	int	i;

	_disable ();
	_outp (DAC_WINDEX, 0);			// Assume auto-increment
	for (i = 0; i < 256; i++)
	{
		_outp (DAC_DATA, red);
		_outp (DAC_DATA, green);
		_outp (DAC_DATA, blue);
	}
	_enable ();
}

//
//		FillDACLength - Load a range of RAMDAC registers with a single value
//
//		Entry:	idx		Starting indx
//					length	Number of registers
//					red		Red value
//					green		Green value
//					blue		Blue value
//		Exit:		None
//
void FillDACLength (BYTE idx, WORD length, BYTE red, BYTE green, BYTE blue)
{
	int	i;

	_disable ();
	_outp (DAC_WINDEX, idx);			// Assume auto-increment
	for (i = 0; i < (int) length; i++)
	{
		_outp (DAC_DATA, red);
		_outp (DAC_DATA, green);
		_outp (DAC_DATA, blue);
	}
	_enable ();
}

//
//		SetDacBlock - Set a range of DAC registers
//
//		Entry:	lpDac		Pointer to DAC array
//					start		Starting index
//					count		Number of registers to load
//		Exit:		None
//
void SetDacBlock (LPBYTE lpDac, BYTE start, WORD count)
{
	_disable ();
	_outp (DAC_WINDEX, start);
	while (count--)
	{
		_outp (DAC_DATA, *lpDac++);
		_outp (DAC_DATA, *lpDac++);
		_outp (DAC_DATA, *lpDac++);
	}
	_enable ();
}

//
//		RotateByteRight - Rotate a data byte a given number of times
//
//		Entry:	byData	Data byte
//					byCount	Number of times to rotate data
//		Exit:		<BYTE>	Resultant data
//
BYTE RotateByteRight (BYTE byData, BYTE byCount)
{
	BYTE	i;

	for (i = 0; i < byCount; i++)
		byData = (BYTE) (((byData & 0x01) << 7) | ((BYTE) (byData >> 1)));

	return (byData);
}

//
//		VGASetPixelAt - Set a pixel at a given video offset
//
//		Entry:	lpVMem		Pointer to video memory
//					byMask		Pixel mask
//					byColor		Color of pixel
//		Exit:		None
//
//		Assume:	16-color planar mode
//
void VGASetPixelAt (BYTE __far *lpVMem, BYTE byMask, BYTE byColor)
{
	BYTE	tmp;

	_outp (GDC_INDEX, 0x08);
	_outp (GDC_DATA, byMask);
	_outp (SEQ_INDEX, 0x02);
	_outp (SEQ_DATA, 0x0F);

	tmp = *lpVMem;							// Latch data
	*lpVMem = 0x00;
	_outp (SEQ_DATA, byColor);
	*lpVMem = 0xFF;

	_outp (SEQ_DATA, 0x0F);
	_outp (GDC_DATA, 0xFF);
}

//
//		MoveBytes - Move a block of memory of a given size
//
//		Entry:	lpDest	Pointer to destination
//					lpSrc		Pointer to source
//					nLength	Number of bytes to move
//		Exit:		None
//
//		Note:	Due to the way VGA latches work, only one BYTE at at time
//				can be moved around video memory. Therefore, the "C" routine,
//				"_fmemmove" is useless since it is optimized for speed, and
//				thus does WORD or DWORD moves.
//
void MoveBytes (BYTE __far *lpDest,  BYTE __far *lpSrc, int nLength)
{
	BYTE	byTemp;

	while (nLength--)
	{
		byTemp = *lpSrc++;			// Hopefully, the C optimizer will leave
		*lpDest++ = byTemp;			//  this alone...
	}
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//



//
//		RotateByteLeft - Rotate a data byte a given number of times
//
//		Entry:	byData	Data byte
//					byCount	Number of times to rotate data
//		Exit:		<BYTE>	Resultant data
//
BYTE RotateByteLeft (BYTE byData, BYTE byCount)
{
	BYTE	i;

	for (i = 0; i < byCount; i++)
		byData = (BYTE) (((byData & 0x80) >> 7) | ((BYTE) (byData << 1)));

	return (byData);
}



//
//		MemoryFill - Fill memory with a byte for a given length
//
//		Entry:	lpAddr	Address of memory
//					byData	Data to fill with
//					wLength	Length of data
//		Exit:		None
//
void MemoryFill (SEGOFF lpAddr, BYTE byData, WORD wLength)
{
   //	printf("lpaddr = %x data=%x length=%x", lpAddr, byData, wLength);
	while (wLength--)
		MemByteWrite (lpAddr++, byData);
}

//
//		MemByteRead - Read a byte from a memory location
//
//		Entry:	lpAddr	Memory address
//		Exit:		<BYTE>	Data read
//
BYTE MemByteRead (SEGOFF lpAddr)
{
	BYTE	byData;

		// Do the memory read
			byData = *((LPBYTE) (lpAddr));

	
	return (byData);
}


void MemByteWrite (SEGOFF lpAddr, BYTE byData)
{
	DWORD dwAddress;

	// Convert segment:offset to linear address
	dwAddress = PtrToLin (lpAddr);

	// If this is non-video memory, then pass it thru. Note that this
	// function may be called on reads and writes to low memory due
	// to maintaining the BIOS state.
	
	// 
		if ((dwAddress < 0xA0000)|| (dwAddress >= 0xC0000))

	{
		MemLinWrite (dwAddress, byData);
		return;
	}

 	else
 	{
		MemLinWrite (dwAddress, byData);

	}
 				
 
}

//
//		MemWordWrite - Write a word to a memory location
//
//		Entry:	lpAddr	Memory address
//					wData		Data to write
//		Exit:		None
//
void MemWordWrite (SEGOFF lpAddr, WORD wData)
{
	// Do a memory write
   			*(LPWORD) lpAddr = wData;

   

   
}


 //
//		IOWordWrite - Write a word to a specific I/O port
//
//		Entry:	wPort		Register address
//					wData		Data to write
//		Exit:		None
//
void IOWordWrite (WORD wPort, WORD wData)
{
   
			// Break word write into byte writes
			IOByteWrite (wPort, LOBYTE (wData));
			IOByteWrite (wPort + 1, HIBYTE (wData));
   
}

//
//		IOWordRead - Read a word from a specific I/O port
//
//		Entry:	wPort		Register address
//		Exit:		<WORD>	Register data
//
WORD IOWordRead (WORD wPort)
{
	WORD wData;

   		wData = (IOByteRead (wPort + 1) & 0xFF) << 8;
   		wData |= IOByteRead (wPort) & 0xFF;
   

	return (wData);
}





//
//		MemWordRead - Read a word from a memory location
//
//		Entry:	lpAddr	Memory address in segment:offset format
//		Exit:		<WORD>	Word read from VGA
//
WORD MemWordRead (SEGOFF lpAddr)
{
	DWORD dwAddress;
	WORD	wData;

	// Convert segment:offset to linear address
	dwAddress = PtrToLin (lpAddr);
	//wData = (OPEN_BUS << 8) | OPEN_BUS;

	// If this is non-video memory, then pass it thru. Note that this
	// function may be called on reads and writes to low memory due
	// to maintaining the BIOS state.
	if ((dwAddress < 0xA0000) || (dwAddress >= 0xC0000))
	{
		// Convert word read to two byte reads
		wData = (WORD) MemLinRead (dwAddress);
		wData |= ((WORD) MemLinRead (dwAddress + 1)) << 8;
		return (wData);
	}
		 
			// Convert word read to two byte reads
			wData = (WORD) MemLinRead (dwAddress);
			wData |= ((WORD) MemLinRead (dwAddress + 1)) << 8;
		 	return (wData);
}

//
//		MemDwordWrite - Write a dword to a memory location
//
//		Entry:	lpAddr	Memory address in segment:offset format
//					dwData	DWORD	to write to memory
//		Exit:		None
//
void MemDwordWrite (SEGOFF lpAddr, DWORD dwData)
{
	DWORD dwAddress;

	// Convert segment:offset to linear address
	dwAddress = PtrToLin (lpAddr);

	// If this is non-video memory, then pass it thru. Note that this
	// function may be called on reads and writes to low memory due
	// to maintaining the BIOS state.
	if ((dwAddress < 0xA0000) || (dwAddress >= 0xC0000))
	{
		// Convert dword write to four byte writes
		MemLinWrite (dwAddress + 0, LOBYTE (LOWORD (dwData)));
		MemLinWrite (dwAddress + 1, HIBYTE (LOWORD (dwData)));
		MemLinWrite (dwAddress + 2, LOBYTE (HIWORD (dwData)));
		MemLinWrite (dwAddress + 3, HIBYTE (HIWORD (dwData)));
		return;
	}

		// Convert dword write to four byte writes
		MemLinWrite (dwAddress + 0, LOBYTE (LOWORD (dwData)));
		MemLinWrite (dwAddress + 1, HIBYTE (LOWORD (dwData)));
		MemLinWrite (dwAddress + 2, LOBYTE (HIWORD (dwData)));
		MemLinWrite (dwAddress + 3, HIBYTE (HIWORD (dwData)));

	
	
}

//
//		MemDwordRead - Read a dword from a memory location
//
//		Entry:	lpAddr	Memory address in segment:offset format
//		Exit:		<DWORD>	Dword read from VGA memory
//
DWORD MemDwordRead (SEGOFF lpAddr)
{
	DWORD dwAddress;
	DWORD	dwData;

	// Convert segment:offset to linear address
	dwAddress = PtrToLin (lpAddr);
   //	dwData = (DWORD) ((OPEN_BUS << 24) | (OPEN_BUS << 16) | (OPEN_BUS << 8) | OPEN_BUS);

	// If this is non-video memory, then pass it thru. Note that this
	// function may be called on reads and writes to low memory due
	// to maintaining the BIOS state.
	if ((dwAddress < 0xA0000) || (dwAddress >= 0xC0000))
	{
		// Convert dword read to four byte reads
		dwData = (DWORD) MemLinRead (dwAddress);
		dwData |= ((DWORD) MemLinRead (dwAddress + 1)) << 8;
		dwData |= ((DWORD) MemLinRead (dwAddress + 2)) << 16;
		dwData |= ((DWORD) MemLinRead (dwAddress + 3)) << 24;
		return (dwData);
	}

			// Convert dword read to four byte reads
			dwData = (DWORD) MemLinRead (dwAddress);
			dwData |= ((DWORD) MemLinRead (dwAddress + 1)) << 8;
			dwData |= ((DWORD) MemLinRead (dwAddress + 2)) << 16;
			dwData |= ((DWORD) MemLinRead (dwAddress + 3)) << 24;

		return (dwData);
}


 void SetRegs (LPPARMENTRY lpParms)
{
	int	i;
	WORD	wCRTC;

	// Blank screen during the register changes (will be
	// re-enabled at the end of this routine)
	IOByteRead (INPUT_MSTATUS_1); IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x00);

	// Load the Sequencer and the clock
	IOWordWrite (SEQ_INDEX, 0x0100);		// Sync reset
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (SEQ_INDEX, (BYTE) (i + 1));
		IOByteWrite (SEQ_DATA, lpParms->seq_data[i]);
	}

	IOByteWrite (MISC_OUTPUT, lpParms->misc);
	IOWordWrite (SEQ_INDEX, 0x0300);		// End sync reset

	// Load the CRTC
	wCRTC = CRTC_MINDEX;
	if (lpParms->misc & 1) wCRTC = CRTC_CINDEX;
	IOWordWrite (wCRTC, 0x2011);				// Disable write protection
	for (i = 0; i < 25; i++)
	{
		IOByteWrite (wCRTC, (BYTE) i);
		IOByteWrite ((WORD) (wCRTC + 1), lpParms->crtc_data[i]);
	}

	// Load the GDC
	for (i = 0; i < 9; i++)
	{
		IOByteWrite (GDC_INDEX, (BYTE) i);
		IOByteWrite (GDC_DATA, lpParms->gdc_data[i]);
	}

	// Load the attribute controller
	IOByteRead ((WORD) (wCRTC + 6));
	for (i = 0; i < 20; i++)
	{
		IOByteWrite (ATC_INDEX, (BYTE) i);
		IOByteWrite (ATC_INDEX, lpParms->atc_data[i]);
	}
	IOByteWrite (ATC_INDEX, 0x34);			// VGA register not in parm table (enable
	IOByteWrite (ATC_INDEX, 0x00);			// CRTC palette access, as well)

	IOByteWrite (wCRTC + 6, 0);				// Clear VSYNC select
}




//
//		WaitVerticalRetrace - Wait for the beginning of vertical retrace
//
//		Entry:	None
//		Exit:		<BOOL>	Retrace status (TRUE = No error, FALSE = Timed out)
//
//		Note:	Since this is an event that happens several (60 to 70) times
//				per second, one second should be sufficient to verify that
//				it is occurring regularly. Therefore, if no retrace occurs
//				within one second, flag is as an error and return.
//
BOOL WaitVerticalRetrace (void)
{
	DWORD	time0, time1;
	WORD	wStatus, wSimType;
	BOOL	bTimeout, bDumpIO, bDumpMem, bDumpDAC;
	// Useful timer tick values (18.2065 timer ticks per second)
#define	FIVE_SECONDS			91
#define	TWO_SECONDS				36
#define	ONE_SECOND				18

	// If vectors are being generated, then only generate a single
	// I/O read (from the Input Status register) into the test vector.
	// Furthermore, if in simulation mode while generating vectors,
	// generate one read and exit. Disabling I/O capture will cause the
	// simulator to return OPEN_BUS when in SIM_TURBOACCESS configuraiton.
  
  	bTimeout = TRUE;
	wStatus = INPUT_MSTATUS_1;
	if (IOByteRead (MISC_INPUT) & 0x01) wStatus = INPUT_CSTATUS_1;

	time0 = GetSystemTicks ();
	time1 = time0 + ONE_SECOND;		// If not status change within 1 sec, exit
	// Wait while in retrace
	while (time0 < time1)
	{
		if ((IOByteRead (wStatus) & 0x08) == 0)
		{
			bTimeout = FALSE;
			break;
		}
		time0 = GetSystemTicks ();
	}
	if (bTimeout) goto WaitVerticalRetrace_exit;

	// Wait while not retrace
	bTimeout = TRUE;
	while (time0 < time1)
	{
		if ((IOByteRead (wStatus) & 0x08) == 0x08)
		{
			bTimeout = FALSE;
			break;
		}
		time0 = GetSystemTicks ();
	}

WaitVerticalRetrace_exit:
 	return (!bTimeout);
}


//
//		GetSystemTicks - Get the current timer tick count
//
//		Entry:	None
//		Exit:		<DWORD>	Current timer tick
//
DWORD GetSystemTicks (void)
{
	static SEGOFF	lpTicks = (SEGOFF) 0x0000046C;	// Timer tick BIOS variable
	static DWORD	dwTicks = 0;
	
	return (MemDwordRead ((SEGOFF) lpTicks));
}



//
//		SimGetType - Get the simulation flags
//
//		Entry:	None
//		Exit:		<BYTE>	VGA type
//
WORD SimGetType (void)
{
	return (_wSimType);
}


//
//		SimSetState - Set the simulation into "dump" mode
//
//		Entry:	bDumpIO		State of I/O simulation
//					bDumpMem		State of Memory simulation
//					bDumpDAC		State of DAC simulation
//		Exit:		None
//
void SimSetState (BOOL bDumpIO, BOOL bDumpMem, BOOL bDumpDAC)
{
	_bSimDumpIO = bDumpIO;
	_bSimDumpMem = bDumpMem;
	_bSimDumpDAC = bDumpDAC;
}
//
//		SimDumpMemory - Dump video memory
//
//		Entry:	szFilename		Memory image filename
//		Exit:		None
//
void SimDumpMemory (LPSTR szFilename)
{
	if (_wSimType & SIM_VECTORS)
		VectorDumpMemory (szFilename);
}



//
//		FlagError - Set global error variables and return DOS ERRORLEVEL value
//
//		Entry:	err		Error code (required)
//					part		Chapter of VGA Core Test Specification (required)
//					test		Test # of VGA Core Test Specification (required)
//					address	Address of failure (if appropriate)
//					index		Index of I/O register (if appropriate)
//					expected	Expected data (if appropriate)
//					actual	Actual data (if appropriate)
//		Exit:		<int>		DOS ERRORLEVEL code
//
int FlagError (int err, int part, int test, DWORD address, WORD index, DWORD expected, DWORD actual)
{
	printf("error #	= %d part=%x test=%x addr=%x index=%x exp=%x act=%x\n", err,part,test,address,index, expected,actual);

	_vcerr_code = err;
	_vcerr_part = part;
	_vcerr_test = test;
	_vcerr_address = address;
	_vcerr_index = index;
	_vcerr_expected = expected;
	_vcerr_actual = actual;

	return (err);
}

//
//		SimSetFrameSize - Allow override of the standard VGA mode table
//
//		Entry:	bSize		Size flag (TRUE = Large, FALSE = Small frame)
//		Exit:		<BOOL>	Previous size flag
//
BOOL SimSetFrameSize (BOOL bSize)
{
	BOOL	bTemp;

	bTemp = !(_wSimType & SIM_SMALLFRAME);
	_wSimType = _wSimType & ~SIM_SMALLFRAME;
	_wSimType |= bSize ? SIM_STDFRAME : SIM_SMALLFRAME;

	return (bTemp);
}


//
//		SystemCleanUp - Clean up and prepare for exit from the test
//
//		Entry:	None
//		Exit:		None
//
void SystemCleanUp (void)
{
	//
	// On physical systems, restore the state of the machine such that
	// the user can return to the DOS prompt. Note that since vectors
	// may or may not be generated on a physical system, only the
	// "SIM_SIMULATION" flag is used to determine whether to clean up
	// or not. Situations in which vectors are being generated on a
	// physical device need to do a final "SetMode" for the physical
	// without doing one for the test vectors.
	//
	if (!(SimGetType () & SIM_SIMULATION))
	{
		SimSetFrameSize (TRUE);
		SimSetState (FALSE, FALSE, FALSE);
		SetScans (400);
		SetMode (0x03);
	}
}



//
//		SetScans - Set the number of scan lines for text modes. This setting
//						will take place on the next mode set.
//
//		Entry:	wScan		Number of scans (200, 350, 400)
//		Exit:		None
//
void SetScans (WORD wScan)
{
	static SEGOFF	lpVGAInfo = (SEGOFF) 0x00000489;
	BYTE				mask;

	switch (wScan)
	{
		case 200:
			mask = 0x80;
			break;

		case 350:
			mask = 0x00;
			break;

		case 400:
		default:
			mask = 0x10;
			break;
	}

	MemByteWrite (lpVGAInfo, (BYTE) ((MemByteRead (lpVGAInfo) & 0x6F) | mask));
}



void VectorDumpMemory (LPSTR szFilename)
{
int dummy;
	dummy=dummy;

}

//		FrameCapture - Get a frame or return "no-support" flag
//
//		Entry:	nPart		Test section number
//					nTest		Test number within section
//		Exit:		<BOOL>	Support flag (TRUE = Supported, FALSE = Capturing unsupported)
//
BOOL FrameCapture (int nPart, int nTest)
{
	static int	nCount = 0;
	char			szFilename[32];

	// stub out for diag purposes
	// Check to see if a key is hit. If a key is hit and that key is
	// an "<ESC>" key, then abort the frame capture.
  //	if (_kbhit ())
  //	{
		// Look at the ASCII part of the key (extended ASCII cannot be
		// "peeked" without removing them from the buffer)
  //		if (PeekKey () == KEY_ESCAPE)
  //			return (FALSE);
  //	}

	if (nCount < 256)
	{
		sprintf (szFilename, "FR%02d%02d%02X.BMP", nPart, nTest, nCount++);
		return (CaptureFrame (szFilename) == 0);
	}
	else
		return (FALSE);
}


//
//		EndCapture - Terminate a stream of frame captures
//
//		Entry:	None
//		Exit:		None
//
void EndCapture (void)
{
	WORD	wCRTC;

	if (_bCaptureStream)
	{
		if ((SimGetType () & SIM_SIMULATION))
		{
			wCRTC = CRTC_MINDEX;
			if (IOByteRead (MISC_INPUT) & 0x01) wCRTC = CRTC_CINDEX;
			IOByteWrite (wCRTC, 0x0A);
			IOByteWrite (wCRTC + 1, IOByteRead (wCRTC + 1) & 0xDF);
		}
		SimEndCapture ();
	}
	_bCaptureStream = FALSE;
}


//
//		VerifyBytes - Scan a block of memory for a non-conforming value
//
//		Entry:	lpBuffer		Pointer to memory
//					chr			Expected data
//					nLength		Number of bytes to scan
//		Exit:		<LPBYTE>		Pointer to failing byte
//									(Return NULL if successful)
//
SEGOFF VerifyBytes (SEGOFF lpBuffer, BYTE chr, int nLength)
{
	WORD	hiword, loword;
	int	i;

	hiword = loword = 0;
	for (i = 0; i < nLength; i++)
	{
		if (MemByteRead (lpBuffer) != chr)
		{
			hiword = HIWORD (lpBuffer);
			loword = LOWORD (lpBuffer);
			break;
		}
		lpBuffer++;
	}

	return ((SEGOFF) ((((DWORD) (hiword)) << 16) | loword));
}


//
//		SimSetCaptureMode - Set frame grab mode
//
//		Entry:	byMode	Capture mode:
//									CAP_SCREEN
//									CAP_OVERSCAN
//									CAP_COMPOSITE
//		Exit:		None
//
void SimSetCaptureMode (BYTE byMode)
{
}



//
//		VGACharOut - Write a character at a specific location in 256 color mode
//
//		Entry:	chr		ASCII character code
//					attr		color
//					col		Column
//					row		Row
//					page		Page (must be 0)
//		Exit:		None
//
//		Assume character height is an even number.
//
void VGACharOut (BYTE chr, BYTE color, BYTE col, BYTE row, BYTE page)
{
	static SEGOFF	lpColumns = (SEGOFF) 0x0000044A;
	static SEGOFF	lpINT43 = (SEGOFF) 0x0000010C;
	static SEGOFF	lpCharHeight = (SEGOFF) 0x00000485;
	SEGOFF			lpVideo;
	LPBYTE			lpFont;
	WORD				offset, height, next;
	BYTE				byTemp, byCount;

	SetCursorPosition (col, row, page);
	lpVideo = (SEGOFF) (0xA0000000);
	next = MemWordRead (lpColumns) * 8;
	height = (WORD) MemByteRead (lpCharHeight);
	offset = (next * (WORD) row * height) + (col * 8);
	lpFont = (LPBYTE) MemDwordRead (lpINT43);
	lpFont += height * (WORD) chr;
	next -= 8;
	lpVideo += offset;
	while (height--)
	{
		byTemp = *lpFont++;
		byCount = 8;
		while (byCount--)
		{
			if ((byTemp & 0x80) == 0x80)
				MemByteWrite (lpVideo++, color);
			else
				MemByteWrite (lpVideo++, 0);
			byTemp = byTemp << 1;
		}
		lpVideo += next;
	}
}


 //
//		PlanarCharOut - Write a character at a specific location in graphics mode
//
//		Entry:	chr		ASCII character code
//					color		Color
//					col		Column
//					row		Row
//					page		Page
//		Exit:		None
//
void PlanarCharOut (BYTE chr, BYTE color, BYTE col, BYTE row, BYTE page)
{
	static SEGOFF	lpColumns = (SEGOFF) 0x0000044A;
	static SEGOFF	lpRegenLength = (SEGOFF) 0x0000044C;
	static SEGOFF	lpINT43 = (SEGOFF) 0x0000010C;
	static SEGOFF	lpCharHeight = (SEGOFF) 0x00000485;
	SEGOFF			lpVideo;
	WORD				offset, height, next;
	LPBYTE			lpFont;

	SetCursorPosition (col, row, page);
	lpVideo = (SEGOFF) (0xA0000000);
	next = MemWordRead (lpColumns);
	height = (WORD) MemByteRead (lpCharHeight);
	offset = ((next * (WORD) row * height) + (WORD) col) +
				(((WORD) page) * MemWordRead (lpRegenLength));
	lpVideo += offset;
	lpFont = (LPBYTE) MemDwordRead (lpINT43);
	lpFont += height * (WORD) chr;

	IOWordWrite (GDC_INDEX, 0xFF08);				// Bit mask = all enabled
	IOWordWrite (GDC_INDEX, 0x0005);				// Write mode 0
	IOWordWrite (GDC_INDEX, 0x0001);				// Disable set/reset stuff
	IOWordWrite (GDC_INDEX, 0x0000);
	IOByteWrite (SEQ_INDEX, 0x02);					// Set sequencer index
	while (height-- != 0)
	{
		IOByteWrite (SEQ_DATA, 0x0F);					// Enable all planes
		MemByteWrite (lpVideo, 0x00);
		IOByteWrite (SEQ_DATA, color);				// Enable some planes
		MemByteWrite (lpVideo, *lpFont++);
		lpVideo += next;
	}
	IOWordWrite (SEQ_INDEX, 0x0F02);				// Enable all planes
}

//
//		CGA4CharOut - Write a character at a specific location in CGA 4-color
//
//		Entry:	chr		ASCII character code
//					attr		color
//					col		Column
//					row		Row
//					page		Page (must be 0)
//		Exit:		None
//
//		Assume character height is an even number.
//
void CGA4CharOut (BYTE chr, BYTE color, BYTE col, BYTE row, BYTE page)
{
	static SEGOFF	lpColumns = (SEGOFF) 0x0000044A;
	static SEGOFF	lpINT43 = (SEGOFF) 0x0000010C;
	static SEGOFF	lpCharHeight = (SEGOFF) 0x00000485;
	SEGOFF			lpVideo;
	LPBYTE			lpFont;
	WORD				offset, height, next, wTemp;
	BYTE				byTemp, byCount;

	color &= 0x03;
	SetCursorPosition (col, row, page);
	lpVideo = (SEGOFF) (0xB8000000);
	next = MemWordRead (lpColumns) * 2;
	height = (WORD) MemByteRead (lpCharHeight);
	offset = (next * (WORD) row * height) / 2 + (col * 2);
	lpFont = (LPBYTE) MemDwordRead (lpINT43);
	lpFont += height * (WORD) chr;
	while (height--)
	{
		// Build the font word
		byTemp = *lpFont++;
		wTemp = 0;
		byCount = 8;
		while (byCount--)
		{
			if (byTemp & 1) wTemp |= color;
			byTemp = byTemp >> 1;
			wTemp = RotateWordRight (wTemp, 2);
		}

		// Swap the low-order and high-order bytes in the word
		wTemp = RotateWordRight (wTemp, 8);

		MemWordWrite (lpVideo + offset, wTemp);
		offset = (offset ^ 0x2000) + (next * (1 - (height & 0x01)));
	}
}

//
//		CGA2CharOut - Write a character at a specific location in CGA 2-color
//
//		Entry:	chr		ASCII character code
//					color		color
//					col		Column
//					row		Row
//					page		Page (must be 0)
//		Exit:		None
//
//		Assume character height is an even number.
//
void CGA2CharOut (BYTE chr, BYTE color, BYTE col, BYTE row, BYTE page)
{
	static SEGOFF	lpColumns = (SEGOFF) 0x0000044A;
	static SEGOFF	lpINT43 = (SEGOFF) 0x0000010C;
	static SEGOFF	lpCharHeight = (SEGOFF) 0x00000485;
	SEGOFF			lpVideo;
	LPBYTE			lpFont;
	WORD				offset, height, next;

	// Prevent compiler warning
	color = color;							// Color is always "1"

	SetCursorPosition (col, row, page);
	lpVideo = (SEGOFF) (0xB8000000);
	next = MemWordRead (lpColumns);
	height = (WORD) MemByteRead (lpCharHeight);
	offset = (next * (WORD) row * height) / 2 + col;
	lpFont = (LPBYTE) MemDwordRead (lpINT43);
	lpFont += height * (WORD) chr;
	while (height--)
	{
		MemByteWrite (lpVideo + offset, *(lpFont++));
		offset = (offset ^ 0x2000) + (next * (1 - (height & 0x01)));
	}
}


//
//		WaitAttrBlink - Wait for the attribute blink state to change
//
//		Entry:	wState	Attribute state to wait for:
//									BLINK_OFF
//									BLINK_ON
//		Exit:		None
//
void WaitAttrBlink (WORD wState)
{
   // stub ... vectorwaitattrblink in vgasim.c is just stubbed anyway
   //	if (_wSimType & SIM_VECTORS)
   //		VectorWaitAttrBlink (wState);
}



//
//		TextCharOut - Write a character attribute at a specific location
//
//		Entry:	chr		ASCII character code
//					attr		Attribute
//					col		Column
//					row		Row
//					page		Page
//		Exit:		None
//
void TextCharOut (BYTE chr, BYTE attr, BYTE col, BYTE row, BYTE page)
{
	static SEGOFF	lpColumns = (SEGOFF) 0x0000044A;
	static SEGOFF	lpRegenLength = (SEGOFF) 0x0000044C;
	static SEGOFF	lpMode = (SEGOFF) 0x00000449;
	SEGOFF			lpVideo;
	WORD				offset;

	SetCursorPosition (col, row, page);
	lpVideo = (SEGOFF) (0xB8000000);
	if (MemByteRead (lpMode) == 0x07) lpVideo = (SEGOFF) (0xB0000000);
	offset = (((MemWordRead (lpColumns) * row) + col) * 2) +
				(page * MemWordRead (lpRegenLength));
	MemByteWrite (lpVideo + offset, chr);
	MemByteWrite (lpVideo + offset + 1, attr);
}




//
//		WaitCursorBlink - Wait for the cursor blink state to change
//
//		Entry:	wState	Cursor state to wait for:
//									BLINK_ON
//									BLINK_OFF
//		Exit:		None
//
void WaitCursorBlink (WORD wState)
{
	// stub ... vectorwaitcursorblink is just a stub anyway
	//if (_wSimType & SIM_VECTORS)
	//	VectorWaitCursorBlink (wState);
}

//
//		SimSetType - Set simulation flags
//
//		Entry:	byType	VGA type:
//									SIM_ADAPTER or SIM_MOTHERBOARD
//									SIM_SIMULATION or SIM_PHYSICAL
//									SIM_VECTORS or SIM_NOVECTORS
//		Exit:		<WORD>	Previous simulation flags
//
WORD SimSetType (WORD wType)
{
	WORD	wTemp;

	wTemp = _wSimType;
	_wSimType = wType;
	return (wTemp);
}


//
//		GetResolution - Return the current resolution (in pixels)
//
//		Entry:	lpwXRes	Pointer to X resolution
//					lpwYRes	Pointer to Y resolution
//		Exit:		None
//
void GetResolution (LPWORD lpwXRes, LPWORD lpwYRes)
{
	WORD	wCRTC, wXRes, wYRes, wCharWidth;
	BYTE	byATCMode;

	wCRTC = CRTC_MINDEX;
	if (IOByteRead (MISC_INPUT) & 1) wCRTC = CRTC_CINDEX;

	IOByteWrite (SEQ_INDEX, 0x01);
	wCharWidth = 9 - (IOByteRead (SEQ_DATA) & 0x01);
	IOByteWrite (wCRTC, 0x01);
	wXRes = (IOByteRead ((WORD) (wCRTC + 1)) + 1) * wCharWidth;

	IOByteWrite (wCRTC, 0x07);
	wYRes = IOByteRead ((WORD) (wCRTC + 1));
	wYRes = ((wYRes & 0x02) << 7) | ((wYRes & 0x40) << 3);
	IOByteWrite (wCRTC, 0x12);
	wYRes |= IOByteRead ((WORD) (wCRTC + 1));
	wYRes++;

	// Determine if pixels are double pumped
	IOByteRead (wCRTC + 6);
	IOByteWrite (ATC_INDEX, 0x30);
	byATCMode = IOByteRead (ATC_RDATA);
	if ((byATCMode & 0x40) == 0x40)
		wXRes = wXRes / 2;

	// Determine if rows are double pumped
	IOByteWrite (wCRTC, 9);
	if ((byATCMode & 0x01) == 0x01)
		wYRes = wYRes >> (IOByteRead (wCRTC + 1) & 0x1F);

	*lpwXRes = wXRes;
	*lpwYRes = wYRes;
}


//
//		TextStringOut - Write a character string at a specific location
//
//		Entry:	lpsz		Pointer to string
//					attr		Attribute
//					col		Column
//					row		Row
//					page		Page
//		Exit:		None
//
void TextStringOut (LPSTR lpsz, BYTE attr, BYTE col, BYTE row, BYTE page)
{
	static SEGOFF	lpColumns = (SEGOFF) 0x0000044A;
	static SEGOFF	lpRegenLength = (SEGOFF) 0x0000044C;
	static SEGOFF	lpMode = (SEGOFF) 0x00000449;
	SEGOFF			lpVideo;
	WORD				offset;
	int				n;

	lpVideo = (SEGOFF) (0xB8000000);
	if (MemByteRead (lpMode) == 0x07) lpVideo = (SEGOFF) (0xB0000000);
	offset = (((MemWordRead (lpColumns) * row) + col) * 2) +
				(page * MemWordRead (lpRegenLength));
	lpVideo += offset;
	n = StringLength (lpsz);
	SetCursorPosition ((BYTE) (col + n), row, page);
	while (n-- != 0)
	{
		MemByteWrite (lpVideo++, *lpsz++);
		MemByteWrite (lpVideo++, attr);
	}
}



//
//		SimGetKey - Get a key from the simulated environment
//
//		Entry:	None
//		Exit:		<WORD>	Scan code read
//
//		Since this is a physical hardware test, a real "GetKey"
//		needs to be implemented. A simulation, though, would
//		just return a "NULL" with no waiting.
//
WORD SimGetKey (void) // stub out for diags
{
//#ifdef __MSVC16__
//	union	REGS	inregs, outregs;

//	inregs.h.ah = 0;
//	int86 (0x16, &inregs, &outregs);

//	if (outregs.h.al == 0)
//		return (outregs.x.ax);
//	else
//		return (outregs.x.ax & 0x00FF);
//#else
//	int	chr;
//	chr = _getch ();
//	if ((chr == 0) || (chr == 0xE0))
//		chr = _getch () << 8;
//
//	return ((WORD) chr);
//#endif
}


//
//		RWMemoryTest - Test a byte of video memory
//
//		Entry:	wSegment		Video memory segment
//					wOffset		Video memory offset
//		Exit:		<BOOL>		Success flag (TRUE = Memory OK, FALSE = Not)
//
//		Assume planar mode
//
BOOL RWMemoryTest (WORD wSegment, WORD wOffset)
{
	SEGOFF		lpVideo;
	int			i, j;
	BYTE			byOrgMask, byOrgPlane, byOrgData, byActual;
	static BYTE	bytblTestValues[] = {0x00, 0xFF, 0xAA, 0x55};
	BOOL			bPassed;

	// Convert segment:offset to a far pointer
	lpVideo = (SEGOFF) ((((DWORD) wSegment) << 16) + wOffset);

	IOByteWrite (SEQ_INDEX, 0x02);
	byOrgMask = (BYTE) IOByteRead (SEQ_DATA);
	IOByteWrite (GDC_INDEX, 0x04);
	byOrgPlane = (BYTE) IOByteRead (GDC_DATA);
	bPassed = TRUE;
	for (i = 0; i < 4; i++)
	{
		IOByteWrite (SEQ_DATA, (BYTE) (1 << i));		// Set map mask (index was preset before loop)
		IOByteWrite (GDC_DATA, (BYTE) i);				// Set read plane select (ditto on index)
		byOrgData = MemByteRead (lpVideo);				// Save original data

		for (j = 0; j < sizeof (bytblTestValues); j++)
		{
			MemByteWrite (lpVideo, bytblTestValues[j]);
			if (!MemByteTest (lpVideo, bytblTestValues[j], &byActual))
			{
				i = 5;											// Set outer loop counter past end
				bPassed = FALSE;								// Flag error
				break;
			}
		}

		MemByteWrite (lpVideo, byOrgData);				// Restore original data
	}
	IOByteWrite (GDC_DATA, byOrgPlane);
	IOByteWrite (SEQ_DATA, byOrgMask);

	return (bPassed);
}

 //
//		DisableCursor - Turn off the text mode cursor
//
//		Entry:	None
//		Exit:		None
//
//		Note:	This is an "absolute" function. In other words, it can't be
//				nested, because it will always disable the cursor.
//
void DisableCursor (void)
{
	WORD	wCRTC;

	wCRTC = CRTC_MINDEX;
	if (IOByteRead (MISC_INPUT) & 1) wCRTC = CRTC_CINDEX;

	IOByteWrite (wCRTC, 0x0A);
	IOByteWrite ((WORD) (wCRTC + 1), (BYTE) (IOByteRead ((WORD) (wCRTC + 1)) | 0x20));
}


//
//		GetFrameRate - Get current frame rate
//
//		Entry:	None
//		Exit:		<DWORD>	Frame rate in hertz*1000
//
DWORD GetFrameRate (void)
{
	DWORD	time0, time1, rate;
	long	counter;

	if (!WaitVerticalRetrace ())
		return (0);

	// Wait a given number of retraces and time it
	time0 = GetSystemTicks ();
	for (counter = 0; counter < 300; counter++)
		WaitVerticalRetrace ();
	time1 = GetSystemTicks ();

	rate = (DWORD) ((counter * 100 * 182) / (time1 - time0));

	return (rate);
}


 //
//		SetLineColumns - Set row offset for the line routines
//
//		Entry:	columns
//		Exit:		None
//
void SetLine4Columns (WORD columns)
{
	_line_columns = columns;
}

//
//		Line4 - Draw a line from pt A to pt B with a given color in 4-color mode
//
//		Entry:	x1			Starting X coordinate
//					y1			Starting Y coordinate
//					x2			Ending X coordinate
//					y2			Ending Y coordinate
//					color		Line color
//		Exit:		None
//
void Line4 (WORD x1, WORD y1, WORD x2, WORD y2, BYTE color)
{
	WORD		errx, erry, major, count;
	int		delta_x, delta_y, next;
	DWORD		dwOffset;
	SEGOFF	lpVideo;
	BYTE		byShifter, byMask;

	_line_color = color;
	IOWordWrite (0x3C4, 0x0F02);						// Enable all planes
	IOWordWrite (0x3CE, 0x0000);						// Clear set/reset
	IOByteWrite (0x3CE, 0x01);
	IOByteWrite (0x3CF, (BYTE) (~_line_color));	// Set enable set/reset to current color
	IOByteWrite (0x3CE, 0x03);
	IOByteWrite (0x3CF, (BYTE) (_line_rop << 3));// Set rasterop
	IOWordWrite (0x3CE, 0x0005);						// Write mode 0

	if (x1 == x2)									// Vertical line
	{
		VLine4Internal (x1, y1, y2);
	}
	else if (y1 == y2)							// Horizontal line
	{
		HLine4Internal (x1, y2, x2);
	}
	else
	{
		if (x1 > x2)								// Limit drawing to one quadrant
		{
			SwapWords (&x1, &x2);
			SwapWords (&y1, &y2);
		}
		errx = 0;
		erry = 0;
		delta_x = x2 - x1;
		dwOffset = CalcStart4 (x1, y1, &byShifter);
		lpVideo = (SEGOFF) (0xA0000000) + dwOffset;
		next = (BYTE) _line_columns;
		delta_y = y2 - y1;
		if (delta_y < 0)
		{
			delta_y = -delta_y;
			next = -next;
		}
		major = __max (delta_x, delta_y);
		count = major + 1;
		byMask = 0x80 >> byShifter;
		while (count--)
		{
			IOByteWrite (0x3CE, 0x08);
			IOByteWrite (0x3CF, byMask);
			MemByteRead (lpVideo);
			MemByteWrite (lpVideo, 0xFF);
			errx += delta_x;
			erry += delta_y;
			if (errx >= major)
			{
				errx -= major;
				byMask = RotateByteRight (byMask, 1);
				if (++byShifter >= 8)
				{
					byShifter = 0;
					lpVideo++;
				}
			}
			if (erry >= major)
			{
				erry -= major;
				lpVideo += next;
			}
		}
	}
}




//
//		GetNonVideoData - Return the video status register value during
//								non-display time (overscan data is returned)
//
//		Entry:	None
//		Exit:		<BYTE>	Current video data
//
BYTE GetNonVideoData (void)
{
	BYTE	index, cp_enab, p0, p1, p2, p3;
	BYTE	video;

	WaitVerticalRetrace ();

	_disable ();
	index = (BYTE) IOByteRead (ATC_INDEX);
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x32);
	cp_enab = (BYTE) IOByteRead (ATC_RDATA);
	IOByteWrite (ATC_INDEX, 0x0F);
	p0 = (BYTE) IOByteRead (INPUT_CSTATUS_1);

	IOByteWrite (ATC_INDEX, 0x32);
	IOByteWrite (ATC_INDEX, 0x1F);
	p1 = (BYTE) IOByteRead (INPUT_CSTATUS_1);

	IOByteWrite (ATC_INDEX, 0x32);
	IOByteWrite (ATC_INDEX, 0x2F);
	p2 = (BYTE) IOByteRead (INPUT_CSTATUS_1);

	IOByteWrite (ATC_INDEX, 0x32);
	IOByteWrite (ATC_INDEX, 0x3F);
	p3 = (BYTE) IOByteRead (INPUT_CSTATUS_1);
	_enable ();

	// Build the video byte
	video = (BYTE) (((p0 & 0x10) >> 4) |	// Bit 0
				((p2 & 0x10) >> 3) |				// Bit 1
				((p0 & 0x20) >> 3) |				// Bit 2
				((p2 & 0x20) >> 2) |				// Bit 3
				(p1 & 0x30) |						// Bits 4 & 5
				((p3 & 0x30) << 2));				// Bits 6 & 7

	IOByteWrite (ATC_INDEX, 0x32);					// Restore color plane enable
	IOByteWrite (ATC_INDEX, cp_enab);
	IOByteWrite (ATC_INDEX, index);					// Restore index

	return (video);
}

//
//		GetVideoData - Return the video status register value
//
//		Entry:	None
//		Exit:		<BYTE>	Current video data
//
//		Note:	Assume that the entire screen is one uniform color,
//				including overscan.
//
BYTE GetVideoData (void)
{
	BYTE	index, cp_enab, p0, p1, p2, p3;
	BYTE	video;

	index = (BYTE) IOByteRead (ATC_INDEX);
	IOByteRead (INPUT_CSTATUS_1);
	IOByteWrite (ATC_INDEX, 0x32);
	cp_enab = (BYTE) IOByteRead (ATC_RDATA);
	IOByteWrite (ATC_INDEX, 0x0F);
	while ((p0 = (BYTE) IOByteRead (INPUT_CSTATUS_1)) & 1);		// Wait for display time

	IOByteWrite (ATC_INDEX, 0x32);
	IOByteWrite (ATC_INDEX, 0x1F);
	while ((p1 = (BYTE) IOByteRead (INPUT_CSTATUS_1)) & 1);		// Wait for display time

	IOByteWrite (ATC_INDEX, 0x32);
	IOByteWrite (ATC_INDEX, 0x2F);
	while ((p2 = (BYTE) IOByteRead (INPUT_CSTATUS_1)) & 1);		// Wait for display time

	IOByteWrite (ATC_INDEX, 0x32);
	IOByteWrite (ATC_INDEX, 0x3F);
	while ((p3 = (BYTE) IOByteRead (INPUT_CSTATUS_1)) & 1);		// Wait for display time

	// Build the video byte
	video =	(BYTE) (((p0 & 0x10) >> 4) |	// Bit 0
				((p2 & 0x10) >> 3) |				// Bit 1
				((p0 & 0x20) >> 3) |				// Bit 2
				((p2 & 0x20) >> 2) |				// Bit 3
				(p1 & 0x30) |						// Bits 4 & 5
				((p3 & 0x30) << 2));				// Bits 6 & 7

	IOByteWrite (ATC_INDEX, 0x32);					// Restore color plane enable
	IOByteWrite (ATC_INDEX, cp_enab);
	IOByteWrite (ATC_INDEX, index);					// Restore index

	return (video);
}

//
//		SetPixelAt - Set a pixel at a given video offset
//
//		Entry:	lpVMem		Pointer to video memory
//					byMask		Pixel mask
//					byColor		Color of pixel
//		Exit:		None
//
//		Assume:	16-color planar mode
//
void SetPixelAt (SEGOFF lpVMem, BYTE byMask, BYTE byColor)
{
	BYTE	tmp;

	IOByteWrite (GDC_INDEX, 0x08);
	IOByteWrite (GDC_DATA, byMask);
	IOByteWrite (SEQ_INDEX, 0x02);
	IOByteWrite (SEQ_DATA, 0x0F);

	tmp = MemByteRead (lpVMem);				// Latch data
	MemByteWrite (lpVMem, 0x00);
	IOByteWrite (SEQ_DATA, byColor);
	MemByteWrite (lpVMem, 0xFF);

	IOByteWrite (SEQ_DATA, 0x0F);
	IOByteWrite (GDC_DATA, 0xFF);
}



//
//		DrawMonoChar - Draw a 1 bit per pixel graphics character into video memory
//
//		Entry:	col		Column position
//					row		Row position
//					chr		ASCII character
//					lpFont	Pointer to font table
//					height	Character height
//					rowoff	Row offset
//		Exit:		None
//
void DrawMonoChar (WORD col, WORD row, BYTE chr, LPBYTE lpFont, WORD height, WORD rowoff)
{
	SEGOFF	lpVideo;

	lpVideo = (SEGOFF) 0xA0000000;
	lpVideo += (row * rowoff * height) + col;
	lpFont += chr * height;
	while (height--)
	{
		MemByteWrite (lpVideo, *lpFont++);
		lpVideo += rowoff;
	}
}



//
//		WaitNotVerticalRetrace - Wait for the end of vertical retrace
//
//		Entry:	None
//		Exit:		<BOOL>	Retrace status (TRUE = No error, FALSE = Timed out)
//
//		Note:	Since this is an event that happens several (60 to 70) times
//				per second, one second should be sufficient to verify that
//				it is occurring regularly. Therefore, if no retrace occurs
//				within one second, flag is as an error and return.
//
BOOL WaitNotVerticalRetrace (void)
{
	DWORD	time0, time1;
	WORD	wStatus, wSimType;
	BOOL	bTimeout, bDumpIO, bDumpMem, bDumpDAC;

	// If vectors are being generated, then only generate a single
	// I/O read (from the Input Status register) into the test vector.
	// Furthermore, if in simulation mode while generating vectors,
	// generate one read and exit. Disabling I/O capture will cause the
	// simulator to return OPEN_BUS when in SIM_TURBOACCESS configuraiton.
	wSimType = SimGetType ();
	if (wSimType & SIM_VECTORS)
	{
		if (wSimType & SIM_SIMULATION)
		{
			IOByteRead (wStatus);							// Single I/O read
			return (TRUE);										//  ...and exit
		}
		SimGetState (&bDumpIO, &bDumpMem, &bDumpDAC);
		SimSetState (FALSE, bDumpMem, bDumpDAC);		// Stop capturing I/O
	}

	bTimeout = TRUE;
	wStatus = INPUT_MSTATUS_1;
	if (IOByteRead (MISC_INPUT) & 0x01) wStatus = INPUT_CSTATUS_1;

	time0 = GetSystemTicks ();
	time1 = time0 + ONE_SECOND;		// If not status change within 1 sec, exit
	// Wait while not in retrace
	while (time0 < time1)
	{
		if ((IOByteRead (wStatus) & 0x08) == 0x08)
		{
			bTimeout = FALSE;
			break;
		}
		time0 = GetSystemTicks ();
	}
	if (bTimeout) goto WaitNotVerticalRetrace_exit;

	// Wait while in retrace
	bTimeout = TRUE;
	while (time0 < time1)
	{
		if ((IOByteRead (wStatus) & 0x08) == 0)
		{
			bTimeout = FALSE;
			break;
		}
		time0 = GetSystemTicks ();
	}

WaitNotVerticalRetrace_exit:
	if (wSimType & SIM_VECTORS)
	{
		SimSetState (bDumpIO, bDumpMem, bDumpDAC);	// Restore I/O state
		IOByteRead (wStatus);								// Single I/O read
	}
	return (!bTimeout);
}

//
//		PreFontLoad - Prepare VGA controller for font load
//
//		Entry:	None
//		Exit:		None
//
void PreFontLoad (void)
{
	IOByteWrite (SEQ_INDEX, 0x01);
	IOByteWrite (SEQ_DATA, (BYTE) (IOByteRead (SEQ_DATA) | 0x20));	// Blank screen
	_disable ();
	IOByteWrite (SEQ_INDEX, 0x00);
	IOByteWrite (SEQ_DATA, 0x01);										// Sync reset
	IOByteWrite (GDC_INDEX, 0x05);
	IOByteWrite (GDC_DATA, 0x00);
	IOByteWrite (GDC_INDEX, 0x06);
	IOByteWrite (GDC_DATA, 0x05);
	IOByteWrite (SEQ_INDEX, 0x02);
	IOByteWrite (SEQ_DATA, 0x04);
	IOByteWrite (SEQ_INDEX, 0x04);
	IOByteWrite (SEQ_DATA, 0x06);
	IOByteWrite (SEQ_INDEX, 0x00);
	IOByteWrite (SEQ_DATA, 0x03);										// End sync reset
	_enable ();
}

//
//		PostFontLoad - Undo preparation for font load
//
//		Entry:	None
//		Exit:		None
//
void PostFontLoad (void)
{
	BYTE	gdc06;

	gdc06 = 0x0E;													// Assume B800h
	if ((IOByteRead (MISC_INPUT) & 0x01) == 0) gdc06 = 0x0A;	// Nope, set to B000h

	_disable ();
	IOByteWrite (SEQ_INDEX, 0x00);
	IOByteWrite (SEQ_DATA, 0x01);										// Sync reset
	IOByteWrite (GDC_INDEX, 0x05);
	IOByteWrite (GDC_DATA, 0x10);
	IOByteWrite (GDC_INDEX, 0x06);
	IOByteWrite (GDC_DATA, gdc06);
	IOByteWrite (SEQ_INDEX, 0x02);
	IOByteWrite (SEQ_DATA, 0x03);
	IOByteWrite (SEQ_INDEX, 0x04);
	IOByteWrite (SEQ_DATA, 0x02);
	IOByteWrite (SEQ_INDEX, 0x00);
	IOByteWrite (SEQ_DATA, 0x03);										// End sync reset
	_enable ();
	IOByteWrite (SEQ_INDEX, 0x01);
	IOByteWrite (SEQ_DATA, (BYTE) (IOByteRead (SEQ_DATA) & 0xDF));	// Unblank screen
}

//
//		LoadFontGlyph - Load a specific character image
//
//		Entry:	chr		Character position within font memory
//					height	Height of character (bytes per glyph)
//					block		Font block
//					lpGlyph	Pointer to glyph data
//		Exit:		None
//
void LoadFontGlyph (BYTE chr, BYTE height, BYTE block, LPBYTE lpGlyph)
{
	SEGOFF	lpVideo;
	BYTE		byClear, i;

	PreFontLoad ();

	lpVideo = (SEGOFF) 0xA0000000;
	lpVideo += tblFontBlock[block] + (chr * 32);
	byClear = 32 - height;

	for (i = 0; i < height; i++)
		MemByteWrite (lpVideo++, *lpGlyph++);
	for (i = 0; i < byClear; i++)
		MemByteWrite (lpVideo++, 0);

	PostFontLoad ();
}

//
//		Load8x8 - Load the 8x8 font into a specific block
//
//		Entry:	block		Font block
//		Exit:		None
//
void Load8x8 (BYTE block)
{
	LoadFont (tblFont8x8, 8, 0, 256, block);
}

//
//		Load8x14 - Load the 8x14 font into a specific block
//
//		Entry:	block		Font block
//		Exit:		None
//
void Load8x14 (BYTE block)
{
	LoadFont (tblFont8x14, 14, 0, 256, block);
}

//
//		Load8x16 - Load the 8x16 font into a specific block
//
//		Entry:	block		Font block
//		Exit:		None
//
void Load8x16 (BYTE block)
{
	LoadFont (tblFont8x16, 16, 0, 256, block);
}

//
//		LoadFont - Load a given font into a specific block
//
//		Entry:	lpFont	Address of font table
//					height	Number of bytes per character
//					start		Starting character code
//					count		Number of font images to load
//					block		Font block
//		Exit:		None
//
void LoadFont (LPBYTE lpFont, BYTE height, BYTE start, WORD count, BYTE block)
{
	SEGOFF	lpVideo;
	BYTE		byClear, i;

	PreFontLoad ();

	lpVideo = (SEGOFF) 0xA0000000;
	lpVideo += tblFontBlock[block] + (((int) start) * 32);
	lpFont += ((int) start) * ((int) height);
	byClear = 32 - height;

	while (count-- > 0)
	{
		for (i = 0; i < height; i++)
			MemByteWrite (lpVideo++, *lpFont++);
		for (i = 0; i < byClear; i++)
			MemByteWrite (lpVideo++, 0);
	}

	PostFontLoad ();
}

//
//		DrawTranparentChar - Draw a font image into graphics mode memory
//
//		Entry:	x			X position
//					y			Y position
//					chr		ASCII character code
//					color		Color of image
//					height	Font height
//					fptr		Font data
//		Exit:		None
//
void DrawTransparentChar (WORD x, WORD y, BYTE chr, BYTE color, BYTE height, LPBYTE lpFont)
{
	BYTE				orgSEQ02, orgGDC00, orgGDC05, orgGDC08;
	BYTE				lmask, rmask, ldata, rdata, lshift, rshift;
	SEGOFF			lpVideo;
	WORD				wNextRow;
	static SEGOFF	lpColumns = (SEGOFF) 0x0000044A;

	wNextRow = MemWordRead (lpColumns);

	IOByteWrite (SEQ_INDEX, 0x02);
	orgSEQ02 = (BYTE) IOByteRead (SEQ_DATA);
	IOByteWrite (SEQ_DATA, 0x0F);								// Enable all planes

	IOByteWrite (GDC_INDEX, 0x00);
	orgGDC00 = (BYTE) IOByteRead (GDC_DATA);
	IOByteWrite (GDC_DATA, color);							// Set/reset

	IOByteWrite (GDC_INDEX, 0x05);
	orgGDC05 = (BYTE) IOByteRead (GDC_DATA);
	IOByteWrite (GDC_DATA, (BYTE) (orgGDC05 | 0x03));	// Write mode 3

	IOByteWrite (GDC_INDEX, 0x08);
	orgGDC08 = (BYTE) IOByteRead (GDC_DATA);

	// Characters may span two bytes
	lpVideo = (SEGOFF) 0xA0000000;
	lpVideo += (wNextRow * y) + (x / 8);
	lshift = (BYTE) (x & 7);
	rshift = (BYTE) (8 - lshift);
	lmask = (BYTE) ((0xFF >> lshift));
	rmask = (BYTE) (~lmask);
	lpFont += height * chr;

	// Draw the character
	while (height > 0)
	{
		ldata = (BYTE) ((*lpFont) >> lshift);
		rdata = (BYTE) ((*lpFont) << rshift);
		if (ldata)
		{
			IOByteWrite (GDC_INDEX, 0x08);
			IOByteWrite (GDC_DATA, lmask);
			MemByteRead (lpVideo);				// Load the latches
			MemByteWrite (lpVideo, ldata);
		}

		if (rdata)
		{
			IOByteWrite (GDC_INDEX, 0x08);
			IOByteWrite (GDC_DATA, rmask);
			MemByteRead (lpVideo + 1);			// Load the latches
			MemByteWrite (lpVideo + 1, rdata);
		}

		height--;
		lpFont++;
		lpVideo += wNextRow;
	}

	IOByteWrite (SEQ_INDEX, 0x02);
	IOByteWrite (SEQ_DATA, orgSEQ02);

	IOByteWrite (GDC_INDEX, 0x00);
	IOByteWrite (GDC_DATA, orgGDC00);

	IOByteWrite (GDC_INDEX, 0x05);
	IOByteWrite (GDC_DATA, orgGDC05);

	IOByteWrite (GDC_INDEX, 0x08);
	IOByteWrite (GDC_DATA, orgGDC08);
}


//
//		LoadFixup - Load specific 9 dot characters from a fixup table
//
//		Entry:	lpFixup	Pointer to fixup table
//					height	Number of bytes per character
//					block		Font block
//		Exit:		None
//
void LoadFixup (LPBYTE lpFixup, BYTE height, BYTE block)
{
	static WORD	tblBlock[] = {
		0x0000, 0x4000, 0x8000, 0xC000, 0x2000, 0x6000, 0xA000, 0xE000
	};
	int		nStart, i;
	SEGOFF	lpVideo;

	lpVideo = (SEGOFF) (0xA0000000 + (DWORD) tblBlock[block]);

	PreFontLoad ();

	while (*lpFixup)
	{
		nStart = (*lpFixup++) * 32;
		for (i = 0; i < (int) height; i++)
			MemByteWrite (lpVideo + nStart++, *lpFixup++);
	}

	PostFontLoad ();
}
//


//		stub this crap out
//		SimComment - Add a comment to a test vector stream
//
//		Entry:	pStr	Pointer to a comment string
//		Exit:		None
//
void SimComment (LPSTR pStr)
{
}


//		 stub out
//		DisplayInspectionMessage - Display a message about visual inspection
//
//		Entry:	None
//		Exit:		<BOOL>	Ok to proceed flag (TRUE = Continue, FALSE = Abort)
//
BOOL DisplayInspectionMessage (void)
{
return(TRUE);
}


//		stub out
//		StartCapture - Begin a stream of frame captures
//
//		Entry:	byFrameDelay	Frames to delay for continous capture
//		Exit:		None
//
void StartCapture (BYTE byFrameDelay)
{
}


//
//		SimEndCapture - End a stream of capture frames
//
//		Entry:	None
//		Exit:		None
//
//		Assume "SimStartCapture" has been called.
//
void SimEndCapture (void)
{
}



//
//		MemByteTest - Test a byte at a memory location
//
//		Entry:	lpAddr	Memory address
//					byExp		Expected data
//					lpbyAct	Pointer to return data read
//		Exit:		<BYTE>	Data read
//
BOOL MemByteTest (SEGOFF lpAddr, BYTE byExp, LPBYTE lpbyAct)
{
	BYTE	byData;

   
		// Do the memory read
		byData = *((LPBYTE) (lpAddr));

   	*lpbyAct = byData;

	// Flag a match vs. mismatch condition
	return (byExp == byData);
}

//
//		SetCursorPosition - Set the cursor position
//
//		Entry:	col		Column
//					row		Row
//					page		Page
//		Exit:		None
//
void SetCursorPosition (BYTE col, BYTE row, BYTE page)
{
	static SEGOFF 	lpModeNumber = (SEGOFF) (0x00000449);
	static SEGOFF 	lpCRTCAddr = (SEGOFF) (0x00000463);
	static SEGOFF 	lpColumns = (SEGOFF) (0x0000044A);
	static SEGOFF 	lpRegenLength = (SEGOFF) (0x0000044C);
	static SEGOFF	lpCursorPos = (SEGOFF) (0x00000450);
	WORD				wOffset, wCRTC;

	// In text modes, write the CRTC register for cursor position
	if ((MemByteRead (lpModeNumber) <= 3) ||
		(MemByteRead (lpModeNumber) == 7))
	{
		wOffset = ((MemWordRead (lpColumns) * row) + col) +
						(page * MemWordRead (lpRegenLength));
		wCRTC = MemWordRead (lpCRTCAddr);
		IOByteWrite (wCRTC, 0x0E);
		IOByteWrite ((WORD) (wCRTC + 1), HIBYTE (wOffset));
		IOByteWrite (wCRTC, 0x0F);
		IOByteWrite ((WORD) (wCRTC + 1), LOBYTE (wOffset));
	}

	MemByteWrite (lpCursorPos + page*2, col);
	MemByteWrite (lpCursorPos + page*2 + 1, row);
}

 //
//		CalcStart4 - Calculate the start address in 4 BPP mode
//
//		Entry:	x		Starting X
//		Exit:		y		Starting Y
//
DWORD CalcStart4 (WORD x, WORD y, LPBYTE byShifter)
{
	*byShifter = (BYTE) (x & 0x07);
	return ((DWORD) ((DWORD) y * (DWORD) _line_columns) + (DWORD) (x / 8));
}


//
//		IsVGAEnabled - Determine if the VGA is enabled
//
//		Entry:	None
//		Exit:		<BOOL>	Enabled flag (TRUE = VGA is enabled, FALSE = not)
//
BOOL IsVGAEnabled (void)
{
	BYTE		temp;

	temp = IsIObitFunctional (MISC_INPUT, MISC_OUTPUT, 0x1C);
	return ((temp == 0) ? TRUE : FALSE);
}



//
//		RotateWordRight - Rotate a data word a given number of times
//
//		Entry:	wData		Data word
//					byCount	Number of times to rotate data
//		Exit:		<WORD>	Resultant data
//
WORD RotateWordRight (WORD wData, BYTE byCount)
{
	BYTE	i;

	for (i = 0; i < byCount; i++)
		wData = (WORD) (((wData & 0x01) << 15) | ((WORD) (wData >> 1)));

	return (wData);
}


//
//		StringLength - Return a model independent string length
//
//		Entry:	lpstr		Pointer to string
//		Exit:		<WORD>	Size of string
//
WORD StringLength (LPSTR lpstr)
{
	WORD	wCount;

	wCount = 0;
	while (*lpstr++)
		wCount++;

	return (wCount);
}

//
//		SwapWords - Swap two WORD variables
//
//		Entry:	pw1	Pointer to first WORD variable
//					pw2	Pointer to second WORD variable
//		Exit:		None
//
void SwapWords (LPWORD pw1, LPWORD pw2)
{
	WORD	wTemp;

	wTemp = *pw1;
	*pw1 = *pw2;
	*pw2 = wTemp;
}


//
//		HLine4Internal - Internal horizontal line handler for 4 BPP mode
//
//		Entry:	x1		Starting X
//					y		Starting and ending Y
//					x2		Ending X
//		Exit:		None
//
void HLine4Internal (WORD x1, WORD y, WORD x2)
{
	DWORD		dwOffset;
	WORD		cPix;
	BYTE		byShifter, byMask, byBitMask;
	SEGOFF	lpVideo;

	// Always go left to right
	if (x1 > x2)
		SwapWords (&x1, &x2);

	dwOffset = CalcStart4 (x1, y, &byShifter);
	lpVideo = (SEGOFF) (0xA0000000) + dwOffset;
	cPix = (x2 - x1) + 1;						// Number of pixels to draw
	if (byShifter != 0)
	{
		byMask = (BYTE) (0xFF >> byShifter);
		if (cPix < (WORD) (8 - byShifter))	// Partial byte on both sides
		{
			byBitMask = 0xFF << ((8 - byShifter) - cPix);
			byBitMask = byBitMask & byMask;
			IOByteWrite (0x3CE, 0x08);
			IOByteWrite (0x3CF, byBitMask);
			MemByteRead (lpVideo);
			MemByteWrite (lpVideo, 0xFF);
			return;
		}
		// Partial byte (mask left side)
		IOByteWrite (0x3CE, 0x08);
		IOByteWrite (0x3CF, byMask);
		MemByteRead (lpVideo);
		MemByteWrite (lpVideo, 0xFF);
		cPix = cPix - (8 - byShifter);
		lpVideo++;
	}

	// Do a run of bytes
	IOWordWrite (0x3CE, 0xFF08);
	while (cPix >= 8)
	{
		MemByteRead (lpVideo);
		MemByteWrite (lpVideo, 0xFF);
		lpVideo++;
		cPix -= 8;
	}

	// Last byte
	if (cPix != 0)
	{
		byMask = (BYTE) ~(0xFF >> cPix);
		IOByteWrite (0x3CE, 0x08);
		IOByteWrite (0x3CF, byMask);
		MemByteRead (lpVideo);
		MemByteWrite (lpVideo, 0xFF);
	}
}

//
//		VLine4Internal - Internal vertical line handler for 4 BPP mode
//
//		Entry:	x		Starting and ending X
//					y1		Starting Y
//					y2		Ending Y
//		Exit:		None
//
void VLine4Internal (WORD x, WORD y1, WORD y2)
{
	DWORD		dwOffset;
	SEGOFF	lpVideo;
	WORD		cPix;
	BYTE		byShifter, byMask;

	// Always go top to bottom
	if (y1 > y2)
		SwapWords (&y1, &y2);

	// Calculate start address
	dwOffset = CalcStart4 (x, y1, &byShifter);
	lpVideo = (SEGOFF) (0xA0000000) + dwOffset;

	// Calculate number of pixels and bitmask
	cPix = (y2 - y1) + 1;
#if 0
	_asm {
		mov	ah,080h
		mov	cl,[byShifter]
		ror	ah,cl
		mov	[byMask],ah
	}
#else
	byMask = RotateByteRight (0x80, byShifter);
#endif

	IOByteWrite (0x3CE, 0x08);
	IOByteWrite (0x3CF, byMask);

	while (cPix--)
	{
		MemByteRead (lpVideo);
		MemByteWrite (lpVideo, 0xFF);
		lpVideo += (BYTE) _line_columns;
	}
}

//
//		SimGetState - Get the simulation's "dump" mode
//
//		Entry:	lpbDumpIO		State of I/O simulation
//					lpbDumpMem		State of Memory simulation
//					lpbDumpDAC		State of DAC simulation
//		Exit:		None
//
void SimGetState (LPBOOL lpbDumpIO, LPBOOL lpbDumpMem, LPBOOL lpbDumpDAC)
{
	*lpbDumpIO = _bSimDumpIO;
	*lpbDumpMem = _bSimDumpMem;
	*lpbDumpDAC = _bSimDumpDAC;
}



// stub for diags 
//		CaptureFrame - Capture a video frame
//
//		Entry:	lpFilename		Filename to capture to
//		Exit:		<int>				Error code (0 = Success, Non-zero = Error)
//
int CaptureFrame (LPSTR lpFilename)
{
  	return (0);
}


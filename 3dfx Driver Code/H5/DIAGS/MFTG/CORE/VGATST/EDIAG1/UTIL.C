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

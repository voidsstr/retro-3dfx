//
//		MEM.CPP - Memory routines for EDIAG
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/12/98
//		Last Modified:	5/12/98
//
//		Routines in this file:
//		FlatMemoryTests		High level routine to test a region of memory
//		SingleMemoryTest		Do a single "one-shot" memory pattern test
//		FullMemoryTest			Do a full blown memory test
//		MemAddressTest			Test the memory addressing
//		FillMemoryRange		Fill a range of memory with a given value
//		TestMemoryRange		Test a range of memory with a given pattern
//		TestMemWalkingOne		Walk a one bit across a BYTE, WORD, or DWORD
//		TestMemWalkingZero	Walk a zero bit across a BYTE, WORD, or DWORD
//		MemReportError			Report errors that have occurred
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<i86.h>
#include	"ediag.h"

//
//		FlatMemoryTests - High level routine to test a region of memory
//
//		Entry:	dwPhysAddress	Physical Address of memory to test
//					nUnits			Memory unit size (BYTE = 1, WORD = 2, DWORD = 4)
//					nLength			Amount of memory to test
//		Exit:		<BOOL>			Success flag (TRUE = No errors, FALSE = Errors)
//
BOOL FlatMemoryTests (DWORD dwPhysAddress, int nUnits, long nLength)
{
	long			nRealLength;
	int			i;
	BYTE	__far *fpMemory;
	static int	nSizes[3] = {1, 2, 4};

	// Convert the physical address into a useable linear address
	fpMemory = (BYTE __far *) MK_FP (selFlat, Phys2Linear (dwPhysAddress, nLength * nUnits));
	nRealLength = nLength * nUnits;

	// No overriding user parameters, so let the test rip
	if (_bMemFullTest)
	{
		if (!FullMemoryTest (fpMemory, nUnits, nLength))
			MemReportError ();

		// If a quick test is requested, just cycle the one bit across
		// all of memory. Otherwise, do a one bit and a zero bit test
		// across all of memory and across all memory unit sizes.
		if (_bExtendedTest)
		{
			for (i = 0; i < 3; i++)
			{
				if (!TestMemWalkingOne (fpMemory, nSizes[i], nRealLength / nSizes[i]))
					MemReportError ();
				if (!TestMemWalkingZero (fpMemory, nSizes[i], nRealLength / nSizes[i]))
					MemReportError ();
			}
		}
//		else		// Quick test
//		{
//			if (!TestMemWalkingOne (fpMemory, _nMemUnitSize, nLength))
//				MemReportError ();
//		}

		if (!MemAddressTest (fpMemory, _nMemUnitSize, nLength))
			MemReportError ();
	}
	else		// User overrode some controls
	{
		if ((_dwMemEnd != 0) || (_dwMemStart != 0))
			nLength = (_dwMemEnd - _dwMemStart) / nUnits;

		if (nLength <= 0)
		{
			LogWriteString ("\nERROR: Video memory test given video memory length of 0.");
			return (FALSE);
		}

		if (!SingleMemoryTest (fpMemory + _dwMemStart, nUnits, nLength, _dwMemPattern))
			MemReportError ();
	}
	return (TRUE);
}

//
//		SingleMemoryTest - Do a single "one-shot" memory pattern test
//
//		Entry:	fpMemory		Linear address (far) of starting memory
//					nUnits		Memory unit size (BYTE = 1, WORD = 2, DWORD = 4)
//					nLength		Amount of memory to test
//					dwData		Data to test with
//		Exit:		<BOOL>		Success flag (TRUE = No errors, FALSE = Errors)
//
BOOL SingleMemoryTest (BYTE __far *fpMemory, int nUnits, long nLength, DWORD dwData)
{
	if (nUnits == 1)
		LogComment ("\n\tTesting with pattern %02Xh", dwData & 0xFF);
	else if (nUnits == 2)
		LogComment ("\n\tTesting with pattern %04Xh", dwData & 0xFFFF);
	else
		LogComment ("\n\tTesting with pattern %08Xh", dwData);

	FillMemoryRange (fpMemory, nUnits, nLength, dwData);
	if (!TestMemoryRange (fpMemory, nUnits, nLength, dwData))
		return (FALSE);

	LogComment (" (passed)");
	return (TRUE);
}

//
//		FullMemoryTest - Do a full blown memory test
//
//		Entry:	fpMemory		Linear address (far) of starting memory
//					nUnits		Memory unit size (BYTE = 1, WORD = 2, DWORD = 4)
//					nLength		Amount of memory to test
//		Exit:		<BOOL>		Success flag (TRUE = No errors, FALSE = Errors)
//
BOOL FullMemoryTest (BYTE __far *fpMemory, int nUnits, long nLength)
{
	int			i, j, k;
	long			nRealLength;
	DWORD			dwPattern;
	static int	nSizes[3] = {1, 2, 4};

	// For the extended test, loop through all patterns with all possible
	// memory unit sizes. Basically, access memory as BYTE, WORD, and DWORD.
	nRealLength = nLength * nUnits;
	if (_bExtendedTest)
	{
		for (i = 0; i < nTestPatterns; i++)
		{
			for (j = 0; j < 3; j++)
			{
				// Shift the pattern out on BYTE, WORD, and DWORD boundaries.
				// Note: When size=1, 0<k<4; when size=2, 0<k<2; and when size=4, 0<k<1
				dwPattern = dwTestPatterns[i];
				for (k = 0; k < nSizes[2-j]; k++)
				{
					if (nSizes[j] == 1)
						LogComment ("\n\tTesting with pattern %02Xh", dwPattern & 0xFF);
					else if (nSizes[j] == 2)
						LogComment ("\n\tTesting with pattern %04Xh", dwPattern & 0xFFFF);
					else
						LogComment ("\n\tTesting with pattern %08Xh", dwPattern);

					FillMemoryRange (fpMemory, nSizes[j], nRealLength / nSizes[j], dwPattern);
					if (!TestMemoryRange (fpMemory, nSizes[j], nRealLength / nSizes[j], dwPattern))
						return (FALSE);
					LogComment (" (passed)");

					dwPattern = dwPattern >> (8 * nSizes[j]);
				}
			}
		}
	}
	else			// Quick test -- only do one pattern as DWORD accesses
	{
		LogComment ("\n\tTesting with pattern %08Xh at %FP", _dwMemPattern, fpMemory);
		FillMemoryRange (fpMemory, nUnits, nLength, _dwMemPattern);
		if (!TestMemoryRange (fpMemory, nUnits, nLength, _dwMemPattern))
			return (FALSE);
		LogComment (" (passed)");
	}
	return (TRUE);
}

//
//		MemAddressTest - Test the memory addressing
//
//		Entry:	fpMemory		Linear address (far) of starting memory
//					nUnits		Memory unit size (BYTE = 1, WORD = 2, DWORD = 4)
//					nLength		Amount of memory to test
//		Exit:		<BOOL>		Success flag (TRUE = No errors, FALSE = Errors)
//
//		Note:	This test is tested as a BYTE access test only
//
BOOL MemAddressTest (BYTE __far *fpMemory, int nUnits, long nLength)
{
	long	nRealLength, nOffset, nSearchOffset;
	DWORD	dwData;

	nRealLength = nUnits * nLength;
	LogComment ("\n\tTesting memory address lines from offset 0 to offset %08Xh", nRealLength);

	// Clear memory
	FillMemoryRange (fpMemory, nUnits, nLength, 0);

	// Write a unit of data to each location in memory where the address
	// line is a "1" and verify that the data shows up nowhere else.
	nOffset = 0;
	while (nOffset < nRealLength)
	{
		// Write the unit of data
		if (nUnits == 1)
			*(fpMemory + nOffset) = (BYTE) (LOBYTE (LOWORD (_dwMemPattern)) & _dwMemMask);
		else if (nUnits == 2)
			*(WORD __far *) (fpMemory + nOffset) = (WORD) (LOWORD (_dwMemPattern) & _dwMemMask);
		else
			*(DWORD __far *) (fpMemory + nOffset) = _dwMemPattern & _dwMemMask;

		// Verify that the data shows up nowhere else. For this, only
		// checking strategic address locations is good enough.
		nSearchOffset = nUnits;
		while (nSearchOffset < nRealLength)
		{
			if (nSearchOffset != nOffset)
			{
				if (nUnits == 1)
					dwData = *(fpMemory + nSearchOffset);
				else if (nUnits == 2)
					dwData = *(WORD __far *) (fpMemory + nSearchOffset);
				else
					dwData = *(DWORD __far *) (fpMemory + nSearchOffset);
				dwData &= _dwMemMask;
				if (dwData != 0) goto MemAddressTest_error;
			}
			nSearchOffset = nSearchOffset << 1;
		}

		// Clear the unit of memory
		if (nUnits == 1)
			*(fpMemory + nOffset) = 0;
		else if (nUnits == 2)
			*(WORD __far *) (fpMemory + nOffset) = 0;
		else
			*(DWORD __far *) (fpMemory + nOffset) = 0;

		// Calculate the next address offset
		if (nOffset == 0)
			nOffset = nUnits;		// Start on the proper BYTE, WORD, or DWORD boundary
		else
			nOffset = nOffset << 1;
	}

	LogComment (" (passed)");
	return (TRUE);

// Handle errors here
MemAddressTest_error:
	LogComment ("\n\tTest aborted");
	_errLinAddr = FP_OFF (fpMemory + nSearchOffset);
	_errActual = dwData;
	_errExpected = 0;
	_errSize = nUnits;
	return (FALSE);
}

//
//		FillMemoryRange - Fill a range of memory with a given value
//
//		Entry:	fpMemory		Linear address (far) of starting memory
//					nUnits		Memory unit size (BYTE = 1, WORD = 2, DWORD = 4)
//					nLength		Number of memory units to fill
//					dwData		Data to fill with
//		Exit:		None
//
void FillMemoryRange (BYTE __far *fpMemory, int nUnits, long nLength, DWORD dwData)
{
	BYTE	byData;
	WORD	wData;

	// If the buffer is zero length, then there is nothing to process
	if (nLength == 0) return;

	switch (nUnits)
	{
		case 1:								// Fill BYTE's

			byData = (BYTE) (LOBYTE (LOWORD (dwData)) & _dwMemMask);
			while (nLength--)
				*fpMemory++ = byData;
			break;

		case 2:								// Fill WORD's

			wData = (WORD) (LOWORD (dwData) & _dwMemMask);
			while (nLength--)
			{
				*(WORD __far *) fpMemory = wData;
				fpMemory += 2;
			}
			break;

		case 4:								// Fill DWORD's

			dwData = dwData & _dwMemMask;
			while (nLength--)
			{
				*(DWORD __far *) fpMemory = dwData;
				fpMemory += 4;
			}
			break;
	}
}

//
//		TestMemoryRange - Test a range of memory with a given pattern
//
//		Entry:	fpMemory		Linear address (far) of starting memory
//					nUnits		Memory unit size (BYTE = 1, WORD = 2, DWORD = 4)
//					nLength		Number of memory units to fill
//					dwData		Data to fill with
//		Exit:		<BOOL>		Success flag (TRUE = No errors, FALSE = Errors)
//
BOOL TestMemoryRange (BYTE __far *fpMemory, int nUnits, long nLength, DWORD dwData)
{
	DWORD	dwActual;

	// If the buffer is zero length, then there is nothing to process
	if (nLength == 0) return (FALSE);

	switch (nUnits)
	{
		case 1:

			dwData = (LOBYTE (LOWORD (dwData)) & _dwMemMask);
			while (--nLength)
			{
				dwActual = (*fpMemory & _dwMemMask);
				if (dwActual != dwData) goto TestMemoryRange_error;
				fpMemory += 1;
			}
			break;

		case 2:

			dwData = (LOWORD (dwData) & _dwMemMask);
			while (--nLength)
			{
				dwActual = (*(WORD __far *) fpMemory) & _dwMemMask;
				if (dwActual != dwData) goto TestMemoryRange_error;
				fpMemory += 2;
			}
			break;

		case 4:

			dwData = dwData & _dwMemMask;
			while (--nLength)
			{
				dwActual = (*(DWORD __far *) fpMemory) & _dwMemMask;
				if (dwActual != dwData) goto TestMemoryRange_error;
				fpMemory += 4;
			}
			break;
	}

	return (TRUE);			// All is well, test passed

// Handle errors here
TestMemoryRange_error:
	LogComment ("\n\tTest aborted");
	_errLinAddr = FP_OFF (fpMemory);
	_errActual = dwActual;
	_errExpected = dwData;
	_errSize = nUnits;
	return (FALSE);
}

//
//		TestMemWalkingOne - Walk a one bit across a BYTE, WORD, or DWORD
//
//		Entry:	fpMemory	Linear address (far) of starting memory
//					nUnits	Memory unit size (BYTE = 1, WORD = 2, DWORD = 4)
//					nLength	Length of memory to test
//		Exit:		<BOOL>	Success flag (TRUE = No errors, FALSE = Errors)
//
BOOL TestMemWalkingOne (BYTE __far *fpMemory, int nUnits, long nLength)
{
	int	i;
	DWORD	dwTestPat;

	LogComment ("\n\tTesting with a walking 1 bit across %s",
		(nUnits == 1) ? "BYTES" : ((nUnits == 2) ? "WORDS" : "DWORDS"));

	// Walk a "1" bit across the length of a BYTE, WORD, or DWORD depending
	// on the defined unit size.
	dwTestPat = 0x00000001;
	for (i = 0; i < (8 * nUnits); i++)
	{
		LogComment ("\n\t\tPattern = %08Xh", dwTestPat);
		FillMemoryRange (fpMemory, nUnits, nLength, dwTestPat);
		if (!TestMemoryRange (fpMemory, nUnits, nLength, dwTestPat))
			return (FALSE);
		dwTestPat = dwTestPat << 1;
	}

	LogComment (" (passed)");
	return (TRUE);
}

//
//		TestMemWalkingZero - Walk a zero bit across a BYTE, WORD, or DWORD
//
//		Entry:	fpMemory	Linear address (far) of starting memory
//					nUnits	Memory unit size (BYTE = 1, WORD = 2, DWORD = 4)
//					nLength	Length of memory to test
//		Exit:		<BOOL>	Success flag (TRUE = No errors, FALSE = Errors)
//
BOOL TestMemWalkingZero (BYTE __far *fpMemory, int nUnits, long nLength)
{
	int	i;
	DWORD	dwTestPat;

	LogComment ("\n\tTesting with a walking 0 bit across %s",
		(nUnits == 1) ? "BYTES" : ((nUnits == 2) ? "WORDS" : "DWORDS"));

	// Walk a "0" bit across the length of a BYTE, WORD, or DWORD depending
	// on the defined unit size.
	dwTestPat = 0xFFFFFFFE;
	for (i = 0; i < (8 * nUnits); i++)
	{
		LogComment ("\n\t\tPattern = %08Xh", dwTestPat);
		FillMemoryRange (fpMemory, nUnits, nLength, dwTestPat);
		if (!TestMemoryRange (fpMemory, nUnits, nLength, dwTestPat))
			return (FALSE);
		dwTestPat = _lrotl (dwTestPat, 1);
	}

	LogComment (" (passed)");
	return (TRUE);
}

//
//		MemError - Log a memory error
//
//		Entry:	
//		Exit:		None
//
void MemError (void)
{

	LogComment("\nfnTestMem: %s Memory Test", 
					  (_bMemFullTest ? "FULL" : "SINGLE"));

	// Explain the test
	if (!_bMemFullTest)
	{
		LogComment("\n\t_dwMemPattern=%08lXh, _nMemUnitSize=%ld",
			  							_dwMemPattern, _nMemUnitSize);
		LogComment("\n\t_dwMemStart=%08lXh, _dwMemEnd=%08lXh",
			  				 _dwMemStart, _dwMemEnd);
	}
	else
	{
		LogComment("\n\t_dwMemPattern= WALKING 1's & 0's and");
		LogComment("\n\t0xA, 0x5, 0x6, 0x9, 0xC, 0x3, 0x0, and 0xF's,");
		LogComment("\n\n\t_nMemUnitSize = BYTE, WORD, and DWORD");
		LogComment(
			"\n\t_dwMemStart=%08lXh,\n\t_dwMemEnd=%08lXh,ExtendedTest=%s",
			  _dwMemStart, _dwMemEnd, _bExtendedTest ? "Extended" : "Quick");
		LogComment("\n\tAnd random memory location tests");
	}

	SetMode (3);
	exit(0);
}

//
//		MemReportError - Report errors that have occurred
//
//		Entry:	None
//		Exit:		None
//
void MemReportError (void)
{
	LogComment ("\n\tERROR: Memory comparison failure."
					"\n\t\tLinear Address = %08Xh"
					"\n\t\tExpected Data = %08Xh"
					"\n\t\tActual Data = %08Xh"
					"\n\t\tData Size = %s"
					"\n\t\tMemory Mask = %08Xh",
					_errLinAddr, _errExpected, _errActual,
					(_errSize == 1) ? "BYTE" : ((_errSize == 2) ? "WORD" : "DWORD"),
					_dwMemMask);

	if (_errCount++ > MAX_ERRORS)
	{
		LogComment ("\n\nMaximum Errors exceeded (%d) - Tests terminated.", MAX_ERRORS);
		PrepareExit ();
		exit (1);
	}
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//

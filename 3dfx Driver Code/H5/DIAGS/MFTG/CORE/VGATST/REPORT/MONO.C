//
//		MONO.CPP - Monochrome specific init routines for REPDEV.LIB
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/17/98
//		Last Modified:	5/11/98
//
//		Routines in this file:
//		OpenMonoDevice				Initialize monochrome reporting device
//		CloseMonoDevice			Terminate monochrome reporting device
//		WriteStringMonoDevice	Write a string to the monochrome device
//		MonoFullScreenScroll		Scroll up the screen by one row
//		MonoSetCursorPosition	Set the text cursor position in hardware
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include "ediag.h"
#include	"repdev.h"

//
//		OpenMonoDevice - Initialize monochrome reporting device
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Opened, FALSE = Device failed)
//
BOOL OpenMonoDevice (void)
{
	BYTE	by3C2, by6845_C, by6845_D;
	BYTE	__far *lp;
	int	i;

	// Determine if a monochrome card is available by doing a read/write
	// I/O test to the 6845 start address register (one of the few R/W
	// registers on a 6845).
	_bMonoOpen = FALSE;
	by3C2 = _inp (MISC_INPUT);
	_outp (MISC_OUTPUT, (BYTE) (by3C2 | 0x01));	// Force VGA to color (just in case)

	// Save the original values in the Motorola 6845 registers C & D
	_outp (CRTC_MINDEX, 0x0C);
	by6845_C = _inp (CRTC_MDATA);
	_outp (CRTC_MINDEX, 0x0D);
	by6845_D = _inp (CRTC_MDATA);

	// Test by writing and reading to the display start address
	_outpw (CRTC_MINDEX, 0xAA0D);
	if (_inp (CRTC_MDATA) != 0xAA) goto OpenMonoDevice_exit;
	_outpw (CRTC_MINDEX, 0x2A0C);
	if (_inp (CRTC_MDATA) != 0x2A) goto OpenMonoDevice_exit;
	_outpw (CRTC_MINDEX, 0x550D);
	if (_inp (CRTC_MDATA) != 0x55) goto OpenMonoDevice_exit;
	_outpw (CRTC_MINDEX, 0x150C);
	if (_inp (CRTC_MDATA) != 0x15) goto OpenMonoDevice_exit;

	// Restore misc output and flag mono as existing
	_outp (MISC_OUTPUT, by3C2);
	_bMonoOpen = TRUE;

	// Create a linear address pointer to the monochrome framebuffer
	_linMono = Phys2Linear (0xB0000, 64*1024);
	_lpMono = (BYTE __far *) MK_FP (_selFlatDescriptor, _linMono);

	// Clear memory
	lp = _lpMono;
	for (i = 0; i < 80*25; i++)
	{
		*lp++ = ' ';
		*lp++ = 0x07;
	}
	_nMonoCol = 0;
	_nMonoRow = 0;

OpenMonoDevice_exit:
	// Restore the original register values
	_outp (CRTC_MINDEX, 0x0C);
	_outp (CRTC_MDATA, by6845_C);
	_outp (CRTC_MINDEX, 0x0D);
	_outp (CRTC_MDATA, by6845_D);
	_outp (MISC_OUTPUT, by3C2);
	return (_bMonoOpen);
};

//
//		CloseMonoDevice - Terminate monochrome reporting device
//
//		Entry:	None
//		Exit:		<BOOL>		Success flag (TRUE = Closed, FALSE = Device failed)
//
BOOL CloseMonoDevice (void)
{
	if (!_bMonoOpen) return (FALSE);
	return (TRUE);
}

//
//		WriteStringMonoDevice - Write a string to the monochrome device
//
//		Entry:	lpstr		Pointer to string to write
//		Exit:		<BOOL>	Success flag (TRUE = Written, FALSE = Device failed)
//
BOOL WriteStringMonoDevice (LPSTR lpstr)
{
	char	chr;
	BYTE	__far *lp;

	if (!_bMonoOpen) return (FALSE);

	while ((chr = *lpstr++) != '\0')
	{
		lp = _lpMono + (_nMonoRow * 160) + (_nMonoCol * 2);
		if (chr == '\n')
		{
			_nMonoCol = 0;
			_nMonoRow++;
		}
		else if (chr == '\r')
		{
			_nMonoCol = 0;
		}
		else if (chr == '\t')
		{
			_nMonoCol = ((_nMonoCol + 8) & ~0x07);
		}
		else
		{
			*lp = chr;
			_nMonoCol++;
		}

		if (_nMonoCol >= 80)
		{
			_nMonoCol = 0;
			_nMonoRow++;
		}

		if (_nMonoRow >= 25)
		{
			_nMonoRow = 24;
			MonoFullScreenScroll ();
		}
	}

	MonoSetCursorPosition (_nMonoCol, _nMonoRow);
	return (TRUE);
}

//
//		MonoFullScreenScroll - Scroll up the screen by one row
//
//		Entry:	None
//		Exit:		None
//
void MonoFullScreenScroll (void)
{
	BYTE	__far *lp;
	int	i, j;

	lp = _lpMono;
	// Move up 24 rows
	for (i = 0; i < 24; i++)
	{
		for (j = 0; j < 80; j++)
		{
			*lp = *(lp + 160);
			lp++;
			*lp = *(lp + 160);
			lp++;
		}
	}

	// Clear the last row
	for (i = 0; i < 80; i++)
	{
		*lp++ = ' ';
		*lp++ = 0x07;
	}
}

//
//		MonoSetCursorPosition - Set the text cursor position in hardware
//
//		Entry:	nCol		Currrent column
//					nRow		Current row
//		Exit:		None
//
void MonoSetCursorPosition (int nCol, int nRow)
{
	WORD	wPos;
	
	wPos = (WORD) (((nRow * 160) + (nCol * 2)) / 2);
	_outp (CRTC_MINDEX, 0x0E);
	_outp (CRTC_MDATA, HIBYTE (wPos));
	_outp (CRTC_MINDEX, 0x0F);
	_outp (CRTC_MDATA, LOBYTE (wPos));
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//

//
//		VBEAPI.CPP - VBE graphics primitives for EDIAG.EXE
//		Copyright (c) 1993-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				12/8/94
//		Last modified:	4/29/98
//
//		Routines in this file:
//		GetVBEInfo				Return the VESA BIOS info block
//		GetVBEModeInfo			Return mode specific information
//		VBEHLine					Draw a horizontal line in the current mode
//		VBEVLine					Draw a vertical line in the current mode
//		VBETextOut				Write a string to the screen
//		VBEWriteCharAt			Write a character at a given location
//		VBEWriteChar			Write a single character at the current cursor position
//		VBECircle				Draw a circle in a given color
//		VBEPixel					Draw a pixel in a given color
//		VBESetBank				Set the video memory bank
//		VBEGetBank				Get the video memory bank
//		VBEIncBank				Increment the bank number
//		SetCursorPositionW		Set the current cursor position
//		SetTextColor			Set text colors on future character writes
//		DrawGradient			Draw a color gradient within specified rectangle
//		GetNextColor			Return color based on position in gradient
//		SetDAC					Set RAMDAC to given values
//		GetColorIntensity		Return color value based on percentage of two values
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<conio.h>
#include	<dos.h>
#include	<string.h>
#include	<malloc.h>
#include	<graph.h>
#include	<limits.h>
#include	"ediag.h"

//
//		GetVBEInfo - Return the VESA mode info block
//
//		Entry:	pvbeib	Pointer to 512 byte VBE info block buffer
//		Exit:		<BOOL>	Success flag (TRUE = Got info, FALSE = VBE not present)
//
BOOL GetVBEInfo (LPVBEINFOBLOCK pvbeib)
{
	union REGS		regs;
	REALMODECALL	rmc;
	WORD				wSel, wSeg, wRetVal;
	LPBYTE			lpvbeibTmp;

	// Allocate DOS Memory Block
	regs.w.ax = 0x0100;
	regs.w.bx = (sizeof (VBEINFOBLOCK) / 16) + 1;
	int386 (0x31, &regs, &regs);
	wSeg = regs.w.ax;
	wSel = regs.w.dx;

	// Return VBE 2.0 (and above) information
	_fmemcpy (MK_FP (wSel, 0), "VBE2", 4);

	// Do a VBE get info
	// AX = 4F00h, ES:DI = Info block ptr, INT 10h
	rmc.eax = 0x4F00;
	rmc.es = wSeg;
	rmc.edi = 0;
	RealModeINT (&rmc, 0x10);
	wRetVal = (WORD) rmc.eax;

	// Copy the info out of the Real mode segment
	_fmemcpy (pvbeib, MK_FP (wSel, 0), sizeof (VBEINFOBLOCK));

	// Convert the segment:offset pointers to selector:offset pointers.
	// Since the original offset from the DOS block was a base of 0, all
	// we need to do is to add the address of the structure to the returned
	// WORD offset.
	// However!!! The Matrox Mystique has a bug that leaves the pointers
	// pointing into the ROM. Check for the segment being equal to "wSeg"
	// before doing the conversion!
	lpvbeibTmp = (LPBYTE) pvbeib;
	if (HIWORD (pvbeib->OEMStringPtr) == wSeg)
		pvbeib->OEMStringPtr = (LPBYTE) (lpvbeibTmp + (FP_OFF (pvbeib->OEMStringPtr) & 0xFFFF));
	else
		pvbeib->OEMStringPtr = (LPBYTE) szVBE20Error;

	if (HIWORD (pvbeib->VideoModePtr) == wSeg)
		pvbeib->VideoModePtr = (LPWORD) (lpvbeibTmp + (FP_OFF (pvbeib->VideoModePtr) & 0xFFFF));
	else
		pvbeib->VideoModePtr = &wFFFF;

	if (HIWORD (pvbeib->VBE_VendorNamePtr) == wSeg)
		pvbeib->VBE_VendorNamePtr = (LPBYTE) (lpvbeibTmp + (FP_OFF (pvbeib->VBE_VendorNamePtr) & 0xFFFF));
	else
		pvbeib->VBE_VendorNamePtr = (LPBYTE) szVBE20Error;

	if (HIWORD (pvbeib->VBE_ProductNamePtr) == wSeg)
		pvbeib->VBE_ProductNamePtr = (LPBYTE) (lpvbeibTmp + (FP_OFF (pvbeib->VBE_ProductNamePtr) & 0xFFFF));
	else
		pvbeib->VBE_ProductNamePtr = (LPBYTE) szVBE20Error;

	if (HIWORD (pvbeib->VBE_ProductRevPtr) == wSeg)
		pvbeib->VBE_ProductRevPtr = (LPBYTE) (lpvbeibTmp + (FP_OFF (pvbeib->VBE_ProductRevPtr) & 0xFFFF));
	else
		pvbeib->VBE_ProductRevPtr = (LPBYTE) szVBE20Error;

	// Free allocated DOS block
	regs.w.ax = 0x0101;
	regs.w.dx = wSel;
	int386 (0x31, &regs, &regs);

	// Check and see if we passed
	if (wRetVal == 0x004F)
		return (TRUE);
	else
		return (FALSE);
}

//
//		GetVBEModeInfo - Return mode specific information
//
//		Entry:	pvbemi	Pointer to 256 byte VESA info block buffer
//					wMode		Mode number
//		Exit:		<BOOL>	Success flag (TRUE = Got info, FALSE = VBE not present)
//
BOOL GetVBEModeInfo (LPVBEMODEINFOBLOCK pvbemi, WORD wMode)
{
	union REGS		regs;
	REALMODECALL	rmc;
	WORD				wSel, wSeg, wRetVal;

	// Allocate DOS Memory Block
	regs.w.ax = 0x0100;
	regs.w.bx = (sizeof (VBEMODEINFOBLOCK) / 16) + 1;
	int386 (0x31, &regs, &regs);
	wSeg = regs.w.ax;
	wSel = regs.w.dx;

	// Do a VBE get mode info
	// AX = 4F01h, CX = Mode #, ES:DI = Info block ptr, INT 10h
	rmc.eax = 0x4F01;
	rmc.ecx = (DWORD) wMode;
	rmc.es = wSeg;
	rmc.edi = 0;
	RealModeINT (&rmc, 0x10);
	wRetVal = (WORD) rmc.eax;

	// Copy the info out of the Real mode segment
	_fmemcpy (pvbemi, MK_FP (wSel, 0), sizeof (VBEMODEINFOBLOCK));

	// Free allocated DOS block
	regs.w.ax = 0x0101;
	regs.w.dx = wSel;
	int386 (0x31, &regs, &regs);

	// Check and see if we passed
	if (wRetVal == 0x004F)
		return (TRUE);
	else
		return (FALSE);
}

//
//		VBEHLine - Draw a horizontal line in the current mode
//
//		Entry:	x		Starting X position
//					y		Starting Y position
//					wx		Length of the line
//					lpclr	Pointer to COLORREF data structure for color to draw in
//		Exit:		None
//
void VBEHLine (WORD x, WORD y, WORD wx, LPCOLORREF lpclr)
{
	switch (vbeModeInfo.BitsPerPixel)
	{
		case 1:
			if (vbeModeInfo.MemoryModel != 0x01)	// Non-CGA?
				HLine4 (x, y, wx, lpclr->bpp1);
			else												// CGA style 1-bit
				HLineCGA1 (x, y, wx, lpclr->bpp1);
			break;

		case 2:

			if (vbeModeInfo.MemoryModel == 0x01)	// CGA?
				HLineCGA (x, y, wx, lpclr->bpp2);
			break;		// Not supported

		case 4:

			if (vbeModeInfo.MemoryModel == 0x04)	// Packed-pixel
				HLinePacked4 (x, y, wx, lpclr->bpp4);
			else
				HLine4 (x, y, wx, lpclr->bpp4);		// Planar
			break;

		case 8:

			HLine8 (x, y, wx, lpclr->bpp8);
			break;

		case 15:

			HLine16 (x, y, wx, lpclr->bpp15);
			break;

		case 16:

			HLine16 (x, y, wx, lpclr->bpp16);
			break;

		case 24:
		case 32:

			HLine24 (x, y, wx, lpclr->bpp24);
			break;
	}
}

//
//		VBEVLine - Draw a vertical line in the current mode
//
//		Entry:	x		Starting X position
//					y		Starting Y position
//					wy		Length of the line
//					lpclr	Pointer to COLORREF data structure for color to draw in
//		Exit:		None
//
void VBEVLine (WORD x, WORD y, WORD wy, LPCOLORREF lpclr)
{
	switch (vbeModeInfo.BitsPerPixel)
	{
		case 1:
		case 2:

			break;		// Not supported

		case 4:

			VLine4 (x, y, wy, lpclr->bpp4);
			break;

		case 8:

			VLine8 (x, y, wy, lpclr->bpp8);
			break;

		case 15:

			VLine16 (x, y, wy, lpclr->bpp15);
			break;

		case 16:

			VLine16 (x, y, wy, lpclr->bpp16);
			break;

		case 24:
		case 32:

			VLine24 (x, y, wy, lpclr->bpp24);
			break;
	}
}

//
//		VBETextOut - Write a string to the screen
//
//		Entry:	col	Text column
//					row	Text row
//					psz	Pointer to text string
//					attr	Color of text
//		Exit:		None
//
void VBETextOut (WORD col, WORD row, LPSTR psz, BYTE attr)
{
	if (vbeModeInfo.ModeAttributes & 0x10)	// Graphics Mode
	{
		while (*psz)
		{
			SetCursorPositionW (col++, row);
			VBEWriteChar (*psz);
			psz++;
		}
	}
	else
	{
		WriteBIOSString ((BYTE) col, (BYTE) row, (BYTE) strlen (psz), (LPBYTE) psz, attr);
	}
}

//
//		VBEWriteCharAt - Write a character at a given location
//
//		Entry:	col	Text column
//					row	Text row
//					chr	ASCII character
//					attr	Text attribute
//		Exit:		None
//
void VBEWriteCharAt (WORD col, WORD row, BYTE chr, BYTE attr)
{
	union REGS	regs;

	SetCursorPositionW (col, row);
	if (vbeModeInfo.ModeAttributes & 0x10)	// Graphics Mode
	{
		VBEWriteChar (chr);
	}
	else												// Text Mode
	{
		regs.h.ah = 0x09;
		regs.h.al = (BYTE) chr;
		regs.h.bh = 0;
		regs.h.bl = (BYTE) attr;
		regs.w.cx = 1;
		int386 (0x10, &regs, &regs);
	}
}

//
//		VBEWriteChar - Write a single character at the current cursor position
//
//		Entry:	chr	Character
//		Exit:		None
//
void VBEWriteChar (BYTE chr)
{
	BYTE			attr;
	union REGS	regs;

	switch (vbeModeInfo.BitsPerPixel)
	{
		case 1:
		case 2:

			attr = lpclrTextFore->bpp2;
			regs.h.ah = 0x09;
			regs.h.al = chr;
			regs.h.bh = 0;
			regs.h.bl = attr;
			regs.w.cx = 1;
			int386 (0x10, &regs, &regs);
			break;

		case 4:

			if (vbeModeInfo.MemoryModel == 0x04)	// Packed-pixel
				WriteCharPacked4 (chr);
			else												// Planar
				WriteChar4 (chr);
			break;

		case 8:

			WriteChar8 (chr);
			break;

		case 15:
		case 16:

			WriteChar16 (chr);
			break;

		case 24:
		case 32:

			WriteChar24 (chr);
			break;
	}
}

//
//		VBECircle - Draw a circle in a given color
//
//		Entry:	orgx		X coordinate of origin
//					orgy		Y coordinate of origin
//					radius	Radius of circle
//					lpclr		Pointer to COLORREF data structure for color of circle
//		Exit:		None
//
void VBECircle (int orgx, int orgy, int radius, LPCOLORREF lpclr)
{
	int	x, y;
	int	di, d;
	int	limit;
	int	diameter;
	int	minx, miny, maxx, maxy;

	if ((vbeModeInfo.ModeAttributes & 0x10)	== 0) return;	// Text mode

	x = 0;
	y = radius;
	limit = 0;
	di = 2 * (1 - radius);
	diameter = 2 * radius;

	minx = miny = 0;
	maxx = vbeModeInfo.XResolution - 1;
	maxy = vbeModeInfo.YResolution - 1;

	while (y >= limit)
	{
		VBEPixel ((WORD) __max (__min (orgx + x, maxx), minx),
					  (WORD) __max (__min (orgy + y, maxy), miny), lpclr);
		VBEPixel ((WORD) __max (__min (orgx + x, maxx), minx),
					  (WORD) __max (__min (orgy - y, maxy), miny), lpclr);
		VBEPixel ((WORD) __max (__min (orgx - x, maxx), minx),
					  (WORD) __max (__min (orgy - y, maxy), miny), lpclr);
		VBEPixel ((WORD) __max (__min (orgx - x, maxx), minx),
					  (WORD) __max (__min (orgy + y, maxy), miny), lpclr);
		if (di < 0)
		{
			d = 2*di + 2*y - 1;
			if (d <= 0)
			{
				x++;
				di = di + 2*x + 1;
			}
			else
			{
				x++;
				y--;
				di = di + 2*x - 2*y + 2;
			}
		}
		else if (di > 0)
		{
			d = 2*di - 2*x - 1;
			if (d <= 0)
			{
				x++;
				y--;
				di = di + 2*x - 2*y + 2;
			}
			else
			{
				y--;
				di = di - 2*y + 1;
			}
		}
		else
		{
			x++;
			y--;
			di = di + 2*x - 2*y + 2;
		}
	}
}

//
//		VBEPixel - Draw a pixel in a given color
//
//		Entry:	x			X screen coordinate
//					y			Y screen coordinate
//					lpclr		Pointer to COLORREF data structure for color of pixel
//		Exit:		None
//
void VBEPixel (WORD x, WORD y, LPCOLORREF lpclr)
{
	switch (vbeModeInfo.BitsPerPixel)
	{
		case 1:
		case 2:

			break;		// Not supported

		case 4:

			DrawPixel4 (x, y, lpclr->bpp4);
			break;

		case 8:

			DrawPixel8 (x, y, lpclr->bpp8);
			break;

		case 15:

			DrawPixel16 (x, y, lpclr->bpp15);
			break;

		case 16:

			DrawPixel16 (x, y, lpclr->bpp16);
			break;

		case 24:
		case 32:

			DrawPixel24 (x, y, lpclr->bpp24);
			break;
	}
}

//
//		VBESetBank - Set the video memory bank
//
//		Entry:	bank		Bank number
//		Exit:		None
//
//		Call the VBE window function with these parameters:
//			BL = Window number (0 = A, 1 = B)
//			BH = Subfunction:
//					00h: Set bank
//					01h: Get bank
//					80h: Set bank during vertical retrace
//			DX = Bank number
//		The VBE call returns:
//			If subfunction 1:
//				DX = Bank number
//
void VBESetBank (BYTE bank)
{
	REALMODECALL	rmc;

	rmc.eax = 0x4F05;
	rmc.edx = (DWORD) (bank << byBankShifter);
	rmc.ebx = 0;
	RealModeINT (&rmc, 0x10);
}

//
//		VBEGetBank - Get the video memory bank
//
//		Entry:	None
//		Exit:		<WORD>	Bank number
//
//		Call the VBE window function with these parameters:
//			BL = Window number (0 = A, 1 = B)
//			BH = Subfunction:
//					00h: Set bank
//					01h: Get bank
//					80h: Set bank during vertical retrace
//			DX = Bank number
//		The VBE call returns:
//			If subfunction 1:
//				DX = Bank number
//
WORD VBEGetBank (void)
{
	REALMODECALL	rmc;
	WORD				bank;

	rmc.eax = 0x4F05;
//	rmc.edx = (DWORD) bank;
	rmc.ebx = 0x0100;
	RealModeINT (&rmc, 0x10);
	bank = (WORD) rmc.edx;

	bank = (WORD) (bank << byBankShifter);
	return (bank);
}

//
//		VBEIncBank - Increment the bank number
//
//		Entry:	None
//		Exit:		None
//
void VBEIncBank (void)
{
	VBESetBank ((BYTE) (VBEGetBank () + 1));
}

#pragma optimize("egl",off)			// Disable assembly precludes optimizatons
//
//		SetCursorPositionW - Set the current cursor position
//
//		Entry:	col	Text column
//					row	Text row
//		Exit:		None
//
void EnableCursor(int enable) {
	union REGS	regs;

    if (enable) {
        regs.h.ah = 0x01;
        regs.h.ch = 14;
	regs.h.cl = 15;
	int386 (0x10, &regs, &regs);
    } else {
	regs.h.ah = 0x01;
	regs.h.ch = 0x20;
	int386 (0x10, &regs, &regs);
    }

}

void SetCursorPositionW (WORD col, WORD row)
{
	union REGS	regs;

	yTextRow = row;
	xTextCol = col;

	regs.h.ah = 0x02;
	regs.h.dh = (BYTE) row;
	regs.h.dl = (BYTE) col;
	regs.w.bx = 0;
	int386 (0x10, &regs, &regs);
}
#pragma optimize("",on)					// Re-enable optimizations

//
//		SetTextColor - Set text colors on future character writes
//
//		Entry:	fore				Foreground text color
//					back				Background text color
//					bTransparent	Transparency flag (TRUE = Transparent, FALSE = Opaque)
//		Exit:		None
//
void SetTextColorTrans (LPCOLORREF lpfore, LPCOLORREF lpback, BOOL bTransparent)
{
	lpclrTextFore = lpfore;
	lpclrTextBack = lpback;
	bOpaque = !bTransparent;
}

//
//		DrawGradient - Draw a color gradient within specified rectangle
//
//		Entry:	ulx	Upper left X coordinate
//					uly	Upper left Y coordinate
//					lrx	Lower right X coordinate
//					lry	Lower right Y coordinate
//		Exit:		None
//
void DrawGradient (WORD ulx, WORD uly, WORD lrx, WORD lry)
{
	WORD			xWidth, yHeight, i;
	WORD			x, y, steps, stepctr;
	LPCOLORREF	lpclr;
	BYTE			attr;

	if (vbeModeInfo.ModeAttributes & 0x10)	// Graphics mode
	{
		xWidth = (WORD) ((lrx - ulx) + 1);
		steps = yHeight = (WORD) ((lry - uly) + 1);

		if (vbeModeInfo.BitsPerPixel == 8)
			NewSetDac (16, 255, (LPBYTE) dactable);

		y = uly;
		stepctr = 0;
		while (stepctr < yHeight)
		{
			lpclr = GetNextColor (steps, stepctr);
			VBEHLine (ulx, y, xWidth, lpclr);
			stepctr++; y++;
		}
	}
	else													// Text mode
	{
		// Don't draw in the first or last row or column
		if (ulx == 0) ulx++;
		if (uly == 0) uly++;
		if (lrx == (vbeModeInfo.XResolution - 1)) lrx--;
		if (lry == (vbeModeInfo.YResolution - 1)) lry--;

		// Fit text stuff into rectangle, if it doesn't fit exactly, then
		// ignore the remainder (don't draw).
		xWidth = (WORD) (lrx - ulx);
		yHeight = (WORD) (lry - uly);
		steps = (WORD) (yHeight / 15);
		y = uly;
		for (attr = 1; attr < 16; attr++)
		{
			for (i = 0; i < steps; i++)
			{
				for (x = ulx; x < (ulx + xWidth); x++)
					WriteBIOSChar ((BYTE) (x), (BYTE) y, 0xDB, attr);
				y++;
			}
		}
	}
}

//
//		GetNextColor - Return color based on position in gradient
//
//		Entry:	steps				Number of gradient steps
//					counter			Position within "steps"
//		Exit:		<LPCOLORREF>	Pointer to COLORREF data structure
//
LPCOLORREF GetNextColor (WORD steps, WORD counter)
{
	DWORD		color, bucket, groupsize;
	WORD		idx;
	BYTE		perc, red, green, blue;
	static BYTE	byLookup[8][3] = {
					{0, 0, 0},			{0, 0, 255},
					{0, 255, 0},		{0, 255, 255},
					{255, 0, 0},		{255, 0, 255},
					{255, 255, 0},		{255, 255, 255}
				};
	static COLORREF clrBuffer = {0x00, 0x00, 0x00, 0x00, 0x0000, 0x0000, 0x00000000};

	switch (vbeModeInfo.BitsPerPixel)
	{
		case 1:

			clrBuffer.bpp1 = 1;
			break;

		case 2:

			clrBuffer.bpp2 = (BYTE) ((((DWORD) counter) * 3l) / ((DWORD) steps) + 1);
			break;

		case 4:

			clrBuffer.bpp4 = (BYTE) ((((DWORD) counter) * 15l) / ((DWORD) steps) + 1);
			break;

		case 8:

			clrBuffer.bpp8 = (BYTE) ((((DWORD) counter) * 240l) / ((DWORD) steps) + 16);
			break;

		case 15:			// 5-5-5 format

			groupsize = (((DWORD) steps) * 100l) / 7l;				// Blocksize * 100
			bucket = (((DWORD) counter) * 10000l) / groupsize;		// "Bucket" * 100
			perc = (BYTE) (bucket % 100);									// Percent color intensity
			idx = (WORD) (bucket / 100);
			red = GetColorIntensity ((BYTE) (byLookup[idx][0] >> 3), (BYTE) (byLookup[idx+1][0] >> 3), perc);
			green = GetColorIntensity ((BYTE) (byLookup[idx][1] >> 3), (BYTE) (byLookup[idx+1][1] >> 3), perc);
			blue = GetColorIntensity ((BYTE) (byLookup[idx][2] >> 3), (BYTE) (byLookup[idx+1][2] >> 3), perc);
			color = (DWORD) ((((DWORD) red) << 10) | (((DWORD) green) << 5) | ((DWORD) blue));
			clrBuffer.bpp15 = (WORD) color;
			break;

		case 16:			// 5-6-5 format

			groupsize = (((DWORD) steps) * 100l) / 7l;				// Blocksize * 100
			bucket = (((DWORD) counter) * 10000l) / groupsize;		// "Bucket" * 100
			perc = (BYTE) (bucket % 100);									// Percent color intensity
			idx = (WORD) (bucket / 100);
			red = GetColorIntensity ((BYTE) (byLookup[idx][0] >> 3), (BYTE) (byLookup[idx+1][0] >> 3), perc);
			green = GetColorIntensity ((BYTE) (byLookup[idx][1] >> 2), (BYTE) (byLookup[idx+1][1] >> 2), perc);
			blue = GetColorIntensity ((BYTE) (byLookup[idx][2] >> 3), (BYTE) (byLookup[idx+1][2] >> 3), perc);
			color = (DWORD) ((((DWORD) red) << 11) | (((DWORD) green) << 5) | ((DWORD) blue));
			clrBuffer.bpp16 = (WORD) color;
			break;

		case 24:			// BGR format
		case 32:

			groupsize = (((DWORD) steps) * 100l) / 7l;				// Blocksize * 100
			bucket = (((DWORD) counter) * 10000l) / groupsize;		// "Bucket" * 100
			perc = (BYTE) (bucket % 100);									// Percent color intensity
			idx = (WORD) (bucket / 100);
			red = GetColorIntensity (byLookup[idx][0], byLookup[idx+1][0], perc);
			green = GetColorIntensity (byLookup[idx][1], byLookup[idx+1][1], perc);
			blue = GetColorIntensity (byLookup[idx][2], byLookup[idx+1][2], perc);
			clrBuffer.bpp24 = (((DWORD) red) << 16) | (((DWORD) green) << 8) | ((DWORD) blue);
			break;
	}

	return (&clrBuffer);
}

//
//		NewSetDac - Set RAMDAC to given values
//
//		Entry:	start		First RAMDAC index
//					end		Last RAMDAC index
//					lpdac		Pointer to array of RGB values
//		Exit:		None
//
void NewSetDac (BYTE start, BYTE end, LPBYTE lpdac)
{
	outp (0x3C8, start);
	while (start < end)
	{
		outp (0x3C9, *lpdac++);
		outp (0x3C9, *lpdac++);
		outp (0x3C9, *lpdac++);
		start++;
	}
}

//
//		GetColorIntensity - Return color value based on percentage of two values
//
//		Entry:	lower		Lower color value
//					upper		Upper color value
//					uperc		Percentage of color intensity (upper value)
//		Exit:		<BYTE>	New color value
//
BYTE GetColorIntensity (BYTE lower, BYTE upper, BYTE uperc)
{
	BYTE	lperc;
	WORD	wLower, wUpper;

	if (lower == upper)
		return (lower);

	lperc = (BYTE) (100 - uperc);

	wLower = (WORD) (lower * lperc);
	wUpper = (WORD) (upper * uperc);
	return ((BYTE) ((wLower + wUpper + 50) / 100));
}

//
//		Copyright (c) 1993-1998 Elpin Systems, Inc.
//		All rights reserved.
//

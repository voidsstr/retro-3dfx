//
//		VBEDRVR.CPP - VBE mode specific graphics functions for EDIAG.EXE
//		Copyright (c) 1993-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				12/8/94
//		Last modified:	5/4/98
//
//		Routines in this file:
//		HLineCGA1			Draw a horizontal line in CGA 1-color graphics mode
//		HLineCGA				Draw a horizontal line in CGA 4-color graphics mode
//		HLinePacked4		Draw a horizontal line in packed 4 BPP mode
//		HLine4				Draw a horizontal line in planar 4 BPP mode
//		HLine8				Draw a horizontal line in 8 BPP mode
//		BankHLine8			Draw a horizontal line in banked 8 BPP mode
//		HLine16				Draw a horizontal line in 15 or 16 BPP mode
//		BankHLine16			Draw a horizontal line in banked 15 or 16 BPP mode
//		HLine24				Draw a horizontal line in 24 BPP mode
//		BankHLine24			Draw a horizontal line in banked 24 BPP mode
//		VLine4				Draw a vertical line in 4 BPP mode
//		VLine8				Draw a vertical line in 8 BPP mode
//		VLine16				Draw a vertical line in 15 or 16 BPP mode
//		VLine24				Draw a vertical line in 24 BPP mode
//		WriteCharPacked4	Write a character in packed 4 BPP mode
//		WriteChar4			Write a character in planar 4 BPP mode
//		WriteChar8			Write a character in 8 BPP mode
//		BankWriteChar8		Write a character in banked 8 BPP mode
//		WriteChar16			Write a character in 15 or 16 BPP mode
//		BankWriteChar16	Write a character in banked 15 or 16 BPP mode
//		WriteChar24			Write a character in 24 BPP mode
//		BankWriteChar24	Write a character in banked 24 BPP mode
//		DrawPixel4			Draw a pixel in 4 BPP mode
//		DrawPixel8			Draw a pixel in 8 BPP mode
//		DrawPixel16			Draw a pixel in 15 or 16 BPP mode
//		DrawPixel24			Draw a pixel in 24 BPP mode
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
//		HLineCGA1 - Draw a horizontal line in CGA 1-color graphics mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
void HLineCGA1 (WORD x, WORD y, WORD wx, BYTE color)
{
	WORD		orgwx;
	WORD		offset;
	BYTE		__far *pVMem;
	BYTE		data, mask;

	if (wx == 0)
		return;

	orgwx = wx;
	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoB800);
	offset = (WORD) ((vbeModeInfo.BytesPerScanLine * (y & 0xFFFE)) / 2 + (x / 8));
	if (y & 1) offset += 0x2000;
	data = (BYTE) (color | (color << 1) | (color << 2) | (color << 3) |
					(color << 4) | (color << 5) | (color << 6) | (color << 7));

	// First byte
	mask = (BYTE) (0xFF >> (x & 7));
	*(pVMem + offset) &= ~mask;
	*(pVMem + offset) |= data & mask;
	offset++;

	// Did we run out of pixels?
	wx -= 8 - (x & 7);
	if (wx == 0)
		return;

	wx /= 8;
	if (wx)
		_fmemset (pVMem + offset, data, wx);

	// Last byte
	wx = (WORD) (((wx + x) & 7));
	if (wx)						// If no sub-byte pixels, then none to move
	{
		mask = (BYTE) (0xFF << (wx));
		*(pVMem + offset) &= ~mask;
		*(pVMem + offset) |= data & mask;
	}
}

//
//		HLineCGA - Draw a horizontal line in CGA 4-color graphics mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
void HLineCGA (WORD x, WORD y, WORD wx, BYTE color)
{
	WORD		orgwx;
	WORD		offset;
	BYTE		__far *pVMem;
	BYTE		data, mask;

	if (wx == 0) return;

	orgwx = wx;
	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoB800);
	offset = (WORD) ((vbeModeInfo.BytesPerScanLine * (y & 0xFFFE)) / 2 + (x / 4));
	if (y & 1) offset += 0x2000;
	data = (BYTE) (color | (color << 2) | (color << 4) | (color << 6));

	// First byte
	mask = (BYTE) (0xFF >> ((x & 3) * 2));
	*(pVMem + offset) &= ~mask;
	*(pVMem + offset) |= data & mask;
	offset++;

	// Did we run out of pixels?
	wx -= 4 - (x & 3);
	if (wx == 0)
		return;

	wx /= 4;
	if (wx)
		_fmemset (pVMem + offset, data, wx);

	// Last byte
	wx = (WORD) (((wx + x) & 3) * 2);
	if (wx)						// If no sub-byte pixels, then none to move
	{
		mask = (BYTE) (0xFF << (wx));
		*(pVMem + offset) &= ~mask;
		*(pVMem + offset) |= data & mask;
	}
}

//
//		HLinePacked4 - Draw a horizontal line in packed 16-color graphics mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
void HLinePacked4 (WORD x, WORD y, WORD wx, BYTE color)
{
	DWORD		offset;
	BYTE		__far *pVMem;
	BYTE		data, mask;

	if (wx == 0) return;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) + (DWORD) (x / 2);
	data = (BYTE) (color | (color << 4));

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	// First byte
	mask = (BYTE) (0xFF >> ((x & 0x01) * 4));
	*(pVMem + offset) &= ~mask;
	*(pVMem + offset) |= data & mask;
	offset++;
	if (offset >= 0x10000)
	{
		VBEIncBank ();
		offset = LOWORD (offset);
	}

	// Did we run out of pixels?
	wx -= 2 - (x & 1);
	if (wx == 0)
		return;

	wx /= 2;
	while (wx--)
	{
		*(pVMem + offset) = data;
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}

	// Last byte
	wx = (WORD) (((wx + x) & 1) * 4);
	if (wx)						// If no sub-byte pixels, then none to move
	{
		mask = (BYTE) (0xFF << (wx));
		*(pVMem + offset) &= ~mask;
		*(pVMem + offset) |= data & mask;
	}
}

#pragma optimize("",off)			// Disable optimizations that wipe out latch reads
//
//		HLine4 - Draw a horizontal line in planar 4 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
void HLine4 (WORD x, WORD y, WORD wx, BYTE color)
{
	DWORD	offset;
	WORD	length;
	BYTE	maskl, maskr;
	volatile BYTE	__far *pVMem;
	volatile BYTE	temp;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);

	//	Initialize VGA hardware
	outpw (SEQ_INDEX, 0x0F02);			// Map mask = all planes enabled
	outpw (GDC_INDEX, 0x0F01);			// Enable all planes for set/reset
	outpw (GDC_INDEX, 0x0003);			// Move data, no rotate
	outp (GDC_INDEX, 0x00);
	outp (GDC_DATA, color);				// Set color to draw
	outp (GDC_INDEX, 0x05);
	outp (GDC_DATA, inp (GDC_DATA) & 0xF4);	// Set write mode 0

	//	Calculate starting position & masks
	offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) + (x / 8);
	length = (WORD) (wx / (vbeModeInfo.XCharSize));
	maskl = (BYTE) (0xFF >> (x & 7));
	maskr = (BYTE) ~(0xFF >> ((x + wx) & 7));
	if ((x & 7) > ((x + wx) & 7))
		length++;

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	//	Draw line
	if (length == 0)
	{
		outp (GDC_INDEX, 0x08);
		outp (GDC_DATA, maskl & maskr);
		temp = *(pVMem + offset);		// Latch data...
		*(pVMem + offset) = 0xFF;		//	...and write
	}
	else
	{
		outp (GDC_INDEX, 0x08);
		outp (GDC_DATA, maskl);			// Left side
		temp = *(pVMem + offset);		// Latch data...
		*(pVMem + offset) = 0xFF;		//	...and write
		offset++;
		length--;
		outp (GDC_DATA, 0xFF);			// Rest of line
		temp = *(pVMem + offset);		// Latch data...
		while (length--)
		{
			*(pVMem + offset) = 0xFF;	//	...and write
			offset++;
			if (offset >= 0x10000)
			{
				VBEIncBank ();
				offset = LOWORD (offset);
			}
		}
		outp (GDC_DATA, maskr);			// Right side
		temp = *(pVMem + offset);		// Latch data...
		*(pVMem + offset) = 0xFF;		//	...and write
	}

	// Restore VGA to "normal" state
	outpw (GDC_INDEX, 0x0001);			// Disable all planes for set/reset
	outpw (GDC_INDEX, 0xFF08);			// Enable bit mask
}
#pragma optimize("",on)					// Re-enable optimizations

//
//		HLine8 - Draw a horizontal line in 8 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
void HLine8 (WORD x, WORD y, WORD wx, BYTE color)
{
	DWORD	offset;
	BYTE	__far *pVMem;

	if (wx == 0) return;
	if (!bLinear)
	{
		BankHLine8 (x, y, wx, color);
		return;
	}

	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) y) + (DWORD) x;
	pVMem = lpLinFrameBuffer + offset;

	while (wx--)
	{
		*(pVMem) = color;
		pVMem++;
	}
}

//
//		BankHLine8 - Draw a horizontal line in banked 8 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
void BankHLine8 (WORD x, WORD y, WORD wx, BYTE color)
{
	DWORD	offset;
	BYTE	__far *pVMem;

	if (wx == 0) return;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) y) + (DWORD) x;

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	while (wx--)
	{
		*(pVMem + offset) = color;
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

//
//		HLine16 - Draw a horizontal line in 15 or 16 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
void HLine16 (WORD x, WORD y, WORD wx, WORD color)
{
	DWORD	offset;
	BYTE	__far *pVMem;

	if (wx == 0) return;
	if (!bLinear)
	{
		BankHLine16 (x, y, wx, color);
		return;
	}

	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) y) + (DWORD) (x * 2);
	pVMem = lpLinFrameBuffer + offset;

	while (wx--)
	{
		*(LPWORD) (pVMem) = color;
		pVMem += 2;
	}
}

//
//		BankHLine16 - Draw a horizontal line in banked 15 or 16 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
void BankHLine16 (WORD x, WORD y, WORD wx, WORD color)
{
	DWORD	offset;
	BYTE	__far *pVMem;

	if (wx == 0) return;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) y) + (DWORD) (x * 2);

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	while (wx--)
	{
		*(LPWORD) (pVMem + offset) = color;
		offset += 2;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

//
//		HLine24 - Draw a horizontal line in 24 and 32 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
//		Note:	Bank switches may occur mid-pixel
//
void HLine24 (WORD x, WORD y, WORD wx, DWORD color)
{
	DWORD	offset;
	BYTE	__far *pVMem;
	int	nBytes;

	if (wx == 0) return;
	if (!bLinear)
	{
		BankHLine24 (x, y, wx, color);
		return;
	}

	nBytes = vbeModeInfo.BitsPerPixel / 8;		// Should be 3 or 4
	offset = ((DWORD) (vbeModeInfo.BytesPerScanLine) * (DWORD) y) + ((DWORD) x * nBytes);
	pVMem = lpLinFrameBuffer + offset;
#if 1
	if (nBytes == 3)
	{
		while (wx--)
		{
			*(pVMem++) = LOBYTE (LOWORD (color));
			*(pVMem++) = HIBYTE (LOWORD (color));
			*(pVMem++) = LOBYTE (HIWORD (color));
		}
	}
	else
	{
		while (wx--)
		{
			*(DWORD *)pVMem = color;
			pVMem += 4;
		}
	}
#else
	nBytes -= 3;										// Set to 0 or 1
	while (wx--)
	{
		*(pVMem++) = LOBYTE (LOWORD (color));
		*(pVMem++) = HIBYTE (LOWORD (color));
		*(pVMem++) = LOBYTE (HIWORD (color));
		pVMem += nBytes;								// Add another byte if 32 BPP
	}
#endif
}

//
//		BankHLine24 - Draw a horizontal line in banked 24 and 32 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wx			Width of line
//					color		Color of line
//		Exit:		<None>
//
//		Note:	Bank switches may occur mid-pixel
//
void BankHLine24 (WORD x, WORD y, WORD wx, DWORD color)
{
	DWORD	offset;
	BYTE	__far *pVMem;
	int	nBytes;

	if (wx == 0) return;

	nBytes = vbeModeInfo.BitsPerPixel / 8;			// Should be 3 or 4
	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = ((DWORD) (vbeModeInfo.BytesPerScanLine) * (DWORD) y) + ((DWORD) x * nBytes);

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	nBytes -= 3;											// Set to 0 or 1
	while (wx--)
	{
		*(pVMem + offset) = LOBYTE (LOWORD (color));
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
		*(pVMem + offset) = HIBYTE (LOWORD (color));
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
		*(pVMem + offset) = LOBYTE (HIWORD (color));
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
		offset += nBytes;							// Add another byte if 32 BPP
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

#pragma optimize("",off)			// Disable optimizations that wipe out latch reads
//
//		VLine4 - Draw a vertical line in 4 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wy			Height of line
//					color		Color of line
//		Exit:		<None>
//
void VLine4 (WORD x, WORD y, WORD wy, BYTE color)
{
	DWORD	offset;
	BYTE	mask;
	BYTE	__far *pVMem;
	BYTE	temp;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);

	//	Initialize VGA hardware
	outpw (SEQ_INDEX, 0x0F02);			// Map mask = all planes enabled
	outpw (GDC_INDEX, 0x0F01);			// Enable all planes for set/reset
	outpw (GDC_INDEX, 0x0003);			// Move data, no rotate
	outp (GDC_INDEX, 0x00);
	outp (GDC_DATA, color);				// Set color to draw
	outp (GDC_INDEX, 0x05);
	outp (GDC_DATA, inp (GDC_DATA) & 0xF4);	// Set write mode 0

	//	Calculate starting position & mask
	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) y) + (DWORD) (x / 8);
	mask = (BYTE) (0x80 >> (x & 7));

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	//	Draw line
	outp (GDC_INDEX, 0x08);
	outp (GDC_DATA, mask);
	while (wy--)
	{
		temp = *(pVMem + offset);		// Latch data...
		*(pVMem + offset) = 0xFF;		//	...and write
		offset += vbeModeInfo.BytesPerScanLine;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}

	// Restore VGA to "normal" state
	outpw (GDC_INDEX, 0x0001);			// Disable all planes for set/reset
	outpw (GDC_INDEX, 0xFF08);			// Enable bit mask
}
#pragma optimize("",on)					// Re-enable optimizations

//
//		VLine8 - Draw a vertical line in 8 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wy			Height of line
//					color		Color of line
//		Exit:		<None>
//
void VLine8 (WORD x, WORD y, WORD wy, BYTE color)
{
	DWORD	offset;
	BYTE	__far *pVMem;

	if (wy == 0)
		return;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) y) + (DWORD) (x);

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	while (wy--)
	{
		*(pVMem + offset) = color;

		offset += (vbeModeInfo.BytesPerScanLine);
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

//
//		VLine16 - Draw a vertical line in 15 or 16 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wy			Height of line
//					color		Color of line
//		Exit:		<None>
//
void VLine16 (WORD x, WORD y, WORD wy, WORD color)
{
	DWORD	offset;
	BYTE	__far *pVMem;

	if (wy == 0)
		return;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) y) + (DWORD) (x * 2);

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	while (wy--)
	{
		*(LPWORD) (pVMem + offset) = color;
		offset += vbeModeInfo.BytesPerScanLine;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

//
//		VLine24 - Draw a vertical line in 24 BPP mode
//
//		Entry:	x			Starting X screen coordinate
//					y			Starting Y screen coordinate
//					wy			Height of line
//					color		Color of line
//		Exit:		<None>
//
//		Note:	Bank switches may occur mid-pixel
//
void VLine24 (WORD x, WORD y, WORD wy, DWORD color)
{
	DWORD	offset;
	BYTE	__far *pVMem;
	WORD	nextline;

	if (wy == 0) return;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = ((DWORD) (vbeModeInfo.BytesPerScanLine) * (DWORD) y) + ((DWORD) x * 3);

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	nextline = (WORD) (vbeModeInfo.BytesPerScanLine - 2);
	while (wy--)
	{
		*(pVMem + offset) = LOBYTE (LOWORD (color));
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
		*(pVMem + offset) = HIBYTE (LOWORD (color));
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
		*(pVMem + offset) = LOBYTE (HIWORD (color));
		offset += nextline;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

//
//		WriteCharPacked4 - Write a character in packed 4 BPP mode
//
//		Entry:	chr	Character to draw
//		Exit:		None
//
void WriteCharPacked4 (BYTE chr)
{
	DWORD	offset, next;
	BYTE	__far *pVMem;
	WORD	ychr, xchr;
	BYTE	__far *pFontLocal;
	BYTE	scan, tmp, mask;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) (yTextRow * vbeModeInfo.YCharSize)) + (DWORD) (xTextCol * 4);
	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	pFontLocal = lpFont + (vbeModeInfo.YCharSize * chr);
	ychr = vbeModeInfo.YCharSize;
	next = vbeModeInfo.BytesPerScanLine - 4;
	while (ychr--)
	{
		scan = *pFontLocal++;

		// Draw single scan of glyph
		xchr = 4;
		while (xchr--)
		{
			tmp = mask = 0;		// Assume draw every pixel
			if (scan & 0x80)		// Draw foreground
				tmp = (BYTE) (lpclrTextFore->bpp4 << 4);
			else						// Draw background
			{
				if (bOpaque)
					tmp = (BYTE) (lpclrTextBack->bpp4 << 4);
				else
					mask = 0xF0;
			}
			if (scan & 0x40)		// Draw foreground
				tmp |= lpclrTextFore->bpp4;
			else						// Draw background
			{
				if (bOpaque)
					tmp |= lpclrTextBack->bpp4;
				else
					mask |= 0x0F;
			}
			scan <<= 2;
			*(pVMem + offset) &= mask;
			*(pVMem + offset) |= tmp;
			offset++;
		}

		// Next scan line
		offset += next;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

#pragma optimize("",off)			// Disable optimizations that wipe out latch reads
//
//		WriteChar4 - Write a character in 4 BPP mode
//
//		Entry:	chr	Character to draw
//		Exit:		None
//
//		Add opaque later - LGC
//
void WriteChar4 (BYTE chr)
{
	DWORD	offset, next;
	BYTE	__far *pVMem;
	BYTE	__far *pVMemTemp;
	WORD	ychr;
	BYTE	__far *pFontLocal;
	BYTE	scan, temp;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) (yTextRow * vbeModeInfo.YCharSize)) + (DWORD) (xTextCol);

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	pFontLocal = lpFont + (vbeModeInfo.YCharSize * chr);
	ychr = vbeModeInfo.YCharSize;
	next = vbeModeInfo.BytesPerScanLine;
	while (ychr--)
	{
		scan = *pFontLocal++;

		// Draw single scan of glyph
		outpw (SEQ_INDEX, 0x0F02);		// Enable all planes
		outpw (GDC_INDEX, 0xFF08);		// No bit mask
		pVMemTemp = pVMem + offset;
		temp = *pVMemTemp;				// Latch data
		*pVMemTemp = 0x00;				// Clear memory
		outp (GDC_INDEX, 0x08);
		outp (GDC_DATA, scan);
		outp (SEQ_INDEX, 0x02);
		outp (SEQ_DATA, lpclrTextFore->bpp4);
		temp = *pVMemTemp;				// Latch data
		*pVMemTemp = 0xFF;				// Foreground image
		outp (GDC_INDEX, 0x08);
		outp (GDC_DATA, ~scan);
		outp (SEQ_INDEX, 0x02);
		outp (SEQ_DATA, lpclrTextBack->bpp4);
		temp = *pVMemTemp;				// Latch data
		*pVMemTemp = 0xFF;				// Background image

		// Next scan line
		offset += next;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}

	outpw (SEQ_INDEX, 0x0F02);		// Enable all planes
	outpw (GDC_INDEX, 0xFF08);		// No bit mask
}
#pragma optimize("",on)					// Re-enable optimizations

//
//		WriteChar8 - Write a character in 8 BPP mode
//
//		Entry:	chr	Character to draw
//		Exit:		None
//
void WriteChar8 (BYTE chr)
{
	DWORD	offset, next;
	BYTE	__far *pVMem;
	WORD	ychr, xchr;
	BYTE	__far *pFontLocal;
	BYTE	scan;

	if (!bLinear)
	{
		BankWriteChar8 (chr);
		return;
	}

	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) (yTextRow * vbeModeInfo.YCharSize)) + (DWORD) (xTextCol * 8);
	pVMem = lpLinFrameBuffer + offset;

	pFontLocal = lpFont + (vbeModeInfo.YCharSize * chr);
	ychr = vbeModeInfo.YCharSize;
	next = vbeModeInfo.BytesPerScanLine - 8;
	while (ychr--)
	{
		scan = *pFontLocal++;

		// Draw single scan of glyph
		xchr = 8;
		while (xchr--)
		{
			if (scan & 0x80)		// Draw foreground
				*(pVMem) = lpclrTextFore->bpp8;
			else						// Draw background
			{
				if (bOpaque)
					*(pVMem) = lpclrTextBack->bpp8;
			}
			scan <<= 1;
			pVMem++;
		}
		pVMem += next;				// Next scan line
	}
}

//
//		BankWriteChar8 - Write a character in 8 BPP mode
//
//		Entry:	chr	Character to draw
//		Exit:		None
//
void BankWriteChar8 (BYTE chr)
{
	DWORD	offset, next;
	BYTE	__far *pVMem;
	WORD	ychr, xchr;
	BYTE	__far *pFontLocal;
	BYTE	scan;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) (yTextRow * vbeModeInfo.YCharSize)) + (DWORD) (xTextCol * 8);

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	pFontLocal = lpFont + (vbeModeInfo.YCharSize * chr);
	ychr = vbeModeInfo.YCharSize;
	next = vbeModeInfo.BytesPerScanLine - 8;
	while (ychr--)
	{
		scan = *pFontLocal++;

		// Draw single scan of glyph
		xchr = 8;
		while (xchr--)
		{
			if (scan & 0x80)		// Draw foreground
				*(pVMem + offset) = lpclrTextFore->bpp8;
			else						// Draw background
			{
				if (bOpaque)
					*(pVMem + offset) = lpclrTextBack->bpp8;
			}
			scan <<= 1;
			offset++;
		}

		// Next scan line
		offset += next;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

//
//		WriteChar16 - Write a character in 15 or 16 BPP mode
//
//		Entry:	chr	Character to draw
//		Exit:		None
//
void WriteChar16 (BYTE chr)
{
	DWORD	offset, next;
	BYTE	__far *pVMem;
	WORD	ychr, xchr;
	BYTE	__far *pFontLocal;
	BYTE	scan;

	if (!bLinear)
	{
		BankWriteChar16 (chr);
		return;
	}

	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) (yTextRow * vbeModeInfo.YCharSize)) + (DWORD) (xTextCol * 8 * 2);
	pVMem = lpLinFrameBuffer + offset;

	pFontLocal = lpFont + (vbeModeInfo.YCharSize * chr);
	ychr = vbeModeInfo.YCharSize;
	next = vbeModeInfo.BytesPerScanLine - 16;
	while (ychr--)
	{
		scan = *pFontLocal++;

		// Draw single scan of glyph
		xchr = 8;
		while (xchr--)
		{
			if (scan & 0x80)		// Draw foreground
				*(LPWORD) (pVMem) = lpclrTextFore->bpp16;
			else						// Draw background
			{
				if (bOpaque)
					*(LPWORD) (pVMem) = lpclrTextBack->bpp16;
			}
			scan <<= 1;
			pVMem += 2;
		}

		pVMem += next;				// Next scan line
	}
}

//
//		BankWriteChar16 - Write a character in banked 15 or 16 BPP mode
//
//		Entry:	chr	Character to draw
//		Exit:		None
//
void BankWriteChar16 (BYTE chr)
{
	DWORD	offset, next;
	BYTE	__far *pVMem;
	WORD	ychr, xchr;
	BYTE	__far *pFontLocal;
	BYTE	scan;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	offset = (((DWORD) vbeModeInfo.BytesPerScanLine) * (DWORD) (yTextRow * vbeModeInfo.YCharSize)) + (DWORD) (xTextCol * 8 * 2);

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	pFontLocal = lpFont + (vbeModeInfo.YCharSize * chr);
	ychr = vbeModeInfo.YCharSize;
	next = vbeModeInfo.BytesPerScanLine - 16;
	while (ychr--)
	{
		scan = *pFontLocal++;

		// Draw single scan of glyph
		xchr = 8;
		while (xchr--)
		{
			if (scan & 0x80)		// Draw foreground
				*(LPWORD) (pVMem + offset) = lpclrTextFore->bpp16;
			else						// Draw background
			{
				if (bOpaque)
					*(LPWORD) (pVMem + offset) = lpclrTextBack->bpp16;
			}
			scan <<= 1;

			// Next WORD could cross bank boundary
			offset += 2;
			if (offset >= 0x10000)
			{
				VBEIncBank ();
				offset = LOWORD (offset);
			}
		}

		// Next scan line
		offset += next;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

//
//		WriteChar24 - Write a character in 24 BPP mode
//
//		Entry:	chr	Character to draw
//		Exit:		None
//
void WriteChar24 (BYTE chr)
{
	DWORD	offset, next, color;
	BYTE	__far *pVMem;
	WORD	ychr, xchr;
	BYTE	__far *pFontLocal;
	BYTE	scan;
	int	nBytes;

	if (!bLinear)
	{
		BankWriteChar24 (chr);
		return;
	}

	nBytes = vbeModeInfo.BitsPerPixel / 8;			// Set to 3 or 4
	offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) (yTextRow * vbeModeInfo.YCharSize)) + ((DWORD) (xTextCol * 8)) * nBytes;
	pVMem = lpLinFrameBuffer + offset;

	pFontLocal = lpFont + (vbeModeInfo.YCharSize * chr);
	ychr = vbeModeInfo.YCharSize;
	next = vbeModeInfo.BytesPerScanLine - vbeModeInfo.BitsPerPixel;
	nBytes -= 3;											// Set to 0 or 1
	while (ychr--)
	{
		scan = *pFontLocal++;

		// Draw single scan of glyph
		xchr = 8;
		while (xchr--)
		{
			if (scan & 0x80)		// Draw foreground
				color = lpclrTextFore->bpp24;
			else						// Draw background
			{
				if (bOpaque)
					color = lpclrTextBack->bpp24;
				else
				{
					pVMem += nBytes + 3;
					scan <<= 1;
					continue;
				}
			}

			if (nBytes == 1)
			{
				*(DWORD *) pVMem = color;
				pVMem += 4;
			}
			else
			{
				*(pVMem++) = LOBYTE (LOWORD (color));
				*(pVMem++) = HIBYTE (LOWORD (color));
				*(pVMem++) = LOBYTE (HIWORD (color));
			}
			scan <<= 1;
		}
		pVMem += next;				// Next scan line
	}
}

//
//		BankWriteChar24 - Write a character in banked 24 BPP mode
//
//		Entry:	chr	Character to draw
//		Exit:		None
//
void BankWriteChar24 (BYTE chr)
{
	DWORD	offset, next, color;
	BYTE	__far *pVMem;
	WORD	ychr, xchr;
	BYTE	__far *pFontLocal;
	BYTE	scan;
	int	nBytes;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
	nBytes = vbeModeInfo.BitsPerPixel / 8;			// Set to 3 or 4
	offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) (yTextRow * vbeModeInfo.YCharSize)) + ((DWORD) (xTextCol * 8)) * nBytes;

	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	pFontLocal = lpFont + (vbeModeInfo.YCharSize * chr);
	ychr = vbeModeInfo.YCharSize;
	next = vbeModeInfo.BytesPerScanLine - vbeModeInfo.BitsPerPixel;
	nBytes -= 3;											// Set to 0 or 1
	while (ychr--)
	{
		scan = *pFontLocal++;

		// Draw single scan of glyph
		xchr = 8;
		while (xchr--)
		{
			if (scan & 0x80)		// Draw foreground
				color = lpclrTextFore->bpp24;
			else						// Draw background
			{
				if (bOpaque)
					color = lpclrTextBack->bpp24;
				else
				{
					offset = offset + nBytes + 3;
					if (offset >= 0x10000)
					{
						VBEIncBank ();
						offset = LOWORD (offset);
					}
					scan <<= 1;
					continue;
				}
			}

			*(pVMem + offset) = LOBYTE (LOWORD (color));
			offset++;				// Next pixel could cross bank boundary
			if (offset >= 0x10000)
			{
				VBEIncBank ();
				offset = LOWORD (offset);
			}
			*(pVMem + offset) = HIBYTE (LOWORD (color));
			offset++;				// Next pixel could cross bank boundary
			if (offset >= 0x10000)
			{
				VBEIncBank ();
				offset = LOWORD (offset);
			}
			*(pVMem + offset) = LOBYTE (HIWORD (color));
			offset++;				// Next pixel could cross bank boundary
			if (offset >= 0x10000)
			{
				VBEIncBank ();
				offset = LOWORD (offset);
			}
			offset += nBytes;		// Skip a byte for 32 BPP (could cross bank boundary)
			if (offset >= 0x10000)
			{
				VBEIncBank ();
				offset = LOWORD (offset);
			}
			scan <<= 1;
		}

		// Next scan line
		offset += next;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
	}
}

#pragma optimize("",off)			// Disable optimizations that wipe out latch reads
//
//		DrawPixel4 - Draw a pixel in 4 BPP mode
//
//		Entry:	x			X screen coordinate
//					y			Y screen coordinate
//					color		Color of line
//		Exit:		<None>
//
void DrawPixel4 (WORD x, WORD y, BYTE color)
{
	volatile DWORD	offset;
	volatile BYTE	__far *pVMem;
	volatile BYTE	mask;
	volatile BYTE	temp;

	pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);

	//	Calculate starting position & mask
	offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) 
			+ (DWORD) (x / vbeModeInfo.XCharSize);
	mask = (BYTE) (0x80 >> (x & 7));
//	mask = 0xff;


	VBESetBank (LOBYTE (HIWORD (offset)));
	offset = LOWORD (offset);

	outp (GDC_INDEX, 0x08);
	outp (GDC_DATA, mask);
	outpw (SEQ_INDEX, 0x0F02);	// Map mask = all planes enabled

	pVMem += offset;
	temp = *pVMem;			// Latch data
	*pVMem = 0;

	outp (SEQ_INDEX, 0x02);	 	// Set map mask to proper color
	outp (SEQ_DATA, color);
	*pVMem = 0xFF;

	outpw (SEQ_INDEX, 0x0F02);	// Restore to "normal"
	outpw (GDC_INDEX, 0xFF08);
}
#pragma optimize("",on)			// Re-enable optimizations

//
//		DrawPixel8 - Draw a pixel in 8 BPP mode
//
//		Entry:	x			X screen coordinate
//					y			Y screen coordinate
//					color		Color of line
//		Exit:		<None>
//
void DrawPixel8 (WORD x, WORD y, BYTE color)
{
	DWORD	offset;
	BYTE	__far *pVMem;

	if (bLinear)
	{
		offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) + (DWORD) x;
		*(lpLinFrameBuffer + offset) = color;
	}
	else
	{
		pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
		offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) + (DWORD) x;

		VBESetBank (LOBYTE (HIWORD (offset)));
		offset = LOWORD (offset);
		*(pVMem + offset) = color;
	}
}

//
//		DrawPixel16 - Draw a pixel in 15 or 16 BPP mode
//
//		Entry:	x			X screen coordinate
//					y			Y screen coordinate
//					color		Color of line
//		Exit:		<None>
//
void DrawPixel16 (WORD x, WORD y, WORD color)
{
	DWORD	offset;
	BYTE	__far *pVMem;

	if (bLinear)
	{
		offset = (((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) + (DWORD) (x * 2));
		*(LPWORD) (lpLinFrameBuffer + offset) = color;
	}
	else
	{
		pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
		offset = (((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) + (DWORD) (x * 2));

		VBESetBank (LOBYTE (HIWORD (offset)));
		offset = LOWORD (offset);
		*(LPWORD) (pVMem + offset) = color;
	}
}

//
//		DrawPixel24 - Draw a pixel in 24 BPP mode
//
//		Entry:	x			X screen coordinate
//					y			Y screen coordinate
//					color		Color of line
//		Exit:		<None>
//
//		Note:	Bank switches may occur mid-pixel
//
void DrawPixel24 (WORD x, WORD y, DWORD color)
{
	DWORD	offset;
	BYTE	__far *pVMem;
	int	nBytes;

	nBytes = vbeModeInfo.BitsPerPixel / 8;		// Should be 3 or 4
	if (bLinear)
	{
		offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) + ((DWORD) x) * nBytes;
		pVMem = lpLinFrameBuffer + offset;

		if (nBytes == 4)
		{
			*(DWORD *)pVMem = color;
		}
		else
		{
			*(pVMem++) = LOBYTE (LOWORD (color));
			*(pVMem++) = HIBYTE (LOWORD (color));
			*(pVMem) = LOBYTE (HIWORD (color));
		}
	}
	else
	{
		pVMem = (BYTE __far *) MK_FP (selFlat, lpVideoA000);
		offset = ((DWORD) vbeModeInfo.BytesPerScanLine * (DWORD) y) + ((DWORD) x) * nBytes;

		VBESetBank (LOBYTE (HIWORD (offset)));
		offset = LOWORD (offset);
		*(pVMem + offset) = LOBYTE (LOWORD (color));
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
		*(pVMem + offset) = HIBYTE (LOWORD (color));
		offset++;
		if (offset >= 0x10000)
		{
			VBEIncBank ();
			offset = LOWORD (offset);
		}
		*(pVMem + offset) = LOBYTE (HIWORD (color));
	}
}

//
//		Copyright (c) 1993-1998 Elpin Systems, Inc.
//		All rights reserved.
//

//
//		DISPLAY.CPP - Display interface functions for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/29/98
//		Last modified:	5/12/98
//
//		Routines in this file:
//		DrawCommonScreen		Draw the common interface screen
//		WriteBIOSChar			Write a character at a given column and row
//		VerifyScreen			Verify that the screen is as it should be
//		VideoMemoryChecksum	Do a checksum on video memory
//
#include	<stdio.h>
#include	<i86.h>
#include	<string.h>
#include	"ediag.h"

//
//		DrawCommonScreen - Draw the common interface screen
//
//		Entry:	wMode		Mode number
//					lpvbe		Pointer to VBEMODEINFOBLOCK data structure (NULL = use current)
//		Exit:		None
//
//		Assume:	vbeModeInfo has the correct values
//
void DrawCommonScreen (WORD wMode, LPVBEMODEINFOBLOCK lpvbe, int interactive)
{
	int			i, nRadius, nRadInc;
	WORD			wColumns, wRows, xRes, yRes, xMargin, yMargin, xWidth;
	WORD			colCenter, rowCenter;
	static char	szBuffer[64];
	static BYTE	__far *lpFrameB = NULL;

	/* VGA mode switches are not responsible for setting the last
         * eight palette entries, so we are going to do it right here
         */
        if (vbeModeInfo.BitsPerPixel == 8) {
 		for (i=0;i<8;i++) {
			SetDac(255-i,0,0,0);
		}
        }

	sprintf (szBuffer, "Color Depth: %d bits/pixel", vbeModeInfo.BitsPerPixel);
	// If a data structure is provided, use this one for all the
	// primitives. Otherwise, do not write over existing structure.
	if (lpvbe != NULL)
	{
		_fmemcpy (&vbeModeInfo, lpvbe, sizeof (VBEMODEINFOBLOCK));

		// Initialize the font pointer since the char height (may) have changed.
		if (vbeModeInfo.YCharSize <= 8)
			lpFont = tblFont8x8;
		else if (vbeModeInfo.YCharSize <= 14)
			lpFont = tblFont8x14;
		else
			lpFont = tblFont8x16;
	}

	if ((vbeModeInfo.ModeAttributes & 0x01) == 0) return;

	// If the mode is capable of a linear framebuffer, then use it.
	// Note that this stuff is here in case "OEMSetMode" does not call
	// back to the "SetMode" function. Also, if it is a linear mode,
	// assume that it is an extended mode and clear the screen. For the
	// size calculation on the physical to linear translation assume
	// a framebuffer of 32MB.
	bLinear = FALSE;
	if ((vbeModeInfo.ModeAttributes & 0x80) == 0x80)
	{
		bLinear = TRUE;
		// Clear framebuffer A
		lpLinFrameBuffer = (BYTE __far *) MK_FP (selFlat, Phys2Linear (vbeModeInfo.PhysBasePtr, 0x2000000));
		for (i = 0; i < CardInfo.nSizeAddr0; i += 4)
			*(DWORD *)(lpLinFrameBuffer + i) = 0;
		// Clear framebuffer B
		if (CardInfo.nSizeAddr1 != 0)
		{
			if (lpFrameB == NULL)
				lpFrameB = (BYTE __far *) MK_FP (selFlat, Phys2Linear (CardInfo.physAddr1, 0x2000000));
			for (i = 0; i < CardInfo.nSizeAddr1; i += 4)
				*(DWORD *)(lpFrameB + i) = 0;
		}
	}

	// Calculate useful values
	if ((vbeModeInfo.ModeAttributes & 0x10) == 0)
	{	// Text modes
		wColumns = vbeModeInfo.XResolution;
		wRows = (WORD) (vbeModeInfo.YResolution - 1);
	}
	else
	{	// Graphics modes
		wColumns = (WORD) (vbeModeInfo.XResolution / vbeModeInfo.XCharSize);
		wRows = (WORD) ((vbeModeInfo.YResolution / vbeModeInfo.YCharSize) - 1);
	}
	colCenter = (WORD) (wColumns / 2);
	rowCenter = (WORD) (wRows / 2);
	xRes = vbeModeInfo.XResolution;
	yRes = vbeModeInfo.YResolution;

	//	Draw a color gradient down the right side of the screen.
	xMargin = (WORD) (xRes / 32);
	yMargin = (WORD) (yRes / 32);
	xWidth = (WORD) (xRes / 4);
	DrawGradient ((WORD) (xRes - (xMargin + xWidth)), (WORD) yMargin, (WORD) (xRes - xMargin), (WORD) (yRes - yMargin));

	// Draw three horizontal lines and three vertical lines
	SetTextColorTrans (&clrWhite, &clrBlack, TRUE);
	for (i = 1; i < (wColumns - 1); i++)
	{
		VBEWriteCharAt ((WORD) i, 0, CHR_DLINE_HORZ, 0x0F);
		VBEWriteCharAt ((WORD) i, wRows, CHR_DLINE_HORZ, 0x0F);
		VBEWriteCharAt ((WORD) i, rowCenter, CHR_SLINE_HORZ, 0x0F);
	}
	for (i = 1; i < (int) wRows; i++)
	{
		VBEWriteCharAt (0, (WORD) i, CHR_DLINE_VERT, 0x0F);
		VBEWriteCharAt ((WORD) (wColumns - 1), (WORD) i, CHR_DLINE_VERT, 0x0F);
		VBEWriteCharAt (colCenter, (WORD) i, CHR_SLINE_VERT, 0x0F);
	}

	// Draw the intersection of each of the above lines
	VBEWriteCharAt (0, 0, CHR_DLINE_UL, 0x0F);
	VBEWriteCharAt ((WORD) (wColumns - 1), 0, CHR_DLINE_UR, 0x0F);
	VBEWriteCharAt (0, wRows, CHR_DLINE_LL, 0x0F);
	VBEWriteCharAt ((WORD) (wColumns - 1), wRows, CHR_DLINE_LR, 0x0F);
	VBEWriteCharAt (colCenter, rowCenter, CHR_SLINE_CROSS, 0x0F);
	VBEWriteCharAt (colCenter, 0, CHR_DLINE_SDOWNT, 0x0F);
	VBEWriteCharAt ((WORD) (wColumns - 1), rowCenter, CHR_DLINE_SLEFTT, 0x0F);
	VBEWriteCharAt (colCenter, wRows, CHR_DLINE_SUPT, 0x0F);
	VBEWriteCharAt (0, rowCenter, CHR_DLINE_SRIGHTT, 0x0F);

	// Display the current mode, resolution and other relevent data
	sprintf (szBuffer, "Mode %Xh", wMode);
	VBETextOut (2, 2, szBuffer, 0x0F);
	sprintf (szBuffer, "(%dx%d, %ld colors)", xRes, yRes, (vbeModeInfo.BitsPerPixel == 32) ? power (2, 24) : power (2, vbeModeInfo.BitsPerPixel));
	VBETextOut (2, 3, szBuffer, 0x0F);
	sprintf (szBuffer, "Mode type: %s", (vbeModeInfo.ModeAttributes & 0x10) ? "Graphics" : "Text");
	VBETextOut (2, 5, szBuffer, 0x0F);
	sprintf (szBuffer, "Color Depth: %d bits/pixel", vbeModeInfo.BitsPerPixel);
	VBETextOut (2, 6, szBuffer, 0x0F);
	sprintf (szBuffer, "Character size: %dx%d", vbeModeInfo.XCharSize, vbeModeInfo.YCharSize);
	VBETextOut (2, 7, szBuffer, 0x0F);
	sprintf (szBuffer, "CRTC Type: %s", (vbeModeInfo.ModeAttributes & 0x08) ? "Color" : "Monochrome");
	VBETextOut (2, 8, szBuffer, 0x0F);
//	VBETextOut (2, (WORD) (wRows - 1), lpMsg, 0x0F);


        if (interactive) {
	 VBETextOut (2, (WORD) (wRows - 3), "PRESS Y IF CORRECT" , 0x0F);
	 VBETextOut (2, (WORD) (wRows - 2), "      N IF NOT-CORRECT", 0x0F);
        } else {
	 VBETextOut (2, (WORD) (wRows - 3), "AUTOMATIC VIDEO OUTPUT" , 0x0F);
	 VBETextOut (2, (WORD) (wRows - 2), "CHECK...", 0x0F);
        }

	// Draw circles on top of everything
	nRadius = __min (xRes, yRes) / 3;
	nRadInc = ((__min (xRes, yRes) / 2) - nRadius) / 6;
	VBECircle (xRes / 2, yRes / 2, nRadius, &clrWhite);
	nRadius += nRadInc;
	VBECircle (xRes / 2, yRes / 2, nRadius, &clrBlue);
	nRadius += nRadInc;
	VBECircle (xRes / 2, yRes / 2, nRadius, &clrGreen);
	nRadius += nRadInc;
	VBECircle (xRes / 2, yRes / 2, nRadius, &clrRed);
	nRadius += nRadInc;
	VBECircle (xRes / 2, yRes / 2, nRadius, &clrWhite);
}

//
//		WriteBIOSChar - Write a character at a given column and row
//
//		Entry:	col		Column
//					row		Row
//					chr		Character to display
//					attr		Color of character
//		Exit:		None
//
void WriteBIOSChar (BYTE col, BYTE row, BYTE chr, BYTE attr)
{
	union REGS	regs;

	SetCursorPositionB (col, row);

	regs.h.ah = 0x09;
	regs.h.al = chr;
	regs.h.bh = 0;
	regs.h.bl = attr;
	regs.w.cx = 1;
	int386 (0x10, &regs, &regs);
}

//
//		VerifyScreen - Verify that the screen is as it should be
//
//		Entry:	wMode		Current mode number
//		Exit:		<BOOL>	Success flag (TRUE = Screen is verified, FALSE = Error)
//
BOOL VerifyScreen (WORD wMode)
{
	DWORD		dwCRCIn, dwCRCOut;
	BOOL		bSuccess;

	if (_bUseCRC) dwCRCIn = ReadCRC ();
	bSuccess = OEMVerifyScreen (&CardInfo, wMode, _bUseCRC, dwCRCIn, _bGenCRC, &dwCRCOut);
	if (_bGenCRC) SaveCRC (dwCRCOut);

	if (_bUseCRC)
	{
		if (bSuccess)
			LogComment ("\n\tDisplay verified for mode %04Xh (passed).", wMode);
		else
			LogComment ("\n\tERROR: CRC mismatch (mode %04Xh). Expected=%08Xh, Actual=%08Xh", wMode, dwCRCIn, dwCRCOut);
	}

	return (bSuccess);
}

//
//		VideoMemoryChecksum - Do a checksum on video memory
//
//		Entry:	lpVideo		Linear address into video memory
//					nPlanes		Number of planes
//					nSize			Amount of memory to check (in BYTE's)
//		Exit:		<DWORD>		Checksum (as a DWORD)
//
//		Note:	Do not assume a VGA is present unless the number of planes
//				is greater than 1.
//
DWORD VideoMemoryChecksum (BYTE __far *lpVideo, int nPlanes, long int nSize)
{
	DWORD		dwSum;
	long int	i, j;
	BYTE		bySEQIdx, byGDCIdx, bySEQ04, byGDC04, byGDC05, byGDC06;

	if (nPlanes > 1)
	{
		bySEQIdx = _inp (SEQ_INDEX);
		byGDCIdx = _inp (GDC_INDEX);

		_outp (SEQ_INDEX, 0x04);
		bySEQ04 = _inp (SEQ_DATA);
		_outp (SEQ_DATA, 0x06);

		_outp (GDC_INDEX, 0x04);
		byGDC04 = _inp (GDC_DATA);
		_outp (GDC_DATA, 0x00);

		_outp (GDC_INDEX, 0x05);
		byGDC05 = _inp (GDC_DATA);
		_outp (GDC_DATA, 0x00);

		_outp (GDC_INDEX, 0x06);
		byGDC06 = _inp (GDC_DATA);
		_outp (GDC_DATA, 0x05);
	}

	dwSum = 0;
	for (i = 0; i < nPlanes; i++)
	{
		if (nPlanes > 1)
		{
			_outp (GDC_INDEX, 0x04);
			_outp (GDC_DATA, (BYTE) i);			// Set the read map select
		}
		for (j = 0; j < nSize; j += 4)
		{
			dwSum += *(DWORD __far *) (lpVideo + j);
		}
	}

	if (nPlanes > 1)
	{
		_outp (SEQ_INDEX, 0x04);
		_outp (SEQ_DATA, bySEQ04);

		_outp (GDC_INDEX, 0x04);
		_outp (GDC_DATA, byGDC04);

		_outp (GDC_INDEX, 0x05);
		_outp (GDC_DATA, byGDC05);

		_outp (GDC_INDEX, 0x06);
		_outp (GDC_DATA, byGDC06);

		_outp (SEQ_INDEX, bySEQIdx);
		_outp (GDC_INDEX, byGDCIdx);
	}

	return (dwSum);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


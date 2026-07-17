//
//		VGA.CPP - VGA-specific test routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				5/7/98
//		Last modified:	5/7/98
//
//		Routines in this file:
//		VGAIOTest			Test the VGA I/O registers
//		VGATestRAMDAC		Test the VGA style RAMDAC
//		VGAMemSetReset		Test the VGA set/reset circuitry
//		VGAColorCompare	Test the VGA color compare circuitry
//		VGAROPs				Test the VGA rasterop and data rotation
//		VGAWriteModeOne	Test the VGA write mode one functionality
//		VGAWriteModeTwo	Test the VGA write mode two functionality
//		VGAWriteModeThree	Test the VGA write mode three functionality
//		VGABitMask			Test the VGA bit mask
//		SetLatchedBytes	Set a block of video memory after latching a data value
//
#include	<stdio.h>
#include	<i86.h>
#include	<string.h>
#include	"ediag.h"

//
//		VGAIOTest - Test the VGA I/O registers
//
//		Entry:	None
//		Exit:		None
//
BOOL VGAIOTest (void)
{
	// Though the following table is lifted, almost "as-is", straight out of
	// the Elpin Systems "VGA Core Test Suite", strict compatibility is not
	// the issue here -- functionality is. Therefore, a "don't care" field
	// has been added to this table.
	static IOTABLE iot[] = {
//	Write address	Read Address	Indexed	Idx	Mask	Rsvd	Data	!Care	Error		Exp	Act
{      DAC_MASK,	     DAC_MASK,	FALSE,	0x00,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    DAC_WINDEX,	   DAC_WINDEX,	FALSE,	0x00,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     SEQ_INDEX,	    SEQ_INDEX,	FALSE,	0x00,	0x07,	0xF8,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      SEQ_DATA,	     SEQ_DATA,	 TRUE,	0x00,	0x03,	0xFC,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      SEQ_DATA,	     SEQ_DATA,	 TRUE,	0x01,	0x3D,	0xC2,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      SEQ_DATA,	     SEQ_DATA,	 TRUE,	0x02,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      SEQ_DATA,	     SEQ_DATA,	 TRUE,	0x03,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      SEQ_DATA,	     SEQ_DATA,	 TRUE,	0x04,	0x0E,	0xF1,	0x00,	0x00,	FALSE,	0x00,	0x00},
{   CRTC_CINDEX,	  CRTC_CINDEX,	FALSE,	0x00,	0xBF,	0x40,	0x00,	0xC0,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x00,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x01,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x02,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x03,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x04,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x05,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x06,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x07,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x08,	0x7F,	0x80,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x09,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x0A,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x0B,	0x7F,	0x80,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x0C,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x0D,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x0E,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x0F,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x10,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x11,	0xCF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},		// Don't test interrupt bits here
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x12,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x13,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x14,	0x7F,	0x80,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x15,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x16,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x17,	0xEF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{    CRTC_CDATA,	   CRTC_CDATA,	 TRUE,	0x18,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     GDC_INDEX,	    GDC_INDEX,	FALSE,	0x00,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x00,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x01,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x02,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x03,	0x1F,	0xE0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x04,	0xF3,	0x0C,	0x00,	0xF0,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x05,	0xFB,	0x04,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x06,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x07,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{      GDC_DATA,	     GDC_DATA,	 TRUE,	0x08,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_INDEX,	FALSE,	0x00,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x00,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x01,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x02,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x03,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x04,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x05,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x06,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x07,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x08,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x09,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x0A,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x0B,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x0C,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x0D,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x0E,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x0F,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x10,	0xEF,	0x10,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x11,	0xFF,	0x00,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x12,	0x3F,	0xC0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x13,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{     ATC_INDEX,	    ATC_RDATA,	 TRUE,	0x34,	0x0F,	0xF0,	0x00,	0x00,	FALSE,	0x00,	0x00},
{   MISC_OUTPUT,	   MISC_INPUT,	FALSE,	0x00,	0xEF,	0x10,	0x00,	0x00,	FALSE,	0x00,	0x00},
{FEAT_CWCONTROL,	FEAT_RCONTROL,	FALSE,	0x00,	0x3B,	0xC4,	0x00,	0x00,	FALSE,	0x00,	0x00}
	};
	WORD	wIOWrt, wIORd;
	int	nTableSize, i;
	BYTE	temp;
	BOOL	bError;

	// Set a known VGA color mode
	OEMSetVGAMode (&CardInfo);
	SetMode (0x12);
	bError = FALSE;			// Assume no errors

	nTableSize = sizeof (iot) / sizeof (IOTABLE);

	// Unlock the CRTC and turn off the display
	_outp (CRTC_CINDEX, 0x11);
	_outp (CRTC_CDATA, (BYTE) (_inp (CRTC_CDATA) & 0x7F));
	_outp (SEQ_INDEX, 0x01);
	_outp (SEQ_DATA, (BYTE) (_inp (SEQ_DATA) | 0x20));

	// Clear previous errors
	for (i = 0; i < nTableSize; i++)
		iot[i].bError = FALSE;

	// Test the entire VGA I/O space
	for (i = 0; i < nTableSize; i++)
	{
		wIOWrt = iot[i].wport;
		wIORd = iot[i].rport;

		if (wIOWrt == ATC_INDEX)
		{
			ClearIObitDataBus ();				// Set index state
			if (iot[i].fIndexed)
			{
				_outp (wIOWrt, iot[i].idx);
				ClearIObitDataBus ();			// Set back to index state
			}
		}
		else if (iot[i].fIndexed)
			_outp ((WORD) (wIOWrt - 1), iot[i].idx);

		// Verify that the writeable bits are writeable
		if ((temp = IsIObitFunctional (wIORd, wIOWrt, (BYTE) ~iot[i].mask)) != 0x00)
		{
			if (((~iot[i].caremask) & temp) == temp)
			{
				iot[i].bError = TRUE;
				iot[i].byexp = 0;
				iot[i].byact = temp;
			}
		}

		// First verify that the reserved bits are indeed reserved, then
		// verify that the reserved bits are the same as the expected data.
		if (iot[i].rsvdmask)
		{
			if (wIOWrt == ATC_INDEX) ClearIObitDataBus ();
			if ((temp = IsIObitFunctional (wIORd, wIOWrt, (BYTE) ~(iot[i].rsvdmask | iot[i].mask))) != iot[i].rsvdmask)
			{
				if (((~iot[i].caremask) & temp) == temp)
				{
					iot[i].bError = TRUE;
					iot[i].byexp = iot[i].rsvdmask;
					iot[i].byact = temp;
				}
			}
			else
			{
				temp = (BYTE) (_inp (wIORd) & iot[i].rsvdmask);
				if (temp != iot[i].rsvddata)
				{
					if (((~iot[i].caremask) & temp) == temp)
					{
						iot[i].bError = TRUE;
						iot[i].byexp = iot[i].rsvddata;
						iot[i].byact = temp;
					}
				}
			}
		}
	}

	// Display the errors
	for (i = 0; i < nTableSize; i++)
	{
		if (iot[i].bError)
		{
			if (iot[i].fIndexed)
				LogComment ("\n\tERROR (VGA I/O): %04X[%02X] failed (exp=%02Xh, act=%02Xh)", iot[i].wport, iot[i].idx, iot[i].byexp, iot[i].byact);
			else
				LogComment ("\n\tERROR (VGA I/O): Write Address = %04X; Read Address = %04X failed (exp=%02Xh, act=%02Xh)", iot[i].wport, iot[i].rport, iot[i].byexp, iot[i].byact);

			// Set external error flag
			bError = iot[i].bError;
		}
	}

	SetMode (0x03);
	return (!bError);
}

//
//		VGATestRAMDAC - Test the VGA style RAMDAC
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL VGATestRAMDAC (void)
{
	static BYTE	dacShadow[256][3];
	BYTE			__far *lpVideo;
	BYTE			red, green, blue, ridx, widx;
	int			i, j;
	BOOL			bSuccess;

	OEMSetVGAMode (&CardInfo);
	SetMode (0x13);

	bSuccess = TRUE;				// Assume the best
	lpVideo = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xA0000, 0x10000));

	// Draw a series of horizontal lines
	for (i = 0; i < 200; i++)
	{
		for (j = 0; j < 320; j++)
			*lpVideo++ = (BYTE) i;
	}

	SetDacBlock (dactable, 0, 256);
	for (j = 0, i = 0; i < 256; i++, j += 3)
	{
		dacShadow[i][0] = dactable[j];
		dacShadow[i][1] = dactable[j+1];
		dacShadow[i][2] = dactable[j+2];
	}

	ridx = 2;
	widx = 1;
	for (i = 0; i < 64*4*2; i++)
	{
		for (j = 0; j < 256; j++)
		{
			// Get DAC values at this index
			GetDac (ridx, &red, &green, &blue);
			if ((dacShadow[ridx][0] != red) || (dacShadow[ridx][1] != green) || (dacShadow[ridx][2] != blue))
			{
				LogComment ("\n\tERROR: Reading RAMDAC at index %02Xh. Expected: (%02Xh,%02Xh,%02Xh) Actual: (%02Xh,%02Xh,%02Xh)",
					ridx, dacShadow[ridx][0], dacShadow[ridx][1], dacShadow[ridx][2], red, green, blue);
				bSuccess = FALSE;
				goto VGATestRAMDAC_exit;
			}

			// Set next DAC location to new values and update DAC shadow
			SetDac (widx, red, green, blue);
			dacShadow[widx][0] = red;
			dacShadow[widx][1] = green;
			dacShadow[widx][2] = blue;

			// Next location(s)
			if (++ridx == 0) ridx++;
			if (++widx == 0) widx++;
		}
	}
	LogWriteString ("\n\tVGA RAMDAC read/write test (passed).");

VGATestRAMDAC_exit:
	return (bSuccess);
}

//
//		VGAMemSetReset - Test the VGA set/reset circuitry
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL VGAMemSetReset (void)
{
#define	COUNT_SETRESET				16
#define	COUNT_SETRESETENABLE		16
	BOOL	bSuccess;
	BYTE	byMemData, byExpected, byActual, byTemp;
	volatile BYTE	__far *lpVideo;
	int	i, j, k, n;

	bSuccess = TRUE;				// Assume the best
	lpVideo = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xA0000, 0x10000));
	SetMode (0x12);

	// Fill memory with a non-zero, non-FFh value
	byMemData = 0x55;
	for (i = 0; i < COUNT_SETRESET*COUNT_SETRESETENABLE; i++)
		*(lpVideo + i) = byMemData;

	// Write each possible combination of Set/Reset and Set/Reset Enable
	n = 0;
	for (i = 0; i < COUNT_SETRESET; i++)				// Cycle through each Set/Reset
	{
		_outp (GDC_INDEX, 0x00);
		_outp (GDC_DATA, (BYTE) i);
		for (j = 0; j < COUNT_SETRESETENABLE; j++)	// Cycle through each Enable
		{
			_outp (GDC_INDEX, 0x01);
			_outp (GDC_DATA, (BYTE) j);
			byTemp = *(lpVideo + n);						// Load latches
			*(lpVideo + n) = 0;
			n++;
		}
	}
	n = byTemp;	// Prevent the C optimizer from discarding the load latch line

	// Verify the data
	_outp (GDC_INDEX, 0x04);
	n = 0;
	for (i = 0; i < COUNT_SETRESET; i++)				// Cycle through each Set/Reset
	{
		for (j = 0; j < COUNT_SETRESETENABLE; j++)	// Cycle through each enable
		{
			for (k = 0; k < 4; k++)							// Cycle through each plane
			{
				_outp (GDC_DATA, (BYTE) k);
				// Calculate enable
				byExpected = (j & (0x01 << k)) == 0 ? 0x00 : 0xFF;
				// Calculate memory byte
				byExpected = (BYTE) ((i & (0x01 << k)) == 0 ? 0x00 : 0xFF) & byExpected;
				byActual = *(lpVideo + n);
				if (byActual != byExpected)
				{
					bSuccess = FALSE;
					LogComment ("\n\tERROR: VGA Set/Reset Test failed."
									"\n\t\tAddress = %08Xh"
									"\n\t\tGDC[0] = %02Xh, GDC[1] = %02Xh, Plane = %d"
									"\n\t\tExpected = %02Xh, Actual = %02Xh",
									FP_OFF (lpVideo + n), i, j, k, byExpected, byActual);
					goto VGAMemSetReset_exit;
				}
			}
			n++;
		}
	}

VGAMemSetReset_exit:
	return (bSuccess);
}

//
//		VGAColorCompare - Test the VGA color compare circuitry
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL VGAColorCompare (void)
{
	static BYTE abyExpected[80] = {
		0xFF, 0xFF, 0xFF, 0x55, 0xAA, 0x55, 0x33, 0x33,
		0xCC, 0x44, 0x22, 0x11, 0x0F, 0x0F, 0x0F, 0x05,
		0xA0, 0x50, 0x30, 0x30, 0x0C, 0x04, 0x02, 0x01,
		0x00, 0xFF, 0x00, 0x55, 0x00, 0x55, 0x00, 0x33,
		0xCC, 0x00, 0x22, 0x00, 0x0F, 0x00, 0x0F, 0x00,
		0x00, 0x50, 0x00, 0x30, 0x00, 0x04, 0x00, 0x01,
		0xFF, 0xFF, 0xFF, 0x55, 0xAA, 0x55, 0x33, 0x33,
		0xCC, 0x44, 0x22, 0x11, 0x0F, 0x0F, 0x0F, 0x05,
		0xA0, 0x50, 0x30, 0x30, 0x0C, 0x04, 0x02, 0x01,
		0x00, 0xFF, 0x00, 0x55, 0x00, 0x55, 0x00, 0x33
	};
	BOOL	bSuccess;
	BYTE	__far *lpVideo;
	WORD	wOffset;
	BYTE	byColor, byMask, tmp, byActual;
	int	i, j;

	bSuccess = TRUE;				// Assume the best
	lpVideo = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xA0000, 0x10000));
	SetMode (0x12);

	wOffset = (640/8)*(480/2);				// Half way down the screen
	byColor = 0;
	for (i = 0; i < 80; i++)
	{
		byMask = 0x01;
		for (j = 0; j < 8; j++)
		{
			byMask = RotateByteRight (byMask, 1);
			VGASetPixelAt (lpVideo + wOffset + i, byMask, byColor);
			byColor = (BYTE) ((byColor + 1) % 16);
		}
	}

	_outp (GDC_INDEX, 0x05);
	tmp = _inp (GDC_DATA);
	_outp (GDC_DATA, (BYTE) (tmp | 0x08));		// Set read mode 1
	for (i = 0; i < 80; i++)
	{
		_outp (GDC_INDEX, 0x07);					// Color don't care
		_outp (GDC_DATA, (BYTE) ((i/3) % 16));	// Cycle colors on every third pass
		_outp (GDC_INDEX, 0x02);					// Color compare
		_outp (GDC_DATA, (BYTE) (i % 16));		// Cycle through colors
		byActual = *(lpVideo + wOffset + i);
		if (byActual != abyExpected[i])
		{
			bSuccess = FALSE;
			LogComment ("\n\tERROR: VGA color compare failed."
							"\n\t\tAddress = %08Xh"
							"\n\t\tExpected = %02Xh, Actual = %02Xh",
							FP_OFF (lpVideo + wOffset + i), abyExpected[i], byActual);
			goto VGAColorCompare_exit;
		}
	}

VGAColorCompare_exit:
	return (bSuccess);
}

//
//		VGAROPs - Test the VGA rasterop and data rotation
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL VGAROPs (void)
{
	BOOL	bSuccess;
	BYTE	__far *lpVideo;
	WORD	wOffset;
	int	i, j;
	BYTE	byRop, byRotate, byData, byBefore[4], byExpected, byActual;

	bSuccess = TRUE;						// Assume the best
	lpVideo = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xA0000, 0x10000));
	SetMode (0x12);

	_outpw (GDC_INDEX, 0x0F01);		// Enable set/reset
	wOffset = 0;
	for (i = 0; i < 16; i++)
	{
		_outp (GDC_INDEX, 0x00);
		_outp (GDC_DATA, (BYTE) i);
		for (j = 0; j < 256; j++)
		{
			*(lpVideo + wOffset) = 0;
			wOffset++;
		}
	}
	_outpw (GDC_INDEX, 0x0001);		// Disable set/reset

	// Go through every byte on the display
	wOffset = 0;
	byRop = byRotate = byData = 0;
	while (wOffset < (WORD) (16*256))
	{
		_outp (GDC_INDEX, 0x03);
		_outp (GDC_DATA, (BYTE) ((byRop << 3) | byRotate));	// Set ROP and rotate value'

		// Get video memory needed for diagnostic (latches data as well)
		_outp (GDC_INDEX, 0x04);
		for (i = 0; i < 4; i++)
		{
			_outp (GDC_DATA, (BYTE) i);
			byBefore[i] = *(lpVideo + wOffset);
		}

		// Write to memory
		*(lpVideo + wOffset) = byData;

		// Get the results of the operation
		for (i = 0; i < 4; i++)
		{
			_outp (GDC_DATA, (BYTE) i);
			byExpected = RotateByteRight (byData, byRotate);
			switch (byRop)
			{
				case 1:
					byExpected &= byBefore[i];
					break;
				case 2:
					byExpected |= byBefore[i];
					break;
				case 3:
					byExpected ^= byBefore[i];
					break;
			}
			byActual = *(lpVideo + wOffset);
			if (byExpected != byActual)
			{
				bSuccess = FALSE;
				LogComment ("\n\tERROR: VGA rasterops and data rotation failed."
								"\n\t\tAddress = %08Xh"
								"\n\t\tROP = %02Xh, Rotate = %02Xh, Data = %02Xh, Plane = %d"
								"\n\t\tExpected = %02Xh, Actual = %02Xh",
								FP_OFF (lpVideo + wOffset), byRop, byRotate, byData, i, byExpected, byActual);
				goto VGAROPs_exit;
			}
		}

		wOffset++;
		byRop = (BYTE) ((byRop + 1) & 0x03);
		byRotate = (BYTE) ((byRotate + 1) & 0x7);
		byData++;								// Let byte wrap around
	}

VGAROPs_exit:
	return (bSuccess);
}

//
//		VGAWriteModeOne - Test the VGA write mode one functionality
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL VGAWriteModeOne (void)
{
	BOOL	bSuccess;
	int	i, dest, nLength;
	BYTE	__far *lpVideo;
	WORD	j, wScanSize;
	BYTE	byExpected, byActual;
	BYTE	byData, byIncr;

	bSuccess = TRUE;				// Assume the best
	lpVideo = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xA0000, 0x10000));
	wScanSize = 120;
	SetMode (0x12);

	// Draw a pattern into memory
	byData = 0;
	byIncr = 1;
	for (i = 0; i < 640; i++)
	{
		VLine4 ((WORD) i, 0, (WORD) (wScanSize - 1), byData);
		// Don't use a regular pattern
		if ((++byData & 0x0F) == 0)
		{
			byData += byIncr;
			byIncr++;
		}
	}

	_outpw (GDC_INDEX, 0x0105);			// Write mode 1
	_outpw (SEQ_INDEX, 0x0F02);			// Enable all planes

	// Move "wScanSize" scan lines
	dest = wScanSize*80;						// Nth scan line
	nLength = wScanSize*80;					// Move N scan lines
	MoveBytes (lpVideo + dest, lpVideo, nLength);

	// Set bit mask and do it again
	_outpw (GDC_INDEX, 0x5508);			// Bit mask
	dest = (wScanSize*2)*80;
	MoveBytes (lpVideo + dest, lpVideo, nLength);

	// Set map mask and do it again
	_outpw (GDC_INDEX, 0xFF08);			// Bit mask
	_outpw (SEQ_INDEX, 0x0302);			// Map mask
	dest = (wScanSize*3)*80;
	MoveBytes (lpVideo + dest, lpVideo, nLength);

	// Read back memory and verify that it is what is expected
	for (i = 0; i < 4; i++)
	{
		_outp (GDC_INDEX, 0x04);
		_outp (GDC_DATA, (BYTE) i);		// Read map select
		dest = wScanSize*80;					// Nth scan line
		for (j = 0; j < nLength; j++)
		{
			byExpected = *(lpVideo + j);
			byActual = *(lpVideo + dest + j);
			if (byExpected != byActual) goto VGAWriteModeOne_error;
		}

		dest = (wScanSize*2)*80;			// (N*2)th scan line
		for (j = 0; j < nLength; j++)
		{
			byExpected = *(lpVideo + j);
			byActual = *(lpVideo + dest + j);
			if (byExpected != byActual) goto VGAWriteModeOne_error;
		}

		// Plane 0 & 1 only should compare
		dest = (wScanSize*3)*80;		// (N*3)th scan line
		if ((i == 0) || (i == 1))
		{
			for (j = 0; j < nLength; j++)
			{
				byExpected = *(lpVideo + j);
				byActual = *(lpVideo + dest + j);
				if (byExpected != byActual) goto VGAWriteModeOne_error;
			}
		}
		else
		{
			byExpected = 0;
			for (j = 0; j < nLength; j++)
			{
				byActual = *(lpVideo + dest + j);
				if (byExpected != byActual) goto VGAWriteModeOne_error;
			}
		}
	}

	return (bSuccess);

VGAWriteModeOne_error:
	LogComment ("\n\tERROR: VGA write mode one failed."
					"\n\t\tAddress = %08Xh"
					"\n\t\tbyExpected = %02Xh, byActual = %02Xh",
					FP_OFF (lpVideo + dest + j), byExpected, byActual);
	return (FALSE);
}

//
//		VGAWriteModeTwo - Test the VGA write mode two functionality
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL VGAWriteModeTwo (void)
{
	BOOL	bSuccess;
	BYTE	__far *lpVideo;
	int	i, j, n;
	BYTE	byROP, byRotate, byData, byCPU, byExpected, byActual;
	BYTE	byMem;
	int	nBlockSize, nColSize;

	bSuccess = TRUE;				// Assume the best
	lpVideo = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xA0000, 0x10000));
	nBlockSize = 80;
	nColSize = (480/16);
	SetMode (0x12);

	_outp (GDC_INDEX, 0x05);
	_outp (GDC_DATA, (BYTE) (_inp (GDC_DATA) | 2));	// Write mode 2

	n = 0;
	for (i = 0; i < 16; i++)								// Cycle through each color
	{
		for (j = 0; j < nColSize*nBlockSize; j++)
		{
			*(lpVideo + n) = (BYTE) i;
			n++;
		}
	}

	byData = byROP = byRotate = 0;
	n = 0;
	for (i = 0; i < nBlockSize*16; i++)
	{
		_outp (GDC_INDEX, 0x03);
		_outp (GDC_DATA, (BYTE) (byRotate | (byROP << 3)));
		SetLatchedBytes (lpVideo + n, byData, nColSize);

		for (j = 0; j < 4; j++)							// Cycle through each plane
		{
			_outpw (GDC_INDEX, (WORD) ((j << 8) | 0x04));

			// Calculate video memory byte
			byMem = 0;
			if ((i / nBlockSize) & (1 << j))
				byMem = 0xFF;

			// Calculate CPU byte
			byCPU = 0;
			if (byData & (1 << j))
				byCPU = 0xFF;

			byExpected = byCPU;
			switch (byROP)
			{
				case 1:
					byExpected &= byMem;
					break;
				case 2:
					byExpected |= byMem;
					break;
				case 3:
					byExpected ^= byMem;
					break;
			}
			for (j = 0; j < nColSize; j++)
			{
				byActual = *(lpVideo + j + n);
				if (byExpected != byActual)
				{
					bSuccess = FALSE;
					LogComment ("\n\tERROR: VGA write mode two failed."
									"\n\t\tAddress = %08Xh"
									"\n\t\tExpected = %02Xh, Actual = %02Xh",
									FP_OFF (lpVideo + j + n), byExpected, byActual);
					goto VGAWriteModeTwo_exit;
				}
			}
		}

		n += nColSize;
		byData++;
		byROP = (BYTE) ((byROP + 1) & 0x03);
		byRotate = (BYTE) ((byRotate + 1) & 0x07);
	}

VGAWriteModeTwo_exit:
	return (bSuccess);
}

//
//		VGAWriteModeThree - Test the VGA write mode three functionality
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL VGAWriteModeThree (void)
{
#define		COUNT_BITMASKS			2
#define		CHR_INCER_1				3
#define		CHR_INCER_2				7
	BOOL	bSuccess;
	BYTE	__far *lpVideo;
	int	i, j, k, l, n, nCountChars;
	BYTE	byMemData, byCPUData, byExpected, byActual, byBitMask, temp;
	BYTE	tblBitMasks[COUNT_BITMASKS] = {0x55, 0xFF};

	bSuccess = TRUE;				// Assume the best
	lpVideo = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xA0000, 0x10000));
	nCountChars = 256;
	SetMode (0x12);

	// Fill memory with various data combinations
	byMemData = 0x00;
	_outp (SEQ_INDEX, 0x02);
	n = 0;
	for (i = 0; i < 16*nCountChars*COUNT_BITMASKS; i++)
	{
		for (j = 0; j < 4; j++)
		{
			_outp (SEQ_DATA, (BYTE) (0x01 << j));
			*(lpVideo + n) = byMemData;
			byMemData += CHR_INCER_1;
		}
		n++;
	}

	_outpw (SEQ_INDEX, 0x0F02);

	// Set write mode 3 and draw a combination of data into memory
	_outpw (GDC_INDEX, 0x0305);
	byCPUData = 0x00;
	n = 0;
	for (i = 0; i < nCountChars; i++)
	{
		for (j = 0; j < COUNT_BITMASKS; j++)
		{
			_outpw (GDC_INDEX, (WORD) ((tblBitMasks[j] << 8) | 0x08));
			_outp (GDC_INDEX, 0);
			for (k = 0; k < 16; k++)
			{
				_outp (GDC_DATA, (BYTE) k);
				temp = *(lpVideo + n);						// Load latches
				*(lpVideo + n) = byCPUData;
				n++;
			}
		}
		byCPUData += CHR_INCER_2;
	}

	// Verify video data
	byMemData = 0x00;
	byCPUData = 0x00;
	_outp (GDC_INDEX, 0x04);
	n = 0;
	for (i = 0; i < nCountChars; i++)			// Cycle through CPU data
	{
		for (j = 0; j < COUNT_BITMASKS; j++)	// Cycle through bitmasks
		{
			for (k = 0; k < 16; k++)				// Cycle through Set/Reset data
			{
				for (l = 0; l < 4; l++)				// Cycle through each plane
				{
					_outp (GDC_DATA, (BYTE) l);
					// Calculate Set/Reset data
					byExpected = ((k & (1 << l)) == 0) ? 0x00 : 0xFF;
					byBitMask = byCPUData & tblBitMasks[j];
					byExpected = (BYTE) ((byExpected & byBitMask) | (byMemData & ~byBitMask));
					byActual = *(lpVideo + n);
					if (byExpected != byActual)
					{
						bSuccess = FALSE;
						LogComment ("\n\tERROR: VGA write mode three failed."
										"\n\t\tAddress = %08Xh"
										"\n\t\tCPU Data = %02Xh, Bit Mask = %02Xh, Set/Reset = %02Xh, Plane = %02Xh"
										"\n\t\tExpected = %02Xh, Actual = %02Xh",
										FP_OFF (lpVideo + n), byCPUData, tblBitMasks[j], k, l, byExpected, byActual);
						goto VGAWriteModeThree_exit;
					}
					byMemData += CHR_INCER_1;
				}
				n++;
			}
		}
		byCPUData += CHR_INCER_2;
	}

VGAWriteModeThree_exit:
	return (bSuccess);
}

//
//		VGABitMask - Test the VGA bit mask
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL VGABitMask (void)
{
	BOOL	bSuccess;
	int	i, j, nBlockSize, nColSize;
	BYTE	__far *lpVideo;
	BYTE	byMask, byChr, byData, byExpected, byActual, bySR, temp;
	WORD	wOffset, n;

	bSuccess = TRUE;				// Assume the best
	lpVideo = (BYTE __far *) MK_FP (selFlat, Phys2Linear (0xA0000, 0x10000));
	nBlockSize = (480/16);
	nColSize = 80;
	SetMode (0x12);

	// Fill memory with a pattern
	_outpw (GDC_INDEX, 0x0F01);			// Enable set/reset
	for (i = 0; i < 16; i++)
	{
		_outp (GDC_INDEX, 0x00);
		_outp (GDC_DATA, (BYTE) i);
		wOffset = (WORD) (i*nColSize*nBlockSize);
		for (j = 0; j < nColSize*nBlockSize; j++)
			*(lpVideo + wOffset++) = 0;
	}

	_outpw (GDC_INDEX, 0x0001);			// Disable set/reset

	// Cycle through all combinations of bit masks
	byMask = 0;
	byChr = 0xFF;
	n = 0;
	while ((WORD) n < (WORD) (nColSize*nBlockSize*16))
	{
		_outp (GDC_INDEX, 0x08);
		_outp (GDC_DATA, byMask);
		byMask++;
		temp = *lpVideo;					// Load latches
		*(lpVideo + n) = byChr;
		n++;
	}

	// Read all of memory and verify that it is correct
	byMask = 0;
	n = 0;
	while ((WORD) n < (WORD) (nColSize*nBlockSize*16))
	{
		// Calculate the set/reset data
		bySR = (BYTE) ((LOWORD (lpVideo)) / ((WORD) (nColSize*nBlockSize)));

		// Read each plane
		for (i = 0; i < 4; i++)
		{
			// Calculate the video data on this plane
			byData = (BYTE) (bySR & (1 << i));
			if (byData != 0) byData = 0xFF;

			// Calculate the new video data after the CPU write
			byExpected = (BYTE) ((byData & ~byMask) | (byChr & byMask));

			// Set read map select and get the actual data
			_outp (GDC_INDEX, 0x04);
			_outp (GDC_DATA, (BYTE) i);
			byActual = *(lpVideo + n);
			if (byExpected != byActual)
			{
				bSuccess = FALSE;
				LogComment ("\n\tERROR: VGA bit mask failed."
								"\n\t\tAddress = %08Xh"
								"\n\t\tMask = %02Xh, Expected = %02Xh, Actual = %02Xh",
								FP_OFF (lpVideo + n), byMask, byExpected, byActual);
				goto VGABitMask_exit;
			}
		}
		byMask++;
		n++;
	}

VGABitMask_exit:
	return (bSuccess);
}

//
//		SetLatchedBytes - Set a block of video memory after latching a data value
//
//		Entry:	lpDest	Pointer to destination
//					byData	CPU data value
//					nLength	Number of bytes to move
//		Exit:		None
//
//		Note:	Due to the way VGA latches work, only one BYTE at at time can
//				be moved into video memory. Therefore, the "C" routine, "_fmemset"
//				is useless since it doesn't do the latch load step and is also
//				optimized for speed which may use WORD or DWORD moves.
//
void SetLatchedBytes (BYTE __far *lpDest, BYTE byData, int nLength)
{
	int	i;
	BYTE	temp;

	for (i = 0; i < nLength; i++)
	{
		temp = *lpDest;					// Load VGA latches
		*lpDest++ = byData;
	}
	i = temp;	// Hopefully this prevents the C optimizer from wiping out the latch read
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


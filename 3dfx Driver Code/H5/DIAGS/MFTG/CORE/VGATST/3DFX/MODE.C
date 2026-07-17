//
//		MODE.CPP - OEM specific mode routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/30/98
//		Last Modified:	5/13/98
//
//		Routines in this file:
//		OEMVerifyScreen		Verify that the displayed screen is correct
//		OEMSetMode				Set an OEM mode
//		OEMSetResolution		Set an OEM mode of a given resolution
//		LookupModeNumber		Given a resolution, return an OEM mode number
//		GetCRC					Generate a CRC for one frame
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include "ediag.h"
#include "oem.h"

//
//		OEMVerifyScreen - Verify that the displayed screen is correct
//
//		Entry:	lpci			Pointer to CARDINFO data structure
//					wMode			Mode number
//					bUseCRC		Use the CRC for checking
//					dwCRCIn		CRC input value
//					bGenCRC		Generate the CRC value, don't check against it
//					lpdwCRCOut	Pointer to CRC value (returned)
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
BOOL OEMVerifyScreen (LPCARDINFO lpci, WORD wMode, BOOL bUseCRC, DWORD dwCRCIn, BOOL bGenCRC, LPDWORD lpdwCRCOut)
{
	BOOL	bSuccess;

	bSuccess = TRUE;		// Assume the best
	if ((bGenCRC) || (bUseCRC))
	{
		*lpdwCRCOut = GetCRC (lpci, wMode);
		if (bUseCRC) bSuccess = (dwCRCIn == *lpdwCRCOut);
	}
	else
	{
	//	if (GetKey () == KEY_ESCAPE) bSuccess = FALSE;
	}
	return (bSuccess);
}

//
//		OEMSetMode - Set an OEM mode
//
//		Entry:	lpci					Pointer to CARDINFO data structure
//					wMode					OEM Mode number
//		Exit:		<LPVBEMODEINFO>	Pointer to VBEMODEINFO data structure (NULL = failure)
//
LPVBEMODEINFOBLOCK OEMSetMode (LPCARDINFO lpci, WORD wMode)
{
	lpci = lpci;				// Prevent compiler warning
	SetMode (wMode);			// This will set the VESA mode
	if (!GetVBEModeInfo (&vbemiBanshee, wMode))
		return (NULL);
	return (&vbemiBanshee);
}

//
//		OEMSetResolution - Set an OEM mode of a given resolution
//
//		Entry:	lpci					Pointer to CARDINFO data structure
//					wXRes					X Resolution
//					wYRes					Y Resolution
//					wBPP					Bits per pixel
//					wRate					Refresh rate
//					lpMode				If found, mode number is returned here
//		Exit:		<LPVBEMODEINFO>	Pointer to VBEMODEINFO data structure (NULL = failure)
//
LPVBEMODEINFOBLOCK OEMSetResolution (LPCARDINFO lpci, WORD wXRes, WORD wYRes, WORD wBPP, WORD wRate, LPWORD lpMode)
{
	LPVBEMODEINFOBLOCK	lpvbe;

	lpci = lpci;				// Prevent compiler warning
	*lpMode = LookupModeNumber (wXRes, wYRes, wBPP, wRate);
	if (*lpMode == 0xFFFF)
		return (FALSE);

	lpvbe = OEMSetMode (lpci, *lpMode);

	// set refresh rate goes here - LGC

	return (lpvbe);
}

//
//		LookupModeNumber - Given a resolution, return an OEM mode number
//
//		Entry:	wXRes		X Resolution
//					wYRes		Y Resolution
//					wBPP		Bits per pixel
//					wRate		Refresh rate
//		Exit:		<WORD>	Mode number (0FFFFh = Does not exist)
//
WORD LookupModeNumber (WORD wXRes, WORD wYRes, WORD wBPP, WORD wRate)
{
	LPWORD	lpModes=0;

	wRate = wRate;			// Prevent compiler warning
	while (*lpModes != 0xFFFF)
	{
		if (GetVBEModeInfo (&vbemiBanshee, *lpModes))
		{
			if ((vbemiBanshee.XResolution == wXRes) &&
					(vbemiBanshee.YResolution == wYRes) &&
					(vbemiBanshee.BitsPerPixel == wBPP))
				break;
		}
		lpModes++;
	}
	return (*lpModes);
}

//
//		GetCRC - Generate a CRC for one frame
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//					wMode		VBE style mode number
//		Exit:		<DWORD>	Generated CRC value
//
//		Note:	Since this RAMDAC does not create a CRC, the best that
//				can be done is to do a checksum on memory.
//
DWORD GetCRC (LPCARDINFO lpci, WORD wMode)
{
	DWORD			dwCRC;
	static BYTE	__far *lpVideo = NULL;
	static BYTE __far *lpLinear = NULL;

	lpci = lpci;			// Prevent compiler warnings

	if (lpVideo == NULL)
		lpVideo = (BYTE __far *) MK_FP (wSelFlat, Phys2Linear (0xA0000, 0x10000));

	if ((wMode < 0x100) || (wMode == 0x102) || ((wMode >= 0x108) && (wMode <= 0x10C)))
	{
		dwCRC = VideoMemoryChecksum (lpVideo, 4, 64*1024);
	}
	else
	{
		if (lpLinear == NULL)
			lpLinear = (BYTE __far *) MK_FP (wSelFlat, Phys2Linear (lpci->physAddr0, lpci->nSizeAddr0));
		dwCRC = VideoMemoryChecksum (lpLinear, 1, lpci->nSizeAddr0);
	}

	return (dwCRC);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//

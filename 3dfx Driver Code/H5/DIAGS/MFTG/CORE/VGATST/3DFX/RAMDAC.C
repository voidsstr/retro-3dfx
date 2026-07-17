//
//		RAMDAC.CPP - OEM specific RAMDAC test routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				5/8/98
//		Last Modified:	5/13/98
//
//		Routines in this file:
//		OEMTestRAMDAC			Test the native mode RAMDAC
//		OEMTestPLL				Test the RAMDAC's PLL
//		OEMTestHWCursor		Test the RAMDAC's hardware cursor
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include "ediag.h"
#include "oem.h"

//
//		OEMTestRAMDAC - Test the native mode RAMDAC
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//		Exit:		<BOOL>	Success flag (TRUE = Successful test, FALSE = Not)
//
BOOL OEMTestRAMDAC (LPCARDINFO lpci)
{
	lpci = lpci;			// Prevent compiler warning
	return (TRUE);
}

//
//		OEMTestPLL - Test the RAMDAC's PLL
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//		Exit:		<BOOL>	Success flag (TRUE = Successful test, FALSE = Not)
//
BOOL OEMTestPLL (LPCARDINFO lpci)
{
	lpci = lpci;			// Prevent compiler warning
	return (TRUE);
}

//
//		OEMTestHWCursor - Test the RAMDAC's hardware cursor
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//		Exit:		<BOOL>	Success flag (TRUE = Successful test, FALSE = Not)
//
BOOL OEMTestHWCursor (LPCARDINFO lpci)
{
	lpci = lpci;			// Prevent compiler warning
	return (TRUE);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//

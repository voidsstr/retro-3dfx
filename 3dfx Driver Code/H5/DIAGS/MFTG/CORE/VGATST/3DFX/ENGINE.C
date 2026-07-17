//
//		ENGINE.CPP - OEM specific engine test routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				5/7/98
//		Last Modified:	5/13/98
//
//		Routines in this file:
//		OEMPreMemoryTest		Prepare for memory test
//		OEMTestAccel2D			Test 2D acceleration
//		OEMTestBitBlt			Test specific BitBlt capabilities
//		OEMTestAccel3D			Test 3D acceleration
//		OEMTestDMA				Test DMA channel functionality
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include "ediag.h"
#include "oem.h"

//
//		OEMPreMemoryTest - Prepare for memory test
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//					nRegion	Region ID
//		Exit:		None
//
//		Assume native mode has been set.
//
void OEMPreMemoryTest (LPCARDINFO lpci, int nRegion)
{
	lpci = lpci;
	nRegion = nRegion;	// Prevent comipler warning
}

//
//		OEMTestAccel2D - Test 2D acceleration
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//		Exit:		<BOOL>	Success flag (TRUE = Successful test, FALSE = Not)
//
BOOL OEMTestAccel2D (LPCARDINFO lpci)
{
	lpci = lpci;			// Prevent compiler warning
	return (TRUE);
}

//
//		OEMTestBitBlt - Test specific BitBlt capabilities
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//		Exit:		<BOOL>	Success flag (TRUE = Successful test, FALSE = Not)
//
BOOL OEMTestBitBlt (LPCARDINFO lpci)
{
	lpci = lpci;			// Prevent compiler warning
	return (TRUE);
}

//
//		OEMTestAccel3D - Test 3D acceleration
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//		Exit:		<BOOL>	Success flag (TRUE = Successful test, FALSE = Not)
//
BOOL OEMTestAccel3D (LPCARDINFO lpci)
{
	lpci = lpci;			// Prevent compiler warning
	return (TRUE);
}

//
//		OEMTestDMA - Test DMA channel functionality
//
//		Entry:	lpci		Pointer to CARDDATA info structure
//		Exit:		<BOOL>	Success flag (TRUE = Successful test, FALSE = Not)
//
BOOL OEMTestDMA (LPCARDINFO lpci)
{
	lpci = lpci;			// Prevent compiler warning
	return (TRUE);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//

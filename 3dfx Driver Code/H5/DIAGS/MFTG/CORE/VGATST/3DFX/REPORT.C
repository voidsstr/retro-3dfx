//
//		REPORT.CPP - OEM specific reporting routines required by REPDEV.LIB
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/17/98
//		Last Modified:	4/17/98
//
//		Routines in this file:
//		OpenOEMReportDevice			Initialize OEM reporting device
//		CloseOEMReportDevice			Terminate OEM reporting device
//		WriteStringOEMReportDevice	Write a string to the OEM reporting device
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include "ediag.h"

//
//		OpenOEMReportDevice - Initialize OEM reporting device
//
//		Entry:	None
//		Exit:		<BOOL>	Success flag (TRUE = Opened, FALSE = Device failed)
//
BOOL OpenOEMReportDevice (void)
{
	return (FALSE);
};

//
//		CloseOEMReportDevice - Terminate OEM reporting device
//
//		Entry:	None
//		Exit:		<BOOL>		Success flag (TRUE = Closed, FALSE = Device failed)
//
BOOL CloseOEMReportDevice (void)
{
	return (FALSE);
}

//
//		WriteStringOEMReportDevice - Write a string to the OEM reporting device
//
//		Entry:	lpstr		Pointer to string to write
//		Exit:		<BOOL>	Success flag (TRUE = Written, FALSE = Device failed)
//
BOOL WriteStringOEMReportDevice (LPSTR lpstr)
{
	lpstr = lpstr;
	return (FALSE);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//

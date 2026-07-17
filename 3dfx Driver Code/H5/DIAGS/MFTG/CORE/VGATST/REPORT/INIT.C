//
//		INIT.CPP - Reporting device specific init routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/17/98
//		Last Modified:	4/17/98
//
//		Routines in this file:
//		OpenReportDevice			Start a console-style reporting device
//		CloseReportDevice			Terminate a console-style reporting device
//		WriteStringReportDevice	Write a string to the reporting device
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include "ediag.h"
#include	"repdev.h"

//
//		OpenReportDevice - Start a console-style reporting device
//
//		Entry:	nDevice	Device ID to open
//					selFlat	Selector to a flat descriptor
//					nExtra	Device dependent parameter
//		Exit:		<BOOL>	Success flag (TRUE = Opened, FALSE = Device failed)
//
BOOL OpenReportDevice (int nDevice, WORD selFlat, int nExtra)
{
	_selFlatDescriptor = selFlat;
	switch (nDevice)
	{
		case REPDEV_MONO:

			return (OpenMonoDevice ());

		case REPDEV_SERIAL:

			return (OpenSerialDevice (nExtra));

		case REPDEV_OEM:

			return (OpenOEMReportDevice ());
	}

	return (FALSE);
};

//
//		CloseReportDevice - Terminate a console-style reporting device
//
//		Entry:	nDevice		Device ID to close
//		Exit:		<BOOL>		Success flag (TRUE = Closed, FALSE = Device failed)
//
BOOL CloseReportDevice (int nDevice)
{
	switch (nDevice)
	{
		case REPDEV_MONO:

			return (CloseMonoDevice ());

		case REPDEV_SERIAL:

			return (CloseSerialDevice ());

		case REPDEV_OEM:

			return (CloseOEMReportDevice ());
	}

	return (FALSE);
}

//
//		WriteStringReportDevice - Write a string to the reporting device
//
//		Entry:	nDevice		Device to write to
//					lpstr			Pointer to NULL-terminated string
//		Exit:		<BOOL>		Success flag (TRUE = Wrote, FALSE = Device failed)
//
BOOL WriteStringReportDevice (int nDevice, LPSTR lpstr)
{
	switch (nDevice)
	{
		case REPDEV_MONO:

			return (WriteStringMonoDevice (lpstr));

		case REPDEV_SERIAL:

			return (WriteStringSerialDevice (lpstr));

		case REPDEV_OEM:

			return (WriteStringOEMReportDevice (lpstr));
	}

	return (FALSE);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//

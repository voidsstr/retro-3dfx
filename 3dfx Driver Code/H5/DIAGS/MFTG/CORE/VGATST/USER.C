//
//		USER.CPP - User interface functions for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/4/98
//		Last modified:	5/10/98
//
//		Routines in this file:
//		DisplayChar					Write a single character to the console
//		DisplayString				Write a string to the console
//		DisplayFormattedString	Write a formatted string to the console
//		DisplayCopyright			Display the initial sign-on screen
//		DisplayUsage				Display the usage sign-on screen
//		GetKey						Wait for a key to be pressed and return a key-code
//		ReportOpen					Open reporting devices
//		ReportClose					Close reporting devices
//		ReportString				Write a string to the reporting devices
//		ReportFormattedString	Write a formatted string to the reporting devices
//
#include	<stdio.h>
#include	<i86.h>
#include	"ediag.h"

//
//		DisplayChar - Write a single character to the console
//
//		Entry:	chr		Character to display
//		Exit:		None
//
void DisplayChar (char chr)
{
	fprintf (hOutDevice, "%c", chr);
}

//
//		DisplayString - Write a string to the console
//
//		Entry:	lpstr		Pointer to string
//		Exit:		None
//
void DisplayString (LPSTR lpstr)
{
	if (lpstr != NULL)
		fprintf (hOutDevice, "%s", lpstr);
}

//
//		DisplayFormattedString - Write a formatted string to the console
//
//		Entry:	lpstr		Pointer to string
//					...		Other possible paramters in "printf" syntax
//		Exit:		None
//
void DisplayFormattedString (LPSTR lpstr, ...)
{
	char	*pArgs;

	if (lpstr != NULL)
	{
		pArgs = ((char *) &lpstr) + sizeof (lpstr);
		vfprintf (hOutDevice, lpstr, &pArgs);
	}
}

//
//		DisplayCopyright - Display the initial sign-on screen
//
//		Entry:	None
//		Exit:		None
//
void DisplayCopyright (void)
{
	DisplayString (szCopyright);
	ReportString (szCopyright);
}

//
//		DisplayUsage - Display the usage sign-on screen
//
//		Entry:	None
//		Exit:		None
//
void DisplayUsage (void)
{
	DisplayString (szUsage);
}

//
//		GetKey - Wait for a key to be pressed and return a key-code
//
//		Entry:	None
//		Exit:		<WORD>	Key code
//
WORD GetKey (void)
{
	union REGS	regs;

	regs.w.ax = 0x0000;
	int386 (0x16, &regs, &regs);
	if (regs.h.al != 0) regs.h.ah = 0;

	return (regs.w.ax);
}

//
//		ReportOpen - Open reporting devices
//
//		Entry:	None
//		Exit:		None
//
void ReportOpen (void)
{
	bRepMono = OpenReportDevice (REPDEV_MONO, selFlat, 0);
	bRepSerial = OpenReportDevice (REPDEV_SERIAL, selFlat, nCOMPort);
	bRepOEM = OpenReportDevice (REPDEV_OEM, selFlat, 0);
}

//
//		ReportClose - Close reporting devices
//
//		Entry:	None
//		Exit:		None
//
void ReportClose (void)
{
	if (bRepMono) CloseReportDevice (REPDEV_MONO);
	if (bRepSerial) CloseReportDevice (REPDEV_SERIAL);
	if (bRepOEM) CloseReportDevice (REPDEV_OEM);
}

//
//		ReportString - Write a string to the reporting devices
//
//		Entry:	lpstr		Pointer to the string
//		Exit:		None
//
void ReportString (LPSTR lpstr)
{
	if (bRepMono) WriteStringReportDevice (REPDEV_MONO, lpstr);
	if (bRepSerial) WriteStringReportDevice (REPDEV_SERIAL, lpstr);
	if (bRepOEM) WriteStringReportDevice (REPDEV_OEM, lpstr);
}

////
////		ReportFormattedString - Write a formatted string to the reporting devices
////
////		Entry:	*szFormat	Pointer to format string
////					...			Other possible parameters in "printf" syntax
////		Exit:		None
////
//void ReportFormattedString (char *szFormat, ...)
//{
//	char			*pArguments;
//	static char	szString[500];
//
//	pArguments = (char *) &szFormat + sizeof szFormat;
//	vsprintf (szString, szFormat, &pArguments);
//	ReportString (szString);
//}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


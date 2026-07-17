//
//		MAIN.CPP - Entry point to EDIAG.EXE diagnostic program
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/9/98
//		Last Modified:	5/10/98
//
//		Routines in this file:
//		main					Entry point
//		InitVars				Initialize several global variables
//		PrepareExit			Prepare to exit the program
//
//		The command line can be:
//		DIAG [[-?] | [-HELP]] | [-INFO]
//		DIAG @{filename} [-CARD={card ID}] | [-SPEED={QUICK | EXTENDED}] | [-LOG={log file}] | [-GENCRC={CRC file} | -USECRC={CRC file}]
//		DIAG -T={module name} [-P={parameter list}] | [-CARD={card ID}] | [-SPEED={QUICK | EXTENDED}] | [-LOG={log file}]
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include	"ediag.h"

//
//		main - Entry point
//
//		Entry:	argc		Number of command line arguments
//					argv		Pointer to array of command line arguments
//		Exit:		<int>		DOS ERRORLEVEL
//

void (*callback_fn)(int) = NULL;

int ediag_main (int argc, char **argv,void (*cbfn)(int))
{
	int	i;
	BOOL	bSuccess, bError, bFound;

        callback_fn = cbfn;

	InitVars ();

	ReportOpen ();
//	DisplayCopyright ();

	// Invalid arguments on the command line
	if (argc <= 1)
	{
		DisplayString (szInvalidArgc);
		exit (1);
	}

	// Parse the command line one command at a time. After each command,
	// check to see if an action dictates that the program should exit.
	for (i = 1; i < argc; i++)
	{
		if (!ProcessCmdLineParm (argv[i], i, &bError))
			break;
	}

	// Look for the device; if it is found continue, otherwise
	// bail out and let user know.
	bFound = OEMInit (&CardInfo, selFlat);
	if (!bFound)
	{
		DisplayString (szDeviceNotFound);
		exit (1);
	}

	// Execute any pending tests
	if ((_bTestPending) && (!bError))
	{
		if (tblModuleList[_nTestIndex].functest != NULL)
			bSuccess = (*tblModuleList[_nTestIndex].functest) ();
		_bTestPending = FALSE;
	}

	// Process the command file
	if ((hInDevice != stdin) && (!bError))
	{
		// Read a line, parse it, and then execute the test. Continue
		// reading lines from the file til the entire file is processed
		nCmdFileLine = 1;
		while (ReadCmdFile (hInDevice, szCmdLine, MAX_LINE))
		{
			ProcessCmdLine (szCmdLine, &bError);
			if (bError) break;

			// Execute any pending tests
			if (_bTestPending)
			{
				if (tblModuleList[_nTestIndex].functest != NULL)
					bSuccess = (*tblModuleList[_nTestIndex].functest) ();
				_bTestPending = FALSE;
				if (!bSuccess) break;
			}
			nCmdFileLine++;
		}
	}

	PrepareExit ();

	if (!bSuccess) return (1);
	return (0);
}

//
//		InitVars - Initialize several global variables
//
//		Entry:	None
//		Exit:		None
//
void InitVars (void)
{
	// Allocate a flat-model descriptor for use in all memory operations.
	// Note that, even with a "flat" descriptor, physical addresses will
	// still have to be converted to linear addresses prior to use.
	selData = FP_SEG (&selData);			// Use current DS for all DPMI calls
	selFlat = AllocateFlatDescriptor ();

	// Initialize font pointer
	lpFont = tblFont8x16;
}

//
//		PrepareExit - Prepare to exit the program
//
//		Entry:	None
//		Exit:		None
//
void PrepareExit (void)
{
	if (_bClearScreen)
	{
		OEMSetVGAMode (&CardInfo);
		SetMode (0x03);
		//DisplayCopyright ();
	}

	// If a command file or other files are still in use, close
	// them before exiting
	if (hInDevice != stdin) fclose (hInDevice);
	if (hOutDevice != stdout) fclose (hOutDevice);
	if (hLogDevice != NULL) fclose (hLogDevice);
	if (hCRCDevice != NULL) fclose (hCRCDevice);

	ReportClose ();

	// After the last OEM function is called, terminate the OEM library.
	// Note that "ReportClose" will call the "CloseReportDevice" which
	// in turn calls the "CloseOEMReportDevice" if an OEM reporting
	// device exists.
	OEMTerminate (&CardInfo);

	// Free the flat model descriptor
	FreeDescriptor (selFlat);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


//
//		CMDLINE.CPP - Command line parsing routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/12/98
//		Last Modified:	5/10/98
//
//		Routines in this file:
//		ProcessCmdLineParm		Process one entry and dispatch to command handler
//		ProcessRawCommand		Process a possible command syntax
//		fnShowHelp			Handler for "HELP" on the command line
//		fnShowAll			Handler for "HELPALL" on the command line
//		fnShowInfo			Handler for "INFO" on the command line
//		fnSetCard			Handler for "CARD" on the command line
//		fnSetSpeed			Handler for "SPEED" on the command line
//		fnOpenLog			Handler for "LOG" on the command line
//		fnSetTest			Handler for "T" on the command line
//		fnSetParm			Handler for "P" on the command line
//		GetEntry			Separate parameter entry from raw parameter string
//		EvaluateEntry			Evaluate and store a parameter from a parameter list
//		fnWaitKey			Handler for "WAITKEY" in the command file
//		fnWait				Handler for "WAIT" on the command line
//		fnSetSerial			Handler for "SERIAL" on the command line
//		fnGenCRC			Handler for "GENCRC" on the command line
//		fnUseCRC			Handler for "USECRC" on the command line
//
#include	<stdio.h>
#include	<string.h>
#include	"ediag.h"

//
//		ProcessCmdLineParm - Process one entry and dispatch to command handler
//
//		Entry:	lpArg		Pointer to the argument
//			nParmIdx	Parameter number on the command line
//			lpbError	Pointer to syntax error flag (returned TRUE = Error)
//		Exit:	<BOOL>		Exit flag (TRUE = Continue, FALSE = Exit program)
//
//		Parsing definitions:
//		cmd_leader = '-' | '/'
//		special_leader = '@'
//		parameter = {text}[={text}]
//		parameter_list = {parameter}[;{parameter}]
//		raw_cmd = {text}[={text}]
//
//		entry = {cmd_leader} {raw_cmd} | {special_leader} {filename}
//
//		The command line can be:
//		DIAG [[-?] | [-HELP]] | [-INFO]
//		DIAG @{filename} [-CARD={card ID}] | [-SPEED={QUICK | EXTENDED}] | [-LOG={log file}]
//		DIAG -T={module name} [-P={parameter list}] | [-CARD={card ID}] | [-SPEED={QUICK | EXTENDED}] | [-LOG={log file}]
//
BOOL ProcessCmdLineParm (LPSTR lpArg, int nParmIdx, LPBOOL lpbError)
{
	BOOL	bContinue;

	bContinue = TRUE;						// Assume no error
	*lpbError = FALSE;					// Assume no error

	// Determine if the first character is valid.
	if ((*lpArg == chLeader1) || (*lpArg == chLeader2))
	{
		bContinue = ProcessRawCommand (lpArg + 1, lpbError);
	}
	else if (*lpArg == chSpecialLeader)
	{
		// Verify the next set of text is actually a filename for
		// an existing command file.
		if ((hInDevice = fopen (lpArg + 1, "rb")) == NULL)
		{
			DisplayString (szNoCommandFile);
			bContinue = FALSE;				// End processing immediately
			hInDevice = stdin;
			_bClearScreen = FALSE;			// Prevent erasing the error message
		}
	}
	else
	{
		// No leading '-', '/', or '@'. Therefore it's a bad argument.
		*lpbError = TRUE;
	}

	// On errors, display the syntax error message and stop processing
	if (*lpbError)
	{
		DisplayFormattedString (sznInvalidArgv, nParmIdx);
		bContinue = FALSE;
		_bClearScreen = FALSE;	// Prevent the screen clear at the end
	}

	return (bContinue);
}

//
//		ProcessRawCommand - Process a possible command syntax
//
//		Entry:	lpCmd		Pointer to command string
//					lpbError	Pointer to syntax error flag (returned TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL ProcessRawCommand (LPSTR lpCmd, LPBOOL lpbError)
{
	int		n, nSize;
	long int	nValue;
	BOOL		bNoMatch, bContinue;

	// Traverse the list of possible command line parameters looking
	// for matches. On a match, parse the rest of the string.
	nSize = strlen (lpCmd);
	bNoMatch = TRUE;
	bContinue = TRUE;
	*lpbError = FALSE;
	n = 0;
	while ((n < nCmdLineList) && (bNoMatch))
	{
		switch (tblCmdLineList[n].type)
		{
			//
			// The command takes the form of "-CMD" (e.g. "-HELP" or "-INFO")
			//
			case CMD_NOPARM:

				if (nSize == tblCmdLineList[n].len)
				{
					if (stricmp (tblCmdLineList[n].cmd, lpCmd) == 0)
					{
						bNoMatch = FALSE;
						if (tblCmdLineList[n].func != NULL)
							bContinue = (*tblCmdLineList[n].func) ("\0", 0, lpbError);
					}
				}
				break;

			//
			// The command takes the form of "-CMD=TEXT" (e.g. "-SPEED=QUICK")
			//
			case CMD_TEXTPARM:

				if (strnicmp (tblCmdLineList[n].cmd, lpCmd, tblCmdLineList[n].len) == 0)
				{
					lpCmd += tblCmdLineList[n].len;
					// The next character should be a '='
					if (*lpCmd == cCmdDelimiter)
					{
						bNoMatch = FALSE;				// Error, or not, found the string
						if (tblCmdLineList[n].func != NULL)
							bContinue = (*tblCmdLineList[n].func) (lpCmd + 1, 0, lpbError);
					}
				}
				break;

			//
			// The command takes the form of "-CMD=99" (e.g. "-CARD=0")
			//
			case CMD_NUMBERPARM:

				if (strnicmp (tblCmdLineList[n].cmd, lpCmd, tblCmdLineList[n].len) == 0)
				{
					lpCmd += tblCmdLineList[n].len;
					// The next character should be a '='
					if (*lpCmd == cCmdDelimiter)
					{
						bNoMatch = FALSE;				// Error, or not, found the string
						nValue = ConvertStr2Value (lpCmd + 1, lpbError);
						if (*lpbError)
							bContinue = FALSE;		// Force program to end on error
						else
						{
							if (tblCmdLineList[n].func != NULL)
								bContinue = (*tblCmdLineList[n].func) (lpCmd + 1, nValue, lpbError);
						}
					}
				}
				break;

			//
			// The command takes the form of "-CMD=FILE" (e.g. "-LOG=ERROR.LOG")
			//
			case CMD_FILEPARM:

				if (strnicmp (tblCmdLineList[n].cmd, lpCmd, tblCmdLineList[n].len) == 0)
				{
					lpCmd += tblCmdLineList[n].len;
					// The next character should be a '='
					if (*lpCmd == cCmdDelimiter)
					{
						bNoMatch = FALSE;				// Error, or not, found the string
						if (tblCmdLineList[n].func != NULL)
							bContinue = (*tblCmdLineList[n].func) (lpCmd + 1, 0, lpbError);
					}
				}
				break;
		}
		n++;
	}

	if (bNoMatch)
	{
		*lpbError = TRUE;
		bContinue = FALSE;
	}

	return (bContinue);
}

//
//		fnShowHelp - Handler for "HELP" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnShowHelp (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	n = n;
	lpsz = lpsz;				// Prevent compiler warnings
	_bClearScreen = FALSE;	// Prevent the screen clear at the end
	*lpbError = FALSE;		// Not possible to have error by this point

	DisplayString (szUsage);
	return (FALSE);
}

//
//		fnShowAll - Handler for "HELPALL" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnShowAll (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	int	i;

	n = n;
	lpsz = lpsz;				// Prevent compiler warnings
	*lpbError = FALSE;		// Not possible to have error by this point
	_bClearScreen = FALSE;	// Prevent the screen clear at the end

	// First, display the normal usage string
	DisplayString (szUsage);

	for (i = 0; i < nModuleList; i++)
	{
		DisplayString ("\n\n");
		DisplayString (tblModuleList[i].desc);
		DisplayString ("\n\tModule Keyword: ");
		DisplayString (tblModuleList[i].name);
		switch (tblModuleList[i].type)
		{
			case PARM_NONE:		DisplayString (szParmNone);
										break;
			case PARM_MEMORY:	DisplayString (szParmMemory);
										break;
			case PARM_VGAMODE:	DisplayString (szParmVGAMode);
										break;
			case PARM_NATMODE:	DisplayString (szParmNatMode);
										break;
			case PARM_RAMDAC:	DisplayString (szParmRAMDAC);
										break;
		}
	}
	DisplayString (szUseMore);

	return (FALSE);
}

//
//		fnShowInfo - Handler for "INFO" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnShowInfo (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	n = n;
	lpsz = lpsz;				// Prevent compiler warnings
	*lpbError = FALSE;		// Not possible to have error by this point

	DisplayString ("\nAdapter Info");
	DisplayFormattedString ("\n\tPCI Vendor ID: %04Xh\tPCI Device ID: %04Xh", CardInfo.pciVendorID, CardInfo.pciDeviceID);
	DisplayString ("\n");
	return (FALSE);
}

//
//		fnSetCard - Handler for "CARD" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnSetCard (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	lpsz = lpsz;				// Prevent compiler warnings

	*lpbError = FALSE;		// Not possible to have error by this point
	_nCardID = n;
	return (TRUE);
}

//
//		fnSetSpeed - Handler for "SPEED" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnSetSpeed (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	n = n;						// Prevent compiler warnings

	*lpbError = FALSE;		// Assume no error
	if (stricmp (lpsz, szQuick) == 0)
		_bExtendedTest = FALSE;
	else if (stricmp (lpsz, szExtended) == 0)
		_bExtendedTest = TRUE;
	else
	{
		*lpbError = TRUE;		// Syntax error!
		return (FALSE);
	}

	return (TRUE);
}

//
//		fnOpenLog - Handler for "LOG" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnOpenLog (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	n = n;						// Prevent compiler warnings

	*lpbError = FALSE;		// Assume no error
	hLogDevice = fopen (lpsz, "w+");
	if (hLogDevice == NULL)
	{
		DisplayString (szLogOpenError);
		*lpbError = TRUE;		// Oops
		return (FALSE);
	}

#ifdef DEBUG
	bUseLogFile = TRUE;
	strcpy (szLogFile, lpsz);
	fclose (hLogDevice);
	hLogDevice = NULL;
#endif

	LogWriteString (szLogHeader);

	return (TRUE);
}

//
//		fnSetTest - Handler for "T" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//			n		Pointer to numeric parameter
//			lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:	<BOOL>		Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnSetTest (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	int	i, nTestSize;
	BOOL	bContinue;

	n = n;						// Prevent compiler warnings

	// At first, assume a syntax error will occur, causing the
	// program to exit.
	*lpbError = TRUE;
	bContinue = FALSE;

	// If a test is already pending, inform the user that a syntax
	// error has occurred since only one test may be executed from
	// the command line.
	if (_bTestPending) return (bContinue);

	// Loop through the list of test names for a match.
	nTestSize = strlen (lpsz);
	for (i = 0; i < nModuleList; i++)
	{
		if (tblModuleList[i].len == nTestSize)
		{
			if (stricmp (tblModuleList[i].name, lpsz) == 0)
			{
				// A matched occured! Set the flags to indicate that a
				// test was found and save the test index for later.
				*lpbError = FALSE;
				bContinue = TRUE;
				_nTestIndex = i;
				_bTestPending = TRUE;
				break;
			}
		}
	}

	return (bContinue);
}

//
//		fnSetParm - Handler for "P" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
//		Note:		The input string is coming in as a list of parameters
//					that are [possibly] separated by a ';'.
//
BOOL fnSetParm (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	LPSTR	lpszTmp;
	int	nSize;
	BOOL	bContinue, bMore;

	n = n;						// Prevent compiler warnings

	// At first, assume a syntax error will occur, causing the
	// program to exit.
	*lpbError = TRUE;
	bContinue = FALSE;

	// If a test is not yet pending, inform the user that a syntax
	// error has occurred since the parameters can't be validated
	// until a test is selected.
	if (!_bTestPending) return (bContinue);

	// If there are no parameters, then flag that there are no syntax
	// errors and allow the program to continue. Tests that take parameters
	// will use default settings.
	if (*lpsz == '\0')
	{
		*lpbError = FALSE;
		return (TRUE);
	}

	bMore = TRUE;										// Assume more than one
	switch (tblModuleList[_nTestIndex].type)
	{
		case PARM_NONE:

			// No processing needed. If a parameter was passed in a
			// "no parameter" module, then a syntax error will result.
			// If not, then this function would have exited above in
			// in the check for no parameters.
			break;

		case PARM_MEMORY:

			// Flag that there are parameters and therefore a full memory
			// test is not to be run. However, one of the parameters may
			// be "FULL", which will set the flag back to "TRUE".
			_bMemFullTest = FALSE;
			while (bMore)
			{
				lpszTmp = GetEntry (lpsz, &nSize, cParmDelimiter, &bMore);
				EvaluateEntry (tblParmMemoryList, nParmMemoryList, lpsz, nSize, lpbError);
				if (*lpbError) return (FALSE);
				lpsz = lpszTmp;
			}
			break;

		case PARM_VGAMODE:

			// Flag that there are parameters and therefore all VGA modes
			// are not to be set. However, one of the parameters may
			// be "ALL", which will set the flag back to "TRUE".
			_bAllVGAModes = FALSE;
			_bSelectVGAModes = FALSE;
			while (bMore)
			{
				lpszTmp = GetEntry (lpsz, &nSize, cParmDelimiter, &bMore);
				EvaluateEntry (tblParmVGAModeList, nParmVGAModeList, lpsz, nSize, lpbError);
				if (*lpbError) return (FALSE);
				lpsz = lpszTmp;
			}
			break;

		case PARM_NATMODE:

			// Flag that there are parameters and therefore all native
			// modes are not to be set. However, one of the parameters
			// may be "ALL", which will set the flag back to "TRUE".
			_bAllNatModes = FALSE;
			_bSelNatModes = FALSE;
			while (bMore)
			{
				lpszTmp = GetEntry (lpsz, &nSize, cParmDelimiter, &bMore);
				EvaluateEntry (tblParmNatModeList, nParmNatModeList, lpsz, nSize, lpbError);
				if (*lpbError) return (FALSE);
				lpsz = lpszTmp;
			}
			break;

		case PARM_RAMDAC:

			// Flag that there are parameters and therefore all native
			// modes are not to be set. However, one of the parameters
			// may be "FULL", which will set the flag back to "TRUE".
			_bRAMDACFull = FALSE;
			while (bMore)
			{
				lpszTmp = GetEntry (lpsz, &nSize, cParmDelimiter, &bMore);
				EvaluateEntry (tblParmRAMDACList, nParmRAMDACList, lpsz, nSize, lpbError);
				if (*lpbError) return (FALSE);
				lpsz = lpszTmp;
			}
			break;
	}

	// If an error occured, then don't continue
	return (!(*lpbError));
}

//
//		GetEntry - Separate parameter entry from raw parameter string
//
//		Entry:	lpszParm		Pointer to raw parameter string
//					pnSize		Pointer to string size variable (returned)
//					cDelim		Delimiter character between parameters
//					lpbMore		Pointer "more" flag (TRUE = String hasn't ended yet)
//		Exit:		<LPSTR>		Pointer to string past evaluated positions
//
LPSTR GetEntry (LPSTR lpszParm, int *pnSize, char cDelim, LPBOOL lpbMore)
{
	int	n;

	// Count the characters until the NULL terminator or the
	// delimitor is reached.
	n = 0;
	while (!((*lpszParm == '\0') || (*lpszParm == cDelim)))
	{
		lpszParm++;
		n++;
	}

	// There is no more data if the character that caused the loop to
	// exit was the NULL terminator. Furthermore, there is no more data
	// if the character following the delimitor is the NULL character.
	if (*lpszParm == '\0')
		*lpbMore = FALSE;
	else
	{
		lpszParm++;								// Increment past delimiter
		*lpbMore = !(*lpszParm == '\0');
	}

	*pnSize = n;
	return (lpszParm);
}

//
//		EvaluateEntry - Evaluate and store a parameter from a parameter list
//
//		Entry:	lptblParmList	Pointer to PARMLIST
//					nTableSize		Number of entries in the PARMLIST table
//					lpszParm			Pointer to parameter entry string
//					nSize				Entry "string" size
//					lpbError			Pointer to syntax error flag (TRUE = Error)
//		Exit:		None
//
//		Note:		The "string" passed in is NOT a NULL terminated string. It
//					is a "live" substring of the in-the-process of being parsed
//					string. That is, it should be treated as READ ONLY.
//
void EvaluateEntry (LPPARMLIST lptblParmList, int nTableSize, LPSTR lpszParm, int nSize, LPBOOL lpbError)
{
	int			i, n;
	BOOL			bMatch;
	long int		nValue;
	static char	szTmp[MAX_LINE];

	bMatch = FALSE;				// Assume no match
	*lpbError = FALSE;			// Assume no error
	for (i = 0; (i < nTableSize) && (!bMatch); i++)
	{
		if (strnicmp (lptblParmList->name, lpszParm, lptblParmList->len) == 0)
		{
			if (lptblParmList->type == CMD_NOPARM)
			{
				// Make sure this parameter is not part of another string or
				// that there is garbage behind it.
				n = lptblParmList->len;
				if ((*(lpszParm + n) == cParmDelimiter) || (*(lpszParm + n) == '\0'))
				{
					bMatch = TRUE;				// String was found
					// A "no parameter" switch was found, therefore set
					// the global variable associated with it to "TRUE".
					if (lptblParmList->pvar != NULL)
						*(LPBOOL) (lptblParmList->pvar) = TRUE;
				}
			}
			else
			{
				// All commands with parameters, must be delimited by the '='.
				if (*(lpszParm + lptblParmList->len) == cCmdDelimiter)
				{
					bMatch = TRUE;			// Bad or not, we found the string
					// Copy the string locally so that we can stick the
					// NULL on it to evaluate it. (Sending non-strings to
					// string functions is BAD!)
					memcpy (szTmp, lpszParm + lptblParmList->len + 1, nSize - (lptblParmList->len + 1));
					szTmp[nSize - (lptblParmList->len + 1)] = '\0';
					if (lptblParmList->type == CMD_NUMBERPARM)
					{
						// A numeric parameter was found, so convert and store
						// the value.
						nValue = ConvertStr2Value (szTmp, lpbError);
						if (!(*lpbError))
							*(LPDWORD) (lptblParmList->pvar) = (DWORD) nValue;
					}
					else
					{
						// Handle CMD_TEXTPARM & CMD_FILEPARM
					}
				}
			}
		}
		lptblParmList++;
	}

	// If no match occured, then flag this as an error
	if (!bMatch) *lpbError = TRUE;
}

//
//		fnWaitKey - Handler for "WAITKEY" in the command file
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnWaitKey (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	n = n;
	lpsz = lpsz;				// Prevent compiler warnings
	*lpbError = FALSE;		// Not possible to have error by this point

	GetKey ();

	return (FALSE);
}

//
//		fnWait - Handler for "WAIT" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnWait (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	lpsz = lpsz;				// Prevent compiler warnings
	*lpbError = FALSE;		// Not possible to have error by this point

	// Wait "n" seconds
	Delay15us ((1000000 * n)/15);

	return (TRUE);
}

//
//		fnSetSerial - Handler for "SERIAL" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnSetSerial (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	lpsz = lpsz;				// Prevent compiler warnings

	*lpbError = FALSE;		// Not possible to have error by this point
	if (bRepSerial)
		CloseReportDevice (REPDEV_SERIAL);

	nCOMPort = n;
	bRepSerial = OpenReportDevice (REPDEV_SERIAL, selFlat, nCOMPort);
	return (TRUE);
}

//
//		fnGenCRC - Handler for "GENCRC" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnGenCRC (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	n = n;						// Prevent compiler warnings

	*lpbError = TRUE;			// Assume the worst

	if (_bUseCRC)
	{
		DisplayString (szCRCUseAndGenError);
		return (FALSE);
	}

	hCRCDevice = fopen (lpsz, "w+");
	if (hCRCDevice == NULL)
	{
		DisplayString (szCRCOpenError);
		return (FALSE);
	}

	_bGenCRC = TRUE;
	*lpbError = FALSE;
	strcpy (szCRCFile, lpsz);
	fclose (hCRCDevice);
	hCRCDevice = NULL;

	return (TRUE);
}

//
//		fnUseCRC - Handler for "USECRC" on the command line
//
//		Entry:	lpsz		Pointer to additional text parameter(s)
//					n			Pointer to numeric parameter
//					lpbError	Pointer to syntax error flag (TRUE = Error)
//		Exit:		<BOOL>	Exit flag (TRUE = Continue, FALSE = Exit program)
//
BOOL fnUseCRC (LPSTR lpsz, long int n, LPBOOL lpbError)
{
	n = n;						// Prevent compiler warnings

	*lpbError = TRUE;			// Assume the worst

	if (_bGenCRC)
	{
		DisplayString (szCRCUseAndGenError);
		return (FALSE);
	}

	hCRCDevice = fopen (lpsz, "rb");
	if (hCRCDevice == NULL)
	{
		DisplayString (szCRCOpenError);
		return (FALSE);
	}

	_bUseCRC = TRUE;
	*lpbError = FALSE;
	strcpy (szCRCFile, lpsz);

	return (TRUE);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


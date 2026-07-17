//
//		FILE.CPP - File routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/11/98
//		Last Modified:	5/10/98
//
//		Routines in this file:
//		LogWriteString			Write a string to the log file
//		LogComment				Formatted output to the log file
//		ReadCmdFile				Read a line from the command file and preprocess it
//		ProcessCmdLine			Process a single line from the command file
//		SaveCRC					Save the CRC value to a file
//		ReadCRC					Read a CRC value from a file
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<io.h>
#include	<ctype.h>
#include	"ediag.h"

//
//		LogWriteString - Write a string to the log file
//
//		Entry:	lpsz		Pointer to string to write to file
//		Exit:		None
//
void LogWriteString (LPSTR lpsz)
{
	ReportString (lpsz);

#ifdef DEBUG
	if (bUseLogFile)
	{
		hLogDevice = fopen (szLogFile, "a+");
		if (hLogDevice != NULL)
		{
			fwrite (lpsz, sizeof (char), strlen (lpsz), hLogDevice);
			fclose (hLogDevice);
			hLogDevice = NULL;
		}
	}
#else
	if (hLogDevice != NULL)
		fwrite (lpsz, sizeof (char), strlen (lpsz), hLogDevice);
#endif
}

//
//		LogComment - Formatted output to the log file
//
//		Entry:	*szFormat	Pointer to format string
//					...			Other possible parameters in "printf" syntax
//		Exit:		None
//
void LogComment (char *szFormat, ...)
{
	static char	szString[500];
	char			*pArguments;

	pArguments = (char *) &szFormat + sizeof szFormat;
	vsprintf (szString, szFormat, &pArguments);
	ReportString (szString);

#ifdef DEBUG
	if (bUseLogFile)
	{
		hLogDevice = fopen (szLogFile, "a+");
		if (hLogDevice != NULL)
		{
			fwrite (szString, sizeof (char), strlen (szString), hLogDevice);
			fclose (hLogDevice);
		}
	}
#else
	if (hLogDevice != NULL)
		fwrite (szString, sizeof (char), strlen (szString), hLogDevice);
#endif
}

//
//		ReadCmdFile - Read a line from the command file and preprocess it
//
//		Entry:	hFile			File handle of the command file
//					lpCmd			Pointer to where command line data is returned into
//					nMaxLine		Maximum size of the command line
//		Exit:		<BOOL>		End of file flag (TRUE = More data, FALSE = EOF)
//
BOOL ReadCmdFile (FILE *hFile, LPSTR lpCmd, int nMaxLine)
{
	static BOOL	bEOFPending = FALSE;
	LPSTR			lpCmdOrg;
	int			nChars, n;

	nChars = 0;
	lpCmdOrg = lpCmd;
	do
	{
		if (nFileBufferIdx >= nFileBufferSize)
		{
			if (bEOFPending)
			{
				if (nChars == 0)
					return (FALSE);
				else
					lpCmd++;			// Make room for NULL terminator
				break;				// We'll return TRUE this time, and FALSE next
			}

			nFileBufferSize = fread (byFileBuffer, 1, FILEIO_BUFFER, hFile);
			if (nFileBufferSize < FILEIO_BUFFER) bEOFPending = TRUE;
			nFileBufferIdx = 0;
		}
		*lpCmd = byFileBuffer[nFileBufferIdx++];
		nChars++;
		if (nChars >= nMaxLine) break;
	} while (*lpCmd++ != '\n');

	// Tack on NULL terminator to complete the command string
	*(lpCmd - 1) = '\0';

	// Erase white space at the end of the line. This gets rid of
	// characters like '\r' and '\t'.
	lpCmd = lpCmdOrg;
	n = (strlen (lpCmd));
	while (n--)
	{
		if (isspace (*(lpCmd + n)))
			*(lpCmd + n) = '\0';
		else
			break;
	}

	return (TRUE);
}

//
//		ProcessCmdLine - Process a single line from the command file
//
//		Entry:	lpCmd		Pointer to command line data
//					lpbError	Pointer to syntax error flag (returned TRUE = Error)
//		Exit:		None
//
void ProcessCmdLine (LPSTR lpCmd, LPBOOL lpbError)
{
	static char	szSingleCmd[MAX_LINE];
	int			nIdx, nCmdLength;

	// If the command line is zero length, skip it. This is a legal
	// line in the file, but does not need to be processed.
	nCmdLength = strlen (lpCmd);
	if (nCmdLength == 0) return;

	// Copy the command into a single buffer. If a space is encountered,
	// then immediately execute the command. If no errors are found, then
	// continue until all space-delimited commands are executed.
	nIdx = 0;
	while (nCmdLength--)
	{
		szSingleCmd[nIdx] = *lpCmd++;
		if (szSingleCmd[nIdx] == ' ')
		{
			if (nIdx != 0)
			{
				szSingleCmd[nIdx] = '\0';
				ProcessRawCommand (szSingleCmd, lpbError);
				if (*lpbError)
				{
					DisplayFormattedString (sznInvalidCmdLine, nCmdFileLine, szSingleCmd);
					_bClearScreen = FALSE;	// Prevent the screen clear at the end
					return;
				}
				nIdx = 0;
			}
		}
		else if (szSingleCmd[nIdx] == '/')
		{
			if (*lpCmd == '/')		// If a "//" occurs, ignore the rest of the line
				break;
			else
				nIdx++;
		}
		else
		{
			nIdx++;
		}
	}

	if (nIdx != 0)
	{
		szSingleCmd[nIdx] = '\0';
		ProcessRawCommand (szSingleCmd, lpbError);
		if (*lpbError)
		{
			DisplayFormattedString (sznInvalidCmdLine, nCmdFileLine, szSingleCmd);
			_bClearScreen = FALSE;		// Prevent the screen clear at the end
		}
	}
}

//
//		SaveCRC - Save the CRC value to a file
//
//		Entry:	dwCRC		Value to write to the file
//		Exit:		None
//
void SaveCRC (DWORD dwCRC)
{
	static char	szCRC[16];

	if (_bGenCRC)
	{
		hCRCDevice = fopen (szCRCFile, "a+");
		if (hCRCDevice != NULL)
		{
			sprintf (szCRC, "0x%08X\n", dwCRC);
			fwrite (szCRC, sizeof (char), strlen (szCRC), hCRCDevice);
			fclose (hCRCDevice);
			hCRCDevice = NULL;
		}
	}
}

//
//		ReadCRC - Read a CRC value from a file
//
//		Entry:	None
//		Exit:		<DWORD>		CRC value read from the file
//
DWORD ReadCRC (void)
{
	static char	szCRC[16];
	int			n;
	DWORD			dw;
	BOOL			bError;

	dw = 0;
	if (_bUseCRC)
	{
		if (hCRCDevice != NULL)
		{
			n = fread (szCRC, sizeof (char), 12, hCRCDevice);
			szCRC[10] = '\0';
			dw = (DWORD) ConvertStr2Value (szCRC, &bError);
		}
	}
	return (dw);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//


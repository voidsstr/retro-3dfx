//
//		TEST.CPP - Front end to VGA simulator (VGASIM.LIB)
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				12/18/94
//		Last Modified:	5/2/97
//
//		Routines in this file:
//		DisplayCopyright		Display the initial sign-on screen
//		DisplayChar				Write a single character to the console
//		DisplayString			Write a string to the console
//		DisplayPrompt			Write the prompt to the console
//		ParseCommand			Interpret the command line and distribute to the appropriate routines
//		CmdDump					Handle the dump memory command
//		CmdEnter					Handle the enter command
//		CmdIn						Handle the I/O read command
//		CmdOut					Handle the I/O write command
//		CmdQuit					Handle the quit command
//		CmdHelp					Handle the help command
//		GetDataType				Return an appropriate data type if buffer contains one
//		GetAddress				Interpret the address
//		DumpBytes				Dump memory in BYTEs
//		DumpWords				Dump memory in WORDs
//		DumpDwords				Dump memory in DWORDs
//		GetHexData				Retrieve hex data from the command line
//		CmdMode					Handle the mode set command
//		CmdWrite					Handle the write frame command
//		CmdTest					Handle a test condition
//
#include <stdlib.h>
#include	<ctype.h>
#include <stdio.h>
#include <string.h>
#include	<conio.h>
#include	<signal.h>
#include	"vgacore.h"
#include	"vgasim.h"
#include "test.h"

// Global variables
#define	MAX_CMDSTRING		127
char		szCopyright[] =   "\nCopyright (c) 1993-1996 by Elpin Systems"
									"\nAll rights reserved.\n";
char		szSimError[] = "\nERROR - Cannot allocate enough memory to start VGA simulation.";
char		szInBuffer[MAX_CMDSTRING + 2] = {MAX_CMDSTRING, 0};
char		szOutBuffer[MAX_CMDSTRING];
char		szHelp[] = "\nD[type][<range>] - dump memory"
							"\nE[type] <address> [<list>] - enter"
							"\nI[type] <value> - input from port"
							"\nM <value> - mode set"
							"\nO[type] <value> <byte> - output to port"
							"\nQ - quit"
							"\nW <filename> - write frame capture"
							"\n";
CMDLINE	tblCmdLine[] = {
	{'D', CmdDump},
	{'E', CmdEnter},
	{'I', CmdIn},
	{'M', CmdMode},
	{'O', CmdOut},
	{'Q', CmdQuit},
	{'W', CmdWrite},
	{'X', CmdTest},
	{'?', CmdHelp}
};
int	nCommands = sizeof (tblCmdLine) / sizeof (CMDLINE);

//
//		main - Entry point to manual front end of simulator
//
//		Entry:	argc	Number of command line arguments
//					argv	Pointer to array of arguments
//		Exit:		<int>	DOS ERRORLEVEL value
//
int main (int argc, char **argv)
{
	BOOL	bDone;
	int	nerr;

	// Initialize the simulator */
	DisplayCopyright ();
	if (SimStart ())
	{
		DisplayString (szSimError);
		return (1);
	}
	printf ("\n");

	// Disable CTRL+C handling
	signal (SIGINT, SIG_IGN);
	SimSetCaptureMode (CAP_COMPOSITE);

	bDone = FALSE;
	while (!bDone)
	{
		// Display the user prompt
		DisplayPrompt ();

		// Get a command from the command line and parse it.
		if (!ParseCommand (fgets (szInBuffer, MAX_CMDSTRING, stdin), &bDone, &nerr))
		{
			while (nerr--) DisplayChar (' ');
			DisplayString ("^ Error\n");
		}
	}

	SimEnd();
	DisplayString ("\nBye...");
	return (0);
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
}

//
//		DisplayChar - Write a single character to the console
//
//		Entry:	chr		Character to display
//		Exit:		None
//
void DisplayChar (char chr)
{
	// For now, just use a "printf"
	printf ("%c", chr);
}

//
//		DisplayString - Write a string to the console
//
//		Entry:	lpstr		Pointer to string
//		Exit:		None
//
void DisplayString (LPSTR lpstr)
{
	// For now, just use a "printf"
	printf ("%Fs", lpstr);
}

//
//		DisplayPrompt - Write the prompt to the console
//
//		Entry:	None
//		Exit:		None
//
void DisplayPrompt (void)
{
	DisplayString ("-");
}

//
//		ParseCommand - Interpret the command line and distribute to the
//							appropriate routines
//
//		Entry:	pstr		Pointer to input string
//					pbExit	Pointer to exit flag
//					pnError	Pointer to error variable
//		Exit:		<BOOL>	Success flag (TRUE = Command(s) parsed, FALSE = Error)
//
BOOL ParseCommand (char *pstr, BOOL *pbExit, int *pnError)
{
	static char szTemp[MAX_CMDSTRING];
	char			chr;
	int			nState, nPtr, i, nCount, nCountLast, nErrPos;
	BOOL			bCmdComplete;
	LPFV			lpproc;

	*pnError = 0;
	bCmdComplete = TRUE;
	nState = 0;
	nCountLast = nCount = 0;
	lpproc = NULL;
	while ((chr = *pstr++) != '\0')
	{
		nCount++;				// Number of characters processed
		switch (nState)
		{
			case 0:				// Beginning of command, skipping white space

				nPtr = 0;
				if (!isspace (chr) && (chr != ';'))
				{
					// Search table of commands for a match. If a match is
					// found, then get the "goto" procedure pointer.
					lpproc = NULL;
					chr = toupper (chr);
					for (i = 0; i < nCommands; i++)
					{
						if (chr == (char) tblCmdLine[i].cmd)
						{
							lpproc = tblCmdLine[i].proc;
							break;
						}
					}
					// If a command wasn't found, then skip processing and
					// return an error. Otherwise, toggle the state and
					// begin gathering the command string.
					if (lpproc == NULL)
					{
						*pnError = nCount;
						return (FALSE);
					}
					nState = 1;
				}
				break;

			case 1:				// Gathering command data string

				if (chr == ';')
				{
					szTemp[nPtr] = '\0';
					if (!lpproc (szTemp, nPtr, pbExit, &nErrPos))
					{
						*pnError = nCountLast + nErrPos;
						return (FALSE);
					}
					nCountLast = nCount;
					lpproc = NULL;
					nPtr = 0;
					nState = 0;
				}
				else
					szTemp[nPtr++] = chr;
				break;
		}
	}

	// If there are any commands in the buffer, then execute them
	// before returning.
	if (lpproc != NULL)
	{
		szTemp[nPtr] = '\0';
		if (!lpproc (szTemp, nPtr, pbExit, &nErrPos))
		{
			*pnError = nCountLast + nErrPos;
			return (FALSE);
		}
	}

	return (TRUE);
}

//
//		CmdDump - Handle the dump memory command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
//		Syntax:	[type][<range>]
//
BOOL CmdDump (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	static char		cLastType = 'B';
	static DWORD	dwLastAddr = 0;
	char				cNewType;
	DWORD				dwNewAddr;
	BOOL				bGotIt;
	int				nCountOrg;
	SEGOFF			lpptr;

	*pnErrPos = 0;
	nCountOrg = nCount;
	if (nCount > 0)
	{
		// Determine if a data type is being specified
		cNewType = GetDataType (*pParm);
		if (cNewType != '\0')
		{
			pParm++;
			cLastType = cNewType;
			nCount--;
		}

		// Now get address
		dwNewAddr = GetAddress (&pParm, &nCount, dwLastAddr, FALSE, &bGotIt);
		if (!bGotIt)
		{
			*pnErrPos = nCountOrg - nCount;
			return (FALSE);
		}
		dwLastAddr = dwNewAddr;
	}

	// Dump the memory buffer
	lpptr = (SEGOFF) dwLastAddr;
	switch (cLastType)
	{
		case 'B':

			DumpBytes (lpptr, lpptr + 0x7F);
			dwLastAddr += 0x80;
			break;

		case 'W':

			DumpWords (lpptr, lpptr + 0x7F);
			dwLastAddr += 0x80;
			break;

		case 'D':

			DumpDwords (lpptr, lpptr + 0x7F);
			dwLastAddr += 0x80;
			break;
	}

	return (TRUE);
}

//
//		CmdEnter - Handle the enter command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
//		Syntax:	E[type] <address> <data>
//
BOOL CmdEnter (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	static char		cLastType = 'B';
	static DWORD	dwLastAddr = 0;
	char				cNewType;
	int				nCountOrg;
	DWORD				dwData, dwAddr;
	BOOL				bGotIt;

	*pnErrPos = 0;
	nCountOrg = nCount;

	// There must be data. A memory address and data value MUST exist.
	if (nCount == 0)
	{
		*pnErrPos = 1;
		return (FALSE);
	}

	// Determine if a data type is being specified
	cNewType = GetDataType (*pParm);
	if (cNewType != '\0')
	{
		pParm++;
		cLastType = cNewType;
		nCount--;
	}

	// Now get memory address
	dwAddr = GetAddress (&pParm, &nCount, dwLastAddr, TRUE, &bGotIt);
	if (!bGotIt)
	{
		*pnErrPos = nCountOrg - nCount;
		return (FALSE);
	}

	// Now get data value
	dwData = GetHexData (&pParm, &nCount, &bGotIt, sizeof (DWORD));
	if (!bGotIt)
	{
		*pnErrPos = nCountOrg - nCount;
		return (FALSE);
	}

	// Verify nothing else is on the line
	while (nCount--)
	{
		if (!isspace (*pParm++))
		{
			*pnErrPos = nCountOrg - nCount;
			return (FALSE);
		}
	}

	dwLastAddr = dwAddr;
	switch (cLastType)
	{
		case 'B':

			MemByteWrite ((SEGOFF) dwLastAddr, (BYTE) dwData);
			break;

		case 'W':

			MemWordWrite ((SEGOFF) dwLastAddr, (WORD) dwData);
			break;

		case 'D':

			MemDwordWrite ((SEGOFF) dwLastAddr, dwData);
			break;
	}

	return (TRUE);
}

//
//		CmdIn - Handle the I/O read command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
//		Syntax:	I[type] <value>
//
BOOL CmdIn (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	static char	cLastType = 'B';
	char			cNewType;
	WORD			wPort;
	BOOL			bGotIt;
	int			nCountOrg;

	*pnErrPos = 0;
	nCountOrg = nCount;

	// There must be data. A port address MUST exist.
	if (nCount == 0)
	{
		*pnErrPos = 1;
		return (FALSE);
	}

	// Determine if a data type is being specified
	cNewType = GetDataType (*pParm);
	if (cNewType != '\0')
	{
		pParm++;
		cLastType = cNewType;
		nCount--;
	}

	// Now get port address
	wPort = (WORD) GetHexData (&pParm, &nCount, &bGotIt, sizeof (WORD));
	if (!bGotIt)
	{
		*pnErrPos = nCountOrg - nCount;
		return (FALSE);
	}

	// Verify nothing else is on the line
	while (nCount--)
	{
		if (!isspace (*pParm++))
		{
			*pnErrPos = nCountOrg - nCount;
			return (FALSE);
		}
	}

	switch (cLastType)
	{
		case 'B':

			sprintf (szOutBuffer, "%02X\n", IOByteRead (wPort));
			break;

		case 'W':

			sprintf (szOutBuffer, "%04X\n", IOWordRead (wPort));
			break;

		case 'D':

			sprintf (szOutBuffer, "%04X%04X\n", IOWordRead (wPort + 2), IOWordRead (wPort));
			break;
	}
	DisplayString (szOutBuffer);

	return (TRUE);
}

//
//		CmdOut - Handle the I/O write command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
//		Syntax:	O[type] <value> <byte>
//
BOOL CmdOut (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	static char	cLastType = 'B';
	char			cNewType;
	WORD			wPort;
	int			nCountOrg;
	DWORD			dwData;
	BOOL			bGotIt;

	*pnErrPos = 0;
	nCountOrg = nCount;

	// There must be data. A port address and data value MUST exist.
	if (nCount == 0)
	{
		*pnErrPos = 1;
		return (FALSE);
	}

	// Determine if a data type is being specified
	cNewType = GetDataType (*pParm);
	if (cNewType != '\0')
	{
		pParm++;
		cLastType = cNewType;
		nCount--;
	}

	// Now get port address
	wPort = (WORD) GetHexData (&pParm, &nCount, &bGotIt, sizeof (WORD));
	if (!bGotIt)
	{
		*pnErrPos = nCountOrg - nCount;
		return (FALSE);
	}

	// Now get data value
	dwData = GetHexData (&pParm, &nCount, &bGotIt, sizeof (DWORD));
	if (!bGotIt)
	{
		*pnErrPos = nCountOrg - nCount;
		return (FALSE);
	}

	// Verify nothing else is on the line
	while (nCount--)
	{
		if (!isspace (*pParm++))
		{
			*pnErrPos = nCountOrg - nCount;
			return (FALSE);
		}
	}

	switch (cLastType)
	{
		case 'B':

			IOByteWrite (wPort, (BYTE) dwData);
			break;

		case 'W':

			IOWordWrite (wPort, (WORD) dwData);
			break;

		case 'D':

			IOWordWrite (wPort, LOWORD (dwData));
			IOWordWrite (wPort + 2, HIWORD (dwData));
			break;
	}

	return (TRUE);
}

//
//		CmdQuit - Handle the quit command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
//		Syntax:	Q (ignore the rest of the line)
//
BOOL CmdQuit (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	*pnErrPos = 0;
	*pbExit = TRUE;
	return (TRUE);
}

//
//		CmdHelp - Handle the help command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
BOOL CmdHelp (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	*pnErrPos = 0;
	DisplayString (szHelp);
	return (TRUE);
}

//
//		GetDataType - Return an appropriate data type if buffer contains one
//
//		Entry:	chr		Character to test
//		Exit:		<char>	Data type:
//									'B'	Byte
//									'W'	Word
//									'D'	Dword
//									'\0'	Error, none found
//
char GetDataType (char chr)
{
	chr = toupper (chr);
	if ((chr == 'B') || (chr == 'W') || (chr == 'D'))
		return (chr);
	return ('\0');
}

//
//		GetAddress - Interpret the address
//
//		Entry:	ppParm		Pointer to command line pointer
//					pnCount		Pointer to number of characters left
//					dwDefault	Default address
//					bForceAddr	Data must exist if TRUE; otherwise return "dwDefault" if FALSE
//					pbFlag		Pointer to success flag (TRUE = Got address, FALSE = Not)
//		Exit:		<DWORD>		Address
//
DWORD GetAddress (char **ppParm, int *pnCount, DWORD dwDefault, BOOL bForceAddr, BOOL *pbFlag)
{
	DWORD	dw;
	char	*pParm, *pParmOrg;
	char	*pTemp, *pEnd;
	char	chrOld;
	int	nCountOrg;

	pParmOrg = pParm = *ppParm;
	nCountOrg = *pnCount;
	*pbFlag = !bForceAddr;
	dw = 0;
	if (*pnCount == 0) goto GetAddress_error;

	// Skip white space if any
	while (isspace (*pParm))
	{
		pParm++;
		if (--(*pnCount) == 0) break;
	}
	if (*pnCount == 0) goto GetAddress_error;

	// Initialize segment portion in case only offset is specified
	dw = dwDefault;

	// Convert hex digits (could be segment or offset - this is
	// determined by a ':' character).
	*pbFlag = FALSE;											// Assume error
	pTemp = (char *)memchr (pParm, ':', *pnCount);
	if (pTemp != NULL)										// Get segment value
	{
		if (pTemp == pParm) goto GetAddress_error;	// Error? No data?

		// Convert ASCII hex to integer
		*pTemp = '\0';
		dw = strtoul (pParm, &pEnd, 16);
		if (*pEnd != '\0') goto GetAddress_error;

		// Segment is the upper word
		dw = dw << 16;
		*pTemp = ':';								// Restore the destroyed character
		pParm = pTemp + 1;						// Point to second half (offset)
	}

	// If there is nothing after the ':' then flag it as an error.
	if ((*pParm == '\0') || (isspace (*pParm))) goto GetAddress_error;

	// Find end of string or next white space
	pTemp = pParm;
	while ((*pTemp != '\0') && (!isspace (*pTemp)))
		pTemp++;
	chrOld = *pTemp;
	*pTemp = '\0';

	// Convert ASCII hex to integer
	dw = (WORD) strtoul (pParm, &pEnd, 16) | (dw & 0xFFFF0000);
	pParm = pEnd;
	if (*pParm != '\0')
	{
		*pTemp = chrOld;
		goto GetAddress_error;
	}
	*pTemp = chrOld;

	*pbFlag = TRUE;
	*ppParm = pParm;
	*pnCount = nCountOrg - (pParm - pParmOrg);
	return (dw);

GetAddress_error:
	*pnCount = nCountOrg - (pParm - pParmOrg);
	*ppParm = pParm;
	return (dwDefault);
}

//
//		DumpBytes - Dump memory in BYTEs
//
//		Entry:	lpStart	Pointer to starting BYTE
//					lpEnd		Pointer to ending BYTE
//		Exit:		None
//
void DumpBytes (SEGOFF lpStart, SEGOFF lpEnd)
{
	SEGOFF	lpptr, lpptrEnd;
	BOOL		bFirst, bData;
	int		i;
	char		chr;

	bFirst = TRUE;
	bData = FALSE;

	lpptr = (SEGOFF) ((DWORD) (lpStart) & 0xFFFFFFF0);
	lpptrEnd = (SEGOFF) ((DWORD) (lpEnd) | 0x0000000F);
	while (lpptr <= lpptrEnd)
	{
		// Check for the start of a new line
		if ((LOWORD (lpptr) & 0x0F) == 0)
		{
			// Add previous line's ASCII data
			if (!bFirst)
			{
				lpptr -= 16;
				strcat (szOutBuffer, "  ");
				for (i = 0; i < 16; i++)
				{
					if ((lpptr < lpStart) || (lpptr > lpEnd))
						chr = ' ';
					else
					{
						chr = MemoryByteRead (lpptr);
						if ((chr < 0x20) || (chr > 0x7F)) chr = '.';
					}
					sprintf (&szOutBuffer[strlen (szOutBuffer)], "%c", chr);
					lpptr++;
				}
				strcat (szOutBuffer, "\n");
				DisplayString (szOutBuffer);
				bData = FALSE;
			}
			bFirst = FALSE;
			sprintf (szOutBuffer, "%04X:%04X ", HIWORD (lpptr), LOWORD (lpptr));
		}

		if ((LOWORD (lpptr) & 0x0F) == 8)
			chr = '-';
		else
			chr = ' ';

		if ((lpptr < lpStart) || (lpptr > lpEnd))
			strcat (szOutBuffer, "   ");
		else
		{
			sprintf (&szOutBuffer[strlen (szOutBuffer)], "%c%02X", chr, MemoryByteRead (lpptr));
			bData = TRUE;
		}

		lpptr++;
	}

	// Handle the last line special
	if (bData)
	{
		// Add previous line's ASCII data
		if (!bFirst)
		{
			lpptr -= 16;
			strcat (szOutBuffer, "  ");
			for (i = 0; i < 16; i++)
			{
				if ((lpptr < lpStart) || (lpptr > lpEnd))
					chr = ' ';
				else
				{
					chr = MemoryByteRead (lpptr);
					if ((chr < 0x20) || (chr > 0x7F)) chr = '.';
				}
				sprintf (&szOutBuffer[strlen (szOutBuffer)], "%c", chr);
				lpptr++;
			}
			strcat (szOutBuffer, "\n");
			DisplayString (szOutBuffer);
		}
	}
}

//
//		DumpWords - Dump memory in WORDs
//
//		Entry:	lpStart	Pointer to starting BYTE
//					lpEnd		Pointer to ending BYTE
//		Exit:		None
//
void DumpWords (SEGOFF lpStart, SEGOFF lpEnd)
{
	SEGOFF	lpptr, lpptrEnd;
	BOOL		bFirst;
	int		nCounter;

	bFirst = TRUE;
	nCounter = 8;

	lpptr = lpStart;
	lpptrEnd = lpEnd;
	while (lpptr <= lpptrEnd)
	{
		// Check for the start of a new line
		if (nCounter == 8)
		{
			if (!bFirst)
			{
				strcat (szOutBuffer, "\n");
				DisplayString (szOutBuffer);
			}
			bFirst = FALSE;
			nCounter = 0;
			sprintf (szOutBuffer, "%04X:%04X ", HIWORD (lpptr), LOWORD (lpptr));
		}

		sprintf (&szOutBuffer[strlen (szOutBuffer)], " %04X", MemoryWordRead (lpptr));
		nCounter++;

		lpptr += 2;
	}

	strcat (szOutBuffer, "\n");
	DisplayString (szOutBuffer);
}

//
//		DumpDwords - Dump memory in DWORDs
//
//		Entry:	lpStart	Pointer to starting BYTE
//					lpEnd		Pointer to ending BYTE
//		Exit:		None
//
void DumpDwords (SEGOFF lpStart, SEGOFF lpEnd)
{
	SEGOFF	lpptr, lpptrEnd;
	BOOL		bFirst;
	int		nCounter;
	DWORD		dw;

	bFirst = TRUE;
	nCounter = 4;

	lpptr = lpStart;
	lpptrEnd = lpEnd;
	while (lpptr <= lpptrEnd)
	{
		// Check for the start of a new line
		if (nCounter == 4)
		{
			if (!bFirst)
			{
				strcat (szOutBuffer, "\n");
				DisplayString (szOutBuffer);
			}
			bFirst = FALSE;
			nCounter = 0;
			sprintf (szOutBuffer, "%04X:%04X ", HIWORD (lpptr), LOWORD (lpptr));
		}

		dw = MemoryDwordRead (lpptr);
		sprintf (&szOutBuffer[strlen (szOutBuffer)], " %04X:%04X", HIWORD (dw), LOWORD (dw));
		nCounter++;

		lpptr += 4;
	}

	strcat (szOutBuffer, "\n");
	DisplayString (szOutBuffer);
}

//
//		GetHexData - Retrieve hex data from the command line
//
//		Entry:	ppParm		Pointer to data
//					pnCount		Pointer to number of characters left
//					pbFlag		Pointer to success flag (TRUE = Got word, FALSE = Not)
//					nSize			Size (in BYTEs) of binary data
//		Exit:		<WORD>		Hex word
//
DWORD GetHexData (char **ppParm, int *pnCount, BOOL *pbFlag, int nSize)
{
	char	*pParm;
	DWORD	dw;
	int	n;
	BYTE	hex;

	pParm = *ppParm;
	*pbFlag = TRUE;
	nSize = (nSize * 2) - 1;			// Number of nibbles, normalized to 0

	// Missing data is flagged as an error
	if (*pnCount == 0) goto GetHexData_error;

	// Skip white space if any
	while (isspace (*pParm))
	{
		pParm++;
		if (--(*pnCount) == 0) break;
	}

	// Missing data is again flagged as an error
	if (*pnCount == 0) goto GetHexData_error;

	dw = 0;
	n = 0;
	while (TRUE)
	{
		dw = dw << 4;
		hex = toupper (*pParm) - '0';
		if ((hex >= ('A' - '0')) && (hex <= ('F' - '0')))
			hex -= (('A' - '0') - 10);

		if (hex > 0x0F) goto GetHexData_error;

		dw = dw | hex;
		pParm++;
		(*pnCount)--;
		if ((*pnCount == 0) || (*pParm == '\0') || (isspace (*pParm))) break;

		if (++n > nSize) goto GetHexData_error;
	}
	*ppParm = pParm;
	return (dw);

// Error handler
GetHexData_error:
	*ppParm = pParm;
	*pbFlag = FALSE;
	return (0);
}

//
//		CmdMode - Handle the mode set command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
//		Syntax:	M <value>
//
BOOL CmdMode (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	WORD			wMode;
	BOOL			bGotIt;
	int			nCountOrg;

	*pnErrPos = 0;
	nCountOrg = nCount;

	// There must be data. A mode number MUST exist.
	if (nCount == 0)
	{
		*pnErrPos = 1;
		return (FALSE);
	}

	// Now get mode number
	wMode = (WORD) GetHexData (&pParm, &nCount, &bGotIt, sizeof (WORD));
	if (!bGotIt)
	{
		*pnErrPos = nCountOrg - nCount;
		return (FALSE);
	}

	// Verify nothing else is on the line
	while (nCount--)
	{
		if (!isspace (*pParm++))
		{
			*pnErrPos = nCountOrg - nCount;
			return (FALSE);
		}
	}

	SetMode (wMode);

	return (TRUE);
}

//
//		CmdWrite - Handle the write frame command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
//		Syntax:	W <filename>
//
BOOL CmdWrite (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	char	*pStr, *pEnd;
	char	chrOrg;
	int	nCountOrg, nRet;

	nCountOrg = nCount;
	*pnErrPos = 0;
	if (nCount == 0)
	{
		*pnErrPos = 1;
		return (FALSE);
	}

	// Find non-white space
	while ((*pParm != '\0') && (isspace (*pParm)))
	{
		nCount--;
		pParm++;
	}
	if (nCount == 0)
	{
		*pnErrPos = nCountOrg - nCount;
		return (FALSE);
	}
	pStr = pParm;						// Start of filename

	// Find white space
	while ((*pParm != '\0') && (!isspace (*pParm)))
	{
		nCount--;
		pParm++;
	}
	pEnd = pParm;						// End of filename

	// Verify nothing else is on the line
	while (nCount--)
	{
		if (!isspace (*pParm++))
		{
			*pnErrPos = nCountOrg - nCount;
			return (FALSE);
		}
	}

	chrOrg = *pEnd;
	*pEnd = '\0';
	nRet = CaptureFrame (pStr);
	*pEnd = chrOrg;

	return (nRet == 0);
}

//
//		CmdTest - Handle the test condition command
//
//		Entry:	pParm			Pointer to rest of command string
//					nCount		Number of bytes in command string
//					pbExit		Pointer to exit flag
//					pnErrPos		Pointer to error position
//		Exit:		<BOOL>		Success flag (TRUE = Command handled, FALSE = Not)
//
//		Syntax:	X
//
#define	RES_X			640
#define	RES_Y			480
//
BOOL CmdTest (char *pParm, int nCount, BOOL *pbExit, int *pnErrPos)
{
	SetMode (0x12);
	Line4 (0, 0, RES_X - 1, 0, 0x0F);
	Line4 (RES_X - 1, 0, RES_X - 1, RES_Y - 1, 0x0F);
	Line4 (0, RES_Y - 1, RES_X - 1, RES_Y - 1, 0x0F);
	Line4 (0, 0, 0, RES_Y - 1, 0x0F);
	Line4 (0, 0, RES_X - 1, RES_Y - 1, 0x0F);
	Line4 (0, RES_Y - 1, RES_X - 1, 0, 0x0F);
	return (TRUE);
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

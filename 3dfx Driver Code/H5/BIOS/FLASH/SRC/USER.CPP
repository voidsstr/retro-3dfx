//
//		USER.CPP - User interface functions for FLASH.EXE
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				3/4/98
//		Last modified:	8/3/98
//
//		Routines in this file:
//		DisplayChar					Write a single character to the console
//		DisplayString				Write a string to the console
//		DisplayCopyright			Display the initial sign-on screen
//		DisplayUsage				Display the usage sign-on screen
//		DisplayFormattedString	Write a formatted string to the console
//
#include	<stdio.h>
#include	"flash.h"


BOOL ContinueYN(LPSTR lpstr)
{
   char  cChoice;
   BOOL  bContinue = FALSE;

   fprintf(hOutDevice,"%s Continue(y/n)? ", lpstr);
   fscanf(stdin, "%c", &cChoice);
   fflush(stdin);

   if( (cChoice == 'y') || (cChoice == 'Y') )
   {
      bContinue = TRUE;
   }
   return bContinue;
}
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
	fprintf (hOutDevice, "%s", lpstr);
	fflush (hOutDevice);
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
//		Copyright (c) 1998 3Dfx Interactive, Inc.
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//


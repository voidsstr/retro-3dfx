/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 3$
** $Date: 10/11/00 8:18:50 PM$
*/

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <fxos.h>

main(int argc, char **argv)
{
	int i, retval;
	float totalTime;
	int hours, minutes, seconds;
	char command[256];

	strcpy(command, "");
	for(i=1; i<argc; i++) {
		strcat(command, argv[i]);
		strcat(command, " ");
	}

	// reset timer
	timer(0);

	// Launch command
	retval = system(command);

	// Stop timer
	totalTime = timer(1);
	hours = (int) (totalTime / (float) 3600.);
	minutes = (int) ((totalTime - hours*(float) 3600.) / (float) 60.);
	seconds = (int) (totalTime - hours*(float) 3600. - minutes*(float) 60.);
	fflush(stderr);
	fflush(stdout);
	printf("\n%2d:%2d:%2d (%.2fs)\n",
		(int) hours, (int) minutes, (int) seconds, totalTime);
	fflush(stdout);

	return(retval);
}

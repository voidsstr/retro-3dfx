/*
** Copyright 1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"


long verbose;


void Output(char *format, ...)
{
    va_list args;

    va_start(args, format);
    if (verbose) {
	vprintf(format, args);
	fflush(stdout);
    }
    va_end(args);
}

static long Init(int argc, char **argv)
{
    long i;

    verbose = 0;

    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-h") == 0) {
	    printf("Options:\n");
	    printf("\t-h     Print this help screen.\n");
	    printf("\t-v     Verbose mode ON.\n");
	    printf("\n");
	    return 1;
	} else if (strcmp(argv[i], "-v") == 0) {
	    verbose = 1;
	} else {
	    printf("%s (Bad option).\n", argv[i]);
	    return 1;
	}
    }
    return 0;
}

int main(int argc, char **argv)
{

    printf("OpenGL X Coverage Test.\n");
    printf("Version 1.1.1\n");
    printf("\n");

    if (Init(argc, argv)) {
	exit(1);
    }

    VerifyEnums();
    DoTest();

    printf("covglx passed.\n\n");
    return 0;
}

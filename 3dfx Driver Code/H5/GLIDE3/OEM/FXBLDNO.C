/*
 * Copyright (c) 1997, 3Dfx Interactive, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of 3Dfx Interactive, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
 * and Computer Software clause at DFARS 252.227-7013, and/or in similar or
 * successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
 * rights reserved under the Copyright Laws of the United States.
 *
 * $Header: fxbldno.c, 2, 10/11/00 8:26:20 PM, Brent$
 * $Log: 
 *  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
 *       branching.
 *  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
 * $
** 
** 2     7/24/98 1:38p Hohn
 * 
 * 1     7/25/97 9:05a Pgj
 * generate fxbldno.h which defines BUILD_NUMBER
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

main(int argc, char **argv)
{
    struct tm	locTime;
    time_t	sysTime;
    char	*build;

    time(&sysTime);
    locTime = *localtime(&sysTime);

    if (build = getenv("BUILD_NUMBER")) {
	printf("#define BUILD_NUMBER	%s\n", build);
    } else {
	unsigned short magic;
	magic = (locTime.tm_yday << 7) |
	        (locTime.tm_hour << 2) |
		(locTime.tm_min / 15);
	printf("#define BUILD_NUMBER	%d\n", magic);
    }
    return 0;

} /* end main() */

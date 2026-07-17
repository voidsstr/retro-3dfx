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
** $Revision: 2$
** $Date: 10/11/00 8:19:22 PM$
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gdebug.h>
#include <gsim.h>
#include <sst.h>
#include <sstsim.h>

#include "udiag.h"
#include "sstdiag.h"

int exponents[256];

static float plusMinus(void)
{
    return iRandom(1) ? 1.0F : -1.0F;
}

void
main (int argc, char **argv)
{
    int n,x,exp;
    float f,fexp;
    SstRegs *sst;

    sst = SST_BEGIN(argc,argv);

    while (DIAG_STARTPASS()) {			// for each pass
	for (n=0; n<100; n++) {
	    f = fexpRandom(-3,15);
	    x = (int)(f * (1<<16));
	    fexp = (float)(log(f)/log(2.0));
	    if (fexp < 0.0F) fexp -= 1.0F;
	    exp = (int)fexp;
	    gdbg_info(3,"f=%12.6f %4x.%04x  exp=%d\n",
			f,x>>16,x&0xFFFF,exp);
	    exponents[exp+127]++;
	}
	for (n=0; n<100; n++) {
	    f = plusMinus();
	    gdbg_info(3,"f = %g\n",f);
	}
    }

    gdbg_info(2,"Histogram of exponents:\n");
    for (n=0; n<256; n++) {
	if (exponents[n])
	    gdbg_info(2,"e=%3d %4d times\n",n-127,exponents[n]);
    }
    DIAG_PASS(0);
}

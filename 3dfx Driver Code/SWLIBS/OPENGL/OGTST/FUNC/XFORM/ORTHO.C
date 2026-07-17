/**************************************************************************
 *									  *
 * 		 Copyright (C) 1989, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* ortho.c - $Revision: 2$ */

/*
 * This routine tests the ortho and ortho2 commands by comparing the matrix
 * loaded with what's supposed to be there. 
 *
 * Debug levels: 1 - print the ortho parameters 2 - print display-list/immediate
 * mode 
 */

#include "ogtst.h"		/* include test environment		 */
#include "xform.h"		/* include test environment		 */

TESTMOD(ortho)
{
    int n;
    float m[4][4];
    float x1, x2, y1, y2, z1, z2;
    int max;

    max = 2;

    gtst_init_matrixstuff();

    while (pass--) {
	ogEnvLog(1, "----- Start New Pass -----\n");

	switch (ogLibIntRand(0,max)) {
	case 0:
	    gtst_mmode(GL_MODELVIEW);
	    break;
	case 1:
	    gtst_mmode(GL_PROJECTION);
	    break;
	case 2:
	    gtst_mmode(GL_TEXTURE);
	    break;
	}

	for (n = 1 + ogLibBitRand(3); n > 0; n--) {
	    x1 = ogLibFloatRand(-1e4, 1e4);
	    x2 = ogLibFloatRand(-1e4, 1e4);
	    y1 = ogLibFloatRand(-1e4, 1e4);
	    y2 = ogLibFloatRand(-1e4, 1e4);
	    z1 = ogLibFloatRand(-1e4, 1e4);
	    z2 = ogLibFloatRand(-1e4, 1e4);

	    if (ogLibBitRand(1))	/* XXXblythe */
		gtst_ortho2(x1, x2, y1, y2);
	    else
		gtst_ortho(x1, x2, y1, y2, z1, z2);

	    gtst_getmatrix(m[0], 1);
	}
    }
}

CLEANUP(ortho)
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

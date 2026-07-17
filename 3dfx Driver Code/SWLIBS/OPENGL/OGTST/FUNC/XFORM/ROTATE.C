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

/* rotate.c - $Revision: 2$ */

/*
 * This program tests rot/rotate by loading the identity matrix and then
 * rotating it and reading it back.  It also tests many rots in a row. 
 *
 * Debug levels: 1 - print the rot parameters 2 - print display-list/immediate
 * mode 
 */

#include "ogtst.h"		/* include test environment		 */
#include "xform.h"		/* include test environment		 */

TESTMOD(rotate)
{
    int n;
    float m[4][4];
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
	    gtst_rotate(ogLibExpRand(4), ogLibExpRand(2), ogLibExpRand(2), ogLibExpRand(2));
	    gtst_getmatrix(m[0], 0);
	}
    }
}

CLEANUP(rotate)
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

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

/* scale.c - $Revision: 2$ */

/*
 * This program tests scale by loading the identity matrix and then scaling
 * it and reading it back.  It also tests many scales in a row. 
 *
 * Debug levels: 1 - print the scale parameters 2 - print display-list/immediate
 * mode 
 */

#include "ogtst.h"		/* include test environment		 */
#include "xform.h"		/* include test environment		 */

TESTMOD(scale)
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

	gtst_scale(ogLibExpRand(ogLibIntRand(0,4)-2), ogLibExpRand(ogLibIntRand(0,4)-2), ogLibExpRand(ogLibIntRand(0,4)-2));
	gtst_getmatrix(m[0], 1);

	for (n = ogLibBitRand(3); n > 0; n--) {
	   gtst_scale(ogLibExpRand(ogLibIntRand(0,4)-2),ogLibExpRand(ogLibIntRand(0,4)-2),ogLibExpRand(ogLibIntRand(0,4)-2));
	   gtst_getmatrix(m[0], 1);
	}
    }
}

CLEANUP(scale)
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

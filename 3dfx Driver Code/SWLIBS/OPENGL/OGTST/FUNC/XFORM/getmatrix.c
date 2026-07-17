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

/* xxx.c - $Revision: 2$ */

/*
 * This program tests loadmatrix/getmatrix by loading a random matrix and
 * then reading it back twice. 
 *
 * Debug levels: 1 - print the matrix that is loaded 2 - print
 * display-list/immediate mode 
 */

#include "ogtst.h"		/* include test environment		 */
#include "xform.h"		/* include test environment		 */

TESTMOD(getmatrix)
{
    int i;
    float m[4][4],identity[4][4];
    float *p;
    int max;

    max = 2;

    ogLibUntMatrix(identity[0]);
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

	for (p = (float *) m, i = 0; i < 16; i++)
	    *p++ = (ogLibExpRand(ogLibIntRand(-12, 12)));

	gtst_loadmatrix(m[0]);

	gtst_getmatrix(m[0], 1);
	gtst_getmatrix(m[0], 1);

	/*
	 * do this due to factorization of viewing matrix out of composite
	 * matrix 
	 */
	if (gtst_getmmode() == GL_PROJECTION)
	    gtst_loadmatrix(identity[0]);
    }
}

CLEANUP(getmatrix)
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

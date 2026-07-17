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

/* disk.c - $Revision: 2$ */

/*
 * This program tests basic disk functionality.
 * 
 */
#include "ogtst.h"	/* include test environment		*/

static GLint saveSeed;

/*ARGSUSED*/
TESTMOD(disks)
{
    int i, obj;
    int xmax,ymax,x1,y1;
    GLUquadricObj *qo;
    GLfloat oradius,iradius;


    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.,(double) xmax+1., 0., (double) ymax+1., -1.,1.);

    qo = gluNewQuadric(); 
    ogLibClear(0) ;
    glMatrixMode(GL_MODELVIEW);
    /*
     * To simplify the code, the test used the random number generator.
     * As this is an checksum test, we need to set the seed.
     * The current seed it saved and restored at the end of the test.
     */
    saveSeed = ogLibGetSeed();
    ogLibSetSeed(0);
    for (i=0; i<200; i++) {
        (void) ogLibColor();
	x1 = ogLibIntRand(2,xmax - 2);
	y1 = ogLibIntRand(2,ymax - 2);
        glLoadIdentity();
        glTranslatef(x1, y1, 0);

        oradius = ogLibFloatRand(2.,xmax/8.);
	iradius = oradius > 10 ? oradius - 10 : 0;

	START_DL_OR_IM(1);

        gluDisk(qo,iradius, oradius, 120, 4.);

	FINIS_DL_OR_IM(1);
    }
}

CLEANUP(disks)
{
    ogLibSetSeed(saveSeed);
    ogLibSetDefaultColors();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

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

/* stencilmask.c - $Revision: 2$ */

#include "ogtst.h"		/* include test environment		 */

static GLint stbits;
static 
teststencil(int, int, int, int);

TESTMOD(stencilmask) {
    unsigned int mask, draw;
    int x, y, xmax, ymax, testmask[1];

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    stbits = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
    glEnable(GL_DEPTH_TEST);
    glStencilMask((1 << stbits)-1);
    glGetIntegerv(GL_STENCIL_WRITEMASK, testmask);
    ogEnvLog(2,"Stencil WriteMask: 0x%x\n", testmask[0]);
    glClearDepth(1.);
    glClearColor(0., 0., 0., 0.);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    x = 1 + ogLibIntRand(0,xmax - 2);
    y = 1 + ogLibIntRand(0,ymax - 2);

    ogEnvLog(1, "----- test default stencilmask -----\n");
    glClearStencil(15);
    glClear(GL_STENCIL_BUFFER_BIT);
    teststencil(x, y, 15, (1 << stbits) - 1);

    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    teststencil(x, y, 0, (1 << stbits) - 1);

    ogEnvLog(1, "----- test general stencilmask -----\n");

    while (pass--) {
	x = 1 + ogLibIntRand(0,xmax - 2);
	y = 1 + ogLibIntRand(0,ymax - 2);

	mask = ogLibBitRand(stbits);
	glStencilMask(mask);
	ogEnvLog(1, "glStencilMask(0x%x);\n", mask);

	draw = ogLibBitRand(stbits);
	glClearStencil(draw);
	glClear(GL_STENCIL_BUFFER_BIT);
	ogEnvLog(1, "glClearStencil(0x%x);glClear(GL_STENCIL_BUFFER_BIT);\n", draw);

	teststencil(x, y, draw, mask);
    }
}

CLEANUP(stencilmask) {
    glStencilMask((1 << ogEnvCurVisualInfo(GLX_STENCIL_SIZE)) - 1);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

#define  NC          -1		/* don't-care value */

/*************************************************************
* teststencil  - direct check of stencil plane values (instead
*   of old brain damaged indirect test)
*************************************************************/
static
teststencil(int x, int y, int expd, int mask)
{
    GLuint stencilVal;

    glReadPixels(x,y,1,1,GL_STENCIL_INDEX,GL_UNSIGNED_INT,&stencilVal);
    if (expd != NC && (stencilVal & mask) != (expd & mask)) {
        ogEnvLog(OG_LFAIL, "expected stencil value 0x%x, got 0x%x\n", expd & mask,
stencilVal);
    }
    return 0;
}


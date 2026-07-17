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

/* bitmaps.c - $Revision: 2$ */

/*
 * This program tests basic triangle functionality.
 * 
 *      Debug levels :
 *        1 - begin, end, vertex* parameters
 *        2 - display mode, color mode
 */

#include "ogtst.h"	/* include test environment		*/

static int rand_bitmap(void);
static GLboolean CImode;
static GLubyte *bmap;
static GLboolean testCompleted;

TESTMOD(bitmaps)
{
    long xmax, ymax;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    bmap = (GLubyte *) ogLibMalloc((sizeof(GLubyte) * (xmax+1) * (ymax+1)) / 8);
    if (bmap == NULL) {
        ogEnvLog(OG_LALWAYS, "Bummer: can malloc memory for bitmap test\n");
        testCompleted = GL_TRUE; /* So cleanup frees memory */
        return;
    }
    testCompleted = GL_FALSE;
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.,(double) xmax + 1., 0., (double) ymax + 1., -1.,1.);

    CImode = !ogEnvCurVisualInfo(GLX_RGBA);

    /* 
     * try to catch double hits! 
     */
    if (CImode) 
	glLogicOp(GL_XOR);
    else
	glBlendFunc(GL_ONE,GL_ONE);

    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    while (pass--) {
        rand_bitmap();
    }
    testCompleted = GL_TRUE;
}

CLEANUP(bitmaps)
{
    if (testCompleted)
        ogLibFree(bmap);
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    if (CImode) {
        glIndexf(1);
	glLogicOp(GL_COPY);
    } else {
        glColor4f(1, 1, 1, 1);
	glBlendFunc(GL_ONE,GL_ZERO);
    }
    ogLibSetDefaultRasterPos();
}

/*
 *  make a bitmap pattern with the lower left of every 2x2 bits on.
 */

static void
make_bitmap(int w, int h)
{
    int x,y;
    GLubyte pat = 0xaa;
    GLubyte last = 0xff;
    GLubyte *p;

    /* mask the last byte of the pattern */
    for (x=8 - (w%8); x; x--)
        last >>= 1;
    for (x=8 - (w%8); x; x--)
        last <<= 1;
    last &= pat;

    p = bmap;
    for (y=0; y<h; y++) {
	if (y & 1) {
	    for (x=w/8; x; x--) {
		*p++ = 0;
	    }
	    if (w%8)
		*p++ = 0;
	} else {
	    for (x=w/8; x; x--) {
		*p++ = pat;
	    }
	    if (w%8)
		*p++ = last;
	}
    }
}

static int
rand_bitmap(void)
{
    GLfloat pos[4];
    GLsizei w,h;
    GLfloat xo,yo;
    GLfloat xp,yp;
    int Xsize,Ysize;
    GLboolean valid,myvalid;
    int errs = 0;
    int maxcolor,obj;
    int x0,y0,x1,y1,x,y;
    int clipped,empty;

    Xsize = ogEnvQuery(OG_XWSIZE);
    Ysize = ogEnvQuery(OG_YWSIZE);

    w = ogLibIntRand(0,Xsize) & ~0x1;
    h = ogLibIntRand(0,Ysize) & ~0x1;
    make_bitmap(w,h);

    xo = ogLibIntRand(-Xsize/2,Xsize/2);
    yo = ogLibIntRand(-Ysize/2,Ysize/2);

    glGetFloatv(GL_CURRENT_RASTER_POSITION,pos);
    xp = (int) pos[0];
    yp = (int) pos[1];

    /* 
     *  Draw a quarter-filled bitmap 4 times to make a filled box
     */
    ogLibClear(0) ;
    maxcolor = ogLibColor() ;
    glRasterPos2fv(pos);  /* set a new color */

    ogEnvLog(1,"bitmap test: c-r-p:[%f,%f] glBitmap(%d,%d,%f,%f,%f,%f,bmap)\n",
			xp,yp,w,h,xo,yo,1.,0.);
    if (obj = ogLibBitRand(1))
	glNewList(1, GL_COMPILE);

    glBitmap(w,h,xo,yo,1.,0.,bmap);
    glBitmap(w,h,xo,yo,0.,1.,bmap);
    glBitmap(w,h,xo,yo,-1.,0.,bmap);
    glBitmap(w,h,xo,yo,0.,-1.,bmap);
    pos[0] = ogLibIntRand((int) (-0.5 * Xsize), (int) (1.5 * Xsize));
    pos[1] = ogLibIntRand((int) (-0.5 * Ysize), (int) (1.5 * Ysize));
    ogEnvLog(1,"glRasterPos2fv(%f,%f)\n",pos[0],pos[1]);
    glRasterPos2fv(pos);

    if (obj) {
	glEndList();
	glCallList(1);
	glDeleteLists( 1, 1 );
	ogEnvLog(2, "display-list mode\n");
    }
    else ogEnvLog(2, "immediate mode\n");

    /*
     * figure out the clipped box and check against actual box
     */

    x0 = xp - xo;
    y0 = yp - yo;
    x1 = x0 + w - 1;
    y1 = y0 + h - 1;
    clipped = 0;
    empty = 0;
    if (x0 <= 0) {
	clipped = 1;
	x0 = 0;
	if (x1 < 0) {
	    x1 = Xsize-1;
	    empty = 1;
	}
    }
    if (x1 >= Xsize - 1) {
	clipped = 1;
	x1 = Xsize-1;
	if (x0 > x1) {
	    x0 = 0;
	    empty = 1;
	}
    }
    if (y0 <= 0) {
	clipped = 1;
	y0 = 0;
	if (y1 < 0) {
	    y1 = Ysize-1;
	    empty = 1;
	}
    }
    if (y1 >= Ysize - 1) {
	clipped = 1;
	y1 = Ysize-1;
	if (y0 > y1) {
	    y0 = 0;
	    empty = 1;
	}
    }
    if (clipped == 0) {
        ogEnvLog(1, "   should cover (%d,%d) (%d,%d)\n",x0,y0,x1,y1);
	errs += ogLibRectCheck(x0,y0,x1,y1,maxcolor,0);
    } else if (empty) {
        ogEnvLog(1, "   should be empty\n");
	errs += ogLibRectCheck(x0+1,y0+1,x1-1,y1-1,0,0);
    } else if ((x1-x0 > 2) && (y1-y0 > 2)) {
        ogEnvLog(1, "   should be clipped to (%d,%d) (%d,%d)\n",x0,y0,x1,y1);
	errs += ogLibRectCheck(x0+1,y0+1,x1-1,y1-1,maxcolor,maxcolor);
    } else {
        ogEnvLog(1, "   should be clipped to (%d,%d) (%d,%d)\n",x0,y0,x1,y1);
	for (y=y0; y<y1; y++) {
	    for (x=x0; x<x1; x++) {
		errs += ogLibPixelCheck(x,y,maxcolor);
	    }
	} 
    } 


    /* see if we agree whether the crpos is valid */
    xp = pos[0];
    yp = pos[1];
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&valid);
    glGetFloatv(GL_CURRENT_RASTER_POSITION,pos);
    myvalid = (xp >= 0. && xp <= Xsize && yp >= 0. && yp <= Ysize);
    if (valid != myvalid) {
	errs++;
	ogEnvLog(OG_LFAIL,
	    "rand_bitmap: validity of raster pos. wrong, expected: %d got %d\n",
	    myvalid,valid);
    }
    if (valid == 0) {
	/* set a valid position for next pass */
	pos[0] = ogLibIntRand(0, Xsize-2);
	pos[1] = ogLibIntRand(0, Ysize-2);
	glRasterPos2fv(pos);
        return errs;
    }
    if (((int) (pos[0]+0.5) != xp) || ((int) (pos[1]+0.5) != yp)) {
	errs++;
	ogEnvLog(OG_LFAIL,
	    "rand_bitmap: raster pos. wrong, expected: %f,%f got %f,%f\n",
	    xp,yp,pos[0],pos[1]);
	xp = pos[0];
	yp = pos[1];
    }
    if (pos[0] > Xsize-2. || pos[1] > Ysize-2.) {
	/* set a valid position for next pass */
	pos[0] = ogLibIntRand(0, Xsize-2);
	pos[1] = ogLibIntRand(0, Ysize-2);
	glRasterPos2fv(pos);
    }
    return errs;
}

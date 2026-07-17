/*************************************************************************
*								         *
* 		 Copyright (C) 1989, Silicon Graphics, Inc.	         *
*								         *
*  These coded instructions, statements, and computer programs  contain  *
*  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
*  are protected by Federal copyright law.  They  may  not be disclosed  *
*  to  third  parties  or copied or duplicated in any form, in whole or  *
*  in part, without the prior written consent of Silicon Graphics, Inc.  *
*								         *
**************************************************************************/
      
/* nest.c - $Revision: 2$ */
      
/*
 * This program tests the ability to call display lists from within other
 * display lists down to the maximum nesting level.
 *
 */
      
#include "ogtst.h"	/* include test environment */

/* Function prototypes */
static void checkRectangles(GLint min, GLint max, 
		unsigned int wid, 
		unsigned int ht, 
		unsigned int xoff, 
		unsigned int yoff, 
		unsigned int color);



static void
checkRectangles(GLint min, GLint max, 
		unsigned int wid, 
		unsigned int ht, 
		unsigned int xoff, 
		unsigned int yoff, 
		unsigned int color)
{
  int rects;
  for (rects = min; rects < max; rects++) {
      (void) ogLibRectCheck(rects*xoff + 1,
			    rects*yoff + 1,
			    rects*xoff + wid,
			    rects*yoff + ht,
			    color,
			    0);
  }
  /* should be no rectangles beyond max */
  (void) ogLibRectCheck(rects*xoff + 1,
                        rects*yoff + 1,
                        rects*xoff + wid,
                        rects*yoff + ht,
                        0,
                        0);
}

TESTMOD(nest)
{
  unsigned int xmax, ymax, color, wid, ht;
  GLint  i, dlist_limit;
  GLint  max; /* max nesting for this round */
  GLuint dlist; /* dlist name */
  int do_test1, do_test2; /* iteration when we should do 
                           * time-consuming tests */

  glMatrixMode(GL_PROJECTION);
  glOrtho(0.0, (double)ogEnvQuery(OG_XWSIZE), /* map EC 1 to 1 with DC */
	  0.0, (double)ogEnvQuery(OG_YWSIZE),
	  -1.,1.);

  glMatrixMode(GL_MODELVIEW);

  xmax = ogEnvQuery(OG_XWSIZE) - 1; /* window x values 0 -> xmax */
  ymax = ogEnvQuery(OG_YWSIZE) - 1; /* window y values 0 -> ymax */

  glGetIntegerv(GL_MAX_LIST_NESTING, &dlist_limit);

  wid = xmax/dlist_limit - 3;
  ht  = ymax/dlist_limit - 3;

  while(pass--) {
    ogLibClear(0);
    ogEnvLog(1,"---check normal range of display list nesting---\n");

    max = ogLibIntRand(1, dlist_limit - 1);

    color = ogLibColor(); /* new color for each dlist */


    /* create nested display lists, with random nesting levels */
    for(i = 1; i <= max; i++) {
      glNewList(i, GL_COMPILE);
      glRecti(1, 1, wid + 1, ht + 1);
      if(i < max) {
	glTranslatef((float)wid + 2.0,  (float)ht + 2.0, 0.0);
	glCallList(i + 1);
	glTranslatef(-(float)wid - 2.0, -(float)ht - 2.0, 0.0);
      }
      glEndList();
    }

    ogLibClear(0);
    glCallList(1); /* draw all the dlists */

    /* check all rectangles drawn */
    checkRectangles(0, max, wid, ht, wid + 2, ht + 2, color);

    glDeleteLists(1, max); /* delete created display lists */

    ogEnvLog(1,"---nested compile-and-execute display lists---\n");

    max = ogLibIntRand(1, dlist_limit - 1);

    /* here we must work from the top down since we will call list i+1 */
    /* testing every iteration would take to long -- just test a couple of
     * times */
    do_test1 = ogLibIntRand(1, max);
    do_test2 = ogLibIntRand(1, max);
    for(i = max; i >= 1; i--) {
      ogLibClear(0);

      glNewList(i, GL_COMPILE_AND_EXECUTE);
      glRecti(1, 1, wid + 1, ht + 1);
      if(i < max) {
	glTranslatef((float)wid + 2.0,  (float)ht + 2.0, 0.0);
	glCallList(i + 1);
	glTranslatef(-(float)wid - 2.0, -(float)ht - 2.0, 0.0);
      }
      glEndList();

      if (i == do_test1 || i == do_test2) {
	ogEnvLog(1, "Verifying execute during compilation...\n");
        checkRectangles(0, max-i+1, wid, ht, wid + 2, ht + 2, color);
      }
    }    

    /* check here since everything should already be drawn */
    checkRectangles(0, max, wid, ht, wid + 2, ht + 2, color);
    ogLibClear(0);

    glCallList(1); /* draw all */

    checkRectangles(0, max, wid, ht, wid + 2, ht + 2, color);
    ogLibClear(0);

    glDeleteLists(1, max);

    ogEnvLog(1,"---create and call empty display lists---\n");

    /* make sure an empty display list doesn't crash */
    if ((dlist = ogLibBitRand(sizeof(GLuint))) == 0)
        dlist = 1;
    glNewList(dlist, GL_COMPILE);
    glEndList();
    glCallList(dlist); /* call it; no core dump */
    glDeleteLists(dlist,1); /* delete it */


    ogEnvLog(1,"---go beyond the max number of display lists---\n");

    max = dlist_limit + ogLibIntRand(0,dlist_limit);
    color = ogLibColor(); /* new color for each dlist */

    /* the display list depth should truncate to GL_MAX_LIST_NESTING */
    for(i = 1; i <= max; i++) {
      glNewList(i, GL_COMPILE);
      glRecti(1, 1, wid + 1, ht + 1);
      if(i < max) {
	glTranslatef(0.0,  (float)ht + 2.0, 0.0);
	glCallList(i + 1);
	glTranslatef(0.0, -(float)ht - 2.0, 0.0);
      }
      glEndList();
    }
    glCallList(1); /* draw lists */
    checkRectangles(0, dlist_limit, wid, ht, 0, ht + 2, color);
    glDeleteLists(1, max);      /* delete them all */
  }
}

CLEANUP(nest)
{
  int maxNest;
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  /* delete created display lists */
  glGetIntegerv(GL_MAX_LIST_NESTING, &maxNest);
  glDeleteLists(1, 2*maxNest);
  ogLibSetDefaultColors();
}

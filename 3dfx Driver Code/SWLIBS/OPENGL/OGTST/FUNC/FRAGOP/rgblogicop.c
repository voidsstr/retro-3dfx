/**************************************************************************
 *									  *
 * 		 Copyright (C) 1994, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 * Test color_logic_op using XOR.
 * The test draw a background with several polygons.  It then
 * uses the extension to draw rectangles over the background with
 * XOR in two passes.  After the second pass, it checks that the
 * background is restored to its original color.
 */

#include "ogtst.h"	/* include test environment		*/

TESTMOD(rgblogicop)
{
#ifdef GL_VERSION_1_1

    GLint xmax, ymax;
    GLint x1,y1,x2,y2;
    GLuint clearColor, blendColor;

    if (IS_ONEONE()) {

       /* This test should run with dithering either enabled and
	* disabled on platforms designed for OGL 1.1 and when
	* rendering to a pixmap. Unfortunately, it's necessary to
	* disable dithering for the EXPRESS, a pre-OGL 1.1 machine.  
	*/
       /* glDisable(GL_DITHER); */

       xmax = ogEnvQuery(OG_XWSIZE) - 1;
       ymax = ogEnvQuery(OG_YWSIZE) - 1;
       glMatrixMode(GL_PROJECTION);
       glOrtho(0.,(double) xmax+1., 0., (double) ymax+1., -1.,1.);

       while (pass--) {
	   clearColor = ogLibColor();
	   blendColor = ogLibColor();

	   ogEnvLog(1, "clearColor %s blendColor %s\n",
		    ogEnvColorString(clearColor), ogEnvColorString(blendColor));
	   ogLibClear(clearColor);

	   glLogicOp(GL_XOR);
	   glEnable(GL_COLOR_LOGIC_OP);

	   x1 = 2 + ogLibIntRand(0,xmax - 8);
	   y1 = 2 + ogLibIntRand(0,ymax - 8);
	   x2 = x1 + ogLibIntRand(4, (xmax-x1)/2);
	   y2 = y1 + ogLibIntRand(4, (ymax-y1)/2);

	   ogEnvLog(1, "glBegin(GL_POLYGON);  \n");
	   glBegin(GL_POLYGON);
	      glVertex2i(x1, y1);
	      glVertex2i(x2, y1);
	      glVertex2i(x2,y2);
	      glVertex2i(x1, y2);
	   glEnd();
	   ogEnvLog(1, "glEnd(); \n");

	   ogEnvLog(1, "glBegin(GL_POLYGON);  \n");
	   glBegin(GL_POLYGON);
	      glVertex2i(x1, y1);
	      glVertex2i(x2, y1);
	      glVertex2i(x2,y2);
	      glVertex2i(x1, y2);
	   glEnd();
	   ogEnvLog(1, "glEnd(); \n");

	   ogLibPixelCheck(x1+1, y1+1, clearColor);

       }

    }
#endif
}

CLEANUP(rgblogicop)
{
#ifdef GL_VERSION_1_1

    if (IS_ONEONE()) {

       glEnable(GL_DITHER);
       glDisable(GL_BLEND);
       glBlendFunc(GL_ONE,GL_ZERO);
       glDisable(GL_COLOR_LOGIC_OP);
       glLogicOp(GL_COPY);
       glLoadIdentity();
       glMatrixMode(GL_MODELVIEW);
       ogLibSetDefaultBuffers();
       ogLibSetDefaultColors();
       ogLibSetDefaultClears();

    }
#endif
}

/*
** Copyright 1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/

/*
** readpix.c
** glReadPixel() Test.
**
** Description -
**    Test glReadPixels() by comparing buffer as read by the window
**    system with buffer read by glReadPixels(). An assymetric
**    rectangle is drawn to check that buffer fills left to
**    right, bottom to top.
**
** Technical Specification -
**    Buffer requirements:
**        Color buffer.
**    Color requirements:
**        RGBA or color index mode. 2 colors.
**    States requirements:
**    Error epsilon:
*/

#include "conform.h"
#include "util.h"


static char errStr[240];
static char errStrFmt[][160] = {
    "Buffers disagree at pixel %d, with window system buffer = (%g, %g, %g), gl buffer = (%g, %g, %g).",
    "Buffers disagree at location %d, with window system buffer = %g, gl buffer = %g."
};


long ReadPixelsExec(void)
{
    TK_ScreenImageRec data;
    GLfloat *buf;
    float *trueBuf;
    GLfloat x1, y1, x2, y2;
    long i;

    /* 
    ** Test ReadPixels by comparing buffer as read by the window
    ** system with buffer read by glReadPixels.  An assymetric
    ** rectangle is drawn to check that buffer fills left to right,
    ** bottom to top.
    */

    buf = (GLfloat *)MALLOC(WINDSIZEX*WINDSIZEY*3*sizeof(GLfloat));
    trueBuf = (float *)MALLOC(WINDSIZEX*WINDSIZEY*3*sizeof(float));

    Ortho2D(0, WINDSIZEX, 0, WINDSIZEY);

    x1 = WINDSIZEX * 3.0 / 7.0;
    y1 = WINDSIZEY * 2.0 / 6.0;
    x2 = WINDSIZEX * 6.0 / 7.0;
    y2 = WINDSIZEY * 5.0 / 6.0;

    SETCLEARCOLOR(RED);
    glClear(GL_COLOR_BUFFER_BIT);

    SETCOLOR(GREEN);
    glRectf(x1, y1, x2, y2);

    if (buffer.colorMode == GL_RGB) {
	glReadPixels(0, 0, WINDSIZEX, WINDSIZEY, GL_RGB, GL_FLOAT, buf);

	data.x = 0;
	data.y = 0;
	data.width = WINDSIZEX;
	data.height = WINDSIZEY;
	data.colorMode = TK_WIND_RGB;
	data.data = trueBuf;
	tkGet(TK_SCREENIMAGE, (void *)&data);
	 
	/*
	** Compare the buffer read with the window system to the glRead buffer.
	*/
	for (i = 0; i < WINDSIZEX*WINDSIZEY*3; i += 3) {
	    if (ABS(trueBuf[i]-buf[i]) > epsilon.color[0] ||
		ABS(trueBuf[i+1]-buf[i+1]) > epsilon.color[1] ||
		ABS(trueBuf[i+2]-buf[i+2]) > epsilon.color[2]) {
		StrMake(errStr, errStrFmt[0], i/3, trueBuf[i], trueBuf[i+1],
			trueBuf[i+2], buf[i], buf[i+1], buf[i+2]);
		ErrorReport(__FILE__, __LINE__, errStr);
		FREE(trueBuf);
		FREE(buf);
		return ERROR;
	    }
	}
    } else {
	glReadPixels(0, 0, WINDSIZEX, WINDSIZEY, GL_COLOR_INDEX, GL_FLOAT,
		     (unsigned char *)buf);

	data.x = 0;
	data.y = 0;
	data.width = WINDSIZEX;
	data.height = WINDSIZEY;
	data.colorMode = TK_WIND_CI;
	data.data = trueBuf;
	tkGet(TK_SCREENIMAGE, (void *)&data);
	 
	/*
	** Compare the buffer read with the window system to the glRead buffer.
	*/
	for (i = 0; i < WINDSIZEX*WINDSIZEY; i++) {
	    if (ABS(trueBuf[i]-buf[i]) > epsilon.zero) {
		StrMake(errStr, errStrFmt[1], i, trueBuf[i], buf[i]);
		ErrorReport(__FILE__, __LINE__, errStr);
		FREE(trueBuf);
		FREE(buf);
		return ERROR;
	    }
	}
    }

    FREE(trueBuf);
    FREE(buf);
    return NO_ERROR;
}

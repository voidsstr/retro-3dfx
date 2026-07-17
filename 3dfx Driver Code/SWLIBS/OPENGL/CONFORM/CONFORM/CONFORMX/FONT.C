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
** font.c
** Font Test.
**
** Description -
**    This tests compares text produced through the window system
**    with strings produced through display lists with UseFont().
**    First, the letters of the alphabet, divided into two
**    strings, are written using the window system calls. The TK
**    routine chooses the font and returns the name of the font
**    used. The results are read read from the framebuffer and
**    stored in a local buffer. The same strings are drawn using
**    the GL window system calls, through TK, with the font name
**    returned from the first call. The framebuffer results are
**    also stored to a buffer. The buffers are then compared.
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
    "Failure in tkDrawFont.",
    "Buffers disagree at location %d, with window system buffer = %g, gl buffer = %g."
};


long FontExec(void)
{
    TK_ScreenImageRec data;
    char fontName[200];
    float *wsBuf, *glBuf, x, y;
    long width, height; 
    int x1, y1, x2, y2, i;
    GLint save;
    GLuint base;

    save = buffer.doubleBuf;
    buffer.doubleBuf = GL_FALSE;
    glDrawBuffer(GL_FRONT);
    glReadBuffer(GL_FRONT);
      
    wsBuf = (float *)MALLOC(WINDSIZEX*WINDSIZEY*sizeof(float));
    glBuf = (float *)MALLOC(WINDSIZEX*WINDSIZEY*sizeof(float));

    SETCLEARCOLOR(BLACK);
    SETCOLOR(GREEN);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glClear(GL_COLOR_BUFFER_BIT);

    x1 = 10;
    y1 = 10;
    x2 = 10;
    y2 = 50;

    /*
    ** Note that X coordinates have (0,0) in upper left, GL in lower left.
    ** Glyphs are drawn above their origin, not on it, so the y-coord
    ** sent down is one greater than the origin, to match the GL placement.
    */

    if (tkDrawFont(fontName, x1, WINDSIZEY-y1, "abcdefghijklm", 13) == 0) {
	StrMake(errStr, errStrFmt[0]);
	ErrorReport(__FILE__, __LINE__, errStr);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	FREE(glBuf);
	FREE(wsBuf);
	buffer.doubleBuf = save;
	return ERROR;
    }
    if (tkDrawFont(fontName, x2, WINDSIZEY-y2, "nopqrstuvwxyz", 13) == 0) {
	StrMake(errStr, errStrFmt[0]);
	ErrorReport(__FILE__, __LINE__, errStr);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	FREE(glBuf);
	FREE(wsBuf);
	buffer.doubleBuf = save;
	return ERROR;
    }

    /*
    ** Even in RGB mode, request CI data to get single pixel back.
    */
    data.x = 0;
    data.y = 0;
    data.width = WINDSIZEX;
    data.height = WINDSIZEY;
    data.colorMode = TK_WIND_CI;
    data.data = wsBuf;
    tkGet(TK_SCREENIMAGE, &data);

    glClear(GL_COLOR_BUFFER_BIT);

    width = WINDSIZEX;
    height = WINDSIZEY;
    base = (GLuint)tkLoadFont(fontName, &width, &height);
    if (base == 0) {
	ErrorReport(__FILE__, __LINE__, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	FREE(glBuf);
	FREE(wsBuf);
	buffer.doubleBuf = save;
	return ERROR;
    } else {
	glListBase(base);

	x = -1.0 + ((float) x1 + 0.25) * 2.0 / WINDSIZEX;
	y = -1.0 + ((float) y1 + 0.25) * 2.0 / WINDSIZEY;
	glRasterPos2f(x, y);
	glCallLists(13, GL_UNSIGNED_BYTE, (unsigned char *)"abcdefghijklm");

	x = -1.0 + ((float)x2 + 0.25) * 2.0 / WINDSIZEX;
	y = -1.0 + ((float)y2 + 0.25) * 2.0 / WINDSIZEY;
	glRasterPos2f(x, y);
	glCallLists(13, GL_UNSIGNED_BYTE, (unsigned char *)"nopqrstuvwxyz");

	data.data = glBuf;
	glFlush();
	tkGet(TK_SCREENIMAGE, &data);

	for (i = 0; i < data.width*data.height; i++) {
	    if (ABS(glBuf[i]-wsBuf[i]) > epsilon.ci) {
		StrMake(errStr, errStrFmt[1], i, glBuf[i], wsBuf[i]);
		ErrorReport(__FILE__, __LINE__, errStr);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		FREE(glBuf);
		FREE(wsBuf);
		buffer.doubleBuf = save;
		return ERROR;
	    }
	}
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    FREE(glBuf);
    FREE(wsBuf);
    buffer.doubleBuf = save;
    return NO_ERROR;
}

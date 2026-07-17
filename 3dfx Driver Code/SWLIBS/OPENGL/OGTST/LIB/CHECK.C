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

/* check.c - $Revision: 2$ */

#include <alloca.h>
#include "ogtst.h"
#include "../env/env.h"

#define BUFF_ALLOCA(BUF, SIZE) do { \
    BUF = (GLuint *) alloca(sizeof(GLuint) * (SIZE)); \
    if (BUF == NULL) \
            ogEnvLog(OG_LINTERNALERROR, "Alloca of buffer for check failed\n"); \
} while (0)

#define R	0
#define G	1
#define B	2
#define A	3

#define COMP(col, comp) (((col) >> ((3-comp) << 3)) & 0xff)

typedef enum { FILLED, FILLED_OUTLINE, OUTLINE } RectCheckMode;

extern opts_t opts;

static int            RGBAbits[4], RGBAoffComp, RGBAonAlpha, RGBfull,cimode;
#define CORRECT_MISSING_COMP(c) c = (c | RGBAonAlpha) & RGBAoffComp

static int	      DitherOn;
static unsigned int   IndexMask;
static GLboolean      multiSampled;
static GLboolean      ignoreMultiSampling = GL_FALSE;

static int  checkOutline(int x1, int y1, int x2, int y2, GLuint inc,
                         GLuint outc, RectCheckMode checkMode,
                         GLuint inError, GLuint outError, char *caller);
static int  checkRectBuff(GLuint *buff, int x1,int y1, int x2, int y2,
                          GLuint inc, GLuint outc, RectCheckMode checkMode,
                          GLuint inError, GLuint outError, char *caller);
static int  diffCheck(GLuint, GLuint, int);
static int  diffCheckByComponent(GLuint *color, GLuint *refColor,
                                 GLuint *maxError);
static int  checkSinglePixelLineBuff(register GLuint *buff, GLuint x, GLuint y,
                                     GLuint inc, GLuint outc, 
                                     GLuint inError, GLuint outError,
                                     GLuint n, GLboolean horiz, char *caller);
static int  lineCheck(int x1, int y1, int x2, int y2, GLuint inc, GLuint outc,
                      GLuint inError, GLuint outError, char *caller);
static int  checkLineBuff(GLuint *buff, int n, int x1, int y1, int x2, int y2,
                          GLuint inc, GLuint outc, GLuint inError,
                          GLuint outError, char *caller);
static int  lineTest(int,int,int,int,unsigned int);
#if 0
static int  dupBits(int,int);
#endif
static void colorToHighLow(unsigned int, unsigned int *, unsigned int *);

/*************************************************************
* ogLibColCheck() -
*************************************************************/
int
ogLibColCheck(GLuint actual, GLuint expected, int max_error)
{
    ogEnvLog(OG_LINTERNALDEBUG, "\t\t\t...ogLibColCheck() called\n");

    return(diffCheck(actual,expected,max_error));
}

/*************************************************************
* ogLibPixelCheck() - 
*************************************************************/
int
ogLibPixelCheck(int x, int y, unsigned int col)
{
    GLuint rcol;

    ogEnvLog(OG_LINTERNALDEBUG, "\t\t\t...ogLibPixelCheck() called\n");

    if (opts.flags & FLAG_NORDBACK) /* readback/checking disabled */
      return 0;

    ogLibReadPixels(x,y,x,y,&rcol);

    ogEnvLog(OG_LINTERNALDEBUG, "ogLibPixelCheck(): pixel (%d,%d) actual %s expected %s\n",
             x,y,ogEnvColorString(rcol),ogEnvColorString(col));
    
    if (diffCheck(rcol, col,0)) {
        if(!cimode)
           CORRECT_MISSING_COMP(col);
	ogEnvLog(OG_LFAIL, 
		 "ogLibPixelCheck(): pixel (%d,%d) actual %s expected %s\n", 
		 x,y,ogEnvColorString(rcol),ogEnvColorString(col));
        return(1);
    }

    return(0);
}

/*************************************************************
* ogLibRectCheck() - 
*************************************************************/
int
ogLibRectCheck(int x1, int y1, int x2, int y2,
               unsigned int inc, unsigned int outc)
{
    int err = 0;
    int dy,dx;

    ogEnvLog(OG_LINTERNALDEBUG, "\t\t\t...ogLibRectCheck() called\n");

    /* sort the coords */
    if (x2 < x1) { int tmp; tmp = x2; x2 = x1; x1 = tmp; }
    if (y2 < y1) { int tmp; tmp = y2; y2 = y1; y1 = tmp; }

    dx = x2-x1;
    dy = y2-y1;

    /* special case w or h of 1 */
    if (dx == 0 && dy == 0) 
	return lineCheck(x1, y1, x1, y1, inc, outc, 0, 0,
                         "Rect");
    else if (dx == 0 || dy == 0) 
	return lineCheck(x1, y1, x2, y2, inc, outc, 0, 0,
                         "Rect");

    if (multiSampled)
        err += checkOutline(x1, y1, x2, y2, inc, outc, FILLED_OUTLINE,
                            1, 1, "Rect");
    else {
        /* check outside */
        err += lineTest(x1-1,y1-1,x2-x1+3,1,outc);
        err += lineTest(x1-1,y2+1,x2-x1+3,1,outc);
        err += lineTest(x2+1,y1,y2-y1+1,0,outc);
        err += lineTest(x1-1,y1,y2-y1+1,0,outc);
    
        /* check perimeter */
        err += lineTest(x1,y1,x2-x1+1,1,inc);
        err += lineTest(x1,y2,x2-x1+1,1,inc);
        err += lineTest(x1,y1+1,y2-y1-1,0,inc);
        err += lineTest(x2,y1+1,y2-y1-1,0,inc);
    }
    /* check some interior area */
    if (dx > 4) {
	err += lineTest(ogLibIntRand(x1+1, x2-1), y1+1, y2-y1-1,0,inc);
	err += lineTest(ogLibIntRand(x1+1, x2-1), y1+1, y2-y1-1,0,inc);
    }
    if (dy > 4) {
	err += lineTest(x1+1,ogLibIntRand(y1+1, y2-1), x2-x1-1,1,inc);
	err += lineTest(x1+1,ogLibIntRand(y1+1, y2-1), x2-x1-1,1,inc);
    }

    return(err);
}

/*************************************************************
* ogLibRectEdgeCheck() -
*  Check an outlined rectangle of color inc in a background of
*  outc.
*************************************************************/
int
ogLibRectEdgeCheck(int x1, int y1, int x2, int y2, unsigned int inc,
                   unsigned int outc)
{
    int err = 0;
    int dy,dx;

    ogEnvLog(OG_LINTERNALDEBUG, "\t\t\t...ogLibRectEdgeCheck() called\n");

    /* sort the coords */
    if (x2 < x1) { int tmp; tmp = x2; x2 = x1; x1 = tmp; }
    if (y2 < y1) { int tmp; tmp = y2; y2 = y1; y1 = tmp; }

    dx = x2-x1;
    dy = y2-y1;

    /* special case w or h of 1 */
    if (dx == 0 && dy == 0) 
	return lineCheck(x1, y1, x1, y1, inc, outc, 0, 0, "RectEdge");
    else if (dx == 0 || dy == 0) 
	return lineCheck(x1, y1, x2, y2, inc, outc, 0, 0, "RectEdge");

    if (multiSampled) {
        return checkOutline(x1, y1, x2, y2, inc, outc, OUTLINE,
                            1, 1, "RectEdge");
    } else {
        /* check outside */
        err += lineTest(x1-1,y1-1,x2-x1+3,1,outc);
        err += lineTest(x1-1,y2+1,x2-x1+3,1,outc);
        err += lineTest(x2+1,y1,y2-y1+1,0,outc);
        err += lineTest(x1-1,y1,y2-y1+1,0,outc);

        /* check perimeter */
        err += lineTest(x1,y1,x2-x1+1,1,inc);
        err += lineTest(x1,y2,x2-x1+1,1,inc);
        err += lineTest(x1,y1+1,y2-y1-1,0,inc);
        err += lineTest(x2,y1+1,y2-y1-1,0,inc);

        /* check inside */
        err += lineTest(x1+1,y1+1,x2-x1-1,1,outc);
        err += lineTest(x1+1,y2-1,x2-x1-1,1,outc);
        err += lineTest(x1+1,y1+1,y2-y1-3,0,outc);
        err += lineTest(x2-1,y1+1,y2-y1-3,0,outc);
    }
    return(err);
}

/*
 * Check the outline of a rectangle, rectangle can be either filled or empty.
 * In the latter case the outside and inside colors must be the same.
 */
static int
checkOutline(int x1, int y1, int x2, int y2, GLuint inc, GLuint outc,
             RectCheckMode checkMode, GLuint inError, GLuint outError,
             char *caller)
{
    GLuint *buff;
    int dx, dy;

    if (opts.flags & FLAG_NORDBACK) /* readback/checking disabled */
      return 0;

    dx = x2 - x1;
    dy = y2 - y1;
    if (checkMode == FILLED_OUTLINE) { /* Don't deal with the interior */
        BUFF_ALLOCA(buff, 4*(dx + dy + 2));
        ogLibReadPixels(x1 - 1, y1 - 1, x1, y2 + 1, &buff[0]);
        ogLibReadPixels(x1 + 1, y2, x2 + 1, y2 + 1, &buff[2*(dy + 3)]);
        ogLibReadPixels(x2, y1 - 1, x2 + 1, y2 - 1, &buff[2*(dx + dy + 4)]);
        ogLibReadPixels(x1 + 1, y1 - 1, x2 - 1, y1,
                        &buff[2*(dx + 2*dy + 4 + 1)]);
    } else {
        BUFF_ALLOCA(buff, 6*(dx + dy + 2));
        ogLibReadPixels(x1 - 1, y1 - 1, x1 + 1, y2 + 1, &buff[0]);
        ogLibReadPixels(x1 + 2, y2 - 1, x2 + 1, y2 + 1, &buff[3*(dy + 3)]);
        ogLibReadPixels(x2 - 1, y1 - 1, x2 + 1, y2 - 2,
                        &buff[3*(dx + dy + 3)]);
        ogLibReadPixels(x1 + 2, y1 - 1, x2 - 2, y1 + 1,
                        &buff[3*(dx + 2*dy + 3)]);
    }
    return checkRectBuff(buff, x1, y1, x2, y2, inc, outc, checkMode,
                         inError, outError, caller);
}

/*
 * checkRectBuff -
 *  Verify that a buffer representing the edge pixels of a filled or
 *  outlined rectangle the sum to the right color.   The buffer contains the
 *  pixels of the neighborhood that has to be checked.
 *  There are three possible checking modes:
 *   - Check the edges of a filled rectangle.
 *   - Check the edges of an outlined rectangle
 *   - Check the edges and the inside of a filled rectangle.
 *  The checking modes are specified via the checkMode parameter.
 *  
 *  Although this routine works for non multisampled rectangles, its main use is
 *  for multisampled rectangles where due to subsampling the pixels covered by
 *  the rectangle are different from the non multisampled case, and there is
 *  also "blending" of the background and foreground color.
 *
 *  This routine assumes that the one pixel neighborhood around the rectangle/
 *  rectangle edges contains the a single background color.
 *
 *  No assumptions are made about the ordering of the pixels insided the buffer.
 *  The routine only checks that the sum total of the pixel colors is "close" to
 *  what it should be.
 */
static int
checkRectBuff(GLuint *buff, int x1,int y1, int x2, int y2, GLuint inc,
              GLuint outc, RectCheckMode checkMode,
              GLuint inError, GLuint outError, char *caller)
{
    GLuint refColor[4], totColor[4], pixColor[4], maxError[4];
    int nonOut, i, j,  halfCircum, err = 0, dx, dy;
    int totOut, totIn;

    totColor[R] = totColor[G] = totColor[B] = totColor[A] = 0;

    dx = x2 - x1; dy = y2 - y1;
    if (dx < 0) {
        dx = -dx;
        i = x2;
        x2 = x1;
        x1 = i;
    }
    if (dy < 0) {
        dy = -dy;
        i = y2;
        y2 = y1;
        y1 = i;
    }
    halfCircum = dx + dy;
    if (dx == 0)
        /* Works for points as well (i.e., dy == 0) */
        return checkLineBuff(buff, dy + 1,
                             x1, y1, x2, y2, inc, outc, inError, outError,
                             caller);
    else if (dy == 0)
        return checkLineBuff(buff, dx + 1,
                             x1, y1, x2, y2, inc, outc, inError, outError,
                             caller);
    maxError[R] = maxError[G] = maxError[B] = maxError[A] = outError;
    switch (checkMode) {
      case FILLED_OUTLINE:
        i = 4 * halfCircum + 8;
        totOut = 2 * (halfCircum + 4);
        totIn = 2*halfCircum;
        break;
      case OUTLINE:
        totOut = 4 * halfCircum;
        i = 6 * halfCircum;
        totIn = 2*halfCircum;
        break;
      case FILLED:
        i = (dx + 3) * (dy + 3);
        totOut = 2 * (halfCircum + 4);
        totIn = (dx + 1) * (dy + 1);
        break;
    }
    
    CORRECT_MISSING_COMP(outc);
    for (j = R; j <= A; j++)
        refColor[j] = COMP(outc, j);
    for (nonOut = 0; i; i--, buff++) {
        for (j = R; j <= A; j++) {
            pixColor[j] = COMP(*buff, j);
            totColor[j] += pixColor[j];
        }
        if (diffCheckByComponent(pixColor, refColor, maxError))
            nonOut++;
    }
    if (checkMode == OUTLINE && nonOut > 4 * (halfCircum + 2)) {
        ogEnvLog(OG_LFAIL, 
                 "ogLib%sCheck(): multisampled rectangle (%d,%d)->(%d,%d)\
 has %d (> %d) pixels covered for colors in=%s and out=%s\n", caller,
                 x1, y1, x2, y2, nonOut, 4 * (halfCircum + 2),
                 ogEnvColorString(inc), ogEnvColorString(outc));
        err += nonOut - 4 * (halfCircum + 2);
    } else {
        if (inError == 0)
            inError = 1;
        CORRECT_MISSING_COMP(inc);
        for (j = R; j <= A; j++) {
            refColor[j] = totIn * COMP(inc, j) +
                totOut * refColor[j];
            /*
             * Allows error of in/outError per component per possible in/out
             * covered pixel.
             * For outline we allow additional 4 * inc trying to
             * account for overlaps at the corners.
             * The extra error for the corners is too high but figuring out
             * a tighter bound is too much work - Ziv.
             */
            maxError[j] = outError * totOut + inError * totIn;
            if (checkMode == OUTLINE)
                maxError[j] += 4 * COMP(inc, j);                
        }
        if (diffCheckByComponent(totColor, refColor, maxError)) {
            ogEnvLog(OG_LFAIL,
                     "ogLibRect%s(): multisampled rectangle (%d,%d)->(%d,%d)\
 neighborhood is (%d, %d, %d, %d) not (%d, %d, %d, %d).\n",
                     caller, x1, y1, x2, y2,
                     totColor[R], totColor[G], totColor[B], totColor[A],
                     refColor[R], refColor[G], refColor[B], refColor[A]);
        }
    }
    return err;
}

int
ogLibRectBufCheck(GLuint *buff, int x1,int y1, int x2, int y2, GLuint inc,
                  GLuint outc, GLboolean filled,
                  GLuint inError, GLuint outError)
{
    return checkRectBuff(buff, x1, y1, x2, y2, inc, outc,
                         filled ? FILLED : OUTLINE, inError, outError,
                         "RectBuf");
}

/*************************************************************
*  Check whether a color is in the range of 2 other colors.
*  Range is corrected for low bit systems and dithering.
*  This function is intended for testing things like gouraud
*  shading.
*************************************************************/

int
ogLibRangeCheck(unsigned int testcol, unsigned int col1, unsigned int col2)
{
    int i;
    unsigned int low1,high1,low2,high2,high,low;
    unsigned int c1,c2,t;
    
    colorToHighLow(col1,&low1,&high1);
    colorToHighLow(col2,&low2,&high2);
    if (cimode) {
	if (col1 < col2) {
	    low = low1;
	    high = high2;
	} else {
	    low = low2;
	    high = high1;
	}
	if (testcol < low || testcol > high) {
	    ogEnvLog(OG_LFAIL,
		"ogLibRangeCheck: index 0x%x is outside range [0x%x,0x%x]\n", 
			testcol,low,high);
	    return 1;
	}
    } else {
	for (i = R; i <= A; i++) {
	    c1 = COMP(col1,i);
	    c2 = COMP(col2,i);
	    t = COMP(testcol,i);
	    if (c1 < c2) {
		low = COMP(low1,i);
		high = COMP(high2,i);
	    } else {
		low = COMP(low2,i);
		high = COMP(high1,i);
	    }
	    if (t < low || t > high) {
		ogEnvLog(OG_LFAIL,
		"ogLibRangeCheck: comp: %d of 0x%x is outside range [0x%x,0x%x]\n", i,testcol,low,high);
		return 1;
	    }
	}
    }
    return 0;
}


/*************************************************************
*  Do a pixel equality comparison, taking RGB dithering into account.
*  Returns TRUE if the actual is different from the expected.  
*  Use this routine to check the results of things that are rendered 
*  (cf. writing pixels directly).
*  The max_error provided should be in the range 0-255.
*************************************************************/
static int
diffCheck(GLuint actual, GLuint expected, int max_error)
{
    int i, a, e;
    float shade;

    if (cimode) {
	return (ABS((int)actual-(int)expected) > (max_error+2*DitherOn));
    }

    CORRECT_MISSING_COMP(expected);
    if (RGBfull) {
	/* 24+ bits RGB color */
        max_error += DitherOn;  /* allow a variance if dithering */
	for (i = R; i <= A; i++) {
	    e = COMP(expected, i);
	    a = COMP(actual, i);
	    if (ABS(e-a) > max_error)
		return(1);
	}
    } else {
	/* 4,8,12 bits RGB color */
	for (i = 0; i <= A; i++) {
	    if (DitherOn && RGBAbits[i]) {
		shade = 255.0 / ((1 << RGBAbits[i]) - 1);
		e = (float)((int)(COMP(expected,i)/shade+0.5))*shade+0.5;
		a = (float)((int)(COMP(actual,i)/shade+0.5))*shade+0.5;
                if (e < 0) {
                    e = 0;
                } else if (e > 255) {
                    e = 255;
                }
                if (a < 0) {
                    a = 0;
                } else if (a > 255) {
                    a = 255;
                }
	    } else {
		e = COMP(expected, i);
		a = COMP(actual, i);
		shade = 0.0;
	    }
	    if (ABS(e-a) > (max_error+2*(int)(shade+0.5)))
		return(1);
	}
    }
    
    return(0);
}

/*
 * Componet by component comparison of two "colors".  The components can
 *  be any integer.
 */
static int
diffCheckByComponent(GLuint *color, GLuint *refColor, GLuint *maxError)
{
    return ABS((int) color[R] - (int) refColor[R]) > maxError[R] ||
        ABS((int) color[G] - (int) refColor[G]) > maxError[G] ||
        ABS((int) color[B] - (int) refColor[B]) > maxError[B] ||
        ABS((int) color[A] - (int) refColor[A]) > maxError[A];
}


/*************************************************************
*  determine the minimum and maximum representation of a color
*  based on the number of bits per component and dithering.
*************************************************************/

static void
colorToHighLow(unsigned int color, unsigned int *low, unsigned int *high)
{
    int ic[4];
    int lc[4],hc[4];
    int hicol,locol;
    int i;
    float shade;
    
    if (cimode) {
	color &= IndexMask;
	locol = color - 1;
	hicol = color + 1;
	*low = locol < 0 ? 0 : locol;
	*high = hicol > IndexMask ? IndexMask : hicol;
	return;
    }
    CORRECT_MISSING_COMP(color);

    ic[0] = COMP(color,0);
    ic[1] = COMP(color,1);
    ic[2] = COMP(color,2);
    ic[3] = COMP(color,3);
    /* 24+ bits RGB color */
    if (RGBfull) {
	lc[0] = ic[0] - 1;
	lc[1] = ic[1] - 1;
	lc[2] = ic[2] - 1;
	lc[3] = ic[3] - 1;
	lc[0] = lc[0] < 0 ? 0 : lc[0];
	lc[1] = lc[1] < 0 ? 0 : lc[1];
	lc[2] = lc[2] < 0 ? 0 : lc[2];
	lc[3] = lc[3] < 0 ? 0 : lc[3];
        *low = (lc[0]<<24) | (lc[1]<<16) | (lc[2]<<8) | lc[3];
	hc[0] = ic[0] + 1;
	hc[1] = ic[1] + 1;
	hc[2] = ic[2] + 1;
	hc[3] = ic[3] + 1;
	hc[0] = hc[0] > 255 ? 255 : hc[0];
	hc[1] = hc[1] > 255 ? 255 : hc[1];
	hc[2] = hc[2] > 255 ? 255 : hc[2];
	hc[3] = hc[3] > 255 ? 255 : hc[3];
        *high = (hc[0]<<24) | (hc[1]<<16) | (hc[2]<<8) | hc[3];
    } else {
        /* 4,8,12 bits RGB color */
	for (i = 0; i <= A; i++) {
	    if (DitherOn && RGBAbits[i]) {
		shade = 255.0 / ((1 << RGBAbits[i]) - 1);
		locol = ((int)((float)ic[i]/shade+0.5) - 1) * shade + 0.5;
		hicol = ((int)((float)ic[i]/shade+0.5) + 1) * shade + 0.5;
	    } else {
		locol = ic[i];
		hicol = ic[i];
	    }
	    if (locol < 0) {
		locol = 0;
	    } else if (locol > 255) {
		locol = 255;
	    }
	    if (hicol < 0) {
		hicol = 0;
	    } else if (hicol > 255) {
		hicol = 255;
	    }
	    lc[i] = locol;
	    hc[i] = hicol;
	}
        *low = (lc[0]<<24) | (lc[1]<<16) | (lc[2]<<8) | lc[3];
        *high = (hc[0]<<24) | (hc[1]<<16) | (hc[2]<<8) | hc[3];
    }
}

/*************************************************************
* ogLibPointCheck() -
*************************************************************/
int
ogLibPointCheck(int x,int y,unsigned int inc,unsigned int outc,
                unsigned int maxErr)
{
    ogEnvLog(OG_LINTERNALDEBUG, "\t\t\t...ogLibPointCheck() called\n");

    return lineCheck(x, y, x, y, inc, outc, maxErr, maxErr, "Point");
}

/*
 * checkSinglePixelLineBuff - 
 *  Check that a horizonal/vertical line represented by the given buffer is
 *  of the required colors within the given error.
 * The line is of length n, with lower/left endpoint given by x,y.
 * Note that the size of the buffer has to be n+2.
 */
static int
checkSinglePixelLineBuff(register GLuint *buff, GLuint x, GLuint y,
                         GLuint inc, GLuint outc,
                         GLuint inError, GLuint outError, 
                         GLuint n, GLboolean horiz, char *caller)
{
    int i, err = 0;

    /* Outside the first endpoints */
    if (diffCheck(*buff, outc, outError)) {
        if(!cimode)
           CORRECT_MISSING_COMP(outc);
        ogEnvLog(OG_LFAIL, 
                 "ogLib%sCheck(): pixel (%d,%d) is %s not %s\n",
                 caller, horiz ? x - 1 : x, horiz ? y : y - 1, 
                 ogEnvColorString(*buff),
                 ogEnvColorString(outc));
        err++;
    }
     *buff++;
    /* The inside of the line */
    for (i = 0; i < n; i++) {
        if (diffCheck(*buff, inc, inError)) {
            if(!cimode)
               CORRECT_MISSING_COMP(inc);
            ogEnvLog(OG_LFAIL,
                     "ogLib%sCheck(): pixel (%d,%d) is %s not %s\n",
                     caller, horiz ? x + i : x, horiz ? y : y + i, 
                     ogEnvColorString(*buff),
                     ogEnvColorString(inc));
            err++;
        }
      *buff++;
    }
    /* Outside the last endpoint */
    if (diffCheck(*buff, outc, outError)) {
        if(!cimode) 
           CORRECT_MISSING_COMP(outc);
        ogEnvLog(OG_LFAIL,
                 "ogLib%sCheck(): pixel (%d,%d) is %s not %s\n",
                 caller, horiz ? x + n : x, horiz ? y : y + n, 
                 ogEnvColorString(*buff),
                 ogEnvColorString(outc));
        err++;
    }
    return err;
}

static int 
lineCheck(int x1, int y1, int x2, int y2, GLuint inc, GLuint outc,
          GLuint inError, GLuint outError, char *caller)
{
    GLuint *center, *right, *left, *buff;
    int n,horiz;
    int err;

    if (opts.flags & FLAG_NORDBACK) /* readback/checking disabled */
      return 0;

    err = 0;
    if (y1 != y2) {
	horiz = 0;
	n = y2 - y1 + 1;
    } else {
	horiz = 1;
	n = x2 - x1 + 1;
    }

    BUFF_ALLOCA(buff, 3*(n + 2));
    right = &buff[0];
    center = &buff[n + 2];
    left = &buff[2 * (n + 2)];
    /* grab the line and the two rows bordering it */
    if (horiz) {
        ogLibReadPixels(x1 - 1, y1 - 1, x2 + 1, y1 + 1, buff);
    } else {
        ogLibReadPixels(x1 - 1, y1 - 1, x1 - 1, y2 + 1, left);
        ogLibReadPixels(x1, y1 - 1, x1, y2+1, center);
        ogLibReadPixels(x1 + 1, y1 - 1, x1 + 1, y2 + 1, right);
    }

    if (multiSampled) 
        return checkLineBuff(buff, n, x1, y1, x2, y2, inc, outc,
                             inError, outError < 1 ? 1 : outError,
                             caller);
    else {
        /* The inc pixels */
        err = checkSinglePixelLineBuff(center, x1, y1, inc, outc,
                                       inError, outError, n, horiz, caller);
        /* Outside the line */
        err += checkSinglePixelLineBuff(left, horiz ? x1 : x1 - 1,
                                        horiz ? y1 + 1 : y1, outc, outc,
                                        inError, outError, n, horiz, caller);
        err += checkSinglePixelLineBuff(right, horiz ? x1 : x1 + 1,
                                        horiz ? y1 - 1 : y1, outc, outc,
                                        inError, outError, n, horiz, caller);
    }
    return err;
}

/*
 * n is the length of the line.  That is the buffer should contain
 * 3*(n+2) pixels: the line itself and the 1pixel neighborhood
 * around it. The x1/2,y1/2 are used just for the error message.
 */
static int
checkLineBuff(GLuint *buff, int n, int x1, int y1, int x2, int y2,
              GLuint inc, GLuint outc, GLuint inError, GLuint outError,
              char *caller)
{
    int nonOut, i, j, err = 0;
    GLuint refColor[4], totColor[4], pixColor[4], maxError[4];
        
    totColor[R] = totColor[G] = totColor[B] = totColor[A] = 0;
    if (inError == 0)
        inError = 1;
    maxError[R] = maxError[G] = maxError[B] = maxError[A] = outError;
    CORRECT_MISSING_COMP(outc);
    for (j = R; j <= A; j++)
        refColor[j] = COMP(outc, j);
        
    for (nonOut = 0, i = 3 * (n + 2); i; i--, buff++) {
        for (j = R; j <= A; j++) {
            pixColor[j] = COMP(*buff, j);
            totColor[j] += pixColor[j];
        }
        if (diffCheckByComponent(pixColor, refColor, maxError))
            nonOut++;
    }
        
    if (nonOut > 2 * (n + 1)) {
        ogEnvLog(OG_LFAIL, 
                 "ogLib%sCheck(): multisampled point/line (%d,%d)->(%d,%d)\
has %d (> %d) pixels covered for colors in=%s and out=%s\n",
                 caller, x1, y1, x2, y2, nonOut, 2 * (n + 1),
                 ogEnvColorString(inc), ogEnvColorString(outc));
#if DEBUG_MULTISAMPLE
        for (i = 0; i < 3; i++)
            printf(" %s %s %s\n",
                   ogEnvColorString(col[3*i]),
                   ogEnvColorString(col[3*i + 1]),
                   ogEnvColorString(col[3*i + 2]));
#endif
        err = nonOut - 2*(n + 1);
    } else {
        CORRECT_MISSING_COMP(inc);
        for (j = R; j <= A; j++) {
            refColor[j] = n * COMP(inc, j) + (2 * n + 6) * refColor[j];
            maxError[j] = n * inError + (2 * n + 6) * outError ;
        }
        if (diffCheckByComponent(totColor, refColor, maxError)) {
#if DEBUG_MULTISAMPLE
            printf("In %s out %s\n",
                   ogEnvColorString(inc),ogEnvColorString(outc));
            for (i = 0; i < 3; i++)
                printf(" %s %s %s\n",
                       ogEnvColorString(col[3*i]),
                       ogEnvColorString(col[3*i + 1]),
                       ogEnvColorString(col[3*i + 2]));
#endif
            ogEnvLog(OG_LFAIL, 
                     "ogLib%sCheck(): multisampled point/line\
 (%d,%d)->(%d,%d) neighborhood is (%d, %d, %d, %d) not (%d, %d, %d, %d).\n",
            caller, x1, y1, x2, y2,
            totColor[R], totColor[G], totColor[B], totColor[A],
            refColor[R], refColor[G], refColor[B], refColor[A]);
            err++;
        }
    }
    return err;
}

int
ogLibLineBufCheck(GLuint *buff, int size, int x1, int y1, int x2, int y2,
                   GLuint inc, GLuint outc, GLuint inError, GLuint outError)
{
    return checkLineBuff(buff, size, x1, y1, x2, y2, inc, outc,
                         inError, outError, "LineBuff");
}

/*************************************************************
* lineTest() -
*************************************************************/
static int 
lineTest(int x, int y, int n, int horiz,unsigned int expc)
{
    int i,err;
    GLuint *col;

    if (opts.flags & FLAG_NORDBACK) /* readback/checking disabled */
      return 0;

    if (n <= 0)
	return 0;

    err = 0;
    BUFF_ALLOCA(col, n);
    if (horiz) {
	ogLibReadPixels(x,y,x+n-1,y,col);
    } else {
	ogLibReadPixels(x,y,x,y+n-1,col);
    }
    for (i=0; i < n; i++) {
	if (diffCheck(col[i], expc, 0)) {
            if(!cimode)
               CORRECT_MISSING_COMP(expc);
	    ogEnvLog(OG_LFAIL,
		"ogLibRectCheck(): pixel (%d,%d) is %s not %s\n",
		 horiz ? x+i : x, horiz ? y : y+i, 
		     ogEnvColorString(col[i]),
		 ogEnvColorString(expc));
	    err++;
	}	
    }
    return err;
}

#if 0
/*************************************************************
*  dupBits()  -  duplicate the bits when less than 8
*************************************************************/
static int 
dupBits(int col,int bits)
{
    int ret=0;

    if (col < 0)
      col = 0;
    else if (col > 255)
      col = 255;

    col &= (((1<<bits)-1) << (8-bits));

    switch (bits)  {
      case 0 :
        ret = 0xff;
        break;

      case 1 :
	ret = (col > 127) ? 0xff : 0x00;
        break;

      case 2 :
        ret |= col >> (bits+bits+bits);
      case 3 :
        ret |= col >> (bits+bits);
      case 4 :
        ret |= col >> bits;
        ret |= col;
        break;

      default:
	ogEnvLog(OG_LINTERNALERROR,"duplicate_bits(%d,%d)\n",col,bits);
        break;
      }

    return(ret);
}
#endif

/*************************************************************
 * ogLibCheckPerVisualInit() -
 *  Get color buffer configuration for checking purposes.
 *************************************************************/
void
ogLibCheckPerVisualInit(void)
{
    int i;
    
    multiSampled = ignoreMultiSampling ? GL_FALSE : ogEnvIsMultiSampled();
    DitherOn = !multiSampled && !(opts.flags & FLAG_COLOR) && glIsEnabled(GL_DITHER);
    
    ogEnvColorBits(&RGBAbits[R], &RGBAbits[G], &RGBAbits[B], &RGBAbits[A],
                   &cimode);
    if (cimode)  {
        IndexMask = (1 << ogEnvCurVisualInfo(GLX_BUFFER_SIZE)) - 1;
    } else {
        RGBfull = RGBAbits[R] + RGBAbits[G] + RGBAbits[B] >= 24;
        /* turn off dither flag if greater than 8 bits/comp */
        if (RGBAbits[R] > 8 && RGBAbits[G] > 8 && 
            RGBAbits[B] > 8 && RGBAbits[A] > 8) {
            DitherOn = 0;
        }
        RGBAoffComp = 0xffffffff;
        for (i = 0; i < 3; i++)
            if (!RGBAbits[i])
                RGBAoffComp &= ~(0xff000000 >> (8*i));
        RGBAonAlpha = RGBAbits[A] ? 0 : 0xff;
            
    }
}


/*
 * ogLibCheckIgnoreMultiSampling(int mode)
 *  Turns ignoring of the multiSampling state on/off.
 *  This is to enable tests to control whether the checking takes multisampling
 *  into account.
 */
void
ogLibCheckIgnoreMultiSampling(GLboolean mode)
{
    ignoreMultiSampling = mode;
    multiSampled = ignoreMultiSampling ? GL_FALSE : ogEnvIsMultiSampled();
    DitherOn = multiSampled ? 0 : glIsEnabled(GL_DITHER);
}

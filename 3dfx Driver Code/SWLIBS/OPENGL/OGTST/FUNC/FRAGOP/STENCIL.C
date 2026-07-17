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

/* stencil.c - $Revision: 2$ */

#include "ogtst.h"		/* include test environment		 */

#if 0
#define  CA          0xfedcba98	/* color for box A */
#define  CB          0x01234567	/* color for box B */
#endif

#define  NC          -1		/* don't-care value */
#define  ST_NORMAL(s)   dotest(CA,s, CA,s, CB,s,  CA,s, CB,s, CB,s)

static unsigned int mask;

static GLint CA, CB;
static GLint stbits;
static void runtests(void);
static void do_one_bit_test(void);
static void dotest(int, int, int, int, int, int, int, int, int, int, int, int);
static void teststencil(int, int, int, int);
static void doclear(int);
static void check(int, int, int);

static int
BitsToColor(int bits)
{
    int x;

    if (bits == 0) {
        return 0;
    } else {
        if (bits > 8) {
            bits = 8;
        }
        x = (1 << bits) - 1;
        return (255 / x) * ogLibIntRand(0, x);
    }
}

/*ARGSUSED*/
TESTMOD(stencil) {
    int r, g, b, a, rBits, gBits, bBits, aBits, ciBits;

    ogEnvColorBits(&rBits, &gBits, &bBits, &aBits, &ciBits);

    r = BitsToColor(rBits);
    g = BitsToColor(gBits);
    b = BitsToColor(bBits);
    a = BitsToColor(aBits);
    CA = ((r << 24) & 0xFF000000) | ((g << 16) & 0x00FF0000) |
	 ((b << 8) & 0x0000FF00) | (a & 0x000000FF);

    r = BitsToColor(rBits);
    g = BitsToColor(gBits);
    b = BitsToColor(bBits);
    a = BitsToColor(aBits);
    CB = ((r << 24) & 0xFF000000) | ((g << 16) & 0x00FF0000) |
	 ((b << 8) & 0x0000FF00) | (a & 0x000000FF);

    glEnable(GL_DEPTH_TEST);
    stbits = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
    ogEnvLog(1, "a %i bit stencil buffer is being used\n", stbits);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    if (stbits == 1) {
	do_one_bit_test();
	return;
    }

    /* test default swritemask */
    ogEnvLog(1, "--- default test ---\n");
    mask = (1 << stbits) - 1;
    doclear(0);
    ST_NORMAL(0);
    doclear(0xFF);
    ST_NORMAL(0xFF);

    mask = (1 << stbits) - 1;
    ogEnvLog(1, "mask = %d\n", mask);
    runtests();

    ogEnvLog(1, "--- swritemask test ---\n");
    doclear(0);
    glStencilMask(0x12);
    glClearStencil(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    ST_NORMAL(0x12);
    glStencilMask(0x5A);
    glClearStencil(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    ST_NORMAL(0x5A);

    ogEnvLog(1, "--- masked stencil test ---\n");
    doclear(0);
    mask = 7;
    ogEnvLog(1, "mask = %d\n", mask);
    runtests();
}

CLEANUP(stencil) {
    ogLibSetDefaultColors();

    glDepthFunc(GL_LESS);
    glDepthRange(0., 1.);
    glDisable(GL_DEPTH_TEST);
    glClearDepth(1.0);

    stbits = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
    glStencilMask((1 << stbits) - 1);
    glStencilFunc(GL_ALWAYS, 0, (1 << stbits) - 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);
    glClearStencil(0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

/*************************************************************
*  runtests()  -
*************************************************************/
static void
runtests(void) {
    int i, j, k;

    glStencilMask(mask);
    ogEnvLog(1, "enable test\n");
    doclear(0x55);
    ST_NORMAL(0x55);

    doclear(0);
    glStencilFunc(GL_NEVER, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);
    ST_NORMAL(0);

    doclear(0);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glDisable(GL_STENCIL_TEST);
    ST_NORMAL(0);

    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NEVER, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    dotest(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);


    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    ST_NORMAL(0);

    doclear(0);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glDisable(GL_STENCIL_TEST);
    ST_NORMAL(0);

    ogEnvLog(1, "basic plane ops\n");
    ogEnvLog(1, "  REPLACE\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    ST_NORMAL(mask);
    doclear(0xFF);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x0, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    ST_NORMAL(0);

    ogEnvLog(1, "  ZERO\n");
    doclear(0xFF);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    ST_NORMAL(0);

    ogEnvLog(1, "  INCR\n");
    for (i = 1; i <= 8; i++) {
	j = (i + 1);
	if (j > mask)
	    break;
	doclear(i - 1);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOp(GL_INCR, GL_INCR, GL_INCR);
	dotest(CA, i, CA, j, CB, i, CA, i, CB, j, CB, i);
    }
    for (i = 0xFF; i != 7; i = i >> 1) {
	if ((i + 2) > mask)
	    continue;
	doclear(i);
	j = i + 1;
	if (j > 255)
	    j = 255;
	k = i + 2;
	if (k > 255)
	    k = 255;
	ogEnvLog(2, "  i=%x j=%x k=%x\n", i, j, k);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOp(GL_INCR, GL_INCR, GL_INCR);
	dotest(CA, j, CA, k, CB, j, CA, j, CB, k, CB, j);
    }

    ogEnvLog(1, "  DECR\n");
    for (i = 1; i < 8; i++) {
	if ((i * 2) > mask)
	    break;
	doclear(255 - i + 1);
	k = 255 - i;
	j = k - 1;
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0x0, 0xFF);
	glStencilOp(GL_DECR, GL_DECR, GL_DECR);
	dotest(CA, k, CA, j, CB, k, CA, k, CB, j, CB, k);
    }
    for (i = 128; i; i = i >> 1) {
	if (i > mask)
	    continue;
	doclear(i);
	j = i - 1;
	if (j < 0)
	    j = 0;
	k = i - 2;
	if (k < 0)
	    k = 0;
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0x0, 0xFF);
	glStencilOp(GL_DECR, GL_DECR, GL_DECR);
	dotest(CA, j, CA, k, CB, j, CA, j, CB, k, CB, j);
    }

    ogEnvLog(1, "  INVERT\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xF);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    dotest(CA, 0xFF, CA, 0, CB, 0xFF, CA, 0xFF, CB, 0, CB, 0xFF);

    ogEnvLog(1, "stencil compare\n");
    ogEnvLog(1, "  GREATER\n");	/* ref > screen */
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_GREATER, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, 0, 2, 0, 1, 0, 1, 0, 2, 0, 1);
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_GREATER, 1, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(CA, 1, CA, 2, CB, 1, CA, 1, CA, 2, CB, 1);
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_GREATER, 2, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(CA, 1, CA, 2, CB, 1, CA, 1, CB, 2, CB, 1);

    ogEnvLog(1, "  EQUAL\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(CA, 1, CA, 2, CB, 1, CA, 1, CA, 2, CB, 1);
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, CB, 2, 0, 1, 0, 1, CB, 2, 0, 1);

    ogEnvLog(1, "  GEQUAL\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_GEQUAL, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(CA, 1, CA, 2, CB, 1, CA, 1, CA, 2, CB, 1);
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_GEQUAL, 1, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    ST_NORMAL(NC);

    ogEnvLog(1, "  LESS\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, CB, 2, 0, 1, 0, 1, CB, 2, 0, 1);
    doclear(1);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(CA, 2, CA, 3, CB, 2, CA, 2, CB, 3, CB, 2);
    if ((mask & 15) == 15) {
	if (stbits > 15) {
	    doclear(14);
	    glEnable(GL_STENCIL_TEST);
	    glStencilFunc(GL_LESS, 13, 0xFF);
	    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
	    dotest(CA, 15, CA, 16, CB, 15, CA, 15, CB, 16, CB, 15);
	} else {
	    doclear(13);
	    glEnable(GL_STENCIL_TEST);
	    glStencilFunc(GL_LESS, 12, 0xFF);
	    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
	    dotest(CA, 14, CA, 15, CB, 14, CA, 14, CB, 15, CB, 14);
	}
    }

    ogEnvLog(1, "  NOTEQUAL\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, CB, 2, 0, 1, 0, 1, CB, 2, 0, 1);
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(CA, 1, CA, 2, CB, 1, CA, 1, CA, 2, CB, 1);

    ogEnvLog(1, "  LEQUAL\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LEQUAL, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    ST_NORMAL(NC);
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LEQUAL, 1, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, CB, 2, 0, 1, 0, 1, CB, 2, 0, 1);
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LEQUAL, 2, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, 0, 2, 0, 1, 0, 1, 0, 2, 0, 1);

    ogEnvLog(1, "differing stencil ops\n");
    ogEnvLog(1, "  zpass\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    dotest(CA, 0xFF, CA, 0xFF, CB, 0xFF, CA, 0xFF, CB, 0, CB, 0xFF);
    ogEnvLog(1, "  pass\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_INVERT, GL_KEEP);
    dotest(CA, 0, CA, 0xFF, CB, 0, CA, 0, CB, 0, CB, 0);
    ogEnvLog(1, "  fail\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_INVERT, GL_KEEP, GL_INCR);
    dotest(CA, 1, CA, 0xFE, CB, 1, CA, 1, CA, 0xFE, CB, 1);

    ogEnvLog(1, "compare mask\n");
    for (i = 128; i; i = i >> 1) {
	if (i > mask)
	    continue;
	j = ~i & 0xFF;
	ogEnvLog(2, "   %x vs mask %x\n", i, j);
	doclear(i);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, j);
	glStencilOp(GL_ZERO, GL_KEEP, GL_KEEP);
	/* w/ 1 masked out, planes shd equal 0 -- if not, clr */
	ST_NORMAL(i);
    }
    for (i = 128; i; i = i >> 1) {
	if (i > mask)
	    continue;
	doclear(i);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0xFF, i);
	glStencilOp(GL_KEEP, GL_ZERO, GL_ZERO);
	/* w/ only 1 tested, plane shd equal 1 -- if so, clr */
	dotest(CA, 0, CA, 0, CB, 0, CA, 0, CA, 0, CB, 0);
    }

    ogEnvLog(1, "pass done.\n");
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NEVER, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

/*************************************************************
*  do_one_bit_test()  - workaround for braindead test
*************************************************************/
void do_one_bit_test(void) {
    int i, j, k;

    /* test default swritemask */
    ogEnvLog(1, "--- default test ---\n");
    mask = (1 << stbits) - 1;
    doclear(0);
    ST_NORMAL(0);
    doclear(0xFF);
    ST_NORMAL(0xFF);

    glStencilMask(mask);
    ogEnvLog(1, "enable test\n");
    doclear(0x55);
    ST_NORMAL(0x55);

    doclear(0);
    glStencilFunc(GL_NEVER, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);
    ST_NORMAL(0);

    doclear(0);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glDisable(GL_STENCIL_TEST);
    ST_NORMAL(0);

    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NEVER, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    dotest(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);


    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    ST_NORMAL(0);

    doclear(0);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glDisable(GL_STENCIL_TEST);
    ST_NORMAL(0);

    ogEnvLog(1, "basic plane ops\n");
    ogEnvLog(1, "  REPLACE\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    ST_NORMAL(mask);
    doclear(0xFF);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x0, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    ST_NORMAL(0);

    ogEnvLog(1, "  ZERO\n");
    doclear(0xFF);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    ST_NORMAL(0);

    ogEnvLog(1, "  INCR\n");
    for (i = 1; i <= 8; i++) {
        j = (i + 1);
        if (j > mask)
            break;
        doclear(i - 1);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilOp(GL_INCR, GL_INCR, GL_INCR);
        dotest(CA, i, CA, j, CB, i, CA, i, CB, j, CB, i);
    }
    for (i = 0xFF; i != 7; i = i >> 1) {
        if ((i + 2) > mask)
            continue;
        doclear(i);
        j = i + 1;
        if (j > 255)
            j = 255;
        k = i + 2;
        if (k > 255)
            k = 255;
        ogEnvLog(2, "  i=%x j=%x k=%x\n", i, j, k);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilOp(GL_INCR, GL_INCR, GL_INCR);
        dotest(CA, j, CA, k, CB, j, CA, j, CB, k, CB, j);
    }

    ogEnvLog(1, "  DECR\n");
    for (i = 1; i < 8; i++) {
        if ((i * 2) > mask)
            break;
        doclear(255 - i + 1);
        k = 255 - i;
        j = k - 1;
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0x0, 0xFF);
        glStencilOp(GL_DECR, GL_DECR, GL_DECR);
        dotest(CA, k, CA, j, CB, k, CA, k, CB, j, CB, k);
    }
    for (i = 128; i; i = i >> 1) {
        if (i > mask)
            continue;
        doclear(i);
        j = i - 1;
        if (j < 0)
            j = 0;
        k = i - 2;
        if (k < 0)
            k = 0;
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0x0, 0xFF);
        glStencilOp(GL_DECR, GL_DECR, GL_DECR);
        dotest(CA, j, CA, k, CB, j, CA, j, CB, k, CB, j);
    }

    ogEnvLog(1, "  INVERT\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xF);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    dotest(CA, 0xFF, CA, 0, CB, 0xFF, CA, 0xFF, CB, 0, CB, 0xFF);

    ogEnvLog(1, "stencil compare\n");
    ogEnvLog(1, "  GREATER\n"); /* ref > screen */
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_GREATER, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1);

    ogEnvLog(1, "  EQUAL\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(CA, 1, CA, 1, CB, 1, CA, 1, CA, 1, CB, 1);

    ogEnvLog(1, "  GEQUAL\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_GEQUAL, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(CA, 1, CA, 1, CB, 1, CA, 1, CA, 1, CB, 1);

    ogEnvLog(1, "  LESS\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, CB, 1, 0, 1, 0, 1, CB, 1, 0, 1);

    ogEnvLog(1, "  NOTEQUAL\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, CB, 1, 0, 1, 0, 1, CB, 1, 0, 1);

    ogEnvLog(1, "  LEQUAL\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LEQUAL, 1, 0xFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    dotest(0, 1, CB, 1, 0, 1, 0, 1, CB, 1, 0, 1);

    ogEnvLog(1, "differing stencil ops\n");
    ogEnvLog(1, "  zpass\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    dotest(CA, 0x1, CA, 0x1, CB, 0x1, CA, 0x1, CB, 0, CB, 0x1);
    ogEnvLog(1, "  pass\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_INVERT, GL_KEEP);
    dotest(CA, 0, CA, 0x1, CB, 0, CA, 0, CB, 0, CB, 0);
    ogEnvLog(1, "  fail\n");
    doclear(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_INVERT, GL_KEEP, GL_INCR);
    dotest(CA, 1, CA, 0x0, CB, 1, CA, 1, CA, 0x0, CB, 1);

    ogEnvLog(1, "pass done.\n");
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NEVER, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


}

/*************************************************************
*  dotest()  -
*************************************************************/
static void
dotest(int lc1, int ls1, int lc2, int ls2, int lc3, int ls3, int rc1, int rs1, int rc2, int rs2, int rc3, int rs3) {
    static GLuint cbuf[10];

    /* draw the boxes */
    glColor4ub(((CA>>24)&0xff),(CA>>16)&0xff,(CA>>8)&0xff,(CA>>0)&0xff);	/* left: A over B */
    /* OGLXXX glDepthRange params must be scaled to [0, 1] */
    glDepthRange(0x20/10000., 0x20/10000.);
    glRecti(20, 20, 50, 50);
    glColor4ub(((CB>>24)&0xff),(CB>>16)&0xff,(CB>>8)&0xff,(CB>>0)&0xff);	/* left: A over B */
    /* OGLXXX glDepthRange params must be scaled to [0, 1] */
    glDepthRange(0x30/10000., 0x30/10000.);
    glRecti(30, 30, 60, 60);

    glTranslatef(80.0, 0.0, 0.0);
    glColor4ub(((CA>>24)&0xff),(CA>>16)&0xff,(CA>>8)&0xff,(CA>>0)&0xff);	/* left: A over B */
    /* OGLXXX glDepthRange params must be scaled to [0, 1] */
    glDepthRange(0x30/10000., 0x30/10000.);
    glRecti(20, 20, 50, 50);
    glColor4ub(((CB>>24)&0xff),(CB>>16)&0xff,(CB>>8)&0xff,(CB>>0)&0xff);	/* left: A over B */
    /* OGLXXX glDepthRange params must be scaled to [0, 1] */
    glDepthRange(0x20/10000., 0x20/10000.);
    glRecti(30, 30, 60, 60);
    glTranslatef(-80.0, 0.0, 0.0);

    ogLibReadPixels (25, 25, 25, 25, &cbuf[0]);   /* left: in left box */
    ogLibReadPixels (40, 35, 40, 35, &cbuf[1]);   /* in intersection */
    ogLibReadPixels (55, 55, 55, 55, &cbuf[2]);   /* in rt box */
    ogLibReadPixels (105, 25, 105, 25, &cbuf[3]); /* right: in left box etc */
    ogLibReadPixels (120, 35, 120, 35, &cbuf[4]);
    ogLibReadPixels (135, 55, 135, 55, &cbuf[5]);

    ogEnvLog(2, "ztest:\n");
    ogEnvLog(3, "left, left box\n"); teststencil(25, 25, ls1, mask);
    ogEnvLog(3, "left, intersect\n"); teststencil(40, 35, ls2, mask);
    ogEnvLog(3, "left, right box\n"); teststencil(55, 55, ls3, mask);
    ogEnvLog(3, "right, left box\n"); teststencil(105, 25, rs1, mask);
    ogEnvLog(3, "right, intersect\n"); teststencil(120, 35, rs2, mask);
    ogEnvLog(3, "right, right box\n"); teststencil(135, 55, rs3, mask);

    ogEnvLog(2, "ctest:\n");
    ogEnvLog(3, "left, left box\n"); check(cbuf[0], lc1, 0xffffffff);
    ogEnvLog(3, "left, intersect\n"); check(cbuf[1], lc2, 0xffffffff);
    ogEnvLog(3, "left, right box\n"); check(cbuf[2], lc3, 0xffffffff);
    ogEnvLog(3, "right, left box\n"); check(cbuf[3], rc1, 0xffffffff);
    ogEnvLog(3, "right, intersect\n"); check(cbuf[4], rc2, 0xffffffff);
    ogEnvLog(3, "right, right box\n"); check(cbuf[5], rc3, 0xffffffff);
}

/*************************************************************
* doclear() -
*************************************************************/
static void
doclear(int sten) {
    glDisable(GL_STENCIL_TEST);
    /* OGLXXX change glClearDepth parameter to be in [0, 1] */
    glClearDepth(0x100/10000.);
    glClearColor(0.,0.,0.,0.);
    glStencilMask(mask);
    glClearStencil(sten);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

/*************************************************************
* teststencil  - direct check of stencil plane values (instead 
*   of old brain damaged indirect test)
*************************************************************/
static void
teststencil(int x, int y, int expd, int mask) 
{
    GLuint stencilVal;

    glReadPixels(x,y,1,1,GL_STENCIL_INDEX,GL_UNSIGNED_INT,&stencilVal);

    if (expd != NC && stencilVal != (expd & mask)) {
	ogEnvLog(OG_LFAIL, "expected stencil value 0x%x, got 0x%x\n", expd & mask, stencilVal);
    }
}

/*************************************************************
* check() -
*************************************************************/
static void
check(int got, int expd, int mask) {
    if (expd != NC) {
	got |= 0xFF;
	expd |= 0xFF;
	if (ogLibColCheck(got&mask, expd&mask, 0)) {
	    ogEnvLog(OG_LFAIL, "expected 0x%x  got 0x%x\n", expd&mask, got&mask);
	}
    }
}

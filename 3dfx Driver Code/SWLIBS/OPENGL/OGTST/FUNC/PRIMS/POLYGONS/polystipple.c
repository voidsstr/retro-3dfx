/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1996, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************/


/* polystipple.c - $Revision: 2$ */

/* 
 * Tests downloading, reading back, and application of the polygon stipple
 * pattern.
 *
 * Assumes:
 * 	Default state is checked in the default state checker, not here.
 *	Errors are checked in the error test, not here.
 *	Drawing of all primitives is working correctly when POLYGON_STIPPLE
 *		is not enabled.
 *
 * TBI:
 *	Multisampling
 *	Polymode
 *	Pixel store stuff
 *	Drawing of pixel rectangles and bitmaps with stippling enabled
 *	Stippling of clipped polygons
 *	Drawing of primitives should be more random
 *	Texturing? (might not be necessary...)
 *	Other rasterization modes (fog, blending, etc)?
 */

#include <alloca.h>
#include <strings.h>

#include "ogtst.h"
#include "imbuildutil.h"

#define DO_PUSHATTRIB		1
#define DO_GETSTIPPLE		1
#define DO_PIXELSTORE		1

static int hwtype;

static void
create_polystipple(GLubyte *pat)
{
    int i;
    GLboolean swapBytes;

    /* create a random 32*32 pattern */
    for (i = 0; i < 32*4; i++) {
	pat[i] = ogLibIntRand(0, 255);
    }

    swapBytes = (ogLibIntRand(0, 8) == 0);
    if (swapBytes) {
	/* swapBytes shouldn't matter for single-bit data */
	ogEnvLog(2, "glPixelStorei(GL_UNPACK_SWAP_BYTES, 1);\n");
	glPixelStorei(GL_UNPACK_SWAP_BYTES, 1);
    }
    ogEnvLog(2, "glPolygonStipple(...);\n");
    glPolygonStipple(pat);
    if (swapBytes) {
	ogEnvLog(2, "glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);\n");
	glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
    }
}

static void
create_inverse(const GLubyte *pat, GLubyte *inv)
{
    int i;

    for (i = 0; i < 32*4; i++) {
	inv[i] = ~pat[i];
    }
}

static void
check_polystipple(const GLubyte *pat)
{
#if DO_GETSTIPPLE
    GLubyte gotpat[32*4];
    int i;

    ogEnvLog(2, "glGetPolygonStipple(...);\n");
    glGetPolygonStipple(gotpat);

    for (i = 0; i < 32*4; i++) {
	if (gotpat[i] != pat[i]) {
	    ogEnvLog(OG_LFAIL, 
		     "Error at line %d, byte %d (exp 0x%x, got 0x%x)\n", 
		     i/4, i%4, pat[i], gotpat[i]);
	    return;
	}
    }
#endif
}

static void
draw_nonstipple_prims(GLint *w, GLint *h, GLboolean rgbMode, GLint seed)
{
    GLint bitmap[32];
    GLint x0, x1, y0, y1;
    int i, j;

    ogLibSetSeed(seed);

    *w = 0;
    *h = 32;

    ogEnvLog(3, "glPushMatrix()\n");
    glPushMatrix();

    /* fudge off the center of a pixel */
    ogEnvLog(3, "glTranslatef(.1, .1, 0);\n");
    glTranslatef(.1, .1, 0);

    /* lines... */
    ogEnvLog(2, "Drawing lines...\n");
    if (rgbMode) {
	ogEnvLog(3, "glColor3f(1, 0, 0);\n");
	glColor3f(1, 0, 0);
    } else {
	ogEnvLog(3, "glIndexi(1);\n");
	glIndexi(1);
    }
    for (i = 0; i < 32; i += ogLibIntRand(0, 8)) {
	ogEnvLog(2, "glBegin(GL_LINES);\n");
	glBegin(GL_LINES);
	y0 = ogLibIntRand(0, 32);
	y1 = ogLibIntRand(0, 32);
	ogEnvLog(3, "glVertex2f(%f, %f)\n", i, y0);
	glVertex2f(i, y0);
	ogEnvLog(3, "glVertex2f(%f, %f)\n", i, y1);
	glVertex2f(i, y1);
	x0 = ogLibIntRand(0, 32);
	x1 = ogLibIntRand(0, 32);
	ogEnvLog(3, "glVertex2f(%f, %f)\n", x0, i);
	glVertex2f(x0, i);
	ogEnvLog(3, "glVertex2f(%f, %f)\n",  x1, i);
	glVertex2f(x1, i);
	/* diagonal lines except with slope 1 do not rasterize consistantly
	 * on several platforms, so draw only lines with that slope. */
	ogEnvLog(3, "glVertex2f(%f, %f)\n", i, i);
	glVertex2f(i, i);
	j = ogLibIntRand(0, 32-i);
	ogEnvLog(3, "glVertex2f(%f, %f)\n", i+j, i+j);	
	glVertex2f(i+j, i+j);
	ogEnvLog(2, "glEnd()\n");
	glEnd();
    }
    *w += 32;
    ogEnvLog(3, "glTranslatef(32, 0, 0);\n");
    glTranslatef(32, 0, 0);
    
    /* points... */
    ogEnvLog(2, "Drawing points...\n");
    if (rgbMode) {
	ogEnvLog(3, "glColor3f(0, 1, 0);\n");
	glColor3f(0, 1, 0);
    } else {
	ogEnvLog(3, "glIndexi(2);\n");
	glIndexi(2);
    }
    ogEnvLog(2, "glBegin(GL_POINTS);\n");
    glBegin(GL_POINTS);
    for (i = 0; i < 64; i++) {
	x0 = ogLibIntRand(0, 32);
	x1 = ogLibIntRand(0, 32);
	ogEnvLog(3, "glVertex2i(%d, %d);\n");
	glVertex2i(x0, x1);
    }
    ogEnvLog(2, "glEnd();\n");
    glEnd();
    *w += 32;
    ogEnvLog(3, "glTranslatef(32, 0, 0);\n");
    glTranslatef(32, 0, 0);

    /* bitmaps... */
    ogEnvLog(2, "Drawing bitmaps...\n");
    for (i = 0; i < 32; i++) {
	bitmap[i] = ogLibBitRand(32);
    }
    ogEnvLog(3, "glRasterPos2i(0, 0);\n");
    glRasterPos2i(0, 0);
    ogEnvLog(2, "glBitmap(32, 32, 0, 0, 0, 0, (const GLubyte *)bitmap);\n");
    glBitmap(32, 32, 0, 0, 0, 0, (const GLubyte *)bitmap);
    *w += 32;
    ogEnvLog(3, "glTranslatef(32, 0, 0);\n");
    glTranslatef(32, 0, 0);

    ogEnvLog(3, "glPopMatrix();\n");
    glPopMatrix();
}

static void
draw_stipple_prims(GLint *w, GLint *h, GLboolean rgbMode, GLint seed)
{
    int w0, w1;
    GLint x0, y0;

    ogLibSetSeed(seed);

    w0 = w1 = 0;

    *h = 32;

    glPushMatrix();

    glPushMatrix();
    if (rgbMode) {
	glColor3f(1, 0, 0);
    } else {
	glIndexi(1);
    }

    /* triangles */
    ogEnvLog(2, "Drawing triangles...\n");
    ogEnvLog(2, "glBegin(GL_TRIANGLES);\n");
    glBegin(GL_TRIANGLES);
    glVertex2i(0, 0);
    x0 = ogLibIntRand(1, 64);
    ogEnvLog(3, "glVertex2i(%d, 0);\n", x0);
    glVertex2i(x0, 0);
    x0 = ogLibIntRand(1, 64);
    y0 = ogLibIntRand(1, 32);
    ogEnvLog(3, "glVertex2i(%d, %d);\n", x0, y0);
    glVertex2i(x0, y0);
    ogEnvLog(2, "glEnd();\n");
    glEnd();
    w0 += 64;
    ogEnvLog(3, "glTranslatef(64, 0, 0);\n");
    glTranslatef(64, 0, 0);

    /* tstrips */
    ogEnvLog(2, "Drawing tstrips...\n");
    ogEnvLog(2, "glBegin(GL_TRIANGLE_STRIP);\n");
    glBegin(GL_TRIANGLE_STRIP);
    ogEnvLog(3, "glVertex2i(0, 0);\n");
    glVertex2i(0, 0);
    x0 = ogLibIntRand(1, 48);
    ogEnvLog(3, "glVertex2i(%d, 0);\n", x0);
    glVertex2i(x0, 0);
    y0 = ogLibIntRand(1, 32);
    ogEnvLog(3, "glVertex2i(0, %d);\n", y0);
    glVertex2i(0, y0);
    x0 = ogLibIntRand(1, 48);
    y0 = ogLibIntRand(1, 32);
    ogEnvLog(3, "glVertex2i(%d, %d);\n", x0, y0);
    glVertex2i(x0, y0);
    ogEnvLog(2, "glEnd();\n");
    glEnd();
    w0 += 48;
    ogEnvLog(3, "glTranslatef(48, 0, 0);\n");
    glTranslatef(48, 0, 0);

    /* tfans */
    ogEnvLog(2, "Drawing triangle fans...\n");
    ogEnvLog(2, "glBegin(GL_TRIANGLE_FAN);\n");
    glBegin(GL_TRIANGLE_FAN);
    ogEnvLog(3, "glVertex2i(16, 16);\n");
    glVertex2i(16, 16);
    glVertex2i(33, 0);
    glVertex2i(33, 32);
    glEnd();
    w0 += 32;
    glTranslatef(33, 0, 0);

    glPopMatrix();
    *h += 32;
    glTranslatef(0, 32, 0);
    glPushMatrix();
    if (rgbMode) {
	glColor3f(0, 1, 0);
    } else {
	glIndexi(2);
    }

    /* quads */
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(39, 0);
    glVertex2i(39, 11);
    glVertex2i(0, 11);
    glEnd();
    w1 += 39;
    glTranslatef(39, 0, 0);

    /* qstrips */
    glBegin(GL_QUAD_STRIP);
    glVertex2i(0, 0);
    glVertex2i(17, 0);
    glVertex2i(0, 3);
    glVertex2i(17, 3);
    glEnd();
    w1 += 17;
    glTranslatef(17, 0, 0);

    /* polygons */
    glBegin(GL_POLYGON);
    glVertex2i(0, 0);
    glVertex2i(65, 0);
    glVertex2i(65, 15);
    glVertex2i(16, 32);
    glVertex2i(0, 15);
    glEnd();
    w1 += 65;
    glTranslatef(65, 0, 0);

    /* rects */
    glRecti(0, 0, 40, 31);
    w1 += 40;

    glPopMatrix();

    glPopMatrix();

    *w = (w0 > w1) ? w0 : w1;
}

static int
rect_check(GLint x0, GLint y0, GLint x1, GLint y1, GLint w, GLint h,
	   GLboolean rgbMode)
{
    GLubyte *ibuf0, *ibuf1;
    GLuint ibufSize = 0;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    if (rgbMode) {
	if (ibufSize < w*h*3*sizeof(GLubyte)) {
	    ibuf0 = alloca(w*h*3*sizeof(GLubyte));
	    ibuf1 = alloca(w*h*3*sizeof(GLubyte));
	    ibufSize = w*h*3;
	}
	glReadPixels(x0, y0, w, h, GL_RGB, GL_UNSIGNED_BYTE, ibuf0);
	glReadPixels(x1, y1, w, h, GL_RGB, GL_UNSIGNED_BYTE, ibuf1);
    } else {
	if (ibufSize < w*h*sizeof(GLint)) {
	    ibuf0 = alloca(w*h*sizeof(GLint));
	    ibuf1 = alloca(w*h*sizeof(GLint));
	    ibufSize = w*h;
	}
	glReadPixels(x0, y0, w, h, GL_COLOR_INDEX, GL_INT, ibuf0);
	glReadPixels(x1, y1, w, h, GL_COLOR_INDEX, GL_INT, ibuf1);
    }    

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);


    if (bcmp(ibuf0, ibuf1, ibufSize)) {
	return GL_FALSE;
    } else {
	return GL_TRUE;
    }
}

#define ENABLE							\
do {								\
    if (!stippleEnabled || !ogLibIntRand(0, 4)) {		\
        ogEnvLog(2, "glEnable(GL_POLYGON_STIPPLE);\n");		\
	glEnable(GL_POLYGON_STIPPLE);				\
	stippleEnabled = 1;					\
    }								\
} while (0)

#define DISABLE							\
do {								\
    if (stippleEnabled || !ogLibIntRand(0, 3)) {		\
        ogEnvLog(2, "glDisable(GL_POLYGON_STIPPLE);\n");	\
	glDisable(GL_POLYGON_STIPPLE);				\
	stippleEnabled = 0;					\
    }								\
} while (0)

#define MAYBE_ENABLE(odds)					\
do {								\
    if (!stippleEnabled && !ogLibIntRand(0, (odds)-1)) {	\
	ENABLE;							\
    } 								\
} while (0)

#define MAYBE_DISABLE(odds)					\
do {								\
    if (stippleEnabled && !ogLibIntRand(0, (odds)-1)) {		\
	DISABLE;						\
    } 								\
} while (0)

TESTMOD(polystipple)
{
    GLubyte real[32*4], realInv[32*4], fake[32*4];
    GLint xwin = ogEnvQuery(OG_XWSIZE), ywin = ogEnvQuery(OG_YWSIZE);
    GLint ypos;
    GLint x, y, w, h;
    GLboolean rgbMode;
    GLboolean tex3D, tex4D;
    GLboolean setStore, stippleEnabled = 0;
    GLint seed;

    glGetBooleanv(GL_RGBA_MODE, &rgbMode);

    hwtype = ogEnvQuery(OG_HW);

    tex3D = 0;
    tex4D = 0;

    while (pass--) {
	ypos = 0;

	ogLibClear(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, xwin, 0, ywin, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* download a pattern... */
	create_polystipple(real);
	
	/* invert it... */
	create_inverse(real, realInv);

	/* maybe push and pop it... */
#if DO_PUSHATTRIB
	if (ogLibBitRand(1)) {
	    ogEnvLog(1, "Testing PushAttrib / PopAttrib\n");
	    if (ogLibIntRand(0, 8)) {
		/* pixel pack should have no influence on push/pop attrib,
		 * but this is a common implementation bug */
		ogEnvLog(1, "Setting fake pixel pack params\n");
		glPixelStorei(GL_PACK_SWAP_BYTES, ogLibIntRand(0, 32));
		glPixelStorei(GL_PACK_LSB_FIRST, ogLibIntRand(0, 32));
		glPixelStorei(GL_PACK_ROW_LENGTH, ogLibIntRand(0, 32));
		glPixelStorei(GL_PACK_SKIP_ROWS, ogLibIntRand(0, 32));
		glPixelStorei(GL_PACK_ALIGNMENT, 1 << ogLibIntRand(0, 3));
		setStore = 1;
	    } else {
		setStore = 0;
	    }
	    if (ogLibBitRand(1)) {
		ogEnvLog(2, "glPushAttrib(GL_POLYGON_STIPPLE_BIT);\n");
		glPushAttrib(GL_POLYGON_STIPPLE_BIT);
	    } else {
		ogEnvLog(2, "glPushAttrib(GL_ALL_ATTRIB_BITS);\n");
		glPushAttrib(GL_ALL_ATTRIB_BITS);		
	    }
	    if (setStore) {
		ogEnvLog(1, "Resetting pixel pack params\n");
		glPixelStorei(GL_PACK_SWAP_BYTES, 0);
		glPixelStorei(GL_PACK_LSB_FIRST, 0);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_SKIP_ROWS, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	    }
	    create_polystipple(fake);
	    check_polystipple(fake);
	    if (ogLibIntRand(0, 8)) {
		ogEnvLog(1, "Setting fake pixel unpack params\n");
		glPixelStorei(GL_UNPACK_SWAP_BYTES, ogLibIntRand(0, 32));
		glPixelStorei(GL_UNPACK_LSB_FIRST, ogLibIntRand(0, 32));
		glPixelStorei(GL_UNPACK_ROW_LENGTH, ogLibIntRand(0, 32));
		glPixelStorei(GL_UNPACK_SKIP_ROWS, ogLibIntRand(0, 32));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1 << ogLibIntRand(0, 3));
		setStore = 1;
	    } else {
		setStore = 0;
	    }
	    ogEnvLog(2, "glPopAttrib();\n");
	    glPopAttrib();
	    if (setStore) {
		ogEnvLog(1, "Resetting pixel unpack params\n");
		glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
		glPixelStorei(GL_UNPACK_LSB_FIRST, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	    }
	}
#endif

	/* maybe read it back and check it */
	if (ogLibBitRand(1)) {
	    check_polystipple(real);
	}

	if (ogLibIntRand(0, 9)) {
	    ENABLE;
	    if (!ogLibIntRand(0, 4) && !glIsEnabled(GL_POLYGON_STIPPLE)) {
		ogEnvLog(OG_LFAIL, "IsEnabled returned false\n");
	    }
	    seed = ogLibGetSeed();
	    draw_stipple_prims(&w, &h, rgbMode, seed);
	    DISABLE;
	    if (!ogLibIntRand(0, 4) && glIsEnabled(GL_POLYGON_STIPPLE)) {
		ogEnvLog(OG_LFAIL, "IsEnabled returned true\n");
	    }
	    glPushMatrix();
	    glTranslatef(0, h + h/2, 0);
	    draw_stipple_prims(&w, &h, rgbMode, seed);
	    /* now draw bitmaps of the inverse stipple pattern in the 
	     * background color. */
	    x = y = 0;
	    if (rgbMode) {
		glColor3f(0, 0, 0);
	    } else {
		glIndexi(0);
	    }
	    for (y = 0; y < h; y += 32) {
		glRasterPos2i(0, 0);
		for (x = 0; x < w; x += 32) {
		    glBitmap(32, 32, 0, 0, 32, 0, realInv);
		}
		glTranslatef(0, 32, 0);
	    }
	    glPopMatrix();	    
	    if (!rect_check(0, ypos, 0, ypos + h + h/2, w, h, rgbMode)) {
		ogEnvLog(OG_LFAIL, "Stippled polygons wrong!\n");	    
	    }
	} else {
	    /* every once in a while do it without the stippling, just to 
	     * be sure it really did get disabled */
	    seed = ogLibGetSeed();
	    draw_stipple_prims(&w, &h, rgbMode, seed);
	    glPushMatrix();
	    glTranslatef(0, h + h/2, 0);
	    draw_stipple_prims(&w, &h, rgbMode, seed);
	    glPopMatrix();
	    if (!rect_check(0, ypos, 0, ypos + h + h/2, w, h, rgbMode)) {
		ogEnvLog(OG_LFAIL, "Non-stippled polygons wrong!\n");
	    }
	}

	ypos += 3*h;
	glTranslatef(0, 3*h, 0);

	/* is it not applied to some non-polygonal primitives? */
	MAYBE_ENABLE(4);
	seed = ogLibGetSeed();
	draw_nonstipple_prims(&w, &h, rgbMode, seed);
	MAYBE_DISABLE(4);
	glPushMatrix();
	glTranslatef(0, h + h/2, 0);
	draw_nonstipple_prims(&w, &h, rgbMode, seed);
	glPopMatrix();
	/* check that the chksums for the stippled & non-stippled rectangles
	 * are the same... */
	if (!rect_check(0, ypos, 0, ypos + h + h/2, w, h, rgbMode)) {
	    ogEnvLog(OG_LFAIL, "Non-stippled prims wrong!\n");	    
	}

	glTranslatef(0, 4*h, 0);
	
    }

}

CLEANUP(polystipple)
{
    GLint pat[32];
    int i;

    /* basic stuff */
    ogLibSetDefaultColors();
    ogLibSetDefaultMatrices();
    ogLibSetDefaultRasterPos();

    /* reset pixel pack & unpack modes */
#if DO_PIXELSTORE
    ogLibSetDefaultPixelStore();
#endif

    /* reset stipple pattern */
    for (i = 0; i < 32; i++) {
	pat[i] = -1;
    }
    glPolygonStipple((const GLubyte *)pat);

    /* disable stippling */
    glDisable(GL_POLYGON_STIPPLE);
}

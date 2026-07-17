/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1993, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************/

/* $RCSfile: pixel.c,v $ $Revision: 2$ */


/* XXX These changes were made temporary to work around existing bugs for
 *     RE:
 *      1) indexShift = 0, indexOffset = 0
 *      2) swapBytes = 0; lsbFirst = 0;
 *      3)  if (type == GL_BITMAP && ((misalignment != 0) || (alignment != 4)))
 *         	return GL_TRUE;
 *      4) multisampling
 *       - No stencil testing
 *
 */


#include "ogtst.h"		/* include test environment             */

#define DONT_STOP_ON_ERROR	0
#define CLEAR_SCREEN		0

#define MIN_WIDTH_HEIGHT	16
#define MAX_WIDTH_HEIGHT	32
#define MAX_SKIP_BEFORE        	16        /* Maximum skip for rows or pixels */
#define MAX_SKIP_AFTER		8		
/*
 * Memory allocated for "inImage".
 *  maxMisAlignment + maxElementSize*maxElementsPerGroup*
 *                 MaxRowSize*MaxNumberOfRows
 * where MaxRowSize = initial pixel skip + max image width + final skip skip +
 *   max extra alignment
 */
#define MAX_IMAGE_SIZE \
  (4 + 4*4*((MAX_SKIP_BEFORE + MAX_WIDTH_HEIGHT + MAX_SKIP_AFTER + 2) * \
            (MAX_SKIP_BEFORE + MAX_WIDTH_HEIGHT)))

#define MAXPIXENTRIES	32
#define MAXPIXBITS	6

#define MAX_ZOOM 3              /* Maximum allowed zoom */
#define MAX_COLOR_ERR	3       /* Maximum error in color component */

  /* snap x onto a (2n+1)/16 grid */
#define SNAP(x) ((int)(x * 8.0) / 8.0) + (1/16.0)
   
#define SNAP_CRM(x) (((int) (x * 16.0)) / 16.0) + (1/32.0)

#define CLAMP01(C) if (1) { if (C < 0) C = 0; else if (C > 1) C = 1; } else

typedef struct map {
    GLint size;

    GLfloat entries[MAXPIXENTRIES];
} PixelMap;

static GLsizei width, height;
static GLenum type, format;
static GLubyte *inImage;
static GLuint *outImage;
static GLint skipPixels, skipRows, lineLength;
static GLint alignment, misalignment;
static GLboolean swapBytes = GL_FALSE, lsbFirst = GL_FALSE;
static GLboolean mapColor, mapStencil;
static GLfloat rScale, gScale, bScale, aScale, dScale = 1;
static GLfloat rBias, gBias, bBias, aBias, dBias = 0;
static GLfloat xZoom, yZoom;
static GLint indexShift = 0, indexOffset = 0;
static GLfloat rastX, rastY;
static GLboolean rgbMode;
static GLboolean hasDepth, hasStencil;
static GLint indexBufMask, stencilBufMask;
static int xsize, ysize;
static int hwtype;

static PixelMap mapII, mapSS, mapIR, mapIG, mapIB, mapIA;
static PixelMap mapAA, mapRR, mapGG, mapBB;

const GLenum formatArray[] = {
    GL_STENCIL_INDEX, GL_COLOR_INDEX, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA,
    GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_DEPTH_COMPONENT,
#ifdef GL_EXT_abgr    
    GL_ABGR_EXT
#endif
};

const GLenum typeArray[] = {
    GL_BITMAP, GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, 
    GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT,
};

static void performPixelTest(void);
static GLboolean invalidTest(void);
static void buildDefaultMaps(void);

static float roundErr;
static GLboolean errorFail;  /* set to GL_TRUE if something isn't sure */
static GLboolean multiSampled;

/* Function Prototypes */
static int                elementsPerGroup(void);
static int                elementSize(void);
static GLboolean          invalidTest(void);
static void               uploadModes(int log);
static void               uploadDefaults(void);
static void               buildCCMap(PixelMap *map);
static void               buildIIMap(PixelMap *map);
static void               buildICMap(PixelMap *map);
static void               setDefaultModes(void);
static void               buildDefaultMaps(void);
static GLubyte *          rowOffset(int y);
static void               swap4(void *where);
static void               swap2(void *where);
static void               generateCtype(void *where);
static float              getCtype(void *where);
static void               checkResults(void);
static void               performPixelTest(void);

TESTMOD(pixel)
{
    int i;
    
    hwtype = ogEnvQuery(OG_HW);
    multiSampled = ogEnvIsMultiSampled();

    inImage = ogLibMalloc(MAX_IMAGE_SIZE);
    outImage = (GLuint *) ogLibMalloc(4*4*(MAX_WIDTH_HEIGHT * MAX_ZOOM)*
                                      (MAX_WIDTH_HEIGHT * MAX_ZOOM));
    if (inImage == NULL || outImage == NULL) {
        ogEnvLog(OG_LINTERNALERROR, "Can not malloc memory for pixel test\n");
    }
    xsize = ogEnvQuery(OG_XWSIZE);
    ysize = ogEnvQuery(OG_YWSIZE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, xsize, 0, ysize, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ogLibClear(0);

    buildDefaultMaps();

    rgbMode = ogEnvCurVisualInfo(GLX_RGBA);
    stencilBufMask = (1 << ogEnvCurVisualInfo(GLX_STENCIL_SIZE)) - 1;

    /*
     * XXXZiv - multisampling should be on half the time, but test
     *  needs to be modified to account for the smearing.
     *  I had the code that accounts for smearing in previous versions
     *  of this file.  However, that code assumed other bugs, so it
     *  needs some work.
     */
    hasDepth = ogEnvCurVisualInfo(GLX_DEPTH_SIZE) != 0;
    hasStencil = stencilBufMask != 0;

    /*
     * XXX special for VENICE: when rgbMode and input format is index. The input
     * representation is 12 bits. This matters when the input index is negative
     * and right shift is non-zero i.e. zero fill. 
     */
    indexBufMask =
        rgbMode ? 0xfff : ((1 << ogEnvCurVisualInfo(GLX_BUFFER_SIZE)) - 1);

    roundErr = 0.01;	/* Rounding should be fairly accurate. */

    if (hasDepth) {
	glDepthFunc(GL_ALWAYS);
    }
    if (hasStencil) {
	glStencilFunc(GL_ALWAYS, 0, 0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }

    while (pass--) {
restart:
	/* Pick a bunch of random values for DrawPixels */

	/* Size [16-32]x[16-32] */
	width = ogLibIntRand(MIN_WIDTH_HEIGHT, MAX_WIDTH_HEIGHT);
	height = ogLibIntRand(MIN_WIDTH_HEIGHT, MAX_WIDTH_HEIGHT);

	rastX = ogLibFloatRand(0, (float) xsize-width/2);
	rastY = ogLibFloatRand(0, (float) ysize-height/2);

        /*
         * Choose zoom factors.  We give preference to the 1,1 case setting
         *  about 0.25 (+ 1/18) of the tests to 1,1.
         */
        i = ogLibIntRand(0, 7);
        if (i < 4) {
            xZoom = (i & 0x1 ? -1 : 1) * ogLibIntRand(1,MAX_ZOOM);
            yZoom = (i & 0x2 ? -1 : 1) * ogLibIntRand(1,MAX_ZOOM);
	} else
	    xZoom = yZoom = 1;

	/* Pick completely random format and type */
	type = typeArray[
            ogLibIntRand(0,sizeof(typeArray)/sizeof(GLenum) - 1)];
	format = formatArray[
            ogLibIntRand(0,sizeof(formatArray)/sizeof(GLenum) - 1)];
	/* 50% chance bias, scale, shift and offset image */
	if (ogLibBitRand(3) & 1) {
	    rScale = ogLibFloatRand(-1.0,2.0);
	    rBias = ogLibFloatRand(-1.0,1.0);
	    gScale = ogLibFloatRand(-1.0,2.0);
	    gBias = ogLibFloatRand(-1.0,1.0);
	    bScale = ogLibFloatRand(-1.0,2.0);
	    bBias = ogLibFloatRand(-1.0,1.0);
	    aScale = ogLibFloatRand(-1.0,2.0);
            aBias = ogLibFloatRand(-1.0,1.0);
            dScale = ogLibFloatRand(-1.0,2.0);
            dBias = ogLibFloatRand(-1.0,1.0);
            indexShift = ogLibIntRand(0,6) - 3;
            indexOffset = ogLibIntRand(0,16) - 8;
	} else {
	    rScale = gScale = bScale = aScale =dScale = 1.0;
	    rBias = gBias = bBias = aBias = dBias= 0.0;
	    indexShift = 0;
	    indexOffset = 0;
	}
	/* XXX currently not handling this right. */

	/* 50% chance map colors */
	if (ogLibBitRand(3) & 1) {
	    mapColor = GL_TRUE;
	    mapStencil = GL_TRUE;
        } else {
            mapColor = GL_FALSE;
            mapStencil = GL_FALSE;
        }

	alignment = 1<<ogLibIntRand(0,2);	/* 1,2,4,XXX 8 omitted */

	i = 4 - elementSize();  /* BITMAP return 0 which works ok */
        if (i && ogLibIntRand(0,4) == 1) {
	    /* Misalign the data (put data on non word boundaries, when
             *  legal
             */
	    misalignment = i == 2 ? 2 : ogLibIntRand(1, 3);
        } else {
	    misalignment = 0;
	}

	if (ogLibBitRand(3) & 1) {
	    skipPixels = ogLibIntRand(0,MAX_SKIP_BEFORE);
	    skipRows = ogLibIntRand(0,MAX_SKIP_BEFORE);
	    lineLength = skipPixels + width + ogLibIntRand(0,MAX_SKIP_AFTER);
	} else {
	    skipPixels = skipRows = lineLength = 0;
	}
	swapBytes = ogLibIntRand(0,1);
	lsbFirst = ogLibIntRand(0,1);
	if (invalidTest()) goto restart;

	/* Randomly choose if we should enable stenciling/depth testing */
	if (hasStencil) {
	    if (ogLibBitRand(3) & 1) {
		ogEnvLog(2, "glEnable(GL_STENCIL);\n");
		glEnable(GL_STENCIL_TEST);
	    } else {
		ogEnvLog(2, "glDisable(GL_STENCIL);\n");
		glDisable(GL_STENCIL_TEST);
	    }
	}

	if (hasDepth) {
	    if (format == GL_DEPTH_COMPONENT || (ogLibBitRand(3) & 1)) {
		ogEnvLog(2, "glEnable(GL_DEPTH_TEST);\n");
		glEnable(GL_DEPTH_TEST);
	    } else {
		ogEnvLog(2, "glDisable(GL_DEPTH_TEST);\n");
		glDisable(GL_DEPTH_TEST);
	    }
	}

	if (format == GL_DEPTH_COMPONENT) {
	    /*
	    ** Mask color writes about half of the time when drawing to 
	    ** the depth buffer.
	    */
	    if (ogLibBitRand(3) & 1) {
		ogEnvLog(2, "glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);\n");
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		ogEnvLog(2, "glIndexMask(0);\n");
		glIndexMask(0);
	    } 
	}

	performPixelTest();

	ogEnvLog(2, "glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);\n");
	ogEnvLog(2, "glIndexMask(0xffff);\n");
        glDisable(GL_DEPTH_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glIndexMask(0xffff);
    }
    ogLibFree(inImage);
    ogLibFree(outImage);
}

static GLboolean
invalidTest(void)
{
    /* Return GL_TRUE if the test requested is illegal */
    if (format == GL_STENCIL_INDEX && !hasStencil) return GL_TRUE;
    if (format == GL_DEPTH_COMPONENT && !hasDepth) return GL_TRUE;
    if (type == GL_BITMAP && format != GL_COLOR_INDEX && 
	    format != GL_STENCIL_INDEX) return GL_TRUE;
	
    if (!rgbMode && format != GL_STENCIL_INDEX && format != GL_COLOR_INDEX &&
        format != GL_DEPTH_COMPONENT)
        return GL_TRUE;

    /* Let's invalidate a couple to make the tests more entertaining */
    if (format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX &&
        format != GL_DEPTH_COMPONENT) {
	/* RGB mode */
	if (type == GL_UNSIGNED_INT || type == GL_UNSIGNED_SHORT || 
            type == GL_UNSIGNED_BYTE) {
	    /* numbers can only be represented from 0 to 1, let's 
	    ** make sure that at least something gets drawn...
	    */
	    if ((rScale < 0 && rBias < 0) || 
                (gScale < 0 && gBias < 0) ||
                (bScale < 0 && bBias < 0))
                return GL_TRUE;
	}
    } else if (format == GL_DEPTH_COMPONENT) {
	if (type == GL_UNSIGNED_INT || type == GL_UNSIGNED_SHORT ||
            type == GL_UNSIGNED_BYTE) {
	    /* numbers can only be represented from 0 to 1, let's
	    ** make sure that at least something gets drawn...
	    */
	    if (dScale < 0 && dBias < 0)
                return GL_TRUE;
	}
    }
    /* This behavior is underspecified in the OpenGL spec.  Don't test
    ** it.
    */
    if (type == GL_BITMAP && indexShift < 0)
        return GL_TRUE;

    return GL_FALSE;
}

static void
uploadModes(int log)
{
    /* Upload all pixel state to OpenGL */
    glPixelStoref(GL_UNPACK_SWAP_BYTES, swapBytes);
    glPixelStoref(GL_UNPACK_LSB_FIRST, lsbFirst);
    glPixelStoref(GL_UNPACK_ROW_LENGTH, lineLength);
    glPixelStoref(GL_UNPACK_SKIP_ROWS, skipRows);
    glPixelStoref(GL_UNPACK_SKIP_PIXELS, skipPixels);
    glPixelStoref(GL_UNPACK_ALIGNMENT, alignment);

    if (log) {
	ogEnvLog(2, "glPixelStoref(GL_UNPACK_SWAP_BYTES, %d);\n", swapBytes);
	ogEnvLog(2, "glPixelStoref(GL_UNPACK_LSB_FIRST, %d);\n", lsbFirst);
	ogEnvLog(2, "glPixelStoref(GL_UNPACK_ROW_LENGTH, %d);\n", lineLength);
	ogEnvLog(2, "glPixelStoref(GL_UNPACK_SKIP_ROWS, %d);\n", skipRows);
	ogEnvLog(2, "glPixelStoref(GL_UNPACK_SKIP_PIXELS, %d);\n", skipPixels);
	ogEnvLog(2, "glPixelStoref(GL_UNPACK_ALIGNMENT, %d);\n", alignment);
    }

    glPixelTransferf(GL_MAP_COLOR, mapColor);
    glPixelTransferf(GL_MAP_STENCIL, mapStencil);
    glPixelTransferf(GL_INDEX_SHIFT, indexShift);
    glPixelTransferf(GL_INDEX_OFFSET, indexOffset);
    glPixelTransferf(GL_RED_SCALE, rScale);
    glPixelTransferf(GL_RED_BIAS, rBias);
    glPixelTransferf(GL_GREEN_SCALE, gScale);
    glPixelTransferf(GL_GREEN_BIAS, gBias);
    glPixelTransferf(GL_BLUE_SCALE, bScale);
    glPixelTransferf(GL_BLUE_BIAS, bBias);
    glPixelTransferf(GL_ALPHA_SCALE, aScale);
    glPixelTransferf(GL_ALPHA_BIAS, aBias);
    glPixelTransferf(GL_DEPTH_SCALE, dScale);
    glPixelTransferf(GL_DEPTH_BIAS, dBias);
    glPixelZoom(xZoom, yZoom);

    if (log) {
	ogEnvLog(2, "glPixelTransferf(GL_MAP_COLOR, %d);\n", mapColor);
	ogEnvLog(2, "glPixelTransferf(GL_MAP_STENCIL, %d);\n", mapStencil);
	ogEnvLog(2, "glPixelTransferf(GL_INDEX_SHIFT, %d);\n", indexShift);
	ogEnvLog(2, "glPixelTransferf(GL_INDEX_OFFSET, %d);\n", indexOffset);
	ogEnvLog(2, "glPixelTransferf(GL_RED_SCALE, %g);\n", rScale);
	ogEnvLog(2, "glPixelTransferf(GL_RED_BIAS, %g);\n", rBias);
	ogEnvLog(2, "glPixelTransferf(GL_GREEN_SCALE, %g);\n", gScale);
	ogEnvLog(2, "glPixelTransferf(GL_GREEN_BIAS, %g);\n", gBias);
	ogEnvLog(2, "glPixelTransferf(GL_BLUE_SCALE, %g);\n", bScale);
	ogEnvLog(2, "glPixelTransferf(GL_BLUE_BIAS, %g);\n", bBias);
	ogEnvLog(2, "glPixelTransferf(GL_ALPHA_SCALE, %g);\n", aScale);
	ogEnvLog(2, "glPixelTransferf(GL_ALPHA_BIAS, %g);\n", aBias);
	ogEnvLog(2, "glPixelTransferf(GL_DEPTH_SCALE, %g);\n", dScale);
	ogEnvLog(2, "glPixelTransferf(GL_DEPTH_BIAS, %g);\n", dBias);
	ogEnvLog(2, "glPixelZoom(%g, %g);\n", xZoom, yZoom);
    }

    if (log) ogEnvLog(2, "... uploading bucket of pixel maps\n");
    glPixelMapfv(GL_PIXEL_MAP_I_TO_I, mapII.size, mapII.entries);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, mapIR.size, mapIR.entries);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, mapIG.size, mapIG.entries);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, mapIB.size, mapIB.entries);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_A, mapIA.size, mapIA.entries);
    glPixelMapfv(GL_PIXEL_MAP_R_TO_R, mapRR.size, mapRR.entries);
    glPixelMapfv(GL_PIXEL_MAP_G_TO_G, mapGG.size, mapGG.entries);
    glPixelMapfv(GL_PIXEL_MAP_B_TO_B, mapBB.size, mapBB.entries);
    glPixelMapfv(GL_PIXEL_MAP_A_TO_A, mapAA.size, mapAA.entries);
    glPixelMapfv(GL_PIXEL_MAP_S_TO_S, mapSS.size, mapSS.entries);
}

static void
uploadDefaults(void)
{
    /* Upload all pixel state to OpenGL */
    glPixelStoref(GL_UNPACK_SWAP_BYTES, 0);
    glPixelStoref(GL_UNPACK_LSB_FIRST, 0);
    glPixelStoref(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStoref(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStoref(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStoref(GL_UNPACK_ALIGNMENT, 4);

    glPixelTransferf(GL_MAP_COLOR, 0);
    glPixelTransferf(GL_MAP_STENCIL, 0);
    glPixelTransferf(GL_INDEX_SHIFT, 0);
    glPixelTransferf(GL_INDEX_OFFSET, 0);
    glPixelTransferf(GL_RED_SCALE, 1);
    glPixelTransferf(GL_RED_BIAS, 0);
    glPixelTransferf(GL_GREEN_SCALE, 1);
    glPixelTransferf(GL_GREEN_BIAS, 0);
    glPixelTransferf(GL_BLUE_SCALE, 1);
    glPixelTransferf(GL_BLUE_BIAS, 0);
    glPixelTransferf(GL_ALPHA_SCALE, 1);
    glPixelTransferf(GL_ALPHA_BIAS, 0);
    glPixelTransferf(GL_DEPTH_SCALE, 1);
    glPixelTransferf(GL_DEPTH_BIAS, 0);
    glPixelZoom(1, 1);
}

static void
buildCCMap(PixelMap *map)
{
    int size;
    int i;

    size = ogLibIntRand(0,MAXPIXENTRIES-1) + 1;
    map->size = size;
    for (i=0; i<size; i++) {
	map->entries[i] = ogLibFloatRand(-0.5,1.5);
    }
}

static void
buildIIMap(PixelMap *map)
{
    int size;
    int i;

    size = 1 << ogLibIntRand(0,MAXPIXBITS-1);
    map->size = size;
    for (i=0; i<size; i++) {
	map->entries[i] = ogLibIntRand(0,270) - 7;
    }
}

static void
buildICMap(PixelMap *map)
{
    int size;
    int i;

    size = 1 << ogLibIntRand(0,MAXPIXBITS-1);
    map->size = size;
    for (i=0; i<size; i++) {
	map->entries[i] = ogLibFloatRand(-0.5,1.5);
    }
}

static void
setDefaultModes(void)
{
    skipPixels = 0;
    swapBytes = 0;
    lsbFirst = 0;
    lineLength = 0;
    skipRows = 0;
    alignment = 4;
    mapColor = 0;
    mapStencil = 0;
    indexShift = 0;
    indexOffset = 0;
    rScale = gScale = bScale = aScale = dScale = 1.0;
    rBias = gBias = bBias = aBias = dBias = 0.0;
}

static void
buildDefaultMaps(void)
{
    mapRR.size = 1;
    mapGG.size = 1;
    mapBB.size = 1;
    mapAA.size = 1;
    mapSS.size = 1;
    mapIR.size = 1;
    mapIG.size = 1;
    mapIB.size = 1;
    mapIA.size = 1;
    mapII.size = 1;

    mapRR.entries[0] = 0;
    mapGG.entries[0] = 0;
    mapBB.entries[0] = 0;
    mapAA.entries[0] = 0;
    mapSS.entries[0] = 0;
    mapIR.entries[0] = 0;
    mapIG.entries[0] = 0;
    mapIB.entries[0] = 0;
    mapIA.entries[0] = 0;
    mapII.entries[0] = 0;
}

static int
elementsPerGroup(void)
{
    switch(format) {
      case GL_COLOR_INDEX:
      case GL_STENCIL_INDEX:
      case GL_DEPTH_COMPONENT:
      case GL_RED:
      case GL_BLUE:
      case GL_GREEN:
      case GL_ALPHA:
      case GL_LUMINANCE:
	return 1;
      case GL_LUMINANCE_ALPHA:
	return 2;
      case GL_RGB:
	return 3;
      case GL_RGBA:
#ifdef GL_EXT_abgr   
      case GL_ABGR_EXT:
#endif
	return 4;
    }
    return 0;
}

static int
elementSize(void)
{
    switch(type) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
	return 1;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
	return 2;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	return 4;
    }
    return 0;
}

static GLubyte *
rowOffset(int y)
{
    int epl;
    int rowSize;

    epl = elementsPerGroup() * (lineLength == 0 ? width : lineLength);

    rowSize = type == GL_BITMAP ? (epl + 7) / 8 : epl * elementSize();
    rowSize = (rowSize + alignment - 1) & ~(alignment-1);
    return (rowSize * (y + skipRows)) + inImage;
}

static void
swap4(void *where)
{
    char t;
    char *d;

    d = where;
    t = d[0];
    d[0] = d[3];
    d[3] = t;
    t = d[1];
    d[1] = d[2];
    d[2] = t;
}

static void
swap2(void *where)
{
    char t;
    char *d;

    d = where;
    t = d[0];
    d[0] = d[1];
    d[1] = t;
}

static void
generateCtype(void *where)
{
    float value;

    switch(type) {
      case GL_FLOAT:
	value = ogLibFloatRand(-2.0,2.0);
	break;
      case GL_BYTE:
      case GL_INT:
      case GL_SHORT:
	value = ogLibFloatRand(-1.0,1.0);
	break;
      case GL_UNSIGNED_BYTE:
      case GL_UNSIGNED_SHORT:
      case GL_UNSIGNED_INT:
	value = ogLibFloatRand(0.0,1.0);
	break;
    }

    switch(type) {
      case GL_FLOAT:
	*(GLfloat *) where = value;
	if (swapBytes) swap4(where);
	break;
      case GL_INT:
	*(GLint *) where = OGTST_F_TO_I(value);
	if (swapBytes) swap4(where);
	break;
      case GL_SHORT:
	*(GLshort *) where = OGTST_F_TO_S(value);
	if (swapBytes) swap2(where);
	break;
      case GL_BYTE:
	*(GLbyte *) where = OGTST_F_TO_B(value);
	break;
      case GL_UNSIGNED_INT:
	*(GLuint *) where = OGTST_F_TO_UI(value);
	if (swapBytes) swap4(where);
	break;
      case GL_UNSIGNED_SHORT:
	*(GLushort *) where = OGTST_F_TO_US(value);
	if (swapBytes) swap2(where);
	break;
      case GL_UNSIGNED_BYTE:
	*(GLubyte *) where = OGTST_F_TO_UB(value);
	break;
    }
}

static float
getCtype(void *where)
{
    float value;

    switch(type) {
      case GL_FLOAT:
        if (swapBytes) swap4(where);
	value = *(GLfloat *) where;
        if (swapBytes) swap4(where);
	break;
      case GL_INT:
        if (swapBytes) swap4(where);
	value = OGTST_I_TO_F(*(GLint *) where);
        if (swapBytes) swap4(where);
	break;
      case GL_SHORT:
        if (swapBytes) swap2(where);
	value = OGTST_S_TO_F(*(GLshort *) where);
        if (swapBytes) swap2(where);
	break;
      case GL_BYTE:
	value = OGTST_B_TO_F(*(GLbyte *) where);
	break;
      case GL_UNSIGNED_INT:
        if (swapBytes) swap4(where);
	value = OGTST_UI_TO_F(*(GLuint *) where);
        if (swapBytes) swap4(where);
	break;
      case GL_UNSIGNED_SHORT:
        if (swapBytes) swap2(where);
	value = OGTST_US_TO_F(*(GLushort *) where);
        if (swapBytes) swap2(where);
	break;
      case GL_UNSIGNED_BYTE:
	value = OGTST_UB_TO_F(*(GLubyte *) where);
	break;
    }

    return value;
}

static void
generateItype(void *where, unsigned int bufMask)
{
    int value;

    /*
    ** No fractional value to this number because the spec doesn't require
    ** any index precision to the right of the decimal point, but it doesn't
    ** prohibit it either (it is legal for "1.5 << 1" to be turned into 
    ** either 2 or 3 [using indexShift]).
    */
    value = ogLibIntRand(0,270) - 7;

    /*
    ** Due to intentional vagueness of the spec as far as the size of the
    ** internal representation (makes a difference when using right shifts
    ** conjunction with negative numbers) we mask the value to the size of
    ** the frame buffer.
    */
    value &= bufMask;
    /*
    ** If we are shifting right, we make sure all of the bits 
    ** that we shift away are all zeros in case we are color mapping (either
    ** to RGBA or to another index).  This way, we are not dependend upon
    ** whether the implementation has any fractional bits.
    */
    if (indexShift < 0) {
	value = value & ~((1 << (-indexShift)) - 1);
    }
    switch(type) {
      case GL_FLOAT:
	*(GLfloat *) where = value;
        if (swapBytes) swap4(where);
	break;
      case GL_INT:
	*(GLint *) where = value;
        if (swapBytes) swap4(where);
	break;
      case GL_SHORT:
	*(GLshort *) where = value;
        if (swapBytes) swap2(where);
	break;
      case GL_BYTE:
	*(GLbyte *) where = value;
	break;
      case GL_UNSIGNED_INT:
	*(GLuint *) where = value;
        if (swapBytes) swap4(where);
	break;
      case GL_UNSIGNED_SHORT:
	*(GLushort *) where = value;
        if (swapBytes) swap2(where);
	break;
      case GL_UNSIGNED_BYTE:
	*(GLubyte *) where = value;
	break;
    }
}

static int
getItype(void *where)
{
    float value;

    switch(type) {
      case GL_FLOAT:
        if (swapBytes) swap4(where);
	value = *(GLfloat *) where;
        if (swapBytes) swap4(where);
	break;
      case GL_INT:
        if (swapBytes) swap4(where);
	value = *(GLint *) where;
        if (swapBytes) swap4(where);
	break;
      case GL_SHORT:
        if (swapBytes) swap2(where);
	value = *(GLshort *) where;
        if (swapBytes) swap2(where);
	break;
      case GL_BYTE:
	value = *(GLbyte *) where;
	break;
      case GL_UNSIGNED_INT:
        if (swapBytes) swap4(where);
	value = *(GLuint *) where;
        if (swapBytes) swap4(where);
	break;
      case GL_UNSIGNED_SHORT:
        if (swapBytes) swap2(where);
	value = *(GLushort *) where;
        if (swapBytes) swap2(where);
	break;
      case GL_UNSIGNED_BYTE:
	value = *(GLubyte *) where;
	break;
    }

    return value;
}

static void
generatePixel(int x, int y)
{
    char *offset;
    int el;

    offset = (char*)rowOffset(y);
    el = elementsPerGroup() * (x + skipPixels);
    
    if (type == GL_BITMAP) {
	int byte, bitnum;

	byte = el>>3;
	bitnum = el & 7;

	if (!lsbFirst) bitnum = 7-bitnum;

	if (ogLibBitRand(3) & 1) {
	    offset[byte] |= (1 << bitnum);
	} else {
	    offset[byte] &= ~(1 << bitnum);
	}
    } else {
	int bpe = elementSize();
        offset += el * bpe;
	switch(format) {
	  case GL_RED:
	  case GL_DEPTH_COMPONENT:
	  case GL_BLUE:
	  case GL_GREEN:
	  case GL_ALPHA:
	  case GL_LUMINANCE:
	    generateCtype(offset);
	    break;
	  case GL_LUMINANCE_ALPHA:
	    generateCtype(offset);
	    generateCtype(offset+bpe);
	    break;
	  case GL_RGB:
	    generateCtype(offset);
	    generateCtype(offset+bpe);
	    generateCtype(offset+bpe+bpe);
	    break;
	  case GL_RGBA:
#ifdef GL_EXT_abgr   
	  case GL_ABGR_EXT:
#endif
	    generateCtype(offset);
	    generateCtype(offset+bpe);
	    generateCtype(offset+bpe+bpe);
	    generateCtype(offset+bpe+bpe+bpe);
	    break;
	  case GL_COLOR_INDEX:
	    generateItype(offset, indexBufMask);
	    break;
	  case GL_STENCIL_INDEX:
	    generateItype(offset, stencilBufMask);
	    break;
	}
    }
}

static void
buildImage(void)
{
    int i,j;

    for (i=0; i < height; i++) {
        for (j=0; j < width; j++) {
            generatePixel(j,i);
        }
    }
}

static float
CompToComp(PixelMap *map, float comp)
{
    int i;
    float scale;
    float error;

    i = comp * (map->size - 1) + 0.5;
    scale = comp * (map->size - 1) + 0.5;
    if (i < 0) i = 0;
    else if (i > (map->size - 1)) i = map->size - 1;
    else if (scale > 0.5 && scale < map->size - 1.5) {
	/* Check for possible rounding error problems */
	error = scale - i;
	if (error < roundErr || error > (1 - roundErr)) {
	    /* Our expected results cannot be trusted */
	    errorFail = GL_TRUE;
	}
    }
    return map->entries[i];
}

static float
IndexToComp(PixelMap *map, int index)
{
    int i;

    i = index & (map->size - 1);
    return map->entries[i];
}

static int
IndexToIndex(PixelMap *map, int index)
{
    int i;

    i = index & (map->size - 1);
    return map->entries[i];
}

#if DONT_STOP_ON_ERROR
#define ERROR_STATUS 1
#define NO_ERROR_STATUS 0
static int
#else
#define ERROR_STATUS
#define NO_ERROR_STATUS
static void
#endif
checkPixel(int x, int y, int imX, int imY, int imW)
{
    GLfloat left, right, bottom, top;
    int li,ri,bi,ti;
    char *offset;
    int el;
    int bpe;
    int index;
    float r,g,b,a,d;
    GLuint expected;
    GLubyte expectedDepth;

    errorFail = GL_FALSE;
    /* Compute screen real estate we expect to cover */
    if (xZoom >= 0) {
	left = rastX + xZoom * x;
	right = rastX + xZoom * (x+1);
    } else {
	left = rastX + xZoom * (x+1);
	right = rastX + xZoom * x;
    }
    if (yZoom >= 0) {
	bottom = rastY + yZoom * y;
	top = rastY + yZoom * (y+1);
    } else {
	bottom = rastY + yZoom * (y+1);
	top = rastY + yZoom * y;
    }

    li = ceilf(left + 0.5) - 1;
    ri = ceilf(right + 0.5) - 1;
    bi = ceilf(bottom + 0.5) - 1;
    ti = ceilf(top + 0.5) - 1;

    if (li < 0)
        li = 0;
    if (ri > xsize)
        ri = xsize;
    if (bi < 0)
        bi = 0;
    if (ti > ysize)
        ti = ysize;
    if (ri <= li || ti <= bi)
        return NO_ERROR_STATUS;
    li -= imX; ri -= imX;
    bi -= imY; ti -= imY;

    /* Compute desired color */
    offset = (char*)rowOffset(y);
    el = elementsPerGroup() * (x + skipPixels);
    bpe = elementSize();

    if (type == GL_BITMAP) {
	int byte, bitnum;

	byte = el>>3;
	bitnum = el & 7;

	if (!lsbFirst) bitnum = 7-bitnum;

	if (offset[byte] & (1 << bitnum)) {
	    index = 1;
	} else {
	    index = 0;
	}
    } else {
	offset += el * bpe;
	switch(format) {
	  case GL_RED:
	    r = getCtype(offset);
	    g = b = 0;
	    a = 1;
	    break;
	  case GL_DEPTH_COMPONENT:
	    d = getCtype(offset);
	    break;
	  case GL_BLUE:
	    b = getCtype(offset);
	    r = g = 0;
	    a = 1;
	    break;
	  case GL_GREEN:
	    g = getCtype(offset);
	    r = b = 0;
	    a = 1;
	    break;
	  case GL_ALPHA:
	    a = getCtype(offset);
	    r = g = b = 0;
	    break;
	  case GL_LUMINANCE:
	    r = g = b = getCtype(offset);
	    a = 1;
	    break;
	  case GL_LUMINANCE_ALPHA:
	    r = g = b = getCtype(offset);
	    a = getCtype(offset+bpe);
	    break;
	  case GL_RGB:
	    r = getCtype(offset);
	    g = getCtype(offset+bpe);
	    b = getCtype(offset+bpe+bpe);
	    a = 1;
	    break;
	  case GL_RGBA:
	    r = getCtype(offset);
	    g = getCtype(offset+bpe);
	    b = getCtype(offset+bpe+bpe);
	    a = getCtype(offset+bpe+bpe+bpe);
	    break;
#ifdef GL_EXT_abgr   
	  case GL_ABGR_EXT:
	    a = getCtype(offset);
	    b = getCtype(offset+bpe);
	    g = getCtype(offset+bpe+bpe);
	    r = getCtype(offset+bpe+bpe+bpe);
	    break;
#endif
	  case GL_COLOR_INDEX:
	  case GL_STENCIL_INDEX:
	    index = getItype(offset);
            /*
             * Prior to KONA, misguided implementation did not perform sign
             * extension on signed types for color/stencil indices.is no sign
             * Thus mask out the sign extension bits for formats
             * smaller than an int.
             */
	    switch(type) {
	      case GL_BYTE:
	      case GL_UNSIGNED_BYTE:
		index &= 0xff;
		break;
	      case GL_SHORT:
	      case GL_UNSIGNED_SHORT:
		index &= 0xffff;
		break;
	    }
	    break;
	}
    }

    /* pixel modification */
    switch(format) {
      case GL_COLOR_INDEX:
      case GL_STENCIL_INDEX:
	if (indexShift > 0) {
	    index = index << indexShift;
	} else if (indexShift < 0) {
	    index = index >> (-indexShift);
	} 
	index += indexOffset;
	if (format == GL_COLOR_INDEX) {
	    if (rgbMode) {
		r = IndexToComp(&mapIR, index);
		g = IndexToComp(&mapIG, index);
		b = IndexToComp(&mapIB, index);
		a = IndexToComp(&mapIA, index);
                CLAMP01(r);
                CLAMP01(g);
                CLAMP01(b);
                CLAMP01(a);
	    } else {
		if (mapColor)
		    index = IndexToIndex(&mapII, index);
                expected = index & indexBufMask;
                for (; bi < ti; bi++)
                    for (; li < ri; li++)
                        if (outImage[bi * imW + li] != expected) {
                            ogEnvLog(OG_LFAIL, 
                                     "[%d,%d] Expected color index %d, got %d\n", 
                                     li, bi, expected, outImage[bi * imW + li]);
                            return ERROR_STATUS;
                        }
                return NO_ERROR_STATUS;
            }
	} else {
	    /* format == GL_STENCIL_INDEX */
	    if (mapStencil) {
		index = IndexToIndex(&mapSS, index);
	    }
            expected = index & stencilBufMask;
            for (; bi < ti; bi++)
                for (; li < ri; li++)
                    if ( outImage[bi * imW + li] != expected) {
                        ogEnvLog(OG_LFAIL, "[%d,%d] Expected stencil %d, got %d\n", 
                                 li, bi, expected, outImage[bi * imW + li]);
                        return ERROR_STATUS;
                    }
            return NO_ERROR_STATUS;
	}
	break;
      case GL_DEPTH_COMPONENT: {
          int diff;
          d = d * dScale + dBias;
          CLAMP01(d);
          expectedDepth = OGTST_F_TO_UB(d);
          for (; bi < ti; bi++)
              for (; li < ri; li++) {
                  diff = ((GLubyte *) outImage)[bi * imW + li] -
                      expectedDepth;
                  if (ABS(diff) > 3) {
                      ogEnvLog(OG_LFAIL, "[%d,%d] Expected depth %d+/-3, got %d\n",
                               bi, li, expectedDepth,
                               ((GLubyte *) outImage)[bi * imW + li]);
                      return ERROR_STATUS;
                  }
              }
          return NO_ERROR_STATUS;
      }
      /* break; Can't get here */
      case GL_RGB:
      case GL_RGBA:
#ifdef GL_EXT_abgr   
      case GL_ABGR_EXT:
#endif
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
	r = r * rScale + rBias;
	g = g * gScale + gBias;
	b = b * bScale + bBias;
	a = a * aScale + aBias;
	CLAMP01(r);
	CLAMP01(g);
	CLAMP01(b);
	CLAMP01(a);
	if (mapColor) {
	    r = CompToComp(&mapRR, r);
	    g = CompToComp(&mapGG, g);
	    b = CompToComp(&mapBB, b);
	    a = CompToComp(&mapAA, a);
            if (errorFail)
                return NO_ERROR_STATUS;
	    CLAMP01(r);
	    CLAMP01(g);
	    CLAMP01(b);
	    CLAMP01(a);
	}
	break;
    }
    /* If we get here we're in rgbMode */
    expected = OGTST_F_TO_UB(r) << 24 | OGTST_F_TO_UB(g) << 16 |
        OGTST_F_TO_UB(b) << 8  | OGTST_F_TO_UB(a);
    for (; bi < ti; bi++)
        for (; li < ri; li++) {
	    GLuint actual = outImage[bi * imW + li];
#ifdef WIN32
	    swap4((void*)&actual);
#endif
            if (ogLibColCheck(actual, expected, MAX_COLOR_ERR)) {
                ogEnvLog(OG_LFAIL, "[%d,%d] Expected %s, got %s\n", 
                         li, bi, ogEnvColorString(expected), 
                         ogEnvColorString(outImage[bi * imW + li]));
                return ERROR_STATUS;
            }
	}
    return NO_ERROR_STATUS;
}

/* Ugh.  Figure out if what appeared on the screen seems reasonable. */
static void checkResults(void)
{
    int i,j;
    GLfloat left, bottom;
    int li,bi, inX, inY;

    errorFail = GL_FALSE;
    /* Compute screen real estate we expect to cover */
    if (xZoom >= 0) {
        left = rastX;
        inX = xZoom * width;
    } else {
        left = rastX + xZoom * width;
        inX = -xZoom * width;
    }
    if (yZoom >= 0) {
        bottom = rastY;
        inY = yZoom * height;
    } else {
        bottom = rastY + yZoom * height;
        inY = -yZoom * height;
    }
    li = ceilf(left + 0.5) - 1;
    bi = ceilf(bottom + 0.5) - 1;

    if (li < 0) {
        inX += li;
        li = 0;
    }
    if (bi < 0) {
        inY += bi;
        bi = 0;
    }
    if (li + inX >= xsize)
        inX = xsize - li;
    if (bi + inY >= ysize)
        inY = ysize - bi;
        
    switch (format) {
      case GL_DEPTH_COMPONENT:
        glReadPixels(li, bi, inX, inY, format, GL_UNSIGNED_BYTE, outImage);
        /* Need to account for word alignment in check pixel */
        inX = (inX + 3) & ~3;
        break;
      case GL_STENCIL_INDEX:
        glReadPixels(li, bi, inX, inY, format, GL_UNSIGNED_INT, outImage);
        break;
      default:
        if (rgbMode)
            glReadPixels(li, bi, inX, inY, GL_RGBA, GL_UNSIGNED_BYTE, outImage);
        else
            glReadPixels(li, bi, inX, inY, GL_COLOR_INDEX, GL_UNSIGNED_INT,
                         outImage);
    }
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
#if DONT_STOP_ON_ERROR                    
            if (checkPixel(j, i, li, bi, inX))
                return;
#else
            checkPixel(j, i, li, bi, inX);
#endif /* DONT_STOP_ON_ERROR */
        }
    }
}

static void performPixelTest(void)
{
#if CLEAR_SCREEN
    GLubyte clearR = 0, clearG = 0, clearB = 0, clearA = 0;
#endif
    if (mapColor) {
	if (rgbMode) {
	    buildCCMap(&mapRR);
	    buildCCMap(&mapGG);
	    buildCCMap(&mapBB);
	    buildCCMap(&mapAA);
	} else {
	    buildIIMap(&mapII);
	}
    } 
    if (mapStencil) {
	buildIIMap(&mapSS);
    }
    if (rgbMode && format == GL_COLOR_INDEX) {
	buildICMap(&mapIR);
	buildICMap(&mapIG);
	buildICMap(&mapIB);
	buildICMap(&mapIA);
    }

    uploadModes(1);

    buildImage();

    glRasterPos2i(1,1);
    glBitmap(0,0,0,0,rastX-1,rastY-1,NULL);

#if CLEAR_SCREEN
    ogLibClear((clearR << 24) | (clearG << 16) | (clearB << 8) | clearA);
#endif
    ogEnvLog(1, "RasterPos at %g %g\n", rastX, rastY);
    ogEnvLog(1, "glDrawPixels(%d, %d, %s, %s, 0x%x);\n", width, height, 
             ogEnvPixelFormatName(format), ogEnvDataTypeName(type),
             inImage+misalignment);

    if (misalignment) {
        ogEnvLog(1, "Misalignment %d\n", misalignment);
	memcpy(inImage+misalignment, inImage, MAX_IMAGE_SIZE-4);
	glDrawPixels(width, height, format, type, inImage+misalignment);
	memcpy(inImage, inImage+misalignment, MAX_IMAGE_SIZE-4);
    } else {
	glDrawPixels(width, height, format, type, inImage);
    }
    /* Reset transfer modes so we can read the image */
    uploadDefaults();

    checkResults();
}

CLEANUP(pixel)
{
    buildDefaultMaps();
    setDefaultModes();
    uploadModes(0);
    /* Free only when there was no failure */
    if (hasDepth) {
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
    }
    if (hasStencil) {
	glDisable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0, stencilBufMask);
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glIndexMask(rgbMode ? 0 : indexBufMask);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glIndexf(1);
    glColor4f(1, 1, 1, 1);
    ogLibSetDefaultRasterPos();
    glPixelZoom(1, 1);
}

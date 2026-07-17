/*
 *  ImageUtil.c
 *      This is an utility library useful for building a variety of
 *      test image pattern in varies OpenGL PixelStore format. These 
 *      will eventually include checker board, ramp and .rgb file.
 *      Typical usage:
 *
 *    IU_FColor color0, color1;
 *    IU_Image image;
 *    IU_Rect rect, tileRect;
 *
 *    color0.r = 0.8; color0.g = 0.6; color0.b = 0.4; color0.a = 0.2;
 *    color1.r = 0.1; color1.g = 0.3; color1.b = 0.5; color1.a = 0.7;
 *    IU_InitChecker(color0, color1, 4, 4);
 *
 *    rect.x1 = 0;   rect.y1 = 0;
 *    rect.x2 = 640; rect.y2 = 480;
 *
 *    image.width = 32;
 *    image.height = 32;
 *    image.format = GL_LUMINANCE;
 *    image.type = GL_UNSIGNED_BYTE;
 *    image.alignment = 1;
 *
 *    numByteInImage = IU_NumByteInImage(&image);
 *    if (!(image.pImage = (unsigned char *)malloc(numByteInImage))) {
 *	ogEnvLog(OG_LINTERNALERROR, "Cannot malloc image buffer memory. \n");
 *    }
 *
 *    if (IU_BuildImage(&image, IU_CHECKER)) {
 *	ogEnvLog(OG_LALWAYS, "Image build failed. \n");
 *	...
 *    }
 * 
 */
#include <assert.h>
#include <bstring.h>
#include <stdio.h>
#include <string.h>
#include "ogtst.h"
#include "imbuildutil.h"

typedef struct {
    GLint numByteInLine, numInvalidByteInLine;
    GLint numByteInRow;
    GLint numByteInPix;
    GLint numByteInComp;
    GLint numByteInRect;
} HostImage;


static IU_Checker checker;
static IU_Ramp ramp;
static IU_Tile tile;
static verbose = 0;

static GLubyte byteReverse[256], byteReverseInit = 0;

GLint 
IU_NumCompInPix(GLenum format)
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
      default:
	ogEnvLog(OG_LINTERNALERROR, "NumCompInPix: %x not supported. \n", format);
	return 0;
    }
}

GLint 
IU_NumByteInType(GLenum type)
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
#ifdef GL_EXT_packed_pixels
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
	return 1;
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
	return 2;
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
	return 2;
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
	return 4;
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
	return 4;
#endif
      default:
	ogEnvLog(OG_LALWAYS, "NumByteInType: %x type not supported. \n", type);
	return 0;
    }
}

static GLint
IsPackedType(GLenum type) {

#ifdef GL_EXT_packed_pixels
    return GL_UNSIGNED_BYTE_3_3_2_EXT <= type &&
        type <= GL_UNSIGNED_INT_10_10_10_2_EXT;
#else
    return GL_FALSE;
#endif
}


	
/* HostFomat
 *    Compute parameter essential in describing host image.
 *    Note: line is longer than row so line contains invalid bytes.
 */
static GLint
HostFormat(IU_Image *pI, HostImage *pH) {
    GLint numCompInPix, numByteInType, numByteInPix, numByteInRow;
    GLint numInvalidByteInLine, numByteInLine, numByteInRect;
    GLint numByteRemain;
    
    if (pI->type == GL_BITMAP) {
	numCompInPix = IU_NumCompInPix(pI->format);
	if (numCompInPix != 1) {
	    ogEnvLog(OG_LALWAYS, "Format %s not compatible with GL_BITMAP\n",
                     ogEnvPixelFormatName(pI->format));
	}

	/* the following 2 values are unused for GL_BITMAP */
	numByteInPix = 0;
	numByteInType = 0;

	numByteInRow = (pI->width + 7) >> 3;
	numByteInLine = numByteInRow;
	while (numByteInLine % pI->alignment) numByteInLine++;
	numInvalidByteInLine = numByteInLine - numByteInRow;
	numByteInRect = numByteInLine * pI->height;

	pH->numByteInLine = numByteInLine; 
	pH->numInvalidByteInLine = numInvalidByteInLine;
	pH->numByteInRow = numByteInRow;
	pH->numByteInPix = numByteInPix;
	pH->numByteInRect = numByteInRect;
	pH->numByteInComp = numByteInType;
    } else {
	numCompInPix = IU_NumCompInPix(pI->format);
	numByteInType = IU_NumByteInType(pI->type);
	
	if (IsPackedType(pI->type)) {
	    numByteInPix = numByteInType;
	} else {
	    numByteInPix = numCompInPix * numByteInType;
	}

	numByteInRow = pI->width * numByteInPix;
	numByteRemain = numByteInRow % pI->alignment;
	numInvalidByteInLine = numByteRemain ? 
	    pI->alignment - numByteRemain : 0;
	numByteInLine = numByteInRow + numInvalidByteInLine;
	numByteInRect = numByteInLine * pI->height;

	pH->numByteInLine = numByteInLine; 
	pH->numInvalidByteInLine = numInvalidByteInLine;
	pH->numByteInRow = numByteInRow;
	pH->numByteInPix = numByteInPix;
	pH->numByteInRect = numByteInRect;
	pH->numByteInComp = numByteInType;
    }
    return 0;
}

static GLint
ParseInput(IU_Image *pI, HostImage *pH) {
    
    switch(pI->format) {
      case GL_RGB:
      case GL_RGBA:
#ifdef GL_EXT_abgr
      case GL_ABGR_EXT:
#endif
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_COLOR_INDEX:
      case GL_STENCIL_INDEX:
      case GL_DEPTH_COMPONENT:
	break;
      default:
	ogEnvLog(OG_LALWAYS, "Format %s not supported yet. \n",
                 ogEnvPixelFormatName(pI->format));
	return 1;
    }

    switch(pH->numByteInComp) {
      case 2:
	/* Image cannot begin on odd boundary because component size is 2 */
	if ((__psint_t)pI->pImage & 0x1) return 1;
	break;
      case 4:
	/* Image cannot begin on non-word boundary because component size is 2 */
	if ((__psint_t)pI->pImage & 0x3) return 1;
	break;
      default:
	break;
    }
    return 0;
}

static void Swap2Byte(GLbyte *pStart) {
    GLbyte tmp;
    tmp = *pStart;
    *pStart = *(pStart+1);
    *(pStart+1) = tmp;
}

static void Swap4Byte(GLbyte *pStart) {
    GLbyte tmp;

    tmp = *(pStart+0);
    *(pStart+0) = *(pStart+3);
    *(pStart+3) = tmp;

    tmp = *(pStart+1);
    *(pStart+1) = *(pStart+2);
    *(pStart+2) = tmp;    
}

static void
SwapByteInChecker(IU_Checker *pCol) {
    /* Short */
    Swap2Byte((GLbyte *) &pCol->color0.sColor.r);
    Swap2Byte((GLbyte *) &pCol->color0.sColor.g);
    Swap2Byte((GLbyte *) &pCol->color0.sColor.b);
    Swap2Byte((GLbyte *) &pCol->color0.sColor.a);

    Swap2Byte((GLbyte *) &pCol->color1.sColor.r);
    Swap2Byte((GLbyte *) &pCol->color1.sColor.g);
    Swap2Byte((GLbyte *) &pCol->color1.sColor.b);
    Swap2Byte((GLbyte *) &pCol->color1.sColor.a);

    /* Unsigned short */
    Swap2Byte((GLbyte *) &pCol->color0.usColor.r);
    Swap2Byte((GLbyte *) &pCol->color0.usColor.g);
    Swap2Byte((GLbyte *) &pCol->color0.usColor.b);
    Swap2Byte((GLbyte *) &pCol->color0.usColor.a);

    Swap2Byte((GLbyte *) &pCol->color1.usColor.r);
    Swap2Byte((GLbyte *) &pCol->color1.usColor.g);
    Swap2Byte((GLbyte *) &pCol->color1.usColor.b);
    Swap2Byte((GLbyte *) &pCol->color1.usColor.a);

    /* Int */
    Swap4Byte((GLbyte *) &pCol->color0.iColor.r);
    Swap4Byte((GLbyte *) &pCol->color0.iColor.g);
    Swap4Byte((GLbyte *) &pCol->color0.iColor.b);
    Swap4Byte((GLbyte *) &pCol->color0.iColor.a);

    Swap4Byte((GLbyte *) &pCol->color1.iColor.r);
    Swap4Byte((GLbyte *) &pCol->color1.iColor.g);
    Swap4Byte((GLbyte *) &pCol->color1.iColor.b);
    Swap4Byte((GLbyte *) &pCol->color1.iColor.a);

    /* Unsigned Unsigned */
    Swap4Byte((GLbyte *) &pCol->color0.uiColor.r);
    Swap4Byte((GLbyte *) &pCol->color0.uiColor.g);
    Swap4Byte((GLbyte *) &pCol->color0.uiColor.b);
    Swap4Byte((GLbyte *) &pCol->color0.uiColor.a);

    Swap4Byte((GLbyte *) &pCol->color1.uiColor.r);
    Swap4Byte((GLbyte *) &pCol->color1.uiColor.g);
    Swap4Byte((GLbyte *) &pCol->color1.uiColor.b);
    Swap4Byte((GLbyte *) &pCol->color1.uiColor.a);

    /* Float */
    Swap4Byte((GLbyte *) &pCol->color0.fColor.r);
    Swap4Byte((GLbyte *) &pCol->color0.fColor.g);
    Swap4Byte((GLbyte *) &pCol->color0.fColor.b);
    Swap4Byte((GLbyte *) &pCol->color0.fColor.a);

    Swap4Byte((GLbyte *) &pCol->color1.fColor.r);
    Swap4Byte((GLbyte *) &pCol->color1.fColor.g);
    Swap4Byte((GLbyte *) &pCol->color1.fColor.b);
    Swap4Byte((GLbyte *) &pCol->color1.fColor.a);
}

static void
BuildChecker(IU_Image *pI, HostImage *pH) {
    GLint iRow, iCol, i;
    GLint numByteInComp;
    GLbyte *pColR, *pColG, *pColB, *pColA, *pIndex;
    GLbyte *pCol0R, *pCol0G, *pCol0B, *pCol0A, *pIndex0;
    GLbyte *pCol1R, *pCol1G, *pCol1B, *pCol1A, *pIndex1;
    GLbyte *pBuf;
    IU_Checker col;
    GLint numByteInPix, Col0Pack = 0, Col1Pack = 0;
    GLbyte *pCol0Pack, *pCol1Pack, *pColPack;
    unsigned short r0US,g0US,b0US,a0US, r1US,g1US,b1US,a1US;


    pBuf = (GLbyte *)pI->pImage;
    col = checker;
    if (pI->swap)
	SwapByteInChecker(&col);

    /* init for packed color */
#ifdef GL_EXT_abgr
    if (pI->format != GL_ABGR_EXT) {
	r0US = col.color0.usColor.r;
	g0US = col.color0.usColor.g;
	b0US = col.color0.usColor.b;
	a0US = col.color0.usColor.a;
	r1US = col.color1.usColor.r;
	g1US = col.color1.usColor.g;
	b1US = col.color1.usColor.b;
	a1US = col.color1.usColor.a;
    } else {
	r0US = col.color0.usColor.a;
	g0US = col.color0.usColor.b;
	b0US = col.color0.usColor.g;
	a0US = col.color0.usColor.r;
	r1US = col.color1.usColor.a;
	g1US = col.color1.usColor.b;
	b1US = col.color1.usColor.g;
	a1US = col.color1.usColor.r;
    }
#else
    r0US = col.color0.usColor.r;
    g0US = col.color0.usColor.g;
    b0US = col.color0.usColor.b;
    a0US = col.color0.usColor.a;
    r1US = col.color1.usColor.r;
    g1US = col.color1.usColor.g;
    b1US = col.color1.usColor.b;
    a1US = col.color1.usColor.a;
#endif

    switch(pI->type) {
      case GL_BYTE:
	pCol0R = (GLbyte *) &col.color0.bColor.r;
	pCol0G = (GLbyte *) &col.color0.bColor.g;
	pCol0B = (GLbyte *) &col.color0.bColor.b;
	pCol0A = (GLbyte *) &col.color0.bColor.a;
	pIndex0 = (GLbyte *) &col.index0.bIndex;

	pCol1R = (GLbyte *) &col.color1.bColor.r;
	pCol1G = (GLbyte *) &col.color1.bColor.g;
	pCol1B = (GLbyte *) &col.color1.bColor.b;
	pCol1A = (GLbyte *) &col.color1.bColor.a;
	pIndex1 = (GLbyte *) &col.index1.bIndex;

	numByteInComp = 1;
	break;
      case GL_UNSIGNED_BYTE:
	pCol0R = (GLbyte *) &col.color0.ubColor.r;
	pCol0G = (GLbyte *) &col.color0.ubColor.g;
	pCol0B = (GLbyte *) &col.color0.ubColor.b;
	pCol0A = (GLbyte *) &col.color0.ubColor.a;
	pIndex0 = (GLbyte *) &col.index0.ubIndex;

	pCol1R = (GLbyte *) &col.color1.ubColor.r;
	pCol1G = (GLbyte *) &col.color1.ubColor.g;
	pCol1B = (GLbyte *) &col.color1.ubColor.b;
	pCol1A = (GLbyte *) &col.color1.ubColor.a;
	pIndex1 = (GLbyte *) &col.index1.ubIndex;

	numByteInComp = 1;
	break;
      case GL_SHORT:
	pCol0R = (GLbyte *) &col.color0.sColor.r;
	pCol0G = (GLbyte *) &col.color0.sColor.g;
	pCol0B = (GLbyte *) &col.color0.sColor.b;
	pCol0A = (GLbyte *) &col.color0.sColor.a;
	pIndex0 = (GLbyte *) &col.index0.sIndex;

	pCol1R = (GLbyte *) &col.color1.sColor.r;
	pCol1G = (GLbyte *) &col.color1.sColor.g;
	pCol1B = (GLbyte *) &col.color1.sColor.b;
	pCol1A = (GLbyte *) &col.color1.sColor.a;
	pIndex1 = (GLbyte *) &col.index1.sIndex;

	numByteInComp = 2;
	break;
      case GL_UNSIGNED_SHORT:
	pCol0R = (GLbyte *) &col.color0.usColor.r;
	pCol0G = (GLbyte *) &col.color0.usColor.g;
	pCol0B = (GLbyte *) &col.color0.usColor.b;
	pCol0A = (GLbyte *) &col.color0.usColor.a;
	pIndex0 = (GLbyte *) &col.index0.usIndex;

	pCol1R = (GLbyte *) &col.color1.usColor.r;
	pCol1G = (GLbyte *) &col.color1.usColor.g;
	pCol1B = (GLbyte *) &col.color1.usColor.b;
	pCol1A = (GLbyte *) &col.color1.usColor.a;
	pIndex1 = (GLbyte *) &col.index1.usIndex;

	numByteInComp = 2;
	break;
      case GL_INT:
	pCol0R = (GLbyte *) &col.color0.iColor.r;
	pCol0G = (GLbyte *) &col.color0.iColor.g;
	pCol0B = (GLbyte *) &col.color0.iColor.b;
	pCol0A = (GLbyte *) &col.color0.iColor.a;
	pIndex0 = (GLbyte *) &col.index0.iIndex;

	pCol1R = (GLbyte *) &col.color1.iColor.r;
	pCol1G = (GLbyte *) &col.color1.iColor.g;
	pCol1B = (GLbyte *) &col.color1.iColor.b;
	pCol1A = (GLbyte *) &col.color1.iColor.a;
	pIndex1 = (GLbyte *) &col.index1.iIndex;

	numByteInComp = 4;
	break;
      case GL_UNSIGNED_INT:
	pCol0R = (GLbyte *) &col.color0.uiColor.r;
	pCol0G = (GLbyte *) &col.color0.uiColor.g;
	pCol0B = (GLbyte *) &col.color0.uiColor.b;
	pCol0A = (GLbyte *) &col.color0.uiColor.a;
	pIndex0 = (GLbyte *) &col.index0.uiIndex;

	pCol1R = (GLbyte *) &col.color1.uiColor.r;
	pCol1G = (GLbyte *) &col.color1.uiColor.g;
	pCol1B = (GLbyte *) &col.color1.uiColor.b;
	pCol1A = (GLbyte *) &col.color1.uiColor.a;
	pIndex1 = (GLbyte *) &col.index1.uiIndex;

	numByteInComp = 4;
	break;
      case GL_FLOAT:
	pCol0R = (GLbyte *) &col.color0.fColor.r;
	pCol0G = (GLbyte *) &col.color0.fColor.g;
	pCol0B = (GLbyte *) &col.color0.fColor.b;
	pCol0A = (GLbyte *) &col.color0.fColor.a;
	pIndex0 = (GLbyte *) &col.index0.fIndex;

	pCol1R = (GLbyte *) &col.color1.fColor.r;
	pCol1G = (GLbyte *) &col.color1.fColor.g;
	pCol1B = (GLbyte *) &col.color1.fColor.b;
	pCol1A = (GLbyte *) &col.color1.fColor.a;
	pIndex1 = (GLbyte *) &col.index1.fIndex;

	numByteInComp = 4;
	break;
#ifdef GL_EXT_packed_pixels
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
	*((unsigned char *) & Col0Pack) = (unsigned char) (
                    ((col.color0.usColor.r >> 13) << 5) |
		    ((col.color0.usColor.g >> 13) << 2) |
		    ((col.color0.usColor.b >> 14) << 0));
	pCol0Pack = (GLbyte *) & Col0Pack;
	
	*((unsigned char *) & Col1Pack) = (unsigned char) (
	            ((col.color1.usColor.r >> 13) << 5) |
		    ((col.color1.usColor.g >> 13) << 2) |
		    ((col.color1.usColor.b >> 14) << 0));	
	pCol1Pack = (GLbyte *) & Col1Pack;

	numByteInPix = 1;
	break;
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
	*((unsigned short *) & Col0Pack) = (unsigned short) (
                    ((r0US >> 12) << 12) |
		    ((g0US >> 12) << 8)  |
		    ((b0US >> 12) << 4)  |
		    ((a0US >> 12) << 0));
	pCol0Pack = (GLbyte *) & Col0Pack;
	
	*((unsigned short *) & Col1Pack) = (unsigned short) (
                    ((r1US >> 12) << 12) |
		    ((g1US >> 12) << 8)  |
		    ((b1US >> 12) << 4)  |	
		    ((a1US >> 12) << 0));	
	pCol1Pack = (GLbyte *) & Col1Pack;

	numByteInPix = 2;
	break;
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
	*((unsigned short *) & Col0Pack) = (unsigned short) (
	            ((r0US >> 11) << 11) |
		    ((g0US >> 11) << 6)  |
		    ((b0US >> 11) << 1)  |
		    ((a0US >> 15) << 0));
	pCol0Pack = (GLbyte *) & Col0Pack;
	
	*((unsigned short *) & Col1Pack) = (unsigned short) (
		    ((r1US >> 11) << 11) |
		    ((g1US >> 11) << 6)  |
		    ((b1US >> 11) << 1)  |	
		    ((a1US >> 15) << 0));	
	pCol1Pack = (GLbyte *) & Col1Pack;

	numByteInPix = 2;
	break;
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
	Col0Pack = (((r0US >> 8) << 24) |
		    ((g0US >> 8) << 16) |
		    ((b0US >> 8) << 8)  |
		    ((a0US >> 8) << 0));
	pCol0Pack = (GLbyte *) & Col0Pack;
	
	Col1Pack = (((r1US >> 8) << 24) |
		    ((g1US >> 8) << 16) |
		    ((b1US >> 8) << 8)  |	
		    ((a1US >> 8) << 0));	
	pCol1Pack = (GLbyte *) & Col1Pack;

	numByteInPix = 4;
	break;
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
	Col0Pack = (((r0US >> 6 ) << 22) |
		    ((g0US >> 6 ) << 12) |
		    ((b0US >> 6 ) << 2)  |
		    ((a0US >> 14) << 0));
	pCol0Pack = (GLbyte *) & Col0Pack;
	
	Col1Pack = (((r1US >> 6 ) << 22) |
		    ((g1US >> 6 ) << 12) |
		    ((b1US >> 6 ) << 2)  |	
		    ((a1US >> 14) << 0));	
	pCol1Pack = (GLbyte *) & Col1Pack;

	numByteInPix = 4;
	break;
#endif
      default:
	ogEnvLog(OG_LINTERNALERROR, "BuildChecker:Type unknown.\n");
        /* The above call exists */
    }

    for (iRow=0; iRow<pI->height; iRow++) {
	for (iCol=0; iCol<pI->width; iCol++) {
	    if (((iCol/col.xMod) & 1) ^
		((iRow/col.yMod) & 1)) {		
		pColR = pCol0R;
		pColG = pCol0G;
		pColB = pCol0B;
		pColA = pCol0A;
		pIndex = pIndex0;
		pColPack = pCol0Pack;		
	    } else {
		pColR = pCol1R;
		pColG = pCol1G;
		pColB = pCol1B;
		pColA = pCol1A;
		pIndex = pIndex1;
		pColPack = pCol1Pack;	
	    }

	    if (IsPackedType(pI->type)) {
		bcopy(pColPack, pBuf, numByteInPix); pBuf += numByteInPix;
	    } else {		  
		switch(pI->format) {
		case GL_RGBA:
		    bcopy(pColR, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColG, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColB, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColA, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
#ifdef GL_EXT_abgr
		case GL_ABGR_EXT:
		    bcopy(pColA, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColB, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColG, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColR, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
#endif
		case GL_RGB:
		    bcopy(pColR, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColG, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColB, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
		case GL_DEPTH_COMPONENT:
		case GL_LUMINANCE:
		    bcopy(pColR, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_LUMINANCE_ALPHA:
		    bcopy(pColR, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(pColA, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_STENCIL_INDEX:
		case GL_COLOR_INDEX:
		    bcopy(pIndex, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		}
	    
	    }
	}
	for (i=0; i<pH->numInvalidByteInLine; i++)
	    *pBuf++ = 0x69;
    }
}

static GLubyte
SwapBitsInByte(GLubyte val) {
    int i;

    if (!byteReverseInit) {
	for (i = 0; i < 256; i++) {
	    byteReverse[i]  = (i << 7) & 0x80;
	    byteReverse[i] |= (i << 5) & 0x40;
	    byteReverse[i] |= (i << 3) & 0x20;
	    byteReverse[i] |= (i << 1) & 0x10;
	    byteReverse[i] |= (i >> 1) & 0x08;
	    byteReverse[i] |= (i >> 3) & 0x04;
	    byteReverse[i] |= (i >> 5) & 0x02;
	    byteReverse[i] |= (i >> 7) & 0x01;
	}
#ifdef DEBUG
	for (i = 0; i < 256; i++) {
	    assert(i == byteReverse[byteReverse[i]]);
	}
#endif
	byteReverseInit = 1;
    }
    
    return byteReverse[val];
}

static void
BuildCheckerBitmap(IU_Image *pI, HostImage *pH) {
    GLint iRow, iCol, i, bit, pos;
    GLbyte *pBuf, bitm;
    IU_Checker col;

    pBuf = (GLbyte *)pI->pImage;
    col = checker;

    for (iRow=0; iRow<pI->height; iRow++) {
	pos = 0;
	bitm = 0;
	for (iCol=0; iCol<pI->width; iCol++) {
	    if (((iCol/col.xMod) & 1) ^
		((iRow/col.yMod) & 1)) {
		bit = 0;
	    } else {
		bit = 1;
	    }
	    bitm |= (bit << (7 - pos));
	    pos++;
	    if (pos == 8) {
		*pBuf++ = pI->lsb ? SwapBitsInByte(bitm) : bitm;
		bitm = 0;
		pos = 0;
	    }
	}
	if (pos)
	    *pBuf++ = pI->lsb ? SwapBitsInByte(bitm) : bitm;

	for (i=0; i<pH->numInvalidByteInLine; i++)
	    *pBuf++ = 0xdb;
	assert((int)pBuf % pI->alignment == 0);
    }
}

static void
BuildRandom(IU_Image *pI, HostImage *pH) {
    GLuint bits;
    GLint numByteInImage, numCompInImage, i;
    GLubyte *ub;
    GLushort *us;
    GLuint *ui;
    GLfloat *f;
    
    numByteInImage = pH->numByteInRect;

    bits = 0;
    switch(pI->type) {
      case GL_BYTE:
	bits = 7;
      case GL_UNSIGNED_BYTE:
      case GL_BITMAP:
#ifdef GL_EXT_packed_pixels
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
#endif
	if (!bits) bits = 8;
	ub = (GLubyte *)pI->pImage;
	for (i = 0; i < numByteInImage; i++) {
	    *ub++ = ogLibBitRand(bits);
	}
	break;
      case GL_SHORT:
	bits = 15;
      case GL_UNSIGNED_SHORT:
#ifdef GL_EXT_packed_pixels
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
#endif
	if (!bits) bits = 16;
	us = (GLushort *)pI->pImage;
	numCompInImage = numByteInImage / 2;
	for (i = 0; i < numCompInImage; i++) {
	    *us++ = ogLibBitRand(bits);
	}
	break;
      case GL_INT:
	bits = 31;
      case GL_UNSIGNED_INT:
#ifdef GL_EXT_packed_pixels
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
#endif
	if (!bits) bits = 32;
	numCompInImage = numByteInImage / 4;
	ui = (GLuint *)pI->pImage;
	for (i = 0; i < numCompInImage; i++) {
	    *ui++ = ogLibBitRand(bits);
	}
	break;
      case GL_FLOAT:
	numCompInImage = numByteInImage / 4;
	f = (GLfloat *)pI->pImage;
	for (i = 0; i < numCompInImage; i++) {
	    *f++ = ogLibFloatRand(0, 1);
	}
	break;
      default:
	ogEnvLog(OG_LINTERNALERROR, "BuildRandom:Type unknown.\n");
	return;
    }
}

GLenum
IU_FormatNameToEnum(char * pStr)
{
    if (!strcmp(pStr, "GL_RGBA")) {
	return GL_RGBA;
#ifdef GL_EXT_abgr
    } else if (!strcmp(pStr, "GL_ABGR_EXT")) {
	return GL_ABGR_EXT;
#endif
    } else if (!strcmp(pStr, "GL_RGB")) {
	return GL_RGB;
    } else if (!strcmp(pStr, "GL_LUMINANCE")) {
	return GL_LUMINANCE;
    } else if (!strcmp(pStr, "GL_LUMINANCE_ALPHA")) {
	return GL_LUMINANCE_ALPHA;
    } else if (!strcmp(pStr, "GL_RED")) {
	return(GL_RED);
    } else if (!strcmp(pStr, "GL_GREEN")) {
	return(GL_GREEN);
    } else if (!strcmp(pStr, "GL_BLUE")) {
	return(GL_BLUE);
    } else if (!strcmp(pStr, "GL_ALPHA")) {
	return(GL_ALPHA);
    } else if (!strcmp(pStr, "GL_DEPTH_COMPONENT")) {
	return(GL_DEPTH_COMPONENT);
    } else if (!strcmp(pStr, "GL_STENCIL_INDEX")) {
	return(GL_STENCIL_INDEX);
    } else if (!strcmp(pStr, "GL_COLOR_INDEX")) {
	return(GL_COLOR_INDEX);
    }
    return 0;
}

/*ARGSUSED*/
static void
BuildRamp(IU_Image *pI, HostImage *pH) {
    GLint iRow, iCol;
    GLint numByteInComp;
    GLfloat r = 0, g = 0, b = 0, a = 0, i = 0;
    GLfloat val;
    GLubyte *rB = (GLubyte *)&r, *gB = (GLubyte *)&g, 
    *bB = (GLubyte *)&b, *aB = (GLubyte *)&a, *iB = (GLubyte *)&i;
    GLushort *rS = (GLushort *)&r, *gS = (GLushort *)&g, 
    *bS = (GLushort *)&b, *aS = (GLushort *)&a, *iS = (GLushort *)&i;
    GLuint *rI = (GLuint *)&r, *gI = (GLuint *)&g, 
    *bI = (GLuint *)&b, *aI = (GLuint *)&a, *iI = (GLuint *)&i;
    GLfloat *rF = (GLfloat *)&r, *gF = (GLfloat *)&g,
    *bF = (GLfloat *)&b, *aF = (GLfloat *)&a, *iF = (GLfloat *)&i;
    IU_FColor c0, c1;
    GLfloat i0, i1;
    IU_FColor c;
    GLfloat index;
    GLbyte *pBuf;
    IU_Ramp col;
    GLint  numByteInPix, ColPack = 0;
    GLbyte *pColPack;

    pBuf = (GLbyte *)pI->pImage;
    col = ramp;
    if (pI->swap) {
	ogEnvLog(OG_LINTERNALERROR, "BuildRamp:  Swap byte not supported.\n");
    }
    c0 = col.color0;
    c1 = col.color1;
    i0 = col.index0;
    i1 = col.index1;

    if (!pI->height || !pI->width)
        return;

    switch(pI->type) {
      case GL_BYTE:  case GL_UNSIGNED_BYTE:
	numByteInComp = 1;
	break;
      case GL_SHORT:  case GL_UNSIGNED_SHORT:
	numByteInComp = 2;
	break;
      case GL_INT:  case GL_UNSIGNED_INT:  case GL_FLOAT:
	numByteInComp = 4;
	break;
#ifdef GL_EXT_packed_pixels
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
	numByteInPix = 1;
	break;
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
	numByteInPix = 2;
	break;
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
	numByteInPix = 2;
	break;
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
	numByteInPix = 4;
	break;
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
	numByteInPix = 4;
	break;
#endif
      default:
	ogEnvLog(OG_LINTERNALERROR, "BuildRamp:  Type 0x%x not supported.\n", 
                 pI->type);
    }

    for (iRow=0; iRow<pI->height; iRow++) {
	for (iCol=0; iCol<pI->width; iCol++) {
	    if (pI->height > 1 && pI->width > 1) {
		val = ((float)iRow / (float)(pI->height-1) +
		       (float)iCol / (float)(pI->width-1)) / 2;
	    } else if (pI->height > 1) {
		val = (float)iRow / (float)(pI->height - 1);
	    } else if (pI->width > 1) {
		val = (float)iCol / (float)(pI->width - 1);
	    } else {
		val = 1;
	    }

	    c.r =   (val * c1.r + (1.0 - val) * c0.r);
	    c.g =   (val * c1.g + (1.0 - val) * c0.g);
	    c.b =   (val * c1.b + (1.0 - val) * c0.b);
	    c.a =   (val * c1.a + (1.0 - val) * c0.a);
	    index = (val * i0   + (1.0 - val) * i1);

	    /* init for packed color */
#ifdef GL_EXT_abgr
	    if (pI->format != GL_ABGR_EXT) {
		*rS = c.r * 65535.0;
		*gS = c.g * 65535.0;
		*bS = c.b * 65535.0;
		*aS = c.a * 65535.0;
	    } else {
		*rS = c.a * 65535.0;
		*gS = c.b * 65535.0;
		*bS = c.g * 65535.0;
		*aS = c.r * 65535.0;
	    }
#else
	    *rS = c.r * 65535.0;
	    *gS = c.g * 65535.0;
	    *bS = c.b * 65535.0;
	    *aS = c.a * 65535.0;
#endif

	    switch(pI->type) {
	      case GL_BYTE:
		*rB = c.r * 128.0;
		*gB = c.g * 128.0;
		*bB = c.b * 128.0;
		*aB = c.a * 128.0;
		*iB = index;
		break;
	      case GL_UNSIGNED_BYTE:
		*rB = c.r * 255.0;
		*gB = c.g * 255.0;
		*bB = c.b * 255.0;
		*aB = c.a * 255.0;
		*iB = index;
		break;
	      case GL_SHORT:
		*rS = c.r * 32767.0;
		*gS = c.g * 32767.0;
		*bS = c.b * 32767.0;
		*aS = c.a * 32767.0;
		*iS = index;
		break;
	      case GL_UNSIGNED_SHORT:
		*rS = c.r * 65535.0;
		*gS = c.g * 65535.0;
		*bS = c.b * 65535.0;
		*aS = c.a * 65535.0;
		*iS = index;
		break;
	      case GL_INT:
		*rI = c.r * 2147483648.0;
		*gI = c.g * 2147483648.0;
		*bI = c.b * 2147483648.0;
		*aI = c.a * 2147483648.0;
		*iI = index;
		break;
	      case GL_UNSIGNED_INT:
		*rI = c.r * 4294967296.0;
		*gI = c.g * 4294967296.0;
		*bI = c.b * 4294967296.0;
		*aI = c.a * 4294967296.0;
		*iI = index;
		break;
	      case GL_FLOAT:
		*rF = c.r;
		*gF = c.g;
		*bF = c.b;
		*aF = c.a;
		*iF = index;
		break;
#ifdef GL_EXT_packed_pixels
	    case GL_UNSIGNED_BYTE_3_3_2_EXT:
		*((unsigned char *) & ColPack) = (unsigned char) (
		           ((*rS >> 13) << 5) |
			   ((*gS >> 13) << 2) |
			   ((*bS >> 14) << 0));
		pColPack = (GLbyte *) & ColPack;
		break;
	    case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
		*((unsigned short *) & ColPack) = (unsigned short) (
                           ((*rS >> 12) << 12) |
			   ((*gS >> 12) << 8)  |
			   ((*bS >> 12) << 4)  |
			   ((*aS >> 12) << 0));
		pColPack = (GLbyte *) & ColPack;
		break;
	    case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
		*((unsigned short *) & ColPack) = (unsigned short) (
		           ((*rS >> 11) << 11) |
			   ((*gS >> 11) << 6)  |
			   ((*bS >> 11) << 1)  |
			   ((*aS >> 15) << 0));
		pColPack = (GLbyte *) & ColPack;
		break;
	    case GL_UNSIGNED_INT_8_8_8_8_EXT:
		ColPack = (((*rS >> 8) << 24) |
			   ((*gS >> 8) << 16) |
			   ((*bS >> 8) << 8)  |
			   ((*aS >> 8) << 0));
		pColPack = (GLbyte *) & ColPack;
		break;	
	    case GL_UNSIGNED_INT_10_10_10_2_EXT:
		ColPack = (((*rS >> 6 ) << 22) |
			   ((*gS >> 6 ) << 12) |
			   ((*bS >> 6 ) << 2)  |
			   ((*aS >> 14) << 0));
		pColPack = (GLbyte *) & ColPack;
		break;
#endif
              default:
		/* should have caught this already... */
		assert(0);
		break;
	    }

	    if (IsPackedType(pI->type)) {
		bcopy(pColPack, pBuf, numByteInPix); pBuf += numByteInPix;
	    } else {		  
		switch(pI->format) {
		case GL_RGBA:
		    bcopy(&r, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&g, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&b, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&a, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
#ifdef GL_EXT_abgr
		case GL_ABGR_EXT:
		    bcopy(&a, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&b, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&r, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&g, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
#endif
		case GL_RGB:
		    bcopy(&r, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&g, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&b, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_RED:
		    bcopy(&r, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_GREEN:
		    bcopy(&g, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;		
		case GL_BLUE:
		    bcopy(&b, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_ALPHA:
		    bcopy(&a, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_DEPTH_COMPONENT:
		    bcopy(&r, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_LUMINANCE:
		    bcopy(&r, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		case GL_LUMINANCE_ALPHA:
		    bcopy(&r, pBuf, numByteInComp); pBuf += numByteInComp;
		    bcopy(&a, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;	
		case GL_STENCIL_INDEX:
		case GL_COLOR_INDEX:
		    bcopy(&i, pBuf, numByteInComp); pBuf += numByteInComp;
		    break;
		default:
		    ogEnvLog(OG_LINTERNALERROR,
                             "BuildRamp:  Format 0x%x not supported.\n", 
                             pI->format);
		}
	    }
	}
    }
}

GLenum
IU_TypeNameToEnum(char * pStr)
{
    if (!strcmp(pStr, "GL_BYTE"))
	return GL_BYTE;
    if (!strcmp(pStr, "GL_UNSIGNED_BYTE"))
	return GL_UNSIGNED_BYTE;
    if (!strcmp(pStr, "GL_SHORT"))
	return GL_SHORT;
    if (!strcmp(pStr, "GL_UNSIGNED_SHORT"))
	return GL_UNSIGNED_SHORT;
    if (!strcmp(pStr, "GL_INT"))
	return GL_INT;
    if (!strcmp(pStr, "GL_UNSIGNED_INT"))
	return GL_UNSIGNED_INT;
    if (!strcmp(pStr, "GL_FLOAT"))
	return GL_FLOAT;
    if (!strcmp(pStr, "GL_BITMAP"))
	return GL_BITMAP;
#ifdef GL_EXT_packed_pixels
    if (!strcmp(pStr, "GL_UNSIGNED_BYTE_3_3_2_EXT"))
	return GL_UNSIGNED_BYTE_3_3_2_EXT;
    if (!strcmp(pStr, "GL_UNSIGNED_SHORT_4_4_4_4_EXT"))
	return GL_UNSIGNED_SHORT_4_4_4_4_EXT;
    if (!strcmp(pStr, "GL_UNSIGNED_SHORT_5_5_5_1_EXT"))
	return GL_UNSIGNED_SHORT_5_5_5_1_EXT;
    if (!strcmp(pStr, "GL_UNSIGNED_INT_8_8_8_8_EXT"))
	return GL_UNSIGNED_INT_8_8_8_8_EXT;	
    if (!strcmp(pStr, "GL_UNSIGNED_INT_10_10_10_2_EXT"))
	return GL_UNSIGNED_INT_10_10_10_2_EXT;	
#endif
    return 0;
}


GLvoid
IU_InitBitmapToRGB(IU_FColor col0, IU_FColor col1)
{
    GLfloat itor[2], itog[2], itob[2], itoa[2];

    itor[0] = col0.r;
    itor[1] = col1.r;
    itog[0] = col0.g;
    itog[1] = col1.g;
    itob[0] = col0.b;
    itob[1] = col1.b;
    itoa[0] = col0.a;
    itoa[1] = col1.a;

    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 2, itor);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 2, itog);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 2, itob);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, itoa);
}

static GLfloat rgvals[8] = {
	0.0, 1.0/7.0, 2.0/7.0, 3.0/7.0, 4.0/7.0, 5.0/7.0, 6.0/7.0, 1.0
};

static GLfloat bvals[4] = {
	0.0, 1.0/3.0, 2.0/3.0, 1.0
};

GLvoid
IU_InitIndexToRGB(GLvoid)
{
    GLint ir, ig, ib;
    GLfloat itor[256], itog[256], itob[256], itoa[256];
    GLfloat *pr, *pg, *pb, *pa;

    pr = itor;
    pg = itog;
    pb = itob;
    pa = itoa;
    for (ib = 0; ib < 4; ib++) {
	for (ig = 0; ig < 8; ig++) {
	    for (ir = 0; ir < 8; ir++) {
		*pr++ = rgvals[ir];
		*pg++ = rgvals[ig];
		*pb++ = bvals[ib];
		*pa++ = 1.0;
	    }
	}
    }
    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 256, itor);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 256, itog);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 256, itob);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 256, itoa);
}


void 
IU_InitChecker(IU_FColor col0, IU_FColor col1, GLubyte i0, GLubyte i1,
	       GLint xMod, GLint yMod) {

    if (xMod)
	checker.xMod = xMod; 
    else
	checker.xMod = 8;

    if (yMod)
	checker.yMod=yMod;
    else
	checker.xMod = 8;

    checker.color0.fColor.r = col0.r;
    checker.color0.fColor.g = col0.g;
    checker.color0.fColor.b = col0.b;
    checker.color0.fColor.a = col0.a;

    checker.color1.fColor.r = col1.r;
    checker.color1.fColor.g = col1.g;
    checker.color1.fColor.b = col1.b;
    checker.color1.fColor.a = col1.a;

    checker.color0.bColor.r = IU_FLOAT_TO_B(col0.r);
    checker.color0.bColor.g = IU_FLOAT_TO_B(col0.g);
    checker.color0.bColor.b = IU_FLOAT_TO_B(col0.b);
    checker.color0.bColor.a = IU_FLOAT_TO_B(col0.a);

    checker.color1.bColor.r = IU_FLOAT_TO_B(col1.r);
    checker.color1.bColor.g = IU_FLOAT_TO_B(col1.g);
    checker.color1.bColor.b = IU_FLOAT_TO_B(col1.b);
    checker.color1.bColor.a = IU_FLOAT_TO_B(col1.a);

    checker.color0.ubColor.r = IU_FLOAT_TO_UB(col0.r);
    checker.color0.ubColor.g = IU_FLOAT_TO_UB(col0.g);
    checker.color0.ubColor.b = IU_FLOAT_TO_UB(col0.b);
    checker.color0.ubColor.a = IU_FLOAT_TO_UB(col0.a);

    checker.color1.ubColor.r = IU_FLOAT_TO_UB(col1.r);
    checker.color1.ubColor.g = IU_FLOAT_TO_UB(col1.g);
    checker.color1.ubColor.b = IU_FLOAT_TO_UB(col1.b);
    checker.color1.ubColor.a = IU_FLOAT_TO_UB(col1.a);

    checker.color0.sColor.r = IU_FLOAT_TO_S(col0.r);
    checker.color0.sColor.g = IU_FLOAT_TO_S(col0.g);
    checker.color0.sColor.b = IU_FLOAT_TO_S(col0.b);
    checker.color0.sColor.a = IU_FLOAT_TO_S(col0.a);

    checker.color1.sColor.r = IU_FLOAT_TO_S(col1.r);
    checker.color1.sColor.g = IU_FLOAT_TO_S(col1.g);
    checker.color1.sColor.b = IU_FLOAT_TO_S(col1.b);
    checker.color1.sColor.a = IU_FLOAT_TO_S(col1.a);

    checker.color0.usColor.r = IU_FLOAT_TO_US(col0.r);
    checker.color0.usColor.g = IU_FLOAT_TO_US(col0.g);
    checker.color0.usColor.b = IU_FLOAT_TO_US(col0.b);
    checker.color0.usColor.a = IU_FLOAT_TO_US(col0.a);

    checker.color1.usColor.r = IU_FLOAT_TO_US(col1.r);
    checker.color1.usColor.g = IU_FLOAT_TO_US(col1.g);
    checker.color1.usColor.b = IU_FLOAT_TO_US(col1.b);
    checker.color1.usColor.a = IU_FLOAT_TO_US(col1.a);

    checker.color0.iColor.r = IU_FLOAT_TO_I(col0.r);
    checker.color0.iColor.g = IU_FLOAT_TO_I(col0.g);
    checker.color0.iColor.b = IU_FLOAT_TO_I(col0.b);
    checker.color0.iColor.a = IU_FLOAT_TO_I(col0.a);

    checker.color1.iColor.r = IU_FLOAT_TO_I(col1.r);
    checker.color1.iColor.g = IU_FLOAT_TO_I(col1.g);
    checker.color1.iColor.b = IU_FLOAT_TO_I(col1.b);
    checker.color1.iColor.a = IU_FLOAT_TO_I(col1.a);

    checker.color0.uiColor.r = IU_FLOAT_TO_UI(col0.r);
    checker.color0.uiColor.g = IU_FLOAT_TO_UI(col0.g);
    checker.color0.uiColor.b = IU_FLOAT_TO_UI(col0.b);
    checker.color0.uiColor.a = IU_FLOAT_TO_UI(col0.a);

    checker.color1.uiColor.r = IU_FLOAT_TO_UI(col1.r);
    checker.color1.uiColor.g = IU_FLOAT_TO_UI(col1.g);
    checker.color1.uiColor.b = IU_FLOAT_TO_UI(col1.b);
    checker.color1.uiColor.a = IU_FLOAT_TO_UI(col1.a);

    checker.index0.fIndex = (GLfloat) i0;
    checker.index1.fIndex = (GLfloat) i1;

    checker.index0.bIndex = (GLbyte) (i0 >> 1);
    checker.index1.bIndex = (GLbyte) (i1 >> 1);

    checker.index0.ubIndex = i0;
    checker.index1.ubIndex = i1;

    checker.index0.sIndex = (GLshort) i0;
    checker.index1.sIndex = (GLshort) i1;

    checker.index0.usIndex = (GLushort) i0;
    checker.index1.usIndex = (GLushort) i1;

    checker.index0.iIndex = (GLint) i0;
    checker.index1.iIndex = (GLint) i1;

    checker.index0.uiIndex = (GLuint) i0;
    checker.index1.uiIndex = (GLuint) i1;

    if (verbose) {
	ogEnvLog(OG_LALWAYS, "InitCol: \n");
	ogEnvLog(OG_LALWAYS, "    Float Color 0 %f %f %f %f color 1 %f %f %f %f \n",
                 checker.color0.fColor.r,  checker.color0.fColor.g,
                 checker.color0.fColor.b,  checker.color0.fColor.a,
                 checker.color1.fColor.r,  checker.color1.fColor.g,
                 checker.color1.fColor.b,  checker.color1.fColor.a);
	ogEnvLog(OG_LALWAYS, "    Byte Color 0 %x %x %x %x color 1 %x %x %x %x \n",
                 checker.color0.bColor.r,  checker.color0.bColor.g,
                 checker.color0.bColor.b,  checker.color0.bColor.a,
                 checker.color1.bColor.r,  checker.color1.bColor.g,
                 checker.color1.bColor.b,  checker.color1.bColor.a);
	ogEnvLog(OG_LALWAYS, "    Unsigned Byte Color 0 %x %x %x %x color 1 %x %x %x %x \n",
                 checker.color0.ubColor.r,  checker.color0.ubColor.g,
                 checker.color0.ubColor.b,  checker.color0.ubColor.a,
                 checker.color1.ubColor.r,  checker.color1.ubColor.g,
                 checker.color1.ubColor.b,  checker.color1.ubColor.a);
	ogEnvLog(OG_LALWAYS, "    Short Color 0 %x %x %x %x color 1 %x %x %x %x \n",
                 checker.color0.sColor.r,  checker.color0.sColor.g,
                 checker.color0.sColor.b,  checker.color0.sColor.a,
                 checker.color1.sColor.r,  checker.color1.sColor.g,
                 checker.color1.sColor.b,  checker.color1.sColor.a);
	ogEnvLog(OG_LALWAYS, "    Unsigned Short Color 0 %x %x %x %x color 1 %x %x %x %x \n",
                 checker.color0.usColor.r,  checker.color0.usColor.g,
                 checker.color0.usColor.b,  checker.color0.usColor.a,
                 checker.color1.usColor.r,  checker.color1.usColor.g,
                 checker.color1.usColor.b,  checker.color1.usColor.a);
	ogEnvLog(OG_LALWAYS, "    Int Color 0 %x %x %x %x color 1 %x %x %x %x \n",
                 checker.color0.iColor.r,  checker.color0.iColor.g,
                 checker.color0.iColor.b,  checker.color0.iColor.a,
                 checker.color1.iColor.r,  checker.color1.iColor.g,
                 checker.color1.iColor.b,  checker.color1.iColor.a);
	ogEnvLog(OG_LALWAYS, "    Unsigned Int Color 0 %x %x %x %x color 1 %x %x %x %x \n",
                 checker.color0.uiColor.r,  checker.color0.uiColor.g,
                 checker.color0.uiColor.b,  checker.color0.uiColor.a,
                 checker.color1.uiColor.r,  checker.color1.uiColor.g,
                 checker.color1.uiColor.b,  checker.color1.uiColor.a);
	ogEnvLog(OG_LALWAYS, "    Float Index 0 %f index 1 %f\n",
                 checker.index0.fIndex, checker.index1.fIndex);
	ogEnvLog(OG_LALWAYS, "    Byte Index 0 %f index 1 %f\n",
                 checker.index0.bIndex, checker.index1.bIndex);
	ogEnvLog(OG_LALWAYS, "    Unsigned Byte Index 0 %f index 1 %f\n",
                 checker.index0.ubIndex, checker.index1.ubIndex);
	ogEnvLog(OG_LALWAYS, "    Short Index 0 %f index 1 %f\n",
                 checker.index0.sIndex, checker.index1.sIndex);
	ogEnvLog(OG_LALWAYS, "    Unsigned Short Index 0 %f index 1 %f\n",
                 checker.index0.usIndex, checker.index1.usIndex);
	ogEnvLog(OG_LALWAYS, "    Int Index 0 %f index 1 %f\n",
                 checker.index0.iIndex, checker.index1.iIndex);
	ogEnvLog(OG_LALWAYS, "    Unsigned Int Index 0 %f index 1 %f\n",
                 checker.index0.uiIndex, checker.index1.uiIndex);
    }
}

void
IU_InitRamp(IU_FColor col0, IU_FColor col1, GLubyte i0, GLubyte i1) {
    ramp.color0.r = col0.r;
    ramp.color0.g = col0.g;
    ramp.color0.b = col0.b;
    ramp.color0.a = col0.a;

    ramp.color1.r = col1.r;
    ramp.color1.g = col1.g;
    ramp.color1.b = col1.b;
    ramp.color1.a = col1.a;

    ramp.index0 = i0;
    ramp.index1 = i1;
}


void
IU_InitTile(IU_Rect rect, GLint numHorTile, GLint numVerTile) {

    if (rect.x2 > rect.x1) {
	tile.screen.x1 = rect.x1;
	tile.screen.x2 = rect.x2;
    } else {
	tile.screen.x1 = rect.x2;
	tile.screen.x2 = rect.x1;
    }
    if (rect.y2 > rect.y1) {
	tile.screen.y1 = rect.y1;
	tile.screen.y2 = rect.y2;
    } else {
	tile.screen.y1 = rect.y2;
	tile.screen.y2 = rect.y1;
    }

    tile.width = (rect.x2 - rect.x1) / numHorTile;
    tile.height = (rect.y2 - rect.y1) / numVerTile;

    tile.numHorTile = numHorTile;
    tile.numVerTile = numVerTile;
    tile.curXTile = 0; tile.curYTile = 0;
}

IU_Rect
IU_TileLocation(GLint x, GLint y) {
    IU_Rect rect;

    tile.curXTile = x;
    tile.curYTile = y;

    rect.x1 = tile.screen.x1 + x * tile.width;
    rect.x2 = tile.screen.x1 + x * tile.width + tile.width;
    rect.y1 = tile.screen.y1 + y * tile.height;
    rect.y2 = tile.screen.y1 + y * tile.height + tile.height;
    return rect;
}

IU_Rect
IU_NextTileLocation(void) {
    IU_Rect rect;

    rect = IU_TileLocation(tile.curXTile, tile.curYTile);
    if (++tile.curXTile >= tile.numHorTile) {
	tile.curXTile = 0;
	tile.curYTile++;
    }
    return rect;
}

/* ImageSize
 *     Given an image data structure, figure out
 *     the host image size. This function is typically called
 *     to obtain the size need for malloc.
 */
GLint 
IU_NumByteInImage(IU_Image *pI) {
    HostImage hostImage;
    
    HostFormat(pI, &hostImage);
    return hostImage.numByteInRect;
}

GLint
IU_BuildImage(IU_Image *pI, GLint imageType) {
    HostImage hostImage;

    switch(imageType) {
      case IU_CHECKER:
	HostFormat(pI, &hostImage);
	if (ParseInput(pI, &hostImage)) return GL_FALSE;
	if (pI->type == GL_BITMAP)
	    BuildCheckerBitmap(pI, &hostImage);
	else
	    BuildChecker(pI, &hostImage);

	return GL_TRUE;
      case IU_RAMP:
	HostFormat(pI, &hostImage);
	if (ParseInput(pI, &hostImage)) return GL_FALSE;
	if (pI->type == GL_BITMAP) {
	    ogEnvLog(OG_LALWAYS, "IU_BuildImage:  Bitmap ramps not supported.\n");
	    return GL_FALSE;
	} else 
	   BuildRamp(pI, &hostImage);
	return GL_TRUE;
      case IU_REFERENCE:
	pI->width  = 32;
	pI->height = 32;
	pI->format = GL_RGBA; 
	pI->type   = GL_UNSIGNED_BYTE; 
	pI->swap   = GL_FALSE;
	pI->alignment = 1;
	pI->pImage = (GLvoid *) refImage;

	return GL_TRUE;
      case IU_RANDOM:
	HostFormat(pI, &hostImage);
	if (ParseInput(pI, &hostImage)) return GL_FALSE;
	BuildRandom(pI, &hostImage);
	return GL_TRUE;
      default:
	ogEnvLog(OG_LALWAYS, "imageType not implemented yet. \n");
	return GL_FALSE;
    }
}


/* Given numByteInLineMod8, numInvalidByte, format and type, this function
 * figures out if there is a numPixInLine and numByteInLine that would
 * satisfy the constrain.
 */
GLint
IU_Validate_NumPixInLine(GLint baseNumByteInLine, GLint numByteInLineMod8, 
			 GLint numInvalidByte, 
			 GLenum format, GLenum type, 
			 GLint *pNumPixInLine, GLint *pNumByteInLine) {
    GLint  numCompInPix, numByteInType, numByteInPix;
    GLfloat y, fY;
    GLint iY, i;

    if (format == GL_BITMAP) {
	ogEnvLog(OG_LALWAYS,
                 "GL_BITMAP not supported for IU_Validate_NumPixInLine\n");
	return GL_FALSE;
    }
    numCompInPix  = IU_NumCompInPix(format);
    numByteInType = IU_NumByteInType(type);
    if (IsPackedType(type)) {
	numByteInPix = numByteInType;
    } else {
	numByteInPix = numCompInPix * numByteInType;
    }
    
    for (i=0; i< (numByteInPix+1); i++) {
	y = ((GLfloat) baseNumByteInLine + (GLfloat) numByteInLineMod8) /
	    (GLfloat) numByteInPix - 
		(GLfloat) numInvalidByte / (GLfloat) numByteInPix;
	iY = (GLint) y;
	fY = (GLfloat) iY;
	if (fY == y && iY > 0) {
	    /* found am integer y */
	    *pNumPixInLine = iY; 
	    *pNumByteInLine = baseNumByteInLine + numByteInLineMod8;
	    return GL_TRUE;
	}
	baseNumByteInLine += 8;
    }
    return GL_FALSE;
    
}

GLint
IU_Validate_Alignment(GLint numByteInLine, GLint numByteInRow, 
		      GLint alignment) {
    GLint numByteRemain, numInvalidByte;
    numByteRemain = numByteInRow % alignment;
    switch(alignment) {
      case 1:
	numInvalidByte = 0;
	break;
      case 2:
	numInvalidByte = (numByteRemain ? 2 - numByteRemain : 0);
	break;
      case 4:
	numInvalidByte = (numByteRemain ? 4 - numByteRemain : 0);
	break;
      case 8:
	numInvalidByte = (numByteRemain ? 8 - numByteRemain : 0);
	break;
      default:
	return GL_FALSE;
    }
    if ((numByteInRow + numInvalidByte) == numByteInLine)
	return GL_TRUE;
    else
	return GL_FALSE;
}

/* Turn verbose on or off */
GLvoid
IU_Verbose(GLint on) {
    verbose = on;
}

int refImage[] = {
0x7c7679ff,
0x7d797fff,
0xa79a9cff,
0xdddbd5ff,
0xa8c5d7ff,
0x2f527fff,
0x202d3fff,
0x272027ff,
0x2b2123ff,
0x4f3c33ff,
0x6d6860ff,
0x677782ff,
0x697a8eff,
0x607091ff,
0x657683ff,
0x62747aff,
0x546971ff,
0x3d5765ff,
0x4a5b60ff,
0x516570ff,
0x4b6072ff,
0x3e5c6bff,
0x122a3dff,
0x0f1117ff,
0x1b1c14ff,
0x1d1d17ff,
0x1c1f18ff,
0x1f2019ff,
0x23201dff,
0x252020ff,
0x434030ff,
0x3f4e41ff,
0x887882ff,
0x948593ff,
0xac9aa0ff,
0xc7c3ceff,
0xd1d5eaff,
0x5d71afff,
0x141b34ff,
0x23221eff,
0x271f1dff,
0x291e1dff,
0x4e4038ff,
0x736862ff,
0x5e6579ff,
0x5f6981ff,
0x57627cff,
0x576174ff,
0x546373ff,
0x42546bff,
0x4d5967ff,
0x4c5b6eff,
0x515a68ff,
0x32455dff,
0x091629ff,
0x1e1b14ff,
0x241f19ff,
0x1d1d1aff,
0x212018ff,
0x2b221bff,
0x2d221fff,
0x221d20ff,
0x39332cff,
0x444b45ff,
0xa48b8eff,
0xb49895ff,
0xc6a59bff,
0xddcecaff,
0xbac2ecff,
0x213e95ff,
0x1b1727ff,
0x25211fff,
0x201c1eff,
0x1b151eff,
0x30251fff,
0x5e503bff,
0x444d58ff,
0x57565eff,
0x595b6fff,
0x565c72ff,
0x49556eff,
0x3e4b66ff,
0x515666ff,
0x495469ff,
0x4c5769ff,
0x1b2943ff,
0x12121aff,
0x2b221bff,
0x201a19ff,
0x1a191aff,
0x1e1d19ff,
0x292017ff,
0x33271eff,
0x281d1eff,
0x342a25ff,
0x494a43ff,
0xa48585ff,
0xb29184ff,
0xca9f93ff,
0xded0c9ff,
0x889cd6ff,
0x18245eff,
0x32201fff,
0x302223ff,
0x211f21ff,
0x1b191fff,
0x161511ff,
0x493e23ff,
0x4a4846ff,
0x4f4947ff,
0x574f5aff,
0x575969ff,
0x4d576aff,
0x434e63ff,
0x475564ff,
0x445062ff,
0x374a6aff,
0x0d1f38ff,
0x261e12ff,
0x2c2223ff,
0x211d1eff,
0x191718ff,
0x1d1919ff,
0x252018ff,
0x30271aff,
0x2c1f1cff,
0x2f2420ff,
0x4a4a41ff,
0xb49891ff,
0xb29b99ff,
0xc5aaafff,
0xd7d2e7ff,
0x4d62aaff,
0x29223cff,
0x3d2a28ff,
0x2d2729ff,
0x1f1e22ff,
0x19191bff,
0x0c1219ff,
0x2e2619ff,
0x514633ff,
0x4d4a43ff,
0x4d4b51ff,
0x4e5155ff,
0x4d5359ff,
0x4c565eff,
0x485767ff,
0x374763ff,
0x2f3d5bff,
0x1f1d2cff,
0x4b2c1eff,
0x593332ff,
0x542e36ff,
0x42222bff,
0x241920ff,
0x241e1bff,
0x2e211aff,
0x331e20ff,
0x241e25ff,
0x454849ff,
0xbaa3a9ff,
0xbeadb4ff,
0xd5c1cbff,
0xb1b9e6ff,
0x2e3d7fff,
0x452c2dff,
0x3d282eff,
0x292224ff,
0x211b20ff,
0x16181dff,
0x091115ff,
0x222017ff,
0x4d412cff,
0x4f4b43ff,
0x555251ff,
0x5f5a51ff,
0x5d584eff,
0x5a595cff,
0x424f5fff,
0x3a475bff,
0x333d4fff,
0x433032ff,
0x723e35ff,
0x773e41ff,
0x6e3a40ff,
0x613536ff,
0x41262cff,
0x241e22ff,
0x2d201cff,
0x381f20ff,
0x281d26ff,
0x4a4a40ff,
0xc4a79dff,
0xcab3b7ff,
0xd4c7cfff,
0x97abe4ff,
0x3e3766ff,
0x51302eff,
0x5d3335ff,
0x5e2f36ff,
0x4b2d33ff,
0x312227ff,
0x17151aff,
0x1e1d1aff,
0x483c27ff,
0x564c3fff,
0x69594eff,
0x796858ff,
0x7f7064ff,
0x7f726dff,
0x796c63ff,
0x535561ff,
0x41424bff,
0x61403bff,
0x703d39ff,
0x5b333bff,
0x5c3136ff,
0x593335ff,
0x552e33ff,
0x312427ff,
0x291f1cff,
0x3a221eff,
0x2e1d28ff,
0x4e4c42ff,
0x8e7a6cff,
0xb09690ff,
0xe1d0c6ff,
0x84a0e1ff,
0x483a5dff,
0x5f3835ff,
0x6b393cff,
0x6b323cff,
0x5c2f3dff,
0x4f2c33ff,
0x39222aff,
0x2f2423ff,
0x6a5734ff,
0x846e58ff,
0x967b65ff,
0x977f70ff,
0x928073ff,
0x907f6eff,
0x94826bff,
0x726871ff,
0x5c4144ff,
0x784437ff,
0x824541ff,
0x643540ff,
0x582c37ff,
0x5a3035ff,
0x613233ff,
0x472d2cff,
0x2d211fff,
0x3c251eff,
0x2f1e27ff,
0x544e44ff,
0x8c7c72ff,
0x917f77ff,
0xd5b599ff,
0x8b85afff,
0x513d5eff,
0x5e3a3eff,
0x59323cff,
0x4a2538ff,
0x381c31ff,
0x402229ff,
0x472929ff,
0x312323ff,
0x836638ff,
0xa58c74ff,
0x98867cff,
0x97837bff,
0x988178ff,
0x978173ff,
0x8b7d70ff,
0x5d4d5eff,
0x6d423dff,
0x583434ff,
0x523038ff,
0x54333dff,
0x4a3238ff,
0x46292cff,
0x623334ff,
0x5f3734ff,
0x3e2826ff,
0x37231fff,
0x31261dff,
0x585340ff,
0x9b8781ff,
0x9d8c86ff,
0x988989ff,
0x715363ff,
0x6e434dff,
0x56303cff,
0x45262fff,
0x4b2435ff,
0x351a2dff,
0x2b1922ff,
0x462c23ff,
0x301f27ff,
0x523c2bff,
0xab8767ff,
0xa28879ff,
0x9e887bff,
0x9c8679ff,
0x9d8a79ff,
0x7b6e74ff,
0x624047ff,
0x6b3f3eff,
0x543032ff,
0x533435ff,
0x55323fff,
0x513036ff,
0x37262bff,
0x452b29ff,
0x61392fff,
0x483031ff,
0x281b22ff,
0x32291eff,
0x5d5130ff,
0xa18e88ff,
0xa8958cff,
0x7e7185ff,
0x6a4554ff,
0x754247ff,
0x713c47ff,
0x703d49ff,
0x54323cff,
0x432633ff,
0x372023ff,
0x462c26ff,
0x372429ff,
0x3b2b27ff,
0xb18558ff,
0xad8d7dff,
0xa58b7dff,
0xa38b79ff,
0xa38c7aff,
0x7a6873ff,
0x75424cff,
0x6d3941ff,
0x603238ff,
0x593536ff,
0x37242dff,
0x2e1f24ff,
0x402a29ff,
0x462b29ff,
0x5d332cff,
0x4b2f32ff,
0x372827ff,
0x55472bff,
0x5c4b2fff,
0xab9989ff,
0xa69390ff,
0x735a74ff,
0x73444dff,
0x794149ff,
0x6d384aff,
0x563141ff,
0x452b33ff,
0x352129ff,
0x462823ff,
0x59302eff,
0x38252eff,
0x30201eff,
0xab7b4aff,
0xb39781ff,
0xa89080ff,
0xaa9181ff,
0xa58c7dff,
0x805e66ff,
0x89484eff,
0x7f3c4aff,
0x612e3fff,
0x622f37ff,
0x65303bff,
0x492233ff,
0x3c2427ff,
0x4c2b28ff,
0x60312dff,
0x512f32ff,
0x50372dff,
0x715a37ff,
0x5c4932ff,
0xb89f95ff,
0x988494ff,
0x71495fff,
0x874a4fff,
0x8b4b52ff,
0x743d4cff,
0x763b42ff,
0x5e2d44ff,
0x3e2030ff,
0x462626ff,
0x653533ff,
0x4a2937ff,
0x221820ff,
0x75512eff,
0xba9678ff,
0xb29882ff,
0xb09580ff,
0x8b787bff,
0x774a56ff,
0xa2554aff,
0xbb5757ff,
0x8b3c50ff,
0xab4b4fff,
0xbf535dff,
0x883955ff,
0x5b2c3bff,
0x653331ff,
0x7f3d3bff,
0x6e3439ff,
0x523431ff,
0x6c5037ff,
0x614f2aff,
0xbfaaa0ff,
0x8c7b99ff,
0x7f4c58ff,
0x965655ff,
0x98545cff,
0x934c56ff,
0xa75557ff,
0x964b66ff,
0x4e293fff,
0x703a35ff,
0x7e4140ff,
0x5c313fff,
0x201423ff,
0x4e381cff,
0xc1956dff,
0xb49883ff,
0xb39a84ff,
0x6a5878ff,
0x824647ff,
0xb45f54ff,
0xbe5f62ff,
0xbe5a62ff,
0xb4555cff,
0xb4505eff,
0x7c3251ff,
0x773a36ff,
0x9b4b48ff,
0x9c494bff,
0x843c45ff,
0x553036ff,
0x5c362dff,
0x69472aff,
0xbaa4a4ff,
0x6b6294ff,
0x905359ff,
0x9d5b5aff,
0x7c4a55ff,
0x79444eff,
0xa45551ff,
0x924b5bff,
0x59313fff,
0x7a443eff,
0x774248ff,
0x60353fff,
0x241828ff,
0x493522ff,
0xbe926eff,
0xc09f88ff,
0x9f8b92ff,
0x493a6bff,
0x874b3dff,
0xa35a54ff,
0x844c51ff,
0x894b4dff,
0xa85451ff,
0xb0505aff,
0x75334cff,
0x863f39ff,
0x8b4647ff,
0x854947ff,
0x814841ff,
0x563538ff,
0x513329ff,
0x69452cff,
0xaa969dff,
0x434069ff,
0x804c4aff,
0x603e45ff,
0x302634ff,
0x3e292fff,
0x7d453aff,
0x85454cff,
0x5b343bff,
0x4c2f36ff,
0x3e2734ff,
0x40272bff,
0x261d27ff,
0x26201aff,
0xb98b60ff,
0xd0aa86ff,
0x82768cff,
0x302651ff,
0x72422bff,
0x5c373bff,
0x452d36ff,
0x3f2a2aff,
0x703f2fff,
0xab5550ff,
0x6a3249ff,
0x582d35ff,
0x502f32ff,
0x503531ff,
0x613f33ff,
0x543535ff,
0x432c27ff,
0x68432aff,
0x91899eff,
0x1e2a57ff,
0x5d4038ff,
0x4c3439ff,
0x402a30ff,
0x3e292eff,
0x4e322eff,
0x522d38ff,
0x462a2eff,
0x2e2129ff,
0x1c1a22ff,
0x211d1cff,
0x1b181aff,
0x0f1511ff,
0xb98551ff,
0xdbaa7cff,
0x8a7f8dff,
0x2d2e56ff,
0x693d2aff,
0x342331ff,
0x0d1019ff,
0x161713ff,
0x4c301fff,
0x894c3aff,
0x522a40ff,
0x261b25ff,
0x161619ff,
0x211d15ff,
0x472e23ff,
0x492e2bff,
0x2f1e23ff,
0x5e3d24ff,
0x415699ff,
0x07122dff,
0x573a21ff,
0x31272cff,
0x241d22ff,
0x2d231cff,
0x55332aff,
0x5c2e3cff,
0x2e1d30ff,
0x322429ff,
0x302426ff,
0x14141cff,
0x050a14ff,
0x161609ff,
0xbf8c4fff,
0xd7ac83ff,
0x948c95ff,
0x1c2846ff,
0x784823ff,
0x6e3e38ff,
0x48292dff,
0x4a2824ff,
0x69352aff,
0x7c3c39ff,
0x63323eff,
0x301e26ff,
0x1d171aff,
0x2c2019ff,
0x3e2a1dff,
0x3e2a2bff,
0x1e1919ff,
0x554018ff,
0x0e246dff,
0x000a17ff,
0x453219ff,
0x5d3c2aff,
0x603a3fff,
0x432d3eff,
0x2a262aff,
0x60372eff,
0x472c35ff,
0x0e1322ff,
0x050f15ff,
0x060b0dff,
0x00080bff,
0x000803ff,
0x98752fff,
0xdfb388ff,
0x727594ff,
0x011538ff,
0x76431eff,
0xaa544aff,
0xb15654ff,
0xb75558ff,
0xb8555aff,
0xb6595dff,
0xa54f5dff,
0x88414eff,
0x6d363fff,
0x5f3233ff,
0x54302cff,
0x271b27ff,
0x101513ff,
0x53441cff,
0x061d59ff,
0x00080eff,
0x071311ff,
0x241e14ff,
0x352726ff,
0x101523ff,
0x000911ff,
0x181711ff,
0x261e25ff,
0x0a0d17ff,
0x00080bff,
0x010908ff,
0x010908ff,
0x000305ff,
0x6c571bff,
0xb99977ff,
0x424f6eff,
0x000d1fff,
0x6c3e1eff,
0xa65549ff,
0xb75d56ff,
0xc9625dff,
0xcd6464ff,
0xc86569ff,
0xbb5d64ff,
0xa35157ff,
0x92454dff,
0x7f3c3eff,
0x592e32ff,
0x121120ff,
0x020b0bff,
0x4b3c18ff,
0x091e59ff,
0x000d0fff,
0x000d10ff,
0x00060bff,
0x000809ff,
0x00090dff,
0x02080eff,
0x000809ff,
0x00080eff,
0x000709ff,
0x000807ff,
0x000808ff,
0x000808ff,
0x000709ff,
0x675222ff,
0x786651ff,
0x051a41ff,
0x030f09ff,
0x5d381eff,
0x9d5243ff,
0xaf5951ff,
0xbf615cff,
0xc96763ff,
0xc86767ff,
0xb65c60ff,
0x9d4d4eff,
0x894145ff,
0x7c3c3bff,
0x512932ff,
0x0c101dff,
0x000706ff,
0x372e0dff,
0x0b2261ff,
0x000e16ff,
0x010f11ff,
0x010e11ff,
0x010f0dff,
0x010e08ff,
0x000808ff,
0x010808ff,
0x010808ff,
0x000808ff,
0x000808ff,
0x010808ff,
0x000809ff,
0x031006ff,
0xba8c4cff,
0xc1a58fff,
0x09194eff,
0x000603ff,
0x402814ff,
0x8d4836ff,
0xb4574aff,
0xc6635bff,
0xd46b66ff,
0xcf6b70ff,
0xb55b63ff,
0x9f4e51ff,
0x894048ff,
0x76393cff,
0x4b2b37ff,
0x060c1cff,
0x000607ff,
0x25230aff,
0x2b4281ff,
0x000f2eff,
0x01100fff,
0x000e0fff,
0x000c0aff,
0x00090aff,
0x00080cff,
0x000807ff,
0x000808ff,
0x000a08ff,
0x000a08ff,
0x020908ff,
0x000b0aff,
0x1e1d04ff,
0xca9c63ff,
0xb9a5a0ff,
0x061854ff,
0x000403ff,
0x1b160fff,
0x653821ff,
0xac5944ff,
0xcf695fff,
0xce6a66ff,
0xc16468ff,
0xb75b5dff,
0xa75359ff,
0x85424bff,
0x693538ff,
0x372129ff,
0x000713ff,
0x000708ff,
0x1f1f0bff,
0x797792ff,
0x05194dff,
0x000a0fff,
0x010a10ff,
0x00090aff,
0x000709ff,
0x00080aff,
0x000808ff,
0x000708ff,
0x000a08ff,
0x010b08ff,
0x010a09ff,
0x000806ff,
0x56410cff,
0xd6ac77ff,
0xc6b0a3ff,
0x203760ff,
0x00050fff,
0x020a0cff,
0x1c1611ff,
0x553329ff,
0x73423fff,
0x663e40ff,
0x5a3a38ff,
0x6a433cff,
0x683c40ff,
0x532f36ff,
0x382326ff,
0x161216ff,
0x00070bff,
0x000708ff,
0x2a2708ff,
0xb7a39eff,
0x435685ff,
0x000127ff,
0x03080aff,
0x020a09ff,
0x000809ff,
0x000808ff,
0x000908ff,
0x010a09ff,
0x020a0aff,
0x000a08ff,
0x000d08ff,
0x2c2a12ff,
0xb88d4bff,
0xd4b898ff,
0xd3b89eff,
0x6a6e84ff,
0x000934ff,
0x030809ff,
0x000809ff,
0x000b09ff,
0x050f0aff,
0x08100cff,
0x0a110fff,
0x0d1211ff,
0x0a1110ff,
0x081011ff,
0x010b0bff,
0x000a07ff,
0x000608ff,
0x121703ff,
0x574311ff,
0xbaaa9eff,
0xb9afa2ff,
0x4c5986ff,
0x00062fff,
0x00010aff,
0x010805ff,
0x020909ff,
0x030e09ff,
0x000d0dff,
0x000306ff,
0x080c03ff,
0x483d1eff,
0xae884dff,
0xdfb795ff,
0xcbb3a6ff,
0xc9b5a2ff,
0xc2b09fff,
0x3e4d75ff,
0x000622ff,
0x010707ff,
0x030a08ff,
0x010908ff,
0x000808ff,
0x000709ff,
0x00070cff,
0x000708ff,
0x000808ff,
0x020908ff,
0x000b06ff,
0x0c1307ff,
0x624e29ff,
0x6d5436ff,
0xbea999ff,
0xd0b6a4ff,
0xc7b5b1ff,
0x707395ff,
0x1f2d52ff,
0x00111eff,
0x000610ff,
0x000508ff,
0x0f1407ff,
0x3f3418ff,
0x706045ff,
0xb19776ff,
0xc2a889ff,
0xbeb193ff,
0xb8a99eff,
0xbca99cff,
0xd0b6a0ff,
0xc0a69cff,
0x575e80ff,
0x021533ff,
0x00030aff,
0x00060aff,
0x030908ff,
0x030a08ff,
0x03090aff,
0x030a09ff,
0x02090aff,
0x020c09ff,
0x3a3413ff,
0x8a6f45ff,
0x9c8165ff,
0x6c5744ff,
0xc7af9eff,
0xceb5adff,
0xceb6b2ff,
0xd9bcadff,
0xc6aea7ff,
0xa29095ff,
0x8a7b85ff,
0x756c6fff,
0x978679ff,
0xccaf93ff,
0xdec0a6ff,
0xddc2b0ff,
0xd7c1b4ff,
0xd3c2acff,
0xd5bfadff,
0xd1bbabff,
0xd0b9a8ff,
0xd1b5a3ff,
0xc9b0a3ff,
0x998a93ff,
0x494c63ff,
0x0b1c31ff,
0x000412ff,
0x000106ff,
0x000505ff,
0x000506ff,
0x000501ff,
0x2d2a0eff,
0x87714bff,
0xb3986dff,
0x958369ff,
0x6d5b49ff,
0xd0b5a4ff,
0xd0b9b2ff,
0xcfb5b4ff,
0xd3baabff,
0xddbdacff,
0xe6c1b2ff,
0xe3c9bdff,
0xecc3b8ff,
0xdec2b7ff,
0xdbc4b6ff,
0xd9beb2ff,
0xdfc3b0ff,
0xdfcab9ff,
0xe2cbbcff,
0xe0c8baff,
0xdcc3b4ff,
0xdbbfa9ff,
0xdec0a5ff,
0xdac1aaff,
0xd9bfa8ff,
0xd5bba9ff,
0xad9c96ff,
0x7b7176ff,
0x4d4d56ff,
0x343d3aff,
0x3e3f38ff,
0x59533bff,
0xa1885dff,
0x9c8f7dff,
0xac956dff,
0x98896dff,
0x6c5b4aff,
0xd1b5a9ff,
0xd1c0b0ff,
0xd6c0b1ff,
0xd4bdacff,
0xd5b9acff,
0xe2c0b3ff,
0xdbc6bbff,
0xdabcacff,
0xe0bfaaff,
0xd5bdb2ff,
0xd0bfb3ff,
0xe0c9b4ff,
0xdfcdbdff,
0xe0d1c0ff,
0xe0cfc6ff,
0xe0c7bcff,
0xe1c3b1ff,
0xe1c8b6ff,
0xe0ccbfff,
0xdcc4b1ff,
0xd7c0afff,
0xd3b49fff,
0xd9b599ff,
0xd5b59bff,
0xccac95ff,
0xc5ac8dff,
0xc1a889ff,
0xc0a68dff,
0xa39183ff,
0xaf976eff,
0x9b886dff,
0x6c5b4fff,
0xceb6a9ff,
0xd0b9b0ff,
0xd7bfb7ff,
0xd8c0b7ff,
0xdcc1b3ff,
0xe2c5baff,
0xdec5c1ff,
0xe0c1bcff,
0xc9bab7ff,
0xc5b8b1ff,
0xdecab8ff,
0xe4cfc0ff,
0xe2cdc3ff,
0xe3d1c4ff,
0xe5d9d0ff,
0xe2dbd6ff,
0xe2d8d4ff,
0xe5dcd4ff,
0xe0dbd9ff,
0xe1d5d7ff,
0xded0c8ff,
0xd4c0afff,
0xd2b9a5ff,
0xd5b69fff,
0xcaae99ff,
0xc6ac8fff,
0xc4a78cff,
0xc1a889ff,
0xac9785ff,
0xae9675ff,
0x9b886cff,
0x6e5e4fff,
0xceb6aaff,
0xd0b8afff,
0xd2bcb3ff,
0xd8c0b2ff,
0xdbbfb4ff,
0xe0c2baff,
0xdec6c0ff,
0xe3c1b9ff,
0xc9b9baff,
0xd9c6baff,
0xe6d0bbff,
0xe6d6ccff,
0xe9dedfff,
0xe7e6e7ff,
0xe0ebeaff,
0xdceef1ff,
0xdbf0f6ff,
0xdbf1f7ff,
0xdbf0f6ff,
0xdbedf4ff,
0xdce8eaff,
0xd9dcdbff,
0xd8cfc8ff,
0xd7bfacff,
0xccb29bff,
0xc8af91ff,
0xc7ac8fff,
0xc2a98aff,
0xaf9b80ff,
0xae9578ff,
0x9b886eff,
0x6e604dff
};


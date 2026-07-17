/*
 *  tdRGB.c
 *                                                     04/26/95  1.00  NDR
 *                                                     09/01/95  4.00  NDR
 *
 *    Three-D Object Library - Library of 3-D object
 *    manipulation routines.
 *
 *    Iris RGB Texture Image Module
 *
 *    author: Nathan Drew Robins
 *    email: narobins@es.com
 *
 *    Originally Part of the SGI Sample-Implementation Code (libaux)
 *    which is probably copyrighted by SGI, but there was not copyright
 *    in the code.  Code that I added uses the following copyright:
 *
 *
 *    Copyright 1996 by Evans & Sutherland Computer Corporation.  
 *    ALL RIGHTS RESERVED. 
 *    
 *    Permission to use, copy, modify, and distribute this software 
 *    for any purpose and without fee is hereby granted, provided 
 *    that the above copyright notice appear in all copies and that 
 *    both the copyright notice and this notice appear in supporting
 *    documentation.  You may not use the name "Evans & Sutherland 
 *    Computer Corporation" or any trademark of Evans & Sutherland 
 *    Computer Corporation in advertising or publicity pertaining 
 *    to the software without specific, written prior permission. 
 *    
 *    THIS SOFTWARE AND ANY ASSOCIATED MATERIALS IS PROVIDED TO YOU 
 *    "AS-IS" AND WITHOUT REPRESENTATION OR WARRANTY OF ANY KIND, 
 *    EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, 
 *    ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *    PURPOSE OR NONINFRINGEMENT OF ANY PATENT, COPYRIGHT, TRADE 
 *    SECRET OR OTHER RIGHT.  EVANS & SUTHERLAND COMPUTER CORPORATION 
 *    DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE UNINTERRUPTED
 *    OR ERROR-FREE, OR THAT DEFECTS IN THE SOFTWARE WILL BE CORRECTED.
 *    
 *    IN NO EVENT SHALL EVANS & SUTHERLAND COMPUTER CORPORATION BE 
 *    LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, 
 *    INDIRECT OR CONSEQUENTIAL  DAMAGES OF ANY KIND, OR ANY DAMAGES 
 *    WHATSOEVER, INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS 
 *    OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, 
 *    WHETHER OR NOT EVANS & SUTHERLAND COMPUTER CORPORATION  HAS 
 *    BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED 
 *    AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION 
 *    WITH THE POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *    
 *    US Government Users Restricted Rights 
 *    Use, duplication, or disclosure by the Government is subject 
 *    to restricted rights set forth in FAR 52.227.19(c)(2) or 
 *    subparagraph (c)(1)(ii) of the Rights in Technical Data and 
 *    Computer Software clause at DFARS 252.227-7013 and/or in 
 *    similar or successor clauses in the FAR or the DOD or NASA FAR 
 *    Supplement. Contractor/manufacturer is Evans & Sutherland Computer 
 *    Corporation, 600 Komas Drive, Salt Lake City, UT 84108. 
 *    
 *    Use of this software outside the United States is subject to 
 *    US export laws and regulations.  You agree to comply with all 
 *    such laws and regulations.
 *    
 *    This statement is made under and shall be construed and 
 *    governed by the laws of the State of Utah, without regard to 
 *    conflict of laws principles.
 * 
 */


/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tdPrivate.h"


/* typedefs */
typedef struct _rawImageRec {
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short sizeX, sizeY, sizeZ;
    unsigned long min, max;
    unsigned long wasteBytes;
    char name[80];
    unsigned long colorMap;
    FILE *file;
    unsigned char *tmp, *tmpR, *tmpG, *tmpB;
    unsigned long rleEnd;
    GLuint *rowStart;
    GLint *rowSize;
} rawImageRec;

static void ConvertShort(unsigned short *array, long length)
{
    unsigned long b1, b2;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
	b1 = *ptr++;
	b2 = *ptr++;
	*array++ = (b1 << 8) | (b2);
    }
}

static void ConvertLong(GLuint *array, long length)
{
    unsigned long b1, b2, b3, b4;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
	b1 = *ptr++;
	b2 = *ptr++;
	b3 = *ptr++;
	b4 = *ptr++;
	*array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
    }
}

static rawImageRec *RawImageOpen(char *fileName)
{
    union {
	int testWord;
	char testByte[4];
    } endianTest;
    rawImageRec *raw;
    GLenum swapFlag;
    int x;

    endianTest.testWord = 1;
    if (endianTest.testByte[0] == 1) {
	swapFlag = GL_TRUE;
    } else {
	swapFlag = GL_FALSE;
    }

    raw = (rawImageRec *)malloc(sizeof(rawImageRec));
    if (raw == NULL) {
	return NULL;
    }
    if ((raw->file = fopen(fileName, "rb")) == NULL) {
	return NULL;
    }

    fread(raw, 1, 12, raw->file);

    if (swapFlag) {
	ConvertShort(&raw->imagic, 6);
    }

    raw->tmp = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpR = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpG = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpB = (unsigned char *)malloc(raw->sizeX*256);
    if (raw->tmp == NULL || raw->tmpR == NULL || raw->tmpG == NULL ||
	raw->tmpB == NULL) {
	return NULL;
    }

    if ((raw->type & 0xFF00) == 0x0100) {
	x = raw->sizeY * raw->sizeZ * sizeof(GLuint);
	raw->rowStart = (GLuint *)malloc(x);
	raw->rowSize = (GLint *)malloc(x);
	if (raw->rowStart == NULL || raw->rowSize == NULL) {
	    return NULL;
	}
	raw->rleEnd = 512 + (2 * x);
	fseek(raw->file, 512, SEEK_SET);
	fread(raw->rowStart, 1, x, raw->file);
	fread(raw->rowSize, 1, x, raw->file);
	if (swapFlag) {
	    ConvertLong(raw->rowStart, x/sizeof(GLuint));
	    ConvertLong((GLuint *)raw->rowSize, x/sizeof(GLint));
	}
    }
    return raw;
}

static void RawImageClose(rawImageRec *raw)
{

    fclose(raw->file);
    free(raw->tmp);
    free(raw->tmpR);
    free(raw->tmpG);
    free(raw->tmpB);

    /* added these because they are a 
     * memory leak -- small, but still
     * a leak.
     */
/*
    free(raw->rowSize);
    free(raw->rowStart);
*/

    free(raw);
}

static void RawImageGetRow(rawImageRec *raw, unsigned char *buf, int y, int z)
{
    unsigned char *iPtr, *oPtr, pixel;
    int count;

    if ((raw->type & 0xFF00) == 0x0100) {
	fseek(raw->file, raw->rowStart[y+z*raw->sizeY], SEEK_SET);
	fread(raw->tmp, 1, (unsigned int)raw->rowSize[y+z*raw->sizeY],
	      raw->file);

	iPtr = raw->tmp;
	oPtr = buf;
	while (1) {
	    pixel = *iPtr++;
	    count = (int)(pixel & 0x7F);
	    if (!count) {
		return;
	    }
	    if (pixel & 0x80) {
		while (count--) {
		    *oPtr++ = *iPtr++;
		}
	    } else {
		pixel = *iPtr++;
		while (count--) {
		    *oPtr++ = pixel;
		}
	    }
	}
    } else {
	fseek(raw->file, 512+(y*raw->sizeX)+(z*raw->sizeX*raw->sizeY),
	      SEEK_SET);
	fread(buf, 1, raw->sizeX, raw->file);
    }
}

static void RawImageGetData(rawImageRec *raw, TDteximage *final)
{
    unsigned char *ptr;
    int i, j;

    final->data = (unsigned char *)malloc((raw->sizeX+1)*(raw->sizeY+1)*4);
    if (final->data == NULL) {
	return;
    }

    ptr = final->data;
    for (i = 0; i < raw->sizeY; i++) {
	RawImageGetRow(raw, raw->tmpR, i, 0);
	RawImageGetRow(raw, raw->tmpG, i, 1);
	RawImageGetRow(raw, raw->tmpB, i, 2);
	for (j = 0; j < raw->sizeX; j++) {
	    *ptr++ = *(raw->tmpR + j);
	    *ptr++ = *(raw->tmpG + j);
	    *ptr++ = *(raw->tmpB + j);
	}
    }
}

/*
 *  _tdReadRGBTeximage()
 *
 *    Reads a texture file in the Iris RGB (.rgb) file format.
 *
 *    filename - name of the file containing the texture data of the
 *    form path + name + ".rgb" extension. 
 *
 */
TDteximage *_tdReadRGBTeximage(char *filename)
{
    rawImageRec *raw;
    TDteximage *final;

    _tdPrintf(TD_INFO, "reading texture image file `%s'", filename);
    raw = RawImageOpen(filename);
    if(!raw)
    {
	_tdPrintf(TD_ERROR, "can't open image file `%s'", filename);
	return(NULL);
    }
    final = (TDteximage *)malloc(sizeof(TDteximage));
    if (final == NULL) 
    {
	_tdPrintf(TD_NOMEMORY, "malloc() failed for texture image");
	return(NULL);
    }
    final->width = raw->sizeX;
    final->height = raw->sizeY;
    RawImageGetData(raw, final);
    RawImageClose(raw);
    return final;
}

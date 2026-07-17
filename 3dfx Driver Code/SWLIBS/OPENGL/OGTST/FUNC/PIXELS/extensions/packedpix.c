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

/* packedpix.c - $Revision: 2$ */

/*
 * This module tests the packed pixels extension.
 * It runs through type = 
 * 			GL_FLOAT, 
 * 			GL_UNSIGNED_BYTE_3_3_2_EXT,
 * 			GL_UNSIGNED_SHORT_4_4_4_4_EXT,
 * 			GL_UNSIGNED_SHORT_5_5_5_1_EXT,
 * 			GL_UNSIGNED_INT_8_8_8_8_EXT,
 * 			GL_UNSIGNED_INT_10_10_10_2_EXT
 * for 
 * 			ReadPixels,
 * 			DrawPixels,
 * 			TexImage2D,
 * 			GetTexImage
 */
#include <stdlib.h>
#include <stdio.h>

#include "ogtst.h"
#include "imgutil.h"

#ifdef GL_EXT_packed_pixels

static int winWidth, winHeight;

static ogImgImageRec byteImage;
static ogImgNewImageRec imageData;
static ogImgNewImageRec textureData;
void *tempbuf;

/*----------------------------------------------------------------------*/

static void InitRampedImage(GLubyte *imageBuf, int width, int height)
{
    int i,j;
    GLubyte *pBuf;

    pBuf = imageBuf;
    for (i=0; i < width; i++) {
	for (j=0; j < height; j++) {
	    *pBuf++ = 255 * (float)i/width;
	    *pBuf++ = 255 * (1-(float)j/height);
	    *pBuf++ = 255 * (1-(float)i/width);
	}
    }
}

static unsigned int maxTexSize(unsigned int value)
{
    int power;

    if (value == 0) return value;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &power);
    if (value < power) {
	while(!(power & value)) power >>= 1;
    }
    return power;
}

/*----------------------------------------------------------------------*/

typedef struct cellStruct {
	int numRows;
	int numCols;
	int xoffset;
	int yoffset;
	int cellSizeX;
	int cellSizeY;
	int row;
	int col;
	int x;
	int y;
} cellStruct;

static cellStruct cell = {4,6, 0,0, 0,0, 0,0, 0,0};

static void updateXY(cellStruct *cell)
{
    cell->x = cell->xoffset + cell->col * cell->cellSizeX;
    cell->y = cell->yoffset + cell->row * cell->cellSizeY;
}

static void setCell(cellStruct *cell, int col, int row)
{
    cell->col = col; cell->row = row;
    updateXY(cell);
}

static void nextCell(cellStruct *cell)
{
    cell->col ++;
    if (cell->col == cell->numCols) {
	cell->col = 0;
	cell->row ++;
	if (cell->row == cell->numRows) {
	    cell->row = 0;
	}
    }
    updateXY(cell);
}

/*----------------------------------------------------------------------*/

static void InitPackedPix(void)
{
    GLfloat quant4[4];
    int i;


    winWidth = ogEnvQuery(OG_XWSIZE);
    winHeight = ogEnvQuery(OG_YWSIZE);

    byteImage.sizeX = (winWidth-4*(cell.numCols-1))/cell.numCols;
    byteImage.sizeY = (winHeight-4*(cell.numRows-1))/cell.numRows;
    if (byteImage.sizeX < byteImage.sizeY) {
	cell.yoffset = (byteImage.sizeY-byteImage.sizeX)*cell.numRows/2;
	byteImage.sizeY = byteImage.sizeX;
    }
    else {
	cell.xoffset = (byteImage.sizeX-byteImage.sizeY)*cell.numCols/2;
	byteImage.sizeX = byteImage.sizeY;
    }
    byteImage.data = (unsigned char *)
			ogLibMalloc(byteImage.sizeX*byteImage.sizeY*3);

    InitRampedImage(byteImage.data, byteImage.sizeX, byteImage.sizeY);

    cell.cellSizeX = byteImage.sizeX + 4;
    cell.cellSizeY = byteImage.sizeY + 4;

    imageData.imageWidth = byteImage.sizeX;
    imageData.imageHeight = byteImage.sizeY;
    imageData.swapBytes = GL_FALSE;
    imageData.alignShift = GL_FALSE;
    imageData.type = GL_FLOAT;
    imageData.format = GL_RGBA;
    imageData.components = 4;

    ogImgCreateFloatImage(&byteImage, &imageData);
    ogImgCreateNewImage(&imageData);

    textureData.imageWidth = maxTexSize((unsigned int) imageData.imageWidth);
    textureData.imageHeight = maxTexSize((unsigned int) imageData.imageHeight);
    textureData.swapBytes = GL_FALSE;
    textureData.alignShift = GL_FALSE;
    textureData.type = GL_FLOAT;
    textureData.format = GL_RGBA;
    textureData.components = 4;

    byteImage.sizeX = textureData.imageWidth;
    byteImage.sizeY = textureData.imageHeight;
    InitRampedImage(byteImage.data, byteImage.sizeX, byteImage.sizeY);

    ogImgCreateFloatImage(&byteImage, &textureData);
    ogImgCreateNewImage(&textureData);

    tempbuf = ogLibMalloc(imageData.imageSize+3);

    for (i=0; i<4; i++) {
	quant4[i] = (2*i+1)/8.0;
    }

    glPixelMapfv(GL_PIXEL_MAP_R_TO_R, 4, quant4);
    glPixelMapfv(GL_PIXEL_MAP_G_TO_G, 4, quant4);
    glPixelMapfv(GL_PIXEL_MAP_B_TO_B, 4, quant4);
    glPixelMapfv(GL_PIXEL_MAP_A_TO_A, 4, quant4);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
    glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_PACK_LSB_FIRST, GL_TRUE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, imageData.imageWidth);
    glPixelStorei(GL_PACK_ROW_LENGTH, imageData.imageWidth);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, winWidth, 0, winHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

/*----------------------------------------------------------------------*/

static int numTypes = 6;

static void nextType(ogImgNewImageRec *imageData)
{
    switch(imageData->type) {
      case GL_FLOAT:
	imageData->type = GL_UNSIGNED_BYTE_3_3_2_EXT;
	imageData->format = GL_RGB;
	imageData->components = 3;
	break;
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
	imageData->type = GL_UNSIGNED_SHORT_4_4_4_4_EXT;
	imageData->format = GL_RGBA;
	imageData->components = 4;
	break;
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
	imageData->type = GL_UNSIGNED_SHORT_5_5_5_1_EXT;
	imageData->format = GL_RGBA;
	imageData->components = 4;
	break;
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
	imageData->type = GL_UNSIGNED_INT_8_8_8_8_EXT;
	imageData->format = GL_RGBA;
	imageData->components = 4;
	break;
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
	imageData->type = GL_UNSIGNED_INT_10_10_10_2_EXT;
	imageData->format = GL_RGBA;
	imageData->components = 4;
	break;
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
	imageData->type = GL_FLOAT;
	imageData->format = GL_RGBA;
	imageData->components = 4;
	break;
    }
}

/*----------------------------------------------------------------------*/

static void DrawPackedPix(void)
{
    int i;

    /*
    ** No display lists are used because ReadPixels is not display listable.
    */

    glClear(GL_COLOR_BUFFER_BIT);

    setCell(&cell, 0,0);

    /* test DrawPixels */

    for (i=0; i < numTypes; i++) {
	glRasterPos3f(0,0,-1);
	glBitmap(0,0,0,0, cell.x, cell.y, NULL);
	ogEnvLog(3, "glDrawPixels(%d, %d, %s, %s);\n",
		 imageData.imageWidth, imageData.imageHeight,
		 ogEnvPixelFormatName(imageData.format),
		 ogEnvDataTypeName(imageData.type));
	glDrawPixels(imageData.imageWidth, imageData.imageHeight, 
		imageData.format, imageData.type, imageData.newimage);
	nextType(&imageData);
	ogImgCreateNewImage(&imageData);
	nextCell(&cell);
    }

    /* test ReadPixels */

    for (i=0; i < numTypes; i++) {
	ogEnvLog(2, "glReadPixels(%d, %d, %s, %s);\n",
		 imageData.imageWidth, imageData.imageHeight,
		 ogEnvPixelFormatName(imageData.format),
		 ogEnvDataTypeName(imageData.type));
	glReadPixels(cell.xoffset,cell.yoffset, 
		imageData.imageWidth, imageData.imageHeight, 
		imageData.format, imageData.type, tempbuf);
	glRasterPos3f(0,0,-1);
	glBitmap(0,0,0,0, cell.x, cell.y, NULL);
	ogEnvLog(3, "glDrawPixels(%d, %d, %s, %s);\n",
		 imageData.imageWidth, imageData.imageHeight,
		 ogEnvPixelFormatName(imageData.format),
		 ogEnvDataTypeName(imageData.type));
	glDrawPixels(imageData.imageWidth, imageData.imageHeight, 
		imageData.format, imageData.type, tempbuf);

	nextType(&imageData);
	nextCell(&cell);
    }
    ogImgCreateNewImage(&imageData);

    /* test TexImage */

    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, textureData.imageWidth);
    glPixelStorei(GL_PACK_ROW_LENGTH, textureData.imageWidth);

    for (i=0; i < numTypes; i++) {
	ogEnvLog(3, 
		 "glTexImage2D(GL_TEXTURE_2D, 0, 0x%x, %d, %d, 0, %s, %s);\n",
		 textureData.components,
		 textureData.imageWidth, textureData.imageHeight,
		 ogEnvPixelFormatName(textureData.format),
		 ogEnvPixelFormatName(textureData.type));
	glTexImage2D(GL_TEXTURE_2D, 0, textureData.components,
		    textureData.imageWidth, textureData.imageHeight,
		    0, textureData.format, textureData.type, 
		    textureData.newimage);
	glBegin(GL_QUADS);
	    glTexCoord2f(0.0, 0.0);
	    glVertex2i(cell.x, cell.y);
	    glTexCoord2f(1.0, 0.0);
	    glVertex2i(cell.x+imageData.imageWidth, cell.y);
	    glTexCoord2f(1.0, 1.0);
	    glVertex2i(cell.x+imageData.imageWidth, 
			cell.y+imageData.imageHeight);
	    glTexCoord2f(0.0, 1.0);
	    glVertex2i(cell.x, cell.y+imageData.imageHeight);
	glEnd();

	nextType(&textureData);
	ogImgCreateNewImage(&textureData);
	nextCell(&cell);
    }

    /* test GetTexImage */

    for (i=0; i < numTypes; i++) {
	ogEnvLog(3, "glTexImage2D(GL_TEXTURE_2D, 0, 4, %d, %d, 0, GL_RGBA, GL_FLOAT);\n",
		 textureData.imageWidth, textureData.imageHeight);
	glTexImage2D(GL_TEXTURE_2D, 0, 4,
			textureData.imageWidth, textureData.imageHeight,
			0, GL_RGBA, GL_FLOAT, textureData.newimage);
	ogEnvLog(3, "glGetTexImage(GL_TEXTURE_2D, 0, %s, %s);\n",
		 ogEnvPixelFormatName(textureData.format),
		 ogEnvPixelFormatName(textureData.type));
	glGetTexImage(GL_TEXTURE_2D, 0, 
			textureData.format, textureData.type, tempbuf);
	ogEnvLog(3, 
		 "glTexImage2D(GL_TEXTURE_2D, 0, 0x%x, %d, %d, 0, %s, %s);\n",
		 textureData.components,
		 textureData.imageWidth, textureData.imageHeight,
		 ogEnvPixelFormatName(textureData.format),
		 ogEnvPixelFormatName(textureData.type));
	glTexImage2D(GL_TEXTURE_2D, 0, textureData.components,
			textureData.imageWidth, textureData.imageHeight,
			0, textureData.format, textureData.type, tempbuf);
	glBegin(GL_QUADS);
	    glTexCoord2f(0.0, 0.0);
	    glVertex2i(cell.x, cell.y);
	    glTexCoord2f(1.0, 0.0);
	    glVertex2i(cell.x+imageData.imageWidth, cell.y);
	    glTexCoord2f(1.0, 1.0);
	    glVertex2i(cell.x+imageData.imageWidth, 
			cell.y+imageData.imageHeight);
	    glTexCoord2f(0.0, 1.0);
	    glVertex2i(cell.x, cell.y+imageData.imageHeight);
	glEnd();

	nextType(&textureData);
	nextCell(&cell);
    }
    glDisable(GL_TEXTURE_2D);

    glFlush();

}

TESTMOD(packedpix)
{
    SUPPORTED_EXTENSION("GL_EXT_packed_pixels");
    InitPackedPix();

    while (pass--) {
	DrawPackedPix();
    }
}

CLEANUP(packedpix)
{
    GLfloat val = 0;

    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0,0,0,0);

    glPixelMapfv(GL_PIXEL_MAP_R_TO_R, 1, &val);
    glPixelMapfv(GL_PIXEL_MAP_G_TO_G, 1, &val);
    glPixelMapfv(GL_PIXEL_MAP_B_TO_B, 1, &val);
    glPixelMapfv(GL_PIXEL_MAP_A_TO_A, 1, &val);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    ogLibSetDefaultBuffers();
    ogLibSetDefaultTextures();
    ogLibSetDefaultRasterPos();

    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
}

#else  /* def GL_EXT_packed_pixels */

TESTMOD(packedpix) {}

CLEANUP(packedpix) {}

#endif /* GL_EXT_packed_pixels */

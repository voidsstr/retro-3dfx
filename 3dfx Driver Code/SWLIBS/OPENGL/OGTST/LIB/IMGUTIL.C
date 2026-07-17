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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ogtst.h"
#include "imgutil.h"

/*
** These utilities take a byte rgb image and convert it into
** a given format/type image in preparation for calling DrawPixels,
** TexImage, etc.
*/

void ogImgCreateFloatImage(ogImgImageRec *imageIn, ogImgNewImageRec *imageOut)
{
    GLubyte *rawData;
    GLint x,y;
    GLfloat *fltImage, *pFltImage;
    GLfloat alpha;
    GLint imageWidth, imageHeight;

    imageWidth = imageIn->sizeX;
    imageHeight = imageIn->sizeY;
    rawData = imageIn->data;

    imageOut->imageSize = imageWidth * imageHeight * sizeof(GLfloat) * 4;
    fltImage = (float *)ogLibMalloc(imageOut->imageSize);
    imageOut->fltImage = fltImage;
    if (NULL == fltImage) {
	fprintf(stderr, "Out of memory!\n");
	exit(1);
    }


    pFltImage = fltImage;
    for (y = 0; y < imageHeight; y++) {
        for (x = 0; x < imageWidth; x++) {
            *pFltImage++ = *rawData++ / 255.0;
            alpha = *pFltImage++ = *rawData++ / 255.0;
            *pFltImage++ = *rawData++ / 255.0;
            *pFltImage++ = alpha;
        }
    }
}

void ogImgCreateNewImage(ogImgNewImageRec *imageData)
{
    GLint imageWidth = imageData->imageWidth;
    GLint imageHeight = imageData->imageHeight;
    GLboolean swapBytes = imageData->swapBytes;
    GLboolean alignShift = imageData->alignShift; 
    GLenum type = imageData->type;
    GLenum format = imageData->format;

    void *newimage;
    GLfloat *myimage = imageData->fltImage;

    int components;
    int bytesPerComp;
    int i,j,k;
    int index;
    GLfloat indexscale;
    int imageSize;

    imageData->copyFormat = GL_COLOR;
    index = 0;
    switch(type) {
      case GL_BYTE:
        bytesPerComp = 1;
        break;
      case GL_UNSIGNED_BYTE:
        bytesPerComp = 1;
        break;
      case GL_INT:
        bytesPerComp = 4;
        break;
      case GL_UNSIGNED_INT:
        bytesPerComp = 4;
        break;
      case GL_BITMAP:
        break;
      case GL_FLOAT:
        bytesPerComp = 4;
        break;
      case GL_UNSIGNED_SHORT:
        bytesPerComp = 2;
        break;
      case GL_SHORT:
        bytesPerComp = 2;
        break;
#ifdef GL_EXT_packed_pixels
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
        bytesPerComp = 1;
        break;
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
        bytesPerComp = 2;
        break;
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
        bytesPerComp = 2;
        break;
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
        bytesPerComp = 4;
        break;
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
        bytesPerComp = 4;
        break;
#endif
    }
    switch(format) {
      case GL_RGB:
        components = 3;
        break;
      case GL_RED:
        components = 1;
        break;
      case GL_GREEN:
        components = 1;
        break;
      case GL_BLUE:
        components = 1;
        break;
      case GL_ALPHA:
        components = 1;
        break;
      case GL_LUMINANCE_ALPHA:
        components = 2;
        break;
      case GL_LUMINANCE:
        components = 1;
        break;
      case GL_RGBA:
        components = 4;
        break;
      case GL_COLOR_INDEX:
        components = 1;
        index = 1;
        break;
      case GL_DEPTH_COMPONENT:
        components = 1;
        imageData->copyFormat = GL_DEPTH;
        break;
      case GL_STENCIL_INDEX:
        components = 1;
        index = 1;
        imageData->copyFormat = GL_STENCIL;
        break;
    }
#ifdef GL_EXT_packed_pixels
    switch (type) {
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
        components = 1;
	index = 0;
	break;
    }
#endif
    if (index) {
	GLint bufferbits, typebits;
	switch (format) {
	  case GL_COLOR_INDEX:
	    glGetIntegerv(GL_INDEX_BITS, &bufferbits);
	    break;
	  case GL_STENCIL_INDEX:
	    glGetIntegerv(GL_STENCIL_BITS, &bufferbits);
	    break;
	}
	imageData->maxindex = (bufferbits > 0) ? ~(~1 << bufferbits-1) : 0;
	switch (type) {
	   case GL_BITMAP:
	   case GL_FLOAT:
	     typebits = bufferbits;
	     break;
	   case GL_BYTE:
	     typebits = 8*sizeof(GLbyte) - 1;
	     break;
	   case GL_UNSIGNED_BYTE:
	     typebits = 8*sizeof(GLubyte);
	     break;
	   case GL_INT:
	     typebits = 8*sizeof(GLint) - 1;
	     break;
	   case GL_UNSIGNED_INT:
	     typebits = 8*sizeof(GLuint);
	     break;
	   case GL_SHORT:
	     typebits = 8*sizeof(GLshort) - 1;
	     break;
	   case GL_UNSIGNED_SHORT:
	     typebits = 8*sizeof(GLushort);
	     break;
#ifdef GL_EXT_packed_pixels
	   case GL_UNSIGNED_BYTE_3_3_2_EXT:
	     typebits = 8*sizeof(GLubyte);
	     break;
	   case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
	   case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
	     typebits = 8*sizeof(GLushort);
	     break;
	   case GL_UNSIGNED_INT_8_8_8_8_EXT:
	   case GL_UNSIGNED_INT_10_10_10_2_EXT:
	     typebits = 8*sizeof(GLuint);
	     break;
#endif
	}
	if (typebits < bufferbits) {
	    indexscale = (GLfloat) pow(2.0, (double) typebits) - 1.0;
	} else {
	    indexscale = (GLfloat) pow(2.0, (double) bufferbits) - 1.0;
	}
    } else {
	imageData->maxindex = 0;
	indexscale = 0;
    }

    /* Build the new image */
    if (imageData->oldimage) free((void *) ((__psint_t)(imageData->oldimage) & 0xfffffffc));

    if (type != GL_BITMAP) {
        imageSize = imageWidth*imageHeight*components*bytesPerComp;
    } else {
        imageSize = ((imageWidth + 7)/8)*imageHeight;
    }
    newimage = (void *)ogLibMalloc(imageSize + 3);
    if (NULL == newimage) {
	fprintf(stderr, "Out of memory!\n");
	exit(1);
    }

    if (format == GL_GREEN) myimage += 1;
    else if (format == GL_BLUE || format == GL_LUMINANCE ||
            format == GL_LUMINANCE_ALPHA) myimage += 2;
    else if (format == GL_ALPHA) myimage += 3;
    switch (type) {
	case GL_BITMAP:
	{
	    GLubyte *data;
	    GLint bit;
	    GLubyte byte;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		bit = 0;
		byte = 0;
		for (j=0; j<imageWidth; j++) {
		    if (*myimage++ > 0.5) {
			if (swapBytes) {
			    byte |= 1<<(7-bit);
			} else {
			    byte |= 1<<bit;
			}
		    }
		    bit++;
		    if (bit == 8) {
			bit = 0;
			*data++ = byte;
			byte = 0;
		    }
		    myimage += 3;
		}
		if (bit) {
		    *data++ = byte;
		}
	    }
	} 
	break;
	case GL_UNSIGNED_BYTE:
	{
	    GLubyte *data;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    for (k=0; k<components; k++) {
			if (index) {
			    *data++ = *myimage++ * indexscale;
			} else {
			    *data++ = *myimage++ * 255.0;
			}
		    }
		    myimage+=(4-components);
		}
	    }
	}
	break;
	case GL_BYTE:
	{
	    GLbyte *data;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    for (k=0; k<components; k++) {
			if (index) {
			    *data++ = *myimage++ * indexscale;
			} else {
			    *data++ = *myimage++ * 127.0;
			}
		    }
		    myimage+=(4-components);
		}
	    }
	}
	break;
	case GL_UNSIGNED_SHORT:
	{
	    GLushort *data;
	    GLubyte *temp1, *temp2;
	    GLushort answer;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    for (k=0; k<components; k++) {
			if (index) {
			    answer = *myimage++ * indexscale;
			} else {
			    answer = *myimage++ * (GLfloat) 65535.0;
			}
			if (swapBytes) {
			    temp2 = (GLubyte *) &answer;
			    temp1 = (GLubyte *) data;
			    temp1[0] = temp2[1];
			    temp1[1] = temp2[0];
			} else {
			    data[0] = answer;
			}
			data++;
		    }
		    myimage+=(4-components);
		}
	    }
	} 
	break;
	case GL_SHORT:
	{
	    GLshort *data;
	    GLubyte *temp1, *temp2;
	    GLshort answer;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    for (k=0; k<components; k++) {
			if (index) {
			    answer = *myimage++ * indexscale;
			} else {
			    answer = *myimage++ * (GLfloat) 32767.0;
			}
			if (swapBytes) {
			    temp2 = (GLubyte *) &answer;
			    temp1 = (GLubyte *) data;
			    temp1[0] = temp2[1];
			    temp1[1] = temp2[0];
			} else {
			    data[0] = answer;
			}
			data++;
		    }
		    myimage+=(4-components);
		}
	    }
	}
	break;
	case GL_INT:
	{
	    GLint *data;
	    GLubyte *temp1, *temp2;
	    GLint answer;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    for (k=0; k<components; k++) {
			if (index) {
			    answer = *myimage++ * indexscale;
			} else {
			    answer = *myimage++ * (GLfloat) 2147482500.0;
			}
			if (swapBytes) {
			    temp2 = (GLubyte *) &answer;
			    temp1 = (GLubyte *) data;
			    temp1[0] = temp2[3];
			    temp1[1] = temp2[2];
			    temp1[2] = temp2[1];
			    temp1[3] = temp2[0];
			} else {
			    data[0] = answer;
			}
			data++;
		    }
		    myimage+=(4-components);
		}
	    }
	}
	break;
	case GL_UNSIGNED_INT:
	{
	    GLuint *data;
	    GLubyte *temp1, *temp2;
	    GLuint answer;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    for (k=0; k<components; k++) {
			if (index) {
			    answer = *myimage++ * indexscale;
			} else {
			    answer = *myimage++ * (GLfloat) 4294965000.0;
			}
			if (swapBytes) {
			    temp2 = (GLubyte *) &answer;
			    temp1 = (GLubyte *) data;
			    temp1[0] = temp2[3];
			    temp1[1] = temp2[2];
			    temp1[2] = temp2[1];
			    temp1[3] = temp2[0];
			} else {
			    data[0] = answer;
			}
			data++;
		    }
		    myimage+=(4-components);
		}
	    }
	}
	break;
	case GL_FLOAT:
	{
	    GLfloat *data;
	    GLubyte *temp1, *temp2;
	    GLfloat answer;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    for (k=0; k<components; k++) {
			if (index) {
			    answer = *myimage++ * indexscale;
			} else {
			    answer = *myimage++;
			}
			if (swapBytes) {
			    temp2 = (GLubyte *) &answer;
			    temp1 = (GLubyte *) data;
			    temp1[0] = temp2[3];
			    temp1[1] = temp2[2];
			    temp1[2] = temp2[1];
			    temp1[3] = temp2[0];
			} else {
			    data[0] = answer;
			}
			data++;
		    }
		    myimage+=(4-components);
		}
	    }
	}
	break;
#ifdef GL_EXT_packed_pixels
	case GL_UNSIGNED_BYTE_3_3_2_EXT:
	{
	    GLubyte *data;
	    GLubyte *answer, swapped_answer[4];
	    GLfloat *invalue;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    /* first bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data = ((GLubyte)(*invalue * 7) << 5) & 0xE0;

		    /* second bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLubyte)(*invalue * 7) << 2) & 0x1C;

		    /* third bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= (GLubyte)(*invalue * 3) & 0x03;

		    data++;
		    myimage+=1; /* skip the final component */
		}
	    }
	}
	break;
	case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
	{
	    GLushort *data;
	    GLubyte *answer, swapped_answer[4];
	    GLfloat *invalue;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    /* first bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data = ((GLushort)(*invalue * 15) << 12) & 0xF000;

		    /* second bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLushort)(*invalue * 15) << 8) & 0x0F00;

		    /* third bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLushort)(*invalue * 15) << 4) & 0x00F0;

		    /* fourth bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= (GLushort)(*invalue * 15) & 0x000F;

		    data++;
		}
	    }
	}
	break;
	case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
	{
	    GLushort *data;
	    GLubyte *answer, swapped_answer[4];
	    GLfloat *invalue;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    /* first bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data = ((GLushort)(*invalue * 31) << 11) & 0xF800;

		    /* second bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLushort)(*invalue * 31) << 6) & 0x07C0;

		    /* third bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLushort)(*invalue * 31) << 1) & 0x003E;

		    /* fourth bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= (GLushort)(*invalue) & 0x0001;

		    data++;
		}
	    }
	}
	break;
	case GL_UNSIGNED_INT_8_8_8_8_EXT:
	{
	    GLuint *data;
	    GLubyte *answer, swapped_answer[4];
	    GLfloat *invalue;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    /* first bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data = ((GLuint)(*invalue * 255) << 24) & 0xFF000000;

		    /* second bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLushort)(*invalue * 255) << 16) & 0x00FF0000;

		    /* third bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLuint)(*invalue * 255) << 8) & 0x0000FF00;

		    /* fourth bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= (GLuint)(*invalue * 255) & 0x000000FF;

		    data++;
		}
	    }
	}
	break;
	case GL_UNSIGNED_INT_10_10_10_2_EXT:
	{
	    GLuint *data;
	    GLubyte *answer, swapped_answer[4];
	    GLfloat *invalue;

	    data = newimage;
	    for (i=0; i<imageHeight; i++) {
		for (j=0; j<imageWidth; j++) {
		    /* first bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data = ((GLuint)(*invalue * 1023) << 22) & 0xFFC00000;

		    /* second bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLuint)(*invalue * 1023) << 12) & 0x003FF000;

		    /* third bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= ((GLuint)(*invalue * 1023) << 2) & 0x00000FFC;

		    /* fourth bitfield */
		    invalue = myimage++;
		    if (swapBytes) {
			answer = (GLubyte *)invalue;
			swapped_answer[0] = answer[3];
			swapped_answer[1] = answer[2];
			swapped_answer[2] = answer[1];
			swapped_answer[3] = answer[0];
			invalue = (GLfloat *)swapped_answer;
		    }
		    *data |= (GLuint)(*invalue * 3) & 0x00000003;

		    data++;
		}
	    }
	}
	break;
#endif
    } 

    if (alignShift) {
        memcpy((void *) (((__psint_t) newimage)+alignShift), newimage, imageSize);
        newimage = (void *) (((__psint_t) newimage)+alignShift);
    }
    imageData->newimage = newimage;
}


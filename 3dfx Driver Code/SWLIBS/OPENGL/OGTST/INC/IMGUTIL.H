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

/* 
** Utilities designed to convert a byte rgb image into any of the
** format/type combinations taken by image routines such as
** DrawPixels and TexImage.
*/

/*
** Raw byte rgb image.  Set this image however you want, and then
** feed it into ogImgCreateFloatImage to set up the base float image.
*/
typedef struct _imageRec {
    long sizeX, sizeY;
    unsigned char *data;
} ogImgImageRec;

/*
** Includes a base float image that can be converted into any format/type
** image.  The float image is set with ogImgCreateFloatImage, and the
** format/type image is created with ogImgCreateNewImage.
*/
typedef struct _newImageRec {
    GLfloat *fltImage;
    void *oldimage;
    void *newimage;
    GLenum copyFormat;
    GLint maxindex;
    GLint imageWidth;
    GLint imageHeight;
    GLboolean swapBytes;
    GLboolean alignShift;
    GLenum type;
    GLenum format;
    GLint components;
    GLint imageSize;
} ogImgNewImageRec;

extern void ogImgCreateFloatImage(ogImgImageRec *imageIn, 
					ogImgNewImageRec *imageOut);
extern void ogImgCreateNewImage(ogImgNewImageRec *imageData);


/*******************************************************************************
 * 
 * $Header: egimageio1.c, 4, 10/11/00 7:33:00 PM, Brent$
 * $Revision: 4$
 * $Date: 10/11/00 7:33:00 PM$
 *
 ******************************************************************************/
/*******************************************************************************

  Sample file: EGIMAGEIO1.C

  Objective:	Show how to access and manipulate an image.

  Program functions:	Reads an image from the command line.
							Swaps the channels of the image.
							Writes the swapped image.
	
  API functions used:mgReadImage(), mgReadImageAttributes(), mgWriteImage(),
							mgWriteImageAttributes()
 ******************************************************************************/
/*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgapiall.h"

static void print_error(char *msg)
{
	if (msg)
		printf("%s\n", msg);
}

static int swap_channels(unsigned char *pixels, unsigned char *newpixels,
					int type, int width, int height)
{
	unsigned char *iptr1, *iptr2, *iptr3;
	unsigned char *optr1, *optr2, *optr3;
	int i, size;

	size = width * height;
	switch(type)
	{
		case 2:		/* intensity - 1 channel - no need to swap */
			memcpy(newpixels, pixels, size * (type -1));
			return(mgTRUE);
		case 3:		/* intensity alpha - 2 channels */
			iptr1 = pixels;
			iptr2 = pixels + size;
			optr1 = newpixels;
			optr2 = newpixels + size;
			i = size;
			while (i--)
			{
				*optr1++ = *iptr2++;
				*optr2++ = *iptr1++;
			}
			return (mgTRUE);
		case 4:		/* RGB - 3 channesl - only need to swap channels 1 & 3 */
			iptr1 = pixels;
			iptr2 = pixels + size;
			iptr3 = pixels + 2*size;
			optr1 = newpixels;
			optr2 = newpixels + size;
			optr3 = newpixels + 2*size;
			i = size;
			while (i--)
			{
				*optr1++ = *iptr3++;
				*optr2++ = *iptr2++;
				*optr3++ = *iptr1++;
			}
			return (mgTRUE);
		case 5:		/* RGBA - 4 channels */
				/* Note the different way the channels are stored */
			iptr1 = pixels;
			optr1 = newpixels;
			i = size;
			while (i--)
			{
				*optr1 = *(iptr1+3);
				*(optr1+1) = *(iptr1+2);
				*(optr1+2) = *(iptr1+1);
				*(optr1+3) = *iptr1;
				iptr1 += 4;
				optr1 += 4;
			}
			return (mgTRUE);
		default:
			return (mgFALSE);
	}
}

void main (int argc, char **argv)
{
	int type, width, height, status;
	unsigned char *pixels, *newpixels;
	mgrec *attr_rec;

	/* check for proper arguments */
	if (argc < 3) {
		printf("Usage: %s infile outfile\n", argv[0]);
		exit (0);
	}

		/* Read the input image */
	status = mgReadImage(argv[1], &pixels, &type, &width, &height);

	if (status != 0) {
		print_error("Could not read image");
		exit(EXIT_FAILURE);
	}

		/* Read the input image attributes */
	if (attr_rec = mgReadImageAttributes(argv[1])) {
		print_error("Could not read image attributes");
		exit(EXIT_FAILURE);		
	}

		/* Allocate the swapped image */
	newpixels = (unsigned char *) malloc (width * height * (type -1));
	if (!newpixels) {
		print_error("Could not allocate swapped image");
		exit(EXIT_FAILURE);		
	}

		/* Swap the channels */
	if (!swap_channels(pixels, newpixels, type, width, height)) {
		print_error("Could not swap the channels for this image");
		exit(EXIT_FAILURE);		
	}

		/* Write the swapped image */
	status = mgWriteImage(argv[2], newpixels, type, width, height, mgFALSE);
	if (status != 0) {
		print_error("Could not write image");
		exit(EXIT_FAILURE);
	}

		/* Write the swapped image attributes */
	if (!mgWriteImageAttributes(argv[2], attr_rec)) {
		print_error("Could not write image attributes");
		exit(EXIT_FAILURE);		
	}

	free(pixels);
	free(newpixels);

	/* exit */
	mgExit();
	exit(0);
}


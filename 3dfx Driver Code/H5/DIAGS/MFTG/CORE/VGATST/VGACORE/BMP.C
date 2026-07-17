//
//		BMP.CPP - Global definitions for reading and writing BMP files
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Rich Goodin, Larry Coffey
//		Date:				1/1/95
//		Last Modified:	5/2/97
//
//		Routines in this file:
//		WriteBMP8Header	Allocate storage for maps and bios memory.
//		WriteBMP8Color		Write an 8 bit color value to the output file.
//		CloseBMP8			Close output file.
// 	AppendExtension	Appends the .BMP extension to the file if none exists.
// 	SetupBMPHeader		Sets up a header for a BMP file.
//		HandleCaptureMode	Handles the different capture mode.
//
#include "vgaint.h"

// Globals
unsigned int bmpWidth;			// width of scan line in bytes
unsigned int bmpCurWidth;		// bytes remaining in scan line
unsigned int bmpPad;				// pad at end of scanline
unsigned long bmpCurrent;		// position of start of scan line in file
unsigned long bmpPadWidth;		// width of padded scanline
LPBYTE	bfCurPtr;				// pointer to output buffer

// Buffer definition for output data
#define BMP_MAX_WIDTH 2048
BYTE buffer[BMP_MAX_WIDTH];

//
//	When capturing in COMPOSITE mode, the top 16 colors of the DAC color
//	map are overwritten by the following 16 colors.  These color values are
//	then used to represent the sync and blanking signals.
//
unsigned long ulOverRide[16] = 
{
	0X00000000L,
	0X00000077L,
	0X000000FFL,
	0X00007700L,
	0X0000FF00L,
	0X00007777L,
	0X0000FFFFL,
	0X00770000L,
	0X00FF0000L,
	0X00770077L,
	0X00FF00FFL,
	0X00777700L,
	0X00FFFF00L,
	0X00555555L,
	0X00AAAAAAL,
	0X00FFFFFFL,
};


//
//		WriteBMP8Header	-	Allocate storage for maps and bios memory
//
//		Entry:	pfpScanfile -	Pointer to file pointer.  Returns the file pointer
//    								of the open output file.
//    			szFilename		Null terminated character string specifying to output
//    								file to open
//    			nnWidth			Width of image in pixels
//    			nHeight			Height of image in pixels
//
//		Exit:		<BOOL>	(0 if success, nonzero if failure)
//
int WriteBMP8Header (FILE **pfpScanfile, LPSTR szFilename, WORD nWidth, WORD nHeight)
{
	static char		szFullname[64];	// processed file name storage
	DWORD				lImageSize;  		// number of pixels in image
	LPBYTE			bPtr;         		// pointer to data buffer for header writes
	int 				i;

#ifdef __MSVC16__
	_fstrncpy (szFullname, szFilename, 60);
#else
	strncpy (szFullname, szFilename, 60);
#endif

	// If the filename has no extension, append '.bmp'
	AppendExtension (szFullname);

 	//	Open file for binary write
	if ((*pfpScanfile = fopen (szFullname,"wb")) == NULL)
	{
   	fprintf (stderr, "cannot open output file <%s>\n", szFullname);
    	return (-1);
	}

	//	BMP's have DWORD padding
  	if	((bmpPad = (nWidth % 4)) != 0)
   	bmpPad = 4 - bmpPad;
	else
		bmpPad = 0;

	// Calculate padded scanline width
	bmpPadWidth = (((unsigned long) nWidth) + (unsigned long) bmpPad);

	// Calculate number of pixels in image for header
	lImageSize = ((DWORD) bmpPadWidth) * ((DWORD) nHeight);

	if (bmpPadWidth > BMP_MAX_WIDTH)
	{
		fprintf (stderr, "file routines cannot handle width > %d\n",
					BMP_MAX_WIDTH);
		return (-1);
  	}

	// Point to start of buffer
	bPtr = (BYTE *)buffer;

	// Now let's setup the BMP file header
	SetupBMPHeader (bPtr, lImageSize, nWidth, nHeight);

	//	Point to color map location in buffer
	bfCurPtr = buffer+54;

	// Write out first 240 colors in colormap
  	for (i = 0; i < 240; i++)
  	{
    	*bfCurPtr++ = (BYTE)(dwDACReg[i] & 0xFF);
    	*bfCurPtr++ = (BYTE)((dwDACReg[i] >> 8) & 0xFF);
    	*bfCurPtr++ = (BYTE)((dwDACReg[i] >> 16) & 0xFF);
		*bfCurPtr++ = 0;
  	}

	// Handle the different Capture Modes
	HandleCaptureMode (bfCurPtr);

	// Calculate bytes remaining in scan line and scan line width
	bmpWidth = bmpCurWidth = nWidth;

	// Calculate position of start of last scan line in file
	bmpCurrent = (bmpPadWidth * ((unsigned long) (nHeight - 1)))
						+ 54L + (256L * 4L);

	// Write out buffered header
	fwrite (buffer, 54 + (4 * 256), 1, *pfpScanfile);
	bfCurPtr = buffer;

	// Point to start of last scan line
	// BMP's are bottom to top
	fseek (*pfpScanfile, bmpCurrent, SEEK_SET);

	return (ferror (*pfpScanfile));
}

//
//		WriteBMP8Color - Write an 8 bit color value to the output file.
//
//		Entry:	fpScanfile	File pointer returned from WriteBMP8Header
//					wColor		8 bit color value
//		Exit:		<BOOL>		(0 if success, nonzero if failure)
//
int WriteBMP8Color (FILE *fpScanfile, BYTE wColor)
{
	// Write pixel to buffer
	*bfCurPtr++ = wColor;

	// Decrement remaining pixel count
	bmpCurWidth--;  

  	// If last pixel in scanline - write out scanline buffer
 	if(bmpCurWidth == 0)
	{

   	// DWORD padding
    	switch(bmpPad)
		{
    		case 3:
      		*bfCurPtr++ = 0;
      	// NOTE: this falls thru
    		case 2:
      		*bfCurPtr++ = 0;
      	// NOTE: this falls thru
    		case 1:
      		*bfCurPtr++ = 0;
      	// NOTE: this falls thru
   		case 0:
      	;
    	}

		// Write out buffer
		fwrite (buffer, (int)bmpPadWidth, 1, fpScanfile);

		// Point to start of buffer
		bfCurPtr = buffer;

		// Reset bytes remaining
		bmpCurWidth = bmpWidth;

		// Point to prevoius scanline in file
		bmpCurrent -= bmpPadWidth;
		fseek (fpScanfile, bmpCurrent, SEEK_SET);
	}

	return (ferror (fpScanfile));
}

//
//		CloseBMP8 - Close output file
//
//		Entry:	fpScanfile		File pointer returned from WriteBMP8Header.
//		Exit:		None
//
void CloseBMP8 (FILE *fpScanfile)
{
	fclose (fpScanfile);
}

//
// 	AppendExtension - Appends the .BMP extension to the file if none exists.
//
//		Entry:		szFullName	Pointer to the filename
//		Exit:			None
//
void AppendExtension (char *szFullname)
{
	if (strchr (szFullname, '.') == NULL) 
   	strcat (szFullname, ".bmp");
} 

//
// 	SetupBMPHeader - Sets up a header for a BMP file
//
//		Entry:	wPtr			Word pointer
//					lImageSize	Image Size!
//					nWidth		Width of Image
//					nHeight		Height of Image
//		Exit:		None
//
void SetupBMPHeader (LPBYTE bPtr, DWORD lImageSize, WORD nWidth, WORD nHeight)
{
	// Bitmap header
	*bPtr++ = 0x42; /* magic Number */
	*bPtr++ = 0x4d;
	// file size (pixels + header + colormap)
	*bPtr++ = (BYTE)((lImageSize+54+(256*4))&0xff);;
	*bPtr++ = (BYTE)(((lImageSize+54+(256*4))>>8)&0xff);
	*bPtr++ = (BYTE)(((lImageSize+54+(256*4))>>16)&0xff);;
	*bPtr++ = (BYTE)(((lImageSize+54+(256*4))>>24)&0xff);;
	*bPtr++ = 0x0; /* reserved */
	*bPtr++ = 0x0;
	*bPtr++ = 0x0; /* reserved */
	*bPtr++ = 0x0;
	*bPtr++ = (BYTE)((54+(4*256))&0xff); /* start of image data */
	*bPtr++ = (BYTE)(((54+(4*256))>>8)&0xff);;
	*bPtr++ = 0x0; /* reserved */
	*bPtr++ = 0x0;

	// Bitmap Info
	*bPtr++ = 40; /* header size */
	*bPtr++ = 0x0;

	*bPtr++ = 0x0;
	*bPtr++ = 0x0;

	*bPtr++ = (BYTE)(nWidth&0xff); /* image width */
	*bPtr++ = (BYTE)((nWidth>>8)&0xff);

	*bPtr++ = 0x0;
	*bPtr++ = 0x0;

	*bPtr++ = (BYTE)(nHeight&0xff);  /* image height */
	*bPtr++ = (BYTE)((nHeight>>8)&0xff);

	*bPtr++ = 0x0;
	*bPtr++ = 0x0;

	*bPtr++ = 1;  /* number of image planes */
	*bPtr++ = 0x0;

	*bPtr++ = 8;  /* bits per pixel */
	*bPtr++ = 0x0;

	*bPtr++ = 0x0; /* compression method (none) */
	*bPtr++ = 0x0;
	*bPtr++ = 0x0;
	*bPtr++ = 0x0;

	*bPtr++ = (BYTE)(lImageSize&0xff); /* image size (pixels) */
	*bPtr++ = (BYTE)((lImageSize>>8)&0xff);
	*bPtr++ = (BYTE)((lImageSize>>16)&0xff);
	*bPtr++ = (BYTE)((lImageSize>>24)&0xff);

	*bPtr++ = 0x0; /* horiz pixels per meter */
	*bPtr++ = 0x0;
	*bPtr++ = 0x0;
	*bPtr++ = 0x0;

	*bPtr++ = 0x0;  /* vert pixels per meter */
	*bPtr++ = 0x0;
	*bPtr++ = 0x0;
	*bPtr++ = 0x0;

	*bPtr++ = 0x00; /* number of colors */
	*bPtr++ = 0x01;
	*bPtr++ = 0x0;
	*bPtr++ = 0x0;

	*bPtr++ = 0x00; /* number of significant colors */
	*bPtr++ = 0x01;
	*bPtr++ = 0x0;
	*bPtr++ = 0x0;
}

//
//		HandleCaptureMode - Handles the different capture mode.
//
// 	Entry:	bfCurPtr Pointer to current BYTE in file.
//		Exit:		None
//
void HandleCaptureMode (LPBYTE bfCurPtr)
{
	int i;

	if(byGlobalCaptureMode == CAP_COMPOSITE)
	{
  		// COMPOSIT - write out override colors
  		for (i = 240; i < 256; i++)
		{
    		*bfCurPtr++ = (BYTE)(ulOverRide[i - 240] & 0xFF);
    		*bfCurPtr++ = (BYTE)((ulOverRide[i - 240] >> 8) & 0xFF);
    		*bfCurPtr++ = (BYTE)((ulOverRide[i - 240] >> 16) & 0xFF);
    		*bfCurPtr++ = 0;
  		}
	}
	else
	{
   	// Not !COMPOSIT - write out colormap colors
    	for (i = 240; i < 256; i++)
		{
      	*bfCurPtr++ = (BYTE)(dwDACReg[i] & 0xFF);
      	*bfCurPtr++ = (BYTE)((dwDACReg[i] >> 8) & 0xFF);
      	*bfCurPtr++ = (BYTE)((dwDACReg[i] >> 16) & 0xFF);
      	*bfCurPtr++ = 0;
    	}
	}
}

//
//		Copyright (c) 1994-1997 Elpin Systems, Inc.
//		All rights reserved.
//

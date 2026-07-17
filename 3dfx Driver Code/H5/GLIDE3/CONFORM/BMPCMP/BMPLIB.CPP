/*
 *      Windows bitmap compare utility
 *
 *      Copyright (c) 1998 by Goodin & Associates, Inc.
 *      All Rights Reserved.
 *
 */

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include "resource.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

#ifndef RMG
#include <stdio.h>
#include <glide.h>
extern "C" {
#include <fximg.h>
}
#endif

// Functions for extracting information from DIB memory blocks

DWORD GetDibInfoHeaderSize (BYTE * lpDib)
{
  return ((BITMAPINFOHEADER *) lpDib)->biSize ;
}

LONG GetDibWidth (BYTE * lpDib)
{
  return (LONG) (((BITMAPINFOHEADER *) lpDib)->biWidth) ;
}

LONG GetDibHeight (BYTE * lpDib)
{
  return (LONG) (((BITMAPINFOHEADER *) lpDib)->biHeight) ;
}

WORD GetDibNumColors (BYTE * lpDib)
{
  WORD  wBitCount ;
  
  wBitCount = ((BITMAPINFOHEADER *) lpDib)->biBitCount ;
  
  return (WORD) (1L << wBitCount) ;
}

WORD GetDibDepth (BYTE * lpDib)
{
  WORD  wBitCount ;
  
  wBitCount = ((BITMAPINFOHEADER *) lpDib)->biBitCount ;
  
  return (wBitCount) ;
}

BYTE * GetDibColorAddr (BYTE * lpDib)
{
  return lpDib + GetDibInfoHeaderSize (lpDib);
}

BYTE * GetDibBitsAddr (BYTE * lpDib)
{
  DWORD dwNumColors, dwColorTableSize ;
  WORD  wBitCount ;
  
  wBitCount = ((BITMAPINFOHEADER *) lpDib)->biBitCount ;
  
  if(wBitCount == 24) {
    /* 24 bit */
    return lpDib + GetDibInfoHeaderSize (lpDib) ;
  } else {
    /* 8 bit */
    dwNumColors = 1L << wBitCount ;
    dwColorTableSize = dwNumColors * sizeof (RGBQUAD) ;
    return lpDib + GetDibInfoHeaderSize (lpDib) + dwColorTableSize ;
  }
}

// Read an image file using fximg
BYTE * ReadFxFile (char * szFileName)
{
  ImgInfo info;
  BITMAPINFOHEADER header;
  BYTE * lpDib;
  BYTE * srcptr;
  BYTE * scanptr;
  BYTE * dstptr;
  FxU32 scanlineLength;
  FxU32 width, height;

  imgReadFile((const char *)szFileName, &info);

  // end of scan line byte padding for DIB
  if((scanlineLength = ((info.any.width*3)%4)) != 0) {
    scanlineLength =  4-scanlineLength;
  } else {
    scanlineLength = 0;
  }
  // scan line len in bytes
  scanlineLength += (info.any.width*3);

  // Allocate DIB storage
  if((lpDib = (BYTE * ) GlobalAllocPtr (GMEM_MOVEABLE, 
				    (scanlineLength*info.any.height)+
					sizeof(BITMAPINFOHEADER))) == NULL) {
    return(NULL);
  }
  
  // Build Header
  header.biSize = sizeof(BITMAPINFOHEADER);
  header.biWidth = info.any.width;
  header.biHeight = info.any.height;
  header.biPlanes = 1;
  header.biBitCount = 24;
  header.biCompression = BI_RGB;
  header.biSizeImage = 0;
  header.biXPelsPerMeter = 0;
  header.biYPelsPerMeter = 0;
  header.biClrUsed = 0;
  header.biClrImportant = 0;

  // Copy Header
  memcpy(lpDib,&header,sizeof(BITMAPINFOHEADER));

  scanptr = lpDib + sizeof(BITMAPINFOHEADER);

  // Copy Data
  // Windows "dib" format is inverted in Y from ".ppm"
  for(height = 0; height < info.any.height; height++) {
    dstptr = scanptr;
    srcptr = info.any.data;
    srcptr += (((info.any.height - height - 1) * info.any.width) * 4);
    for(width = 0; width < info.any.width; width++) {
      // Convert BGRA to BGR
      *dstptr++ = *srcptr++; // blue
      *dstptr++ = *srcptr++; // green
      *dstptr++ = *srcptr++; // red
      *srcptr++;
    }
    scanptr += scanlineLength;
  }

  // Free FXIMG data
  free(info.any.data);

  return(lpDib);
}

// Read a DIB from a file into memory

BYTE * ReadBmpFile (char * szFileName)
{
  BITMAPFILEHEADER bmfh ;
  BYTE *      lpDib ;
  DWORD            dwDibSize, dwOffset;
  int              hFile ;
  WORD             wDibRead ;
  
  if (-1 == (hFile = _lopen (szFileName, OF_READ | OF_SHARE_DENY_WRITE)))
    return NULL ;
  
  if (_lread (hFile, (LPSTR) &bmfh, sizeof (BITMAPFILEHEADER)) !=
      sizeof (BITMAPFILEHEADER))
    {
      _lclose (hFile) ;
      return NULL ;
    }
  
  if (bmfh.bfType != * (WORD *) "BM")
    {
      _lclose (hFile) ;
      return NULL ;
    }
  
  dwDibSize = bmfh.bfSize - sizeof (BITMAPFILEHEADER) ;
  
  lpDib = (BYTE * ) GlobalAllocPtr (GMEM_MOVEABLE, dwDibSize) ;
  
  if (lpDib == NULL)
    {
      _lclose (hFile) ;
      return NULL ;
    }
  
  dwOffset = 0 ;
  
  while (dwDibSize > 0)
    {
      wDibRead = (WORD) MIN (32768ul, dwDibSize) ;
      
      if (wDibRead != _lread (hFile, (LPSTR) (lpDib + dwOffset), wDibRead))
	{
	  _lclose (hFile) ;
	  GlobalFreePtr (lpDib) ;
	  return NULL ;
	}
      
      dwDibSize -= wDibRead ;
      dwOffset  += wDibRead ;
    }
  
  _lclose (hFile) ;
  
  /* we don't support old style bitmaps */
  if (GetDibInfoHeaderSize (lpDib) == sizeof (BITMAPCOREHEADER))
    {
      GlobalFreePtr (lpDib) ;
      return NULL ;
    }
  
  return lpDib ;
}

HPALETTE CreateBmpPalette (BYTE * lpDib)
{
  LOGPALETTE *plgpl;
  int i;
  BYTE *colorbits;
  HPALETTE hpal;
  
  plgpl = (LOGPALETTE *)LocalAlloc(LPTR,
				   sizeof(LOGPALETTE)+
				   ((GetDibNumColors(lpDib)-1)*
				    sizeof(PALETTEENTRY)));
  plgpl->palNumEntries = GetDibNumColors(lpDib);
  plgpl->palVersion = 0x300;
  
  colorbits = GetDibColorAddr(lpDib);
  for(i=0;i<plgpl->palNumEntries;i++) {
    plgpl->palPalEntry[i].peRed = *colorbits++; 
    plgpl->palPalEntry[i].peGreen = *colorbits++; 
    plgpl->palPalEntry[i].peBlue = *colorbits++; 
    colorbits++; 
  }
  
  hpal = CreatePalette(plgpl);
  
  LocalFree((HLOCAL)plgpl);
  
  return hpal;
}

WORD CompareDibCMAP(BYTE * testbits, BYTE * refbits)
{
  
  BYTE *testcmap;
  BYTE *refcmap;
  int i;
  
  testcmap = GetDibColorAddr(testbits);
  refcmap = GetDibColorAddr(refbits);
  
  for(i=0;i<(256*4);i++) {
    if(*testcmap++ != *refcmap++)
      return TRUE;
  }
  return FALSE;
}

BYTE * CreateDiffBits(WORD * diff, 
			   BYTE *testhdr,
			   WORD testdepth,
			   BYTE *refhdr,
			   WORD refdepth,
			   WORD dpytype)
{
  BYTE *diffaddr;
  BYTE *testbits;
  BYTE *refbits;
  BYTE *imgaddr;
  BYTE *colorbits;
  BYTE *truecolor;
  DWORD size;
  WORD i,j,k;
  LONG height;
  LONG width;
  
  *diff = FALSE;
  
  width = GetDibWidth(refhdr);
  height = GetDibHeight(refhdr);
  
  size = (DWORD)sizeof(BITMAPINFOHEADER) + (DWORD)(256*sizeof(RGBQUAD)) +
    ((DWORD)width * (DWORD)height);
  
  diffaddr = (BYTE * ) GlobalAllocPtr (GMEM_MOVEABLE, size) ;
  
  if(diffaddr == NULL) return NULL;
  
  ((BITMAPINFOHEADER *) diffaddr)->biSize = sizeof(BITMAPINFOHEADER);
  ((BITMAPINFOHEADER *) diffaddr)->biWidth = width;
  ((BITMAPINFOHEADER *) diffaddr)->biHeight = height;
  ((BITMAPINFOHEADER *) diffaddr)->biPlanes = 1;
  ((BITMAPINFOHEADER *) diffaddr)->biBitCount = 8;
  ((BITMAPINFOHEADER *) diffaddr)->biCompression = 0;
  ((BITMAPINFOHEADER *) diffaddr)->biSizeImage = 0;
  ((BITMAPINFOHEADER *) diffaddr)->biXPelsPerMeter = 0;
  ((BITMAPINFOHEADER *) diffaddr)->biYPelsPerMeter = 0;
  ((BITMAPINFOHEADER *) diffaddr)->biClrUsed = 5;
  ((BITMAPINFOHEADER *) diffaddr)->biClrImportant = 5;
  
  imgaddr = GetDibColorAddr(diffaddr);
  *imgaddr++ = 0xff; // white
  *imgaddr++ = 0xff;
  *imgaddr++ = 0xff;
  *imgaddr++ = 0x00;

  *imgaddr++ = 0x00; // black
  *imgaddr++ = 0x00;
  *imgaddr++ = 0x00;
  *imgaddr++ = 0x00;

  *imgaddr++ = 0xff; // blue
  *imgaddr++ = 0x00;
  *imgaddr++ = 0x00;
  *imgaddr++ = 0x00;

  *imgaddr++ = 0x00; // green
  *imgaddr++ = 0xff;
  *imgaddr++ = 0x00;
  *imgaddr++ = 0x00;

  *imgaddr++ = 0x80; // gray
  *imgaddr++ = 0x80;
  *imgaddr++ = 0x80;
  *imgaddr++ = 0x00;
  
  
  imgaddr = GetDibBitsAddr(diffaddr);
  testbits = GetDibBitsAddr(testhdr);
  refbits = GetDibBitsAddr(refhdr);
  
  /* 8 bit compare */
  if((testdepth == 8) && (refdepth == 8)) {
    for(i=0;i<height;i++)
      for(j=0;j<width;j++) {
	if(*testbits++ == *refbits++) {
	  *imgaddr++ = 0x01; /* black */
	} else {
	  *diff = TRUE;
	  *imgaddr++ = 0x00; /* white */
	}
      }
    
    return diffaddr;
  }
  
  /* ref is 24 mixed */
  if((testdepth == 8) && (refdepth == 24)) {
    
    colorbits = GetDibColorAddr(testhdr);
    
    for(i=0;i<height;i++)
      for(j=0;j<width;j++) {
	*imgaddr = 0x01; /* black */
	truecolor = colorbits + ((*testbits++)*4);
	for(k=0;k<3;k++) {
	  if(*refbits++ != *truecolor++) {
	    *diff = TRUE;
	    *imgaddr = 0x00; /* white */
	  }
	}
	imgaddr++;
      }
    
    return diffaddr;
  }
  
  /* test is 24 mixed */
  if((testdepth == 24) && (refdepth == 8)) {
    
    colorbits = GetDibColorAddr(refhdr);
    
    for(i=0;i<height;i++)
      for(j=0;j<width;j++) {
	*imgaddr = 0x01; /* black */
	truecolor = colorbits + ((*refbits++)*4);
	for(k=0;k<3;k++) {
	  if(*testbits++ != *truecolor++) {
	    *diff = TRUE;
	    *imgaddr = 0x00; /* white */
	  }
	}
	imgaddr++;
      }
    
    return diffaddr;
  }

  // If we're here, then we're comparing 24-bit images.

  if (dpytype == CM_DISPTESTOVERLAY24DIFF ||
      dpytype == CM_DISPREFOVERLAY24DIFF) {

    /* 24 bit compare, assuming a black background */
    int testpix, refpix; // 32 bit
    for (i=0; i<height; i++) {
      for (j=0; j<width; j++) {
        testpix = (testbits[0] << 16) | (testbits[1] << 8) | testbits[2];
        refpix  = (refbits[0] << 16) | (refbits[1] << 8) | refbits[2];
        *imgaddr = 0x01; /* black */
        if (testpix != refpix) {
          *diff = TRUE;
          if (testpix != 0x0 && refpix == 0x0) { // test image contains set pixel
            *imgaddr = 0x02; /* blue */
          } else if (refpix != 0x0  && testpix == 0x0) { // ref image contains set pixel
            *imgaddr = 0x03; /* green */
          } else if (refpix != 0x0 && testpix != 0x0){ // both not black, both not equal
            *imgaddr = 0x00; /* white */
          }
        } else if (testpix != 0x0) { // both equal, both not black
            *imgaddr = 0x04; /* gray */
        }
        testbits += 3;
        refbits += 3;
        imgaddr++;
      }
    }

  } else {

    /* 24 bit compare */
    for(i=0;i<height;i++) {
      for(j=0;j<width;j++) {
        *imgaddr = 0x01; /* black */
        for(k=0;k<3;k++) {  
          if(*testbits++ != *refbits++) {
	        *diff = TRUE;
	        *imgaddr = 0x00; /* white */
	      }
        }
        imgaddr++;
      }
    }
  }

  return diffaddr;
}

void GetDibColor(BYTE *dib, WORD depth,
		 signed short mousex, signed short mousey,
		 BYTE *index, BYTE *red, BYTE *green, BYTE *blue)
{
  
  BYTE *cmap;
  BYTE *colors;
  LONG width;
  LONG height;
  DWORD offset;
  
  /* point to data */
  colors = GetDibBitsAddr(dib);
  height = GetDibHeight(dib);
  
  /* dib is inverted */
  mousey = height-mousey-1;
  
  /* get width for address conversion */
  width = GetDibWidth(dib);
  
  if(depth == 8) {
    /* get index */
    offset = ((DWORD)mousey*(DWORD)width)+(DWORD)mousex;
    *index = *(colors + offset);
    
    /* find colormap */
    cmap = GetDibColorAddr(dib);
    offset = ((DWORD)*index)*4L;
    
    *blue = *(cmap + offset);
    offset++;
    *green = *(cmap + offset);
    offset++;
    *red = *(cmap + offset);
  } else {
    offset = (((DWORD)mousey*(DWORD)width)+(DWORD)mousex)*3L;
    *blue = *(colors + offset);
    offset++;
    *green = *(colors + offset);
    offset++;
    *red = *(colors + offset);
  }
}

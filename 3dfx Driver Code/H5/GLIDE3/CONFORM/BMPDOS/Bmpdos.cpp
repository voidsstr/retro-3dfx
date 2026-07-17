/*
 *      Windows command line bitmap compare utility
 *
 *      Copyright (c) 1998 by Goodin & Associates, Inc.
 *      All Rights Reserved.
 *
 */

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>

WORD GetDibInfoHeaderSize (BITMAPINFOHEADER *bmih)
{
  return (WORD) (bmih->biSize);
}

WORD GetDibWidth (BITMAPINFOHEADER *bmih)
{
  return (WORD) (bmih->biWidth) ;
}

WORD GetDibHeight (BITMAPINFOHEADER *bmih)
{
  return (WORD) (bmih->biHeight) ;
}

WORD GetDibDepth (BITMAPINFOHEADER *bmih)
{
  return (WORD) (bmih->biBitCount) ;
}

FILE *ReadBmpHeader (char * szFileName, BITMAPINFOHEADER *bmih)
{
  FILE *hFile ;
  BITMAPFILEHEADER bmfh;

  if (NULL == (hFile = fopen (szFileName, "rb")))
    return NULL ;
  
  if (fread (&bmfh, 1, sizeof (BITMAPFILEHEADER), hFile) !=
      sizeof (BITMAPFILEHEADER))
    {
      fclose (hFile) ;
      return NULL ;
    }
  
  if (bmfh.bfType != * (WORD *) "BM")
    {
      fclose (hFile) ;
      return NULL ;
    }
  
  if (fread (bmih, 1, sizeof (BITMAPINFOHEADER), hFile) !=
      sizeof (BITMAPINFOHEADER))
    {
      fclose (hFile) ;
      return NULL ;
    }
  
  /* we don't support old style bitmaps */
  if (GetDibInfoHeaderSize (bmih) == sizeof (BITMAPCOREHEADER))
    {
      fclose(hFile);
      return NULL ;
    }
  
  return hFile ;
}

void GetDibCMAP(FILE *hFile, DWORD *cmap)
{
  fread (cmap, 1, 4*256, hFile);
}

BYTE ref_buffer[1280*3];
BYTE test_buffer[1280*3];
DWORD ref_cmap[256];
DWORD test_cmap[256];

void main(int argc, char **argv)
{
  FILE *reffile, *testfile;
  WORD ref_depth, test_depth;
  BITMAPINFOHEADER ref_bmih;
  BITMAPINFOHEADER test_bmih;
  WORD width, height;
  BYTE verbose;
  DWORD cmap_error;
  DWORD data_error;

  if(argc < 3) {
    fprintf(stderr,"Usage: bmpdos <reference file> <test file>\n");
    exit(-1);
  }
  
  if(argc > 3)
    verbose = 1;
  else 
    verbose = 0;

  /* open reffile */
  if((reffile = ReadBmpHeader(argv[1],&ref_bmih)) == NULL) {
    fprintf(stderr,"reference file <%s> not found\n",argv[1]);
    exit(-1);
  }
  
  if((ref_depth = GetDibDepth(&ref_bmih)) == 8) {
    /* get colormap */
    GetDibCMAP(reffile, ref_cmap);
  }
  
  /* open testfile */
  if((testfile = ReadBmpHeader(argv[2],&test_bmih)) == NULL) {
    fprintf(stderr,"test file <%s> not found\n",argv[2]);
    fclose(reffile);
    exit(-1);
  }
  
  if((test_depth = GetDibDepth(&test_bmih)) == 8) {
    /* get colormap */
    GetDibCMAP(testfile, test_cmap);
  }
  
  /* compare size */
  width = GetDibWidth(&ref_bmih);
  height = GetDibHeight(&ref_bmih);

  if(width != GetDibWidth(&test_bmih)) {
    fprintf(stderr,"width mismatch - ref %d test %d\n",
	    width,GetDibWidth(&test_bmih));
    fclose(reffile);
    fclose(testfile);
    exit(-1);
  }

  if(height != GetDibHeight(&test_bmih)) {
    fprintf(stderr,"height mismatch - ref %d test %d\n",
	    height,GetDibHeight(&test_bmih));
    fclose(reffile);
    fclose(testfile);
    exit(-1);
  }

  /* set no errors */
  cmap_error = 0L;
  data_error = 0L;

  if((ref_depth == 8) && (test_depth == 8)) {
    /* ref = 8, test = 8 */
    int x,y;
    BYTE *tptr,*rptr;

    /* compare colormaps */
    for(x=0;x<256;x++)
      if(ref_cmap[x] != test_cmap[x]) {
	if(verbose)
	  fprintf(stdout,"cmap compare failed: expected %08x got %08x\n",
		  ref_cmap[x],test_cmap[x]);
	cmap_error++;
      }

    /* compare 8 bit data */
    for(y = 0; y < height; y++) {
      if(fread(ref_buffer, 1, width, reffile) != width) {
	fprintf(stderr,"error reading reference file\n");
	fclose(reffile);
	fclose(testfile);
	exit(-1);
      }
      if(fread(test_buffer, 1, width, testfile) != width) {
	fprintf(stderr,"error reading test file\n");
	fclose(reffile);
	fclose(testfile);
	exit(-1);
      }
      rptr = ref_buffer;
      tptr = test_buffer;
      for(x = 0; x < width; x++) {
	if(*tptr != *rptr) {
	  if(verbose)
	    fprintf(stdout,"data mismatch at (%d,%d): expected %02x got %02x\n",
		    x,height-y-1,*rptr,*tptr);
	  data_error++;
	}
	tptr++;
	rptr++;
      }
    }

  } else if((ref_depth == 24) && (test_depth == 24)) {
    /* ref = 24, test = 24 */
    int x,y;
    BYTE *tptr,*rptr;
    DWORD test,ref;

    /* compare 24 bit data */
    for(y = 0; y < height; y++) {
      if(fread(ref_buffer, 1, width*3, reffile) != (size_t)(width*3)) {
	fprintf(stderr,"error reading reference file\n");
	fclose(reffile);
	fclose(testfile);
	exit(-1);
      }
      if(fread(test_buffer, 1, width*3, testfile) != (size_t)(width*3)) {
	fprintf(stderr,"error reading test file\n");
	fclose(reffile);
	fclose(testfile);
	exit(-1);
      }
      rptr = ref_buffer;
      tptr = test_buffer;
      for(x = 0; x < width; x++) {
	ref = (DWORD)*rptr++;
	ref |= ((DWORD)*rptr++)<<8;
	ref |= ((DWORD)*rptr++)<<16;
	test = (DWORD)*tptr++;
	test |= ((DWORD)*tptr++)<<8;
	test |= ((DWORD)*tptr++)<<16;
	if(test != ref) {
	  if(verbose)
	    fprintf(stdout,"data mismatch at (%d,%d): expected %06lx got %06lx\n",
		    x,height-y-1,ref,test);
	  data_error++;
	}
      }
    }
  } else if(ref_depth == 24) {
    /* ref = 24, test = 8 */
    int x,y;
    BYTE *tptr,*rptr;
    BYTE tindex;
    DWORD test,ref;

    /* compare 24 bit reference to 8 bit test data */
    for(y = 0; y < height; y++) {
      if(fread(ref_buffer, 1, width*3, reffile) != (size_t)(width*3)) {
	fprintf(stderr,"error reading reference file\n");
	fclose(reffile);
	fclose(testfile);
	exit(-1);
      }
      if(fread(test_buffer, 1, width, testfile) != (size_t)width) {
	fprintf(stderr,"error reading test file\n");
	fclose(reffile);
	fclose(testfile);
	exit(-1);
      }
      rptr = ref_buffer;
      tptr = test_buffer;
      for(x = 0; x < width; x++) {
	ref = (DWORD)*rptr++;
	ref |= ((DWORD)*rptr++)<<8;
	ref |= ((DWORD)*rptr++)<<16;
	tindex = *tptr++;
	test = test_cmap[tindex];
	if(test != ref) {
	  if(verbose)
	    fprintf(stdout,"data mismatch at (%d,%d): expected %06lx got %06lx\n",
		    x,height-y-1,ref,test);
	  data_error++;
	}
      }
    }
  } else {
    /* ref = 8, test = 24 */
    int x,y;
    BYTE *tptr,*rptr;
    BYTE rindex;
    DWORD test,ref;

    /* compare 8 bit reference to 24 bit test data */
    for(y = 0; y < height; y++) {
      if(fread(test_buffer, 1, width*3, testfile) != (size_t)(width*3)) {
	fprintf(stderr,"error reading test file\n");
	fclose(reffile);
	fclose(testfile);
	exit(-1);
      }
      if(fread(ref_buffer, 1, width, reffile) != (size_t)width) {
	fprintf(stderr,"error reading reference file\n");
	fclose(reffile);
	fclose(testfile);
	exit(-1);
      }
      rptr = ref_buffer;
      tptr = test_buffer;
      for(x = 0; x < width; x++) {
	test = (DWORD)*tptr++;
	test |= ((DWORD)*tptr++)<<8;
	test |= ((DWORD)*tptr++)<<16;
	rindex = *rptr++;
	ref = ref_cmap[rindex];
	if(test != ref) {
	  if(verbose)
	    fprintf(stdout,"data mismatch at (%d,%d): expected %06lx got %06lx\n",
		    x,height-y-1,ref,test);
	  data_error++;
	}
      }
    }
  }

  /* write out terse results */
  if((cmap_error == 0) && (data_error == 0)) {
    printf("<%s> Images Compare\n",argv[1]);
  } else {
    if((ref_depth == 8) && (test_depth == 8)) {
      printf("<%s> Images DO NOT Compare! - CMAP %ld/256 DATA %ld/%ld\n",
	     argv[1],cmap_error,data_error,((DWORD)width)*((DWORD)height));
    } else {
      printf("<%s> Images DO NOT Compare! - DATA %ld/%ld\n",
	     argv[1],data_error,((DWORD)width)*((DWORD)height));
    }
  }

  fclose(reffile);
  fclose(testfile);
  
  exit(0);
}

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

#include <stdio.h>
#include <glide.h>
extern "C" {
#include <fximg.h>
}

void main(int argc, char **argv)
{
  ImgInfo refinfo, testinfo;
  DWORD width, height;
  BYTE verbose;
  DWORD data_error;
  DWORD x,y;
  BYTE *tptr,*rptr;
  DWORD test,ref;
  
  if(argc < 3) {
    fprintf(stderr,"Usage: fxdos <reference file> <test file>\n");
    exit(-1);
  }
  
  if(argc > 3)
    verbose = 1;
  else 
    verbose = 0;
  
  /* open reffile */
  if(!imgReadFile(argv[1],&refinfo)) {
    fprintf(stderr,"reference file <%s> not found\n",argv[1]);
    exit(-1);
  }
  
  /* open testfile */
  if(!imgReadFile(argv[2],&testinfo)) {
    fprintf(stderr,"test file <%s> not found\n",argv[2]);
    free(refinfo.any.data);
    exit(-1);
  }
  
  /* compare size */
  width = refinfo.any.width;
  height = refinfo.any.height;
  
  if(width != testinfo.any.width) {
    fprintf(stderr,"width mismatch - ref %d test %d\n",
	    width,testinfo.any.width);
    free(refinfo.any.data);
    free(testinfo.any.data);
    exit(-1);
  }
  
  if(height != testinfo.any.height) {
    fprintf(stderr,"height mismatch - ref %d test %d\n",
	    width,testinfo.any.height);
    free(refinfo.any.data);
    free(testinfo.any.data);
    exit(-1);
  }
  
  /* set no errors */
  data_error = 0L;
  
  /* compare 24 bit data */
  rptr = refinfo.any.data;
  tptr = testinfo.any.data;
  for(y = 0; y < height; y++) {
    for(x = 0; x < width; x++) {
      ref = (DWORD)*rptr++;
      ref |= ((DWORD)*rptr++)<<8;
      ref |= ((DWORD)*rptr++)<<16;
      test = (DWORD)*tptr++;
      test |= ((DWORD)*tptr++)<<8;
      test |= ((DWORD)*tptr++)<<16;
      if(test != ref) {
	if(verbose) {
	fprintf(stdout,"data mismatch at (%d,%d): expected %06lx got %06lx\n",
		x,height-y-1,ref,test);
	}
	data_error++;
      }
    }
  }

  /* write out terse results */
  if(data_error == 0) {
    printf("<%s> Images Compare\n",argv[1]);
  } else {
    printf("<%s> Images DO NOT Compare! - DATA %ld/%ld\n",
	   argv[1],data_error,((DWORD)width)*((DWORD)height));
  }
  
  free(refinfo.any.data);
  free(testinfo.any.data);
  
  exit(0);
}

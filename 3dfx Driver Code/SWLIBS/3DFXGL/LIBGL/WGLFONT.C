/* 
** Copyright (c) 1997, 3Dfx Interactive, Inc. 
** All Rights Reserved. 
** 
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.; 
** the contents of this file may not be disclosed to third parties, copied or 
** duplicated in any form, in whole or in part, without the prior written 
** permission of 3Dfx Interactive, Inc. 
** 
** RESTRICTED RIGHTS LEGEND: 
** Use, duplication or disclosure by the Government is subject to restrictions 
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data 
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or 
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished - 
** rights reserved under the Copyright Laws of the United States. 
** 
** 
** 
*/ 
#include <windows.h>
#include <glide.h>
#include <math.h>
#include <GL/gl.h>
#include "glint.h"

#include <stdio.h>

BOOL  APIENTRY wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, 
				  DWORD listBase)
{
  static DWORD bitsize = 0;
  static DWORD *bitbucket = NULL;

  int glyph;
  DWORD cursize;
  GLYPHMETRICS metrics;

  for(glyph = 0;glyph < (int)count; glyph++) {
    cursize = GetGlyphOutline(hdc, (UINT)(count+glyph), GGO_BITMAP,
			      &metrics,0,NULL,NULL);
    if(bitsize == 0) {
      bitsize = cursize;
      bitbucket = (DWORD *)malloc(bitsize*4);
    } else {
      if(cursize > bitsize) {
	free(bitbucket);
	bitsize = cursize;
	bitbucket = (DWORD *)malloc(bitsize*4);
      }
    }

    cursize = GetGlyphOutline(hdc, (UINT)(count+glyph), GGO_BITMAP,
			      &metrics,bitsize,bitbucket,NULL);
  }

  return(GL_FALSE);
}

BOOL  APIENTRY wglUseFontBitmapsW(HDC hdc, DWORD first, DWORD count, 
				  DWORD listBase)
{
  return(wglUseFontBitmapsA(hdc, first, count, listBase));
}

BOOL  APIENTRY wglUseFontOutlinesA(HDC hdc, DWORD first, DWORD count,
				   DWORD listBase, FLOAT deviation,
				   FLOAT extrusion, int format,
				   LPGLYPHMETRICSFLOAT lpgmf)
{
  return(GL_FALSE);
}

BOOL  APIENTRY wglUseFontOutlinesW(HDC hdc, DWORD first, DWORD count,
				   DWORD listBase, FLOAT deviation,
				   FLOAT extrusion, int format,
				   LPGLYPHMETRICSFLOAT lpgmf)
{
  return(wglUseFontOutlinesA(hdc, first, count, listBase, deviation,
			     extrusion, format, lpgmf));
}

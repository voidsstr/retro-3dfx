/*
 * Pixel related utilities functions for texture tests.
 *
 * $Id: pixutil.c,v 1.1 1997/02/05 08:05:16 pho Exp $
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bstring.h>
#include <math.h>
#include "ogtst.h"

int
ogLibBytesPerComp(GLenum type)
{
    switch (type) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
	return 1;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
	return 2;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	return 4;
    }
    return 0;
}

int
ogLibCompsPerPixel(GLenum format)
{
    switch (format) {
      case GL_RED: 		
      case GL_GREEN: 		
      case GL_BLUE: 		
      case GL_ALPHA:
      case GL_COLOR_INDEX: 	
      case GL_LUMINANCE:
 	return 1;
      case GL_LUMINANCE_ALPHA:
	return 2;
      case GL_RGB:
	return 3;
      case GL_RGBA: 		
#ifdef GL_EXT_texture
      case GL_ABGR_EXT:
#endif
	return 4;
    }
    return 0;
}

int
ogLibImageSize(int width, int height, GLenum format, GLenum type)
{
    return width * height *
        ogLibCompsPerPixel(format) * ogLibBytesPerComp(type);
}

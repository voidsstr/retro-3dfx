/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
*/
#include <ddraw.h>

#include "texture.h"
#include "texfmt.h"
#include "ddtexmgr.h"
#include "ddtexfmt.h"



/*****************************************************************/
/*                  Texture internal formats --> RGBA8           */
/*****************************************************************/

__GLDDrawTextureFormat __glDDrawTexFormatLuminance = {
    __glDDTexMgrSlurpLuminanceImage,
    __glDDTexMgrSlurpLuminanceSubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatLuminanceAlpha = {
    __glDDTexMgrSlurpLuminanceAlphaImage,
    __glDDTexMgrSlurpLuminanceAlphaSubImage,
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};


__GLDDrawTextureFormat __glDDrawTexFormatRGB = {
    __glDDTexMgrSlurpRGBImage,
    __glDDTexMgrSlurpRGBSubImage,

    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGB332 = {
    __glDDTexMgrSlurpRGB332Image,
    __glDDTexMgrSlurpRGB332SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGB5 = {
    __glDDTexMgrSlurpRGB5Image,
    __glDDTexMgrSlurpRGB5SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGB565 = {
    __glDDTexMgrSlurpRGB565Image,
    __glDDTexMgrSlurpRGB565SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGBA = {
    __glDDTexMgrSlurpRGBAImage,
    __glDDTexMgrSlurpRGBASubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGBA4 = {
    __glDDTexMgrSlurpRGBA4Image,
    __glDDTexMgrSlurpRGBA4SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatARGB4 = {
    __glDDTexMgrSlurpARGB4Image,
    __glDDTexMgrSlurpARGB4SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};


__GLDDrawTextureFormat __glDDrawTexFormatRGB5_A1 = {
    __glDDTexMgrSlurpRGBA5_1Image,
    __glDDTexMgrSlurpRGBA5_1SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatARGB5_1 = {
    __glDDTexMgrSlurpRGBA5_1Image,
    __glDDTexMgrSlurpRGBA5_1SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};


__GLDDrawTextureFormat __glDDrawTexFormatAlpha = {
    __glDDTexMgrSlurpAlphaImage,
    __glDDTexMgrSlurpAlphaSubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatIntensity = {
    __glDDTexMgrSlurpIntensityImage,
    __glDDTexMgrSlurpIntensitySubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatColorIndex8 = {
    __glDDTexMgrSlurpColorIndex8Image,
    __glDDTexMgrSlurpColorIndex8SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatColorIndex16 = {
    __glDDTexMgrSlurpColorIndex16Image,
    __glDDTexMgrSlurpColorIndex16SubImage,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};




/*****************************************************************/
/*                  Texture internal formats -> BGRA8            */
/*****************************************************************/

__GLDDrawTextureFormat __glDDrawTexFormatLuminanceBGRA8 = {
    __glDDTexMgrSlurpLuminanceImageBGRA8,
    __glDDTexMgrSlurpLuminanceSubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatLuminanceAlphaBGRA8 = {
    __glDDTexMgrSlurpLuminanceAlphaImageBGRA8,
    __glDDTexMgrSlurpLuminanceAlphaSubImageBGRA8,
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};


__GLDDrawTextureFormat __glDDrawTexFormatRGB8BGRA8 = {
    __glDDTexMgrSlurpRGBImageBGRA8,
    __glDDTexMgrSlurpRGBSubImageBGRA8,

    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGB332BGRA8 = {
    __glDDTexMgrSlurpRGB332ImageBGRA8,
    __glDDTexMgrSlurpRGB332SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGB5BGRA8 = {
    __glDDTexMgrSlurpRGB5ImageBGRA8,
    __glDDTexMgrSlurpRGB5SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGB565BGRA8 = {
    __glDDTexMgrSlurpRGB565ImageBGRA8,
    __glDDTexMgrSlurpRGB565SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGBA8BGRA8 = {
    __glDDTexMgrSlurpRGBAImageBGRA8,
    __glDDTexMgrSlurpRGBASubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatRGBA4BGRA8 = {
    __glDDTexMgrSlurpRGBA4ImageBGRA8,
    __glDDTexMgrSlurpRGBA4SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatARGB4BGRA8 = {
    __glDDTexMgrSlurpARGB4ImageBGRA8,
    __glDDTexMgrSlurpARGB4SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};


__GLDDrawTextureFormat __glDDrawTexFormatRGB5_A1BGRA8 = {
    __glDDTexMgrSlurpRGBA5_1ImageBGRA8,
    __glDDTexMgrSlurpRGBA5_1SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatARGB5_1BGRA8 ={
    __glDDTexMgrSlurpRGBA5_1ImageBGRA8,
    __glDDTexMgrSlurpRGBA5_1SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};


__GLDDrawTextureFormat __glDDrawTexFormatAlphaBGRA8 = {
    __glDDTexMgrSlurpAlphaImageBGRA8,
    __glDDTexMgrSlurpAlphaSubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatIntensityBGRA8 = {
    __glDDTexMgrSlurpIntensityImageBGRA8,
    __glDDTexMgrSlurpIntensitySubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatColorIndex8BGRA8 = {
    __glDDTexMgrSlurpColorIndex8ImageBGRA8,
    __glDDTexMgrSlurpColorIndex8SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};

__GLDDrawTextureFormat __glDDrawTexFormatColorIndex16BGRA8 = {
    __glDDTexMgrSlurpColorIndex16ImageBGRA8,
    __glDDTexMgrSlurpColorIndex16SubImageBGRA8,
 
    {
        sizeof(DDPIXELFORMAT),
        DDPF_RGB,
        0,
        32,
        0x00ff0000,
        0x0000ff00,
        0x000000ff,
        0xff000000
    }
};










/*****************************************************************/
/*                       Texture formats                         */
/*****************************************************************/

__GLDDrawTextureFormat *
__glLookupDDrawTextureFormat(GLint internalStorageFormat)
{
    switch(internalStorageFormat) {
      case __GL_FORMAT_LUMINANCE8:
	  return &__glDDrawTexFormatLuminance;
      case __GL_FORMAT_LUMINANCE_ALPHA8:
	  return &__glDDrawTexFormatLuminanceAlpha;
      case __GL_FORMAT_RGB8:
	  return &__glDDrawTexFormatRGB;
      case __GL_FORMAT_RGB332:
	  return &__glDDrawTexFormatRGB332;
      case __GL_FORMAT_XRGB1555:
	  return &__glDDrawTexFormatRGB5;
      case __GL_FORMAT_RGB565:
	  return &__glDDrawTexFormatRGB565;
      case __GL_FORMAT_RGBA8:
	  return &__glDDrawTexFormatRGBA;
      case __GL_FORMAT_RGBA4:
	  return &__glDDrawTexFormatRGBA4;
      case __GL_FORMAT_ARGB4:
	  return &__glDDrawTexFormatARGB4;
      case __GL_FORMAT_RGBA5551:
	  return &__glDDrawTexFormatRGB5_A1;
      case __GL_FORMAT_ARGB1555:
	  return &__glDDrawTexFormatARGB5_1;
      case __GL_FORMAT_ALPHA8:
	  return &__glDDrawTexFormatAlpha;
      case __GL_FORMAT_INTENSITY8:
	  return &__glDDrawTexFormatIntensity;
      case __GL_FORMAT_COLOR_INDEX8:
	  return &__glDDrawTexFormatColorIndex8;
      case __GL_FORMAT_COLOR_INDEX16:
	  return &__glDDrawTexFormatColorIndex16;
      default:
      case __GL_FORMAT_VENDOR_SPECIFIC:
	  return NULL;
    }
}

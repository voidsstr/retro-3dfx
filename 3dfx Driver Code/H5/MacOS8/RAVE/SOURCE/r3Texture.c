/*________________________________________________________________________________________
** 
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
**________________________________________________________________________________________
**
**  Description: 
**
** 
**
*/
//: Contributions by 
//: alt.drivers inc.
//: Glenn Nissen
//: Vadim Kochubievski
//: Rohit Gundecha


#include <stdlib.h>
#include <string.h>

#include "r3Tweaks.h"
#include "r3Core.h"
#include "r3Texture.h"

#include <gl.h>

#define ONE_MEGABYTE 0x00100000

#define MAX_TEXTURE_LOD (gls_2k_texture_size ? GR_LOD_LOG2_2048 : GR_LOD_LOG2_256)

#define MAX_ASPECT_RATIO 3

#pragma mark -
#pragma mark /* Global Vars */


extern TRvInfo	gRvEngInfo;

// now goes up to GR_TEXFMT_ARGB_8888  0x12 (see g3ext.h)
static FxU8 bitsPerTexelMap[19] = 
{
   8,  8,  8,  8,  8,  8,  8,  8,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 32, 32
};

long		gEngID;

#pragma mark -
#if RESIDENT_LIST_SANITY_CHECK
#define SANITY_CHECK rvSanityCheckResidentList(inContext, __FILE__, __LINE__)
#else
#define SANITY_CHECK
#endif

static GrTextureFormat_t format[TNSL_PIXEL_FORMAT_COUNT] = {
	0xFF,
	GR_TEXFMT_ARGB_1555,
	GR_TEXFMT_ARGB_1555,
	GR_TEXFMT_ARGB_1555,
	GR_TEXFMT_ARGB_4444,
	GR_TEXFMT_P_8,
	GR_TEXFMT_ALPHA_8
};

void (*BltMipMap[TNSL_PIXEL_FORMAT_COUNT])(void*, TQAImage*, UInt32) = {
	NULL,
	BltMipMap_16ARGBto16ARGB,
	BltMipMap_16ARGBto16ARGB,
	BltMipMap_32RGBto16RGB,
	BltMipMap_32ARGBto16ARGB,
	BltMipMap_CL4toCL8,
	BltMipMap_CL8toCL8
};

UInt8 RaveBpp( UInt32 raveDestFormat );
static char * RaveFormatString( TQAImagePixelType raveFormat );

#pragma mark /* static functions */

static UInt32 NextPowerOfTwo( UInt32 v )
{
    if ( v <=    1 )  return    1;
    if ( v <=    2 )  return    2;
    if ( v <=    4 )  return    4;
    if ( v <=    8 )  return    8;
    if ( v <=   16 )  return   16;
    if ( v <=   32 )  return   32;
    if ( v <=   64 )  return   64;
    if ( v <=  128 )  return  128;
    if ( v <=  256 )  return  256;
    if ( v <=  512 )  return  512;
    if ( v <= 1024 )  return 1024;
    else              return 2048;
}
/*
________________________________________________________________________________________

      CheckLOD
________________________________________________________________________________________

*/

static GrLOD_t CheckLOD(GLsizei size)
{
	switch (size) 
	{
	case 1:
		return GR_LOD_LOG2_1;
	case 2:
		return GR_LOD_LOG2_2;
	case 4:
		return GR_LOD_LOG2_4;
	case 8:
		return GR_LOD_LOG2_8;
	case 16:
		return GR_LOD_LOG2_16;
	case 32:
		return GR_LOD_LOG2_32;
	case 64:
		return GR_LOD_LOG2_64;
	case 128:
		return GR_LOD_LOG2_128;
	case 256:
		return GR_LOD_LOG2_256;
	case 512:
		return GR_LOD_LOG2_512;
	case 1024:
		return GR_LOD_LOG2_1024;
	case 2048:
		return GR_LOD_LOG2_2048;
	default:
		return -1;
	}
}

static char * RaveFormatString( TQAImagePixelType raveFormat )
{
	switch (raveFormat) {
	case kQAPixel_Alpha1: 		return "kQAPixel_Alpha1"; break;
	case kQAPixel_RGB16:		return "kQAPixel_RGB16"; break;
	case kQAPixel_ARGB16:		return "kQAPixel_ARGB16"; break;
	case kQAPixel_RGB32:		return "kQAPixel_RGB32"; break;
	case kQAPixel_ARGB32:		return "kQAPixel_ARGB32"; break;
	case kQAPixel_CL4:			return "kQAPixel_CL4"; break;
	case kQAPixel_CL8:			return "kQAPixel_CL8"; break;
	case kQAPixel_RGB16_565:	return "kQAPixel_RGB16_565"; break;
	case kQAPixel_RGB24:		return "kQAPixel_RGB24"; break;
	case kQAPixel_RGB8_332:		return "kQAPixel_RGB8_332"; break;
	case kQAPixel_ARGB16_4444:	return "kQAPixel_ARGB16_4444"; break;
	default: 					return ""; break;
	}
}

static char * GlideFormatString( GLenum glideFormat )
{
	switch (glideFormat) {
	case GR_TEXFMT_8BIT:					return  "GR_TEXFMT_8BIT"; break;
	case GR_TEXFMT_YIQ_422:             	return  "GR_TEXFMT_YIQ_422"; break;
	case GR_TEXFMT_ALPHA_8:               	return  "GR_TEXFMT_ALPHA_8"; break;
	case GR_TEXFMT_INTENSITY_8:           	return  "GR_TEXFMT_INTENSITY_8"; break;
	case GR_TEXFMT_ALPHA_INTENSITY_44:    	return  "GR_TEXFMT_ALPHA_INTENSITY_44"; break;
	case GR_TEXFMT_P_8:                  	return  "GR_TEXFMT_P_8"; break;
	case GR_TEXFMT_RSVD0:                	return  "GR_TEXFMT_RSVD0"; break;
	case GR_TEXFMT_RSVD1:               	return  "GR_TEXFMT_RSVD1"; break;
	case GR_TEXFMT_16BIT:                	return  "GR_TEXFMT_16BIT"; break;
	case GR_TEXFMT_AYIQ_8422:             	return  "GR_TEXFMT_AYIQ_8422"; break;
	case GR_TEXFMT_RGB_565:               	return  "GR_TEXFMT_RGB_565"; break;
	case GR_TEXFMT_ARGB_1555:             	return  "GR_TEXFMT_ARGB_1555"; break;
	case GR_TEXFMT_ARGB_4444:             	return  "GR_TEXFMT_ARGB_4444"; break;
	case GR_TEXFMT_ARGB_8888:				return  "GR_TEXFMT_ARGB_8888"; break;
	case GR_TEXFMT_ALPHA_INTENSITY_88:    	return  "GR_TEXFMT_ALPHA_INTENSITY_88"; break;
	default: 								return  "";
	}
}

/*
________________________________________________________________________________________

RAVEToGlideTextureFormat
________________________________________________________________________________________

RAVEToGlideTextureFormat maps Rave formats to Glide formats

*/

// translate from kQAPixel_Alpha1 to GR_TEXFMT_ALPHA_8
static void TranslateRow_Alpha1(const void * source, void * dest, int pixelCount)
{
	const unsigned char * s = source;
	unsigned char * d = dest;
	int i;
	
	unsigned char mask = 0x80;
	
	for(i = 0; i < pixelCount; i++){
		if(*s & mask){
			*d++ = 0xFF;
		} else {
			*d++ = 0x00;
		}
		mask >>= 1;
		if(mask == 0){
			mask = 0x80;
			s++;
		}
	}
}

// translate from kQAPixel_RGB16 to GR_TEXFMT_ARGB_1555
static void TranslateRow_RGB16(const void * source, void * dest, int pixelCount)
{
	const unsigned short * s = source;
	unsigned short * d = dest;
	int i;
		
	for(i = 0; i < pixelCount; i++){
		*d++ = *s++ | 0x8000;
	}
}

// translate from kQAPixel_ARGB16 to GR_TEXFMT_ARGB_1555
// translate from kQAPixel_RGB16_565 to GR_TEXFMT_RGB_565
// translate from kQAPixel_ARGB16_4444 to GR_TEXFMT_ARGB_4444
// translate from kQAPixel_ACL16_88 to GR_TEXFMT_AP_88
// translate from kQAPixel_AI16_88 to GR_TEXFMT_ALPHA_INTENSITY_88
static void TranslateRow_16Bit(const void * source, void * dest, int pixelCount)
{
	memcpy(dest, source, pixelCount * 2);
}

// translate from kQAPixel_RGB32 to GR_TEXFMT_RGB_565
static void TranslateRow_RGB32_to_565(const void * source, void * dest, int pixelCount)
{
	const unsigned char * s = source;
	unsigned short * d = dest;
	int i;
	
	unsigned short r, g, b;
		
	for(i = 0; i < pixelCount; i++){
		s++;
		r = *s++;
		g = *s++;
		b = *s++;
		*d++ =
			r << 8 & 0xf800 |
			g << 3 & 0x07e0 |
			b >> 3 & 0x001f;
	}
}

static void TranslateRow_ARGB32(const void * source, void * dest, int pixelCount)
{
	memcpy(dest, source, pixelCount * 4);
}

static void TranslateRow_32Bit_NoAlpha(const void * source, void * dest, int pixelCount)
{
	int i;
	UInt32 quiz = 0;
	UInt32 *s = (UInt32 *)source;
	UInt32 *d = (UInt32 *)dest;
	
	/*
	if (pixelCount % 4 == 0)
		// unroll the loop for speed
		for (i = 0; i < pixelCount; i += 4)
		{
			quiz = *d++ = 0xFF000000 | (*s++ & 0x00FFFFFF);
			quiz = *d++ = 0xFF000000 | (*s++ & 0x00FFFFFF);
			quiz = *d++ = 0xFF000000 | (*s++ & 0x00FFFFFF);
			quiz = *d++ = 0xFF000000 | (*s++ & 0x00FFFFFF);
		}
	else
	*/
		for (i = 0; i < pixelCount; i++)
		{
			quiz = *d++ = 0xFF000000 | (*s++ & 0x00FFFFFF);
		}
}


// translate from kQAPixel_ARGB32 to GR_TEXFMT_ARGB_4444
static void TranslateRow_ARGB32_to_4444(const void * source, void * dest, int pixelCount)
{
	const unsigned char * s = source;
	unsigned short * d = dest;
	int i;
	
	unsigned short a, r, g, b;
		
	for(i = 0; i < pixelCount; i++){
		a = *s++ & 0xF0;
		r = *s++ & 0xF0;
		g = *s++ & 0xF0;
		b = *s++ & 0xF0;
		*d++ =
			a << 8 |
			r << 4 |
			g |
			b >> 4;
	}
}

// translate from kQAPixel_CL4 to GR_TEXFMT_P_8
static void TranslateRow_CL4(const void * source, void * dest, int pixelCount)
{
	const unsigned char * s = source;
	unsigned char * d = dest;
	int i;
	
	unsigned char mask = 0x80;
	
	for(i = 0; i < pixelCount; i += 2){
		*d++ = *s >> 4;
		*d++ = *s++ & 0x0F;
	}
}

// translate from kQAPixel_CL8 to GR_TEXFMT_P_8
// translate from kQAPixel_RGB8_332 to GR_TEXFMT_RGB_332
// translate from kQAPixel_I8 to GR_TEXFMT_INTENSITY_8
static void TranslateRow_8Bit(const void * source, void * dest, int pixelCount)
{
	memcpy(dest, source, pixelCount);
}

// translate from kQAPixel_RGB24 to GR_TEXFMT_RGB_565
static void TranslateRow_RGB24(const void * source, void * dest, int pixelCount)
{
	const unsigned char * s = source;
	unsigned short * d = dest;
	int i;
	
	unsigned short r, g, b;
		
	for(i = 0; i < pixelCount; i++){
		r = *s++;
		g = *s++;
		b = *s++;
		*d++ =
			r << 8 & 0xf800 |
			g << 3 & 0x07e0 |
			b >> 3 & 0x001f;
	}
}

typedef void (*TranslateRowFunc)(const void * source, void * dest, int pixelCount);


static TranslateRowFunc GetTranslateRowFunction(TQAImagePixelType ravePixelFormat)
{
	DebugStr("\nTranslate "); DebugNum(ravePixelFormat); DebugStr(" to ");
	switch(ravePixelFormat){
	case kQAPixel_Alpha1:
		DebugStr("Alpha1 \n");
		return TranslateRow_Alpha1;

	case kQAPixel_RGB16:
		DebugStr("RGB16 \n");
		return TranslateRow_RGB16;

	default:	// this is just to quit the compiler... this should not ever happen
	case kQAPixel_ARGB16:
	case kQAPixel_RGB16_565:
	case kQAPixel_ARGB16_4444:
	case kQAPixel_ACL16_88:
	case kQAPixel_AI16_88:
		DebugStr("16Bit \n");
		return TranslateRow_16Bit;

	case kQAPixel_RGB32:
		DebugStr("RGB32 \n");
		if (NAPALM_AND_BEYOND)
			return TranslateRow_32Bit_NoAlpha;
		else
			return TranslateRow_RGB32_to_565;

	case kQAPixel_ARGB32:
		DebugStr("ARGB32 \n");
		if (NAPALM_AND_BEYOND)
			return TranslateRow_ARGB32;
		else
			return TranslateRow_ARGB32_to_4444;

	case kQAPixel_CL4:
		DebugStr("CL4 \n");
		return TranslateRow_CL4;

	case kQAPixel_CL8:
	case kQAPixel_RGB8_332:
	case kQAPixel_I8:
		DebugStr("8Bit \n");
		return TranslateRow_8Bit;

	case kQAPixel_RGB24:
		DebugStr("RGB24 \n");
		return TranslateRow_RGB24;
	}
}


static GrTextureFormat_t RAVEToGlideTextureFormat(TQAImagePixelType ravePixelFormat)
{

	switch (ravePixelFormat){
	case kQAPixel_Alpha1:
		return GR_TEXFMT_ALPHA_8;
	case kQAPixel_RGB16:
		return GR_TEXFMT_ARGB_1555;
	case kQAPixel_ARGB16:
		return GR_TEXFMT_ARGB_1555;
	case kQAPixel_RGB32:
		if (NAPALM_AND_BEYOND)
			return GR_TEXFMT_ARGB_8888;
		else
			return GR_TEXFMT_RGB_565;
	case kQAPixel_ARGB32:
		if (NAPALM_AND_BEYOND)
			return GR_TEXFMT_ARGB_8888;
		else
			return GR_TEXFMT_ARGB_4444;
	case kQAPixel_CL4:
		return GR_TEXFMT_P_8;
	case kQAPixel_CL8:
		return GR_TEXFMT_P_8;
	case kQAPixel_RGB16_565:
		return GR_TEXFMT_RGB_565;
	case kQAPixel_RGB24:
		return GR_TEXFMT_RGB_565;
	case kQAPixel_RGB8_332:
		return GR_TEXFMT_RGB_332;
	case kQAPixel_ARGB16_4444:
		return GR_TEXFMT_ARGB_4444;
	case kQAPixel_ACL16_88:
		return GR_TEXFMT_AP_88;
	case kQAPixel_I8:
		return GR_TEXFMT_INTENSITY_8;
	case kQAPixel_AI16_88:
		return GR_TEXFMT_ALPHA_INTENSITY_88;
	case kQAPixel_YUVS:
		return -1;
	case kQAPixel_YUVU:
		return -1;
	case kQAPixel_YVYU422:
		return -1;
	case kQAPixel_UYVY422:
		return -1;
	}

	return -1;	// unknown format
}

/*
________________________________________________________________________________________

      CreateTexStructInList
________________________________________________________________________________________

*/

static TQATexture * CreateTexStructInList(void)
{
	TQATexture * theTexture;
	
	theTexture = (TQATexture *) AllocPtr(sizeof(TQATexture));
	if( theTexture == NULL) {
		// TBFL RvSetError( inContext, GL_OUT_OF_MEMORY);
		DebugStr("\nrvCreateTexture ? Out of mem for new TQATexture\n");
		return NULL;
	}
	gRvEngInfo.textureCount++;

	
//_____ Init the texture state

	memset(theTexture, 0, sizeof( TQATexture ));
//_____ Add the new texture to table
	theTexture->safetyCheck = 'txtr';
	theTexture->belongsToBitmap = false;
	theTexture->next = gRvEngInfo.textureList;
	gRvEngInfo.textureList = theTexture;
	
	return theTexture;
}


int IsValidTexture(TQATexture * theTexture)
{
	if(theTexture == NULL) 
		return false;
	if(theTexture->safetyCheck != 'txtr') 
		return false;
	#if TNSL_DEBUG
	DebugStr("theTexture->alphaBits="); DebugNum(theTexture->alphaBits);
	assert (theTexture->alphaBits >= 0 && theTexture->alphaBits <= 8);
	#endif 
	return true;
}


/*
 * CheckAlpha
 *
 * is there no alpha, 1-bit, or multi-bit alpha in the texture?
 */
static int CheckAlpha( TQAImagePixelType pixelType )
{
	switch (pixelType) {
		case kQAPixel_Alpha1:			return 1;
		case kQAPixel_RGB16:			return 0;
		case kQAPixel_ARGB16:			return 1;
		case kQAPixel_RGB32:			return 0;
		case kQAPixel_ARGB32:	
					if (NAPALM_AND_BEYOND)
										return 8;
					else
										return 4;		// gets converted to ARGB_4444
		case kQAPixel_CL4:				return 0;
		case kQAPixel_CL8:				return 0;
		case kQAPixel_RGB16_565:		return 0;
		case kQAPixel_RGB24:			return 0;
		case kQAPixel_RGB8_332:			return 0;
		case kQAPixel_ARGB16_4444:		return 4;
		case kQAPixel_ACL16_88:			return 8;
		case kQAPixel_I8:				return 8;
		case kQAPixel_AI16_88:			return 8;
		default:
			assert(0);
			return 0;
			
	}
}
/*
________________________________________________________________________________________

      FreeTextureVRAM
________________________________________________________________________________________

	Note: this simply says it is not resident, but does not actually free any VRAM.
*/

static void FreeTextureVRAM( TQATexture * theTexture)
{
	TQATexture * prev;
	TQATexture * next;
		
	assert( IsValidTexture(theTexture) );

	if(theTexture->resident == false) return;	// texture VRAM already freed

	prev = theTexture->prev_resident;
	next = theTexture->next_resident;
	
	if(prev != NULL){
		prev->next_resident = next;
	} else {
		gRvEngInfo.first_resident_texture = next;
	}

	if(next != NULL){
		next->prev_resident = prev;
	}

	if(gRvEngInfo.latest_resident_texture == theTexture) {
		gRvEngInfo.latest_resident_texture = prev;
	}
	
	theTexture->resident = false;
	theTexture->prev_resident = NULL;
	theTexture->next_resident = NULL;

	SANITY_CHECK;
	return;
}



/*
________________________________________________________________________________________

      ScaleMap
________________________________________________________________________________________

*/

static void ScaleMap(TQATexture *theTexture, 
            GLuint src_width, GLuint src_height, void *src_data,
            GLuint dst_width, GLuint dst_height, void *dst_data, GrTextureFormat_t format)
{
#pragma unused (theTexture, dst_width, dst_height)
  FxU8 *src0, *src1, *src2, *src3;
  FxU8 *dst;
  FxU32 vOffset, hOffset, bytesPerTexel, x, y;
  
  DebugStr("gggg ScaleMap\n");
  /* Horizontal byte offset from one 2x2 block to the next */
  hOffset = bitsPerTexelMap[format] >> 2;
  bytesPerTexel = hOffset >> 1;
  
  /* Vertical offset from one row to the next */
  vOffset = (bitsPerTexelMap[format] >> 3) * src_width;
  dst = (FxU8 *)dst_data; 
  
  for (y = 0; y < src_height; y+= 2)
  {
    src0 = (FxU8 *)src_data + y * vOffset;
    src1 = src0 + hOffset;
    src2 = src0 + vOffset;
    src3 = src2 + hOffset;
     
    /* Collapse pointers for width || height == 1 */
    if (src_width == 1)  {src1 = src0; src3 = src2; } 
    if (src_height == 1) {src2 = src0; src3 = src1; }

    switch(format) {
      case GR_TEXFMT_INTENSITY_8:
      case GR_TEXFMT_ALPHA_8:
      case GR_TEXFMT_ALPHA_INTENSITY_88:
      {
        for(x = 0; x < src_width * bytesPerTexel; x+= 2)
        {
          FxU32 tmp;
          tmp = *src0; tmp += *src0; tmp += *src2; tmp += *src3;
          src0 += 2; src1 += 2; src2 += 2; src3 += 2;
          *dst++ = (FxU8)(tmp >> 2);
        }
      }
      break;
      
      case GR_TEXFMT_RGB_565:
      {
        FxU16 srcTex, dstTex;
        FxU32 r, g, b;
        for(x = 0; x < src_width; x += 2)
        {
          srcTex = *((FxU16 *)src0);
          r = srcTex >> 11;
          g = (srcTex >> 5) & 0x3f;
          b = srcTex & 0x1f;
          srcTex = *((FxU16 *)src1);
          r += srcTex >> 11;
          g += (srcTex >> 5) & 0x3f;
          b += srcTex & 0x1f;
          srcTex = *((FxU16 *)src2);
          r += srcTex >> 11;
          g += (srcTex >> 5) & 0x3f;
          b += srcTex & 0x1f;
          srcTex = *((FxU16 *)src3);
          r += srcTex >> 11;
          g += (srcTex >> 5) & 0x3f;
          b += srcTex & 0x1f;
          dstTex = ((r >> 2) << 11) | ((g >> 2) << 5) | (b >> 2);
          *((FxU16 *)dst)++ = dstTex;
          src0 += 4; src1 += 4; src2 += 4; src3 += 4;
          }
      }
      break;

      case GR_TEXFMT_ARGB_1555:
      {
        FxU16 srcTex, dstTex;
        FxU32 a, r, g, b;
        for(x = 0; x < src_width; x += 2)
        {
          srcTex = *((FxU16 *)src0);
          a = srcTex >> 15;
          r = (srcTex >> 10) & 0x1f;
          g = (srcTex >> 5) & 0x1f;
          b = srcTex & 0x1f;
          srcTex = *((FxU16 *)src1);
          a += srcTex >> 15;
          r += (srcTex >> 10) & 0x1f;
          g += (srcTex >> 5) & 0x1f;
          b += srcTex & 0x1f;
          srcTex = *((FxU16 *)src2);
          a += srcTex >> 15;
          r += (srcTex >> 10) & 0x1f;
          g += (srcTex >> 5) & 0x1f;
          b += srcTex & 0x1f;
          srcTex = *((FxU16 *)src3);
          a += srcTex >> 15;
          r += (srcTex >> 10) & 0x1f;
          g += (srcTex >> 5) & 0x3f;
          b += srcTex & 0x1f;
          dstTex = ((a >>2) << 15) | ((r >> 2) << 10) | ((g >> 2) << 5) | (b >> 2);
          *((FxU16 *)dst)++ = dstTex;
          src0 += 4; src1 += 4; src2 += 4; src3 += 4;
          }
      }
      break;

      case GR_TEXFMT_ARGB_4444:
      {
        FxU16 srcTex, dstTex;
        FxU32 a, r, g, b;
        for(x = 0; x < src_width; x += 2)
        {
          srcTex = *((FxU16 *)src0);
          //glr_debug_printf("%04lx",srcTex);
          a = srcTex >> 12;
          r = (srcTex >> 8) & 0xf;
          g = (srcTex >> 4) & 0xf;
          b = srcTex & 0xf;
          srcTex = *((FxU16 *)src1);
          a += srcTex >> 12;
          r += (srcTex >> 8) & 0xf;
          g += (srcTex >> 4) & 0xf;
          b += srcTex & 0xf;
          srcTex = *((FxU16 *)src2);
          a += srcTex >> 12;
          r += (srcTex >> 8) & 0xf;
          g += (srcTex >> 4) & 0xf;
          b += srcTex & 0xf;
          srcTex = *((FxU16 *)src3);
          a += srcTex >> 12;
          r += (srcTex >> 8) & 0xf;
          g += (srcTex >> 4) & 0xf;
          b += srcTex & 0xf;
          dstTex = ((a >> 2) << 12) | ((r >> 2) << 8) | ((g >> 2) << 4) | (b >> 2);
          *((FxU16 *)dst)++ = dstTex;
          src0 += 4; src1 += 4; src2 += 4; src3 += 4;
        }
        //glr_debug_printf("\n");
      }
      break;
      
      default:
        DebugStr("unsupported scale: ");
        DebugNum( format );
        break;
        
    }
  }
}



/*
________________________________________________________________________________________

      InitTextureMem
________________________________________________________________________________________

*/
/* FIXME -- This should have a return value */

static int InitTextureMem(TQADrawPrivate *dp)
{
	int		retStat = 0;
	FxI32 surface_size = 0, bigChunk, halfChunk = 0;
	FxU32 value;
	GrSurface_t texture_surface = NULL;

	GrSurfaceDesc_t		surfaceDesc;

	DebugStr("--> InitTextureMem - \n");
	
	assert (gRvEngInfo.totalVRAMcard > 4 * ONE_MEGABYTE );
	
	if(dp->renderSurface && gRvEngInfo.surface_size == 0 && gRvEngInfo.totalVRAMcard > 0)
	{
		// windowed UMA
	    	
		value = grGet(GR_MEMORY_UMA, sizeof surface_size, &surface_size);
		assert(value == sizeof surface_size);
		// bigChunk = surface_size;
		// 6 MB is a safety factor; TBFL
		bigChunk = gRvEngInfo.totalVRAMcard - gRvEngInfo.VRAMusedForNonTextures - 6 * ONE_MEGABYTE;
		assert (bigChunk < surface_size );
	
		// initialize the texture texture_surface 
		memset( &surfaceDesc, 0, sizeof(surfaceDesc) );
		surfaceDesc.height = 1;
		surfaceDesc.bytesPerPixel = 1;
        surfaceDesc.notifyCallback = RvSurfaceNotify;
        surfaceDesc.userData = dp;
        
		surfaceDesc.width = bigChunk;		
		texture_surface = grSurfaceCreateExt( &surfaceDesc );
       	
       	if (texture_surface == NULL)
       	{
		    /* 
		     * Presently, this scheme gives the first engine
		     * 1/2 + 1/4 + 1/16 = 13/16.
		     */	
		    bigChunk = gRvEngInfo.totalVRAMcard - gRvEngInfo.VRAMusedForNonTextures;
			do {
				bigChunk >>= 1;	
				halfChunk = bigChunk / 2;
				halfChunk +=  bigChunk / 8;
				/* Don't bother when less than a single page */
				if (bigChunk + halfChunk < 4096) break;
				
				surfaceDesc.width = bigChunk + halfChunk;		
				texture_surface = grSurfaceCreateExt( &surfaceDesc );
			} while(texture_surface == NULL);
		}
		if(!texture_surface)
		  return 1;
		
		gRvEngInfo.texture_surface = texture_surface;
		gRvEngInfo.surface_size = bigChunk + halfChunk;

		grSurfaceSetTextureSurfaceExt( GR_TMU0, texture_surface );
		grSurfaceSetTextureSurfaceExt( GR_TMU1, texture_surface );
		DebugStr(" New Global Texture Surface bytes= "); DebugNum( bigChunk + halfChunk ); 
		DebugStr(" VRAMusedForNonTexture= "); DebugNum(gRvEngInfo.VRAMusedForNonTextures); 
		DebugStr(" unused VRAM= "); DebugNum(gRvEngInfo.totalVRAMcard - gRvEngInfo.VRAMusedForNonTextures - bigChunk - halfChunk); 
		DebugStr("\n\n");
	}
	else	// full screen UMA
	{
		UInt32	low, high;
		
 		gRvEngInfo.texture_surface = texture_surface;
	      grGet(GR_MEMORY_TMU, sizeof(gRvEngInfo.surface_size), 
        								(FxI32 *)&gRvEngInfo.surface_size);
    	grGet( GR_MEMORY_UMA, sizeof(gRvEngInfo.surface_size), 
    								(FxI32 *)&gRvEngInfo.surface_size);
    	high = grTexMaxAddress(GR_TMU0);
    	low  = grTexMinAddress(GR_TMU0);
    	// gRvEngInfo.surface_size = high - low;
    	// the above gives garbage; don't know why
    	// Actually of all the calls that are supposed to tell you how much
    	// texture memory you have, only GR_MEMORY_UMA works, but it tells you
    	// the TOTAL, not the texture amt.
    	gRvEngInfo.surface_size -= 1600*1200*2*2;
    	gRvEngInfo.surface_size  = 20 * ONE_MEGABYTE;
    	assert( high - low > 4 * ONE_MEGABYTE );
    	gRvEngInfo.surface_size	 = high - low;			// 8/1
    	DebugStr(" high, low "); DebugNum(high); DebugNum(low);
        DebugStr(" full screen UMA tex surface_size="); DebugNum(gRvEngInfo.surface_size);
	}
		
	return retStat;
}



/*
________________________________________________________________________________________

      AllocateTextureSpaceOnCard
________________________________________________________________________________________

*/

static int AllocateTextureSpaceOnCard(TQADrawPrivate *dp, TQATexture * theTexture)
{
	TQATexture * latest_resident_texture;
	TQATexture * texture2;
	long vramSpace, size, offset;
	FxU32		lowTexMem = 0;
	
	/********/
	if(gRvEngInfo.surface_size == 0)
	{
		if (InitTextureMem(dp) != 0)
			return 1;	
	}
 
    assert(IsValidTexture(theTexture));
        
    lowTexMem = grTexMinAddress( GR_TMU0 );    
    size = theTexture->size;
    
	latest_resident_texture = gRvEngInfo.latest_resident_texture;
	
	if(latest_resident_texture == NULL)
	{
		texture2 = gRvEngInfo.first_resident_texture;

		if (texture2 == NULL)
		{
			// todo
			// what if the texture won't fit...
			theTexture->next_resident = NULL;
			theTexture->prev_resident = NULL;
			theTexture->offset = lowTexMem;
			theTexture->resident = true;

			gRvEngInfo.first_resident_texture = theTexture;
			gRvEngInfo.latest_resident_texture = theTexture;
			SANITY_CHECK;
			return 0;
		}
		offset = 0;
	} 
	else 
	{
		texture2 = latest_resident_texture->next_resident;
		offset = latest_resident_texture->offset + latest_resident_texture->size;

		vramSpace = gRvEngInfo.surface_size - offset;
		if (vramSpace < size)
		{
			DebugStr(" VRAM running out ... ");
			offset = lowTexMem;
			texture2 = gRvEngInfo.first_resident_texture;
			// Since we're starting at the beginning, force the current texture to become
			// the new latest resident.
			latest_resident_texture = NULL;
        }
	}
	
	// delete textures from current point until we have room.
	while( texture2    &&      texture2->offset < offset + size)
	{
		TQATexture *next_texture;
		DebugStr("FreeTextureVRAM to get room for "); DebugNum(size); DebugStr("bytes ");
		next_texture = texture2->next_resident;
		if ( !(texture2->flags & kQATexture_Lock))
		{
			FreeTextureVRAM( texture2);
			DebugStr("FreeTextureVRAM to get room for "); DebugNum(size); DebugStr("bytes ");
		}
		texture2 = next_texture;
	}
	
	// Note, if we walked off the end of the list, that's fine and the following line of code is still okay.
	theTexture->next_resident = texture2;
	theTexture->prev_resident = latest_resident_texture;	
	theTexture->offset = offset;
	theTexture->resident = true;
	
	DebugStr("\nAllocateTextureSpaceOnCard new texture offset in VRAM is "); DebugNum(offset);
	// Fix up previous link for next texture in list (if there is one)
	if(texture2) 
	{
	  texture2->prev_resident = theTexture;
	}
	
	// Fix up next link for the old latest texture if we're not starting out at
	// the beginning of the list.
	if(latest_resident_texture) 
	{
	  latest_resident_texture->next_resident = theTexture;
	}
	
	if(latest_resident_texture == NULL)
	{
		gRvEngInfo.first_resident_texture = theTexture;
	}

	gRvEngInfo.latest_resident_texture = theTexture;
	SANITY_CHECK;
	
	DebugStr("\n");
	return 0;
}



/*
________________________________________________________________________________________

      DownloadSysTexture
________________________________________________________________________________________
	Called only if not already resident.
*/

static Boolean
DownloadSysTexture( TQADrawPrivate *dp, TQATexture *inTexture )
{
	DebugStr("not already resident  DownloadSysTexture ");
	DebugHex( inTexture ); DebugStr("\n");
	
	if (AllocateTextureSpaceOnCard(dp, inTexture) != 0)
		return false;
		
	DebugStr("not resident. grTexDownloadMipMapLevel at offset "); DebugNum( inTexture->offset ); DebugStr("\n");
    assert( inTexture->storage );
    
    HLock(inTexture->storage);
    
    inTexture->glideTextureInfo.data = *inTexture->storage;
    
    grTexDownloadMipMap( GR_TMU0,
        inTexture->offset,
         GR_MIPMAPLEVELMASK_BOTH,
         &inTexture->glideTextureInfo
         );

    inTexture->glideTextureInfo.data = NULL;
    HUnlock(inTexture->storage);

	return true;
}

TQAError CalcTextureSize( const TQAImage **pImage, UInt32 flags, GrLOD_t *outWidthLog2, GrLOD_t *outHeightLog2 )
{
	UInt8 quiz;
	TQAImage *pIm = (TQAImage *)*pImage;
	// note dimensions & calculate aspect ratio
	TQAError err = kQANoErr;

	*outWidthLog2 = CheckLOD(pIm->width);
	*outHeightLog2 = CheckLOD(pIm->height);

	if((*outWidthLog2 < 0) || (*outHeightLog2 < 0)) {
		DebugStr("*** bad (non power of 2) texture size");
		return kQAError;
	}

	if(flags & kQATexture_Mipmap)
	{
		
		DebugStr(" Mipmap "); 
		quiz = MAX_TEXTURE_LOD;
		while(*outWidthLog2 > quiz || *outHeightLog2 > quiz) 
		{
			DebugNum(*outWidthLog2 ); DebugNum(*outHeightLog2 ); DebugStr("/ ");
			// the base mipmap is too big... try the next smaller one
			pIm++;
			DebugStr(" baseImage too big, but smaller mipmap is OK. ");
			*outWidthLog2 = CheckLOD(pIm->width);
			*outHeightLog2 = CheckLOD(pIm->height);
		}
	} 
	else 
	{
		if(*outWidthLog2 > MAX_TEXTURE_LOD || *outHeightLog2 > MAX_TEXTURE_LOD) 
		{
			DebugStr("*** texture too large");
			return kQAError;
		}
	}
	
	// force aspect ratio to valid values
	if(*outWidthLog2 - *outHeightLog2 > 3) 
	{
		*outHeightLog2 = *outWidthLog2 - 3;
		DebugStr("? Extreme aspect ratio error; height adjusted ");
	}
	if(*outHeightLog2 - *outWidthLog2 > 3) 
	{
		*outWidthLog2 = *outHeightLog2 - 3;
		DebugStr("? Extreme aspect ratio error; width adjusted ");
	}
	
	*pImage = pIm;
	return err;
}


TQAError CalcBitmapSize( const TQAImage *pImage, GrLOD_t *outWidthLog2, GrLOD_t *outHeightLog2 )
{
	// note dimensions & calculate aspect ratio; bitmaps do not have to be powers of two
	TQAError err = kQANoErr;

	*outWidthLog2 = CheckLOD( NextPowerOfTwo(pImage->width));
	*outHeightLog2 = CheckLOD( NextPowerOfTwo(pImage->height));

	if(*outWidthLog2 > MAX_TEXTURE_LOD || *outHeightLog2 > MAX_TEXTURE_LOD) 
	{
		DebugStr("*** bitmap too large");
		return kQANotSupported;
	}
	
	// force aspect ratio to valid values
	if(*outWidthLog2 - *outHeightLog2 > 3) *outHeightLog2 = *outWidthLog2 - 3;
	if(*outHeightLog2 - *outWidthLog2 > 3) *outWidthLog2 = *outHeightLog2 - 3;

	if((*outWidthLog2 < 0) || (*outHeightLog2 < 0)) {
		DebugStr("*** bad (non power of 2) bitmap/texture size");
		return kQAParamErr;
	}

	return err;
}

#define MAX_BITMAP_SPLIT_SIZES	3
#define MAX_BITMAP_SPLITS		32

static int  PowersOfTwoSplits( UInt32 inDim, int* outArray )
{
	int n256 = 0, n128 = 0, n64 = 0, rem;
	
	n256 = inDim / 256;
	rem = max( 0, inDim - n256 * 256);
	n128 = max( 0, rem / 128 );
	rem = max( 0, rem - n128 * 128);
	n64 = rem / 64;
	rem = max( 0, rem - n64 * 64 );
	if ( rem > 0 )
	{
		if (n64)
		{
			++n128;
			n64 = 0;
			if (n128 > 1)
			{
				++n256;
				n128 = 0;
			}
		}
		else
		{
			n64 = 1;
		}
	}
	outArray[0] = n256;
	outArray[1] = n128;
	outArray[2] = n64;
	return rem;
}
	
static void CalcGridOfPow2Rects( short n256x, short n128x, short n64x, 
								 short n256y, short n128y, short n64y, Rect *outGrid )
{
	// set values in list of rects
	int x, y;
	Rect	r, *pRect = outGrid;
	
	y = n256y;
	
	while (y--)
	{
		r.top = 256 * (n256y - y - 1);
		r.bottom = r.top + 256;
		r.left = r.right = 0;
		x = n256x;
		
		while (x--)
		{
			r.left = 256 * (n256x - x - 1);
			r.right = r.left + 256;
			*pRect++ = r;
		}
		r.left = 256 * n256x;
		
		if (n128x)
		{
			r.right = r.left + 128;
			*pRect++ = r;
			r.left += 128;
		}
		
		if (n64x)
		{
			r.right = r.left + 64;
			*pRect++ = r;
		}
	}
	y = n128y;
	r.top = 256 * n256y;
	while (y--)
	{
		r.top += 128 * (n128y - y - 1);
		r.bottom = r.top + 128;
		r.left = r.right = 0;
		x = n256x;
		
		while (x--)
		{
			r.left = 256 * (n256x - x - 1);
			r.right = r.left + 256;
			*pRect++ = r;
		}
		r.left = 256 * n256x;
		
		if (n128x)
		{
			r.right = r.left + 128;
			*pRect++ = r;
			r.left += 128;
		}
		
		if (n64x)
		{
			r.right = r.left + 64;
			*pRect++ = r;
		}
	}
	r.top = 256 * n256y + 128 * n128y;
	y = n64y;
	while (y--)
	{
		r.top += 64 * (n64y - y - 1);
		r.bottom = r.top + 64;
		r.left = r.right = 0;
		x = n256x;
		
		while (x--)
		{
			r.left = 256 * (n256x - x - 1);
			r.right = r.left + 256;
			*pRect++ = r;
		}
		r.left = 256 * n256x;
		
		if (n128x)
		{
			r.right = r.left + 128;
			*pRect++ = r;
			r.left += 128;
		}
		
		if (n64x)
		{
			r.right = r.left + 64;
			*pRect++ = r;
		}
	}
}

//////////////////////
//                  //
//  GetAspectRatio  //
//                  //
//////////////////////

static GrAspectRatio_t 
GetAspectRatio(GrLOD_t width, GrLOD_t height)
{
	return width - height;
}


// stretches out a row of pixels horizontally
static void SpewChuncks(unsigned char * chunkBase, int chunkCount, int chunkSize, int repeat)
{
	int i, j;

	unsigned char * s;
	unsigned char * d;
	
	s = chunkBase + (chunkSize * chunkCount);
	d = chunkBase + (chunkSize * chunkCount * repeat);
	
	for(i = 0; i < chunkCount; i++){
		s -= chunkSize;
		for(j = 0; j < repeat; j++){
			d -= chunkSize;
			
			memcpy(d, s, chunkSize);
		}
	}
}



static int TranslateMipMaps(
	TQAImagePixelType pixelType, 
	const TQAImage * baseImage, 
	GrTexInfo * info, 
	unsigned long flags)
{
	unsigned char * destPixels = info->data;
	GrAspectRatio_t ar = info->aspectRatioLog2;

	GrLOD_t   widthLog2;
	GrLOD_t   heightLog2;
	GrLOD_t   lod;
	
	int h, w;
	int row;
	
	int destBytesPerPixel = bitsPerTexelMap[info->format] / 8;
	int destRowBytes;
	
	int stretchWidth;
	int stretchHeight;
	
	unsigned char * destRow;
	unsigned char * sourceRow;
	
	int sourceRowBytes;
	
	TranslateRowFunc TranslateRow;
	
	TranslateRow = GetTranslateRowFunction(pixelType);
	
	if (ar > 0)
	{
		widthLog2 = info->largeLodLog2;
		heightLog2 = info->largeLodLog2 - ar;
	} else {
		widthLog2 = info->largeLodLog2 + ar;
		heightLog2 = info->largeLodLog2;
	}

	destRow = destPixels;

	for(lod = info->largeLodLog2; lod >= info->smallLodLog2; lod--){
		sourceRowBytes = baseImage->rowBytes;
		sourceRow = baseImage->pixmap;
		if(!(flags & kQATexture_FlipOrigin)){
			sourceRow += (baseImage->height - 1) * sourceRowBytes;
			sourceRowBytes = -sourceRowBytes;
		}
		h = 1 << heightLog2;
		w = 1 << widthLog2;
		
		stretchWidth = w / baseImage->width;
		stretchHeight = h / baseImage->height;
	
		destRowBytes = w * destBytesPerPixel;
		
		for(row = 0; row < baseImage->height; row++){
			TranslateRow(sourceRow, destRow, baseImage->width);
			
			if(stretchWidth > 1)
			{
				// stretch horizontally
				SpewChuncks(destRow, baseImage->width, destBytesPerPixel, stretchWidth);
			}
			
			if(stretchHeight > 1)
			{
				// stretch vertically
				int copy;
				for(copy = 1; copy < stretchHeight; copy++)
				{
					memcpy(destRow + destRowBytes, destRow, destRowBytes);
					destRow += destRowBytes;
				}
			}
			
			destRow += destRowBytes;
			sourceRow += sourceRowBytes;
		}
		
		if(widthLog2 == 0 && widthLog2 == 0) break;	// just finished the last (1 pixel x 1 pixel) mipmap
		
		baseImage ++;
		if(widthLog2 > 0) widthLog2--;
		if(heightLog2 > 0) heightLog2--;
	}
	return kQANoErr;
}

/*
 * Used to delete, even when a triangle in the cache needs it.
 */
void 
TextureForceDelete(TQATexture* texture)
{
	texture->cached = FXFALSE;
	RvTextureDelete( texture );
}


#pragma mark -
#pragma mark /* RAVE Entry Points */

//////////////////////
//                  //
//  RvTextureNew  //
//                  //
//////////////////////

/*
create a buffer in system ram to store our glide version of the texture
convert the images from rave format to glide format
*/



TQAError 
RvTextureNew(unsigned long     flags,
               TQAImagePixelType pixelType,
               const TQAImage    images[],
               TQATexture**      textureOut)
{
	TQAError err = kQANoErr;

	GrLOD_t   widthLog2;
	GrLOD_t   heightLog2;
	GrAspectRatio_t aspectRatio;
	GrTextureFormat_t glideTextureFormat;

	UInt32 width = 0;
	UInt32 height = 0;
	SInt32 levels = 0;
	UInt32 size = 0;
	UInt32 bFlipped = 0;
	
	TQATexture * newTexture = NULL;
	const TQAImage *   baseImage = images;
	/*
	enum {
	kQATexture_None				= 0,							// No flags 
	kQATexture_Lock				= (1 << 0),						// Don't swap this texture out 
	kQATexture_Mipmap			= (1 << 1),						/* This texture is mipmapped 
	kQATexture_NoCompression	= (1 << 2),						/* Do not compress this texture
	kQATexture_HighCompression	= (1 << 3),						/* Compress texture, even if it takes a while 
	kQATexture_NonRelocatable	= (1 << 4),						/* Image buffer in VRAM should be non-relocatable 
	kQATexture_NoCopy			= (1 << 5),						/* Don't copy image to VRAM when creating it 
	kQATexture_FlipOrigin		= (1 << 6),						// The image(s) is(are) in a bottom-up format. (The image(s) is(are) flipped vertically.) 
	kQATexture_PriorityBits		= (1 << 31) | (1 << 30) | (1 << 29) | (1 << 28) // Texture priority: 4 upper bits for 16 levels of priority
	*/
	AppTimerEnd();
	TexTimerStart();

	DebugStr("--> RvTextureNew - ");
	if (baseImage == NULL)
		return kQAParamErr;
		
	DebugNum(baseImage->width);
	DebugNum(baseImage->height);
	DebugStr("Rave Pixel Type=");
	DebugStr(RaveFormatString(pixelType));
#if TNSL_DEBUG
	if (flags & kQATexture_Mipmap)
		DebugStr(" Mipmaps ");
#endif

	DebugStr(" ");
	// This identifies which card if there are more than one Voodoo card.
	gEngID = QAGetCurrentEngineRefCon();
		
	if(gRvEngInfo.totalTextureSize > MAX_TEXTUREBYTES_PER_ENGINE)
	{
		err = kQAOutOfVideoMemory;
		DebugStr("\n\n ? Out of Video Memory @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n\n");
		goto bail;
	}
		
	if ( (err = CalcTextureSize( &baseImage, flags, &widthLog2, &heightLog2 )) != kQANoErr)
		goto bail;
				
	aspectRatio = GetAspectRatio(widthLog2, heightLog2);
	
	// check the format
	DebugStr("RAVE Texture Format=");
	DebugStr(RaveFormatString(pixelType));
	
	glideTextureFormat = RAVEToGlideTextureFormat(pixelType);
	
	DebugStr(" Glide Texture Format=");
	DebugStr(GlideFormatString(glideTextureFormat));
	DebugStr(" ");
	
	if(glideTextureFormat == -1) {
		DebugStr("*** unsupported pixel format");
		err = kQAError;
		goto bail;
	}

	// allocates texture
	// initializes to zero
	// sets next & safetyCheck
	// inserts texture into gRvEngInfo.textureList
	newTexture = CreateTexStructInList();
	
	if(newTexture == NULL) 
	{
		DebugStr("\n*** ? new TQATexture failed\n");
		err = kQAOutOfMemory;
		goto bail;
	}

	
	newTexture->glideTextureInfo.largeLodLog2    = (widthLog2 > heightLog2) ? widthLog2 : heightLog2;
	newTexture->glideTextureInfo.smallLodLog2    = (flags & kQATexture_Mipmap) ? GR_LOD_LOG2_1 : newTexture->glideTextureInfo.largeLodLog2;
	newTexture->glideTextureInfo.aspectRatioLog2 = aspectRatio;
	newTexture->glideTextureInfo.format          = glideTextureFormat;
	newTexture->glideTextureInfo.data            = NULL;
	
	size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &newTexture->glideTextureInfo);
	
	{
		OSErr err;
		newTexture->storage = TempNewHandle(size, &err);
		if(err != noErr){
			newTexture->storage = NULL;
		}	
	}
	
	if(newTexture->storage != NULL){
		gRvEngInfo.totalTextureSize += size;
	}
	
	if(newTexture->storage == NULL) 
	{
		DebugStr("*** ? texCreate failed, size=");
		DebugNum(size);
		err = kQAOutOfMemory;
		goto bail;
	}

	// is there alpha in the converted texture?
	newTexture->alphaBits = CheckAlpha( pixelType );
	
	newTexture->flags       		= flags;
	newTexture->pixelType   		= pixelType;
	newTexture->size      		= size;	
	// newTexture->next was assigned in call to CreateTexStructInList
	newTexture->resident		= GL_FALSE;
	newTexture->offset			= 0;
	newTexture->next_resident	= NULL;
	newTexture->prev_resident	= NULL;
		
	newTexture->cached			= FXFALSE;
	newTexture->toBeDeleted		= FXFALSE;

	
	assert( newTexture->glideTextureInfo.largeLodLog2 >= 0 );
	
	if (newTexture->glideTextureInfo.aspectRatioLog2 >= 0)
	{
		newTexture->ratioX      = (FxFloat)256;
		newTexture->ratioY      = (FxFloat)(256 >> newTexture->glideTextureInfo.aspectRatioLog2);
	}
	else
	{
		newTexture->ratioX      = (FxFloat)(256 >> -newTexture->glideTextureInfo.aspectRatioLog2);
		newTexture->ratioY      = (FxFloat)256;
	
	}
	
    HLock(newTexture->storage);
    newTexture->glideTextureInfo.data = *newTexture->storage;
	
	TranslateMipMaps(pixelType, baseImage, &newTexture->glideTextureInfo, flags);
	DebugStr(" size="); DebugNum(size);
    newTexture->glideTextureInfo.data = NULL;
    HUnlock(newTexture->storage);

	DebugHex((int) newTexture );
	
	*textureOut = newTexture;
	
	err = kQANoErr;
	
bail:
	
	if(err){
		if(newTexture){
			RvTextureDelete(newTexture);
		}

		*textureOut = NULL;
	}	

	DebugStr("\n");
	
	TexTimerEnd();
	AppTimerStart();

	return err;
}



/////////////////////////
//                     //
//  RvTextureDetach  //
//                     //
/////////////////////////

TQAError 
RvTextureDetach(TQATexture* texture)
{
	#pragma unused (texture)
	return kQANoErr;
}


/////////////////////////
//                     //
//  RvTextureDelete  //
//                     //
/////////////////////////

void 
RvTextureDelete(TQATexture* texture)
{
	TQATexture		*theTexture, *prev_texture;
	AppTimerEnd();
	TexTimerStart();

	DebugStr("--- RvTextureDelete --- ");
	DebugHex(texture);
	
	gEngID = QAGetCurrentEngineRefCon();

	if(!IsValidTexture(texture) ){
		DebugStr("Attempted to delete non valid texture\n");
		return;
	}
	
	DebugHex(texture);
	
	// if this texture is needed during FlushCache, don't delete now
	// dont really delete the texture yet if it is in use by the triangle cache
	if(texture->cached){
		texture->toBeDeleted = FXTRUE;
		DebugStr(" inTriCache deferred Tex Delete\n");
		goto bail;
	}
	// This only affects the residency status.
	FreeTextureVRAM(texture);
	
	prev_texture = NULL;
	theTexture = gRvEngInfo.textureList;
	
	while (theTexture)
	{
		if (theTexture == texture) break;
		
		prev_texture = theTexture;
		theTexture = theTexture->next;
	}
	
	if (!theTexture)
		return;
		
	if (prev_texture) {
		prev_texture->next = theTexture->next;
	} else {
		gRvEngInfo.textureList = theTexture->next;
	}
		
	if (theTexture->storage)
	{
		gRvEngInfo.totalTextureSize -= theTexture->size;

		DisposeHandle( theTexture->storage);
		theTexture->storage = NULL;
	}
		
	// set safetyCheck to 0 right before we free the memory...
	theTexture->safetyCheck = 0;
	FreePtr( (Ptr)theTexture );
	gRvEngInfo.textureCount--;
bail:
	DebugStr("\n");
	
	TexTimerEnd();
	AppTimerStart();

	DebugStr( " global numDrawContexts="); DebugNum( gRvEngInfo.numDrawContexts );
	DebugStr( "\n global textureCount="); DebugNum( gRvEngInfo.textureCount);
	DebugStr( "\n global totalTextureSize="); DebugNum( gRvEngInfo.totalTextureSize);
	DebugStr( "\n global avail tex VRAM="); DebugNum( max(0, gRvEngInfo.surface_size - gRvEngInfo.totalTextureSize));
	DebugStr( "\n");

	return;
}

/////////////////////
//                 //
//  RvBitmapNew  //
//                 //
/////////////////////

TQAError 
RvBitmapNew(unsigned long     flags,
              TQAImagePixelType pixelType,
              const TQAImage*   image,
              TQABitmap**       newBitmap)
{
	TQAError		err = kQANoErr, sizeErr = kQANoErr;
	GrLOD_t			log2width;
	GrLOD_t			log2height;
	GrAspectRatio_t aspectRatio;
	Boolean 		bMustSplit, bMustPadX, bMustPadY;
	short			remX = 0, remY = 0;
	int				n256x = 0, n128x = 0, n64x = 0, n256y = 0, n128y = 0, n64y = 0, nRects = 0;
	int				numSplitsArray[3];
	int				p = 0;
	Rect			rectPow;
	Rect			rectUnpad[MAX_BITMAP_SPLITS];
	Rect			rectGrid[MAX_BITMAP_SPLITS];
	GrTextureFormat_t glideTextureFormat;
	int 			destBytesPerPixel;
	UInt32 			size = 0;
	UInt8			*pu8src = NULL, *pu8dst = NULL;
	UInt16			*pu16src = NULL, *pu16dst = NULL;
	UInt32			*pu32src = NULL, *pu32dst = NULL;
	TQATexture * 	newTexture = NULL;
	BmPiece 		*prevPiece = NULL;
	BmPiece			*texPieceInfo;

	DebugStr("--> RvBitmapNew - ");
	if (newBitmap == NULL)
		return kQAParamErr;
	if (image == NULL)
		return kQAParamErr;
	DebugNum(image->width);
	DebugNum(image->height);
	if (image->pixmap == NULL)
		return kQAParamErr;
	if (image->width == 0 || image->height == 0 || image->rowBytes == 0)
		return kQAParamErr; 
	rectUnpad[0].top 		= rectUnpad[0].left		= 0;
	rectUnpad[0].right		= image->width;
	rectUnpad[0].bottom		= image->height;
	
	// check the format
	glideTextureFormat = RAVEToGlideTextureFormat(pixelType);
	
	DebugStr("Glide Texture Format=");
	DebugStr(GlideFormatString(glideTextureFormat));
	DebugStr(" ");
	
	gEngID = QAGetCurrentEngineRefCon();

	if(glideTextureFormat == -1) {
		DebugStr("*** unsupported pixel format");
		err = kQAError;
		goto bail;
	}

	*newBitmap = (TQABitmap *) AllocPtr(sizeof(TQABitmap));
	if (*newBitmap == NULL)
	{
		DebugStr("\n ? Out of mem for new TQABitmap\n");
		err = kQAOutOfMemory;
		goto bail;
	}
	
	destBytesPerPixel = bitsPerTexelMap[glideTextureFormat] / 8;

	// This will return kQANotSupported if the bitmap image is too big.
	sizeErr = CalcBitmapSize( image, &log2width, &log2height );
	if (sizeErr == kQANoErr)
			bMustSplit = false;
	else
			bMustSplit = true;
	
	if (bMustSplit)
	{
		// identify number of splits and allocate array of data for splits	
		bMustPadX = remX = PowersOfTwoSplits( image->width, numSplitsArray );
		n256x = numSplitsArray[0];
		n128x = numSplitsArray[1];
		n64x  = numSplitsArray[2];
		bMustPadY = remY = PowersOfTwoSplits( image->height, numSplitsArray );
		n256y = numSplitsArray[0];
		n128y = numSplitsArray[1];
		n64y  = numSplitsArray[2];
		
		(*newBitmap)->numPieces = nRects = (n256x + n128x + n64x) * (n256y + n128y + n64y);
		DebugStr( "\nGridOfSplits horiz "); DebugNum(n256x); DebugNum(n128x); DebugNum(n64x);
		DebugStr( "  vert "); DebugNum(n256y); DebugNum(n128y); DebugNum(n64y); DebugStr("\n");
		
		CalcGridOfPow2Rects( n256x, n128x, n64x, n256y, n128y, n64y, rectGrid );
	}
	else
	{
		(*newBitmap)->numPieces = nRects = nRects = 1;
		rectGrid[0].top 	= 0;
		rectGrid[0].left	= 0;
		rectGrid[0].right	= 1 << log2width;	// NextPowerOfTwo( image->width ); 
		rectGrid[0].bottom	= 1 << log2height;	// NextPowerOfTwo( image->height ); 
	}
	assert (nRects > 0 );
	
	// allocate memory for data about splits
	texPieceInfo = (BmPiece *) AllocPtr(sizeof(BmPiece) * nRects);
	if (texPieceInfo == NULL)
	{
		DebugStr("\n ? Out of mem for new texPieceInfo\n");
		err = kQAOutOfMemory;
		goto bail;
	}
	
	for ( p = 0; p < nRects; p++)
	{
		texPieceInfo[p].rectPow = rectGrid[p];
		texPieceInfo[p].texPiece = NULL;
	}
			
	(*newBitmap)->pieces = texPieceInfo;
	
	// create one or more texture pieces
	for (p = 0; p < nRects; p++)
	{
		rectPow 	= texPieceInfo[p].rectPow;
		if (bMustSplit)
		{
			log2width 	= CheckLOD( NextPowerOfTwo( rectPow.right - rectPow.left ));
			log2height 	= CheckLOD( NextPowerOfTwo( rectPow.bottom - rectPow.top ));
		}
		aspectRatio = GetAspectRatio(log2width, log2height);
		
		// calculate rect of actual displayed bitmap
		rectUnpad[p]			= rectPow;
		rectUnpad[p].right 		= min( rectPow.right, image->width );
		rectUnpad[p].bottom 	= min( rectPow.bottom, image->height );
		
		// allocates texture
		// initializes to zero
		// sets next & safetyCheck
		// inserts texture into gRvEngInfo.textureList
		newTexture = CreateTexStructInList();
		
		if(newTexture == NULL) 
		{
			DebugStr("\n*** ? new TQATexture failed\n");
			err = kQAOutOfMemory;
			goto bail;
		}
		newTexture->glideTextureInfo.largeLodLog2    = (log2width > log2height) ? log2width : log2height;
		newTexture->glideTextureInfo.smallLodLog2    = newTexture->glideTextureInfo.largeLodLog2;
		newTexture->glideTextureInfo.aspectRatioLog2 = aspectRatio;
		newTexture->glideTextureInfo.format          = glideTextureFormat;
		newTexture->glideTextureInfo.data            = NULL;
		
		size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &newTexture->glideTextureInfo);
		{
			OSErr err;
			newTexture->storage = TempNewHandle(size, &err);
			if(err != noErr)
				newTexture->storage = NULL;
		}
		
		if(newTexture->storage == NULL) 
		{
			DebugStr("*** ? texCreate failed ");
			DebugNum(size);
			err = kQAOutOfMemory;
			goto bail;
		}
		// is there alpha in the converted texture?
		newTexture->alphaBits = CheckAlpha( pixelType );
		newTexture->colorTable = NULL;
		
		newTexture->flags       		= flags;
		newTexture->pixelType   		= pixelType;
		newTexture->size      		= size;	
		// newTexture->next was assigned in call to CreateTexStructInList
		newTexture->resident		= GL_FALSE;
		newTexture->offset			= 0;
		newTexture->next_resident	= NULL;
		newTexture->prev_resident	= NULL;
			
		newTexture->cached			= FXFALSE;
		newTexture->toBeDeleted		= FXFALSE;

		
		assert( newTexture->glideTextureInfo.largeLodLog2 >= 0 );
		
		if (newTexture->glideTextureInfo.aspectRatioLog2 >= 0)
		{
			newTexture->ratioX      = (FxFloat)256;
			newTexture->ratioY      = (FxFloat)(256 >> newTexture->glideTextureInfo.aspectRatioLog2);
		}
		else
		{
			newTexture->ratioX      = (FxFloat)(256 >> -newTexture->glideTextureInfo.aspectRatioLog2);
			newTexture->ratioY      = (FxFloat)256;
		
		}
		
		gRvEngInfo.totalTextureSize += size;
		
		HLock(newTexture->storage);
		newTexture->glideTextureInfo.data = *newTexture->storage;
		
		if (bMustSplit)
		{
			// copy rect of image into new smaller image
			TQAImage	im;	
			int 		v, h;
			long		rowBytesSrc, rowBytesDst, realWidth;
			Ptr			srcPtr, dstPtr, srcRow, dstRow;
			// GDHandle saveDevice;
			// CGrafPtr savePort, newGW;
			// PixMapHandle	pixMap;
	
			Rect		srcRect, dstRect;
			
			im.width	= rectPow.right - rectPow.left;
			im.height	= rectPow.bottom - rectPow.top;
			im.rowBytes = destBytesPerPixel * im.width;
			
			im.pixmap	= AllocPtr( im.width * im.height * destBytesPerPixel);
			
			if (im.pixmap)
			{
				dstRect.top		= 0;
				dstRect.left	= 0;
				dstRect.right	= im.width;
				dstRect.bottom	= im.height;
				srcRect.top		= rectPow.top;
				srcRect.left	= rectPow.left;
				srcRect.right	= rectPow.right;
				srcRect.bottom	= rectPow.bottom;
				
				rowBytesSrc = rowBytesDst = im.rowBytes;
				srcPtr = (Ptr)image->pixmap + rectPow.top * image->rowBytes + rectPow.left * destBytesPerPixel;
				srcRow = srcPtr;
				dstPtr = dstRow = (Ptr)im.pixmap;
				realWidth = (rectUnpad[p].right - rectUnpad[p].left) * destBytesPerPixel;
				
				for (v = 0; v < rectUnpad[p].bottom - rectUnpad[p].top; v++)
				{
					for (h = 0; h < realWidth; h++)	
					{
						*dstPtr++ = *srcPtr++;
					}
					srcPtr = srcRow = srcRow + image->rowBytes;
					dstPtr = dstRow = dstRow + im.rowBytes;
				}
				TranslateMipMaps(pixelType, &im, &newTexture->glideTextureInfo, flags);
				FreePtr( im.pixmap );
				(*newBitmap)->pieces[p].bitmapWidth		= rectUnpad[p].right - rectUnpad[p].left;  // srcImageRealWidth; 	//  im.width;
				(*newBitmap)->pieces[p].bitmapHeight	= rectUnpad[p].bottom - rectUnpad[p].top;	//  im.height;
				(*newBitmap)->pieces[p].bitmapUoverW = (*newBitmap)->pieces[p].bitmapWidth / (float)im.width;
				(*newBitmap)->pieces[p].bitmapVoverW = 1.00;
				(*newBitmap)->pieces[p].bitmapBiasU	= 0;	
				(*newBitmap)->pieces[p].bitmapBiasV	= 1.00 - ((*newBitmap)->pieces[p].bitmapHeight) / (float)im.height;
			}
		}
		else			// not splitting
		{
			TranslateMipMaps(pixelType, image, &newTexture->glideTextureInfo, flags);
			(*newBitmap)->pieces[p].bitmapWidth		= image->width;
			(*newBitmap)->pieces[p].bitmapHeight	= image->height;
			(*newBitmap)->pieces[p].bitmapUoverW = (float)image->width / (rectPow.right - rectPow.left);
			(*newBitmap)->pieces[p].bitmapVoverW = (float)image->height / (rectPow.bottom - rectPow.top);
			(*newBitmap)->pieces[p].bitmapBiasU	= 0.0;	
			(*newBitmap)->pieces[p].bitmapBiasV	= 0.0;
		}

		assert( newTexture->storage );
		assert( newTexture->glideTextureInfo.data == *newTexture->storage );
		// set flag so that mass delete will not hose bitmaps
		newTexture->belongsToBitmap = true;
		
		HUnlock(newTexture->storage);

		DebugHex((int) newTexture );
		assert( newTexture->safetyCheck );	// this passes
		
		(*newBitmap)->pieces[p].texPiece		= newTexture;
		assert(  (*newBitmap)->pieces[p].texPiece );
		assert( ((*newBitmap)->pieces[p].texPiece)->safetyCheck );	
	}	// loop on p
	assert( (*newBitmap)->numPieces > 0 );
	assert( (*newBitmap)->pieces->texPiece );
	// check the first one here
	assert(((*newBitmap)->pieces->texPiece)->safetyCheck );
	
	
bail:
	DebugStr("\n");
	return err;
#pragma unused(flags)
}


////////////////////////
//                    //
//  RvBitmapDetach  //
//                    //
////////////////////////

TQAError 
RvBitmapDetach(TQABitmap* bitmap)
{
	DebugStr("--> RvBitmapDetach - ");
	
	#pragma unused (bitmap)

bail:
	DebugStr("\n");
	return kQANoErr;
}


////////////////////////
//                    //
//  RvBitmapDelete  //
//                    //
////////////////////////

void 
RvBitmapDelete(TQABitmap* bitmap)
{
	int n;
	BmPiece	*bmp;
	
	if (bitmap == NULL)
		return;
		
	DebugStr("--> RvBitmapDelete - ");
	DebugHex(bitmap); DebugStr(" numPieces="); DebugNum(bitmap->numPieces);
	
	bmp = bitmap->pieces;
	gEngID = QAGetCurrentEngineRefCon();

	if (bmp)
	{
		for (n = 0; n < bitmap->numPieces; n++)
		{
			assert( bmp[n].texPiece );
			// fails, but that might be ok;    assert( (bmp[n].texPiece)->safetyCheck );
			RvTextureDelete( bmp[n].texPiece );
		}
		FreePtr( (Ptr)bitmap->pieces );
	}
	FreePtr( (Ptr)bitmap );
	DebugStr("\n");
}



#if 0


// TBFL UNFINISHED !!!!!!!!!!!!

TQAError RvAccessTexture(	TQATexture			*texture,
							long 				mipmapLevel,
							long 				flags,
							TQAPixelBuffer		*buffer )
{

#pragma unused (mipmapLevel, flags, buffer)

	FxBool		ret;
    GrLfbInfo_t lfbInfo;
	
	/* This is a TQAPixelBuffer
	
struct TQADeviceMemory {
	long 							rowBytes;					/* Rowbytes 
	TQAImagePixelType 				pixelType;					/* Depth, color space, etc.
	long 							width;						/* Width in pixels 
	long 							height;						/* Height in pixels 
	void *							baseAddr;					/* Base address of pixmap 
}	
typedef struct {
    int                size;
    void               *lfbPtr;
    FxU32              strideInBytes;        
    GrLfbWriteMode_t   writeMode;
    GrOriginLocation_t origin;
} GrLfbInfo_t

*/
	DebugStr("/n   RvAccessTexture ^&*(#$^&#$%*%^&*%&^*$*%&^(%&*(*(*(*)^*&^&%");
	
	
	assert( IsValidTexture(texture) );
	
	
    lfbInfo.size = sizeof(lfbInfo);
    if(grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
      &lfbInfo)) 
	{
        if(tnsl_surface_ext) 
        {
          // _h3ReadRGBPixelsUByteLfb(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
          //    lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
        } 
        else 
        {
          //  _h3ReadRGBPixelsUByte16(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
          //    lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
    }

	
	return( kQANoErr );
}



TQAError RvAccessTextureEnd(TQATexture			*texture,
							const TQARect		*dirtyRect );
TQAError RvAccessTextureEnd(TQATexture			*texture,
							const TQARect		*dirtyRect )
{
#pragma unused (texture, dirtyRect)
	DebugStr( "RvAccessTextureEnd" );
	
	return( kQANoErr );
}

#endif


#pragma mark -
#pragma mark /* OpenGL driver functions */


/*
________________________________________________________________________________________

      EradicateTextures
________________________________________________________________________________________

*/

void
EradicateTextures( void  )
{
	DebugStr( " EradicateTextures textureList ");
	DebugStr( "\n global numDrawContexts="); DebugNum( gRvEngInfo.numDrawContexts );
	DebugStr( "\n global textureCount="); DebugNum( gRvEngInfo.textureCount);
	DebugStr( "\n global totalTextureSize="); DebugNum( gRvEngInfo.totalTextureSize);
	DebugStr( "\n");
	while( gRvEngInfo.textureList )
	{
			TextureForceDelete( gRvEngInfo.textureList );
	}

}

/*________________________________________________________________________________________

      FreeAllNormalTextures
________________________________________________________________________________________

*/

void
FreeAllNormalTextures(  )
{
	TQATexture *ptex = gRvEngInfo.textureList;
	DebugStr( " FreeAllNormalTextures textureList ");
	DebugStr( "\n global numDrawContexts="); DebugNum( gRvEngInfo.numDrawContexts );
	DebugStr( "\n global textureCount="); DebugNum( gRvEngInfo.textureCount);
	DebugStr( "\n global totalTextureSize="); DebugNum( gRvEngInfo.totalTextureSize);
	DebugStr( "\n");
	while( ptex )
	{
		assert( ptex->alphaBits == 0 ||
				ptex->alphaBits == 1 ||
				ptex->alphaBits == 4 ||
				ptex->alphaBits == 8 );
				
		if (ptex->belongsToBitmap || (ptex->flags & kQATexture_Lock))
		{
			ptex = ptex->next;	// skip over it
		}
		else
		{
			TextureForceDelete(ptex);
			ptex  = gRvEngInfo.textureList;
		}
	}
}


FxBool LoadTextureAndTableToVRAM(TQADrawPrivate	*dp, TQATexture *theTexture )
{
	
	if(!IsValidTexture(theTexture))
	{
		DebugStr(" Texture is NOT VALID\n");
		return false;
	}
	    
	/* Load textures into hardware */
	
	if(!theTexture->resident) {		
		if (!DownloadSysTexture(dp, theTexture))
			return false;
	}
	// we don't necessaily need to call grTexSource each time... could be that the TMU is already pointing at this texture... 
	if ( dp->lastBaseTextureProcessed != theTexture )
	{
		grTexSource(GR_TMU0, theTexture->offset, GR_MIPMAPLEVELMASK_BOTH, &theTexture->glideTextureInfo);
		DebugStr("\ngrTexSource() ");
	
		// download color table if necessary
		if (theTexture->glideTextureInfo.format == GR_TEXFMT_P_8)
		{
			if ((theTexture->colorTable != dp->currColorTable) && (theTexture->colorTable != NULL) )
			{
				grTexDownloadTable( GR_TEXTABLE_PALETTE, (void *)theTexture->colorTable->pixelData );
				DebugStr( " indirect color texture format ="); DebugNum( theTexture->glideTextureInfo.format );
				DebugStr(" grTexDownloadTable( GR_TEXTABLE_PALETTE, ");
				DebugHex(theTexture->colorTable); DebugStr(" );\n");
				dp->currColorTable = theTexture->colorTable;
				
				if (dp->currColorTable->transparentIndexFlag)
				{
					dp->tState[kQATag_ChromakeyEnable].i = 1;
					grChromakeyMode(GR_CHROMAKEY_ENABLE);
					dp->currChromakeyValue = ((UInt32*) dp->currColorTable->pixelData)[0];
					grChromakeyValue(dp->currChromakeyValue);
					DebugStr(" grChromakeyValue=");
					DebugHex( (UInt32*) (dp->currColorTable->pixelData)[0] );
				}
				else
				{
					dp->tState[kQATag_ChromakeyEnable].i = 0;
					grChromakeyMode(GR_CHROMAKEY_DISABLE);
				}
			}
		}
	}
	else
	{
		DebugStr(" loading SAME texture again, skipped grTexSource \n");
	}
	return true;
}

/*
________________________________________________________________________________________

      LoadAsBaseTexture
________________________________________________________________________________________

*/


GLuint LoadAsBaseTexture( TQADrawPrivate	*dp, TQATexture *theTexture)
{
	GLuint active_units = 0;
	static SInt32		oldAlphaBits = 0;
	
	DebugStr(" LoadAsBaseTexture Texture=");
	DebugHex( theTexture );
	if (theTexture == NULL)
		return 0;
		
	DebugStr(" bound colorTable= ");
	DebugHex( dp->currBaseTexture->colorTable );
	
	if(!LoadTextureAndTableToVRAM(dp, theTexture)) 
	{
		DebugStr(" LoadTextureAndTableToVRAM fail ?\n");
		return 0;
	}
	dp->currBaseTexture = theTexture;
	DebugStr(" LoadTextureAndTableToVRAM-success ");
	DebugStr(GlideFormatString(theTexture->glideTextureInfo.format));

	/* temp 8/2 didn't make a diff. Try removing later and see ... 
	if (theTexture->alphaBits != oldAlphaBits)
	{
		oldAlphaBits = theTexture->alphaBits;
		dp->usingTextureAlpha[ GR_TMU0 ] = theTexture->alphaBits > 1 ? true : false;
	}
	*/
	DebugStr(" alphaBits="); DebugNum(theTexture->alphaBits);
	DebugStr("\n");
				
	return 1;
}


void
FreeAllTexturesVRAM( void )
{
	TQATexture  *theTexture;
	
	DebugStr( "FreeAllTexturesVRAM \n");
	DebugStr( " global numDrawContexts="); DebugNum( gRvEngInfo.numDrawContexts );
	DebugStr( "\n global textureCount="); DebugNum( gRvEngInfo.textureCount);
	DebugStr( "\n global totalTextureSize="); DebugNum( gRvEngInfo.totalTextureSize);
	DebugStr( "\n");
	
	theTexture = gRvEngInfo.textureList;
	while (theTexture)
	{
		FreeTextureVRAM(theTexture);
		theTexture = theTexture->next;
	}
	
	gRvEngInfo.first_resident_texture = NULL;
	gRvEngInfo.latest_resident_texture = NULL;
#if TNSL_DEBUG
	{
		TQATexture * tex = gRvEngInfo.textureList;
		while (tex)
		{
			assert(tex->resident == false );
			tex = tex->next;
		}
		assert( gRvEngInfo.first_resident_texture == NULL);
		assert( gRvEngInfo.latest_resident_texture == NULL);
	}	
#endif
	DebugStr("\n End of FreeAllTexturesVRAM\n");
}




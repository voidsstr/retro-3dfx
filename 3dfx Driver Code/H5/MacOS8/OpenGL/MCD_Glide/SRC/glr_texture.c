/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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

#include <stdlib.h>
#include <string.h>

#include "glr.h"

#include "glr_glide.h"

#include "GlideWrapper.h"

#define CHECK(x) if(!(x)) DEBUG_PRINTF("%s", "Assertion \"" #x "\" failed!")


#define ONE_MEGABYTE 0x00100000

// change ALLOW_32_BIT_TEXTURES to 1 to support napalm's 32 bit texture format
#define ALLOW_32_BIT_TEXTURES 1

//#define MAX_TEXTURE_LOD (gls_2k_texture_size ? GR_LOD_LOG2_2048 : GR_LOD_LOG2_256)

#define MAX_ASPECT_RATIO 3


#pragma mark -
#pragma mark /* Global Vars */

static FxU8 bitsPerTexelMap[19] = 
{
   8,  8,  8,  8,  8,  8,  8,  8,
  16, 16, 16, 16, 16, 16, 16, 16,
   0, /* Reserved */
   4, /* FXT1 */
  32  /* 8888 */
};


#pragma mark -
#pragma mark /* static functions */

#if RESIDENT_LIST_SANITY_CHECK
#define SANITY_CHECK glrSanityCheckResidentList(inContext, hwTextureUnit, __FILE__, __LINE__)
#else
#define SANITY_CHECK
#endif


/*
________________________________________________________________________________________

      glrGetTexture
________________________________________________________________________________________

*/

static glrTexture_t * 
glrGetTexture(
	GLDContext			inContext,
	GLenum 				target,
	GLuint				inName)
{
	GLuint				theIndex;
	glrTexture_t *		theTexture;
			
	theIndex = inName & GLR_TEXTURE_HASH_MASK;
	
	theTexture = inContext->textures[theIndex];

	while( theTexture ) {
		if(theTexture->state->objects.name == inName){
			if(inName != 0){
				break;
			} else {
				if(target == theTexture->target){
					break;
				}
			}
		}
		theTexture = theTexture->next;
	}
	
	return theTexture;
}








/*
________________________________________________________________________________________

      glrCreateTexture
________________________________________________________________________________________

*/

static 
glrTexture_t *
glrCreateTexture(
	GLDContext			inContext,
	GLenum				inTarget,
	GLuint				inName)
{
	GLuint				theIndex;
	glrTexture_t *		theTexture;
	
	DEBUG_ENTRY( glrCreateTexture );


	theIndex = inName & GLR_TEXTURE_HASH_MASK;
	
//_____ Look for an existing texture

	theTexture = glrGetTexture(inContext, inTarget, inName);	
	if(theTexture) return theTexture;
	
//_____ Create a new texture

	theTexture = (glrTexture_t *) glmMalloc(sizeof(glrTexture_t));
	if( !theTexture ) {
		glrSetError( inContext, GL_OUT_OF_MEMORY);
		return NULL;
	}
	
//_____ Init the texture state

	memset(theTexture, 0, sizeof * theTexture);
	theTexture->target             = inTarget;
	
	
//_____ Add the new texture to table

	theTexture->next = inContext->textures[ theIndex ];
	inContext->textures[ theIndex ] = theTexture;
	
	return theTexture;
}



/*
________________________________________________________________________________________

      glrSanityCheckResidentList
________________________________________________________________________________________

*/

#if RESIDENT_LIST_SANITY_CHECK
static void glrSanityCheckResidentList(GLDContext inContext, GLuint hwTextureUnit, char *file, unsigned long line)
{
  glrTexture_t *texture, *next, *next2, *prev;
  
  texture = next2 = inContext->first_resident_texture[hwTextureUnit];
  if(next2) {
    next2 = next2->next_resident[hwTextureUnit];
  }
  
  if(!texture) {
    /* Empty list. Make sure last resident is also NULL */
    if(inContext->latest_resident_texture[hwTextureUnit] != NULL) {
      glr_debug_printf("hw unit %d list check failed, file %s line %d.  Empty list but non-NULL latest_resident texture: %08lx\n",
        hwTextureUnit, file, line, 
        inContext->latest_resident_texture[hwTextureUnit]);
    }
  } else {
    GLboolean foundLatestResident = GL_FALSE;
    
    /* This is actually an okay case, such as if we've freed the first texture on the list and it was the latest at that time. */
    //if(inContext->latest_resident_texture[hwTextureUnit] == NULL) {
    //  glr_debug_printf("hw unit %d list check failed, file %s line %d.  Non-emtpy list with NULL latest_resident_texture pointer.\n",
    //    hwTextureUnit, file, line);
    //}
    
    /* Walk the list, checking for proper linkage and residency status. */
    while(texture) {
      next = texture->next_resident[hwTextureUnit];
      prev = texture->prev_resident[hwTextureUnit];
    
      if(next2 == texture) {
        glr_debug_printf("hw unit %d list check failed, file %s line %d.  Circularly linked list!\n",
          hwTextureUnit, file, line);
          return;
      }
      
      if(texture == inContext->latest_resident_texture[hwTextureUnit])
        foundLatestResident = GL_TRUE;
        
      if(!texture->resident[hwTextureUnit]) {
        glr_debug_printf("hw unit %d list check failed, file %s line %d.  Texture %08lx marked as non-resident but is on resident list.\n",
          hwTextureUnit, file, line,
          texture);
      }
      
      if(next != NULL) {
        if(next->prev_resident[hwTextureUnit] != texture) {
          glr_debug_printf("hw unit %d list check failed, file %s line %d.  Next->prev in node %08lx is %08lx but sholud be %08lx\n",
            hwTextureUnit, file, line,
            next,next->prev_resident[hwTextureUnit],texture);
        }
      }
      
      if(prev != NULL) {
        if(prev->next_resident[hwTextureUnit] != texture) {
          glr_debug_printf("hw unit %d list check failed, file %s line %d.  prev->next in node %08lx is %08lx but sholud be %08lx\n",
            hwTextureUnit, file, line,
            prev,prev->next_resident[hwTextureUnit],texture);
        }
      }
        
      texture = next;
      
      /* Circular linkage sanity checking.  Advance next2 pointer by two nodes each time as long as we're still finding valid nodes. */
      if(next2) {
        next2 = next2->next_resident[hwTextureUnit];
        if(next2) {
          next2 = next2->next_resident[hwTextureUnit];
        }
      }
    }
    if(!foundLatestResident && inContext->latest_resident_texture[hwTextureUnit]) {
      glr_debug_printf("hw unit %d list check failed, file %s line %d.  Couldn't find latest_resident_texture (%08lx) on list\n",
        hwTextureUnit, file, line,
        inContext->latest_resident_texture[hwTextureUnit]);
	}     
  }
  
}
#endif

/*
________________________________________________________________________________________

      glrInitTextureMem
________________________________________________________________________________________

*/
/* FIXME -- This should have a return value and be done at context creation time */
/* Can't be done until after the drawable is attached. */
static void glrInitTextureMem(GLDContext inContext)
{
	FxI32 surface_size;
	FxU32 value;
	GrSurface_t texture_surface = NULL;

	GrSurfaceDesc_t		surfaceDesc;

	if(inContext->renderSurface){
		// windowed UMA
	    	
		value = grGet(GR_MEMORY_UMA, sizeof surface_size, &surface_size);
		CHECK(value == sizeof surface_size);
	
	    /* Start with 1/2 of total video memory, which means we will really start with 1/4th. */	
	    surface_size >>= 1;	    
		do{
			surface_size >>= 1;
			/* Don't bother when less than a single page */
			if (surface_size < 4096) break;
			
			/* initialize the texture texture_surface */
			memset( &surfaceDesc, 0, sizeof(surfaceDesc) );
			surfaceDesc.width = surface_size;		
			surfaceDesc.height = 1;
			surfaceDesc.bytesPerPixel = 1;
	        surfaceDesc.notifyCallback = glrSurfaceNotify;
	        surfaceDesc.userData = inContext;
			texture_surface = grSurfaceCreateExt( &surfaceDesc );
		} while(texture_surface == NULL);
				
		CHECK(texture_surface != NULL);
		
		if(!texture_surface)
		  return;
		  
		grSurfaceGetDescExt(inContext->texture_surface, &inContext->textureSurfaceDesc);
		
		inContext->texture_surface = texture_surface;
		inContext->surface_size[0] = surface_size;

		grSurfaceSetTextureSurfaceExt( GR_TMU0, texture_surface );
		grSurfaceSetTextureSurfaceExt( GR_TMU1, texture_surface );
	} else if(inContext->isUMA){
		// full screen UMA
        grGet(GR_MEMORY_TMU, sizeof(inContext->surface_size[0]), (FxI32 *)&inContext->surface_size[0]);
	} else {
		// full screen non UMA (voodoo2)
		inContext->surface_size[0] = grTexMaxAddress(GR_TMU0) - grTexMinAddress(GR_TMU0);
		inContext->surface_size[1] = grTexMaxAddress(GR_TMU1) - grTexMinAddress(GR_TMU1);		
	}

	/* Adjust maximum texture size based on available memory. */
	if(inContext->glideExtensions.gls_2k_texture_size && inContext->surface_size[0] >= 2048*2048*4*2*4/3) {
	  inContext->max_texture_lod = GR_LOD_LOG2_2048;
	} else if (inContext->glideExtensions.gls_2k_texture_size && inContext->surface_size[0] >= 1024*1024*4*2*4/3) {
	  inContext->max_texture_lod = GR_LOD_LOG2_1024;
	} else if (inContext->glideExtensions.gls_2k_texture_size && inContext->surface_size[0] >= 512*512*4*2*4/3) {
	  inContext->max_texture_lod = GR_LOD_LOG2_512;
	} else if (inContext->surface_size[0] >= 256*256*2*4/3) {
	  inContext->max_texture_lod = GR_LOD_LOG2_256;
	} else if (inContext->surface_size[0] >= 128*128*2*4/3) {
	  inContext->max_texture_lod = GR_LOD_LOG2_128;
	} else if (inContext->surface_size[0] >= 64*64*2*4/3) {
	  inContext->max_texture_lod = GR_LOD_LOG2_64;
	} else {
	  inContext->max_texture_lod = GR_LOD_LOG2_32;
	} 
}
/*
________________________________________________________________________________________

      glrFreeTextureVRAM
________________________________________________________________________________________

*/

static void glrFreeTextureVRAM(GLDContext inContext, glrTexture_t * theTexture, GLuint hwTextureUnit)
{
	glrTexture_t * prev;
	glrTexture_t * next;
		
	if(theTexture->resident[hwTextureUnit] == GL_FALSE) return;

	prev = theTexture->prev_resident[hwTextureUnit];
	next = theTexture->next_resident[hwTextureUnit];
	
	if(prev != NULL){
		prev->next_resident[hwTextureUnit] = next;
	} else {
		inContext->first_resident_texture[hwTextureUnit] = next;
	}

	if(next != NULL){
		next->prev_resident[hwTextureUnit] = prev;
	}

	if(inContext->latest_resident_texture[hwTextureUnit] == theTexture) {
		inContext->latest_resident_texture[hwTextureUnit] = prev;
	}
	
	theTexture->resident[hwTextureUnit] = GL_FALSE;
	theTexture->prev_resident[hwTextureUnit] = NULL;
	theTexture->next_resident[hwTextureUnit] = NULL;

	SANITY_CHECK;
	return;
}
/*
________________________________________________________________________________________

      glrScaleMap
________________________________________________________________________________________

*/

static void glrScaleMap(glrTexture_t *theTexture, 
            GLuint src_width, GLuint src_height, void *src_data,
            GLuint dst_width, GLuint dst_height, void *dst_data, GLuint format)
{
#pragma unused (theTexture, dst_width, dst_height)
  FxU8 *src0, *src1, *src2, *src3;
  FxU8 *dst;
  FxU32 vOffset, hOffset, bytesPerTexel, x, y;
  
  /* Horizontal byte offset from one 2x2 block to the next */
  hOffset = bitsPerTexelMap[format] >> 2;
  bytesPerTexel = hOffset >> 1;
  
  /* Vertical offset from one row to the next */
  vOffset = (bitsPerTexelMap[format] >> 3) * src_width;
  dst = (FxU8 *)dst_data; 
  
  for(y = 0; y < src_height; y+= 2)
  {
    src0 = (FxU8 *)src_data + y * vOffset;
    src1 = src0 + hOffset;
    src2 = src0 + vOffset;
    src3 = src2 + hOffset;
     
    /* Collapse pointers for width || height == 1 */
    if(src_width == 1)  {src1 = src0; src3 = src2; } 
    if(src_height == 1) {src2 = src0; src3 = src1; }

    switch(format) {
      case GR_TEXFMT_INTENSITY_8:
      case GR_TEXFMT_ALPHA_8:
      case GR_TEXFMT_ALPHA_INTENSITY_88:
      case GR_TEXFMT_ARGB_8888:
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
        glr_debug_printf("unsupported scale: %d\n",format);
        break;
        
    }
  }
}
/*
________________________________________________________________________________________

      glrGenMipMap
________________________________________________________________________________________

*/

static void glrGenMipMap(glrTexture_t *theTexture, GLint minLevel)
{
  FxU32 size;
  FxU32 src_level;
  FxU32 src_width, src_height, dst_width, dst_height;
  
  src_width = theTexture->internal_width;
  src_height = theTexture->internal_height;
  
  for(src_level = theTexture->largeLodLog2; src_level > minLevel; src_level--)
  {
     dst_width = src_width >> 1;
     dst_height = src_height >> 1;
     if(dst_width == 0) dst_width = 1;
     if(dst_height == 0) dst_height = 1;
     
     if(!theTexture->sys_data[src_level-1])
     {
       size = dst_width * dst_height * (bitsPerTexelMap[theTexture->sys_texture.format] >> 3);
       theTexture->sys_data[src_level-1] = glmMalloc(size);
       glrScaleMap(theTexture,
         src_width,src_height,theTexture->sys_data[src_level],
         dst_width,dst_height,theTexture->sys_data[src_level-1],
         theTexture->sys_texture.format);
       
     }
  }
}
/*
________________________________________________________________________________________

      glrValidateTexture
________________________________________________________________________________________

*/

static void glrValidateTexture(GLDContext inContext, glrTexture_t *theTexture)
{
	FxU32 largeLodLog2;

    largeLodLog2 = theTexture->largeLodLog2;
    if(largeLodLog2 > inContext->max_texture_lod)
      largeLodLog2 = inContext->max_texture_lod;

    theTexture->sys_texture.largeLodLog2 = largeLodLog2;
    theTexture->sys_texture.smallLodLog2 = theTexture->smallLodLog2;
    if(theTexture->sys_texture.smallLodLog2 > theTexture->sys_texture.largeLodLog2)
      theTexture->sys_texture.smallLodLog2 = theTexture->sys_texture.largeLodLog2;

    /* Now make sure that we have at least one displayable mipmap level. */
    if(!theTexture->sys_data[theTexture->sys_texture.largeLodLog2]) 
    {
      //glr_debug_printf("generating level %d for texture: %08lx\n",theTexture->sys_texture.largeLodLog2,theTexture);
      glrGenMipMap(theTexture,theTexture->sys_texture.largeLodLog2);
    }
}
/*
________________________________________________________________________________________

      glrAllocateTextureVram
________________________________________________________________________________________

*/

static void glrAllocateTextureVram(GLDContext inContext, glrTexture_t * theTexture, GLuint hwTextureUnit)
{
	glrTexture_t * latest_resident_texture;
	glrTexture_t * texture2;
	long space, size, offset;
	
	/********/
	if(inContext->surface_size[0] == 0){
		glrInitTextureMem(inContext);	
	}
 
    glrValidateTexture(inContext,theTexture);
            
    /* Calc size here at the last possible moment */
    size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH ,&theTexture->sys_texture);
    theTexture->size = size;
    
	latest_resident_texture = inContext->latest_resident_texture[hwTextureUnit];
	
	if(latest_resident_texture == NULL){
		texture2 = inContext->first_resident_texture[hwTextureUnit];

		if(texture2 == NULL){	
			theTexture->next_resident[hwTextureUnit] = NULL;
			theTexture->prev_resident[hwTextureUnit] = NULL;
			theTexture->offset[hwTextureUnit] = 0;
			theTexture->resident[hwTextureUnit] = GL_TRUE;

			inContext->first_resident_texture[hwTextureUnit] = theTexture;
			inContext->latest_resident_texture[hwTextureUnit] = theTexture;
			SANITY_CHECK;
			return;
		}
		offset = 0;
	} else {
		texture2 = latest_resident_texture->next_resident[hwTextureUnit];
		offset = latest_resident_texture->offset[hwTextureUnit] + latest_resident_texture->size;

		space = inContext->surface_size[hwTextureUnit] - offset;
		if(space < size){
			offset = 0;
			texture2 = inContext->first_resident_texture[hwTextureUnit];
			// Since we're starting at the beginning, force the current texture to become
			// the new latest resident.
			latest_resident_texture = NULL;
        } else if(inContext->surface_size[hwTextureUnit] > 2*1024*1024) {
		  /* 2MB boundary check for Voodoo2. */
		  space = 2*1024*1024 - offset;
	      if(space > 0 && size > space) {
	        offset = 2*1024*1024;
	      }		
	    }
	}
	
	// delete textures from current point until we have room.
	while(texture2 && texture2->offset[hwTextureUnit] < offset + size){
		glrTexture_t *next_texture;
		next_texture = texture2->next_resident[hwTextureUnit];
		glrFreeTextureVRAM(inContext, texture2, hwTextureUnit);
		texture2 = next_texture;
	}
	
	// Note, if we walked off the end of the list, that's fine and the following line of code is still okay.
	theTexture->next_resident[hwTextureUnit] = texture2;
	theTexture->prev_resident[hwTextureUnit] = latest_resident_texture;	
	theTexture->offset[hwTextureUnit] = offset;
	theTexture->resident[hwTextureUnit] = GL_TRUE;
	
	// Fix up previous link for next texture in list (if there is one)
	if(texture2) {
	  texture2->prev_resident[hwTextureUnit] = theTexture;
	}
	
	// Fix up next link for the old latest texture if we're not starting out at
	// the beginning of the list.
	if(latest_resident_texture) {
	  latest_resident_texture->next_resident[hwTextureUnit] = theTexture;
	}
	
	if(latest_resident_texture == NULL){
		inContext->first_resident_texture[hwTextureUnit] = theTexture;
	}

	inContext->latest_resident_texture[hwTextureUnit] = theTexture;
	SANITY_CHECK;

}



/*
________________________________________________________________________________________

glrBytesPerSourcePixel
________________________________________________________________________________________
*/

inline GLint glrBytesPerSourcePixel(GLenum source_format)
{
	switch(source_format){
	case GL_LUMINANCE:
	case GL_INTENSITY:
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
		return 1;
	case GL_LUMINANCE_ALPHA:
		return 2;
	case GL_RGB:
		return 3;
	case GL_RGBA:
	case GL_ABGR_EXT:
		return 4;
	}

	return 0;	// error case
}

/*
________________________________________________________________________________________

glrMapTextureFormat
________________________________________________________________________________________

glrMapTextureFormat maps OpenGL internal formats to Glide formats

*/

static GrTextureFormat_t glrMapTextureFormat(GLint internalformat)
{
	switch (internalformat){
	// OpenGL 1.0 texture formats
	case 1:
		return GR_TEXFMT_INTENSITY_8;
		
	case 2:
		return GR_TEXFMT_ALPHA_INTENSITY_88;
		
	case 3:
		return GR_TEXFMT_RGB_565;
		
	case 4:
		return GR_TEXFMT_ARGB_4444;
		
	// OpenGL 1.1 texture internal formats
	case GL_ALPHA:
	case GL_ALPHA4:
	case GL_ALPHA8:
	case GL_ALPHA12:
	case GL_ALPHA16:
		return GR_TEXFMT_ALPHA_8;
		
	case GL_LUMINANCE:
	case GL_LUMINANCE4:
	case GL_LUMINANCE8:
	case GL_LUMINANCE12:
	case GL_LUMINANCE16:
		return GR_TEXFMT_INTENSITY_8;
		
	case GL_LUMINANCE4_ALPHA4:
		return GR_TEXFMT_ALPHA_INTENSITY_44;
		
	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE6_ALPHA2:
	case GL_LUMINANCE8_ALPHA8:
	case GL_LUMINANCE12_ALPHA4:
	case GL_LUMINANCE12_ALPHA12:
	case GL_LUMINANCE16_ALPHA16:
		return GR_TEXFMT_ALPHA_INTENSITY_88;
		
	case GL_INTENSITY:
	case GL_INTENSITY4:
	case GL_INTENSITY8:
	case GL_INTENSITY12:
	case GL_INTENSITY16:
		return GR_TEXFMT_ALPHA_8;
	
	case GL_R3_G3_B2:
		return GR_TEXFMT_RGB_332;
		
	case GL_RGB:
	case GL_RGB4:
	case GL_RGB5:
		return GR_TEXFMT_RGB_565;
		
	case GL_RGB8:
	case GL_RGB10:
	case GL_RGB12:
	case GL_RGB16:
#if ALLOW_32_BIT_TEXTURES
		return GR_TEXFMT_ARGB_8888;
#else
		return GR_TEXFMT_RGB_565;
#endif

	case GL_RGBA:
	case GL_RGBA2:
	case GL_RGBA4:
		return GR_TEXFMT_ARGB_4444;
	
	case GL_RGB5_A1:
		return GR_TEXFMT_ARGB_1555;
		
	case GL_RGBA8:
	case GL_RGB10_A2:
	case GL_RGBA12:
	case GL_RGBA16:
#if ALLOW_32_BIT_TEXTURES
		return GR_TEXFMT_ARGB_8888;
#else
		return GR_TEXFMT_ARGB_4444;
#endif
	
	}

	return -1;	// unknown format
}

/*
________________________________________________________________________________________

glrCalcBaseInternalFormat
________________________________________________________________________________________

glrCalcBaseInternalFormat maps OpenGL internal formats to OpenGL base internal formats

*/

static GLenum glrCalcBaseInternalFormat(GLint internalformat)
{
	switch (internalformat){
	// OpenGL 1.0 texture formats
	case 1:
		return GL_LUMINANCE;
		
	case 2:
		return GL_LUMINANCE_ALPHA;
		
	case 3:
		return GL_RGB;
		
	case 4:
		return GL_RGBA;
		
	// OpenGL 1.1 texture internal formats
	case GL_ALPHA:
	case GL_ALPHA4:
	case GL_ALPHA8:
	case GL_ALPHA12:
	case GL_ALPHA16:
		return GL_ALPHA;
		
	case GL_LUMINANCE:
	case GL_LUMINANCE4:
	case GL_LUMINANCE8:
	case GL_LUMINANCE12:
	case GL_LUMINANCE16:
		return GL_LUMINANCE;
				
	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE6_ALPHA2:
	case GL_LUMINANCE4_ALPHA4:
	case GL_LUMINANCE8_ALPHA8:
	case GL_LUMINANCE12_ALPHA4:
	case GL_LUMINANCE12_ALPHA12:
	case GL_LUMINANCE16_ALPHA16:
		return GL_LUMINANCE_ALPHA;
		
	case GL_INTENSITY:
	case GL_INTENSITY4:
	case GL_INTENSITY8:
	case GL_INTENSITY12:
	case GL_INTENSITY16:
		return GL_INTENSITY;
	
	case GL_RGB:
	case GL_R3_G3_B2:
	case GL_RGB4:
	case GL_RGB5:
	case GL_RGB8:
	case GL_RGB10:
	case GL_RGB12:
	case GL_RGB16:
		return GL_RGB;

	case GL_RGBA:
	case GL_RGBA2:
	case GL_RGBA4:
	case GL_RGB5_A1:
	case GL_RGBA8:
	case GL_RGB10_A2:
	case GL_RGBA12:
	case GL_RGBA16:
		return GL_RGBA;
	
	}

	return -1;	// unknown format
}

/*
________________________________________________________________________________________

      glrTranslatePixelsInline
________________________________________________________________________________________

*/


inline void glrTranslatePixelsInline(
	const void * source_pixels, 
	void * dest_pixels, 
	GLenum source_format, 
	GrTextureFormat_t dest_format, 
	GLsizei pixel_count, 
	GLsizei repeat_pixels)
{
	GLsizei i;

	if(source_format == GL_RGBA && dest_format == GR_TEXFMT_ARGB_4444){
		unsigned const long * source = source_pixels;
		unsigned short * dest = dest_pixels;
		unsigned long v1, v2;
		
		while(pixel_count--){
			v1 = *source++;
			
			v2 =	v1 << 8  & 0xf000 |	// alpha
					v1 >> 20 & 0x0f00 |	// red
					v1 >> 16 & 0x00f0 |	// green
					v1 >> 12 & 0x000f;	// blue
					
			for(i = 0; i < repeat_pixels; i++){
				*dest++ = v2;
			}
		}
		
		return;
		
	} else {
		// slow catch-all method (is fast when inlined)
		unsigned const char * source = source_pixels;
		unsigned char * dest = dest_pixels;
		unsigned char r, g, b, a;
		
		r = g = b = 0;
		a = 0xFF;
		
		
		while(pixel_count--){
			switch(source_format){
			case GL_RED:
				r = *source++;
				break;
			case GL_GREEN:
				g = *source++;
				break;
			case GL_BLUE:
				b = *source++;
				break;
			case GL_ALPHA:
				a = *source++;
				break;
			case GL_RGB:
				r = *source++;
				g = *source++;
				b = *source++;
				break;
			case GL_RGBA:
				r = *source++;
				g = *source++;
				b = *source++;
				a = *source++;
				break;
			case GL_LUMINANCE:
				r = g = b = *source++;
				break;
			case GL_LUMINANCE_ALPHA:
				r = g = b = *source++;
				a = *source++;
				break;
			case GL_INTENSITY:
				a = r = g = b = *source++;
				break;
			case GL_ABGR_EXT:
				a = *source++;
				b = *source++;
				g = *source++;
				r = *source++;
				break;			    	
			}

			for(i = 0; i < repeat_pixels; i++)
			{
				switch(dest_format){
				case GR_TEXFMT_INTENSITY_8:
					*dest++ = r;
					break;
				case GR_TEXFMT_ALPHA_INTENSITY_88:
					*dest++ = a;
					*dest++ = r;
					break;
				case GR_TEXFMT_ALPHA_8:
					*dest++ = a;
					break;
				case GR_TEXFMT_ALPHA_INTENSITY_44:
					*dest++ = a & 0xf0 | r >> 4 & 0x0f;
					break;
				case GR_TEXFMT_RGB_332:
					*dest++ =
						r >> 0 & 0xe0 |
						g >> 3 & 0x1c |
						b >> 6 & 0x03;
					break;
				case GR_TEXFMT_RGB_565:
					*((unsigned short *)dest)++ =
						(unsigned short)r << 8 & 0xf800 |
						(unsigned short)g << 3 & 0x07e0 |
						(unsigned short)b >> 3 & 0x001f;
					break;
				case GR_TEXFMT_ARGB_4444:
					*((unsigned short *)dest)++ =
						(unsigned short)a << 8 & 0xf000 |
						(unsigned short)r << 4 & 0x0f00 |
						(unsigned short)g << 0 & 0x00f0 |
						(unsigned short)b >> 4 & 0x000f;
					break;
				case GR_TEXFMT_ARGB_1555:
					*((unsigned short *)dest)++ =
						(unsigned short)a << 8 & 0x8000 |
						(unsigned short)r << 7 & 0x7c00 |
						(unsigned short)g << 2 & 0x03e0 |
						(unsigned short)b >> 3 & 0x001f;
					break;
				case GR_TEXFMT_ARGB_8888:
					*dest++ = a;
					*dest++ = r;
					*dest++ = g;
					*dest++ = b;
					break;
				}
			}
		}	
	}
}

/*
________________________________________________________________________________________

      glrTranslatePixelsDispatch
________________________________________________________________________________________

*/


static void glrTranslatePixelsDispatch(const void * source_pixels, void * dest_pixels, GLenum source_format, GrTextureFormat_t dest_format, GLsizei pixel_count, GLsizei repeat_pixels)
{
	if(source_pixels == NULL){
		// this is a legal condition
		return;
	}

#define FORMAT_COMBO(gl_format, fx_format) ((unsigned long)gl_format << 4 | fx_format)
#define SPECIAL_CASE(sf, df) \
  case FORMAT_COMBO(sf, df): \
  glrTranslatePixelsInline(source_pixels, dest_pixels, sf, df, pixel_count, 1); break \

	if(repeat_pixels == 1){

		switch(FORMAT_COMBO(source_format, dest_format)){
		// special case - most common translations
		
		SPECIAL_CASE(GL_RGBA, GR_TEXFMT_RGB_332);
		SPECIAL_CASE(GL_RGBA, GR_TEXFMT_RGB_565);
		SPECIAL_CASE(GL_RGBA, GR_TEXFMT_ARGB_4444);
		SPECIAL_CASE(GL_RGBA, GR_TEXFMT_ARGB_1555);
		SPECIAL_CASE(GL_RGBA, GR_TEXFMT_ARGB_8888);

		SPECIAL_CASE(GL_RGB, GR_TEXFMT_RGB_332);
		SPECIAL_CASE(GL_RGB, GR_TEXFMT_RGB_565);
		SPECIAL_CASE(GL_RGB, GR_TEXFMT_ARGB_4444);
		SPECIAL_CASE(GL_RGB, GR_TEXFMT_ARGB_1555);
		SPECIAL_CASE(GL_RGB, GR_TEXFMT_ARGB_8888);
		
		SPECIAL_CASE(GL_LUMINANCE_ALPHA, GR_TEXFMT_ALPHA_INTENSITY_88);
		SPECIAL_CASE(GL_LUMINANCE_ALPHA, GR_TEXFMT_ALPHA_INTENSITY_44);
		
		// special case - direct mem copy - no swizling needed
		case FORMAT_COMBO(GL_ALPHA, GR_TEXFMT_ALPHA_8):
		case FORMAT_COMBO(GL_LUMINANCE, GR_TEXFMT_INTENSITY_8):
		case FORMAT_COMBO(GL_INTENSITY, GR_TEXFMT_ALPHA_8):
			memcpy(dest_pixels, source_pixels, pixel_count);
			break;
		
		// generic case
		default:
			glrTranslatePixelsInline(source_pixels, dest_pixels, source_format, dest_format, pixel_count, 1);
		}
	} else {
		glrTranslatePixelsInline(source_pixels, dest_pixels, source_format, dest_format, pixel_count, repeat_pixels);	
	}
}
/*
________________________________________________________________________________________

      glrCheckLOD
________________________________________________________________________________________

*/

static GrLOD_t glrCheckLOD(GLsizei size)
{
	switch (size) {
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
/*
________________________________________________________________________________________

      glrTranslatePixelsDispatch2
________________________________________________________________________________________

*/

static void glrTranslatePixelsDispatch2(const void * source_pixels, void * dest_pixels, GLenum source_format, GrTextureFormat_t dest_format, GLsizei width, GLsizei height)
{
	FxI32 wlod, hlod, ar;

	if(source_pixels == NULL){
		// this is a legal condition
		return;
	}

	wlod = glrCheckLOD(width);		
	hlod = glrCheckLOD(height);
	ar = wlod - hlod;

	if(ar < -MAX_ASPECT_RATIO){
		// texture is too tall
		// stretch width
		//glrTranslatePixelsDispatch(source_pixels, dest_pixels, source_format, dest_format, width * height, 1 << (-ar - MAX_ASPECT_RATIO));
	} else if (ar > MAX_ASPECT_RATIO){
		// texture is to wide
		// stretch height
		long row_bytes, repeat, i, j;
		char * source;
		char * dest;
		
		row_bytes = width * (bitsPerTexelMap[dest_format] >> 3);
		repeat = 1 << (ar - MAX_ASPECT_RATIO);
		
		glrTranslatePixelsDispatch(source_pixels, dest_pixels, source_format, dest_format, width * height, 1);
		
		for(i = height - 1; i >= 0; i--){
			source = (char *)dest_pixels + row_bytes * i;
			dest = (char *)dest_pixels + row_bytes * i * repeat;
			for(j = 0; j < repeat; j++){
				memcpy(dest, source, row_bytes);
				dest += row_bytes;
			}
		}
	} else {
		glrTranslatePixelsDispatch(source_pixels, dest_pixels, source_format, dest_format, width * height, 1);
	}
}


/*
________________________________________________________________________________________

      glrCalcMipmapSize
________________________________________________________________________________________

*/
// calculates the size of a single mipmap level
// takes into account stretching to maintain 8:1/1:8 aspect ratio
static GLsizei glrCalcMipmapSize(GrTextureFormat_t dest_format, GLsizei width, GLsizei height)
{
	FxI32 wlod, hlod, ar, bpt, size;

	/*****/

	bpt = bitsPerTexelMap[dest_format];

	size = (width * height * bpt) >> 3;

	wlod = glrCheckLOD(width);		
	hlod = glrCheckLOD(height);
	ar = wlod - hlod;

	if(ar < -MAX_ASPECT_RATIO) ar = -ar;	
	
	if(ar > MAX_ASPECT_RATIO) {
		size *= (1 << (ar - MAX_ASPECT_RATIO));
	}

	return size;
}


/*
________________________________________________________________________________________

      glrImportTexture
________________________________________________________________________________________

*/

// return a GLI error code
static GLenum glrImportTexture2(
	GLDContext inContext, 
	glrTexture_t * inTexture, 
	GLint                     level,
	GLsizei                   width,
	GLsizei                   height,
	GLenum                    base_fmt,
	GLenum                    req_fmt,
	const GLTtexComp         *pixels)
{
	GrLOD_t wlod;
	GrLOD_t hlod;
	GrLOD_t lod;
	GrAspectRatio_t aspect_ratio;
	GrTextureFormat_t format;
	FxU32 size;

	/*******/
	
	glrFreeTextureVRAM(inContext, inTexture, 0);
	glrFreeTextureVRAM(inContext, inTexture, 1);	

	if(level == 0)
	{
		// new base level texture
		// or
		// replacing an existing texture
		
		// to do: handle 1d textures
				
		wlod = glrCheckLOD(width);
		if(wlod == -1) return GLI_BAD_VALUE;
		
		hlod = glrCheckLOD(height);
		if(hlod == -1) return GLI_BAD_VALUE;
		
		// aspect ratio and format are only calculated for base level textures
		aspect_ratio = wlod - hlod;
		if (aspect_ratio > MAX_ASPECT_RATIO) {
			hlod += aspect_ratio - MAX_ASPECT_RATIO; 
			aspect_ratio = MAX_ASPECT_RATIO;
		} else if (aspect_ratio < -MAX_ASPECT_RATIO) {
			wlod += -MAX_ASPECT_RATIO - aspect_ratio;
			aspect_ratio = -MAX_ASPECT_RATIO;
		};
		
		format = glrMapTextureFormat(req_fmt);
		if(format == -1) return GLI_BAD_VALUE;
		
		lod = wlod > hlod ? wlod : hlod;
		
		// calculate the size
		size = glrCalcMipmapSize(format, width, height);
		
		// clear out any pre-existing texture
		{
		  FxU32 i;
		  for(i = 0; i < GLR_MAX_TEXTURE_LEVEL; i++) {
		    if(inTexture->sys_data[i]) {
		      glmFree(inTexture->sys_data[i]);
		      inTexture->sys_data[i] = NULL;
		    }
		  }
		}

		// allocate a local buffer for the pixels
		inTexture->sys_data[lod] = glmMalloc(size);
		
		if(inTexture->sys_data[lod] == NULL) return GLI_BAD_ALLOC;
		
		// translate the pixels
		glrTranslatePixelsDispatch2(
			pixels, 
			inTexture->sys_data[lod], 
			base_fmt, 
			format, 
			width,
			height);
					
		inTexture->largeLodLog2 = lod;
		inTexture->smallLodLog2 = lod;			
		inTexture->sys_texture.aspectRatioLog2 = aspect_ratio;
		inTexture->sys_texture.format = format;
		inTexture->baseInternalFormat = glrCalcBaseInternalFormat(req_fmt);
		
		if(aspect_ratio >= 0){
			inTexture->s_scale = (FxFloat)256;
			inTexture->t_scale = (FxFloat)(256 >> aspect_ratio);
		} else {
			inTexture->s_scale = (FxFloat)(256 >> -aspect_ratio);
			inTexture->t_scale = (FxFloat)256;		
		}
		
		//inTexture->size = size;
		inTexture->max_level = 0;
		inTexture->width = width;
		inTexture->height = height;
		inTexture->internal_width = 1 << wlod;
		inTexture->internal_height = 1 << hlod;
		
		return GLI_NO_ERROR;
		
	}
	else if (level == inTexture->max_level + 1)
	{
		// adding a level to an existing texture
		GLsizei expected_width;
		GLsizei expected_height;
		
		expected_width = inTexture->width >> level;
		expected_height = inTexture->height >> level;
		
		if(expected_width < 1 && expected_height < 1) return GLI_BAD_VALUE;
		
		if(expected_width < 1) expected_width = 1;
		if(expected_height < 1) expected_height = 1;

		// check the width
		if(width != expected_width) return GLI_BAD_VALUE;
		
		// check the height
		if(height != expected_height) return GLI_BAD_VALUE;
				
		lod = inTexture->smallLodLog2 - 1;
		
		// calcualte the new size
		//size = grTexCalcMemRequired(lod, inTexture->sys_texture.largeLodLog2, inTexture->sys_texture.aspectRatioLog2, inTexture->sys_texture.format);
		size = glrCalcMipmapSize(inTexture->sys_texture.format, width, height);
		
		// upsize the local buffer
		//inTexture->sys_texture.data = glmRealloc(inTexture->sys_texture.data, size);
		inTexture->sys_data[lod] = glmMalloc(size);
		
		if(inTexture->sys_data[lod] == NULL){
		    //glr_debug_printf("alloc fail!\n");
			// clear the texture
			//inTexture->max_level = 0;
			//inTexture->size = 0;
			//inTexture->width = 0;
			//inTexture->height = 0;
			return GLI_BAD_ALLOC;
		}
		
		// translate the pixels
		glrTranslatePixelsDispatch2(
			pixels, 
			inTexture->sys_data[lod],
			base_fmt, 
			inTexture->sys_texture.format, 
			width,
			height);
		
		// update the fields that need to change
		inTexture->smallLodLog2 = lod;
		
		inTexture->size = size;
		inTexture->max_level = level;	
		return GLI_NO_ERROR;
	}
	
	return GLI_BAD_VALUE;
}

static GLenum glrImportTexture(GLDContext inContext, glrTexture_t * inTexture, const GLDTexture * inGLDTexture)
{
	return glrImportTexture2(
		inContext, 
		inTexture, 
		inGLDTexture->level,
		inGLDTexture->width,
		inGLDTexture->height,
		inGLDTexture->base_fmt,
		inGLDTexture->req_fmt,
		inGLDTexture->pixels);

}



/*
________________________________________________________________________________________

      glrLoadSysTexture
________________________________________________________________________________________

*/

static GLboolean
glrLoadSysTexture(
	GLDContext			inContext,
	glrTexture_t *		inTexture,
	GLuint              hwTextureUnit)
{
    FxU32 lod;

	if(!inTexture->resident[hwTextureUnit]) {
		glrAllocateTextureVram(inContext, inTexture, hwTextureUnit);
	    
	    /* Load mipmap levels individually */
	    for(lod = inTexture->sys_texture.smallLodLog2; lod <= inTexture->sys_texture.largeLodLog2; lod++) {
 		  grTexDownloadMipMapLevel(hwTextureUnit,
            inTexture->offset[hwTextureUnit],
            lod, 
            inTexture->sys_texture.largeLodLog2,
            inTexture->sys_texture.aspectRatioLog2,
            inTexture->sys_texture.format, 
            GR_MIPMAPLEVELMASK_BOTH, 
            inTexture->sys_data[lod]);
        }
	}
	return GL_TRUE;
}

#pragma mark -
#pragma mark /* exported functions */

/*
________________________________________________________________________________________

      glrLoadCurrentTexture
________________________________________________________________________________________

*/


GLuint
glrLoadCurrentTexture(
	GLDContext			inContext)
{
	glrTexture_t  *theTexture0, *theTexture1;
		
	GLuint active_units = 0;
	
	/* Reset theTexture enabled flag */
	theTexture0 = theTexture1 = NULL;
	
	inContext->hw_texture[0] = NULL;
	inContext->hw_texture[1] = NULL;
	
    inContext->arb_unit_map[0] = GR_TMU0;
    inContext->arb_unit_map[1] = GR_TMU1;
	
	/* Check for enabled textures, 2D has priority */
	if(inContext->state->texture_mode[0].enable_2d)
	{
		theTexture0 = glrGetTexture(inContext, GL_TEXTURE_2D, inContext->cur_texture_name[0][1]);
	}
	else if(inContext->state->texture_mode[0].enable_1d)
	{
		theTexture0 = glrGetTexture(inContext, GL_TEXTURE_1D, inContext->cur_texture_name[0][0]);
	}

	if(inContext->state->texture_mode[1].enable_2d)
	{
		theTexture1 = glrGetTexture(inContext, GL_TEXTURE_2D, inContext->cur_texture_name[1][1]);
	}
	else if(inContext->state->texture_mode[1].enable_1d)
	{
		theTexture1 = glrGetTexture(inContext, GL_TEXTURE_1D, inContext->cur_texture_name[1][0]);
	}
    
	/* If texture1 is present and enabled then we do the remap thing, where ARB unit 0 becomes TMU1 and ARB unit 1 becomes TMU 0 */
	if(theTexture1) {
		inContext->arb_unit_map[0] = GR_TMU1;
		inContext->arb_unit_map[1] = GR_TMU0;
	}
    
	/* Load textures into hardware */
	if(theTexture1)
	{
		GLint tmu, pool;
		tmu = inContext->arb_unit_map[1];
		pool = inContext->isUMA ? 0 : tmu;
		
		glrLoadSysTexture(inContext, theTexture1, pool);
		grTexSource(tmu, theTexture1->offset[pool], GR_MIPMAPLEVELMASK_BOTH, &theTexture1->sys_texture);
		inContext->hw_texture[tmu] = theTexture1;
		active_units |= (1L << tmu);
	}
	
	if(theTexture0)
	{
		GLint tmu, pool;
		tmu = inContext->arb_unit_map[0];
		pool = inContext->isUMA ? 0 : tmu;
		
		glrLoadSysTexture(inContext, theTexture0, pool);
		grTexSource(tmu, theTexture0->offset[pool], GR_MIPMAPLEVELMASK_BOTH, &theTexture0->sys_texture);
		inContext->hw_texture[tmu] = theTexture0;
		active_units |= (1L << tmu);
	}
	
	return active_units;
}


/*
________________________________________________________________________________________

      glrFreeAllTextures
________________________________________________________________________________________

*/

void
glrFreeAllTextures(
	GLDContext			inContext)
{
	GLuint      i, j;
	glrTexture_t  *theTexture, *nextTexture;

	for(j = 0; j < GLR_TEXTURE_HASH_SIZE; j++)
	{
		theTexture = inContext->textures[j];
		while(theTexture)
		{
		    nextTexture = theTexture->next;
		     
			glrFreeTextureVRAM(inContext, theTexture, 0);
			glrFreeTextureVRAM(inContext, theTexture, 1);
             
            for(i = 0; i < GLR_MAX_TEXTURE_LEVEL; i++) {              
			  if(theTexture->sys_data[i]){
				glmFree(theTexture->sys_data[i]);
				theTexture->sys_data[i] = 0;
		      }
			}
			
			glmFree((char *) theTexture);
		     
			theTexture = nextTexture; //inContext->textures[j];
						
		}
		inContext->textures[j] = 0;
	}
	
}

void
glrFreeAllTexturesVRAM(
	GLDContext			inContext)
{
	GLuint      j;
	glrTexture_t  *theTexture;

	for(j = 0; j < GLR_TEXTURE_HASH_SIZE; j++)
	{
		theTexture = inContext->textures[j];
		while(theTexture)
		{
			glrFreeTextureVRAM(inContext, theTexture, 0);
			glrFreeTextureVRAM(inContext, theTexture, 1);             
			theTexture = theTexture->next;
		}
	}	
}


#define INTERNAL_USE_TARGET_2D 0xFEEDFACE

glrTexture_t * glrInternalUseTexture(
	GLDContext inContext,
	GLint                     level,
	GLsizei                   width,
	GLsizei                   height,
	GLenum                    base_fmt,
	GLenum                    req_fmt,
	const GLTtexComp         *pixels);
glrTexture_t * glrInternalUseTexture(
	GLDContext inContext,
	GLint                     level,
	GLsizei                   width,
	GLsizei                   height,
	GLenum                    base_fmt,
	GLenum                    req_fmt,
	const GLTtexComp         *pixels)
{
	static GLDPerTextureState fakeState;
	
	glrTexture_t * theTexture;
	
	theTexture = glrCreateTexture(
		inContext,
		INTERNAL_USE_TARGET_2D,
		0);
	
	fakeState.objects.name = 0;
	theTexture->state = &fakeState;
		
	glrImportTexture2(
		inContext, 
		theTexture, 
		level,
		width,
		height,
		base_fmt,
		req_fmt,
		pixels);
		
	return theTexture;
}


void glrTranslatePixels(const void * source_pixels, void * dest_pixels, GLenum source_format, GrTextureFormat_t dest_format, GLsizei pixel_count);
void glrTranslatePixels(const void * source_pixels, void * dest_pixels, GLenum source_format, GrTextureFormat_t dest_format, GLsizei pixel_count)
{
	glrTranslatePixelsDispatch(source_pixels, dest_pixels, source_format, dest_format, pixel_count, 1);
}

void glrReloadTexture(GLDContext inContext, glrTexture_t * theTexture);
void glrReloadTexture(GLDContext inContext, glrTexture_t * theTexture)
{
	glrFreeTextureVRAM(inContext, theTexture, 0);
	glrLoadSysTexture(inContext, theTexture, 0);
	grTexSource(GR_TMU0, theTexture->offset[0], GR_MIPMAPLEVELMASK_BOTH, &theTexture->sys_texture);
}



#pragma mark -
#pragma mark /* exported GLD interface routines */



/*
________________________________________________________________________________________

      gldCreateTexture
________________________________________________________________________________________

*/

GLenum
gldCreateTexture(
	GLDContext			inContext,
	const GLDTexture *	inGLDTexture,
	GLuint				inTextureUnit)
{
	glrTexture_t *		theTexture;
	GLenum error;
	
	DEBUG_ENTRY( gldCreateTexture );

	switch( inGLDTexture->target )
	{			
		case GL_TEXTURE_1D:
		case GL_TEXTURE_2D:
			theTexture = glrCreateTexture( inContext, inGLDTexture->target, inGLDTexture->state->objects.name);
			break;
		
		case GL_PROXY_TEXTURE_1D:
		case GL_PROXY_TEXTURE_2D:
		default:
			DEBUG_ERROR( gldCreateTexture, "unknown texture target (%d) \n", inGLDTexture->target );
			break;
	}

	if( !theTexture ) {
		DEBUG_ERROR( gldCreateTexture, "unable to create the texture \n" );
		return GLI_BAD_VALUE;
	}
	
	theTexture->state = inGLDTexture->state;
	
	error = glrImportTexture(inContext, theTexture, inGLDTexture);
	
	/* According to the spec, we need to bind to this texture right now... */
	gldBindTexture(inContext, theTexture->target, theTexture->state->objects.name, inTextureUnit);
	
	return error;
}




/*
________________________________________________________________________________________

      gldModifyTexture
________________________________________________________________________________________

*/

GLenum
gldModifyTexture(
	GLDContext			inContext,
	const GLDTexture *	inGLDTexture,
	GLuint				inTextureUnit,
	GLint				xoffset,
	GLint				yoffset,
	GLsizei				width,
	GLsizei				height)
{

	glrTexture_t * theTexture;
	FxI32 wlod, hlod, ar, lod;
	GLint	i, j, dest_row_bytes;
	GLint	pixel_stretch;
	GLint	row_stretch;
	const unsigned char * source;
	unsigned char * dest;
	GLint dest_bytes_per_pixel;
	GLint source_bytes_per_pixel;
	GLint source_row_bytes;
	
	/****/

	if(height == 0 || width == 0){
		return GLI_NO_ERROR;
	}
	
	theTexture = glrGetTexture(inContext, inGLDTexture->target, inGLDTexture->state->objects.name);
	if(theTexture == NULL) {
		return GL_INVALID_VALUE;
	}
	
	wlod = glrCheckLOD(inGLDTexture->width);		
	hlod = glrCheckLOD(inGLDTexture->height);
	ar = wlod - hlod;
	
	lod = wlod > hlod ? wlod : hlod;
	
	if(theTexture->sys_data[lod] == NULL){
		return GL_INVALID_VALUE;
	}
	
	source_bytes_per_pixel = glrBytesPerSourcePixel(inGLDTexture->base_fmt);
	source_row_bytes = inGLDTexture->width * source_bytes_per_pixel;
	source = (const unsigned char *)inGLDTexture->pixels + (yoffset * source_row_bytes + xoffset * source_bytes_per_pixel);


	if(ar < -MAX_ASPECT_RATIO){
		// texture is too tall
		// stretch width
		pixel_stretch = (1 << (-ar - MAX_ASPECT_RATIO));
		row_stretch = 1;
	} else if (ar > MAX_ASPECT_RATIO){
		// texture is too wide
		// stretch height
		pixel_stretch = 1;
		row_stretch = (1 << (ar - MAX_ASPECT_RATIO));
	} else {
		// texture is just right
		// don't stretch
		pixel_stretch = 1;
		row_stretch = 1;
	}

	dest_bytes_per_pixel = pixel_stretch * bitsPerTexelMap[theTexture->sys_texture.format] >> 3;
	dest_row_bytes = inGLDTexture->width * dest_bytes_per_pixel;
	dest = (unsigned char *)theTexture->sys_data[lod] + (row_stretch * yoffset * dest_row_bytes + xoffset * dest_bytes_per_pixel);
	
	for(i = 0; i < height; i++){
		glrTranslatePixelsDispatch(source, dest, inGLDTexture->base_fmt, theTexture->sys_texture.format, width, pixel_stretch);
		source += source_row_bytes;
		dest += dest_row_bytes;
		// copy the row row_stretch-1 times
		for(j = 1; j < row_stretch; j++){
			memcpy(dest, dest - dest_row_bytes, width * dest_bytes_per_pixel);
			dest += dest_row_bytes;
		}
	}

	
	
	{
		FxI32 lod2;
		lod2 = lod;
		// regenerate smaller versions if needed
		// this could benefit from optimization
		while(lod2 > inContext->max_texture_lod && lod2 <= theTexture->smallLodLog2){
			glrFreeTextureVRAM(inContext, theTexture, 0);
			glrFreeTextureVRAM(inContext, theTexture, 1);
			if(theTexture->sys_data[lod2 -1] != NULL){	
				glmFree(theTexture->sys_data[lod2 - 1]);
				theTexture->sys_data[lod2 -1] = NULL;
			}
			lod2--;
		}
	}	
	
	if(lod <= inContext->max_texture_lod){
		GLint hwTextureUnit;
		for(hwTextureUnit = 0; hwTextureUnit < 2; hwTextureUnit++){
			if(theTexture->resident[hwTextureUnit]){
				grTexDownloadMipMapLevelPartial(
					hwTextureUnit,
					theTexture->offset[hwTextureUnit],
					lod,
					theTexture->sys_texture.largeLodLog2,
		            theTexture->sys_texture.aspectRatioLog2,
					theTexture->sys_texture.format,
					GR_MIPMAPLEVELMASK_BOTH,
					(unsigned char *)theTexture->sys_data[lod] + (row_stretch * yoffset * dest_row_bytes),
					row_stretch * yoffset,
					row_stretch * (yoffset + height) - 1);
			}
		}
	}
	
	return GLI_NO_ERROR;
}




/*
________________________________________________________________________________________

      gldBindTexture
________________________________________________________________________________________

*/

GLenum
gldBindTexture(
	GLDContext			inContext,
	GLenum				inTarget,
	GLuint				inName,
	GLuint				inTextureUnit)
{
	DEBUG_ENTRY( gldBindTexture );

	switch( inTarget )
	{
		case GL_TEXTURE_1D:
			inContext->cur_texture_name[inTextureUnit][0] = inName;
		break;
		case GL_TEXTURE_2D:
			inContext->cur_texture_name[inTextureUnit][1] = inName;
		break;
	}
	

	return GLI_NO_ERROR;
}




/*
________________________________________________________________________________________

      gldDeleteTexture
________________________________________________________________________________________

*/

GLenum
gldDeleteTexture(
	GLDContext			inContext,
	GLuint				inName)
{
	GLuint				theIndex;
	glrTexture_t *		theTexture;
	glrTexture_t *		prev_texture;
	FxU32 i;

	theIndex = inName & GLR_TEXTURE_HASH_MASK;
	
	prev_texture = NULL;
	theTexture = inContext->textures[theIndex];

	while(theTexture) {
		if(inName == theTexture->state->objects.name) break;
		
		prev_texture = theTexture;
		theTexture = theTexture->next;
	}
	
	if (!theTexture) {
		return GLI_NO_ERROR;
	}
	
	if(prev_texture)
		prev_texture->next = theTexture->next;
	else
		inContext->textures[theIndex] = theTexture->next;
	
	glrFreeTextureVRAM(inContext, theTexture, 0);
	glrFreeTextureVRAM(inContext, theTexture, 1);

    for(i = 0; i < GLR_MAX_TEXTURE_LEVEL; i++) {
	  if(theTexture->sys_data[i]){
	  	glmFree(theTexture->sys_data[i]);
	  }
	}
	
	glmFree((char *) theTexture);

    return GLI_NO_ERROR;
}





/*
________________________________________________________________________________________

      gldIsTextureResident
________________________________________________________________________________________

*/

GLboolean
gldIsTextureResident(
	GLDContext			inContext,
	GLuint				inName)
{
	glrTexture_t * theTexture;
	
	theTexture = glrGetTexture(inContext, GL_TEXTURE_2D, inName);
	
	if(theTexture) {
		// if the texture is resident in either of the hw pools then we return true
		return theTexture->resident[0] || theTexture->resident[1];
	} else {
		return GL_FALSE;
	}
}






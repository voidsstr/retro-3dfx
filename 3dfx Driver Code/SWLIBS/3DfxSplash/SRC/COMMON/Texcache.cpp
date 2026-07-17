#include <stdio.h>
/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
** $Header: Texcache.cpp, 4, 10/11/00 7:26:18 PM, Brent$
*/
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glide.h>
#include "texcache.h"

#define printf dummy_printf
#define fflush(a)

/*
 * This structure contains the factors to multiply normalized [0.0, 1.0]
 * texture coordinates by in order to get the texture coordinates that
 * Voodoo Graphics expect.
 */
typedef struct
{
  float sMult;
  float tMult;
} TexCoordFactors;

/*
 * An array which is indexed by GrAspectRatio_t to convert from normalized
 * texture coordinates to Voodoo texture coordinates.
 */
TexCoordFactors aspectToTexCoordFactors[7] = {
  { 256.0f,        256.0f / 8.0f },  /* GR_ASPECT_8x1 */
  { 256.0f,        256.0f / 4.0f },  /* GR_ASPECT_4x1 */
  { 256.0f,        256.0f / 2.0f },  /* GR_ASPECT_2x1 */
  { 256.0f,        256.0f        },  /* GR_ASPECT_1x1 */
  { 256.0f / 2.0f, 256.0f        },  /* GR_ASPECT_1x2 */
  { 256.0f / 4.0f, 256.0f        },  /* GR_ASPECT_1x4 */
  { 256.0f / 8.0f, 256.0f        }   /* GR_ASPECT_1x8 */
};

int dummy_printf( const char *, ... )
{
  return 0;
}


void TexCachePrintState( TexCache *cache )
{
  int i;

  printf( "TexCache State:\n-------\n" );
  printf( "min_address = %ld\n", ( long )cache->min_address );
  printf( "max_address = %ld\n", ( long )cache->max_address );
  printf( "max_entries = %ld\n", ( long )cache->max_entries );
  printf( "num_entries = %ld\n", ( long )cache->num_entries );
  printf( "prev_id = %ld\n", ( long )cache->prev_id );
  //  printf( "next_address = %ld\n", ( long )cache->next_address );
  for( i = 0; i < ( int )cache->num_entries; i++ )
    {
      printf( "entry: %d\n", i );
      printf( "\tmin_address: %ld\n", ( long )cache->entries[i].min_address );
      printf( "\tmem_required: %ld\n", ( long )cache->entries[i].mem_required );
      printf( "\tnext_id: %ld\n", ( long )cache->entries[i].next_id );
    }
}

void TexCacheReset( TexCache *cache )
{
  TexCacheEntry         *dummy_entry;

  /*
   * Set the number of entries in the texture cache to one
   * since there is a dummy node.
   */
  cache->num_entries            = 1;

  /*
   * Set next and prev to point to the dummy node.
   */
  cache->prev_id                = 0;

  /*
   * Set up the dummy node.
   */
  dummy_entry                   = &cache->entries[0];
  dummy_entry->texinfo          = NULL;
  dummy_entry->min_address      = cache->min_address;
  dummy_entry->mem_required     = 0;
  dummy_entry->next_id          = 0;
}

/*
 * MUST add one to max_entries since there is a dummy entry at
 * the first entry.
 */
FxBool TexCacheInit( TexCache *cache, FxU32 min_address, 
                     FxU32 max_address, FxU32 max_entries, GrChipID_t chip,
                     const char *name )
{
  /*
   * Save the chip id that the cache will use.
   */
  cache->chip = chip;

  /*
   * Save the name of the cache for debug statements.
   */
  cache->name = strdup( name );

  /*
   * Allocate space for the texture cache entries.  "entries" includes
   * all textures handled by the cache, not just the ones that are in 
   * texture memory.
   */
  cache->entries = ( TexCacheEntry * )malloc( sizeof( TexCacheEntry ) * ( max_entries + 1 ) );
  if( !cache->entries )
    return FXFALSE;

  /*
   * Store the memory bounds of the texture cache.
   * Snap the min_address to the nearest 8 since you can only
   * download textures on 8 byte boundaries.
   */
  cache->min_address            = ( ( min_address + 7 ) >> 3 ) << 3;
  cache->max_address            = max_address;

  /*
   * Store the number textures that can be handled by this
   * texture cache.
   */
  cache->max_entries            = ( max_entries + 1 );

  /*
   * Set the texture cache to an empty state.
   */
  TexCacheReset( cache );

  return FXTRUE;
}

FxU32 TexCacheInsertTexture( TexCache *cache, GrTexInfo *texinfo )
{
  TexCacheEntry *new_entry;

  /*
   * Do we have an empty entry in the texture cache to add this texture.
   */
  if( cache->num_entries == cache->max_entries )
    return TEX_CACHE_NULL_ID;

  new_entry = &cache->entries[cache->num_entries];

  new_entry->texinfo = texinfo;
  new_entry->min_address = TEX_CACHE_INVALID_ADDRESS;
  new_entry->next_id = TEX_CACHE_NULL_ID;
#ifdef USE_GLIDE3
  new_entry->mem_required = grTexCalcMemRequired( texinfo->smallLodLog2,
                                                  texinfo->largeLodLog2,
                                                  texinfo->aspectRatioLog2,
                                                  texinfo->format );
#else
  new_entry->mem_required = grTexCalcMemRequired( texinfo->smallLod,
                                                  texinfo->largeLod,
                                                  texinfo->aspectRatio,
                                                  texinfo->format );
#endif // USE_GLIDE3

  cache->num_entries++;
  return cache->num_entries - 1;
}

/*
 * Force a texture into texture memory.
 */
FxBool TexCacheForceTexture( TexCache *cache, TexCacheId_t entry_id )
{
  TexCacheEntry *entry;
  TexCacheEntry *prev_entry;
  TexCacheEntry *next_entry;
  FxU32 download_address;
  FxBool wrapped = FXFALSE;
  TexCacheId_t next_id;

  entry = &cache->entries[entry_id];

  extern GlideGraphics *graphics;

#ifdef USE_GLIDE3
	// must convert
	// glide2 version was: 8x1 to 1x8 -- 0 to 6
	// glide3 version is: 8x1 to 1x8 -- 3 to -3
  graphics->SetTexCoordFactors( aspectToTexCoordFactors[3-entry->texinfo->aspectRatioLog2].sMult,
                                aspectToTexCoordFactors[3-entry->texinfo->aspectRatioLog2].tMult );
#else
  graphics->SetTexCoordFactors( aspectToTexCoordFactors[entry->texinfo->aspectRatio].sMult,
                                aspectToTexCoordFactors[entry->texinfo->aspectRatio].tMult );
#endif // USE_GLIDE3

  /*
   * Is the texture already in the cache?
   */
  if( entry->min_address != TEX_CACHE_NULL_ID )
    return FXTRUE;
  
  prev_entry = &cache->entries[cache->prev_id];

#if 0
  printf( "cache miss: \"%s\" ", cache->name );
#endif

  /*
   * Is the texture too big to fit in the cache?
   */
  if( cache->max_address - cache->min_address < entry->mem_required )
    return FXFALSE;

  /*
   * Is there enough space between the end of the most recently
   * allocated texture and the end of the texture memory
   * set aside to download this texture?
   */
  download_address = prev_entry->min_address + prev_entry->mem_required;

  /*
   * Snap the download address to the nearest 8 since all textures have
   * to be aligned as such.  Round up.
   */
  download_address = ( ( download_address + 7 ) >> 3 ) << 3;

  /*
   * Given the download_address and the mem_required for this texture,
   * will it fit between the download_address and the end of 
   * texture memory?
   */
  if( download_address + entry->mem_required > cache->max_address )
    {
      download_address = cache->min_address;

      wrapped = FXTRUE;
    }

  next_id = prev_entry->next_id;
  next_entry = &cache->entries[next_id];

  /*
   * If we have wrapped, clear out from the end of the previously
   * allocated to the end of texture memory.
   */
  if( wrapped )
    {
      while( next_entry->min_address > download_address )
        {
          /*
           * If we hit the dummy node, then unlink it.
           */
          if( next_entry->min_address == TEX_CACHE_INVALID_ADDRESS )
            {
              next_id = next_entry->next_id;
              next_entry = &cache->entries[next_id];
              continue;
            }
          next_entry->min_address = TEX_CACHE_INVALID_ADDRESS;
          next_id = next_entry->next_id;
          next_entry = &cache->entries[next_id];

        }
    }

  /*
   * Invalidate textures that are being downloaded over.
   */
  while( next_entry->min_address < download_address + entry->mem_required )
    {
      /*
       * Check if we have wrapped.
       */
      if( next_entry->min_address + next_entry->mem_required < download_address )
        break;
      
      next_entry->min_address = TEX_CACHE_INVALID_ADDRESS;
      next_id = next_entry->next_id;
      next_entry = &cache->entries[next_id];
    }

  /*
   * Link in the new texture.
   */
  prev_entry->next_id = entry_id;
  entry->min_address = download_address;
  entry->next_id = next_id;

  /*
   * Download the texture.
   */
  printf( "\"%s\": TMU%d\n", cache->name, cache->chip );
  fflush( stdout );

  grTexDownloadMipMap( cache->chip, download_address, 
                       GR_MIPMAPLEVELMASK_BOTH, entry->texinfo );

  cache->prev_id = entry_id;

  return FXTRUE;
}

/*
 * Set a texture current. . download if necessary.
 */
FxBool TexCacheSetCurrent( TexCache *cache, TexCacheId_t entry_id,
                           FxBool always_download )
{
  /*
   * Force the texture to be put into texture memory.
   */
  if( !TexCacheForceTexture( cache, entry_id ) )
    return FXFALSE;

  if( always_download )
    {
      grTexDownloadMipMap( cache->chip, cache->entries[entry_id].min_address, 
                           GR_MIPMAPLEVELMASK_BOTH, cache->entries[entry_id].texinfo );
    }

  /*
   * Set the texture current.
   */
  grTexSource( cache->chip, cache->entries[entry_id].min_address, 
               GR_MIPMAPLEVELMASK_BOTH, cache->entries[entry_id].texinfo );

  return FXTRUE;
}

#define MIN(a, b)  ((a) < (b)) ? (a) : (b)
#define MAX(a, b)  ((a) > (b)) ? (a) : (b)

FxBool LoadSplashTextureTGA(ResMem *res_mem, SplashTexture *texture)
{
	TGA_Header header;
	PaletteEntry palette[256];
	FxU32 i, j, image_size;
	FxU32 width, height;
	FxU32 bytes, packet_header, packet_size;
	FxU8 *tmp_data;
	FxU16 *data, *curr_data;
	ARGB32bpp *texels, *curr_texel;

	texels = NULL;
	tmp_data = NULL;
	data = NULL;

	if (mread(&header, sizeof(TGA_Header), 1, res_mem) != 1) goto ERROR_CLEANUP;

	width = (header.width_hi<<8) | header.width_lo;
	height = (header.height_hi<<8) | header.height_lo;

	image_size = width*height;

	// store the texture in 32bpp format
	// to be used later for converting to other formats
	// and creating mipmaps
	texels = new ARGB32bpp[image_size];
	if (!texels) goto ERROR_CLEANUP;

	// read in the palette (if colormapped, i.e. bpp == 8)
	if (header.bpp == 8)
	{
		FxU32 num_entries;
		if (!header.b_color_mapped || header.color_map_entry_size != 24) goto ERROR_CLEANUP;
		num_entries = (header.color_map_entries_hi<<8) | header.color_map_entries_lo;
		if (mread(&palette[0], sizeof(PaletteEntry), num_entries, res_mem) != num_entries) goto ERROR_CLEANUP;
	}

	// read in the data
	bytes = header.bpp/8;
	tmp_data = new FxU8[bytes*image_size];
	if (!tmp_data) goto ERROR_CLEANUP;

	if (header.image_type & TGA_RLE_MASK) // decompress
	{
		FxU8 *dst;

		dst = tmp_data;
		for (i=0; i<image_size; )
		{
			// get the packet header
			packet_header = mgetc(res_mem);
			packet_size = 1 + (packet_header & 0x7f);

			if (packet_header & 0x80) // RLE packet
			{
				if (mread(dst, bytes, 1, res_mem) != 1) goto ERROR_CLEANUP;
				for (j=1; j<packet_size; j++)
				{
					memcpy(&dst[bytes*j], &dst[0], bytes);
				}
			}
			else
			{
				if (mread(dst, bytes, packet_size, res_mem) != packet_size) goto ERROR_CLEANUP;
			}

			i += packet_size;
			dst += bytes*packet_size;
		}
	}
	else
	{
		if (mread(tmp_data, bytes*sizeof(FxU8), image_size, res_mem) != image_size) goto ERROR_CLEANUP;
	}

	// convert the texture from 8, 16, or 24 bpp to a 32 bpp argb format
	// ******** 8 bpp ********
	if (header.bpp == 8)
	{
		// convert the 8 bpp color mapped texture to a 32 bpp ARGB-8888 texture
		for (i=0; i<image_size; i++)
		{
			texels[i].blue  = palette[tmp_data[i]].blue;
			texels[i].green = palette[tmp_data[i]].green;
			texels[i].red   = palette[tmp_data[i]].red;
			texels[i].alpha = 0;
		}
	}
	// ******** 16 bpp ********
	else if (header.bpp == 16)
	{
		// convert the 15 bpp RGB-555 texture to a 32 bpp ARGB-8888 texture
		for (i=0; i<image_size; i++)
		{
			texels[i].blue  = (FxU8)( (float)((((FxU16 *)tmp_data)[i] & 0x001f)    )*(256.0f/32.0f) );
			texels[i].green = (FxU8)( (float)((((FxU16 *)tmp_data)[i] & 0x03e0)>>5 )*(256.0f/32.0f) );
			texels[i].red   = (FxU8)( (float)((((FxU16 *)tmp_data)[i] & 0x7c00)>>10)*(256.0f/32.0f) );
			texels[i].alpha = 0;
		}
	}
	// ******** 24 bpp ********
	else if (header.bpp == 24)
	{
		// convert the 24 bpp RGB-888 texture to a 32 bpp ARGB-8888 texture
		for (i=0; i<image_size; i++)
		{
			texels[i].blue  = tmp_data[3*i + 0];
			texels[i].green = tmp_data[3*i + 1];
			texels[i].red   = tmp_data[3*i + 2];
			texels[i].alpha = 0;
		}
	}
	// ******** 32 bpp ********
	else if (header.bpp == 32)
	{
		// copy the 32 bpp ARGB-8888 texture to another 32 bpp ARGB-8888 texture
		for (i=0; i<image_size; i++)
		{
			texels[i].blue  = tmp_data[4*i + 0];
			texels[i].green = tmp_data[4*i + 1];
			texels[i].red   = tmp_data[4*i + 2];
			texels[i].alpha = tmp_data[4*i + 3];
		}
	}
	else // unknown bpp, fail!!!!
	{
		goto ERROR_CLEANUP;
	}

	delete [] tmp_data;
	tmp_data = NULL;

	// data will be 16 bpp
	data = new FxU16[image_size];
	if (!data) goto ERROR_CLEANUP;

	// convert the 32 bpp argb to 565-rgb
	// also flip the image since tgas are upside down
	curr_texel = texels;
	for (i=0; i<height; i++)
	{
		curr_data = &data[(height-i-1)*width];
		for (j=0; j<width; j++)
		{
			*curr_data = ((curr_texel->red  >>3)<<11) |
									 ((curr_texel->green>>2)<< 5) |
										(curr_texel->blue >>3);
			curr_texel++;
			curr_data++;
		}
	}

	// we don't need the 32 bpp texels anymore, delete them
	delete [] texels;
	texels = NULL;

	texture->width = width;
	texture->height = height;
	texture->data = data;
	texture->stride = width*2;

	return FXTRUE;

ERROR_CLEANUP:
	if (texels)
	{
		delete [] texels;
		texels = NULL;
	}
	if (tmp_data)
	{
		delete [] tmp_data;
		tmp_data = NULL;
	}
	if (data)
	{
		delete [] data;
		data = NULL;
	}

	return FXFALSE;
}

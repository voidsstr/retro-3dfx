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
** $Header: texcache.cpp, 4, 10/11/00 7:41:23 PM, Brent$
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

static void ConvertTexInfoTo3dfInfo( GrTexInfo *texinfo, Gu3dfInfo *tdfinfo )
{
  FxU32 largest_length;

  tdfinfo->header.small_lod     = texinfo->smallLod;
  tdfinfo->header.large_lod     = texinfo->largeLod;
  tdfinfo->header.aspect_ratio  = texinfo->aspectRatio;
  tdfinfo->header.format        = texinfo->format;
  tdfinfo->mem_required         = grTexCalcMemRequired( texinfo->smallLod,
                                                        texinfo->largeLod,
                                                        texinfo->aspectRatio,
                                                        texinfo->format );
  tdfinfo->data                 = texinfo->data;
  
  /*
   * Need to figure out width and height for the 3dfinfo since
   * it isn't explictly stored in the texinfo.
   */
  switch( tdfinfo->header.large_lod )
    {
    case GR_LOD_256:
      largest_length = 256;
      break;
    case GR_LOD_128:
      largest_length = 128;
      break;
    case GR_LOD_64:
      largest_length = 64;
      break;
    case GR_LOD_32:
      largest_length = 32;
      break;
    case GR_LOD_16:
      largest_length = 16;
      break;
    case GR_LOD_8:
      largest_length = 8;
      break;
    case GR_LOD_4:
      largest_length = 4;
      break;
    case GR_LOD_2:
      largest_length = 2;
      break;
    case GR_LOD_1:
      largest_length = 1;
      break;
    }

  switch( tdfinfo->header.aspect_ratio )
    {
    case GR_ASPECT_8x1:
      tdfinfo->header.width = largest_length;
      tdfinfo->header.height = largest_length >> 3;
      break;
    case GR_ASPECT_4x1:
      tdfinfo->header.width = largest_length;
      tdfinfo->header.height = largest_length >> 2;
      break;
    case GR_ASPECT_2x1:
      tdfinfo->header.width = largest_length;
      tdfinfo->header.height = largest_length >> 1;
      break;
    case GR_ASPECT_1x1:
      tdfinfo->header.width = largest_length;
      tdfinfo->header.height = largest_length;
      break;
    case GR_ASPECT_1x2:
      tdfinfo->header.width = largest_length >> 1;
      tdfinfo->header.height = largest_length;
      break;
    case GR_ASPECT_1x4:
      tdfinfo->header.width = largest_length >> 2;
      tdfinfo->header.height = largest_length;
      break;
    case GR_ASPECT_1x8:
      tdfinfo->header.width = largest_length >> 3;
      tdfinfo->header.height = largest_length;
      break;
    }
}

static void Convert3dfInfoToTexInfo( Gu3dfInfo *tdfinfo, GrTexInfo *texinfo )
{
  texinfo->smallLod     = tdfinfo->header.small_lod;
  texinfo->largeLod     = tdfinfo->header.large_lod;
  texinfo->aspectRatio  = tdfinfo->header.aspect_ratio;
  texinfo->format       = tdfinfo->header.format;
  texinfo->data         = tdfinfo->data;
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
  new_entry->mem_required = grTexCalcMemRequired( texinfo->smallLod,
                                                  texinfo->largeLod,
                                                  texinfo->aspectRatio,
                                                  texinfo->format );

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

  graphics->SetTexCoordFactors( aspectToTexCoordFactors[entry->texinfo->aspectRatio].sMult,
                                aspectToTexCoordFactors[entry->texinfo->aspectRatio].tMult );

  /*
   * Is the texture already in the cache?
   */
  if( entry->min_address != TEX_CACHE_NULL_ID )
    return FXTRUE;
  
  prev_entry = &cache->entries[cache->prev_id];

#if 1
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

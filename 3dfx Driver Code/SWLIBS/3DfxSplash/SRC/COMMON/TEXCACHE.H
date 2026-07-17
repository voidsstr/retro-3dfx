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
** $Header: TEXCACHE.H, 4, 10/11/00 7:26:17 PM, Brent$
*/


#ifndef __TEXCACHE_H__
#define __TEXCACHE_H__

#if 0
#ifdef __cplusplus
extern "C" {
#endif
#endif

#include <glide.h>
#include "GlideGraphics.h"
#include "convexhull.h"

#define TEX_CACHE_INVALID_ADDRESS ( ( FxU32 )( 0xffffffff ) )

typedef FxU32 TexCacheId_t;
#define TEX_CACHE_NULL_ID ( ( FxU32 )( 0xffffffff ) )

typedef struct
{
  GrTexInfo *texinfo;

  /*
   * Min and max addresses in texture memory for this texture.
   * If min_address and max_address == 0xffffffff, then this
   * texture is not in the cache.
   */
  FxU32 min_address;
  FxU32 mem_required;
  FxU32 next_id;
} TexCacheEntry;

typedef struct
{
  GrChipID_t chip;

  /*
   * Addresses in texture memory for the beginning and
   * end of this texture cache pool.
   */
  FxU32                 min_address;
  FxU32                 max_address;

  /*
   * The maximum number of textures that can be handled by this 
   * texture cache.  Defined when calling TexCacheInit.
   */
  FxU32                 max_entries;

  /*
   * The number of textures currently handled by the this texture
   * cache.
   */
  FxU32                 num_entries;

  /*
   * The ID (index into entries) of the most recently downloaded
   * texture.  Keep this around so that linking into the 
   * circular linked list of downloaded textures is easy.
   */
  FxU32                 prev_id;

#if 0
  /*
   * The next available address is texture memory to download to.
   */
  FxU32                 next_address;
#endif

  /*
   * Info about all textures that are handled by this texture
   * cache.
   */
  TexCacheEntry         *entries;

  char *name;
} TexCache;

void TexCachePrintState( TexCache *cache );
FxBool TexCacheInit( TexCache *cache, FxU32 min_address, 
                     FxU32 max_address, FxU32 max_entries, GrChipID_t chip,
                     const char *name );
void    TexCacheReset( TexCache *cache );
FxU32   TexCacheInsertTexture( TexCache *cache, GrTexInfo *texinfo );
FxBool  TexCacheForceTexture( TexCache *cache, TexCacheId_t entry_id );
FxBool  TexCacheSetCurrent( TexCache *cache, TexCacheId_t entry_id,
                            FxBool always_download );

// tga image types
#define TGA_TYPE_MASK  0x3
#define TGA_RLE_MASK  0x8
#define TGA_NULL	0
#define TGA_CMAP	1
#define TGA_TRUE	2
#define TGA_MONO	3
#define TGA_CMAP_RLE	(TGA_CMAP | TGA_RLE_MASK)
#define TGA_TRUE_RLE	(TGA_TRUE | TGA_RLE_MASK)
#define TGA_MONO_RLE	(TGA_MONO | TGA_RLE_MASK)

typedef struct
{
	FxU8 id_length;
	FxU8 b_color_mapped;
	FxU8 image_type;
	FxU8 color_map_start_lo; // tmu_index of first...
	FxU8 color_map_start_hi; // ...color map entry
	FxU8 color_map_entries_lo; // number of entries...
	FxU8 color_map_entries_hi; // ...in color map
	FxU8 color_map_entry_size;
	FxU8 x_origin_lo;
	FxU8 x_origin_hi;
	FxU8 y_origin_lo;
	FxU8 y_origin_hi;
	FxU8 width_lo;
	FxU8 width_hi;
	FxU8 height_lo;
	FxU8 height_hi;
	FxU8 bpp;
	FxU8 reserved;
} TGA_Header;

typedef struct
{
	FxU8 blue;
	FxU8 green;
	FxU8 red;
} PaletteEntry;

typedef struct
{
	FxU8 blue;
	FxU8 green;
	FxU8 red;
	FxU8 alpha;
} ARGB32bpp;

typedef struct _splash_texture_
{
	int width, height;
	void *data;
	int stride; // in bytse
	GrLfbWriteMode_t format;
} SplashTexture;

FxBool LoadSplashTextureTGA(ResMem *res_mem, SplashTexture *texture);

#if 0

#ifdef __cplusplus
}
#endif
#endif

#endif /* __TEXCACHE_H__ */

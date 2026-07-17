#ifdef __GL_CODEGEN
/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
** Online-generated code cache
*/

#include "context.h"
#include "global.h"
#include "stdlib.h"

#define __GL_ENVVARS 1

#ifdef __GL_ENVVARS
static GLboolean disable_cache = GL_FALSE;
#endif


#if 0
#define CODE_ALIGN 4096		/* Align code on 4096-byte boundaries */
#endif
#define CODE_ALIGN 1		/* Don't do any alignment */

static void __glDoInitializeCodeCache(__GLcontext *gc, __GLcodeCache *cache,
				      GLuint codeCount, GLuint codeSize)
{
    unsigned int ii;

#ifdef __GL_ENVVARS
    static int once = 1;

    if (once) {
	char *env = getenv("__GL_DISABLE_OG_CACHE");
	if (env)
	    disable_cache = atoi(env) ? GL_TRUE : GL_FALSE;
    }
#endif

    assert(NULL == cache->codeInfo);
    assert(NULL == cache->codeSpace);

    cache->codeInfo = (__GLcodeCell *)
	(*gc->imports.malloc)(gc, codeCount * sizeof(__GLcodeCell));
    cache->codeSpace =
	(*gc->imports.malloc)(gc, (codeCount * codeSize) + (CODE_ALIGN-1));
#if (CODE_ALIGN != 1)
    cache->codeSpace =
	(void*)(((int)cache->codeSpace + (CODE_ALIGN-1)) & ~(CODE_ALIGN-1));
#endif

    cache->codeList = NULL;

    for (ii = 0; ii < codeCount; ii++) {
	cache->codeInfo[ii].modeFlags = ~0L;	/* illegal mode == unused */
	cache->codeInfo[ii].signature = 0;
	cache->codeInfo[ii].code =
	    ((GLubyte *)cache->codeSpace) + ii * codeSize;
	cache->codeInfo[ii].next = cache->codeList;
	cache->codeList = &cache->codeInfo[ii];
    }
}

void __glInitializeCodeCache(__GLcontext *gc)
{
    __glDoInitializeCodeCache(gc, &gc->textureReadCache,
			      gc->constants.textureReadCacheCount,
			      gc->constants.textureReadCacheSize);

    __glDoInitializeCodeCache(gc, &gc->depthTestCache,
			      gc->constants.depthTestCacheCount,
			      gc->constants.depthTestCacheSize);

    __glDoInitializeCodeCache(gc, &gc->triSetupCache,
			      gc->constants.triSetupCacheCount,
			      gc->constants.triSetupCacheSize);

    __glDoInitializeCodeCache(gc, &gc->triRasterCache,
			      gc->constants.triRasterCacheCount,
			      gc->constants.triRasterCacheSize);

    __glDoInitializeCodeCache(gc, &gc->drawVertexesCache,
			      gc->constants.drawVertexesCacheCount,
			      gc->constants.drawVertexesCacheSize);

    assert(NULL == gc->triSetupData);
    gc->triSetupData =
	(gc->imports.malloc)(gc, gc->constants.triSetupDataSize);
}

static void __glDoReleaseCodeCache(__GLcontext *gc, __GLcodeCache *cache)
{
    (*gc->imports.free)(gc, cache->codeInfo);
    (*gc->imports.free)(gc, cache->codeSpace);

    cache->codeInfo = NULL;
    cache->codeSpace = NULL;
}

void __glReleaseCodeCache(__GLcontext *gc)
{
    __glDoReleaseCodeCache(gc, &gc->textureReadCache);
    __glDoReleaseCodeCache(gc, &gc->depthTestCache);
    __glDoReleaseCodeCache(gc, &gc->triSetupCache);
    __glDoReleaseCodeCache(gc, &gc->triRasterCache);
    __glDoReleaseCodeCache(gc, &gc->drawVertexesCache);
    (gc->imports.free)(gc, gc->triSetupData);
    gc->triSetupData = NULL;
}

static void __glDoInvalidateCodeCache(__GLcontext *gc, __GLcodeCache *cache)
{
    __GLcodeCell *cell = cache->codeList;

    while (cell) {
	cell->modeFlags = ~0L;
	cell = cell->next;
    }
}

void __glInvalidateCodeCache(__GLcontext *gc)
{
    __glDoInvalidateCodeCache(gc, &gc->triSetupCache);
    __glDoInvalidateCodeCache(gc, &gc->triRasterCache);
}

/* Search cache for existing code space which satisfies current settings,
 * or an unused code space.  Returns GL_TRUE if the returned space meets
 * request, GL_FALSE otherwise.
 */
GLboolean __glAllocCodeSpace(__GLcodeCache *cache, GLuint modeFlags,
			     GLuint signature, __GLnameSpace **ppns,
			     GLvoid **code)
{
    __GLcodeCell *cell, *prev;

    /* Search for current request in LRU cache */
    cell = cache->codeList;
    prev = NULL;

#ifdef __GL_ENVVARS
    if (disable_cache) {
	*code = cell->code;
	if (ppns)
	  *ppns = &(cell->tr);
	return GL_FALSE;
    }
#endif

    while (1) {
	if ((cell->modeFlags == modeFlags && cell->signature == signature) ||
	    (cell->modeFlags == ~0L)) {

	    /* Move existing proc or free cell move to front of MRU cache */
	    if (prev) {
		prev->next = cell->next;
		cell->next = cache->codeList;
		cache->codeList = cell;
	    } else {
		assert(cell == cache->codeList);
	    }

	    if (ppns)
	      *ppns = &(cell->tr);
	    *code = cell->code;

	    if (cell->modeFlags == ~0L) {
		/* cell is free, store keys */
		cell->modeFlags = modeFlags;
		cell->signature = signature;
		return GL_FALSE;
	    } else {
		return GL_TRUE;
	    }
	}

	if (cell->next) {
	    /* Try next element */
	    prev = cell;
	    cell = cell->next;
	} else {
	    /* We've hit the end of the queue.  Move this element to
	     * the head and return.
	     */
	    cell->modeFlags = modeFlags;
	    cell->signature = signature;
	    if (ppns)
	      *ppns = &(cell->tr);
	    *code = cell->code;
	    cell->next = cache->codeList;
	    cache->codeList = cell;
	    prev->next = NULL;
	    return GL_FALSE;
	}
    }
}
#endif

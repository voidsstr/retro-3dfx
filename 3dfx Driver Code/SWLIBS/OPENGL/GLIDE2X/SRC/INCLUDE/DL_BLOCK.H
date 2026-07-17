#ifndef __gl_dlblock_h
#define __gl_dlblock_h

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
** Display list internal structure description.
**
** $Revision: 4$
** $Date: 10/11/00 7:52:03 PM$
*/

/*
** Here is the state for the block allocator contained in dl_block.c.
** The calling program should probably not be looking directly at and
** certainly should not be changing any of the values.
*/
typedef struct __GLdlistBlockStateRec{
    unsigned int block_size;
    unsigned int region_size;

    char *region_addr;

    void *(*sbrk)(int, void *);
    void *(*memmove)(void *, const void *, size_t, void *);
    void *sbrk_state;

    unsigned int *memmap;
    unsigned int blocks_free;

    /* XXXCF hackity hackity hackity */
    void *last_free;
    unsigned *transmap;
    unsigned blocks_transient;
} __GLdlistBlockState;

/*************************************************************
 *		       Block Allocator Routines              *
 *************************************************************/

/*
** Initialize a BlockState.  sbrk and memmove are function pointers to
** the appropriate routines.  Returns zero on failure (malloc or sbrk failing),
** non-zero otherwise.  
** All the other block state routines are in dlistint.h -- this one needs
** to be called (for now) by the context creation routine.
*/
extern int __glDlistBlockStateInit(__GLcontext *gc,
				   unsigned int block_size,
				   void *(*sbrk)(int, void*),
				   void *(*memmove)(void *, const void *, 
						    size_t, void *),
				   void *sbrk_state,
				   __GLdlistBlockState *);

/*
** Free all the memory associated with a block state, but not the
** BlockState structure itself.  Returns zero on failure, non-zero on
** success.
*/
extern int __glDlistBlockStateFree(__GLcontext *gc, __GLdlistBlockState *);

/* 
** Has a BlockState been initialized?  Non-zero if yes, zero if no.
*/
extern int __glDlistBlockStateIsInitialized(__GLdlistBlockState *);

/*
** Prealloc n blocks.  This reserves space.  If this routine returns
** true, the next n calls to AllocBlock will succeed. 
*/
extern int __glDlistPreallocBlocks(__GLdlistBlockState *b, int n);

/*
** Alloc a block.  Returns 0 for failure of malloc/realloc or sbrk
*/
extern void *__glDlistAllocBlock(__GLdlistBlockState *);

/*
** Free a block.
*/
extern void __glDlistFreeBlock(__GLdlistBlockState *, void *ptr);

/*
** Perform compaction on block memory.  Pretty useless right now since 
** there's no way to tell where your blocks got moved to.  Needs a new API.
** Returns 0 if realloc or sbrk fails, non-zero otherwise.
*/
extern int __glDlistCompactBlocks(__GLdlistBlockState *);

/*
** Returns true if p came from the block allocator, false 
** otherwise. May return the wrong thing if p came from the block allocator,
** but has subsequently been freed or was not updated after a compaction
** operation.  p does not need to point to the start of a block.
*/
extern int __glDlistIsBlock(__GLdlistBlockState *, void *);

extern void __glDlistTransientBlock(__GLdlistBlockState *b, void *ptr);
extern void __glDlistFreeTransient(__GLdlistBlockState *b);

#endif /* __gl_dlblock_h */

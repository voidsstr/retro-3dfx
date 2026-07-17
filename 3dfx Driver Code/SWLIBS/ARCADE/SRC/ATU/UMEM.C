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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 4$ 
** $Date: 10/11/00 7:34:54 PM$ 
**
*/

#include <stdio.h>
#include "atutil.h"

static FxU32 peakMemAllocated = 0;
static FxU32 currentMemAllocated = 0;

#ifdef AT_DEBUGGING
static _AtuMemEntry *head = 0;
static _AtuMemEntry *tail = 0;
#endif

static FxU32 debugFlags = 0;

/*-------------------------------------------------------------------
  Function: atuMemCheck
  Date: 11/9/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description:
    Validate the allocated memory
  Arguments:
    None
  Return:
    FXTRUE if allocated memory ok, FXFALSE otherwise
  -------------------------------------------------------------------*/

static void
_atuMemInit( void *ptr ) {
    _AtuMemEntry *e = ptr;
    FxU8 *p;
    int i;

    /* init before block */
    p = e->gap;
    for ( i = 0; i < AT_GAP_SIZE; i++ ) {
        *p++ = AT_MEM_GAP;
    } 
    /* init after block */
    p = p+e->size;
    for ( i = 0; i < AT_GAP_SIZE; i++ ) {
        *p++ = AT_MEM_GAP ;
    } 
}

FxBool
_atuMemCheck( char *where, FxU32 line ) {
#ifdef AT_DEBUGGING
    _AtuMemEntry *current;
    FxU8 *p;
    int i;

    for( current = head; current; current = current->next ) {
        /* check before block */
        p = current->gap;
        for ( i = 0; i < AT_GAP_SIZE; i++ ) {
            if ( *p++ != AT_MEM_GAP ) {
                atuError(FXTRUE, "%s(%d) atuMemCheck: corrupted memory area\n",
                         where, line);
                return FXFALSE;
            }
        } 
        /* check after block */
        p = p+current->size;
        for ( i = 0; i < AT_GAP_SIZE; i++ ) {
            if ( *p++ != AT_MEM_GAP ) {
                atuError(FXTRUE, "%s(%d) atuMemCheck: corrupted memory area\n",
                         where, line);
                return FXFALSE;
            }
        } 
    }
#endif
    return FXTRUE;
}

static void 
_atuMemLink(void *ptr) {
#ifdef AT_DEBUGGING
   _atuMemInit( ptr );

    if ( !head ) {
        tail = head = ptr;
    } else {
        tail->next = ptr;
        tail = tail->next;
    }
#endif
}

static void 
_atuMemUnlink( void *mem, char *where, FxU32 line ) {
#ifdef AT_DEBUGGING
    _AtuMemEntry *current, *previous = NULL;

    if ( mem == NULL ) {
        atuError( FXTRUE, "%s(%d) atuMemUnlink(): trying to unlink NULL ptr\n",
                  where, line);
    }

    for( current = head; current; current = current->next ) {
        if ( mem == current->pointer ) {
            break;
        }
        previous = current;
    }
    if ( !current ) {
        atuError( FXTRUE, "%s(%d) atuMemUnlink(): "
                          "Tried to free memory not allocated with the "
                          "atuMem* api.\n",
                          where, line );
    }

    /* unlink from allocated memory list */

    if (!previous) {
        head = current->next;
    } else {
        previous->next = current->next;
    }
       
    currentMemAllocated -= current->size;

    if ( tail == current )
        tail = previous;
#endif
}

/*-------------------------------------------------------------------
  Function: atuMemRealloc
  Date: 11/9/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Just like CAPI realloc, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes. If the input pointer is NULL then this acts just
    like atuMemAlloc.
  Arguments:
    ptr  - pointer to allocated memory
    size - size in bytes of to be allocated
    where - source file where function was called
    line  - line where function was called
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/

void *_atuMemRealloc(void *ptr, size_t size, char *where, FxU32 line ) {
    size_t bytes_needed;
    _AtuMemEntry *e ;

    if ( debugFlags & ATU_MEM_CHECK )
        _atuMemCheck( where, line );

    if ( !size ) {
        atuError( 
          FXTRUE, 
          "%s(%d) atuMemRealloc(): Error tried to allocate 0 byte chunk.\n",
          where, line );
    }

    bytes_needed = sizeof(_AtuMemEntry)+size+AT_GAP_SIZE;

    if ( ptr != NULL ) {
        e = atuMemEntry(ptr);
        if ( e != e->actual_pointer ) {
            atuError(FXTRUE, "%s(%d): atuMemRealloc: atempt to resize aligned "
                             "pointer\n", where, line);
        }
        _atuMemUnlink(ptr, where, line);
        e = realloc( e, bytes_needed );
    } else {
        e = calloc( bytes_needed, 1 );
    }

    if ( !e ) {
        atuError( FXTRUE, "%s(%d) atuMemRealloc(): allocation failure\n",
                          where, line );
    }

    if ( ptr == NULL ) {
        e->use_count = 0;
        e->type = 0;
    }

    e->size = size;
    currentMemAllocated += size;
    if ( currentMemAllocated > peakMemAllocated ) 
        peakMemAllocated = currentMemAllocated;
    e->actual_pointer =  e;
    e->pointer = (void *)(e+1);
    e->where = where;
    e->line = line;
    e->next = NULL;
    _atuMemLink(e);
    return e->pointer;
}

/*-------------------------------------------------------------------
  Function: _atuStrDup
  Date: 1/15/97
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Just like CAPI strdup, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes. 
  Arguments:
    s     - string to be duplicated
    where - source file where function was called
    line  - line where function was called
  Return:
    duplicated string
  -------------------------------------------------------------------*/

char *
_atuStrDup(char *s, char *where, FxU32 line ) {
    size_t nBytes = strlen(s)+1;
    char *t;

    t = _atuMemRealloc(NULL, nBytes, where, line );
    strcpy(t, s);

    return t;
}

/*-------------------------------------------------------------------
  Function: _atuMemAlignedCalloc
  Date: 3/18/96
        4/24/96 added support for reference counting and a type field
		reduced the number of allocs performed to 1.
  Implementor(s): jdt
  Library: AT Util
  Description:
    Return a chunk of cleared memory that is aligned to a given memory
    boundary. A memory area created with this function may NOT be resized
    later. 
  Arguments:
    line - line where function was called
    where - source file where function was called
    alignment - memory boundary on which to align first element
    size - number of bytes per element
    num - number of elements to allocate
  Return:
    void pointer to memory
  -------------------------------------------------------------------*/
void *_atuMemAlignedCalloc( size_t num, 
                            size_t size, 
                            FxU32 alignment, 
                            char *where, 
                            FxU32 line ) {
    size_t bytes_needed;
    char *ptr, *dptr;
    _AtuMemEntry *e;

    if ( debugFlags & ATU_MEM_CHECK )
        _atuMemCheck( where, line );

    if ( !size ) {
        atuError( 
          FXTRUE, 
          "%s(%d) atuMemAlignedCalloc(): Error tried"
          " to allocate 0 byte chunk.\n",
          where, line );
    }

    if ( !alignment ) {
        atuError( FXTRUE,
                  "%s(%d) atuMemAlignedCalloc(): Bad alignment.\n" );
    }

    /* ensure alignment of memory is at least on a 4 byte boundary */

    alignment = ( alignment > 4 ) ? alignment : 4;

    bytes_needed = sizeof(_AtuMemEntry) + num * size + alignment+AT_GAP_SIZE;

    ptr = calloc( bytes_needed, 1 );

    if ( !ptr ) {
       atuError( FXTRUE, 
                 "%s(%d) atuMemAlignedCalloc(): Bookeeping memory "
                 "allocation failure\n",
                 where, 
                 line );
    }

    /* compute pointer to aligned data area */

    dptr = (void *)(ptr+sizeof(_AtuMemEntry));

    {
        FxU32 remainder = ((FxU32)(dptr)) % alignment;
        if ( remainder ) {
            dptr = (void*)(((FxU32)dptr) + alignment - remainder);
        }
    }        

    /* ensure object header immediately precedes data area */
    
    e = (void *)(dptr-sizeof(_AtuMemEntry));

    e->size = num * size;
    currentMemAllocated += e->size;
    if ( currentMemAllocated > peakMemAllocated ) 
        peakMemAllocated = currentMemAllocated;

    _atuMemInit( e );

    e->pointer = dptr;
    e->actual_pointer = ptr;
    e->where = where;
    e->line = line;
    e->use_count = 0;
    e->type = 0;
    e->next = NULL;
    _atuMemLink(e);
    return e->pointer;
}

/*-------------------------------------------------------------------
  Function: atuMemStats
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( memory management )
  Description:
    Fills a structure with simple memory allocation statistics.
  Arguments:
    s - pointer to an AtuMemStat structure to be filled with
        information about dynamically allocated memory.
  Return:
    none
  -------------------------------------------------------------------*/
void atuMemStats( AtuMemStats *s ) {
    s->peakMemAllocated = peakMemAllocated;
    s->currentMemAllocated = currentMemAllocated;
}

/*-------------------------------------------------------------------
  Function: _atuMemFree
  Date: 3/15/96
  Implementor(s): jdt
  Library: AT Util ( memory managment )
  Description:
    Like CAPI free but with memory tracking and parameter checking.
    This function is paired with a macro that automates source file/
    line number tracking.
  Arguments:
    mem - pointer to memory on the heap to be freed
    where - pointer to filename where function was called
    line - line number in source file where funciton is called    
  Return:
    none
  -------------------------------------------------------------------*/
void _atuMemFree( void *mem, char *where, FxU32 line ) {
    if ( debugFlags & ATU_MEM_CHECK )
        _atuMemCheck( where, line );

    if ( mem == NULL )
        return;

    _atuMemUnlink( mem, where, line );

    if ( debugFlags & ATU_MEM_NO_FREE ) {
       _AtuMemEntry *e = mem;
       FxU8 *p;
       int i;

       /* mark block as deleted */
       p = e->gap;
       for ( i = 0; i < AT_GAP_SIZE; i++ ) {
           *p++ = AT_MEM_FREE;
       } 
    } else {
        free( atuMemEntry(mem)->actual_pointer);
    }
}

/*-------------------------------------------------------------------
  Function: atuMemDerefFree
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Decrement the reference count of this memory object allocated by 
    the ATB memory manager. If the reference count falls to zero the
    object is deleted.
  Arguments:
    ptr - pointer to memory object
  Return:
    The number of outstanding references to this object
  -------------------------------------------------------------------*/
int _atuMemDerefFree( void *mem, char *where, FxU32 line ) {
    int uc = atuMemDeref( mem ) ;

    if ( debugFlags & ATU_MEM_CHECK )
        _atuMemCheck( where, line );

    if ( uc <= 0 )
        _atuMemFree( mem, where, line );

    return uc;
}
/*-------------------------------------------------------------------
  Function: atuMemCleanup
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( memory allocation )
  Description:
    Checks to see that all dynamically allocated memory has been
    freed.  If not, then generates a warning message and a list of 
    chunks that haven't been freed.  Also frees resources allocated
    for memory tracking.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
#ifdef AT_DEBUGGING
static void 
_checkAndFree( _AtuMemEntry *e ) {
    if ( !e ) return;
    if ( e->pointer ) 
        atuError( FXFALSE, 
                  "atuMemCleanup(): Left Over Memory Allocated"
                  " at %s(%d) size: %d\n",
                  e->where,
                  e->line,
                  e->size );
    _checkAndFree( e->next );
    if ( !(debugFlags & ATU_MEM_NO_FREE) )
        free( e );
}
#endif

void atuMemCleanup( void ) {
#ifdef AT_DEBUGGING
    _checkAndFree( head );
    head = tail = 0;
#endif
}


/*-------------------------------------------------------------------
  Function: atuMemDebug
  Date: 11/9/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description:
    Enable/disable memory debugging
  Arguments:
    flags - memory debugging state
  Return:
    void
  -------------------------------------------------------------------*/

void atuMemDebug(FxU32 flags) {
    debugFlags = flags;
}

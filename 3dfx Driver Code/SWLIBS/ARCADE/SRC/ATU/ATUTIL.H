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
** $Date: 10/11/00 7:34:51 PM$ 
**
*/

#ifndef _ATUTIL_H_
#   define _ATUTIL_H_

#ifdef AT_BUILD_LIBRARY
#       if ! (defined( AT_DEBUGGING ) ^ defined( AT_PRODUCTION ) )
#               error "Either AT_DEBUGGING or AT_PRODUCTION must be defined but not both."
#       endif
#endif

/*
**------------------------------------------------------------------------
** TYPES
**------------------------------------------------------------------------
*/

#include <stdio.h>
#include <3dfx.h>
#include <stdlib.h>

/*
**------------------------------------------------------------------------
** USEFUL MACROS
**------------------------------------------------------------------------
*/

/* so we don't have to worry about which OS we are on - djh */
#if defined(__WIN32__) || defined(__DOS32__)
#define ATU_FILE_SEPARATOR_STRING "\\"
#define ATU_CURRENT_DIRECTORY_STRING ".\\"
#elif macintosh
#define ATU_FILE_SEPARATOR_STRING ":"
#define ATU_CURRENT_DIRECTORY_STRING ":"
#endif

#if defined(__WIN32__) || defined(__DOS32__)
#define ATU_CDECL _cdecl 
#elif macintosh
#define ATU_CDECL
#endif

/* Make C Decls Palatable to C++ Compilers  */
#ifdef __cplusplus
#   define BEGINFDECL() extern "C" {
#   define ENDFDECL()   }
#else
#   define BEGINFDECL()
#   define ENDFDECL()  
#endif

/* Force Compiler to Treat A Float As A Long w/o Conversion */
#define FLOAT_BITS(X) (*(long*)&(X))

BEGINFDECL()

/*
**------------------------------------------------------------------------
** UNIFORM ERROR REPORTING ( uerror.c )
**------------------------------------------------------------------------
*/

void _atuStdErrorCallback( FxBool fatal, const char *format, ... );
typedef void (*AtuErrorCallback)( FxBool fatal, const char *format, ... );
extern AtuErrorCallback atuError;

void atuErrorSetCallback( AtuErrorCallback c  );

/*
**------------------------------------------------------------------------
** MEMORY MANAGEMENT ( umem.c )
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: _AtuMemEntry
  Date: 3/15/96
  Implementor(s): jdt
  Library: AT Utility Library ( memory management )
  Description: 
    Linked structure that keeps track of allocated memory.
  Members:
    size    - size in bytes of memory allocated
    type    - type of memory object
    use_count - number of references to this object
    pointer - pointer returned to application
    actual_pointer - actual heap pointer returned by the
                     os, it may be different than pointer
                     for aligned mallocs
    where   - pointer to string with source file
    line    - line number in source file where allocated
    next    - points to next entry in list ( NULL at end )
    gap     - buffer area before and after allocated space to
              check for data corruption
  -------------------------------------------------------------------*/

#define AT_GAP_SIZE    4

#define AT_MEM_GAP  ((FxU8)0xFD)
#define AT_MEM_FREE ((FxU8)0xDD)

typedef struct _AME {
    FxU32  size;
    FxU16  type;
    FxU16  use_count;
    void   *pointer;
    void   *actual_pointer;
    char   *where;
    FxU32  line;
    struct _AME *next;
    FxU8   gap[AT_GAP_SIZE];
} _AtuMemEntry;

/*-------------------------------------------------------------------
  Structure: AtuMemStats
  Date: 3/15/96
  Implementor(s): jdt
  Library: AT Utility Library ( memory managment )
  Description: 
    Vehicle for returning memory allocation statistics to the user.
  Members:
    peakMemAllocated - peak amount of memory allocated over life of
        application, in bytes.
    currentMemAllocated - amount of memory currently allocated using
        atu memory allocators
  -------------------------------------------------------------------*/
typedef struct {
    FxU32 peakMemAllocated;
    FxU32 currentMemAllocated;
} AtuMemStats;

/*-------------------------------------------------------------------
  Function: atuMemEntry
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Return the _AtuMemEntry associated with a memory object 
    allocated by the ATB memory manager. This is an internal
    function and in general should not be accessed directly.
    Use the higher level functions to access this data structure.
  Arguments:
    ptr - pointer to memory object
  Return:
    pointer to _AtuMemEntry associated with this object
  -------------------------------------------------------------------*/
#define atuMemEntry( ptr ) (((_AtuMemEntry *)(ptr))-1)

/*-------------------------------------------------------------------
  Function: atuMemGetSize
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Return the size in bytes of this memory object 
    allocated by the ATB memory manager. 
  Arguments:
    ptr - pointer to memory object
  Return:
    size in bytes of this object
  -------------------------------------------------------------------*/
#define atuMemGetSize( ptr ) ( atuMemEntry( ptr )->size )

/*-------------------------------------------------------------------
  Function: atuMemType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Set the type of this memory object allocated by 
    the ATB memory manager. 
  Arguments:
    ptr - pointer to memory object
    type - type of the memory object
  Return:
    type of object
  -------------------------------------------------------------------*/
#define atuMemType( ptr, t ) ( atuMemEntry( ptr )->type = t )

/*-------------------------------------------------------------------
  Function: atuMemGetType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Return the type of this memory object allocated by 
    the ATB memory manager. 
  Arguments:
    ptr - pointer to memory object
  Return:
    type of this object
  -------------------------------------------------------------------*/
#define atuMemGetType( ptr ) ( atuMemEntry( ptr )->type )

/*-------------------------------------------------------------------
  Function: atuMemRef
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Increment the reference count of this memory object allocated by 
    the ATB memory manager. 
  Arguments:
    ptr - pointer to memory object
  Return:
    The number of references to this object
  -------------------------------------------------------------------*/
#define atuMemRef( ptr ) ( ++(atuMemEntry( ptr )->use_count) )

/*-------------------------------------------------------------------
  Function: atuMemGetRef
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Return the number of references to this memory object allocated by 
    the ATB memory manager. 
  Arguments:
    ptr - pointer to memory object
  Return:
    Number of references to this object
  -------------------------------------------------------------------*/
#define atuMemGetRef( ptr ) ( atuMemEntry( ptr )->use_count )

/*-------------------------------------------------------------------
  Function: atuMemDeref
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Decrement the reference count of this memory object allocated by 
    the ATB memory manager. Returns the remaining number of references
    to the object.
  Arguments:
    ptr - pointer to memory object
  Return:
    The number of outstanding references to this object
  -------------------------------------------------------------------*/
#define atuMemDeref( ptr ) ( --(atuMemEntry( ptr )->use_count) )

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
#define atuMemDerefFree( mem ) _atuMemDerefFree( (mem), __FILE__, __LINE__ )
/*-------------------------------------------------------------------
  Function: atuMemCalloc
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( memory managment )
  Description: 
    Just like CAPI calloc, except is does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes
  Arguments:
    size - size in bytes of an element
    num - number of elements to allocate
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/
#define atuMemCalloc( num, size ) _atuMemRealloc( NULL, (num)*(size), __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: atuMemAlignedCalloc
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Util ( memory managment )
  Description: 
    Allocateds an aligned chunk of memory so that the base of the
    memory falls on an even alignment boundary. NOTE!!! This memory
    MUST BE FREED THROUGH THE ATUMEMFREE API OR THE HEAP WILL BE
    FRAGGED.
  Arguments:
    alignment - n-byte aligment
    size - size in bytes of an element
    num - number of elements to allocate
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/
#define atuMemAlignedCalloc( num, size, alignment ) \
        _atuMemAlignedCalloc( (num), (size), \
        (alignment), __FILE__, __LINE__ )


/*-------------------------------------------------------------------
  Function: atuMemMalloc
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( memory managment )
  Description: 
    Just like CAPI malloc, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes
  Arguments:
    size - size in bytes of to be allocated
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/
#define atuMemMalloc( size ) _atuMemRealloc( NULL, (size), __FILE__, __LINE__ )

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
  Return:
    void pointer to new heap memory
  -------------------------------------------------------------------*/
#define atuMemRealloc( ptr, size ) _atuMemRealloc((ptr),(size), __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: atuMemFree
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Util ( memory managment )
  Description: 
    Just like CAPI free, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes
  Arguments:
    mem - pointer to heap memory to be freed
  Return:
    none
  -------------------------------------------------------------------*/
#define atuMemFree( mem ) _atuMemFree( (mem), __FILE__, __LINE__ )

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
#define atuMemCheck() _atuMemCheck( __FILE__, __LINE__ )

/*-------------------------------------------------------------------
  Function: atuStrDup
  Date: 1/15/97
  Implementor(s): mlwp
  Library: AT Util ( memory managment )
  Description: 
    Just like CAPI strdup, except it does error checking and
    keeps track of allocated memory for statistical/debugging
    purposes. 
  Arguments:
    s  - string to be duplicated
  Return:
    duplicated string
  -------------------------------------------------------------------*/
#define atuStrDup( s ) _atuStrDup((s), __FILE__, __LINE__ )

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

#define ATU_MEM_CHECK   1
#define ATU_MEM_NO_FREE 2

void atuMemDebug(FxU32 flags) ;

void *_atuMemRealloc(void *ptr, size_t size, char *where, FxU32 line );
char *_atuStrDup(char *s, char *where, FxU32 line );
FxBool _atuMemCheck(char *where, FxU32 line );
void *_atuMemAlignedCalloc( size_t num, size_t size, FxU32 alignment, char *where, FxU32 line );
void _atuMemFree( void *mem, char *where, FxU32 line );
int _atuMemDerefFree( void *mem, char *where, FxU32 line );
void atuMemStats( AtuMemStats *s );
void atuMemCleanup( void );

/*
**------------------------------------------------------------------------
** GROWABLE STACK ( ustack.c )
**------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
  Structure: AtuStack
  Date: 3/15/96
  Implementor(s): jdt
  Library: AT Utility Library ( memory management )
  Description:
    Growable universal stack.  
  Members:
    base - points to base of stack
    top - points to current top of stack
    max - points to current limit of stack
    elementSize - size of a stack element in bytes
    granularity - size by which to grow the stack at overflow ( in elems )
  -------------------------------------------------------------------*/
typedef struct {
    unsigned char *base;
    unsigned char *top;
    unsigned char *max;
    FxU32         elementSize;
    FxU32         granularity; 
} AtuStack;

AtuStack *atuStackAllocate( FxU32 num ); 
void     atuStackDeallocate( AtuStack *stack );
void     atuStackInit( AtuStack *stack, FxU32 element_size, FxU32 granularity );
void     atuStackPush( AtuStack *stack, void *data );
void     atuStackPop( AtuStack *stack, void *data );
void     atuStackReset( AtuStack *stack );
FxU32    atuStackSize( AtuStack *stack );

/*
**------------------------------------------------------------------------
** TIME STAMP COUNTER UTILS ( utsc.asm )
**------------------------------------------------------------------------
*/

extern void ATU_CDECL atuTscRead( FxU32 *low, FxU32 *high );

/*
**------------------------------------------------------------------------
** INI FILE HANDLING ( uini.c )
**------------------------------------------------------------------------
*/

typedef FxU32 AtuIniType;
#define ATU_INI_STRING 0x0
#define ATU_INI_INT    0x1
#define ATU_INI_FLOAT  0x2

void   atuIniPath( const char *path );
FxBool atuIniGet( const char *filename, const char *tag, 
                  AtuIniType type, void *data, FxU32 size );
FxBool atuIniSet( const char *filename, const char *tag, 
                  AtuIniType type, void *data );

/*
**------------------------------------------------------------------------
** DEMO IMAGE UTILITIES ( banner.c )
**------------------------------------------------------------------------
*/

void atuDrawBanner( int x, int y, int width, int height, unsigned short *data );
void atuDrawHelp( int width, int height, unsigned short *data );
void atuDrawLogo( int width, int height, unsigned short *data );

/*
**------------------------------------------------------------------------
** FILE SEARCH PATH UTILITIES ( ufile.c )
**------------------------------------------------------------------------
*/

void atuSetLoadPath(const char *load_path);
char * atuFileLocate(const char *filename, char *full_path);
char *atuGetLoadPath(void);
FxBool atuStringCompare(const char *s1, const char *s2);

typedef char * (* AtuLoadHook)(const char *filename, char *full_path);

AtuLoadHook atuSetLoadHook(AtuLoadHook hook);

/*
**------------------------------------------------------------------------
** Read/Write arrays of 8, 16, 32 bit values MSB first
**------------------------------------------------------------------------
*/

FxBool atuRead8 (void *d, FxU32 count, FILE *stream);
FxBool atuRead16 (void *d, FxU32 count, FILE *stream);
FxBool atuRead32 (void *d, FxU32 count, FILE *stream);

FxBool atuWrite8 (const void *d, FxU32 count, FILE *stream);
FxBool atuWrite16 (const void *d, FxU32 count, FILE *stream);
FxBool atuWrite32 (const void *d, FxU32 count, FILE *stream);

ENDFDECL()

#endif

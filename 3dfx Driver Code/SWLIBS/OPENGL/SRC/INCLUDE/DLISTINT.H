#ifndef __gl_dlistint_h
#define __gl_dlistint_h

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
** $Revision: 2$
** $Date: 10/11/00 8:04:54 PM$
*/
#include "dlist.h"

#define __GL_DLIST_MIN_ARRAY_BLOCK	16
#define __GL_DLIST_MAX_ARRAY_BLOCK	1024

/* Priority heap data structure */
typedef struct __GLdlistHeapRec {
    GLuint n;			/* number of elems in the heap */
    GLuint arr_size;		/* amount of space allocated in heap */
    __GLdlist **arr;		/* array... */
} __GLdlistHeap;

struct __GLdlistArrayRec {
    GLint refcount; 			/*# ctxs using this array: create with 1, delete at 0*/
    __GLdlistHeap heap;			/* priority queue */
    void *machdep;			/* pointer to machine dependent state */
};


/*************************************************************
 *			 Priority Heap Routines              *
 *************************************************************/

/*
** Initialize the heap information that's stored in dlistArray.
*/
__GLdlistArray *__glDlistNewArray(__GLcontext *gc);

/*
** Free the heap information that's stored in dlistArray.
*/
void __glDlistFreeArray(__GLcontext *gc, __GLdlistArray *array);


/*
** Initialize a heap.  Returns true on success, false on failure. 
*/
GLint __glDlistHeapInit(__GLdlistHeap *);

/*
** Free memory associated with a heap, but not the heap itself.
*/
GLvoid __glDlistHeapFree(__GLdlistHeap *);

/*
** Return the element with the lowest priority, or NULL if the heap is
** empty.
*/
__GLdlist *__glDlistHeapQueryMin(__GLdlistHeap *);

/* 
** Remove and return the element with the lowest priority 
** (our heap is kinda backwards in that things with the lowest priority
** go at the top) 
*/
__GLdlist *__glDlistHeapRemoveMin(__GLdlistHeap *);

/* 
** Add an element to the heap.  this may fail if we are unable to grow
** arr. 
*/
GLint __glDlistHeapInsert(__GLdlistHeap *, __GLdlist *);

/* 
** Remove an item from the heap.  this routine assumes that the item 
** is in the heap. 
*/
GLvoid __glDlistHeapRemove(__GLdlistHeap *, __GLdlist *);

/* 
** Update the heap after an item changes priority
*/
GLvoid __glDlistHeapPriorityChanged(__GLdlistHeap *, __GLdlist *);


#endif /* __gl_dlistint_h */

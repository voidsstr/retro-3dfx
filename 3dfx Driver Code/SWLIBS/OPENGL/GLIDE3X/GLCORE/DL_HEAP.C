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
** Display list init/destroy code.
**
** $Revision: 2$
** $Date: 10/11/00 7:55:04 PM$
*/
#ifdef NEW_LISTS

#define PPL_LIST
#include "dlistint.h"
#include "ppdlist.h"
#include <assert.h>
#include <math.h>

#ifdef NDEBUG
#define HEAP_SANITY(h)
#else
static void heap_sanity(__GLdlistHeap *);
#define HEAP_SANITY(h) heap_sanity(h)
#endif

static void upheap(__GLdlistHeap *, int);
static void downheap(__GLdlistHeap *, int);

/*
 * Note:  arr[0] of the heap is the dummy node.  The first real node is 
 * at index 1, the last real node is at index n-1
*/

static __GLpplist dummy;

GLint __glDlistHeapInit(__GLdlistHeap *h)
{
    memset((void *)h, 0, sizeof(__GLdlistHeap));

    /* set dummy priority to be so low that it will always remain the 
     * 0th element */
    memset(&dummy, 0, sizeof(__GLpplist));
    dummy.flags = SP_RESIDENT;
    dummy.epriority = -1;
    __glDlistHeapInsert(h, (__GLdlist *)&dummy);

    HEAP_SANITY(h);

    return 1;
}

/* XXXCFtoDB what version of malloc and free are we supposed to use here? */
GLvoid __glDlistHeapFree(__GLdlistHeap *h)
{
    HEAP_SANITY(h);

    if (h->arr) free(h->arr);
    h->arr = 0;
}

__GLdlist *__glDlistHeapQueryMin(__GLdlistHeap *h)
{
    if (h->n == 1) return 0;
    else return h->arr[1];
}

__GLdlist *__glDlistHeapRemoveMin(__GLdlistHeap *h)
{
    __GLdlist *dl;

    if (h->n == 1) return 0;
    dl = h->arr[1];
    __glDlistHeapRemove(h, dl);

    return dl;
}

GLint __glDlistHeapInsert(__GLdlistHeap *h, __GLdlist *dl)
{
    __GLpplist *pl = (__GLpplist *)dl;

    if (!h->arr_size) {
	h->arr = (__GLdlist **)malloc(32 * sizeof(__GLdlist *));
	h->arr_size = 32;
    } else if (h->n + 1 == h->arr_size) {
	h->arr = (__GLdlist **)realloc((void *)h->arr,
				       (h->arr_size + 32)*sizeof(__GLdlist *));
	if (h->arr == 0) return 0;
	h->arr_size += 32;
    }
    
    h->arr[h->n] = dl;
    pl->queue_pos = h->n;
    upheap(h, h->n++);

    HEAP_SANITY(h);

    return 1;
}

GLvoid __glDlistHeapRemove(__GLdlistHeap *h, __GLdlist *dl) 
{
    __GLpplist *pl = (__GLpplist *)dl;
    __GLpplist **arr = (__GLpplist **)h->arr;
    GLint pos = pl->queue_pos;

    assert(h);

    assert(pos > 0);
    assert(arr[pos] == pl);

    arr[pos] = arr[--h->n];
    arr[pos]->queue_pos = pos;
    pl->queue_pos = 0;
 
    if (arr[pos]->epriority > pl->epriority) {
	downheap(h, pos);
    } else {
	upheap(h, pos);
    }

    HEAP_SANITY(h);
}

GLvoid __glDlistHeapPriorityChanged(__GLdlistHeap *h, __GLdlist *dl)
{
    __GLpplist *pl = (__GLpplist *)dl;

    assert(h);
    assert(pl->queue_pos > 0);

    /* only one of these calls will be necessary, but it is easier to just
     * call them both rather than figure out which call that is */
    upheap(h, pl->queue_pos);
    downheap(h, pl->queue_pos);

    HEAP_SANITY(h);
}

GLvoid __glDlistPrintHeap(__GLdlistHeap *h)
{
    __GLpplist **arr = (__GLpplist **)h->arr;
    int i;
    for (i = 1; i < h->n; i++) {
	printf("%d:\t0x%x\t%f\n", i, arr[i], arr[i]->epriority);
    }
    printf("\n");
}

static void upheap(__GLdlistHeap *h, GLint k)
{
    __GLpplist **arr = (__GLpplist **)h->arr;
    __GLpplist *pl = arr[k];
    while (arr[k/2]->epriority > pl->epriority) {
	arr[k] = arr[k / 2];
	assert(arr[k]->queue_pos == k / 2);
	arr[k]->queue_pos = k;
	k /= 2;
    }

    arr[k] = pl;
    pl->queue_pos = k;
}

static void downheap(__GLdlistHeap *h, GLint k)
{
    __GLpplist **arr = (__GLpplist **)h->arr;
    __GLpplist *pl = arr[k];
    int j;
    
    assert(h->n);
    while (k <= (h->n-1) / 2) {
	j = 2*k;
	if (j+1 < h->n && arr[j]->epriority > arr[j+1]->epriority) j++;
	if (pl->epriority <= arr[j]->epriority) break;
	arr[k] = arr[j];
	assert(arr[j]->queue_pos == j);
	arr[k]->queue_pos = k;
	k = j;
    }
    arr[k] = pl;
    arr[k]->queue_pos = k;
}

#ifndef NDEBUG
static void heap_sanity(__GLdlistHeap *h)
{
    __GLpplist *pl;
    int i;
    assert(h);
    assert(h->n <= h->arr_size);
    /* n must be non-zero since we should always have a dummy node */
    assert(h->n);
    assert(h->arr);

    /* make sure that the dummy node is in position 0 and has a priority 
     * which is < 0 */
    assert(h->arr[0] == (__GLdlist *)&dummy);
    assert(((__GLpplist *)h->arr[0])->epriority < 0.0);
       

    /* check that the heap condition has not been violated and that everyone's
     * queue_pos is correct */
    for (i = 0; i < h->n; i++) {
	pl = (__GLpplist *)h->arr[i];
	assert(pl);
	assert(pl->queue_pos == i);
	if (i*2 < h->n) 
	    assert(((__GLpplist *)h->arr[i*2])->epriority >= pl->epriority);
	if (i*2 + 1 < h->n) 
	    assert(((__GLpplist *)h->arr[i*2 + 1])->epriority >= pl->epriority);
	
	/* things should be marked resident before they're in the heap... */
	assert(pl->flags & SP_RESIDENT);
    }
}
#endif

#endif

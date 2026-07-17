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
** Display list table management routines.
**
** $Revision: 2$
** $Date: 10/11/00 7:55:10 PM$
*/
#include "context.h"
#include <GL/gl.h>
#include <stdio.h>
#include "dlistint.h"
#include "namesint.h"
#include "global.h"
#include "g_imfncs.h"

/*
** The next three routines are used as callbacks by the 
** name space management code.
*/

/*
** Delete a specified display list.  This typically just means free it,
** but if it is refcounted we just decrement the ref count.
*/
void __glDisposeDlist(__GLcontext *gc, void *pData)
{
    __GLdlist *list;

    list = (__GLdlist *)pData;

    list->refcount--;
    assert(list->refcount >= 0); /* less than zero references? */
    if (list->refcount == 0) {
	/*
	** No one is using this list, we can nuke it.
	*/
	list->free(gc, list);
	return;
    }
}

/*
** These functions are used by the empty dlist of dlist array.
** empty dlist is used as a sentinel function.
*/

/* ARGSUSED */
void
__glEmptyFreeDlist(__GLcontext *gc, __GLdlist *dlist)
{
  /* 
  ** do nothing; empty dlist is a statically allocated structure and
  ** should not be freed.
  */
}

/* ARGSUSED */
void
__glEmptyExecuteDlist(__GLcontext *gc, __GLdlist *dlist)
{
  /* do nothing; empty dlist is a sentinel value */
}

/*
** Sets up a new names tree and returns a pointer to it.
*/
__GLdlistArray *__glDlistNewArray(__GLcontext *gc)
{
    __GLdlistArray *array;

    __GL_DLIST_SEMAPHORE_LOCK();

    array = (__GLdlistArray *) 
	    (*gc->dlist.malloc)(gc, sizeof(__GLdlistArray));
    if (array == NULL) {
	__glSetError(GL_OUT_OF_MEMORY);
	__GL_DLIST_SEMAPHORE_UNLOCK();
	return NULL;
    }
    array->refcount = 1;

    /* call machine-dependent function */
    /*XXXblythe should this be able to fail?*/
    if (gc->dlist.newArray)
	(*gc->dlist.newArray)(gc, array);

    __GL_DLIST_SEMAPHORE_UNLOCK();
    return array;
}

void __glDlistFreeArray(__GLcontext *gc, __GLdlistArray *array)
{
    __GL_DLIST_SEMAPHORE_LOCK();
    /* call machine-dependent function */
    if (gc->dlist.freeArray)
	(*gc->dlist.freeArray)(gc, array);

    (*gc->dlist.free)(gc, array);
    __GL_DLIST_SEMAPHORE_UNLOCK();
}

/*
** __glim entry points for list name management.
*/

GLboolean APIENTRY __glim_IsList(GLuint list)
{
    __GLdlistMachine *dlstate;
    __GLnamesArray *array;
    GLboolean exists;

    __GL_SETUP_NOT_IN_BEGIN2();
    __GL_API_GET();

    dlstate = &gc->dlist;
    array = dlstate->namesArray;

    /*
    ** Lock access to dlarray.
    */
    __GL_DLIST_SEMAPHORE_LOCK();
    exists = __glNamesIsName(gc, array, list);
    __GL_DLIST_SEMAPHORE_UNLOCK();

    return exists;
}


GLuint APIENTRY __glim_GenLists(GLsizei range)
{
    __GLdlistMachine *dlstate;
    __GLnamesArray *array;
    GLuint start;

    __GL_SETUP_NOT_IN_BEGIN2();
    __GL_API_BLAND();

    dlstate = &gc->dlist;
    array = dlstate->namesArray;

    if (range < 0) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }
    if (range == 0) {
	return 0;
    }

    __GL_DLIST_SEMAPHORE_LOCK();

    start = __glNamesGenRange(gc, array, range);

    __GL_DLIST_SEMAPHORE_UNLOCK();

    return start;

}

void APIENTRY __glim_ListBase(GLuint base)
{ 
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_BLAND();

    gc->state.list.listBase = base;
}

void APIENTRY __glim_DeleteLists(GLuint list, GLsizei range)
{
    __GLdlistMachine *dlstate;
    __GLnamesArray *array;

    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_BLAND();

    if (range < 0) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }
    if (range == 0) return;

    dlstate = &gc->dlist;
    array = dlstate->namesArray;

    __GL_DLIST_SEMAPHORE_LOCK();

    __glNamesDeleteRange(gc, array, list, range);

    __GL_DLIST_SEMAPHORE_UNLOCK();

}

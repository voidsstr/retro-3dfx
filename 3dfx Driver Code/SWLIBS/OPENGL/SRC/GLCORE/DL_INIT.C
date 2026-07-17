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
** $Date: 10/11/00 8:03:12 PM$
*/
#include <stdlib.h>
#include "context.h"
#include "dlistint.h"
#include "dlistopt.h"
#include "namesint.h"

/*
** Used to share display lists between two different contexts.
*/
void __glShareDlist(__GLcontext *dst, __GLcontext *src)
{
    /* First get rid of our private display list state */
    __glFreeDlistState(dst);

    /*
    ** There are two structures for dlist array info, because
    ** the namesArray info is dlist independent, and there is
    ** some dlist-dependent memory management information left in
    ** the dlistArray structure.
    */
    dst->dlist.dlistArray = src->dlist.dlistArray;
    dst->dlist.dlistArray->refcount++;
    dst->dlist.namesArray = src->dlist.namesArray;
    dst->dlist.namesArray->refcount++;
}

void __glInitDlistState(__GLcontext *gc)
{
    __GLdlistMachine *dlist;

    dlist = &gc->dlist;

    dlist->nesting = 0;
    dlist->currentList = 0;

    /* By default use the regular memory routines for dlist memory too */
    dlist->malloc = gc->imports.malloc;
    dlist->realloc = gc->imports.realloc;
    dlist->free = gc->imports.free;

    /*
    ** To preserve some dlist memory usage housekeeping that was being 
    ** stored in the dlistArray structure, we need to still allocate
    ** and initialize the dlistArray.
    */
    if (dlist->dlistArray == NULL) {
	dlist->dlistArray = __glDlistNewArray(gc);
    }
    if (dlist->namesArray == NULL) {
	__GL_DLIST_SEMAPHORE_LOCK();
	dlist->namesArray = __glNamesNewArray(gc, __GL_NAMES_DLIST);
	__GL_DLIST_SEMAPHORE_UNLOCK();
    }
}

void __glFreeDlistState(__GLcontext *gc)
{
    gc->dlist.namesArray->refcount--;

    if (gc->dlist.namesArray->refcount == 0) {
	__GL_DLIST_SEMAPHORE_LOCK();
	__glNamesFreeArray(gc, gc->dlist.namesArray);
	__GL_DLIST_SEMAPHORE_UNLOCK();
    }
    gc->dlist.namesArray = NULL;

    /*
    ** The stuff in dlistArray takes care of memory management issues
    ** for display lists. 
    */
    gc->dlist.dlistArray->refcount--;

    if (gc->dlist.dlistArray->refcount == 0) {
	__glDlistFreeArray(gc, gc->dlist.dlistArray);
    }
    gc->dlist.dlistArray = NULL;
}

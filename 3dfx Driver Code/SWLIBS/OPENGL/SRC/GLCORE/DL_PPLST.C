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
** Basic display list routines.
**
*/

#ifdef NEW_LISTS
#define PPL_LIST
#include "dlistint.h"
#include "ppdlist.h" 
#include "dlistopt.h"
#include "global.h"
#include "imports.h"
#include "context.h"
#include "g_lcomp.h"
#include "g_listop.h"

void __glSPValidateDlist(__GLcontext *gc, __GLdlist *dlist);

/* ARGSUSED */
void
__glSPExecuteDlist(__GLcontext *gc, __GLdlist *dlist) {
  __GLpplist *pl = (__GLpplist *)dlist;
  extern void __glSPInterpret(GLubyte *PC);

  __glSPInterpret(pl->ilist);
}

extern void __glEmptyExecuteDlist(__GLcontext *gc, __GLdlist *dlist);

static GLvoid __glSPDlistFreeSegments(__GLcontext *gc, GLvoid *ptr);

/*XXXblythe this is repeated in so_list.c and needs to be added to an include */
/* 
** Format of Free Function structures in Local data.
** This is only used by the default implementation.
*/
typedef struct __GLDlistFreeFnsRec {
    void (*freeFn)(__GLcontext *gc, GLubyte *);
    GLubyte *data;
} __GLDlistFreeFns;

/*
 * Add a new free function callout onto this display list
 */
void
__glSPDlistAddFree(__GLcontext *gc, void (*freeFn)(__GLcontext *gc, GLubyte *), GLubyte *data) {
    __GLdlistMachine *dlist = &gc->dlist;
    __GLDlistFreeFns *freeLst = (__GLDlistFreeFns *)dlist->spFreeLst;
    GLint freeCnt = dlist->spFreeCnt;
    freeLst = dlist->realloc(gc, freeLst, (freeCnt+1)*sizeof(__GLDlistFreeFns));
    if (!freeLst) return;
    freeLst[freeCnt].freeFn = freeFn;
    freeLst[freeCnt].data = data;
    dlist->spFreeLst = (void *)freeLst; /*XXXblythe*/
    dlist->spFreeCnt = freeCnt+1;
}

void
__glSPFreeDlist(__GLcontext *gc, __GLdlist *dlist) {
  __GLpplist *pl = (__GLpplist *)dlist;
  GLuint freeCount = pl->freecnt;
  __GLDlistFreeFns *freeRec = pl->flist;

  if(freeCount) { /* if non-zero, segment isn't empty */
    __glSPDlistFreeSegments(gc, pl->segment);
    freeCount--;
  }

  /* call any free functions */

  while(freeCount--) {
    (*freeRec->freeFn)(gc, freeRec->data);
    freeRec++;
  }
  (*gc->dlist.free)(gc, pl->flist); /*XXXblythe unify with pl structure */
  (*gc->dlist.free)(gc, pl->ilist); /*XXXblythe unify with pl structure */
  (*gc->dlist.free)(gc, pl->plist); /*XXXblythe unify with pl structure */
  (*gc->dlist.free)(gc, pl);
}

__GLdlist *__glSPDlistCompiler(__GLcontext*gc, __GLcompiledDlist *compDlist) {
    __GLpplist *pl;
    GLuint segsize = __gl_spdl->start != 0;
    GLint freeCount = gc->dlist.spFreeCnt;

    pl = (__GLpplist *) (*gc->dlist.malloc)(gc, sizeof *pl);
    if (pl == NULL) return NULL;
    pl->refcount = 1;
    pl->free = gc->dlist.machineListFree;
    /* XXXCFtoDB what is this doing here?  flags gets set again below... */
    pl->flags = gc->dlist.spFlags;
    if(segsize) {
	GLvoid __glsplc_AddSentinel(GLvoid);
	__glsplc_AddSentinel();
#ifdef DLPULL
	pl->execute = gc->dlist.machineListValidateExec;
#else
        pl->execute = __glSPExecuteDlist;
#endif
    } else /* an empty display list, do nothing when executed */
      pl->execute = __glEmptyExecuteDlist;

    /* the AddSentinel routine may set a NO_PULL flag, so set list 
     * flags after calling that routine */
    pl->flags = gc->dlist.spFlags;

    pl->segment = (GLubyte *) compDlist->dlist;
    if(segsize) /* count non-empty segment as 1 free function */
	freeCount++;
    /* !segsize => freeCount == 0 */
    assert(segsize || freeCount == 0);
    pl->freecnt = freeCount;
    pl->flist = gc->dlist.spFreeLst;
    /*
    if (freeCount > 1)
	gc->dlist.free(gc, gc->dlist.spFreeLst);
    */
    pl->ilist = gc->dlist.spIlist;

    pl->pcnt = gc->dlist.spIcnt;
    pl->plist = 0;

    /* priority = medium.  priorities should have vaguely the same
     * meaning on all architectures, so this should be legal. */
    pl->priority = pl->epriority = .5;

    /* invoke any machine-dependent post compilation processing */
    if (gc->dlist.spCompMachdep)
	(*gc->dlist.spCompMachdep)(gc, (__GLdlist *)pl);

    return (__GLdlist *)pl;
}

typedef struct ppinterp {
    void *fcn;
    void *data;
} ppinterp_t;

typedef struct dlchunkh {	/* chunk head */
    GLvoid *next;	/* pointer to next chunk */
#if _MIPS_SZPTR == 32
    GLuint align;
#endif
} dlchunkh_t;

typedef struct dlpushblk {
    GLuint count;
} dlpushblk_t;

GLuint
__glSPDlistSpace(GLuint nopush) {
    dlchunkh_t *ptr = (dlchunkh_t *)__gl_spdl->start;
    GLuint amount;
    __GL_SETUP();

    /*
     * heavily overloaded use of nopush, its both a flag and a
     * space request for really large requests
     */
    if (nopush > 1) {
	amount = nopush;
	/* add in amount we need for bookkeeping */
	amount += sizeof(dlchunkh_t) >> 2;
	/* allocate more space */
#ifdef DLIST_DEBUG
	printf("In SPDlistSpace:  using malloc (nopush == %d)\n", nopush);
	fflush(stdout);
#endif
	__gl_spdl->start = (*gc->imports.malloc)(gc, amount*sizeof(GLuint));
    } else {
#ifdef DLPULL
	/* call machine-dependent "more space" routine */
	__gl_spdl->start = (*gc->dlist.spBalloc)(gc, &amount);
	/* here we try to make enough room in the locked memory to fit 
	 * the list */
	while (!__gl_spdl->start) {
	    /* note endless-loop avoidance if things just won't fit */
	    if (!(*gc->dlist.machineMoreBlocks)(gc)) break;
	    __gl_spdl->start = (*gc->dlist.spBalloc)(gc, &amount);
	}
#else
	    amount = __GLCHUNKSIZE;
	    __gl_spdl->start = 
		(*gc->imports.malloc)(gc, __GLCHUNKSIZE*sizeof(GLuint));
#endif
	if (!__gl_spdl->start) {
#ifdef DLIST_DEBUG
	    printf("In SPDlistSpace:  using malloc (DlistAllocBlock failed\n");
	    fflush(stdout);
#endif
	    /* XXXCF this could be a major problem since it will create
	     * lists that are sort of half-resident */
	    __gl_spdl->start = 
		(*gc->imports.malloc)(gc, __GLCHUNKSIZE*sizeof(GLuint));
	}
    }

    /* XXXCF do something intelligent here (like set an out-of-memory error) */
    assert(__gl_spdl->start);

    if (ptr) {
	ptr->next = __gl_spdl->start;

	if (!nopush && __gl_spdl->pcnt) {
	    /* seal the push block */
	    GLubyte *p = __gl_spdl->ptr;
	    dlpushblk_t *pb = (dlpushblk_t *)(p - (__gl_spdl->pcnt*sizeof(GLuint) + sizeof(dlpushblk_t)));
	    /* XXXblythe optimize for alignment */
	    pb->count = __gl_spdl->pcnt;
	    /*XXX add ptr to pb & interp function to ILIST here */
	    ADD_ILIST(gc->dlist.spPushExec, pb);
	}
    } else {
	/* on first allocation, need to set a pointer in the header block */
	gc->dlist.listData.dlist = __gl_spdl->start;
    }

    /* initialize link to 0 */
    ptr = (dlchunkh_t *)__gl_spdl->start;
    ptr->next = 0;

    __gl_spdl->pcnt = 0;
    if (nopush) {
	/* 1 word for next  */
	__gl_spdl->ptr = (GLubyte *)__gl_spdl->start + sizeof(dlchunkh_t); /* skip first word */
	return __gl_spdl->size = amount - (sizeof(dlchunkh_t) >> 2);
    } else {
	/* initialize for push model */
	/* 1 word for next, 1 word for push preamble */
	__gl_spdl->ptr = (GLubyte *)__gl_spdl->start +  /* skip first 2 words */
			sizeof(dlchunkh_t)+sizeof(dlpushblk_t);
	return __gl_spdl->size = amount - ((sizeof(dlchunkh_t)+sizeof(dlpushblk_t)) >> 2);
    }
}

void
__glsplc_AddIlist(void *fcn, void *data) {
    ppinterp_t *p;
    int i;
    __GL_SETUP();
    
    p = (ppinterp_t *)(*gc->imports.realloc)(gc, gc->dlist.spIlist, sizeof(*p)*((i = gc->dlist.spIcnt)+1));
    p[i].fcn = fcn;
    p[i].data = data;
    gc->dlist.spIlist = p;
    gc->dlist.spIcnt = i+1;
}

static GLvoid
__glSPDlistFreeSegments(__GLcontext *gc, GLvoid *ptr) {
    dlchunkh_t *tmp;

    do {
	tmp = ((dlchunkh_t *)ptr)->next;
#ifdef DLPULL
	(*gc->dlist.spBfree)(gc, ptr);
#else
	(*gc->dlist.free)(gc, ptr); 
#endif
	ptr = tmp;
    } while(ptr);
}

#ifdef DLPULL
typedef struct {
    void *addr;
    int  len;
} plist_t;

typedef struct {
    void *fcn, *data;
} ilist_t;

/* 
 * create the plist
 */
void
__glSPCreatePlist(__GLcontext *gc, __GLdlist *dlist) {
    __GLpplist *pl = (__GLpplist *)dlist;    
    int i = 0;
    plist_t *plist;
    ilist_t *ilist = pl->ilist;
    assert(!pl->plist);
    plist = (*gc->imports.malloc)(gc, sizeof(plist_t)*(pl->pcnt-1));

#ifndef NDEBUG
    memset((void *)plist, 0xab, sizeof(plist_t) * (pl->pcnt - 1));
#endif

    do {
	plist[i].len = *(int *)ilist[i].data;
	plist[i].addr = (GLubyte *)ilist[i].data+sizeof(GLint);
    } while(++i < pl->pcnt-1);
    pl->plist = plist;
    pl->pcnt = i;
}

/*
 * generic validation routine -- see also machine-specific validation
 * routines 
 */
void
__glSPValidateDlist(__GLcontext *gc, __GLdlist *dlist) {
    __GLpplist *pl = (__GLpplist *)dlist;
    
    /* this is the generic routine -- assume that the list is 
     * non-pullable */
    pl->execute = __glSPExecuteDlist;
    (*pl->execute)(gc, dlist);
}
#endif /*DLPULL*/
#endif /* NEW_LISTS */


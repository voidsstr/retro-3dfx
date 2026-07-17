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
#include "dlistint.h"
#include "dlistopt.h"
#include "global.h"
#include "imports.h"
#include "context.h"
#include "g_lcomp.h"
#include "g_listop.h"

#include "spdlist.h"

void
__glSPExecuteDlist(__GLcontext *gc, __GLdlist *dlist) {
  extern void __glDoInterpret(GLubyte *PC);
  __glDoInterpret(dlist->segment+sizeof(GLdouble));
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
  GLuint freeCount;
  __GLDlistFreeFns *freeRec;

  freeCount = *(GLuint *)dlist->local;

  if(freeCount) { /* if non-zero, segment isn't empty */
    __glSPDlistFreeSegments(gc, dlist->segment);
    freeCount--;
  }

  if(freeCount)
    freeRec = (__GLDlistFreeFns *)((GLuint *)dlist->local + 1);

  /* call any free functions */

  while(freeCount--) {
    (*freeRec->freeFn)(gc, freeRec->data);
    freeRec++;
  }
  (*gc->dlist.free)(gc, dlist);
}

__GLdlist *__glSPDlistCompiler(__GLcontext*gc, __GLcompiledDlist *compDlist) {
    __GLdlist *dlist;
    __GLdlist temp;
    size_t memsize;
    GLint i;
    GLuint segsize = __gl_spdl->start != 0;
    GLint freeCount = gc->dlist.spFreeCnt;
    __GLDlistFreeFns *freeList, *ff;

    /*
    ** This is pretty ugly.  It does make this code portable, though.
    */
    memsize = (size_t) (((GLubyte *) temp.local) - ((GLubyte *) &temp))
	+ sizeof(GLuint) + gc->dlist.spFreeCnt * sizeof(__GLDlistFreeFns);
    /*XXXblythe almost guaranteed that freeList isn't double word aligned */

    dlist = (__GLdlist *) (*gc->dlist.malloc)(gc, memsize);
    if (dlist == NULL) return NULL;
    dlist->refcount = 1;
    dlist->free = __glSPFreeDlist; /*__glGenericFreeDlist;*/
    if(segsize) {
      GLvoid __glsplc_AddSentinel(GLvoid);
      __glsplc_AddSentinel();
      dlist->execute = __glSPExecuteDlist;
    } else /* an empty display list, do nothing when executed */
      dlist->execute = __glEmptyExecuteDlist;

    dlist->segment = (GLubyte *) compDlist->dlist; /*(*gc->dlist.malloc)(gc, segsize);*/
    if(segsize) /* count non-empty segment as 1 free function */
	freeCount++;
    /* !segsize => freeCount == 0 */
    assert(segsize || freeCount == 0);
    *(GLint *)dlist->local = freeCount;
    freeList = (__GLDlistFreeFns *)((GLuint *)dlist->local+1);
    ff = (__GLDlistFreeFns *)gc->dlist.spFreeLst;
    for(i = 0; i < freeCount-1; i++)
	freeList[i] = ff[i];
    if (freeCount > 1)
	gc->dlist.free(gc, gc->dlist.spFreeLst);
    return dlist;
}

typedef struct dlchunkh {	/* chunk head */
    GLvoid *next;	/* pointer to next chunk */
    GLint  align;	/* force doubleword alignment for data */
    GLuint data[1];	/* variable length block of data */
} dlchunkh_t;

typedef struct dlchunkt {	/* chunk tail */
    GLvoid *jmpf;	/* pointer block jumping function */
    GLvoid *next;	/* pointer to first function in next block */
} dlchunkt_t;

typedef struct dlpushblk {
    GLvoid *interp;	/* interpreter function */
    GLint count;	/* number of 32-bit words of data */
    GLuint data[1];	/* variable block of data */
} dlpushblk_t;

typedef struct dlchunkh_pushblk {
    GLvoid *next;	/* pointer to next chunk */
    GLint  align;	/* force doubleword alignment for data */
    GLvoid *interp;	/* interpreter function */
    GLint count;	/* number of 32-bit words of data */
    GLuint data[1];	/* variable length block of data */
} dlchunkh_pushblk_t;

/* fcn to jump to next chunk */
GLubyte *__glsple_Jmp(const GLvoid *PC) {
    dlchunkt_t *p = (dlchunkt_t *)PC;
    /* we already advanced the PC, so we return jmpf rather than next */
    return p->jmpf;
}


#define __GLCHUNKSIZE	1024

GLuint
__glSPDlistSpace(GLuint nopush) {
    dlchunkh_t *ptr = (dlchunkh_t *)__gl_spdl->start;
    GLuint amount = __GLCHUNKSIZE;
    __GL_SETUP();

    /*
     * heavily overloaded use of nopush, its both a flag and a
     * space request for really large requests
     */
    if (nopush > 1) {
	amount = nopush;
	/* add in amount we need for bookkeeping */
	amount += (sizeof(dlchunkh_t) + sizeof(dlchunkt_t) - sizeof(ptr->data)) >> 2;
    }
    /* allocate more space */
    __gl_spdl->start = (*gc->imports.malloc)(gc, amount*sizeof(GLuint));

    if (ptr) {
	dlchunkt_t *tail;

	ptr->next = __gl_spdl->start;

	if (nopush)
	    /* not in a push state, so ptr is right at end */
	    tail = (dlchunkt_t *)__gl_spdl->ptr;
	else if (!__gl_spdl->pcnt)
	    /* if haven't stored any pushmodel stuff, roll back */
	    tail = (dlchunkt_t *)((GLubyte *)__gl_spdl->ptr-(sizeof(dlpushblk_t)-sizeof ptr->data));
	else {
	    /* seal the push block */
	    GLubyte *p = __gl_spdl->ptr;
	    dlpushblk_t *pb = (dlpushblk_t *)(p - (__gl_spdl->pcnt*sizeof(GLuint) + sizeof(dlpushblk_t)-sizeof ptr->data));
#ifdef bit64
	    /* fix alignment */
	    tail = (dlchunkt_t *)(((long p)+4)&~4);
#else
	    tail = (dlchunkt_t *)p;
#endif
	    /* XXXblythe optimize for alignment */
	    pb->interp = gc->dlist.spPushExec;
	    pb->count = __gl_spdl->pcnt;
	}
	/* set bridge function */
	tail->jmpf = __glsple_Jmp;
	tail->next = &((dlchunkh_t *)__gl_spdl->start)->data;
    } else {
	/* on first allocation, need to set a pointer in the header block */
	gc->dlist.listData.dlist = __gl_spdl->start;
    }

    /* initialize link to 0 */
    ptr = (dlchunkh_t *)__gl_spdl->start;
    ptr->next = 0;

    __gl_spdl->pcnt = 0;
    if (nopush) {
	/* 1 word for cnt, 2 words for jmp */
	__gl_spdl->ptr = &((dlchunkh_t *)__gl_spdl->start)->data; /* skip first word */
	return __gl_spdl->size = __GLCHUNKSIZE -
		((sizeof(dlchunkh_t) + sizeof(dlchunkt_t) - sizeof(ptr->data))>>2);
    } else {
	/* initialize for push model */
	/* 1 word for cnt, 2 words for push preamble, 2 words for jmp */
	__gl_spdl->ptr = &((dlchunkh_pushblk_t *)__gl_spdl->start)->data; /* skip first 3 words */
	return __gl_spdl->size = __GLCHUNKSIZE -
		((sizeof(dlchunkh_pushblk_t) + sizeof(dlchunkt_t) - sizeof(ptr->data))>>2);
    }
}

static GLvoid
__glSPDlistFreeSegments(__GLcontext *gc, GLvoid *ptr) {
    dlchunkh_t *tmp;

    do {
	tmp = ((dlchunkh_t *)ptr)->next;
	(*gc->dlist.free)(gc, ptr);
	ptr = tmp;
    } while(ptr);
}
#endif /* NEW_LISTS */

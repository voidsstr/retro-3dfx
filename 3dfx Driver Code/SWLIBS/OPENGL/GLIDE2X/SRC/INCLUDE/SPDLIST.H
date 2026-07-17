#ifndef __spdlist_h__
#define __spdlist_h__

/*
** Copyright 1996,1997 Silicon Graphics, Inc.
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
** $Revision: 4$
** $Date: 10/11/00 7:52:31 PM$
*/
#ifdef NEW_LISTS
#include "os.h"

typedef struct {
    GLint size;
    GLvoid *start;
    GLvoid *ptr;
    GLint pcnt;
} __GLSPlist_t;

#define __gl_spdl	((__GLSPlist_t *)(&(PAGE_ONE->pull)))

#define SP_NO_PULL	0x1	/* list is not pullable */
#define SP_RESIDENT	0x2	/* list is resident.  this may be
				 * interpreted in a machine-dependent
				 * fashion.  may later be more than 1 bit */
#define SP_BEGIN	0x4	/* are we inside a begin? */

/*XXXblythe this union is wrong for 64bit implementations */
typedef union { int i; float f; unsigned u; /*void *v;*/ } memptr_t;

extern GLuint __glSPDlistSpace(GLuint);
extern GLvoid __glSPExecuteDlist(__GLcontext *, __GLdlist *);
extern GLvoid __glSPCreatePlist(__GLcontext *, __GLdlist *);
extern GLvoid __glSPFreeDlist(__GLcontext *, __GLdlist *);
extern GLvoid __glsplc_InvalidEnum(GLvoid);
extern GLvoid __glsplc_InvalidValue(GLvoid);

#define __GLCHUNKSIZE	1024

#define __GLCHECK_SPACE_PUSH(n) {  		\
	int cursize = __gl_spdl->size;		\
	memptr_t *ptr;				\
	if (cursize < n)			\
		cursize = __glSPDlistSpace(0);	\
	cursize -= n;				\
	ptr = (memptr_t *) __gl_spdl->ptr		

/*
 * use for really big space allocations, e.g. pixels, textures ...
 */
#define __GLCHECK_SPACE_BIG(n) {  		\
	int cursize = __gl_spdl->size;		\
	memptr_t *ptr;				\
	if (cursize < n)			\
		cursize = __glSPDlistSpace(n > __GLCHUNKSIZE ? n : 1);	\
	cursize -= n;				\
	ptr = (memptr_t *) __gl_spdl->ptr		

#define __GLCHECK_SPACE(n) {  			\
	int cursize = __gl_spdl->size;		\
	memptr_t *ptr;				\
	if (cursize < n)			\
		cursize = __glSPDlistSpace(1);	\
	cursize -= n;				\
	ptr = (memptr_t *) __gl_spdl->ptr		


#define __GLADD_f(k,x)	ptr[k].f = x

#define __GLADD_ul(k,x)	ptr[k].u = x

#define __GLADD_i(k,x)	ptr[k].i = x

#define __GLADD_d(k,x)	{ union {int i[2]; double d;} u; u.d = x; ptr[k].i = u.i[0]; ptr[k+1].i = u.i[1]; }

/*XXXblythe don't use this macro on 64-bit implementations */
#define __GLADD_v(k, x)	/*ptr[k].v = x*/ assert(0);

/* update pointers for non-push case */
#define __GLWRITEBACK(n) 			\
	__gl_spdl->size = cursize;		\
	__gl_spdl->ptr = (ptr+n); }

/* update pointers for push case */
#define __GLWRITEBACK_PUSH(n)			\
	__gl_spdl->size = cursize;		\
	__gl_spdl->ptr = (ptr+n);		\
	__gl_spdl->pcnt += n; }

/* terminate a push chunk if tokens have been added
 * otherwise delete the push_preamble
 */
#define __GLRESOLVE_PUSH() {			\
	memptr_t *ptr = (memptr_t *)__gl_spdl->ptr;	\
	if (!__gl_spdl->pcnt) {			\
	    ptr -= 2;				\
	    __gl_spdl->ptr = (GLvoid *)ptr;	\
	} else {				\
	    ptr[-__gl_spdl->pcnt-1].u = __gl_spdl->pcnt;\
	    ptr[-__gl_spdl->pcnt-2].v = gc->dlist.spPushExec;\
	}					\
	}

/* setup for a push-list, allocate slots for ptr & cnt */
#define __GLESTABLISH_PUSH()			\
	__GLCHECK_SPACE(2);			\
	__gl_spdl->pcnt = 0;			\
	__GLWRITEBACK(2)

/* XXXCF this is acutally a ppdlist structure, but it can live here until 
 * we get a header file for that sort of thing */
typedef struct __GLpplistRec {
    GLint refcount;		/* number of references; created with 1; delete when 0 */
    __GLDlistFreeFn free;	/* function called to free display list */
    __GLDlistExecFn execute;	/* called to execute this display list */
    GLubyte *segment;		/* points to first segment of display list */
    GLvoid *ilist;		/* list of functions to interpret list */
    GLvoid *plist;		/* pull-down data */
    GLint  pcnt;		/* number of entries in plist */
    GLuint flags;		/* bookkeeping flags */
    GLint  freecnt;		/* number of free func/data pairs */
    GLvoid *flist;		/* list of free funcs/data */
    GLfloat priority;		/* priority */
    GLint queue_pos;		/* position in priority queue.  XXXCF
				 * put in machine-dependent trailer? */
} __GLpplist;

#endif /*NEWLISTS*/
#endif /*__spdlist_h__*/

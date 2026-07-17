#ifndef __glnamesint_h
#define __glnamesint_h

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
** $Date: 10/11/00 7:52:24 PM$
*/


/************************************************************************/
/*
** Names Manager Interface
**
** This file contains the Name Space Management types and structures.
**
** The Name Space Management code is used to store and retreive named
** data structures.  The data being stored is referred to with void 
** pointers to allow for the storage of any type of structure.
**
** The Name Space is implemented as a 2-3 tree.  For a detailed
** description of its implementation, see lib/opengl/soft/so_names.c.
**
** __GLnamesArray is declared in types.h.
*/
/************************************************************************/



typedef enum __GLnamesArrayTypeEnum {
    __GL_NAMES_DLIST,
    __GL_NAMES_TEXOBJ
} __GLnamesArrayType;

/*
** The number of spare branches and leaves that we keep about in case
** we run out of memory.  At that point, we complete the current operation
** by using the extra leaves and branches, and we report an OUT_OF_MEMORY
** error when a new operation is requested (unless we can fill our extras
** again!)
**
** These constants were not chosen terribly carefully.  As best as I can
** figure, we only need one spare branch per level in the tree (so 16
** supports a tree with 65536 leaves).  And even then, the user would have
** to be extremely devious to actually force 16 new branches to appear in
** the tree at just the same moment that the system runs out of memory.
**
** The number of spare leaves required, I believe, is one.  Three is chosen
** to allow for some slop.
*/
#define __GL_DL_EXTRA_BRANCHES          16
#define __GL_DL_EXTRA_LEAVES            3

/*
** This is the structure that contains information that is needed
** for each instance of a names tree.  It needs to be public
** so the refcount can be managed.
*/

typedef struct __GLnamesArrayTypeInfoRec __GLnamesArrayTypeInfo;
typedef struct __GLnamesBranchRec __GLnamesBranch;
typedef struct __GLnamesLeafRec __GLnamesLeaf;

struct __GLnamesArrayRec {
    __GLnamesBranch *tree;      /* points to the top of the names tree */
    GLuint depth;               /* depth of tree */
    GLint refcount; /*# ctxs using this array: create with 1, delete at 0*/
    __GLnamesArrayTypeInfo *dataInfo;   /* ptr to data type info */
    GLuint nbranches, nleaves;  /* should basically always be at max */
    __GLnamesBranch *branches[__GL_DL_EXTRA_BRANCHES];
    __GLnamesLeaf *leaves[__GL_DL_EXTRA_LEAVES];
};



/*
** Allocate and initialize a new array structure.
*/
extern __GLnamesArray *__glNamesNewArray(__GLcontext *gc, 
					 __GLnamesArrayType type);

/*
** Free the array structure.
*/
extern void __glNamesFreeArray(__GLcontext *gc, __GLnamesArray *array);

/*
** Save a new display list in the array.  A return value of GL_FALSE
** indicates and OUT_OF_MEMORY error, indicating that the list was 
** not stored.
*/
extern GLboolean __glNamesNewData(__GLcontext *gc, __GLnamesArray *array,
				  GLuint name, void *data);

/*
** Find and lock the list specified with "listnum".  A return value of NULL
** indicates that there was no such list.  __glNamesUnlockList() needs to
** be called to unlock the list otherwise.
*/
extern void *__glNamesLockData(__GLcontext *gc, __GLnamesArray *array,
			       GLuint name);

/*
** Unlock a list locked with __glNamesLockList().  If this is not called, then
** any memory associated with the list will never be freed when the list
** is deleted.
*/
extern void __glNamesUnlockData(__GLcontext *gc, void *data);

/*
** Same as __glNamesLockList() except that a bunch of lists are locked and
** returned simultaneously.  Any listbase previously specified is used as 
** an offset to the entries in the array.
*/
extern void __glNamesLockDataList(__GLcontext *gc, __GLnamesArray *array,
				  GLsizei n, GLenum type, GLuint base,
			          const GLvoid *names, void *dataList[]);

/*
** Same as __glNamesUnlockList() except that the entire array of names
** is unlocked at once.
*/
extern void __glNamesUnlockDataList(__GLcontext *gc, GLsizei n,
				    void *dataList[]);

/*
** Generates a list of names.
*/
extern GLuint __glNamesGenRange(__GLcontext *gc, __GLnamesArray *array, 
				GLsizei range);

/*
** Returns GL_TRUE if name has been generated for this array.
*/
extern GLboolean __glNamesIsName(__GLcontext *gc, __GLnamesArray *array, 
				 GLuint name);

/*
** Deletes a range of names.
*/
extern void __glNamesDeleteRange(__GLcontext *gc, __GLnamesArray *array, 
				 GLuint name, GLsizei range);

/*
** Generates a list of (not necessarily contiguous) names.
*/
extern void __glNamesGenNames(__GLcontext *gc, __GLnamesArray *array, 
			      GLsizei n, GLuint* names);

/*
** Deletes a list of (not necessarily contiguous) names.
*/
extern void __glNamesDeleteNames(__GLcontext *gc, __GLnamesArray *array, 
				 GLsizei n, const GLuint* names);

#endif /* __glnamesint_h */

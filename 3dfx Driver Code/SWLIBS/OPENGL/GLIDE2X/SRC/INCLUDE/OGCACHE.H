#ifndef __gl_ogcache_h_
#define __gl_ogcache_h_

/*
** Copyright 1996,1997, Silicon Graphics, Inc.
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
** Online-generated code cache
*/

typedef struct __GLcodeCellRec {
    GLuint modeFlags;			/* current modeFlags for this cell */
    GLuint signature;			/* modes not covered by modeFlags */
    GLvoid *code;			/* code space for this cell */
    __GLnameSpace tr;			/* What the 'tr' object looks like
					   (only valid if this is 
					   rasterizer code) */
    struct __GLcodeCellRec *next;	/* next LRU cell in cache */
} __GLcodeCell;

typedef struct __GLcodeCacheRec {
    __GLcodeCell *codeInfo;		/* malloc'ed list of codeCells */
    __GLcodeCell *codeList;		/* MRU list of codeCells */
    GLvoid *codeSpace;			/* malloc'ed code space */
} __GLcodeCache;

void __glInitializeCodeCache(__GLcontext *gc);
void __glInvalidateCodeCache(__GLcontext *gc);
void __glReleaseCodeCache(__GLcontext *gc);
GLboolean __glAllocCodeSpace(__GLcodeCache *cache, GLuint modeFlags,
			     GLuint signature, __GLnameSpace **ppns,
			     GLvoid **code);

#endif /* __gl_ogcache_h_ */

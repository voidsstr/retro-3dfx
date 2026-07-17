/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Revision: 2$ 
** $Date: 10/11/00 8:03:02 PM$ 
**
*/
#ifndef __gl_sstcontext_h_
#define __gl_sstcontext_h_

#include "context.h"

#include "glide.h"
#include "sstglide.h"

/*
** Device dependent context state
*/
typedef struct __GLSSTcontextRec {
    /* This must be first in this structure */
    __GLcontext gc;

    GLint       displayBank;

    GLint       lockCnt;

    /* the system lock, and our current pid */
    GLint pid;

    GLuint triCmd;
    GLuint lineCmd;

    GLint swRenderLines;
    GLint swRenderPoints;
    GLint swRenderTri;

    GLuint prevTexOffset;
    GLuint prevDestOffset;
    GLuint prevTexStride;
    GLuint prevDestStride;
    GLuint prevZOffset;
    GLuint prevZStride;

    __GLglideState glide;
} __GLSSTcontext;

/*
** our own thread local structure
*/
typedef struct __GLSSTthreadAreaRec {
    GLint	sstIsMapped;
} __GLSSTthreadArea;

/*
** Primary dispatch table
*/
extern __GLdispatchState __glSSTImmedState;

/* 
** Function prototypes 
*/
/* sstcontext */
__GLcontext *__glSSTCreateContext(__GLimports *imports, __GLcontextModes *modes);

/* sstglide */
GLboolean __glSSTGlideCreateContext(__GLcontext *gc);
void __glSSTGlideDestroyContext(__GLcontext *gc);
void __glSSTGlideValidateHW(__GLcontext *gc);
int grGetInteger(int);

/* sstpick */
void __glSSTPickAllProcs(__GLcontext *gc);
void __glSSTPickBufferProcs(__GLcontext *gc);
int  __glSSTPickDepthProcs(__GLcontext *gc);
void __glSSTPickLineProcs(__GLcontext *gc);
void __glSSTPickPointProcs(__GLcontext *gc);
void __glSSTPickTextureProcs(__GLcontext *gc);
void __glSSTPickTriangleProcs(__GLcontext *gc);
void __glSSTValidate(__GLcontext *gc);

/* sstpoint */
void __glSSTRenderPoint(__GLcontext *gc, __GLvertex *v);
void __glSSTRenderWidePoint(__GLcontext *gc, __GLvertex *v);
void __glSSTRenderWideAAPoint(__GLcontext *gc, __GLvertex *v);

/* sstline.c */
void __glSSTRenderLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void __glSSTRenderWideLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void __glSSTRenderWideAALine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);

/* ssttri */
void  __glSSTRenderTriangle(__GLcontext *gc, __GLvertex *v0,
			    __GLvertex *v1, __GLvertex *v2);

#endif /* __gl_sstcontext_h_ */

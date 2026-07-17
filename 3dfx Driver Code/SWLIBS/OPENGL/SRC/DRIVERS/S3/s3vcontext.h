#ifndef __gl_s3vcontext_h_
#define __gl_s3vcontext_h_

/*
** Copyright 1991-1997 Silicon Graphics, Inc.
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
*/
#include "context.h"

#include "s3virge.h"

typedef struct __GLS3VSystemLockRec {
    GLint lock;
    GLint pid;
} __GLS3VSystemLock;

/*
** Device dependent context state
*/
typedef struct __GLS3VcontextRec {
    /* This must be first in this structure */
    __GLcontext gc;

    GLint       displayBank;

    GLint       lockCnt;

    /* the system lock, and our current pid */
    __GLS3VSystemLock *sLock;
    GLint pid;

    /* The following things are specific to the S3 ViRGE context */
    GLboolean hwLocked;
    GLuint regBase;
    __glS3VTriEngineRegisters *triEngine;
    __glS3VLineEngineRegisters *lineEngine;
    __glS3VMisc3DRegs *s3Regs;
    GLubyte *vidMemBase;

    __glS3VMisc3DRegs preserve;

    GLuint hwZFunc;
    GLuint hwTexFunc;
    GLuint hwTexStride;
    GLuint hwBlendFunc;
    GLuint hwCmdMask;
    __GLfloat rScale;
    __GLfloat gScale;
    __GLfloat bScale;
    __GLfloat aScale;
    __GLfloat zScale;
    __GLfloat xScale;
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
} __GLS3Vcontext;

/*
** our own thread local structure
*/
typedef struct __GLS3VthreadAreaRec {
    GLint	s3IsMapped;
} __GLS3VthreadArea;

/* s3vcontext */
__GLcontext *__glS3VCreateContext(__GLimports *imports, __GLcontextModes *modes);

/* s3vpick */
void __glS3VValidate(__GLcontext *gc);
void __glS3VPickAllProcs(__GLcontext *gc);
void __glS3VPickTextureProcs(__GLcontext *gc);
int __glS3VPickDepthProcs(__GLcontext *gc);
void __glS3VPickTriangleProcs(__GLcontext *gc);
void __glS3VPickLineProcs(__GLcontext *gc);

/* s3vtri */
void  __glS3VRenderTriangle(__GLcontext *gc, __GLvertex *v0,
			    __GLvertex *v1, __GLvertex *v2);
void __glS3VRenderFlatDepthTriangle(__GLcontext *gc, __GLvertex *v0,
				    __GLvertex *v1, __GLvertex *v2);
void __glS3VRenderFlatTriangle(__GLcontext *gc, __GLvertex *v0,
			       __GLvertex *v1, __GLvertex *v2);
void __glS3VRenderSmoothDepthTriangle(__GLcontext *gc, __GLvertex *v0,
				      __GLvertex *v1, __GLvertex *v2);
void __glS3VRenderSmoothTriangle(__GLcontext *gc, __GLvertex *v0,
				 __GLvertex *v1, __GLvertex *v2);

/* s3vtxtri.c */
void __glS3VTexFlatDepthTriangle(__GLcontext *gc, __GLvertex *v0,
				 __GLvertex *v1, __GLvertex *v2);
void __glS3VTexFlatTriangle(__GLcontext *gc, __GLvertex *v0,
			    __GLvertex *v1, __GLvertex *v2);
void __glS3VTexSmoothDepthTriangle(__GLcontext *gc, __GLvertex *v0,
				   __GLvertex *v1, __GLvertex *v2);
void __glS3VTextureSmoothTriangle(__GLcontext *gc, __GLvertex *v0,
				  __GLvertex *v1, __GLvertex *v2);
void __glS3VTexPerspectiveFlatDepthTriangle(__GLcontext *gc, __GLvertex *v0,
					    __GLvertex *v1, __GLvertex *v2);
void __glS3VTexPerspectiveSmoothDepthTriangle(__GLcontext *gc, __GLvertex *v0,
					      __GLvertex *v1, __GLvertex *v2);
void __glS3VTexPerspectiveFlatTriangle(__GLcontext *gc, __GLvertex *v0,
				       __GLvertex *v1, __GLvertex *v2);
void __glS3VTexPerspectiveSmoothTriangle(__GLcontext *gc, __GLvertex *v0,
					 __GLvertex *v1, __GLvertex *v2);

/* s3vtestri.c */
void __glS3VTexPerspTessellatedTriangle(__GLcontext *gc, __GLvertex *a, 
					__GLvertex *b, __GLvertex *c);


/* s3vline.c */
void __glS3VRenderFlatLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void __glS3VRenderSmoothLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void __glS3VRenderFlatDepthLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void __glS3VRenderSmoothDepthLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);


/* s3vrgb.c */
void __glS3InitRGB(__GLcolorBuffer *cfb, __GLcontext *gc);

/* s3vdepth.c */
void __glS3InitDepth(__GLdepthBuffer *dfb, __GLcontext *gc);

/* s3vslock.c */
GLboolean __glS3VInitializeSLock(void);
GLboolean __glS3VInitSLock(__GLS3Vcontext *hwcx);
GLboolean __glS3VsLock(__GLS3Vcontext *hwcx);
GLboolean __glS3VsUnlock(__GLS3Vcontext *hwcx);

#endif /* __gl_s3vcontext_h_ */

#ifndef __glvertexcache
#define __glvertexcache

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
** $Revision: 2$
** $Date: 10/11/00 7:57:38 PM$
*/
/************************************************************************
 *                                                                      *
 *                               GLiNT                                  *
 *                                                                      *
 *                                                                      *
 *                   Copyright (C) 1994  3Dlabs Ltd.                    *
 *                                                                      *
 *                                                                      *
 * This software and its associated documentation contains proprietary, *
 * confidential and trade secret information of 3Dlabs Ltd. and except  *
 * as provided by written agreement with 3Dlabs Ltd.                    *
 *                                                                      *
 * a) no part may be disclosed, distributed, reproduced, transmitted,   *
 *    transcribed, stored in a retieval system, adapted or translated   *
 *    in any form or by any means electronic, mechanical, magnetic,     *
 *    optical, chemical, manual or otherwise,                           *
 *                                                                      *
 *    and                                                               *
 *                                                                      *
 * b) the recipient is not entitled to discover through reverse         *
 *    engineering or reverse compiling or other such techniques or      *
 *    processes the trade secrets contained therein or in the           *
 *    documentation.                                                    *
 *                                                                      *
 ************************************************************************/

/* Size of the vertex cache. */
/* The value of the MAX_VERTEX_CACHE should be a multiple of both
** 3 and 4 - divisible by 12.  This is so that for independent
** quads and triangles, a whole number of them will fit into the 
** buffer thereby making the flushing operation much easier.
*/
#define MAX_VERTEX_CACHE                36  

typedef struct __GLvcacheRec {                   
    __GLvertex 	*vertexCache;		/* Vertex Cache buffer */
    int		vertexCount;		/* Count of the number of vertices */
    GLbitfield	vertexCacheState;	/* state of vertex cache */
    GLboolean	vertexCacheEnabled;	/* Global vertex caching enable flag */
    GLenum	cachedPrimitiveType; 	/* Type of primitive in the cache */
    int		vertexStart;		/* MatValidate starts here */
    int		vertexType;		/* Types of vertexes in the cache */
    void	(__fastcall *save)(__GLcontext*, __GLvertex*);
} __GLvcacheMachine;

/* Types of vertices in a polygon, these are bit-significant. */
#define	CACHE_VERTEX_TYPE_2D	0x01
#define	CACHE_VERTEX_TYPE_3D	0x02
#define	CACHE_VERTEX_TYPE_4D	0x04

/* Flush types */
#define	GL_END_VCACHE		(1<<0)
#define	GL_FLUSH_VCACHE		(1<<1)

/* VertexCacheState flags */
#define VC_IN_LINE_LOOP		(1<<0)  /* Continuing a line loop */
#define VC_IN_POLYGON		(1<<1)  /* Continuing a polygon */
#define VC_CAT_PRIMITIVE	(1<<2)  /* Primitives are being concatenated */
#define VC_MATERIAL_VALIDATE	(1<<3)  /* Some vertexes have been validated */

/* vc_cache */
extern void APIENTRY __glim_VertexCacheBegin(GLenum mode);
extern void APIENTRY __glim_VertexCacheEnd(void);
extern void APIENTRY __glim_VertexCache2fv(const GLfloat *);
extern void APIENTRY __glim_VertexCache3fv(const GLfloat *);
extern void APIENTRY __glim_VertexCache4fv(const GLfloat *);

extern void __glInitVertexCacheState(__GLcontext *gc);
extern void __glFreeVertexCacheState(__GLcontext *gc);
extern void __glProcessVertexCache(__GLcontext *gc, GLuint flushed);
extern void __glPickVcacheProcs (__GLcontext *gc);
extern void __glMatValidateVcache(__GLcontext *gc);

/* vc_line */
extern void __glProcessCachedLineStrip(__GLcontext *gc, GLuint flushed);
extern void __glProcessCachedLineLoop(__GLcontext *gc, GLuint flushed);
extern void __glProcessCachedLines(__GLcontext *gc, GLuint flushed);

/* vc_point */
extern void __glProcessCachedPoints(__GLcontext *gc, GLuint flushed);

/* vc_poly */
extern void __glProcessCachedTriangleStrip(__GLcontext *gc, GLuint flushed);
extern void __glProcessCachedQuadStrip(__GLcontext *gc, GLuint flushed);
extern void __glProcessCachedQuadStripLines(__GLcontext *gc, GLuint flushed);
extern void __glProcessCachedTriangleFan(__GLcontext *gc, GLuint flushed);
extern void __glProcessCachedPolygon(__GLcontext *gc, GLuint flushed);
extern void __glProcessCachedTriangles(__GLcontext *gc, GLuint flushed);
extern void __glProcessCachedQuads(__GLcontext *gc, GLuint flushed);

#endif /* __glvertexcache */

#ifndef __gl_varray_h_
#define __gl_varray_h_

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
** $Revision: 2$
** $Date: 10/11/00 7:57:37 PM$
*/

/* This defines the stuff in gc->vertArrayState.signature, which is a 
 * bitmask describing current vertex array enables.  ven_get.c has to see
 * these definitions */

/* The signature field is set up the following way:
 * (hi)
 * 4 bits of index type 	0xf0000000
 * 1 bit of edge existence	0x08000000
 * 3 bits of texture size	0x07000000
 * 4 bits of texture type	0x00f00000
 * 4 bits of color size		0x000f0000
 * 4 bits of color type		0x0000f000
 * 4 bits of normal type 	0x00000f00
 * 4 bits of vertex size	0x000000f0
 * 4 bits of vertex type	0x0000000f
 * (lo)
 *
 * Note that edge existence flag is cramped in with texture size (just because
 * we don't have room for everything to have 4 bits)
 * Types are mapped into numbers from by TypeFlag(GLenum)
 */

#define VERTARRAY_V_MASK		0x000000ff
#define VERTARRAY_V_INDEX		32
#define VERTARRAY_VTYPE_SHIFT		0
#define VERTARRAY_VTYPE_MASK		(0xf << VERTARRAY_VTYPE_SHIFT)
#define VERTARRAY_VSIZE_SHIFT		4
#define VERTARRAY_VSIZE_MASK		(0xf << VERTARRAY_VSIZE_SHIFT)

#define VERTARRAY_N_MASK		0x00000f00
#define VERTARRAY_N_INDEX		1
#define VERTARRAY_NTYPE_SHIFT		8
#define VERTARRAY_NTYPE_MASK		(0xf << VERTARRAY_NTYPE_SHIFT)

#define VERTARRAY_C_MASK		0x000ff000
#define VERTARRAY_C_INDEX		2
#define VERTARRAY_CTYPE_SHIFT		12
#define VERTARRAY_CTYPE_MASK		(0xf << VERTARRAY_CTYPE_SHIFT)
#define VERTARRAY_CSIZE_SHIFT		16
#define VERTARRAY_CSIZE_MASK		(0xf << VERTARRAY_CSIZE_SHIFT)

#define VERTARRAY_T_MASK		0x07f00000
#define VERTARRAY_T_INDEX		4
#define VERTARRAY_TTYPE_SHIFT		20
#define VERTARRAY_TTYPE_MASK		(0xf << VERTARRAY_TTYPE_SHIFT)
#define VERTARRAY_TSIZE_SHIFT		24
#define VERTARRAY_TSIZE_MASK		(0x7 << VERTARRAY_TSIZE_SHIFT)

#define VERTARRAY_E_MASK		0x08000000
#define VERTARRAY_E_INDEX		16

#define VERTARRAY_I_MASK		0xf0000000
#define VERTARRAY_I_INDEX		8
#define VERTARRAY_ITYPE_SHIFT		28
#define VERTARRAY_ITYPE_MASK		(0xf << VERTARRAY_ITYPE_SHIFT)

/* type flags for signatures.  we add one here to make sure that 
 * the part of the signature corresponding to something that is enabled
 * (and has been correctly specified) is non-zero.  Kona ucode uses
 * this fact, and also the connection to the GL_<whatever> values.  
 * also, all values must be representable as a 4-bit number. */
#define VERTARRAY_BYTE_FLAG               (GL_BYTE - GL_BYTE + 1)
#define VERTARRAY_UNSIGNED_BYTE_FLAG      (GL_UNSIGNED_BYTE - GL_BYTE + 1)
#define VERTARRAY_SHORT_FLAG              (GL_SHORT - GL_BYTE + 1)
#define VERTARRAY_UNSIGNED_SHORT_FLAG     (GL_UNSIGNED_SHORT - GL_BYTE + 1)
#define VERTARRAY_INT_FLAG                (GL_INT - GL_BYTE + 1)
#define VERTARRAY_UNSIGNED_INT_FLAG       (GL_UNSIGNED_INT - GL_BYTE + 1)
#define VERTARRAY_FLOAT_FLAG              (GL_FLOAT - GL_BYTE + 1)
#define VERTARRAY_DOUBLE_FLAG             (GL_DOUBLE_EXT - GL_BYTE + 1)

/* Vertex array control word flags. */
#define VERTARRAY_CW_ALLOW_PRECOMPILE   0x00000001      /* compilable array */
#define VERTARRAY_CW_NEEDS_COMPILE      0x00000002      /* compile needed */
#define VERTARRAY_CW_LOCKED             0x00000004      /* locked currently */
#define VERTARRAY_CW_VX_CULL            0x00000008      /* per vertex cull on */
#define VERTARRAY_CW_VX_CULL_LOCAL      0x00000010      /* local cull */
#define VERTARRAY_CW_SLOWPATH		0x00000020	/* fallback to vcache */
#define VERTARRAY_CW_NO_VERTEX		0x00000040	/* fallback to vcache */

#define VERTARRAY_CW_DISABLE_FAST       0x00000080      /* disable fast paths */
#define VERTARRAY_CW_DISABLE_RASTER     0x00000100      /* for timing */
#define VERTARRAY_CW_SAME_BUF           0x00002000	/* SameBuf enabled */
#define VERTARRAY_CW_SMALL_GLVERTEX     0x00004000	/* small vx struct */
#define VERTARRAY_CW_SKIP_VXCULL     	0x00008000	/* save branches */
#define VERTARRAY_CW_COD                0x00010000      /* compile on demand */

/****************************************************************************/
/*
 * Vertex array state.
 */
typedef struct {
	const char *vertex_pointer;
	GLint vp_size;
	GLenum vp_type;
	GLsizei vp_stride;
	GLsizei vp_usr_stride;
	GLsizei vp_count;
	void (APIENTRY *vp_call)(const char *);
	void (__fastcall *vp_copy)(__GLcontext *gc, const void *vp, __GLvertex *v);

	const char *normal_pointer;
	GLint np_size;
	GLenum np_type;
	GLsizei np_stride;
	GLsizei np_usr_stride;
	GLsizei np_count;
	void (APIENTRY *np_call)(const char *);
	void (__fastcall *np_copy)(__GLcontext *gc, const void *np, __GLvertex *v);

	const char *color_pointer;
	GLint cp_size;
	GLenum cp_type;
	GLsizei cp_stride;
	GLsizei cp_usr_stride;
	GLsizei cp_count;
	void (APIENTRY *cp_call)(const char *);
	void (__fastcall *cp_copy)(__GLcontext *gc, const void *cp, __GLvertex *v);

	const char *index_pointer;
	GLint ip_size;
	GLenum ip_type;
	GLsizei ip_stride;
	GLsizei ip_usr_stride;
	GLsizei ip_count;
	void (APIENTRY *ip_call)(const char *);
	void (__fastcall *ip_copy)(__GLcontext *gc, const void *ip, __GLvertex *v);

	const char *tex_coord_pointer;
	GLint tp_size;
	GLenum tp_type;
	GLsizei tp_stride;
	GLsizei tp_usr_stride;
	GLsizei tp_count;
	void (APIENTRY *tp_call)(const char *);
	void (__fastcall *tp_copy)(__GLcontext *gc, const void *tp, __GLvertex *v);

	const char *edge_pointer;
	GLint ep_size;
	GLenum ep_type;
	GLsizei ep_stride;
	GLsizei ep_usr_stride;
	GLsizei ep_count;
	void (APIENTRY *ep_call)(const char *);
	void (__fastcall *ep_copy)(__GLcontext *gc, const void *ep, __GLvertex *v);

	GLbitfield signature;
	GLbitfield mask;
	GLint index;

	const char *interleavedPointer;	/* pointer to interleaved array */
	GLenum 	interleavedFormat;	/* set by InterleavedArrays */
	GLuint 	controlWord;

	GLuint 	validateMask;		/* Bits for validate */

	GLint	start;			/* set by LockArrays */
	GLint	count;			/* set by LockArrays */

	GLint 	blockSize;		/* size of incremental buf allocs */
	GLint	numVertexes;		/* number of vxs currently supported */

	GLint varrayBufSize;		/* size in bytes of varrayBuf */
	__GLvertex  *varrayBuf;		/* internal vertex array */
	__GLvertex  *varrayPtr;		/* used when processing varrayBuf */

	GLint 	batchSize;		/* number of vertexes in batch */

	GLenum batchMode;
	GLint batchFirst;
	GLuint *batchElements;
	GLint batchIndex;

	GLuint cullCode[2];

	void (*drawVertexes[10])(__GLcontext *gc, GLint first, GLsizei count,
					GLuint *elements);

	void (__fastcall *copyVertex)(__GLcontext *gc, const void *vp, __GLvertex *v);
	void (__fastcall *copyNormal)(__GLcontext *gc, const void *vp, __GLvertex *v);
	void (__fastcall *copyColor)(__GLcontext *gc, const void *vp, __GLvertex *v);
	void (__fastcall *copyIndex)(__GLcontext *gc, const void *vp, __GLvertex *v);
	void (__fastcall *copyTexCoord)(__GLcontext *gc, const void *vp, __GLvertex *v);
	void (__fastcall *copyEdgeFlag)(__GLcontext *gc, const void *vp, __GLvertex *v);

	GLint compileIndex;
	GLbitfield compileMask;

	void (*compileElements)(__GLcontext *gc, GLint offset,
				GLint first, GLsizei count);
	void (*compileElementsIndexed)(__GLcontext *gc, GLint offset,
				GLint first, GLsizei count, GLuint *elements);
	void (*compileElementsNoCull)(__GLcontext *gc, GLint offset,
				GLint first, GLsizei count);
	void (*compileElementsSilhouette)(__GLcontext *gc, __GLvertex *v,
				GLint index);

	void (*xf)(__GLcoord *res, const __GLfloat *v, const __GLmatrix *m);
	GLuint (*clipCheck)(__GLcontext *gc, __GLvertex *vx);
	void (*validateVertex)(__GLcontext *gc, __GLvertex *v, GLuint needs);
	GLboolean continuation;		/* Are we in the rest of strip or fan? */

	/* OPT 0.1.4 GL_ARB_multitexture: client state for texture unit 1.
	** Kept at the end of the struct so no existing field offset moves. */
	GLuint clientTexUnit;		/* glClientActiveTextureARB selector (0/1) */
	const char *tex_coord_pointer1;	/* unit-1 glTexCoordPointer */
	GLint tp1_size;
	GLenum tp1_type;
	GLsizei tp1_stride;
	GLsizei tp1_usr_stride;
	GLboolean texCoord1Enabled;	/* unit-1 GL_TEXTURE_COORD_ARRAY enable */
	void (*compileElementsReal)(__GLcontext *gc, GLint offset,
				GLint first, GLsizei count);

} __GLVertArrayMachine;

extern void __glInitVertexArrayState(__GLcontext *gc);
extern GLboolean __glVertexArrayBackDoor(__GLcontext *gc, GLenum pname, GLfloat *params);

extern void __glCompileElements_NoCopy(__GLcontext *gc, GLint offset,
					GLint first, GLsizei count);
extern void __glCompileElementsSilhouette_NoCopy(__GLcontext *gc, __GLvertex *v,
					GLint index);

/* generic DrawArrays functions  */
extern void (*__gl_varray_funcs[64])(GLenum, GLint, GLsizei);

#endif /* __gl_varray_h_ */

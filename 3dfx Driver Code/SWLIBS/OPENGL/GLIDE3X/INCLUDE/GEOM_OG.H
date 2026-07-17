#ifndef __GEOM_OG_H
#define __GEOM_OG_H

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
** $Date: 10/11/00 7:57:19 PM$
*/

#define GEOM_OG_MAX_KEY_SIZE 24
#define GEOM_OG_NUM_SLOTS 50

struct task {
    GLsizei keysize;
    unsigned long key[GEOM_OG_MAX_KEY_SIZE];
    unsigned char space[4000];
    void* drawVertexes;
};

#if __GL_SST
/*
** Key used when no lighting, user clip plane, or vertex array is enabled.
*/
typedef struct {
    unsigned long mask;
    __GLtransform *modelView;
    GLenum matrixType;
} __GLgeomCoreKey;

/*
** Additional key to support lighting, user clip planes, and vertex arrays.
*/
typedef struct {
    unsigned long lightAndClip;
    __GLlightSourceMachine *lightSources;
    __GLvertex *varrayPtr;
} __GLgeomMoreKey;
#endif

typedef struct {
#if __GL_SST
    __GLgeomCoreKey ckey[GEOM_OG_NUM_SLOTS];
    __GLgeomMoreKey mkey[GEOM_OG_NUM_SLOTS];
#endif
    struct task slots[GEOM_OG_NUM_SLOTS], *last_hit;
    int last_hit_idx;
    int num_keys;
    int last_insert;
} __GLgeomOGData;


void* GenerateCompile(__GLcontext *gc, GLuint vp_size, GLuint flags);
void* GenerateVertexFunc(__GLcontext *gc);
void __glPickVertexShape(__GLcontext *gc);

#define __GL_GEOM_OG_NORMAL	0x0001
#define __GL_GEOM_OG_COLOR	0x0002
#define __GL_GEOM_OG_TEX	0x0004

#define __GL_GEOM_OG_NOCOPY	0x8000

#define __GL_GEOM_OG_NO_Z      	(0x0001 << 16)

#if __GL_SST

#define NEW_OG_KEY 1 /* this turns on the new key lookup code */

/* XXXshui obviously need to move to machdep code */
extern void* __glSSTGenerateCompile(__GLcontext *gc, GLuint vertex_size, GLuint flags);

#define __GL_GEOM_OG_KEY_FLAG_COLOR		0x00000001
#define __GL_GEOM_OG_KEY_FLAG_TEX		0x00000002
#define __GL_GEOM_OG_KEY_FLAG_NOCOPY		0x00000004
#define __GL_GEOM_OG_KEY_MVDIRTY		0x00000008
#define __GL_GEOM_OG_KEY_MVTYPE			0x00000030
#define __GL_GEOM_OG_KEY_VXSIZE			0x000000c0
#define __GL_GEOM_OG_KEY_HAS_WINDOW_TEXTURE	0x00000100
#define __GL_GEOM_OG_KEY_HAS_FOG		0x00000200
#define __GL_GEOM_OG_KEY_HAS_NORMAL		0x00000400
#define __GL_GEOM_OG_KEY_HAS_FRONT_COLOR	0x00000800
#define __GL_GEOM_OG_KEY_HAS_BACK_COLOR		0x00001000
#define __GL_GEOM_OG_KEY_HAS_TEXTURE		0x00002000
#define __GL_GEOM_OG_KEY_HAS_EYE		0x00004000
#define __GL_GEOM_OG_KEY_SHADE_BLEND		0x00008000
#define __GL_GEOM_OG_KEY_SHADE_ALPHA_TEST	0x00010000
#define __GL_GEOM_OG_KEY_SHADE_TWOSIDED		0x00020000
#define __GL_GEOM_OG_KEY_SHADE_DEPTH_TEST	0x00040000
#define __GL_GEOM_OG_KEY_SHADE_TEXTURE		0x00080000
#define __GL_GEOM_OG_KEY_SHADE_TEXTURE_PERSP	0x00100000
#define __GL_GEOM_OG_KEY_SHADE_SMOOTH		0x00200000
#define __GL_GEOM_OG_KEY_SHADE_CHEAP_FOG	0x00400000 /* should be unused */
#define __GL_GEOM_OG_KEY_SHADE_SMOOTH_LIGHT	0x00800000
#define __GL_GEOM_OG_KEY_FRONT_FILL		0x01000000
#define __GL_GEOM_OG_KEY_BACK_FILL		0x02000000
#define __GL_GEOM_OG_KEY_LIGHTING_ENABLE	0x04000000
#define __GL_GEOM_OG_KEY_FOG_ENABLE		0x08000000
#define __GL_GEOM_OG_KEY_NORMALIZE_ENABLE	0x10000000
#define __GL_GEOM_OG_KEY_CLIPPLANE_ENABLE	0x20000000
#define __GL_GEOM_OG_KEY_VERTARRAY_ENABLE	0x40000000

#define __GL_GEOM_OG_KEY_SLOW_LOOKUP_MASK \
	(__GL_GEOM_OG_KEY_LIGHTING_ENABLE |	\
	 __GL_GEOM_OG_KEY_CLIPPLANE_ENABLE |	\
	 __GL_GEOM_OG_KEY_VERTARRAY_ENABLE)
	 
#define __GL_GEOM_OG_KEY_SHADE_MASK \
	(__GL_GEOM_OG_KEY_SHADE_BLEND |		\
	 __GL_GEOM_OG_KEY_SHADE_ALPHA_TEST |	\
	 __GL_GEOM_OG_KEY_SHADE_TWOSIDED |	\
	 __GL_GEOM_OG_KEY_SHADE_DEPTH_TEST |	\
	 __GL_GEOM_OG_KEY_SHADE_TEXTURE |	\
	 __GL_GEOM_OG_KEY_SHADE_TEXTURE_PERSP |	\
	 __GL_GEOM_OG_KEY_SHADE_SMOOTH |	\
	 __GL_GEOM_OG_KEY_SHADE_CHEAP_FOG |	\
	 __GL_GEOM_OG_KEY_SHADE_SMOOTH_LIGHT)

#define __GL_GEOM_OG_KEY_HAS_MASK \
	(__GL_GEOM_OG_KEY_HAS_WINDOW_TEXTURE |	\
	 __GL_GEOM_OG_KEY_HAS_FOG |		\
	 __GL_GEOM_OG_KEY_HAS_NORMAL |		\
	 __GL_GEOM_OG_KEY_HAS_FRONT_COLOR |	\
	 __GL_GEOM_OG_KEY_HAS_BACK_COLOR |	\
	 __GL_GEOM_OG_KEY_HAS_TEXTURE |		\
	 __GL_GEOM_OG_KEY_HAS_EYE)

#define __GL_GEOM_OG_KEY_MVTYPE_SHIFT 4
#define __GL_GEOM_OG_KEY_VXSIZE_SHIFT 6

#define __GL_GEOM_OG_KEY_VXSIZE_2		0x00000000
#define __GL_GEOM_OG_KEY_VXSIZE_3		0x00000040
#define __GL_GEOM_OG_KEY_VXSIZE_4		0x00000080 /* there's room for one more, 0xc */

/* these are not used yet */
     
#define __GL_GEOM_OG_KEY_MVTYPE_IS2D		0x00000000
#define __GL_GEOM_OG_KEY_MVTYPE_IS2DNR		0x00000010
#define __GL_GEOM_OG_KEY_MVTYPE_IS2DNRSC	0x00000020 /* there's room for one more, 0x3 */

#endif
#endif

#ifndef __gl_vertex_h_
#define __gl_vertex_h_

/*
** Copyright 1991, Silicon Graphics, Inc.
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
** $Date: 10/11/00 7:57:39 PM$
*/
#include "types.h"

/*
** Vertex structure.  Each vertex contains enough state to properly
** render the active primitive.
*/
struct __GLvertexRec {
    /*
    ** Keep this data structure aligned: have all vectors start on 
    ** 4-word boundary, and sizeof this struct should be kept at 
    ** a multiple of 4 words. Also helps to bunch together most
    ** frequently used items, helps cache.
    */

    /*
    ** Coordinates straight from client. These fields may not be
    ** set depending on the active modes.  The normal and texture
    ** coordinate are used by lighting and texturing.  These cells
    ** may be overwritten by the eyeNormal and the generated texture
    ** coordinate, depending on the active modes.
    */
    __GLcoord obj;
    __GLcoord normal;

    /* 32 byte boundary */

    /*
    ** Projected eye coodinate.  This field is filled in when the users
    ** eye coordinate has been multiplied by the projection matrix.
    */
    __GLcoord clip;

#if __GL_SST_GLIDE_VTX
    __GLSSTvertex sst;
#endif

    /*
    ** This member holds 2 bitfields:
    **
    ** has: bits are set in this indicating which fields of the vertex are
    ** valid.
    **
    ** clipCode: clipping code mask.  One bit is set for each clipping plane that
    ** the vertex is out on.
    */
    GLuint hasAndClipCode;

    /*
    ** Below this line the fields are not set if clip code != 0.
    */

    /* moved up here to keep coords on cache line boundary */
    __GLcolor *color;

    /* 32 byte boundary */

    /*
    ** Window coordinate. This field is filled in when the eye coordinate
    ** is converted to a drawing surface relative "window" coordinate.
    ** NOTE: the window.w coordinate contains 1/clip.w.
    */
    __GLcoord window;
    __GLcoord eye;

    /* 32 byte boundary */

    /*
    ** Coordinates straight from client. These fields may not be
    ** set depending on the active modes.  The normal and texture
    ** coordinate are used by lighting and texturing.  These cells
    ** may be overwritten by the eyeNormal and the generated texture
    ** coordinate, depending on the active modes.
    */
    __GLcoord texture[__GL_MAX_TEX_UNITS];

    /*
    ** texture LOD*w value for the vertex.  Used when texture mapping
    */
    __GLfloat rhow;

    /*
    ** Edge tag.  When GL_TRUE this vertex and the next form a boundary
    ** edge on the primitive (polygon, tstrip, tfan, qstrip).
    ** Clipping uses this information when
    ** (gc->polygon.mode[face] != __GL_POLYGON_MODE_FILL)
    */
    GLboolean boundaryEdge;

    /*
    ** Fog value for the vertex.  This is only filled when doing cheap
    ** fogging.
    */
    __GLfloat fog;

    /* 32 byte boundary */

    /*
    ** Colors.  colors[0] is the "front" color, colors[1] is the "back" color.
    ** The color pointer points to which color is current for this
    ** vertex.  Verticies can have more than one color when two sided
    ** lighting is enabled. (note color pointer moved up top).
    */
    __GLcolor colors[2];
};

/* Indicies for colors[] array in vertex */
#define __GL_FRONTFACE          0
#define __GL_BACKFACE           1

/* Bits for clipCode (NOTE: MAX of 6 user clip planes) */
#define __GL_CLIP_SHIFT         16
#define __GL_CLIP_LEFT          (0x00000001 << __GL_CLIP_SHIFT)
#define __GL_CLIP_RIGHT         (0x00000002 << __GL_CLIP_SHIFT)
#define __GL_CLIP_BOTTOM        (0x00000004 << __GL_CLIP_SHIFT)
#define __GL_CLIP_TOP           (0x00000008 << __GL_CLIP_SHIFT)
#define __GL_CLIP_NEAR          (0x00000010 << __GL_CLIP_SHIFT)
#define __GL_CLIP_FAR           (0x00000020 << __GL_CLIP_SHIFT)
#define __GL_FRUSTUM_CLIP_MASK  (0x0000003f << __GL_CLIP_SHIFT)
#define __GL_CLIP_USER0         (0x00000040 << __GL_CLIP_SHIFT)
#define __GL_USER_CLIP_MASK     (0x00000fc0 << __GL_CLIP_SHIFT)
#define __GL_CLIP_MASK          (__GL_USER_CLIP_MASK | __GL_FRUSTUM_CLIP_MASK)

#define __GL_CLIP_VX_CULL       (0x00001000 << __GL_CLIP_SHIFT)
#define __GL_ALL_CLIP_MASK      (__GL_CLIP_MASK | __GL_CLIP_VX_CULL)

/* Bits for has */
#define __GL_HAS_FRONT_COLOR    0x0001
#define __GL_HAS_BACK_COLOR     0x0002
            /* for poly clipping */
#define __GL_HAS_BOTH           (__GL_HAS_FRONT_COLOR | __GL_HAS_BACK_COLOR)
#define __GL_HAS_TEXTURE        0x0004
#define __GL_HAS_NORMAL         0x0008
#define __GL_HAS_EYE            0x0010
#define __GL_HAS_CLIP           0x0020
#define __GL_HAS_FOG            0x0040
#define __GL_HAS_WINDOW         0x0080
#define __GL_HAS_LIGHTING       (__GL_HAS_EYE | __GL_HAS_NORMAL)

#define __GL_HAS_VX_CULL        0x0100  /* vertex has been culled */
#define __GL_HAS_WINDOW_TEXTURE 0x0200

                                        /* what was original dimension?
                                         * used to do validation.
                                         */
#define __GL_HAS_VERTEX_0D      0x0000  /* Don't bother validating */
#define __GL_HAS_VERTEX_2D      0x0400
#define __GL_HAS_VERTEX_3D      0x0800
#define __GL_HAS_VERTEX_4D      0x0c00
#define __GL_HAS_VERTEX_MASK    0x0c00

/*
** NOTE: may need to change the raster pos handler if more bits are added
** to the above constants
*/

/************************************************************************/

/*
** Total number of clipping planes supported by this gl.  This includes
** the frustum's six clipping planes.
*/
/*#define       __GL_TOTAL_CLIP_PLANES  (6 + __GL_NUMBER_OF_CLIP_PLANES)*/

/*
** Number of static verticies in the context.  Polygon's larger than
** this number will be decomposed.
*/
#define __GL_NVBUF 16

typedef void (**valarray)(__GLcontext *gc, __GLvertex *v, GLuint needs);

#define DO_VALIDATE(g,v,n) \
     (((valarray)(&gc->procs.validateVertex0))[(v->hasAndClipCode >> 10) & 3])(g,v,n)

/*
** State for managing the vertex machinery.
*/
typedef struct __GLvertexMachineRec {
    /*
    ** Vertex pointers. v0 always points to the next slot in vbuf to
    ** be filled in when a new vertex arrives.  v1, v2 and v3 are
    ** used by the per-primitive vertex handlers.
    */
    __GLvertex *v0;
    __GLvertex *v1;
    __GLvertex *v2;
    __GLvertex *v3;
    __GLvertex vbuf[__GL_NVBUF];

    /*
    ** Provoking vertex.  For flat shaded primitives the triangle
    ** renderer needs to know which vertex provoked the primitive to
    ** properly assign the color during scan conversion.  This is kept
    ** around as its a big pain to remember which vertex was provoking
    ** during clipping (and to keep its parameters right).
    */
    __GLvertex *provoking;

    /*
    ** needs is a bit field that keeps track of what kind of information
    ** is needed in the verticies.  See the vertex->has bits define for
    ** the definition of the bits used here.
    **
    ** frontNeeds is what the front faces need, and backNeeds is what
    ** the back faces need.
    */
    GLuint needs;

    /*
    ** frontNeeds and backNeeds are the needs
    */
    GLuint faceNeeds[2];

    /*
    ** materialNeeds is a bit field indicating what kind of information is
    ** needed in the vertices if the material is going to change.
    */
    GLuint materialNeeds;
} __GLvertexMachine;

/************************************************************************/

extern GLuint __glComputeUserClipCodes(__GLcontext *gc, __GLvertex *vx,
                                        GLuint clipPlanesMask);

GLuint __glClipCheckFrustum(__GLcontext *gc, __GLvertex *vx);
GLuint __glClipCheckAll(__GLcontext *gc, __GLvertex *vx);

GLuint __glClipCheck2D_2(__GLcontext *gc, __GLvertex *vx);
GLuint __glClipCheck2D_3(__GLcontext *gc, __GLvertex *vx);

void __glRasterPos2(__GLcontext *gc, const __GLfloat v[2]);
void __glRasterPos3(__GLcontext *gc, const __GLfloat v[3]);
void __glRasterPos4(__GLcontext *gc, const __GLfloat v[4]);

void __glEndPrim(__GLcontext *gc);

void __glValidateVertex2(__GLcontext *gc, __GLvertex *vx, GLuint needs);
void __glValidateVertex3(__GLcontext *gc, __GLvertex *vx, GLuint needs);
void __glValidateVertex4(__GLcontext *gc, __GLvertex *vx, GLuint needs);
void __glValidateVertex2Clip(__GLcontext *gc, __GLvertex *vx, GLuint needs);
void __glValidateVertex3Clip(__GLcontext *gc, __GLvertex *vx, GLuint needs);
void __glValidateVertex4Clip(__GLcontext *gc, __GLvertex *vx, GLuint needs);

void __glim_NoXFVertex2fv(const GLfloat v[2]);
void __glim_NoXFVertex3fv(const GLfloat v[3]);
void __glim_NoXFVertex4fv(const GLfloat v[4]);

#endif /* __gl_vertex_h_ */

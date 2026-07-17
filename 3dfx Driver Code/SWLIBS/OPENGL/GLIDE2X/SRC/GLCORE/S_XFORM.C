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
** Transformation procedures.
**
** $Revision: 4$
** $Date: 10/11/00 7:51:54 PM$
*/
#include "context.h"
#include "imports.h"
#include "global.h"
#include "g_imfncs.h"
#include "mips.h"
#include "histogram.h"

/*
** constants used by intel inline asm stuff
*/
#define M00 0
#define M01 M00 + 4
#define M02 M00 + 8
#define M03 M00 + 12
#define M10 16
#define M11 M10 + 4
#define M12 M10 + 8
#define M13 M10 + 12
#define M20 32
#define M21 M20 + 4
#define M22 M20 + 8
#define M23 M20 + 12
#define M30 48
#define M31 M30 + 4
#define M32 M30 + 8
#define M33 M30 + 12

#define RES_X 0
#define RES_Y 4
#define RES_Z 8
#define RES_W 12

#define V_X   0
#define V_Y   4
#define V_Z   8
#define V_W   12

/*
** Assuming that a->matrixType and b->matrixType are already correct,
** and dst = a * b, then compute dst's matrix type.
*/
void __glPickMatrixType(__GLcontext *gc, __GLmatrix *dst, __GLmatrix *a, __GLmatrix *b)
{
    switch(a->matrixType) {
      case __GL_MT_GENERAL:
        dst->matrixType = a->matrixType;
        break;
      case __GL_MT_W0001:
        if (b->matrixType == __GL_MT_GENERAL) {
            dst->matrixType = b->matrixType;
        } else {
            dst->matrixType = a->matrixType;
        }
        break;
      case __GL_MT_IS2D:
        if (b->matrixType < __GL_MT_IS2D) {
            dst->matrixType = b->matrixType;
        } else {
            dst->matrixType = a->matrixType;
        }
        break;
      case __GL_MT_IS2DNR:
        if (b->matrixType < __GL_MT_IS2DNR) {
            dst->matrixType = b->matrixType;
        } else {
            dst->matrixType = a->matrixType;
        }
        break;
      case __GL_MT_IDENTITY:
        if (b->matrixType == __GL_MT_IS2DNRSC) {
            dst->width = b->width;
            dst->height = b->height;
        }
        dst->matrixType = b->matrixType;
        break;
      case __GL_MT_IS2DNRSC:
        if (b->matrixType == __GL_MT_IDENTITY) {
            dst->matrixType = __GL_MT_IS2DNRSC;
            dst->width = a->width;
            dst->height = a->height;
        } else if (b->matrixType < __GL_MT_IS2DNR) {
            dst->matrixType = b->matrixType;
        } else {
            dst->matrixType = __GL_MT_IS2DNR;
        }
        break;
    }
    if (gc->state.transform.matrixMode == GL_TEXTURE) {
        if (dst->matrixType == __GL_MT_IDENTITY) {
            gc->slowPath &= ~(__GL_TEXTURE_MATRIX_SLOWPATH_0<<gc->texture.currentTexUnit);
        } else {
            gc->slowPath |= (__GL_TEXTURE_MATRIX_SLOWPATH_0<<gc->texture.currentTexUnit);
        }
    }
}

/*
** Muliply the first matrix by the second one keeping track of the matrix
** type of the newly combined matrix.
*/
void __glMultiplyMatrix(__GLcontext *gc, __GLmatrix *m, void *data)
{
    __GLmatrix *tm;

    tm = data;
    (*gc->procs.matrix.mult)(m, tm, m);
    __glPickMatrixType(gc, m, tm, m);
}

static void SetDepthRange(__GLcontext *gc, double zNear, double zFar)
{
    __GLviewport *vp = &gc->state.viewport;
    double scale, zero = __glZero, one = __glOne;

    /* Clamp depth range to legal values */
    if (zNear < zero) zNear = zero;
    if (zNear > one) zNear = one;
    if (zFar < zero) zFar = zero;
    if (zFar > one) zFar = one;
    vp->zNear = zNear;
    vp->zFar = zFar;

    if (gc->depthBuffer.invertZRange) {
        zNear = one - zNear;
        zFar = one - zFar;
    }

    /* Compute viewport values for the new depth range */
    scale = gc->depthBuffer.scale * __glHalf;
    gc->state.viewport.zScale = (zFar - zNear) * scale;
    gc->state.viewport.zCenter = (zFar + zNear) * scale;
}

void __glUpdateDepthRange(__GLcontext *gc)
{
    __GLviewport *vp = &gc->state.viewport;

    SetDepthRange(gc, vp->zNear, vp->zFar);
}

void __glEarlyInitTransformState(__GLcontext *gc)
{
    GLint numClipPlanes;

    /* Allocate memory for clip planes */
    numClipPlanes = gc->constants.numberOfClipPlanes;
    gc->state.transform.eyeClipPlanes = (__GLcoord *)
        (*gc->imports.calloc)(gc, (size_t) numClipPlanes, sizeof(__GLcoord));
}

void __glInitTransformState(__GLcontext *gc)
{
    GLint i, numClipPlanes;
    __GLtransform *tr;
    __GLvertex *vx;
    void (*pick)(__GLcontext*, __GLmatrix*);

    numClipPlanes = gc->constants.numberOfClipPlanes;

    /*
    ** Memory for clip planes allocated in __glEarlyInitTranformState
    ** in case a copy context occurs before MakeCurrent.
    */

    /* Allocate memory for matrix stacks */
    gc->transform.modelViewStack = (__GLtransform*)
        (*gc->imports.calloc)(gc, (size_t) gc->constants.maxModelViewStackDepth,
                              sizeof(__GLtransform));
    gc->transform.projectionStack = (__GLtransform*)
        (*gc->imports.calloc)(gc, (size_t) gc->constants.maxProjectionStackDepth,
                              sizeof(__GLtransform));
    gc->transform.textureStack[0] = (__GLtransform*)
        (*gc->imports.calloc)(gc, (size_t) gc->constants.maxTextureStackDepth,
                              sizeof(__GLtransform));
    gc->transform.textureStack[1] = (__GLtransform*)
        (*gc->imports.calloc)(gc, (size_t) gc->constants.maxTextureStackDepth,
                              sizeof(__GLtransform));

    /* 
    ** Allocate memory for clipping temporaries.
    ** Each plane can potentially add two temporary vertices, even though
    ** the overall number of vertices can only increase by one per plane.
    */
    gc->transform.clipTemp = (__GLvertex*)
        (*gc->imports.calloc)(gc, (size_t) 2*(6 + numClipPlanes),
                              sizeof(__GLvertex));

    gc->state.transform.matrixMode = GL_MODELVIEW;
    SetDepthRange(gc, __glZero, __glOne);

    gc->transform.modelView = tr = &gc->transform.modelViewStack[0];
    (*gc->procs.matrix.makeIdentity)(&tr->matrix);
    (*gc->procs.matrix.makeIdentity)(&tr->inverseTranspose);
    (*gc->procs.matrix.makeIdentity)(&tr->mvp);
    pick = gc->procs.pickMatrixProcs;
    (*pick)(gc, &tr->matrix);
    (*gc->procs.pickInvTransposeProcs)(gc, &tr->inverseTranspose);

    gc->transform.projection = tr = &gc->transform.projectionStack[0];
    (*gc->procs.matrix.makeIdentity)(&tr->matrix);
    (*pick)(gc, &tr->matrix);

    (*gc->procs.pickMvpMatrixProcs)(gc, &gc->transform.modelView->mvp);

    gc->state.transform.eyePos.z = __glOne;
    gc->state.transform.eyePosObj.z = __glOne;
    gc->state.transform.objEyeSpecified = GL_FALSE;
    (*gc->procs.pickCullVertexProcs)(gc, gc->transform.modelView);

    gc->transform.texture[0] = tr = &gc->transform.textureStack[0][0];
    (*gc->procs.matrix.makeIdentity)(&tr->matrix);
    (*pick)(gc, &tr->matrix);
    gc->transform.texture[1] = tr = &gc->transform.textureStack[1][0];
    (*gc->procs.matrix.makeIdentity)(&tr->matrix);
    (*pick)(gc, &tr->matrix);

    vx = &gc->transform.clipTemp[0];
    for (i = 0; i < 2*(6 + numClipPlanes); i++, vx++) {/*XXX*/
        vx->color = &vx->colors[__GL_FRONTFACE];
        vx->hasAndClipCode = (vx->hasAndClipCode & ~__GL_HAS_VERTEX_MASK) | __GL_HAS_VERTEX_4D;
    }

    gc->state.current.normal.z = __glOne;
}

/*
** An amazing thing has happened.  More than 2^32 changes to the projection
** matrix has occured.  Run through the modelView and projection stacks
** and reset the sequence numbers to force a revalidate on next usage.
*/
void __glInvalidateSequenceNumbers(__GLcontext *gc)
{
    __GLtransform *tr, *lasttr;
    GLuint s;

    /* Make all mvp matricies refer to sequence number zero */
    s = 0;
    tr = &gc->transform.modelViewStack[0];
    lasttr = tr + gc->constants.maxModelViewStackDepth;
    while (tr < lasttr) {
        tr->sequence = s;
        tr++;
    }

    /* Make all projection matricies sequence up starting at one */
    s = 1;
    tr = &gc->transform.projectionStack[0];
    lasttr = tr + gc->constants.maxProjectionStackDepth;
    while (tr < lasttr) {
        tr->sequence = s++;
        tr++;
    }
    gc->transform.projectionSequence = s;
}

/************************************************************************/

void APIENTRY __glim_MatrixMode(GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_BLAND();

    switch (mode) {
      case GL_MODELVIEW:
      case GL_PROJECTION:
      case GL_TEXTURE:
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    gc->state.transform.matrixMode = mode;
    //    __GL_DELAY_VALIDATE(gc);
    gc->procs.pickTransformProcs(gc);
}

void APIENTRY __glim_LoadIdentity(void)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();
    (*gc->procs.loadIdentity)(gc);
}

void APIENTRY __glim_LoadMatrixf(const GLfloat m[16])
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;
    __glDoLoadMatrix(gc, &m1);
}

void APIENTRY __glim_LoadMatrixd(const GLdouble m[16])
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;
    __glDoLoadMatrix(gc, &m1);
}

void APIENTRY __glim_MultMatrixf(const GLfloat m[16])
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;
    __glDoMultMatrix(gc, &m1, __glMultiplyMatrix);
}

void APIENTRY __glim_MultMatrixd(const GLdouble m[16])
{
    __GLmatrix m1;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    m1.matrix[0][0] = m[0];
    m1.matrix[0][1] = m[1];
    m1.matrix[0][2] = m[2];
    m1.matrix[0][3] = m[3];
    m1.matrix[1][0] = m[4];
    m1.matrix[1][1] = m[5];
    m1.matrix[1][2] = m[6];
    m1.matrix[1][3] = m[7];
    m1.matrix[2][0] = m[8];
    m1.matrix[2][1] = m[9];
    m1.matrix[2][2] = m[10];
    m1.matrix[2][3] = m[11];
    m1.matrix[3][0] = m[12];
    m1.matrix[3][1] = m[13];
    m1.matrix[3][2] = m[14];
    m1.matrix[3][3] = m[15];
    m1.matrixType = __GL_MT_GENERAL;
    __glDoMultMatrix(gc, &m1, __glMultiplyMatrix);
}

void APIENTRY __glim_Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    __glDoRotate(gc, angle, x, y, z);
}

void APIENTRY __glim_Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    __glDoRotate(gc, angle, x, y, z);
}

void APIENTRY __glim_Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    __glDoScale(gc, x, y, z);
}

void APIENTRY __glim_Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    __glDoScale(gc, x, y, z);
}

void APIENTRY __glim_Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    __glDoTranslate(gc, x, y, z);
}

void APIENTRY __glim_Translated(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    __glDoTranslate(gc, x, y, z);
}

void APIENTRY __glim_PushMatrix(void)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_BLAND();
    (*gc->procs.pushMatrix)(gc);
}

void APIENTRY __glim_PopMatrix(void)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();
    (*gc->procs.popMatrix)(gc);
}

void APIENTRY __glim_Frustum(GLdouble left, GLdouble right,
                    GLdouble bottom, GLdouble top,
                    GLdouble zNear, GLdouble zFar)
{
    __GLmatrix m;
    __GLfloat deltaX, deltaY, deltaZ;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    deltaX = right - left;
    deltaY = top - bottom;
    deltaZ = zFar - zNear;
    if ((zNear <= __glZero) || (zFar <= __glZero) || (deltaX == __glZero) || 
            (deltaY == __glZero) || (deltaZ == __glZero)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    (*gc->procs.matrix.makeIdentity)(&m);
    m.matrix[0][0] = zNear * __glTwo / deltaX;
    m.matrix[1][1] = zNear * __glTwo / deltaY;
    m.matrix[2][0] = (right + left) / deltaX;
    m.matrix[2][1] = (top + bottom) / deltaY;
    m.matrix[2][2] = -(zFar + zNear) / deltaZ;
    m.matrix[2][3] = __glMinusOne;
    m.matrix[3][2] = ((__GLfloat) -2.0) * zNear * zFar / deltaZ;
    m.matrix[3][3] = __glZero;
    m.matrixType = __GL_MT_GENERAL;
    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

void APIENTRY __glim_Ortho(GLdouble left, GLdouble right, GLdouble bottom, 
                  GLdouble top, GLdouble zNear, GLdouble zFar)
{
    __GLmatrix m;
    GLdouble deltax, deltay, deltaz;
    GLdouble zero;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    deltax = right - left;
    deltay = top - bottom;
    deltaz = zFar - zNear;
    if ((deltax == __glZero) || (deltay == __glZero) || (deltaz == __glZero)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    (*gc->procs.matrix.makeIdentity)(&m);
    m.matrix[0][0] = __glTwo / deltax;
    m.matrix[3][0] = -(right + left) / deltax;
    m.matrix[1][1] = __glTwo / deltay;
    m.matrix[3][1] = -(top + bottom) / deltay;
    m.matrix[2][2] = ((__GLfloat) -2.0) / deltaz;
    m.matrix[3][2] = -(zFar + zNear) / deltaz;

    /* 
    ** Screen coordinates matrix?
    */
    zero = 0.0;
    if (left == zero && 
            bottom == zero && 
            right == (GLdouble) gc->state.viewport.width &&
            top == (GLdouble) gc->state.viewport.height &&
            zNear <= zero && 
            zFar >= zero) {
        m.matrixType = __GL_MT_IS2DNRSC;
        m.width = gc->state.viewport.width;
        m.height = gc->state.viewport.height;
    } else {
        m.matrixType = __GL_MT_IS2DNR;
    }

    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

void __glUpdateViewport(__GLcontext *gc)
{
    __GLfloat ww, hh;

    /* Compute operational viewport values */
    ww = gc->state.viewport.width * __glHalf;
    hh = gc->state.viewport.height * __glHalf;
    gc->state.viewport.xScale = ww;
    /* XXXshui have to check if the sw path needs the __glHalf added here */
    gc->state.viewport.xCenter = gc->state.viewport.x + ww +
        gc->constants.fviewportXAdjust /* + __glHalf */;
    if (gc->constants.yInverted) {
        gc->state.viewport.yScale = -hh;
        gc->state.viewport.yCenter =
            (gc->constants.height - gc->constants.viewportEpsilon) -
            (gc->state.viewport.y + hh) +
            gc->constants.fviewportYAdjust /* + __glHalf*/;
    } else {
        gc->state.viewport.yScale = hh;
        gc->state.viewport.yCenter = gc->state.viewport.y + hh +
            gc->constants.fviewportYAdjust /* + __glHalf */;
    }
}

void __glUpdateViewportTransform(__GLcontext *gc)
{
    /* 
    ** Now that the implementation may have found us a new window size,
    ** we compute these offsets...
    */
    gc->transform.minx = gc->state.viewport.x + gc->constants.viewportXAdjust;
    gc->transform.maxx = gc->transform.minx + gc->state.viewport.width;
    gc->transform.fminx = gc->transform.minx + __glHalf;
    gc->transform.fmaxx = gc->transform.maxx + __glHalf;

    gc->transform.miny = (gc->constants.height - 
                          (gc->state.viewport.y + gc->state.viewport.height)) + 
                              gc->constants.viewportYAdjust;
    gc->transform.maxy = gc->transform.miny + gc->state.viewport.height;
    gc->transform.fminy = gc->transform.miny + __glHalf;
    gc->transform.fmaxy = gc->transform.maxy + __glHalf;

    /*
    ** Pickers that notice when the transformation matches the viewport
    ** exactly need to be revalidated.  Ugh.
    */
    (*gc->procs.pickMvpMatrixProcs)(gc, &gc->transform.modelView->mvp);
}

void APIENTRY __glim_Viewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    if ((w < 0) || (h < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (h > gc->constants.maxViewportHeight) {
        h = gc->constants.maxViewportHeight;
    }
    if (w > gc->constants.maxViewportWidth) {
        w = gc->constants.maxViewportWidth;
    }

    gc->state.viewport.x = x;
    gc->state.viewport.y = y;
    gc->state.viewport.width = w;
    gc->state.viewport.height = h;

    __glUpdateViewport(gc);

    (*gc->procs.applyViewport)(gc);

    __glUpdateViewportTransform(gc);

    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_DepthRange(GLdouble zNear, GLdouble zFar)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    SetDepthRange(gc, zNear, zFar);
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH | __GL_DIRTY_GENERIC);
}

void APIENTRY __glim_Scissor(GLint x, GLint y, GLsizei w, GLsizei h)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    if ((w < 0) || (h < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    /* return if nothing changed */
    if ((x == gc->state.scissor.scissorX) &&
        (y == gc->state.scissor.scissorY) &&
        (w == gc->state.scissor.scissorWidth) &&
        (h == gc->state.scissor.scissorHeight)) {
        return;
    }

    gc->state.scissor.scissorX = x;
    gc->state.scissor.scissorY = y;
    gc->state.scissor.scissorWidth = w;
    gc->state.scissor.scissorHeight = h;
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_SCISSOR);
    (*gc->procs.applyScissor)(gc);
    (*gc->procs.computeClipBox)(gc);
}

void APIENTRY __glim_ClipPlane(GLenum pi, const GLdouble pv[])
{
    __GLfloat p[4];
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    if(pi < GL_CLIP_PLANE0) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    pi -= GL_CLIP_PLANE0;
    if (pi >= (GLuint) gc->constants.numberOfClipPlanes) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    p[0] = pv[0];
    p[1] = pv[1];
    p[2] = pv[2];
    p[3] = pv[3];

    /*
    ** Project user clip plane into eye space.
    */
    tr = gc->transform.modelView;
    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }
    (*tr->inverseTranspose.xf4)(&gc->state.transform.eyeClipPlanes[pi], p,
                                &tr->inverseTranspose);

    __GL_DELAY_VALIDATE(gc);
}

/************************************************************************/

/* XXX the rest of the file should be moved into ../matrix */

void __glPushModelViewMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLint num;

    num = gc->constants.maxModelViewStackDepth;
    trp = &gc->transform.modelView;
    stack = gc->transform.modelViewStack;
    tr = *trp;
    if (tr < &stack[num-1]) {
        tr[1] = tr[0];
        *trp = tr + 1;
    } else {
        __glSetError(GL_STACK_OVERFLOW);
    }
}

void __glPopModelViewMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack, *mvtr, *ptr;

    trp = &gc->transform.modelView;
    stack = gc->transform.modelViewStack;
    tr = *trp;
    if (tr > &stack[0]) {
        *trp = tr - 1;

        /*
        ** See if sequence number of modelView matrix is the same as the
        ** sequence number of the projection matrix.  If not, then
        ** recompute the mvp matrix.
        */
        mvtr = gc->transform.modelView;
        ptr = gc->transform.projection;
        if (mvtr->sequence != ptr->sequence) {
            mvtr->sequence = ptr->sequence;
            (*gc->procs.matrix.mult)(&mvtr->mvp, &mvtr->matrix, &ptr->matrix);
        }
        (*gc->procs.pickMvpMatrixProcs)(gc, &mvtr->mvp);
        (*gc->procs.pickCullVertexProcs)(gc, mvtr);
    } else {
        __glSetError(GL_STACK_UNDERFLOW);
        return;
    }
}

void __glLoadIdentityModelViewMatrix(__GLcontext *gc)
{
    __GLtransform *mvtr, *ptr;
    void (*pick)(__GLcontext*, __GLmatrix*);

    mvtr = gc->transform.modelView;
    (*gc->procs.matrix.makeIdentity)(&mvtr->matrix);
    (*gc->procs.matrix.makeIdentity)(&mvtr->inverseTranspose);
    pick = gc->procs.pickMatrixProcs;
    mvtr->updateInverse = GL_FALSE;
    (*pick)(gc, &mvtr->matrix);
    (*gc->procs.pickInvTransposeProcs)(gc, &mvtr->inverseTranspose);

    /* Update mvp matrix */
    ptr = gc->transform.projection;
    mvtr->sequence = ptr->sequence;
    (*gc->procs.matrix.mult)(&mvtr->mvp, &mvtr->matrix, &ptr->matrix);
    (*gc->procs.pickMvpMatrixProcs)(gc, &mvtr->mvp);
    (*gc->procs.pickCullVertexProcs)(gc, mvtr);
}

void __glComputeInverseTranspose(__GLcontext *gc, __GLtransform *tr)
{
    (*gc->procs.matrix.invertTranspose)(&tr->inverseTranspose, &tr->matrix);
    (*gc->procs.pickInvTransposeProcs)(gc, &tr->inverseTranspose);
    tr->updateInverse = GL_FALSE;

    (*gc->procs.pickCullVertexProcs)(gc, tr);
}

void __glGenericPickCullVertexProcs(__GLcontext *gc, __GLtransform *tr)
{
    /* 
    ** If the user has not specified a culling eye position in object space,
    ** calculate object space eye direction or location.
    */
    if (gc->state.transform.objEyeSpecified) {
        gc->transform.cullEye = gc->state.transform.eyePosObj;
    } else {
        /* Allow inverse (and cull eye position to be lazy evaluated). */
        if (tr->updateInverse) {
            return;
        }

        __glTransposeMatrix(&tr->transpose, &tr->matrix);
        __glGenericPickMatrixProcs(gc, &tr->transpose);
        __glTransposeMatrix(&tr->inverse, &tr->inverseTranspose);
        __glGenericPickMatrixProcs(gc, &tr->inverse);

        if (gc->state.transform.eyePos.w != 0.0F) {
            (*tr->inverse.xf4)(&gc->transform.cullEye, 
                            &gc->state.transform.eyePos.x, 
                            &tr->inverse);
        } else {
            (*tr->transpose.xf3)(&gc->transform.cullEye,
                                &gc->state.transform.eyePos.x, 
                                &tr->transpose);
            gc->transform.cullEye.w = 0.0;
        }
    }
}

/************************************************************************/

void __glPushProjectionMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLint num;

    num = gc->constants.maxProjectionStackDepth;
    trp = &gc->transform.projection;
    stack = gc->transform.projectionStack;
    tr = *trp;
    if (tr < &stack[num-1]) {
        tr[1].matrix = tr[0].matrix;
        tr[1].sequence = tr[0].sequence;
        *trp = tr + 1;
    } else {
        __glSetError(GL_STACK_OVERFLOW);
    }
}

void __glPopProjectionMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack, *mvtr, *ptr;

    trp = &gc->transform.projection;
    stack = gc->transform.projectionStack;
    tr = *trp;
    if (tr > &stack[0]) {
        *trp = tr - 1;

        /*
        ** See if sequence number of modelView matrix is the same as the
        ** sequence number of the projection matrix.  If not, then
        ** recompute the mvp matrix.
        */
        mvtr = gc->transform.modelView;
        ptr = gc->transform.projection;
        if (mvtr->sequence != ptr->sequence) {
            mvtr->sequence = ptr->sequence;
            (*gc->procs.matrix.mult)(&mvtr->mvp, &mvtr->matrix, &ptr->matrix);
        }
        (*gc->procs.pickMvpMatrixProcs)(gc, &mvtr->mvp);
    } else {
        __glSetError(GL_STACK_UNDERFLOW);
        return;
    }
}

void __glLoadIdentityProjectionMatrix(__GLcontext *gc)
{
    __GLtransform *mvtr, *ptr;
    void (*pick)(__GLcontext*, __GLmatrix*);

    ptr = gc->transform.projection;
    (*gc->procs.matrix.makeIdentity)(&ptr->matrix);
    pick = gc->procs.pickMatrixProcs;
    (*pick)(gc, &ptr->matrix);
    if (++gc->transform.projectionSequence == 0) {
        __glInvalidateSequenceNumbers(gc);
    } else {
        ptr->sequence = gc->transform.projectionSequence;
    }

    /* Update mvp matrix */
    mvtr = gc->transform.modelView;
    mvtr->sequence = ptr->sequence;
    (*gc->procs.matrix.mult)(&mvtr->mvp, &mvtr->matrix, &ptr->matrix);
    (*gc->procs.pickMvpMatrixProcs)(gc, &mvtr->mvp);
}

/************************************************************************/

void __glPushTextureMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;
    GLint num;

    num = gc->constants.maxTextureStackDepth;
    trp = &gc->transform.texture[gc->texture.currentTexUnit];
    stack = gc->transform.textureStack[gc->texture.currentTexUnit];
    tr = *trp;
    if (tr < &stack[num-1]) {
        tr[1].matrix = tr[0].matrix;
        *trp = tr + 1;
    } else {
        __glSetError(GL_STACK_OVERFLOW);
    }
}

void __glPopTextureMatrix(__GLcontext *gc)
{
    __GLtransform **trp, *tr, *stack;

    trp = &gc->transform.texture[gc->texture.currentTexUnit];
    stack = gc->transform.textureStack[gc->texture.currentTexUnit];
    tr = *trp;
    if (tr > &stack[0]) {
        *trp = tr - 1;
    } else {
        __glSetError(GL_STACK_UNDERFLOW);
        return;
    }
}

void __glLoadIdentityTextureMatrix(__GLcontext *gc)
{
    __GLtransform *tr = gc->transform.texture[gc->texture.currentTexUnit];

    (*gc->procs.matrix.makeIdentity)(&tr->matrix);
    (*gc->procs.pickMatrixProcs)(gc, &tr->matrix);
    gc->slowPath &= ~(__GL_TEXTURE_MATRIX_SLOWPATH_0<<gc->texture.currentTexUnit);
}

/************************************************************************/

void __glDoLoadMatrix(__GLcontext *gc, const __GLmatrix *m)
{
    __GLtransform *tr, *otr;
    void (*pick)(__GLcontext*, __GLmatrix*);

    switch (gc->state.transform.matrixMode) {
      case GL_MODELVIEW:
        tr = gc->transform.modelView;
        (*gc->procs.matrix.copy)(&tr->matrix, m);
        tr->updateInverse = GL_TRUE;
        pick = gc->procs.pickMatrixProcs;
        (*pick)(gc, &tr->matrix);

        /* Update mvp matrix */
        otr = gc->transform.projection;
        tr->sequence = otr->sequence;
        (*gc->procs.matrix.mult)(&tr->mvp, &tr->matrix, &otr->matrix);
        (*gc->procs.pickMvpMatrixProcs)(gc, &tr->mvp);
        (*gc->procs.pickCullVertexProcs)(gc, tr);
        break;

      case GL_PROJECTION:
        tr = gc->transform.projection;
        (*gc->procs.matrix.copy)(&tr->matrix, m);
        pick = gc->procs.pickMatrixProcs;
        (*pick)(gc, &tr->matrix);
        if (++gc->transform.projectionSequence == 0) {
            __glInvalidateSequenceNumbers(gc);
        } else {
            tr->sequence = gc->transform.projectionSequence;
        }

        /* Update mvp matrix */
        otr = gc->transform.modelView;
        otr->sequence = tr->sequence;
        (*gc->procs.matrix.mult)(&otr->mvp, &otr->matrix, &tr->matrix);
        (*gc->procs.pickMvpMatrixProcs)(gc, &otr->mvp);
        break;

      case GL_TEXTURE:
        tr = gc->transform.texture[gc->texture.currentTexUnit];
        (*gc->procs.matrix.copy)(&tr->matrix, m);
        (*gc->procs.pickMatrixProcs)(gc, &tr->matrix);
        break;
    }
}

void __glDoMultMatrix(__GLcontext *gc, void *data, 
                      void (*multiply)(__GLcontext *gc, __GLmatrix *m, 
                      void *data))
{
    __GLtransform *tr, *otr;
    void (*pick)(__GLcontext*, __GLmatrix*);

    switch (gc->state.transform.matrixMode) {
      case GL_MODELVIEW:
        tr = gc->transform.modelView;
        (*multiply)(gc, &tr->matrix, data);
        tr->updateInverse = GL_TRUE;
        pick = gc->procs.pickMatrixProcs;
        (*pick)(gc, &tr->matrix);

        /* Update mvp matrix */
        (*multiply)(gc, &tr->mvp, data);
        (*gc->procs.pickMvpMatrixProcs)(gc, &tr->mvp);
        (*gc->procs.pickCullVertexProcs)(gc, tr);
        break;

      case GL_PROJECTION:
        tr = gc->transform.projection;
        (*multiply)(gc, &tr->matrix, data);
        pick = gc->procs.pickMatrixProcs;
        (*pick)(gc, &tr->matrix);
        if (++gc->transform.projectionSequence == 0) {
            __glInvalidateSequenceNumbers(gc);
        } else {
            tr->sequence = gc->transform.projectionSequence;
        }

        /* Update mvp matrix */
        otr = gc->transform.modelView;
        otr->sequence = tr->sequence;
        (*gc->procs.matrix.mult)(&otr->mvp, &otr->matrix, &tr->matrix);
        (*gc->procs.pickMvpMatrixProcs)(gc, &otr->mvp);
        break;

      case GL_TEXTURE:
        tr = gc->transform.texture[gc->texture.currentTexUnit];
        (*multiply)(gc, &tr->matrix, data);
        (*gc->procs.pickMatrixProcs)(gc, &tr->matrix);
        if (tr->matrix.matrixType == __GL_MT_IDENTITY) {
            gc->slowPath &= ~(__GL_TEXTURE_MATRIX_SLOWPATH_0<<gc->texture.currentTexUnit);
        } else {
            gc->slowPath |= (__GL_TEXTURE_MATRIX_SLOWPATH_0<<gc->texture.currentTexUnit);;
        }
        break;
    }
}

/************************************************************************/

void __glDoRotate(__GLcontext *gc, __GLfloat angle, __GLfloat ax,
                  __GLfloat ay, __GLfloat az)
{
    __GLmatrix m;
    __GLfloat radians, sine, cosine, ab, bc, ca, t;
    __GLfloat av[4], axis[4];

    av[0] = ax;
    av[1] = ay;
    av[2] = az;
    av[3] = 0;
    (*gc->procs.normalize)(axis, av);

    radians = angle * __glDegreesToRadians;
    sine = __GL_SINF(radians);
    cosine = __GL_COSF(radians);
    ab = axis[0] * axis[1] * (1 - cosine);
    bc = axis[1] * axis[2] * (1 - cosine);
    ca = axis[2] * axis[0] * (1 - cosine);

    (*gc->procs.matrix.makeIdentity)(&m);
    t = axis[0] * axis[0];
    m.matrix[0][0] = t + cosine * (1 - t);
    m.matrix[2][1] = bc - axis[0] * sine;
    m.matrix[1][2] = bc + axis[0] * sine;

    t = axis[1] * axis[1];
    m.matrix[1][1] = t + cosine * (1 - t);
    m.matrix[2][0] = ca + axis[1] * sine;
    m.matrix[0][2] = ca - axis[1] * sine;

    t = axis[2] * axis[2];
    m.matrix[2][2] = t + cosine * (1 - t);
    m.matrix[1][0] = ab - axis[2] * sine;
    m.matrix[0][1] = ab + axis[2] * sine;
    if (ax == __glZero && ay == __glZero) {
        m.matrixType = __GL_MT_IS2D;
    } else {
        m.matrixType = __GL_MT_W0001;
    }
    __glDoMultMatrix(gc, &m, __glMultiplyMatrix);
}

struct __glScaleRec {
    __GLfloat x,y,z;
};

/* ARGSUSED */
void __glScaleMatrix(__GLcontext *gc, __GLmatrix *m, void *data)
{
    struct __glScaleRec *scale;
    __GLfloat x,y,z;
    __GLfloat M0, M1, M2, M3;

    if (m->matrixType > __GL_MT_IS2DNR) {
        m->matrixType = __GL_MT_IS2DNR;
    }
    scale = data;
    x = scale->x;
    y = scale->y;
    z = scale->z;
    
    M0 = x * m->matrix[0][0];
    M1 = x * m->matrix[0][1];
    M2 = x * m->matrix[0][2];
    M3 = x * m->matrix[0][3];
    m->matrix[0][0] = M0;
    m->matrix[0][1] = M1;
    m->matrix[0][2] = M2;
    m->matrix[0][3] = M3;

    M0 = y * m->matrix[1][0];
    M1 = y * m->matrix[1][1];
    M2 = y * m->matrix[1][2];
    M3 = y * m->matrix[1][3];
    m->matrix[1][0] = M0;
    m->matrix[1][1] = M1;
    m->matrix[1][2] = M2;
    m->matrix[1][3] = M3;

    M0 = z * m->matrix[2][0];
    M1 = z * m->matrix[2][1];
    M2 = z * m->matrix[2][2];
    M3 = z * m->matrix[2][3];
    m->matrix[2][0] = M0;
    m->matrix[2][1] = M1;
    m->matrix[2][2] = M2;
    m->matrix[2][3] = M3;
}

void __glDoScale(__GLcontext *gc, __GLfloat x, __GLfloat y, __GLfloat z)
{
    struct __glScaleRec scale;

    scale.x = x;
    scale.y = y;
    scale.z = z;
    __glDoMultMatrix(gc, &scale, __glScaleMatrix);
}

struct __glTranslationRec {
    __GLfloat x,y,z;
};

/*
** Matrix type of m stays the same.
*/
/* ARGSUSED */
void __glTranslateMatrix(__GLcontext *gc, __GLmatrix *m, void *data)
{
    struct __glTranslationRec *trans;
    __GLfloat x,y,z;
    __GLfloat _M30, _M31, _M32, _M33;

    if (m->matrixType > __GL_MT_IS2DNR) {
        m->matrixType = __GL_MT_IS2DNR;
    }
    trans = data;
    x = trans->x;
    y = trans->y;
    z = trans->z;
    _M30 = x * m->matrix[0][0] + y * m->matrix[1][0] + z * m->matrix[2][0] + 
            m->matrix[3][0];
    _M31 = x * m->matrix[0][1] + y * m->matrix[1][1] + z * m->matrix[2][1] + 
            m->matrix[3][1];
    _M32 = x * m->matrix[0][2] + y * m->matrix[1][2] + z * m->matrix[2][2] + 
            m->matrix[3][2];
    _M33 = x * m->matrix[0][3] + y * m->matrix[1][3] + z * m->matrix[2][3] + 
            m->matrix[3][3];
    m->matrix[3][0] = _M30;
    m->matrix[3][1] = _M31;
    m->matrix[3][2] = _M32;
    m->matrix[3][3] = _M33;
}

void __glDoTranslate(__GLcontext *gc, __GLfloat x, __GLfloat y, __GLfloat z)
{
    struct __glTranslationRec trans;

    trans.x = x;
    trans.y = y;
    trans.z = z;
    __glDoMultMatrix(gc, &trans, __glTranslateMatrix);
}

/************************************************************************/

/*
** Compute the clip box from the scissor (if enabled) and the window
** size.  The resulting clip box is used to clip primitive rasterization
** against.  The "window system" is responsible for doing the fine
** grain clipping (i.e., dealing with overlapping windows, etc.).
*/
void __glComputeClipBox(__GLcontext *gc)
{
    __GLscissor *sp = &gc->state.scissor;
    GLint llx, lly, urx, ury;
    GLint y0, y1;

    if (gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE) {
        llx = sp->scissorX;
        lly = sp->scissorY;
        urx = llx + sp->scissorWidth;
        ury = lly + sp->scissorHeight;

        if ((urx < 0) || (ury < 0) ||
            (urx <= llx) || (ury <= lly) ||
            (llx >= gc->constants.width) || (lly >= gc->constants.height)) {
            llx = lly = urx = ury = 0;
        } else {
            if (llx < 0) llx = 0;
            if (lly < 0) lly = 0;
            if (urx > gc->constants.width) urx = gc->constants.width;
            if (ury > gc->constants.height) ury = gc->constants.height;
        }
    } else {
        llx = 0;
        lly = 0;
        urx = gc->constants.width;
        ury = gc->constants.height;
    }

    if (gc->constants.yInverted) {
        y0 = gc->constants.height - ury;
        y1 = gc->constants.height - lly;
    } else {
        y0 = lly;
        y1 = ury;
    }

    (*gc->drawablePrivate->setClipRect)(gc->drawablePrivate,
                                        llx, y0, urx-llx, y1-y0);

    gc->transform.clipX0 = llx + gc->constants.viewportXAdjust;
    gc->transform.clipY0 = y0 + gc->constants.viewportYAdjust;
    gc->transform.clipX1 = urx + gc->constants.viewportXAdjust;
    gc->transform.clipY1 = y1 + gc->constants.viewportYAdjust;
}

/************************************************************************/

/*
** Note: These xform routines must allow for the case where the result
** vector is equal to the source vector.
*/

#ifndef __GL_USE_MIPSASMCODE
/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1
*/
void __glXForm2(__GLcoord *res, const __GLfloat v[2], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
    res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2];
    res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + m->matrix[3][3];
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.
*/
#ifdef __GL_USE_INTEL_ASM
void __glXForm3(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m )
{
    const float *matrix = ( const float * ) m->matrix;

    __asm mov eax, matrix       ; EAX = matrix
    __asm mov esi, v            ; ESI = source vector
    __asm mov edi, res          ; EDI = destination
    
    /*
    ** fill up the FP stack with the first 8 results
    */
    __asm fld   dword ptr [esi+V_X]  ; x
    __asm fmul  dword ptr [eax+M00]  ; x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m00
    __asm fmul  dword ptr [eax+M01]  ; x*m01 | x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M02]  ; x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M03]  ; x*m03 | x*m02 | x*m01 | x*m00
    
    __asm fld   dword ptr [esi+V_Y]  ; y | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M10]  ; y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M11]  ; y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M12]  ; y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M13]  ; y*m13 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    
    /*
    ** start adding 'em up
    */
    __asm fxch  st(7)                ; x*m00 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | y*m13
    __asm faddp st(3), st            ; y*m12 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | x*m01 | y*m13
    __asm fxch  st(5)                ; x*m01 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
    __asm faddp st(1), st            ; x*m01 + y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
    __asm fxch  st(3)                ; x*m02 | x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | y*m12 | y*m13
    __asm faddp st(4), st            ; x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
    __asm fxch  st(1)                ; x*m03 | x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
    __asm faddp st(4), st            ; x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | x*m03 + y*m13
    
    /*
    ** while that's still adding and popping, we do our Z calculations
    */
    __asm fld   dword ptr [esi+V_Z]  ; z | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M20]  ; z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M21]  ; z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M22]  ; z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M23]  ; z*m23 | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    
    /*
    ** stack's full again, roll in the Z values now
    */
    __asm fxch  st(3)                ; z*m20 | z*m22 | z*m21 | z*m23 | xy0 | xy1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m22 | z*m21 | z*m23 | xyz0 | xy1 | xy2 | xy3 
    __asm fxch  st(1)                ; z*m21 | z*m22 | z*m23 | xyz0 | xy1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m22 | z*m23 | xyz0 | xyz1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m23 | xyz0 | xyz1 | xyz2 | xy3 
    __asm faddp st(4), st            ; xyz0 | xyz1 | xyz2 | xyz3 
    
    /*
    ** whaddya know, we have 4 slots left, so load up that last column
    */
    __asm fadd  dword ptr [eax+M30]  ; xyzw0 | xyz1  | xyz2 | xyz3
    __asm fxch  st(1)                ; xyz1  | xyzw0 | xyz2 | xyz3
    __asm fadd  dword ptr [eax+M31]  ; xyzw1 | xyzw0 | xyz2 | xyz3
    __asm fxch  st(2)                ; xyz2  | xyzw0 | xyzw1 | xyz3
    __asm fadd  dword ptr [eax+M32]  ; xyzw2 | xyzw0 | xyzw1 | xyz3
    __asm fxch  st(3)                ; xyz3 | xyzw0 | xyzw1 | xyzw2
    __asm fadd  dword ptr [eax+M33]  ; xyzw3 | xyzw0 | xyzw1 | xyzw2
    __asm fxch  st(1)                ; xyzw0 | xyzw3 | xyzw1 | xyzw2
    __asm fstp  dword ptr [edi+RES_X]; xyzw3 | xyzw1 | xyzw2
    __asm fxch  st(1)                ; xyzw1 | xyzw3 | xyzw2
    __asm fstp  dword ptr [edi+RES_Y]; xyzw3 | xyzw2
    __asm fstp  dword ptr [edi+RES_W]; xyzw2
    __asm fstp  dword ptr [edi+RES_Z]; (empty)
}

void __glXForm3_Batch(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m, int sstride, int dstride, int n )
{
    const float *matrix = ( const float * ) m->matrix;

    __asm mov eax, matrix       ; EAX = matrix
    __asm mov esi, v            ; ESI = source vector
    __asm mov edi, res          ; EDI = destination
    __asm mov ecx, n            ; ECX = count
    __asm mov ebx, sstride      ; EBX = sstride
    __asm mov edx, dstride      ; EDX = dstride

    __asm cmp ecx, 0       
    __asm jle done

top_of_loop:
    /*
    ** fill up the FP stack with the first 8 results
    */
    __asm fld   dword ptr [esi+V_X]  ; x
    __asm fmul  dword ptr [eax+M00]  ; x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m00
    __asm fmul  dword ptr [eax+M01]  ; x*m01 | x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M02]  ; x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M03]  ; x*m03 | x*m02 | x*m01 | x*m00
    
    __asm fld   dword ptr [esi+V_Y]  ; y | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M10]  ; y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M11]  ; y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M12]  ; y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M13]  ; y*m13 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    
    /*
    ** start adding 'em up
    */
    __asm fxch  st(7)                ; x*m00 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | y*m13
    __asm faddp st(3), st            ; y*m12 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | x*m01 | y*m13
    __asm fxch  st(5)                ; x*m01 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
    __asm faddp st(1), st            ; x*m01 + y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
    __asm fxch  st(3)                ; x*m02 | x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | y*m12 | y*m13
    __asm faddp st(4), st            ; x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
    __asm fxch  st(1)                ; x*m03 | x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
    __asm faddp st(4), st            ; x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | x*m03 + y*m13
    
    /*
    ** while that's still adding and popping, we do our Z calculations
    */
    __asm fld   dword ptr [esi+V_Z]  ; z | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M20]  ; z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M21]  ; z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M22]  ; z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M23]  ; z*m23 | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    
    /*
    ** stack's full again, roll in the Z values now
    */
    __asm fxch  st(3)                ; z*m20 | z*m22 | z*m21 | z*m23 | xy0 | xy1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m22 | z*m21 | z*m23 | xyz0 | xy1 | xy2 | xy3 
    __asm fxch  st(1)                ; z*m21 | z*m22 | z*m23 | xyz0 | xy1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m22 | z*m23 | xyz0 | xyz1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m23 | xyz0 | xyz1 | xyz2 | xy3 
    __asm faddp st(4), st            ; xyz0 | xyz1 | xyz2 | xyz3 
    
    /*
    ** whaddya know, we have 4 slots left, so load up that last column
    */
    __asm fadd  dword ptr [eax+M30]  ; xyzw0 | xyz1  | xyz2 | xyz3
    __asm fxch  st(1)                ; xyz1  | xyzw0 | xyz2 | xyz3
    __asm fadd  dword ptr [eax+M31]  ; xyzw1 | xyzw0 | xyz2 | xyz3
    __asm fxch  st(2)                ; xyz2  | xyzw0 | xyzw1 | xyz3
    __asm fadd  dword ptr [eax+M32]  ; xyzw2 | xyzw0 | xyzw1 | xyz3
    __asm fxch  st(3)                ; xyz3 | xyzw0 | xyzw1 | xyzw2
    __asm fadd  dword ptr [eax+M33]  ; xyzw3 | xyzw0 | xyzw1 | xyzw2
    __asm fxch  st(1)                ; xyzw0 | xyzw3 | xyzw1 | xyzw2
    __asm fstp  dword ptr [edi+RES_X]; xyzw3 | xyzw1 | xyzw2
    __asm fxch  st(1)                ; xyzw1 | xyzw3 | xyzw2
    __asm fstp  dword ptr [edi+RES_Y]; xyzw3 | xyzw2
    __asm fstp  dword ptr [edi+RES_W]; xyzw2
    __asm fstp  dword ptr [edi+RES_Z]; (empty)

    /*
    ** decrement count and update iterators
    */
    __asm add  esi, ebx             ; v++
    __asm add  edi, edx             ; res++
    __asm dec  ecx                  ; --n
    __asm jnz  top_of_loop
done:
    __asm nop
}

#else
void __glXForm3(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

        res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
            + m->matrix[3][0];
        res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
            + m->matrix[3][1];
        res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
            + m->matrix[3][2];
        res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
            + m->matrix[3][3];
}
void __glXForm3_Batch(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m, int sstride, int dstride, int n )
{
    while ( n )
    {
        __GLfloat x = v[0];
        __GLfloat y = v[1];
        __GLfloat z = v[2];
        
        res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
            + m->matrix[3][0];
        res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
            + m->matrix[3][1];
        res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
            + m->matrix[3][2];
        res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
            + m->matrix[3][3];

        res = ( __GLcoord * ) ( ( ( char * ) res ) + dstride );
        v   = ( const float * ) ( ( ( char * ) v ) + sstride );
        --n;
    }
}
#endif

/*
** Full 4x4 transformation.
*/
#ifdef __GL_USE_INTEL_ASM
void __glXForm4(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m)
{
    const float *matrix = ( const float * ) m->matrix;

    if ( v[3] == ((__GLfloat) 1.0)) 
    {
        __asm mov eax, matrix       ; EAX = matrix
        __asm mov esi, v            ; ESI = source vector
        __asm mov edi, res          ; EDI = destination
    
        /*
        ** fill up the FP stack with the first 8 results
        */
        __asm fld   dword ptr [esi+V_X]  ; x
        __asm fmul  dword ptr [eax+M00]  ; x*m00
        __asm fld   dword ptr [esi+V_X]  ; x | x*m00
        __asm fmul  dword ptr [eax+M01]  ; x*m01 | x*m00
        __asm fld   dword ptr [esi+V_X]  ; x | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M02]  ; x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_X]  ; x | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M03]  ; x*m03 | x*m02 | x*m01 | x*m00
    
        __asm fld   dword ptr [esi+V_Y]  ; y | x*m03 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M10]  ; y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]  ; y | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M11]  ; y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]  ; y | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M12]  ; y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]  ; y | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M13]  ; y*m13 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    
        /*
        ** start adding 'em up
        */
        __asm fxch  st(7)                ; x*m00 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | y*m13
        __asm faddp st(3), st            ; y*m12 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | x*m01 | y*m13
        __asm fxch  st(5)                ; x*m01 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
        __asm faddp st(1), st            ; x*m01 + y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
        __asm fxch  st(3)                ; x*m02 | x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | y*m12 | y*m13
        __asm faddp st(4), st            ; x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
        __asm fxch  st(1)                ; x*m03 | x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
        __asm faddp st(4), st            ; x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | x*m03 + y*m13
    
        /*
        ** while that's still adding and popping, we do our Z calculations
        */
        __asm fld   dword ptr [esi+V_Z]  ; z | xy0 | xy1 | xy2 | xy3 
        __asm fmul  dword ptr [eax+M20]  ; z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fld   dword ptr [esi+V_Z]  ; z | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fmul  dword ptr [eax+M21]  ; z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fld   dword ptr [esi+V_Z]  ; z | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fmul  dword ptr [eax+M22]  ; z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fld   dword ptr [esi+V_Z]  ; z | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fmul  dword ptr [eax+M23]  ; z*m23 | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    
        /*
        ** stack's full again, roll in the Z values now
        */
        __asm fxch  st(3)                ; z*m20 | z*m22 | z*m21 | z*m23 | xy0 | xy1 | xy2 | xy3 
        __asm faddp st(4), st            ; z*m22 | z*m21 | z*m23 | xyz0 | xy1 | xy2 | xy3 
        __asm fxch  st(1)                ; z*m21 | z*m22 | z*m23 | xyz0 | xy1 | xy2 | xy3 
        __asm faddp st(4), st            ; z*m22 | z*m23 | xyz0 | xyz1 | xy2 | xy3 
        __asm faddp st(4), st            ; z*m23 | xyz0 | xyz1 | xyz2 | xy3 
        __asm faddp st(4), st            ; xyz0 | xyz1 | xyz2 | xyz3 
    
        /*
        ** whaddya know, we have 4 slots left, so load up that last column
        */
        __asm fadd  dword ptr [eax+M30]  ; xyzw0 | xyz1  | xyz2 | xyz3
        __asm fxch  st(1)                ; xyz1  | xyzw0 | xyz2 | xyz3
        __asm fadd  dword ptr [eax+M31]  ; xyzw1 | xyzw0 | xyz2 | xyz3
        __asm fxch  st(2)                ; xyz2  | xyzw0 | xyzw1 | xyz3
        __asm fadd  dword ptr [eax+M32]  ; xyzw2 | xyzw0 | xyzw1 | xyz3
        __asm fxch  st(3)                ; xyz3 | xyzw0 | xyzw1 | xyzw2
        __asm fadd  dword ptr [eax+M33]  ; xyzw3 | xyzw0 | xyzw1 | xyzw2
        __asm fxch  st(1)                ; xyzw0 | xyzw3 | xyzw1 | xyzw2
        __asm fstp  dword ptr [edi+RES_X]; xyzw3 | xyzw1 | xyzw2
        __asm fxch  st(1)                ; xyzw1 | xyzw3 | xyzw2
        __asm fstp  dword ptr [edi+RES_Y]; xyzw3 | xyzw2
        __asm fstp  dword ptr [edi+RES_W]; xyzw2
        __asm fstp  dword ptr [edi+RES_Z]; (empty)
    }
    else
    {
        __asm mov eax, matrix       ; EAX = matrix
        __asm mov esi, v            ; ESI = source vector
        __asm mov edi, res          ; EDI = destination

        /*
        ** fill up the FP stack with the first 8 results
        */
        __asm fld   dword ptr [esi+V_X]  ; x
        __asm fmul  dword ptr [eax+M00]  ; x*m00
        __asm fld   dword ptr [esi+V_X]  ; x | x*m00
        __asm fmul  dword ptr [eax+M01]  ; x*m01 | x*m00
        __asm fld   dword ptr [esi+V_X]  ; x | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M02]  ; x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_X]  ; x | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M03]  ; x*m03 | x*m02 | x*m01 | x*m00
        
        __asm fld   dword ptr [esi+V_Y]  ; y | x*m03 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M10]  ; y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]  ; y | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M11]  ; y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]  ; y | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M12]  ; y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]  ; y | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M13]  ; y*m13 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
        
        /*
        ** start adding 'em up
        */
        __asm fxch  st(7)                ; x*m00 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | y*m13
        __asm faddp st(3), st            ; y*m12 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | x*m01 | y*m13
        __asm fxch  st(5)                ; x*m01 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
        __asm faddp st(1), st            ; x*m01 + y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
        __asm fxch  st(3)                ; x*m02 | x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | y*m12 | y*m13
        __asm faddp st(4), st            ; x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
        __asm fxch  st(1)                ; x*m03 | x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
        __asm faddp st(4), st            ; x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | x*m03 + y*m13

        /*
        ** while that's still adding and popping, we do our Z calculations
        */
        __asm fld   dword ptr [esi+V_Z]  ; z | xy0 | xy1 | xy2 | xy3 
        __asm fmul  dword ptr [eax+M20]  ; z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fld   dword ptr [esi+V_Z]  ; z | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fmul  dword ptr [eax+M21]  ; z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fld   dword ptr [esi+V_Z]  ; z | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fmul  dword ptr [eax+M22]  ; z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fld   dword ptr [esi+V_Z]  ; z | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
        __asm fmul  dword ptr [eax+M23]  ; z*m23 | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 

        /*
        ** stack's full again, roll in the Z values now
        */
        __asm fxch  st(3)                ; z*m20 | z*m22 | z*m21 | z*m23 | xy0 | xy1 | xy2 | xy3 
        __asm faddp st(4), st            ; z*m22 | z*m21 | z*m23 | xyz0 | xy1 | xy2 | xy3 
        __asm fxch  st(1)                ; z*m21 | z*m22 | z*m23 | xyz0 | xy1 | xy2 | xy3 
        __asm faddp st(4), st            ; z*m22 | z*m23 | xyz0 | xyz1 | xy2 | xy3 
        __asm faddp st(4), st            ; z*m23 | xyz0 | xyz1 | xyz2 | xy3 
        __asm faddp st(4), st            ; xyz0 | xyz1 | xyz2 | xyz3 

        /*
        ** whaddya know, we have 4 slots left, so load up that last column
        */
        __asm fld   dword ptr [esi+V_W]  ; w | xyz0 | xyz1 | xyz2 | xyz3 
        __asm fmul  dword ptr [eax+M30]  ; w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
        __asm fld   dword ptr [esi+V_W]  ; w | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
        __asm fmul  dword ptr [eax+M31]  ; w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
        __asm fld   dword ptr [esi+V_W]  ; w | w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
        __asm fmul  dword ptr [eax+M32]  ; w*m32 | w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
        __asm fld   dword ptr [esi+V_W]  ; w | w*m32 | w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
        __asm fmul  dword ptr [eax+M33]  ; w*m33 | w*m32 | w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 

        /*
        ** start adding up final results
        */
        __asm fxch  st(3)                ; w*m30 | w*m32 | w*m31 | w*m33 | xyz0 | xyz1 | xyz2 | xyz3 
        __asm faddp st(4), st            ; w*m32 | w*m31 | w*m33 | xyzw0 | xyz1 | xyz2 | xyz3 
        __asm fxch  st(1)                ; w*m31 | w*m32 | w*m33 | xyzw0 | xyz1 | xyz2 | xyz3 
        __asm faddp st(4), st            ; w*m32 | w*m33 | xyzw0 | xyzw1 | xyz2 | xyz3 
        __asm faddp st(4), st            ; w*m33 | xyzw0 | xyzw1 | xyzw2 | xyz3 
        __asm faddp st(4), st            ; xyzw0 | xyzw1 | xyzw2 | xyzw3 
        __asm fstp  dword ptr [edi+RES_X]; xyzw1 | xyzw2 | xyzw3 
        __asm fstp  dword ptr [edi+RES_Y]; xyzw2 | xyzw3 
        __asm fstp  dword ptr [edi+RES_Z]; xyzw3 
        __asm fstp  dword ptr [edi+RES_W]; (empty)
    }
}
/*
** this version of the routine does NOT optimize for the W == 1.0 case because
** it's more complicated and probably not any faster to do so.  The rationale
** is this:
**
** If W == 1.0 then you can save 4 multiplies, which add up to about 7 clocks 
** per vertex.  But the cost for this is optimization is being forced to do
** a compare and branch per vertex, which is 1 clock of fixed overhead, plus
** an prefetch flush per branch.
**
** If the caller can guarantee that W == 1.0 for ALL incoming vertices it 
** should call __glXForm3_Batch instead.
*/
void __glXForm4_Batch(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m, int sstride, int dstride, int n )
{

    const float *matrix = ( const float * ) m->matrix;

    __asm mov edi, res          ; EDI = destination
    __asm mov esi, v            ; ESI = source vector
    __asm mov eax, matrix       ; EAX = matrix
    __asm mov ecx, n            ; ECX = count
    __asm mov ebx, sstride      ; EBX = sstride
    __asm mov edx, dstride      ; EDX = dstride

    __asm cmp ecx, 0
    __asm jle done

top_of_loop:

    /*
    ** fill up the FP stack with the first 8 results
    */
    __asm fld   dword ptr [esi+V_X]  ; x
    __asm fmul  dword ptr [eax+M00]  ; x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m00
    __asm fmul  dword ptr [eax+M01]  ; x*m01 | x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M02]  ; x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_X]  ; x | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M03]  ; x*m03 | x*m02 | x*m01 | x*m00

    __asm fld   dword ptr [esi+V_Y]  ; y | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M10]  ; y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M11]  ; y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M12]  ; y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]  ; y | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M13]  ; y*m13 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | x*m00

    /*
    ** start adding 'em up
    */
    __asm fxch  st(7)                ; x*m00 | y*m12 | y*m11 | y*m10 | x*m03 | x*m02 | x*m01 | y*m13
    __asm faddp st(3), st            ; y*m12 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | x*m01 | y*m13
    __asm fxch  st(5)                ; x*m01 | y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
    __asm faddp st(1), st            ; x*m01 + y*m11 | x*m00 + y*m10 | x*m03 | x*m02 | y*m12 | y*m13
    __asm fxch  st(3)                ; x*m02 | x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | y*m12 | y*m13
    __asm faddp st(4), st            ; x*m00 + y*m10 | x*m03 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
    __asm fxch  st(1)                ; x*m03 | x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | y*m13
    __asm faddp st(4), st            ; x*m00 + y*m10 | x*m01 + y*m11 | x*m02 + y*m12 | x*m03 + y*m13

    /*
    ** while that's still adding and popping, we do our Z calculations
    */
    __asm fld   dword ptr [esi+V_Z]  ; z | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M20]  ; z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M21]  ; z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M22]  ; z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fld   dword ptr [esi+V_Z]  ; z | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 
    __asm fmul  dword ptr [eax+M23]  ; z*m23 | z*m22 | z*m21 | z*m20 | xy0 | xy1 | xy2 | xy3 

    /*
    ** stack's full again, roll in the Z values now
    */
    __asm fxch  st(3)                ; z*m20 | z*m22 | z*m21 | z*m23 | xy0 | xy1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m22 | z*m21 | z*m23 | xyz0 | xy1 | xy2 | xy3 
    __asm fxch  st(1)                ; z*m21 | z*m22 | z*m23 | xyz0 | xy1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m22 | z*m23 | xyz0 | xyz1 | xy2 | xy3 
    __asm faddp st(4), st            ; z*m23 | xyz0 | xyz1 | xyz2 | xy3 
    __asm faddp st(4), st            ; xyz0 | xyz1 | xyz2 | xyz3 

    /*
    ** whaddya know, we have 4 slots left, so load up that last column
    */
    __asm fld   dword ptr [esi+V_W]  ; w | xyz0 | xyz1 | xyz2 | xyz3 
    __asm fmul  dword ptr [eax+M30]  ; w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
    __asm fld   dword ptr [esi+V_W]  ; w | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
    __asm fmul  dword ptr [eax+M31]  ; w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
    __asm fld   dword ptr [esi+V_W]  ; w | w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
    __asm fmul  dword ptr [eax+M32]  ; w*m32 | w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
    __asm fld   dword ptr [esi+V_W]  ; w | w*m32 | w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 
    __asm fmul  dword ptr [eax+M33]  ; w*m33 | w*m32 | w*m31 | w*m30 | xyz0 | xyz1 | xyz2 | xyz3 

    /*
    ** start adding up final results
    */
    __asm fxch  st(3)                ; w*m30 | w*m32 | w*m31 | w*m33 | xyz0 | xyz1 | xyz2 | xyz3 
    __asm faddp st(4), st            ; w*m32 | w*m31 | w*m33 | xyzw0 | xyz1 | xyz2 | xyz3 
    __asm fxch  st(1)                ; w*m31 | w*m32 | w*m33 | xyzw0 | xyz1 | xyz2 | xyz3 
    __asm faddp st(4), st            ; w*m32 | w*m33 | xyzw0 | xyzw1 | xyz2 | xyz3 
    __asm faddp st(4), st            ; w*m33 | xyzw0 | xyzw1 | xyzw2 | xyz3 
    __asm faddp st(4), st            ; xyzw0 | xyzw1 | xyzw2 | xyzw3 
    __asm fstp  dword ptr [edi+RES_X]; xyzw1 | xyzw2 | xyzw3 
    __asm fstp  dword ptr [edi+RES_Y]; xyzw2 | xyzw3 
    __asm fstp  dword ptr [edi+RES_Z]; xyzw3 
    __asm fstp  dword ptr [edi+RES_W]; (empty)

    __asm add  esi, ebx               ; v++
    __asm add  edi, edx               ; res++
    __asm dec  ecx                    ; -n
    __asm jnz  top_of_loop
done:
    __asm nop
}
#else
void __glXForm4(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];

    if (w == ((__GLfloat) 1.0)) {
        res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
            + m->matrix[3][0];
        res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
            + m->matrix[3][1];
        res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
            + m->matrix[3][2];
        res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
            + m->matrix[3][3];
    } else {
        res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
            + w*m->matrix[3][0];
        res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
            + w*m->matrix[3][1];
        res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
            + w*m->matrix[3][2];
        res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
            + w*m->matrix[3][3];
    }
}

void __glXForm4_Batch(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m, int sstride, int dstride, int n )
{
    while ( n )
    {
        __GLfloat x = v[0];
        __GLfloat y = v[1];
        __GLfloat z = v[2];
        __GLfloat w = v[3];
        
        if (w == ((__GLfloat) 1.0)) {
            res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
                + m->matrix[3][0];
            res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
                + m->matrix[3][1];
            res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
                + m->matrix[3][2];
            res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
                + m->matrix[3][3];
        } else {
            res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
                + w*m->matrix[3][0];
            res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
                + w*m->matrix[3][1];
            res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
                + w*m->matrix[3][2];
            res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
                + w*m->matrix[3][3];
        }

        v   = ( const float * ) ( ( ( char * ) v ) + sstride );
        res = ( __GLcoord * ) ( ( ( char * ) res ) + dstride );
        --n;
    }
}
#endif

/************************************************************************/

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1.  The w column of the matrix is [0 0 0 1].
*/
void __glXForm2_W(__GLcoord *res, const __GLfloat v[2], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
    res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.  The w column of the matrix is [0 0 0 1].
*/
#ifdef __GL_USE_INTEL_ASM
void __glXForm3_W(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m)
{
    const float *matrix = ( const float * ) m->matrix;

    __asm mov   edi, res
    __asm mov   esi, v
    __asm mov   eax, matrix

    __asm fld   dword ptr [esi+V_X]            ; x
    __asm fmul  dword ptr [eax+M00]            ; x*m00
    __asm fld   dword ptr [esi+V_X]            ; x | x*m00
    __asm fmul  dword ptr [eax+M01]            ; x*m01 | x*m00
    __asm fld   dword ptr [esi+V_X]            ; x | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M02]            ; x*m02 | x*m01 | x*m00

    __asm fld   dword ptr [esi+V_Y]            ; y | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M10]            ; y*m10 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]            ; y | y*m10 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M11]            ; y*m11 | y*m10 | x*m02 | x*m01 | x*m00
    __asm fld   dword ptr [esi+V_Y]            ; y | y*m11 | y*m10 | x*m02 | x*m01 | x*m00
    __asm fmul  dword ptr [eax+M12]            ; y*m12 | y*m11 | y*m10 | x*m02 | x*m01 | x*m00

    __asm fxch  st(5)                          ; x*m00 | y*m11 | y*m10 | x*m02 | x*m01 | y*m12
    __asm faddp st(2), st                      ; y*m11 | x*m00 + y*m10 | x*m02 | x*m01 | y*m12
    __asm faddp st(3), st                      ; x*m00 + y*m10 | x*m02 | x*m01+y*m11 | y*m12
    __asm fxch  st(1)                          ; x*m02 | x*m00 + y*m10 | x*m01+y*m11 | y*m12
    __asm faddp st(3), st                      ; x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12

    __asm fld   dword ptr [esi+V_Z]            ; z | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
    __asm fmul  dword ptr [eax+M20]            ; z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
    __asm fld   dword ptr [esi+V_Z]            ; z | z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
    __asm fmul  dword ptr [eax+M21]            ; z*m21 | z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
    __asm fld   dword ptr [esi+V_Z]            ; z | z*m21 | z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
    __asm fmul  dword ptr [eax+M22]            ; z*m22 | z*m21 | z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12

    __asm fxch  st(2)                          ; z*m20 | z*m21 | z*m22 | x*m00+y*m10 | x*m01+y*m11 | x*m02+y*m12
    __asm faddp st(3), st                      ; z*m21 | z*m22 | x*m00+y*m10+z*z20 | x*m01+y*m11 | x*m02+y*m12
    __asm faddp st(3), st                      ; z*m22 | x*m00+y*m10+z*z20 | x*m01+y*m11+z*m21 | x*m02+y*m12
    __asm faddp st(3), st                      ; x*m00+y*m10+z*z20 | x*m01+y*m11+z*m21 | x*m02+y*m12+z*m22
    
    __asm fadd  dword ptr [eax+M30]            ; X' | x*m01+y*m11+z*m21 | x*m02+y*m12+z*m22
    __asm fxch  st(1)                          ; x*m01+y*m11+z*m21 | X' | x*m02+y*m12+z*m22
    __asm fadd  dword ptr [eax+M31]            ; Y' | X' | x*m02+y*m12+z*m22
    __asm fxch  st(2)                          ; x*m02+y*m12+z*m22 | X' | Y'
    __asm fadd  dword ptr [eax+M32]            ; Z' | X' | Y'

    __asm mov   dword ptr [edi+RES_W], 03f800000h

    __asm fxch  st(1)                          ; X' | Z' | Y'
    __asm fstp  dword ptr [edi+RES_X]          ; Z' | Y'
    __asm fstp  dword ptr [edi+RES_Z]          ; Y'
    __asm fstp  dword ptr [edi+RES_Y]          ; (empty)
}
#else
void __glXForm3_W(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
        + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
        + m->matrix[3][1];
    res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
        + m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}
#endif

/*
** Full 4x4 transformation.  The w column of the matrix is [0 0 0 1].
*/
#ifdef __GL_USE_INTEL_ASM
void __glXForm4_W(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m)
{
    const float *matrix = ( const float * ) m->matrix;

    if ( v[3] == ((__GLfloat) 1.0)) 
    {
        __asm mov   edi, res
        __asm mov   esi, v
        __asm mov   eax, matrix
        
        __asm fld   dword ptr [esi+V_X]            ; x
        __asm fmul  dword ptr [eax+M00]            ; x*m00
        __asm fld   dword ptr [esi+V_X]            ; x | x*m00
        __asm fmul  dword ptr [eax+M01]            ; x*m01 | x*m00
        __asm fld   dword ptr [esi+V_X]            ; x | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M02]            ; x*m02 | x*m01 | x*m00
        
        __asm fld   dword ptr [esi+V_Y]            ; y | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M10]            ; y*m10 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]            ; y | y*m10 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M11]            ; y*m11 | y*m10 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]            ; y | y*m11 | y*m10 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M12]            ; y*m12 | y*m11 | y*m10 | x*m02 | x*m01 | x*m00
        
        __asm fxch  st(5)                          ; x*m00 | y*m11 | y*m10 | x*m02 | x*m01 | y*m12
        __asm faddp st(2), st                      ; y*m11 | x*m00 + y*m10 | x*m02 | x*m01 | y*m12
        __asm faddp st(3), st                      ; x*m00 + y*m10 | x*m02 | x*m01+y*m11 | y*m12
        __asm fxch  st(1)                          ; x*m02 | x*m00 + y*m10 | x*m01+y*m11 | y*m12
        __asm faddp st(3), st                      ; x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
        
        __asm fld   dword ptr [esi+V_Z]            ; z | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
        __asm fmul  dword ptr [eax+M20]            ; z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
        __asm fld   dword ptr [esi+V_Z]            ; z | z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
        __asm fmul  dword ptr [eax+M21]            ; z*m21 | z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
        __asm fld   dword ptr [esi+V_Z]            ; z | z*m21 | z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
        __asm fmul  dword ptr [eax+M22]            ; z*m22 | z*m21 | z*m20 | x*m00 + y*m10 | x*m01+y*m11 | x*m02 + y*m12
        
        __asm fxch  st(2)                          ; z*m20 | z*m21 | z*m22 | x*m00+y*m10 | x*m01+y*m11 | x*m02+y*m12
        __asm faddp st(3), st                      ; z*m21 | z*m22 | x*m00+y*m10+z*z20 | x*m01+y*m11 | x*m02+y*m12
        __asm faddp st(3), st                      ; z*m22 | x*m00+y*m10+z*z20 | x*m01+y*m11+z*m21 | x*m02+y*m12
        __asm faddp st(3), st                      ; x*m00+y*m10+z*z20 | x*m01+y*m11+z*m21 | x*m02+y*m12+z*m22
        
        __asm fadd  dword ptr [eax+M30]            ; X' | x*m01+y*m11+z*m21 | x*m02+y*m12+z*m22
        __asm fxch  st(1)                          ; x*m01+y*m11+z*m21 | X' | x*m02+y*m12+z*m22
        __asm fadd  dword ptr [eax+M31]            ; Y' | X' | x*m02+y*m12+z*m22
        __asm fxch  st(2)                          ; x*m02+y*m12+z*m22 | X' | Y'
        __asm fadd  dword ptr [eax+M32]            ; Z' | X' | Y'
        
        __asm mov   dword ptr [edi+RES_W], 03f80000h
        
        __asm fxch  st(1)                          ; X' | Z' | Y'
        __asm fstp  dword ptr [edi+RES_X]          ; Z' | Y'
        __asm fstp  dword ptr [edi+RES_Z]          ; Y'
        __asm fstp  dword ptr [edi+RES_Y]          ; (empty)
    }
    else
    {
        __asm mov   edi, res
        __asm mov   esi, v
        __asm mov   eax, matrix
        
        __asm fld   dword ptr [esi+V_X]            ; x
        __asm fmul  dword ptr [eax+M00]            ; x*m00
        __asm fld   dword ptr [esi+V_X]            ; x | x*m00
        __asm fmul  dword ptr [eax+M01]            ; x*m01 | x*m00
        __asm fld   dword ptr [esi+V_X]            ; x | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M02]            ; x*m02 | x*m01 | x*m00

        __asm fld   dword ptr [esi+V_Y]            ; y | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M10]            ; y*m10 | x*m02 | x*m01 | x*m00 
        __asm fld   dword ptr [esi+V_Y]            ; y | y*m10 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M11]            ; y*m11 | y*m10 | x*m02 | x*m01 | x*m00
        __asm fld   dword ptr [esi+V_Y]            ; y | y*m11 | y*m10 | x*m02 | x*m01 | x*m00
        __asm fmul  dword ptr [eax+M12]            ; y*m12 | y*m11 | y*m10 | x*m02 | x*m01 | x*m00

        __asm fxch  st(2)                          ; y*m10 | y*m11 | y*m12 | x*m02 | x*m01 | x*m00
        __asm faddp st(5), st                      ; y*m11 | y*m12 | x*m02 | x*m01 | x*m00+y*m10
        __asm faddp st(3), st                      ; y*m12 | x*m02 | x*m01+y*m11 | x*m00+y*m10
        __asm faddp st(1), st                      ; x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10

        __asm fld   dword ptr [esi+V_Z]            ; z | x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10
        __asm fmul  dword ptr [eax+M20]            ; z*m20 | x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10
        __asm fld   dword ptr [esi+V_Z]            ; z | z*m20 | x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10
        __asm fmul  dword ptr [eax+M21]            ; z*m21 | z*m20 | x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10
        __asm fld   dword ptr [esi+V_Z]            ; z | z*m21 | z*m20 | x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10
        __asm fmul  dword ptr [eax+M22]            ; z*m22 | z*m21 | z*m20 | x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10

        __asm fxch  st(2)                          ; z*m20 | z*m21 | z*m22 | x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10
        __asm faddp st(5), st                      ; z*m21 | z*m22 | x*m02+y*m12 | x*m01+y*m11 | x*m00+y*m10+z*m20
        __asm faddp st(3), st                      ; z*m22 | x*m02+y*m12 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20
        __asm faddp st(1), st                      ; x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20

        __asm fld   dword ptr [esi+V_W]            ; w | x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20
        __asm fmul  dword ptr [eax+M30]            ; w*m30 | x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20
        __asm fld   dword ptr [esi+V_W]            ; w | w*m30 | x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20
        __asm fmul  dword ptr [eax+M31]            ; w*m31 | w*m30 | x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20
        __asm fld   dword ptr [esi+V_W]            ; w | w*m31 | w*m30 | x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20
        __asm fmul  dword ptr [eax+M32]            ; w*m32 | w*m31 | w*m30 | x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20

        __asm fxch  st(2)                          ; w*m30 | w*m31 | w*m32 | x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | x*m00+y*m10+z*m20
        __asm faddp st(5), st                      ; w*m31 | w*m32 | x*m02+y*m12+z*m22 | x*m01+y*m11+z*m21 | X'
        __asm faddp st(3), st                      ; w*m32 | x*m02+y*m12+z*m22 | Y' | X'
        __asm faddp st(1), st                      ; Z' | Y' | X'
        __asm fxch  st(2)                          ; X' | Y' | Z'

        __asm fstp  dword ptr [edi+RES_X]          ; Y' | Z'
        __asm fstp  dword ptr [edi+RES_Y]          ; Z'
        __asm fstp  dword ptr [edi+RES_Z]          ; (empty)
    }
    res->w = v[3];
}
#else
void __glXForm4_W(__GLcoord *res, const __GLfloat v[4], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];
  
    if (w == ((__GLfloat) 1.0)) {
        res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
            + m->matrix[3][0];
        res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
            + m->matrix[3][1];
        res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
            + m->matrix[3][2];
    } else {
        res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
            + w*m->matrix[3][0];
        res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
            + w*m->matrix[3][1];
        res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
            + w*m->matrix[3][2];
    }
    res->w = w;
}
#endif

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1.
**
** The matrix looks like:
** | . . 0 0 |
** | . . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void __glXForm2_2DW(__GLcoord *res, const __GLfloat v[2], 
                    const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
    res->z = m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.
**
** The matrix looks like:
** | . . 0 0 |
** | . . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void __glXForm3_2DW(__GLcoord *res, const __GLfloat v[3], 
                    const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
    res->z = z*m->matrix[2][2] + m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}

/*
** Full 4x4 transformation.
**
** The matrix looks like:
** | . . 0 0 |
** | . . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void __glXForm4_2DW(__GLcoord *res, const __GLfloat v[4], 
                    const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];

    if (w == ((__GLfloat) 1.0)) {
        res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
        res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
        res->z = z*m->matrix[2][2] + m->matrix[3][2];
    } else {
        res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + w*m->matrix[3][0];
        res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + w*m->matrix[3][1];
        res->z = z*m->matrix[2][2] + w*m->matrix[3][2];
    }
    res->w = w;
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has z=0 and w=1.
**
** The matrix looks like:
** | . 0 0 0 |
** | 0 . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void __glXForm2_2DNRW(__GLcoord *res, const __GLfloat v[2], 
                      const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];

    res->x = x*m->matrix[0][0] + m->matrix[3][0];
    res->y = y*m->matrix[1][1] + m->matrix[3][1];
    res->z = m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}

/*
** Avoid some transformation computations by knowing that the incoming
** vertex has w=1.
**
** The matrix looks like:
** | . 0 0 0 |
** | 0 . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void __glXForm3_2DNRW(__GLcoord *res, const __GLfloat v[3], 
                      const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

    res->x = x*m->matrix[0][0] + m->matrix[3][0];
    res->y = y*m->matrix[1][1] + m->matrix[3][1];
    res->z = z*m->matrix[2][2] + m->matrix[3][2];
    res->w = ((__GLfloat) 1.0);
}

/*
** Full 4x4 transformation.
**
** The matrix looks like:
** | . 0 0 0 |
** | 0 . 0 0 |
** | 0 0 . 0 |
** | . . . 1 |
*/
void __glXForm4_2DNRW(__GLcoord *res, const __GLfloat v[4], 
                      const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];
    __GLfloat w = v[3];

    if (w == ((__GLfloat) 1.0)) {
        res->x = x*m->matrix[0][0] + m->matrix[3][0];
        res->y = y*m->matrix[1][1] + m->matrix[3][1];
        res->z = z*m->matrix[2][2] + m->matrix[3][2];
    } else {
        res->x = x*m->matrix[0][0] + w*m->matrix[3][0];
        res->y = y*m->matrix[1][1] + w*m->matrix[3][1];
        res->z = z*m->matrix[2][2] + w*m->matrix[3][2];
    }
    res->w = w;
}
#endif /* !__GL_USE_MIPSASMCODE */

/************************************************************************/

/*
** Recompute the cached 2D matrix from the current mvp matrix and the viewport
** transformation.  This allows us to transform object coordinates directly
** to window coordinates.
*/
static void ReCompute2DMatrix(__GLcontext *gc, __GLmatrix *mvp)
{
    __GLviewport *vp;
    __GLmatrix *m;

    if (mvp->matrixType >= __GL_MT_IS2D) {
        m = &(gc->transform.matrix2D);
        vp = &(gc->state.viewport);
        m->matrix[0][0] = mvp->matrix[0][0] * vp->xScale;
        m->matrix[0][1] = mvp->matrix[0][1] * vp->yScale;
        m->matrix[1][0] = mvp->matrix[1][0] * vp->xScale;
        m->matrix[1][1] = mvp->matrix[1][1] * vp->yScale;
        m->matrix[2][2] = mvp->matrix[2][2];
        m->matrix[3][0] = mvp->matrix[3][0] * vp->xScale + vp->xCenter;
        m->matrix[3][1] = mvp->matrix[3][1] * vp->yScale + vp->yCenter;
        m->matrix[3][2] = mvp->matrix[3][2];
        m->matrix[3][3] = 1.0;
        m->matrixType = mvp->matrixType;
    }
}


/*
** A special picker for the mvp matrix which picks the mvp matrix, then
** calls the vertex picker, because the vertex picker depends upon the mvp 
** matrix.
*/
void __glGenericPickMvpMatrixProcs(__GLcontext *gc, __GLmatrix *m)
{
    __glPickMatrixType(gc, m,
        &gc->transform.modelView->matrix,
        &gc->transform.projection->matrix);
    ReCompute2DMatrix(gc, m);
    (*gc->procs.pickMatrixProcs)(gc, m);
    (*gc->procs.pickVertexProcs)(gc);
}

/* ARGSUSED */
void __glGenericPickMatrixProcs(__GLcontext *gc, __GLmatrix *m)
{
    switch(m->matrixType) {
      case __GL_MT_GENERAL:
        m->xf2 = __glXForm2;
        m->xf3 = __glXForm3;
        m->xf4 = __glXForm4;
        break;
      case __GL_MT_W0001:
        m->xf2 = __glXForm2_W;
        m->xf3 = __glXForm3_W;
        m->xf4 = __glXForm4_W;
        break;
      case __GL_MT_IS2D:
        m->xf2 = __glXForm2_2DW;
        m->xf3 = __glXForm3_2DW;
        m->xf4 = __glXForm4_2DW;
        break;
      case __GL_MT_IS2DNR:
      case __GL_MT_IS2DNRSC:
      case __GL_MT_IDENTITY:    /* probably never hit */
        m->xf2 = __glXForm2_2DNRW;
        m->xf3 = __glXForm3_2DNRW;
        m->xf4 = __glXForm4_2DNRW;
        break;
    }
}

/* ARGSUSED */
void __glGenericPickInvTransposeProcs(__GLcontext *gc, __GLmatrix *m)
{
    m->xf4 = __glXForm4;

    switch(m->matrixType) {
      case __GL_MT_GENERAL:
        m->xf3 = __glXForm4;
        break;
      case __GL_MT_W0001:
        m->xf3 = __glXForm3_W;
        break;
      case __GL_MT_IS2D:
        m->xf3 = __glXForm3_2DW;
        break;
      case __GL_MT_IS2DNR:
      case __GL_MT_IS2DNRSC:
      case __GL_MT_IDENTITY:    /* probably never hit */
        m->xf3 = __glXForm3_2DNRW;
        break;
    }
}

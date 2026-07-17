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
** $Revision: 4$
** $Date: 10/11/00 7:51:35 PM$
*/
#include "context.h"
#include "global.h"
#include "g_imfncs.h"
#include "vcache.h"

void APIENTRY __glim_Begin(GLenum mode)
{
    __GL_SETUP();
    GLuint beginMode;

    beginMode = __gl_beginMode;

    if (beginMode == __GL_IN_BEGIN) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if ((mode >= GL_TRIANGLES && mode <= GL_POLYGON) && (gc->slowPath == 0)) {
        // Fast path OK
        void __glSSTBegin(__GLcontext *, GLenum);
        void APIENTRY __glsstim_Vertex2fv_0(const GLfloat *);
        void APIENTRY __glsstim_Vertex3fv_0(const GLfloat *);
        void APIENTRY __glsstim_Vertex4fv_0(const GLfloat *);
        void APIENTRY __glsstim_Vertex3fv_0_I(const GLfloat *);
        void APIENTRY __glsstim_Vertex4fv_0_I(const GLfloat *);
        void APIENTRY __glsstim_Vertex2fv_1(const GLfloat *);
        void APIENTRY __glsstim_Vertex3fv_1(const GLfloat *);
        void APIENTRY __glsstim_Vertex4fv_1(const GLfloat *);
        void APIENTRY __glsstim_Vertex2fv_B(const GLfloat *);
        void APIENTRY __glsstim_Vertex3fv_B(const GLfloat *);
        void APIENTRY __glsstim_Vertex4fv_B(const GLfloat *);

        if (gc->validateTexture) {
            __GLtexture *tex;
            void __glSSTEnableTexturing(__GLcontext *);

            gc->texture.currentTexture[0] = 
            gc->texture.currentTexture[1] = NULL;

            if ( gc->state.enables.texture[0] & __GL_TEXTURE_2D_ENABLE ) {
                tex = __glLookUpTexture(gc, GL_TEXTURE_2D, 0 );
                gc->texture.currentTexture[0] = tex;
            }
            
            if ( gc->state.enables.texture[1] & __GL_TEXTURE_2D_ENABLE ) {
                tex = __glLookUpTexture(gc, GL_TEXTURE_2D, 1 );
                gc->texture.currentTexture[1] = tex;
            }
            
            __glSSTEnableTexturing(gc);
            gc->validateTexture = 0;
        }

        __gl_beginMode = __GL_IN_BEGIN;

        __glSSTBegin(gc, mode);

        if ( gc->texture.currentTexture[0] && gc->texture.currentTexture[1] ) {
            gc->dispatchState->vertex.Vertex2fv = __glsstim_Vertex2fv_B;
            gc->dispatchState->vertex.Vertex3fv = __glsstim_Vertex3fv_B;
            gc->dispatchState->vertex.Vertex4fv = __glsstim_Vertex4fv_B;
        } else if ( gc->texture.currentTexture[1] ) {
            gc->dispatchState->vertex.Vertex2fv = __glsstim_Vertex2fv_1;
            gc->dispatchState->vertex.Vertex3fv = __glsstim_Vertex3fv_1;
            gc->dispatchState->vertex.Vertex4fv = __glsstim_Vertex4fv_1;
        } else {
            gc->dispatchState->vertex.Vertex2fv = __glsstim_Vertex2fv_0;
	    if (gc->transform.modelView->mvp.matrixType >= __GL_MT_IS2DNR) {
                gc->dispatchState->vertex.Vertex3fv = __glsstim_Vertex3fv_0_I;
                gc->dispatchState->vertex.Vertex4fv = __glsstim_Vertex4fv_0_I;
	    } else {
                gc->dispatchState->vertex.Vertex3fv = __glsstim_Vertex3fv_0;
                gc->dispatchState->vertex.Vertex4fv = __glsstim_Vertex4fv_0;
	    }
        }
    } else if ((mode >= GL_TRIANGLES && mode <= GL_POLYGON) && (gc->slowPath == __GL_LIGHTING_SLOWPATH) &&
               (gc->procs.calcColor == __glFastCalcRGBColor) && !(gc->state.enables.general & (__GL_NORMALIZE_ENABLE|__GL_COLOR_MATERIAL_ENABLE)) &&
               (gc->transform.modelView->inverseTranspose.matrixType == __GL_MT_W0001) ) {
        __GLtransform *tr = gc->transform.modelView;

        // Fast path OK
        void __glSSTBegin(__GLcontext *, GLenum);
        void APIENTRY __glsstim_Vertex3fv_L(const GLfloat *);

        if (gc->validateTexture) {
            __GLtexture *tex;
            void __glSSTEnableTexturing(__GLcontext *);

            gc->texture.currentTexture[0] = 
            gc->texture.currentTexture[1] = NULL;

            if ( gc->state.enables.texture[0] & __GL_TEXTURE_2D_ENABLE ) {
                tex = __glLookUpTexture(gc, GL_TEXTURE_2D, 0 );
                gc->texture.currentTexture[0] = tex;
            }
            
            if ( gc->state.enables.texture[1] & __GL_TEXTURE_2D_ENABLE ) {
                tex = __glLookUpTexture(gc, GL_TEXTURE_2D, 1 );
                gc->texture.currentTexture[1] = tex;
            }
            
            __glSSTEnableTexturing(gc);
            gc->validateTexture = 0;
        }

        if (beginMode == __GL_NEED_VALIDATE) {
            (*gc->procs.validate)(gc);
        }
        __gl_beginMode = __GL_IN_BEGIN;


        if (tr->updateInverse) {
            (*gc->procs.computeInverseTranspose)(gc, tr);
        }

        __glSSTBegin(gc, mode);

        gc->dispatchState->vertex.Vertex2fv = __glim_Vertex2fv;
        gc->dispatchState->vertex.Vertex3fv = __glsstim_Vertex3fv_L;
        gc->dispatchState->vertex.Vertex4fv = __glim_Vertex4fv;
    } else if ( ( ( mode == GL_LINES ) || ( mode == GL_LINE_STRIP ) ) && ( gc->slowPath == 0 ) && ( gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE ) ) {
        // Fast path OK
        void __glSSTBegin(__GLcontext *, GLenum);
        void APIENTRY __glsstim_Vertex2fv_AALine(const GLfloat *);
        void APIENTRY __glsstim_Vertex3fv_AALine(const GLfloat *);
        void APIENTRY __glsstim_Vertex4fv_AALine(const GLfloat *);

        if (gc->validateTexture) {
            __GLtexture *tex;
            void __glSSTEnableTexturing(__GLcontext *);

            gc->texture.currentTexture[0] = 
            gc->texture.currentTexture[1] = NULL;

            if ( gc->state.enables.texture[0] & __GL_TEXTURE_2D_ENABLE ) {
                tex = __glLookUpTexture(gc, GL_TEXTURE_2D, 0 );
                gc->texture.currentTexture[0] = tex;
            }
            
            if ( gc->state.enables.texture[1] & __GL_TEXTURE_2D_ENABLE ) {
                tex = __glLookUpTexture(gc, GL_TEXTURE_2D, 1 );
                gc->texture.currentTexture[1] = tex;
            }
            
            __glSSTEnableTexturing(gc);
            gc->validateTexture = 0;
        }

        __gl_beginMode = __GL_IN_BEGIN;

        __glSSTBegin(gc, mode);

        gc->dispatchState->vertex.Vertex2fv = __glsstim_Vertex2fv_AALine;
        gc->dispatchState->vertex.Vertex3fv = __glsstim_Vertex3fv_AALine;
        gc->dispatchState->vertex.Vertex4fv = __glsstim_Vertex4fv_AALine;
    } else {
        // Old slow path
            if (beginMode == __GL_NEED_VALIDATE) {
                (*gc->procs.validate)(gc);
                __gl_beginMode = __GL_NOT_IN_BEGIN;
                glBegin(mode);
                return;
        }

        if ((GLuint)mode > GL_POLYGON) {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        __gl_beginMode = __GL_IN_BEGIN;
        __GL_API_BGN_RENDER();

        (*gc->procs.beginPrim[mode])(gc);
        gc->dispatchState->vertex.Vertex2fv = __glim_Vertex2fv;
        gc->dispatchState->vertex.Vertex3fv = __glim_Vertex3fv;
        gc->dispatchState->vertex.Vertex4fv = __glim_Vertex4fv;
    }
}

void APIENTRY __glim_End(void)
{
    __GL_SETUP();
    GLuint beginMode;

    beginMode = __gl_beginMode;
    if (beginMode == __GL_NOT_IN_BEGIN || beginMode == __GL_NEED_VALIDATE) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
    assert(beginMode == __GL_IN_BEGIN);
    __GL_API_END_RENDER();

    (*gc->procs.endPrim)(gc);

    __gl_beginMode = __GL_NOT_IN_BEGIN;
}

/************************************************************************/

void __glNop(void) { }

/*
** End a primitive that needs no special end processing
*/
void __glEndPrim(__GLcontext *gc)
{
    gc->procs.vertex = (void (*)(__GLcontext*, __GLvertex*)) __glNop;
    gc->procs.endPrim = __glEndPrim;
}


void APIENTRY __glim_UnimplementedExtension(void) 
{
    __glSetError(GL_INVALID_OPERATION);
    return;
}

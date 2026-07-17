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
** $Date: 10/11/00 7:56:40 PM$
*/
#include "context.h"
#include "global.h"
#include "g_imfncs.h"
#include "pixel.h"

void APIENTRY __glim_Rectd(GLdouble ax, GLdouble ay, GLdouble bx, GLdouble by)
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_NOTBE_RENDER();

    (*gc->procs.rect)(gc, ax, ay, bx, by);
}

void APIENTRY __glim_Rectdv(const GLdouble v1[2], const GLdouble v2[2])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_NOTBE_RENDER();

    (*gc->procs.rect)(gc, v1[0], v1[1], v2[0], v2[1]);
}

void APIENTRY __glim_Rectf(GLfloat ax, GLfloat ay, GLfloat bx, GLfloat by)
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_NOTBE_RENDER();

    (*gc->procs.rect)(gc, ax, ay, bx, by);
}

void APIENTRY __glim_Rectfv(const GLfloat v1[2], const GLfloat v2[2])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_NOTBE_RENDER();

    (*gc->procs.rect)(gc, v1[0], v1[1], v2[0], v2[1]);
}

void APIENTRY __glim_Recti(GLint ax, GLint ay, GLint bx, GLint by)
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_NOTBE_RENDER();

    (*gc->procs.rect)(gc, ax, ay, bx, by);
}

void APIENTRY __glim_Rectiv(const GLint v1[2], const GLint v2[2])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_NOTBE_RENDER();

    (*gc->procs.rect)(gc, v1[0], v1[1], v2[0], v2[1]);
}

void APIENTRY __glim_Rects(GLshort ax, GLshort ay, GLshort bx, GLshort by)
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_NOTBE_RENDER();

    (*gc->procs.rect)(gc, ax, ay, bx, by);
}

void APIENTRY __glim_Rectsv(const GLshort v1[2], const GLshort v2[2])
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_NOTBE_RENDER();

    (*gc->procs.rect)(gc, v1[0], v1[1], v2[0], v2[1]);
}

void __glRect(__GLcontext *gc, __GLfloat x0, __GLfloat y0, 
	      __GLfloat x1, __GLfloat y1)
{
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

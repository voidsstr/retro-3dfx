#include <stdio.h>
#include <windows.h>
#include <gl/gl.h>
#include "g_imfncs.h"
#include "g_disp.h"
#include "g_lcfncs.h"
#include "..\sst\sst_imfncs.h"
#include "dbfncs.h"
#include "db_trace.h"

#include "context.h"
#include "global.h"
void  APIENTRY __gldb_Accum (GLenum op,
                             GLfloat value)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Accum               (");
        fprintf(__gl_debug_log, "op:%x ", op);
        fprintf(__gl_debug_log, "value:%x ", value);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Accum)(op,value);

        __gl_debug_table.Accum++;
}

void  APIENTRY __gldb_AlphaFunc (GLenum func,
                                 GLclampf ref)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "AlphaFunc           (");
        fprintf(__gl_debug_log, "func:%x ", func);
        fprintf(__gl_debug_log, "ref:%x ", ref);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.AlphaFunc)(func,ref);

        __gl_debug_table.AlphaFunc++;
}

GLboolean  APIENTRY __gldb_AreTexturesResident (GLsizei n,
                                                const GLuint *textures,
                                                GLboolean *residences)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "AreTexturesResident (");
        fprintf(__gl_debug_log, "n:%x ", n);
        fprintf(__gl_debug_log, "textures:%x ", textures);
        fprintf(__gl_debug_log, "residences:%x ", residences);
        fprintf(__gl_debug_log, ")\n");
        }

        return
        (*__gl_real_immed->dispatch.AreTexturesResident)(n,textures,residences);

        __gl_debug_table.AreTexturesResident++;
}

void  APIENTRY __gldb_ArrayElement (GLint i)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ArrayElement        (");
        fprintf(__gl_debug_log, "i:%x ", i);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ArrayElement)(i);

        __gl_debug_table.ArrayElement++;
}

void  APIENTRY __gldb_Begin (GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Begin               (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Begin)(mode);

        __gl_debug_table.Begin++;
        __gldb_StatBegin(mode);
}

void  APIENTRY __gldb_BindTexture (GLenum target,
                                   GLuint texture)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "BindTexture         (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "texture:%x ", texture);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.BindTexture)(target,texture);

        __gl_debug_table.BindTexture++;
}

void  APIENTRY __gldb_Bitmap (GLsizei width,
                              GLsizei height,
                              GLfloat xorig,
                              GLfloat yorig,
                              GLfloat xmove,
                              GLfloat ymove,
                              const GLubyte *bitmap)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Bitmap              (");
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, "xorig:%x ", xorig);
        fprintf(__gl_debug_log, "yorig:%x ", yorig);
        fprintf(__gl_debug_log, "xmove:%x ", xmove);
        fprintf(__gl_debug_log, "ymove:%x ", ymove);
        fprintf(__gl_debug_log, "bitmap:%x ", bitmap);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Bitmap)(width,height,xorig,yorig,xmove,ymove,bitmap);

        __gl_debug_table.Bitmap++;
}

void  APIENTRY __gldb_BlendFunc (GLenum sfactor,
                                 GLenum dfactor)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "BlendFunc           (");
        fprintf(__gl_debug_log, "sfactor:%x ", sfactor);
        fprintf(__gl_debug_log, "dfactor:%x ", dfactor);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.BlendFunc)(sfactor,dfactor);

        __gl_debug_table.BlendFunc++;
}

void  APIENTRY __gldb_CallList (GLuint list)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "CallList            (");
        fprintf(__gl_debug_log, "list:%x ", list);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.CallList)(list);

        __gl_debug_table.CallList++;
}

void  APIENTRY __gldb_CallLists (GLsizei n,
                                 GLenum type,
                                 const GLvoid *lists)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "CallLists           (");
        fprintf(__gl_debug_log, "n:%x ", n);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "lists:%x ", lists);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.CallLists)(n,type,lists);

        __gl_debug_table.CallLists++;
}

void  APIENTRY __gldb_Clear (GLbitfield mask)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Clear               (");
        fprintf(__gl_debug_log, "mask:%x ", mask);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Clear)(mask);

        __gl_debug_table.Clear++;
}

void  APIENTRY __gldb_ClearAccum (GLfloat red,
                                  GLfloat green,
                                  GLfloat blue,
                                  GLfloat alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ClearAccum          (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ClearAccum)(red,green,blue,alpha);

        __gl_debug_table.ClearAccum++;
}

void  APIENTRY __gldb_ClearColor (GLclampf red,
                                  GLclampf green,
                                  GLclampf blue,
                                  GLclampf alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ClearColor          (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ClearColor)(red,green,blue,alpha);

        __gl_debug_table.ClearColor++;
}

void  APIENTRY __gldb_ClearDepth (GLclampd depth)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ClearDepth          (");
        fprintf(__gl_debug_log, "depth:%x ", depth);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ClearDepth)(depth);

        __gl_debug_table.ClearDepth++;
}

void  APIENTRY __gldb_ClearIndex (GLfloat c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ClearIndex          (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ClearIndex)(c);

        __gl_debug_table.ClearIndex++;
}

void  APIENTRY __gldb_ClearStencil (GLint s)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ClearStencil        (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ClearStencil)(s);

        __gl_debug_table.ClearStencil++;
}

void  APIENTRY __gldb_ClipPlane (GLenum plane,
                                 const GLdouble *equation)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ClipPlane           (");
        fprintf(__gl_debug_log, "plane:%x ", plane);
        fprintf(__gl_debug_log, "equation:%x ", equation);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ClipPlane)(plane,equation);

        __gl_debug_table.ClipPlane++;
}

void  APIENTRY __gldb_Color3b (GLbyte red,
                               GLbyte green,
                               GLbyte blue)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3b             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3b)(red,green,blue);

        __gl_debug_table.Color3b++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3bv (const GLbyte *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3bv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3bv)(v);

        __gl_debug_table.Color3bv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3d (GLdouble red,
                               GLdouble green,
                               GLdouble blue)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3d             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3d)(red,green,blue);

        __gl_debug_table.Color3d++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3dv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3dv)(v);

        __gl_debug_table.Color3dv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3f (GLfloat red,
                               GLfloat green,
                               GLfloat blue)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3f             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3f)(red,green,blue);

        __gl_debug_table.Color3f++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3fv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3fv)(v);

        __gl_debug_table.Color3fv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3i (GLint red,
                               GLint green,
                               GLint blue)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3i             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3i)(red,green,blue);

        __gl_debug_table.Color3i++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3iv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3iv)(v);

        __gl_debug_table.Color3iv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3s (GLshort red,
                               GLshort green,
                               GLshort blue)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3s             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3s)(red,green,blue);

        __gl_debug_table.Color3s++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3sv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3sv)(v);

        __gl_debug_table.Color3sv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3ub (GLubyte red,
                                GLubyte green,
                                GLubyte blue)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3ub            (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3ub)(red,green,blue);

        __gl_debug_table.Color3ub++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3ubv (const GLubyte *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3ubv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3ubv)(v);

        __gl_debug_table.Color3ubv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3ui (GLuint red,
                                GLuint green,
                                GLuint blue)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3ui            (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3ui)(red,green,blue);

        __gl_debug_table.Color3ui++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3uiv (const GLuint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3uiv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3uiv)(v);

        __gl_debug_table.Color3uiv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3us (GLushort red,
                                GLushort green,
                                GLushort blue)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3us            (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3us)(red,green,blue);

        __gl_debug_table.Color3us++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color3usv (const GLushort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color3usv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color3usv)(v);

        __gl_debug_table.Color3usv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4b (GLbyte red,
                               GLbyte green,
                               GLbyte blue,
                               GLbyte alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4b             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4b)(red,green,blue,alpha);

        __gl_debug_table.Color4b++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4bv (const GLbyte *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4bv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4bv)(v);

        __gl_debug_table.Color4bv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4d (GLdouble red,
                               GLdouble green,
                               GLdouble blue,
                               GLdouble alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4d             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4d)(red,green,blue,alpha);

        __gl_debug_table.Color4d++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4dv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4dv)(v);

        __gl_debug_table.Color4dv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4f (GLfloat red,
                               GLfloat green,
                               GLfloat blue,
                               GLfloat alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4f             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4f)(red,green,blue,alpha);

        __gl_debug_table.Color4f++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4fv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4fv)(v);

        __gl_debug_table.Color4fv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4i (GLint red,
                               GLint green,
                               GLint blue,
                               GLint alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4i             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4i)(red,green,blue,alpha);

        __gl_debug_table.Color4i++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4iv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4iv)(v);

        __gl_debug_table.Color4iv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4s (GLshort red,
                               GLshort green,
                               GLshort blue,
                               GLshort alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4s             (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4s)(red,green,blue,alpha);

        __gl_debug_table.Color4s++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4sv            (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4sv)(v);

        __gl_debug_table.Color4sv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4ub (GLubyte red,
                                GLubyte green,
                                GLubyte blue,
                                GLubyte alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4ub            (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4ub)(red,green,blue,alpha);

        __gl_debug_table.Color4ub++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4ubv (const GLubyte *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4ubv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4ubv)(v);

        __gl_debug_table.Color4ubv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4ui (GLuint red,
                                GLuint green,
                                GLuint blue,
                                GLuint alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4ui            (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4ui)(red,green,blue,alpha);

        __gl_debug_table.Color4ui++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4uiv (const GLuint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4uiv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4uiv)(v);

        __gl_debug_table.Color4uiv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4us (GLushort red,
                                GLushort green,
                                GLushort blue,
                                GLushort alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4us            (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4us)(red,green,blue,alpha);

        __gl_debug_table.Color4us++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_Color4usv (const GLushort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Color4usv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Color4usv)(v);

        __gl_debug_table.Color4usv++;
        __gl_debug_table.Color++;
}

void  APIENTRY __gldb_ColorMask (GLboolean red,
                                 GLboolean green,
                                 GLboolean blue,
                                 GLboolean alpha)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ColorMask           (");
        fprintf(__gl_debug_log, "red:%x ", red);
        fprintf(__gl_debug_log, "green:%x ", green);
        fprintf(__gl_debug_log, "blue:%x ", blue);
        fprintf(__gl_debug_log, "alpha:%x ", alpha);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ColorMask)(red,green,blue,alpha);

        __gl_debug_table.ColorMask++;
}

void  APIENTRY __gldb_ColorMaterial (GLenum face,
                                     GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ColorMaterial       (");
        fprintf(__gl_debug_log, "face:%x ", face);
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ColorMaterial)(face,mode);

        __gl_debug_table.ColorMaterial++;
}

void  APIENTRY __gldb_ColorPointer (GLint size,
                                    GLenum type,
                                    GLsizei stride,
                                    const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ColorPointer        (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ColorPointer)(size,type,stride,pointer);

        __gl_debug_table.ColorPointer++;
}

void  APIENTRY __gldb_CopyPixels (GLint x,
                                  GLint y,
                                  GLsizei width,
                                  GLsizei height,
                                  GLenum type)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "CopyPixels          (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.CopyPixels)(x,y,width,height,type);

        __gl_debug_table.CopyPixels++;
}

void  APIENTRY __gldb_CopyTexImage1D (GLenum target,
                                      GLint level,
                                      GLenum internalformat,
                                      GLint x,
                                      GLint y,
                                      GLsizei width,
                                      GLint border)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "CopyTexImage1D      (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "internalformat:%x ", internalformat);
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "border:%x ", border);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.CopyTexImage1D)(target,level,internalformat,x,y,width,border);

        __gl_debug_table.CopyTexImage1D++;
}

void  APIENTRY __gldb_CopyTexImage2D (GLenum target,
                                      GLint level,
                                      GLenum internalformat,
                                      GLint x,
                                      GLint y,
                                      GLsizei width,
                                      GLsizei height,
                                      GLint border)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "CopyTexImage2D      (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "internalformat:%x ", internalformat);
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, "border:%x ", border);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.CopyTexImage2D)(target,level,internalformat,x,y,width,height,border);

        __gl_debug_table.CopyTexImage2D++;
}

void  APIENTRY __gldb_CopyTexSubImage1D (GLenum target,
                                         GLint level,
                                         GLint xoffset,
                                         GLint x,
                                         GLint y,
                                         GLsizei width)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "CopyTexSubImage1D   (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "xoffset:%x ", xoffset);
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.CopyTexSubImage1D)(target,level,xoffset,x,y,width);

        __gl_debug_table.CopyTexSubImage1D++;
}

void  APIENTRY __gldb_CopyTexSubImage2D (GLenum target,
                                         GLint level,
                                         GLint xoffset,
                                         GLint yoffset,
                                         GLint x,
                                         GLint y,
                                         GLsizei width,
                                         GLsizei height)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "CopyTexSubImage2D   (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "xoffset:%x ", xoffset);
        fprintf(__gl_debug_log, "yoffset:%x ", yoffset);
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.CopyTexSubImage2D)(target,level,xoffset,yoffset,x,y,width,height);

        __gl_debug_table.CopyTexSubImage2D++;
}

void  APIENTRY __gldb_CullFace (GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "CullFace            (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.CullFace)(mode);

        __gl_debug_table.CullFace++;
}

void  APIENTRY __gldb_DeleteLists (GLuint list,
                                   GLsizei range)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DeleteLists         (");
        fprintf(__gl_debug_log, "list:%x ", list);
        fprintf(__gl_debug_log, "range:%x ", range);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DeleteLists)(list,range);

        __gl_debug_table.DeleteLists++;
}

void  APIENTRY __gldb_DeleteTextures (GLsizei n,
                                      const GLuint *textures)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DeleteTextures      (");
        fprintf(__gl_debug_log, "n:%x ", n);
        fprintf(__gl_debug_log, "textures:%x ", textures);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DeleteTextures)(n,textures);

        __gl_debug_table.DeleteTextures++;
}

void  APIENTRY __gldb_DepthFunc (GLenum func)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DepthFunc           (");
        fprintf(__gl_debug_log, "func:%x ", func);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DepthFunc)(func);

        __gl_debug_table.DepthFunc++;
}

void  APIENTRY __gldb_DepthMask (GLboolean flag)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DepthMask           (");
        fprintf(__gl_debug_log, "flag:%x ", flag);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DepthMask)(flag);

        __gl_debug_table.DepthMask++;
}

void  APIENTRY __gldb_DepthRange (GLclampd zNear,
                                  GLclampd zFar)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DepthRange          (");
        fprintf(__gl_debug_log, "zNear:%x ", zNear);
        fprintf(__gl_debug_log, "zFar:%x ", zFar);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DepthRange)(zNear,zFar);

        __gl_debug_table.DepthRange++;
}

void  APIENTRY __gldb_Disable (GLenum cap)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Disable             (");
        fprintf(__gl_debug_log, "cap:%x ", cap);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Disable)(cap);

        __gl_debug_table.Disable++;
}

void  APIENTRY __gldb_DisableClientState (GLenum array)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DisableClientState  (");
        fprintf(__gl_debug_log, "array:%x ", array);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DisableClientState)(array);

        __gl_debug_table.DisableClientState++;
}

void  APIENTRY __gldb_DrawArrays (GLenum mode,
                                  GLint first,
                                  GLsizei count)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DrawArrays          (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, "first:%x ", first);
        fprintf(__gl_debug_log, "count:%x ", count);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DrawArrays)(mode,first,count);

        __gl_debug_table.DrawArrays++;
}

void  APIENTRY __gldb_DrawBuffer (GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DrawBuffer          (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DrawBuffer)(mode);

        __gl_debug_table.DrawBuffer++;
}

void  APIENTRY __gldb_DrawElements (GLenum mode,
                                    GLsizei count,
                                    GLenum type,
                                    const GLvoid *indices)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DrawElements        (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, "count:%x ", count);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "indices:%x ", indices);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DrawElements)(mode,count,type,indices);

        __gl_debug_table.DrawElements++;
}

void  APIENTRY __gldb_DrawPixels (GLsizei width,
                                  GLsizei height,
                                  GLenum format,
                                  GLenum type,
                                  const GLvoid *pixels)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "DrawPixels          (");
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, "format:%x ", format);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "pixels:%x ", pixels);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.DrawPixels)(width,height,format,type,pixels);

        __gl_debug_table.DrawPixels++;
}

void  APIENTRY __gldb_EdgeFlag (GLboolean flag)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EdgeFlag            (");
        fprintf(__gl_debug_log, "flag:%x ", flag);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EdgeFlag)(flag);

        __gl_debug_table.EdgeFlag++;
}

void  APIENTRY __gldb_EdgeFlagPointer (GLsizei stride,
                                       const GLboolean *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EdgeFlagPointer     (");
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EdgeFlagPointer)(stride,pointer);

        __gl_debug_table.EdgeFlagPointer++;
}

void  APIENTRY __gldb_EdgeFlagv (const GLboolean *flag)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EdgeFlagv           (");
        fprintf(__gl_debug_log, "flag:%x ", flag);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EdgeFlagv)(flag);

        __gl_debug_table.EdgeFlagv++;
}

void  APIENTRY __gldb_Enable (GLenum cap)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Enable              (");
        fprintf(__gl_debug_log, "cap:%x ", cap);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Enable)(cap);

        __gl_debug_table.Enable++;
}

void  APIENTRY __gldb_EnableClientState (GLenum array)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EnableClientState   (");
        fprintf(__gl_debug_log, "array:%x ", array);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EnableClientState)(array);

        __gl_debug_table.EnableClientState++;
}

void  APIENTRY __gldb_End (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "End                 (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.End)();

        __gl_debug_table.End++;
        __gldb_StatEnd();
}

void  APIENTRY __gldb_EndList (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EndList             (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EndList)();

        __gl_debug_table.EndList++;
}

void  APIENTRY __gldb_EvalCoord1d (GLdouble u)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalCoord1d         (");
        fprintf(__gl_debug_log, "u:%x ", u);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalCoord1d)(u);

        __gl_debug_table.EvalCoord1d++;
}

void  APIENTRY __gldb_EvalCoord1dv (const GLdouble *u)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalCoord1dv        (");
        fprintf(__gl_debug_log, "u:%x ", u);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalCoord1dv)(u);

        __gl_debug_table.EvalCoord1dv++;
}

void  APIENTRY __gldb_EvalCoord1f (GLfloat u)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalCoord1f         (");
        fprintf(__gl_debug_log, "u:%x ", u);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalCoord1f)(u);

        __gl_debug_table.EvalCoord1f++;
}

void  APIENTRY __gldb_EvalCoord1fv (const GLfloat *u)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalCoord1fv        (");
        fprintf(__gl_debug_log, "u:%x ", u);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalCoord1fv)(u);

        __gl_debug_table.EvalCoord1fv++;
}

void  APIENTRY __gldb_EvalCoord2d (GLdouble u,
                                   GLdouble v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalCoord2d         (");
        fprintf(__gl_debug_log, "u:%x ", u);
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalCoord2d)(u,v);

        __gl_debug_table.EvalCoord2d++;
}

void  APIENTRY __gldb_EvalCoord2dv (const GLdouble *u)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalCoord2dv        (");
        fprintf(__gl_debug_log, "u:%x ", u);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalCoord2dv)(u);

        __gl_debug_table.EvalCoord2dv++;
}

void  APIENTRY __gldb_EvalCoord2f (GLfloat u,
                                   GLfloat v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalCoord2f         (");
        fprintf(__gl_debug_log, "u:%x ", u);
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalCoord2f)(u,v);

        __gl_debug_table.EvalCoord2f++;
}

void  APIENTRY __gldb_EvalCoord2fv (const GLfloat *u)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalCoord2fv        (");
        fprintf(__gl_debug_log, "u:%x ", u);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalCoord2fv)(u);

        __gl_debug_table.EvalCoord2fv++;
}

void  APIENTRY __gldb_EvalMesh1 (GLenum mode,
                                 GLint i1,
                                 GLint i2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalMesh1           (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, "i1:%x ", i1);
        fprintf(__gl_debug_log, "i2:%x ", i2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalMesh1)(mode,i1,i2);

        __gl_debug_table.EvalMesh1++;
}

void  APIENTRY __gldb_EvalMesh2 (GLenum mode,
                                 GLint i1,
                                 GLint i2,
                                 GLint j1,
                                 GLint j2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalMesh2           (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, "i1:%x ", i1);
        fprintf(__gl_debug_log, "i2:%x ", i2);
        fprintf(__gl_debug_log, "j1:%x ", j1);
        fprintf(__gl_debug_log, "j2:%x ", j2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalMesh2)(mode,i1,i2,j1,j2);

        __gl_debug_table.EvalMesh2++;
}

void  APIENTRY __gldb_EvalPoint1 (GLint i)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalPoint1          (");
        fprintf(__gl_debug_log, "i:%x ", i);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalPoint1)(i);

        __gl_debug_table.EvalPoint1++;
}

void  APIENTRY __gldb_EvalPoint2 (GLint i,
                                  GLint j)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EvalPoint2          (");
        fprintf(__gl_debug_log, "i:%x ", i);
        fprintf(__gl_debug_log, "j:%x ", j);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EvalPoint2)(i,j);

        __gl_debug_table.EvalPoint2++;
}

void  APIENTRY __gldb_FeedbackBuffer (GLsizei size,
                                      GLenum type,
                                      GLfloat *buffer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "FeedbackBuffer      (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "buffer:%x ", buffer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.FeedbackBuffer)(size,type,buffer);

        __gl_debug_table.FeedbackBuffer++;
}

void  APIENTRY __gldb_Finish (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Finish              (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Finish)();

        __gl_debug_table.Finish++;
}

void  APIENTRY __gldb_Flush (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Flush               (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Flush)();

        __gl_debug_table.Flush++;
}

void  APIENTRY __gldb_Fogf (GLenum pname,
                            GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Fogf                (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Fogf)(pname,param);

        __gl_debug_table.Fogf++;
}

void  APIENTRY __gldb_Fogfv (GLenum pname,
                             const GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Fogfv               (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Fogfv)(pname,params);

        __gl_debug_table.Fogfv++;
}

void  APIENTRY __gldb_Fogi (GLenum pname,
                            GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Fogi                (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Fogi)(pname,param);

        __gl_debug_table.Fogi++;
}

void  APIENTRY __gldb_Fogiv (GLenum pname,
                             const GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Fogiv               (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Fogiv)(pname,params);

        __gl_debug_table.Fogiv++;
}

void  APIENTRY __gldb_FrontFace (GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "FrontFace           (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.FrontFace)(mode);

        __gl_debug_table.FrontFace++;
}

void  APIENTRY __gldb_Frustum (GLdouble left,
                               GLdouble right,
                               GLdouble bottom,
                               GLdouble top,
                               GLdouble zNear,
                               GLdouble zFar)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Frustum             (");
        fprintf(__gl_debug_log, "left:%x ", left);
        fprintf(__gl_debug_log, "right:%x ", right);
        fprintf(__gl_debug_log, "bottom:%x ", bottom);
        fprintf(__gl_debug_log, "top:%x ", top);
        fprintf(__gl_debug_log, "zNear:%x ", zNear);
        fprintf(__gl_debug_log, "zFar:%x ", zFar);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Frustum)(left,right,bottom,top,zNear,zFar);

        __gl_debug_table.Frustum++;
}

GLuint  APIENTRY __gldb_GenLists (GLsizei range)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GenLists            (");
        fprintf(__gl_debug_log, "range:%x ", range);
        fprintf(__gl_debug_log, ")\n");
        }

        return
        (*__gl_real_immed->dispatch.GenLists)(range);

        __gl_debug_table.GenLists++;
}

void  APIENTRY __gldb_GenTextures (GLsizei n,
                                   GLuint *textures)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GenTextures         (");
        fprintf(__gl_debug_log, "n:%x ", n);
        fprintf(__gl_debug_log, "textures:%x ", textures);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GenTextures)(n,textures);

        __gl_debug_table.GenTextures++;
}

void  APIENTRY __gldb_GetBooleanv (GLenum pname,
                                   GLboolean *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetBooleanv         (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetBooleanv)(pname,params);

        __gl_debug_table.GetBooleanv++;
}

void  APIENTRY __gldb_GetClipPlane (GLenum plane,
                                    GLdouble *equation)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetClipPlane        (");
        fprintf(__gl_debug_log, "plane:%x ", plane);
        fprintf(__gl_debug_log, "equation:%x ", equation);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetClipPlane)(plane,equation);

        __gl_debug_table.GetClipPlane++;
}

void  APIENTRY __gldb_GetDoublev (GLenum pname,
                                  GLdouble *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetDoublev          (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetDoublev)(pname,params);

        __gl_debug_table.GetDoublev++;
}

GLenum  APIENTRY __gldb_GetError (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetError            (");
        fprintf(__gl_debug_log, ")\n");
        }

        return
        (*__gl_real_immed->dispatch.GetError)();

        __gl_debug_table.GetError++;
}

void  APIENTRY __gldb_GetFloatv (GLenum pname,
                                 GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetFloatv           (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetFloatv)(pname,params);

        __gl_debug_table.GetFloatv++;
}

void  APIENTRY __gldb_GetIntegerv (GLenum pname,
                                   GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetIntegerv         (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetIntegerv)(pname,params);

        __gl_debug_table.GetIntegerv++;
}

void  APIENTRY __gldb_GetLightfv (GLenum light,
                                  GLenum pname,
                                  GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetLightfv          (");
        fprintf(__gl_debug_log, "light:%x ", light);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetLightfv)(light,pname,params);

        __gl_debug_table.GetLightfv++;
}

void  APIENTRY __gldb_GetLightiv (GLenum light,
                                  GLenum pname,
                                  GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetLightiv          (");
        fprintf(__gl_debug_log, "light:%x ", light);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetLightiv)(light,pname,params);

        __gl_debug_table.GetLightiv++;
}

void  APIENTRY __gldb_GetMapdv (GLenum target,
                                GLenum query,
                                GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetMapdv            (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "query:%x ", query);
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetMapdv)(target,query,v);

        __gl_debug_table.GetMapdv++;
}

void  APIENTRY __gldb_GetMapfv (GLenum target,
                                GLenum query,
                                GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetMapfv            (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "query:%x ", query);
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetMapfv)(target,query,v);

        __gl_debug_table.GetMapfv++;
}

void  APIENTRY __gldb_GetMapiv (GLenum target,
                                GLenum query,
                                GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetMapiv            (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "query:%x ", query);
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetMapiv)(target,query,v);

        __gl_debug_table.GetMapiv++;
}

void  APIENTRY __gldb_GetMaterialfv (GLenum face,
                                     GLenum pname,
                                     GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetMaterialfv       (");
        fprintf(__gl_debug_log, "face:%x ", face);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetMaterialfv)(face,pname,params);

        __gl_debug_table.GetMaterialfv++;
}

void  APIENTRY __gldb_GetMaterialiv (GLenum face,
                                     GLenum pname,
                                     GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetMaterialiv       (");
        fprintf(__gl_debug_log, "face:%x ", face);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetMaterialiv)(face,pname,params);

        __gl_debug_table.GetMaterialiv++;
}

void  APIENTRY __gldb_GetPixelMapfv (GLenum map,
                                     GLfloat *values)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetPixelMapfv       (");
        fprintf(__gl_debug_log, "map:%x ", map);
        fprintf(__gl_debug_log, "values:%x ", values);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetPixelMapfv)(map,values);

        __gl_debug_table.GetPixelMapfv++;
}

void  APIENTRY __gldb_GetPixelMapuiv (GLenum map,
                                      GLuint *values)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetPixelMapuiv      (");
        fprintf(__gl_debug_log, "map:%x ", map);
        fprintf(__gl_debug_log, "values:%x ", values);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetPixelMapuiv)(map,values);

        __gl_debug_table.GetPixelMapuiv++;
}

void  APIENTRY __gldb_GetPixelMapusv (GLenum map,
                                      GLushort *values)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetPixelMapusv      (");
        fprintf(__gl_debug_log, "map:%x ", map);
        fprintf(__gl_debug_log, "values:%x ", values);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetPixelMapusv)(map,values);

        __gl_debug_table.GetPixelMapusv++;
}

void  APIENTRY __gldb_GetPointerv (GLenum pname,
                                   GLvoid* *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetPointerv         (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetPointerv)(pname,params);

        __gl_debug_table.GetPointerv++;
}

void  APIENTRY __gldb_GetPolygonStipple (GLubyte *mask)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetPolygonStipple   (");
        fprintf(__gl_debug_log, "mask:%x ", mask);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetPolygonStipple)(mask);

        __gl_debug_table.GetPolygonStipple++;
}

const GLubyte * APIENTRY __gldb_GetString (GLenum name)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetString           (");
        fprintf(__gl_debug_log, "name:%x ", name);
        fprintf(__gl_debug_log, ")\n");
        }

        return
        (*__gl_real_immed->dispatch.GetString)(name);

        __gl_debug_table.GetString++;
}

void  APIENTRY __gldb_GetTexEnvfv (GLenum target,
                                   GLenum pname,
                                   GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexEnvfv         (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexEnvfv)(target,pname,params);

        __gl_debug_table.GetTexEnvfv++;
}

void  APIENTRY __gldb_GetTexEnviv (GLenum target,
                                   GLenum pname,
                                   GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexEnviv         (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexEnviv)(target,pname,params);

        __gl_debug_table.GetTexEnviv++;
}

void  APIENTRY __gldb_GetTexGendv (GLenum coord,
                                   GLenum pname,
                                   GLdouble *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexGendv         (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexGendv)(coord,pname,params);

        __gl_debug_table.GetTexGendv++;
}

void  APIENTRY __gldb_GetTexGenfv (GLenum coord,
                                   GLenum pname,
                                   GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexGenfv         (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexGenfv)(coord,pname,params);

        __gl_debug_table.GetTexGenfv++;
}

void  APIENTRY __gldb_GetTexGeniv (GLenum coord,
                                   GLenum pname,
                                   GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexGeniv         (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexGeniv)(coord,pname,params);

        __gl_debug_table.GetTexGeniv++;
}

void  APIENTRY __gldb_GetTexImage (GLenum target,
                                   GLint level,
                                   GLenum format,
                                   GLenum type,
                                   GLvoid *pixels)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexImage         (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "format:%x ", format);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "pixels:%x ", pixels);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexImage)(target,level,format,type,pixels);

        __gl_debug_table.GetTexImage++;
}

void  APIENTRY __gldb_GetTexLevelParameterfv (GLenum target,
                                              GLint level,
                                              GLenum pname,
                                              GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexLevelParameterfv(");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexLevelParameterfv)(target,level,pname,params);

        __gl_debug_table.GetTexLevelParameterfv++;
}

void  APIENTRY __gldb_GetTexLevelParameteriv (GLenum target,
                                              GLint level,
                                              GLenum pname,
                                              GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexLevelParameteriv(");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexLevelParameteriv)(target,level,pname,params);

        __gl_debug_table.GetTexLevelParameteriv++;
}

void  APIENTRY __gldb_GetTexParameterfv (GLenum target,
                                         GLenum pname,
                                         GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexParameterfv   (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexParameterfv)(target,pname,params);

        __gl_debug_table.GetTexParameterfv++;
}

void  APIENTRY __gldb_GetTexParameteriv (GLenum target,
                                         GLenum pname,
                                         GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "GetTexParameteriv   (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.GetTexParameteriv)(target,pname,params);

        __gl_debug_table.GetTexParameteriv++;
}

void  APIENTRY __gldb_Hint (GLenum target,
                            GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Hint                (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Hint)(target,mode);

        __gl_debug_table.Hint++;
}

void  APIENTRY __gldb_IndexMask (GLuint mask)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "IndexMask           (");
        fprintf(__gl_debug_log, "mask:%x ", mask);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.IndexMask)(mask);

        __gl_debug_table.IndexMask++;
}

void  APIENTRY __gldb_IndexPointer (GLenum type,
                                    GLsizei stride,
                                    const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "IndexPointer        (");
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.IndexPointer)(type,stride,pointer);

        __gl_debug_table.IndexPointer++;
}

void  APIENTRY __gldb_Indexd (GLdouble c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexd              (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Indexd)(c);

        __gl_debug_table.Indexd++;
}

void  APIENTRY __gldb_Indexdv (const GLdouble *c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexdv             (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Indexdv)(c);

        __gl_debug_table.Indexdv++;
}

void  APIENTRY __gldb_Indexf (GLfloat c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexf              (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Indexf)(c);

        __gl_debug_table.Indexf++;
}

void  APIENTRY __gldb_Indexfv (const GLfloat *c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexfv             (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Indexfv)(c);

        __gl_debug_table.Indexfv++;
}

void  APIENTRY __gldb_Indexi (GLint c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexi              (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Indexi)(c);

        __gl_debug_table.Indexi++;
}

void  APIENTRY __gldb_Indexiv (const GLint *c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexiv             (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Indexiv)(c);

        __gl_debug_table.Indexiv++;
}

void  APIENTRY __gldb_Indexs (GLshort c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexs              (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Indexs)(c);

        __gl_debug_table.Indexs++;
}

void  APIENTRY __gldb_Indexsv (const GLshort *c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexsv             (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->color.Indexsv)(c);

        __gl_debug_table.Indexsv++;
}

void  APIENTRY __gldb_Indexub (GLubyte c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexub             (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Indexub)(c);

        __gl_debug_table.Indexub++;
}

void  APIENTRY __gldb_Indexubv (const GLubyte *c)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Indexubv            (");
        fprintf(__gl_debug_log, "c:%x ", c);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Indexubv)(c);

        __gl_debug_table.Indexubv++;
}

void  APIENTRY __gldb_InitNames (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "InitNames           (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.InitNames)();

        __gl_debug_table.InitNames++;
}

void  APIENTRY __gldb_InterleavedArrays (GLenum format,
                                         GLsizei stride,
                                         const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "InterleavedArrays   (");
        fprintf(__gl_debug_log, "format:%x ", format);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.InterleavedArrays)(format,stride,pointer);

        __gl_debug_table.InterleavedArrays++;
}

GLboolean  APIENTRY __gldb_IsEnabled (GLenum cap)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "IsEnabled           (");
        fprintf(__gl_debug_log, "cap:%x ", cap);
        fprintf(__gl_debug_log, ")\n");
        }

        return
        (*__gl_real_immed->dispatch.IsEnabled)(cap);

        __gl_debug_table.IsEnabled++;
}

GLboolean  APIENTRY __gldb_IsList (GLuint list)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "IsList              (");
        fprintf(__gl_debug_log, "list:%x ", list);
        fprintf(__gl_debug_log, ")\n");
        }

        return
        (*__gl_real_immed->dispatch.IsList)(list);

        __gl_debug_table.IsList++;
}

GLboolean  APIENTRY __gldb_IsTexture (GLuint texture)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "IsTexture           (");
        fprintf(__gl_debug_log, "texture:%x ", texture);
        fprintf(__gl_debug_log, ")\n");
        }

        return
        (*__gl_real_immed->dispatch.IsTexture)(texture);

        __gl_debug_table.IsTexture++;
}

void  APIENTRY __gldb_LightModelf (GLenum pname,
                                   GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LightModelf         (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LightModelf)(pname,param);

        __gl_debug_table.LightModelf++;
}

void  APIENTRY __gldb_LightModelfv (GLenum pname,
                                    const GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LightModelfv        (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LightModelfv)(pname,params);

        __gl_debug_table.LightModelfv++;
}

void  APIENTRY __gldb_LightModeli (GLenum pname,
                                   GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LightModeli         (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LightModeli)(pname,param);

        __gl_debug_table.LightModeli++;
}

void  APIENTRY __gldb_LightModeliv (GLenum pname,
                                    const GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LightModeliv        (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LightModeliv)(pname,params);

        __gl_debug_table.LightModeliv++;
}

void  APIENTRY __gldb_Lightf (GLenum light,
                              GLenum pname,
                              GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Lightf              (");
        fprintf(__gl_debug_log, "light:%x ", light);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Lightf)(light,pname,param);

        __gl_debug_table.Lightf++;
}

void  APIENTRY __gldb_Lightfv (GLenum light,
                               GLenum pname,
                               const GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Lightfv             (");
        fprintf(__gl_debug_log, "light:%x ", light);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Lightfv)(light,pname,params);

        __gl_debug_table.Lightfv++;
}

void  APIENTRY __gldb_Lighti (GLenum light,
                              GLenum pname,
                              GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Lighti              (");
        fprintf(__gl_debug_log, "light:%x ", light);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Lighti)(light,pname,param);

        __gl_debug_table.Lighti++;
}

void  APIENTRY __gldb_Lightiv (GLenum light,
                               GLenum pname,
                               const GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Lightiv             (");
        fprintf(__gl_debug_log, "light:%x ", light);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Lightiv)(light,pname,params);

        __gl_debug_table.Lightiv++;
}

void  APIENTRY __gldb_LineStipple (GLint factor,
                                   GLushort pattern)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LineStipple         (");
        fprintf(__gl_debug_log, "factor:%x ", factor);
        fprintf(__gl_debug_log, "pattern:%x ", pattern);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LineStipple)(factor,pattern);

        __gl_debug_table.LineStipple++;
}

void  APIENTRY __gldb_LineWidth (GLfloat width)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LineWidth           (");
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LineWidth)(width);

        __gl_debug_table.LineWidth++;
}

void  APIENTRY __gldb_ListBase (GLuint base)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ListBase            (");
        fprintf(__gl_debug_log, "base:%x ", base);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ListBase)(base);

        __gl_debug_table.ListBase++;
}

void  APIENTRY __gldb_LoadIdentity (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LoadIdentity        (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LoadIdentity)();

        __gl_debug_table.LoadIdentity++;
}

void  APIENTRY __gldb_LoadMatrixd (const GLdouble *m)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LoadMatrixd         (");
        fprintf(__gl_debug_log, "m:%x ", m);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LoadMatrixd)(m);

        __gl_debug_table.LoadMatrixd++;
}

void  APIENTRY __gldb_LoadMatrixf (const GLfloat *m)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LoadMatrixf         (");
        fprintf(__gl_debug_log, "m:%x ", m);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LoadMatrixf)(m);

        __gl_debug_table.LoadMatrixf++;
}

void  APIENTRY __gldb_LoadName (GLuint name)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LoadName            (");
        fprintf(__gl_debug_log, "name:%x ", name);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LoadName)(name);

        __gl_debug_table.LoadName++;
}

void  APIENTRY __gldb_LogicOp (GLenum opcode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "LogicOp             (");
        fprintf(__gl_debug_log, "opcode:%x ", opcode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.LogicOp)(opcode);

        __gl_debug_table.LogicOp++;
}

void  APIENTRY __gldb_Map1d (GLenum target,
                             GLdouble u1,
                             GLdouble u2,
                             GLint stride,
                             GLint order,
                             const GLdouble *points)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Map1d               (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "u1:%x ", u1);
        fprintf(__gl_debug_log, "u2:%x ", u2);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "order:%x ", order);
        fprintf(__gl_debug_log, "points:%x ", points);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Map1d)(target,u1,u2,stride,order,points);

        __gl_debug_table.Map1d++;
}

void  APIENTRY __gldb_Map1f (GLenum target,
                             GLfloat u1,
                             GLfloat u2,
                             GLint stride,
                             GLint order,
                             const GLfloat *points)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Map1f               (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "u1:%x ", u1);
        fprintf(__gl_debug_log, "u2:%x ", u2);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "order:%x ", order);
        fprintf(__gl_debug_log, "points:%x ", points);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Map1f)(target,u1,u2,stride,order,points);

        __gl_debug_table.Map1f++;
}

void  APIENTRY __gldb_Map2d (GLenum target,
                             GLdouble u1,
                             GLdouble u2,
                             GLint ustride,
                             GLint uorder,
                             GLdouble v1,
                             GLdouble v2,
                             GLint vstride,
                             GLint vorder,
                             const GLdouble *points)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Map2d               (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "u1:%x ", u1);
        fprintf(__gl_debug_log, "u2:%x ", u2);
        fprintf(__gl_debug_log, "ustride:%x ", ustride);
        fprintf(__gl_debug_log, "uorder:%x ", uorder);
        fprintf(__gl_debug_log, "v1:%x ", v1);
        fprintf(__gl_debug_log, "v2:%x ", v2);
        fprintf(__gl_debug_log, "vstride:%x ", vstride);
        fprintf(__gl_debug_log, "vorder:%x ", vorder);
        fprintf(__gl_debug_log, "points:%x ", points);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Map2d)(target,u1,u2,ustride,uorder,v1,v2,vstride,vorder,points);

        __gl_debug_table.Map2d++;
}

void  APIENTRY __gldb_Map2f (GLenum target,
                             GLfloat u1,
                             GLfloat u2,
                             GLint ustride,
                             GLint uorder,
                             GLfloat v1,
                             GLfloat v2,
                             GLint vstride,
                             GLint vorder,
                             const GLfloat *points)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Map2f               (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "u1:%x ", u1);
        fprintf(__gl_debug_log, "u2:%x ", u2);
        fprintf(__gl_debug_log, "ustride:%x ", ustride);
        fprintf(__gl_debug_log, "uorder:%x ", uorder);
        fprintf(__gl_debug_log, "v1:%x ", v1);
        fprintf(__gl_debug_log, "v2:%x ", v2);
        fprintf(__gl_debug_log, "vstride:%x ", vstride);
        fprintf(__gl_debug_log, "vorder:%x ", vorder);
        fprintf(__gl_debug_log, "points:%x ", points);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Map2f)(target,u1,u2,ustride,uorder,v1,v2,vstride,vorder,points);

        __gl_debug_table.Map2f++;
}

void  APIENTRY __gldb_MapGrid1d (GLint un,
                                 GLdouble u1,
                                 GLdouble u2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "MapGrid1d           (");
        fprintf(__gl_debug_log, "un:%x ", un);
        fprintf(__gl_debug_log, "u1:%x ", u1);
        fprintf(__gl_debug_log, "u2:%x ", u2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.MapGrid1d)(un,u1,u2);

        __gl_debug_table.MapGrid1d++;
}

void  APIENTRY __gldb_MapGrid1f (GLint un,
                                 GLfloat u1,
                                 GLfloat u2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "MapGrid1f           (");
        fprintf(__gl_debug_log, "un:%x ", un);
        fprintf(__gl_debug_log, "u1:%x ", u1);
        fprintf(__gl_debug_log, "u2:%x ", u2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.MapGrid1f)(un,u1,u2);

        __gl_debug_table.MapGrid1f++;
}

void  APIENTRY __gldb_MapGrid2d (GLint un,
                                 GLdouble u1,
                                 GLdouble u2,
                                 GLint vn,
                                 GLdouble v1,
                                 GLdouble v2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "MapGrid2d           (");
        fprintf(__gl_debug_log, "un:%x ", un);
        fprintf(__gl_debug_log, "u1:%x ", u1);
        fprintf(__gl_debug_log, "u2:%x ", u2);
        fprintf(__gl_debug_log, "vn:%x ", vn);
        fprintf(__gl_debug_log, "v1:%x ", v1);
        fprintf(__gl_debug_log, "v2:%x ", v2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.MapGrid2d)(un,u1,u2,vn,v1,v2);

        __gl_debug_table.MapGrid2d++;
}

void  APIENTRY __gldb_MapGrid2f (GLint un,
                                 GLfloat u1,
                                 GLfloat u2,
                                 GLint vn,
                                 GLfloat v1,
                                 GLfloat v2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "MapGrid2f           (");
        fprintf(__gl_debug_log, "un:%x ", un);
        fprintf(__gl_debug_log, "u1:%x ", u1);
        fprintf(__gl_debug_log, "u2:%x ", u2);
        fprintf(__gl_debug_log, "vn:%x ", vn);
        fprintf(__gl_debug_log, "v1:%x ", v1);
        fprintf(__gl_debug_log, "v2:%x ", v2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.MapGrid2f)(un,u1,u2,vn,v1,v2);

        __gl_debug_table.MapGrid2f++;
}

void  APIENTRY __gldb_Materialf (GLenum face,
                                 GLenum pname,
                                 GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Materialf           (");
        fprintf(__gl_debug_log, "face:%x ", face);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Materialf)(face,pname,param);

        __gl_debug_table.Materialf++;
}

void  APIENTRY __gldb_Materialfv (GLenum face,
                                  GLenum pname,
                                  const GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Materialfv          (");
        fprintf(__gl_debug_log, "face:%x ", face);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Materialfv)(face,pname,params);

        __gl_debug_table.Materialfv++;
}

void  APIENTRY __gldb_Materiali (GLenum face,
                                 GLenum pname,
                                 GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Materiali           (");
        fprintf(__gl_debug_log, "face:%x ", face);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Materiali)(face,pname,param);

        __gl_debug_table.Materiali++;
}

void  APIENTRY __gldb_Materialiv (GLenum face,
                                  GLenum pname,
                                  const GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Materialiv          (");
        fprintf(__gl_debug_log, "face:%x ", face);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Materialiv)(face,pname,params);

        __gl_debug_table.Materialiv++;
}

void  APIENTRY __gldb_MatrixMode (GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "MatrixMode          (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.MatrixMode)(mode);

        __gl_debug_table.MatrixMode++;
}

void  APIENTRY __gldb_MultMatrixd (const GLdouble *m)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "MultMatrixd         (");
        fprintf(__gl_debug_log, "m:%x ", m);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.MultMatrixd)(m);

        __gl_debug_table.MultMatrixd++;
}

void  APIENTRY __gldb_MultMatrixf (const GLfloat *m)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "MultMatrixf         (");
        fprintf(__gl_debug_log, "m:%x ", m);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.MultMatrixf)(m);

        __gl_debug_table.MultMatrixf++;
}

void  APIENTRY __gldb_NewList (GLuint list,
                               GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "NewList             (");
        fprintf(__gl_debug_log, "list:%x ", list);
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.NewList)(list,mode);

        __gl_debug_table.NewList++;
}

void  APIENTRY __gldb_Normal3b (GLbyte nx,
                                GLbyte ny,
                                GLbyte nz)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3b            (");
        fprintf(__gl_debug_log, "nx:%x ", nx);
        fprintf(__gl_debug_log, "ny:%x ", ny);
        fprintf(__gl_debug_log, "nz:%x ", nz);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3b)(nx,ny,nz);

        __gl_debug_table.Normal3b++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3bv (const GLbyte *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3bv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3bv)(v);

        __gl_debug_table.Normal3bv++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3d (GLdouble nx,
                                GLdouble ny,
                                GLdouble nz)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3d            (");
        fprintf(__gl_debug_log, "nx:%x ", nx);
        fprintf(__gl_debug_log, "ny:%x ", ny);
        fprintf(__gl_debug_log, "nz:%x ", nz);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3d)(nx,ny,nz);

        __gl_debug_table.Normal3d++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3dv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3dv)(v);

        __gl_debug_table.Normal3dv++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3f (GLfloat nx,
                                GLfloat ny,
                                GLfloat nz)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3f            (");
        fprintf(__gl_debug_log, "nx:%x ", nx);
        fprintf(__gl_debug_log, "ny:%x ", ny);
        fprintf(__gl_debug_log, "nz:%x ", nz);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3f)(nx,ny,nz);

        __gl_debug_table.Normal3f++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3fv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3fv)(v);

        __gl_debug_table.Normal3fv++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3i (GLint nx,
                                GLint ny,
                                GLint nz)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3i            (");
        fprintf(__gl_debug_log, "nx:%x ", nx);
        fprintf(__gl_debug_log, "ny:%x ", ny);
        fprintf(__gl_debug_log, "nz:%x ", nz);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3i)(nx,ny,nz);

        __gl_debug_table.Normal3i++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3iv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3iv)(v);

        __gl_debug_table.Normal3iv++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3s (GLshort nx,
                                GLshort ny,
                                GLshort nz)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3s            (");
        fprintf(__gl_debug_log, "nx:%x ", nx);
        fprintf(__gl_debug_log, "ny:%x ", ny);
        fprintf(__gl_debug_log, "nz:%x ", nz);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3s)(nx,ny,nz);

        __gl_debug_table.Normal3s++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_Normal3sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Normal3sv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->normal.Normal3sv)(v);

        __gl_debug_table.Normal3sv++;
        __gl_debug_table.Normal++;
}

void  APIENTRY __gldb_NormalPointer (GLenum type,
                                     GLsizei stride,
                                     const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "NormalPointer       (");
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.NormalPointer)(type,stride,pointer);

        __gl_debug_table.NormalPointer++;
}

void  APIENTRY __gldb_Ortho (GLdouble left,
                             GLdouble right,
                             GLdouble bottom,
                             GLdouble top,
                             GLdouble zNear,
                             GLdouble zFar)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Ortho               (");
        fprintf(__gl_debug_log, "left:%x ", left);
        fprintf(__gl_debug_log, "right:%x ", right);
        fprintf(__gl_debug_log, "bottom:%x ", bottom);
        fprintf(__gl_debug_log, "top:%x ", top);
        fprintf(__gl_debug_log, "zNear:%x ", zNear);
        fprintf(__gl_debug_log, "zFar:%x ", zFar);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Ortho)(left,right,bottom,top,zNear,zFar);

        __gl_debug_table.Ortho++;
}

void  APIENTRY __gldb_PassThrough (GLfloat token)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PassThrough         (");
        fprintf(__gl_debug_log, "token:%x ", token);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PassThrough)(token);

        __gl_debug_table.PassThrough++;
}

void  APIENTRY __gldb_PixelMapfv (GLenum map,
                                  GLint mapsize,
                                  const GLfloat *values)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PixelMapfv          (");
        fprintf(__gl_debug_log, "map:%x ", map);
        fprintf(__gl_debug_log, "mapsize:%x ", mapsize);
        fprintf(__gl_debug_log, "values:%x ", values);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PixelMapfv)(map,mapsize,values);

        __gl_debug_table.PixelMapfv++;
}

void  APIENTRY __gldb_PixelMapuiv (GLenum map,
                                   GLint mapsize,
                                   const GLuint *values)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PixelMapuiv         (");
        fprintf(__gl_debug_log, "map:%x ", map);
        fprintf(__gl_debug_log, "mapsize:%x ", mapsize);
        fprintf(__gl_debug_log, "values:%x ", values);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PixelMapuiv)(map,mapsize,values);

        __gl_debug_table.PixelMapuiv++;
}

void  APIENTRY __gldb_PixelMapusv (GLenum map,
                                   GLint mapsize,
                                   const GLushort *values)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PixelMapusv         (");
        fprintf(__gl_debug_log, "map:%x ", map);
        fprintf(__gl_debug_log, "mapsize:%x ", mapsize);
        fprintf(__gl_debug_log, "values:%x ", values);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PixelMapusv)(map,mapsize,values);

        __gl_debug_table.PixelMapusv++;
}

void  APIENTRY __gldb_PixelStoref (GLenum pname,
                                   GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PixelStoref         (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PixelStoref)(pname,param);

        __gl_debug_table.PixelStoref++;
}

void  APIENTRY __gldb_PixelStorei (GLenum pname,
                                   GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PixelStorei         (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PixelStorei)(pname,param);

        __gl_debug_table.PixelStorei++;
}

void  APIENTRY __gldb_PixelTransferf (GLenum pname,
                                      GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PixelTransferf      (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PixelTransferf)(pname,param);

        __gl_debug_table.PixelTransferf++;
}

void  APIENTRY __gldb_PixelTransferi (GLenum pname,
                                      GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PixelTransferi      (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PixelTransferi)(pname,param);

        __gl_debug_table.PixelTransferi++;
}

void  APIENTRY __gldb_PixelZoom (GLfloat xfactor,
                                 GLfloat yfactor)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PixelZoom           (");
        fprintf(__gl_debug_log, "xfactor:%x ", xfactor);
        fprintf(__gl_debug_log, "yfactor:%x ", yfactor);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PixelZoom)(xfactor,yfactor);

        __gl_debug_table.PixelZoom++;
}

void  APIENTRY __gldb_PointSize (GLfloat size)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PointSize           (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PointSize)(size);

        __gl_debug_table.PointSize++;
}

void  APIENTRY __gldb_PolygonMode (GLenum face,
                                   GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PolygonMode         (");
        fprintf(__gl_debug_log, "face:%x ", face);
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PolygonMode)(face,mode);

        __gl_debug_table.PolygonMode++;
}

void  APIENTRY __gldb_PolygonOffset (GLfloat factor,
                                     GLfloat units)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PolygonOffset       (");
        fprintf(__gl_debug_log, "factor:%x ", factor);
        fprintf(__gl_debug_log, "units:%x ", units);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PolygonOffset)(factor,units);

        __gl_debug_table.PolygonOffset++;
}

void  APIENTRY __gldb_PolygonStipple (const GLubyte *mask)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PolygonStipple      (");
        fprintf(__gl_debug_log, "mask:%x ", mask);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PolygonStipple)(mask);

        __gl_debug_table.PolygonStipple++;
}

void  APIENTRY __gldb_PopAttrib (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PopAttrib           (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PopAttrib)();

        __gl_debug_table.PopAttrib++;
}

void  APIENTRY __gldb_PopClientAttrib (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PopClientAttrib     (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PopClientAttrib)();

        __gl_debug_table.PopClientAttrib++;
}

void  APIENTRY __gldb_PopMatrix (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PopMatrix           (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PopMatrix)();

        __gl_debug_table.PopMatrix++;
}

void  APIENTRY __gldb_PopName (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PopName             (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PopName)();

        __gl_debug_table.PopName++;
}

void  APIENTRY __gldb_PrioritizeTextures (GLsizei n,
                                          const GLuint *textures,
                                          const GLclampf *priorities)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PrioritizeTextures  (");
        fprintf(__gl_debug_log, "n:%x ", n);
        fprintf(__gl_debug_log, "textures:%x ", textures);
        fprintf(__gl_debug_log, "priorities:%x ", priorities);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PrioritizeTextures)(n,textures,priorities);

        __gl_debug_table.PrioritizeTextures++;
}

void  APIENTRY __gldb_PushAttrib (GLbitfield mask)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PushAttrib          (");
        fprintf(__gl_debug_log, "mask:%x ", mask);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PushAttrib)(mask);

        __gl_debug_table.PushAttrib++;
}

void  APIENTRY __gldb_PushClientAttrib (GLbitfield mask)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PushClientAttrib    (");
        fprintf(__gl_debug_log, "mask:%x ", mask);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PushClientAttrib)(mask);

        __gl_debug_table.PushClientAttrib++;
}

void  APIENTRY __gldb_PushMatrix (void)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PushMatrix          (");
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PushMatrix)();

        __gl_debug_table.PushMatrix++;
}

void  APIENTRY __gldb_PushName (GLuint name)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PushName            (");
        fprintf(__gl_debug_log, "name:%x ", name);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PushName)(name);

        __gl_debug_table.PushName++;
}

void  APIENTRY __gldb_RasterPos2d (GLdouble x,
                                   GLdouble y)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos2d         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos2d)(x,y);

        __gl_debug_table.RasterPos2d++;
}

void  APIENTRY __gldb_RasterPos2dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos2dv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos2dv)(v);

        __gl_debug_table.RasterPos2dv++;
}

void  APIENTRY __gldb_RasterPos2f (GLfloat x,
                                   GLfloat y)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos2f         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos2f)(x,y);

        __gl_debug_table.RasterPos2f++;
}

void  APIENTRY __gldb_RasterPos2fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos2fv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos2fv)(v);

        __gl_debug_table.RasterPos2fv++;
}

void  APIENTRY __gldb_RasterPos2i (GLint x,
                                   GLint y)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos2i         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos2i)(x,y);

        __gl_debug_table.RasterPos2i++;
}

void  APIENTRY __gldb_RasterPos2iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos2iv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos2iv)(v);

        __gl_debug_table.RasterPos2iv++;
}

void  APIENTRY __gldb_RasterPos2s (GLshort x,
                                   GLshort y)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos2s         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos2s)(x,y);

        __gl_debug_table.RasterPos2s++;
}

void  APIENTRY __gldb_RasterPos2sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos2sv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos2sv)(v);

        __gl_debug_table.RasterPos2sv++;
}

void  APIENTRY __gldb_RasterPos3d (GLdouble x,
                                   GLdouble y,
                                   GLdouble z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos3d         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos3d)(x,y,z);

        __gl_debug_table.RasterPos3d++;
}

void  APIENTRY __gldb_RasterPos3dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos3dv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos3dv)(v);

        __gl_debug_table.RasterPos3dv++;
}

void  APIENTRY __gldb_RasterPos3f (GLfloat x,
                                   GLfloat y,
                                   GLfloat z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos3f         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos3f)(x,y,z);

        __gl_debug_table.RasterPos3f++;
}

void  APIENTRY __gldb_RasterPos3fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos3fv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos3fv)(v);

        __gl_debug_table.RasterPos3fv++;
}

void  APIENTRY __gldb_RasterPos3i (GLint x,
                                   GLint y,
                                   GLint z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos3i         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos3i)(x,y,z);

        __gl_debug_table.RasterPos3i++;
}

void  APIENTRY __gldb_RasterPos3iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos3iv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos3iv)(v);

        __gl_debug_table.RasterPos3iv++;
}

void  APIENTRY __gldb_RasterPos3s (GLshort x,
                                   GLshort y,
                                   GLshort z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos3s         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos3s)(x,y,z);

        __gl_debug_table.RasterPos3s++;
}

void  APIENTRY __gldb_RasterPos3sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos3sv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos3sv)(v);

        __gl_debug_table.RasterPos3sv++;
}

void  APIENTRY __gldb_RasterPos4d (GLdouble x,
                                   GLdouble y,
                                   GLdouble z,
                                   GLdouble w)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos4d         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, "w:%x ", w);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos4d)(x,y,z,w);

        __gl_debug_table.RasterPos4d++;
}

void  APIENTRY __gldb_RasterPos4dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos4dv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos4dv)(v);

        __gl_debug_table.RasterPos4dv++;
}

void  APIENTRY __gldb_RasterPos4f (GLfloat x,
                                   GLfloat y,
                                   GLfloat z,
                                   GLfloat w)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos4f         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, "w:%x ", w);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos4f)(x,y,z,w);

        __gl_debug_table.RasterPos4f++;
}

void  APIENTRY __gldb_RasterPos4fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos4fv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos4fv)(v);

        __gl_debug_table.RasterPos4fv++;
}

void  APIENTRY __gldb_RasterPos4i (GLint x,
                                   GLint y,
                                   GLint z,
                                   GLint w)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos4i         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, "w:%x ", w);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos4i)(x,y,z,w);

        __gl_debug_table.RasterPos4i++;
}

void  APIENTRY __gldb_RasterPos4iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos4iv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos4iv)(v);

        __gl_debug_table.RasterPos4iv++;
}

void  APIENTRY __gldb_RasterPos4s (GLshort x,
                                   GLshort y,
                                   GLshort z,
                                   GLshort w)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos4s         (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, "w:%x ", w);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos4s)(x,y,z,w);

        __gl_debug_table.RasterPos4s++;
}

void  APIENTRY __gldb_RasterPos4sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RasterPos4sv        (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rasterPos.RasterPos4sv)(v);

        __gl_debug_table.RasterPos4sv++;
}

void  APIENTRY __gldb_ReadBuffer (GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ReadBuffer          (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ReadBuffer)(mode);

        __gl_debug_table.ReadBuffer++;
}

void  APIENTRY __gldb_ReadPixels (GLint x,
                                  GLint y,
                                  GLsizei width,
                                  GLsizei height,
                                  GLenum format,
                                  GLenum type,
                                  GLvoid *pixels)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ReadPixels          (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, "format:%x ", format);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "pixels:%x ", pixels);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ReadPixels)(x,y,width,height,format,type,pixels);

        __gl_debug_table.ReadPixels++;
}

void  APIENTRY __gldb_Rectd (GLdouble x1,
                             GLdouble y1,
                             GLdouble x2,
                             GLdouble y2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rectd               (");
        fprintf(__gl_debug_log, "x1:%x ", x1);
        fprintf(__gl_debug_log, "y1:%x ", y1);
        fprintf(__gl_debug_log, "x2:%x ", x2);
        fprintf(__gl_debug_log, "y2:%x ", y2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rect.Rectd)(x1,y1,x2,y2);

        __gl_debug_table.Rectd++;
}

void  APIENTRY __gldb_Rectdv (const GLdouble *v1,
                              const GLdouble *v2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rectdv              (");
        fprintf(__gl_debug_log, "v1:%x ", v1);
        fprintf(__gl_debug_log, "v2:%x ", v2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rect.Rectdv)(v1,v2);

        __gl_debug_table.Rectdv++;
}

void  APIENTRY __gldb_Rectf (GLfloat x1,
                             GLfloat y1,
                             GLfloat x2,
                             GLfloat y2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rectf               (");
        fprintf(__gl_debug_log, "x1:%x ", x1);
        fprintf(__gl_debug_log, "y1:%x ", y1);
        fprintf(__gl_debug_log, "x2:%x ", x2);
        fprintf(__gl_debug_log, "y2:%x ", y2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rect.Rectf)(x1,y1,x2,y2);

        __gl_debug_table.Rectf++;
}

void  APIENTRY __gldb_Rectfv (const GLfloat *v1,
                              const GLfloat *v2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rectfv              (");
        fprintf(__gl_debug_log, "v1:%x ", v1);
        fprintf(__gl_debug_log, "v2:%x ", v2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rect.Rectfv)(v1,v2);

        __gl_debug_table.Rectfv++;
}

void  APIENTRY __gldb_Recti (GLint x1,
                             GLint y1,
                             GLint x2,
                             GLint y2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Recti               (");
        fprintf(__gl_debug_log, "x1:%x ", x1);
        fprintf(__gl_debug_log, "y1:%x ", y1);
        fprintf(__gl_debug_log, "x2:%x ", x2);
        fprintf(__gl_debug_log, "y2:%x ", y2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rect.Recti)(x1,y1,x2,y2);

        __gl_debug_table.Recti++;
}

void  APIENTRY __gldb_Rectiv (const GLint *v1,
                              const GLint *v2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rectiv              (");
        fprintf(__gl_debug_log, "v1:%x ", v1);
        fprintf(__gl_debug_log, "v2:%x ", v2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rect.Rectiv)(v1,v2);

        __gl_debug_table.Rectiv++;
}

void  APIENTRY __gldb_Rects (GLshort x1,
                             GLshort y1,
                             GLshort x2,
                             GLshort y2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rects               (");
        fprintf(__gl_debug_log, "x1:%x ", x1);
        fprintf(__gl_debug_log, "y1:%x ", y1);
        fprintf(__gl_debug_log, "x2:%x ", x2);
        fprintf(__gl_debug_log, "y2:%x ", y2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rect.Rects)(x1,y1,x2,y2);

        __gl_debug_table.Rects++;
}

void  APIENTRY __gldb_Rectsv (const GLshort *v1,
                              const GLshort *v2)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rectsv              (");
        fprintf(__gl_debug_log, "v1:%x ", v1);
        fprintf(__gl_debug_log, "v2:%x ", v2);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->rect.Rectsv)(v1,v2);

        __gl_debug_table.Rectsv++;
}

GLint  APIENTRY __gldb_RenderMode (GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "RenderMode          (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        return
        (*__gl_real_immed->dispatch.RenderMode)(mode);

        __gl_debug_table.RenderMode++;
}

void  APIENTRY __gldb_Rotated (GLdouble angle,
                               GLdouble x,
                               GLdouble y,
                               GLdouble z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rotated             (");
        fprintf(__gl_debug_log, "angle:%x ", angle);
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Rotated)(angle,x,y,z);

        __gl_debug_table.Rotated++;
}

void  APIENTRY __gldb_Rotatef (GLfloat angle,
                               GLfloat x,
                               GLfloat y,
                               GLfloat z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Rotatef             (");
        fprintf(__gl_debug_log, "angle:%x ", angle);
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Rotatef)(angle,x,y,z);

        __gl_debug_table.Rotatef++;
}

void  APIENTRY __gldb_Scaled (GLdouble x,
                              GLdouble y,
                              GLdouble z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Scaled              (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Scaled)(x,y,z);

        __gl_debug_table.Scaled++;
}

void  APIENTRY __gldb_Scalef (GLfloat x,
                              GLfloat y,
                              GLfloat z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Scalef              (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Scalef)(x,y,z);

        __gl_debug_table.Scalef++;
}

void  APIENTRY __gldb_Scissor (GLint x,
                               GLint y,
                               GLsizei width,
                               GLsizei height)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Scissor             (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Scissor)(x,y,width,height);

        __gl_debug_table.Scissor++;
}

void  APIENTRY __gldb_SelectBuffer (GLsizei size,
                                    GLuint *buffer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "SelectBuffer        (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, "buffer:%x ", buffer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.SelectBuffer)(size,buffer);

        __gl_debug_table.SelectBuffer++;
}

void  APIENTRY __gldb_ShadeModel (GLenum mode)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ShadeModel          (");
        fprintf(__gl_debug_log, "mode:%x ", mode);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ShadeModel)(mode);

        __gl_debug_table.ShadeModel++;
}

void  APIENTRY __gldb_StencilFunc (GLenum func,
                                   GLint ref,
                                   GLuint mask)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "StencilFunc         (");
        fprintf(__gl_debug_log, "func:%x ", func);
        fprintf(__gl_debug_log, "ref:%x ", ref);
        fprintf(__gl_debug_log, "mask:%x ", mask);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.StencilFunc)(func,ref,mask);

        __gl_debug_table.StencilFunc++;
}

void  APIENTRY __gldb_StencilMask (GLuint mask)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "StencilMask         (");
        fprintf(__gl_debug_log, "mask:%x ", mask);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.StencilMask)(mask);

        __gl_debug_table.StencilMask++;
}

void  APIENTRY __gldb_StencilOp (GLenum fail,
                                 GLenum zfail,
                                 GLenum zpass)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "StencilOp           (");
        fprintf(__gl_debug_log, "fail:%x ", fail);
        fprintf(__gl_debug_log, "zfail:%x ", zfail);
        fprintf(__gl_debug_log, "zpass:%x ", zpass);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.StencilOp)(fail,zfail,zpass);

        __gl_debug_table.StencilOp++;
}

void  APIENTRY __gldb_TexCoord1d (GLdouble s)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord1d          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord1d)(s);

        __gl_debug_table.TexCoord1d++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord1dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord1dv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord1dv)(v);

        __gl_debug_table.TexCoord1dv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord1f (GLfloat s)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord1f          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord1f)(s);

        __gl_debug_table.TexCoord1f++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord1fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord1fv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord1fv)(v);

        __gl_debug_table.TexCoord1fv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord1i (GLint s)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord1i          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord1i)(s);

        __gl_debug_table.TexCoord1i++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord1iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord1iv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord1iv)(v);

        __gl_debug_table.TexCoord1iv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord1s (GLshort s)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord1s          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord1s)(s);

        __gl_debug_table.TexCoord1s++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord1sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord1sv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord1sv)(v);

        __gl_debug_table.TexCoord1sv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord2d (GLdouble s,
                                  GLdouble t)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord2d          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord2d)(s,t);

        __gl_debug_table.TexCoord2d++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord2dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord2dv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord2dv)(v);

        __gl_debug_table.TexCoord2dv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord2f (GLfloat s,
                                  GLfloat t)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord2f          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord2f)(s,t);

        __gl_debug_table.TexCoord2f++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord2fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord2fv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord2fv)(v);

        __gl_debug_table.TexCoord2fv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord2i (GLint s,
                                  GLint t)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord2i          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord2i)(s,t);

        __gl_debug_table.TexCoord2i++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord2iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord2iv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord2iv)(v);

        __gl_debug_table.TexCoord2iv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord2s (GLshort s,
                                  GLshort t)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord2s          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord2s)(s,t);

        __gl_debug_table.TexCoord2s++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord2sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord2sv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord2sv)(v);

        __gl_debug_table.TexCoord2sv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord3d (GLdouble s,
                                  GLdouble t,
                                  GLdouble r)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord3d          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, "r:%x ", r);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord3d)(s,t,r);

        __gl_debug_table.TexCoord3d++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord3dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord3dv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord3dv)(v);

        __gl_debug_table.TexCoord3dv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord3f (GLfloat s,
                                  GLfloat t,
                                  GLfloat r)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord3f          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, "r:%x ", r);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord3f)(s,t,r);

        __gl_debug_table.TexCoord3f++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord3fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord3fv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord3fv)(v);

        __gl_debug_table.TexCoord3fv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord3i (GLint s,
                                  GLint t,
                                  GLint r)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord3i          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, "r:%x ", r);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord3i)(s,t,r);

        __gl_debug_table.TexCoord3i++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord3iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord3iv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord3iv)(v);

        __gl_debug_table.TexCoord3iv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord3s (GLshort s,
                                  GLshort t,
                                  GLshort r)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord3s          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, "r:%x ", r);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord3s)(s,t,r);

        __gl_debug_table.TexCoord3s++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord3sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord3sv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord3sv)(v);

        __gl_debug_table.TexCoord3sv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord4d (GLdouble s,
                                  GLdouble t,
                                  GLdouble r,
                                  GLdouble q)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord4d          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, "r:%x ", r);
        fprintf(__gl_debug_log, "q:%x ", q);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord4d)(s,t,r,q);

        __gl_debug_table.TexCoord4d++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord4dv (const GLdouble *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord4dv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord4dv)(v);

        __gl_debug_table.TexCoord4dv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord4f (GLfloat s,
                                  GLfloat t,
                                  GLfloat r,
                                  GLfloat q)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord4f          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, "r:%x ", r);
        fprintf(__gl_debug_log, "q:%x ", q);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord4f)(s,t,r,q);

        __gl_debug_table.TexCoord4f++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord4fv (const GLfloat *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord4fv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord4fv)(v);

        __gl_debug_table.TexCoord4fv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord4i (GLint s,
                                  GLint t,
                                  GLint r,
                                  GLint q)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord4i          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, "r:%x ", r);
        fprintf(__gl_debug_log, "q:%x ", q);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord4i)(s,t,r,q);

        __gl_debug_table.TexCoord4i++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord4iv (const GLint *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord4iv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord4iv)(v);

        __gl_debug_table.TexCoord4iv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord4s (GLshort s,
                                  GLshort t,
                                  GLshort r,
                                  GLshort q)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord4s          (");
        fprintf(__gl_debug_log, "s:%x ", s);
        fprintf(__gl_debug_log, "t:%x ", t);
        fprintf(__gl_debug_log, "r:%x ", r);
        fprintf(__gl_debug_log, "q:%x ", q);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord4s)(s,t,r,q);

        __gl_debug_table.TexCoord4s++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoord4sv (const GLshort *v)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoord4sv         (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->texCoord.TexCoord4sv)(v);

        __gl_debug_table.TexCoord4sv++;
        __gl_debug_table.TexCoord++;
}

void  APIENTRY __gldb_TexCoordPointer (GLint size,
                                       GLenum type,
                                       GLsizei stride,
                                       const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoordPointer     (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexCoordPointer)(size,type,stride,pointer);

        __gl_debug_table.TexCoordPointer++;
}

void  APIENTRY __gldb_TexEnvf (GLenum target,
                               GLenum pname,
                               GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexEnvf             (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexEnvf)(target,pname,param);

        __gl_debug_table.TexEnvf++;
}

void  APIENTRY __gldb_TexEnvfv (GLenum target,
                                GLenum pname,
                                const GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexEnvfv            (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexEnvfv)(target,pname,params);

        __gl_debug_table.TexEnvfv++;
}

void  APIENTRY __gldb_TexEnvi (GLenum target,
                               GLenum pname,
                               GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexEnvi             (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexEnvi)(target,pname,param);

        __gl_debug_table.TexEnvi++;
}

void  APIENTRY __gldb_TexEnviv (GLenum target,
                                GLenum pname,
                                const GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexEnviv            (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexEnviv)(target,pname,params);

        __gl_debug_table.TexEnviv++;
}

void  APIENTRY __gldb_TexGend (GLenum coord,
                               GLenum pname,
                               GLdouble param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexGend             (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexGend)(coord,pname,param);

        __gl_debug_table.TexGend++;
}

void  APIENTRY __gldb_TexGendv (GLenum coord,
                                GLenum pname,
                                const GLdouble *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexGendv            (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexGendv)(coord,pname,params);

        __gl_debug_table.TexGendv++;
}

void  APIENTRY __gldb_TexGenf (GLenum coord,
                               GLenum pname,
                               GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexGenf             (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexGenf)(coord,pname,param);

        __gl_debug_table.TexGenf++;
}

void  APIENTRY __gldb_TexGenfv (GLenum coord,
                                GLenum pname,
                                const GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexGenfv            (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexGenfv)(coord,pname,params);

        __gl_debug_table.TexGenfv++;
}

void  APIENTRY __gldb_TexGeni (GLenum coord,
                               GLenum pname,
                               GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexGeni             (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexGeni)(coord,pname,param);

        __gl_debug_table.TexGeni++;
}

void  APIENTRY __gldb_TexGeniv (GLenum coord,
                                GLenum pname,
                                const GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexGeniv            (");
        fprintf(__gl_debug_log, "coord:%x ", coord);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexGeniv)(coord,pname,params);

        __gl_debug_table.TexGeniv++;
}

void  APIENTRY __gldb_TexImage1D (GLenum target,
                                  GLint level,
                                  GLint components,
                                  GLsizei width,
                                  GLint border,
                                  GLenum format,
                                  GLenum type,
                                  const GLvoid *pixels)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexImage1D          (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "components:%x ", components);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "border:%x ", border);
        fprintf(__gl_debug_log, "format:%x ", format);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "pixels:%x ", pixels);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexImage1D)(target,level,components,width,border,format,type,pixels);

        __gl_debug_table.TexImage1D++;
}

void  APIENTRY __gldb_TexImage2D (GLenum target,
                                  GLint level,
                                  GLint components,
                                  GLsizei width,
                                  GLsizei height,
                                  GLint border,
                                  GLenum format,
                                  GLenum type,
                                  const GLvoid *pixels)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexImage2D          (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "components:%x ", components);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, "border:%x ", border);
        fprintf(__gl_debug_log, "format:%x ", format);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "pixels:%x ", pixels);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexImage2D)(target,level,components,width,height,border,format,type,pixels);

        __gl_debug_table.TexImage2D++;
}

void  APIENTRY __gldb_TexParameterf (GLenum target,
                                     GLenum pname,
                                     GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexParameterf       (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexParameterf)(target,pname,param);

        __gl_debug_table.TexParameterf++;
}

void  APIENTRY __gldb_TexParameterfv (GLenum target,
                                      GLenum pname,
                                      const GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexParameterfv      (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexParameterfv)(target,pname,params);

        __gl_debug_table.TexParameterfv++;
}

void  APIENTRY __gldb_TexParameteri (GLenum target,
                                     GLenum pname,
                                     GLint param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexParameteri       (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexParameteri)(target,pname,param);

        __gl_debug_table.TexParameteri++;
}

void  APIENTRY __gldb_TexParameteriv (GLenum target,
                                      GLenum pname,
                                      const GLint *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexParameteriv      (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexParameteriv)(target,pname,params);

        __gl_debug_table.TexParameteriv++;
}

void  APIENTRY __gldb_TexSubImage1D (GLenum target,
                                     GLint level,
                                     GLint xoffset,
                                     GLsizei width,
                                     GLenum format,
                                     GLenum type,
                                     const GLvoid *pixels)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexSubImage1D       (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "xoffset:%x ", xoffset);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "format:%x ", format);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "pixels:%x ", pixels);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexSubImage1D)(target,level,xoffset,width,format,type,pixels);

        __gl_debug_table.TexSubImage1D++;
}

void  APIENTRY __gldb_TexSubImage2D (GLenum target,
                                     GLint level,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLsizei width,
                                     GLsizei height,
                                     GLenum format,
                                     GLenum type,
                                     const GLvoid *pixels)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexSubImage2D       (");
        fprintf(__gl_debug_log, "target:%x ", target);
        fprintf(__gl_debug_log, "level:%x ", level);
        fprintf(__gl_debug_log, "xoffset:%x ", xoffset);
        fprintf(__gl_debug_log, "yoffset:%x ", yoffset);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, "format:%x ", format);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "pixels:%x ", pixels);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexSubImage2D)(target,level,xoffset,yoffset,width,height,format,type,pixels);

        __gl_debug_table.TexSubImage2D++;
}

void  APIENTRY __gldb_Translated (GLdouble x,
                                  GLdouble y,
                                  GLdouble z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Translated          (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Translated)(x,y,z);

        __gl_debug_table.Translated++;
}

void  APIENTRY __gldb_Translatef (GLfloat x,
                                  GLfloat y,
                                  GLfloat z)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Translatef          (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Translatef)(x,y,z);

        __gl_debug_table.Translatef++;
}

void  APIENTRY __gldb_Vertex2d (GLdouble x,
                                GLdouble y)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex2d            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;
        (*__gl_real_immed->vertex.Vertex2d)(x,y);
        gc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;

        __gl_debug_table.Vertex2d++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex2dv (const GLdouble *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex2dv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;
        (*__gl_real_immed->vertex.Vertex2dv)(v);
        gc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;

        __gl_debug_table.Vertex2dv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex2f (GLfloat x,
                                GLfloat y)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex2f            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;
        (*__gl_real_immed->vertex.Vertex2f)(x,y);
        gc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;

        __gl_debug_table.Vertex2f++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex2fv (const GLfloat *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex2fv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;
        (*__gl_real_immed->vertex.Vertex2fv)(v);
        gc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;

        __gl_debug_table.Vertex2fv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex2i (GLint x,
                                GLint y)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex2i            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;
        (*__gl_real_immed->vertex.Vertex2i)(x,y);
        gc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;

        __gl_debug_table.Vertex2i++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex2iv (const GLint *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex2iv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;
        (*__gl_real_immed->vertex.Vertex2iv)(v);
        gc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;

        __gl_debug_table.Vertex2iv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex2s (GLshort x,
                                GLshort y)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex2s            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;
        (*__gl_real_immed->vertex.Vertex2s)(x,y);
        gc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;

        __gl_debug_table.Vertex2s++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex2sv (const GLshort *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex2sv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex2fv = __gl_real_immed->vertex.Vertex2fv;
        (*__gl_real_immed->vertex.Vertex2sv)(v);
        gc->dispatchState->vertex.Vertex2fv = __gldb_Vertex2fv;

        __gl_debug_table.Vertex2sv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex3d (GLdouble x,
                                GLdouble y,
                                GLdouble z)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex3d            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;
        (*__gl_real_immed->vertex.Vertex3d)(x,y,z);
        gc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;

        __gl_debug_table.Vertex3d++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex3dv (const GLdouble *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex3dv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;
        (*__gl_real_immed->vertex.Vertex3dv)(v);
        gc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;

        __gl_debug_table.Vertex3dv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex3f (GLfloat x,
                                GLfloat y,
                                GLfloat z)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex3f            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;
        (*__gl_real_immed->vertex.Vertex3f)(x,y,z);
        gc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;

        __gl_debug_table.Vertex3f++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex3fv (const GLfloat *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex3fv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;
        (*__gl_real_immed->vertex.Vertex3fv)(v);
        gc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;

        __gl_debug_table.Vertex3fv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex3i (GLint x,
                                GLint y,
                                GLint z)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex3i            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;
        (*__gl_real_immed->vertex.Vertex3i)(x,y,z);
        gc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;

        __gl_debug_table.Vertex3i++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex3iv (const GLint *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex3iv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;
        (*__gl_real_immed->vertex.Vertex3iv)(v);
        gc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;

        __gl_debug_table.Vertex3iv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex3s (GLshort x,
                                GLshort y,
                                GLshort z)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex3s            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;
        (*__gl_real_immed->vertex.Vertex3s)(x,y,z);
        gc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;

        __gl_debug_table.Vertex3s++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex3sv (const GLshort *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex3sv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex3fv = __gl_real_immed->vertex.Vertex3fv;
        (*__gl_real_immed->vertex.Vertex3sv)(v);
        gc->dispatchState->vertex.Vertex3fv = __gldb_Vertex3fv;

        __gl_debug_table.Vertex3sv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex4d (GLdouble x,
                                GLdouble y,
                                GLdouble z,
                                GLdouble w)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex4d            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, "w:%x ", w);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;
        (*__gl_real_immed->vertex.Vertex4d)(x,y,z,w);
        gc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;

        __gl_debug_table.Vertex4d++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex4dv (const GLdouble *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex4dv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;
        (*__gl_real_immed->vertex.Vertex4dv)(v);
        gc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;

        __gl_debug_table.Vertex4dv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex4f (GLfloat x,
                                GLfloat y,
                                GLfloat z,
                                GLfloat w)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex4f            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, "w:%x ", w);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;
        (*__gl_real_immed->vertex.Vertex4f)(x,y,z,w);
        gc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;

        __gl_debug_table.Vertex4f++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex4fv (const GLfloat *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex4fv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;
        (*__gl_real_immed->vertex.Vertex4fv)(v);
        gc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;

        __gl_debug_table.Vertex4fv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex4i (GLint x,
                                GLint y,
                                GLint z,
                                GLint w)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex4i            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, "w:%x ", w);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;
        (*__gl_real_immed->vertex.Vertex4i)(x,y,z,w);
        gc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;

        __gl_debug_table.Vertex4i++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex4iv (const GLint *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex4iv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;
        (*__gl_real_immed->vertex.Vertex4iv)(v);
        gc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;

        __gl_debug_table.Vertex4iv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex4s (GLshort x,
                                GLshort y,
                                GLshort z,
                                GLshort w)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex4s            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "z:%x ", z);
        fprintf(__gl_debug_log, "w:%x ", w);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;
        (*__gl_real_immed->vertex.Vertex4s)(x,y,z,w);
        gc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;

        __gl_debug_table.Vertex4s++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_Vertex4sv (const GLshort *v)
{
        __GL_SETUP();

        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Vertex4sv           (");
        fprintf(__gl_debug_log, "v:%x ", v);
        fprintf(__gl_debug_log, ")\n");
        }

        gc->dispatchState->vertex.Vertex4fv = __gl_real_immed->vertex.Vertex4fv;
        (*__gl_real_immed->vertex.Vertex4sv)(v);
        gc->dispatchState->vertex.Vertex4fv = __gldb_Vertex4fv;

        __gl_debug_table.Vertex4sv++;
        __gl_debug_table.Vertex++;
        __gldb_StatVertex();
}

void  APIENTRY __gldb_VertexPointer (GLint size,
                                     GLenum type,
                                     GLsizei stride,
                                     const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "VertexPointer       (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.VertexPointer)(size,type,stride,pointer);

        __gl_debug_table.VertexPointer++;
}

void  APIENTRY __gldb_Viewport (GLint x,
                                GLint y,
                                GLsizei width,
                                GLsizei height)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "Viewport            (");
        fprintf(__gl_debug_log, "x:%x ", x);
        fprintf(__gl_debug_log, "y:%x ", y);
        fprintf(__gl_debug_log, "width:%x ", width);
        fprintf(__gl_debug_log, "height:%x ", height);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.Viewport)(x,y,width,height);

        __gl_debug_table.Viewport++;
}

void  APIENTRY __gldb_ColorPointerEXT (GLint size,
                                       GLenum type,
                                       GLsizei stride,
                                       GLsizei count,
                                       const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "ColorPointerEXT     (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "count:%x ", count);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.ColorPointerEXT)(size,type,stride,count,pointer);

        __gl_debug_table.ColorPointerEXT++;
}

void  APIENTRY __gldb_EdgeFlagPointerEXT (GLsizei stride,
                                          GLsizei count,
                                          const GLboolean *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "EdgeFlagPointerEXT  (");
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "count:%x ", count);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.EdgeFlagPointerEXT)(stride,count,pointer);

        __gl_debug_table.EdgeFlagPointerEXT++;
}

void  APIENTRY __gldb_IndexPointerEXT (GLenum type,
                                       GLsizei stride,
                                       GLsizei count,
                                       const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "IndexPointerEXT     (");
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "count:%x ", count);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.IndexPointerEXT)(type,stride,count,pointer);

        __gl_debug_table.IndexPointerEXT++;
}

void  APIENTRY __gldb_NormalPointerEXT (GLenum type,
                                        GLsizei stride,
                                        GLsizei count,
                                        const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "NormalPointerEXT    (");
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "count:%x ", count);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.NormalPointerEXT)(type,stride,count,pointer);

        __gl_debug_table.NormalPointerEXT++;
}

void  APIENTRY __gldb_TexCoordPointerEXT (GLint size,
                                          GLenum type,
                                          GLsizei stride,
                                          GLsizei count,
                                          const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "TexCoordPointerEXT  (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "count:%x ", count);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.TexCoordPointerEXT)(size,type,stride,count,pointer);

        __gl_debug_table.TexCoordPointerEXT++;
}

void  APIENTRY __gldb_VertexPointerEXT (GLint size,
                                        GLenum type,
                                        GLsizei stride,
                                        GLsizei count,
                                        const GLvoid *pointer)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "VertexPointerEXT    (");
        fprintf(__gl_debug_log, "size:%x ", size);
        fprintf(__gl_debug_log, "type:%x ", type);
        fprintf(__gl_debug_log, "stride:%x ", stride);
        fprintf(__gl_debug_log, "count:%x ", count);
        fprintf(__gl_debug_log, "pointer:%x ", pointer);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.VertexPointerEXT)(size,type,stride,count,pointer);

        __gl_debug_table.VertexPointerEXT++;
}

void  APIENTRY __gldb_PointParameterfEXT (GLenum pname,
                                   GLfloat param)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PointParameterfEXT         (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "param:%x ", param);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PointParameterfEXT)(pname,param);

        __gl_debug_table.PointParameterfEXT++;
}

void  APIENTRY __gldb_PointParameterfvEXT (GLenum pname,
                                    const GLfloat *params)
{
        if (__gl_debug_verbosity >= 5) {
        fprintf(__gl_debug_log, "PointParameterfvEXT        (");
        fprintf(__gl_debug_log, "pname:%x ", pname);
        fprintf(__gl_debug_log, "params:%x ", params);
        fprintf(__gl_debug_log, ")\n");
        }

        (*__gl_real_immed->dispatch.PointParameterfvEXT)(pname,params);

        __gl_debug_table.PointParameterfvEXT++;
}


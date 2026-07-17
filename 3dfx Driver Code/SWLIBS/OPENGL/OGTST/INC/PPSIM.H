#ifndef __PP_SIM_H
#define __PP_SIM_H

#include <GL/gl.h>
#include <X11/Xlib.h>

GLboolean ppInit(GLsizei width, GLsizei height);

GLenum ppGetError(void);

void 
    ppClear(GLbitfield buf),
    ppCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, 
	GLenum type),
    ppDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type,
	const GLvoid *pixels),
    ppReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, 
	GLenum format, GLenum type, GLvoid *pixels),
    ppBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, 
	GLfloat xmove, GLfloat ymove, const GLubyte *bitmap),

    ppGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, 
	GLvoid *pixels),
    ppTexImage1D (GLenum target, GLint level, GLint components, GLsizei width, 
	GLint border, GLenum format, GLenum type, const GLvoid *pixels),
    ppTexImage2D (GLenum target, GLint level, GLint components, GLsizei width, 
	GLsizei height, GLint border, GLenum format, GLenum type, 
	const GLvoid *pixels),

    ppCopyColorTableSGI (GLenum target, GLenum internalformat, GLint x, 
	GLint y, GLsizei width),
    ppGetColorTableSGI (GLenum target, GLenum format, GLenum type, 
	GLvoid *table),
    ppColorTableSGI (GLenum target, GLenum internalformat, GLsizei width, 
	GLenum format, GLenum type, const GLvoid *table),

    ppHistogramEXT(GLenum target, GLsizei width, GLenum internalformat,
	GLboolean sink),
    ppGetHistogramEXT(GLenum target, GLboolean reset, GLenum format, 
	GLenum type, GLvoid *values),
    ppResetHistogramEXT(GLenum target),

    ppMinmaxEXT(GLenum target, GLenum internalformat, GLboolean sink),
    ppGetMinmaxEXT(GLenum target, GLboolean reset, GLenum format, GLenum type,
	GLvoid *values),
    ppResetMinmaxEXT(GLenum target);

Window ppShowFrame(Display *dpy);

void ppAlphaFunc(GLenum func, GLclampf ref);
void ppBlendFunc(GLenum sfactor, GLenum dfactor);
void ppClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
void ppClearIndex(GLfloat index);
void ppClearDepth(GLclampd depth);
void ppClearStencil(GLint s);
void ppColor3b (GLbyte red, GLbyte green, GLbyte blue);
void ppColor3bv (const GLbyte *v);
void ppColor3d (GLdouble red, GLdouble green, GLdouble blue);
void ppColor3dv (const GLdouble *v);
void ppColor3f (GLfloat red, GLfloat green, GLfloat blue);
void ppColor3fv (const GLfloat *v);
void ppColor3i (GLint red, GLint green, GLint blue);
void ppColor3iv (const GLint *v);
void ppColor3s (GLshort red, GLshort green, GLshort blue);
void ppColor3sv (const GLshort *v);
void ppColor3ub (GLubyte red, GLubyte green, GLubyte blue);
void ppColor3ubv (const GLubyte *v);
void ppColor3ui (GLuint red, GLuint green, GLuint blue);
void ppColor3uiv (const GLuint *v);
void ppColor3us (GLushort red, GLushort green, GLushort blue);
void ppColor3usv (const GLushort *v);
void ppColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
void ppColor4bv (const GLbyte *v);
void ppColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
void ppColor4dv (const GLdouble *v);
void ppColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ppColor4fv (const GLfloat *v);
void ppColor4i (GLint red, GLint green, GLint blue, GLint alpha);
void ppColor4iv (const GLint *v);
void ppColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha);
void ppColor4sv (const GLshort *v);
void ppColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void ppColor4ubv (const GLubyte *v);
void ppColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha);
void ppColor4uiv (const GLuint *v);
void ppColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha);
void ppColor4usv (const GLushort *v);
void ppColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void ppDepthFunc(GLenum func);
void ppDepthRange(GLclampd n, GLclampd f);
void ppEnable(GLenum cap);
void ppDisable(GLenum cap);
void ppIndexMask (GLuint mask);
void ppIndexd (GLdouble c);
void ppIndexdv (const GLdouble *c);
void ppIndexf (GLfloat c);
void ppIndexfv (const GLfloat *c);
void ppIndexi (GLint c);
void ppIndexiv (const GLint *c);
void ppIndexs (GLshort c);
void ppIndexsv (const GLshort *c);
void ppLogicOp(GLenum op);
void ppPixelMapfv(GLenum map, GLsizei size, const GLfloat *values);
void ppPixelMapuiv(GLenum map, GLsizei size, const GLuint *values);
void ppPixelMapusv(GLenum map, GLsizei size, const GLushort *values);
void ppPixelStoref(GLenum pname, GLfloat param);
void ppPixelStorei(GLenum pname, GLint param);
void ppPixelTransferf(GLenum pname, GLfloat param);
void ppPixelTransferi(GLenum pname, GLint param);
void ppPixelZoom(GLfloat zx, GLfloat zy);
void ppRasterPos2d(GLdouble x, GLdouble y);
void ppRasterPos2dv(const GLdouble *v);
void ppRasterPos2f(GLfloat x, GLfloat y);
void ppRasterPos2fv(const GLfloat *v);
void ppRasterPos2i(GLint x, GLint y);
void ppRasterPos2iv(const GLint *v);
void ppRasterPos2s(GLshort x, GLshort y);
void ppRasterPos2sv(const GLshort *v);
void ppRasterPos3d(GLdouble x, GLdouble y, GLdouble z);
void ppRasterPos3dv(const GLdouble *v);
void ppRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
void ppRasterPos3fv(const GLfloat *v);
void ppRasterPos3i(GLint x, GLint y, GLint z);
void ppRasterPos3iv(const GLint *v);
void ppRasterPos3s(GLshort x, GLshort y, GLshort z);
void ppRasterPos3sv(const GLshort *v);
void ppRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void ppRasterPos4dv(const GLdouble *v);
void ppRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void ppRasterPos4fv(const GLfloat *v);
void ppRasterPos4i(GLint x, GLint y, GLint z, GLint w);
void ppRasterPos4iv(const GLint *v);
void ppRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
void ppRasterPos4sv(const GLshort *v);
void ppScissor(GLint left, GLint bottom, GLsizei width, GLsizei height);
void ppStencilFunc(GLenum func, GLint ref, GLuint mask);
void ppStencilMask(GLuint mask);
void ppStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
void ppViewport(GLint x, GLint y, GLsizei width, GLsizei height);

void ppGetHistogramParameterfvEXT (GLenum target, GLenum pname, 
    GLfloat *params);
void ppGetHistogramParameterivEXT (GLenum target, GLenum pname,
    GLint *params);

void ppGetMinmaxParameterfvEXT (GLenum target, GLenum pname, GLfloat *params);
void ppGetMinmaxParameterivEXT (GLenum target, GLenum pname, GLint *params);

void ppColorTableParameterfvSGI (GLenum target, GLenum pname, 
    const GLfloat *params);
void ppColorTableParameterivSGI (GLenum target, GLenum pname, 
    const GLint *params);

void ppGetColorTableParameterfvSGI (GLenum target, GLenum pname,
    GLfloat *params);
void ppGetColorTableParameterivSGI (GLenum target, GLenum pname,
    GLint *params);

void ppCopyColorTableSGI (GLenum target, GLenum internalformat, GLint x, 
    GLint y, GLsizei width);
void ppPixelTexGenSGIX(GLenum mode);

void ppCopyTexImage1DEXT (GLenum target, GLint level, GLenum internalformat,
    GLint x, GLint y, GLsizei width, GLint border);
void ppCopyTexImage2DEXT (GLenum target, GLint level, GLenum internalformat,
    GLint x, GLint y, GLsizei width, GLsizei height, GLint border);


void ppConvolutionFilter1DEXT (GLenum target, GLenum internalformat, 
    GLsizei width, GLenum format, GLenum type, const GLvoid *image);
void ppConvolutionFilter2DEXT (GLenum target, GLenum internalformat, 
    GLsizei width, GLsizei height, GLenum format, GLenum type, 
    const GLvoid *image);
void ppConvolutionParameterfEXT (GLenum target, GLenum pname, GLfloat params);
void ppConvolutionParameterfvEXT (GLenum target, GLenum pname, 
    const GLfloat *params);
void ppConvolutionParameteriEXT (GLenum target, GLenum pname, GLint params);
void ppConvolutionParameterivEXT (GLenum target, GLenum pname, 
    const GLint *params);
void ppCopyConvolutionFilter1DEXT (GLenum target, GLenum internalformat, 
    GLint x, GLint y, GLsizei width);
void ppCopyConvolutionFilter2DEXT (GLenum target, GLenum internalformat, 
    GLint x, GLint y, GLsizei width, GLsizei height);
void ppGetConvolutionFilterEXT (GLenum target, GLenum format, GLenum type, 
    GLvoid *image);
void ppSeparableFilter2DEXT (GLenum target, GLenum internalformat, 
    GLsizei width, GLsizei height, GLenum format, GLenum type, 
    const GLvoid *row, const GLvoid *column);

void ppCopyConvolutionFilter1DEXT (GLenum target, GLenum internalformat,
    GLint x, GLint y, GLsizei width);
void ppCopyConvolutionFilter2DEXT (GLenum target, GLenum internalformat,
    GLint x, GLint y, GLsizei width, GLsizei height);
void ppGetConvolutionFilterEXT (GLenum target, GLenum format, GLenum type,
    GLvoid *image);
void ppGetSeparableFilterEXT (GLenum target, GLenum format, GLenum type,
    GLvoid *row, GLvoid *column, GLvoid *span);


void ppCopyMode(GLboolean);
void ppCopyState(void);

#endif /* __PP_SIM_H */

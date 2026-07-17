#ifndef __sst_imfncs_h_
#define __sst_imfncs_h_

#include "types.h"
#include "apimacro.h"

extern void APIENTRY __glsstim_Accum(GLenum op, GLfloat value);
extern void APIENTRY __glsstim_AlphaFunc(GLenum af, GLfloat ref);
extern GLboolean APIENTRY __glsstim_AreTexturesResident(GLsizei n,
							const GLuint* textures,
							GLboolean* residences);
extern void APIENTRY __glsstim_BindTexture(GLenum target, GLuint texture);
extern void APIENTRY __glsstim_BlendFunc(GLenum sf, GLenum df);
extern void APIENTRY __glsstim_Clear( GLbitfield);
extern void APIENTRY __glsstim_ClearAccum( GLfloat, GLfloat, GLfloat, GLfloat);
extern void APIENTRY __glsstim_ClearColor( GLclampf, GLclampf, GLclampf, GLclampf);
extern void APIENTRY __glsstim_ClearDepth( GLclampd);
extern void APIENTRY __glsstim_ColorMask( GLboolean, GLboolean, GLboolean, GLboolean);
extern void APIENTRY __glsstim_CopyTexImage1D(GLenum target, GLint level, 
					      GLenum internalformat,
					      GLint x, GLint y,
					      GLsizei width, GLint border);
extern void APIENTRY __glsstim_CopyTexImage2D(GLenum target, GLint level,
					      GLenum internalformat,
					      GLint x, GLint y,
					      GLsizei w, GLsizei h,
					      GLint border);
extern void APIENTRY __glsstim_CopyTexSubImage1D(GLenum target, GLint level,
						 GLint xoffset,
						 GLint x, GLint y,
						 GLsizei width);
extern void APIENTRY __glsstim_CopyTexSubImage2D(GLenum target, GLint level,
						 GLint xoffset, GLint yoffset,
						 GLint x, GLint y,
						 GLsizei width, GLsizei height);
extern void APIENTRY __glsstim_CullFace(GLenum cfm);
extern void APIENTRY __glsstim_DepthMask( GLboolean);
extern void APIENTRY __glsstim_DeleteTextures(GLsizei n, const GLuint* textures);
extern void APIENTRY __glsstim_DepthFunc(GLenum zf);
extern void APIENTRY __glsstim_DepthMask(GLboolean enabled);
extern void APIENTRY __glsstim_Disable(GLenum cap);
extern void APIENTRY __glsstim_DisableClientState(GLenum mode);
extern void APIENTRY __glsstim_Enable(GLenum cap);
extern void APIENTRY __glsstim_EnableClientState(GLenum mode);
extern void APIENTRY __glsstim_FrontFace(GLenum dir);
extern void APIENTRY __glsstim_GenTextures(GLsizei n, GLuint* textures);
extern void APIENTRY __glsstim_GetTexImage(GLenum target, GLint level, GLenum format, 
					   GLenum type,	GLvoid *texels);
extern GLboolean APIENTRY __glsstim_IsTexture(GLuint texture);
extern void APIENTRY __glsstim_PopAttrib(void);
extern GLvoid APIENTRY __glsstim_PrioritizeTextures(GLsizei n,
						    const GLuint* textures,
						    const GLclampf* priorities);
extern void APIENTRY __glsstim_TexParameterf( GLenum, GLenum, GLfloat);
extern void APIENTRY __glsstim_TexParameterfv( GLenum, GLenum, const GLfloat *);
extern void APIENTRY __glsstim_TexParameteri( GLenum, GLenum, GLint);
extern void APIENTRY __glsstim_TexParameteriv( GLenum, GLenum, const GLint *);
extern void APIENTRY __glsstim_TexImage1D( GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern void APIENTRY __glsstim_TexImage2D( GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern void APIENTRY __glsstim_TexSubImage1D(GLenum target, GLint lod, 
					     GLint xoffset, GLint length,
					     GLenum format, GLenum type, 
					     const GLvoid *buf);
extern void APIENTRY __glsstim_TexSubImage2D(GLenum target, GLint lod,
					     GLint xoffset, GLint yoffset,
					     GLsizei w, GLsizei h, GLenum format,
					     GLenum type, const GLvoid *buf);
extern void APIENTRY __glsstim_TexEnvf( GLenum, GLenum, GLfloat);
extern void APIENTRY __glsstim_TexEnvfv( GLenum, GLenum, const GLfloat *);
extern void APIENTRY __glsstim_TexEnvi( GLenum, GLenum, GLint);
extern void APIENTRY __glsstim_TexEnviv( GLenum, GLenum, const GLint *);
extern void APIENTRY __glsstim_Vertex3fv( const GLfloat *);

#endif /* __sst_imfncs_h_ */

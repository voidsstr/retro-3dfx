#ifndef __gl_texmgr_h__
#define __gl_texmgr_h__

#include "texture.h"

/*
 * Texture residence status 
 */

#define __GL_TEXTURE_NOT_RESIDENT           0
#define __GL_TEXTURE_RESIDENCE_LOADED       1
#define __GL_TEXTURE_RESIDENCE_LOWPERF      2
#define __GL_TEXTURE_RESIDENCE_HIPERF       3


/* Texture manager interface */
typedef struct __GLtextureManagerRec __GLtextureManager;

struct __GLtextureManagerRec {

    int refcount;

    /* Release per-context texture manager state.  If no more contexts
     * reference the texture manager, global state is also released.
     */
    void (*releaseTextureManager)(__GLcontext *gc);

    /*
     * Called after glPrioritizeTextures. Releases all of the texture
     * memory so that the texture manager can create a new hierarchy.
     */
    void (*invalidate)(__GLtextureManager *texmgr);
};


/*
 * These functions define the texture manager interface 
 */

/* The create method is part of the __glDevice and is called during
 * context initialization.
 */
void __glInitTextureManager(__GLcontext *gc);

void __glReleaseTextureManager(__GLcontext *gc);

__GLtexture *__glCreateTexture(__GLcontext *gc, GLuint name,
			       GLuint targetIndex);

void __glFreeTexture(__GLcontext *gc, __GLtexture *tex);

GLenum __glTextureMakeResident(__GLcontext *gc, __GLtexture *tex,
			       GLclampf priority);

void __glInvalidateTextureManager(__GLtextureManager *texmgr);


/*
 * These functions define the texture object interface 
 */

void __glFreeTexture(__GLcontext *gc, __GLtexture *tex);

GLenum __glTexObjMakeResident(__GLtexture *tex, GLclampf priority);

GLvoid __glCopyTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			__GLtexture *tex, GLint lod);

GLvoid __glGenericReadTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			       __GLtexture *tex, GLint lod);

__GLtextureBuffer *
__glTextureCreateLevel(__GLcontext *gc, __GLtexture *tex, GLint lod,
		       GLint components, GLsizei w, GLsizei h, GLsizei d,
		       GLint border, GLint dim);

__GLtextureBuffer *
__glTextureCreateProxyLevel(__GLcontext *gc, __GLtexture *tex, GLint lod,
			    GLint components, GLsizei w, GLsizei h, GLsizei d,
			    GLint border, GLint dim);

void __glTextureDeleteLevel(__GLcontext *gc, __GLtexture *tex, GLint lod);
void __glTextureDeleteProxyLevel(__GLcontext *gc, __GLtexture *tex, GLint lod);

#endif

#ifndef __gl_ddtexmgr_h_
#define __gl_ddtexmgr_h_

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
*/
#include <ddraw.h>

#include "types.h"
#include "texture.h"
#include "texfmt.h"
#include "texmgr.h"

#define __GL_NUM_PRIORITIES 8

typedef struct __GLDDrawMipMapLevelRec  {
    __GLmipMapLevel level;
    IDirectDrawSurface *lpMipSurface;
    GLuint *lpMipBuffer;
    GLint  bytesPerTexel;
    void (*slurpImage)(struct __GLDDrawTextureRec *tex, 
                       struct __GLDDrawMipMapLevelRec *this);
    void (*slurpSubImage)(struct __GLDDrawTextureRec *tex, 
			  struct __GLDDrawMipMapLevelRec *this, 
			  GLint x, GLint y, GLint w, GLint h);
} __GLDDrawMipMapLevel;



typedef struct __GLDDrawTextureRec {
    /* Standard interface */
    __GLtexture texture;

    GLint mgrPriority;

    /* Where the texture manager wants this object */
    GLenum assignedResidence;

    /* pointer to the master ddraw surface, may be in system or vram memory */
    IDirectDrawSurface *lpTexSurface;

    /* pointer to the texture in system memory */
    IDirectDrawSurface *lpSystemSurface;

    /* pointer to the texture in video memory */
    IDirectDrawSurface *lpVRAMSurface;

    /* 
     * Index of next texture. This is an index instead of a pointer 
     * because of the tendency of the objlist to move 
     */
    GLint next;
} __GLDDrawTexture;



typedef struct __GLDDrawTextureManagerRec {
    __GLtextureManager texmgr;

    struct __GLDDrawTextureFormatRec *(*lookupDDrawTextureFormat) (GLint format);

    LPDIRECTDRAW lpDDraw;
    GLuint numObj;
    GLuint maxObj;
    __GLDDrawTexture **objList;

    GLuint numActiveObj;
    GLuint maxActiveObj;
    __GLDDrawTexture **activeList;
    int objListHead;
    int objListTail;
} __GLDDrawTextureManager;

/* the default texmgr instance */
extern __GLDDrawTextureManager __glDDrawTextureManager;

void __glDDrawFreeTexture(__GLcontext *gc, __GLtexture *tex);
void __glDDrawInitTextureManager(__GLcontext *gc);
void __glDDrawReleaseTextureManager(__GLcontext *gc);
void __glDDrawInvalidateTextureManager(__GLtextureManager *texmgr);
__GLtextureBuffer *__glDDrawTextureCreateLevel(__GLcontext *gc, __GLtexture *tex,
					       GLint lod, GLint components,
					       GLsizei w, GLsizei h, GLsizei d,
					       GLint border, GLint dim);
__GLtexture *__glDDrawCreateTexture(__GLcontext *gc, GLint name, GLint targetName);
void __glDDrawFreeTexture(__GLcontext *gc, __GLtexture *tex);
GLenum __glDDrawTextureMakeResident(__GLcontext *gc, __GLtexture *tex,
				    GLclampf priority);
void __glDDrawTextureDeleteLevel(__GLcontext *gc, __GLtexture *tex, GLint lod);


#endif /* __gl_ddtexmgr_h_ */

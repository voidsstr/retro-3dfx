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
** $Date: 10/11/00 8:05:06 PM$
*/

#define GEOM_OG_MAX_KEY_SIZE 24
#define GEOM_OG_NUM_SLOTS 40
#define GEOM_OG_BUFFER_SIZE 3072

struct task {
    GLsizei keysize;
    unsigned long key[GEOM_OG_MAX_KEY_SIZE];
    void *space;
};

typedef struct {
    struct task slots[GEOM_OG_NUM_SLOTS], *last_hit;
} __GLgeomOGData;


void* GenerateCompile(__GLcontext *gc, GLuint vp_size, GLuint flags);
void* GenerateDrawVertexes(__GLcontext *gc, GLenum mode, GLboolean elements);
void* GenerateVertexFunc(__GLcontext *gc);
void __glPickVertexShape(__GLcontext *gc);

#define __GL_GEOM_OG_NORMAL	0x0001
#define __GL_GEOM_OG_COLOR	0x0002
#define __GL_GEOM_OG_TEX	0x0004

#define __GL_GEOM_OG_NOCOPY	0x8000

#define __GL_GEOM_OG_NO_Z      	(0x0001 << 16)

#endif

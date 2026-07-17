/*
** Copyright 1998, 3Dfx Interactive Inc.
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
*/
#include "context.h"
#include "global.h"
#include "dlist.h"
#include "dlistopt.h"
#include "g_imfncs.h"
#include "g_lcfncs.h"
#include "g_lcomp.h"
#include "g_listop.h"
#include "g_xproto.h"

const float tacoOne  = 1.0f;
const float tacoZero = 0.0f;

void APIENTRY __glim_MTexCoord1dSGIS( GLenum target , GLdouble s )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = tacoZero;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;
}

void APIENTRY __glim_MTexCoord1dvSGIS( GLenum target , const GLdouble * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = tacoZero;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;
}

void APIENTRY __glim_MTexCoord1fSGIS( GLenum target , GLfloat s )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = s;
    gc->state.current.texture[target].y = tacoZero;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord1fvSGIS( GLenum target , const GLfloat * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = v[0];
    gc->state.current.texture[target].y = tacoZero;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord1iSGIS( GLenum target , GLint s )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = tacoZero;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord1ivSGIS( GLenum target , const GLint * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = tacoZero;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord1sSGIS( GLenum target , GLshort s )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = tacoZero;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord1svSGIS( GLenum target , const GLshort * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = tacoZero;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord2dSGIS( GLenum target , GLdouble s , GLdouble t )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord2dvSGIS( GLenum target , const GLdouble * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;
}

void APIENTRY __glim_MTexCoord2fSGIS( GLenum target , GLfloat s , GLfloat t )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = s;
    gc->state.current.texture[target].y = t;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord2fvSGIS( GLenum target , const GLfloat * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = v[0];
    gc->state.current.texture[target].y = v[1];
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord2iSGIS( GLenum target , GLint s , GLint t )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord2ivSGIS( GLenum target , const GLint * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord2sSGIS( GLenum target , GLshort s , GLshort t )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord2svSGIS( GLenum target , const GLshort * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = tacoZero;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord3dSGIS( GLenum target , GLdouble s , GLdouble t , GLdouble r )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = (float)r;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord3dvSGIS( GLenum target , const GLdouble * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = (float)v[2];
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord3fSGIS( GLenum target , GLfloat s , GLfloat t , GLfloat r )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = s;
    gc->state.current.texture[target].y = t;
    gc->state.current.texture[target].z = r;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord3fvSGIS( GLenum target , const GLfloat * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = v[0];
    gc->state.current.texture[target].y = v[1];
    gc->state.current.texture[target].z = v[2];
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord3iSGIS( GLenum target , GLint s , GLint t , GLint r )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = (float)r;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord3ivSGIS( GLenum target , const GLint * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = (float)v[2];
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord3sSGIS( GLenum target , GLshort s , GLshort t , GLshort r )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = (float)r;
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord3svSGIS( GLenum target , const GLshort * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = (float)v[2];
    gc->state.current.texture[target].w = tacoOne;

}

void APIENTRY __glim_MTexCoord4dSGIS( GLenum target , GLdouble s , GLdouble t , GLdouble r , GLdouble q )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = (float)r;
    gc->state.current.texture[target].w = (float)q;

}

void APIENTRY __glim_MTexCoord4dvSGIS( GLenum target , const GLdouble * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = (float)v[2];
    gc->state.current.texture[target].w = (float)v[3];

}

void APIENTRY __glim_MTexCoord4fSGIS( GLenum target , GLfloat s , GLfloat t , GLfloat r , GLfloat q )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = s;
    gc->state.current.texture[target].y = t; 
    gc->state.current.texture[target].z = r;
    gc->state.current.texture[target].w = q; 

}

void APIENTRY __glim_MTexCoord4fvSGIS( GLenum target , const GLfloat * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = v[0];
    gc->state.current.texture[target].y = v[1];
    gc->state.current.texture[target].z = v[2];
    gc->state.current.texture[target].w = v[3];

}

void APIENTRY __glim_MTexCoord4iSGIS( GLenum target , GLint s , GLint t , GLint r , GLint q )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = (float)r;
    gc->state.current.texture[target].w = (float)q;

}

void APIENTRY __glim_MTexCoord4ivSGIS( GLenum target , const GLint * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = (float)v[2];
    gc->state.current.texture[target].w = (float)v[3];

}

void APIENTRY __glim_MTexCoord4sSGIS( GLenum target , GLshort s , GLshort t , GLshort r , GLshort q )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)s;
    gc->state.current.texture[target].y = (float)t;
    gc->state.current.texture[target].z = (float)r;
    gc->state.current.texture[target].w = (float)q;

}

void APIENTRY __glim_MTexCoord4svSGIS( GLenum target , const GLshort * v )
{
    __GL_SETUP();
    __GL_API_VAPI();

    /* parameter validation is for the weak */
    target -= TEXTURE0_SGIS;
    gc->state.current.texture[target].x = (float)v[0];
    gc->state.current.texture[target].y = (float)v[1];
    gc->state.current.texture[target].z = (float)v[2];
    gc->state.current.texture[target].w = (float)v[3];

}

void APIENTRY __glim_MTexCoordPointerSGIS( GLenum target , GLint size , GLenum type , GLsizei stride , const void * pointer )
{
    __GL_SETUP();

}

void APIENTRY __glim_SelectTextureSGIS( GLenum target )
{
    __GL_SETUP();
    target -= TEXTURE0_SGIS;
    gc->texture.currentTexUnit = target;
    gc->texture.sst.currentTMU = gc->texture.sst.texUnits[target];
}

void APIENTRY __glim_SelectTextureCoordSetSGIS( GLenum target )
{
    __GL_SETUP();

}

/*--------------------------------------------------------------
  List Compile
  --------------------------------------------------------------*/

void APIENTRY __gllc_MTexCoord1dSGIS( GLenum target , GLdouble s )
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord1dSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord1dSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord1dvSGIS;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MTexCoord1dSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord1dvSGIS);
}

void APIENTRY __gllc_MTexCoord1dvSGIS( GLenum target , const GLdouble * v )
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord1dvSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord1dvSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord1dvSGIS;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MTexCoord1dvSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord1dvSGIS);
}

void APIENTRY __gllc_MTexCoord1fSGIS(GLenum target, GLfloat s)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord1fSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord1fSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord1fvSGIS;
    data = (struct __gllc_MTexCoord1fSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord1fvSGIS);
}

void APIENTRY __gllc_MTexCoord1fvSGIS( GLenum target, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord1fvSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord1fvSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord1fvSGIS;
    data = (struct __gllc_MTexCoord1fvSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord1fvSGIS);
}


void APIENTRY __gllc_MTexCoord1iSGIS( GLenum target, GLint s)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord1iSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord1iSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord1ivSGIS;
    data = (struct __gllc_MTexCoord1iSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord1ivSGIS);
}

void APIENTRY __gllc_MTexCoord1ivSGIS( GLenum target, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord1ivSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord1ivSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord1ivSGIS;
    data = (struct __gllc_MTexCoord1ivSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord1ivSGIS);
}

void APIENTRY __gllc_MTexCoord1sSGIS( GLenum target, GLshort s)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord1sSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord1sSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord1svSGIS;
    data = (struct __gllc_MTexCoord1sSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord1svSGIS);
}

void APIENTRY __gllc_MTexCoord1svSGIS( GLenum target, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord1svSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord1svSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord1svSGIS;
    data = (struct __gllc_MTexCoord1svSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord1svSGIS);
}

void APIENTRY __gllc_MTexCoord2dSGIS( GLenum target, GLdouble s, GLdouble t)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord2dSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord2dSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord2dvSGIS;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MTexCoord2dSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord2dvSGIS);
}

void APIENTRY __gllc_MTexCoord2dvSGIS( GLenum target, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord2dvSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord2dvSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord2dvSGIS;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MTexCoord2dvSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord2dvSGIS);
}

void APIENTRY __gllc_MTexCoord2fSGIS( GLenum target, GLfloat s, GLfloat t)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord2fSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord2fSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord2fvSGIS;
    data = (struct __gllc_MTexCoord2fSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord2fvSGIS);
}

void APIENTRY __gllc_MTexCoord2fvSGIS( GLenum target, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord2fvSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord2fvSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord2fvSGIS;
    data = (struct __gllc_MTexCoord2fvSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord2fvSGIS);
}

void APIENTRY __gllc_MTexCoord2iSGIS( GLenum target, GLint s, GLint t)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord2iSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord2iSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord2ivSGIS;
    data = (struct __gllc_MTexCoord2iSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord2ivSGIS);
}

void APIENTRY __gllc_MTexCoord2ivSGIS(  GLenum target, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord2ivSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord2ivSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord2ivSGIS;
    data = (struct __gllc_MTexCoord2ivSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord2ivSGIS);
}

void APIENTRY __gllc_MTexCoord2sSGIS( GLenum target, GLshort s, GLshort t)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord2sSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord2sSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord2svSGIS;
    data = (struct __gllc_MTexCoord2sSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord2svSGIS);
}

void APIENTRY __gllc_MTexCoord2svSGIS( GLenum target, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord2svSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord2svSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord2svSGIS;
    data = (struct __gllc_MTexCoord2svSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord2svSGIS);
}

void APIENTRY __gllc_MTexCoord3dSGIS( GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord3dSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord3dSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord3dvSGIS;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MTexCoord3dSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    data->r = r;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord3dvSGIS);
}

void APIENTRY __gllc_MTexCoord3dvSGIS( GLenum target, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord3dvSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord3dvSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord3dvSGIS;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MTexCoord3dvSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord3dvSGIS);
}

void APIENTRY __gllc_MTexCoord3fSGIS( GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord3fSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord3fSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord3fvSGIS;
    data = (struct __gllc_MTexCoord3fSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    data->r = r;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord3fvSGIS);
}

void APIENTRY __gllc_MTexCoord3fvSGIS( GLenum target, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord3fvSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord3fvSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord3fvSGIS;
    data = (struct __gllc_MTexCoord3fvSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord3fvSGIS);
}

void APIENTRY __gllc_MTexCoord3iSGIS( GLenum target, GLint s, GLint t, GLint r)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord3iSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord3iSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord3ivSGIS;
    data = (struct __gllc_MTexCoord3iSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    data->r = r;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord3ivSGIS);
}

void APIENTRY __gllc_MTexCoord3ivSGIS( GLenum target, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord3ivSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord3ivSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord3ivSGIS;
    data = (struct __gllc_MTexCoord3ivSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord3ivSGIS);
}

void APIENTRY __gllc_MTexCoord3sSGIS( GLenum target, GLshort s, GLshort t, GLshort r)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord3sSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord3sSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord3svSGIS;
    data = (struct __gllc_MTexCoord3sSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    data->r = r;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord3svSGIS);
}

void APIENTRY __gllc_MTexCoord3svSGIS( GLenum target, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord3svSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord3svSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord3svSGIS;
    data = (struct __gllc_MTexCoord3svSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord3svSGIS);
}

void APIENTRY __gllc_MTexCoord4dSGIS( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord4dSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord4dSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord4dvSGIS;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MTexCoord4dSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    data->r = r;
    data->q = q;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord4dvSGIS);
}

void APIENTRY __gllc_MTexCoord4dvSGIS( GLenum target, const GLdouble *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord4dvSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord4dvSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord4dvSGIS;
    dlop->aligned = GL_TRUE;
    data = (struct __gllc_MTexCoord4dvSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord4dvSGIS);
}

void APIENTRY __gllc_MTexCoord4fSGIS( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord4fSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord4fSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord4fvSGIS;
    data = (struct __gllc_MTexCoord4fSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    data->r = r;
    data->q = q;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord4fvSGIS);
}

void APIENTRY __gllc_MTexCoord4fvSGIS( GLenum target, const GLfloat *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord4fvSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord4fvSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord4fvSGIS;
    data = (struct __gllc_MTexCoord4fvSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord4fvSGIS);
}

void APIENTRY __gllc_MTexCoord4iSGIS( GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord4iSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord4iSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord4ivSGIS;
    data = (struct __gllc_MTexCoord4iSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    data->r = r;
    data->q = q;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord4ivSGIS);
}

void APIENTRY __gllc_MTexCoord4ivSGIS( GLenum target, const GLint *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord4ivSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord4ivSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord4ivSGIS;
    data = (struct __gllc_MTexCoord4ivSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord4ivSGIS);
}

void APIENTRY __gllc_MTexCoord4sSGIS( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord4sSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord4sSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord4svSGIS;
    data = (struct __gllc_MTexCoord4sSGIS_Rec *) dlop->data;
    data->target = target;
    data->s = s;
    data->t = t;
    data->r = r;
    data->q = q;
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord4svSGIS);
}

void APIENTRY __gllc_MTexCoord4svSGIS( GLenum target, const GLshort *v)
{
    __GLdlistOp *dlop;
    struct __gllc_MTexCoord4svSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_MTexCoord4svSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_MTexCoord4svSGIS;
    data = (struct __gllc_MTexCoord4svSGIS_Rec *) dlop->data;
    data->target = target;
    data->v[0] = v[0];
    data->v[1] = v[1];
    data->v[2] = v[2];
    data->v[3] = v[3];
    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_TEXCOORDS;
    __glDlistAppendOp(gc, dlop, __glle_MTexCoord4svSGIS);
}

void APIENTRY __gllc_MTexCoordPointerSGIS( GLenum target , GLint size , GLenum type , GLsizei stride , const void * pointer )
{
    __GL_SETUP();

}

void APIENTRY __gllc_SelectTextureSGIS( GLenum target )
{
    __GLdlistOp *dlop;
    struct __gllc_SelectTextureSGIS_Rec *data;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, sizeof(struct __gllc_SelectTextureSGIS_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_SelectTextureSGIS;
    data = (struct __gllc_SelectTextureSGIS_Rec *) dlop->data;
    data->target = target;
    __glDlistAppendOp(gc, dlop, __glle_SelectTextureSGIS);
}

void APIENTRY __gllc_SelectTextureCoordSetSGIS( GLenum target )
{
    __GL_SETUP();

}

/*--------------------------------------------------------------
  Compile and Execute
  --------------------------------------------------------------*/
void APIENTRY __glce_MTexCoord1dSGIS( GLenum target, GLdouble s)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord1dSGIS;
    __gl_dispatch.dispatch.MTexCoord1dSGIS = gc->listCompState.dispatch.MTexCoord1dSGIS;
    glMTexCoord1dSGIS( target, s);
    __gl_dispatch.dispatch.MTexCoord1dSGIS = gc->savedDispatchState.dispatch.MTexCoord1dSGIS;
    glMTexCoord1dSGIS( target, s);
    __gl_dispatch.dispatch.MTexCoord1dSGIS = save;
}

void APIENTRY __glce_MTexCoord1dvSGIS( GLenum target, const GLdouble *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord1dvSGIS;
    __gl_dispatch.dispatch.MTexCoord1dvSGIS = gc->listCompState.dispatch.MTexCoord1dvSGIS;
    glMTexCoord1dvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord1dvSGIS = gc->savedDispatchState.dispatch.MTexCoord1dvSGIS;
    glMTexCoord1dvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord1dvSGIS = save;
}

void APIENTRY __glce_MTexCoord1fSGIS( GLenum target, GLfloat s)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord1fSGIS;
    __gl_dispatch.dispatch.MTexCoord1fSGIS = gc->listCompState.dispatch.MTexCoord1fSGIS;
    glMTexCoord1fSGIS( target, s);
    __gl_dispatch.dispatch.MTexCoord1fSGIS = gc->savedDispatchState.dispatch.MTexCoord1fSGIS;
    glMTexCoord1fSGIS( target, s);
    __gl_dispatch.dispatch.MTexCoord1fSGIS = save;
}

void APIENTRY __glce_MTexCoord1fvSGIS( GLenum target, const GLfloat *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord1fvSGIS;
    __gl_dispatch.dispatch.MTexCoord1fvSGIS = gc->listCompState.dispatch.MTexCoord1fvSGIS;
    glMTexCoord1fvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord1fvSGIS = gc->savedDispatchState.dispatch.MTexCoord1fvSGIS;
    glMTexCoord1fvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord1fvSGIS = save;
}

void APIENTRY __glce_MTexCoord1iSGIS( GLenum target, GLint s)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord1iSGIS;
    __gl_dispatch.dispatch.MTexCoord1iSGIS = gc->listCompState.dispatch.MTexCoord1iSGIS;
    glMTexCoord1iSGIS( target, s);
    __gl_dispatch.dispatch.MTexCoord1iSGIS = gc->savedDispatchState.dispatch.MTexCoord1iSGIS;
    glMTexCoord1iSGIS( target, s);
    __gl_dispatch.dispatch.MTexCoord1iSGIS = save;
}

void APIENTRY __glce_MTexCoord1ivSGIS( GLenum target, const GLint *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord1ivSGIS;
    __gl_dispatch.dispatch.MTexCoord1ivSGIS = gc->listCompState.dispatch.MTexCoord1ivSGIS;
    glMTexCoord1ivSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord1ivSGIS = gc->savedDispatchState.dispatch.MTexCoord1ivSGIS;
    glMTexCoord1ivSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord1ivSGIS = save;
}

void APIENTRY __glce_MTexCoord1sSGIS( GLenum target, GLshort s)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord1sSGIS;
    __gl_dispatch.dispatch.MTexCoord1sSGIS = gc->listCompState.dispatch.MTexCoord1sSGIS;
    glMTexCoord1sSGIS( target, s);
    __gl_dispatch.dispatch.MTexCoord1sSGIS = gc->savedDispatchState.dispatch.MTexCoord1sSGIS;
    glMTexCoord1sSGIS( target, s);
    __gl_dispatch.dispatch.MTexCoord1sSGIS = save;
}

void APIENTRY __glce_MTexCoord1svSGIS( GLenum target, const GLshort *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord1svSGIS;
    __gl_dispatch.dispatch.MTexCoord1svSGIS = gc->listCompState.dispatch.MTexCoord1svSGIS;
    glMTexCoord1svSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord1svSGIS = gc->savedDispatchState.dispatch.MTexCoord1svSGIS;
    glMTexCoord1svSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord1svSGIS = save;
}

void APIENTRY __glce_MTexCoord2dSGIS( GLenum target, GLdouble s, GLdouble t)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord2dSGIS;
    __gl_dispatch.dispatch.MTexCoord2dSGIS = gc->listCompState.dispatch.MTexCoord2dSGIS;
    glMTexCoord2dSGIS( target, s, t);
    __gl_dispatch.dispatch.MTexCoord2dSGIS = gc->savedDispatchState.dispatch.MTexCoord2dSGIS;
    glMTexCoord2dSGIS( target, s, t);
    __gl_dispatch.dispatch.MTexCoord2dSGIS = save;
}

void APIENTRY __glce_MTexCoord2dvSGIS( GLenum target, const GLdouble *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord2dvSGIS;
    __gl_dispatch.dispatch.MTexCoord2dvSGIS = gc->listCompState.dispatch.MTexCoord2dvSGIS;
    glMTexCoord2dvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord2dvSGIS = gc->savedDispatchState.dispatch.MTexCoord2dvSGIS;
    glMTexCoord2dvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord2dvSGIS = save;
}

void APIENTRY __glce_MTexCoord2fSGIS( GLenum target, GLfloat s, GLfloat t)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord2fSGIS;
    __gl_dispatch.dispatch.MTexCoord2fSGIS = gc->listCompState.dispatch.MTexCoord2fSGIS;
    glMTexCoord2fSGIS( target, s, t);
    __gl_dispatch.dispatch.MTexCoord2fSGIS = gc->savedDispatchState.dispatch.MTexCoord2fSGIS;
    glMTexCoord2fSGIS( target, s, t);
    __gl_dispatch.dispatch.MTexCoord2fSGIS = save;
}

void APIENTRY __glce_MTexCoord2fvSGIS( GLenum target, const GLfloat *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord2fvSGIS;
    __gl_dispatch.dispatch.MTexCoord2fvSGIS = gc->listCompState.dispatch.MTexCoord2fvSGIS;
    glMTexCoord2fvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord2fvSGIS = gc->savedDispatchState.dispatch.MTexCoord2fvSGIS;
    glMTexCoord2fvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord2fvSGIS = save;
}

void APIENTRY __glce_MTexCoord2iSGIS( GLenum target, GLint s, GLint t)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord2iSGIS;
    __gl_dispatch.dispatch.MTexCoord2iSGIS = gc->listCompState.dispatch.MTexCoord2iSGIS;
    glMTexCoord2iSGIS( target, s, t);
    __gl_dispatch.dispatch.MTexCoord2iSGIS = gc->savedDispatchState.dispatch.MTexCoord2iSGIS;
    glMTexCoord2iSGIS( target, s, t);
    __gl_dispatch.dispatch.MTexCoord2iSGIS = save;
}

void APIENTRY __glce_MTexCoord2ivSGIS( GLenum target, const GLint *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord2ivSGIS;
    __gl_dispatch.dispatch.MTexCoord2ivSGIS = gc->listCompState.dispatch.MTexCoord2ivSGIS;
    glMTexCoord2ivSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord2ivSGIS = gc->savedDispatchState.dispatch.MTexCoord2ivSGIS;
    glMTexCoord2ivSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord2ivSGIS = save;
}

void APIENTRY __glce_MTexCoord2sSGIS( GLenum target, GLshort s, GLshort t)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord2sSGIS;
    __gl_dispatch.dispatch.MTexCoord2sSGIS = gc->listCompState.dispatch.MTexCoord2sSGIS;
    glMTexCoord2sSGIS( target, s, t);
    __gl_dispatch.dispatch.MTexCoord2sSGIS = gc->savedDispatchState.dispatch.MTexCoord2sSGIS;
    glMTexCoord2sSGIS( target, s, t);
    __gl_dispatch.dispatch.MTexCoord2sSGIS = save;
}

void APIENTRY __glce_MTexCoord2svSGIS( GLenum target, const GLshort *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord2svSGIS;
    __gl_dispatch.dispatch.MTexCoord2svSGIS = gc->listCompState.dispatch.MTexCoord2svSGIS;
    glMTexCoord2svSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord2svSGIS = gc->savedDispatchState.dispatch.MTexCoord2svSGIS;
    glMTexCoord2svSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord2svSGIS = save;
}

void APIENTRY __glce_MTexCoord3dSGIS( GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord3dSGIS;
    __gl_dispatch.dispatch.MTexCoord3dSGIS = gc->listCompState.dispatch.MTexCoord3dSGIS;
    glMTexCoord3dSGIS( target, s, t, r);
    __gl_dispatch.dispatch.MTexCoord3dSGIS = gc->savedDispatchState.dispatch.MTexCoord3dSGIS;
    glMTexCoord3dSGIS( target, s, t, r);
    __gl_dispatch.dispatch.MTexCoord3dSGIS = save;
}

void APIENTRY __glce_MTexCoord3dvSGIS( GLenum target, const GLdouble *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord3dvSGIS;
    __gl_dispatch.dispatch.MTexCoord3dvSGIS = gc->listCompState.dispatch.MTexCoord3dvSGIS;
    glMTexCoord3dvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord3dvSGIS = gc->savedDispatchState.dispatch.MTexCoord3dvSGIS;
    glMTexCoord3dvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord3dvSGIS = save;
}

void APIENTRY __glce_MTexCoord3fSGIS( GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord3fSGIS;
    __gl_dispatch.dispatch.MTexCoord3fSGIS = gc->listCompState.dispatch.MTexCoord3fSGIS;
    glMTexCoord3fSGIS( target, s, t, r);
    __gl_dispatch.dispatch.MTexCoord3fSGIS = gc->savedDispatchState.dispatch.MTexCoord3fSGIS;
    glMTexCoord3fSGIS( target, s, t, r);
    __gl_dispatch.dispatch.MTexCoord3fSGIS = save;
}

void APIENTRY __glce_MTexCoord3fvSGIS( GLenum target, const GLfloat *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord3fvSGIS;
    __gl_dispatch.dispatch.MTexCoord3fvSGIS = gc->listCompState.dispatch.MTexCoord3fvSGIS;
    glMTexCoord3fvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord3fvSGIS = gc->savedDispatchState.dispatch.MTexCoord3fvSGIS;
    glMTexCoord3fvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord3fvSGIS = save;
}

void APIENTRY __glce_MTexCoord3iSGIS( GLenum target, GLint s, GLint t, GLint r)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord3iSGIS;
    __gl_dispatch.dispatch.MTexCoord3iSGIS = gc->listCompState.dispatch.MTexCoord3iSGIS;
    glMTexCoord3iSGIS( target, s, t, r);
    __gl_dispatch.dispatch.MTexCoord3iSGIS = gc->savedDispatchState.dispatch.MTexCoord3iSGIS;
    glMTexCoord3iSGIS( target, s, t, r);
    __gl_dispatch.dispatch.MTexCoord3iSGIS = save;
}

void APIENTRY __glce_MTexCoord3ivSGIS( GLenum target, const GLint *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord3ivSGIS;
    __gl_dispatch.dispatch.MTexCoord3ivSGIS = gc->listCompState.dispatch.MTexCoord3ivSGIS;
    glMTexCoord3ivSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord3ivSGIS = gc->savedDispatchState.dispatch.MTexCoord3ivSGIS;
    glMTexCoord3ivSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord3ivSGIS = save;
}

void APIENTRY __glce_MTexCoord3sSGIS( GLenum target, GLshort s, GLshort t, GLshort r)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord3sSGIS;
    __gl_dispatch.dispatch.MTexCoord3sSGIS = gc->listCompState.dispatch.MTexCoord3sSGIS;
    glMTexCoord3sSGIS( target, s, t, r);
    __gl_dispatch.dispatch.MTexCoord3sSGIS = gc->savedDispatchState.dispatch.MTexCoord3sSGIS;
    glMTexCoord3sSGIS( target, s, t, r);
    __gl_dispatch.dispatch.MTexCoord3sSGIS = save;
}

void APIENTRY __glce_MTexCoord3svSGIS( GLenum target, const GLshort *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord3svSGIS;
    __gl_dispatch.dispatch.MTexCoord3svSGIS = gc->listCompState.dispatch.MTexCoord3svSGIS;
    glMTexCoord3svSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord3svSGIS = gc->savedDispatchState.dispatch.MTexCoord3svSGIS;
    glMTexCoord3svSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord3svSGIS = save;
}

void APIENTRY __glce_MTexCoord4dSGIS( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord4dSGIS;
    __gl_dispatch.dispatch.MTexCoord4dSGIS = gc->listCompState.dispatch.MTexCoord4dSGIS;
    glMTexCoord4dSGIS( target, s, t, r, q);
    __gl_dispatch.dispatch.MTexCoord4dSGIS = gc->savedDispatchState.dispatch.MTexCoord4dSGIS;
    glMTexCoord4dSGIS( target, s, t, r, q);
    __gl_dispatch.dispatch.MTexCoord4dSGIS = save;
}

void APIENTRY __glce_MTexCoord4dvSGIS( GLenum target, const GLdouble *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord4dvSGIS;
    __gl_dispatch.dispatch.MTexCoord4dvSGIS = gc->listCompState.dispatch.MTexCoord4dvSGIS;
    glMTexCoord4dvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord4dvSGIS = gc->savedDispatchState.dispatch.MTexCoord4dvSGIS;
    glMTexCoord4dvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord4dvSGIS = save;
}

void APIENTRY __glce_MTexCoord4fSGIS( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord4fSGIS;
    __gl_dispatch.dispatch.MTexCoord4fSGIS = gc->listCompState.dispatch.MTexCoord4fSGIS;
    glMTexCoord4fSGIS( target, s, t, r, q);
    __gl_dispatch.dispatch.MTexCoord4fSGIS = gc->savedDispatchState.dispatch.MTexCoord4fSGIS;
    glMTexCoord4fSGIS( target, s, t, r, q);
    __gl_dispatch.dispatch.MTexCoord4fSGIS = save;
}

void APIENTRY __glce_MTexCoord4fvSGIS( GLenum target, const GLfloat *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord4fvSGIS;
    __gl_dispatch.dispatch.MTexCoord4fvSGIS = gc->listCompState.dispatch.MTexCoord4fvSGIS;
    glMTexCoord4fvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord4fvSGIS = gc->savedDispatchState.dispatch.MTexCoord4fvSGIS;
    glMTexCoord4fvSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord4fvSGIS = save;
}

void APIENTRY __glce_MTexCoord4iSGIS( GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord4iSGIS;
    __gl_dispatch.dispatch.MTexCoord4iSGIS = gc->listCompState.dispatch.MTexCoord4iSGIS;
    glMTexCoord4iSGIS( target, s, t, r, q);
    __gl_dispatch.dispatch.MTexCoord4iSGIS = gc->savedDispatchState.dispatch.MTexCoord4iSGIS;
    glMTexCoord4iSGIS( target, s, t, r, q);
    __gl_dispatch.dispatch.MTexCoord4iSGIS = save;
}

void APIENTRY __glce_MTexCoord4ivSGIS( GLenum target, const GLint *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord4ivSGIS;
    __gl_dispatch.dispatch.MTexCoord4ivSGIS = gc->listCompState.dispatch.MTexCoord4ivSGIS;
    glMTexCoord4ivSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord4ivSGIS = gc->savedDispatchState.dispatch.MTexCoord4ivSGIS;
    glMTexCoord4ivSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord4ivSGIS = save;
}

void APIENTRY __glce_MTexCoord4sSGIS( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord4sSGIS;
    __gl_dispatch.dispatch.MTexCoord4sSGIS = gc->listCompState.dispatch.MTexCoord4sSGIS;
    glMTexCoord4sSGIS( target, s, t, r, q);
    __gl_dispatch.dispatch.MTexCoord4sSGIS = gc->savedDispatchState.dispatch.MTexCoord4sSGIS;
    glMTexCoord4sSGIS( target, s, t, r, q);
    __gl_dispatch.dispatch.MTexCoord4sSGIS = save;
}

void APIENTRY __glce_MTexCoord4svSGIS( GLenum target, const GLshort *v)
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoord4svSGIS;
    __gl_dispatch.dispatch.MTexCoord4svSGIS = gc->listCompState.dispatch.MTexCoord4svSGIS;
    glMTexCoord4svSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord4svSGIS = gc->savedDispatchState.dispatch.MTexCoord4svSGIS;
    glMTexCoord4svSGIS( target, v);
    __gl_dispatch.dispatch.MTexCoord4svSGIS = save;
}

void APIENTRY __glce_MTexCoordPointerSGIS( GLenum target , GLint size , GLenum type , GLsizei stride , const void * pointer )
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.MTexCoordPointerSGIS;
    __gl_dispatch.dispatch.MTexCoordPointerSGIS = gc->listCompState.dispatch.MTexCoordPointerSGIS;
    glMTexCoordPointerSGIS( target, size, type, stride, pointer );
    __gl_dispatch.dispatch.MTexCoordPointerSGIS = gc->savedDispatchState.dispatch.MTexCoordPointerSGIS;
    glMTexCoordPointerSGIS( target, size, type, stride, pointer );
    __gl_dispatch.dispatch.MTexCoordPointerSGIS = save;
}

void APIENTRY __glce_SelectTextureSGIS( GLenum target )
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.SelectTextureSGIS;
    __gl_dispatch.dispatch.SelectTextureSGIS = gc->listCompState.dispatch.SelectTextureSGIS;
    glSelectTextureSGIS( target );
    __gl_dispatch.dispatch.SelectTextureSGIS = gc->savedDispatchState.dispatch.SelectTextureSGIS;
    glSelectTextureSGIS( target );
    __gl_dispatch.dispatch.SelectTextureSGIS = save;
}

void APIENTRY __glce_SelectTextureCoordSetSGIS( GLenum target )
{
    GLvoid *save;
    __GL_SETUP();

    save = __gl_dispatch.dispatch.SelectTextureCoordSetSGIS;
    __gl_dispatch.dispatch.SelectTextureCoordSetSGIS = gc->listCompState.dispatch.SelectTextureCoordSetSGIS;
    glSelectTextureCoordSetSGIS( target );
    __gl_dispatch.dispatch.SelectTextureCoordSetSGIS = gc->savedDispatchState.dispatch.SelectTextureCoordSetSGIS;
    glSelectTextureCoordSetSGIS( target );
    __gl_dispatch.dispatch.SelectTextureCoordSetSGIS = save;
}


/*--------------------------------------------------------------
  List Execute
  --------------------------------------------------------------*/

const GLubyte *__glle_MTexCoord1dvSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord1dvSGIS_Rec *data;

    data = (struct __gllc_MTexCoord1dvSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord1dvSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord1dvSGIS_Rec);
}

const GLubyte *__glle_MTexCoord1fvSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord1fvSGIS_Rec *data;

    data = (struct __gllc_MTexCoord1fvSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord1fvSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord1fvSGIS_Rec);
}

const GLubyte *__glle_MTexCoord1ivSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord1ivSGIS_Rec *data;

    data = (struct __gllc_MTexCoord1ivSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord1ivSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord1ivSGIS_Rec);
}

const GLubyte *__glle_MTexCoord1svSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord1svSGIS_Rec *data;

    data = (struct __gllc_MTexCoord1svSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord1svSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord1svSGIS_Rec);
}

const GLubyte *__glle_MTexCoord2dvSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord2dvSGIS_Rec *data;

    data = (struct __gllc_MTexCoord2dvSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord2dvSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord2dvSGIS_Rec);
}

const GLubyte *__glle_MTexCoord2fvSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord2fvSGIS_Rec *data;

    data = (struct __gllc_MTexCoord2fvSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord2fvSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord2fvSGIS_Rec);
}

const GLubyte *__glle_MTexCoord2ivSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord2ivSGIS_Rec *data;

    data = (struct __gllc_MTexCoord2ivSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord2ivSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord2ivSGIS_Rec);
}

const GLubyte *__glle_MTexCoord2svSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord2svSGIS_Rec *data;

    data = (struct __gllc_MTexCoord2svSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord2svSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord2svSGIS_Rec);
}

const GLubyte *__glle_MTexCoord3dvSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord3dvSGIS_Rec *data;

    data = (struct __gllc_MTexCoord3dvSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord3dvSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord3dvSGIS_Rec);
}

const GLubyte *__glle_MTexCoord3fvSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord3fvSGIS_Rec *data;

    data = (struct __gllc_MTexCoord3fvSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord3fvSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord3fvSGIS_Rec);
}

const GLubyte *__glle_MTexCoord3ivSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord3ivSGIS_Rec *data;

    data = (struct __gllc_MTexCoord3ivSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord3ivSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord3ivSGIS_Rec);
}

const GLubyte *__glle_MTexCoord3svSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord3svSGIS_Rec *data;

    data = (struct __gllc_MTexCoord3svSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord3svSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord3svSGIS_Rec);
}

const GLubyte *__glle_MTexCoord4dvSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord4dvSGIS_Rec *data;

    data = (struct __gllc_MTexCoord4dvSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord4dvSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord4dvSGIS_Rec);
}

const GLubyte *__glle_MTexCoord4fvSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord4fvSGIS_Rec *data;

    data = (struct __gllc_MTexCoord4fvSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord4fvSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord4fvSGIS_Rec);
}

const GLubyte *__glle_MTexCoord4ivSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord4ivSGIS_Rec *data;

    data = (struct __gllc_MTexCoord4ivSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord4ivSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord4ivSGIS_Rec);
}

const GLubyte *__glle_MTexCoord4svSGIS(const GLubyte *PC)
{
    struct __gllc_MTexCoord4svSGIS_Rec *data;

    data = (struct __gllc_MTexCoord4svSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.MTexCoord4svSGIS)(data->target,data->v);
    return PC + sizeof(struct __gllc_MTexCoord4svSGIS_Rec);
}

const GLubyte *__glle_SelectTextureSGIS(const GLubyte *PC)
{
    struct __gllc_SelectTextureSGIS_Rec *data;

    data = (struct __gllc_SelectTextureSGIS_Rec *) PC;
    (*__gl_dispatch.dispatch.SelectTextureSGIS)(data->target);
    return PC + sizeof(struct __gllc_SelectTextureSGIS_Rec);
}

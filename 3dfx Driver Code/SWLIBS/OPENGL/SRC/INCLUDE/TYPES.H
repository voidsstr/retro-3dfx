#ifndef __gl_types_h_
#define __gl_types_h_

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
** Low level data types.
*/

#include <limits.h>
#include <float.h>

typedef struct __GLcontextRec __GLcontext;
#include "os.h"

/*
** Typedefs.  These are forward declarations to internal data structures.
** This eases the burden of dealing with circular references among
** the header files.
*/
typedef GLshort __GLaccumCellElement;
typedef GLubyte __GLstencilCell;
typedef struct __GLaccumBufferRec __GLaccumBuffer;
typedef struct __GLattributeRec __GLattribute;
typedef struct __GLclientAttributeRec __GLclientAttribute;
typedef struct __GLbitmapRec __GLbitmap;
typedef struct __GLbufferRec __GLbuffer;
typedef struct __GLownershipBufferRec __GLownershipBuffer;
typedef struct __GLcolorBufferRec __GLcolorBuffer;
typedef struct __GLdepthBufferRec __GLdepthBuffer;
typedef struct __GLfogMachineRec __GLfogMachine;
typedef struct __GLfragmentRec __GLfragment;
typedef struct __GLlightModelStateRec __GLlightModelState;
typedef struct __GLlightSourceMachineRec __GLlightSourceMachine;
typedef struct __GLlineOptionsRec __GLlineOptions;
typedef struct __GLmaterialMachineRec __GLmaterialMachine;
typedef struct __GLmaterialStateRec __GLmaterialState;
typedef struct __GLmatrixRec __GLmatrix;
typedef struct __GLpixelSpanInfoRec __GLpixelSpanInfo;
typedef struct __GLprocTableRec __GLprocTable;
typedef struct __GLscreenRec __GLscreen;
typedef struct __GLshadeRec __GLshade;
#ifdef __GL_PC_RAST
typedef struct __GLrasterRec __GLrasterMachine;
#endif
typedef struct __GLstencilBufferRec __GLstencilBuffer;
typedef struct __GLstippleRec __GLstipple;
typedef struct __GLtransformRec __GLtransform;
typedef struct __GLvertexRec __GLvertex;
typedef struct __GLdispatchStateRec __GLdispatchState;
typedef struct __GLarenaRec __GLarena;
typedef struct __GLnamesArrayRec __GLnamesArray;


/*
** Type of z value used by the software z buffering code.
** NOTE: must be unsigned
*/
typedef unsigned int __GLzValue;
typedef unsigned short __GLzValue16;

typedef int __GLfixedCoord;/*XXX*/

/************************************************************************/

/*
** Implementation data types.  The implementation is designed to run in
** single precision or double precision mode, all of which is controlled
** by an ifdef and the following typedef's.
**
** Use _GL_MAX_FLOAT to get the largest representable GL_FLOAT
*/
#ifdef	__GL_DOUBLE
typedef double __GLfloat;
#define __GL_MAX_FLOAT DBL_MAX
#else
typedef float __GLfloat;
#define __GL_MAX_FLOAT FLT_MAX
#endif

/************************************************************************/

/*
** Coordinate structure.  Coordinates contain x, y, z and w.
*/
typedef struct __GLcoordRec {
    __GLfloat x, y, z, w;
} __GLcoord;

/*
** Color structure.  Colors are composed of red, green, blue and alpha.
*/
typedef struct __GLcolorRec {
    __GLfloat r, g, b, a;
} __GLcolor;

/*
** Generic no-operation procedure.  Used when function pointers need to
** be stubbed out when an operation is disabled.
*/
extern void __glNop(void);

/*
** Use when an extension isn't implemented, but has an entry in the
** dispatch table because some platform is using the extension.
** Returns an INVALID_OPERATION error.
*/
extern void APIENTRY __glim_UnimplementedExtension(void);

/* The display list version of above */

extern void APIENTRY __gllc_UnimplementedExtension(void);

#endif /* __gl_types_h_ */

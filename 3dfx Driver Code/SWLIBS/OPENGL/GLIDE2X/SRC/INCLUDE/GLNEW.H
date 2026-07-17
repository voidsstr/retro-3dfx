#ifndef __gl_new_h_
#define __gl_new_h_

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
** $Revision: 4$
** $Date: 10/11/00 7:52:18 PM$
*/

/*
** Hacks for instrumenting timing.
*/

/* extra parameters for glCullParameter*SGI */

#define __GL_VARRAY_PRECOMPILE			0x6400
#define __GL_VARRAY_DISABLE_FAST		0x6401
#define __GL_VARRAY_DISABLE_RASTER		0x6402
#define __GL_VARRAY_BATCH_SIZE			0x6403
#define __GL_VARRAY_SAME_BUF			0x6404

#define __GL_VARRAY_DISABLE_VCACHE		0x81AD
#define __GL_VARRAY_SMALL_GLVERTEX		0x81AE
#define __GL_VARRAY_SKIP_VXCULL			0x81AF

#endif /* __gl_new_h_ */

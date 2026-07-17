#ifndef __gl_s3vutil_h_
#define __gl_s3vutil_h_

/*
** Copyright 1991-1997 Silicon Graphics, Inc.
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

GLboolean __glS3VMapRegisters(int x);
GLboolean __glS3VUnmapRegisters(int x);
GLuint __glS3GetRegBase(void);

void __glS3VSetupFor3D(__GLcontext *gc);
void __glS3VCleanupAfter3D(__GLcontext* gc);

#endif  /* __gl_s3vutil_h_ */

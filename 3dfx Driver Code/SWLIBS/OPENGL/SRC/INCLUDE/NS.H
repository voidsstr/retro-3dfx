#ifndef __gl_ns_h_
#define __gl_ns_h_

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
** $Date: 10/11/00 8:05:13 PM$
*/

typedef struct __GLnameSpaceRec {
  GLboolean direct_map;		/* Set to GL_TRUE when we're interfacing
				 * with C 
				 */
  GLboolean open;
  GLuint next;
  GLuint crc;
  GLuint members[1000];
} __GLnameSpace;

GLboolean __glNSOpenSpace(__GLnameSpace*);
void __glNSCloseSpace(__GLnameSpace*);
GLuint __glNSWhereIs(__GLnameSpace*, GLuint, GLuint);
void __glNSAlign(__GLnameSpace* ns, GLuint size);
GLuint __glNSSizeof(__GLnameSpace*);

#endif

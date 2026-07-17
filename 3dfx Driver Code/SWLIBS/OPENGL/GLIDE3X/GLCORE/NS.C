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
** $Revision: 2$
** $Date: 10/11/00 7:55:39 PM$
*/
#ifdef __GL_CODEGEN
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>

#include "context.h"
#include "render.h"
#include "global.h"

GLboolean __glNSOpenSpace(__GLnameSpace* ns)
{
  ns->open = GL_TRUE;
  ns->direct_map = GL_FALSE;
  memset(ns->members, ~0, sizeof(ns->members));
  ns->next = 0;
  ns->crc = 0;
  return GL_TRUE;
}

void __glNSCloseSpace(__GLnameSpace* ns)
{
  ns->open = GL_FALSE;
}

void __glNSAlign(__GLnameSpace* ns, GLuint size)
{
  assert(!ns->direct_map);
  ns->next = (ns->next + (size - 1)) & ~(size - 1);
}

GLuint __glNSWhereIs(__GLnameSpace* ns, GLuint where, GLuint size)
{
  if (ns->direct_map)
    return where;

  if (ns->members[where] == ~0UL) {
    assert(ns->open);

    ns->crc ^= where;
    ns->crc = (ns->crc << 7) ^ (ns->crc >> (32 - 7));

    ns->members[where] = ns->next;
    ns->next += size;
  }

  return ns->members[where];
}

GLuint __glNSSizeof(__GLnameSpace* ns)
{
  assert(!ns->open);
  return ns->next;
}

#endif

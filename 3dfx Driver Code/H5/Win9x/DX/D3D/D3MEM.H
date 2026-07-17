/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name: d3mem.h
**
** Description: dynamic and static texture array macros
**
** $Revision: 2$
** $Date: 10/11/00 8:48:26 PM$
**
**
** $Log: 
**  2    3dfx      1.0.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 4     8/20/99 12:40p Russ
** for W2K, redefine CONTEXT_DEFINE as nothing
**
** 3     8/10/99 10:38a Russ
** for DX7, set TXTRHNDL_DEFINE to nothing
**
** 2     8/05/99 4:16p Russ
** redefine of CONTEXT_DEFINE for RC_LINKED_LIST implementation
**
** 1     6/02/99 6:41a Michael
** Branch from H3
**
** 6     5/26/99 11:02a Cwilcox
** Removed D3DSTATIC_ARRAY code.
**
** 5     1/29/99 10:48a Cshaw
** Added unified headers.
**
** 4     10/21/98 1:48p Jw
** Fixup file end to make compiler happy by putting \r at the end.
**
** 3     7/24/98 1:37p Hohn
 *
 * 2     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#ifndef __D3MEM_H__
#define __D3MEM_H__

#include "d3global.h"

#if RC_LINKED_LIST
  // dynamically allocated linked list
#ifdef WINNT
  #define CONTEXT_DEFINE()
#else
  #define CONTEXT_DEFINE()\
  RC    *pContexts;
#endif
#else
  // dynamic array
  #define CONTEXT_DEFINE()\
  BOOL  *contextMap;\
  RC    *contexts;\
  DWORD  numContexts;
#endif  // RC_LINKED_LIST

  #define   TXTRDESC_DEFINE() \
  TXTRDESC *txtrDesc;\
  DWORD     numTextureDescs;

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // for DX7, TXTRHNDL's are allocated as part of the HNDLLIST
  #define  TXTRHNDL_DEFINE()
#else
  #define  TXTRHNDL_DEFINE() \
  TXTRHNDL *txtrHndl;\
  DWORD     numTextureHndls;
#endif

#endif //__D3MEM_H__

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
** File name:   d3contxt.h
**
** Description: Used for allocating and freeing contexts
**
** $Revision: 4$
** $Date: 10/25/00 4:57:01 AM$
**
** $Log: 
**  4    3dfx      1.0.1.0.1.1 10/25/00 Johnny Trainor  Updated so we no longer use
**       surface local pointers.
**  3    3dfx      1.0.1.0.1.0 10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.0.1.0     06/02/00 Russ Lind       removed non RC_LINKED_LIST
**       code
**       modified dynamic memory allocation code so MEMCHECK tracking can be
**       enabled
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 6     8/20/99 12:39p Russ
** in RC_LINKED_LIST code, replace _D3(pContexts) with g_pContexts
** define g_pContexts as _D3(pContexts) for win9x
** removed RC_LINKED_LIST version of CONTEXT_INIT_ARRAY function
**
** 5     8/10/99 10:36a Russ
** bump up debug output level in RC_LINKED_LIST code
**
** 4     8/05/99 4:11p Russ
** linked list implementation for D3D contexts (changes wrapped in #if
** RC_LINKED_LIST)
**
** 3     6/17/99 2:46p Tlittle
** removed un-needed #if check
**
** 2     6/02/99 3:49p Edwin
** Cleanup "if defined(MM) || (defined(WINNT) && _WIN32_WINNT >= 0x0500)",
** multi-monitor is always on. Note, this file is not used by NT4.
**
** 1     6/02/99 6:40a Michael
** Branch from H3
**
** 20    5/30/99 5:58p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
**
** 19    5/26/99 10:59a Cwilcox
** Removed D3DSTATIC_ARRAY code.
**
** 18    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 17    1/26/99 5:27p Peterm
** Added unified header information
**
** 16    12/09/98 7:21a Russ
** NT5 D3D changes for Banshee
**
** 15    12/02/98 12:14p Andrew
** changed error handle to branch out early for MultiMon
**
** 14    12/01/98 10:40a Andrew
** Removed a handle null check since this is now a legitmate internal
** value.
**
** 13    11/30/98 12:52a Andrew
** Fixed a problem where I did not convert a handle which is really a
** pointer into a index which causes us to run out of rc's.
**
** 12    11/22/98 8:55p Andrew
** Changes to support multi-monitor
**
** 11    10/17/98 10:56a Russ
** made inline functions portable between win9x and NT
** changed HeapAlloc and HeapFree to DXMALLOCZ and DXFREE
**
** 10    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
**
** 9     7/24/98 1:37p Hohn
**
** 8     5/06/98 6:07p Adrians
** Changes for DX6 into DX5 driver.
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 7     1/21/98 4:04p Miriam
** Changes that allow D3D to be shared between Voodoo2 & Banshee.
 *
 * 6     11/25/97 4:23p Suninn
 * add #include "h3g.h" for H3
 *
 * 5     11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 *
 * 4     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#ifndef __D3CONTXT_H__
#define __D3CONTXT_H__

#include "d3global.h"
#ifndef WINNT
#include "h3g.h"
#include "ddfxnt95.h"
#endif

// linked list implementation
#define CONTEXT_INVALID  0
#define CONTEXT_PTR(handle) ((RC *)handle)

#define CONTEXT_ALLOC(arg1) context_alloc(arg1)
#define CONTEXT_FREE(arg1,arg2)  context_free(arg1,arg2)
#define CONTEXT_FREE_PID(arg1,arg2) context_free_pid(arg1,arg2)
#define CONTEXT_INIT_ARRAY(arg1) context_init_array(arg1)

// easier to debug functions
#ifdef  KILLERDEBUG
#undef  fxINLINE
#undef  fxCDECL
#define fxINLINE
#define fxCDECL  __stdcall
extern  fxINLINE DWORD fxCDECL CONTEXT_ALLOC(NT9XDEVICEDATA *);
extern  fxINLINE void  fxCDECL CONTEXT_FREE(NT9XDEVICEDATA *, DWORD);
extern  fxINLINE void  fxCDECL CONTEXT_FREE_PID(NT9XDEVICEDATA *, DWORD, DWORD);
extern  fxINLINE BOOL  fxCDECL CONTEXT_INIT_ARRAY(NT9XDEVICEDATA*);

// inline macros
#else
#define fxINLINE __inline
#define fxCDECL
#endif

#ifdef WINNT
// due to calling sequence on W2K, we must have the head of
// the linked list as a global variable
extern RC *g_pContexts;
#else
#define g_pContexts   _D3(pContexts)
#endif

/*-------------------------------------------------------------------
Function Name:  CONTEXT_ALLOC

Description:    Allocate a context and insert it at the head of
                a linked list.

Return:         (DWORD) handle
-------------------------------------------------------------------*/
fxINLINE DWORD fxCDECL CONTEXT_ALLOC(NT9XDEVICEDATA *ppdev)
{
  RC  *pRc;


  pRc = (RC *)D3DMALLOCZ(sizeof(RC), 0);

  if (NULL != pRc)
  {
#ifdef WINNT
    D3DPRINT(D3DDBGLVL, "  allocated RC = %8lXh", pRc);
#endif

    // insert new RC * at head of linked list
    if (NULL != g_pContexts)
    {
      pRc->pNext = g_pContexts;
      UPDATE_BLOCK_DATA(g_pContexts, &pRc->pNext);
      g_pContexts = pRc;
      UPDATE_BLOCK_DATA(pRc, &g_pContexts);
#ifdef WINNT
      D3DPRINT(D3DDBGLVL, "  inserting at head of list, g_pContexts=%8lXh, pRc->pNext=%8lXh",
               g_pContexts, pRc->pNext);
#endif
    }
    else
    {
      pRc->pNext = NULL;
      g_pContexts = pRc;
      UPDATE_BLOCK_DATA(pRc, &g_pContexts);
#ifdef WINNT
      D3DPRINT(D3DDBGLVL, "  adding to empty list, g_pContexts=%8lXh, pRc->pNext=%8lXh",
               g_pContexts, pRc->pNext);
#endif
    }
  }
  else
  {
#ifdef WINNT
    D3DPRINT(D3DDBGLVL, "  unable to allocate RC");
#endif
  }

#if defined(WINNT) && DBG
  // dump RC linked list
  {
    RC  *pRcList2 = (RC *)&g_pContexts;

    D3DPRINT(D3DDBGLVL, "CONTEXT_ALLOC - Current RC list");
    while (pRcList2->pNext)
    {
      D3DPRINT(D3DDBGLVL, "  pNext = %8lXh", pRcList2->pNext);
      pRcList2 = pRcList2->pNext;
    }
  }
#endif

  return (DWORD)pRc;
}

/*-------------------------------------------------------------------
Function Name:  CONTEXT_FREE

Description:    Remove a context from the linked list and free it

Return:         void
-------------------------------------------------------------------*/
fxINLINE void fxCDECL CONTEXT_FREE(NT9XDEVICEDATA *ppdev, DWORD handle)
{
  RC  *pRc = (RC *)handle;
  RC  *pRcList = (RC *)&g_pContexts;


#ifdef WINNT
  ASSERTDD(pRc != NULL, "can't free a NULL RC");
#endif

  // search linked list to find this RC
  while (pRcList->pNext)
  {
    if (pRc == pRcList->pNext)
    {
      // found the RC to be freed
      pRcList->pNext = pRc->pNext;
#ifdef MEMCHECK
      UPDATE_BLOCK_DATA(pRc, 0);
      if (pRc->pNext)
        UPDATE_BLOCK_DATA(pRc->pNext, &pRcList->pNext);
#endif

#ifdef WINNT
      D3DPRINT(D3DDBGLVL, "  after unlinking RC=%8lXh, pRcList=%8lXh, pRcList->pNext=%8lXh",
               pRc, pRcList, pRcList->pNext);
#endif

      pRc->pNext = NULL;
      D3DFREE(pRc);

/*
#if defined(WINNT) && DBG
      // dump RC linked list
      {
        RC  *pRcList2 = (RC *)&g_pContexts;
      
        D3DPRINT(D3DDBGLVL, "CONTEXT_FREE - Current RC list");
        while (pRcList2->pNext)
        {
          D3DPRINT(D3DDBGLVL, "  pNext = %8lXh", pRcList2->pNext);
          pRcList2 = pRcList2->pNext;
        }
      }
#endif
*/
      return;
    }
    pRcList = pRcList->pNext;
  }

#ifdef WINNT
  D3DPRINT(D3DDBGLVL, "  unable to locate RC = %8lXh", pRc);
  ASSERTDD(pRc == NULL, "the RC to be released was not found!");
#endif
}

#ifndef WINNT
/*-------------------------------------------------------------------
Function Name:  CONTEXT_FREE_PID

Description:    Free all contexts associated with passed in process
                id

Return:         void
-------------------------------------------------------------------*/
fxINLINE void fxCDECL CONTEXT_FREE_PID(NT9XDEVICEDATA * ppdev, DWORD freePid)
{
  RC  *pRc;
  RC  *pRcList = (RC *)&g_pContexts;


  // search linked list to find any RC's for this pid
  while (pRcList->pNext)
  {
    if (freePid == pRcList->pNext->pid)
    {
      // found the RC to be freed
      pRc = pRcList->pNext;
      pRcList->pNext = pRc->pNext;
      pRc->pNext = NULL;
      D3DFREE(pRc);
    }
    else
      pRcList = pRcList->pNext;
  }
}
#endif

#define CONTEXT_VALIDATE( handle)\
  (NULL == (void *)(handle)) // invalid context

#endif //__D3CONTXT_H__

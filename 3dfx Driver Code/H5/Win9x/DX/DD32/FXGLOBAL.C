/* $Header: fxglobal.c, 2, 10/11/00 8:52:09 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File Name:	FXGLOBAL.C
**
** Description: Memory Manager heap management functions.
**              Command FIFO debug functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:52:09 PM$
**
** $History: fxglobal.c $
** 
** *****************  Version 4  *****************
** User: Adrians      Date: 2/23/99    Time: 8:24p
** Updated in $/devel/h3/Win95/dx/dd32
** Remove some redundant Voodoo2 code.  This file is not shared with
** Voodoo2.
** 
** *****************  Version 3  *****************
** User: Michael      Date: 12/31/98   Time: 7:34a
** Updated in $/devel/h3/Win95/dx/dd32
** Implement the 3Dfx/STB unified header.
**
*/

#ifndef MM
#define  FX_DEFINE_MACROS 1
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3global.h"
#include "fxpci.h"

fxGlobal _fxGlobal;

/*----------------------------------------------------------------------
Function name: MemInit

Description:   Initailize Memory Manager

Return:        BOOL
               
			   TRUE  - memory allocated and initailized
			   FALSE - unable to allocate memory
----------------------------------------------------------------------*/

BOOL _stdcall MemInit (void)
{
  // First Time ?!?
  if (GLOBAL_MEM_INIT == 0L)
  {

#if defined (CVG) 
    GLOBAL_MEM_HEAP_PTR = HeapCreate (HEAP_SHARED, HEAP_INIT_SIZE, 0 );
#else
#ifndef MM
    GLOBAL_MEM_HEAP_PTR = HeapCreate (HEAP_SHARED, HEAP_INIT_SIZE, 0 );
#endif
#endif
    if (GLOBAL_MEM_HEAP_PTR == NULL)
    {
      // Failure
      return FALSE;
    }
  }

  // Increment Semaphore
  GLOBAL_MEM_INIT++;

  // Success
  return TRUE;
} // End MemInit

/*----------------------------------------------------------------------
Function name: MemFini

Description:   Cleanup Memory Manager

Return:        BOOL
               
			   TRUE  - memory released
			   FALSE - no memory to release
----------------------------------------------------------------------*/

BOOL _stdcall MemFini(void)
{
  // Check for programmer error
  if (GLOBAL_MEM_INIT == 0L)
    return FALSE;

  // Decrement Semaphore
  GLOBAL_MEM_INIT--;

  // Last time ?!?
  if (GLOBAL_MEM_INIT == 0L)
  {
    // Cleanup
    if (GLOBAL_MEM_HEAP_PTR)
    {
      HeapDestroy (GLOBAL_MEM_HEAP_PTR);
      GLOBAL_MEM_HEAP_PTR = NULL;
    }
  }

  // Success
  return TRUE;
} // End MemFini
#endif


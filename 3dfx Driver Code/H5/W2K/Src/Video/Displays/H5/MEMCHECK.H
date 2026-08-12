/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Revision: 2$
** $Date: 10/11/00 8:44:26 PM$
**
*/

#ifndef _MEMCHECK_H_
#define _MEMCHECK_H_

/***************************************************************************
* D E F I N E S
****************************************************************************/

#ifdef MEMCHECK

#define CHECK_MEMORY_INTEGRITY()        CheckMemoryIntegrity()
#define UPDATE_BLOCK_DATA(p,n)          UpdateBlockData((p),(DWORD)(n))

#define ENGALLOCMEM(fl,cj,tag,dwData)   AllocMem((fl),(cj),(tag),__FILE__,__LINE__,(DWORD)(dwData))
#define ENGFREEMEM(p)                   FreeMem((PVOID)(p))

#else

#define CHECK_MEMORY_INTEGRITY()
#define UPDATE_BLOCK_DATA(p,n)

#if ENABLE_LOG_FILE
/* retro3dfx: route every driver pool alloc/free through the counting wrappers
   (LOGFILE.C) so the device-cycle leak hunt sees the outstanding-allocation
   balance. Behaviourally identical to EngAllocMem/EngFreeMem. */
#define ENGALLOCMEM(fl,cj,tag,dwData)   retroEngAllocMem((fl),(cj),(tag))
#define ENGFREEMEM(p)                   retroEngFreeMem((PVOID)(p))
#else
#define ENGALLOCMEM(fl,cj,tag,dwData)   EngAllocMem((fl),(cj),(tag))
#define ENGFREEMEM(p)                   EngFreeMem((p))
#endif

#endif

/***************************************************************************
* F U N C T I O N   P R O T O T Y P E S
****************************************************************************/

#ifdef MEMCHECK

extern void CheckMemoryIntegrity(void);
extern void UpdateBlockData(PVOID p, DWORD dwNewData);

extern PVOID AllocMem(ULONG fl, ULONG cj, ULONG tag, const char *pszFileName, ULONG ulLineNo, DWORD dwData);
extern VOID  FreeMem(PVOID pv);

#endif

#endif /* _MEMCHECK_H_ */
/* Don't write below this endif */


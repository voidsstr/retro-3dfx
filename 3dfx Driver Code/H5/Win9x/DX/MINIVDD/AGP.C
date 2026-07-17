/* -*-c++-*- */
/* $Header: agp.c, 3, 10/11/00 8:53:48 PM, Brent$ */
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   agp.c
**
** Description: AGP and VGART related functions.
**
** $Revision: 3$
** $Date: 10/11/00 8:53:48 PM$
**
** $History: agp.c $
** 
** *****************  Version 4  *****************
** User: Michael      Date: 3/08/99    Time: 5:01p
** Updated in $/devel/h3/Win95/dx/minivdd
** In AgpService, #ifdef DEBUG around _Debug_Printf_Service.  These were
** on all the time including the retail build.
** 
** *****************  Version 3  *****************
** User: Michael      Date: 1/04/99    Time: 1:19p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 2  *****************
** User: Ken          Date: 7/23/98    Time: 4:04p
** Updated in $/devel/h3/win95/dx/minivdd
** added agp command fifo.   not added to NT build.  currently not
** functional on non-win98 systems without running a special enable apg
** script first, see Ken for that.   agp command fifo is enabled by
** setting the environment variable ACF=1 , all other settings disable it.
** Only turned on interatively in debugger in InitFifo call.   
** 
** *****************  Version 1  *****************
** User: Ken          Date: 7/23/98    Time: 2:09p
** Created in $/devel/h3/win95/dx/minivdd
** module for mapping GART and enabling AGP access
**
*/

#define WANTVXDWRAPS

#include <basedef.h>
#include <vmm.h>
#include <debug.h>
#include <vxdwraps.h>
#include <vwin32.h>
#include <winerror.h>
#include <configmg.h>
#include <pci.h>

#include "3dfx.h"
#define VDDONLY
#include "h3g.h"
#undef  VDDONLY

// Fix for building with Windows 98 DDK (VxDWrapper for GART services)
#undef  _GARTReserve
#undef  _GARTCommit
#undef  _GARTUnCommit
#undef  _GARTFree

/*----------------------------------------------------------------------
Function name:  VgartD_Reserve

Description:    

Information:    This function is all _asm.

Return:         DWORD   result of the VMMCall to reserve GART.
----------------------------------------------------------------------*/
DWORD VXDINLINE
VgartD_Reserve(DWORD devObj,
	       DWORD numPages,
	       DWORD alignMask,
	       DWORD pGARTDev,
	       DWORD flags)
{
    DWORD retval;

    __asm pushad;
    
    __asm push flags;
    __asm push pGARTDev;
    __asm push alignMask;
    __asm push numPages;
    __asm push devObj;
    VMMCall(_GARTReserve);
    __asm mov  retval, eax;
    __asm add  esp, 5*4;

    __asm popad;

    return(retval);
}


/*----------------------------------------------------------------------
Function name:  VgartD_Commit

Description:    

Information:    This function is all _asm.

Return:         DWORD   result of the VMMCall to commit GART.
----------------------------------------------------------------------*/
DWORD VXDINLINE
VgartD_Commit(DWORD gartLinAddr,
	      DWORD pageOffset,
	      DWORD numPages,
	      DWORD pGARTDev,
	      DWORD flags)
{
    DWORD retval;

    __asm pushad;
    
    __asm push flags;
    __asm push pGARTDev;
    __asm push numPages;
    __asm push pageOffset;
    __asm push gartLinAddr;
    VMMCall(_GARTCommit);
    __asm mov  retval, eax;
    __asm add  esp, 5*4;

    __asm popad;

    return(retval);
}


/*----------------------------------------------------------------------
Function name:  VgartD_Uncommit

Description:    

Information:    This function is all _asm.

Return:         VOID
----------------------------------------------------------------------*/
void VXDINLINE
VgartD_Uncommit(DWORD gartLinAddr,
		DWORD pageOffset,
		DWORD numPages)
{
    __asm pushad;
    
    __asm push numPages;
    __asm push pageOffset;
    __asm push gartLinAddr;
    VMMCall(_GARTUnCommit);
    __asm add  esp, 3*4;

    __asm popad;
}


/*----------------------------------------------------------------------
Function name:  VgartD_Free

Description:    

Information:    This function is all _asm.

Return:         VOID
----------------------------------------------------------------------*/
void VXDINLINE
VgartD_Free(DWORD gartLinAddr)
{
    __asm pushad;
    
    __asm push gartLinAddr;
    VMMCall(_GARTFree);
    __asm add  esp, 1*4;

    __asm popad;
}


/*----------------------------------------------------------------------
Function name:  AgpService

Description:    This function handles the RESERVE, COMMIT, UNCOMMIT,
                and FREE requests for AGP.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
AgpService(AgpSrvc *pAs)
{
    switch (pAs->service)
    {
      case RESERVE:
#ifdef DEBUG
	  _Debug_Printf_Service("H3VDD: GartReserve: ");
#endif
	  pAs->params.reserve.gartLinAddr = 
	      VgartD_Reserve(pAs->params.reserve.devNode,
			     pAs->params.reserve.nPages,
			     0,	// 4K alignment of start address
			     (DWORD) & pAs->params.reserve.gartPhysAddr,
			     PG_WRITECOMBINED);	// get USWC memory

#ifdef DEBUG
	  _Debug_Printf_Service("NP:0x%lx, LA:0x%08lx, GA:0x%08lx, %s\n\r",
				pAs->params.reserve.nPages,
				pAs->params.reserve.gartLinAddr,
				pAs->params.reserve.gartPhysAddr,
				((pAs->params.reserve.gartLinAddr != 0) ?
				                  "Succeeded" : "FAILED"));
#endif
	  break;
	  
      case COMMIT:
#ifdef DEBUG
	  _Debug_Printf_Service("H3VDD: GartCommit: ");
#endif

	  pAs->params.commit.retval = 
	      VgartD_Commit(pAs->params.commit.gartLinAddr,
			    pAs->params.commit.pageOffset,
			    pAs->params.commit.nPages,
			    (DWORD) &  pAs->params.commit.gartPhysAddr,
			    0);	// don't memfill(0) memory

#ifdef DEBUG
	  _Debug_Printf_Service("LA:0x%08lx, PO:0x%lx, NP:0x%lx, GA:0x%08lx, "
				"%s\n\r",
				pAs->params.commit.gartLinAddr,
				pAs->params.commit.pageOffset,
				pAs->params.commit.nPages,
				pAs->params.commit.gartPhysAddr,
				((pAs->params.commit.retval == 1) ?
				                   "Succeeded" : "FAILED"));
#endif
	  break;
	  
      case UNCOMMIT:
#ifdef DEBUG
	  _Debug_Printf_Service("H3VDD: GartUncommit: ");
#endif

	  VgartD_Uncommit(pAs->params.uncommit.gartLinAddr,
			  pAs->params.uncommit.pageOffset,
			  pAs->params.uncommit.nPages);

#ifdef DEBUG
	  _Debug_Printf_Service("LA:0x%08lx, PO:0x%lx, NP:0x%lx\n\r",
				pAs->params.uncommit.gartLinAddr,
				pAs->params.uncommit.pageOffset,
				pAs->params.uncommit.nPages);
#endif
	  break;
	  
      case FREE:
#ifdef DEBUG
	  _Debug_Printf_Service("H3VDD: GartFree: ");
#endif

	  VgartD_Free(pAs->params.free.gartLinAddr);

#ifdef DEBUG
	  _Debug_Printf_Service("LA: 0x%08lx\n\r",
				pAs->params.free.gartLinAddr);
#endif
	  break;
    }
}


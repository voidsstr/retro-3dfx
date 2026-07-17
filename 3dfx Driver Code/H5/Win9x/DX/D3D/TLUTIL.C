/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** File name: tlutil.c
**
** Description: Utility functions for the T&L HAL
**
** $Revision: 4$
** $Date: 10/11/00 8:49:36 PM$
**
** $Log: 
**  4    3dfx      1.2.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    Napalm Shared1.2         10/29/99 Scott Kephart   Fixed bug in AB32_Grow()
**       -- previously, we padded the size of the allocation by 31 bytes, so that
**       we could allign to a 32 byte boundary. However, we always just added 31 to
**       the allocated size, causing an overrun of the end of the buffer when it
**       *wasn't* aligned to start with. (Because we used part of the extra 31
**       bytes at the start, to align the buffer, but always assumed 31 extra bytes
**       on the end.)
**  2    Napalm Shared1.1         10/27/99 Russ Lind       disabled _matherr on
**       WINNT builds, since it conflicts with the definition in nt5ddk\inc\math.h
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
** 
** 7     10/26/99 12:48a Skephart
** Added #ifdef TnL_HAL
** 
** 6     10/21/99 12:48a Skephart
** Added exception handler from JoeP
** 
** 5     9/27/99 7:54p Skephart
** Cleanup
** 
** 4     9/24/99 2:26p Skephart
** 
** 3     9/16/99 11:13a Skephart
** Cleanup
*/

#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL

#ifndef WINNT
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6global.h"
#include "d3contxt.h"
#endif

#include "dxins.h"
#include "d7fvfext.h"


#ifndef WINNT
/* The function belows catches Exceptions caused by certain operations in the CRT */

int _matherr( struct _exception *pExcept )
{
	if( pExcept->type == _DOMAIN )
	{
	   if( strcmp( pExcept->name, "log" ) == 0 )
	   {
	      D3DPRINT(0,"Handled _DOMAIN CRT MathError Name = log");
	      pExcept->retval = log( -(pExcept->arg1) );
	      return 1;
	   }
	   else if( strcmp( pExcept->name, "log10" ) == 0 )
	   {
	      D3DPRINT(0,"Handled _DOMAIN CRT MathError Name = log10");
	      pExcept->retval = log10( -(pExcept->arg1) );
	      return 1;
	   }
		else if( strcmp( pExcept->name, "sqrt" ) == 0 )
		{
	      D3DPRINT(0,"Handled _DOMAIN CRT MathError Name = sqrt");
		   pExcept->retval = (double)1.0;
		   return 1;
		}
		else
		{
	   	D3DPRINT(0,"Unhandled _DOMAIN CRT MathError Name = %s",pExcept->name); 
		   return 0;
		}
	}
	else
	{
	   D3DPRINT(0,"Unhandled CRT MathError Type = %d, Name = %s", 
	      pExcept->type, pExcept->name);
	   return 0;    /* Else use the default actions */
	}
}
#endif

/*-----------------------------------------------------------------**
** These functions take the place of the RefAlignedBuffer32 class. **
**-----------------------------------------------------------------*/


/*-------------------------------------------------------------------
Function Name:  AB32_Create
Description:    Makes an AlignedBuffer32 struct ready for allocation. 
                This function does not actually allocate any memory -- 
                you have to call AB32_Grow for that.
Parameters:     
                NT9XDEVICEDATA *ppdev -- our global data. We need this 
                                to get the handle to the heap
                AlignedBuffer32 *Buff -- Buffer to initialize
Information:    
Return:         
-------------------------------------------------------------------*/

void AB32_Create(NT9XDEVICEDATA *ppdev, AlignedBuffer32 *Buff)
{
  Buff->allocatedBuf = 0;
  Buff->alignedBuf = 0;
  Buff->size = 0;
}  

/*-------------------------------------------------------------------
Function Name:  AB32_Destroy
Description:    Free the memory associated with an AlignedBuffer32 
                struct
Parameters:     
                NT9XDEVICEDATA *ppdev -- our global data. We need this 
                                to get the handle to the heap
                AlignedBuffer32 *Buff -- Buffer to free
Information:    
Return:         
-------------------------------------------------------------------*/

void AB32_Destroy(NT9XDEVICEDATA *ppdev, AlignedBuffer32 *Buff)
{
  if (Buff->allocatedBuf)
    DXFREE(Buff->allocatedBuf);
  Buff->size = 0;
  Buff->alignedBuf = 0;
  Buff->allocatedBuf = 0;
}  

/*-------------------------------------------------------------------
Function Name:  AB32_Grow
Description:    Allocate some additional memory for a buffer. If a buffer 
                already had been allocated,it's freed first, and then a 
                new, larger buffer is allocated. The contents of the buffer 
Parameters:     
                NT9XDEVICEDATA *ppdev -- our global data. We need this 
                                to get the handle to the heap
                AlignedBuffer32 *Buff -- Buffer to free
                DWORD dwSize -- amount of additional memory to allocate
Information:    
Return:         
                S_OK -- success
                DDERR_OUTOFMEMORY -- failure
-------------------------------------------------------------------*/

HRESULT AB32_Grow(NT9XDEVICEDATA *ppdev, AlignedBuffer32 *Buff, DWORD dwSize)
{
  if (Buff->allocatedBuf)
    DXFREE(Buff->allocatedBuf);

  if ((Buff->allocatedBuf = DXMALLOC(dwSize + 31)) == NULL)
  {
    Buff->allocatedBuf = 0;
    Buff->alignedBuf = 0;
    Buff->size = 0;
    return DDERR_OUTOFMEMORY;
  }
  Buff->alignedBuf = (LPVOID)(((ULONG_PTR)Buff->allocatedBuf  + 31 ) & ~31);
  Buff->size = dwSize;    //yes, we potentially waste a small amount of 
                          //space on the end of the buffer
  return S_OK;
}

/*-------------------------------------------------------------------
Function Name:  AB32_CheckAndGrow
Description:    Allocate some additional memory for a buffer, if the 
                buffer is smaller than the requested size.
Parameters:     
                NT9XDEVICEDATA *ppdev -- our global data. We need this 
                                to get the handle to the heap
                AlignedBuffer32 *Buff -- Buffer to free
                DWORD dwSize -- amount of additional memory to allocate
Information:    
Return:         
                S_OK -- success
                DDERR_OUTOFMEMORY -- failure
-------------------------------------------------------------------*/

HRESULT AB32_CheckAndGrow(NT9XDEVICEDATA *ppdev, AlignedBuffer32 *Buff, DWORD dwSize)
{
  if (dwSize > Buff->size)
    return AB32_Grow(ppdev, Buff, dwSize + 1024);
  else
    return S_OK;

}  


#endif //TnL_HAL
#endif

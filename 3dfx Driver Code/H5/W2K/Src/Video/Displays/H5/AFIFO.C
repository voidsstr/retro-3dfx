/*
** Copyright (c) 1998, 1999, 3Dfx Interactive, Inc.
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
** File name:   afifo.c
**
** Description: command-fifo functions
**
** $Revision: 2$
** $Date: 10/11/00 8:42:52 PM$
**
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 19    8/14/99 9:39a Cwilcox
** Removed confusing comment.
** 
** 18    7/09/99 4:33p Bseitsin
** Backwards compatability changes.
** 
** 16    5/30/99 5:31p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
** 
** 15    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 14    2/11/99 8:27a Stb_skephart
** 
** 13    1/30/99 3:56p Peterm
** Added unified header information
*/

#include "precomp.h"

#ifndef WINNT
#include <d3dhal.h>
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d6global.h"
#endif

// STB-SK 01/30/99 -- KNI changes
#if (STBKNI==1)
extern ASMFifoData _asm_data;
#else
ASMFifoData _asm_data;
#endif

/*-------------------------------------------------------------------
Function Name:  fifo_make_room_asm (if WinNT defined)

Description:    Makes room in the command fifo

Return:         void
-------------------------------------------------------------------*/
#ifdef WINNT
void __cdecl fifo_make_room_asm(FxU32 size)
{
  SETUP_PPDEV(_asm_data.pRc)
  RC * pRc = _asm_data.pRc;
  ASM_SYNC();
  {
    CMDFIFO_PROLOG(cmdFifo);
    CMDFIFO_CHECKROOM(cmdFifo, size);
    CMDFIFO_EPILOG(cmdFifo);
  }
  ASM_SET();
}
#else
/*-------------------------------------------------------------------
Function Name:  fifo_make_room_asm

Description:    Makes room in the command fifo

Return:         void
-------------------------------------------------------------------*/
void fifo_make_room_asm(FxU32 size)
{
   SETUP_PPDEV(_asm_data.pRc)
   RC * pRc = _asm_data.pRc;

   ASM_SYNC();
   CMDFIFOSPACE=0;
#ifdef CMDFIFO
   {
      CMDFIFO_PROLOG(cmdFifo);
      fifo_MakeRoom(ppdev, &cmdFifo,size);
      CMDFIFO_EPILOG(cmdFifo);
   }
#endif // #ifdef CMDFIFO

   ASM_SET();
}
#endif // not WINNT


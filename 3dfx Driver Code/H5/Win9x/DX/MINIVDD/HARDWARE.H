/* -*-c++-*- */
/* $Header: hardware.h, 2, 10/11/00 8:54:36 PM, Brent$ */
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
** File name:   hardware.c
**
** Description: Structures and macors for misc. HW specific functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:36 PM$
**
** $History: hardware.h $
** 
** *****************  Version 15  *****************
** User: Michael      Date: 1/08/99    Time: 1:51p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 14  *****************
** User: Ken          Date: 6/02/98    Time: 10:42a
** Updated in $/devel/h3/win95/dx/minivdd
** removed obsolete macros
** 
** *****************  Version 13  *****************
** User: Michael      Date: 5/15/98    Time: 4:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Backed out my previous changes.
** 
** *****************  Version 12  *****************
** User: Michael      Date: 5/15/98    Time: 9:22a
** Updated in $/devel/h3/Win95/dx/minivdd
** Modify ResetInvariantState to pass in lpDst.
** 
** *****************  Version 11  *****************
** User: Ken          Date: 4/24/98    Time: 1:33p
** Updated in $/devel/h3/win95/dx/minivdd
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** *****************  Version 10  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

//: hardware.h


#ifndef __HARDWARE_H__
#define __HARDWARE_H__


#include "3dfx.h"
#include "h3defs.h"
#include "h3gdefs.h"
#include "h3regs.h"
#include "cmddefs.h"

#if 0
extern SstCRegs* lpCRegs;
extern SstGRegs* lpGRegs;
extern SstIORegs* lpIOregs;
#endif

#define HARDWARE_ADDR  0x10000000

// CSIM access

extern DWORD pfnCSIM;

// FIFO tracking
#if 0
typedef struct
{
    DWORD base;             // frame buffer fifo base
    DWORD size;             // frame buffer fifo size
    DWORD start;            // virtual address of fifo base
    DWORD next;             // virtual address of next fifo entry
    DWORD roomToEnd;        // how much room is left until the end
    DWORD roomToRdPtr;      // how much room is left before we wait
    DWORD lastRdPtr;        // the last known position of the read ptr
    DWORD outOfOrder;       // set to one
}
Fifo;
extern Fifo fifo;
#endif

// hardware access

VOID h3Write( DWORD, DWORD );
DWORD h3Read( DWORD );

#define DUMMY                   0

VOID InitFifo( DWORD fifoBase, DWORD fifoSize );
VOID CheckFifo( DWORD numEntries );

#define RESET_DST	BIT(0)
#define RESET_SRC	BIT(1)
#define RESET_PDEV	BIT(2)

void ResetInvariantState(FxU32 resetMask);

#define RELOAD 1
#define NO_RELOAD 1

/* Use dib cuz BUSY is set
 * Didn't set BUSY so don't clear BUSY.
 * Jumping from BTS so no cmdfifo ops have 
 * been done so no epilog is needed since 
 * fifo ptr hasn't been incremented */
#define BUSY_IS_SET(retval)\
Use_DibEng:   \
   bPunt=TRUE;\
   return retval;

/* no nothing case. just return */
#define NO_WORK(retval)\
No_Work:   \
   return retval;


#endif

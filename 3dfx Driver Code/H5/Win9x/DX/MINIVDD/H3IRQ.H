/* -*-c++-*- */
/* $Header: h3irq.h, 2, 10/11/00 8:54:24 PM, Brent$ */
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
** File name:   h3irq.h
**
** Description: Macros, exported functions, etc. for IRQ handler.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:24 PM$
**
** $History: h3irq.h $
** 
** *****************  Version 9  *****************
** User: Xingc        Date: 3/04/99    Time: 6:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add register vidProcCfg offset
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 1/21/99    Time: 5:47p
** Updated in $/devel/h3/Win95/dx/minivdd
** Cleanup of EnableInterrupt/DisableInterrupt
** 
** *****************  Version 7  *****************
** User: Agus         Date: 1/15/99    Time: 4:09p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added H4_VID_IN_STATUS_CURLINE define
** 
** *****************  Version 6  *****************
** User: Michael      Date: 1/07/99    Time: 1:28p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 5  *****************
** User: Agus         Date: 6/30/98    Time: 4:58p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added defines used in the manual overlay flip section
** 
** *****************  Version 4  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

/*
 * (c) 1996 Microsoft Inc.
 *
 * mgaints.h - interrupt functions etc.
 */

// all locked

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG
#if 0
extern HIRQ    hIRQ;           // IRQ handle
#endif

// exported functions

BOOL InitializeInterrupts( int iIRQ, PDEVTABLE pDev );      // connect to interrupt
VOID InterruptsToDOS( VOID );               // reset default behaviour for DOS vm
VOID InterruptsToSVM( VOID );               // reconnect to system VM
VOID InterruptHandler( VOID );              // handler
VOID SignalVMEvent( VOID );                 // event fire routine
VOID EnableInterrupts (PDEVTABLE pDevTable);
VOID DisableInterrupts (PDEVTABLE pDevTable);

// misc.

#define IPEND_MASK ( softrapen_INT_OCCURRED |   \
                     pickpen_INT_OCCURRED   |   \
                     vsyncpen_INT_OCCURRED      )

#define H3_VID_PROC_CFG		0x5c
#define H3_VID_IN_STATUS	0x74
#define H3_VID_IN_ADDR0		0xec
#define H3_VID_IN_ADDR1		0xf0
#define H3_VID_IN_ADDR2		0xf4
#define H3_VID_IN_STRIDE	0xf8
#define H3_CUR_OVL_ADDR		0xfc

#define H4_VID_IN_STATUS_CURLINE 0x94

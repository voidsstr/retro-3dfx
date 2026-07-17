/* -*-c++-*- */
/* $Header: time.h, 2, 10/11/00 8:55:19 PM, Brent$ */
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
** File name:   time.h
**
** Description: support for CPU independent time functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:55:19 PM$
**
** $History: time.h $
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/15/99    Time: 9:43a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 10/30/98   Time: 7:30a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a new prototype
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#ifndef _TIME_H_
#define _TIME_H_

/********************************************************************************
*
* The file time.h implements the CPU independent time functions needed for
* DDC and I2C.  This routines expect that the CPU will be a Pentium or better
*
*  GetTime -- returns the the Pentium Cycle Counter
*  TotalTime -- Does a 64 bit subtraction {assumes Start < End}
*  CycleTime -- returns the number of nanoseconds per cycle
*  WaitTime -- wait time in nanoseconds
*
*********************************************************************************/
// Defined in pent.asm
DWORD _cdecl TimeStamp(void);
DWORD _cdecl GetTime(DWORD *);
DWORD _cdecl TotalTime(DWORD * pStartTime, DWORD * pEndTime, DWORD * pTotalTime);

// Defined in time.c
WORD CycleTime(void);
DWORD WaitTime(DWORD nTime);

#endif

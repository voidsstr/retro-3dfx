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
** $Date: 10/11/00 8:44:09 PM$
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     11/07/98 7:48a Russ
** DFB_INSTRUMENTATION escape defines
**
*/

#ifndef _DFBINFO_H_
#define _DFBINFO_H_

#define DFBINFO_COMMAND   0xA5A5

// Mode Queries
#define DFBINFO_CLEAR   0x0001
#define DFBINFO_GET     0x0002

typedef struct tagDFBINFOCMD
{
  DWORD dwCommand;
} DFBINFOCMD;

typedef struct tagDFBINFO
{
  DWORD     createCount;
  DWORD     deleteCount;
  LONGLONG  totalCreateTicks;
  LONGLONG  totalDeleteTicks;
  LONGLONG  ticksPerSecond;
} DFBINFO;

extern DFBINFO  DfbInfo;
#endif

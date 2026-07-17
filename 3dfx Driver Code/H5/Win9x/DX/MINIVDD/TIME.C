/* -*-c++-*- */
/* $Header: time.c, 2, 10/11/00 8:55:19 PM, Brent$ */
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
** File name:   time.c
**
** Description: the CPU independent time functions needed for DDC and I2C.
**
** $Revision: 2$
** $Date: 10/11/00 8:55:19 PM$
**
** $History: time.c $
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/15/99    Time: 9:43a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 10/30/98   Time: 7:29a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Code to check for the TimeStamp before using it
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#include "h3vdd.h"
#include "h3.h"
#include "time.h"

#define TIMER_BASE (0x40)
#define TIMER_CTR2 (0x02)
#define TIMER_MODE (0x03)
#define PORT_B (0x61)
#define INPUT inp
#define TEST_PORT (0x3C2)
#define LOOP_COUNT (1000)

/********************************************************************************
*
* The file time.c implements the CPU independent time functions needed for
* DDC and I2C.  This routines expect that the CPU will be a Pentium or better
*
*  CycleTime -- returns the number of nanoseconds per cycle
*  WaitTime -- wait time in nanoseconds
*
*********************************************************************************/
#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

DWORD dwTimeStamp = 0;
WORD nsCycleTime = 0;       // Nanoseconds per cycle


/*----------------------------------------------------------------------
Function name:  CycleTime

Description:    This is where we use Timer2 to figure out the number
                of nanoseconds per pentium cycles.  This should be
                good until 1 GHZ machines hit the market.  Even then
                the results will be that we wait to long.
Information:    

Return:         WORD    NanoSec cycles.
----------------------------------------------------------------------*/
WORD CycleTime(void)
{
   DWORD dwStartTime[2];
   DWORD dwEndTime[2];
   DWORD dwTotalTime[2];
   DWORD i;
   WORD nOldTime;      
   WORD nNewTime;      
   WORD nscycle;
   BYTE bData;

   if (0 == nsCycleTime)
      {
      // Check to see if Time Stamp is supported
      dwTimeStamp = TimeStamp();
      if (dwTimeStamp)
         {
         outp(TIMER_BASE+TIMER_MODE, 0xB4);
         outp(TIMER_BASE+TIMER_CTR2, 0xFF);
         outp(TIMER_BASE+TIMER_CTR2, 0xFF);
         outp(PORT_B, (BYTE)((inp(PORT_B) & 0xFE) | 0x01));
         nNewTime = 0xFFFE;

         GetTime(dwStartTime);
         outp(TIMER_BASE+TIMER_MODE, 0x80);
         nOldTime = inp(TIMER_BASE+TIMER_CTR2);
         nOldTime |= ((WORD)inp(TIMER_BASE+TIMER_CTR2) << 8);
         for (; nOldTime - nNewTime < 1000; )
             {
             outp(TIMER_BASE+TIMER_MODE, 0x80);
             nNewTime = inp(TIMER_BASE+TIMER_CTR2);
             nNewTime |= ((WORD)inp(TIMER_BASE+TIMER_CTR2) << 8);
             }
         GetTime(dwEndTime);
         outp(PORT_B, (BYTE)(inp(PORT_B) & 0xFE));

         TotalTime(dwStartTime, dwEndTime, dwTotalTime);
         nscycle = (WORD)((nOldTime - nNewTime) * 838/dwTotalTime[0]);   
         Debug_Printf("Total %08lx:%08lx cycle\n", dwTotalTime[1], dwTotalTime[0]);
         Debug_Printf("Total %d ns/cycle\n", nscycle);
         Debug_Printf("Start Time %08lx:%08lx\n", dwStartTime[1], dwStartTime[0]);
         Debug_Printf("End Time %08lx:%08lx\n", dwEndTime[1], dwEndTime[0]);
         Debug_Printf("Old Time %04lx New Time %04lx\n", nOldTime, nNewTime);
   
         if (0 == nscycle)
            nscycle++;

         nsCycleTime = nscycle;
         }
      else
         {
         outp(TIMER_BASE+TIMER_MODE, 0xB4);
         outp(TIMER_BASE+TIMER_CTR2, 0xFF);
         outp(TIMER_BASE+TIMER_CTR2, 0xFF);
         outp(PORT_B, (BYTE)((inp(PORT_B) & 0xFE) | 0x01));
         nNewTime = 0xFFFE;

         outp(TIMER_BASE+TIMER_MODE, 0x80);
         nOldTime = inp(TIMER_BASE+TIMER_CTR2);
         nOldTime |= ((WORD)inp(TIMER_BASE+TIMER_CTR2) << 8);

         for (i=0; i<LOOP_COUNT; i++)
            {
            bData = inp(TEST_PORT);
            }

         outp(TIMER_BASE+TIMER_MODE, 0x80);
         nNewTime = inp(TIMER_BASE+TIMER_CTR2);
         nNewTime |= ((WORD)inp(TIMER_BASE+TIMER_CTR2) << 8);
         outp(PORT_B, (BYTE)(inp(PORT_B) & 0xFE));

         nscycle = (WORD)((nOldTime - nNewTime) * 838/(DWORD)(LOOP_COUNT));   
         nscycle++;
         Debug_Printf("Total %d ns per loop\n", nscycle);
         Debug_Printf("Old Time %04lx New Time %04lx\n", nOldTime, nNewTime);
         nsCycleTime = nscycle;
         }
      }
   else
      nscycle = nsCycleTime;

   return nscycle;
}


/*----------------------------------------------------------------------
Function name:  WaitTime

Description:    This is where we use the nsCycleTime we calculated
                before to determine how many cycles to wait.
Information:    

Return:         WORD    1 is always returned.
----------------------------------------------------------------------*/
DWORD WaitTime(DWORD nTime)
{
   DWORD dwStartTime[2];
   DWORD dwEndTime[2];
   DWORD dwTotalTime[2];
   DWORD dwWait = nTime / nsCycleTime;
   DWORD i;   
   BYTE bData;
   
   if (dwTimeStamp)
      {
      GetTime(dwStartTime);
      for (;;)
         {
         GetTime(dwEndTime);
         TotalTime(dwStartTime, dwEndTime, dwTotalTime);
         if (dwTotalTime[1])
            break;
         if (dwTotalTime[0] > dwWait)
            break;         
         }
      }
   else
      {
      // Add one to take into account remainder
      dwWait++;
      for (i=0; i<dwWait; i++)
         {
         bData = inp(TEST_PORT);
         }
      }

   return 1;
}


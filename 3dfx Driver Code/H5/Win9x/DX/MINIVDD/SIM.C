/* -*-c++-*- */
/* $Header: sim.c, 2, 10/11/00 8:55:03 PM, Brent$ */
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
** File name:   sim.c
**
** Description: WIN SIM Interface File
**
** $Revision: 2$
** $Date: 10/11/00 8:55:03 PM$
**
** $History: sim.c $
** 
** *****************  Version 1  *****************
** User: Andrew       Date: 7/27/99    Time: 4:40p
** Created in $/devel/h5/Win9x/dx/minivdd
** Code for Windows Simulator
** 
**
*/

#define WIN40COMPAT
#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#include "h3irq.h"
#undef  VDDONLY

#ifdef WIN_CSIM
#include <inapalm.h>
#endif

extern DWORD SparseMemBase0Map[4];

#ifdef WIN_CSIM

#define MIN(A,B) ((A) <= (B) ? (A) : (B))
/*----------------------------------------------------------------------
Function name:  GetWINSIMIface

Description:    Get Windows Simulator Interface

Information:    

Return:         NULL, 1, or dwNumChips
----------------------------------------------------------------------*/
int GetWINSIMIface(PDEVTABLE pDev, DWORD dwUnitNum)
{
   NLPINTERFACE Interface;
   DWORD dwVersion;
   DWORD dwNumChips;
   int j;

   dwNumChips = 1;
   if (NLPRET_OK == Get_Napalm_Interface(&Interface, NAPALM_VXD_ID))
      {
      if (NLPRET_OK != Get_Napalm_Version(Interface, &dwVersion))
         {
         Debug_Printf( VNAME "Failed to Get Napalm Version");
         pDev->dwDevNode = 0x0;
	      return(NULL);
         }
      if (dwVersion < (2<<24))
         {
         NLPMEMINFO MemInfo;
         MemInfo.dwBase0 = 0;
         MemInfo.dwBase1 = 0;
         MemInfo.dwLen0 = 0;
         MemInfo.dwLen1 = 0;
         MemInfo.dwHostV3Base0 = 0;
         MemInfo.dwHostV3Base1 = 0;
         if (NLPRET_OK == Get_Napalm_Memory_Info(Interface, &MemInfo))
            {
            for (j=0;j<sizeof(SparseMemBase0Map)/sizeof(DWORD);j++)
#ifdef FAST2D
               pDev->RegBase[j] = MemInfo.dwHostV3Base0 + SparseMemBase0Map[j];
#else
               pDev->RegBase[j] = MemInfo.dwBase0 + SparseMemBase0Map[j];
#endif
#ifdef LINEAR_ONLY
            pDev->LfbBase = MemInfo.dwHostV3Base1;
#else
            pDev->LfbBase = MemInfo.dwBase1;
#endif
            pDev->FakeRegBase = MemInfo.dwBase0;
            pDev->FakeLfbBase = MemInfo.dwBase1;
            pDev->RealRegBase = MemInfo.dwHostV3Base0;
            pDev->RealLfbBase = MemInfo.dwHostV3Base1;
            pDev->MemSizeInMB = MIN(pDev->MemSizeInMB, MemInfo.dwLen1 >> 20);
            // Hey we can only handle 16 MB
            if (pDev->MemSizeInMB > 16)
               pDev->MemSizeInMB = 16;
            }
         else
            {
            Debug_Printf( VNAME "Failed to Get memory Interface");
   	      pDev->dwDevNode = 0x0;
	         return(NULL);
            }
         }
      else
         {
         NLPMEMINFOEX MemInfoex;
         MemInfoex.dwBase0[dwUnitNum] = 0;
         MemInfoex.dwBase1[dwUnitNum] = 0;
         MemInfoex.dwLen0[dwUnitNum] = 0;
         MemInfoex.dwLen1[dwUnitNum] = 0;
         MemInfoex.dwHostV3Base0 = 0;
         MemInfoex.dwHostV3Base1 = 0;
         if (NLPRET_OK == Get_Napalm_Memory_InfoEx(Interface, &MemInfoex))
            {
            for (j=0;j<sizeof(SparseMemBase0Map)/sizeof(DWORD);j++)
#ifdef FAST2D
               pDev->RegBase[j] = MemInfoex.dwHostV3Base0 + SparseMemBase0Map[j];
#else
               pDev->RegBase[j] = MemInfoex.dwBase0[dwUnitNum] + SparseMemBase0Map[j];
#endif
#ifdef LINEAR_ONLY
            pDev->LfbBase = MemInfoex.dwHostV3Base1;
#else
            pDev->LfbBase = MemInfoex.dwBase1[dwUnitNum];
#endif
            pDev->FakeRegBase = MemInfoex.dwBase0[dwUnitNum];
            pDev->FakeLfbBase = MemInfoex.dwBase1[dwUnitNum];
            pDev->RealRegBase = MemInfoex.dwHostV3Base0;
            pDev->RealLfbBase = MemInfoex.dwHostV3Base1;
            dwNumChips = MemInfoex.dwNumChips;
            if (0x0 == dwUnitNum)
               pDev->MemSizeInMB = MIN(pDev->MemSizeInMB, MemInfoex.dwLen1[dwUnitNum] >> 20);
            else
               pDev->MemSizeInMB = MemInfoex.dwLen1[dwUnitNum] >> 20;
            // Hey we can only handle 16 MB
            if (pDev->MemSizeInMB > 16)
               pDev->MemSizeInMB = 16;
#ifdef SLI_AA
            if ((dwNumChips > 1) && (0x0 == dwUnitNum))
               pDev->dwType = SLI_AA_MASTER_DEVICE;
#endif
            }
         else
            {
            Debug_Printf( VNAME "Failed to Get memory Interface");
   	      pDev->dwDevNode = 0x0;
	         return(NULL);
            }
         }
      Debug_Printf( VNAME "Reg Base %x LfbBase %x\n", pDev->RegBase[0], pDev->LfbBase);
      Debug_Printf( VNAME "Fake Reg Base %x Fake LfbBase %x\n", pDev->FakeRegBase, pDev->FakeLfbBase);
      Debug_Printf( VNAME "Real Reg Base %x Real LfbBase %x\n", pDev->RealRegBase, pDev->RealLfbBase);
      if (pDev->RegBase[0] == NULL)
         {  
	      Debug_Printf( VNAME "Unable to map memory base 0\n");
	      pDev->dwDevNode = 0x0;
	      return(NULL);
	      }
      if (pDev->LfbBase == NULL)
	      {     
	      Debug_Printf( VNAME "Unable to map memory base 1\n");
	      pDev->dwDevNode = 0x0;
	      return(NULL);
	      }
      if (NLPRET_OK != Release_Napalm_Interface(Interface))
         {
         Debug_Printf( VNAME "Failed to Release Interface");
         pDev->dwDevNode = 0x0;
	      return(NULL);
         }
      }
   else
      {
      Debug_Printf( VNAME "Failed to Get Interface");
      pDev->dwDevNode = 0x0;
      return(NULL);
      }

   if (0x0 == dwUnitNum)
      return dwNumChips;
   else
      return 1;      
}
#endif

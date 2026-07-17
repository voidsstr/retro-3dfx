/* -*-c++-*- */
/* $Header:
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
** File name:   pci.c
**
** Description:  Routines to Read/Write PCI for Napalm SLI.  These are neccessary
** becauses the OS provided functions to not allow one to specific function number.
** Also if the device is hidden as is our case when <Multi-Function Bit is zero>
** these is no devnode.
**
** Notes: Use Win 98 Routines if available else fail to our own routines.
**
** $Revision: 4$
** $Date: 10/11/00 8:54:53 PM$
**
** $History: pci.c $
** 
** *****************  Version 6  *****************
** User: Andrew       Date: 8/31/99    Time: 9:26a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed to compile without SLI_AA 
** 
** *****************  Version 5  *****************
** User: Andrew       Date: 8/05/99    Time: 4:12p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added calls to Fake PCI routines for WIN_CSIM
** 
** *****************  Version 4  *****************
** User: Andrew       Date: 7/28/99    Time: 3:46p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added some debug statements
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 7/09/99    Time: 8:26a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixed a compile bug
** 
** *****************  Version 2  *****************
** User: Cwilcox      Date: 7/08/99    Time: 1:11p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added runtime checking for Napalm versus Voodoo3.
** 
** *****************  Version 1  *****************
** User: Andrew       Date: 6/25/99    Time: 10:03a
** Created in $/devel/h5/Win9x/dx/minivdd
** New file to support PCI functions to use function number in Windows '95
** 
**
*/


#define WIN40SERVICES
#include "h3vdd.h"
#include "h3.h"
#include "pci.h"

#define VDDONLY
#include "h3g.h"
#undef  VDDONLY

// Stolen from Win98ddk\inc\pci.h
/*MACROS*/
// Fix for building with Windows 98 DDK (PCI services already declared)
#ifndef PCI_LOCK
#define	PCI_Service	Declare_Service
Begin_Service_Table(PCI, VxD)
PCI_Service	(_PCI_Get_Version, VxD_CODE)
PCI_Service	(_PCI_Read_Config, VxD_CODE)
PCI_Service	(_PCI_Write_Config, VxD_CODE)
PCI_Service	(_PCI_Lock_Unlock, VxD_CODE)
End_Service_Table(PCI, VxD)
/*ENDMACROS*/
#endif

#define PCI_MECH1_ADDR(Bus, DevFunc, Offset) (CONFIG_ADDRESS_ENABLE_BIT | ((Bus) << 16) | ((DevFunc) << 8) | (Offset))
#define PCI_MECH2_ADDR(DevFunc, Offset) (CONFIG_MAPPING_OFFSET + (((DevFunc & 0xFF) << 8) | (Offset & 0xFC)))

DWORD dwWin98;
#define PCI_UNKNOWN_MECHANISM (0xFFFFFFFF)
DWORD dwConfigMech = PCI_UNKNOWN_MECHANISM;
void our_outpd(DWORD, DWORD);
DWORD our_inpd(DWORD);
void our_outp(DWORD, DWORD);

#ifdef WIN_CSIM
#include <inapalm.h>
NLPINTERFACE Interface = 0x0;
DWORD dwUseFakePCI;
#endif

/*----------------------------------------------------------------------
Function name:  PCI_Config_Check

Description:  And the configuration mechanism is.....

Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD PCI_Config_Check(void)
{
   DWORD dwReturn = PCI_UNKNOWN_MECHANISM;
   DWORD dwBus; 
   DWORD dwDevFunc; 
   DWORD dwValue;
   DWORD dwSavePort;

   // Mechanism 1 ??
   dwSavePort = our_inpd(CONFIG_ADDRESS_PORT);
   for (dwBus=0; ((dwBus<256) && (PCI_UNKNOWN_MECHANISM == dwReturn)); dwBus += 1)
      {
      for (dwDevFunc = 0; dwDevFunc<0x100; dwDevFunc += 0x08)
         {
         our_outpd(CONFIG_ADDRESS_PORT, PCI_MECH1_ADDR(dwBus, dwDevFunc, 0x0));
         dwValue = our_inpd(CONFIG_DATA_PORT) & 0xFFFF;
         if (0xFFFF != dwValue)
            {
            dwReturn = 0x01;
            break;
            }
         }
      } 
   our_outpd(CONFIG_ADDRESS_PORT, dwSavePort);
 
   // Gotta to be Mechanism 2
   if (PCI_UNKNOWN_MECHANISM == dwReturn)
      dwReturn = 0x02;  

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  PCI_Read_Config_W95

Description:  Read a DWORD

Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD PCI_Read_Config_W95(DWORD dwBus, DWORD dwDevFunc, DWORD dwOffset)
{
   DWORD dwReturn;
   DWORD dwSavePort;

   if (PCI_UNKNOWN_MECHANISM == dwConfigMech)
      dwConfigMech = PCI_Config_Check();

  if ( 1 == dwConfigMech)
      {
      dwSavePort = our_inpd(CONFIG_ADDRESS_PORT);
      our_outpd(CONFIG_ADDRESS_PORT, PCI_MECH1_ADDR(dwBus, dwDevFunc, dwOffset));
      dwReturn = our_inpd(CONFIG_DATA_PORT);
      our_outpd(CONFIG_ADDRESS_PORT, dwSavePort);
      } 
   else 
      {                      /* config mechanism 2 */
      our_outp(CONFIG_ADDRESS_PORT, CONFIG_MAPPING_ENABLE_BYTE);
      dwReturn = our_inpd(PCI_MECH2_ADDR(dwDevFunc, dwOffset)); 
      our_outp(CONFIG_ADDRESS_PORT, CONFIG_MAPPING_DISABLE_BYTE);
      }
  
   return (dwReturn);
}

/*----------------------------------------------------------------------
Function name:  PCI_Write_Config_W95

Description:  Write a DWORD

Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD PCI_Write_Config_W95(DWORD dwBus, DWORD dwDevFunc, DWORD dwOffset, DWORD dwValue)
{
   DWORD dwSavePort;

   if (PCI_UNKNOWN_MECHANISM == dwConfigMech)
      dwConfigMech = PCI_Config_Check();

   if (1 == dwConfigMech)
      {
      dwSavePort = our_inpd(CONFIG_ADDRESS_PORT);
      our_outpd(CONFIG_ADDRESS_PORT, PCI_MECH1_ADDR(dwBus, dwDevFunc, dwOffset));
      our_outpd(CONFIG_DATA_PORT, dwValue);
      our_outpd(CONFIG_ADDRESS_PORT, dwSavePort);
      } 
   else
      {
      our_outp(CONFIG_ADDRESS_PORT, CONFIG_MAPPING_ENABLE_BYTE);
      our_outpd(PCI_MECH2_ADDR(dwDevFunc, dwOffset), dwValue);
      our_outp(CONFIG_ADDRESS_PORT, CONFIG_MAPPING_DISABLE_BYTE);
      }
 
   return (0x01);
}

/*----------------------------------------------------------------------
Function name:  PCI_Read_Config

Description:  Call the PCI Device Manager to do a Read  

Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD PCI_Read_Config(DWORD dwBus, DWORD dwDevFunc, DWORD dwOffset)
{
   DWORD dwReturn;
#ifdef WIN_CSIM
   NPLPCICONFIG PCI;

   if (dwUseFakePCI)
      {
      if (0x0 == Interface)
         {
         Get_Napalm_Interface(&Interface, NAPALM_VXD_ID);
         }
      PCI.dwBus = dwBus;
      PCI.dwDevFunc = dwDevFunc;
      PCI.dwOffset = dwOffset;
      PCI.dwValue = 0x0;
      Napalm_PCI_Read_Config(Interface, &PCI);
      dwReturn = PCI.dwValue;
      }
   else {
#endif
   if (dwWin98)   
      {
      __asm pushad
      __asm push  dwOffset
      __asm push  dwDevFunc
      __asm push  dwBus
      VxDCall(_PCI_Read_Config);
      __asm add   esp, 12
      __asm mov dwReturn, eax
      __asm popad
      }
   else
      dwReturn = PCI_Read_Config_W95(dwBus,dwDevFunc,dwOffset);
#ifdef WIN_CSIM
   }
#endif       

#if 0
   Debug_Printf("PCI Read Cycle Bus %x DevFunc %x Offset %x Value %x\n", dwBus, dwDevFunc, dwOffset, dwReturn);     
#endif

   return (dwReturn);
}

/*----------------------------------------------------------------------
Function name:  PCI_Write_Config

Description:  Call the PCI Device Manager to do a Write

Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD PCI_Write_Config(DWORD dwBus, DWORD dwDevFunc, DWORD dwOffset, DWORD dwValue)
{
   DWORD dwReturn;
   
#ifdef WIN_CSIM
   NPLPCICONFIG PCI;

   if (dwUseFakePCI)
      {
      if (0x0 == Interface)
         {
         Get_Napalm_Interface(&Interface, NAPALM_VXD_ID);
         }
      PCI.dwBus = dwBus;
      PCI.dwDevFunc = dwDevFunc;
      PCI.dwOffset = dwOffset;
      PCI.dwValue = dwValue;
      dwReturn = Napalm_PCI_Write_Config(Interface, &PCI);
      }
   else
      {
#endif
   if (dwWin98)
      {
      __asm pushad
      __asm push dwValue
      __asm push dwOffset
      __asm push dwDevFunc
      __asm push dwBus
      VxDCall(_PCI_Write_Config);
      __asm add   esp, 16
      __asm mov dwReturn, eax
      __asm popad
      }
   else
      dwReturn = PCI_Write_Config_W95(dwBus,dwDevFunc,dwOffset,dwValue);
#ifdef WIN_CSIM
   }
#endif       

#if 0
   Debug_Printf("PCI Write Cycle Bus %x DevFunc %x Offset %x Value %x\n", dwBus, dwDevFunc, dwOffset, dwValue);     
#endif

   return (dwReturn);
}


/*----------------------------------------------------------------------
Function name:  PCIGetBusDevFunc

Description:  Why the hell's this is not in the PCI Device Manager is
beyond me.  This function converts  a devNode into a Bus, Device Function 
number.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
DWORD PCIGetBusDevFunc(DWORD dwDevNode, DWORD * pdwBus, DWORD * pdwDevFunc)
{
  DWORD dwIOBase;
  DWORD dwFB;
  DWORD dwMMR;
  DWORD dwDevID;

  // Read IoBase, FB, and MMR
  CM_Call_Enumerator_Function( dwDevNode,
                               PCI_ENUM_FUNC_GET_DEVICE_INFO,
                               SST_PCI_MMIO_ID, &dwMMR, 
                               sizeof(DWORD), 0 );

  CM_Call_Enumerator_Function( dwDevNode,
                               PCI_ENUM_FUNC_GET_DEVICE_INFO,
                               SST_PCI_FB_ID, &dwFB, 
                               sizeof(DWORD), 0 );

  CM_Call_Enumerator_Function( dwDevNode,
                               PCI_ENUM_FUNC_GET_DEVICE_INFO,
                               SST_PCI_IOBASE_ID, &dwIOBase, 
                               sizeof(DWORD), 0 );
   

  for (*pdwBus=0; *pdwBus<256; *pdwBus += 1)
  {
    for (*pdwDevFunc = 0; *pdwDevFunc<0x100; *pdwDevFunc += 0x08)
    {
      dwDevID = PCI_Read_Config(*pdwBus, *pdwDevFunc, 0x0);

      if ((IS_VOODOO3(dwDevID) || IS_NAPALM(dwDevID)) && 
          (dwMMR    == PCI_Read_Config(*pdwBus, *pdwDevFunc, SST_PCI_MMIO_ID)) &&
          (dwFB     == PCI_Read_Config(*pdwBus, *pdwDevFunc, SST_PCI_FB_ID)) &&
          (dwIOBase == PCI_Read_Config(*pdwBus, *pdwDevFunc, SST_PCI_IOBASE_ID)))
      {
        return 1;
      }
    }
  }

  return 0;
}

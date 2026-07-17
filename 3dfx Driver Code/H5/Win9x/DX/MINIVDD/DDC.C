/* -*-c++-*- */
/* $Header: ddc.c, 6, 10/11/00 8:54:00 PM, Brent$ */
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
** File name:   ddc.c
**
** Description: DDC functions and support.
**
** $Revision: 6$ 
** $Date: 10/11/00 8:54:00 PM$
** $History: ddc.c $
** 
** *****************  Version 19  *****************
** User: Rbissell     Date: 9/07/99    Time: 2:15p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Deployed the new modularized I2C code.
** 
** *****************  Version 18  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** tvout merge from V3_OEM_100
** 
** *****************  Version 17  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
** 
** *****************  Version 15  *****************
** User: Andrew       Date: 5/06/99    Time: 5:28p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a Typo
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 5/06/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code for New Windows "C" Simulator
** 
** *****************  Version 13  *****************
** User: Stb_srogers  Date: 4/15/99    Time: 4:27p
** Updated in $/devel/h3/win95/dx/minivdd
** Fixes PRS 5275: Fix for new Gateway monitor. Fixed the DoWakeUp()
** function so that it follows VESA DDC spec.
** 
** *****************  Version 12  *****************
** User: Michael      Date: 1/04/99    Time: 4:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 11    12/24/98 11:52a Andrew
** Added some code to increase retrys
** 
** 10    12/09/98 5:25p Andrew
** Removed some redundant error checking as these seems to improve
** reliability of DDC.
** 
** 9     10/30/98 7:27a Andrew
** Changed a "||" to a "&&" to match GLOP.  The early exit is believed to
** cause problems on certain monitors.
** 
** 8     10/08/98 5:29p Andrew
** Fixed a early exit to exit on either SCL or SDA not being asserted
** 
** 7     10/05/98 3:58p Andrew
** Updated to support Multimonitor
** 
** 6     8/23/98 6:53p Andrew
** Fixed the problem with the Vivitron and Driver DDC code.  It seems the
** Vivitron does not like a stop before the read command.
** 
** 5     6/23/98 10:15p Andrew
** Changed the error checking for dmon97.exe so that they could pass in a
** bogus di for function 0.
** 
** 4     5/27/98 8:46a Andrew
** Removed 256 byte DDC stuff
** 
** 3     4/23/98 4:41p Andrew
** Changed return code when DDC fails on a monitor from 024f to 004f.
** 
** 2     4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 1     4/06/98 5:29p Andrew
** DDC 1.0 & 2.0 support
** 
*/

#include "h3vdd.h"
#include "h3.h"
#define VDDONLY
#include "devtable.h"
   #include "i2c/di_i2c.h"
   #define DDC_NUMRETRY (5)
   #define MSEC 1000
   #define DELAY(usecs) CM_Yield(usecs, CM_YIELD_RESUME_EXEC)
#include "ddc2b.h"
#include "time.h"

int DDCCommand(PDEVTABLE pDev, I2CKEY key, FxU8 bAddr);
int DoWakeUp(PDEVTABLE pDev, I2CKEY key, FxU8 bAddr);

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

#undef DDC_DEBUG


/*----------------------------------------------------------------------
Function name:  DoMapFlat

Description:    Perform a flat mapping.

Information:    

Return:         DWORD       Value returned from the VMMCall.
----------------------------------------------------------------------*/
DWORD VXDINLINE DoMapFlat(BYTE bAH, BYTE bAL)
{
  DWORD retval;

  __asm pushad;
  __asm mov ah, bAH;
  __asm mov al, bAL;
  VMMCall(Map_Flat);
  __asm mov retval, eax;
  __asm popad;

  return retval;
}


/*----------------------------------------------------------------------
Function name:  DDCSupport

Description:    This routine is used to handle the VBE/DDC
                function calls.

Information:    

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int DDCSupport(PDEVTABLE pDevTable, PCRS pCR)
{
  I2CKEY key;
  //********I2CMASK I2CMask;
  CLIENT_STRUCT * pCSRS = (CLIENT_STRUCT *)pCR;
  PBYTE pByte;
  int nReturn = 0x0;
  int i;
  WORD nStatus;
  FxU8 bAddr;
   
  if (NULL != pDevTable)
  {
    key = i2c_getaccess(pDevTable, I2C_MONITORDDC, I2C_NORMALSPEED);
    if (key == I2C_NOTAKEY)
      return nReturn;

    bAddr = DDC_ADDR_1;

/********
#ifdef WIN_CSIM
    I2CMask.pReg = (DWORD *)(pDevTable->RealRegBase + I2COUT_PORT);
#else
    I2CMask.pReg = (DWORD *)(pDevTable->RegBase[HWINFO_SST_IOREGS_INDEX] + I2COUT_PORT);
#endif

    I2CMask.bAddr = DDC_ADDR_1;

    I2CMask.bEnableBit = DDC_ENABLE;
    I2CMask.bSCLOutBit = DDC_SCL_OUT_BIT;
    I2CMask.bSDAOutBit = DDC_SDA_OUT_BIT;
    I2CMask.bSCLInBit = DDC_SCL_IN_BIT;
    I2CMask.bSDAInBit = DDC_SDA_IN_BIT;
    I2CInit(&I2CMask);
*********/

    switch (pCSRS->CBRS.Client_BL)
    {
      // Report DDC Capabilities
      case 0:
        // ES:DI should be zero on this call
        // dmon97.exe which is a WHQL test has this wrong
        //
#ifdef NULL_PTR_CHK
        if (0x0 == (pCSRS->CWRS.Client_DI | pCSRS->CRS.Client_ES))
#else
        if (0x0 == pCSRS->CRS.Client_ES)
#endif
        {
          nStatus = 0x0100;
          for (i=0; i < DDC_NUMRETRY; i++)
            if (DoWakeUp(pDevTable, key, bAddr))
            {
              nStatus |= 0x0002;
              break;
            }

          // This IS_NAPALM check is a workaround for an initialization problem that I don't understand
          // enough to fix properly.  Here is what happens.  On a Voodoo3 board this code tries to talk to 
          // a non-existant DFP and it fails and gives up as expected.  However it leaves the I2C bus
          // in a non-working state so that when the BT868 TvOut code tries to talk to the Brooktree
          // part it fails and reports back that TvOut is not present.  This does not happen on a Napalm 
          // board.  Customers have been working around this problem by unchecking the "Automatically detect
          // Plug & Play monitors" box on the Monitor control panel, which works because that cause this 
          // routine to never be called.  Dan O'Connell 7/24/2000
          if (IS_NAPALM(pDevTable->dwVendorDeviceID))
          {
            if(nStatus == 0x0100)
            {
              i2c_endaccess(pDevTable, key);
              key = i2c_getaccess(pDevTable, I2C_FLATPANELDDC, I2C_NORMALSPEED);
              if (key == I2C_NOTAKEY)
                return 0;

              for (i=0; i < DDC_NUMRETRY; i++)
                if (DoWakeUp(pDevTable, key, bAddr))
                {
                    nStatus |= 0x0002;
                    break;
                }
            }
          }

          // EDID 2.0 Capable??
#ifdef DDC_20
          if (nStatus & 0x0002)
          {
            bAddr = DDC_ADDR_2;
            for (i=0; i < DDC_NUMRETRY; i++)
              if (DoWakeUp(key, bAddr))
              {
                nStatus |= 0x0008;
                break;
              }
            pCSRS->CWRS.Client_AX = 0x004F;
            pCSRS->CWRS.Client_BX = nStatus;
          }
          else
#endif
          {
            pCSRS->CWRS.Client_AX = 0x004F;
            pCSRS->CWRS.Client_BX = nStatus;
          }
        }
        else
          pCSRS->CWRS.Client_AX = 0x014F;

        nReturn = 1;
        break;

      // Read EDID
      case 1:
        pByte = (PBYTE)CLIENT_PTR_FLAT(ES, DI);
        if (0xFFFFFFFF != (DWORD)pByte)
        {
          if (ReadDDC(pDevTable, key, DDC_ADDR_1, pByte, A0_EDID_SIZE))
            pCSRS->CWRS.Client_AX = 0x004F;
          else
          {
             i2c_endaccess(pDevTable, key);
             key = i2c_getaccess(pDevTable, I2C_FLATPANELDDC, I2C_NORMALSPEED);
             if (key == I2C_NOTAKEY)
               return 0;

             if (ReadDDC(pDevTable, key, DDC_ADDR_1, pByte, A0_EDID_SIZE))
               pCSRS->CWRS.Client_AX = 0x004F;
             else
               pCSRS->CWRS.Client_AX = 0x014F;
          }
        }
        else
          pCSRS->CWRS.Client_AX = 0x014F;

        nReturn = 1;
        break;

      // Read VDIF <unsupported>
      case 2:
        pCSRS->CBRS.Client_AL = 0x0;
        nReturn = 1;
        break;

      // Read EDID 2
      case 3:
        pByte = (PBYTE)CLIENT_PTR_FLAT(ES, DI);
        if (0xFFFFFFFF != (DWORD)pByte)
        {
          if (ReadDDC(pDevTable, key, DDC_ADDR_2, pByte, A2_EDID_SIZE))
            pCSRS->CWRS.Client_AX = 0x004F;
          else
          {
             i2c_endaccess(pDevTable, key);
             key = i2c_getaccess(pDevTable, I2C_FLATPANELDDC, I2C_NORMALSPEED);
             if (key == I2C_NOTAKEY)
               return 0;

             if (ReadDDC(pDevTable, key, DDC_ADDR_2, pByte, A2_EDID_SIZE))
               pCSRS->CWRS.Client_AX = 0x004F;
             else
               pCSRS->CWRS.Client_AX = 0x014F;
          }
        }
        else
          pCSRS->CWRS.Client_AX = 0x014F;

        nReturn = 1;
        break;

      // Undefined and Unsupported
      default:
        pCSRS->CBRS.Client_AL = 0x0;
        nReturn = 1;
        break;
    }
  }

  i2c_endaccess(pDevTable, key);
  return nReturn;
}


/*----------------------------------------------------------------------
Function name:  ReadDDC

Description:    High-level routine to read DDC for either
                128 or 256 EDID.
Information:    

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int ReadDDC(PDEVTABLE pDev, I2CKEY key, int nAddr, PBYTE pBuffer, int nSize)
{
  //********I2CMASK I2CMask;
  int i;
  int j;
  int nReturn = 0;
  BYTE bChkSum;

/*********
   I2CMask.pReg = (DWORD *)(pMap + I2COUT_PORT);
   I2CMask.bAddr = nAddr;

   I2CMask.bEnableBit = DDC_ENABLE;
   I2CMask.bSCLOutBit = DDC_SCL_OUT_BIT;
   I2CMask.bSDAOutBit = DDC_SDA_OUT_BIT;
   I2CMask.bSCLInBit = DDC_SCL_IN_BIT;
   I2CMask.bSDAInBit = DDC_SDA_IN_BIT;

   I2CInit(&I2CMask);
**********/

  for (j=0; j<DDC_NUMRETRY<<1; j++)
  {
    if (DoWakeUp(pDev, key, (FxU8)nAddr))
    {   
      if (DDCCommand(pDev, key, (FxU8)nAddr))
      {
        for (i=0; i<nSize-1; i++)
          i2c_readbyte(pDev, key, &pBuffer[i], 1);
        i2c_readbyte(pDev, key, &pBuffer[i], 0);

        i2c_stop(pDev, key);
        bChkSum=0;

        for (i=0; i<nSize; i++)
        {
#ifdef DDC_DEBUG
          Debug_Printf("%d %02x\n", i, pBuffer[i]);
#endif
          bChkSum += pBuffer[i];
        }

        if (0 == bChkSum)
        {
          nReturn = 1;
          break;
        }
        else
        {
          Debug_Printf("Chksum failure %02x \n", bChkSum);
        }
      }
      else
      {
        Debug_Printf("DDCCommand failed\n");
      }
    }
    else
    {
      Debug_Printf("Wake Up Failed\n");
    }
  }

  return nReturn;
}


/*----------------------------------------------------------------------
Function name:  DDCCommand

Description:    Routine to check if this is a DDC Monitor.

Information:    

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int DDCCommand(PDEVTABLE pDev, I2CKEY key, FxU8 bAddr)
{
  int nReturn = 0;

  i2c_start(pDev, key);
  if (i2c_sendbyte(pDev, key, (BYTE)(bAddr|I2C_WRITESLAVE)))
  {
    if (i2c_sendbyte(pDev, key, 0x00))
    {
      // Does not work on Vivitron ?
#if 0
      i2c_stop(pDev, key);
#endif
      i2c_start(pDev, key);
      i2c_sendbyte(pDev, key, (BYTE)(bAddr|I2C_READSLAVE));
      i2c_setsda(pDev, key, 1, 0);
      nReturn = 1;
    }
    else
    {
      i2c_stop(pDev, key);
      Debug_Printf("failed to send %x\n", 0x00);
    }
  }
  else
  {
    i2c_stop(pDev, key);
    Debug_Printf("failed to send %x\n", bAddr|I2C_WRITESLAVE);
  }

  return nReturn;
}


/*----------------------------------------------------------------------
Function name:  DoWakeUp

Description:    Routine to query the monitor to see if it is alive.

Information:    

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
//********int read_sda(PI2CMASK pI2CMask);
int DoWakeUp(PDEVTABLE pDev, I2CKEY key, FxU8 bAddr)
{
  int i;
  int nReturn = 0;
  BYTE bCH;

  // Possible Switch Box Fix?
  i2c_setsda(pDev, key, 1, 0);

  // Set the clock line low to let the monitor know we want the bus
  i2c_setscl(pDev, key, 0, 0);

  DELAY(MSEC);

  i2c_setsda(pDev, key, 1, 0);
  i2c_setscl(pDev, key, 1, 0);

  // 166 MSEC is approx 60 Hz (Editor's note: Actually, 60Hz is 16.6 MSEC.  But I ain't changin' it!)
  // Wait for the clock line and the data line to be released (both high)
  for (i=0; i<166; i++)
  {
    if ((1 == i2c_getscl(pDev, key)) && (1 == i2c_getsda(pDev, key)))
      break;
    DELAY(MSEC);
    DELAY(MSEC);
  }

  // Send the stop command to make sure that there are 
  // no outstanding writes on the bus
  i2c_stop(pDev, key);

  // Wait for the clock line and the data line to be released (both high)
  if ((1 != i2c_getscl(pDev, key)) && (1 != i2c_getsda(pDev, key)))
    return nReturn;

  if (DDCCommand(pDev, key, bAddr))
  {
    i2c_readbyte(pDev, key, &bCH, 1);
    if (0x00 == bCH)
    {
      i2c_readbyte(pDev, key, &bCH, 0);
      if (0xFF == bCH)
      {
        nReturn = 1;
        i2c_stop(pDev, key);
      }
      else
      {
        i2c_stop(pDev, key);
        Debug_Printf("Read Failed expected 0xFF got %02x\n", bCH);
      }
    }
    else
    {
      i2c_stop(pDev, key);
      Debug_Printf("Read Failed expected 0x00 got %02x\n", bCH);
    }
  }
  else
    Debug_Printf("DDC Command Failed\n");

  return nReturn;
}


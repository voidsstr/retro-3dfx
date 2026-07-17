/* -*-c++-*- */
/* $Header: gpio.c, 4, 10/11/00 8:55:37 PM, Brent$ 
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
** File name:   gpio.c
**
** Description: GPIO functions  functions.
**
** $Revision: 4$ 
** $Date: 10/11/00 8:55:37 PM$
** 
** $History:$
** 
** 
*/

#define WIN40SERVICES
#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#include "gpio.h"
#include "pci.h"
#include "time.h"

/*----------------------------------------------------------------------
Function name:  GetGpio

Description:    Read a dword
                
Information:

Return: Read DWORD
----------------------------------------------------------------------*/
DWORD GetGpio(PPCI_BIT pPCI_BIT)
{
   return PCI_Read_Config(pPCI_BIT->dwBus, pPCI_BIT->dwDevFunc, pPCI_BIT->RN);
}

/*----------------------------------------------------------------------
Function name:  GetGpio_Bit

Description:    Read a bit
                
Information:

Return: Bit
----------------------------------------------------------------------*/
DWORD GetGpio_Bit(PPCI_BIT pPCI_BIT)
{
   DWORD bData;
   bData = PCI_Read_Config(pPCI_BIT->dwBus, pPCI_BIT->dwDevFunc, pPCI_BIT->RN);
   return (bData & pPCI_BIT->dInMask) >> pPCI_BIT->dInShift;
}

/*----------------------------------------------------------------------
Function name:  SetGpio_Bit

Description:    Write a dword
                
Information:

Return: PCI return value
----------------------------------------------------------------------*/
DWORD SetGpio_Bit(PPCI_BIT pPCI_BIT, DWORD dwValue)
{
   DWORD dwReg;

   dwReg = GetGpio(pPCI_BIT);
   dwReg = dwReg & ~pPCI_BIT->dOutMask; 

   dwReg |= ((dwValue & 0x01) << pPCI_BIT->dOutShift);   
   return PCI_Write_Config(pPCI_BIT->dwBus, pPCI_BIT->dwDevFunc, pPCI_BIT->RN, dwReg);
}

/*----------------------------------------------------------------------
Function name:  SetGpio

Description:    Write a dword
                
Information:

Return: Success or Failure 
----------------------------------------------------------------------*/
DWORD SetGpio(PPCI_BIT pPCI_BIT, DWORD dwValue)
{
   return PCI_Write_Config(pPCI_BIT->dwBus, pPCI_BIT->dwDevFunc, pPCI_BIT->RN, dwValue);
}


/*----------------------------------------------------------------------
Function name:  gpio_scl

Description:    Sets the Clock to bit
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void gpio_scl(PGPIOMASK pGPIOMask, DWORD bit)
{
	int nCount = 0;

	DELAY(DTIME);

   SetGpio_Bit(&pGPIOMask->Clk, bit);
	DELAY(DTIME);

	if (bit)
		while (!gpio_read_scl(pGPIOMask) && (nCount++ < CLOCK_STRETCH))
			DELAY(DTIME);

	if ((bit) && (!gpio_read_scl(pGPIOMask)))
		Debug_Printf("Slave did not respond %s %d\n", __FILE__, __LINE__);
}


/*----------------------------------------------------------------------
Function name:  gpio_sda

Description:    Sets the Data to bit.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void gpio_sda(PGPIOMASK pGPIOMask, DWORD bit)
{
	DELAY(DTIME);
   bit = bit >> 7;
   SetGpio_Bit(&pGPIOMask->Data, bit);
	DELAY(DTIME);
}


/*----------------------------------------------------------------------
Function name:  gpio_read_scl

Description:    Reads the Clock
                
Information:

Return:  Clock bit
----------------------------------------------------------------------*/
int gpio_read_scl(PGPIOMASK pGPIOMask)
{
	return GetGpio_Bit(&pGPIOMask->Clk);
}


/*----------------------------------------------------------------------
Function name:  gpio_read_sda

Description:    Reads the Data line
                
Information:

Return:   Data bit
----------------------------------------------------------------------*/
int gpio_read_sda(PGPIOMASK pGPIOMask)
{
	return GetGpio_Bit(&pGPIOMask->Data);
}


/*----------------------------------------------------------------------
Function name:  gpio_send_byte

Description:    Sends the byte data
                
Information:

Return: 0
----------------------------------------------------------------------*/
int gpio_send_byte(PGPIOMASK pGPIOMask, BYTE b1, BYTE b2, BYTE b3)
{
	int	i;
	int nReturn=0;

	for (i = 0; i < 8; i++)
	{
	  gpio_sda(pGPIOMask, (b1 << i) & 0x80);
	  gpio_scl(pGPIOMask, 1);
	  if (gpio_read_scl(pGPIOMask) == 0)
	  {
	     Debug_Printf("nc\n");
	  }
	  gpio_scl(pGPIOMask, 0);
	}

	for (i = 0; i < 8; i++)
	{
	  gpio_sda(pGPIOMask, (b2 << i) & 0x80);
	  gpio_scl(pGPIOMask, 1);
	  if (gpio_read_scl(pGPIOMask) == 0)
	  {
	     Debug_Printf("nc\n");
	  }
	  gpio_scl(pGPIOMask, 0);
	}

	for (i = 0; i < 8; i++)
	{
	  gpio_sda(pGPIOMask, (b3 << i) & 0x80);
	  gpio_scl(pGPIOMask, 1);
	  if (gpio_read_scl(pGPIOMask) == 0)
	  {
	     Debug_Printf("nc\n");
	  }
	  gpio_scl(pGPIOMask, 0);
	}

   // Strobe to make it happen
   DELAY(DTIME);
   SetGpio_Bit(&pGPIOMask->Strobe, 1);
	DELAY(DTIME);

   DELAY(DTIME);
   SetGpio_Bit(&pGPIOMask->Strobe, 0);
	DELAY(DTIME);


	return nReturn;
}

/*----------------------------------------------------------------------
Function name:  FindGPIODevice

Description:    Used to init a GPIO Port
                
Information:

Return: 1 for success and 0 for error
----------------------------------------------------------------------*/
int FindGPIODevice(DWORD dwDevVenID, DWORD dwBus, PDWORD pdwBus, PDWORD pdwDevFunc)
{
   for (*pdwBus=0; *pdwBus<256; *pdwBus += 1)
      {
      for (*pdwDevFunc = 0; *pdwDevFunc<0x100; *pdwDevFunc += 0x08)
         {
         if (dwDevVenID == PCI_Read_Config(*pdwBus, *pdwDevFunc, 0x0))
            if (dwBus == (PCI_Read_Config(*pdwBus, *pdwDevFunc, 0x19) & 0xFF))
               return 1;
         }
      }

   return 0;
}

/*----------------------------------------------------------------------
Function name:  GPIOInit

Description:    Used to init a GPIO Port
                
Information:

Return:  1 if success or 0 on error
----------------------------------------------------------------------*/
int isGPIOInit = 0;
int GPIOInit(PGPIOMASK pGPIOMask, DWORD dwNapalmBus)
{
   DWORD dwBus;
   DWORD dwDevFunc;
   DWORD dwData;
   
   if (0 == isGPIOInit)
      {
      if (FindGPIODevice(0x00213388, dwNapalmBus, &dwBus, &dwDevFunc))
         {      
         isGPIOInit = 1;
         pGPIOMask->Data.dwBus = pGPIOMask->Clk.dwBus = pGPIOMask->Strobe.dwBus = dwBus;
         pGPIOMask->Data.dwDevFunc = pGPIOMask->Clk.dwDevFunc = pGPIOMask->Strobe.dwDevFunc = dwDevFunc;
         pGPIOMask->Data.RN = 0xC4;
         pGPIOMask->Data.dInMask = (DWORD)0x0100;
         pGPIOMask->Data.dInShift = (DWORD)0x08;
         pGPIOMask->Data.dOutMask = (DWORD)0x0400;
         pGPIOMask->Data.dOutShift = (DWORD)0x0A;

         pGPIOMask->Clk.RN = (DWORD)0xC4;
         pGPIOMask->Clk.dInMask = (DWORD)0x010000;
         pGPIOMask->Clk.dInShift = (DWORD)0x10;
         pGPIOMask->Clk.dOutMask = (DWORD)0x040000;
         pGPIOMask->Clk.dOutShift = (DWORD)0x12;

         pGPIOMask->Strobe.RN = (DWORD)0xC4;
         pGPIOMask->Strobe.dInMask = (DWORD)0x1000;
         pGPIOMask->Strobe.dInShift = (DWORD)0x0C;
         pGPIOMask->Strobe.dOutMask = (DWORD)0x4000;
         pGPIOMask->Strobe.dOutShift = (DWORD)0x0E;

         pGPIOMask->HiVoltage.RN = (DWORD)0xC4;
         pGPIOMask->HiVoltage.dInMask = (DWORD)0x100000;
         pGPIOMask->HiVoltage.dInShift = (DWORD)0x14;
         pGPIOMask->HiVoltage.dOutMask = (DWORD)0x400000;
         pGPIOMask->HiVoltage.dOutShift = (DWORD)0x16;

         // Enable all GPIO as Output
         dwData = PCI_Read_Config(dwBus, dwDevFunc, 0xC4);
         dwData &= 0xFF0000FF;
         //          C7C6C5C4 
         dwData |= 0x00222200;
         PCI_Write_Config(dwBus, dwDevFunc, 0xC4, dwData);
         }
      }
   else
      {
      dwData = PCI_Read_Config(pGPIOMask->Data.dwBus, pGPIOMask->Data.dwDevFunc, 0xC4);
      dwData &= 0xFF0000FF;
      //          C7C6C5C4 
      dwData |= 0x00222200;
      PCI_Write_Config(pGPIOMask->Data.dwBus, pGPIOMask->Data.dwDevFunc, 0xC4, dwData);
      }

   return isGPIOInit;
}


#define FPU_FUNCTION_SAVE		0
#define FPU_FUNCTION_RESTORE	1
void FPU_State( int function );
/*----------------------------------------------------------------------
Function name:  Clock

Description:    Calculate and set GPIO serial clock
                
Information:

Return: clock
----------------------------------------------------------------------*/
DWORD OD[]={2,3,4,5,6,7,8,10};
DWORD S2S1S0[]={15, 15, 1, 6, 3, 4, 7, 5, 2, 15, 0};
#define ABS(A, B) ((A) < (B) ? (B) - (A) : (A) - (B))
#define MHz(A) ((A) * 1000000)
#define kHz(A) ((A) * 1000)
DWORD clock(PDEVTABLE pDev)
{
   double d_clk1;
   double clk1;
   double d_diff;
   double d_partial;
   DWORD rdw;
   DWORD vdw;
   DWORD od;
   DWORD i;
   DWORD b_rdw;
   DWORD b_vdw;
   DWORD b_od;
   DWORD b_i;
   DWORD bFound;
   SstIORegs * pIORegs;
   DWORD ic;
   DWORD pixelclock;
   DWORD n;   
   DWORD m;   
   DWORD k;
   DWORD kpow;   

   // Get the Clock
   pIORegs = (SstIORegs *)pDev->RegBase[HWINFO_SST_IOREGS_INDEX];
   pixelclock = pIORegs->pllCtrl0;
   n = ((pixelclock & 0xFF00) >> 8)+2;
   m = ((pixelclock & 0xFC) >> 2)+2;
   k = pixelclock & 0x03;

   kpow = 1;
   for (i=0; i<k; i++)
      kpow<<=1;

   // This should work to 4 GigaHertz
   ic = 14318180*n/m/kpow;                    
   ic >>= 2;
   
	FPU_State( FPU_FUNCTION_SAVE );

   d_diff = MHz(500.0);
   i = 0;
   bFound =0;
   for (i=0; i < sizeof(OD)/sizeof(DWORD); i++)
      {
      od = OD[i];
      for (rdw=1; rdw<128; rdw++)
         {
         for (vdw=4; vdw<512; vdw++)
            {
            clk1 = 14318180.0*2.0*((double)vdw+8.0)/(((double)rdw+2.0)*(double)od);
            if (ABS(clk1, (double)ic) < d_diff)
               {
               // Some constraints on vdw & rdw
               // 55 Mhz < 14318180 * 2* (vdw+8)/(rdw + 2) < 400 Mhz
               d_partial = 14318180.0*2.0*((double)vdw+8.0)/((double)rdw+2.0);
               if ((d_partial > MHz(55)) && (d_partial < MHz(400)))
                  {
                  // Another constraints
                  // 200 kHz < 14318180/(rdw+2)
                  if (14318180/(rdw+2) > kHz(200))
                     {
                     d_diff = ABS(clk1, (double)ic);
                     bFound = 1;
                     b_rdw = rdw;
                     b_vdw = vdw;
                     b_od = od;
                     b_i = i;
                     d_clk1 = clk1;
                     }
                  }
               }
            }
         }
      }   


	FPU_State( FPU_FUNCTION_RESTORE );

   if (bFound)
      Output_Clock(pDev, 0x0, 0x1, 0x0, S2S1S0[b_od], b_vdw, b_rdw);
   else
      _asm int 03;

   return ic;      
}

/*----------------------------------------------------------------------
Function name:  Output_Clock

Description:    Build and Send clock
                
Information:

Return:         0
----------------------------------------------------------------------*/
#define BUILD_BYTE(data, mask, position) ((BYTE)((data & mask)<<position))
GPIOMASK GpioMask;
DWORD Output_Clock(PDEVTABLE pDev, DWORD c, DWORD ttl, DWORD f, DWORD s, DWORD v, DWORD r)
{
   BYTE b1, b2, b3;

   b1 = BUILD_BYTE(c, 3, 6);
   b1 |= BUILD_BYTE(ttl, 1, 5);
   b1 |= BUILD_BYTE(f, 3, 3);
   b1 |= BUILD_BYTE(s, 7, 0);

   b2 = (BYTE)((v & 0x1Fe) >> 1);
   b3 = BUILD_BYTE(v, 1, 7);
   b3 |= BUILD_BYTE(r, 0x7F, 0);

   if (GPIOInit(&GpioMask, pDev->dwBus))
      gpio_send_byte(&GpioMask, b1, b2, b3);

   return 0;
}

/*----------------------------------------------------------------------
Function name:  HiVoltageSet

Description:    Set the voltage level
                
Information:

Return:         0
----------------------------------------------------------------------*/
void HiVoltageSet(PDEVTABLE pDev, DWORD bit)
{
   if (GPIOInit(&GpioMask, pDev->dwBus))
      {
      SetGpio_Bit(&GpioMask.HiVoltage, bit);
	   DELAY(DTIME);
      }   
}

/*----------------------------------------------------------------------
Function name:  HiVoltageOn

Description:    Set High Voltage On
                
Information:

Return:         0
----------------------------------------------------------------------*/
void HiVoltageOn(PDEVTABLE pDev)
{
   HiVoltageSet(pDev, 0x01);
}

/*----------------------------------------------------------------------
Function name:  HiVoltageOff

Description:    Set High Voltage Off
                
Information:

Return:         0
----------------------------------------------------------------------*/
void HiVoltageOff(PDEVTABLE pDev)
{
   HiVoltageSet(pDev, 0x00);
}

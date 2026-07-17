/* -*-c++-*- */
/* $Header: gpio.h, 3, 10/11/00 8:55:40 PM, Brent$ 
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
** File name:   gpio.h
**
** Description: GPIO functions  functions.
**
** $Revision: 3$ 
** $Date: 10/11/00 8:55:40 PM$
** 
** $History:$
** 
** 
*/

#ifndef _GPIO_H_
#define _GPIO_H_

#define USEC (1000)
#define DTIME (5*USEC)
#define MSEC (1000000)
#define D60HZ (166*MSEC)
#define DELAY(x) WaitTime(x)
#define CLOCK_STRETCH (1000)
#define NUM_RETRY (5)

typedef struct pci_bit {
   DWORD dwBus;            // Bus
   DWORD dwDevFunc;       // DevNode
   DWORD RN;               // Register Number
   DWORD dInMask;          // Data Mask
   DWORD dOutMask;          // Data Mask
   DWORD dInShift;         // Data Shift
   DWORD dOutShift;         // Data Shift
   } PCI_BIT, * PPCI_BIT;
   

typedef struct gpiomask {
   PCI_BIT Data;
   PCI_BIT Clk;
   PCI_BIT Strobe;
   PCI_BIT HiVoltage;
   } GPIOMASK, * PGPIOMASK;

DWORD GetGpio(PPCI_BIT pPCI_BIT);
DWORD GetGpio_Bit(PPCI_BIT pPCI_BIT);
DWORD SetGpio_Bit(PPCI_BIT pPCI_BIT, DWORD dwValue);
DWORD SetGpio(PPCI_BIT pPCI_BIT, DWORD dwValue);

void gpio_scl(PGPIOMASK pGPIOMask, DWORD bit);
void gpio_sda(PGPIOMASK pGPIOMask, DWORD bit);
int gpio_read_scl(PGPIOMASK pGPIOMask);
int gpio_read_sda(PGPIOMASK pGPIOMask);
int gpio_send_byte(PGPIOMASK pGPIOMask, BYTE b1, BYTE b2, BYTE b3);
int FindGPIODevice(DWORD dwDevVenID, DWORD dwBus, PDWORD pdwBus, PDWORD pdwDevFunc);
int GPIOInit(PGPIOMASK pGPIOMask, DWORD dwNapalmBus);
DWORD clock(PDEVTABLE pDev);
DWORD Output_Clock(PDEVTABLE pDev, DWORD c, DWORD ttl, DWORD f, DWORD s, DWORD v, DWORD r);
void HiVoltageSet(PDEVTABLE pDev, DWORD bit);
void HiVoltageOn(PDEVTABLE pDev);
void HiVoltageOff(PDEVTABLE pDev);
#endif

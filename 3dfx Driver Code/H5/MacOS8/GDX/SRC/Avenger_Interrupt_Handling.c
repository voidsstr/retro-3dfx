/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   avenger_interrupt_handling.c
**
**
** $Header: Avenger_Interrupt_Handling.c, 6, 10/11/00 8:33:45 PM, Brent$
**
** $History: avenger_interrupt_handling.c $
** 
** *****************  Version 7  *****************
** User: Kcd          Date: 8/02/99    Time: 12:24p
** Updated in $/devel/h3/MacOS8/GDX/src
** Workaround for audio trashing problem.
** 
** *****************  Version 6  *****************
** User: Kcd          Date: 7/30/99    Time: 1:02p
** Updated in $/devel/h3/MacOS8/GDX/src
** Code formatting cleanup.
**
** *****************  Version 5  *****************
** User: Shuaulme     Date: 7/22/99    Time: 5:32p
** Updated in $/devel/h3/MacOS8/GDX/src
** interrupt handling stuff
**
** *****************  Version 3  *****************
** User: Kcd          Date: 7/19/99    Time: 3:44p
** Updated in $/devel/h3/MacOS8/GDX/src
** Fixed code that waits for VBL to do it properly.
**
** *****************  Version 2  *****************
** User: Kcd          Date: 7/02/99    Time: 3:53p
** Updated in $/devel/h3/MacOS8/GDX/src
** Implemented code to wait for vblank.
**
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
**
**
*/

#include "GraphicsPriv.h"
#include "GraphicsPrivHwc.h"
#include "GraphicsPrivData.h"
#include "avenger_interrupt_handling.h"
#include "gdx_debug.h"
#include <DCon.h>
#include <PCI.h>
#include <Devices.h>
#include <DriverServices.h>

/*
**=====================================================================================================
** 
** AvengerHandleVBLInterrupts()
**  This routines task is to clear the internal VBL interrupt source and re-prime it
**  for the next occurrence.  However, Avenger does not require any clearing or re-priming.
**
**=====================================================================================================
*/

#define FBI_FIFO_IDLE \
    do { \
	  HWC_SST_LOAD(bInfo->regInfo,status,status0); \
      HWC_SST_LOAD(bInfo->regInfo,status,status1); \
      HWC_SST_LOAD(bInfo->regInfo,status,status2); \
    } while(((status0 & SST_PCIFIFO_FREE) != SST_PCIFIFO_FREE) || \
            ((status1 & SST_PCIFIFO_FREE) != SST_PCIFIFO_FREE) || \
            ((status2 & SST_PCIFIFO_FREE) != SST_PCIFIFO_FREE));

#define FBI_FIFO_IDLE_SLAVES \
  do { \
    FxU32 chipNum; \
    for(i = 1; i < avengerHALData->numChips; i++) { \
      do { \
        HWC_SST_LOAD(avengerHALData->slaveRegInfo[i],status,status0); \
        HWC_SST_LOAD(avengerHALData->slaveRegInfo[i],status,status1); \
        HWC_SST_LOAD(avengerHALData->slaveRegInfo[i],status,status2); \
     } while(((status0 & SST_PCIFIFO_FREE) != SST_PCIFIFO_FREE) || \
             ((status1 & SST_PCIFIFO_FREE) != SST_PCIFIFO_FREE) || \
             ((status2 & SST_PCIFIFO_FREE) != SST_PCIFIFO_FREE)); \
   } \
 } while(0)
      
	  
      
#define kCfgInitEnable 64

void AvengerHandleVBLInterrupts(void *vblRefCon)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 status0, status1, status2, miscInit0;
  FxU32 cfgInitEnable, cfgInitEnableSave, i;

  /* Toggle some video memory */
  /*(FxU32 *)(avengerHALData->frameBufferBase) += 0x01010101;
  
  /* Scott's workaround for the PCI bus hogging problem. */
  /* Supposedly Napalm has a different workaround, revisit once we have silicon back. */
  
  /* Oh man this is ugly... */
#if 1
  /* Wait for PCI front end to idle. */
  FBI_FIFO_IDLE;
  FBI_FIFO_IDLE_SLAVES;
  
  /* Reset FBI fifo (bit 1 of miscInit0 */
  HWC_IO_LOAD(bInfo->regInfo,miscInit0,miscInit0);
  miscInit0 |= SST_FBI_FIFO_RESET;
  HWC_IO_STORE(bInfo->regInfo,miscInit0,miscInit0);
#endif
  
  /* Now do intrCtrl write so that it bypasses the FBI FIFO. */
  HWC_SST_STORE(bInfo->regInfo,intrCtrl,SST_INTR_PCI_INTA | avengerHALData->intrCrtl);
#if 1
  for(i = 1; i < avengerHALData->numChips; i++) {
    /* Since that probably just got broadcast to all chips, do another write to all
       slaves forcing interrupts off. */
    HWC_SST_STORE(avengerHALData->slaveRegInfo[i], intrCtrl, SST_INTR_PCI_INTA);
  }
#endif         
#if 1
  /* Again, wait for PCI front end to idle. */
  FBI_FIFO_IDLE;
  FBI_FIFO_IDLE_SLAVES;
  
  /* Unreset FBI fifo */
  HWC_IO_LOAD(bInfo->regInfo,miscInit0,miscInit0);
  miscInit0 &= ~SST_FBI_FIFO_RESET;
  HWC_IO_STORE(bInfo->regInfo,miscInit0,miscInit0);

  /* One last time...wait for PCI front end to idle. */
  FBI_FIFO_IDLE;
  FBI_FIFO_IDLE_SLAVES;
  
#endif

}

/*
__________________________________________________________________________________________

        AvengerEnableVBLInterrupts()

*/

void AvengerEnableVBLInterrupts(void *vblRefCon)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 pciInit0, syncPolarity;
  
  /* activate PCI interrupt */
  syncPolarity = ((*(volatile FxU8 *)(bInfo->regInfo.ioPortBase + 0xcc)) & 0x80);
  if(syncPolarity) { 
    /* Negative sync, trigger on falling edge */
    avengerHALData->intrCrtl |= SST_INTR_VSYNC_FALLING_ENABLE;
  } else {
    /* Positive sync, trigger on rising edge */
    avengerHALData->intrCrtl |= SST_INTR_VSYNC_RISING_ENABLE;
  }
  HWC_SST_STORE(bInfo->regInfo,intrCtrl, avengerHALData->intrCrtl );
}


/*
__________________________________________________________________________________________

        AvengerDisableVBLInterrupts()

*/
#define H3_IMASK (BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))

unsigned char AvengerDisableVBLInterrupts(void *vblRefCon)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 pciInit0;
  
  /* disable the interrupt */
  avengerHALData->intrCrtl &= ~(H3_IMASK);
  HWC_SST_STORE(bInfo->regInfo,intrCtrl, SST_INTR_PCI_INTA | avengerHALData->intrCrtl );

  return( NULL );
}


/*===================================================================================================== */
/* *
/* AvengerWaitForVBL() */
/* */
/*  This routine does not return until it senses that a VBL has occurred.  It is usually called before */
/*  updating the contents of the CLUT, with the assumption that the CLUT can be written during the  */
/*  Vertical Blanking Interval.   */
/* */
/*  This routine does not make any assumptions about whether VBL interrupts are */
/*  enabled or disabled prior to its calling.  However, it is recommended that the */
/*  VBL interrupts be disabled prior to calling this if any type of accurate waiting */
/*  takes place. */
/* */
/* */
/*===================================================================================================== */
void AvengerWaitForVBL( void )
{
  /* Before waiting in a loop, one should make sure that interrupts are enabled and that any */
  /* clock sources that cause the interrupts to fire away are also active.  If that is not the */
  /* case, the routine can exit right away.  Otherwise, sit in a loop waiting for the interrupt. */
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 intrCrtl, status, syncPolarity;
  HWC_SST_LOAD(bInfo->regInfo,intrCtrl,intrCrtl);
  syncPolarity = ((*(volatile FxU8 *)(bInfo->regInfo.ioPortBase + 0xcc)) & 0x80) >> 1;
  
  /* Fixme... this will never wait... we need to have some other flag that keeps */
  /* track of whether or not the display is running or not. */   
  if(intrCrtl & (SST_INTR_VSYNC_RISING_ENABLE|SST_INTR_VSYNC_FALLING_ENABLE)) {
    /* It's safe to wait. */
    /* First, spin until we are out of vblank. */
    do {
      HWC_SST_LOAD(bInfo->regInfo,status,status);
    } while((status & SST_VRETRACE) ^ syncPolarity);

    /* Next, spin until we get vblank again. */
    do {
      HWC_SST_LOAD(bInfo->regInfo,status,status);
    } while(!((status & SST_VRETRACE) ^ syncPolarity));
  }
}

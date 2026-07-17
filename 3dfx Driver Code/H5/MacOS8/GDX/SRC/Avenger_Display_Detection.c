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
** File name:   avenger_display_detection.c
**
**
** $Header: Avenger_Display_Detection.c, 7, 10/11/00 8:33:43 PM, Brent$
**
** $History: avenger_display_detection.c $
** 
** *****************  Version 8  *****************
** User: Kcd          Date: 7/30/99    Time: 1:02p
** Updated in $/devel/h3/MacOS8/GDX/src
** Code formatting cleanup.
**
** *****************  Version 7  *****************
** User: Kcd          Date: 7/27/99    Time: 3:29p
** Updated in $/devel/h3/MacOS8/GDX/src
** Power management support.
**
** *****************  Version 6  *****************
** User: Shuaulme     Date: 7/22/99    Time: 5:22p
** Updated in $/devel/h3/MacOS8/GDX/src
** target renamed to v3_release / interrupt handling code is working
**
** *****************  Version 5  *****************
** User: Kcd          Date: 7/19/99    Time: 3:47p
** Updated in $/devel/h3/MacOS8/GDX/src
** Added VGA monitor detection code.
**
** *****************  Version 4  *****************
** User: Shuaulme     Date: 6/21/99    Time: 5:25p
** Updated in $/devel/h3/MacOS8/GDX/src
** Chars cleanup
**
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
**
**
*/

#define TYPE_LONGLONG 1

#include "GraphicsPriv.h"
#include "GraphicsPrivHwc.h"
#include "GraphicsPrivData.h"
#include "GraphicsOSS.h"
#include "avenger_CLUT.h"
#include "avenger_interrupt_handling.h"
#include "avenger_display_detection.h"
#include "avenger_VMI.h"

#include "gdx_debug.h"
#include "gdx_runtime.h"


GDXErr AvengerSetMode(FxU32 width, FxU32 height, FxU32 refresh);
static void 	AvengerDelayForDuration(Duration duration);





/*----------------------------------------------------------------------
Function name:  AvengerResetSenseLines

Description:    Before reading the Sense Lines to determine what monitor
                is connected, the sense lines need to be reset.  Puts
                sense lines in a known state so the sensing process can
                start.

Information:

Return:         
----------------------------------------------------------------------*/
void AvengerResetSenseLines(void)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();

  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  UInt32 senseLineValue = 0;

  /*
      Tri-state the DDC sense lines, and make sure we are reading
      the VGA DDC lines.
  */
  
  HWC_IO_LOAD(bInfo->regInfo, vidSerialParallelPort, senseLineValue);
  __eieio();

#if H5
  senseLineValue &= ~(SST_SERPAR_GPIO_1 | SST_SERPAR_DDC_EN);
#else
  senseLineValue &= ~SST_SERPAR_DDC_EN;
#endif

  HWC_IO_STORE(bInfo->regInfo, vidSerialParallelPort, senseLineValue);
  __eieio();

  /*
      Wait for lines to stabilize
  */
  
  AvengerDelayForDuration(3*durationMillisecond);
}





/*----------------------------------------------------------------------
Function name:  AvengerReadSenseLines

Description:    Read the state of the sense lines.

Information:

Return:         senseLineValue   read value of the sense lines
----------------------------------------------------------------------*/
UInt32 AvengerReadSenseLines(void)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  UInt32 senseLineValue = 0;

  /*
      The rawSenseCode is obtained by making sure the sense lines are
      reset (i.e., none of the lines are being actively driven by the
      frame buffer controller) and then reading the state of the lines.
      
      In all Voodoo implementation, we do not have all three sense lines,
      only sense1 and sense2 are being read.

  */
  
  AvengerResetSenseLines();

  /* Load value. */
  __eieio();
  HWC_IO_LOAD(bInfo->regInfo, vidSerialParallelPort, senseLineValue);

  senseLineValue = (senseLineValue >> 21) & 0x3;    /* Get sense bits into least significant bits */

  /* Note: Don't use the monitor sense pin.  It seems to confuse monitor tagging. */
  /*senseLineValue |= (*(FxU8 *)(avengerHALData->bInfo.regInfo.ioPortBase + 0xc2) & 0x10) >> 2; */

  return(senseLineValue);
}





/*----------------------------------------------------------------------
Function name:  AvengerFillDAC

Description:    fills the DAC with the given color values.

Return:         -
----------------------------------------------------------------------*/
static void AvengerFillDAC(hwcBoardInfo *bInfo, FxU32 red, FxU32 green, FxU32 blue)
{
  FxU32 regBase = bInfo->regInfo.ioPortBase;
  Boolean vblInterruptsEnabled;
  FxU32 i;

  /* Now really program hardware */
  for(i = 0; i < 512; i++)
  {
    FxU32 data = (red << 16) | (green << 8) | blue;

    CHECKFORROOM;
    HWC_IO_STORE( bInfo->regInfo, dacAddr, i);
    P6FENCE;

    CHECKFORROOM;
    HWC_IO_STORE( bInfo->regInfo, dacData, data);
    P6FENCE;
  }
}





/*----------------------------------------------------------------------
Function name:  AvengerGetTrigger

Description:    reads the value of the analog feedback register used
                to detect wether or not a display is connected to the
                vga connector.
                the DAC must be setup properly (using AvengerFillDAC)
                for this function to work properly

Return:         true if trigger is active
                flase if not
----------------------------------------------------------------------*/
static FxU32 AvengerGetTrigger(hwcBoardInfo *bInfo)
{
  FxU32 i, result, status;
  Boolean vblInterruptsEnabled;

  /* Wait for blanking */
  do {
    status = *(volatile FxU8 *)(bInfo->regInfo.ioPortBase + 0xda);
  } while (!(status & 1));

  /* Wait for non-blanking */
  do {
    status = *(volatile FxU8 *)(bInfo->regInfo.ioPortBase + 0xda);
  } while (status & 1);

  /* Return status of trigger (inverted). */
  /* %%KCD - I found that unless I read this a few times, I would pretty much */
  /* never get back a useful result.  We bail out as soon as we see a valid */
  /* trigger. */
  for(i = 0; i < 10; i++) {
    result = ~(*(volatile FxU8 *)(bInfo->regInfo.ioPortBase + 0xc2)) & 0x10;
    if(result) {
      break;
    }
  }

  return result;
}





/*----------------------------------------------------------------------
Function name:  AvengerGetMonitorType

Description:    

Return:         
----------------------------------------------------------------------*/
FxI32 AvengerGetMonitorType(void)
{
#define FN_NAME "AvengerGetMonitorType"
#define FN_LEVEL 1
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 dacRed, dacGreen, dacBlue, redTrigger, greenTrigger;
  FxI32 result = -1, i;

  LOG_ENTRY(FN_LEVEL);

  /* If we haven't been told to set up some display mode (very likely), then */
  /* we set the hardware for our default mode, which should be the same as what */
  /* gets set by the OpenFirmware stuff.   The obvious better choice would be */
  /* to set it to whatever mode was saved in NVRAM, but for now this will have */
  /* to do.   */
  if(avengerHALData->displayMode == 0) {
    AvengerSetMode(640,480,60);
  }

  dacRed = dacGreen = dacBlue = 0x0;
  redTrigger = greenTrigger = 0;

  avengerHALData->redTrigger = avengerHALData->greenTrigger = 0;

  /* First find the red trigger value. */
  while(dacRed < 256) {
    AvengerFillDAC(bInfo, dacRed, dacGreen, dacBlue);
    if(AvengerGetTrigger(bInfo)) {
      redTrigger = dacRed;
      break;
    }
    dacRed += 1;
  }

  /* Check for error condition */
  if(dacRed == 256) {
    goto error;
  }

  dacRed = 0; /*0x08 << 2; */

  /* Now find the green trigger value. */
  while(dacGreen < 256) {
    AvengerFillDAC(bInfo, dacRed, dacGreen, dacBlue);
    if(AvengerGetTrigger(bInfo)) {
      greenTrigger = dacGreen;
      break;
    }
    dacGreen += 1;
  }

  /* Check for error condition */
  if(dacGreen == 256) {
    goto error;
  }

  /* Some debugging... */
  avengerHALData->redTrigger = redTrigger;
  avengerHALData->greenTrigger = greenTrigger;

  /* Assume no monitor. */
  result = 0;

  /* Now figure out monitor type. */
  if(greenTrigger >= (0x18 << 2)) {
    /* We at least know we have a mono monitor at this point. */
    result = 1;
    if(redTrigger >= (0x18 << 2)) {
      /* We know it's color. */
      result = 2;
    }
  }

error:
#if DEBUG
  switch( result ) {
    case 0:
      LOG_PRINTF(FN_LEVEL,"*NO* monitor detected!!\n");
      break;
    
    case 1:
      LOG_PRINTF(FN_LEVEL,"monochrom monitor detected!!\n");
      break;
    
    case 2:
      LOG_PRINTF(FN_LEVEL,"color monitor detected!!\n");
      break;
    }
#endif
  /* Clear out test values. */
  AvengerFillDAC(bInfo,0,0,0);

  LOG_EXIT(FN_LEVEL,0);
  return result;
#undef FN_NAME
#undef FN_LEVEL
}





/*----------------------------------------------------------------------
Function name:  AvengerDelayForDuration

Description:    The DSL routine DelayFor() cannot be called at Primary
                Interrupt Level or Secondary Task Level.
                This provides equivalent functionality by calling the
                special DelayForHardware().

Information:    -> duration   the number of microseconds/milliseconds for the delay
                                  if the number is negative, it is the # of microseconds to delay
                                  if the number is positive, it is the # of milliseconds to delay

Return:         senseLineValue   read value of the sense lines
----------------------------------------------------------------------*/
static void AvengerDelayForDuration(Duration duration)
{
  AbsoluteTime delayTime;           /* The duration of delay in # ticks */

	delayTime = DurationToAbsolute(duration);
	DelayForHardware(delayTime);

}


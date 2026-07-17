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
** File name:   avenger_CLUT.c
**
**
** $Header: Avenger_CLUT.c, 7, 10/11/00 8:33:53 PM, Brent$
**
** $History: avenger_CLUT.c $
** 
** *****************  Version 4  *****************
** User: Kcd          Date: 7/30/99    Time: 1:02p
** Updated in $/devel/h3/MacOS8/GDX/src
** Code formatting cleanup.
**
** *****************  Version 3  *****************
** User: Kcd          Date: 7/19/99    Time: 3:45p
** Updated in $/devel/h3/MacOS8/GDX/src
** More compact CLUT handling code.
**
** *****************  Version 2  *****************
** User: Kcd          Date: 7/08/99    Time: 1:12p
** Updated in $/devel/h3/MacOS8/GDX/src
** Removed external DAC code (commented out).
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
#include "GraphicsOSS.h"
#include "avenger_CLUT.h"
#include "avenger_interrupt_handling.h"
#include "avenger_VMI.h"

#include "gdx_runtime.h"

#include <3dfx.h>

/*
**=====================================================================================================
**
** AvengerMapDepthModeToCLUTAttributes()
**  This simple routine maps a 'DepthMode' to the corresponding CLUT attritributes associate with it.
**
**    -> depthMode  The 'DepthMode' for which the CLUT attributes are desired.
**    <- startAddress The physical address that corresponds to logical address 0.
**    <- entryOffset  The physical offset between each logical address.
**
**  This routine is private to the Avenger Graphics HAL.
**
**=====================================================================================================
*/
GDXErr AvengerMapDepthModeToCLUTAttributes(DepthMode depthMode, UInt32 *startAddress,
		UInt32 *entryOffset)
{
  /* Define a new type which maps a 'DepthMode' to the corresponding physical attributes of a CLUT.
  ** These attributes are needed because the logical addresses when in 1, 2, or 4 bpp do not
  ** correspond to the physical address.  Instead, they are distributed evenly through out the CLUT
  ** address space.
  */
	typedef struct DepthModeToCLUTAttributesMap DepthModeToCLUTAttributesMap;
	struct DepthModeToCLUTAttributesMap
	{
    DepthMode depthMode;      /* DepthMode for which these attributes apply   */
    UInt32 startAddress;      /* Physical address of logical address 0        */
    UInt32 entryOffset;       /* Physical offset between each logical address */
	};

	enum {kMapSize = 3};

	DepthModeToCLUTAttributesMap depthModeMap[kMapSize] =
	{
    {kDepthMode1, 0x00, 0x01},        /* 8 bpp  */
    {kDepthMode2, 0x00, 0x08},        /* 16 bpp */
    {kDepthMode3, 0x00, 0x01},        /* 32 bpp */
	};

  UInt32 i;

  GDXErr err = kGDXErrDepthModeUnsupported;

  /* Scan the 'DepthModeToCLUTAttributesMap' to find CLUT attributes for the given depth mode.   */

  for (i = 0 ; i < kMapSize ; i++) {
    if (depthModeMap[i].depthMode == depthMode) {
			*startAddress = depthModeMap[i].startAddress;
			*entryOffset = depthModeMap[i].entryOffset;
			err = kGDXErrNoError;
			break;
		}
	}

ErrorExit:

	return (err);
}


void AvengerProgramCLUT(void)
{
  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 regBase = avengerHALData->bInfo.regInfo.ioPortBase;
  Boolean vblInterruptsEnabled;
  FxU32 i;
  FxU32 temp;

  /* Turn off VBL interrupts so the CLUT can be adjusted during VBL time, without having to worry
     about the VBL interrupt handler stealing time. */

  //vblInterruptsEnabled = GraphicsOSSSetVBLInterrupt(false);
#if ENABLE_INTERRUPTS  
  vblInterruptsEnabled = (avengerHALData->intrCrtl & (SST_INTR_VSYNC_FALLING_ENABLE|SST_INTR_VSYNC_RISING_ENABLE)) ? 1 : 0;
  if(vblInterruptsEnabled) {
    AvengerDisableVBLInterrupts(0);
  }	  
  AvengerWaitForVBL();
#endif

  /* Now really program hardware */
  for(i = 0; i < 512; i++) {
    FxU32 data;
    data = (avengerHALData->baseCLUT.entry[i].r << RED_SHIFT) |
           (avengerHALData->baseCLUT.entry[i].g << GREEN_SHIFT) |
           (avengerHALData->baseCLUT.entry[i].b << BLUE_SHIFT);

retry1:
    CHECKFORROOM;
    HWC_IO_STORE( bInfo->regInfo, dacAddr, i);
    P6FENCE;
    HWC_IO_LOAD(bInfo->regInfo, dacAddr, temp);  /* Hardware quirk */
    if(temp != i)
      goto retry1;
      
retry2:      
    CHECKFORROOM;
    HWC_IO_STORE( bInfo->regInfo, dacData, data);
    P6FENCE;
    HWC_IO_LOAD(bInfo->regInfo, dacData, temp);  /* Hardware quirk */
    if(temp != data)
      goto retry2;
  }
#if ENABLE_INTERRUPTS
  if (vblInterruptsEnabled) {
    //(void) GraphicsOSSSetVBLInterrupt(true);
    AvengerEnableVBLInterrupts(0);
  }
#endif  
}





void AvengerProgramCLUTColor(FxU32 color)
{
  enum { kClutSize = 256 };

  AvengerHALData *avengerHALData = GraphicsHALGetHALData();
  hwcBoardInfo *bInfo = &avengerHALData->bInfo;
  FxU32 regBase = avengerHALData->bInfo.regInfo.ioPortBase;
  Boolean vblInterruptsEnabled;
  FxU32 i;
  FxU32 temp;

  /* Turn off VBL interrupts so the CLUT can be adjusted during VBL time, without having to worry
     about the VBL interrupt handler stealing time. */

  //vblInterruptsEnabled = GraphicsOSSSetVBLInterrupt(false);
#if ENABLE_INTERRUPTS
  vblInterruptsEnabled = (avengerHALData->intrCrtl & (SST_INTR_VSYNC_FALLING_ENABLE|SST_INTR_VSYNC_RISING_ENABLE)) ? 1 : 0;
  if(vblInterruptsEnabled) {
    AvengerDisableVBLInterrupts(0);
  }	  
  AvengerWaitForVBL();
#endif

  /* Now really program hardware */
  for (i = 0; i < kClutSize*2; i++) {
//  for(i = 0; i < 1; i++) {
    FxU32 data;
    data = color;

retry1:
    CHECKFORROOM;
    HWC_IO_STORE( bInfo->regInfo, dacAddr, i);
    P6FENCE;
    HWC_IO_LOAD(bInfo->regInfo, dacAddr, temp);  /* Hardware quirk */
    if(temp != i)
      goto retry1;
      
retry2:      
    CHECKFORROOM;
    HWC_IO_STORE( bInfo->regInfo, dacData, data);
    P6FENCE;
    HWC_IO_LOAD(bInfo->regInfo, dacData, temp);  /* Hardware quirk */
    if(temp != data)
      goto retry2;
  }
#if ENABLE_INTERRUPTS  
  if (vblInterruptsEnabled) {
    //(void) GraphicsOSSSetVBLInterrupt(true);
    AvengerEnableVBLInterrupts(0);
  }
#endif  
}





#if 0
void SetDAC555(FxU32 regBase, FxU32 enable)
{
	VMI_IDLE(regBase);
	VMI_ADDR_phase(regBase,0x06);
	VMI_WRITE_phase(regBase,0xd0 | enable);
	VMI_ENDW_phase(regBase);
	VMI_IDLE(regBase);
	VID_DELAY(regBase);
}
#endif

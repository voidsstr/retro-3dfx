/*
 ** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
 **
 ** $Revision: 5$ 
 ** $Date: 10/11/00 7:40:03 PM$ 
 **
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3dfx.h>
#include <gdebug.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <fxmemmap.h>
#include "fxhwc.h"
#include "hwcpio.h"
#include "init.h"

#define DEVICE_NUMBER(_dev) ( (_dev)-hwcInfo.devices)

#if defined( __unix__ )

static char pciIdent[] = "@#% fxPCI for Windows NT";

extern "C" int unixError(void);

int 
unixError(void) {
    fprintf(stderr,"ERROR: pioIn* and pioOut* not implemented on UNIX\n");
    exit(1);
    return 0;
}

FxBool hwcInitializeDDio(void) {
    return FXFALSE;
}

FX_EXPORT FxBool FX_CSTYLE
hwcMapPhysicalToLinear( FxU32 *linear_addr, FxU32 physical_addr, FxU32 *length ) 
{ 
    return FXFALSE;
}

void hwcUnmapPhysical( FxU32 linear_addr, FxU32 length ) 
{
}

FX_EXPORT FxBool FX_CSTYLE
hwcPCIShutdown( void )
{
    return FXTRUE;
}

#endif /* __unix__ */

/* PRIVATE FUNCTIONS */

FxU32
_hwcCreateConfigAddress( FxU32 bus_number, FxU32 device_number,  
                         FxU32 function_number, FxU32 register_offset )
{ 
  FxU32 retval = CONFIG_ADDRESS_ENABLE_BIT;
  
  retval |= ( bus_number & 0xFF ) << 16;
  retval |= ( device_number & 0x1F ) << 11;
  retval |= ( function_number & 0x7 ) << 8;
  retval |= ( register_offset & 0xFC );
  return retval;
} /* _hwcCreateConfigAddress */

FxU16 
_hwcCreateConfigSpaceMapping( FxU32 device_number, FxU32 register_offset )
{

  FxU16 retval = 0;
  retval |= ( device_number & 0xFF ) << 8;
  retval |= ( register_offset & 0xFC );
  retval += CONFIG_MAPPING_OFFSET;
  return retval;
} /* _hwcCreateConfigSpaceMapping */

// Fetch register is currently being used for hardware access only, csim/hsim have backdoor methods to do this
#ifdef notdef
FxU32 
_hwcFetchRegister( FxU32 offset, FxU32 size_in_bytes, 
                   FxU32 device_number, FxU32 config_mechanism  )
{ 
  FxU32 retval, retval1;
  FxU32 slot, bus;
  
  bus  = device_number >> 5;
  slot = device_number & 0x1f;
  
  if ( config_mechanism == 1 ) {
    hwcIOStore32( CONFIG_ADDRESS_PORT, _hwcCreateConfigAddress( bus, slot, 0, offset ) );
    retval = hwcIOLoad32( CONFIG_DATA_PORT );
    retval >>= 8 * ( offset & 0x3 );
#ifndef  __unix__
    _outpd(CONFIG_ADDRESS_PORT, _hwcCreateConfigAddress( bus, slot, 0, offset ) );
    retval1 = _inpd(CONFIG_DATA_PORT );
    retval1 >>= 8 * ( offset & 0x3 );
#endif
  } else {                      /* config mechanism 2 */
    hwcIOStore8( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_ENABLE_BYTE );
    retval = hwcIOLoad32( _hwcCreateConfigSpaceMapping( device_number, offset ) ); 
    retval >>= 8 * ( offset & 0x3 );
    hwcIOStore8( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_DISABLE_BYTE );
  }
  
  switch( size_in_bytes ) {
  case 1:
    retval &= 0xFF;
    break;
  case 2:
    retval &= 0xFFFF;
    break;
  default:                      /* 4 bytes */
    break;
  }
  
  return retval;
} /* _hwcFetchRegister */
#else
FxU32 
_hwcFetchRegister( FxU32 offset, FxU32 size_in_bytes, 
                        FxU32 device_number, FxU32 config_mechanism  )
{ 
  FxU32 retval;
  FxU32 slot, bus, function;
  
  /*
   * device_number[0:4]   = slot
   * device_number[5:12]  = bus
   * device_number[13:15] = function
   */

  slot     = (device_number) & 0x1f;
  bus      = (device_number >> 5) & 0xFF; 
  function = (device_number >> 13) & 0x7;
  
  if ( config_mechanism == 1 ) {
    _pioOutLong( CONFIG_ADDRESS_PORT, _hwcCreateConfigAddress( bus, slot, function, offset ) );
    retval = _pioInLong( CONFIG_DATA_PORT );
    retval >>= 8 * ( offset & 0x3 );
  } else {                      /* config mechanism 2 */
    _pioOutByte( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_ENABLE_BYTE );
    retval = _pioInLong( _hwcCreateConfigSpaceMapping( device_number, offset ) ); 
    retval >>= 8 * ( offset & 0x3 );
    _pioOutByte( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_DISABLE_BYTE );
  }
  
  switch( size_in_bytes ) {
  case 1:
    retval &= 0xFF;
    break;
  case 2:
    retval &= 0xFFFF;
    break;
  default:                      /* 4 bytes */
    break;
  }
  
  return retval;
} /* _pciFetchRegister */
#endif

// Update register is currently being used for hardware access only, csim/hsim have backdoor methods to do this
#ifdef notdef
void
_hwcUpdateRegister( FxU32 offset, FxU32 data, FxU32 size_in_bytes,  
                    FxU32 device_number, FxU32 config_mechanism  ) 
{
  FxU32
    regval =  _hwcFetchRegister( offset & ( ~0x3 ), 4,
                                 device_number, config_mechanism );
  FxU32 mask = (FxU32) ~0l;
  FxU32 bus, slot;
  
  bus  = device_number >> 5;
  slot = device_number & 0x1f;
  
  switch( size_in_bytes ) {
  case 1:
    mask &= 0xFF;
    data &= 0xFF;
    break;
  case 2:
    mask &= 0xFFFF;
    data &= 0xFFFF;
    break;
  case 4:
  default:
    break;
  }
  
  data <<= 8 * ( offset & 0x03 );
  mask <<= 8 * ( offset & 0x03 );
  
  regval = ( regval & ~mask ) | data;
  
  if ( config_mechanism == 1 ) {
    hwcIOStore32( CONFIG_ADDRESS_PORT, _hwcCreateConfigAddress( bus, slot, 0, offset ) );
    hwcIOStore32( CONFIG_DATA_PORT, regval );
  } else {                      /* config mechanism 2 */
    hwcIOStore8( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_ENABLE_BYTE );
    hwcIOStore32( _hwcCreateConfigSpaceMapping( device_number, offset ), regval );
    hwcIOStore8( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_DISABLE_BYTE );
  }
  
  return;
} /* _hwcUpdateRegister */
#else
void
_hwcUpdateRegister( FxU32 offset, FxU32 data, FxU32 size_in_bytes,  
                   FxU32 device_number, FxU32 config_mechanism  ) 
{
  FxU32
    regval =  _hwcFetchRegister( offset & ( ~0x3 ), 4,
                                device_number, config_mechanism );
  FxU32 mask = (FxU32) ~0l;
  FxU32 bus, slot, function;
  
  /*
   * device_number[0:4]   = slot
   * device_number[5:12]  = bus
   * device_number[13:15] = function
   */

  slot     = (device_number) & 0x1f;
  bus      = (device_number >> 5) & 0xFF;
  function = (device_number >> 13) & 0x7;
  
  switch( size_in_bytes ) {
  case 1:
    mask &= 0xFF;
    data &= 0xFF;
    break;
  case 2:
    mask &= 0xFFFF;
    data &= 0xFFFF;
    break;
  case 4:
  default:
    break;
  }
  
  data <<= 8 * ( offset & 0x03 );
  mask <<= 8 * ( offset & 0x03 );
  
  regval = ( regval & ~mask ) | data;
  
  if ( config_mechanism == 1 ) {
    _pioOutLong( CONFIG_ADDRESS_PORT, _hwcCreateConfigAddress( bus, slot, function, offset ) );
    _pioOutLong( CONFIG_DATA_PORT, regval );
  } else {                      /* config mechanism 2 */
    _pioOutByte( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_ENABLE_BYTE );
    _pioOutLong( _hwcCreateConfigSpaceMapping( device_number, offset ), regval );
    _pioOutByte( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_DISABLE_BYTE );
  }
  
  return;
} /* _pciUpdateRegister */
#endif

/* PUBLIC DATA  */

const HwcPciRegister pciRegs[] = {
                    { 0x0,  2, HWC_READ_ONLY },   /* PCI_VENDOR_ID       */
                    { 0x2,  2, HWC_READ_ONLY },   /* PCI_DEVICE_ID       */
                    { 0x4,  2, HWC_READ_WRITE },  /* PCI_COMMAND         */
                    { 0x6,  2, HWC_READ_WRITE },  /* PCI_STATUS          */
                    { 0x8,  1, HWC_READ_ONLY },   /* PCI_REVISION_ID     */
                    { 0x9,  3, HWC_READ_ONLY },   /* PCI_CLASS_CODE      */
                    { 0xC,  1, HWC_READ_WRITE },  /* PCI_CACHE_LINE_SIZE */
                    { 0xD,  1, HWC_READ_WRITE },  /* PCI_LATENCY_TIMER   */
                    { 0xE,  1, HWC_READ_ONLY },   /* PCI_HEADER_TYPE     */
                    { 0xF,  1, HWC_READ_WRITE },  /* PCI_BIST            */
                    { 0x10, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_0  */
                    { 0x14, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_1  */
                    { 0x18, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_2  */
                    { 0x1C, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_3  */
                    { 0x20, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_4  */
                    { 0x24, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_5  */
                    { 0x28, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_6  */ 
                    { 0x2c, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_7  */ 
                    { 0x30, 4, HWC_READ_WRITE },  /* PCI_BASE_ADDRESS_8  */ 
                    /* 0x34->3B - Reserved */
                    { 0x3C, 1, HWC_READ_WRITE },  /* PCI_INTERRUPT_LINE  */
                    { 0x3D, 1, HWC_READ_ONLY },   /* PCI_INTERRUPT_PIN   */
                    { 0x3E, 1, HWC_READ_ONLY },   /* PCI_MIN_GNT         */
                    { 0x3F, 1, HWC_READ_ONLY }    /* PCI_MAX_LAT         */
};

/* 
   internal routine used during library initialization to determine 
   if device exists and if so add it to the list of available 
   hardware devices. We need to use the low level hardware io 
   routines here rather than FetchRegister since we have not
   established device state yet. 
*/

static void 
probeDevice(int deviceNumber, int configMode) {
  FxU32 regVal, slot, bus;
  HwcDevice *dev = &hwcInfo.devices[deviceNumber];
  
  /* set the config mode: this routine also determines 
     which config mode works */

  hwcInfo.configMechanism = configMode;
  bus  = deviceNumber >> 5;
  slot = deviceNumber & 0x1f;

  /* read first DWORD in config space vendor id/device id */
  
  if ( configMode == 1 ) {
    _pioOutLong(CONFIG_ADDRESS_PORT, _hwcCreateConfigAddress( bus, slot, 0, 0 ) );
    regVal = _pioInLong(CONFIG_DATA_PORT );
  } else {                      /* config mechanism 2 */
    _pioOutByte( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_ENABLE_BYTE );
    regVal = _pioInLong( _hwcCreateConfigSpaceMapping( deviceNumber, 0 ) );
    _pioOutByte( CONFIG_ADDRESS_PORT, CONFIG_MAPPING_DISABLE_BYTE );
  }

  memset( dev, 0, sizeof( *dev ) );

  if ( ( regVal & 0xFFFF0000 ) != 0xFFFF0000 ) {
    dev->state = HWC_HAS_HW;
    dev->vendorID = (FxU16)(regVal & 0xFFFF);
    dev->deviceID = (FxU16)(regVal >> 16);
    hwcInfo.busDetected = FXTRUE;
  } else dev->state = HWC_NONE;
}

/* find an existing device of the specified type which we can coexecute with or an unused 
   device address we can use for simulation */

FX_EXPORT HwcDevice * FX_CSTYLE
hwcFakeDevice( FxU16 vendorID, FxU16 deviceID, HwcPCIState state)
{
  FxU32 devNum;
  HwcDevice *dev;

  // see if device already exists if it does just add in desired simulators
  if ( dev = hwcFindDevice(vendorID, deviceID, 0)) {
      dev->state |= state;
      return dev;
  }

  for ( devNum = 0; devNum < hwcInfo.maxDevices; devNum++ ) {
    dev = &hwcInfo.devices[devNum];
    if ( dev->state == HWC_NONE ) {
      dev->vendorID = vendorID;
      dev->deviceID = deviceID;
      dev->state = state;
      return dev;
    }
  }
  return NULL;
}

/* map a device number to a HwcContext, if its unknown return NULL */

HwcContext *
hwcDeviceToHwcContext(FxU32 deviceNumber)
{
  int i;

  for(i = 0; i < HWC_MAX_BOARDS; i++) {
    if ( deviceNumber == hwcInfo.boardInfo[i].devNum )
        return &hwcInfo.boardInfo[i];
  }

  return NULL;
}

FX_EXPORT FxBool FX_CSTYLE
hwcPCIInit( void ) {
  int devNum;
  
  hwcInfo.pciRegs = pciRegs;
  hwcInfo.busDetected = FXFALSE;
  
  for ( devNum = 0; devNum < HWC_MAX_PCI_DEVICES; devNum++ ) {
    hwcInfo.devices[devNum].state = HWC_NONE;
  }

  if ( hwcInitializeDDio() ) {
   
    /*
     **      Scan All PCI device numbers
     */ 
    
    for ( devNum = 0; devNum < HWC_MAX_PCI_DEVICES; devNum++ ) {
      probeDevice(devNum,1);
    }
    
    if ( !hwcInfo.busDetected )  { 
      /* Try Configuration Mechanism 2 (only 16 devices) */
      /* Since Configuration Mech#2 is obsolete this does not
         support multiple busses */
  
      for ( devNum = 0; devNum < 16; devNum++ ) {
        probeDevice(devNum,2);
      }
      hwcInfo.maxDevices = 16;
    } else hwcInfo.maxDevices = HWC_MAX_PCI_DEVICES;
    if ( !hwcInfo.busDetected ) {
      GDBG_ERROR("hwcInit", "No PCI Bus detected.\n");
      return FXFALSE;
    }    
 } else { // just do simulation
    hwcInfo.maxDevices = HWC_MAX_PCI_DEVICES;
 } 
  
  return FXTRUE;
} /* hwcPCIInit */

FX_EXPORT HwcPCIState FX_CSTYLE
hwcDeviceState( HwcDevice *dev ) {
  FxU32 device_number = DEVICE_NUMBER(dev);

  if ( !hwcInfo.initialized ) {
    GDBG_ERROR("hwcDeviceState", "HWC library not initialized\n");
    return FXFALSE;
  }
  if ( device_number > hwcInfo.maxDevices ) {
    GDBG_ERROR("hwcDeviceState", "device number (%d) out of range max %d\n", 
                device_number, hwcInfo.maxDevices);
    return FXFALSE;
  }
  return dev->state;
} /* hwcDeviceState */

FX_EXPORT FxBool FX_CSTYLE 
hwcGetConfigData( HwcDevice *dev, HwcPciRegister reg, FxU32 *data )
{
  FxU32 device_number = DEVICE_NUMBER(dev);

  if ( !hwcInfo.initialized ) {
    GDBG_ERROR("hwcGetConfigData", "HWC library not initialized\n");
    return FXFALSE;
  }
  
  if ( reg.rwFlag == HWC_WRITE_ONLY ) {
    GDBG_ERROR("hwcGetConfigData", "Cannot read a WRITE_ONLY register.\n");
    return FXFALSE;
  }
  
  if ( device_number > hwcInfo.maxDevices ) {
    GDBG_ERROR("hwcGetConfigData", "device number (%d) out of range max %d\n", 
                device_number, hwcInfo.maxDevices);
    return FXFALSE;
  }

  if ( dev->state == HWC_NONE ) {
    GDBG_ERROR("hwcGetConfigData", "device (0x%lx) does not exist\n", 
                device_number);
    return FXFALSE;
  }

  // if ( dev->hwc && ( dev->hwc->state == HWC_CTX_MAPPED )) {
  if ( dev->hwc ) {
	HwcSimulator *hws;

	for (hws = dev->hwc->hws; hws; hws = hws->next) {
		(*hws->GetConfigData) (hws, reg, data);
	    hwcIoVector(hws, HWC_CFG_READ, reg.sizeInBytes<<3, reg.regAddress, *data);
	}
  } else {
    if ( dev->state & HWC_HAS_HW ) {
      *data = _hwcFetchRegister( reg.regAddress, reg.sizeInBytes, 
                                 device_number, hwcInfo.configMechanism );
    } else {
      switch ( reg.regAddress ) {
      case 0:
        *data = dev->vendorID;
        break;
      case 2:
        *data = dev->deviceID;
        break;
      case 9:
        *data = 0x040000;
        break;
      default:
        *data = 0xdeadbeef;
        break;
      }
    }

  }

  return FXTRUE;
} /* hwcGetConfigData */

FX_EXPORT FxBool FX_CSTYLE
hwcSetConfigData( HwcDevice *dev, HwcPciRegister reg, FxU32 *data )
{
  FxU32 device_number = DEVICE_NUMBER(dev);

  if ( !hwcInfo.initialized ) {
    GDBG_ERROR("hwcSetConfigData", "HWC library not initialized\n");
    return FXFALSE;
  }
  
  if ( reg.rwFlag == HWC_READ_ONLY ) {
    GDBG_ERROR("hwcGetConfigData", "Cannot write a READ_ONLY register.\n");
    return FXFALSE;
  }
  
  if ( device_number > hwcInfo.maxDevices ) {
    GDBG_ERROR("hwcSetConfigData", "device number (%d) out of range max %d\n", 
                device_number, hwcInfo.maxDevices);
    return FXFALSE;
  }

  if ( dev->state == HWC_NONE ) {
    GDBG_ERROR("hwcSetConfigData", "device (0x%lx) does not exist\n", 
                device_number);
    return FXFALSE;
  }


  // if ( dev->hwc && ( dev->hwc->state == HWC_CTX_MAPPED )) {
  if ( dev->hwc ) {
	HwcSimulator *hws;

	for (hws = dev->hwc->hws; hws; hws = hws->next) {
	    hwcIoVector(hws, HWC_CFG_READ, reg.sizeInBytes<<3, reg.regAddress, *data);
		(*hws->SetConfigData) (hws, reg, *data);
	}
  } else {
    if ( dev->state & HWC_HAS_HW ) {
      _hwcUpdateRegister( reg.regAddress, *data, reg.sizeInBytes, 
                          device_number, hwcInfo.configMechanism );
    }
    switch (reg.regAddress) {
    case 4:            /* status | command     */
      dev->hwc->memEnabled = *data & SST_PCIMEM_ACCESS_EN;
      dev->hwc->ioEnabled = *data & SST_PCIIO_ACCESS_EN;
      GDBG_INFO(4, "    memEnabled = %d\n", dev->hwc->memEnabled != 0);
      GDBG_INFO(4, "    ioEnabled = %d\n", dev->hwc->ioEnabled != 0);
      break;
    }
  }
  
  return FXTRUE;
} /* hwcSetConfigData */

/* find the device number of the cardNum'th device which has the
   specified vendorID and deviceID
 */

FX_EXPORT HwcDevice * FX_CSTYLE
hwcFindDevice(FxU32 vendorID, FxU32 deviceID, FxU32 cardNum)
{
  FxU32 deviceNumber;
  HwcDevice *dev;

  if ( !hwcInfo.initialized ) {
    GDBG_ERROR("hwcSetConfigData", "HWC library not initialized\n");
    return FXFALSE;
  }
  
  /*      2) scan the existing devices for a match */
  for ( deviceNumber = 0; deviceNumber < hwcInfo.maxDevices; deviceNumber++ ) {
    dev = &hwcInfo.devices[deviceNumber];
    if ( dev->state != HWC_NONE ) {
      FxU32 vID, dID;

      hwcGetConfigData( dev, hwcInfo.pciRegs[PCI_VENDOR_ID], &vID );
      hwcGetConfigData( dev, hwcInfo.pciRegs[PCI_DEVICE_ID], &dID );
      if ((vID == vendorID) && ((dID == deviceID) || deviceID==0xFFFF)) {
        if (cardNum == 0) {
          return dev;
        }
        cardNum--;
      }
    }
  }
  return NULL;         /* didn't find the card, return false */
} /* hwcFindDevice */

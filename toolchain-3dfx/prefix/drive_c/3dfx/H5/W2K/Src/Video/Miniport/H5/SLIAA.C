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
** File name:   sliaa.c
**
** Description: Routines to Initialize SLI AA for Napalm.
**
** $Revision: 35$
** $Date: 10/11/00 8:46:24 PM$
**
** $History: SLIAA.C $
**
** *****************  Version 1  *****************
** User: Jw           Date: 9/09/99    Time: 10:56a
** Created in $/devel/h5/W2K/Src/Video/Miniport/h5
**
** *****************  Version 7  *****************
** User: Andrew       Date: 7/30/99    Time: 2:00p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixed a bug with rendermask and SLI copies
**
** *****************  Version 6  *****************
** User: Andrew       Date: 7/28/99    Time: 3:46p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixes to support SLI on WINSIM
**
** *****************  Version 5  *****************
** User: Andrew       Date: 7/26/99    Time: 5:53p
** Updated in $/devel/h5/Win9x/dx/minivdd
** fixed some bugs and added code to setup the fb for entering SLI mode
** and leaving sli mode
**
** *****************  Version 4  *****************
** User: Andrew       Date: 7/20/99    Time: 10:57a
** Updated in $/devel/h5/Win9x/dx/minivdd
** code to init SLICTRL
**
** *****************  Version 3  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
**
** *****************  Version 2  *****************
** User: Andrew       Date: 7/07/99    Time: 2:43p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Dibengine workaround
**
** *****************  Version 1  *****************
** User: Andrew       Date: 6/25/99    Time: 10:05a
** Created in $/devel/h5/Win9x/dx/minivdd
** Code to enable/disable SLI/AA
**
**
*/

#ifdef SLI_AA

#include <miniport.h>
#include "ntddvdeo.h"
#include "video.h"
#include "dderror.h"
#include "h3.h"
#include "sliaa.h"

#pragma alloc_text(PAGE,GlideMapSlaveChips)
#pragma alloc_text(PAGE,GlideUnmapSlaveChips)
#pragma alloc_text(PAGE,H3_DISABLE_SLI_AA)
#pragma alloc_text(PAGE,H3_SETUP_SLI_AA)
#pragma alloc_text(PAGE,EnableSLIAA)
#pragma alloc_text(PAGE,DisableSLIAA)

//#undef MIN
//#define MIN(A,B) ((A) <= (B) ? (A) : (B))

#define MEMBASE0_MASTER_TO_SLAVE_SPACING    32*1024*1024                            // 32MB between master and slave membase0 phys addrs
#define MEMBASE1_MASTER_TO_SLAVE_SPACING    2*HwDeviceExtension->AdapterMemorySize  // MB between master and slave membase1 phys addrs
#define MEMBASE0_DECODE_SIZE_MASTER         32*1024*1024                            // 32MB (I'd like to use 16MB to include YUV Planar
                                                                                    //       registers but the hw doesn't seem to work
                                                                                    //       with only a 16MB decode)
#define MEMBASE0_DECODE_SIZE_SLAVE          32*1024*1024                            // 32MB (I'd like to use 8MB since slaves don't
                                                                                    //       need the YUV Planar registers but the hw
                                                                                    //       doesn't seem to work with an 8MB decode)
#define MEMBASE1_DECODE_SIZE                2*HwDeviceExtension->AdapterMemorySize  // ??MB

static const ULONG SparseMemBase0Map[] =
{
  SST_IO_OFFSET,
  SST_CMDAGP_OFFSET,
  SST_2D_OFFSET,
  SST_3D_OFFSET
};
static const ULONG SparseMemBase0Size[] =
{
  (sizeof(SstIORegs) + 4095) & ~4095,
  (sizeof(SstCRegs)  + 4095) & ~4095,
  (sizeof(SstGRegs)  + 4095) & ~4095,
  (sizeof(SstRegs)   + 4095) & ~4095
};

static __inline void
GetStrideAndAperature(PHW_DEVICE_EXTENSION HwDeviceExtension, DWORD *pStrideAndAperature)
{
  // GetStrideAndAperature basically reads the lfbMemoryConfig register
  // and keeps bits 13-22
  *pStrideAndAperature = PCI_IO_RD((DWORD)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + LFBMEMORYCONFIG) & 0x7FE000;
}

#if USE_INTERNAL_BUS_DATA_FUNCTIONS
#define PCI_ADDR_PORT		        0xCF8
#define PCI_DATA_PORT		        0xCFC

typedef struct tagCFGREG
{
  ULONG   res1   : 2;   // bits  0-1
  ULONG   regNum : 6;   // bits  2-7
  ULONG   func   : 3;   // bits  8-10
  ULONG   devNum : 5;   // bits 11-15
  ULONG   busNum : 8;   // bits 16-23
  ULONG   res2   : 7;   // bits 24-30
  ULONG   enable : 1;   // bit  31
} CFGREG;

ULONG
H3HalSetBusDataByOffset(BUS_DATA_TYPE BusDataType,
                        ULONG         BusNumber,
                        ULONG         SlotNumber,
                        PVOID         Buffer,
                        ULONG         Offset,
                        ULONG         Length)
{
  CFGREG        cfgReg;
  unsigned int  regNum;
  unsigned int  data;


  if (256 <= BusNumber)
    return 0;

  H3_ASSERT(PCIConfiguration == BusDataType, "H3HalSetBusDataByOffset called with wrong BusDataType\n");
  H3_ASSERT(0 == (Offset & 0x3), "H3HalSetBusDataByOffset called with non-dword aligned starting offset\n");
  H3_ASSERT(0 == (Length & 0x03), "H3HalSetBusDataByOffset called with non-dword multiple length\n");
#ifndef MS_VIEW
  H3_ASSERT((0 <= Offset) && (256 >= Offset), "H3HalSetBusDataByOffset called with invalid Offset\n");
  H3_ASSERT((0 <= (Offset + Length)) && (256 >= (Offset + Length)), "H3HalSetBusDataByOffset called with invalid range\n");
#else
  H3_ASSERT( (256 >= Offset), "H3HalSetBusDataByOffset called with invalid Offset\n");
  H3_ASSERT( (256 >= (Offset + Length)), "H3HalSetBusDataByOffset called with invalid range\n");
#endif //MS_VIEW

  cfgReg.enable = 1;
  cfgReg.res1   = 0;
  cfgReg.res2   = 0;

  cfgReg.busNum = BusNumber;
  cfgReg.devNum = ((PCI_SLOT_NUMBER *)&SlotNumber)->u.bits.DeviceNumber;
  cfgReg.func   = ((PCI_SLOT_NUMBER *)&SlotNumber)->u.bits.FunctionNumber;

  for (regNum = Offset/4; regNum < (Offset+Length)/4; regNum++)
  {
    cfgReg.regNum = regNum;

    data = *(ULONG *)((ULONG *)Buffer + (regNum - Offset / 4));

    _outpd(PCI_ADDR_PORT, *(unsigned int *)&cfgReg);
    _outpd(PCI_DATA_PORT, data);
  }

  return Length;
}

ULONG
H3HalGetBusDataByOffset(BUS_DATA_TYPE BusDataType,
                        ULONG         BusNumber,
                        ULONG         SlotNumber,
                        PVOID         Buffer,
                        ULONG         Offset,
                        ULONG         Length)
{
  CFGREG        cfgReg;
  unsigned int  regNum;
  unsigned int  data;


  if (256 <= BusNumber)
    return 0;

  H3_ASSERT(PCIConfiguration == BusDataType, "H3HalGetBusDataByOffset called with wrong BusDataType\n");
  H3_ASSERT(0 == (Offset & 0x3), "H3HalGetBusDataByOffset called with non-dword aligned starting offset\n");
  H3_ASSERT(0 == (Length & 0x03), "H3HalGetBusDataByOffset called with non-dword multiple length\n");
#ifndef MS_VIEW
  H3_ASSERT((0 <= Offset) && (256 >= Offset), "H3HalGetBusDataByOffset called with invalid Offset\n");
  H3_ASSERT((0 <= (Offset + Length)) && (256 >= (Offset + Length)), "H3HalGetBusDataByOffset called with invalid range\n");
#else
  H3_ASSERT((256 >= Offset), "H3HalGetBusDataByOffset called with invalid Offset\n");
  H3_ASSERT((256 >= (Offset + Length)), "H3HalGetBusDataByOffset called with invalid range\n");
#endif //MS_VIEW

  cfgReg.enable = 1;
  cfgReg.res1   = 0;
  cfgReg.res2   = 0;

  cfgReg.busNum = BusNumber;
  cfgReg.devNum = ((PCI_SLOT_NUMBER *)&SlotNumber)->u.bits.DeviceNumber;
  cfgReg.func   = ((PCI_SLOT_NUMBER *)&SlotNumber)->u.bits.FunctionNumber;

  for (regNum = Offset/4; regNum < (Offset+Length)/4; regNum++)
  {
    cfgReg.regNum = regNum;
    _outpd(PCI_ADDR_PORT, *(unsigned int *)&cfgReg);
    data = _inpd(PCI_DATA_PORT);

    *(ULONG *)((ULONG *)Buffer + (regNum - Offset / 4)) = data;
  }

  return Length;
}
#endif

/*----------------------------------------------------------------------
Function name:  FindPCIDevice

Description:

Information:

Return:
----------------------------------------------------------------------*/

BOOLEAN
FindPCIDevice(PHW_DEVICE_EXTENSION  HwDeviceExtension,
              DWORD                 dwVendorID,
              DWORD                 dwDeviceID,
              ULONG                 *pBusNumber,
              PCI_SLOT_NUMBER       *pSlotNumber)
{
  ULONG             functionNumber;
  PCI_SLOT_NUMBER   slotNumber;
  ULONG             length;
  ULONG             moreBuses;
  ULONG             busNumber, deviceNumber;
//  PCI_COMMON_CONFIG pciConfigInfo;
  ULONG             pciConfigInfo;  // we only need the vendor & device id's


  moreBuses = TRUE;
  for (busNumber = 0; moreBuses; busNumber++)
  {
    for (deviceNumber = 0; moreBuses && deviceNumber < PCI_MAX_DEVICES; deviceNumber++)
    {
      for (functionNumber = 0; functionNumber < PCI_MAX_FUNCTION; functionNumber++)
      {
        slotNumber.u.bits.Reserved       = 0;
        slotNumber.u.bits.DeviceNumber   = deviceNumber;
        slotNumber.u.bits.FunctionNumber = functionNumber;
        length = HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                            busNumber,
                                            slotNumber.u.AsULONG,
                                            &pciConfigInfo,
                                            0,
                                            sizeof(pciConfigInfo));
        if (0 == length)
        {
          VideoDebugPrint((2, "  no more buses\n"));
          moreBuses = FALSE;
          break;
        }

        if (PCI_INVALID_VENDORID == ((PCI_COMMON_CONFIG *)&pciConfigInfo)->VendorID)
          continue;

        VideoDebugPrint((1, "  pci bus %2ld, device %2ld, function %2ld contains vendorID %04lXh, deviceID %04lXh\n",
                         busNumber, slotNumber.u.bits.DeviceNumber,
                         functionNumber, ((PCI_COMMON_CONFIG *)&pciConfigInfo)->VendorID,
                         ((PCI_COMMON_CONFIG *)&pciConfigInfo)->DeviceID));

        if ((((PCI_COMMON_CONFIG *)&pciConfigInfo)->VendorID == dwVendorID) &&
            (((PCI_COMMON_CONFIG *)&pciConfigInfo)->DeviceID == dwDeviceID))
        {
          VideoDebugPrint((0, "  pci device %04lXh %04lXh found!\n", dwVendorID, dwDeviceID));
          if ((NULL != pBusNumber) && (NULL != pSlotNumber))
          {
            *pBusNumber = busNumber;
            *pSlotNumber = slotNumber;
          }
          return TRUE;
        }
      }
    }
  }
  VideoDebugPrint((0, "  pci device %04lXh %04lXh not found\n", dwVendorID, dwDeviceID));
  return FALSE;
}

/*----------------------------------------------------------------------
Function name:  FixAGPPerformanceOnViaChipsets

Description:    Via chipsets are programmed wrong for AGP performance

Information:

Return:
----------------------------------------------------------------------*/

#define VIA_VENDOR_ID                           0x1106
#define VIA_APOLLO_PRO_133A_HOST_TO_PCI_BRIDGE  0x0691
#define VIA_APOLLO_PRO_133A_PCI_TO_PCI_BRIDGE   0x8598
#define VIA_KX133_HOST_TO_PCI_BRIDGE            0x0391
#define VIA_KX133_PCI_TO_PCI_BRIDGE             0x8391
#define VIA_KT133_HOST_TO_PCI_BRIDGE            0x0305
#define VIA_KT133_PCI_TO_PCI_BRIDGE             0x8305

void
FixAGPPerformanceOnViaChipsets(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  typedef struct _VIA_CHIPSETS
  {
    USHORT  HostToPciBridge;
    USHORT  PciToPciBridge;
    BOOLEAN bCheckRevision;
    BOOLEAN bSetVIACoreLogicFlag;
  } VIA_CHIPSETS;

  static const VIA_CHIPSETS ViaChipsets[] =
  {
    { VIA_APOLLO_PRO_133A_HOST_TO_PCI_BRIDGE,
      VIA_APOLLO_PRO_133A_PCI_TO_PCI_BRIDGE,
      TRUE,
      TRUE
    },
    {
      VIA_KX133_HOST_TO_PCI_BRIDGE,
      VIA_KX133_PCI_TO_PCI_BRIDGE,
      FALSE,
      FALSE
    },
    {
      VIA_KT133_HOST_TO_PCI_BRIDGE,
      VIA_KT133_PCI_TO_PCI_BRIDGE,
      FALSE,
      FALSE
    }
  };

  const VIA_CHIPSETS  *pChipset;
  PCI_SLOT_NUMBER     slotNumber;
  ULONG               busNumber;
  ULONG               pciConfigData;


  // loop over chipsets
  for (pChipset = &ViaChipsets[0];
       pChipset < &ViaChipsets[sizeof(ViaChipsets)/sizeof(ViaChipsets[0])];
       pChipset++)
  {
    if (FindPCIDevice(HwDeviceExtension,
                      VIA_VENDOR_ID,
                      pChipset->HostToPciBridge,
                      &busNumber,
                      &slotNumber))
    {
      // we found a Via chipset that needs to be fixed up

      // see if we need to set the Via Core Logic flag
      if (pChipset->bSetVIACoreLogicFlag)
        HwDeviceExtension->bVIACoreLogic = TRUE;

      // see if we need to check the revision
      if (pChipset->bCheckRevision)
      {
        // get revision
        HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                   busNumber,
                                   slotNumber.u.AsULONG,
                                   &pciConfigData,
                                   0x8,
                                   sizeof(pciConfigData));

        pciConfigData &= 0xF0;
        // if it's not the right revision, skip it
        if (! ((0x80 == pciConfigData) || (0xC0 == pciConfigData)))
          continue;
      }

      // make sure bit 3 of offset 70h is set on the HostToPciBridge
      HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                 busNumber,
                                 slotNumber.u.AsULONG,
                                 &pciConfigData,
                                 0x70,
                                 sizeof(pciConfigData));
      if (! (pciConfigData & 0x8))
      {
        VideoDebugPrint((0, "    bit 3 of offset 70h not set, setting it now\n"));
        pciConfigData |= 0x8;
        HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                   busNumber,
                                   slotNumber.u.AsULONG,
                                   &pciConfigData,
                                   0x70,
                                   sizeof(pciConfigData));
      }

      // make sure offset 40h = C8h and offset 45h = 72h on the PciToPci Bridge
      if (FindPCIDevice(HwDeviceExtension,
                        VIA_VENDOR_ID,
                        pChipset->PciToPciBridge,
                        &busNumber,
                        &slotNumber))
      {
        // verify offset 40h = C8h
        HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                   busNumber,
                                   slotNumber.u.AsULONG,
                                   &pciConfigData,
                                   0x40,
                                   sizeof(pciConfigData));
        if (! (0xC8 == (0xFF & pciConfigData)))
        {
          VideoDebugPrint((0, "    offset 40h was %02Xh, setting to C8h now\n", pciConfigData & 0xFF));
          pciConfigData &= ~0xFF;
          pciConfigData |= 0xC8;
          HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                     busNumber,
                                     slotNumber.u.AsULONG,
                                     &pciConfigData,
                                     0x40,
                                     sizeof(pciConfigData));
        }
        // verify offset 45h = 72h
        HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                   busNumber,
                                   slotNumber.u.AsULONG,
                                   &pciConfigData,
                                   0x44,
                                   sizeof(pciConfigData));
        if (! (0x7200 == (0xFF00 & pciConfigData)))
        {
          VideoDebugPrint((0, "    offset 45h was %02Xh, setting to 72h now\n", (pciConfigData & 0xFF00) >> 8));
          pciConfigData &= ~0xFF00;
          pciConfigData |= 0x7200;
          HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                     busNumber,
                                     slotNumber.u.AsULONG,
                                     &pciConfigData,
                                     0x44,
                                     sizeof(pciConfigData));
        }
      } // if PciToPciBridge found
    } // if HostToPciBridge found
  } // for loop of chipsets
}


/*======================================================================
  v56k (2026-07): Voodoo5 6000 (4-chip) support, ported from the Win9x
  MiniVDD (GPIO.C / SLIAA.C).

  The 6000 generates its graphics clock with an external serial clock
  synthesizer wired to GPIO pins on the board's HiNT HB1-SE66 PCI-PCI
  bridge (PCI id 3388h:0021h), bit-banged through bridge config register
  C4h.  Without this the four chips never get a correct clock, so 4-way
  SLI cannot run.  The PLL search is integer math here because NT kernel
  code must not touch the FPU without saving state; the precision loss
  vs the Win9x double version is under the synthesizer's own step size.
======================================================================*/

#define V56K_HINT_BRIDGE_ID     0x00213388  // HiNT HB1-SE66: device 0021h, vendor 3388h
#define V56K_GPIO_REG           0xC4        // GPIO register in bridge config space
#define V56K_DTIME              5           // microseconds between GPIO transitions
#define V56K_CLOCK_STRETCH      1000

#ifndef PCI_MAX_DEVICES
#define PCI_MAX_DEVICES         32
#endif

typedef unsigned __int64 V56K_U64;

typedef struct _V56K_PCI_BIT {
  ULONG           BusNumber;
  PCI_SLOT_NUMBER SlotNumber;
  ULONG           dInMask;
  ULONG           dInShift;
  ULONG           dOutMask;
  ULONG           dOutShift;
} V56K_PCI_BIT;

typedef struct _V56K_GPIOMASK {
  V56K_PCI_BIT Data;
  V56K_PCI_BIT Clk;
  V56K_PCI_BIT Strobe;
  V56K_PCI_BIT HiVoltage;
} V56K_GPIOMASK;

static ULONG
V56KFindHintBridge(ULONG secondaryBus, PULONG pBridgeBus, PCI_SLOT_NUMBER *pBridgeSlot)
{
  ULONG           busNumber, deviceNumber;
  PCI_SLOT_NUMBER slotNumber;
  ULONG           id, busInfo;
  ULONG           length;

  // the bridge lives on a bus upstream of the chips' (secondary) bus
  for (busNumber = 0; busNumber < secondaryBus; busNumber++)
  {
    for (deviceNumber = 0; deviceNumber < PCI_MAX_DEVICES; deviceNumber++)
    {
      slotNumber.u.AsULONG = 0;
      slotNumber.u.bits.DeviceNumber   = deviceNumber;
      slotNumber.u.bits.FunctionNumber = 0;
      length = HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                          busNumber,
                                          slotNumber.u.AsULONG,
                                          &id,
                                          0,
                                          sizeof(id));
      if (0 == length)
        break;                      // no such bus
      if (PCI_INVALID_VENDORID == (id & 0xFFFF))
        continue;
      if (V56K_HINT_BRIDGE_ID != id)
        continue;
      // config offset 19h = secondary bus number
      length = HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                          busNumber,
                                          slotNumber.u.AsULONG,
                                          &busInfo,
                                          0x18,
                                          sizeof(busInfo));
      if ((sizeof(busInfo) == length) && (((busInfo >> 8) & 0xFF) == secondaryBus))
      {
        if (pBridgeBus)
          *pBridgeBus = busNumber;
        if (pBridgeSlot)
          *pBridgeSlot = slotNumber;
        return 1;
      }
    }
  }
  return 0;
}

static ULONG
V56KGetGpioReg(V56K_PCI_BIT *pBit)
{
  ULONG data = 0;

  HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                             pBit->BusNumber,
                             pBit->SlotNumber.u.AsULONG,
                             &data,
                             V56K_GPIO_REG,
                             sizeof(data));
  return data;
}

static void
V56KSetGpioReg(V56K_PCI_BIT *pBit, ULONG data)
{
  HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                             pBit->BusNumber,
                             pBit->SlotNumber.u.AsULONG,
                             &data,
                             V56K_GPIO_REG,
                             sizeof(data));
}

static ULONG
V56KGetGpioBit(V56K_PCI_BIT *pBit)
{
  return (V56KGetGpioReg(pBit) & pBit->dInMask) >> pBit->dInShift;
}

static void
V56KSetGpioBit(V56K_PCI_BIT *pBit, ULONG value)
{
  ULONG data;

  data = V56KGetGpioReg(pBit) & ~pBit->dOutMask;
  data |= (value & 0x01) << pBit->dOutShift;
  V56KSetGpioReg(pBit, data);
}

static void
V56KGpioScl(V56K_GPIOMASK *pMask, ULONG bit)
{
  ULONG nCount = 0;

  VideoPortStallExecution(V56K_DTIME);
  V56KSetGpioBit(&pMask->Clk, bit);
  VideoPortStallExecution(V56K_DTIME);

  if (bit)
  {
    while (!V56KGetGpioBit(&pMask->Clk) && (nCount++ < V56K_CLOCK_STRETCH))
      VideoPortStallExecution(V56K_DTIME);

    if (!V56KGetGpioBit(&pMask->Clk))
      VideoDebugPrint((0, "v56k: clock synth did not release SCL\n"));
  }
}

static void
V56KGpioSda(V56K_GPIOMASK *pMask, ULONG bit)
{
  VideoPortStallExecution(V56K_DTIME);
  V56KSetGpioBit(&pMask->Data, bit >> 7);
  VideoPortStallExecution(V56K_DTIME);
}

static void
V56KGpioSendByte(V56K_GPIOMASK *pMask, UCHAR b1, UCHAR b2, UCHAR b3)
{
  ULONG i;

  for (i = 0; i < 8; i++)
  {
    V56KGpioSda(pMask, (ULONG)(b1 << i) & 0x80);
    V56KGpioScl(pMask, 1);
    V56KGpioScl(pMask, 0);
  }
  for (i = 0; i < 8; i++)
  {
    V56KGpioSda(pMask, (ULONG)(b2 << i) & 0x80);
    V56KGpioScl(pMask, 1);
    V56KGpioScl(pMask, 0);
  }
  for (i = 0; i < 8; i++)
  {
    V56KGpioSda(pMask, (ULONG)(b3 << i) & 0x80);
    V56KGpioScl(pMask, 1);
    V56KGpioScl(pMask, 0);
  }

  // strobe latches the shifted word into the synthesizer
  VideoPortStallExecution(V56K_DTIME);
  V56KSetGpioBit(&pMask->Strobe, 1);
  VideoPortStallExecution(2 * V56K_DTIME);
  V56KSetGpioBit(&pMask->Strobe, 0);
  VideoPortStallExecution(V56K_DTIME);
}

static ULONG
V56KGpioInit(PHW_DEVICE_EXTENSION HwDeviceExtension, V56K_GPIOMASK *pMask)
{
  ULONG           bridgeBus;
  PCI_SLOT_NUMBER bridgeSlot;
  ULONG           data;

  if (!V56KFindHintBridge(HwDeviceExtension->BusNumber, &bridgeBus, &bridgeSlot))
  {
    VideoDebugPrint((0, "v56k: no HiNT bridge above bus %ld - cannot set external clock\n",
                     HwDeviceExtension->BusNumber));
    return 0;
  }

  pMask->Data.BusNumber    = bridgeBus;
  pMask->Data.SlotNumber   = bridgeSlot;
  pMask->Clk.BusNumber     = bridgeBus;
  pMask->Clk.SlotNumber    = bridgeSlot;
  pMask->Strobe.BusNumber  = bridgeBus;
  pMask->Strobe.SlotNumber = bridgeSlot;
  pMask->HiVoltage.BusNumber  = bridgeBus;
  pMask->HiVoltage.SlotNumber = bridgeSlot;

  // GPIO wiring per the Win9x driver (all in bridge config reg C4h)
  pMask->Data.dInMask       = 0x00000100;
  pMask->Data.dInShift      = 8;
  pMask->Data.dOutMask      = 0x00000400;
  pMask->Data.dOutShift     = 10;
  pMask->Clk.dInMask        = 0x00010000;
  pMask->Clk.dInShift       = 16;
  pMask->Clk.dOutMask       = 0x00040000;
  pMask->Clk.dOutShift      = 18;
  pMask->Strobe.dInMask     = 0x00001000;
  pMask->Strobe.dInShift    = 12;
  pMask->Strobe.dOutMask    = 0x00004000;
  pMask->Strobe.dOutShift   = 14;
  pMask->HiVoltage.dInMask  = 0x00100000;
  pMask->HiVoltage.dInShift = 20;
  pMask->HiVoltage.dOutMask = 0x00400000;
  pMask->HiVoltage.dOutShift = 22;

  // enable GPIO C4..C7 as outputs
  data = V56KGetGpioReg(&pMask->Data);
  data &= 0xFF0000FF;
  data |= 0x00222200;
  V56KSetGpioReg(&pMask->Data, data);
  return 1;
}

// serial synthesizer divider encoding, from the Win9x driver
static const ULONG V56KOdTable[] = {2, 3, 4, 5, 6, 7, 8, 10};
static const ULONG V56KS2S1S0[]  = {15, 15, 1, 6, 3, 4, 7, 5, 2, 15, 0};

static void
V56KOutputClock(PHW_DEVICE_EXTENSION HwDeviceExtension,
                ULONG c, ULONG ttl, ULONG f, ULONG s, ULONG v, ULONG r)
{
  V56K_GPIOMASK gpioMask;
  UCHAR         b1, b2, b3;

  b1  = (UCHAR)((c & 3) << 6);
  b1 |= (UCHAR)((ttl & 1) << 5);
  b1 |= (UCHAR)((f & 3) << 3);
  b1 |= (UCHAR)(s & 7);
  b2  = (UCHAR)((v & 0x1FE) >> 1);
  b3  = (UCHAR)((v & 1) << 7);
  b3 |= (UCHAR)(r & 0x7F);

  if (V56KGpioInit(HwDeviceExtension, &gpioMask))
    V56KGpioSendByte(&gpioMask, b1, b2, b3);
}

static ULONG
V56KSetExternalClock(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  SstIORegs *pMasterIO;
  ULONG     pixelclock, n, m, k;
  ULONG     ic;
  ULONG     i, rdw, vdw, od;
  ULONG     bFound, b_rdw, b_vdw, b_od;
  V56K_U64  bestDiff, diff, clk1, partial;

  pMasterIO = (SstIORegs *)HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_IOREGS_INDEX];

  // recover the target graphics clock from the master's PLL setting
  pixelclock = pMasterIO->pllCtrl0;
  n = ((pixelclock & 0xFF00) >> 8) + 2;
  m = ((pixelclock & 0x00FC) >> 2) + 2;
  k = pixelclock & 0x03;
  ic = ((14318180 * n) / m) >> k;
  ic >>= 2;

  VideoDebugPrint((0, "v56k: external clock target %ld Hz (pllCtrl0=%08lXh)\n",
                   ic, pixelclock));

  bFound   = 0;
  b_rdw    = 0;
  b_vdw    = 0;
  b_od     = 0;
  bestDiff = 500000000;   // same 500 MHz acceptance window as the Win9x driver
  for (i = 0; i < sizeof(V56KOdTable) / sizeof(V56KOdTable[0]); i++)
  {
    od = V56KOdTable[i];
    for (rdw = 1; rdw < 128; rdw++)
    {
      // constraint: 200 kHz < 14318180 / (rdw+2); only gets worse as rdw grows
      if (14318180 / (rdw + 2) <= 200000)
        break;
      for (vdw = 4; vdw < 512; vdw++)
      {
        // constraint: 55 MHz < 14318180 * 2 * (vdw+8) / (rdw+2) < 400 MHz
        partial = ((V56K_U64)28636360 * (vdw + 8)) / (rdw + 2);
        if ((partial <= 55000000) || (partial >= 400000000))
          continue;
        clk1 = partial / od;
        diff = (clk1 > ic) ? (clk1 - ic) : (ic - clk1);
        if (diff < bestDiff)
        {
          bestDiff = diff;
          bFound   = 1;
          b_rdw    = rdw;
          b_vdw    = vdw;
          b_od     = od;
        }
      }
    }
  }

  if (bFound)
  {
    VideoDebugPrint((0, "v56k: synth solution od=%ld rdw=%ld vdw=%ld (err %ld Hz)\n",
                     b_od, b_rdw, b_vdw, (ULONG)bestDiff));
    V56KOutputClock(HwDeviceExtension, 0x0, 0x1, 0x0, V56KS2S1S0[b_od], b_vdw, b_rdw);
  }
  else
  {
    // the Win9x driver breakpoints here; we leave the clock alone instead
    VideoDebugPrint((0, "v56k: NO synth solution for %ld Hz - external clock unchanged\n",
                     ic));
  }

  return ic;
}

/*----------------------------------------------------------------------
Function name:  DetectNumUnits

Description:    determine how many slave chips are on this bus/device
                combination

Information:

Return:
----------------------------------------------------------------------*/

void
DetectNumUnits(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  ULONG             functionNumber;
  PCI_SLOT_NUMBER   slotNumber;
  ULONG             length;


#if 1
  ULONG             pciConfigInfo;  // we only need the vendor & device id's


  // the easy way
  HwDeviceExtension->numUnits = 0;

  slotNumber.u.bits.Reserved     = 0;
  slotNumber.u.bits.DeviceNumber = HwDeviceExtension->PCISlot.u.bits.DeviceNumber;

  // walk the function space on this bus/device combo
  for (functionNumber = 0; functionNumber < PCI_MAX_FUNCTION; functionNumber++)
  {
    slotNumber.u.bits.FunctionNumber = functionNumber;
    length = HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                        HwDeviceExtension->BusNumber,
                                        slotNumber.u.AsULONG,
                                        &pciConfigInfo,
                                        0,
                                        sizeof(pciConfigInfo));
    if (0 == length)
    {
      VideoDebugPrint((0, "  invalid bus\n"));
      break;
    }

    if (PCI_INVALID_VENDORID == ((PCI_COMMON_CONFIG *)&pciConfigInfo)->VendorID)
      continue;

    VideoDebugPrint((0, "  pci bus %2ld, device %2ld, function %2ld contains vendorID %04lXh, deviceID %04lXh\n",
                     HwDeviceExtension->BusNumber, slotNumber.u.bits.DeviceNumber,
                     functionNumber, ((PCI_COMMON_CONFIG *)&pciConfigInfo)->VendorID,
                     ((PCI_COMMON_CONFIG *)&pciConfigInfo)->DeviceID));

    if ((((PCI_COMMON_CONFIG *)&pciConfigInfo)->VendorID == HwDeviceExtension->PCIVendorID) &&
        (((PCI_COMMON_CONFIG *)&pciConfigInfo)->DeviceID == HwDeviceExtension->PCIDeviceID))
    {
      HwDeviceExtension->numUnits++;
      HwDeviceExtension->sliSlotNumber[functionNumber] = slotNumber;
    }
  }

  // v56k: defensive fallback.  On a stock 6000 the slave chips answer as
  // functions 1-3 of the master's device (found above), but if a rebuilt
  // board straps them as separate devices behind the HiNT bridge, sweep
  // the rest of this (secondary) bus too.  Gated on the HiNT bridge so a
  // second Voodoo board in the system is never mistaken for slave chips,
  // and on IS_NAPALM so a Voodoo3 (single chip, no SLI) can never grow
  // phantom slaves even on a bridged backplane.
  if (IS_NAPALM &&
      (HwDeviceExtension->numUnits < 4) &&
      V56KFindHintBridge(HwDeviceExtension->BusNumber, NULL, NULL))
  {
    ULONG deviceNumber;

    for (deviceNumber = 0; deviceNumber < PCI_MAX_DEVICES; deviceNumber++)
    {
      if (deviceNumber == HwDeviceExtension->PCISlot.u.bits.DeviceNumber)
        continue;                   // covered by the function walk above

      for (functionNumber = 0; functionNumber < PCI_MAX_FUNCTION; functionNumber++)
      {
        if (PCI_MAX_FUNCTION <= HwDeviceExtension->numUnits)
          break;
        slotNumber.u.bits.DeviceNumber   = deviceNumber;
        slotNumber.u.bits.FunctionNumber = functionNumber;
        length = HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                            HwDeviceExtension->BusNumber,
                                            slotNumber.u.AsULONG,
                                            &pciConfigInfo,
                                            0,
                                            sizeof(pciConfigInfo));
        if (0 == length)
          break;
        if (PCI_INVALID_VENDORID == ((PCI_COMMON_CONFIG *)&pciConfigInfo)->VendorID)
          continue;
        if ((((PCI_COMMON_CONFIG *)&pciConfigInfo)->VendorID == HwDeviceExtension->PCIVendorID) &&
            (((PCI_COMMON_CONFIG *)&pciConfigInfo)->DeviceID == HwDeviceExtension->PCIDeviceID))
        {
          VideoDebugPrint((0, "  v56k: extra chip at bus %ld dev %ld fn %ld\n",
                           HwDeviceExtension->BusNumber, deviceNumber, functionNumber));
          HwDeviceExtension->sliSlotNumber[HwDeviceExtension->numUnits] = slotNumber;
          HwDeviceExtension->numUnits++;
        }
      }
    }
  }
  VideoDebugPrint((0, "  num chip(s) = %ld\n", HwDeviceExtension->numUnits));
#else
  // the hard way
  ULONG             moreBuses;
  ULONG             busNumber, deviceNumber;
  PCI_COMMON_CONFIG pciConfigInfo;


  HwDeviceExtension->numUnits = 0;
  moreBuses = TRUE;
  for (busNumber = 0; moreBuses; busNumber++)
  {
    for (deviceNumber = 0; moreBuses && deviceNumber < PCI_MAX_DEVICES; deviceNumber++)
    {
      for (functionNumber = 0; functionNumber < PCI_MAX_FUNCTION; functionNumber++)
      {
        slotNumber.u.bits.Reserved       = 0;
        slotNumber.u.bits.DeviceNumber   = deviceNumber;
        slotNumber.u.bits.FunctionNumber = functionNumber;
        length = HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                            busNumber,
                                            slotNumber.u.AsULONG,
                                            &pciConfigInfo,
                                            0,
                                            sizeof(pciConfigInfo));
        if (0 == length)
        {
          VideoDebugPrint((2, "  no more buses\n"));
          moreBuses = FALSE;
          break;
        }

        if (PCI_INVALID_VENDORID == pciConfigInfo.VendorID)
          continue;

        VideoDebugPrint((0, "  pci bus %2ld, device %2ld, function %2ld contains vendorID %04lXh, deviceID %04lXh, BaseClass=%02lXh, SubClass=%02lXh\n",
                         busNumber, deviceNumber, functionNumber, pciConfigInfo.VendorID, pciConfigInfo.DeviceID, pciConfigInfo.BaseClass, pciConfigInfo.SubClass));

        if ((slotNumber.u.bits.DeviceNumber == HwDeviceExtension->PCISlot.u.AsULONG) &&
            (pciConfigInfo.VendorID == HwDeviceExtension->PCIVendorID) &&
            (pciConfigInfo.DeviceID == HwDeviceExtension->PCIDeviceID))
        {
          HwDeviceExtension->numUnits++;
          HwDeviceExtension->sliSlotNumber[functionNumber] = slotNumber;
        }
      }
    }
  }
  VideoDebugPrint((0, "  num chip(s) = %ld\n", HwDeviceExtension->numUnits));
#endif

  FixAGPPerformanceOnViaChipsets(HwDeviceExtension);
}

/*----------------------------------------------------------------------
Function name: GetDecodeSize

Description:    Return Decode Size

Information:

Return: MemBase0/MemBase1 Decode Size
----------------------------------------------------------------------*/
static ULONG
GetDecodeSize(ULONG dwSize)
{
  DWORD dwReturn;

  switch (dwSize >> 20)
  {
    case 4:
      dwReturn = CFG_MEMBASE0_4MB_DECODE;
      break;

    case 8:
      dwReturn = CFG_MEMBASE0_8MB_DECODE;
      break;

    case 16:
      dwReturn = CFG_MEMBASE0_16MB_DECODE;
      break;

    case 32:
      dwReturn = CFG_MEMBASE0_32MB_DECODE;
      break;

    case 64:
      dwReturn = CFG_MEMBASE0_64MB_DECODE;
      break;

    case 128:
      dwReturn = CFG_MEMBASE0_128MB_DECODE;
      break;

    case 256:
      dwReturn = CFG_MEMBASE0_256MB_DECODE;
      break;

    case 512:
      dwReturn = CFG_MEMBASE0_512MB_DECODE;
      break;

    case 1024:
      dwReturn = CFG_MEMBASE0_1024MB_DECODE;
      break;

    default:
      dwReturn = CFG_MEMBASE0_128MB_DECODE;
      break;
  }

  return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  InitializeSlaveChipsPCIConfigSpace

Description:

Information:

Return:
----------------------------------------------------------------------*/

void
InitializeSlaveChipsPCIConfigSpace(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  PCI_COMMON_CONFIG   PCIBuffer;
  ULONG               functionNumber;
  ULONG               bar[NUM_H3_ACCESS_RANGES];
  ULONG               command, pciDecode, cfgInitEnable;
  ULONG               i;


  // copy master AccessRanges into sliAccessRanges
  memcpy(&HwDeviceExtension->sliAccessRanges[0][0],
         &HwDeviceExtension->AccessRanges[0],
         NUM_H3_ACCESS_RANGES * sizeof(VIDEO_ACCESS_RANGE));

  if (IS_NAPALM)
  {
    // first reset the pciDecode for the master
    // set io decode to 256, membase0 decode to MEMBASE0_DECODE_SIZE_MASTER &
    // membase1 decode to MEMBASE1_DECODE_SIZE
    // read pciDecode register
    if (sizeof(pciDecode) != HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                                        HwDeviceExtension->BusNumber,
                                                        HwDeviceExtension->sliSlotNumber[0].u.AsULONG,
                                                        &pciDecode,
                                                        CFG_PCI_DECODE,
                                                        sizeof(pciDecode)))
    {
      // read failed, disable slave chips
      VideoDebugPrint((0, "  read of pciDecode reg for master (chip0) failed!!!\n"));
      HwDeviceExtension->numUnits = 1;
      return;
    }
    pciDecode &= ~(CFG_MEMBASE0_DECODE | CFG_MEMBASE1_DECODE | CFG_IOBASE_DECODE);
    pciDecode |= ((GetDecodeSize(MEMBASE0_DECODE_SIZE_MASTER) << CFG_MEMBASE0_DECODE_SHIFT) |
                  (GetDecodeSize(MEMBASE1_DECODE_SIZE)        << CFG_MEMBASE1_DECODE_SHIFT) |
                  (0 << CFG_IOBASE_DECODE_SHIFT));
    // write the pcidecode register
    if (0 == HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                        HwDeviceExtension->BusNumber,
                                        HwDeviceExtension->sliSlotNumber[0].u.AsULONG,
                                        &pciDecode,
                                        CFG_PCI_DECODE,
                                        sizeof(pciDecode)))
    {
      // update failed, disable slave chips
      VideoDebugPrint((0, "  update to pciDecode reg of slave (chip%ld) failed!!!\n"));
      HwDeviceExtension->numUnits = 1;
      return;
    }
    VideoDebugPrint((0, "  updated master (chip0) pciDecode reg = %08lXh\n",
                     PCI_CFG_RD(CFG_PCI_DECODE, 0)));
  }

  // if there are any slaves
  // initialize their bars in pci config space & enable the slave chips
  if (1 < HwDeviceExtension->numUnits)
  {
    // Get PCI Config Space for master
    VideoPortGetBusData(HwDeviceExtension,
                        PCIConfiguration,
                        HwDeviceExtension->PCISlot.u.AsULONG,
                        &PCIBuffer,
                        0,
                        PCI_COMMON_HDR_LENGTH);

    VideoDebugPrint((0, "  master (chip0) bars are membase0=%08lXh, membase1=%08lXh, io=%08lXh\n",
                     PCIBuffer.u.type0.BaseAddresses[0],
                     PCIBuffer.u.type0.BaseAddresses[1],
                     PCIBuffer.u.type0.BaseAddresses[2]));

    // start at device 1 so we skip the master device
    for (functionNumber = 1; functionNumber < HwDeviceExtension->numUnits; functionNumber++)
    {
      // read cfgInitEnable
      if (sizeof(cfgInitEnable) != HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                                              HwDeviceExtension->BusNumber,
                                                              HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                                              &cfgInitEnable,
                                                              CFG_INIT_ENABLE,
                                                              sizeof(cfgInitEnable)))
      {
        VideoDebugPrint((0, "  read of cfgInitEnable reg for slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        return;
      }

      // set bit 10
      cfgInitEnable |= CFG_UPDATE_MEMBASE_LSBS;
      // write the cfgInitEnable register
      if (0 == HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                          HwDeviceExtension->BusNumber,
                                          HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                          &cfgInitEnable,
                                          CFG_INIT_ENABLE,
                                          sizeof(cfgInitEnable)))
      {
        // update failed, disable slave chips
        VideoDebugPrint((0, "  update to cfgInitEnable reg of slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        return;
      }
      VideoDebugPrint((0, "  updated slave (chip%ld) cfgInitEnable reg = %08lXh\n",
                       functionNumber, PCI_CFG_RD(CFG_INIT_ENABLE, functionNumber)));

      // copy master AccessRange to this slave
      // we'll fix up the RangeStart.LowPart below
      memcpy(&HwDeviceExtension->sliAccessRanges[functionNumber][0],
             &HwDeviceExtension->AccessRanges[0],
             NUM_H3_ACCESS_RANGES * sizeof(VIDEO_ACCESS_RANGE));

      // compute phys addrs for this slave
      for (i = 0; i < NUM_H3_ACCESS_RANGES; i++)
      {
        if (PCI_ADDRESS_IO_SPACE & PCIBuffer.u.type0.BaseAddresses[i])
        {
          // grab the io BaseAddress
          bar[i] = PCIBuffer.u.type0.BaseAddresses[i] & PCI_ADDRESS_IO_ADDRESS_MASK;
          // park slaves at same io address as master
          bar[i] += 0;                                        //IO_DECODE_SIZE * functionNumber;
          // fix up RangeStart.LowPart for this slave
          HwDeviceExtension->sliAccessRanges[functionNumber][i].RangeStart.LowPart = bar[i];
          // set the io space indicator
          bar[i] |= PCI_ADDRESS_IO_SPACE;
        }
        else
        {
          // grab the memory BaseAddress
          bar[i] = PCIBuffer.u.type0.BaseAddresses[i] & PCI_ADDRESS_MEMORY_ADDRESS_MASK;
          // bump to this functionNumber
          if (0 == i) // membase0
            bar[i] += MEMBASE0_MASTER_TO_SLAVE_SPACING * functionNumber;
          else        // membase1, park all slaves at same physical address
            bar[i] += MEMBASE1_MASTER_TO_SLAVE_SPACING; // * functionNumber;
          // fix up RangeStart.LowPart for this slave
          HwDeviceExtension->sliAccessRanges[functionNumber][i].RangeStart.LowPart = bar[i];
          // copy memory type and prefetchable bits from master
          bar[i] |= (PCIBuffer.u.type0.BaseAddresses[i] & (PCI_ADDRESS_MEMORY_TYPE_MASK | PCI_ADDRESS_MEMORY_PREFETCHABLE));
        }
      }

      // hit the bars
      if (0 == HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                          HwDeviceExtension->BusNumber,
                                          HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                          &bar[0],
                                          FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.BaseAddresses[0]),
                                          sizeof(bar)))
      {
        // update failed, disable slave chips
        VideoDebugPrint((0, "  update to bars of slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        break;
      }
      VideoDebugPrint((0, "  updated slave (chip%ld) bars to membase0=%08lXh, membase1=%08lXh, io=%08lXh\n",
                       functionNumber, PCI_CFG_RD(MEMBASEADDR0, functionNumber),
                       PCI_CFG_RD(MEMBASEADDR1, functionNumber), PCI_CFG_RD(IOBASEADDR, functionNumber)));
#if DBG
      {
        ULONG temp[NUM_H3_ACCESS_RANGES];

        if (0 == HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                            HwDeviceExtension->BusNumber,
                                            HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                            &temp[0],
                                            FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.BaseAddresses[0]),
                                            sizeof(temp)))
        {
          // unable to read back bars
          VideoDebugPrint((0, "  read of bars of slave (chip%ld) failed!!!\n", functionNumber));
        }
        else
        {
          int i;

          for (i = 0; i < NUM_H3_ACCESS_RANGES; i++)
          {
            if (bar[i] != temp[i])
            {
              VideoDebugPrint((0, "  Bar%ld doesn't match programmed value!  bar[%ld]=%08lXh, temp[%ld]=0x%08lXh\n",
                               i, i, bar[i], i, temp[i]));
            }
          }
        }
      }
#endif

      // set io decode to 256, membase0 decode to MEMBASE0_DECODE_SIZE_SLAVE &
      // membase1 decode to MEMBASE1_DECODE_SIZE
      // read pciDecode register
      if (sizeof(pciDecode) != HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                                          HwDeviceExtension->BusNumber,
                                                          HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                                          &pciDecode,
                                                          CFG_PCI_DECODE,
                                                          sizeof(pciDecode)))
      {
        // read failed, disable slave chips
        VideoDebugPrint((0, "  read of pciDecode reg for slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        break;
      }
      pciDecode &= ~(CFG_MEMBASE0_DECODE | CFG_MEMBASE1_DECODE | CFG_IOBASE_DECODE |
                     CFG_SNOOP_MEMBASE0_DECODE | CFG_SNOOP_MEMBASE1_DECODE);
      pciDecode |= ((GetDecodeSize(MEMBASE0_DECODE_SIZE_SLAVE) << CFG_MEMBASE0_DECODE_SHIFT) |
                    (GetDecodeSize(MEMBASE1_DECODE_SIZE)       << CFG_MEMBASE1_DECODE_SHIFT) |
                    (0 << CFG_IOBASE_DECODE_SHIFT) |
                    (GetDecodeSize(MEMBASE0_DECODE_SIZE_MASTER) << CFG_SNOOP_MEMBASE0_DECODE_SHIFT) |
                    (GetDecodeSize(MEMBASE1_DECODE_SIZE)        << CFG_SNOOP_MEMBASE1_DECODE_SHIFT));
      // write the pcidecode register
      if (0 == HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                          HwDeviceExtension->BusNumber,
                                          HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                          &pciDecode,
                                          CFG_PCI_DECODE,
                                          sizeof(pciDecode)))
      {
        // update failed, disable slave chips
        VideoDebugPrint((0, "  update to pciDecode reg of slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        break;
      }
      VideoDebugPrint((0, "  updated slave (chip%ld) pciDecode reg = %08lXh\n",
                       functionNumber, PCI_CFG_RD(CFG_PCI_DECODE, functionNumber)));

      // enable this chip
      // read the pci command register
      if (sizeof(command) != HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                                        HwDeviceExtension->BusNumber,
                                                        HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                                        &command,
                                                        FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                                                        sizeof(command)))
      {
        // read failed, disable slave chips
        VideoDebugPrint((0, "  read of pci command reg for slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        break;
      }
      // only enable memory on slaves
      command |= PCI_ENABLE_MEMORY_SPACE;
      // write the pci command register
      if (0 == HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                          HwDeviceExtension->BusNumber,
                                          HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                          &command,
                                          FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                                          sizeof(command)))
      {
        // update failed, disable slave chips
        VideoDebugPrint((0, "  update to pci command reg of slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        break;
      }
      VideoDebugPrint((0, "  updated slave (chip%ld) command reg = %08lXh\n",
                       functionNumber, PCI_CFG_RD(PCICOMMAND, functionNumber)));

      // read cfgInitEnable
      if (sizeof(cfgInitEnable) != HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                                              HwDeviceExtension->BusNumber,
                                                              HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                                              &cfgInitEnable,
                                                              CFG_INIT_ENABLE,
                                                              sizeof(cfgInitEnable)))
      {
        VideoDebugPrint((0, "  read of cfgInitEnable reg for slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        return;
      }

      // clear bit 10
      cfgInitEnable &= ~CFG_UPDATE_MEMBASE_LSBS;
      // write the cfgInitEnable register
      if (0 == HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                          HwDeviceExtension->BusNumber,
                                          HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                          &cfgInitEnable,
                                          CFG_INIT_ENABLE,
                                          sizeof(cfgInitEnable)))
      {
        // update failed, disable slave chips
        VideoDebugPrint((0, "  update to cfgInitEnable reg of slave (chip%ld) failed!!!\n", functionNumber));
        HwDeviceExtension->numUnits = 1;
        return;
      }
      VideoDebugPrint((0, "  updated slave (chip%ld) cfgInitEnable reg = %08lXh\n",
                       functionNumber, PCI_CFG_RD(CFG_INIT_ENABLE, functionNumber)));
    }
  }
}

/*----------------------------------------------------------------------
Function name:  MapSlaveChipsAccessRanges

Description:

Information:

Return:
----------------------------------------------------------------------*/

void
MapSlaveChipsAccessRanges(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  ULONG               i, j;
  VIDEO_ACCESS_RANGE  AccessRange;


  // master addresses were mapped elsewhere
#if REDUCED_MEMORY_MAPPINGS
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_IOREGS_INDEX     ] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_CMDFIFOREGS_INDEX] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_CMDAGP_REGS_INDEX];
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_2DREGS_INDEX     ] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_2D_REGS_INDEX];
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_3DREGS_INDEX     ] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_3D_REGS_INDEX];
#else
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_IOREGS_INDEX     ] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + SST_IO_OFFET;
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_CMDFIFOREGS_INDEX] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + SST_CMDAGP_OFFSET;
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_2DREGS_INDEX     ] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + SST_2D_OFFSET;
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_3DREGS_INDEX     ] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + SST_3D_OFFSET;
#endif
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_FB_INDEX         ] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_FB_INDEX];
  HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_IO_INDEX         ] = (UCHAR *)HwDeviceExtension->MappedAddress[SST_IO_INDEX];

  // if nothing failed in the above
  // map the slave access ranges
  if (1 < HwDeviceExtension->numUnits)
  {
    // now map the slaves
    for (i = 1; i < HwDeviceExtension->numUnits; i++)
    {
      // make local copy of physical address of regs of master
      memcpy(&AccessRange, &HwDeviceExtension->AccessRanges[0], sizeof(AccessRange));

      // point to physical address of registers for this slave
      AccessRange.RangeStart.LowPart += MEMBASE0_MASTER_TO_SLAVE_SPACING * i;

      AccessRange.RangeLength = MEMBASE0_DECODE_SIZE_SLAVE;

      // sparse map the registers of the slave
      VideoDebugPrint((0, "Slave %ld MemBase0 AccessRange\n", i));
      VideoDebugPrint((0, "  RangeStart=%lX%08lXh  Length=%08lXh  InIoSpace=%lXh\n",
                       AccessRange.RangeStart.HighPart,
                       AccessRange.RangeStart.LowPart,
                       AccessRange.RangeLength,
                       AccessRange.RangeInIoSpace));
      VideoDebugPrint((0, "Slave %ld MemBase0 Mapped Ranges\n", i));

      for (j = 0; j < sizeof(SparseMemBase0Map)/sizeof(SparseMemBase0Map[0]); j++)
      {
        PHYSICAL_ADDRESS  physAddr;

        memcpy(&physAddr, &AccessRange.RangeStart, sizeof(PHYSICAL_ADDRESS));
        physAddr.LowPart += SparseMemBase0Map[j];
        HwDeviceExtension->sliMappedAddress[i][j] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                           physAddr,
                                                                           SparseMemBase0Size[j],
                                                                           AccessRange.RangeInIoSpace);
        if (NULL == HwDeviceExtension->sliMappedAddress[i][j])
        {
          VideoDebugPrint((0, "InitializeSlaveChips: DeviceBase mapping failed on membase0 for slave %ld, sparse range &ld\n", i, j));
          HwDeviceExtension->numUnits = 1;
        }

        VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                         physAddr.HighPart, physAddr.LowPart,
                         HwDeviceExtension->sliMappedAddress[i][j],
                         SparseMemBase0Size[j]));
      }

      // do we need to map the slave frame buffers?
#if 1
      // just stuff the master's frame buffer address into the slave frame buffer address
      HwDeviceExtension->sliMappedAddress[i][j] = HwDeviceExtension->MappedAddress[SST_FB_INDEX];

      VideoDebugPrint((0, "Slave %ld MemBase1 Mapped Range (copied from chip0)\n", i));
#if (_WIN32_WINNT >= 0x0500)
      VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                       HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.HighPart,
                       HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.LowPart,
                       HwDeviceExtension->sliMappedAddress[i][j],
                       HwDeviceExtension->AdapterMemorySize * 2));
#else
      VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                       HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.HighPart,
                       HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.LowPart,
                       HwDeviceExtension->sliMappedAddress[i][j],
                       HwDeviceExtension->AdapterMemorySize));
#endif
#else
      // make local copy of physical address of frame buffer of master
      memcpy(&AccessRange, &HwDeviceExtension->AccessRanges[1], sizeof(AccessRange));

      // point to physical address of frame buffer for this slave
      AccessRange.RangeStart.LowPart += MEMBASE1_MASTER_TO_SLAVE_SPACING * i;

      // map the frame buffer of the slave
#if (_WIN32_WINNT >= 0x0500)
      HwDeviceExtension->sliMappedAddress[i][j] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                         AccessRange.RangeStart,
                                                                         HwDeviceExtension->AdapterMemorySize * 2,
                                                                         AccessRange.RangeInIoSpace);
#else
      HwDeviceExtension->sliMappedAddress[i][j] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                         AccessRange.RangeStart,
                                                                         HwDeviceExtension->AdapterMemorySize,
                                                                         AccessRange.RangeInIoSpace);
#endif
      if (NULL == HwDeviceExtension->sliMappedAddress[i][j])
      {
        VideoDebugPrint((0, "InitializeSlaveChips: DeviceBase mapping failed on membase1 for slave %ld\n", i));
        HwDeviceExtension->numUnits = 1;
      }
#endif
      j++;


#if 1
      // just stuff the master's i/o address into the slave i/o address
      HwDeviceExtension->sliMappedAddress[i][j] = HwDeviceExtension->MappedAddress[SST_IO_INDEX];

      VideoDebugPrint((0, "Slave %ld IOBase AccessRange (copied from chip0)\n", i));
      VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                       HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart.HighPart,
                       HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart.LowPart,
                       HwDeviceExtension->sliMappedAddress[i][j],
                       256));
#else
      // make local copy of physical address of io space of master
      memcpy(&AccessRange, &HwDeviceExtension->AccessRanges[2], sizeof(AccessRange));

      // point to physical address of io spave for this slave
      AccessRange.RangeStart.LowPart += IO_DECODE_SIZE * i;

      AccessRange.RangeLength = 256;

      // map the io space of the slave
      VideoDebugPrint((0, "Slave %ld IOBase AccessRange\n", i));
      VideoDebugPrint((0, "  RangeStart=%lX%08lXh  Length=%08lXh  InIoSpace=%lXh\n",
                       AccessRange.RangeStart.HighPart,
                       AccessRange.RangeStart.LowPart,
                       AccessRange.RangeLength,
                       AccessRange.RangeInIoSpace));

      HwDeviceExtension->sliMappedAddress[i][j] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                         AccessRange.RangeStart,
                                                                         AccessRange.RangeLength,
                                                                         AccessRange.RangeInIoSpace);
      if (NULL == HwDeviceExtension->sliMappedAddress[i][j])
      {
        VideoDebugPrint((0, "InitializeSlaveChips: DeviceBase mapping failed on io for slave %ld\n", i));
        HwDeviceExtension->numUnits = 1;
      }

      VideoDebugPrint((0, "Slave %ld IOBase Mapped Range\n", i));
      VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                       AccessRange.RangeStart.HighPart,
                       AccessRange.RangeStart.LowPart,
                       HwDeviceExtension->sliMappedAddress[i][j],
                       AccessRange.RangeLength));
#endif
    }
  }
}

/*----------------------------------------------------------------------
Function name:  UpdatePCICommandReg

Description:

Information:

Return:
----------------------------------------------------------------------*/

void
UpdatePCICommandReg(PHW_DEVICE_EXTENSION  HwDeviceExtension,
                    ULONG                 functionNumber,
                    ULONG                 andMask,
                    ULONG                 orMask)
{
  ULONG   command;


  // read the pci command register
  if (sizeof(command) != HAL_GET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                                    HwDeviceExtension->BusNumber,
                                                    HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                                    &command,
                                                    FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                                                    sizeof(command)))
  {
    // read failed
    VideoDebugPrint((0, "  read of pci command reg for chip%ld failed!!!\n", functionNumber));
    return;
  }

  // apply masks
  command &= andMask;
  command |= orMask;

  // write the pci command register
  if (0 == HAL_SET_BUS_DATA_BY_OFFSET(PCIConfiguration,
                                      HwDeviceExtension->BusNumber,
                                      HwDeviceExtension->sliSlotNumber[functionNumber].u.AsULONG,
                                      &command,
                                      FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                                      sizeof(command)))
  {
    // update failed
    VideoDebugPrint((0, "  update to pci command reg of chip%ld failed!!!\n", functionNumber));
    return;
  }
  VideoDebugPrint((0, "  updated chip%ld command reg = %08lXh\n",
                   functionNumber, PCI_CFG_RD(PCICOMMAND, functionNumber)));

}

/*----------------------------------------------------------------------
Function name:  InitializeSlaveChipsInitRegs

Description:

Information:

Return:
----------------------------------------------------------------------*/

void
InitializeSlaveChipsInitRegs(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  ULONG i;
  SstIORegs *pMasterIO, *pSlaveIO;


  // initialize the slave chips init regs
  if (1 < HwDeviceExtension->numUnits)
  {
    pMasterIO = HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_IOREGS_INDEX];

    // disable i/o on master
    UpdatePCICommandReg(HwDeviceExtension, 0, ~PCI_ENABLE_IO_SPACE, 0);

    for (i = 1; i < HwDeviceExtension->numUnits; i++)
    {
      // enable i/o on slave
      UpdatePCICommandReg(HwDeviceExtension, i, (LONG)-1, PCI_ENABLE_IO_SPACE);

      // initialize slave device
      H3InitializeSecondaryDevice(HwDeviceExtension,
                                  HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IOREGS_INDEX],
                                  (PUCHAR)HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IO_INDEX]);


      // disable i/o on slave
      UpdatePCICommandReg(HwDeviceExtension, i, ~PCI_ENABLE_IO_SPACE, 0);

      // copy tiled mode configuration regs from master to slave
      pSlaveIO = HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IOREGS_INDEX];

      pSlaveIO->tmuGbeInit = pMasterIO->tmuGbeInit;
    }

    // enable i/o on master
    UpdatePCICommandReg(HwDeviceExtension, 0, (LONG)-1, PCI_ENABLE_IO_SPACE);
  }
}

/*----------------------------------------------------------------------
Function name:  GlideMapSlaveChips

Description:

Information:

Return:
----------------------------------------------------------------------*/

VP_STATUS
GlideMapSlaveChips(PHW_DEVICE_EXTENSION         HwDeviceExtension,
                   HANDLE                       processHandle,
                   PVIDEO_PUBLIC_ACCESS_RANGES  portAccess)
{
  VP_STATUS                   status = NO_ERROR;
  ULONG                       i, j;
  PVIDEO_PUBLIC_ACCESS_RANGES pCurPortAccess;
  PHYSICAL_ADDRESS            physAddr;
  ULONG                       physLength;


  // sparse mappings for master chip
  pCurPortAccess = &portAccess[3];
  memset(pCurPortAccess, 0, HWINFO_SST_MAX_NUM_CHIPS * HWINFO_SST_MAX_CHIP_INDEX * sizeof(VIDEO_PUBLIC_ACCESS_RANGES));

  pCurPortAccess[HWINFO_SST_IOREGS_INDEX].InIoSpace       = portAccess[0].InIoSpace;
  pCurPortAccess[HWINFO_SST_IOREGS_INDEX].MappedInIoSpace = portAccess[0].MappedInIoSpace;
  pCurPortAccess[HWINFO_SST_IOREGS_INDEX].VirtualAddress  = (PUCHAR)portAccess[0].VirtualAddress + SST_IO_OFFSET;

  pCurPortAccess[HWINFO_SST_CMDFIFOREGS_INDEX].InIoSpace       = portAccess[0].InIoSpace;
  pCurPortAccess[HWINFO_SST_CMDFIFOREGS_INDEX].MappedInIoSpace = portAccess[0].MappedInIoSpace;
  pCurPortAccess[HWINFO_SST_CMDFIFOREGS_INDEX].VirtualAddress  = (PUCHAR)portAccess[0].VirtualAddress + SST_CMDAGP_OFFSET;

  pCurPortAccess[HWINFO_SST_2DREGS_INDEX].InIoSpace       = portAccess[0].InIoSpace;
  pCurPortAccess[HWINFO_SST_2DREGS_INDEX].MappedInIoSpace = portAccess[0].MappedInIoSpace;
  pCurPortAccess[HWINFO_SST_2DREGS_INDEX].VirtualAddress  = (PUCHAR)portAccess[0].VirtualAddress + SST_2D_OFFSET;

  pCurPortAccess[HWINFO_SST_3DREGS_INDEX].InIoSpace       = portAccess[0].InIoSpace;
  pCurPortAccess[HWINFO_SST_3DREGS_INDEX].MappedInIoSpace = portAccess[0].MappedInIoSpace;
  pCurPortAccess[HWINFO_SST_3DREGS_INDEX].VirtualAddress  = (PUCHAR)portAccess[0].VirtualAddress + SST_3D_OFFSET;

  pCurPortAccess[HWINFO_SST_FB_INDEX].InIoSpace       = portAccess[1].InIoSpace;
  pCurPortAccess[HWINFO_SST_FB_INDEX].MappedInIoSpace = portAccess[1].MappedInIoSpace;
  pCurPortAccess[HWINFO_SST_FB_INDEX].VirtualAddress  = (PUCHAR)portAccess[1].VirtualAddress;

  pCurPortAccess[HWINFO_SST_IO_INDEX].InIoSpace       = portAccess[2].InIoSpace;
  pCurPortAccess[HWINFO_SST_IO_INDEX].MappedInIoSpace = portAccess[2].MappedInIoSpace;
  pCurPortAccess[HWINFO_SST_IO_INDEX].VirtualAddress  = (PUCHAR)portAccess[2].VirtualAddress;

  // map the slave access ranges
  if (1 < HwDeviceExtension->numUnits)
  {
    for (i = 1; i < HwDeviceExtension->numUnits; i++)
    {
      pCurPortAccess = &portAccess[3 + i * HWINFO_SST_MAX_CHIP_INDEX];

      // sparse map membase0 (the memory mapped regs) to user mode
      for (j = 0; j < sizeof(SparseMemBase0Map)/sizeof(SparseMemBase0Map[0]); j++)
      {
        pCurPortAccess[j].VirtualAddress = processHandle;
        pCurPortAccess[j].InIoSpace =
        pCurPortAccess[j].MappedInIoSpace = HwDeviceExtension->MemBase0InIOSpace;

        physLength = SparseMemBase0Size[j];

        memcpy(&physAddr, &HwDeviceExtension->sliAccessRanges[i][0], sizeof(PHYSICAL_ADDRESS));
        physAddr.LowPart += SparseMemBase0Map[j];

        status = VideoPortMapMemory(HwDeviceExtension,
                                    physAddr,
                                    &physLength,
                                    &(pCurPortAccess[j].MappedInIoSpace),
                                    &(pCurPortAccess[j].VirtualAddress));

        if (status != NO_ERROR)
        {
          VideoDebugPrint((0, "  GlideMapSlaveChips - failed mapping slave chip %ld, membase0 range %ld\n", i, j));
          return status;
        }
      }

      // copy master frame buffer mapping to slave
      pCurPortAccess[HWINFO_SST_FB_INDEX].InIoSpace       = portAccess[1].InIoSpace;
      pCurPortAccess[HWINFO_SST_FB_INDEX].MappedInIoSpace = portAccess[1].MappedInIoSpace;
      pCurPortAccess[HWINFO_SST_FB_INDEX].VirtualAddress  = portAccess[1].VirtualAddress;

#if 1
      // copy master i/o mapping to slave
      pCurPortAccess[HWINFO_SST_IO_INDEX].InIoSpace       = portAccess[2].InIoSpace;
      pCurPortAccess[HWINFO_SST_IO_INDEX].MappedInIoSpace = portAccess[2].MappedInIoSpace;
      pCurPortAccess[HWINFO_SST_IO_INDEX].VirtualAddress  = portAccess[2].VirtualAddress;
#else
      // map i/o to user mode
      pCurPortAccess[HWINFO_SST_IO_INDEX].VirtualAddress = processHandle;
      pCurPortAccess[HWINFO_SST_IO_INDEX].InIoSpace =
      pCurPortAccess[HWINFO_SST_IO_INDEX].MappedInIoSpace = HwDeviceExtension->IOBaseInIoSpace;

      physLength = HwDeviceExtension->IOBaseLength;

      memcpy(&physAddr, &HwDeviceExtension->sliAccessRanges[i][2], sizeof(PHYSICAL_ADDRESS));

      status = VideoPortMapMemory(HwDeviceExtension,
                                  physAddr,
                                  &physLength,
                                  &(pCurPortAccess[HWINFO_SST_IO_INDEX].MappedInIoSpace),
                                  &(pCurPortAccess[HWINFO_SST_IO_INDEX].VirtualAddress));

      if (status != NO_ERROR)
      {
        VideoDebugPrint((0, "  GlideMapSlaveChips - failed mapping slave chip %ld i/o range\n", i));
        return status;
      }
#endif
    }
  }

  return status;
}

/*----------------------------------------------------------------------
Function name:  GlideUnmapSlaveChips

Description:

Information:

Return:
----------------------------------------------------------------------*/

VP_STATUS
GlideUnmapSlaveChips(PHW_DEVICE_EXTENSION HwDeviceExtension,
                     GLIDE_BASE_INFO      *pglideBaseInfo)
{
  VP_STATUS     status = NO_ERROR;
  ULONG         i, j;
  PVIDEO_MEMORY mappedMemory;


  // unmap the slave access ranges
  if (1 < HwDeviceExtension->numUnits)
  {
    for (i = 1; i < HwDeviceExtension->numUnits; i++)
    {
      // sparse map membase0 (the memory mapped regs) to user mode
      for (j = 0; j < sizeof(SparseMemBase0Map)/sizeof(SparseMemBase0Map[0]); j++)
      {
        mappedMemory = &pglideBaseInfo->VideoMemory[3 + i * HWINFO_SST_MAX_CHIP_INDEX + j];

        status = VideoPortUnmapMemory(HwDeviceExtension,
                                      mappedMemory->RequestedVirtualAddress,
                                      pglideBaseInfo->hProcess);

        if (status != NO_ERROR)
        {
          VideoDebugPrint((0, "  GlideUnmapSlaveChips - failed unmapping slave chip %ld, membase0 range %ld\n", i, j));
          return status;
        }
      }

      // nothing to do for the frame buffer and i/o
#if 0
      // unmap i/o to user mode
      mappedMemory = &pglideBaseInfo->VideoMemory[3 + i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_IO_INDEX];

      status = VideoPortUnmapMemory(HwDeviceExtension,
                                    mappedMemory->RequestedVirtualAddress,
                                    pglideBaseInfo->hProcess);

      if (status != NO_ERROR)
      {
        VideoDebugPrint((0, "  GlideUnmapSlaveChips - failed unmapping slave chip %ld i/o range %ld\n", i));
        return status;
      }
#endif
    }
  }

  return status;
}

#if DBG
#define DUMP_SLI_AA_CONFIG(HwDeviceExtension, dwFunc, i)   DumpSLIAAConfig((HwDeviceExtension),(dwFunc),(i))

/*----------------------------------------------------------------------
Function name:  DumpSLIAAConfig

Description:

Information:

Return:    Nothing

----------------------------------------------------------------------*/

void
DumpSLIAAConfig(PHW_DEVICE_EXTENSION HwDeviceExtension, DWORD dwFunc, DWORD i)
{
  DWORD     h3ChipSetupIoBase;
  DWORD     h3ChipSetup3DBase;
  DWORD     h3ChipSetupDeviceNum;


  h3ChipSetupIoBase = (DWORD)HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IOREGS_INDEX];
  h3ChipSetup3DBase = (DWORD)HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_3DREGS_INDEX];
  //if (SLI_AA_MASTER_DEVICE != pDevTable->dwType)
    h3ChipSetupDeviceNum = i;

  VideoDebugPrint((0, "Chip%ld\n", i));
  if (SLI_AA_DISABLE == dwFunc)
  {
    VideoDebugPrint((0, "  PCIINIT0              = %08lXh\n", PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0)));
    VideoDebugPrint((0, "  CFG_INIT_ENABLE       = %08lXh\n", PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_SLI_LFB_CTRL      = %08lXh\n", PCI_CFG_RD(CFG_SLI_LFB_CTRL, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  SLICTRL               = %08lXh\n", PCI_IO_RD(h3ChipSetup3DBase+SLICTRL)));
    VideoDebugPrint((0, "  AACTRL                = %08lXh\n", PCI_IO_RD(h3ChipSetupIoBase+AACTRL)));
    VideoDebugPrint((0, "  CFG_AA_LFB_CTRL       = %08lXh\n", PCI_CFG_RD(CFG_AA_LFB_CTRL, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_SLI_AA_MISC       = %08lXh\n", PCI_CFG_RD(CFG_SLI_AA_MISC, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_VIDEO_CTRL0       = %08lXh\n", PCI_CFG_RD(CFG_VIDEO_CTRL0, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_VIDEO_CTRL1       = %08lXh\n", PCI_CFG_RD(CFG_VIDEO_CTRL1, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_VIDEO_CTRL2       = %08lXh\n", PCI_CFG_RD(CFG_VIDEO_CTRL2, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  DACMODE               = %08lXh\n", PCI_IO_RD(h3ChipSetupIoBase+DACMODE)));
    VideoDebugPrint((0, "  VIDPROCCFG            = %08lXh\n", PCI_IO_RD(h3ChipSetupIoBase+VIDPROCCFG)));

  }
  else
  {
    VideoDebugPrint((0, "  PCIINIT0              = %08lXh\n", PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0)));
    VideoDebugPrint((0, "  TMUGBEINIT            = %08lXh\n", PCI_IO_RD(h3ChipSetupIoBase+TMUGBEINIT)));
    VideoDebugPrint((0, "  CFG_INIT_ENABLE       = %08lXh\n", PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_PCI_DECODE        = %08lXh\n", PCI_CFG_RD(CFG_PCI_DECODE, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_SLI_LFB_CTRL      = %08lXh\n", PCI_CFG_RD(CFG_SLI_LFB_CTRL, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  SLICTRL               = %08lXh\n", PCI_IO_RD(h3ChipSetup3DBase+SLICTRL)));
    VideoDebugPrint((0, "  LFBMEMORYCONFIG       = %08lXh\n", PCI_IO_RD(h3ChipSetupIoBase+LFBMEMORYCONFIG)));
    VideoDebugPrint((0, "  CFG_AA_LFB_CTRL       = %08lXh\n", PCI_CFG_RD(CFG_AA_LFB_CTRL, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_AA_ZBUFF_APERTURE = %08lXh\n", PCI_CFG_RD(CFG_AA_ZBUFF_APERTURE, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_SLI_AA_MISC       = %08lXh\n", PCI_CFG_RD(CFG_SLI_AA_MISC, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_VIDEO_CTRL0       = %08lXh\n", PCI_CFG_RD(CFG_VIDEO_CTRL0, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_VIDEO_CTRL1       = %08lXh\n", PCI_CFG_RD(CFG_VIDEO_CTRL1, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  CFG_VIDEO_CTRL2       = %08lXh\n", PCI_CFG_RD(CFG_VIDEO_CTRL2, h3ChipSetupDeviceNum)));
    VideoDebugPrint((0, "  MISCINIT1             = %08lXh\n", PCI_IO_RD(h3ChipSetupIoBase+MISCINIT1)));
  }
}

#else
#define DUMP_SLI_AA_CONFIG(HwDeviceExtension, dwFunc, i)
#endif

/*----------------------------------------------------------------------
Function name:  H3_DISABLE_SLI_AA

Description:    Disables SLI and AA on Napalm

Information:

Return:    Nothing

----------------------------------------------------------------------*/

void
H3_DISABLE_SLI_AA(PHW_DEVICE_EXTENSION  HwDeviceExtension,
                  PCHIPINFO             pChipInfo,
                  PSLI_AA_MEMINFO       pMemInfo)
{
  DWORD     i;
  DWORD     h3ChipSetupIoBase;
  DWORD     h3ChipSetup3DBase;
  DWORD     h3ChipSetupDeviceNum;


  (void)pMemInfo;

  VideoDebugPrint((0, "H3_DISABLE_SLI_AA() called\n"));

  for (i = 0; i < pChipInfo->dwChips; i++)
  {
    h3ChipSetupIoBase = (DWORD)HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IOREGS_INDEX];
    h3ChipSetup3DBase = (DWORD)HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_3DREGS_INDEX];
    //if (SLI_AA_MASTER_DEVICE != pDevTable->dwType)
      h3ChipSetupDeviceNum = i;

#if (_WIN32_WINNT < 0x0500) || (PCI_LATENCY_REQUIRED_TO_PASS == 0)
    // This results in a different value than boot and should be verified
    // when we get real hardware
    PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
              ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~SST_PCI_RETRY_INTERVAL) |
               (0 << SST_PCI_RETRY_INTERVAL_SHIFT)));
#endif

    PCI_CFG_WR(CFG_INIT_ENABLE,
               (PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) &
                ~(CFG_SNOOP_MEMBASE0      |
                  CFG_SNOOP_EN            |
                  CFG_SNOOP_MEMBASE0_EN   |
                  CFG_SNOOP_MEMBASE1_EN   |
                  CFG_SNOOP_SLAVE         |
                  CFG_SNOOP_FBIINIT_WR_EN |
                  CFG_SWAP_ALGORITHM      |
                  CFG_SWAP_QUICK)),
               h3ChipSetupDeviceNum);

    PCI_CFG_WR(CFG_SLI_LFB_CTRL,
               (PCI_CFG_RD(CFG_SLI_LFB_CTRL, h3ChipSetupDeviceNum) &
                ~(CFG_SLI_LFB_CPU_WR_EN   |
                  CFG_SLI_LFB_DPTCH_WR_EN |
                  CFG_SLI_RD_EN)),
               h3ChipSetupDeviceNum);

    PCI_IO_WR(h3ChipSetup3DBase + SLICTRL, 0x0);
    PCI_IO_WR(h3ChipSetup3DBase + AACTRL, 0x0);

    PCI_CFG_WR(CFG_AA_LFB_CTRL,
               (PCI_CFG_RD(CFG_AA_LFB_CTRL, h3ChipSetupDeviceNum) &
                ~(CFG_AA_LFB_CPU_WR_EN   |
                  CFG_AA_LFB_DPTCH_WR_EN |
                  CFG_AA_LFB_RD_EN)),
               h3ChipSetupDeviceNum);

    PCI_CFG_WR(CFG_SLI_AA_MISC,
               ((PCI_CFG_RD(CFG_SLI_AA_MISC, h3ChipSetupDeviceNum) & ~CFG_VGA_VSYNC_OFFSET) |
                (0x0 << CFG_VGA_VSYNC_OFFSET_PIXELS_SHIFT) |
                (0x0 << CFG_VGA_VSYNC_OFFSET_CHARS_SHIFT)  |
                (0x0 << CFG_VGA_VSYNC_OFFSET_HXTRA_SHIFT)),
               h3ChipSetupDeviceNum);

    // Tristate slave Hsync and Vsync's
    if (0 != i)
    {
      PCI_CFG_WR(CFG_VIDEO_CTRL0, CFG_DAC_VSYNC_TRISTATE | CFG_DAC_HSYNC_TRISTATE, h3ChipSetupDeviceNum);
    }
    else
    {
      PCI_CFG_WR(CFG_VIDEO_CTRL0, 0x0, h3ChipSetupDeviceNum);
    }
    PCI_CFG_WR(CFG_VIDEO_CTRL1, 0x0, h3ChipSetupDeviceNum);
    PCI_CFG_WR(CFG_VIDEO_CTRL2, 0x0, h3ChipSetupDeviceNum);

#if (_WIN32_WINNT >= 0x0500) && (PCI_LATENCY_REQUIRED_TO_PASS != 0)
    // This results in a different value then boot and should be verified
    // when we get real hardware
    if (pChipInfo->dwChips > 1)
    {
      PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
                ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_DISABLE_IO  |
                                                            SST_PCI_DISABLE_MEM |
                                                            SST_PCI_RETRY_INTERVAL)) |
                 (0 << SST_PCI_RETRY_INTERVAL_SHIFT) |
                 SST_PCI_FORCE_FB_HIGH));
    }
    else
    {
      PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
                ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_DISABLE_IO  |
                                                            SST_PCI_DISABLE_MEM |
                                                            SST_PCI_RETRY_INTERVAL)) |
                 (0 << SST_PCI_RETRY_INTERVAL_SHIFT)));
    }
    if (IS_NAPALM)
    {
      if (66 == HwDeviceExtension->PciSpeed)
      {
        // agp board gets an automatic pass on PCI Latency DriverScenario
        // assuming a FoxFireII is installed in the system
        // so leave these bits enabled in order to pass Win2000 GDI tests on agp boards
        PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
                  ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_RETRY_INTERVAL)) |
                   SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM |
                   (0 << SST_PCI_RETRY_INTERVAL_SHIFT)));
      }
    }
#endif

    if (i > 0)
    {
      PCI_IO_WR(h3ChipSetupIoBase+DACMODE, SST_DAC_DPMS_ON_VSYNC | SST_DAC_DPMS_ON_HSYNC);
      PCI_IO_WR(h3ChipSetupIoBase+VIDPROCCFG,
                (PCI_IO_RD(h3ChipSetupIoBase+VIDPROCCFG) & ~SST_VIDEO_PROCESSOR_EN));
    }

    DUMP_SLI_AA_CONFIG(HwDeviceExtension, SLI_AA_DISABLE, i);
  }
}

/*----------------------------------------------------------------------
Function name:  H3_SETUP_SLI_AA

Description:    Sets up SLI and AA on Napalm

Information:
  // nChips: Number of chips in multi-chip configuration (1-4)
  // sliEn: Sli is to be enabled (0,1)
  // aaEn: Anti-aliasing is to be enabled (0,1)
  // aaSampleHigh: 0->Enable 2-sample AA, 1->Enable 4-sample AA
   //               2->Enable 8-sample AA
  // sliAaAnalog: 0->Enable digital SLI/AA, 1->Enable analog Sli/AA
  // sli_nLines: Number of lines owned by each chip in SLI (2-128)
  // fbMem: Amount of memory, in megabytes...

Return:    Nothing

----------------------------------------------------------------------*/

void
H3_SETUP_SLI_AA(PHW_DEVICE_EXTENSION  HwDeviceExtension,
                DWORD                 dwFunc,
                PCHIPINFO             pChipInfo,
                PSLI_AA_MEMINFO       pMemInfo)
{
  DWORD     sli_renderMask, sli_compareMask, sli_scanMask;
  DWORD     sli_nLinesLog2, nChipsLog2;
  DWORD     i;
  DWORD     aaClkOutDel;
  DWORD     aaSample8x = (pChipInfo->dwaaSampleHigh == 2);
#if 0
  DWORD     dwStrideAndAperature;
#endif
  DWORD     MEMBASE0;
  DWORD     MEMBASE1;
  DWORD     h3ChipSetupIoBase;
  DWORD     h3ChipSetup3DBase;
  DWORD     h3ChipSetupDeviceNum;
  DWORD     dwFormat;


  if (SLI_AA_DISABLE == dwFunc)
  {
    H3_DISABLE_SLI_AA(HwDeviceExtension, pChipInfo, pMemInfo);
    return;
  }

  VideoDebugPrint((0, "H3_SETUP_SLI_AA() called\n"));

  VideoDebugPrint((0, "  ChipInfo: dwChips=%d, dwsliEn=%d, dwaaEn=%d, dwaaSampleHigh=%d\n",
                   pChipInfo->dwChips, pChipInfo->dwsliEn, pChipInfo->dwaaEn, pChipInfo->dwaaSampleHigh));
  VideoDebugPrint((0, "            dwsliAaAnalog=%d, dwsli_nlines=%d, dwCfgSwapAlgorithm=%d\n",
                   pChipInfo->dwsliAaAnalog, pChipInfo->dwsli_nlines, pChipInfo->dwCfgSwapAlgorithm));
  VideoDebugPrint((0, "  MemInfo: dwTotalMemory=%08lXh, dwTileMark=%08lXh, dwTileCmpMark=%08lXh\n",
                   pMemInfo->dwTotalMemory, pMemInfo->dwTileMark, pMemInfo->dwTileCmpMark));
  VideoDebugPrint((0, "           dwaaSecondaryColorBufBegin=%08lXh, dwaaSecondaryDepthBufBegin=%08lXh\n",
                   pMemInfo->dwaaSecondaryColorBufBegin, pMemInfo->dwaaSecondaryDepthBufBegin));
  VideoDebugPrint((0, "           dwaaSecondaryDepthBufEnd=%08lXh, dwBpp=%ldh\n",
                   pMemInfo->dwaaSecondaryDepthBufEnd, pMemInfo->dwBpp));

#if 0
  GetStrideAndAperature(HwDeviceExtension, &dwStrideAndAperature);
#endif

  // Some sanity checking...
  if ((pChipInfo->dwChips == 0) ||
      (pChipInfo->dwChips == 3) ||
      (pChipInfo->dwChips  > 4) ||
      (pChipInfo->dwChips == 1 && (pChipInfo->dwsliEn || pChipInfo->dwaaSampleHigh)) ||
      (pChipInfo->dwChips == 2 && (pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh)) ||
      (aaSample8x && !(pChipInfo->dwChips == 4 && ! pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwsliAaAnalog)))
  {
    VideoDebugPrint((0, "H3_SETUP_SLI_AA() ERROR: Unsupported input params...\n"));
    return;
  }

  switch(pChipInfo->dwsli_nlines)
  {
    case 2:   sli_nLinesLog2 = 1; break;
    case 4:   sli_nLinesLog2 = 2; break;
    case 8:   sli_nLinesLog2 = 3; break;
    case 16:  sli_nLinesLog2 = 4; break;
    case 32:  sli_nLinesLog2 = 5; break;
    case 64:  sli_nLinesLog2 = 6; break;
    case 128: sli_nLinesLog2 = 7; break;
    default:
      VideoDebugPrint((0, "H3_SETUP_SLI_AA() ERROR: Unsupported sli_nLines=%d...\n",
                       pChipInfo->dwsli_nlines));
      return;
  }

  switch(pChipInfo->dwChips)
  {
    case 1: nChipsLog2 = 0; break;
    case 2: nChipsLog2 = 1; break;
    case 4: nChipsLog2 = 2; break;
    case 8: nChipsLog2 = 3; break;
    default:
      VideoDebugPrint((0, "H3_SETUP_SLI_AA() ERROR:  Unsupported nChips=%d...\n",
                       pChipInfo->dwChips));
      return;
  }

  // Each chip has now been setup.  Now connect them all together with SLI...

  MEMBASE0 = HwDeviceExtension->PhysicalMemBaseAddr0.LowPart;
  MEMBASE1 = HwDeviceExtension->PhysicalMemBaseAddr1.LowPart;

  for (i = 0; i < pChipInfo->dwChips; i++)
  {
    h3ChipSetupIoBase = (DWORD)HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IOREGS_INDEX];
    h3ChipSetup3DBase = (DWORD)HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_3DREGS_INDEX];
    //if (SLI_AA_MASTER_DEVICE != pDevTable->dwType)
      h3ChipSetupDeviceNum = i;

    // Each chip MUST must use 2ws reads and 1ws writes.  Snooping does not
    // work with 0ws writes or 1ws reads...
    // Also, each chip MUST use at least a retry interval of ~12...
    // (0x7 gets added to the timeout interval by the chip automatically...)
#if (_WIN32_WINNT >= 0x0500) && (PCI_LATENCY_REQUIRED_TO_PASS != 0)
    if (FALSE == HwDeviceExtension->bVIACoreLogic)
    {
      PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
                ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_RETRY_INTERVAL | SST_PCI_FORCE_FB_HIGH)) |
                 SST_PCI_READ_WS | SST_PCI_WRITE_WS | SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM |
                 (0 << SST_PCI_RETRY_INTERVAL_SHIFT)));
    }
    else
    {
      PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
                ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~(SST_PCI_RETRY_INTERVAL | SST_PCI_FORCE_FB_HIGH)) |
                 SST_PCI_READ_WS | SST_PCI_WRITE_WS));
    }
#else
    PCI_IO_WR(h3ChipSetupIoBase+PCIINIT0,
              ((PCI_IO_RD(h3ChipSetupIoBase+PCIINIT0) & ~SST_PCI_RETRY_INTERVAL) |
               SST_PCI_READ_WS | SST_PCI_WRITE_WS |
               (0 << SST_PCI_RETRY_INTERVAL_SHIFT)));
#endif

    // This will have to be set properly once hw comes back...For now, use
    // a value that works for simulation...
    aaClkOutDel = 0x2;
    PCI_IO_WR(h3ChipSetupIoBase+TMUGBEINIT,
              ((PCI_IO_RD(h3ChipSetupIoBase+TMUGBEINIT) & ~(SST_AA_CLK_DELAY | SST_AA_CLK_INVERT)) |
               (aaClkOutDel << SST_AA_CLK_DELAY_SHIFT) | SST_AA_CLK_INVERT));

    // Setup buffer swapping to use external sli_syncin/sli_syncout signals
    if(pChipInfo->dwChips > 1)
    {
      PCI_CFG_WR(CFG_INIT_ENABLE,
                 (PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) |
                  ((pChipInfo->dwCfgSwapAlgorithm & 0x01) << CFG_SWAPBUFFER_ALGORITHM_SHIFT) |
                  CFG_SWAP_ALGORITHM |
                  ((i == 0) ? CFG_SWAP_MASTER : 0x0)),
                 h3ChipSetupDeviceNum);
    }

    // Setup snooping...
    if (i == 0 && pChipInfo->dwChips > 1)
    {
      PCI_CFG_WR(CFG_INIT_ENABLE,
                 (PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) | CFG_SNOOP_EN),
                 h3ChipSetupDeviceNum);
    }
    else if (pChipInfo->dwChips > 1)
    {
      // For real hardware, MEMBASE0 and MEMBASE1 need to be replaced
      // with the memBaseAddr0 and memBaseAddr1 addresses of the
      // first chip (i.e. the Master chip), respectively.
      // Also, we may not need to run with CFG_SWAP_QUICK with real
      // hardware...
      PCI_CFG_WR(CFG_PCI_DECODE,
                 ((PCI_CFG_RD(CFG_PCI_DECODE, h3ChipSetupDeviceNum) & ~CFG_SNOOP_MEMBASE1) |
                  (((MEMBASE1 >> 22) & 0x3ff) << CFG_SNOOP_MEMBASE1_SHIFT)),
                 h3ChipSetupDeviceNum);

      PCI_CFG_WR(CFG_INIT_ENABLE,
                 ((PCI_CFG_RD(CFG_INIT_ENABLE, h3ChipSetupDeviceNum) & ~CFG_SNOOP_MEMBASE0)|
                  CFG_SNOOP_EN |
                  CFG_SNOOP_MEMBASE0_EN |
                  CFG_SNOOP_MEMBASE1_EN |
                  CFG_SNOOP_SLAVE |
                  CFG_SNOOP_FBIINIT_WR_EN |
                  (((MEMBASE0 >> 22) & 0x3ff) << CFG_SNOOP_MEMBASE0_SHIFT) |
                  ((pChipInfo->dwChips > 2) ? CFG_SWAP_QUICK : 0x0)),
                 h3ChipSetupDeviceNum);
    }

    // Setup cfgSliLfbCtrl
    if (pChipInfo->dwsliEn && (!pChipInfo->dwaaEn || !pChipInfo->dwaaSampleHigh))
    {
      // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples spread
      // over 2 chips, then with each pair analog SLI'ed...
      if (pChipInfo->dwChips == 4 && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog)
      {
        sli_renderMask = ((pChipInfo->dwChips>>1)-1) << sli_nLinesLog2;
        sli_compareMask = (i>>1) << sli_nLinesLog2;
        sli_scanMask = pChipInfo->dwsli_nlines - 1;
#ifdef RD_ABORT_ERROR
        PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                   ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                    (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                    (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                    ((nChipsLog2-1) << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
                    CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN),
                   h3ChipSetupDeviceNum);
#else
        PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                   ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                    (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                    (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                    ((nChipsLog2-1) << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
                    CFG_SLI_LFB_CPU_WR_EN | CFG_SLI_LFB_DPTCH_WR_EN | CFG_SLI_RD_EN),
                   h3ChipSetupDeviceNum);
#endif

        PCI_IO_WR(h3ChipSetup3DBase + SLICTRL,
                  ((sli_renderMask << SLICTL_3D_RENDERMASK_SHIFT) |
                   (sli_compareMask << SLICTL_3D_COMPAREMASK_SHIFT) |
                   (sli_scanMask << SLICTL_3D_SCANMASK_SHIFT) |
                   ((nChipsLog2-1) << SLICTL_3D_NUMCHIPS_LOG2_SHIFT) |
                   SLICTL_3D_EN));
      }
      else
      {
        sli_renderMask = (pChipInfo->dwChips-1) << sli_nLinesLog2;
        sli_compareMask = i << sli_nLinesLog2;
        sli_scanMask = pChipInfo->dwsli_nlines - 1;
#ifdef RD_ABORT_ERROR
        PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                   ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                    (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                    (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                    (nChipsLog2 << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
                    CFG_SLI_LFB_CPU_WR_EN |
                    CFG_SLI_LFB_DPTCH_WR_EN),
                   h3ChipSetupDeviceNum);
#else
        PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                   ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                    (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                    (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                    (nChipsLog2 << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
                    CFG_SLI_LFB_CPU_WR_EN |
                    CFG_SLI_LFB_DPTCH_WR_EN |
                    CFG_SLI_RD_EN),
                   h3ChipSetupDeviceNum);
#endif

        PCI_IO_WR(h3ChipSetup3DBase + SLICTRL,
                  ((sli_renderMask << SLICTL_3D_RENDERMASK_SHIFT) |
                   (sli_compareMask << SLICTL_3D_COMPAREMASK_SHIFT) |
                   (sli_scanMask << SLICTL_3D_SCANMASK_SHIFT) |
                   ((nChipsLog2) << SLICTL_3D_NUMCHIPS_LOG2_SHIFT) |
                   SLICTL_3D_EN));
      }
    }
    else if (!pChipInfo->dwsliEn && pChipInfo->dwaaEn)
    {
      sli_renderMask = 0x0;
      sli_compareMask = 0x0;
      sli_scanMask = 0x0;
      PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                 ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                  (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                  (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                  (0x0 << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT)),
                 h3ChipSetupDeviceNum);

      PCI_IO_WR(h3ChipSetup3DBase + SLICTRL,
                ((sli_renderMask << SLICTL_3D_RENDERMASK_SHIFT) |
                 (sli_compareMask << SLICTL_3D_COMPAREMASK_SHIFT) |
                 (sli_scanMask << SLICTL_3D_SCANMASK_SHIFT) |
                 (0x0 << SLICTL_3D_NUMCHIPS_LOG2_SHIFT)));
    }
    else
    {
      // pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh
      sli_renderMask = ((pChipInfo->dwChips>>1)-1) << sli_nLinesLog2;
      sli_compareMask = (i >> 1) << sli_nLinesLog2;
      sli_scanMask = pChipInfo->dwsli_nlines - 1;
#ifdef RD_ABORT_ERROR
      PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                 ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                  (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                  (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                  ((nChipsLog2-1) << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
                  CFG_SLI_LFB_CPU_WR_EN |
                  CFG_SLI_LFB_DPTCH_WR_EN),
                 h3ChipSetupDeviceNum);
#else
      PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                 ((sli_renderMask << CFG_SLI_LFB_RENDERMASK_SHIFT) |
                  (sli_compareMask << CFG_SLI_LFB_COMPAREMASK_SHIFT) |
                  (sli_scanMask << CFG_SLI_LFB_SCANMASK_SHIFT) |
                  ((nChipsLog2-1) << CFG_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
                  CFG_SLI_LFB_CPU_WR_EN |
                  CFG_SLI_LFB_DPTCH_WR_EN |
                  CFG_SLI_RD_EN),
                 h3ChipSetupDeviceNum);
#endif

      PCI_IO_WR(h3ChipSetup3DBase + SLICTRL,
                ((sli_renderMask << SLICTL_3D_RENDERMASK_SHIFT) |
                 (sli_compareMask << SLICTL_3D_COMPAREMASK_SHIFT) |
                 (sli_scanMask << SLICTL_3D_SCANMASK_SHIFT) |
                 ((nChipsLog2-1) << SLICTL_3D_NUMCHIPS_LOG2_SHIFT) |
                 SLICTL_3D_EN));
    }

    // Sanity checking...
    if ((sli_renderMask > 255) || (sli_compareMask > 255) || (sli_scanMask > 255))
    {
      VideoDebugPrint((0, "H3_SETUP_SLI_AA() ERROR: sli render/compare/scan masks greater than 255...\n"));
      // so what should we do here?
      // just continue on like there's nothing wrong?
#if DBG
      //_asm int 3;
#endif
    }

    // Setup cfgSliAaTiledAperture
    if (pChipInfo->dwsliEn && !pChipInfo->dwaaEn)
    {
      // SLI only...

#if 0
      // By default, setup beginning of SLI/AA tiled aperture to be evenly
      // split between linear and tiled (based on memory size...)
      // Make 1/2 memory size be the split...
      PCI_IO_WR(h3ChipSetupIoBase+LFBMEMORYCONFIG,
                ((pMemInfo->dwTileMark >> 12) & 0x1fff) | // tile aperture base bits(12:0)
                 ((pMemInfo->dwTileMark >> 13) << 23) | // base bits(14:13)
                 dwStrideAndAperature | // number of sgram tiles in X...
                 (0x0 << 31));          // write to lfbMemoryTileCtrl
#endif
#if 0
      PCI_IO_WR(h3ChipSetupIoBase+LFBMEMORYCONFIG,
                ((pMemInfo->dwTileCmpMark >> 12) & 0x7fff) | // tile aperture base bits(14:0)
                 (0x1<<15) |	// Use lfbMemoryTileCompare[14:0] for tiled/linear comparison
                 (0x1<<31));  // write to lfbMemoryTileCompare
#endif

#ifdef RD_ABORT_ERROR
      {
        DWORD tileBegin;
        DWORD tileEnd;

        tileBegin = pMemInfo->dwTileMark;
        tileEnd = pMemInfo->dwTotalMemory;

        PCI_CFG_WR(CFG_AA_LFB_CTRL, CFG_AA_LFB_RD_EN, h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_AA_ZBUFF_APERTURE,
                   (((tileBegin >> 12) << CFG_AA_DEPTH_BUFFER_BEG_SHIFT) |
                    ((tileEnd >> 12) << CFG_AA_DEPTH_BUFFER_END_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
#endif
    }
    else
    {
      // AA is enabled...
      DWORD tileBegin, tileEnd, aaSecondaryBuffersBegin, aaDepthBufferBegin;
      DWORD aaDepthBufferEnd;


      // By default for simulation, setup the memory map as follows:
      // (for a 64MB memory example)
      //
      // +==========================+ 64 MB
      // +                          +
      // +  Secondary depth buffer  +
      // +                          +
      // +==========================+ 56 MB
      // +                          +
      // +  Secondary color buffers +
      // +                          +
      // +==========================+ 40 MB
      // +                          +
      // +  Primary depth buffer    +
      // +                          +
      // +==========================+ 32 MB
      // +                          +
      // +  Primary color buffers   +
      // +                          +
      // +==========================+ 16 MB  <-- Tile memory starts here...
      // +                          +
      // +  Linear memory space     +
      // +                          +
      // +==========================+ 0
      tileBegin = pMemInfo->dwTileMark;
      tileEnd = pMemInfo->dwTotalMemory;
      if (aaSample8x ||
         // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
         // spread over 4 chips, then with each pair analog SLI'ed...
         (pChipInfo->dwChips == 4   &&
          !pChipInfo->dwsliEn       &&
          pChipInfo->dwaaEn         &&
          pChipInfo->dwaaSampleHigh &&
          pChipInfo->dwsliAaAnalog))
      {
        //
        // Since the hardware will not properly support averaging
        // the subsamples coming from 4 separate chips, we just punt
        // and only return aliased data (color from the first chip)
        // by making all tiled memory look like it is the Z-Buffer...
        aaDepthBufferBegin = tileBegin;
        aaDepthBufferEnd = tileEnd;
      }
      else
      {
        aaDepthBufferBegin = pMemInfo->dwaaSecondaryDepthBufBegin;
        aaDepthBufferEnd = pMemInfo->dwaaSecondaryDepthBufEnd;
      }
      if ((pChipInfo->dwChips == 2 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh) ||
          (pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog) ||
          (pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && !aaSample8x && pChipInfo->dwsliAaAnalog))
         // Two chips-- 2-sample AA (1 subsampled stored in each chip)
         // Fool the PCI frontend section...the AA reads/writes will still
         // be duplicated to the secondary AA buffers, but the secondary
         // buffer start will point to the primary buffers.  This will,
         // in effect, make it as though we only have one AA buffer...
         //
         // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples
         // spread over 2 chips, then with each pair analog SLI'ed...
         //
         // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
         // spread over 4 chips, then with each pair analog SLI'ed...
         aaSecondaryBuffersBegin = tileBegin;
      else
        aaSecondaryBuffersBegin = pMemInfo->dwaaSecondaryColorBufBegin;

#if 0
      PCI_IO_WR(h3ChipSetupIoBase+LFBMEMORYCONFIG,
                ((tileBegin >> 12) & 0x1fff) |  // tile aperture base bits(12:0)
                (((tileBegin >> 12) >> 13) << 23) | // base bits(14:13)
                dwStrideAndAperature |
                (0x0<<31));                     // write to lfbMemoryTileCtrl
#endif

#if 0
      PCI_IO_WR(h3ChipSetupIoBase+LFBMEMORYCONFIG,
                ((tileBegin >> 12) & 0x7fff) | // tile aperture base bits(14:0)
                (0x1<<15) |	// Use lfbMemoryTileCompare[14:0] for tiled/linear comparison
                (0x1<<31));	// write to lfbMemoryTileCompare
#endif

      if (15 == pMemInfo->dwBpp)
        dwFormat = CFG_AA_LFB_RD_FORMAT_15BPP;
      else if (32 == pMemInfo->dwBpp)
        dwFormat = CFG_AA_LFB_RD_FORMAT_32BPP;
      else
        dwFormat = CFG_AA_LFB_RD_FORMAT_16BPP;

      if ((pChipInfo->dwaaSampleHigh ||
         (pChipInfo->dwChips == 2 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh) ||
         (pChipInfo->dwChips == 4 && pChipInfo->dwsliEn && pChipInfo->dwaaEn && !pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog) ||
         (pChipInfo->dwChips == 4 && !pChipInfo->dwsliEn && pChipInfo->dwaaEn && pChipInfo->dwaaSampleHigh && pChipInfo->dwsliAaAnalog)))
        dwFormat |= CFG_AA_LFB_RD_DIVIDE_BY_4;

      PCI_CFG_WR(CFG_AA_LFB_CTRL,
                 ((aaSecondaryBuffersBegin << CFG_AA_BASEADDR_SHIFT) |
                  CFG_AA_LFB_CPU_WR_EN |
                  CFG_AA_LFB_DPTCH_WR_EN |
                  CFG_AA_LFB_RD_EN |
                  dwFormat),
                 h3ChipSetupDeviceNum);

      // By default, setup beginning of depth buffer to be at (3/8) * total
      // memory size...
      PCI_CFG_WR(CFG_AA_ZBUFF_APERTURE,
                 (((aaDepthBufferBegin >> 12) << CFG_AA_DEPTH_BUFFER_BEG_SHIFT) |
                  ((aaDepthBufferEnd >> 12) << CFG_AA_DEPTH_BUFFER_END_SHIFT)),
                 h3ChipSetupDeviceNum);
    }

    // Setup vga_vsync_offset field...
    if (pChipInfo->dwChips > 1 && i > 0 && (pChipInfo->dwaaEn || pChipInfo->dwsliEn))
    {
      DWORD vsyncOffsetPixels, vsyncOffsetChars, vsyncOffsetHXtra;

      if ((pChipInfo->dwsliAaAnalog &&
           // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples
           // spread over 2 chips, then with each pair analog SLI'ed...
           !(pChipInfo->dwChips == 4 &&
             pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             !pChipInfo->dwaaSampleHigh &&
             pChipInfo->dwsliAaAnalog &&
             i != 2) &&
           // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
           // spread over 4 chips, then with each pair analog SLI'ed...
           // This covers the aaSample8x case also...
           !(pChipInfo->dwChips == 4 &&
             !pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             pChipInfo->dwaaSampleHigh &&
             pChipInfo->dwsliAaAnalog &&
             i != 2)) ||
          // Handle four chips, 2-way analog SLI with digital 4-sample AA...
          (pChipInfo->dwChips == 4 &&
           pChipInfo->dwsliEn &&
           pChipInfo->dwaaEn &&
           pChipInfo->dwaaSampleHigh &&
           !pChipInfo->dwsliAaAnalog &&
           i == 2))
      {
        // The desired value for the hardware is actually
        // vsyncOffsetPixels = 7, vsyncOffsetChars = 3, but the
        // vga_crtc_fast module has a bug in it which causes us to
        // have to bump the vsyncOffsetChars field when using
        // vsyncOffsetPixels = 7
        vsyncOffsetPixels = 7;
        vsyncOffsetChars = 4;
        vsyncOffsetHXtra = 0;
      }
      else
      {
        // Run slave 8 clocks ahead...

        // Desired value is vsyncOffsetPixels = 7, vsyncOffsetChars = 4
        // but workaround bug in vga_crtc_fast as explained above...
        vsyncOffsetPixels = 7;
        vsyncOffsetChars = 5;
        vsyncOffsetHXtra = 0;
      }

      PCI_CFG_WR(CFG_SLI_AA_MISC,
                 ((PCI_CFG_RD(CFG_SLI_AA_MISC, h3ChipSetupDeviceNum) & ~CFG_VGA_VSYNC_OFFSET) |
                  (vsyncOffsetPixels << CFG_VGA_VSYNC_OFFSET_PIXELS_SHIFT) |
                  (vsyncOffsetChars << CFG_VGA_VSYNC_OFFSET_CHARS_SHIFT) |
                  (vsyncOffsetHXtra << CFG_VGA_VSYNC_OFFSET_HXTRA_SHIFT)),
                 h3ChipSetupDeviceNum);
    }

    if (pChipInfo->dwChips == 1 && pChipInfo->dwaaEn)
    {
      // Single chip-- 2-sample AA...
      PCI_CFG_WR(CFG_VIDEO_CTRL0,
                 (CFG_ENHANCED_VIDEO_EN | CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                  (CFG_VIDEO_OTHERMUX_SEL_PIPE<<CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                  CFG_DIVIDE_VIDEO_BY_2),
                 h3ChipSetupDeviceNum);
      PCI_CFG_WR(CFG_VIDEO_CTRL1,
                 ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                  (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                  (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                  (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                 h3ChipSetupDeviceNum);
      PCI_CFG_WR(CFG_VIDEO_CTRL2,
                 ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                  (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                 h3ChipSetupDeviceNum);
    }
    else if (pChipInfo->dwChips == 2 &&
             !pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             pChipInfo->dwaaSampleHigh &&
             !pChipInfo->dwsliAaAnalog)
    {
      // Two chips-- 4-sample digital AA...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 2 &&
             !pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             pChipInfo->dwaaSampleHigh &&
             pChipInfo->dwsliAaAnalog)
    {
      // Two chips-- 4-sample analog AA...
      if (i == 0)
      {
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Tristate HSYNC since both chips share a common hsync signal
        // (since we want to support analog SLI also...)
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_DAC_HSYNC_TRISTATE |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
      }

      PCI_CFG_WR(CFG_VIDEO_CTRL1,
                 ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                  (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                  (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                  (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                 h3ChipSetupDeviceNum);
      PCI_CFG_WR(CFG_VIDEO_CTRL2,
                 ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                  (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                 h3ChipSetupDeviceNum);

    }
    else if (pChipInfo->dwChips == 2 &&
             pChipInfo->dwsliEn &&
             !pChipInfo->dwaaEn &&
             !pChipInfo->dwsliAaAnalog)
    {
      // Two chips-- 2-way digital SLI...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    (CFG_VIDEO_OTHERMUX_SEL_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) <<
                    CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if ((pChipInfo->dwChips == 2 || pChipInfo->dwChips == 4) &&
             pChipInfo->dwsliEn &&
             !pChipInfo->dwaaEn &&
             pChipInfo->dwsliAaAnalog)
    {
      // Two or four chips-- 2/4-way analog SLI...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second, third, fourth chips...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 2 &&
             pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             !pChipInfo->dwaaSampleHigh &&
             !pChipInfo->dwsliAaAnalog)
    {
      // Two chips-- 2-sample AA with 2-way digital SLI...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 2 &&
             pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             !pChipInfo->dwaaSampleHigh &&
             pChipInfo->dwsliAaAnalog)
    {
      // Two -- 2-sample AA with 2/4-way analog SLI...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 4 &&
             pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             !pChipInfo->dwaaSampleHigh &&
             pChipInfo->dwsliAaAnalog)
    {
      // Now instead of {4-way SLI, 2 sample AA} we do 2 subsamples
      // spread over 2 chips, then with each pair analog SLI'ed...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else if (i == 1 || i == 3)
      {
        // Second and fourth chips...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_DAC_HSYNC_TRISTATE |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (((i>>1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((0xff<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Third chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 4 &&
             !pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             pChipInfo->dwaaSampleHigh &&
             !aaSample8x &&
             pChipInfo->dwsliAaAnalog)
    {
      // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
      // spread over 4 chips, then with each pair analog SLI'ed...

      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else if (i == 1 || i == 3)
      {
        // Second and fourth chips...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_DAC_HSYNC_TRISTATE |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Third chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_DAC_HSYNC_TRISTATE |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 4 &&
             !pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             pChipInfo->dwaaSampleHigh &&
             aaSample8x &&
             pChipInfo->dwsliAaAnalog)
    {
      // 8x AA case...Store 2 subsamples in each of the 4 chips...

      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_8),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else if (i == 1 || i == 3)
      {
        // Second and fourth chips...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    CFG_DAC_HSYNC_TRISTATE |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Third chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    CFG_DAC_HSYNC_TRISTATE |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_8),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 2 &&
             !pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             !pChipInfo->dwaaSampleHigh &&
             !pChipInfo->dwsliAaAnalog)
    {
      // Two chips-- 2-sample digital AA (1 subsampled stored in each chip)
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 2 &&
             !pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             !pChipInfo->dwaaSampleHigh &&
             pChipInfo->dwsliAaAnalog)
    {
      // Two chips-- 2-sample analog AA (1 subsampled stored in each chip)
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second chip...
        // Tristate HSYNC since both chips share a common hsync signal
        // (since we want to support analog SLI also...)
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_DAC_HSYNC_TRISTATE |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
      }
      PCI_CFG_WR(CFG_VIDEO_CTRL1,
                 ((0x0 << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                  (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                  (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                  (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                 h3ChipSetupDeviceNum);
      PCI_CFG_WR(CFG_VIDEO_CTRL2,
                 ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                  (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                 h3ChipSetupDeviceNum);
    }
    else if (pChipInfo->dwChips == 4 &&
             pChipInfo->dwsliEn &&
             !pChipInfo->dwaaEn &&
             !pChipInfo->dwsliAaAnalog)
    {
      // Four chips-- 4-way digital SLI...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    (CFG_VIDEO_OTHERMUX_SEL_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_SLI_AAFIFO_COMPARE_INV |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second, third, fourth chips...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 4 &&
             pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             !pChipInfo->dwaaSampleHigh &&
             !pChipInfo->dwsliAaAnalog)
    {
      // Four chips-- 2-sample AA with 4-way digital SLI...
      if(i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_SLI_AAFIFO_COMPARE_INV |
                    CFG_DIVIDE_VIDEO_BY_2),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Second, third, fourth chips...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    (0x0 << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((((pChipInfo->dwChips-1)<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    ((i<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 4 &&
             pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             pChipInfo->dwaaSampleHigh &&
             !pChipInfo->dwsliAaAnalog)
    {
      // Four chips-- 2-way analog SLI with digital 4-sample AA...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else if (i == 1 || i == 3)
      {
        // Second and fourth chips...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_DAC_HSYNC_TRISTATE |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_1),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (((i>>1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((0xff<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Third chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE_PLUS_AAFIFO << CFG_VIDEO_OTHERMUX_SEL_FALSE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0xff << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
    }
    else if (pChipInfo->dwChips == 4 &&
             pChipInfo->dwsliEn &&
             pChipInfo->dwaaEn &&
             pChipInfo->dwaaSampleHigh &&
             pChipInfo->dwsliAaAnalog)
    {
      // Four chips-- 2-way analog SLI with analog 4-sample AA...
      if (i == 0)
      {
        // First chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((0x0<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else if (i == 1 || i == 3)
      {
        // Second and fourth chips...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_DAC_HSYNC_TRISTATE |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    (((i>>1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    (((i>>1)<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }
      else
      {
        // Third chip...
        PCI_CFG_WR(CFG_VIDEO_CTRL0,
                   (CFG_ENHANCED_VIDEO_EN |
                    CFG_ENHANCED_VIDEO_SLV |
                    CFG_VIDEO_LOCALMUX_DESKTOP_PLUS_OVERLAY |
                    (CFG_VIDEO_OTHERMUX_SEL_PIPE << CFG_VIDEO_OTHERMUX_SEL_TRUE_SHIFT) |
                    CFG_DIVIDE_VIDEO_BY_4),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL1,
                   (((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_FETCH_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_RENDERMASK_CRT_SHIFT) |
                    ((0x1<<sli_nLinesLog2) << CFG_SLI_COMPAREMASK_CRT_SHIFT)),
                   h3ChipSetupDeviceNum);
        PCI_CFG_WR(CFG_VIDEO_CTRL2,
                   ((0x0 << CFG_SLI_RENDERMASK_AAFIFO_SHIFT) |
                    (0x0 << CFG_SLI_COMPAREMASK_AAFIFO_SHIFT)),
                   h3ChipSetupDeviceNum);
      }

    }
    else
    {
      VideoDebugPrint((0, "H3_SETUP_SLI_AA() ERROR: Unsupported combination of {nChips, sliEn, aaEn, ...}\n"));
    }

    if ((pChipInfo->dwChips == 4 &&
         pChipInfo->dwsliEn &&
         pChipInfo->dwaaEn &&
         pChipInfo->dwaaSampleHigh &&
         i == 3) ||
        (pChipInfo->dwChips == 4 &&
         pChipInfo->dwsliEn &&
         pChipInfo->dwaaEn &&
         !pChipInfo->dwaaSampleHigh &&
         pChipInfo->dwsliAaAnalog &&
         i == 3))

    {
      // Make sure that last chip properly waits for data to be xfered
      // over the PCI bus before driving...
      PCI_CFG_WR(CFG_SLI_AA_MISC,
                 (PCI_CFG_RD(CFG_SLI_AA_MISC, h3ChipSetupDeviceNum) |
                  CFG_AA_LFB_RD_SLV_WAIT),
                 h3ChipSetupDeviceNum);
    }

    // Deal with the problem for LFB reads where the data really needs to
    // come from 4 different chips...Since the hardware does not support
    // this, we figure out a way to only return aliased data back back...
    // This is accomplished by having the Master return its lfb data
    //
    // By turning off AA LFB reads, chips 3/4  no longer snoop lfb reads
    // at all.  Then, there is only handshaking between chips 1 & 2 so
    // the Master stays happy...
    //
    // Now instead of {2-way SLI, 4 sample AA} we do 4 subsamples
    // spread over 4 chips, then with each pair analog SLI'ed...
    // This covers the aaSample8x case also...
    if (pChipInfo->dwChips == 4 &&
        !pChipInfo->dwsliEn &&
        pChipInfo->dwaaEn &&
        pChipInfo->dwaaSampleHigh &&
        pChipInfo->dwsliAaAnalog &&
        i > 1)
    {
      PCI_CFG_WR(CFG_AA_LFB_CTRL,
                 ((PCI_CFG_RD(CFG_AA_LFB_CTRL, h3ChipSetupDeviceNum) & ~CFG_AA_LFB_RD_EN)),
                 h3ChipSetupDeviceNum);
    }

    if(i > 0)
    {
      // For the slave chips, make the video PLL lock to the Master's
      // sync_clk_out clock output...
      PCI_CFG_WR(CFG_VIDEO_CTRL0,
                 (PCI_CFG_RD(CFG_VIDEO_CTRL0, h3ChipSetupDeviceNum) |
                  CFG_VIDPLL_SEL),
                 h3ChipSetupDeviceNum);

      // Power down Slave(s) RAMDAC...
      PCI_IO_WR(h3ChipSetupIoBase+MISCINIT1,
                (PCI_IO_RD(h3ChipSetupIoBase+MISCINIT1) | SST_POWERDOWN_DAC));
    }
    // TO DO:
    // Adjust pci fifo thresholds?

    DUMP_SLI_AA_CONFIG(HwDeviceExtension, SLI_AA_ENABLE, i);
  } // for(i=0; i<pChipInfo->dwChips; i++) ...
}

/*----------------------------------------------------------------------
Function name:  EnableSLIAA

Description:    Enables SLI and AA on Napalm

Information:

Return:    Nothing

----------------------------------------------------------------------*/

void
EnableSLIAA(PHW_DEVICE_EXTENSION HwDeviceExtension, PSLI_AA_REQUEST pRequest)
{
  ULONG     i;
  ULONG     nProgram;   /* V56K-CHIPCOHERENCE: units we may touch */
  CHIPINFO  ChipInfo;
  SstIORegs *pMasterIO, *pSlaveIO;
#if DBG
  SstCRegs  *pMasterCmdAgp, *pSlaveCmdAgp;
  SstGRegs  *pMaster2D, *pSlave2D;
  SstRegs   *pMaster3D, *pSlave3D;
#endif


  pMasterIO = HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_IOREGS_INDEX];

#if DBG
  pMasterCmdAgp = HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_CMDFIFOREGS_INDEX];
  pMaster2D     = HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_2DREGS_INDEX];
  pMaster3D     = HwDeviceExtension->sliMappedAddress[0][HWINFO_SST_3DREGS_INDEX];
#endif

  /* V56K-CHIPCOHERENCE: refuse a request that does not cover every detected
  ** unit.  The loop below reprograms each slave's mode/video registers, but
  ** only ChipInfo.dwChips of them are then given an SLI role by
  ** H3_SETUP_SLI_AA -- and 4-way is forced ANALOG combining where 2-way is
  ** digital.  Asking a 4-chip board for 2 chips therefore left units 2-3
  ** driving the video path with no role, which wedged the machine hard
  ** (no network, flickering output, power cycle to recover).  Losing SLI is
  ** strictly better than half-programming the board. */
  if (pRequest->ChipInfo.dwChips != HwDeviceExtension->numUnits)
  {
    VideoDebugPrint((0, "retro3dfx SLIAA-MISMATCH: req=%ld units=%ld -> refusing enable\n",
                     pRequest->ChipInfo.dwChips, HwDeviceExtension->numUnits));
    DisableSLIAA(HwDeviceExtension, pRequest);
    return;
  }

  // v56k: the 6000 has 4 chips and an external clock (see Win9x SLIAA.C)
  if (4 == pRequest->ChipInfo.dwChips)
  {
    V56KSetExternalClock(HwDeviceExtension);
    VideoDebugPrint((0, "retro3dfx V56K-CLOCK: external clock programmed (chips=%ld)\n",
                     pRequest->ChipInfo.dwChips));
  }

  // disable i/o on master
  UpdatePCICommandReg(HwDeviceExtension, 0, ~PCI_ENABLE_IO_SPACE, 0);

  /* V56K-CHIPCOHERENCE: never touch a unit the setup pass will not configure. */
  nProgram = pRequest->ChipInfo.dwChips;
  if (nProgram > HwDeviceExtension->numUnits)
    nProgram = HwDeviceExtension->numUnits;

  for (i = 1; i < nProgram; i++)
  {
    // enable i/o on slave
    UpdatePCICommandReg(HwDeviceExtension, i, (LONG)-1, PCI_ENABLE_IO_SPACE);

    // set mode on slave so that master and slave have same mode
    H3SetMode(HwDeviceExtension,
              HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IOREGS_INDEX],
              HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IO_INDEX]);

    // copy tiled mode configuration regs from master to slave
    pSlaveIO = HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_IOREGS_INDEX];

#if DBG
    pSlaveCmdAgp = HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_CMDFIFOREGS_INDEX];
    pSlave2D     = HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_2DREGS_INDEX];
    pSlave3D     = HwDeviceExtension->sliMappedAddress[i][HWINFO_SST_3DREGS_INDEX];
#endif

    pSlaveIO->lfbMemoryConfig              = pMasterIO->lfbMemoryConfig;
    pSlaveIO->vidDesktopStartAddr          = pMasterIO->vidDesktopStartAddr;
    pSlaveIO->vidProcCfg                   = pMasterIO->vidProcCfg;
    pSlaveIO->vidOverlayDudxOffsetSrcWidth = pMasterIO->vidOverlayDudxOffsetSrcWidth;
    pSlaveIO->vidDesktopOverlayStride      = pMasterIO->vidDesktopOverlayStride;

    // disable i/o on slave
    UpdatePCICommandReg(HwDeviceExtension, i, ~PCI_ENABLE_IO_SPACE, 0);
  }

  // enable i/o on master
  UpdatePCICommandReg(HwDeviceExtension, 0, (LONG)-1, PCI_ENABLE_IO_SPACE);

  ChipInfo.dwChips = pRequest->ChipInfo.dwChips;
  ChipInfo.dwsliEn = pRequest->ChipInfo.dwsliEn;
  ChipInfo.dwaaEn = pRequest->ChipInfo.dwaaEn;
  ChipInfo.dwaaSampleHigh = pRequest->ChipInfo.dwaaSampleHigh;
  ChipInfo.dwsliAaAnalog = pRequest->ChipInfo.dwsliAaAnalog;
  ChipInfo.dwsli_nlines = pRequest->ChipInfo.dwsli_nlines;
  ChipInfo.dwCfgSwapAlgorithm = pRequest->ChipInfo.dwCfgSwapAlgorithm;
  ChipInfo.dwMasterID = 0x0;
  //ChipInfo.pDevTable = (DWORD)pMaster;

#ifdef RD_ABORT_ERROR
  memcpy(&HwDeviceExtension->SLIAARequest, pRequest, sizeof(SLI_AA_REQUEST));
#endif

  H3_SETUP_SLI_AA(HwDeviceExtension, SLI_AA_ENABLE, &ChipInfo, &pRequest->MemInfo);
}

/*----------------------------------------------------------------------
Function name:  DisableSLIAA

Description:    Disable SLI and AA on Napalm

Information:

Return:    Nothing

----------------------------------------------------------------------*/
void
DisableSLIAA(PHW_DEVICE_EXTENSION HwDeviceExtension, PSLI_AA_REQUEST pRequest)
{
  CHIPINFO ChipInfo;


  ChipInfo.dwChips = pRequest->ChipInfo.dwChips;
  ChipInfo.dwsliEn = pRequest->ChipInfo.dwsliEn;
  ChipInfo.dwaaEn = pRequest->ChipInfo.dwaaEn;
  ChipInfo.dwaaSampleHigh = pRequest->ChipInfo.dwaaSampleHigh;
  ChipInfo.dwsliAaAnalog = pRequest->ChipInfo.dwsliAaAnalog;
  ChipInfo.dwsli_nlines = pRequest->ChipInfo.dwsli_nlines;
  ChipInfo.dwCfgSwapAlgorithm = pRequest->ChipInfo.dwCfgSwapAlgorithm;
  ChipInfo.dwMasterID = 0x0;
  //ChipInfo.pDevTable = (DWORD)pMaster;

#ifdef RD_ABORT_ERROR
  memcpy(&HwDeviceExtension->SLIAARequest, pRequest, sizeof(SLI_AA_REQUEST));
#endif

  H3_SETUP_SLI_AA(HwDeviceExtension, SLI_AA_DISABLE, &ChipInfo, &pRequest->MemInfo);
}

#ifdef RD_ABORT_ERROR
/*----------------------------------------------------------------------
Function name:  SLI_Read_Enable

Description:    Enable SLI Reads

Information:

Return:    Nothing

----------------------------------------------------------------------*/

void
SLI_Read_Enable(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  ULONG   i;


  if (HwDeviceExtension->SLIAARequest.ChipInfo.dwsliEn)
  {
    for (i = 0; i < HwDeviceExtension->numUnits; i++)
    {
      // Turn on SLI Read
      PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                 PCI_CFG_RD(CFG_SLI_LFB_CTRL, i) | CFG_SLI_RD_EN,
                 i);

      // Turn off AA if it is not enabled
      if (! HwDeviceExtension->SLIAARequest.ChipInfo.dwaaEn)
      {
        PCI_CFG_WR(CFG_AA_LFB_CTRL,
                   PCI_CFG_RD(CFG_AA_LFB_CTRL, i) & ~CFG_AA_LFB_RD_EN,
                   i);
      }
    }
  }
}

/*----------------------------------------------------------------------
Function name:  SLI_Read_Disable

Description:    Disable SLI Reads

Information:

Return:    Nothing

----------------------------------------------------------------------*/

void
SLI_Read_Disable(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  ULONG   i;


  if (HwDeviceExtension->SLIAARequest.ChipInfo.dwsliEn)
  {
    for (i = 0; i < HwDeviceExtension->numUnits; i++)
    {
      // Turn off SLI Read
      PCI_CFG_WR(CFG_SLI_LFB_CTRL,
                 PCI_CFG_RD(CFG_SLI_LFB_CTRL, i) & ~CFG_SLI_RD_EN,
                 i);

      // Turn on AA Read
      PCI_CFG_WR(CFG_AA_LFB_CTRL,
                 PCI_CFG_RD(CFG_AA_LFB_CTRL, i) | CFG_AA_LFB_RD_EN,
                 i);
    }
  }
}
#endif // RD_ABORT_ERROR
#endif // SLI_AA


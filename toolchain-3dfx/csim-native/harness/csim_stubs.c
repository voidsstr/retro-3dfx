/* CSIM-mode stubs for the SWLIBS pci and img symbols the HAL references at link
 * time. In HAL_CSIM the simulator IS the board, so PCI enumeration only needs to
 * report a VSA-100 present with sane config data; the real platform pcilib
 * (Win/DOS port I/O) is neither available nor needed on Linux. img symbols are
 * for 3dfx's own screenshot format, unused (the harness writes its own PPM).
 * Types come from fxpci.h so signatures match the HAL's expectations exactly. */
#include <h3.h>
#include <fxpci.h>

#define VSA100_VENDOR 0x121a
#define VSA100_DEVICE 0x0009    /* Voodoo4/5 (Napalm) */
#define VSA100_REV    0x01

/* the const PciRegister descriptors the HAL passes to pciGetConfigData;
 * regAddress = PCI config-space byte offset. */
const PciRegister PCI_VENDOR_ID       = { 0x00, 2, 0 };
const PciRegister PCI_DEVICE_ID       = { 0x02, 2, 0 };
const PciRegister PCI_COMMAND         = { 0x04, 2, 0 };
const PciRegister PCI_REVISION_ID     = { 0x08, 1, 0 };
const PciRegister PCI_IO_BASE_ADDRESS = { 0x10, 4, 0 };

FxBool pciOpenEx(PciHwcCallbacks *cb)                 { (void)cb; return FXTRUE; }
FxBool pciClose(void)                                 { return FXTRUE; }

FxBool pciFindCardMultiFunc(FxU32 vID, FxU32 dID, FxU32 *devNum,
                            FxU32 *funcNum, FxU32 functionIndex)
{
    (void)vID; (void)dID;
    if (functionIndex != 0) return FXFALSE;   /* exactly one simulated board */
    if (devNum)  *devNum  = 0;
    if (funcNum) *funcNum = 0;
    return FXTRUE;
}

FxBool pciGetConfigData(PciRegister reg, FxU32 device_number, FxU32 *data)
{
    (void)device_number;
    if (!data) return FXFALSE;
    switch (reg.regAddress) {
      case 0x00: *data = VSA100_VENDOR;   break;
      case 0x02: *data = VSA100_DEVICE;   break;
      case 0x08: *data = VSA100_REV;      break;
      case 0x10: *data = 0x10000000;      break;  /* base addr the sim uses */
      default:   *data = 0;               break;
    }
    return FXTRUE;
}

FxBool pciSetConfigData(PciRegister reg, FxU32 device_number, FxU32 *data)
{ (void)reg; (void)device_number; (void)data; return FXTRUE; }

const char *pciGetErrorString(void) { return "csim-stub: no pci error"; }

/* 3dfx image lib (screenshot writer) -- unused; the harness writes its own PPM. */
int imgWriteFile(const char *name, void *data, int w, int h, int fmt)
{ (void)name; (void)data; (void)w; (void)h; (void)fmt; return 0; }

const char *imgGetErrorString(void) { return "csim-stub: img unused"; }

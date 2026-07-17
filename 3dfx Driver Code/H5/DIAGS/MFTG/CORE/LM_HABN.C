/* SST2/Habenero */

#include <stdio.h>
#include <string.h>

#include <3dfx.h>
#include <fxpci.h>

#include "lua.h"
#include "luadebug.h"
#include "lualib.h"
#include "lauxlib.h"

#include "mdclua.h"
#include "banshee.h"


#define PCI_3DFX_VENDORID 0x121A
#define PCI_HABENERO_DEVICEID 0x10      /* ??? */

#define HABENERO_MMIO_SIZE     32 * 1024 * 1024
#define HABENERO_LFBMEM_SIZE   64 * 1024 * 1024

// card info
// register peek/poke
// memory peek/poke

void lm_habn_pci_test(void);
void lm_habn_fbi_test(void);
void lm_habn_fbi_size(void);


static struct luafn_reg_struct luafn_list[] = {
  { "habn_pci_test",lm_habn_pci_test},
  { "habn_fbi_test",lm_habn_fbi_test},
  { "habn_fbi_size", lm_habn_fbi_size },
  { NULL, NULL }};


int init_habenero(LPCARDINFO card);

void lm_habn_init() {
  mdc_register_pcivendor(PCI_3DFX_VENDORID,"3dfx Interactive, Inc.");
  mdc_register_boardinitfn(PCI_3DFX_VENDORID,PCI_HABENERO_DEVICEID, init_habenero);
 
  mdcl_register_fns(luafn_list);
}

/*
 * this is the habenero init function, it should map the card number requested
 */

int init_habenero(LPCARDINFO card) {
  FxU32 base_addr;
  
  pciGetConfigData( PCI_BASE_ADDRESS_0, card->pciDevNum, &base_addr); 
  card->PCIBase0 = base_addr & ~0xF; /* MMIO */
  pciGetConfigData( PCI_BASE_ADDRESS_1, card->pciDevNum, &base_addr); 
  card->PCIBase1 = base_addr & ~0xF; /* FBMEM */
  pciGetConfigData( PCI_IO_BASE_ADDRESS, card->pciDevNum, &base_addr); 
  card->PCIBase2 = base_addr & ~0x1; /* IO SPACE */

  card->NatMem0.PhysAddr = card->PCIBase0;
  card->NatMem1.MappedSize = card->NatMem0.PhysSize = HABENERO_MMIO_SIZE;

  if (pciMapPhysicalToLinear(&card->NatMem0.MappedAddr, card->NatMem0.PhysAddr, 
			     &card->NatMem0.PhysSize) != FXTRUE) {
    card->NatMem0.MappedAddr = NULL;
    return (FXFALSE);
  }

  card->NatMem1.PhysAddr = card->PCIBase1;
  card->NatMem1.MappedSize = card->NatMem1.PhysSize = HABENERO_LFBMEM_SIZE;

  if (pciMapPhysicalToLinear(&card->NatMem1.MappedAddr, card->NatMem1.PhysAddr, 
			     &card->NatMem1.PhysSize) != FXTRUE) {
    card->NatMem1.MappedAddr = NULL;
    return (FXFALSE);
  }
  
  /*  card->OEM_version = */
  return FXTRUE;
}

// need to make some raw register access functions... but not fbwr8, etc...

void lm_habn_pci_test() {
  // pick a PCI scratch register
}

void lm_habn_fbi_test() {
  // use the memory test functions to test a block of memory
}

void lm_habn_fbi_size() {
}

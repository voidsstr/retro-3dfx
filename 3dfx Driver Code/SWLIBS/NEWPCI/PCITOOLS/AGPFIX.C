/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
**
**
** $Revision: 3$ 
** $Date: 10/11/00 7:41:08 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <3dfx.h>
#include <fxpci.h>

const PciRegister PCI_INIT_ENABLE  = { 0x40, 4, READ_WRITE };
const PciRegister PCI_BUS_SNOOP0   = { 0x44, 4, WRITE_ONLY };
const PciRegister PCI_BUS_SNOOP1   = { 0x48, 4, WRITE_ONLY };
const PciRegister PCI_CFG_STATUS   = { 0x4C, 4, READ_ONLY };

const PciRegister P2PB_MEM0_BASE   = { 0x20, 4, READ_WRITE };
const PciRegister P2PB_MEM1_BASE   = { 0x24, 4, READ_WRITE };

/* New Napalm specific registers */
const PciRegister PCI_CFG_PCI_DECODE =  {  72, 4, READ_WRITE };
const PciRegister PCI_CFG_VIDEO_CTRL0 = { 128, 4, READ_WRITE };
const PciRegister PCI_CFG_VIDEO_CTRL1 = { 132, 4, READ_WRITE };
const PciRegister PCI_CFG_VIDEO_CTRL2 = { 136, 4, READ_WRITE };
const PciRegister PCI_CFG_SLI_LFB_CTRL = { 140, 4, READ_WRITE };
const PciRegister PCI_CFG_AA_DEPTH_BUFFER_APERTURE = { 144, 4, READ_WRITE };
const PciRegister PCI_CFG_AA_LFB_CTRL = { 148, 4, READ_WRITE };
const PciRegister PCI_CFG_AGP_TEST_CTRL = { 152, 4, READ_WRITE };
const PciRegister PCI_CFG_AGP_TEST_DATA0 = { 156, 4, READ_WRITE };
const PciRegister PCI_CFG_AGP_TEST_DATA1 = { 160, 4, READ_WRITE };
const PciRegister PCI_CFG_AGP_TEST_DATA2 = { 164, 4, READ_WRITE };
const PciRegister PCI_CFG_AGP_TEST_DATA3 = { 168, 4, READ_WRITE };
const PciRegister PCI_CFG_SLI_AA_MISC    = { 172, 4, READ_WRITE };
const PciRegister P2PB_PRIMARY_BUS_NUMBER = { 0x18, 1, READ_WRITE };
const PciRegister P2PB_SECONDARY_BUS_NUMBER = { 0x19, 1, READ_WRITE };

const PciRegister *pciRegArray[] = {
        &PCI_VENDOR_ID,
        &PCI_DEVICE_ID,
        &PCI_COMMAND,
        &PCI_STATUS,
        &PCI_REVISION_ID,
        &PCI_CLASS_CODE,
        &PCI_CACHE_LINE_SIZE,
        &PCI_LATENCY_TIMER,
        &PCI_HEADER_TYPE,
        &PCI_BIST,
        &PCI_BASE_ADDRESS_0,
        &PCI_BASE_ADDRESS_1,
#ifdef OLD_PCI
        &PCI_BASE_ADDRESS_2,
        &PCI_BASE_ADDRESS_3,
        &PCI_BASE_ADDRESS_4,
        &PCI_BASE_ADDRESS_5,
#else
	&PCI_IO_BASE_ADDRESS,
#endif
        &PCI_INTERRUPT_LINE,
        &PCI_INTERRUPT_PIN,
        &PCI_MIN_GNT,
        &PCI_MAX_LAT,
        &PCI_INIT_ENABLE,
        &PCI_BUS_SNOOP0,
        &PCI_BUS_SNOOP1,
        &PCI_CFG_STATUS,
        &P2PB_MEM0_BASE,
        &P2PB_MEM1_BASE,
        // Napalm stuff
        &PCI_CFG_PCI_DECODE,
        &PCI_CFG_VIDEO_CTRL0,
        &PCI_CFG_VIDEO_CTRL1,
        &PCI_CFG_VIDEO_CTRL2,
        &PCI_CFG_SLI_LFB_CTRL,
        &PCI_CFG_AA_DEPTH_BUFFER_APERTURE,
        &PCI_CFG_AA_LFB_CTRL,
        &PCI_CFG_AGP_TEST_CTRL,
        &PCI_CFG_AGP_TEST_DATA0,
        &PCI_CFG_AGP_TEST_DATA1,
        &PCI_CFG_AGP_TEST_DATA2,
        &PCI_CFG_AGP_TEST_DATA3,
        &PCI_CFG_SLI_AA_MISC
};

static FxU32 memDecode[16] =
{
  128*1024*1024,
  256*1024*1024,
  512*1024*1024,
  1024*1024*1024,

  64*1024*1024,
  32*1024*1024,
  16*1024*1024,
   8*1024*1024,

   4*1024*1024,
   0,
   0,
   0,

   0,
   0,
   0,
   0
};
                   
void cmdFix(void)
{
    FxU32 deviceNumber;
    FxU32 deviceID = 0;
    FxU32 vendorID = 0;
    FxU32 classCode = 0;
    FxU32 napalmDevice = 0;
    
    if (!pciOpen()) {
        fprintf(stderr, pciGetErrorString());
        exit(100);
    }
    for ( deviceNumber = 0; deviceNumber < MAX_PCI_DEVICES; deviceNumber++ ) {
        if ( pciDeviceExists( deviceNumber ) ) {
            pciGetConfigData( PCI_DEVICE_ID, deviceNumber, &deviceID );
            pciGetConfigData( PCI_VENDOR_ID, deviceNumber, &vendorID );       
            if(vendorID == _3DFX_PCI_ID && (deviceID >= 0x7 && deviceID <= 0xf)) {
              napalmDevice = deviceNumber;
              break;
            }  
        }
   }

   if(napalmDevice) {
     FxU32 secondaryBus;
     FxU32 bridgePrimaryBus;
     FxU32 bridgeSecondaryBus;
     FxU32 slotNumber;

     FxU32 memBase0Min, memBase0Max, memBase0BaseLimit;
     FxU32 memBase1Min, memBase1Max, memBase1BaseLimit;
     FxU32 cfgPciDecode;

     secondaryBus = deviceNumber>>5;
     slotNumber = deviceNumber & 0x1f;

     pciGetConfigData( PCI_BASE_ADDRESS_0, napalmDevice, &memBase0Min);
     pciGetConfigData( PCI_BASE_ADDRESS_1, napalmDevice, &memBase1Min);
     pciGetConfigData( PCI_CFG_PCI_DECODE, napalmDevice, &cfgPciDecode);

     memBase0Max = (memBase0Min & 0xFFFFFFF0) + memDecode[cfgPciDecode & 0xf] - 1;
     memBase1Max = (memBase1Min & 0xFFFFFFF0) + memDecode[(cfgPciDecode >> 4) & 0xf] - 1;

     memBase0BaseLimit = (memBase0Min >> 16) | (memBase0Max & 0xFFF00000);
     memBase1BaseLimit = (memBase1Min >> 16) | (memBase1Max & 0xFFF00000);

     printf("Napalm located on bus: %d, slot: %d\n",secondaryBus,slotNumber);
     printf("MemBase0 %08lx - %08lx\n",memBase0Min,memBase0Max);
     printf("MemBase1 %08lx - %08lx\n",memBase1Min,memBase1Max);

     /* Walk PCI bridges from Napalm to bus 0, fixing memory apertures along the way. */
     do {
       for ( deviceNumber = 0; deviceNumber < MAX_PCI_DEVICES; deviceNumber++ ) {
          if ( pciDeviceExists( deviceNumber ) ) {
            /* Check for PCI-PCI bridge */
            pciGetConfigData( PCI_CLASS_CODE, deviceNumber, &classCode );
            if((classCode & 0xffff00) == 0x060400) {
              /* Okay, it's a bridge, see if it's secondary bus is the one Napalm is downstream from. */
              pciGetConfigData( P2PB_SECONDARY_BUS_NUMBER, deviceNumber, &bridgeSecondaryBus );
              if(bridgeSecondaryBus == secondaryBus) {
                printf("Fixing up apertures on bridge %08lx\n",deviceNumber);
                /* This bridge is the one we want.  Fix up it's addresses to include both Napalm
                 * memory ranges. */
                pciSetConfigData( P2PB_MEM0_BASE, deviceNumber, &memBase0BaseLimit);
                pciSetConfigData( P2PB_MEM1_BASE, deviceNumber, &memBase1BaseLimit);
                /* Snarf primary bus number and continue search... */

                secondaryBus = bridgeSecondaryBus;
                pciGetConfigData( P2PB_PRIMARY_BUS_NUMBER, deviceNumber, &bridgePrimaryBus );
                break;                 
              }  
            }              
          }
       }   
       if(deviceNumber == MAX_PCI_DEVICES) {
         fprintf(stderr, "Woah, couldn't find PCI bridge for bus %d, aborting\n",secondaryBus);
         break;
       }
     } while(bridgePrimaryBus != 0); 

   } else {
     fprintf(stderr, "No napalm boards found\n");
   } 
   
   if ( !pciClose() ) {
       fprintf(stderr, pciGetErrorString());
       exit(3);
   }
}
      

int main(int argc, char **argv)
{

    cmdFix();
    
    return 0;
}

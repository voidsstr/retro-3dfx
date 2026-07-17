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
** $Revision: 4$ 
** $Date: 10/11/00 7:38:50 PM$ 
**
*/

#include <stdio.h>
#include "3dfx.h"
#include "fxpci.h"

int print_pci_space( void ) { 
    FxU32         deviceNumber, functionNumber;
    FxBool                firstDeviceDetected = FXFALSE;
    FxU32 deviceID = 0;
    FxU32 vendorID = 0;
    FxU32 headerType = 0;
    FxU32 baseAddress0 = 0;
    FxU32 command = 0;
    FxU32 classCode = 0;
    FxU32 maxFunctionNumber;
    int multi_fn = 0;
    
    putchar( '\n' );
    
    if ( !pciOpen() ) {
        puts( pciGetErrorString() );
        return -1;
    }
    
    for ( deviceNumber = 0; deviceNumber < MAX_PCI_DEVICES; deviceNumber++ ) {
        if ( pciDeviceExists( deviceNumber ) ) {
            if ( !firstDeviceDetected ) {                                       
                puts( "fn bus slot vendId  devId   baseAddr0   cmd     description" );
                puts( "-- --- ---- ------  ------  ----------  ------  -----------" );
                firstDeviceDetected = FXTRUE;
            }

            pciGetConfigData( PCI_HEADER_TYPE, deviceNumber, &headerType);
            pciGetConfigData( PCI_VENDOR_ID, deviceNumber, &vendorID );
            pciGetConfigData( PCI_DEVICE_ID, deviceNumber, &deviceID );
            if  (headerType & (1 << 7)) {
               maxFunctionNumber = 8; /* multifunction! */
               multi_fn = 1;
            } else {
               multi_fn = 0;
               if  ((vendorID == 0x121a) && (deviceID == 0x02)) {
                  maxFunctionNumber = 8; /* check for single board SLI */
               } else {
                  maxFunctionNumber = 1;
               }
            }

	    for (functionNumber = 0; functionNumber < maxFunctionNumber;
                      functionNumber++) {
               FxU32 t_deviceNumber = deviceNumber | (functionNumber <<13);

               pciGetConfigData( PCI_VENDOR_ID, t_deviceNumber, &vendorID );
               if (vendorID != 0xFFFF) {

                 if (!multi_fn && (functionNumber > 0)) {
                    printf("** VooDoo2 single board SLI pair:\n");
                    // command = 0x2;
                    // pciSetConfigData( PCI_COMMAND, t_deviceNumber, &command);
                 }
                 pciGetConfigData( PCI_DEVICE_ID, t_deviceNumber, &deviceID );
                 pciGetConfigData( PCI_BASE_ADDRESS_0, t_deviceNumber, 
                                   &baseAddress0 );
                 pciGetConfigData( PCI_COMMAND, t_deviceNumber, &command );
                 pciGetConfigData( PCI_CLASS_CODE, t_deviceNumber, &classCode );

                 printf( "%.02d  %.02d  %.02d  0x%.04lx  0x%.04lx  0x%.08x  0x%.04x  %.8s:%.22s\n", 
                    functionNumber, deviceNumber>>5, deviceNumber&0x1f, vendorID, deviceID, baseAddress0, command, 
                    pciGetVendorName( (FxU16)vendorID ), pciGetClassName( classCode, deviceID ) );


                }

             }


        }
    }
    
    if ( !pciClose() ) {
        puts( pciGetErrorString() );
        return -1;
    }
    
    if ( !firstDeviceDetected ) puts( "No PCI devices detected." );
    
    putchar( '\n' );
    
    puts( "Completed Successfully." );
    return 0;
}


int main(int argc, char **argv) {
   return (print_pci_space());
}

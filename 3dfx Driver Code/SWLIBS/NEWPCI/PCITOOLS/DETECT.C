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
** $Date: 10/11/00 7:41:04 PM$ 
**
*/

#include <stdio.h>
#include "3dfx.h"
#include "fxpci.h"

int main( void ) { 
    FxU32         deviceNumber;
    FxBool                firstDeviceDetected = FXFALSE;
    FxU32 deviceID = 0;
    FxU32 vendorID = 0;
    FxU32 baseAddress0 = 0;
    FxU32 command = 0;
    FxU32 classCode = 0;
    
    putchar( '\n' );
    
    if ( !pciOpen() ) {
        puts( pciGetErrorString() );
        return -1;
    }
    
    for ( deviceNumber = 0; deviceNumber < MAX_PCI_DEVICES; deviceNumber++ ) {
        if ( pciDeviceExists( deviceNumber ) ) {
            if ( !firstDeviceDetected ) {                                       
                puts( "bus slot vendId  devId   baseAddr0   cmd     description" );
                puts( "--- ---- ------  ------  ----------  ------  -----------" );
                firstDeviceDetected = FXTRUE;
            }
    
            pciGetConfigData( PCI_DEVICE_ID, deviceNumber, &deviceID );
            pciGetConfigData( PCI_VENDOR_ID, deviceNumber, &vendorID );
            pciGetConfigData( PCI_BASE_ADDRESS_0, deviceNumber, &baseAddress0 );
            pciGetConfigData( PCI_COMMAND, deviceNumber, &command );
            pciGetConfigData( PCI_CLASS_CODE, deviceNumber, &classCode );

            printf( " %.02d  %.02d  0x%.04lx  0x%.04lx  0x%.08x  0x%.04x  %.8s:%.25s\n", 
                    deviceNumber>>5, deviceNumber&0x1f, vendorID, deviceID, baseAddress0, command, 
                    pciGetVendorName( (FxU16)vendorID ), pciGetClassName( classCode, deviceID ) );
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

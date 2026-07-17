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
** $Revision: 2$ 
** $Date: 10/11/00 8:16:13 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <3dfx.h>
#include <fxpci.h>

int main( void ) {
    FxU32 physicalAddressOfFrameBuffer = 0xFF000000;
    FxU32 sizeOfFrameBuffer = 0x400000;    
    FxU32 linearAddressOfFrameBuffer = 0;

    const FxU32 fbWidth = 1280;
    const FxU32 fbHeight = 1024;
    const FxU8  saturated = 0xFF;
    FxU32 index;
    FxU8  *fbPointer;

    fprintf( stderr, "This software is going to draw a little line in the framebuffer\n" );
    fprintf( stderr, "of an ATI-GP Turbo 4M card.  If you don't understand the implications\n" );
    fprintf( stderr, "of this, CTRL-C NOW! Otherwise, press any key.\n" );
    getch();

	if ( !pciOpen() ) {
		puts( pciGetErrorString() );
		return -1;
	}

    if ( !pciMapPhysicalToLinear( &linearAddressOfFrameBuffer, 
                                   physicalAddressOfFrameBuffer,
                                  &sizeOfFrameBuffer ) ) {
        puts( pciGetErrorString() );
        return -1;
    }

    // Draw A Little White Line In the Corner of the frame buffer
    fbPointer = (FxU8 *)linearAddressOfFrameBuffer;

    for ( index = 0; index < 30; index++ ) {    
        *fbPointer++ = saturated;
        *fbPointer++ = saturated;
        *fbPointer++ = saturated;
        fbPointer += ( 3 * fbWidth ) + 3;
    }    

    fprintf( stderr, "press a key\n" );
    getch();

    pciUnmapPhysical( linearAddressOfFrameBuffer, sizeOfFrameBuffer );

	if ( !pciClose() ) {
		puts( pciGetErrorString() );
		return -1;
	}

	puts( "Test completed successfully." );

	return 0;
}

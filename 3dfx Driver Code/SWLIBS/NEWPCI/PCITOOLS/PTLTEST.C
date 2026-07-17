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
** $Revision: 4$ 
** $Date: 10/11/00 7:41:07 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef __linux__
#include <conio.h>
#else	/* defined(__linux__) */
#include <linutil.h>
#endif	/* defined(__linux__) */

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

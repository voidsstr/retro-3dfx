/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
*/

#include <stdlib.h>
#include <stdio.h>
#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include "fxpci.h"
#include "pcilib.h"
#include <fxdpmi.h>
#include <fxmemmap.h>

static char pciIdent[] = "@#% fxPCI for DOS";

FxBool pciInitializeDDio(void)
{
    pciIdent[0] = '@';		/* to prevent compiler warnings ONLY! */
    return(FXTRUE);
}
    
FX_EXPORT FxBool FX_CSTYLE
pciMapPhysicalToLinear( FxU32 *linear_addr, FxU32 physical_addr,
                       FxU32 *length ) 
{ 
    FxBool onWindows;

    if ( !pciLibraryInitialized ) {
	pciErrorCode = PCI_ERR_NOTOPEN;
	return FXFALSE;
    }
    
    /*  
    **  First, check to see if we're a DOS app under Windows, and if
    **  so, then check to see if there's already an app connected to
    **  the VXD.
    */
    DpmiCheckVxd((FxBool *) &onWindows, &pciVxdVer);

    if (onWindows) {
      if (BYTE1(pciVxdVer) != FX_MAJOR_VER || BYTE0(pciVxdVer) < FX_MINOR_VER) {
        pciErrorCode = PCI_ERR_WRONGVXD;
        return FXFALSE;
      }
      
      if (VXDREFCOUNT(pciVxdVer) > 0) {
        pciErrorCode = PCI_ERR_VXDINUSE;
        return FXFALSE;
      }
    }

    /* If we got here, it's OK to map the memory */
    *linear_addr = DpmiMapPhysicalToLinear( physical_addr, *length );
    return FXTRUE;
}

void pciUnmapPhysicalDD( FxU32 linear_addr, FxU32 length ) 
{
    DpmiUnmapMemory();
    return;
}

FX_EXPORT FxBool FX_CSTYLE
pciClose( void )
{
    if ( !pciLibraryInitialized ) {
        pciErrorCode = PCI_ERR_NOTOPEN2;
        return FXFALSE;
    }
    DpmiUnmapMemory();
    pciLibraryInitialized = FXFALSE;
    return FXTRUE;
}

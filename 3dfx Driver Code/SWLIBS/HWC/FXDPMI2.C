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
#include "gdebug.h"
#include "fxhwc.h"
#include <fxdpmi.h>
#include <fxmemmap.h>

static char pciIdent[] = "@#% fxPCI for DOS";

FxBool hwcInitializeDDio(void)
{
    pciIdent[0] = '@';		/* to prevent compiler warnings ONLY! */
    return(FXTRUE);
}
    
FX_EXPORT FxBool FX_CSTYLE
pciMapPhysicalToLinear( FxU32 *linear_addr, FxU32 physical_addr,
                       FxU32 *length ) 
{ 
    FxBool onWindows;

    if ( !hwcInfo.initialized ) {
        GDBG_ERROR("hwcMapPhysicalToLinear", "HWC library not initialized\n");
	return FXFALSE;
    }
    
    /*  
    **  First, check to see if we're a DOS app under Windows, and if
    **  so, then check to see if there's already an app connected to
    **  the VXD.
    */
    DpmiCheckVxd((FxBool *) &onWindows, &hwcInfo.pciVxdVer);

    if (onWindows) {
      if (BYTE1(hwcInfo.pciVxdVer) != FX_MAJOR_VER || BYTE0(hwcInfo.pciVxdVer) < FX_MINOR_VER) {
        GDBG_ERROR("hwcMapPhysicalToLinear", 
                   "Expected VxD version V%d.%d, got V%d.%d\n", 
                    FX_MAJOR_VER, FX_MINOR_VER,
                    BYTE1(hwcInfo.pciVxdVer), BYTE0(hwcInfo.pciVxdVer));
        return FXFALSE;
      }
      
      if (VXDREFCOUNT(hwcInfo.pciVxdVer) > 0) {
        GDBG_ERROR("hwcMapPhysicalToLinear", "Mutual exclusion prohibits this\n");
        return FXFALSE;
      }
    }

    /* If we got here, it's OK to map the memory */
    *linear_addr = DpmiMapPhysicalToLinear( physical_addr, *length );
    return FXTRUE;
}

void hwcUnmapPhysical( FxU32 linear_addr, FxU32 length ) 
{
    DpmiUnmapMemory();
    return;
}

FX_EXPORT FxBool FX_CSTYLE
hwcPCIShutdown( void )
{
    if ( !hwcInfo.initialized ) {
        GDBG_ERROR("hwcPCIShutdown", "HWC library not initialized\n");
        return FXFALSE;
    }
    DpmiUnmapMemory();
    return FXTRUE;
}

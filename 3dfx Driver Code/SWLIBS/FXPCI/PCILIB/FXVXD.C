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

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <fxmemmap.h>
#include <fxdpmi.h>
#include "fxpci.h"
#include "pcilib.h"


/* -------------------------------------------------- */
/* Memmap through Conrad's memmap vxd                 */

static char pciIdent[] = "@#% fxPCI for Windows '95";

HANDLE hMemmapFile;

FxBool pciInitializeDDio(void)
{
    if (pciIdent[0]);
    hMemmapFile = CreateFile("\\\\.\\FXMEMMAP.VXD", 0, 0, NULL, 0,
                           FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if ( hMemmapFile == INVALID_HANDLE_VALUE ) {
	pciErrorCode = PCI_ERR_MEMMAPVXD;
	return FXFALSE;
    }
    return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE
pciMapPhysicalToLinear( FxU32 *linear_addr, FxU32 physical_addr,
                       FxU32 *length ) 
{ 
    FxU32 nret;
    FxU32 Physical [2];         /* Physical address[0] & size[1] */
    FxU32 Linear [2];           /* Linear address[0] & size[1] */
    LPDWORD pPhysical = Physical;
    LPDWORD pLinear = Linear;

    Physical[0] = physical_addr;
    Physical[1] = *length;

    if ( !pciLibraryInitialized ) {
	pciErrorCode = PCI_ERR_NOTOPEN;
	return FXFALSE;
    }
    
    /*
     * Check version:
     * The policy is that major and minor versions must match, and
     * further that the reference count is less than or equal to one. 
     */
    DeviceIoControl(hMemmapFile, GETAPPVERSIONDWORD, NULL, 0, &pciVxdVer, 
                    sizeof(pciVxdVer), &nret, NULL);
    if (HIBYTE(pciVxdVer) != FX_MAJOR_VER || LOBYTE(pciVxdVer) < FX_MINOR_VER) {
      pciErrorCode = PCI_ERR_WRONGVXD;
      return FXFALSE;
    }

    if (VXDREFCOUNT(pciVxdVer) > 1) {
      pciErrorCode = PCI_ERR_VXDINUSE;
      return FXFALSE;
    }

    /* Map physical to linear */
    /* xxx - returns 0 in Linear if fails, but really should
       check return value, 0 is suceess, -1 is failure. */

#ifdef DIRECTX
    DeviceIoControl(hMemmapFile, GETLINEARADDR, 
                    &pPhysical, sizeof(pPhysical), 
                    &pLinear, sizeof(pLinear), 
                    &nret, NULL);
#else
    /* Stuff added to auto-switch passthru using fxmemmap */
    if (getenv("SST_DUALHEAD") == NULL) {
      DeviceIoControl(hMemmapFile, GETLINEARADDR_AUTO, 
                      &pPhysical, sizeof(pPhysical), 
                      &pLinear, sizeof(pLinear), 
                      &nret, NULL);
    } else {
      DeviceIoControl(hMemmapFile, GETLINEARADDR, 
                      &pPhysical, sizeof(pPhysical), 
                      &pLinear, sizeof(pLinear), 
                      &nret, NULL);
    }
#endif /* DIRECTX */


    *linear_addr = Linear[0];

    if ( nret == 0 ) {
      pciErrorCode = PCI_ERR_MEMMAP;
      return FXFALSE;
    }
    return FXTRUE;
}

void pciUnmapPhysicalDD( FxU32 linear_addr, FxU32 length ) 
{
    FxU32 nret;
    DeviceIoControl(hMemmapFile, DECREMENTMUTEX,
                    NULL, 0, NULL, 0,
                    &nret, NULL);
    return;
}

FX_EXPORT FxBool FX_CSTYLE
pciClose( void )
{
    if ( !pciLibraryInitialized ) {
        pciErrorCode = PCI_ERR_NOTOPEN2;
        return FXFALSE;
    }
    CloseHandle( hMemmapFile );
    pciLibraryInitialized = FXFALSE;
    return FXTRUE;
}

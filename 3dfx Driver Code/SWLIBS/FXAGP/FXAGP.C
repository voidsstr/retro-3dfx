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
** $Revision: 2$
** $Date: 10/11/00 7:37:03 PM$
*/

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <3dfx.h>

#include <fxagp.h>

static HANDLE gartVXDHandle = 0;
static char fxAGPErrorString[1024];
static FxU32 dwErrorCode;

FxBool
fxAGPInit() {
  /* Load and prepare to call  */
  gartVXDHandle = CreateFile("\\\\.\\3DFXGART.VXD", 0,0,0,
    CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);

  if ( gartVXDHandle == INVALID_HANDLE_VALUE ) {
    dwErrorCode = GetLastError();
    if ( dwErrorCode == ERROR_NOT_SUPPORTED ) {
      sprintf(fxAGPErrorString, "Unable to open VxD, \n device does not support DeviceIOCTL\n");
    }  else {
      sprintf(fxAGPErrorString, "Unable to open VxD, Error code: %lx\n", dwErrorCode);
    }
    return FXFALSE;
  }
  return FXTRUE;
} /* fxAGPInit */


FxBool
fxAGPReserve(FxU32 nPages, FxU32 *linearAddr, FxU32 *physAddr, FxU32 pciID) 
{
    FxU32
	cbBytesReturned,
	parmInfo[3],
	devNode,
	retInfo[2];
  
    // locate the devNode
    parmInfo[0] = pciID;
	
    if ( DeviceIoControl(gartVXDHandle, FXAGPCMLOCATE,
			 (LPVOID)parmInfo, sizeof(parmInfo),
			 (LPVOID)retInfo, sizeof(retInfo),
			 &cbBytesReturned, NULL) )
    {
	devNode = retInfo[0];
	if (devNode != 0)
	{
	    printf("located pciID 0x%08lx, devNode = 0x%08lx\n",
		   pciID, retInfo[0]);
	}
	else
	{
	    printf("did not locate pciID 0x%08lx\n", pciID);
	}
    }
    else
    {
	printf("did not locate pciID 0x%08lx\n", pciID);
    }

    parmInfo[0] = nPages;
    parmInfo[1] = devNode;

    if ( DeviceIoControl(gartVXDHandle, FXAGPGARTRESERVE, 
			 (LPVOID)parmInfo, sizeof(parmInfo),
			 (LPVOID)retInfo, sizeof(retInfo),
			 &cbBytesReturned, NULL) ) {
	*linearAddr = retInfo[0];
	*physAddr = retInfo[1];
//    GDBG_INFO(10, "return value from GartReserve: 0x%08lx\n", gartLinAddr);
//    GDBG_INFO(10, "AGP memory GART physical address: 0x%08lx\n",gartPhysAddr);
    } else {
	dwErrorCode = GetLastError();
	sprintf(fxAGPErrorString, "fxAGPReserve call: Something went wrong!\n");
	return FXFALSE;
    }

    return FXTRUE;

} /* fxAGPReserve */

FxBool
fxAGPCommit(FxU32 linearAddr, FxU32 offset, FxU32 nPages )
{
  FxU32
    cbBytesReturned,
    parmInfo[3],
    retInfo[2];
  
  parmInfo[0] = linearAddr;
  parmInfo[1] = offset;
  parmInfo[2] = nPages;

  if ( DeviceIoControl(gartVXDHandle, FXAGPGARTCOMMIT,
    (LPVOID)parmInfo, sizeof(parmInfo),
    (LPVOID)retInfo, sizeof(retInfo),
    &cbBytesReturned, NULL) ) {
  } else {
    dwErrorCode = GetLastError();
    sprintf(fxAGPErrorString, "fxAGPCommit call: Something went wrong!\n");
    return FXFALSE;
  }

  return FXTRUE;
  
} /* fxAGPCommit */

FxBool
fxAGPUncommit(FxU32 linearAddr, FxU32 offset, FxU32 nPages) 
{
  FxU32
    cbBytesReturned,
    parmInfo[3],
    retInfo[2];

  parmInfo[0] = linearAddr;
  parmInfo[1] = offset;
  parmInfo[2] = nPages;

  if ( DeviceIoControl(gartVXDHandle, FXAGPGARTUNCOMMIT,
    (LPVOID)parmInfo, sizeof(parmInfo),
    (LPVOID)retInfo, sizeof(retInfo),
    &cbBytesReturned, NULL) ) {
  } else {
    dwErrorCode = GetLastError();
    sprintf(fxAGPErrorString, "fxAGPUncommit call: Something went wrong!\n");
    return FXFALSE;
  }

  return FXTRUE;
  
} /* fxAGPUncommit */

FxBool
fxAGPFree(FxU32 linearAddr) 
{
  FxU32
    cbBytesReturned,
    parmInfo[3],
    retInfo[2];

  parmInfo[0] = linearAddr;

  if ( DeviceIoControl(gartVXDHandle, FXAGPGARTFREE,
    (LPVOID)parmInfo, sizeof(parmInfo),
    (LPVOID)retInfo, sizeof(retInfo),
    &cbBytesReturned, NULL) ) {
  } else {
    dwErrorCode = GetLastError();
    sprintf(fxAGPErrorString, "fxAGPFree call: Something went wrong freeing %x\n", linearAddr);
    return FXFALSE;
  }
  return FXTRUE;
} /* fxAGPFree */

void
fxAGPShutdown() 
{
  CloseHandle(gartVXDHandle);

} /* fxAGPShutdown */

char *
fxAGPGetErrorString()
{
  return fxAGPErrorString;
} /* fxAGPGetErrorString */

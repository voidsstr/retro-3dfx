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

/* ------------------------------------------------------------- */
/* Memmap and portio through Microsoft's sample mapmem miniport  */

#ifdef __WIN32__
#include <windows.h>
#include <winioctl.h>
#endif
#include <stddef.h>
#include <gpioctl.h>

static char pciIdent[] = "@#% fxPCI for Windows NT";

/* xxx The following two typedefs are here to avoid including
   miniport.h and ntddk.h from the NT DDK */

typedef enum _INTERFACE_TYPE
{
  Internal,
  Isa,
  Eisa,
  MicroChannel,
  TurboChannel,
  PCIBus,
  MaximumInterfaceType
} INTERFACE_TYPE, *PINTERFACE_TYPE;

typedef LARGE_INTEGER PHYSICAL_ADDRESS;

#include <mapmem.h>

static HANDLE hGpdFile;
static HANDLE hMapmemFile;

FxBool hwcInitializeDDio(void)
{
    if (pciIdent[0]);

    hGpdFile = CreateFile( "\\\\.\\GpdDev", GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL, OPEN_EXISTING, 0, NULL );
    if ( hGpdFile == INVALID_HANDLE_VALUE ) {
      GDBG_ERROR("hwcInitializeDDio", 
                 "Genport I/O initialization failure.\n");
      return FXFALSE;
    }

    hMapmemFile = CreateFile("\\\\.\\MAPMEM", GENERIC_READ | GENERIC_WRITE,
                           0, NULL, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if ( hMapmemFile == INVALID_HANDLE_VALUE ) {
      GDBG_ERROR("hwcInitializeDDio", 
                 "Mapmem driver initialization failure.\n");
        return FXFALSE;
    }    
    return FXTRUE;
}

FX_EXPORT FxU8 FX_CSTYLE
pioInByte ( unsigned short port )
{
  BOOL   IoctlResult;
  LONG   IoctlCode;
  ULONG  PortNumber;
  UCHAR  DataBuffer;
  ULONG  DataLength;
  ULONG  ReturnedLength;

  if (pciHwcCallbacks.pioInByte)
	DataBuffer = pciHwcCallbacks.pioInByte(port);
  if (!pciHwcCallbacks.doHW)
	return DataBuffer;

  IoctlCode = IOCTL_GPD_READ_PORT_UCHAR;
  PortNumber = port;
  DataLength = sizeof(DataBuffer);

  IoctlResult = DeviceIoControl( hGpdFile, IoctlCode, &PortNumber,
                                 sizeof(PortNumber), 
                                 &DataBuffer, DataLength,
                                 &ReturnedLength, NULL ); 

  if ( IoctlResult && ReturnedLength == DataLength )
    return DataBuffer;

  else
    return FXFALSE;
} /* pioInByte */

FX_EXPORT FxU16 FX_CSTYLE
pioInWord ( unsigned short port )
{
  BOOL   IoctlResult;
  LONG   IoctlCode;
  ULONG  PortNumber;
  USHORT DataBuffer;
  ULONG  DataLength;
  ULONG  ReturnedLength;

  if (pciHwcCallbacks.pioInWord)
	DataBuffer = pciHwcCallbacks.pioInWord(port);
  if (!pciHwcCallbacks.doHW)
	return DataBuffer;

  IoctlCode = IOCTL_GPD_READ_PORT_USHORT;
  PortNumber = port;
  DataLength = sizeof(DataBuffer);

  IoctlResult = DeviceIoControl( hGpdFile, IoctlCode, &PortNumber,
                                sizeof(PortNumber), 
                                &DataBuffer, DataLength,
                                &ReturnedLength, NULL ); 
  
  if ( IoctlResult && ReturnedLength == DataLength )
    return DataBuffer;

  else
    return FXFALSE;
} /* pioInWord */

FX_EXPORT FxU32 FX_CSTYLE
pioInLong ( unsigned short port )
{
  BOOL   IoctlResult;
  LONG   IoctlCode;
  ULONG  PortNumber;
  ULONG  DataBuffer;
  ULONG  DataLength;
  ULONG  ReturnedLength;

  if (pciHwcCallbacks.pioInLong)
	DataBuffer = pciHwcCallbacks.pioInLong(port);
  if (!pciHwcCallbacks.doHW)
	return DataBuffer;

  IoctlCode = IOCTL_GPD_READ_PORT_ULONG;
  PortNumber = port;
  DataLength = sizeof(DataBuffer);

  IoctlResult = DeviceIoControl( hGpdFile, IoctlCode, &PortNumber, sizeof(PortNumber),
                                 &DataBuffer, DataLength, &ReturnedLength, NULL );

  if ( IoctlResult && ReturnedLength == DataLength )
    return DataBuffer;

  else
    return FXFALSE;
} /* pioInLong */

FX_EXPORT FxBool FX_CSTYLE
pioOutByte ( unsigned short port, FxU8 data )
{
  BOOL                IoctlResult;
  LONG                IoctlCode;
  GENPORT_WRITE_INPUT InputBuffer;
  ULONG               DataLength;
  ULONG               ReturnedLength;

  if (pciHwcCallbacks.pioOutByte)
	pciHwcCallbacks.pioOutByte(port,data);
  if (!pciHwcCallbacks.doHW)
	return FXTRUE;

  IoctlCode = IOCTL_GPD_WRITE_PORT_UCHAR;
  InputBuffer.PortNumber = port;
  InputBuffer.CharData = (UCHAR) data;
  DataLength = offsetof(GENPORT_WRITE_INPUT, CharData) +
               sizeof(InputBuffer.CharData);

  IoctlResult = DeviceIoControl( hGpdFile, IoctlCode, &InputBuffer, DataLength,
                                 NULL, 0, &ReturnedLength, NULL );

  if ( IoctlResult )
    return FXTRUE;
  else
    return FXFALSE;
} /* pioOutByte */

FX_EXPORT FxBool FX_CSTYLE
pioOutWord ( unsigned short port, FxU16 data )
{
  BOOL                IoctlResult;
  LONG                IoctlCode;
  GENPORT_WRITE_INPUT InputBuffer;
  ULONG               DataLength;
  ULONG               ReturnedLength;

  if (pciHwcCallbacks.pioOutWord)
	pciHwcCallbacks.pioOutWord(port,data);
  if (!pciHwcCallbacks.doHW)
	return FXTRUE;

  IoctlCode = IOCTL_GPD_WRITE_PORT_USHORT;
  InputBuffer.PortNumber = port;
  InputBuffer.ShortData = (USHORT) data;
  DataLength = offsetof(GENPORT_WRITE_INPUT, ShortData) +
               sizeof(InputBuffer.ShortData);

  IoctlResult = DeviceIoControl( hGpdFile, IoctlCode, &InputBuffer, DataLength,
                                 NULL, 0, &ReturnedLength, NULL );

  if ( IoctlResult )
    return FXTRUE;
  else
    return FXFALSE;
} /* pioOutWord */

FX_EXPORT FxBool FX_CSTYLE
pioOutLong ( unsigned short port, FxU32 data )
{
  BOOL                IoctlResult;
  LONG                IoctlCode;
  GENPORT_WRITE_INPUT InputBuffer;
  ULONG               DataLength;
  ULONG               ReturnedLength;

  if (pciHwcCallbacks.pioOutLong)
	pciHwcCallbacks.pioOutLong(port,data);
  if (!pciHwcCallbacks.doHW)
	return FXTRUE;

  IoctlCode = IOCTL_GPD_WRITE_PORT_ULONG;
  InputBuffer.PortNumber = port;
  InputBuffer.LongData = (ULONG) data;
  DataLength = offsetof(GENPORT_WRITE_INPUT, LongData) +
               sizeof(InputBuffer.LongData);

  IoctlResult = DeviceIoControl( hGpdFile, IoctlCode, &InputBuffer, DataLength,
                                 NULL, 0, &ReturnedLength, NULL );

  if ( IoctlResult )
    return FXTRUE;
  else
    return FXFALSE;
} /* pioOutLong */


FX_EXPORT FxBool FX_CSTYLE
hwcMapPhysicalToLinear( FxU32 *linear_addr, FxU32 physical_addr, FxU32 *length ) 
{ 
    FxU32 cbReturned;
    PHYSICAL_MEMORY_INFO pmi;

    if ( !hwcInfo.initialized ) {
        GDBG_ERROR("hwcMapPhysicalToLinear", "HWC library not initialized\n");
        return FXFALSE;
    }
    
    pmi.InterfaceType       = PCIBus;
    pmi.BusNumber           = 0;
    pmi.BusAddress.HighPart = 0x00000000;
    pmi.BusAddress.LowPart  = physical_addr;
    pmi.AddressSpace        = 0;
    pmi.Length              = *length;

    /* Hack

       The hwc library only handles the first 16 available pci busses. 
       NT requires that you map physical memory with the correct associated
       pci bus number.  Our current pci library doesn't really allow the 
       calling layer to know what bus a device is on, so we will just try
       all 16 possible busses before we fail to map a board.  */
    while ( !DeviceIoControl( hMapmemFile,
                              (FxU32) IOCTL_MAPMEM_MAP_USER_PHYSICAL_MEMORY,
                              &pmi, sizeof(PHYSICAL_MEMORY_INFO),
                              linear_addr, sizeof(PVOID),
                              &cbReturned, NULL ) ) {
      pmi.BusNumber++;
      if ( pmi.BusNumber > 15 ) {
        GDBG_ERROR("hwcMapPhysicalToLinear", 
                   "Memmap returned an error trying to map memory.\n");
        return FXFALSE;
      }
    }
    return FXTRUE;
}

void hwcUnmapPhysical( FxU32 linear_addr, FxU32 length ) 
{
    FxU32 cbReturned;
  
    DeviceIoControl( hMapmemFile,
                  (FxU32) IOCTL_MAPMEM_UNMAP_USER_PHYSICAL_MEMORY,
                  &linear_addr, sizeof(PVOID),
                  NULL, 0,
                  &cbReturned, NULL );
    return;
}

FX_EXPORT FxBool FX_CSTYLE
hwcPCIShutdown( void )
{
    if ( !hwcInfo.initialized ) {
        GDBG_ERROR("hwcPCIShutdown", "HWC library not initialized\n");
        return FXFALSE;
    }
    CloseHandle( hGpdFile );
    CloseHandle( hMapmemFile );
    return FXTRUE;
}

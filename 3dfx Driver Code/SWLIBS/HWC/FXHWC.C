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
**
** $Revision: 12$
** $Date: 10/11/00 7:39:54 PM$
*/

#if defined(SST2_DRV) || defined(HWC_GDI)
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <fxhwc.h>
#include <init.h>
#include <time.h>
#include <fxos.h>
#include <fximg.h>
#include <streams.c>
#ifdef HWC_GDI
#include "guimv.h"
#endif
#ifdef HWC_TCL
#include "hwctcl.h"
#endif

extern FILE *gdbg_msgfile;	// GDBG info/error file

static void hwcSimVectorShutdown(HwcSimulator *hws);

FxBool hwcGUIInit( void );
char szMainClassName[] = "CsimClassMain";
char szViewClassName[] = "CsimClassView";

/* Allow gwhat to find info */
#define VERSIONSTR    "HwcCode " "$Revision: 12$" "\0"
static char codeIdent[] = "@#% " VERSIONSTR ;

HwcInfo hwcInfo;
static oneFile = FXTRUE;		// put all test vectors in 1 file
static traceIO = FXFALSE, traceSGRAM = FXFALSE, traceAGP = FXFALSE;
static char *ioFileName = NULL, *agpFileName = NULL, *sgramFileName = NULL;
static FxU32 ioDetail = 0, agpDetail = 0, sgramDetail = 0;
static char *traceFileName = NULL;
static FILE *traceFile = NULL;
#define OUTBUFF_SIZE 2048
static char outBuff[OUTBUFF_SIZE];
FxU32 outCount = 0;

#ifdef SST2_DRV
HANDLE hwcHeap;
#define HEAP_SHARED 0x04000000UL
#endif

/* Table of know devices, final entry determined by NULL in name field */

#ifdef HWC_BUILD_SST2
FxBool sst2HwcInitBoard(HwcContext *hwc);
#endif

#ifdef HWC_BUILD_H3
FxBool h3HwcInitBoard(HwcContext *hwc);
#endif

static HwcDeviceType deviceTypes[] = {
/* device name  device class   vendor ID      device Id   Init Board */
#ifdef HWC_BUILD_SST2
   { "SST-2",     HWC_SST2,   HWC_VID_3DFX, HWC_DID_SST2, sst2HwcInitBoard },
#endif
#ifdef HWC_BUILD_H3
   { "H3",        HWC_H3,     HWC_VID_3DFX, HWC_DID_H3,   h3HwcInitBoard },
#endif
   { NULL,          0,            0,             0,          NULL },
};

//CSIM inteface functions
#ifdef HWC_COSIM
extern "C" {
void Sv_writeStrVsFile( int streamNo, char *str );
void Sv_EnableStream( HwcFCStreamInfo *HwcStreamInfo);
int Sv_moduleCosimTst();
}
#else // define stubs so everything compiles
void 
Sv_writeStrVsFile( int streamNo, char *str )
{
    GDBG_ERROR("Sv_writeStrVsFile","tracing for co simulation not enabled\n");
}

void 
Sv_EnableStream( HwcFCStreamInfo *HwcStreamInfo)
{
    GDBG_ERROR("Sv_EnableStream","tracing for co simulation not enabled\n");
}
#endif

FX_EXPORT void FX_CSTYLE
hwcInvokeDebugger( void )
{
#ifdef __WIN32__
    hwcInfo.debug = FXFALSE;
    _asm { int 3 };
#else
    hwcInfo.debug = FXFALSE;
#endif
}



/* reset a context to its unitialized state */
void hwcResetContext( HwcContext *hwc )
{
  hwc->size = sizeof(HwcContext);
  hwc->virtAddr[0] = NULL;
  hwc->virtAddr[1] = NULL;
  hwc->physAddr[0] = 0;
  hwc->physAddr[1] = 0;
  hwc->virtPort = 0;
  hwc->physPort = 0;
  hwc->devNum = DEAD;
  hwc->fbiRevision = DEAD;
  hwc->fbiVideoWidth = DEAD;
  hwc->fbiVideoHeight = DEAD;
  hwc->fbiVideoRefresh = DEAD;
  hwc->fbiConfig = DEAD;
  hwc->fbiMemType = DEAD;
  hwc->fbiMemoryFifoEn = 0;
  hwc->memSizeInBytes = DEAD;
  hwc->agpSizeInBytes = DEAD;
}

void
hwcInitPixelFormats(void)
{
  FxI32 i;

  for ( i = 0; i < HWC_NUM_PIXEL_FORMATS; i++ ) {
    switch ( i ) {
    case HWC_PIXFMT_1BPP:
      hwcInfo.bitsPerPixel[i] = 1;
      break;
    case HWC_PIXFMT_TXCMP_4:
      hwcInfo.bitsPerPixel[i] = 4;
      break;
    case HWC_PIXFMT_P_8:
    case HWC_PIXFMT_P_8_6666:
    case HWC_PIXFMT_A_8:
    case HWC_PIXFMT_I_8:
    case HWC_PIXFMT_AI_44:
    case HWC_PIXFMT_RGB_332:
    case HWC_PIXFMT_YUV_422:
    case HWC_PIXFMT_UYV_422:
    case HWC_PIXFMT_YUV_411:
    case HWC_PIXFMT_YIQ_422:
    case HWC_PIXFMT_VIP_ANC:
      hwcInfo.bitsPerPixel[i] = 8;
      break;
    case HWC_PIXFMT_AP_88:
    case HWC_PIXFMT_AI_88:
    case HWC_PIXFMT_AYIQ_8422:
    case HWC_PIXFMT_RGB_565:
    case HWC_PIXFMT_ARGB_1555:
    case HWC_PIXFMT_ARGB_8332:
    case HWC_PIXFMT_ARGB_4444:
    case HWC_PIXFMT_FBCMP_16:
    case HWC_PIXFMT_AA_16:
      hwcInfo.bitsPerPixel[i] = 16;
      break;
    case HWC_PIXFMT_RGB_888:
    case HWC_PIXFMT_ARGB_8888:
    case HWC_PIXFMT_FBCMP_32:
    case HWC_PIXFMT_AA_32:
      hwcInfo.bitsPerPixel[i] = 32;
      break;
    }
  }
}

/*------------------------------------------------------------------------------
@func hwcSetSeed
@date 3/27/98
@arg FxU32 - seed - new seed value
@return void -
@imp mlwp
@key random
@sect Random Functions
@html
This function sets a new random number seed
@end
------------------------------------------------------------------------------*/
FX_EXPORT void FX_CSTYLE
hwcSetSeed(FxU32 seed)
{
    hwcInfo.randx = seed;
}

/*------------------------------------------------------------------------------
@func hwcGetSeed
@date 3/27/98
@return FxU32 - current random seed
@imp mlwp
@key random
@sect Random Functions
@html
This function returns the current random number seed
@end
------------------------------------------------------------------------------*/
FX_EXPORT FxU32 FX_CSTYLE
hwcGetSeed(void)
{
    return hwcInfo.randx;
}

/*------------------------------------------------------------------------------
@func hwcInt32Random
@date 4/30/99
@arg FxU32 - maxr - maximum value of random number
@return FxU32 - random number
@imp mlwp
@key random
@sect Random Functions
@html
This function generate a random unsigned 32 bit integer number r, 
where 0 <= r <= maxr
@end
------------------------------------------------------------------------------*/
FX_EXPORT FxU32 FX_CSTYLE
hwcInt32Random(FxU32 maxr)
{
    FxU32 n, retval;

    if (maxr > 0xFFFFFFF) {
        do {
            retval = hwcInt32Random(0xFFFF);
            retval |= hwcInt32Random(maxr >> 16) << 16;
        } while (retval > maxr);
        return retval;
    }
    for (n = 1; n < 32; n++)
        if (((unsigned) 1 << n) > maxr)
            break;
    do {
        hwcInfo.randx = hwcInfo.randx * 1103515245 + 12345;
        retval = (hwcInfo.randx & 0x7fffffff) >> (31 - n);
    } while (retval > maxr);
    return retval;
}

/*---------------------------------------------------------------------------
   The first initialization routine
   Environment variables:
   HWC_VIDEO - not yet supported 
   HWC_AGP_MEM - agp memory size in bytes
   HWC_POLL - how often to poll for input
   HWC_HW - use real hardware
   HWC_HSIM - use hardware simulator
   HWC_CSIM - use C simulator (on which device?)
   SST_DUALHEAD - don't switch pass thru
   GDBG_FILE - what file to send debug info to
   GDBG_LEVEL - what debug levels to capture
   SST_NODEVICEINFO - get device info from hardware or environment
   SST_FBICFG - board identification
   SST_TMUCFG - TMU identification
   SST_FBIMEM_SIZE - fbi memory size
   SST_TMUMEM_SIZE - tmu memory size
   SST_DEVICE_ID - device id
   SST_FBI_REV - FBI revision
   CSIM_RECIP -
   CSIM_FASTFILL
   CSIM_STATS
   HSIM_TREX_STANDALONE
  ---------------------------------------------------------------------------*/
FX_EXPORT HwcInfo * FX_CSTYLE
hwcInit( void )
{
  int i;
  char *s;

  GDBG_INFO(1,"hwcInit %s (Headers %s)\n", VERSIONSTR, HWC_H_REV);

#if !defined(FX_DLL_ENABLE)
  GDBG_INIT();	/* if static library, call this here instead of DllMain */
#endif

  if (hwcInfo.initialized)
    return &hwcInfo;

  if ( s =getenv("HWC_MEM_DEBUG")) {
      hwcInfo.memDebugLevel = atoi(s);
  } else {
      hwcInfo.memDebugLevel = 0;
  }

  hwcMemInit((hwcInfo.memDebugLevel > 0) ? HWC_MEM_CHECK:0);
 
  hwcInfo.errorCount = 0;

  hwcInfo.deviceTypes = deviceTypes;

  hwcInfo.fbiMemSizeMB = hwcGetEnvI32("SST_FBI_MEMSIZE", 16,"     default SST_FBI_MEMSIZE: %d\n");

  hwcInfo.agpSizeInBytes = HWC_MBYTE(hwcGetEnvI32("SST_AGP_MEMSIZE",8,"     default SST_AGP_MEMSIZE: %d\n"));
   
  for(i = 0; i < HWC_MAX_BOARDS; i++) {
    hwcResetContext( &hwcInfo.boardInfo[i] );
  }
  for(i = 0; i < HWC_MAX_PCI_DEVICES; i++) {
      hwcInfo.devices[i].hwc = NULL;
  }

  hwcInfo.gamma.r = hwcInfo.gamma.g = hwcInfo.gamma.b = 1.0f;
  hwcInfo.randx = 1; // default random seed

  if(hwcGetEnv(("HWC_RGAMMA"))) {
     hwcInfo.gamma.r = (FxFloat)atof(hwcGetEnv(("HWC_RGAMMA")));
  }
  if(hwcGetEnv(("HWC_GGAMMA"))) {
     hwcInfo.gamma.g = (FxFloat) atof(hwcGetEnv(("HWC_GGAMMA")));
  }
  if(hwcGetEnv(("HWC_BGAMMA"))) {
     hwcInfo.gamma.b = (FxFloat) atof(hwcGetEnv(("HWC_BGAMMA")));
  }
  if(hwcGetEnv(("HWC_GAMMA"))) {
     hwcInfo.gamma.r = (FxFloat) atof(hwcGetEnv(("HWC_GAMMA")));
     hwcInfo.gamma.g = hwcInfo.gamma.r;
     hwcInfo.gamma.b = hwcInfo.gamma.r;
  }
  

    hwcInfo.pfnInitSim = hwcInitSim;
    
    hwcInfo.pfnSetDebugLevel = gdbg_set_debuglevel;
    hwcInfo.pfnGetDebugLevel = gdbg_get_debuglevel;

    hwcInfo.pfnGdbgVprintf = gdbg_vprintf;
    hwcInfo.pfnGdbgPrintf = gdbg_printf;
    hwcInfo.pfnGdbgVinfo = NULL; // gdbg_vinfo;
    hwcInfo.pfnGdbgInfo = gdbg_info;
    hwcInfo.pfnGdbgVinfoMore = NULL; // gdbg_vinfo_more;
    hwcInfo.pfnGdbgInfoMore = gdbg_info_more;
    hwcInfo.pfnGdbgVerror = NULL; // gdbg_verror;
    hwcInfo.pfnGdbgError = gdbg_error;
    hwcInfo.pfnPrintRegisterFields = hwcPrintRegisterFields;

    hwcInfo.pfnStore32 = hwcStore32;
    hwcInfo.pfnLoad32 = hwcLoad32;
    hwcInfo.pfnRead32 = hwcRead32;
    hwcInfo.pfnWrite32 = hwcWrite32;
    hwcInfo.pfnWriteVStruct = hwcWriteVStruct;
    hwcInfo.pfnWriteVvector = NULL; // hwcWriteVvector;
    hwcInfo.pfnWriteVector = hwcWriteVector;

    hwcInfo.pfnMemInit = hwcMemInit;
    hwcInfo.pfnMemShutdown = hwcMemShutdown;
    hwcInfo.pfnInitMemRegion = _hwcInitMemRegion;
    hwcInfo.pfnAGPVirtToPhys = hwcAGPVirtToPhys;
    hwcInfo.pfnAGPMemAlloc = NULL; // hwcAGPMemAlloc;
    hwcInfo.pfnMemCheck = _hwcMemCheck;
    hwcInfo.pfnMemRealloc = _hwcMemRealloc;
    hwcInfo.pfnStrDup = _hwcStrDup;
    hwcInfo.pfnMemAlignedCalloc = _hwcMemAlignedCalloc;
    hwcInfo.pfnMemFree = _hwcMemFree;
    hwcInfo.pfnMemDerefFree = _hwcMemDerefFree;
    hwcInfo.pfnMemStats = hwcMemStats;



  hwcInfo.initialized = 1;

  /* Make Watcom happy */
#ifndef  __unix__
  codeIdent[0];
#endif
  hwcInfo.pollLimit = 10000;

#ifdef GDBG_INFO_ON
  hwcInfo.pollLimit /= 10;
#endif

#ifndef  __unix__
#endif
 if (!hwcPCIInit())
    return NULL;

  hwcInitPixelFormats();

  hwcGUIInit();

  return &hwcInfo;
}

/*-------------------------------------------------------------------
  Searches list of known devices for one matching the specified
  vendor and device id.
  -------------------------------------------------------------------*/

FX_EXPORT HwcDeviceType * FX_CSTYLE
hwcFindDeviceType(FxU32 vendorID, FxU32 deviceID) {
  HwcDeviceType *d;

  GDBG_INFO(1,"hwcDindDeviceType(0x%x, 0x%x)\n", vendorID, deviceID);

  for ( d = deviceTypes; d->name; d++ ) {
    if (( d->vendorID == vendorID ) && ( d->deviceID == deviceID ))
      return d;
  }
  return NULL;
}

FX_EXPORT FxBool FX_CSTYLE
hwcInitSim(HwcContext *hwc, HwcSimulator *hws) {
  hws->hwc = hwc;
  hws->environment.flushCount = 100;
  hws->environment.ieeeMath = FXFALSE;
  hws->environment.videoEnabled = FXTRUE;

  // Default SLI is disabled 
  hws->environment.sliConfig = 0;

  return FXTRUE;
}

/*-------------------------------------------------------------------
  find a simulator given the context and the name of the simulator we
  want.
  -------------------------------------------------------------------*/

FX_EXPORT HwcSimulator * FX_CSTYLE 
hwcFindSimulator(HwcContext *hwc, char *name)
{
  HwcSimulator *hws;

  for ( hws = hwc->hws; hws; hws = hws->next ) {
      if ( strcmp(hws->name, name) == 0 )
        return hws;
  }
  return NULL;
}

/*-------------------------------------------------------------------
  allocate a hardware context which is used in subsequent calls
  to refer to this device.
  -------------------------------------------------------------------*/

extern HwcContext *lastContext;

// TODO now we have exceeded the  256MB size that worked with the old find board
// scheme we need to add a new one.

FxU32
hwcFakeAddressGetBoard(FxU32 a)
{
    return lastContext->bn;
}

FxBool
hwcBadAddress(FxU32 a)
{
    return FXFALSE;
}

FX_EXPORT HwcContext * FX_CSTYLE
hwcAllocContext(HwcDevice *dev)
{
  HwcContext *hwc = NULL;
  HwcDeviceType *d;
  FxU32 bn;
  HwcSimulator *hws;

  GDBG_INFO(1,"hwcAllocContext(0x%lx)\n", dev);

#ifdef SST2_DRV
  hwcHeap = HeapCreate(HEAP_SHARED, 0x1ffff, 0);
#endif

  if ( dev->memSizeInBytes == 0 )
      dev->memSizeInBytes = HWC_MBYTE(hwcInfo.fbiMemSizeMB);

  if ( dev->agpSizeInBytes == 0 )
      dev->agpSizeInBytes = hwcInfo.agpSizeInBytes;

  /* find free context */

  for ( bn = 0; bn < HWC_MAX_BOARDS; bn++ ) {
    if ( hwcInfo.boardInfo[bn].state == HWC_CTX_UNALLOCATED ) {
      hwc = &hwcInfo.boardInfo[bn];
      break;
    }
  }

  if ( hwc == NULL ) {
    GDBG_ERROR("hwcAllocContext", "no more contexts\n");
    return NULL;
  }

  /* find device type */

  for ( d = deviceTypes; d->name; d++ ) {
    if (( d->vendorID == dev->vendorID ) && ( d->deviceID == dev->deviceID ))
        break;
  }

  if ( d->name == NULL ) {
    GDBG_ERROR("hwcAllocContext", "unknown device class 0x%lx 0x%lx\n", 
               dev->vendorID, dev->deviceID);
    return NULL;
  }

   /* initialize context, link back to device */

  memset( hwc, 0, sizeof( *hwc ) );
  hwc->bn = bn;
  hwc->devNum = dev-hwcInfo.devices;
  hwc->dev = dev;
  dev->hwc = hwc;
  hwc->devType = d;
  hwc->hwcInfo = &hwcInfo;
  hwc->memSizeInBytes = dev->memSizeInBytes;
  hwc->agpSizeInBytes = dev->agpSizeInBytes;

  if (getenv("HWC_NO_HW") ) { // disable any hardware that's out there
      hwc->dev->state &= ~HWC_HAS_HW;
  }

  if (!(* d->InitBoard)(hwc)) {
    return NULL;
  }

  for ( hws = hwc->hws; hws; hws = hws->next ) {
#ifdef HWC_TCL
    hwcTCLSimInit(hws);
#endif
#ifndef  __unix__
    if( hwc->hwcInfo->uiMethod )  
    {
      // Version 1 functions access our host framebuffer
      // If not in kernel mode and not using app windows. 
      hws->GUIKeepAlive = _hwcGUIKeepAlive1;
      hws->GUIChange = _hwcGUIChange1;
      hws->GUIUpdateRect = _hwcGUIUpdateRect1;
    }
    else 
#endif
    {
      hws->GUIKeepAlive = _hwcGUIKeepAlive;
      hws->GUIChange = _hwcGUIChange;
      hws->GUIUpdateRect = _hwcGUIUpdateRect;
    }
  }

  // delay AGP initialization until clients have had an opportunity to
  // set desired agp memory size
  if (!hwcInfo.agpInitialized)
      hwcAGPInit();

  // allocate ms buffer for diags

  hwcInitBuffer(&hwc->msBuffer, hwc, 640, 480, HWC_BUF_MS, HWC_PIXFMT_ARGB_8888);
  hwc->msBuffer.name = "ms buffer";
  hwc->stdBuffers[HWC_BUF_MS] = &hwc->msBuffer;

  hwc->state = HWC_CTX_ALLOCATED;
  lastContext = hwc; // XXX LOOOK hack for port io
  return hwc;
}

FX_EXPORT void FX_CSTYLE
hwcFreeSimulator(HwcSimulator *hws)
{
  GDBG_INFO(1,"hwcFreeSimulator(0x%lx)\n", hws);
}

FX_EXPORT void FX_CSTYLE
hwcFreeContext(HwcContext *hwc)
{
  GDBG_INFO(1,"hwcFreeContext(0x%lx)\n", hwc);

  if ( hwc->state == HWC_CTX_MAPPED )
    hwcUnmapBoard(hwc);
    
  if ( hwc->state == HWC_CTX_ALLOCATED ) {
    (* hwc->ShutdownBoard)(hwc);
  }

  hwc->dev->state = HWC_NONE;
  hwc->state = HWC_CTX_UNALLOCATED;
}

/*-------------------------------------------------------------------
  Detect recognized devices on the PCI bus and determine their
  configuration.
  -------------------------------------------------------------------*/

FX_EXPORT FxU32 FX_CSTYLE
hwcEnumHardware( HwcHWEnumCallback *cb )
{
  FxU32  devNum, count = 0;
  HwcDevice *dev;

  GDBG_INFO(1,"hwcEnumHardware(0x%lx)\n", cb);

  for ( devNum = 0; devNum < hwcInfo.maxDevices; devNum++ ) {
    dev = &hwcInfo.devices[devNum];
    if ( cb && dev->state ) {
      ( * cb )(dev);
      count++;
    }
  }

  return count;
}

/*---------------------------------------------------------------------------
   Map a board
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcMapBoard( HwcContext *hwc)
{
  HwcSimulator *hws;

  GDBG_INFO(1,"hwcMapBoard(0x%lx)\n", hwc);

  if ( hwc->MapBoard(hwc)) {
    GDBG_INFO(1,"hwcMapBoard board (0x%lx) mapped\n",hwc);
    GDBG_INFO(2,"hwcMapBoard done,  deviceNumber:0x%x(%d) boards=%d\n",
              hwc->devNum, hwc->devNum, hwc->bn);
    hwc->state = HWC_CTX_MAPPED;
  } else {
    GDBG_ERROR("hwcMapBoard", "Map failed\n");
    return FXFALSE;
  }

  /* open up a CSIM main control window */
  if( !(hwc->hwcInfo->uiMethod) ) {
    for ( hws = hwc->hws; hws; hws = hws->next ) {
#ifdef HWC_GDI
      if (hws->environment.videoEnabled && !hwcGDIGUIOpen( hws )) {
        hws->environment.videoEnabled = 0;
        return FXFALSE;
      }
#endif
#ifdef HWC_TCL
      if (hws->environment.videoEnabled && !hwcTCLGUIOpen( hws )) {
        hws->environment.videoEnabled = 0;
        return FXFALSE;
      }
#endif
    }
  }

  hwcInitGammaRGB(hwc, hwcInfo.gamma.r, hwcInfo.gamma.g, hwcInfo.gamma.b );

  return FXTRUE;
}

/*---------------------------------------------------------------------------
    Initialize all devices registers
  ---------------------------------------------------------------------------*/
FxBool FX_EXPORT FX_CSTYLE
hwcInitRegisters( HwcContext *hwc )
{
  int save120 = GDBG_GET_DEBUGLEVEL(120);
  FxBool res;

  GDBG_INFO(1,"hwcInitRegisters(0x%x)\n",hwc);

  if(!hwc || (hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  /* shut up the init code for now */
  GDBG_SET_DEBUGLEVEL(120,GDBG_GET_DEBUGLEVEL(320));

  res = (* hwc->InitRegisters)(hwc);

  GDBG_SET_DEBUGLEVEL(120,save120);	/* restore level 120 debug */
  return res;
}

/*---------------------------------------------------------------------------
    Initialize 2d registers
  ---------------------------------------------------------------------------*/
FxBool FX_EXPORT FX_CSTYLE
hwcInitGuiRegisters( HwcContext *hwc)
{
  GDBG_INFO(2,"hwcInitGuiRegisters(0x%x)\n",hwc);

  if(!hwc || (hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);
  
  return (* hwc->InitGuiRegisters)(hwc);
}

/*---------------------------------------------------------------------------
    Initialize 3d registers
  ---------------------------------------------------------------------------*/
FxBool FX_EXPORT FX_CSTYLE
hwcInitRenderingRegisters( HwcContext *hwc )
{
  GDBG_INFO(2,"hwcInitRenderingRegisters(0x%x)\n",hwc);

  if(!hwc || (hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  return (* hwc->InitRenderingRegisters)(hwc);
}

FxBool FX_EXPORT FX_CSTYLE
hwcStall( HwcContext *hwc, FxU32 nClocks)
{
  HwcSimulator *hws;

  GDBG_INFO(2,"hwcStall(0x%x, %d)\n",hwc, nClocks);

  if(!hwc || (hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  /* stall the simulators */

  for ( hws = hwc->hws; hws; hws = hws->next ) {
    if (hws->Stall && !(* hws->Stall)(hws, nClocks)) {
      GDBG_ERROR("sst2HwcMapBoard","Could not stall simulator %s\n", hws->name);
    }
  }

  return FXTRUE;
}

FxBool FX_EXPORT FX_CSTYLE
hwcUnmapBoard( HwcContext *hwc)
{
  HwcSimulator *hws;

  GDBG_INFO(2,"hwcUnmapBoard(0x%x)\n",hwc);

  if(!hwc || (hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  /* unmap the simulators */

  for ( hws = hwc->hws; hws; hws = hws->next ) {
    if (hws->Unmap && !(* hws->Unmap)(hws)) {
      GDBG_ERROR("sst2HwcMapBoard","Could not unmap simulator %s\n", hws->name);
    }
  }

  /* finish writing vectors */

  if (hwc->streamSim) {
      hwcSimVectorShutdown(hwc->streamSim);
  }

#ifdef HWC_COSIM
  if ((hwc->dev->state &  HWC_HAS_HSIM ) || Sv_moduleCosimTst()) {
      if ( hwcInfo.errorCount == 0 )
           TESTBENCH_PASS();
      else TESTBENCH_FAIL(); 
  }
#endif

  if( !hwc->hwcInfo->uiMethod) {
    for ( hws = hwc->hws; hws; hws = hws->next ) {
      hwcGUIUnmapBoard(hws);
    }
  }

  if(!hwc || (hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  hwc->state = HWC_CTX_ALLOCATED;

  if (! (* hwc->UnmapBoard)(hwc))
     return FXFALSE;

  return FXTRUE;
}

void FX_EXPORT FX_CSTYLE
hwcShutdown( void )
{
  FxU32 bn;
  HwcContext *hwc;

  hwcGUIShutdown();

  GDBG_INFO(1,"hwcShutdown()\n");
  for (bn = 0; bn < HWC_MAX_BOARDS; bn++) {
    hwc = &hwcInfo.boardInfo[bn];
    if ( hwc->state != HWC_CTX_UNALLOCATED )
      hwcFreeContext(hwc);
  }

#ifndef  __unix__
  hwcPCIShutdown();
#endif

  hwcMemShutdown(); 
}

/*
** hwcInitUSWC
**
** Sets up memory caching on P6 systems.
**
*/
FX_EXPORT FxBool FX_CSTYLE 
hwcInitUSWC(HwcContext *hwc)
{
  GDBG_INFO(1,"hwcInitUSWC(0x%x)\n", hwc);
#if ! defined( __unix__ ) && ! defined( KERNEL )
  {
  FxBool res;
  FxU32  physAddr;
  static FxU32  mtrr;

  if(!hwc || (hwc->state != HWC_CTX_MAPPED))
    return FXFALSE;

  physAddr = hwc->physAddr[1];

  /* For some reason, there sometimes is a 008 at the end of the
  physical address, so mask that puppy RTF out */
  physAddr &= 0xfffff000;

  /* Set up USWC MTRR */
  res = hwcFindMTRRMatch(physAddr, hwc->memLength[1],
                         HWC_MEM_TYPE_WRITE_COMBINING, &mtrr);

  if (!res)
    res = hwcFindFreeMTRR(&mtrr);
  else
    return FXTRUE;		/* It's already there.  We're done. */
  
  if (res)
    hwcSetMTRR(mtrr, physAddr, hwc->memLength[1], HWC_MEM_TYPE_WRITE_COMBINING );
  }
#endif
  return FXTRUE;
} /* hwcInitUSWC */

static void
hwcStreamPrintf( const char *format, ...)
{
  va_list args;
  int i;
 
  va_start( args, format );
  i = vsprintf( outBuff+outCount, format, args );
  va_end( args );
 
  if ( i >= 0 )
    outCount += i;
  else {
    GDBG_ERROR("hwcStreamPrintf","Error writing vector\n");
    return;
   }
 
   if ( outCount >= OUTBUFF_SIZE ) {
    GDBG_ERROR("hwcStreamPrintf","Maximum line length exceeded\n");
    return;
   }
}

static long byteOffset = 0;
extern FxBool UseDebugString;

static void
hwcStreamFlush( HwcSimulator *hws, FxU32 id)
{
  if ((!hws->vecInfo.initialized) || ( outCount == 0))
    return;
  
  outBuff[outCount] = 0;
 
  if ( hws->vecInfo.useHSIM ) {
       Sv_writeStrVsFile( id, outBuff );
#if defined(__WIN32__)
  } else if ( UseDebugString ) {
      OutputDebugString(outBuff);
#endif
  } else {
    void *stream = hws->vecInfo.module[id].stream;
 
    if ( byteOffset ) {
      long offset = ftell((FILE*) stream); 
      if ( offset >= byteOffset ) {
         printf("// got to stream breakpoint\n");
      }
    }

    fprintf((FILE*) stream, outBuff);
  }
  outCount = 0;
}

FX_EXPORT FxBool FX_CSTYLE 
hwcSimVectorInit(HwcSimulator *hws, HwcTestDescription *test ) 
{
  FxU32 i;
  void *stream;
  HwcStreamElement **ppStream;
  FxI32 idx;

  if (hws->vecInfo.initialized) {
     GDBG_ERROR("hwcSimVectorInit","vector capture already initialized\n");
     return FXFALSE;
  }
  hws->vecInfo.control = test->ctrl;
  hws->vecInfo.initialized = FXTRUE;
  hws->vecInfo.nextVector = 1;
  hws->vecInfo.type = test->type;

  for ( i = 0; i < hws->vecInfo.numModules; i++ ) {
    hws->vecInfo.module[i].stream = NULL;
    hws->vecInfo.module[i].detail = test->detail;
    hws->vecInfo.module[i].enabled = FXFALSE;
    hws->vecInfo.module[i].nextVector = 1;
  }

  hws->vecInfo.initialized = hwcSimCaptureVector(hws, HWC_VEC_GLOBAL, 
                                                 test->detail, 
                                traceFileName ? traceFileName : test->fileName, -1);

  if ( traceIO )
      hwcSimCaptureVector(hws, HWC_VEC_IO, ioDetail, ioFileName, -1);

  if ( traceSGRAM )
      hwcSimCaptureVector(hws, HWC_VEC_MEM_SGRAM, sgramDetail, sgramFileName, -1);

  if ( traceAGP )
      hwcSimCaptureVector(hws, HWC_VEC_MEM_AGP, agpDetail, agpFileName, -1);

  hws->vecInfo.module[HWC_VEC_GLOBAL].name = "global";
  hws->vecInfo.module[HWC_VEC_IO].name = "io";
  hws->vecInfo.module[HWC_VEC_MEM_SGRAM].name = "sgram.ram";
  hws->vecInfo.module[HWC_VEC_MEM_AGP].name = "agp.ram";
  assert(NUM_PREDEFINED_VECTORS==4);

  for ( ppStream = streams, idx = NUM_PREDEFINED_VECTORS; *ppStream; ppStream++, idx++ ) {
    HwcStreamElement *pStream = *ppStream;

	if (pStream->name) {
        hws->vecInfo.module[idx].name = pStream->name;
    } else GDBG_ERROR("hwcSimVectorInit","stream without name\n");

	if (pStream->generate) {
		hwcSimCaptureVector(hws, idx, pStream->detail, pStream->fileName, pStream->breakPoint);
	}
  }

  stream = hws->vecInfo.module[0].stream;

  if ( hws->vecInfo.useHSIM ) {
    HwcFCStreamInfo sInfo;
 
    sInfo.maxStreams = hws->vecInfo.numModules;
    sInfo.stream = hws->vecInfo.module;
    Sv_EnableStream( &sInfo );
    hws->vecInfo.control &= ~HWC_VECCTL_ONE_FILE;
  } else {
    fprintf((FILE*) stream, "sim: %s, version %s\n", hws->name, hws->version);
    fprintf((FILE*) stream, "test name: %s\n", test->name ? test->name : "unknown");
    fprintf((FILE*) stream, "test version: %s\n", test->version ? test->version : "unknown");
    fprintf((FILE*) stream, "test author: %s\n", test->author ? test->author : "unknown");
    fprintf((FILE*) stream, "test description: %s\n", test->description ? test->description :"unknown");
    fprintf((FILE*) stream, "****************************************\n");
  
    printAllModules((FILE *)stream, hws);
  
    fprintf((FILE*) stream, "****************************************\n");
  }

  return hws->vecInfo.initialized;
}

FX_EXPORT FxBool FX_CSTYLE 
hwcVectorInit(HwcContext *hwc, HwcTestDescription *test ) {
  return hwcSimVectorInit(hwc->hws, test);
}

static void
hwcSimVectorShutdown(HwcSimulator *hws) 
{
  FxU32 i;

  if (!hws->vecInfo.initialized)
    return;

  if ( hws->vecInfo.useHSIM ) {
    for ( i = 0; i < hws->vecInfo.numModules; i++ ) {
      if ( hws->vecInfo.module[i].enabled ) {
        sprintf(outBuff, "%d #EOS\n", hws->vecInfo.nextVector++);
        Sv_writeStrVsFile( i, outBuff);
      }
    }
  } else {
    if ( hws->vecInfo.control & HWC_VECCTL_ONE_FILE ) {
      FILE *f = (FILE *)hws->vecInfo.module[0].stream;
      if (( f != gdbg_msgfile ) && ( f != stdout ))
          fclose(f);
    } else {
      for ( i = 0; i < hws->vecInfo.numModules; i++ ) {
        if ( hws->vecInfo.module[i].enabled ) {
          fclose((FILE *)hws->vecInfo.module[i].stream);
        }
      }
    }
  }

  for ( i = 0; i < hws->vecInfo.numModules; i++ ) {
    hws->vecInfo.module[i].stream = NULL;
  }
}

#ifdef  __unix__
extern "C" int _stricmp( char *s1, char *s2 );
int 
_stricmp( char *s1, char *s2 )
{
   for ( ; tolower(*s1) == tolower(*s2); ++s1, ++s2 ) {
     if ( *s1 == '\0' )
         return (0);
    }
    return ( tolower(*s1) < tolower(*s2) ? -1 : +1 );
}
#endif

FX_EXPORT FxBool FX_CSTYLE 
hwcSimCaptureVector(HwcSimulator *hws, FxU32 id, FxU32 detail, char *name, FxI32 breakPoint)
{
  char fileName[40];
  FILE *fp;

  if (!hws->vecInfo.initialized) {
     GDBG_ERROR("hwcSimCaptureVector","vector capture not initialized\n");
     return FXFALSE;
  }

  if ( id >= hws->vecInfo.numModules ) {
    GDBG_ERROR("hwcCaptureVector","unknown vector id\n");
    return FXFALSE;
  }

  if ( hws->vecInfo.module[id].enabled ) {
    GDBG_ERROR("hwcCaptureVector","vector %d allready being captured\n", id);
     return FXFALSE;
  }

  if ( name == NULL ) {
    hws->vecInfo.module[id].stream = traceFile;
  } else if ( !_stricmp(name, "gdbg")) {
    hws->vecInfo.module[id].stream = traceFile = gdbg_msgfile;
  } else if ( !_stricmp(name, "stdout")) {
    hws->vecInfo.module[id].stream = traceFile = stdout;
  } else if ( !_stricmp(name, "hsim")) {
    hws->vecInfo.useHSIM = FXTRUE;
  } else if (( hws->vecInfo.control & HWC_VECCTL_ONE_FILE ) && traceFile ) {
    hws->vecInfo.module[id].stream = traceFile;
  } else {
    strcpy(fileName, name);
    strcat(fileName, ".tv");
  
    fp = fopen(fileName, "w");
    if (fp == NULL) {
      GDBG_ERROR("hwcCaptureVector",
                 "could not open vector file '%s'\n", fileName);
       return FXFALSE;
    }
 
    hws->vecInfo.module[id].stream = fp;
    if ( traceFile == NULL ) 
        traceFile = fp;
  } 

  hws->vecInfo.module[id].detail = detail;
  hws->vecInfo.module[id].enabled = FXTRUE;
  hws->vecInfo.module[id].breakPoint = breakPoint;

  return FXTRUE;
}

FX_EXPORT FxBool FX_CSTYLE 
hwcCaptureVector(HwcContext *hwc, FxU32 id, FxU32 detail, char *name, FxI32 breakPoint)
{
    return hwcSimCaptureVector(hwc->hws, id, detail, name, breakPoint);
}

FX_EXPORT FxBool FX_CSTYLE 
hwcSetVector(char *name, FxBool trace, FxU32 detail, char *fileName, FxI32 breakPoint)
{
    TestVectorGroup *pGroup;
    HwcModuleElement *pModule;
	HwcStreamElement **ppStream;
	FxI32 idx;

    // see if this is memory

	if (!_stricmp(name, "mem")) {
        hwcSetVector("sgram", trace, detail, fileName, breakPoint);
        hwcSetVector("agp", trace, detail, fileName, breakPoint);
		return FXTRUE;
    }

	if (!_stricmp(name, "agp")) {
        traceAGP = FXTRUE;
        agpDetail = detail;
		if (trace && (fileName != NULL)) {
			agpFileName = strdup(fileName);
			if (traceFileName) {
				if (_stricmp(traceFileName, fileName))
					oneFile = FXFALSE;
			} else {
				traceFileName = agpFileName;
			}
		}
		return FXTRUE;
	}

	if (!_stricmp(name, "sgram")) {
        traceSGRAM = FXTRUE;
        sgramDetail = detail;
		if (trace && (fileName != NULL)) {
			sgramFileName = strdup(fileName);
			if (traceFileName) {
				if (_stricmp(traceFileName, fileName))
					oneFile = FXFALSE;
			} else {
				traceFileName = sgramFileName;
			}
		}
		return FXTRUE;
	}

	if (!_stricmp(name, "io")) {
        traceIO = FXTRUE;
        ioDetail = detail;
		if (trace && (fileName != NULL)) {
			ioFileName = strdup(fileName);
			if (traceFileName) {
				if (_stricmp(traceFileName, fileName))
					oneFile = FXFALSE;
			} else {
				traceFileName = ioFileName;
			}
		}
		return FXTRUE;
	}

	// see if name is a group

	for (pGroup = vGroups; pGroup->name != NULL; pGroup++) {
		if (!_stricmp(name, pGroup->name)) {
			char **ids;

			for (ids = pGroup->ids; *ids != NULL; ids++) {
				hwcSetVector(*ids, trace, detail, fileName, breakPoint);
			}

			return FXTRUE;
		}
	}

	// see if name is a module

    if ( (pModule = findModule(name)) != NULL) {
        for ( ; pModule->name; pModule++ ) {
            switch ( pModule->type ) {
            case HWC_MODULE:
                break;
            case HWC_INPUT_STREAM:
            case HWC_OUTPUT_STREAM:
				hwcSetVector(pModule->name, trace, detail, fileName, breakPoint);
                break;
            case HWC_MEMORY_STREAM:
                if( !_stricmp("agp", pModule->name)) {
                    traceAGP = FXTRUE;
                    agpDetail = detail;
                }
                if( !_stricmp("sgram", pModule->name)) {
                    traceSGRAM = FXTRUE;
                    sgramDetail = detail;
                }
                break;
            }
        }

        return FXTRUE;
    }

	// see if name is an individual trace vector

    for ( ppStream = streams, idx = NUM_PREDEFINED_VECTORS; *ppStream; ppStream++, idx++ ) {
	    HwcStreamElement *pStream = *ppStream;
        
		if (pStream->name && !_stricmp(name, pStream->name)) {
            // see if this is an alias
            if ( pStream->type ) {
                return hwcSetVector((char *)pStream->type, trace, detail, fileName, breakPoint);
            }
            pStream->generate = trace;
            pStream->breakPoint = breakPoint;
            pStream->detail = detail;
			if (trace && (fileName != NULL)) {
				pStream->fileName = strdup(fileName);
				if (traceFileName) {
					if (_stricmp(traceFileName, fileName))
						oneFile = FXFALSE;
				} else {
					traceFileName = pStream->fileName;
				}
			}
			return FXTRUE;
		}
	}
	GDBG_ERROR("setVector", "Unknown vector %s\n", name);

	return FXFALSE;
}

static char *
printStreamStruct(HwcSimulator *hws, FxU32 id, HwcStreamElement *pStream, char *p) 
{
    FxU32 i;
    HwcStreamElement *pElem = pStream, *pChild;
    pElem++; // skip header

    for ( ; pElem->name; pElem++ ) {
        for ( i = 0; i < pElem->count; i++ ) {
            switch ( HWC_PARAM_TYPE(pElem->type) ) {
            case HWC_U_PARAM:
            case HWC_I_PARAM:
                {
                    FxU32 j, numWords = (pElem->size+31)>>5;
                    FxU32 *pval = (FxU32 *)p;
                    
                    p += numWords*sizeof(FxU32);
                    // if we have multiple words write them out from msb to lsb, word[0] has lsb's
                    for ( j = numWords; j > 0; j-- ) {
                        FxU32 val = pval[j-1];
                        if ( j == numWords) { // trim of any excess bits in leading word
                            FxU32 width = ( pElem->size > 32 ) ? ( pElem->size % 32 ) : pElem->size;
			    //map width of 0 to 32
			    width = (width)? width: 32;
                            hwcStreamPrintf(" 0x%lx", val&SST_MASK(width));
                        } else {
                            hwcStreamPrintf(" 0x%lx", val);
                        }
                    } 
                }
                break;
            case HWC_S_PARAM:
		        pChild = (HwcStreamElement *)(pElem->size);
                p = printStreamStruct(hws, id, pChild, p);
                break;
            }
            if (( pElem->count > 1 ) && (( i == (pElem->count-1)) && ( pElem[1].name == NULL )))
                continue;
       
            if ( pElem->type & HWC_NEWLINE ) {
                hwcStreamPrintf(CONTINUE_STRING);
                hwcStreamFlush(hws, id);
            } else if ( pElem->type & HWC_SPACES ) {
                hwcStreamPrintf(SPACES_STRING);
            }
        }
    }
    return p;
}

static char *
printStreamStructDebug(HwcSimulator *hws, FxU32 id, HwcStreamElement *pStream, char *p, char *prefix, char *suffix ) 
{
    char b[200];
    FxU32 i;
    HwcStreamElement *pElem = pStream, *pChild;
    pElem++; // skip header

    for ( ; pElem->name; pElem++ ) {
        for ( i = 0; i < pElem->count; i++ ) {
            switch ( HWC_PARAM_TYPE(pElem->type) ) {
            case HWC_U_PARAM:
            case HWC_I_PARAM:
                {
                    FxU32 j, numWords = (pElem->size+31)>>5;
                    FxU32 *pval = (FxU32 *)p;
                    
                    p += numWords*sizeof(FxU32);
                    // if we have multiple words write them out from msb to lsb, word[0] has lsb's
                    if ( pElem->count > 1 )
                         gdbg_info_more(0, " %s%s%ld%s ", prefix, pElem->name, i,suffix);
                    else gdbg_info_more(0, " %s%s%s ", prefix, pElem->name, suffix );
                    for ( j = numWords; j > 0; j-- ) {
                        FxU32 val = pval[j-1];
                        if ( j == numWords) { // trim of any excess bits in leading word
                            FxU32 width = ( pElem->size > 32 ) ? ( pElem->size % 32 ) : pElem->size;
			                //map width of 0 to 32
			                width = (width)? width: 32;
                            // gdbg_info_more(0, " 0x%lx", val&SST_MASK(width));
                            gdbg_info_more(0, " 0x%lx", val);
                        } else {
                            gdbg_info_more(0, " 0x%lx", val);
                        }
                    } 
                }
                break;
            case HWC_S_PARAM:
		        pChild = (HwcStreamElement *)(pElem->size);
                if ( pElem->count > 1 ) {
                    if ( pElem->type & HWC_OPT_SUFFIX ) {
                        sprintf(b, "_%s%s%ld", pElem->name, suffix, i);
                        p = printStreamStructDebug(hws, id, pChild, p, prefix, b);
                    } else {
                        sprintf(b, "%s%s%ld_", prefix, pElem->name, i);
                        p = printStreamStructDebug(hws, id, pChild, p, b, suffix);
                    }
                } else {
                    if ( pElem->type & HWC_OPT_SUFFIX ) {
                        sprintf(b, "_%s%s", pElem->name, suffix);
                        p = printStreamStructDebug(hws, id, pChild, p, prefix, b);
                    } else {
                        sprintf(b, "%s%s_", prefix, pElem->name);
                        p = printStreamStructDebug(hws, id, pChild, p, b, suffix);
                    }
                }
                break;
            }
            if (( pElem->count > 1 ) && (( i == (pElem->count-1)) && ( pElem[1].name == NULL )))
                continue;
       
            if ( pElem->type & HWC_NEWLINE ) {
                gdbg_info_more(0, "\n//            ");
            } else if ( pElem->type & HWC_SPACES ) {
                gdbg_info_more(0, SPACES_STRING);
            }
        }
    }
    return p;
}

FX_EXPORT void FX_CSTYLE 
hwcWriteVStruct(HwcSimulator *hws, FxU32 id, void *p)
{
    HwcTransInfo *tInfo;
    HwcStreamElement *pStream = streams[id-NUM_PREDEFINED_VECTORS];

    if ( hwcInfo.memDebugLevel > 1 )
        hwcMemCheck();

    if (!hws->vecInfo.initialized)
       return;

    // check if we need to ouput vector

    if ( !hws->vecInfo.module[id].enabled  )
       return;

    if (( hws->vecInfo.module[id].breakPoint >= 0 ) && 
        ( hws->vecInfo.module[id].nextVector == (FxU32)hws->vecInfo.module[id].breakPoint  )) {
        hwcInvokeDebugger();
    }

    if ( pStream->detail > 0 ) {
        gdbg_info(0, " %d %s ", hws->vecInfo.module[id].nextVector, pStream->name);
        printStreamStructDebug(hws, id, pStream, (char *)p, pStream->description ? pStream->description :"", "");
        gdbg_info_more(0, "\n");
    }

    // print the vector number if we are writing to multiple files
    if ( !(hws->vecInfo.control & HWC_VECCTL_ONE_FILE) )
         hwcStreamPrintf("%05ld ", hws->vecInfo.nextVector);

    // print the stream name
    hwcStreamPrintf("%s ", pStream->name);

    // print the transaction time if we are running psim
    if ( hws->GetTransInfo && (( tInfo = (* hws->GetTransInfo)(hws)) != NULL )) {
        hwcStreamPrintf("0x%08lx 0x%08lx ", tInfo->tValid, tInfo->tTaken );
    }

    // print the stream data
    printStreamStruct(hws, id, pStream, (char *)p);
    hwcStreamPrintf("\n");
    hwcStreamFlush(hws, id);

    hws->vecInfo.module[id].nextVector++; 
    hws->vecInfo.nextVector++;
}

FX_EXPORT void FX_CSTYLE 
hwcWriteMemoryStream(HwcSimulator *hws, FxU32 startAddress, FxU32 endAddress, FxBool writeAGP)
{
    FxU32 count, addr;
    FxU32 id = writeAGP ? HWC_VEC_MEM_AGP : HWC_VEC_MEM_SGRAM;
    FxU32 memStart = writeAGP ? hwcInfo.agpBaseAddrL : 0;
    FxU32 memSize = writeAGP ? hws->hwc->agpSizeInBytes : hws->hwc->memSizeInBytes;

    GDBG_INFO(2,"hwcWriteMemoryStream(0x%x, startAddress 0x%lx, endAddress 0x%lx, writeAGP %d)\n",
             hws, startAddress, endAddress, writeAGP);

    // check if we need to ouput vector

    if ((!hws->vecInfo.initialized) ||  !hws->vecInfo.module[id].enabled)
       return;

    if ( writeAGP ? !traceAGP : !traceSGRAM )
        return;

    // check we are within memory region

    if (( startAddress < memStart) ||  ( endAddress > (memStart+memSize ))) {
        GDBG_ERROR("hwcWriteMemoryStream",
                    "invalid memory range (0x%lx, 0x%lx) must be within (0x%lx, 0x%lx)\n",
                   startAddress, endAddress, memStart, memStart+memSize);
        return;
    }

    // print the stream name & data (testbench wants word address for base)
    hwcStreamPrintf("// %s starts from @%08lx to  @%08lx\n", writeAGP? "agp.ram" : "sgram.ram", 
                    startAddress>>2, (endAddress>>2)-1);

    // print the vector number if we are writing to multiple files
    if ( !(hws->vecInfo.control & HWC_VECCTL_ONE_FILE) )
         hwcStreamPrintf("%05ld ", hws->vecInfo.nextVector);

    hwcStreamPrintf("%s @%08lx\\\n", writeAGP? "agp.ram" : "sgram.ram", startAddress>>2);
    for ( addr = startAddress&~0x3, count = 0; addr < endAddress; addr += 4, count++) {
        FxU32 data;
        if (( count > 0 ) && ( count % 8 ) == 0 ) {
            hwcStreamPrintf(" \\\n");
            hwcStreamFlush(hws, id);
        }
        // use backdoor memory read to ensure endianess is handled correctly
        data = (hws->ReadMemory)(hws, addr, 32, writeAGP, hws->hwc->dev->traceChip);
        hwcStreamPrintf(" %08lx", data);
    }
    
    hwcStreamPrintf("\n");
    hwcStreamFlush(hws, id);
    hws->vecInfo.module[id].nextVector++; 
    hws->vecInfo.nextVector++;
}

FX_EXPORT void FX_CSTYLE 
hwcWriteVector(HwcSimulator *hws, 
               FxU32 id, FxU32 detail, const char *format, ...)
{
    va_list args;
    HwcTransInfo *tInfo;
    FxBool cont;
    int i;

    if (!hws->vecInfo.initialized)
       return;

    cont = ((id & HWC_VEC_CONTINUE ) != 0);
    id &= HWC_VEC_ID_MASK;

    // check if we need to ouput vector

    if ( !hws->vecInfo.module[id].enabled || ( detail > hws->vecInfo.module[id].detail))
       return;
     
    if (( hws->vecInfo.module[id].breakPoint >= 0 ) && 
        ( hws->vecInfo.module[id].nextVector == (FxU32)hws->vecInfo.module[id].breakPoint  ) &&
        ( !cont && (detail == 0 ))) {
        hwcInvokeDebugger();
    }

    if ( detail > 0 )
        hwcStreamPrintf("// ");
    else if ( !cont && !(hws->vecInfo.control & HWC_VECCTL_ONE_FILE) )
        hwcStreamPrintf("%05ld ", hws->vecInfo.nextVector);

    if ( hws->GetTransInfo && (( tInfo = (* hws->GetTransInfo)(hws)) != NULL )) {
        hwcStreamPrintf("0x%08lx 0x%08lx ", tInfo->tValid, tInfo->tTaken );
    }

    va_start( args, format );
    i = vsprintf( outBuff+outCount, format, args );
    va_end( args );
 
    if ( i >= 0 )
      outCount += i;
    else {
      GDBG_ERROR("hwcWriteVector","Error writing vector\n");
      return;
    }
 
    if ( outCount >= OUTBUFF_SIZE ) {
        GDBG_ERROR("hwcWriteVector", "Maximum line length exceeded\n");
        return;
    }

    hwcStreamFlush(hws, id);

    if ( !cont && (detail == 0 )) {
        hws->vecInfo.nextVector++;
        hws->vecInfo.module[id].nextVector++; 
    }

    if (( hws->vecInfo.nextVector % 100 ) == 0)
        hws->environment.valid = 0;
}

/*---------------------------------------------------------------------------
   Open up a generic window
  ---------------------------------------------------------------------------*/

HwcWindow *
hwcNewWindow(HwcSimulator *hws, char *className, const char *name, int w, int h)
{
  HwcWindow *cw;

  if (!hws->environment.videoEnabled) return NULL;

  cw = HWC_NEW(HwcWindow);
  cw->next = NULL;
  cw->hws = hws;
  cw->buffer = NULL;
  cw->zoom = 1;
  cw->videoFilter = 0;
  cw->hex = 0;
  cw->statusBar = 1;
  cw->fileSaveName[0] = '\0';
  cw->clientWindow = FXFALSE;

#ifdef HWC_GDI
  hwcGDIRealizeWindow(cw, className, name, w, h);
#endif

#ifdef HWC_TCL
  hwcTCLRealizeWindow(cw, className, name, w, h);
#endif

  hws->hWndCount++;
  
  GDBG_INFO(102,"hwcNewWindow(%s, %d,%s), %d windows open\n", 
            hws->name, hws->hwc->bn, name, hws->hWndCount);
  return cw;
}

FX_EXPORT FxBool FX_CSTYLE 
hwcClientWindowSetBuffer(void *hWnd, HwcBuffer *pBuff)
{
#ifdef HWC_GDI
    if (!hwcGDIWindowSetBuffer(hWnd, NULL, pBuff))
        return FXFALSE;
#endif

#ifdef HWC_TCL
    if (!hwcTCLWindowSetBuffer(hWnd, NULL, pBuff))
        return FXFALSE;
#endif
    return FXTRUE;
}

FxBool 
hwcWindowSetBuffer(HwcWindow *pWnd, HwcBuffer *pBuff)
{
  pWnd->buffer = pBuff;
#ifdef HWC_GDI
    if (!hwcGDIWindowSetBuffer(NULL, pWnd, pBuff))
        return FXFALSE;
#endif

#ifdef HWC_TCL
    if (!hwcTCLWindowSetBuffer(NULL, pWnd, pBuff))
        return FXFALSE;
#endif
    return FXTRUE;
}

/*---------------------------------------------------------------------------
   Open up a CSIM view window
  ---------------------------------------------------------------------------*/
FX_EXPORT HwcWindow * FX_CSTYLE 
hwcNewSimViewWindow(HwcSimulator *hws, HwcBuffer *pBuff)
{
  HwcContext *hwc = pBuff->hwc;
  HwcWindow *cw;

  if (!hws->environment.videoEnabled) return NULL;
    GDBG_INFO(102,"hwcNewSimViewWindow(%s, %d,%s)\n", hws->name, hwc->bn, pBuff->name);
  cw = hwcNewWindow(hws, szViewClassName, pBuff->name,
                    pBuff->width, pBuff->height);
  cw->hws = hws;
  cw->SetBuffer = hwcWindowSetBuffer;
  cw->next = hws->windows;                     /* link into window list */
  hws->windows = cw;

  (* cw->SetBuffer)(cw, pBuff); 
  (* hws->AddViewWindow)(hws, cw);
  return cw;
}

/*---------------------------------------------------------------------------
   Open up a view window for each simulator which has video enabled
  ---------------------------------------------------------------------------*/
FX_EXPORT HwcWindow * FX_CSTYLE 
hwcNewViewWindow(HwcBuffer *pBuff)
{
  HwcContext *hwc = pBuff->hwc;
  HwcSimulator *hws;
  HwcWindow *pWindow = NULL;

  for ( hws = hwc->hws; hws; hws = hws->next ) {
     if (!hws->environment.videoEnabled)
         continue;

     pWindow = hwcNewSimViewWindow(hws, pBuff);
  }
  return pWindow;
}

FX_EXPORT HwcWindow * FX_CSTYLE 
hwcNewClientWindow(void *hWnd, HwcBuffer *pBuff)
{
  HwcContext *hwc = pBuff->hwc;
  HwcSimulator *hws = hwc->hws;
  HwcWindow *cw;

  if (!hws->environment.videoEnabled) return NULL;
    GDBG_INFO(102,"hwcNewClientWindow(%s, %d,%s)\n", hws->name, hwc->bn, pBuff->name);

  cw = HWC_NEW(HwcWindow);
  cw->hWnd = hWnd;
  cw->next = NULL;
  cw->hws = hws;
  cw->buffer = pBuff;
  cw->zoom = 1;
  cw->videoFilter = 0;
  cw->hex = 0;
  cw->statusBar = 0;
  cw->fileSaveName[0] = '\0';
  cw->clientWindow = FXTRUE;

#ifdef HWC_GDI
  hwcGDIRealizeWindow(cw, NULL, pBuff->name, pBuff->width, pBuff->height);
#endif

#ifdef HWC_TCL
  hwcTCLRealizeWindow(cw, NULL, pBuff->name, pBuff->width, pBuff->height);
#endif

  hws->hWndCount++;
  
  GDBG_INFO(102,"hwcNewWindow(%s, %d,%s), %d windows open\n", 
            hws->name, hws->hwc->bn, pBuff->name, hws->hWndCount);
  
  cw->hws = hws;
  cw->next = hws->windows;                     /* link into window list */
  hwc->hws->windows = cw;
  cw->SetBuffer = hwcWindowSetBuffer;
  (* cw->SetBuffer)(cw, pBuff); 
  return cw;
}

/*---------------------------------------------------------------------------
   apply a function to all view windows attached to a device
  ---------------------------------------------------------------------------*/
FX_EXPORT void FX_CSTYLE 
hwcApplyToAllWindows(HwcSimulator *hws, void (FX_CALL *func)(HwcWindow *))
{
  HwcWindow *cw;

  for (cw = hws->windows; cw; cw = cw->next)
    func(cw);
}


/*---------------------------------------------------------------------------
   redisplay the contents of the window
  ---------------------------------------------------------------------------*/
FX_EXPORT void FX_CSTYLE 
hwcRefreshWindow(HwcWindow *cw)
{
  GDBG_INFO(7,"hwcRefreshWindow(0x%x)\n",cw);

#ifdef HWC_GDI
  hwcGDIRefreshWindow(cw);
#endif

#ifdef HWC_TCL
  hwcTCLRefreshWindow(cw);
#endif
}

/*---------------------------------------------------------------------------
   redisplay a rectangle of pixels in a buffer
  ---------------------------------------------------------------------------*/

void 
_hwcGUIUpdateRect(HwcSimulator *hws, int xmin, int ymin, int w, int h, FxU32 addr, FxU32 memType, HwcBufferType which)
{
#ifdef HWC_GDI
  _hwcGDIUpdateRect(hws, xmin, ymin, w, h, addr, memType, which);
#endif

#ifdef HWC_TCL
  hwcTCLUpdateRect(hws, xmin, ymin, w, h, addr, memType, which);
#endif

  _hwcGUIKeepAlive(hws, w*h);
}

/*---------------------------------------------------------------------------
   redisplay the contents of all the windows attached to a device
  ---------------------------------------------------------------------------*/
FX_EXPORT void FX_CSTYLE 
hwcRefresh(HwcSimulator *hws)
{
  hwcApplyToAllWindows(hws,hwcRefreshWindow);
}

FX_EXPORT void FX_CSTYLE 
hwcRefreshAll(HwcContext *hwc) 
{ 
  HwcSimulator *hws;

  for (hws = hwc->hws; hws; hws = hws->next) {
      hwcRefresh(hws);
  }
}

/*---------------------------------------------------------------------------
   redisplay the contents of all the windows attached to a device
  ---------------------------------------------------------------------------*/
FX_EXPORT void FX_CSTYLE 
hwcRefreshBuffer(HwcBuffer *pBuff)
{
  HwcContext *hwc = pBuff->hwc;
  HwcSimulator *hws;
  HwcWindow *cw;

  for (hws = hwc->hws; hws; hws = hws->next) {
      for (cw = hws->windows; cw != NULL; cw = cw->next) {
          if ( cw->buffer == pBuff ) {
              hwcRefreshWindow(cw);
          }
      }
  }
}

void
_hwcGUIChange(HwcSimulator *hws, HwcChangeEvent which, FxI32 count)
{
  HwcBuffer *pBuff = hws->hwc->stdBuffers[HWC_BUF_3D_RENDER];

  hws->environment.valid = 0;

  switch (which) {
  case HWC_GUI_END_FRAME:
    hws->stats.framesDrawn++;
    hws->stats.trisDrawnPerFrame = 0;
    hws->stats.pGroupsDrawnPerFrame = 0;
    if (hws->environment.pauseAfterNextSwap ||
        (hws->stats.framesDrawn == hws->environment.pauseAfterSwapNum))
      hwcGUIDoBreak(hws);
    if (hws->environment.resetPixelsIn)
        hws->stats.pixelsIn = 0;
    break;
  case HWC_GUI_BUFFER_SWAP:
    hws->stats.bufferSwaps++;
    break;
  case HWC_GUI_POINTS_DRAWN:
    hws->stats.pointsDrawn += count;
    break;
  case HWC_GUI_LINES_DRAWN:
    hws->stats.linesDrawn += count;
    break;
  case HWC_GUI_TEX_DOWNLOADS:
    hws->stats.texDownloads += count;
    break;
  case HWC_GUI_TEX_BYTES:
    hws->stats.texBytes += count;
    break;
  case HWC_GUI_NCC_DOWNLOADS:
    hws->stats.nccDownloads += count;
    break;
  case HWC_GUI_NCC_BYTES:
    hws->stats.nccBytes += count;
    break;
  case HWC_GUI_PAL_DOWNLOADS:
    hws->stats.palDownloads += count;
    break;
  case HWC_GUI_PAL_BYTES:
    hws->stats.palBytes += count;
    break;
  case HWC_GUI_PIXELS_OUT_2D:
    hws->stats.pixelsOut2D += count;
    break;
  case HWC_GUI_PIXELS_IN:
    {
      static int pixelBreak = 0;

      hws->stats.pGroupsDrawn++;
      hws->stats.pGroupsDrawnPerTri++;
      hws->stats.pGroupsDrawnPerFrame++;
      hws->stats.pixelsIn += count;
      if ( pixelBreak ) {
        hwcGUIDoBreak(hws);
      }
    }
    break;
  case HWC_GUI_CHROMA_FAIL:
    hws->stats.chromaFail += count;
    break;
  case HWC_GUI_ZFUNC_FAIL:
    hws->stats.zFuncFail += count;
    break;
  case HWC_GUI_AFUNC_FAIL:
    hws->stats.aFuncFail += count;
    break;
  case HWC_GUI_PIXELS_OUT:
    hws->stats.pixelsOut += count;
    break;
  case HWC_GUI_TRIS_PROCESSED:
    hws->stats.trisProcessed += count;
    break;
  case HWC_GUI_TRIS_DRAWN:
    hws->stats.trisDrawn += count;
    hws->stats.trisDrawnPerFrame+= count;
    hws->stats.pGroupsDrawnPerTri = 0;
    if (hws->environment.pauseAfterNextTriangle ||
      (hws->stats.trisDrawnPerFrame == hws->environment.pauseAfterTriangleNum))
      hwcGUIDoBreak(hws);
    break;
 case HWC_GUI_STENCIL_FAIL:
    hws->stats.stencilFail += count;
    break;
  case HWC_GUI_STENCIL_OP:
    hws->stats.stencilOp += count;
    break;
  default:
    GDBG_ERROR("hwcGUIChange","invalid parameter %d\n",which);
    break;
    }
}

FX_EXPORT void FX_CSTYLE 
hwcGUIChange(HwcSimulator *hws, HwcChangeEvent which, FxI32 count)
{
  _hwcGUIChange(hws, which, count);
}

/*---------------------------------------------------------------------------
   Close a CSIM window
  ---------------------------------------------------------------------------*/

FX_EXPORT void FX_CSTYLE 
hwcDeleteWindow(HwcWindow *cw)
{
  HwcSimulator *hws = cw->hws;

  if (cw) {
#ifdef HWC_GDI
    hwcGDIUnrealizeWindow(cw);
#endif
#ifdef HWC_TCL
    hwcTCLUnrealizeWindow(cw);
#endif
    if (hws->mainWindow == cw)    /* if it's the main window */
      hws->mainWindow = NULL;
    else if (hws->windows == cw)
           hws->windows = cw->next;
        else
            GDBG_ERROR("hwcDeleteWindow", "window not in front of list\n");

        hwcMemFree(cw);
        hws->hWndCount--;
        GDBG_INFO(102,"hwcDeleteWindow(0x%x), %d windows open\n", cw, hws->hWndCount);
  } else GDBG_ERROR("hwcDeleteWindow", "window not found\n");
}

FX_EXPORT void FX_CSTYLE 
hwcDeleteAllWindows(HwcSimulator *hws)
{
  while (hws->windows)
      hwcDeleteWindow(hws->windows);
}

/*---------------------------------------------------------------------------
   shut down the CSIM gui environment
  ---------------------------------------------------------------------------*/

FX_EXPORT void FX_CSTYLE 
hwcGUIShutdown( void )
{
#ifdef HWC_GDI
    hwcGDIShutdown();
#endif

#ifdef HWC_TCL
    hwcTCLShutdown();
#endif
}

FX_EXPORT void FX_CSTYLE 
hwcGUIReadMessageQueue(void)
{
#ifdef HWC_TCL
  hwcTCLReadMessageQueue();
#else
#ifdef HWC_GDI
  hwcGDIReadMessageQueue();
#endif
#endif
}

FX_EXPORT int FX_CSTYLE 
hwcKbHit( void )
{
#ifdef HWC_TCL
  return 0;
#else
#ifdef HWC_GDI
  return hwcGDIKbHit();
#endif
#endif
}

FX_EXPORT int FX_CSTYLE 
hwcGetCH( void )
{
#ifdef HWC_TCL
  return 0;
#else
#ifdef HWC_GDI
  return hwcGDIGetCH();
#endif
#endif
}

/* callback to give to GDEBUG to keep things alive */
void
_hwcGUIKeepAlive(HwcSimulator *hws, int count)
{
  hwcInfo.pollCount -= count;

  if ( hwcInfo.memDebugLevel > 1 )
      hwcMemCheck();
    
  if (hwcInfo.pollCount < 0) {
    hwcInfo.pollCount = hwcInfo.pollLimit;
    hwcGUIReadMessageQueue();
  }
}

FX_EXPORT void FX_CSTYLE 
hwcGUIKeepAlive(int count)
{
  if (hwcInfo.uiMethod == 0) {
    _hwcGUIKeepAlive(NULL, count);
  }
}

/*---------------------------------------------------------------------------
  one time init routine
  ---------------------------------------------------------------------------*/

FxBool 
hwcGUIInit( void )
{
#if !defined(FX_DLL_ENABLE)
  GDBG_INIT();
#endif

#ifdef HWC_GDI
  if (!hwcGDIInit())
    return FXFALSE;
#endif

#ifdef HWC_TCL
  if (!hwcTCLInit())
    return FXFALSE;
#endif

  gdbg_set_keepalive(hwcGUIKeepAlive);

#ifdef HWC_TCL
  gdbg_error_set_callback(hwcTCLGdbgErrorCallback);
#else
#ifdef HWC_GDI
  gdbg_error_set_callback(hwcGDIGdbgErrorCallback);
#endif
#endif
  return FXTRUE;
}

/*---------------------------------------------------------------------------
   open a CSIM gui for a board
  ---------------------------------------------------------------------------*/

FX_EXPORT FxBool FX_CSTYLE 
hwcGUIOpen( HwcSimulator *hws)
{
#if !defined(FX_DLL_ENABLE)
  GDBG_INIT();
#endif
  if (!hws->environment.videoEnabled) return FXFALSE;
    GDBG_INFO(101,"hwcGUIOpen(%d)\n", hws->hwc->bn);
#ifdef HWC_GDI
  hwcGDIGUIOpen(hws);
#endif
#ifdef HWC_TCL
  hwcTCLGUIOpen(hws);
#endif
  return FXTRUE;
}

/*---------------------------------------------------------------------------
   shut down the CSIM gui for a board
  ---------------------------------------------------------------------------*/

FX_EXPORT void FX_CSTYLE 
hwcGUIUnmapBoard(HwcSimulator *hws)
{
  HwcWindow *w, *wnext;

  GDBG_INFO(101,"hwcGUIUnmapBoard(0x%x)\n", hws);

  wnext = hws->windows;

  while ((w = wnext) != NULL ) {
    hws->hWndCount--;
    wnext = w->next;
    hwcDeleteWindow(w);
  }

  w = hws->mainWindow;

  if (w) {
    hws->hWndCount--;
    hwcDeleteWindow(w);
  }

  hws->windows = NULL;
  hws->mainWindow = NULL;
}

FX_EXPORT void FX_CSTYLE 
hwcGUIDoBreak(HwcSimulator *hws)
{
#ifdef HWC_TCL
  hwcTCLDoBreak(hws);
#else
#ifdef HWC_GDI
  hwcGDIDoBreak(hws);
#endif
#endif
}

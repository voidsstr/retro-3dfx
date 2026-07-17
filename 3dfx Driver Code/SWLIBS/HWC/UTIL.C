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
** $Revision: 6$
** $Date: 10/11/00 7:40:21 PM$
*/

#include <stdlib.h>

#include <fxhwc.h>
#include <math.h>

// get an environment variable
FX_EXPORT const char * FX_CSTYLE
hwcGetEnv(const char *name)
{
    char *val = NULL;

#ifndef KERNEL
    val = getenv(name);
#endif

    return val;
}

// get and translate a simple numeric environment variable
FX_EXPORT FxI32 FX_CSTYLE
hwcGetEnvI32(const char *name, const FxI32 defaultVal, const char *msg)
{
  int val=defaultVal;
  const char *p;

  if (p = hwcGetEnv(name)) {
    val = atoi(p);
  }

  GDBG_INFO(0, msg, val);
  return val;
}

/*---------------------------------------------------------------------------
    Idle board don't send NOOP
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcIdleNoNop( HwcContext *hwc)
{
  GDBG_INFO(6,"hwcIdleNoNop(0x%x)\n", hwc);

  if(!hwc || ( hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  return (* hwc->IdleNoNop)(hwc);
}

/*---------------------------------------------------------------------------
    Idle board
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcIdle( HwcContext *hwc )
{
  GDBG_INFO(6,"hwcIdle(0x%x)\n", hwc);

  if(!hwc || ( hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  return (* hwc->Idle)(hwc);

  return FXTRUE;
}

/*---------------------------------------------------------------------------
    Wait for vsync
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcVsync( HwcContext *hwc)
{
  GDBG_INFO(6,"hwcVsync(0x%x)\n", hwc);

  if (!hwc || ( hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  return (* hwc->Vsync)(hwc);
}

/*---------------------------------------------------------------------------
    Wait for not vsync
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcVsyncNot( HwcContext *hwc)
{
  FxU32 cntr = 0;

  GDBG_INFO(6,"hwcVsyncNot(0x%x)\n", hwc);

  if(!hwc || ( hwc->state != HWC_CTX_MAPPED))
      return(FXFALSE);

#ifdef HAL_HSIM 
  if ( hwcInfo.hsim  ) { 
    while(1) {
      if(!(IGET(sst->status) & SST_VRETRACE)) {
        if(++cntr >= 3)
	    break;
      } else
        cntr = 0;
    }
  }
#endif

  return (* hwc->VsyncNot)(hwc);
}

/*---------------------------------------------------------------------------
    Initialize command FIFO
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcInitCmdFifo( HwcContext *hwc, int which, FxU32 fifoStart, FxU32 size, 
                FxBool disableHoles, FxBool agpEnable)
{
  GDBG_INFO(1,"hwcInitCmdFifo(0x%x,fifo=%d,start=0x%x,size=0x%x(%d),\n\t\t\t,disHoles=%d,agpEnable=%d)\n",
		hwc,which,fifoStart,size,size, disableHoles,agpEnable);

  gdbg_info(2,"CMD FIFO placed at physical addr 0x%x\n", fifoStart);

  return (* hwc->InitCmdFifo)(hwc, which, fifoStart, size, 
                              disableHoles, agpEnable);
}

/*---------------------------------------------------------------------------
    Initialize color table
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE 
hwcInitGammaRGB(HwcContext *hwc, FxFloat r, FxFloat g, FxFloat b) 
{
static FxBool calledBefore = FXFALSE;
  FxI32 x;

  GDBG_INFO(6,"hwcInitGammaRGB(0x%x)\n", hwc);

  if (!hwc || ( hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  hwc->gamma.r = r;
  hwc->gamma.g = g;
  hwc->gamma.b = b;

  // Initialize the gamma table
  for(x=0; x<256; x++) {
    hwc->gamma.tableR[x] = (FxU32) (pow(x/255.0F, 1.0F/hwc->gamma.r) * 255.0F + 0.5F);
    hwc->gamma.tableG[x] = (FxU32) (pow(x/255.0F, 1.0F/hwc->gamma.g) * 255.0F + 0.5F);
    hwc->gamma.tableB[x] = (FxU32) (pow(x/255.0F, 1.0F/hwc->gamma.b) * 255.0F + 0.5F);
  }

  return (* hwc->InitGammaRGB)(hwc, r, g, b);
}


/*---------------------------------------------------------------------------
    Initialize color table with uniform ramp
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE 
hwcInitGamma(HwcContext *hwc, FxFloat gamma) {
  GDBG_INFO(6,"hwcInitGamma(0x%x)\n", hwc);

  return hwcInitGammaRGB(hwc, gamma, gamma, gamma);
}


/*---------------------------------------------------------------------------
    Initialize video
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE 
hwcInitVideo(HwcContext *hwc, FxU32 resolution, FxU32 refresh, void *timing)
{
  GDBG_INFO(6,"hwcInitVideo(0x%x)\n", hwc);

  if (!hwc || ( hwc->state != HWC_CTX_MAPPED))
    return(FXFALSE);

  return (* hwc->InitVideo)(hwc, resolution, refresh, timing);
}

/*
**  P6 Fence - all memory IO comes thru here, so this is the only place we need it
**
**  Here's the stuff to do P6 Fencing.  This is required for the
**  certain things on the P6
*/

#if defined(__WATCOMC__)
static FxU32 p6FenceVar;
  void p6Fence(void);
  #pragma aux p6Fence = \
    "xchg eax, p6FenceVar" \
    modify [eax];

  #define P6FENCE p6Fence()

#elif defined(__MSC__)
static FxU32 p6FenceVar;
  #define P6FENCE {_asm xchg eax, p6FenceVar}
#elif defined(__unix__)
  #define P6FENCE
#else
  #error "P6 Fencing in-line assembler code needs to be added for this compiler"
#endif

/*
** hwcRead32():
**  Read 32-bit Word from specified address
**
*/
FX_EXPORT FxU32 FX_CSTYLE 
hwcRead32(FxU32 *addr)
{
    P6FENCE;
    return(GET(*addr));
}

/*
** hwcWrite32():
**  Write 32-bit Word to specified address
**
*/
FX_EXPORT void FX_CSTYLE 
hwcWrite32(FxU32 *addr, FxU32 data)
{
    P6FENCE;
    SET(*addr,data);
    P6FENCE;
}

